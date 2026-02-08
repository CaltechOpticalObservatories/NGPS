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

from PyQt5.QtCore import Qt, pyqtSignal, QSettings, QTimer, QThread
from PyQt5.QtWidgets import (
    QWidget, QVBoxLayout, QHBoxLayout, QTabWidget,
    QPushButton, QLabel, QLineEdit, QTableWidget, QTableWidgetItem,
    QMessageBox, QDialog, QDialogButtonBox, QFormLayout, QScrollArea,
    QMenu, QHeaderView, QProgressDialog, QInputDialog
)
from PyQt5.QtGui import QBrush, QColor


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


# Import OTM integration modules
try:
    from otm_integration import (
        OtmSettings, OtmRunner, OtmTimelineRunner,
        generate_otm_input_csv, parse_otm_output_csv, parse_timeline_json,
        otm_flag_severity, otm_flag_color
    )
    from otm_settings_dialog import OtmSettingsDialog
    OTM_AVAILABLE = True
except ImportError as e:
    print(f"Warning: OTM integration not available: {e}")
    OTM_AVAILABLE = False

# Import record editor dialog
try:
    from record_editor_dialog import RecordEditorDialog
    RECORD_EDITOR_AVAILABLE = True
except ImportError as e:
    print(f"Warning: Record editor dialog not available: {e}")
    RECORD_EDITOR_AVAILABLE = False

# Import coordinate utilities for grouping
try:
    from coordinate_utils import (
        parse_sexagesimal_ra, parse_sexagesimal_dec, gnomonic_projection,
        angular_separation_arcsec, compute_coordinate_key, find_nearest_center,
        DEFAULT_GROUP_TOLERANCE_ARCSEC
    )
    COORDINATE_UTILS_AVAILABLE = True
except ImportError as e:
    print(f"Warning: Coordinate utilities not available: {e}")
    COORDINATE_UTILS_AVAILABLE = False

# Import timeline canvas
try:
    from timeline_canvas import TimelineCanvas
    TIMELINE_CANVAS_AVAILABLE = True
except ImportError as e:
    print(f"Warning: Timeline canvas not available: {e}")
    TIMELINE_CANVAS_AVAILABLE = False


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

        # Grouping state
        self._grouping_enabled = False
        self._expanded_groups: Set[str] = set()  # Set of expanded group keys
        self._manual_ungroup_obs_ids: Set[str] = set()  # Manually ungrouped targets
        self._group_data: Dict[str, List[int]] = {}  # group_key -> [row_indices]

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

        # Grouping button (only for targets table)
        if self._is_targets_table() and COORDINATE_UTILS_AVAILABLE:
            top.addSpacing(12)
            self.group_button = QPushButton("Enable Grouping", self)
            self.group_button.setCheckable(True)
            self.group_button.setToolTip("Group targets by coordinate proximity (1 arcsec tolerance)")
            top.addWidget(self.group_button)
            self.group_button.clicked.connect(self._toggle_grouping)
        else:
            self.group_button = None

        if self._search_column:
            top.addSpacing(12)
            top.addWidget(QLabel(f"Search {self._search_column}:", self))
            self.search_input = QLineEdit(self)
            top.addWidget(self.search_input)
            self.search_input.textChanged.connect(self.refresh)
        else:
            self.search_input = None

        top.addStretch()

        # Bulk operations buttons (right side)
        top.addSpacing(12)
        self.bulk_edit_button = QPushButton("Bulk Edit...", self)
        self.bulk_edit_button.setToolTip("Edit column value for multiple selected rows")
        self.bulk_edit_button.clicked.connect(self._bulk_edit_dialog)
        top.addWidget(self.bulk_edit_button)

        self.column_vis_button = QPushButton("Columns...", self)
        self.column_vis_button.setToolTip("Show/hide columns")
        self.column_vis_button.clicked.connect(self._column_visibility_dialog)
        top.addWidget(self.column_vis_button)

        self.filter_button = QPushButton("Filter...", self)
        self.filter_button.setToolTip("Advanced filtering")
        self.filter_button.clicked.connect(self._advanced_filter_dialog)
        top.addWidget(self.filter_button)

        self.export_csv_button = QPushButton("Export CSV...", self)
        self.export_csv_button.setToolTip("Export table to CSV file")
        self.export_csv_button.clicked.connect(self._export_csv)
        top.addWidget(self.export_csv_button)

        self.import_csv_button = QPushButton("Import CSV...", self)
        self.import_csv_button.setToolTip("Import data from CSV file")
        self.import_csv_button.clicked.connect(self._import_csv)
        top.addWidget(self.import_csv_button)

        layout.addLayout(top)

        # Table widget
        self.table = QTableWidget(self)
        self.table.setEditTriggers(QTableWidget.DoubleClicked | QTableWidget.SelectedClicked)
        self.table.setSelectionBehavior(QTableWidget.SelectRows)
        self.table.setSelectionMode(QTableWidget.SingleSelection)
        self.table.itemChanged.connect(self._on_item_changed)
        self.table.itemSelectionChanged.connect(self._on_selection_changed)
        self.table.cellClicked.connect(self._on_cell_clicked)
        layout.addWidget(self.table, 1)

        # Enable context menu for all tables
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

        # Apply grouping if enabled
        if self._grouping_enabled:
            rows = self._apply_grouping_to_display(rows)

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

    def _on_cell_clicked(self, row: int, column: int) -> None:
        """Handle cell click - check for group header clicks."""
        if not self._grouping_enabled:
            return

        # Get row data
        row_values = self._current_row_values(row)
        if not row_values:
            return

        # Check if this is a group header
        is_header = row_values.get("_IS_GROUP_HEADER", False)
        if not is_header:
            return

        # Toggle expansion
        group_key = row_values.get("_GROUP_KEY", "")
        if not group_key:
            return

        if group_key in self._expanded_groups:
            self._expanded_groups.remove(group_key)
        else:
            self._expanded_groups.add(group_key)

        # Refresh to show/hide members
        self.refresh()

    def _on_add_clicked(self) -> None:
        """Handle add button - open dialog to add new record."""
        if not RECORD_EDITOR_AVAILABLE:
            QMessageBox.information(
                self,
                "Add Record",
                "Record editor dialog not available.\n\n"
                "Adding new records requires the full editor."
            )
            return

        try:
            # Show add dialog
            dialog = RecordEditorDialog(self, self._columns, {}, is_new=True)

            if dialog.exec_() == QDialog.Accepted:
                # Get new record values
                new_values = dialog.get_values()

                # Insert into database
                if not self._db.is_open():
                    QMessageBox.warning(self, "Error", "Database not connected")
                    return

                schema = self._db.get_schema(self._config.schema)
                table = schema.get_table(self._table_name)

                # Build INSERT query
                columns = []
                values = []
                for col in self._columns:
                    # Skip auto-increment columns
                    if col.is_auto_increment:
                        continue

                    col_name = col.name
                    col_val = new_values.get(col_name)

                    columns.append(col_name)
                    values.append(col_val)

                # Execute insert
                table.insert(*columns).values(*values).execute()

                # Refresh table
                self.refresh()

                # Show success message
                if hasattr(self._parent_window, "statusBar"):
                    self._parent_window.statusBar().showMessage("Record added successfully", 3000)

        except Exception as e:
            QMessageBox.critical(self, "Error", f"Failed to add record: {e}")

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
        """Show context menu for target operations."""
        # Get the row at the cursor position
        row = self.table.rowAt(pos.y())
        if row < 0:
            return

        # Select the row
        self.table.selectRow(row)

        # Get row data
        row_values = self._current_row_values(row)
        if not row_values:
            return

        # Create context menu
        menu = QMenu(self)

        # Basic operations (all tables)
        refresh_action = menu.addAction("Refresh")
        refresh_action.triggered.connect(self.refresh)

        # Targets table specific operations
        if self._is_targets_table():
            menu.addSeparator()

            # Reordering operations
            move_up_action = menu.addAction("Move Up")
            move_down_action = menu.addAction("Move Down")
            menu.addSeparator()
            move_top_action = menu.addAction("Move to Top")
            move_bottom_action = menu.addAction("Move to Bottom")

            # Disable if first/last row
            total_rows = self.table.rowCount()
            move_up_action.setEnabled(row > 0)
            move_down_action.setEnabled(row < total_rows - 1)

            # Connect reordering actions
            move_up_action.triggered.connect(lambda: self._move_row(row, -1))
            move_down_action.triggered.connect(lambda: self._move_row(row, 1))
            move_top_action.triggered.connect(lambda: self._move_row_to_position(row, 0))
            move_bottom_action.triggered.connect(lambda: self._move_row_to_position(row, total_rows - 1))

            menu.addSeparator()

            # Edit operations
            duplicate_action = menu.addAction("Duplicate")
            duplicate_action.triggered.connect(lambda: self._duplicate_row(row))

            edit_action = menu.addAction("Edit...")
            edit_action.triggered.connect(lambda: self._edit_row_dialog(row))

            # ETC operation
            menu.addSeparator()
            etc_action = menu.addAction("Calculate Exposure Time (ETC)...")
            etc_action.setToolTip("Calculate exposure time using ETC for selected target(s)")
            etc_action.triggered.connect(lambda: self._launch_etc_dialog())

            # Grouping operations (if grouping enabled)
            if self._grouping_enabled and COORDINATE_UTILS_AVAILABLE:
                menu.addSeparator()

                obs_id = row_values.get("OBSERVATION_ID", "")
                is_ungrouped = obs_id in self._manual_ungroup_obs_ids

                if is_ungrouped:
                    regroup_action = menu.addAction("Regroup")
                    regroup_action.setToolTip("Restore automatic grouping for this target")
                    regroup_action.triggered.connect(lambda: self._regroup_target(obs_id))
                else:
                    ungroup_action = menu.addAction("Ungroup")
                    ungroup_action.setToolTip("Remove this target from automatic grouping")
                    ungroup_action.triggered.connect(lambda: self._ungroup_target(obs_id))

            menu.addSeparator()

            # Delete operation
            delete_action = menu.addAction("Delete")
            delete_action.triggered.connect(lambda: self._delete_row_with_confirmation(row))

        # Show menu at cursor position
        menu.exec_(self.table.viewport().mapToGlobal(pos))

    def _move_row(self, row: int, delta: int) -> None:
        """Move row up (-1) or down (+1)."""
        target_row = row + delta
        if target_row < 0 or target_row >= self.table.rowCount():
            return

        try:
            # Get OBS_ORDER values for both rows
            obs_order_col = self._column_index.get("OBS_ORDER")
            obs_id_col = self._column_index.get("OBSERVATION_ID")

            if obs_order_col is None or obs_id_col is None:
                return

            current_obs_id = self.table.item(row, obs_id_col).text()
            target_obs_id = self.table.item(target_row, obs_id_col).text()
            current_order = int(self.table.item(row, obs_order_col).text())
            target_order = int(self.table.item(target_row, obs_order_col).text())

            # Swap OBS_ORDER values in database
            if not self._db.is_open():
                QMessageBox.warning(self, "Error", "Database not connected")
                return

            schema = self._db.get_schema(self._config.schema)
            table = schema.get_table(self._table_name)

            # Update both rows
            table.update().where("OBSERVATION_ID = :oid").set("OBS_ORDER", target_order).bind("oid", current_obs_id).execute()
            table.update().where("OBSERVATION_ID = :oid").set("OBS_ORDER", current_order).bind("oid", target_obs_id).execute()

            # Refresh view
            self.refresh()

            # Reselect the moved row
            self.table.selectRow(target_row)

        except Exception as e:
            QMessageBox.critical(self, "Error", f"Failed to move row: {e}")

    def _move_row_to_position(self, row: int, target_position: int) -> None:
        """Move row to specific position (0=top, n-1=bottom)."""
        if row == target_position:
            return

        try:
            obs_order_col = self._column_index.get("OBS_ORDER")
            obs_id_col = self._column_index.get("OBSERVATION_ID")

            if obs_order_col is None or obs_id_col is None:
                return

            current_obs_id = self.table.item(row, obs_id_col).text()

            if not self._db.is_open():
                QMessageBox.warning(self, "Error", "Database not connected")
                return

            schema = self._db.get_schema(self._config.schema)
            table = schema.get_table(self._table_name)

            # Get all rows sorted by current OBS_ORDER
            if self._fixed_filter_col and self._fixed_filter_val is not None:
                results = table.select().where(f"{self._fixed_filter_col} = :val").bind("val", self._fixed_filter_val).order_by("OBS_ORDER").execute()
            else:
                results = table.select().order_by("OBS_ORDER").execute()

            rows_data = list(results)

            # Find current row and reorder
            current_idx = -1
            for i, r in enumerate(rows_data):
                if str(r["OBSERVATION_ID"]) == current_obs_id:
                    current_idx = i
                    break

            if current_idx < 0:
                return

            # Remove and reinsert at new position
            moved_row = rows_data.pop(current_idx)
            rows_data.insert(target_position, moved_row)

            # Update all OBS_ORDER values
            for new_order, r in enumerate(rows_data):
                table.update().where("OBSERVATION_ID = :oid").set("OBS_ORDER", new_order).bind("oid", r["OBSERVATION_ID"]).execute()

            # Refresh view
            self.refresh()

            # Reselect the moved row
            self.table.selectRow(target_position)

        except Exception as e:
            QMessageBox.critical(self, "Error", f"Failed to move row: {e}")

    def _duplicate_row(self, row: int) -> None:
        """Duplicate the selected row with new OBSERVATION_ID."""
        try:
            row_values = self._current_row_values(row)
            if not row_values:
                return

            if not self._db.is_open():
                QMessageBox.warning(self, "Error", "Database not connected")
                return

            schema = self._db.get_schema(self._config.schema)
            table = schema.get_table(self._table_name)

            # Remove OBSERVATION_ID (will be auto-generated)
            if "OBSERVATION_ID" in row_values:
                del row_values["OBSERVATION_ID"]

            # Increment OBS_ORDER
            if "OBS_ORDER" in row_values:
                try:
                    current_order = int(row_values["OBS_ORDER"])
                    row_values["OBS_ORDER"] = str(current_order + 1)
                except (ValueError, TypeError):
                    pass

            # Insert new row
            columns = []
            values = []
            for col_name, col_val in row_values.items():
                if col_name in self._column_by_name:
                    columns.append(col_name)
                    # Convert empty strings to None for nullable columns
                    if col_val == "" and self._column_by_name[col_name].nullable:
                        values.append(None)
                    else:
                        values.append(col_val)

            if columns:
                table.insert(*columns).values(*values).execute()

            # Refresh view
            self.refresh()

            # Show success message
            QMessageBox.information(self, "Success", "Row duplicated successfully")

        except Exception as e:
            QMessageBox.critical(self, "Error", f"Failed to duplicate row: {e}")

    def _edit_row_dialog(self, row: int) -> None:
        """Open full edit dialog for the row."""
        if not RECORD_EDITOR_AVAILABLE:
            QMessageBox.information(
                self,
                "Edit Row",
                "Record editor dialog not available.\n\nPlease double-click cells to edit inline."
            )
            return

        try:
            # Get current row values
            row_values = self._current_row_values(row)
            if not row_values:
                return

            # Show edit dialog
            dialog = RecordEditorDialog(self, self._columns, row_values, is_new=False)

            if dialog.exec_() == QDialog.Accepted:
                # Get updated values
                new_values = dialog.get_values()

                # Update database
                if not self._db.is_open():
                    QMessageBox.warning(self, "Error", "Database not connected")
                    return

                schema = self._db.get_schema(self._config.schema)
                table = schema.get_table(self._table_name)

                # Build WHERE clause using primary keys
                where_parts = []
                bind_vals = {}
                for col in self._columns:
                    if col.is_primary:
                        col_val = row_values.get(col.name, "")
                        where_parts.append(f"{col.name} = :{col.name}")
                        bind_vals[col.name] = col_val

                if not where_parts:
                    QMessageBox.warning(self, "Error", "Cannot determine primary key")
                    return

                where_clause = " AND ".join(where_parts)

                # Build UPDATE query
                query = table.update().where(where_clause)

                # Set all non-primary-key columns
                for col in self._columns:
                    if not col.is_primary and not col.is_auto_increment:
                        col_name = col.name
                        new_val = new_values.get(col_name)
                        query = query.set(col_name, new_val)

                # Bind primary key values
                for key, val in bind_vals.items():
                    query = query.bind(key, val)

                # Execute update
                query.execute()

                # Refresh table
                self.refresh()

                # Show success message
                if hasattr(self._parent_window, "statusBar"):
                    self._parent_window.statusBar().showMessage("Record updated successfully", 3000)

        except Exception as e:
            QMessageBox.critical(self, "Error", f"Failed to update record: {e}")

    def _delete_row_with_confirmation(self, row: int) -> None:
        """Delete row after user confirmation."""
        row_values = self._current_row_values(row)
        if not row_values:
            return

        # Get row identifier for confirmation message
        row_id = row_values.get("OBSERVATION_ID", row_values.get("SET_ID", f"row {row + 1}"))

        # Confirm deletion
        reply = QMessageBox.question(
            self,
            "Confirm Delete",
            f"Are you sure you want to delete {row_id}?\n\nThis action cannot be undone.",
            QMessageBox.Yes | QMessageBox.No,
            QMessageBox.No
        )

        if reply != QMessageBox.Yes:
            return

        try:
            if not self._db.is_open():
                QMessageBox.warning(self, "Error", "Database not connected")
                return

            schema = self._db.get_schema(self._config.schema)
            table = schema.get_table(self._table_name)

            # Build WHERE clause using primary keys
            where_parts = []
            bind_vals = {}
            for col in self._columns:
                if col.is_primary:
                    col_val = row_values.get(col.name, "")
                    where_parts.append(f"{col.name} = :{col.name}")
                    bind_vals[col.name] = col_val

            if not where_parts:
                QMessageBox.warning(self, "Error", "Cannot determine primary key for deletion")
                return

            where_clause = " AND ".join(where_parts)

            # Delete row
            query = table.delete().where(where_clause)
            for key, val in bind_vals.items():
                query = query.bind(key, val)
            query.execute()

            # Refresh view
            self.refresh()

            # Show success message
            status_msg = f"Deleted {row_id}"
            if hasattr(self._parent_window, "statusBar"):
                self._parent_window.statusBar().showMessage(status_msg, 3000)

        except Exception as e:
            QMessageBox.critical(self, "Error", f"Failed to delete row: {e}")

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

    def _toggle_grouping(self, checked: bool) -> None:
        """Toggle grouping on/off."""
        self._grouping_enabled = checked

        if self.group_button:
            self.group_button.setText("Disable Grouping" if checked else "Enable Grouping")

        # Refresh to apply/remove grouping
        self.refresh()

    def _compute_groups(self, rows_data: List[Dict[str, Any]]) -> Dict[str, str]:
        """
        Compute group assignments for all targets.

        Args:
            rows_data: List of row dictionaries from database

        Returns:
            Dictionary mapping OBSERVATION_ID -> group_key
        """
        if not COORDINATE_UTILS_AVAILABLE:
            return {}

        group_assignments = {}
        science_centers = []  # List of (key, ra_deg, dec_deg)

        # First pass: Identify science centers (targets with science exposures and coordinates)
        for row in rows_data:
            obs_id = str(row.get("OBSERVATION_ID", ""))
            name = str(row.get("NAME", ""))
            ra_str = str(row.get("RA", ""))
            dec_str = str(row.get("DECL", ""))
            offset_ra = float(row.get("OFFSET_RA", 0.0) or 0.0)
            offset_dec = float(row.get("OFFSET_DEC", 0.0) or 0.0)

            # Skip if manually ungrouped
            if obs_id in self._manual_ungroup_obs_ids:
                continue

            # Skip calibration targets (no RA/DEC or name starts with CAL_)
            if not ra_str or not dec_str or name.upper().startswith("CAL_"):
                continue

            # Parse coordinates
            ra_deg = parse_sexagesimal_ra(ra_str)
            dec_deg = parse_sexagesimal_dec(dec_str)

            if ra_deg is None or dec_deg is None:
                continue

            # Apply gnomonic projection if offsets present
            if offset_ra != 0.0 or offset_dec != 0.0:
                ra_deg, dec_deg = gnomonic_projection(ra_deg, dec_deg, offset_ra, offset_dec)

            # Generate coordinate key
            coord_key = compute_coordinate_key(ra_deg, dec_deg, DEFAULT_GROUP_TOLERANCE_ARCSEC)

            # Add to science centers if not already present
            if coord_key not in [c[0] for c in science_centers]:
                science_centers.append((coord_key, ra_deg, dec_deg))

        # Second pass: Assign all targets to groups
        for row in rows_data:
            obs_id = str(row.get("OBSERVATION_ID", ""))
            name = str(row.get("NAME", ""))
            ra_str = str(row.get("RA", ""))
            dec_str = str(row.get("DECL", ""))
            offset_ra = float(row.get("OFFSET_RA", 0.0) or 0.0)
            offset_dec = float(row.get("OFFSET_DEC", 0.0) or 0.0)

            # Manually ungrouped
            if obs_id in self._manual_ungroup_obs_ids:
                group_assignments[obs_id] = f"UNGROUP:{obs_id}"
                continue

            # Calibration target (no coordinates)
            if not ra_str or not dec_str or name.upper().startswith("CAL_"):
                group_assignments[obs_id] = f"CAL:{obs_id}"
                continue

            # Parse coordinates
            ra_deg = parse_sexagesimal_ra(ra_str)
            dec_deg = parse_sexagesimal_dec(dec_str)

            if ra_deg is None or dec_deg is None:
                group_assignments[obs_id] = f"OBS:{obs_id}"
                continue

            # Apply gnomonic projection if offsets present
            if offset_ra != 0.0 or offset_dec != 0.0:
                ra_deg, dec_deg = gnomonic_projection(ra_deg, dec_deg, offset_ra, offset_dec)

            # Find nearest science center
            nearest_key = find_nearest_center(
                ra_deg, dec_deg, science_centers, DEFAULT_GROUP_TOLERANCE_ARCSEC
            )

            if nearest_key:
                group_assignments[obs_id] = nearest_key
            else:
                # Create singleton group
                group_assignments[obs_id] = f"OBS:{obs_id}"

        return group_assignments

    def _apply_grouping_to_display(self, rows_data: List[Dict[str, Any]]) -> List[Dict[str, Any]]:
        """
        Apply grouping to rows before display.

        Groups rows by coordinate proximity and adds group header rows.
        Only shows members of expanded groups.

        Args:
            rows_data: Original rows from database

        Returns:
            Modified rows list with group headers and filtered members
        """
        if not self._grouping_enabled or not rows_data:
            return rows_data

        # Compute group assignments
        group_assignments = self._compute_groups(rows_data)

        # Group rows by assignment
        groups: Dict[str, List[Dict[str, Any]]] = {}
        for row in rows_data:
            obs_id = str(row.get("OBSERVATION_ID", ""))
            group_key = group_assignments.get(obs_id, f"OBS:{obs_id}")

            if group_key not in groups:
                groups[group_key] = []
            groups[group_key].append(row)

        # Build display list with headers
        display_rows = []

        for group_key, group_rows in groups.items():
            # Determine if group is expanded
            is_expanded = group_key in self._expanded_groups

            # Add group header row (first row in group)
            if group_rows:
                header_row = group_rows[0].copy()

                # Add expand/collapse indicator to NAME column
                name_val = str(header_row.get("NAME", ""))
                icon = "▼" if is_expanded else "▶"
                header_row["NAME"] = f"{icon} {name_val} [{len(group_rows)}]"
                header_row["_IS_GROUP_HEADER"] = True
                header_row["_GROUP_KEY"] = group_key

                display_rows.append(header_row)

                # Add group members if expanded
                if is_expanded and len(group_rows) > 1:
                    for member_row in group_rows[1:]:
                        # Indent member names
                        member_copy = member_row.copy()
                        name_val = str(member_copy.get("NAME", ""))
                        member_copy["NAME"] = f"  {name_val}"
                        member_copy["_IS_GROUP_MEMBER"] = True
                        member_copy["_GROUP_KEY"] = group_key
                        display_rows.append(member_copy)

        return display_rows

    def _ungroup_target(self, obs_id: str) -> None:
        """Manually ungroup a target."""
        if obs_id:
            self._manual_ungroup_obs_ids.add(obs_id)
            self.refresh()

    def _regroup_target(self, obs_id: str) -> None:
        """Restore automatic grouping for a target."""
        if obs_id in self._manual_ungroup_obs_ids:
            self._manual_ungroup_obs_ids.remove(obs_id)
            self.refresh()

    def _launch_etc_dialog(self) -> None:
        """Launch ETC dialog for selected target(s)."""
        try:
            from etc_popup import EtcPopup
        except ImportError:
            QMessageBox.warning(
                self,
                "ETC Not Available",
                "ETC dialog is not available."
            )
            return

        # Get selected rows
        selected_rows = self.table.selectionModel().selectedRows()
        if not selected_rows:
            QMessageBox.warning(
                self,
                "No Selection",
                "Please select one or more targets to calculate exposure time."
            )
            return

        # Get observation IDs
        obs_id_col = self._column_index.get("OBSERVATION_ID")
        if obs_id_col is None:
            QMessageBox.warning(self, "Error", "OBSERVATION_ID column not found")
            return

        observation_ids = []
        for index in selected_rows:
            row = index.row()
            item = self.table.item(row, obs_id_col)
            if item:
                observation_ids.append(item.text())

        if not observation_ids:
            return

        # Get initial values from first selected row (for pre-filling form)
        first_row = selected_rows[0].row()
        initial_values = self._current_row_values(first_row)

        # Get logic service from parent
        logic_service = None
        if hasattr(self._parent_window, 'logic_service'):
            logic_service = self._parent_window.logic_service

        # Launch ETC dialog
        dialog = EtcPopup(
            parent=self,
            logic_service=logic_service,
            initial_values=initial_values,
            observation_ids=observation_ids
        )

        # If dialog is accepted, refresh table to show updated values
        if dialog.exec_() == QDialog.Accepted:
            self.refresh()

    def _bulk_edit_dialog(self) -> None:
        """Launch bulk edit dialog for selected rows."""
        try:
            from bulk_operations_dialog import BulkEditDialog
        except ImportError:
            QMessageBox.warning(
                self,
                "Feature Not Available",
                "Bulk edit dialog is not available."
            )
            return

        # Get selected rows
        selected_rows = self.table.selectionModel().selectedRows()
        if not selected_rows:
            QMessageBox.warning(
                self,
                "No Selection",
                "Please select one or more rows to bulk edit."
            )
            return

        # Ask user which column to edit
        column_names = list(self._column_index.keys())
        column_name, ok = QInputDialog.getItem(
            self,
            "Select Column",
            "Choose column to bulk edit:",
            column_names,
            0,
            False
        )

        if not ok or not column_name:
            return

        # Get column metadata
        column_meta = None
        for col in self._columns:
            if col.name == column_name:
                column_meta = col
                break

        if not column_meta:
            QMessageBox.warning(self, "Error", f"Column metadata not found for {column_name}")
            return

        # Launch bulk edit dialog
        dialog = BulkEditDialog(
            parent=self,
            column_name=column_name,
            column_type=column_meta.type,
            num_rows=len(selected_rows)
        )

        if dialog.exec_() != QDialog.Accepted:
            return

        # Apply the bulk edit operation
        operation = dialog.get_operation()
        self._apply_bulk_edit(selected_rows, column_name, operation)

    def _apply_bulk_edit(self, selected_rows, column_name: str, operation: Dict[str, Any]) -> None:
        """Apply bulk edit operation to selected rows."""
        if not self._db.is_open():
            QMessageBox.warning(self, "Error", "Database not connected")
            return

        try:
            schema = self._db.get_schema(self._config.schema)
            table = schema.get_table(self._table_name)

            # Get primary key column
            pk_col = None
            pk_col_name = None
            for col in self._columns:
                if col.is_primary:
                    pk_col = self._column_index.get(col.name)
                    pk_col_name = col.name
                    break

            if pk_col is None:
                QMessageBox.warning(self, "Error", "Primary key column not found")
                return

            # Process each selected row
            updates = 0
            for index in selected_rows:
                row = index.row()
                pk_item = self.table.item(row, pk_col)
                if not pk_item:
                    continue

                pk_value = pk_item.text()

                # Determine new value based on operation
                new_value = None

                if operation["type"] == "set":
                    new_value = operation["value"]
                elif operation["type"] == "null":
                    new_value = None
                elif operation["type"] == "append":
                    # Get current value and append
                    current_item = self.table.item(row, self._column_index[column_name])
                    if current_item:
                        current_value = current_item.text()
                        new_value = current_value + operation["value"]
                elif operation["type"] == "prepend":
                    # Get current value and prepend
                    current_item = self.table.item(row, self._column_index[column_name])
                    if current_item:
                        current_value = current_item.text()
                        new_value = operation["value"] + current_value
                elif operation["type"] == "add":
                    # Get current value and add
                    current_item = self.table.item(row, self._column_index[column_name])
                    if current_item:
                        try:
                            current_value = float(current_item.text())
                            add_value = float(operation["value"])
                            new_value = str(current_value + add_value)
                        except ValueError:
                            continue
                elif operation["type"] == "multiply":
                    # Get current value and multiply
                    current_item = self.table.item(row, self._column_index[column_name])
                    if current_item:
                        try:
                            current_value = float(current_item.text())
                            mult_value = float(operation["value"])
                            new_value = str(current_value * mult_value)
                        except ValueError:
                            continue

                # Update database
                if new_value is None:
                    table.update().set(column_name, None).where(f"{pk_col_name} = '{pk_value}'").execute()
                else:
                    table.update().set(column_name, new_value).where(f"{pk_col_name} = '{pk_value}'").execute()

                updates += 1

            QMessageBox.information(
                self,
                "Bulk Edit Complete",
                f"Successfully updated {updates} rows."
            )

            # Refresh table
            self.refresh()

        except Exception as e:
            QMessageBox.critical(
                self,
                "Bulk Edit Failed",
                f"Error applying bulk edit:\n{e}"
            )

    def _column_visibility_dialog(self) -> None:
        """Launch column visibility dialog."""
        try:
            from bulk_operations_dialog import ColumnVisibilityDialog
        except ImportError:
            QMessageBox.warning(
                self,
                "Feature Not Available",
                "Column visibility dialog is not available."
            )
            return

        # Get current visible columns (all columns are visible by default)
        visible_columns = set(self._column_index.keys())

        # Launch dialog
        dialog = ColumnVisibilityDialog(
            parent=self,
            columns=list(self._column_index.keys()),
            visible_columns=visible_columns
        )

        if dialog.exec_() != QDialog.Accepted:
            return

        # Apply column visibility changes
        new_visible = dialog.get_visible_columns()

        for col_name, col_index in self._column_index.items():
            if col_name in new_visible:
                self.table.showColumn(col_index)
            else:
                self.table.hideColumn(col_index)

    def _advanced_filter_dialog(self) -> None:
        """Launch advanced filter dialog."""
        try:
            from bulk_operations_dialog import AdvancedFilterDialog
        except ImportError:
            QMessageBox.warning(
                self,
                "Feature Not Available",
                "Advanced filter dialog is not available."
            )
            return

        # Launch dialog
        dialog = AdvancedFilterDialog(
            parent=self,
            columns=list(self._column_index.keys())
        )

        if dialog.exec_() != QDialog.Accepted:
            return

        filters = dialog.get_filters()

        if not filters:
            # Clear filters - just refresh to show all rows
            self.refresh()
            return

        # Apply filters by hiding rows that don't match
        self._apply_filters(filters)

    def _apply_filters(self, filters: List[Dict[str, Any]]) -> None:
        """Apply filter conditions to table rows."""
        # Show all rows first
        for row in range(self.table.rowCount()):
            self.table.showRow(row)

        # Apply each filter (AND logic - all must match)
        for row in range(self.table.rowCount()):
            matches = True

            for f in filters:
                col_index = self._column_index.get(f["column"])
                if col_index is None:
                    continue

                item = self.table.item(row, col_index)
                cell_value = item.text() if item else ""

                # Check filter condition
                operator = f["operator"]
                filter_value = f["value"]

                if operator == "equals":
                    if cell_value != filter_value:
                        matches = False
                        break
                elif operator == "not equals":
                    if cell_value == filter_value:
                        matches = False
                        break
                elif operator == "contains":
                    if filter_value.lower() not in cell_value.lower():
                        matches = False
                        break
                elif operator == "starts with":
                    if not cell_value.lower().startswith(filter_value.lower()):
                        matches = False
                        break
                elif operator == "ends with":
                    if not cell_value.lower().endswith(filter_value.lower()):
                        matches = False
                        break
                elif operator == "greater than":
                    try:
                        if float(cell_value) <= float(filter_value):
                            matches = False
                            break
                    except ValueError:
                        matches = False
                        break
                elif operator == "less than":
                    try:
                        if float(cell_value) >= float(filter_value):
                            matches = False
                            break
                    except ValueError:
                        matches = False
                        break
                elif operator == "is NULL":
                    if cell_value.strip() != "":
                        matches = False
                        break
                elif operator == "is not NULL":
                    if cell_value.strip() == "":
                        matches = False
                        break

            # Hide row if it doesn't match all filters
            if not matches:
                self.table.hideRow(row)

    def _export_csv(self) -> None:
        """Export table data to CSV file."""
        try:
            from bulk_operations_dialog import export_to_csv
        except ImportError:
            QMessageBox.warning(
                self,
                "Feature Not Available",
                "CSV export is not available."
            )
            return

        # Get file path from user
        file_path, _ = QFileDialog.getSaveFileName(
            self,
            "Export to CSV",
            f"{self._table_name}.csv",
            "CSV Files (*.csv);;All Files (*)"
        )

        if not file_path:
            return

        # Collect column names
        columns = list(self._column_index.keys())

        # Collect visible rows (respecting filters)
        rows = []
        for row in range(self.table.rowCount()):
            if self.table.isRowHidden(row):
                continue

            row_data = []
            for col_idx in range(self.table.columnCount()):
                item = self.table.item(row, col_idx)
                row_data.append(item.text() if item else "")

            rows.append(row_data)

        # Export
        success = export_to_csv(file_path, columns, rows)

        if success:
            QMessageBox.information(
                self,
                "Export Successful",
                f"Exported {len(rows)} rows to:\n{file_path}"
            )
        else:
            QMessageBox.critical(
                self,
                "Export Failed",
                "Failed to export data to CSV."
            )

    def _import_csv(self) -> None:
        """Import data from CSV file."""
        try:
            from bulk_operations_dialog import import_from_csv
        except ImportError:
            QMessageBox.warning(
                self,
                "Feature Not Available",
                "CSV import is not available."
            )
            return

        # Get file path from user
        file_path, _ = QFileDialog.getOpenFileName(
            self,
            "Import from CSV",
            "",
            "CSV Files (*.csv);;All Files (*)"
        )

        if not file_path:
            return

        # Import data
        result = import_from_csv(file_path)

        if not result:
            QMessageBox.critical(
                self,
                "Import Failed",
                "Failed to import data from CSV."
            )
            return

        csv_columns, csv_rows = result

        # Confirm import
        reply = QMessageBox.question(
            self,
            "Confirm Import",
            f"Import {len(csv_rows)} rows with {len(csv_columns)} columns?\n\n"
            f"Columns: {', '.join(csv_columns[:5])}{'...' if len(csv_columns) > 5 else ''}\n\n"
            f"This will INSERT new rows into the database.",
            QMessageBox.Yes | QMessageBox.No,
            QMessageBox.No
        )

        if reply != QMessageBox.Yes:
            return

        # Perform import
        self._perform_csv_import(csv_columns, csv_rows)

    def _perform_csv_import(self, csv_columns: List[str], csv_rows: List[List[Any]]) -> None:
        """Perform CSV import into database."""
        if not self._db.is_open():
            QMessageBox.warning(self, "Error", "Database not connected")
            return

        try:
            schema = self._db.get_schema(self._config.schema)
            table = schema.get_table(self._table_name)

            # Map CSV columns to database columns
            db_columns = {col.name for col in self._columns}
            valid_columns = [col for col in csv_columns if col in db_columns]

            if not valid_columns:
                QMessageBox.warning(
                    self,
                    "Import Failed",
                    "No matching columns found between CSV and database table."
                )
                return

            # Insert rows
            inserted = 0
            for row_data in csv_rows:
                # Build insert values dict
                values = {}
                for i, col_name in enumerate(csv_columns):
                    if col_name in valid_columns and i < len(row_data):
                        value = row_data[i]
                        # Convert empty strings to None for nullable columns
                        if value == "":
                            value = None
                        values[col_name] = value

                if values:
                    table.insert(valid_columns).values([values[col] for col in valid_columns]).execute()
                    inserted += 1

            QMessageBox.information(
                self,
                "Import Successful",
                f"Successfully imported {inserted} rows."
            )

            # Refresh table
            self.refresh()

        except Exception as e:
            QMessageBox.critical(
                self,
                "Import Failed",
                f"Error importing data:\n{e}"
            )


class DatabaseTab(QWidget):
    """Main database tab widget with target sets and targets."""

    def __init__(self, parent: QWidget, db_connection: Any, main_window: QWidget = None) -> None:
        super().__init__(parent)
        self._parent_window = main_window or parent
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

        if OTM_AVAILABLE:
            top.addSpacing(12)
            self.otm_settings_btn = QPushButton("OTM Settings...", self)
            self.otm_settings_btn.setToolTip("Configure OTM scheduler settings")
            top.addWidget(self.otm_settings_btn)

            self.run_otm_btn = QPushButton("Run OTM", self)
            self.run_otm_btn.setToolTip("Run Optimal Target scheduler")
            top.addWidget(self.run_otm_btn)

        top.addStretch()
        self.conn_status = QLabel("Not connected", self)
        top.addWidget(self.conn_status)
        layout.addLayout(top)

        # Tab widget for sets and targets
        self.tabs = QTabWidget(self)
        layout.addWidget(self.tabs, 1)

        # Connect signals
        self.activate_set_btn.clicked.connect(self._activate_set)

        if OTM_AVAILABLE:
            self.otm_settings_btn.clicked.connect(self._show_otm_settings)
            self.run_otm_btn.clicked.connect(self._run_otm)

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

            # Add Timeline tab if available
            if TIMELINE_CANVAS_AVAILABLE:
                self.timeline_canvas = TimelineCanvas(self)
                self.timeline_canvas.target_selected.connect(self._on_timeline_target_selected)
                self.tabs.addTab(self.timeline_canvas, "Timeline")
            else:
                self.timeline_canvas = None

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

    def _on_timeline_target_selected(self, obs_id: str) -> None:
        """Handle target selection from timeline."""
        # Update timeline canvas
        if self.timeline_canvas:
            self.timeline_canvas.set_selected_obs_id(obs_id)

        # TODO: Sync selection with targets table
        # This would require finding the row with matching OBSERVATION_ID
        # and selecting it in the targets_table

    def _activate_set(self) -> None:
        """Activate selected target set by sending seq targetset command."""
        # Get selected set from the sets table
        set_selection = self.sets_table._current_row_values(self.sets_table.table.currentRow())
        if not set_selection or not set_selection.get("SET_ID"):
            QMessageBox.warning(
                self,
                "No Selection",
                "Please select a target set from the Target Sets tab first."
            )
            return

        set_id = set_selection.get("SET_ID")
        set_name = set_selection.get("SET_NAME", f"Set {set_id}")

        # Send sequencer command
        main_window = self._parent_window
        if hasattr(main_window, 'sequencer_service') and main_window.sequencer_service:
            try:
                command = f"seq targetset {set_id}"
                main_window.sequencer_service.send_command(command)
                if hasattr(main_window, 'statusBar'):
                    main_window.statusBar().showMessage(f"Activated target set: {set_name} (ID={set_id})", 5000)
            except Exception as e:
                QMessageBox.critical(self, "Error", f"Failed to activate target set:\n{e}")
        else:
            QMessageBox.warning(
                self,
                "Sequencer Not Connected",
                "Sequencer service is not available. Please check the connection."
            )

    def _show_otm_settings(self) -> None:
        """Show OTM settings dialog."""
        if not OTM_AVAILABLE:
            QMessageBox.warning(self, "OTM Not Available", "OTM integration modules not loaded.")
            return

        dialog = OtmSettingsDialog(self)
        dialog.exec_()

    def _run_otm(self) -> None:
        """Run OTM scheduler on selected target set."""
        if not OTM_AVAILABLE:
            QMessageBox.warning(self, "OTM Not Available", "OTM integration modules not loaded.")
            return

        # Get selected set
        set_selection = self.sets_table._current_row_values(self.sets_table.table.currentRow())
        if not set_selection or not set_selection.get("SET_ID"):
            QMessageBox.warning(
                self,
                "No Selection",
                "Please select a target set from the Target Sets tab before running OTM."
            )
            return

        set_id = set_selection.get("SET_ID")
        set_name = set_selection.get("SET_NAME", f"Set {set_id}")

        # Confirm
        reply = QMessageBox.question(
            self,
            "Run OTM",
            f"Run OTM scheduler for target set: {set_name}?\n\n"
            "This will:\n"
            "1. Generate OTM input CSV from targets\n"
            "2. Run OTM scheduler\n"
            "3. Generate timeline JSON\n"
            "4. Import OTM results to database\n\n"
            "Existing OTM results will be overwritten.",
            QMessageBox.Yes | QMessageBox.No,
            QMessageBox.No
        )

        if reply != QMessageBox.Yes:
            return

        # Get start time from user
        start_utc, ok = QInputDialog.getText(
            self,
            "Start Time",
            "Enter start time (UTC) in ISO format:\n\nExample: 2024-02-07T18:00:00",
            QLineEdit.Normal,
            "2024-02-07T18:00:00"
        )

        if not ok or not start_utc.strip():
            return

        try:
            # Load settings
            settings = OtmSettings.load()

            # Validate script paths
            if not settings.otm_script_path or not os.path.exists(settings.otm_script_path):
                QMessageBox.warning(
                    self,
                    "OTM Not Configured",
                    "OTM script path not configured.\n\n"
                    "Please click 'OTM Settings' and configure the script paths."
                )
                return

            if not settings.timeline_script_path or not os.path.exists(settings.timeline_script_path):
                QMessageBox.warning(
                    self,
                    "Timeline Script Not Configured",
                    "Timeline script path not configured.\n\n"
                    "Please click 'OTM Settings' and configure the script paths."
                )
                return

            # Get targets for this set
            targets = self._get_targets_for_otm(set_id)
            if not targets:
                QMessageBox.warning(self, "No Targets", "No targets found in selected set.")
                return

            # Create temporary files
            import tempfile
            temp_dir = tempfile.mkdtemp(prefix="ngps_otm_")
            input_csv = os.path.join(temp_dir, "otm_input.csv")
            output_csv = os.path.join(temp_dir, "otm_output.csv")
            timeline_json = os.path.join(temp_dir, "timeline.json")

            # Generate input CSV
            generate_otm_input_csv(targets, input_csv)

            # Create progress dialog
            progress = QProgressDialog("Running OTM scheduler...", "Cancel", 0, 0, self)
            progress.setWindowModality(Qt.WindowModal)
            progress.setMinimumDuration(0)
            progress.setValue(0)

            # Run OTM in thread
            self._otm_thread = QThread()
            self._otm_runner = OtmRunner(settings, input_csv, output_csv, start_utc)
            self._otm_runner.moveToThread(self._otm_thread)

            # Connect signals
            self._otm_thread.started.connect(self._otm_runner.run)
            self._otm_runner.progress.connect(lambda msg: progress.setLabelText(msg))
            self._otm_runner.finished.connect(
                lambda success, msg: self._on_otm_finished(
                    success, msg, input_csv, output_csv, timeline_json, set_id, start_utc, progress
                )
            )
            self._otm_runner.finished.connect(self._otm_thread.quit)
            progress.canceled.connect(self._otm_runner.cancel)
            progress.canceled.connect(self._otm_thread.quit)

            # Start
            self._otm_thread.start()

        except Exception as e:
            QMessageBox.critical(self, "Error", f"Failed to run OTM: {e}")

    def _on_otm_finished(self, success: bool, message: str, input_csv: str, output_csv: str,
                         timeline_json: str, set_id: str, start_utc: str,
                         progress: QProgressDialog) -> None:
        """Handle OTM completion."""
        if not success:
            progress.cancel()
            QMessageBox.critical(self, "OTM Failed", f"OTM scheduler failed:\n\n{message}")
            return

        # Run timeline generation
        progress.setLabelText("Generating timeline JSON...")
        settings = OtmSettings.load()

        self._timeline_thread = QThread()
        self._timeline_runner = OtmTimelineRunner(
            settings, input_csv, output_csv, timeline_json, start_utc
        )
        self._timeline_runner.moveToThread(self._timeline_thread)

        # Connect signals
        self._timeline_thread.started.connect(self._timeline_runner.run)
        self._timeline_runner.progress.connect(lambda msg: progress.setLabelText(msg))
        self._timeline_runner.finished.connect(
            lambda success, msg, json_path: self._on_timeline_finished(
                success, msg, output_csv, json_path, set_id, progress
            )
        )
        self._timeline_runner.finished.connect(self._timeline_thread.quit)
        progress.canceled.connect(self._timeline_runner.cancel)
        progress.canceled.connect(self._timeline_thread.quit)

        # Start
        self._timeline_thread.start()

    def _on_timeline_finished(self, success: bool, message: str, output_csv: str,
                              json_path: str, set_id: str, progress: QProgressDialog) -> None:
        """Handle timeline generation completion."""
        progress.cancel()

        if not success:
            QMessageBox.critical(self, "Timeline Failed", f"Timeline generation failed:\n\n{message}")
            return

        # Import OTM results to database
        try:
            progress.setLabelText("Importing OTM results to database...")
            progress.show()

            otm_results = parse_otm_output_csv(output_csv)
            self._import_otm_results(set_id, otm_results)

            progress.cancel()

            # Refresh tables
            self.targets_table.refresh()

            # Load timeline visualization
            if self.timeline_canvas and TIMELINE_CANVAS_AVAILABLE:
                try:
                    timeline_data = parse_timeline_json(json_path)
                    self.timeline_canvas.set_data(timeline_data)

                    # Switch to timeline tab
                    for i in range(self.tabs.count()):
                        if self.tabs.widget(i) == self.timeline_canvas:
                            self.tabs.setCurrentIndex(i)
                            break

                except Exception as e:
                    print(f"Warning: Failed to load timeline visualization: {e}")

            QMessageBox.information(
                self,
                "OTM Complete",
                f"OTM scheduler completed successfully!\n\n"
                f"Processed {len(otm_results)} targets.\n"
                f"Timeline JSON: {json_path}\n\n"
                f"Results have been imported to the database.\n"
                f"Timeline visualization is now available in the Timeline tab."
            )

        except Exception as e:
            progress.cancel()
            QMessageBox.critical(self, "Import Failed", f"Failed to import OTM results:\n\n{e}")

    def _get_targets_for_otm(self, set_id: str) -> List[Dict[str, Any]]:
        """Get all targets for a set in OTM format."""
        if not self._db.is_open():
            return []

        try:
            schema = self._db.get_schema(self._config.schema)
            table = schema.get_table(self._config.table_targets)

            # Query targets for this set, ordered by OBS_ORDER
            results = table.select().where(f"SET_ID = :sid").bind("sid", set_id).order_by("OBS_ORDER").execute()

            targets = []
            for row in results:
                target = {}
                for col in row.keys():
                    target[col] = row[col]
                targets.append(target)

            return targets

        except Exception as e:
            print(f"Error fetching targets for OTM: {e}")
            return []

    def _import_otm_results(self, set_id: str, results: List) -> None:
        """Import OTM results back to database."""
        if not self._db.is_open():
            raise RuntimeError("Database not connected")

        try:
            schema = self._db.get_schema(self._config.schema)
            table = schema.get_table(self._config.table_targets)

            # Update each target with OTM results
            for result in results:
                obs_id = result.observation_id

                # Build update
                updates = {
                    "OTMstart": result.otmstart if result.otmstart else None,
                    "OTMend": result.otmend if result.otmend else None,
                    "OTMslewgo": result.otmslewgo if result.otmslewgo else None,
                    "OTMflag": result.otmflag if result.otmflag else None,
                    "OTMwait": result.otmwait,
                    "OTMdead": result.otmdead,
                    "OTMslew": result.otmslew,
                    "OBS_ORDER": result.obs_order
                }

                # Execute update
                query = table.update().where("OBSERVATION_ID = :oid AND SET_ID = :sid")
                for key, val in updates.items():
                    query = query.set(key, val)
                query.bind("oid", obs_id).bind("sid", set_id).execute()

        except Exception as e:
            raise RuntimeError(f"Failed to import OTM results: {e}")
