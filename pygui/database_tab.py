"""
Database Tab for NGPS GUI

Integrates the enhanced database GUI functionality from tools/ngps_db_gui.py
into the main pygui application as a dedicated tab.

Features:
- State persistence (column widths, scroll position, window geometry)
- Data normalization system
- Transaction support
- Scroll position preservation during refresh
"""

import os
import sys
import re
import math
from dataclasses import dataclass, field
from typing import Any, Dict, List, Optional, Tuple, Set

try:
    import mysqlx
except ImportError:
    print("Warning: mysqlx module not installed. Database functionality will be limited.")
    mysqlx = None

from PyQt5.QtCore import Qt, pyqtSignal, QSettings, QTimer
from PyQt5.QtWidgets import (
    QWidget, QVBoxLayout, QHBoxLayout, QTabWidget,
    QPushButton, QLabel, QLineEdit, QTableWidget, QTableWidgetItem,
    QMessageBox, QDialog, QDialogButtonBox, QFormLayout, QScrollArea,
    QMenu, QHeaderView
)


# Settings
SETTINGS_ORG = "NGPS"
SETTINGS_APP = "ngps_gui_database"

# Import normalization functions and data structures from parent directory tools
parent_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, parent_dir)

try:
    from tools.ngps_db_gui import (
        ColumnMeta, DbConfig, NormalizationResult, ViewState,
        normalize_target_row, channel_range_nm, default_wrange_for_channel,
        normalize_channel_value, normalize_exptime_value, normalize_slitwidth_value,
        extract_set_numeric, load_config_file, detect_default_config_path,
        DEFAULT_WRANGE_HALF_WIDTH_NM, DEFAULT_EXPTIME, DEFAULT_SLITWIDTH,
        DEFAULT_SLITANGLE, DEFAULT_MAGSYSTEM, DEFAULT_MAGFILTER, DEFAULT_CHANNEL,
        DEFAULT_POINTMODE, DEFAULT_CCDMODE, DEFAULT_NOTBEFORE, DEFAULT_MAGNITUDE,
        DEFAULT_AIRMASS_MAX, DEFAULT_BIN, DEFAULT_OTM_SLITWIDTH, DEFAULT_TARGET_STATE
    )
except ImportError:
    # Fallback if imports fail - define minimal versions
    @dataclass
    class ColumnMeta:
        name: str
        type: str
        nullable: bool
        key: str
        default: Any
        extra: str

        @staticmethod
        def _to_str(value: Any) -> str:
            if value is None:
                return ""
            return str(value)

        @property
        def is_primary(self) -> bool:
            return "PRI" in self._to_str(self.key)

        @property
        def is_auto_increment(self) -> bool:
            return "auto_increment" in self._to_str(self.extra).lower()

    @dataclass
    class ViewState:
        v_scroll: int = 0
        h_scroll: int = 0
        current_row: int = -1
        current_obs_id: str = ""

    def normalize_target_row(values: Dict[str, Any]) -> Any:
        return type('obj', (object,), {'changed_columns': []})()


class DbClient:
    """Database client with transaction support."""

    def __init__(self) -> None:
        self._session = None
        self._in_transaction = False

    def connect(self, cfg: Any) -> None:
        """Connect to database using existing configuration."""
        self.close()
        self._session = mysqlx.get_session({
            "host": cfg.host,
            "port": cfg.port,
            "user": cfg.user,
            "password": cfg.password,
            "compression": "DISABLED",
        })
        self._session.sql(f"USE `{cfg.schema}`").execute()

    def close(self) -> None:
        if self._session:
            try:
                self._session.close()
            except Exception:
                pass
        self._session = None
        self._in_transaction = False

    def is_open(self) -> bool:
        return self._session is not None and self._session.is_open()

    def start_transaction(self) -> None:
        if not self.is_open():
            raise RuntimeError("Database not connected")
        if not self._in_transaction:
            self._session.start_transaction()
            self._in_transaction = True

    def commit(self) -> None:
        if self._in_transaction:
            self._session.commit()
            self._in_transaction = False

    def rollback(self) -> None:
        if self._in_transaction:
            self._session.rollback()
            self._in_transaction = False

    def _escape_value(self, value: Any) -> str:
        if value is None:
            return "NULL"
        if isinstance(value, bool):
            return "1" if value else "0"
        if isinstance(value, (int, float)):
            return str(value)
        text = str(value)
        text = text.replace("\\", "\\\\").replace("'", "''")
        return f"'{text}'"

    def _format_sql(self, sql: str, params: Optional[Tuple[Any, ...]]) -> str:
        if not params:
            return sql
        parts = sql.split("%s")
        if len(parts) - 1 != len(params):
            raise ValueError("SQL parameter count mismatch")
        out = parts[0]
        for idx, part in enumerate(parts[1:]):
            out += self._escape_value(params[idx]) + part
        return out

    def _execute(self, sql: str, params: Optional[Tuple[Any, ...]] = None):
        if not self.is_open():
            raise RuntimeError("Database session not open")
        final_sql = self._format_sql(sql, params)
        return self._session.sql(final_sql).execute()

    def _column_names_from_result(self, res) -> List[str]:
        try:
            cols_meta = res.get_columns()
            if cols_meta:
                names = []
                for col in cols_meta:
                    try:
                        names.append(col.get_column_name())
                    except Exception:
                        names.append(col.get_column_label())
                return names
        except Exception:
            pass
        try:
            return list(res.get_column_names())
        except Exception:
            return []

    def fetch_columns(self, table: str) -> List[ColumnMeta]:
        res = self._execute(f"SHOW COLUMNS FROM `{table}`")
        col_names = self._column_names_from_result(res)
        if not col_names:
            raise RuntimeError("Unable to read column metadata")
        rows = res.fetch_all()
        cols: List[ColumnMeta] = []
        for row in rows:
            row_dict = {col_names[i]: row[i] for i in range(len(col_names))}
            cols.append(ColumnMeta(
                name=ColumnMeta._to_str(row_dict.get("Field", "")),
                type=ColumnMeta._to_str(row_dict.get("Type", "")),
                nullable=ColumnMeta._to_str(row_dict.get("Null", "")).upper() == "YES",
                key=ColumnMeta._to_str(row_dict.get("Key", "")),
                default=row_dict.get("Default"),
                extra=ColumnMeta._to_str(row_dict.get("Extra", "")),
            ))
        return cols

    def fetch_rows(
        self,
        table: str,
        where: str = "",
        params: Optional[Tuple[Any, ...]] = None,
        order_by: str = "",
    ) -> List[Dict[str, Any]]:
        sql = f"SELECT * FROM `{table}`"
        if where:
            sql += f" WHERE {where}"
        if order_by:
            sql += f" ORDER BY {order_by}"
        res = self._execute(sql, tuple(params) if params else None)
        col_names = self._column_names_from_result(res)
        if not col_names:
            return []
        rows = res.fetch_all()
        out: List[Dict[str, Any]] = []
        for row in rows:
            row_dict = {col_names[i]: row[i] for i in range(len(col_names))}
            out.append(row_dict)
        return out

    def insert_record(self, table: str, values: Dict[str, Any]) -> None:
        keys = list(values.keys())
        placeholders = ", ".join(["%s"] * len(keys))
        sql = f"INSERT INTO `{table}` ({', '.join(['`%s`' % k for k in keys])}) VALUES ({placeholders})"
        self._execute(sql, tuple(values[k] for k in keys))

    def update_record(
        self,
        table: str,
        key_values: Dict[str, Any],
        updates: Dict[str, Any],
    ) -> None:
        set_clause = ", ".join([f"`{k}`=%s" for k in updates.keys()])
        where_clause = " AND ".join([f"`{k}`=%s" for k in key_values.keys()])
        sql = f"UPDATE `{table}` SET {set_clause} WHERE {where_clause}"
        self._execute(sql, tuple(updates.values()) + tuple(key_values.values()))

    def delete_record(self, table: str, key_values: Dict[str, Any]) -> None:
        where_clause = " AND ".join([f"`{k}`=%s" for k in key_values.keys()])
        sql = f"DELETE FROM `{table}` WHERE {where_clause}"
        self._execute(sql, tuple(key_values.values()))

    def update_obs_order(
        self,
        table: str,
        key_cols: List[str],
        ordered_keys: List[Dict[str, Any]],
    ) -> None:
        """Batch update OBS_ORDER using CASE statement."""
        if not ordered_keys:
            return

        if len(key_cols) == 1:
            key = key_cols[0]
            cases = []
            ids = []
            for idx, key_vals in enumerate(ordered_keys, start=1):
                val = key_vals[key]
                cases.append(f"WHEN %s THEN %s")
                ids.append(val)

            case_expr = " ".join(cases)
            in_clause = ", ".join(["%s"] * len(ids))
            sql = f"UPDATE `{table}` SET `OBS_ORDER` = CASE `{key}` {case_expr} END WHERE `{key}` IN ({in_clause})"

            params = []
            for idx, key_vals in enumerate(ordered_keys, start=1):
                params.extend([key_vals[key], idx])
            params.extend(ids)

            self._execute(sql, tuple(params))
            return

        raise ValueError("Unsupported key column count for OBS_ORDER update.")


class DatabaseTableWidget(QWidget):
    """Enhanced table widget with state persistence and normalization."""

    selection_changed = pyqtSignal(dict)

    def __init__(
        self,
        title: str,
        parent: QWidget,
        db_client: DbClient,
        config: Any,
        table_name: str,
        allow_reorder: bool = False,
        search_column: Optional[str] = None,
    ) -> None:
        super().__init__(parent)
        self._title = title
        self._db = db_client
        self._config = config
        self._table_name = table_name
        self._columns: List[ColumnMeta] = []
        self._column_by_name: Dict[str, ColumnMeta] = {}
        self._column_index: Dict[str, int] = {}
        self._loading = False
        self._hidden_columns: List[str] = []
        self._allow_reorder = allow_reorder
        self._search_column = search_column
        self._order_by: str = ""
        self._fixed_filter_col: Optional[str] = None
        self._fixed_filter_val: Optional[Any] = None
        self._row_keys: List[Dict[str, Any]] = []
        self._settings_key_prefix = title.lower().replace(" ", "_")
        self._column_width_save_timer = QTimer(self)
        self._column_width_save_timer.setSingleShot(True)
        self._column_width_save_timer.timeout.connect(self._save_column_widths)

        self._init_ui()
        self._setup_database()

    def _init_ui(self) -> None:
        """Initialize UI components."""
        layout = QVBoxLayout(self)

        # Top toolbar
        top = QHBoxLayout()
        self.add_button = QPushButton("Add", self)
        self.delete_button = QPushButton("Delete", self)
        self.refresh_button = QPushButton("Refresh", self)
        top.addWidget(self.add_button)
        top.addWidget(self.delete_button)
        top.addWidget(self.refresh_button)

        if self._search_column:
            top.addSpacing(12)
            top.addWidget(QLabel(f"Search {self._search_column}:", self))
            self.search_input = QLineEdit(self)
            top.addWidget(self.search_input)
            self.search_input.textChanged.connect(self.refresh)
        else:
            self.search_input = None

        top.addStretch()
        layout.addLayout(top)

        # Table widget
        self.table = QTableWidget(self)
        self.table.setEditTriggers(QTableWidget.DoubleClicked | QTableWidget.SelectedClicked)
        self.table.setSelectionBehavior(QTableWidget.SelectRows)
        self.table.setSelectionMode(QTableWidget.SingleSelection)
        self.table.itemChanged.connect(self._on_item_changed)
        self.table.itemSelectionChanged.connect(self._on_selection_changed)
        layout.addWidget(self.table, 1)

        if self._allow_reorder:
            self.table.setContextMenuPolicy(Qt.CustomContextMenu)
            self.table.customContextMenuRequested.connect(self._show_context_menu)

        # Connect signals
        self.refresh_button.clicked.connect(self.refresh)
        self.add_button.clicked.connect(self._on_add_clicked)
        self.delete_button.clicked.connect(self._on_delete_clicked)

    def _setup_database(self) -> None:
        """Setup database connection and load schema."""
        if not self._db or not self._db.is_open():
            return

        self._columns = self._db.fetch_columns(self._table_name)
        self._column_by_name = {c.name: c for c in self._columns}
        self._column_index = {c.name: idx for idx, c in enumerate(self._columns)}

        self.table.setColumnCount(len(self._columns))
        self.table.setHorizontalHeaderLabels([c.name for c in self._columns])

        # Connect column resize signal
        header = self.table.horizontalHeader()
        header.sectionResized.connect(self._on_column_resized)

        # Restore column widths
        self._restore_column_widths()

    def set_hidden_columns(self, columns: List[str]) -> None:
        """Set which columns should be hidden."""
        self._hidden_columns = [c.upper() for c in columns]
        self._apply_hidden_columns()

    def set_order_by(self, column: str) -> None:
        """Set default sort column."""
        self._order_by = column

    def set_fixed_filter(self, col: Optional[str], value: Optional[Any]) -> None:
        """Set a fixed filter (e.g., filter by SET_ID)."""
        self._fixed_filter_col = col
        self._fixed_filter_val = value
        self.refresh()

    def refresh(self) -> None:
        """Refresh table data with state preservation."""
        if not self._db or not self._db.is_open():
            return

        # Save view state
        view_state = self._save_view_state()

        # Build query
        where_clauses = []
        params: List[Any] = []
        if self._fixed_filter_col is not None and self._fixed_filter_val is not None:
            where_clauses.append(f"`{self._fixed_filter_col}`=%s")
            params.append(self._fixed_filter_val)
        if self._search_column and self.search_input:
            text = self.search_input.text().strip()
            if text:
                where_clauses.append(f"LOWER(`{self._search_column}`) LIKE %s")
                params.append(f"%{text.lower()}%")

        where = " AND ".join(where_clauses)
        order_by = self._order_by or ""
        rows = self._db.fetch_rows(self._table_name, where=where, params=tuple(params), order_by=order_by)

        # Populate table
        self._loading = True
        self.table.setRowCount(0)
        self._row_keys = []

        for row_idx, row in enumerate(rows):
            self.table.insertRow(row_idx)
            key_vals: Dict[str, Any] = {}
            for col_idx, col in enumerate(self._columns):
                val = row.get(col.name)
                text = "" if val is None else str(val)
                item = QTableWidgetItem(text)
                if col.is_auto_increment or col.is_primary:
                    item.setFlags(item.flags() & ~Qt.ItemIsEditable)
                if text == "":
                    item.setData(Qt.UserRole, None)
                else:
                    item.setData(Qt.UserRole, text)
                self.table.setItem(row_idx, col_idx, item)
                if col.is_primary:
                    key_vals[col.name] = row.get(col.name)
            self._row_keys.append(key_vals)

        self._apply_hidden_columns()
        self._loading = False

        # Restore view state
        self._restore_view_state(view_state)

    def _save_view_state(self) -> ViewState:
        """Save current scroll position and selection."""
        state = ViewState()
        state.v_scroll = self.table.verticalScrollBar().value()
        state.h_scroll = self.table.horizontalScrollBar().value()
        state.current_row = self.table.currentRow()

        if state.current_row >= 0 and state.current_row < len(self._row_keys):
            key_vals = self._row_keys[state.current_row]
            state.current_obs_id = str(key_vals.get("OBSERVATION_ID", ""))

        return state

    def _restore_view_state(self, state: ViewState) -> None:
        """Restore scroll position and selection."""
        # Try to restore by OBSERVATION_ID
        if state.current_obs_id:
            for row_idx, key_vals in enumerate(self._row_keys):
                if str(key_vals.get("OBSERVATION_ID", "")) == state.current_obs_id:
                    self.table.setCurrentCell(row_idx, 0)
                    state.current_row = row_idx
                    break

        # Fallback to row index
        if state.current_row >= 0 and state.current_row < self.table.rowCount():
            self.table.setCurrentCell(state.current_row, 0)

        # Restore scroll (delayed)
        QTimer.singleShot(0, lambda: self._restore_scroll(state))

    def _restore_scroll(self, state: ViewState) -> None:
        """Restore scroll bars."""
        self.table.verticalScrollBar().setValue(state.v_scroll)
        self.table.horizontalScrollBar().setValue(state.h_scroll)

    def _save_column_widths(self) -> None:
        """Save column widths to QSettings."""
        settings = QSettings(SETTINGS_ORG, SETTINGS_APP)
        widths = {}
        for col_idx, col in enumerate(self._columns):
            widths[col.name] = self.table.columnWidth(col_idx)
        settings.setValue(f"{self._settings_key_prefix}/columnWidths", widths)

    def _restore_column_widths(self) -> None:
        """Restore column widths from QSettings."""
        settings = QSettings(SETTINGS_ORG, SETTINGS_APP)
        widths = settings.value(f"{self._settings_key_prefix}/columnWidths", {})
        if isinstance(widths, dict):
            for col_idx, col in enumerate(self._columns):
                if col.name in widths:
                    try:
                        width = int(widths[col.name])
                        self.table.setColumnWidth(col_idx, width)
                    except (ValueError, TypeError):
                        pass

    def _on_column_resized(self, logical_index: int, old_size: int, new_size: int) -> None:
        """Debounced save of column widths."""
        self._column_width_save_timer.start(200)

    def _apply_hidden_columns(self) -> None:
        """Apply hidden column settings."""
        for col_idx, col in enumerate(self._columns):
            hidden = col.name.upper() in self._hidden_columns
            self.table.setColumnHidden(col_idx, hidden)

    def _on_item_changed(self, item: QTableWidgetItem) -> None:
        """Handle item edit with normalization."""
        if self._loading:
            return
        if not self._db or not self._db.is_open():
            return

        row = item.row()
        col_idx = item.column()
        col_meta = self._columns[col_idx]
        if col_meta.is_auto_increment or col_meta.is_primary:
            return

        new_text = item.text().strip()
        old_text = item.data(Qt.UserRole)
        if old_text is None:
            old_text = ""
        if new_text == old_text:
            return

        if new_text == "" or new_text.upper() == "NULL":
            if not col_meta.nullable:
                QMessageBox.critical(self, "Error", f"{col_meta.name} cannot be NULL.")
                self._loading = True
                item.setText(str(old_text))
                self._loading = False
                return
            new_value = None
        else:
            new_value = new_text

        # Apply normalization for target table
        if self._is_targets_table():
            row_values = self._current_row_values(row)
            row_values[col_meta.name] = new_value if new_value is not None else ""
            norm_result = normalize_target_row(row_values)

            if norm_result.changed_columns:
                # Show status message
                parent_window = self.window()
                if hasattr(parent_window, 'statusBar'):
                    parent_window.statusBar().showMessage(norm_result.message, 5000)

                # Update all changed columns
                try:
                    for changed_col in norm_result.changed_columns:
                        if changed_col in row_values:
                            update_val = row_values[changed_col]
                            key_vals = self._row_keys[row]
                            self._db.update_record(self._table_name, key_vals, {changed_col: update_val})
                            if changed_col in self._column_index:
                                changed_col_idx = self._column_index[changed_col]
                                self._loading = True
                                self.table.item(row, changed_col_idx).setText(str(update_val))
                                self._loading = False
                except Exception as exc:
                    QMessageBox.critical(self, "Error", f"Normalization failed: {exc}")
                return

        key_vals = self._row_keys[row]
        try:
            self._db.update_record(self._table_name, key_vals, {col_meta.name: new_value})
            item.setData(Qt.UserRole, "" if new_value is None else str(new_value))
        except Exception as exc:
            QMessageBox.critical(self, "Error", f"Update failed: {exc}")
            self._loading = True
            item.setText(str(old_text))
            self._loading = False

    def _on_selection_changed(self) -> None:
        """Handle selection change."""
        values = self._current_row_values(self.table.currentRow())
        if values:
            self.selection_changed.emit(values)

    def _on_add_clicked(self) -> None:
        """Handle add button (placeholder)."""
        QMessageBox.information(self, "Add", "Add functionality will be implemented next.")

    def _on_delete_clicked(self) -> None:
        """Handle delete button."""
        row = self.table.currentRow()
        if row < 0:
            return
        if QMessageBox.question(self, "Delete", "Delete selected row?",
                                QMessageBox.Yes | QMessageBox.No) != QMessageBox.Yes:
            return
        key_vals = self._row_keys[row]
        try:
            self._db.delete_record(self._table_name, key_vals)
        except Exception as exc:
            QMessageBox.critical(self, "Error", f"Delete failed: {exc}")
            return
        self.refresh()

    def _show_context_menu(self, pos) -> None:
        """Show context menu (placeholder)."""
        pass

    def _current_row_values(self, row: int) -> Dict[str, Any]:
        """Get values for a specific row."""
        if row < 0:
            return {}
        values: Dict[str, Any] = {}
        for col_name, idx in self._column_index.items():
            item = self.table.item(row, idx)
            values[col_name] = item.text() if item else ""
        return values

    def _is_targets_table(self) -> bool:
        """Check if this is the targets table."""
        return "OBSERVATION_ID" in self._column_by_name and "SET_ID" in self._column_by_name


class DatabaseTab(QWidget):
    """Main database tab widget with target sets and targets."""

    def __init__(self, parent: QWidget, db_connection: Any) -> None:
        super().__init__(parent)
        self._parent_window = parent
        self._db_connection = db_connection
        self._db = DbClient()
        self._config = None

        self._init_ui()
        self._init_database()

    def _init_ui(self) -> None:
        """Initialize UI components."""
        layout = QVBoxLayout(self)

        # Top toolbar
        top = QHBoxLayout()
        self.activate_set_btn = QPushButton("Activate Target Set", self)
        top.addWidget(self.activate_set_btn)
        top.addStretch()
        self.conn_status = QLabel("Not connected", self)
        top.addWidget(self.conn_status)
        layout.addLayout(top)

        # Tab widget for sets and targets
        self.tabs = QTabWidget(self)
        layout.addWidget(self.tabs, 1)

        # Connect signals
        self.activate_set_btn.clicked.connect(self._activate_set)

    def _init_database(self) -> None:
        """Initialize database connection."""
        # Try to load config
        config_path = detect_default_config_path()
        if not config_path:
            self.conn_status.setText("Config not found")
            return

        self._config = load_config_file(config_path)
        if not self._config or not self._config.is_complete():
            self.conn_status.setText("Incomplete config")
            return

        try:
            self._db.connect(self._config)
            self.conn_status.setText(
                f"Connected: {self._config.user}@{self._config.host}:{self._config.port}/{self._config.schema}"
            )

            # Create table widgets
            self.sets_table = DatabaseTableWidget(
                "Target Sets",
                self,
                self._db,
                self._config,
                self._config.table_sets,
                allow_reorder=False,
                search_column=None
            )
            self.targets_table = DatabaseTableWidget(
                "Targets",
                self,
                self._db,
                self._config,
                self._config.table_targets,
                allow_reorder=True,
                search_column="NAME"
            )

            # Setup tables
            self.sets_table.set_order_by("SET_ID")
            self.targets_table.set_order_by("OBS_ORDER")
            self.targets_table.set_hidden_columns([
                "OBSERVATION_ID", "SET_ID", "OBS_ORDER",
                "TARGET_NUMBER", "SEQUENCE_NUMBER", "SLITOFFSET", "OBSMODE"
            ])

            # Add to tabs
            self.tabs.addTab(self.sets_table, "Target Sets")
            self.tabs.addTab(self.targets_table, "Targets")

            # Connect signals
            self.sets_table.selection_changed.connect(self._on_set_selected)

            # Initial refresh
            self.sets_table.refresh()
            self.targets_table.refresh()

        except Exception as exc:
            self.conn_status.setText(f"Connection failed: {exc}")
            QMessageBox.critical(self, "Database Error", f"Failed to connect: {exc}")

    def _on_set_selected(self, values: Dict[str, Any]) -> None:
        """Handle target set selection."""
        set_id = values.get("SET_ID")
        if set_id is not None and set_id != "":
            self.targets_table.set_fixed_filter("SET_ID", set_id)

    def _activate_set(self) -> None:
        """Activate selected target set."""
        # This will be connected to sequencer commands
        QMessageBox.information(self, "Activate Set", "Sequencer integration coming soon.")
