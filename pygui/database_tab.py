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
import subprocess
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
    QMenu, QHeaderView, QProgressDialog, QInputDialog, QCheckBox
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
        OtmSettings, OtmRunner, OtmTimelineRunner, parse_timeline_json,
        generate_otm_input_csv, parse_otm_output_csv,
        otm_flag_severity, otm_flag_color, OTM_CSV_TO_DB
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

# Import QWebEngineView and QWebChannel for Plotly timeline display
try:
    from PyQt5.QtWebEngineWidgets import QWebEngineView
    from PyQt5.QtWebChannel import QWebChannel
    WEBENGINE_AVAILABLE = True
except ImportError as e:
    print(f"Warning: QWebEngineView not available: {e}")
    WEBENGINE_AVAILABLE = False

# Import timeline bridge and Plotly generator
try:
    from timeline_bridge import TimelineBridge
    from plotly_timeline import generate_timeline_html
    TIMELINE_AVAILABLE = True
except ImportError as e:
    print(f"Warning: Plotly timeline not available: {e}")
    TIMELINE_AVAILABLE = False


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
    data_changed = pyqtSignal()
    write_requested = pyqtSignal(dict, dict)    # (key_values, updates) for centralized DB write
    views_need_refresh = pyqtSignal()            # DB was written internally, all views need refresh

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
        top.addWidget(self.add_button)
        top.addWidget(self.delete_button)

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
            self.search_input.setMaximumWidth(150)
            top.addWidget(self.search_input)
            self.search_input.textChanged.connect(self.refresh)

            # OTM start time field (Palomar local time)
            top.addSpacing(12)
            top.addWidget(QLabel("OTM time:", self))
            self.otm_time_input = QLineEdit(self)
            self.otm_time_input.setMaximumWidth(160)
            self.otm_time_input.setPlaceholderText("YYYY-MM-DDTHH:MM:SS")
            self.otm_time_input.setToolTip("OTM start time in Palomar local time (Pacific)")
            top.addWidget(self.otm_time_input)

            # "Live" checkbox — auto-updates the time field to current Palomar time
            self.otm_live_checkbox = QCheckBox("Live", self)
            self.otm_live_checkbox.setToolTip("Continuously update OTM time to current Palomar local time")
            self.otm_live_checkbox.setChecked(True)
            top.addWidget(self.otm_live_checkbox)

            # Timer to update time field when Live is checked
            self._otm_live_timer = QTimer(self)
            self._otm_live_timer.setInterval(1000)
            self._otm_live_timer.timeout.connect(self._update_otm_live_time)
            self._otm_live_timer.start()

            # When user unchecks Live, stop overwriting; when checked, resume
            self.otm_live_checkbox.toggled.connect(
                lambda checked: self._otm_live_timer.start() if checked else self._otm_live_timer.stop()
            )

            # Set initial value
            self._update_otm_live_time()
        else:
            self.search_input = None
            self.otm_time_input = None
            self.otm_live_checkbox = None

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

        layout.addLayout(top)

        # Table widget
        self.table = QTableWidget(self)
        self.table.setEditTriggers(QTableWidget.DoubleClicked | QTableWidget.SelectedClicked)
        self.table.setSelectionBehavior(QTableWidget.SelectRows)
        self.table.setSelectionMode(QTableWidget.SingleSelection)
        self.table.horizontalHeader().setStyleSheet(
            "QHeaderView::section { padding: 2px 4px; }"
        )
        self.table.horizontalHeader().setMinimumSectionSize(40)
        self.table.itemChanged.connect(self._on_item_changed)
        self.table.itemSelectionChanged.connect(self._on_selection_changed)
        self.table.cellClicked.connect(self._on_cell_clicked)
        layout.addWidget(self.table, 1)

        # Enable context menu for all tables
        self.table.setContextMenuPolicy(Qt.CustomContextMenu)
        self.table.customContextMenuRequested.connect(self._show_context_menu)

        # Connect signals
        self.add_button.clicked.connect(self._on_add_clicked)
        self.delete_button.clicked.connect(self._on_delete_clicked)

    def _update_otm_live_time(self) -> None:
        """Update the OTM time field with current Palomar local time."""
        if not self.otm_time_input:
            return
        try:
            from datetime import datetime
            import pytz
            palomar_tz = pytz.timezone("US/Pacific")
            now_palomar = datetime.now(palomar_tz)
            self.otm_time_input.setText(now_palomar.strftime("%Y-%m-%dT%H:%M:%S"))
        except Exception:
            pass

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
        """Handle item edit with normalization. Emits write_requested for centralized DB write."""
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

        key_vals = self._row_keys[row]
        updates = {col_meta.name: new_value}

        # Apply normalization for target table
        if self._is_targets_table():
            row_values = self._current_row_values(row)
            row_values[col_meta.name] = new_value if new_value is not None else ""
            norm_result = normalize_target_row(row_values)

            if norm_result.changed_columns:
                parent_window = self.window()
                if hasattr(parent_window, 'statusBar'):
                    parent_window.statusBar().showMessage(norm_result.message, 5000)
                for changed_col in norm_result.changed_columns:
                    if changed_col in row_values:
                        updates[changed_col] = row_values[changed_col]

        # Emit centralized write request instead of writing directly
        self.write_requested.emit(dict(key_vals), updates)

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

                # Refresh table and notify
                self.refresh()
                self.data_changed.emit()
                self.views_need_refresh.emit()

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
        self.data_changed.emit()
        self.views_need_refresh.emit()

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

            # Refresh view and notify
            self.refresh()
            self.table.selectRow(target_row)
            self.data_changed.emit()
            self.views_need_refresh.emit()

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

            # Refresh view and notify
            self.refresh()
            self.table.selectRow(target_position)
            self.data_changed.emit()
            self.views_need_refresh.emit()

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

            # Refresh view and notify
            self.refresh()
            self.data_changed.emit()
            self.views_need_refresh.emit()

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

                # Refresh table and notify
                self.refresh()
                self.data_changed.emit()
                self.views_need_refresh.emit()

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

            # Refresh view and notify
            self.refresh()
            self.data_changed.emit()
            self.views_need_refresh.emit()

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

            # Refresh table and notify
            self.refresh()
            self.data_changed.emit()
            self.views_need_refresh.emit()

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

            # Refresh table and notify
            self.refresh()
            self.data_changed.emit()
            self.views_need_refresh.emit()

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

        # Auto-run OTM state
        self._current_set_id = None
        self._timeline_stale = True
        self._otm_running = False
        self._otm_thread = None
        self._otm_runner = None
        self._timeline_thread = None
        self._timeline_runner = None

        # Debounce timer for auto-run OTM (1.5s after last data change)
        self._otm_debounce_timer = QTimer(self)
        self._otm_debounce_timer.setSingleShot(True)
        self._otm_debounce_timer.setInterval(1500)
        self._otm_debounce_timer.timeout.connect(self._auto_run_otm)

        self._init_ui()
        self._init_database()

    def _init_ui(self) -> None:
        """Initialize UI components."""
        layout = QVBoxLayout(self)

        # Tab widget for sets and targets
        self.tabs = QTabWidget(self)
        layout.addWidget(self.tabs, 1)

    def _init_database(self) -> None:
        """Initialize database connection."""
        # Try to load config
        config_path = detect_default_config_path()
        if not config_path:
            print("DatabaseTab: config not found")
            return

        self._config = load_config_file(config_path)
        if not self._config or not self._config.is_complete():
            print("DatabaseTab: incomplete config")
            return

        try:
            self._db.connect(self._config)
            print(f"DatabaseTab: connected to {self._config.user}@{self._config.host}:{self._config.port}/{self._config.schema}")

            # Create targets table widget
            self.targets_table = DatabaseTableWidget(
                "Targets",
                self,
                self._db,
                self._config,
                self._config.table_targets,
                allow_reorder=True,
                search_column="NAME"
            )

            # Setup table
            self.targets_table.set_order_by("OBS_ORDER")
            self.targets_table.set_hidden_columns([
                "OBSERVATION_ID", "SET_ID", "OBS_ORDER",
                "TARGET_NUMBER", "SEQUENCE_NUMBER", "SLITOFFSET", "OBSMODE"
            ])

            # Add to tabs
            self.tabs.addTab(self.targets_table, "Targets")

            # Add Timeline tab with QWebChannel bridge if available
            if WEBENGINE_AVAILABLE:
                self.timeline_view = QWebEngineView(self)
                self.timeline_view.setMinimumHeight(400)
                self.timeline_view.page().setBackgroundColor(QColor("#2b2b2b"))

                # Set up QWebChannel bridge for JS↔Python communication
                if TIMELINE_AVAILABLE:
                    self._timeline_bridge = TimelineBridge(self)
                    self._timeline_channel = QWebChannel(self.timeline_view.page())
                    self._timeline_channel.registerObject("bridge", self._timeline_bridge)
                    self.timeline_view.page().setWebChannel(self._timeline_channel)

                    # Connect bridge signals
                    self._timeline_bridge.targetSelected.connect(self._on_timeline_target_selected)
                    self._timeline_bridge.targetDoubleClicked.connect(self._on_timeline_exptime_edit)
                    self._timeline_bridge.flagClicked.connect(self._on_timeline_flag_clicked)
                    self._timeline_bridge.contextMenuRequested.connect(self._on_timeline_context_menu)
                    self._timeline_bridge.targetReorderRequested.connect(self._on_timeline_reorder)

                self.tabs.addTab(self.timeline_view, "Timeline")
            else:
                self.timeline_view = None

            # Table→Timeline+ControlTab sync: when user selects a row in targets table,
            # highlight corresponding target in timeline and populate control tab fields
            self.targets_table.selection_changed.connect(self._on_targets_table_selection)

            # Centralized DB write signals from table widget
            self.targets_table.write_requested.connect(
                lambda k, u: self._write_db(k, u, "table")
            )
            self.targets_table.views_need_refresh.connect(self._refresh_all_views)

            # Auto-run OTM when data changes or timeline tab is selected
            self.targets_table.data_changed.connect(self._on_targets_data_changed)
            self.tabs.currentChanged.connect(self._on_tab_changed)

            # Wire ControlTab editingFinished → refresh all views after live DB writes
            main_window = self.window()
            if main_window and hasattr(main_window, 'layout_service'):
                ct = getattr(main_window.layout_service, 'control_tab', None)
                if ct:
                    for field in [ct.exposure_time_box, ct.slit_width_box, ct.slit_angle_box,
                                  ct.num_of_exposures_box, ct.bin_spect_box, ct.bin_spat_box]:
                        field.editingFinished.connect(self._on_control_tab_confirmed)

                    # Wire OTM buttons from ControlTab
                    if OTM_AVAILABLE and hasattr(ct, 'otm_settings_btn'):
                        ct.otm_settings_btn.clicked.connect(self._show_otm_settings)
                    if OTM_AVAILABLE and hasattr(ct, 'run_otm_btn'):
                        ct.run_otm_btn.clicked.connect(self._run_otm)

            # Initial refresh
            self.targets_table.refresh()

            # Wire OTM time field changes → re-run OTM
            if hasattr(self.targets_table, 'otm_time_input') and self.targets_table.otm_time_input:
                self.targets_table.otm_time_input.editingFinished.connect(self._on_otm_time_changed)

        except Exception as exc:
            print(f"DatabaseTab: connection failed: {exc}")
            QMessageBox.critical(self, "Database Error", f"Failed to connect: {exc}")

    def _get_otm_start_utc(self) -> str:
        """Read the OTM time field (Palomar local) and return UTC string, or '' on error."""
        local_time_str = ""
        if hasattr(self, 'targets_table') and hasattr(self.targets_table, 'otm_time_input'):
            inp = self.targets_table.otm_time_input
            if inp:
                local_time_str = inp.text().strip()
        if not local_time_str:
            return ""
        try:
            from datetime import datetime
            import pytz
            palomar_tz = pytz.timezone("US/Pacific")
            naive_dt = datetime.strptime(local_time_str, "%Y-%m-%dT%H:%M:%S")
            local_dt = palomar_tz.localize(naive_dt)
            return local_dt.astimezone(pytz.utc).strftime("%Y-%m-%dT%H:%M:%S")
        except Exception as e:
            print(f"OTM: invalid time format '{local_time_str}': {e}")
            return ""

    def _on_otm_time_changed(self) -> None:
        """Re-run OTM when the user manually changes the OTM time field."""
        # Only re-run if Live is unchecked (manual edit) — Live changes are continuous
        if hasattr(self.targets_table, 'otm_live_checkbox') and self.targets_table.otm_live_checkbox:
            if self.targets_table.otm_live_checkbox.isChecked():
                return
        self._timeline_stale = True
        self._otm_debounce_timer.start()

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
            print("OTM: integration modules not loaded")
            return
        if self._otm_running:
            print("OTM: already running, please wait")
            return

        # Get current set
        set_id = self._current_set_id
        if not set_id:
            print("OTM: no target set selected")
            return

        # Read start time from the OTM time field (Palomar local → UTC)
        start_utc = self._get_otm_start_utc()
        if not start_utc:
            print("OTM: no valid start time in the OTM time field")
            return

        try:
            # Load settings
            settings = OtmSettings.load()

            # Validate script paths
            if not settings.otm_script_path or not os.path.exists(settings.otm_script_path):
                print("OTM: script path not configured or missing")
                return

            if not settings.timeline_script_path or not os.path.exists(settings.timeline_script_path):
                print("OTM: timeline script path not configured or missing")
                return

            # Get targets for this set
            targets = self._get_targets_for_otm(set_id)
            if not targets:
                print(f"OTM: no targets found in set {set_id}")
                return

            # Create temporary files
            import tempfile
            temp_dir = tempfile.mkdtemp(prefix="ngps_otm_")
            input_csv = os.path.join(temp_dir, "otm_input.csv")
            output_csv = os.path.join(temp_dir, "otm_output.csv")
            timeline_json = os.path.join(temp_dir, "timeline.json")

            # Generate input CSV (filters targets outside observable DEC range)
            skipped = generate_otm_input_csv(targets, input_csv)
            if skipped:
                print(f"OTM: {len(skipped)} target(s) skipped (DEC outside observable range): {skipped}")

            # Verify we still have targets after filtering
            with open(input_csv, 'r') as f:
                line_count = sum(1 for _ in f) - 1  # subtract header
            if line_count <= 0:
                print("OTM: all targets filtered out (DEC outside observable range)")
                return

            # Show status bar message
            parent_window = self.window()
            if hasattr(parent_window, 'statusBar'):
                parent_window.statusBar().showMessage("Running OTM scheduler...", 5000)

            # Mark running and clean up old threads
            self._otm_running = True
            self._timeline_stale = False
            self._cleanup_otm_threads()

            # Run OTM in thread (no progress dialog)
            self._otm_thread = QThread()
            self._otm_runner = OtmRunner(settings, input_csv, output_csv, start_utc)
            self._otm_runner.moveToThread(self._otm_thread)

            # Connect signals
            self._otm_thread.started.connect(self._otm_runner.run)
            self._otm_runner.finished.connect(
                lambda success, msg: self._on_otm_finished(
                    success, msg, input_csv, output_csv, timeline_json, set_id, start_utc,
                    progress=None
                )
            )
            self._otm_runner.finished.connect(self._otm_thread.quit)

            # Start
            self._otm_thread.start()

        except Exception as e:
            print(f"OTM: failed to run: {e}")

    def _on_otm_finished(self, success: bool, message: str, input_csv: str, output_csv: str,
                         timeline_json: str, set_id: str, start_utc: str,
                         progress: QProgressDialog, silent: bool = False) -> None:
        """Handle OTM completion: import results, run timeline subprocess, display Plotly chart."""
        if not success:
            if progress:
                progress.cancel()
            print(f"OTM: scheduler failed: {message}")
            self._finish_otm_pipeline(silent)
            return

        # Import OTM results to database
        try:
            if progress:
                progress.setLabelText("Importing OTM results to database...")

            otm_results = parse_otm_output_csv(output_csv)
            self._import_otm_results(set_id, otm_results)

            # Refresh tables — then suppress the data_changed → stale cascade
            # since we're already inside the OTM pipeline
            self.targets_table.refresh()
            self._timeline_stale = False
            self._otm_debounce_timer.stop()

        except Exception as e:
            if progress:
                progress.cancel()
            print(f"OTM: failed to import results: {e}")
            self._finish_otm_pipeline(silent)
            return

        # Run timeline JSON generation subprocess (needs OTM venv / astropy)
        if progress:
            progress.setLabelText("Generating timeline data...")
        settings = OtmSettings.load()

        self._cleanup_timeline_thread()
        self._timeline_thread = QThread()
        self._timeline_runner = OtmTimelineRunner(
            settings, input_csv, output_csv, timeline_json, start_utc
        )
        self._timeline_runner.moveToThread(self._timeline_thread)

        self._timeline_thread.started.connect(self._timeline_runner.run)
        if progress:
            self._timeline_runner.progress.connect(lambda msg: progress.setLabelText(msg))
        self._timeline_runner.finished.connect(
            lambda ok, msg, json_path: self._on_timeline_finished(
                ok, msg, json_path, len(otm_results), settings.airmass_max, progress, silent
            )
        )
        self._timeline_runner.finished.connect(self._timeline_thread.quit)
        if progress:
            progress.canceled.connect(self._timeline_runner.cancel)
            progress.canceled.connect(self._timeline_thread.quit)

        self._timeline_thread.start()

    def _on_timeline_finished(self, success: bool, message: str, json_path: str,
                              num_results: int, airmass_limit: float,
                              progress: QProgressDialog, silent: bool = False) -> None:
        """Handle timeline JSON generation: parse, render Plotly HTML, display in WebView."""
        auto_run = silent

        if not success:
            if progress:
                progress.cancel()
            print(f"OTM: timeline generation failed: {message}")
            self._finish_otm_pipeline(auto_run)
            return

        # Parse timeline JSON and generate Plotly HTML (in-process, no astropy needed)
        try:
            if progress:
                progress.setLabelText("Rendering timeline chart...")
            timeline_data = parse_timeline_json(json_path)
            html_content = generate_timeline_html(timeline_data, airmass_limit)

            # Write HTML to temp file (plotly.js is ~4.8MB, exceeds setHtml() limit)
            import tempfile
            html_fd, html_path = tempfile.mkstemp(prefix="ngps_timeline_", suffix=".html")
            with os.fdopen(html_fd, 'w') as f:
                f.write(html_content)
            self._timeline_html_path = html_path

        except Exception as e:
            if progress:
                progress.cancel()
            print(f"OTM: failed to generate timeline chart: {e}")
            self._finish_otm_pipeline(auto_run)
            return

        if progress:
            progress.cancel()

        # Load HTML into QWebEngineView (preserving scroll position)
        if self.timeline_view and WEBENGINE_AVAILABLE:
            try:
                switch_to_tab = not silent
                self._load_timeline_html(html_path, switch_to_tab=switch_to_tab)
            except Exception as e:
                print(f"OTM: failed to load timeline HTML: {e}")

        print(f"OTM: completed, processed {num_results} targets")

        self._finish_otm_pipeline(auto_run)

    # ── Centralized DB write + view refresh ──

    def _write_db(self, key_values, updates, source="") -> bool:
        """Single entry point for all DB writes to the targets table.

        After a successful write, refreshes the table, control tab fields,
        and marks the timeline stale so all views stay in sync.
        """
        try:
            self._db.update_record(self._config.table_targets, key_values, updates)
        except Exception as exc:
            print(f"DB write failed ({source}): {exc}")
            return False
        self._refresh_all_views()
        return True

    def _refresh_all_views(self):
        """Refresh targets table, control tab fields, and mark timeline stale."""
        self.targets_table.refresh()
        row = self.targets_table.table.currentRow()
        if row >= 0:
            values = self.targets_table._current_row_values(row)
            if values:
                self._update_control_tab_fields(values)
        self._mark_stale_and_debounce()

    def _update_control_tab_fields(self, values: Dict[str, Any]) -> None:
        """Sync the selected target's values to the ControlTab fields on the right."""
        main_window = self.window()
        if not main_window:
            return

        # Find the ControlTab — it's in layout_service.control_tab or parent's control_tab
        ct = None
        if hasattr(main_window, 'layout_service') and hasattr(main_window.layout_service, 'control_tab'):
            ct = main_window.layout_service.control_tab
        if ct is None:
            return

        # Store current observation ID on the main window for "Go" button etc.
        obs_id = values.get("OBSERVATION_ID", "")
        if obs_id:
            main_window.current_observation_id = obs_id
        main_window.current_ra = values.get("RA", "")
        main_window.current_dec = values.get("DECL", "")
        main_window.current_offset_ra = values.get("OFFSET_RA", "")
        main_window.current_offset_dec = values.get("OFFSET_DEC", "")
        main_window.current_bin_spect = values.get("BINSPECT", "")
        main_window.current_bin_spat = values.get("BINSPAT", "")
        main_window.num_of_exposures = values.get("NEXP", "")

        # Update ControlTab fields — show full DB value including mode prefix
        # (e.g. "SET 300", "SNR 50", "SET 1.5")
        ct.exposure_time_box.setText(str(values.get("EXPTIME", "")))
        ct.slit_width_box.setText(str(values.get("SLITWIDTH", "")))
        ct.slit_angle_box.setText(str(values.get("SLITANGLE", "0")))
        ct.bin_spect_box.setText(str(values.get("BINSPECT", "")))
        ct.bin_spat_box.setText(str(values.get("BINSPAT", "")))
        ct.num_of_exposures_box.setText(str(values.get("NEXP", "")))

        # Update target name and RA/Dec labels
        name = values.get("NAME", "")
        ra = values.get("RA", "")
        dec = values.get("DECL", "")
        if name:
            ct.target_name_label.setText(f"Selected Target: {name}")
            ct.ra_dec_label.setText(f"RA: {ra}, Dec: {dec}")
        else:
            ct.target_name_label.setText("Selected Target: Not Selected")
            ct.ra_dec_label.setText("RA: Not Set, Dec: Not Set")

        # Sync button states now that target selection changed
        if hasattr(main_window, 'layout_service'):
            main_window.layout_service._sync_control_buttons()

    def _on_control_tab_confirmed(self) -> None:
        """Called after ControlTab writes to DB via its own path.

        Delays the refresh slightly so the ControlTab's DB commits finish first.
        """
        from PyQt5.QtCore import QTimer
        QTimer.singleShot(200, self._refresh_all_views)

    # ── Auto-run OTM helpers ──

    def _cleanup_otm_threads(self) -> None:
        """Wait for and clean up old OTM/timeline threads before starting new ones."""
        self._cleanup_timeline_thread()
        if self._otm_thread is not None:
            if self._otm_thread.isRunning():
                self._otm_thread.quit()
                self._otm_thread.wait(3000)
            self._otm_thread = None
            self._otm_runner = None

    def _cleanup_timeline_thread(self) -> None:
        """Wait for and clean up old timeline thread."""
        if self._timeline_thread is not None:
            if self._timeline_thread.isRunning():
                self._timeline_thread.quit()
                self._timeline_thread.wait(3000)
            self._timeline_thread = None
            self._timeline_runner = None

    def _finish_otm_pipeline(self, auto_run: bool) -> None:
        """Clean up after OTM pipeline completes. Re-run if data changed while running."""
        self._otm_running = False
        if auto_run and self._timeline_stale:
            # Data changed while OTM was running — re-run
            self._otm_debounce_timer.start()

    def _on_targets_data_changed(self) -> None:
        """Handle data_changed from targets table: mark stale and restart debounce."""
        self._timeline_stale = True
        self._otm_debounce_timer.start()

    def _on_tab_changed(self, index: int) -> None:
        """Auto-run OTM when switching to the Timeline tab if data is stale."""
        if not self._timeline_stale:
            return
        widget = self.tabs.widget(index)
        if hasattr(self, 'timeline_view') and widget is self.timeline_view:
            self._auto_run_otm()

    def _mark_stale_and_debounce(self) -> None:
        """Mark timeline as stale and restart the debounce timer."""
        self._timeline_stale = True
        self._otm_debounce_timer.start()

    def _get_selected_obs_id(self) -> str:
        """Return the OBSERVATION_ID of the currently selected row, or ''."""
        row = self.targets_table.table.currentRow()
        if row < 0:
            return ""
        obs_col = self.targets_table._column_index.get("OBSERVATION_ID")
        if obs_col is None:
            return ""
        item = self.targets_table.table.item(row, obs_col)
        return item.text() if item else ""

    def _select_obs_id_in_table(self, obs_id: str) -> None:
        """Select the row matching obs_id in the targets table."""
        table = self.targets_table.table
        obs_col = self.targets_table._column_index.get("OBSERVATION_ID")
        if obs_col is None:
            return
        for row in range(table.rowCount()):
            item = table.item(row, obs_col)
            if item and item.text() == str(obs_id):
                table.selectRow(row)
                table.scrollToItem(item)
                values = self.targets_table._current_row_values(row)
                if values:
                    self._update_control_tab_fields(values)
                break

    def _load_timeline_html(self, html_path: str, switch_to_tab: bool = False) -> None:
        """Load timeline HTML into QWebEngineView, preserving scroll and selection."""
        from PyQt5.QtCore import QUrl

        url = QUrl.fromLocalFile(html_path)

        # Capture current selection before reload
        selected_obs_id = self._get_selected_obs_id()

        def _do_load(scroll_json=None):
            """Load the URL and restore scroll + selection after load completes."""
            scroll_x, scroll_y = 0, 0
            if scroll_json:
                try:
                    import json
                    pos = json.loads(scroll_json)
                    scroll_x = pos.get("x", 0)
                    scroll_y = pos.get("y", 0)
                except Exception:
                    pass

            def _on_load_finished(ok):
                try:
                    self.timeline_view.loadFinished.disconnect(_on_load_finished)
                except Exception:
                    pass
                if not ok:
                    return
                # First restore selection (rebuilds plot with expanded row),
                # then restore scroll to override any auto-scroll from selection.
                js_parts = []
                if selected_obs_id:
                    escaped = str(selected_obs_id).replace("'", "\\'")
                    js_parts.append(
                        f"if (typeof selectTargetInTimeline === 'function') "
                        f"selectTargetInTimeline('{escaped}');"
                    )
                if scroll_x or scroll_y:
                    js_parts.append(f"window.scrollTo({scroll_x}, {scroll_y});")
                if js_parts:
                    self.timeline_view.page().runJavaScript("\n".join(js_parts))

            self.timeline_view.loadFinished.connect(_on_load_finished)
            self.timeline_view.load(url)

            if switch_to_tab:
                for i in range(self.tabs.count()):
                    if self.tabs.widget(i) == self.timeline_view:
                        self.tabs.setCurrentIndex(i)
                        break

        # Capture current scroll position before loading new content
        self.timeline_view.page().runJavaScript(
            "JSON.stringify({x: window.scrollX || 0, y: window.scrollY || 0})",
            _do_load
        )

    def _auto_run_otm(self) -> None:
        """Silently run the full OTM pipeline (no dialogs, no user prompts)."""
        if self._otm_running:
            return
        if not OTM_AVAILABLE:
            return

        set_id = self._current_set_id
        if not set_id:
            return

        # Validate settings silently
        try:
            settings = OtmSettings.load()
        except Exception:
            return

        if not settings.otm_script_path or not os.path.exists(settings.otm_script_path):
            return
        if not settings.timeline_script_path or not os.path.exists(settings.timeline_script_path):
            return

        # Get targets
        targets = self._get_targets_for_otm(set_id)
        if not targets:
            return

        # Generate input CSV
        import tempfile
        temp_dir = tempfile.mkdtemp(prefix="ngps_otm_auto_")
        input_csv = os.path.join(temp_dir, "otm_input.csv")
        output_csv = os.path.join(temp_dir, "otm_output.csv")
        timeline_json = os.path.join(temp_dir, "timeline.json")

        try:
            generate_otm_input_csv(targets, input_csv)
        except Exception:
            return

        # Verify we have targets after filtering
        try:
            with open(input_csv, 'r') as f:
                line_count = sum(1 for _ in f) - 1
            if line_count <= 0:
                return
        except Exception:
            return

        start_utc = self._get_otm_start_utc()
        if not start_utc:
            return

        # Mark running and clear stale flag
        self._otm_running = True
        self._timeline_stale = False
        self._cleanup_otm_threads()

        # Show status bar message
        parent_window = self.window()
        if hasattr(parent_window, 'statusBar'):
            parent_window.statusBar().showMessage("Auto-running OTM scheduler...", 5000)

        # Run OTM in thread (silent — no progress dialog)
        self._otm_thread = QThread()
        self._otm_runner = OtmRunner(settings, input_csv, output_csv, start_utc)
        self._otm_runner.moveToThread(self._otm_thread)

        self._otm_thread.started.connect(self._otm_runner.run)
        self._otm_runner.finished.connect(
            lambda success, msg: self._on_otm_finished(
                success, msg, input_csv, output_csv, timeline_json, set_id, start_utc,
                progress=None, silent=True
            )
        )
        self._otm_runner.finished.connect(self._otm_thread.quit)

        self._otm_thread.start()

    def _get_targets_for_otm(self, set_id: str) -> List[Dict[str, Any]]:
        """Get all targets for a set in OTM format."""
        if not self._db.is_open():
            return []

        try:
            targets = self._db.fetch_rows(
                self._config.table_targets,
                where="`SET_ID`=%s",
                params=(set_id,),
                order_by="OBS_ORDER"
            )
            return targets

        except Exception as e:
            print(f"Error fetching targets for OTM: {e}")
            return []

    def _import_otm_results(self, set_id: str, results: List) -> None:
        """Import OTM results back to database.

        Maps OTM output CSV column names to database column names using
        OTM_CSV_TO_DB mapping (matches C++ addUpdate calls).
        """
        if not self._db.is_open():
            raise RuntimeError("Database not connected")

        # Map from OTM CSV field name to OtmResult attribute name
        csv_field_to_attr = {
            "OTMstart": "otmstart",
            "OTMend": "otmend",
            "OTMslewgo": "otmslewgo",
            "OTMflag": "otmflag",
            "OTMlast": "otmlast",
            "OTMwait": "otmwait",
            "OTMdead": "otmdead",
            "OTMslew": "otmslew",
            "OTMexptime": "otmexptime",
            "OTMslitwidth": "otmslitwidth",
            "OTMpa": "otmpa",
            "OTMslitangle": "otmslitangle",
            "OTMcass": "otmcass",
            "OTMSNR": "otmsnr",
            "OTMres": "otmres",
            "OTMseeing": "otmseeing",
            "OTMairmass_start": "otmairmass_start",
            "OTMairmass_end": "otmairmass_end",
            "OTMsky": "otmsky",
            "OTMmoon": "otmmoon",
        }

        try:
            for result in results:
                updates = {}

                # Map each OTM CSV column to its database column name
                for csv_col, db_col in OTM_CSV_TO_DB.items():
                    attr = csv_field_to_attr.get(csv_col)
                    if not attr:
                        continue
                    val = getattr(result, attr, '')
                    # Store empty strings as NULL
                    updates[db_col] = val if val else None

                # Always update OBS_ORDER
                updates["OBS_ORDER"] = result.obs_order

                key_values = {
                    "OBSERVATION_ID": result.observation_id,
                    "SET_ID": set_id,
                }

                self._db.update_record(
                    self._config.table_targets, key_values, updates
                )

        except Exception as e:
            raise RuntimeError(f"Failed to import OTM results: {e}")

    # ── Timeline bridge signal handlers ──

    def _on_targets_table_selection(self, values: Dict[str, Any]) -> None:
        """Sync table selection → control tab fields + timeline highlight."""
        # Update the existing ControlTab fields on the right-hand side
        self._update_control_tab_fields(values)

        # Update timeline highlight
        obs_id = values.get("OBSERVATION_ID", "")
        if obs_id and self.timeline_view and WEBENGINE_AVAILABLE:
            escaped = str(obs_id).replace("'", "\\'")
            self.timeline_view.page().runJavaScript(
                f"if (typeof selectTargetInTimeline === 'function') selectTargetInTimeline('{escaped}');"
            )

    def _on_timeline_target_selected(self, obs_id: str) -> None:
        """Timeline click → select row in targets table."""
        if not hasattr(self, 'targets_table') or not self.targets_table:
            return
        table = self.targets_table.table
        obs_col = self.targets_table._column_index.get("OBSERVATION_ID")
        if obs_col is None:
            return
        for row in range(table.rowCount()):
            item = table.item(row, obs_col)
            if item and item.text() == str(obs_id):
                table.selectRow(row)
                table.scrollToItem(item)
                break

    def _on_timeline_exptime_edit(self, obs_id: str) -> None:
        """Timeline double-click → edit exposure time dialog."""
        if not self._db.is_open():
            return

        # Get current EXPTIME from the table
        table = self.targets_table.table
        obs_col = self.targets_table._column_index.get("OBSERVATION_ID")
        exptime_col = self.targets_table._column_index.get("EXPTIME")
        if obs_col is None or exptime_col is None:
            return

        current_exptime = ""
        target_row = -1
        for row in range(table.rowCount()):
            item = table.item(row, obs_col)
            if item and item.text() == str(obs_id):
                target_row = row
                exp_item = table.item(row, exptime_col)
                if exp_item:
                    current_exptime = exp_item.text()
                break

        if target_row < 0:
            return

        new_val, ok = QInputDialog.getText(
            self,
            "Edit Exposure Time",
            f"Exposure time for observation {obs_id}:\n\n"
            "Format: 'SET <seconds>' or 'SNR <value>'",
            QLineEdit.Normal,
            current_exptime
        )

        if not ok or not new_val.strip():
            return

        # Get SET_ID for this row
        set_col = self.targets_table._column_index.get("SET_ID")
        set_id = ""
        if set_col is not None:
            set_item = table.item(target_row, set_col)
            if set_item:
                set_id = set_item.text()

        key_values = {"OBSERVATION_ID": obs_id}
        if set_id:
            key_values["SET_ID"] = set_id
        self._write_db(key_values, {"EXPTIME": new_val.strip()}, "timeline_exptime")

    # Detailed OTM flag descriptions from Python/OTM/OTM.py
    _OTM_FLAG_DESC = {
        "DAY-0":   "Observation delayed until sunset (astronomical twilight).",
        "DAY-0-1": "After waiting for target, sunrise has arrived. "
                    "This target (and all remaining) cannot be observed tonight.",
        "DAY-1":   "Exposure completes after sunrise — observation runs into daylight.",
        "ALT-0":   "Target altitude is outside [6.5\u00b0, 90\u00b0] at start. "
                    "Scheduler waits for target to reach valid altitude.",
        "ALT-1":   "Target altitude goes outside [6.5\u00b0, 90\u00b0] by end of exposure.",
        "AIR-0":   "Target airmass exceeds per-target maximum at start. "
                    "Scheduler waits for airmass to improve.",
        "AIR-1":   "Target airmass exceeds per-target maximum by end of exposure.",
        "HA-0":    "Target hour angle is outside [\u2212100\u00b0, +100\u00b0] at start. "
                    "Scheduler waits for target to reach valid HA.",
        "HA-1":    "Target hour angle goes outside [\u2212100\u00b0, +100\u00b0] by end of exposure.",
        "SKY":     "Target altitude below 19.5\u00b0 (airmass \u22483). "
                    "Sky background simulation unreliable — estimate may be inaccurate.",
        "EXPT":    "Exposure time \u2265 1000 s (16.7 min), which is unusually long.",
    }

    def _on_timeline_flag_clicked(self, obs_id: str, flag_text: str) -> None:
        """Timeline flag click → show flag explanation dialog."""
        from otm_integration import otm_flag_severity
        severity = otm_flag_severity(flag_text)
        severity_label = {0: "OK", 1: "Warning", 2: "Error"}.get(severity, "Unknown")

        # Build per-flag explanations from the compound flag string
        details = []
        if flag_text:
            for token in flag_text.strip().split():
                desc = self._OTM_FLAG_DESC.get(token.upper())
                if desc:
                    details.append(f"  {token}:  {desc}")
                else:
                    details.append(f"  {token}:  (unknown flag)")
        detail_block = "\n".join(details) if details else "  No flags — target scheduled without issues."

        QMessageBox.information(
            self,
            f"OTM Flag — {severity_label}",
            f"Observation: {obs_id}\n"
            f"Flag: {flag_text}\n"
            f"Severity: {severity_label}\n\n"
            f"{detail_block}"
        )

    def _on_timeline_context_menu(self, obs_id: str, screen_x: int, screen_y: int) -> None:
        """Timeline right-click → show context menu."""
        from PyQt5.QtCore import QPoint
        menu = QMenu(self)

        edit_action = menu.addAction("Edit Exposure Time")
        menu.addSeparator()
        move_up_action = menu.addAction("Move Up")
        move_down_action = menu.addAction("Move Down")

        action = menu.exec_(QPoint(screen_x, screen_y))
        if action == edit_action:
            self._on_timeline_exptime_edit(obs_id)
        elif action == move_up_action:
            self._timeline_move_target(obs_id, -1)
        elif action == move_down_action:
            self._timeline_move_target(obs_id, +1)

    def _timeline_move_target(self, obs_id: str, direction: int) -> None:
        """Move a target up or down in OBS_ORDER."""
        if not self._db.is_open():
            return

        # Find the target row and its neighbor
        table = self.targets_table.table
        obs_col = self.targets_table._column_index.get("OBSERVATION_ID")
        order_col = self.targets_table._column_index.get("OBS_ORDER")
        if obs_col is None or order_col is None:
            return

        target_row = -1
        for row in range(table.rowCount()):
            item = table.item(row, obs_col)
            if item and item.text() == str(obs_id):
                target_row = row
                break

        if target_row < 0:
            return

        swap_row = target_row + direction
        if swap_row < 0 or swap_row >= table.rowCount():
            return

        # Get OBS_ORDER and OBSERVATION_ID for both rows
        cur_order_item = table.item(target_row, order_col)
        swap_order_item = table.item(swap_row, order_col)
        swap_obs_item = table.item(swap_row, obs_col)

        if not all([cur_order_item, swap_order_item, swap_obs_item]):
            return

        cur_order = cur_order_item.text()
        swap_order = swap_order_item.text()
        swap_obs_id = swap_obs_item.text()

        # Get SET_ID
        set_col = self.targets_table._column_index.get("SET_ID")
        set_id = ""
        if set_col is not None:
            set_item = table.item(target_row, set_col)
            if set_item:
                set_id = set_item.text()

        key1 = {"OBSERVATION_ID": obs_id}
        key2 = {"OBSERVATION_ID": swap_obs_id}
        if set_id:
            key1["SET_ID"] = set_id
            key2["SET_ID"] = set_id

        # Do both writes then one refresh
        try:
            self._db.update_record(self._config.table_targets, key1, {"OBS_ORDER": swap_order})
            self._db.update_record(self._config.table_targets, key2, {"OBS_ORDER": cur_order})
        except Exception as e:
            print(f"DB write failed (timeline_move): {e}")
            return
        self._refresh_all_views()

    def _on_timeline_reorder(self, from_obs_id: str, after_obs_id: str) -> None:
        """Timeline drag-reorder → swap OBS_ORDER values in DB and refresh."""
        if not self._db.is_open():
            return

        table = self.targets_table.table
        obs_col = self.targets_table._column_index.get("OBSERVATION_ID")
        order_col = self.targets_table._column_index.get("OBS_ORDER")
        if obs_col is None or order_col is None:
            return

        # Find the rows
        from_row = -1
        after_row = -1
        for row in range(table.rowCount()):
            item = table.item(row, obs_col)
            if not item:
                continue
            if item.text() == str(from_obs_id):
                from_row = row
            if item.text() == str(after_obs_id):
                after_row = row

        if from_row < 0 or after_row < 0 or from_row == after_row:
            return

        # Swap OBS_ORDER values
        from_order_item = table.item(from_row, order_col)
        after_order_item = table.item(after_row, order_col)
        if not from_order_item or not after_order_item:
            return

        from_order = from_order_item.text()
        after_order = after_order_item.text()

        set_col = self.targets_table._column_index.get("SET_ID")
        set_id = ""
        if set_col is not None:
            set_item = table.item(from_row, set_col)
            if set_item:
                set_id = set_item.text()

        key1 = {"OBSERVATION_ID": from_obs_id}
        key2 = {"OBSERVATION_ID": after_obs_id}
        if set_id:
            key1["SET_ID"] = set_id
            key2["SET_ID"] = set_id

        # Do both writes then one refresh
        try:
            self._db.update_record(self._config.table_targets, key1, {"OBS_ORDER": after_order})
            self._db.update_record(self._config.table_targets, key2, {"OBS_ORDER": from_order})
        except Exception as e:
            print(f"DB write failed (timeline_reorder): {e}")
            return
        self._refresh_all_views()

        # Re-select the dragged target so timeline highlights the correct row
        self._select_obs_id_in_table(from_obs_id)
