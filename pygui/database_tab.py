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
    QMenu, QHeaderView, QProgressDialog, QInputDialog, QCheckBox,
    QAbstractItemView
)
from PyQt5.QtGui import QBrush, QColor, QPalette, QCursor


# Settings
SETTINGS_ORG = "NGPS"
SETTINGS_APP = "ngps_gui_database"
GROUP_COORD_TOL_ARCSEC = 1.0
OFFSET_ZERO_TOL_ARCSEC = 0.1

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

    def get_schema(self, schema_name: str):
        """Compatibility helper for call sites that use mysqlx table API."""
        if not self.is_open():
            raise RuntimeError("Database session not open")
        return self._session.get_schema(schema_name)

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


class ReorderableTableWidget(QTableWidget):
    """QTableWidget that emits source/target OBS IDs for drag-reorder."""

    reorder_requested = pyqtSignal(str, str)  # (from_obs_id, after_obs_id), after_obs_id may be "" for top

    def __init__(self, parent: QWidget = None) -> None:
        super().__init__(parent)
        self._obs_id_col: Optional[int] = None
        self._drag_source_obs_id: str = ""
        self._drag_source_row: int = -1

    def set_obs_id_column(self, col_idx: Optional[int]) -> None:
        self._obs_id_col = col_idx

    def _obs_id_for_row(self, row: int) -> str:
        if self._obs_id_col is None or row < 0 or row >= self.rowCount():
            return ""
        item = self.item(row, self._obs_id_col)
        return item.text().strip() if item else ""

    def mousePressEvent(self, event) -> None:
        if event.button() == Qt.LeftButton:
            row = self.rowAt(event.pos().y())
            self._drag_source_row = row
            self._drag_source_obs_id = self._obs_id_for_row(row)
        super().mousePressEvent(event)

    def dropEvent(self, event) -> None:
        # Keep table order source-of-truth in DB: on drop, emit reorder request
        # and let existing DB reorder+refresh logic redraw the table.
        if event.source() is not self:
            event.ignore()
            return

        from_obs_id = self._drag_source_obs_id
        if not from_obs_id:
            event.ignore()
            return

        row_count = self.rowCount()
        if row_count == 0:
            event.ignore()
            return

        # Robust top detection: if release point is above first-row midpoint,
        # always interpret as "move to top". This matches timeline semantics.
        first_rect = self.visualRect(self.model().index(0, 0))
        first_mid_y = first_rect.center().y() if first_rect.isValid() else 0
        if event.pos().y() < first_mid_y:
            self.reorder_requested.emit(from_obs_id, "")
            event.acceptProposedAction()
            return

        target_row = self.rowAt(event.pos().y())
        if target_row < 0:
            target_row = row_count - 1
            insert_above = False
        else:
            insert_above = False
            row_rect = self.visualRect(self.model().index(target_row, 0))
            if row_rect.isValid():
                insert_above = event.pos().y() < row_rect.center().y()

        if insert_above:
            if target_row == 0:
                self.reorder_requested.emit(from_obs_id, "")
                event.acceptProposedAction()
                return
            after_obs_id = self._obs_id_for_row(target_row - 1)
        else:
            after_obs_id = self._obs_id_for_row(target_row)

        if not after_obs_id or from_obs_id == after_obs_id:
            event.ignore()
            return

        self.reorder_requested.emit(from_obs_id, after_obs_id)
        event.acceptProposedAction()


class DatabaseTableWidget(QWidget):
    """Enhanced table widget with state persistence and normalization."""

    selection_changed = pyqtSignal(dict)
    data_changed = pyqtSignal()
    write_requested = pyqtSignal(dict, dict)    # (key_values, updates) for centralized DB write
    views_need_refresh = pyqtSignal()            # DB was written internally, all views need refresh
    reorder_requested = pyqtSignal(str, str)     # (from_obs_id, after_obs_id)

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
        self._manual_group_key_by_obs_id: Dict[str, str] = {}
        self._selected_obs_id_by_header: Dict[str, str] = {}
        self._group_data: Dict[str, Dict[str, Any]] = {}
        self._row_metadata: List[Dict[str, Any]] = []  # Parallel to _row_keys

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
        if self._is_targets_table():
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
        self.bulk_edit_button.setToolTip("Edit one column for all targets in the current view")
        self.bulk_edit_button.clicked.connect(self._bulk_edit_dialog)
        top.addWidget(self.bulk_edit_button)

        self.column_vis_button = QPushButton("Columns...", self)
        self.column_vis_button.setToolTip("Show/hide columns")
        self.column_vis_button.clicked.connect(self._column_visibility_dialog)
        top.addWidget(self.column_vis_button)

        layout.addLayout(top)

        # Table widget
        self.table = ReorderableTableWidget(self)
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
        self.table.reorder_requested.connect(self.reorder_requested.emit)
        self._update_reorder_drag_state()

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
        self.table.set_obs_id_column(self._column_index.get("OBSERVATION_ID"))

        self.table.setColumnCount(len(self._columns))
        self.table.setHorizontalHeaderLabels([c.name for c in self._columns])

        # Connect column resize signal
        header = self.table.horizontalHeader()
        header.sectionResized.connect(self._on_column_resized)

        # Restore column widths
        self._restore_column_widths()

        # Restore grouping state from QSettings
        self._restore_grouping_state()
        if self.group_button:
            self.group_button.setChecked(self._grouping_enabled)
            self.group_button.setText(
                "Disable Grouping" if self._grouping_enabled else "Enable Grouping"
            )
        self._update_reorder_drag_state()

    def _update_reorder_drag_state(self) -> None:
        """Enable drag-reorder for targets table when reordering is allowed."""
        enabled = self._allow_reorder and self._is_targets_table()
        self.table.setDragEnabled(enabled)
        self.table.setAcceptDrops(enabled)
        self.table.viewport().setAcceptDrops(enabled)
        self.table.setDropIndicatorShown(enabled)
        self.table.setDefaultDropAction(Qt.MoveAction)
        self.table.setDragDropOverwriteMode(False)
        self.table.setDragDropMode(QAbstractItemView.DragDrop if enabled else QAbstractItemView.NoDragDrop)

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
        self._row_metadata = []

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

            # Store grouping metadata from the row dict
            meta: Dict[str, Any] = {
                key: value for key, value in row.items()
                if isinstance(key, str) and key.startswith("_")
            }
            self._row_metadata.append(meta)
            self._apply_group_name_style(row_idx, meta)

        self._apply_hidden_columns()
        self._loading = False

        # Restore view state
        self._restore_view_state(view_state)

    def _apply_group_name_style(self, row_idx: int, meta: Dict[str, Any]) -> None:
        """Make selected sequencer target stand out within grouped rows."""
        if not self._grouping_enabled or not meta:
            return

        name_col = self._column_index.get("NAME")
        if name_col is None:
            return

        item = self.table.item(row_idx, name_col)
        if item is None:
            return

        is_header = bool(meta.get("_IS_GROUP_HEADER"))
        is_member = bool(meta.get("_IS_GROUP_MEMBER"))
        is_selected = bool(meta.get("_GROUP_IS_SELECTED"))
        if not (is_header or is_member):
            return

        font = item.font()
        font.setBold(is_selected)
        item.setFont(font)

        # Slight contrast boost for selected target relative to peers.
        dark_theme = self.palette().color(QPalette.Base).lightness() < 128
        if dark_theme:
            selected_color = QColor(230, 230, 230)
            member_color = QColor(200, 200, 200)
        else:
            selected_color = QColor(68, 68, 68)
            member_color = QColor(86, 86, 86)

        if is_selected:
            item.setForeground(QBrush(selected_color))
        elif is_member:
            item.setForeground(QBrush(member_color))

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
        """Handle cell click - toggle group only when clicking header icon hotspot."""
        if not self._grouping_enabled:
            return

        # Read metadata from parallel structure
        if row < 0 or row >= len(self._row_metadata):
            return
        meta = self._row_metadata[row]
        if not meta.get("_IS_GROUP_HEADER"):
            return

        # C++ parity: only clicks on the expand/collapse icon region should toggle.
        name_col = self._column_index.get("NAME")
        if name_col is None or column != name_col:
            return

        model_index = self.table.model().index(row, column)
        if not model_index.isValid():
            return

        cell_rect = self.table.visualRect(model_index)
        click_pos = self.table.viewport().mapFromGlobal(QCursor.pos())
        if not cell_rect.contains(click_pos):
            return

        # Arrow icon + left padding hotspot width
        if click_pos.x() > (cell_rect.left() + 24):
            return

        # Toggle expansion
        group_key = meta.get("_GROUP_KEY", "")
        if not group_key:
            return

        if group_key in self._expanded_groups:
            self._expanded_groups.remove(group_key)
        else:
            self._expanded_groups.add(group_key)

        # Persist expanded state
        self._save_grouping_state()

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
            if self._grouping_enabled:
                menu.addSeparator()

                meta = self._row_metadata[row] if row < len(self._row_metadata) else {}
                obs_id = row_values.get("OBSERVATION_ID", "")
                group_key = meta.get("_GROUP_KEY", "")
                group_members = meta.get("_GROUP_MEMBERS", [])
                header_obs_id = meta.get("_GROUP_HEADER_OBS_ID", "")
                selected_obs_id = meta.get("_GROUP_SELECTED_OBS_ID", "")

                if (
                    group_key
                    and (meta.get("_IS_GROUP_MEMBER") or meta.get("_IS_GROUP_HEADER"))
                    and isinstance(group_members, list)
                    and group_members
                ):
                    seq_menu = menu.addMenu("Use For Sequencer")
                    ordered_members = sorted(
                        group_members,
                        key=lambda m: int(str(m.get("obs_order", "0")).strip())
                        if str(m.get("obs_order", "0")).strip().lstrip("-").isdigit() else 0
                    )
                    if not selected_obs_id and header_obs_id:
                        selected_obs_id = self._selected_obs_id_by_header.get(header_obs_id, "")

                    for member in ordered_members:
                        member_obs_id = str(member.get("obs_id", "")).strip()
                        if not member_obs_id:
                            continue
                        label = str(member.get("name", member_obs_id)).strip() or member_obs_id
                        action = seq_menu.addAction(label)
                        action.setCheckable(True)
                        if selected_obs_id and member_obs_id == selected_obs_id:
                            action.setChecked(True)
                        action.triggered.connect(
                            lambda checked, gk=group_key, oid=member_obs_id: self._set_sequencer_target(gk, oid)
                        )
                    menu.addSeparator()

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

        self._save_grouping_state()
        self._update_reorder_drag_state()

        # Refresh to apply/remove grouping
        self.refresh()

    def _grouping_settings_key(self, suffix: str) -> str:
        if not self._table_name:
            return suffix
        return f"tableGrouping/{self._table_name}/{suffix}"

    def _row_value_case_insensitive(self, row_data: Dict[str, Any], key: str) -> Tuple[Any, bool]:
        if key in row_data:
            return row_data[key], True
        key_lower = key.lower()
        for row_key, row_value in row_data.items():
            if str(row_key).lower() == key_lower:
                return row_value, True
        return None, False

    def _row_text_case_insensitive(self, row_data: Dict[str, Any], key: str) -> str:
        value, found = self._row_value_case_insensitive(row_data, key)
        if not found or value is None:
            return ""
        return str(value).strip()

    def _parse_float(self, value: Any) -> Optional[float]:
        text = str(value).strip() if value is not None else ""
        if not text:
            return None
        try:
            return float(text)
        except (TypeError, ValueError):
            return None

    def _parse_angle_degrees(self, text: str, is_ra: bool) -> Optional[float]:
        value = str(text).strip()
        if not value:
            return None

        if ":" in value:
            parts = value.split(":")
            if len(parts) < 2:
                return None
            first = parts[0].strip()
            neg = (not is_ra) and first.startswith("-")
            try:
                a = abs(float(first))
                b = abs(float(parts[1].strip()))
            except (TypeError, ValueError):
                return None
            c = 0.0
            if len(parts) > 2:
                try:
                    c = abs(float(parts[2].strip()))
                except (TypeError, ValueError):
                    c = 0.0

            deg = (a + b / 60.0 + c / 3600.0) * (15.0 if is_ra else 1.0)
            if not is_ra and neg:
                deg = -deg
        else:
            try:
                deg = float(value)
            except (TypeError, ValueError):
                return None
            if is_ra and abs(deg) <= 24.0:
                deg *= 15.0

        if is_ra:
            while deg < 0.0:
                deg += 360.0
            while deg >= 360.0:
                deg -= 360.0

        return deg

    def _offset_arcsec_from_row(self, row_data: Dict[str, Any], keys: Tuple[str, ...]) -> Tuple[float, bool]:
        for key in keys:
            raw_value, found = self._row_value_case_insensitive(row_data, key)
            if not found:
                continue
            text = str(raw_value).strip() if raw_value is not None else ""
            if not text:
                continue
            parsed = self._parse_float(text)
            if parsed is not None:
                return parsed, True
        return 0.0, False

    def _compute_projected_science_coord(self, row_data: Dict[str, Any]) -> Tuple[bool, float, float]:
        ra_text = self._row_text_case_insensitive(row_data, "RA")
        dec_text = self._row_text_case_insensitive(row_data, "DECL")
        if not ra_text or not dec_text:
            return False, 0.0, 0.0

        ra_deg = self._parse_angle_degrees(ra_text, is_ra=True)
        dec_deg = self._parse_angle_degrees(dec_text, is_ra=False)
        if ra_deg is None or dec_deg is None:
            return False, 0.0, 0.0

        offset_ra, _ = self._offset_arcsec_from_row(row_data, ("OFFSET_RA", "DRA"))
        offset_dec, _ = self._offset_arcsec_from_row(row_data, ("OFFSET_DEC", "DDEC"))

        deg2rad = math.pi / 180.0
        ra0 = ra_deg * deg2rad
        dec0 = dec_deg * deg2rad
        xi = (offset_ra / 3600.0) * deg2rad
        eta = (offset_dec / 3600.0) * deg2rad

        sin_dec0 = math.sin(dec0)
        cos_dec0 = math.cos(dec0)
        denom = cos_dec0 - eta * sin_dec0
        ra1 = ra0 + math.atan2(xi, denom)
        dec1 = math.atan2(sin_dec0 + eta * cos_dec0, math.sqrt((denom * denom) + (xi * xi)))

        ra_out = ra1 / deg2rad
        dec_out = dec1 / deg2rad
        while ra_out < 0.0:
            ra_out += 360.0
        while ra_out >= 360.0:
            ra_out -= 360.0

        return True, ra_out, dec_out

    def _angular_separation_arcsec(self, ra1_deg: float, dec1_deg: float,
                                   ra2_deg: float, dec2_deg: float) -> float:
        deg2rad = math.pi / 180.0
        ra1 = ra1_deg * deg2rad
        dec1 = dec1_deg * deg2rad
        ra2 = ra2_deg * deg2rad
        dec2 = dec2_deg * deg2rad
        cosd = (
            math.sin(dec1) * math.sin(dec2) +
            math.cos(dec1) * math.cos(dec2) * math.cos(ra1 - ra2)
        )
        clamped = max(-1.0, min(1.0, cosd))
        sep_rad = math.acos(clamped)
        return sep_rad * (180.0 / math.pi) * 3600.0

    def _coordinate_key(self, ra_deg: float, dec_deg: float) -> str:
        tol_deg = GROUP_COORD_TOL_ARCSEC / 3600.0
        if tol_deg > 0.0:
            ra_steps = ra_deg / tol_deg
            dec_steps = dec_deg / tol_deg
            if ra_steps >= 0:
                ra_steps = math.floor(ra_steps + 0.5)
            else:
                ra_steps = math.ceil(ra_steps - 0.5)
            if dec_steps >= 0:
                dec_steps = math.floor(dec_steps + 0.5)
            else:
                dec_steps = math.ceil(dec_steps - 0.5)
            ra_deg = ra_steps * tol_deg
            dec_deg = dec_steps * tol_deg
        while ra_deg < 0.0:
            ra_deg += 360.0
        while ra_deg >= 360.0:
            ra_deg -= 360.0
        return f"{ra_deg:.6f}:{dec_deg:.6f}"

    def _build_group_rows(self, rows_data: List[Dict[str, Any]]) -> List[Dict[str, Any]]:
        row_infos: List[Dict[str, Any]] = []
        for row_data in rows_data:
            obs_id = str(row_data.get("OBSERVATION_ID", "")).strip()
            name = str(row_data.get("NAME", "")).strip()

            offset_ra, has_offset_ra = self._offset_arcsec_from_row(row_data, ("OFFSET_RA", "DRA"))
            offset_dec, has_offset_dec = self._offset_arcsec_from_row(row_data, ("OFFSET_DEC", "DDEC"))
            is_science = (
                (not has_offset_ra and not has_offset_dec) or
                (abs(offset_ra) <= OFFSET_ZERO_TOL_ARCSEC and abs(offset_dec) <= OFFSET_ZERO_TOL_ARCSEC)
            )

            obs_order = 0
            obs_order_raw = row_data.get("OBS_ORDER", 0)
            try:
                obs_order = int(float(obs_order_raw))
            except (TypeError, ValueError):
                obs_order = 0

            coord_ok, ra_deg, dec_deg = self._compute_projected_science_coord(row_data)
            row_infos.append({
                "obs_id": obs_id,
                "name": name,
                "row_data": row_data,
                "group_key": "",
                "is_science": is_science,
                "obs_order": obs_order,
                "coord_ok": coord_ok,
                "ra_deg": ra_deg,
                "dec_deg": dec_deg,
            })

        centers: List[Dict[str, Any]] = []
        for info in row_infos:
            if info["obs_id"] in self._manual_ungroup_obs_ids:
                continue
            if info["coord_ok"] and info["is_science"]:
                centers.append({
                    "key": self._coordinate_key(info["ra_deg"], info["dec_deg"]),
                    "ra": info["ra_deg"],
                    "dec": info["dec_deg"],
                })

        has_science_centers = len(centers) > 0
        for info in row_infos:
            obs_id = info["obs_id"]
            if obs_id in self._manual_ungroup_obs_ids:
                info["group_key"] = f"UNGROUP:{obs_id}"
                continue

            manual_key = self._manual_group_key_by_obs_id.get(obs_id, "")
            if manual_key:
                info["group_key"] = manual_key
                continue

            if not info["coord_ok"]:
                info["group_key"] = f"OBS:{obs_id}"
                continue

            if not has_science_centers:
                info["group_key"] = f"OBS:{obs_id}"
                continue

            best_sep = 1e12
            best_idx = -1
            for idx, center in enumerate(centers):
                sep = self._angular_separation_arcsec(
                    info["ra_deg"], info["dec_deg"], center["ra"], center["dec"]
                )
                if sep < best_sep:
                    best_sep = sep
                    best_idx = idx

            if best_idx >= 0 and best_sep <= GROUP_COORD_TOL_ARCSEC:
                info["group_key"] = centers[best_idx]["key"]
            else:
                info["group_key"] = f"OBS:{obs_id}"

        return row_infos

    def _apply_grouping_to_display(self, rows_data: List[Dict[str, Any]]) -> List[Dict[str, Any]]:
        if not self._grouping_enabled or not rows_data:
            self._group_data = {}
            return rows_data

        rows = self._build_group_rows(rows_data)
        if not rows:
            self._group_data = {}
            return rows_data

        rows_by_key: Dict[str, List[Dict[str, Any]]] = {}
        for info in rows:
            rows_by_key.setdefault(info["group_key"], []).append(info)

        selected_by_key: Dict[str, str] = {}
        valid_header_obs_ids: Set[str] = set()
        selection_changed = False
        self._group_data = {}
        group_order_records: List[Tuple[int, str]] = []

        for group_key, members in rows_by_key.items():
            ordered_members = sorted(members, key=lambda info: info["obs_order"])
            header_info = next(
                (member for member in ordered_members if member.get("is_science", False)),
                ordered_members[0]
            )
            header_obs_id = str(header_info.get("obs_id", "")).strip()
            if not header_obs_id:
                continue
            valid_header_obs_ids.add(header_obs_id)
            group_order_records.append((int(header_info.get("obs_order", 0)), group_key))

            member_obs_ids = {
                str(member.get("obs_id", "")).strip()
                for member in ordered_members
                if str(member.get("obs_id", "")).strip()
            }

            selected_obs_id = self._selected_obs_id_by_header.get(header_obs_id, "")
            if selected_obs_id not in member_obs_ids:
                default_obs_id = ""
                for member in ordered_members:
                    member_obs_id = str(member.get("obs_id", "")).strip()
                    if member_obs_id and not member.get("is_science", False):
                        default_obs_id = member_obs_id
                        break
                if not default_obs_id:
                    default_obs_id = header_obs_id
                selected_obs_id = default_obs_id
                self._selected_obs_id_by_header[header_obs_id] = selected_obs_id
                selection_changed = True

            selected_by_key[group_key] = selected_obs_id
            self._group_data[group_key] = {
                "header_obs_id": header_obs_id,
                "members": ordered_members,
            }

        for header_obs_id in list(self._selected_obs_id_by_header.keys()):
            if header_obs_id not in valid_header_obs_ids:
                del self._selected_obs_id_by_header[header_obs_id]
                selection_changed = True

        if selection_changed:
            self._save_grouping_state()

        # C++ parity: enforce group STATE selection semantics.
        # For rows whose current STATE is empty/pending/unassigned:
        # - selected member in a group -> DEFAULT_TARGET_STATE (pending)
        # - non-selected members       -> unassigned
        state_updates: List[Tuple[Dict[str, Any], str, Dict[str, Any], str]] = []

        def _row_state_key(row_dict: Dict[str, Any]) -> str:
            if "STATE" in row_dict:
                return "STATE"
            for k in row_dict.keys():
                if str(k).upper() == "STATE":
                    return str(k)
            return "STATE"

        def _should_override_state(raw_state: Any) -> bool:
            s = str(raw_state).strip().lower() if raw_state is not None else ""
            return (s == "") or (s == "pending") or (s == "unassigned")

        desired_pending_state = str(DEFAULT_TARGET_STATE).strip() if str(DEFAULT_TARGET_STATE).strip() else "pending"

        if self._db and self._db.is_open() and rows_by_key:
            for group_key, members in rows_by_key.items():
                selected_obs_id = selected_by_key.get(group_key, "")
                if not selected_obs_id:
                    continue
                for member in members:
                    row_data = member.get("row_data", {})
                    if not isinstance(row_data, dict):
                        continue
                    state_key = _row_state_key(row_data)
                    current_state = row_data.get(state_key, "")
                    if not _should_override_state(current_state):
                        continue

                    member_obs_id = str(member.get("obs_id", "")).strip()
                    if not member_obs_id:
                        continue
                    is_selected = member_obs_id and (member_obs_id == selected_obs_id)
                    desired_state = desired_pending_state if is_selected else "unassigned"
                    if str(current_state).strip().lower() == desired_state.lower():
                        continue

                    key_values: Dict[str, Any] = {"OBSERVATION_ID": member_obs_id}
                    if "SET_ID" in row_data and row_data.get("SET_ID") not in ("", None):
                        key_values["SET_ID"] = row_data.get("SET_ID")

                    state_updates.append((key_values, desired_state, row_data, state_key))

            for key_values, desired_state, row_data, state_key in state_updates:
                try:
                    self._db.update_record(self._table_name, key_values, {"STATE": desired_state})
                    row_data[state_key] = desired_state
                except Exception as err:
                    print(f"WARN: failed to update STATE selection: {err}")

        group_any_pending: Dict[str, bool] = {}
        for group_key, members in rows_by_key.items():
            any_pending = False
            for member in members:
                row_data = member.get("row_data", {})
                if not isinstance(row_data, dict):
                    continue
                state_key = _row_state_key(row_data)
                state_value = str(row_data.get(state_key, "")).strip().lower()
                if state_value == "pending":
                    any_pending = True
                    break
            group_any_pending[group_key] = any_pending

        group_order = [key for _, key in sorted(group_order_records, key=lambda pair: pair[0])]

        display_rows: List[Dict[str, Any]] = []
        for group_key in group_order:
            group_info = self._group_data.get(group_key, {})
            ordered_members = group_info.get("members", [])
            if not ordered_members:
                continue
            header_obs_id = str(group_info.get("header_obs_id", "")).strip()
            selected_obs_id = selected_by_key.get(group_key, "")
            expanded = group_key in self._expanded_groups
            group_size = len(ordered_members)

            header_member = None
            for member in ordered_members:
                if str(member.get("obs_id", "")).strip() == header_obs_id:
                    header_member = member
                    break
            if header_member is None:
                header_member = ordered_members[0]

            group_members_meta = []
            for member in ordered_members:
                group_members_meta.append({
                    "obs_id": str(member.get("obs_id", "")).strip(),
                    "name": str(member.get("name", "")).strip(),
                    "obs_order": int(member.get("obs_order", 0)),
                    "is_science": bool(member.get("is_science", False)),
                })

            header_row = dict(header_member.get("row_data", {}))
            header_name = str(header_row.get("NAME", ""))
            header_selected = bool(selected_obs_id and selected_obs_id == header_obs_id)
            if header_selected:
                header_name = f"★ {header_name}"
            if group_size > 1:
                icon = "▼" if expanded else "▶"
                header_row["NAME"] = f"{icon} {header_name}"
            else:
                header_row["NAME"] = header_name
            header_row["_IS_GROUP_HEADER"] = True
            header_row["_GROUP_KEY"] = group_key
            header_row["_GROUP_MEMBERS"] = group_members_meta
            header_row["_GROUP_HEADER_OBS_ID"] = header_obs_id
            header_row["_GROUP_SELECTED_OBS_ID"] = selected_obs_id
            header_row["_GROUP_IS_SELECTED"] = header_selected
            if (not expanded) and group_any_pending.get(group_key, False):
                state_key = _row_state_key(header_row)
                header_row[state_key] = desired_pending_state
            display_rows.append(header_row)

            if expanded and group_size > 1:
                for member in ordered_members:
                    member_obs_id = str(member.get("obs_id", "")).strip()
                    if member_obs_id == header_obs_id:
                        continue
                    member_row = dict(member.get("row_data", {}))
                    member_name = str(member_row.get("NAME", ""))
                    member_selected = bool(selected_obs_id and member_obs_id == selected_obs_id)
                    if member_selected:
                        member_row["NAME"] = f"  ★ {member_name}"
                    else:
                        member_row["NAME"] = f"    {member_name}"
                    member_row["_IS_GROUP_MEMBER"] = True
                    member_row["_GROUP_KEY"] = group_key
                    member_row["_GROUP_MEMBERS"] = group_members_meta
                    member_row["_GROUP_HEADER_OBS_ID"] = header_obs_id
                    member_row["_GROUP_SELECTED_OBS_ID"] = selected_obs_id
                    member_row["_GROUP_IS_SELECTED"] = member_selected
                    display_rows.append(member_row)

        return display_rows

    def _ungroup_target(self, obs_id: str) -> None:
        """Manually ungroup a target."""
        if obs_id:
            self._manual_ungroup_obs_ids.add(obs_id)
            self._manual_group_key_by_obs_id.pop(obs_id, None)
            self._save_grouping_state()
            self.refresh()

    def _regroup_target(self, obs_id: str) -> None:
        """Restore automatic grouping for a target."""
        if obs_id in self._manual_ungroup_obs_ids or obs_id in self._manual_group_key_by_obs_id:
            self._manual_ungroup_obs_ids.discard(obs_id)
            self._manual_group_key_by_obs_id.pop(obs_id, None)
            self._save_grouping_state()
            self.refresh()

    def _set_sequencer_target(self, group_key: str, obs_id: str) -> None:
        """Store sequencer selection using C++ parity: selected obs id keyed by header obs id."""
        group_info = self._group_data.get(group_key, {})
        members = group_info.get("members", [])
        header_obs_id = str(group_info.get("header_obs_id", "")).strip()
        if not header_obs_id or not members:
            return
        in_group = any(str(member.get("obs_id", "")).strip() == obs_id for member in members)
        if not in_group:
            return
        self._selected_obs_id_by_header[header_obs_id] = obs_id
        self._save_grouping_state()
        self.refresh()

    def _save_grouping_state(self) -> None:
        """Persist grouping state to QSettings."""
        settings = QSettings(SETTINGS_ORG, SETTINGS_APP)
        prefix = self._settings_key_prefix
        settings.setValue(f"{prefix}/groupingEnabled", self._grouping_enabled)
        settings.setValue(f"{prefix}/expandedGroups", sorted(self._expanded_groups))

        ungrouped = sorted([obs_id for obs_id in self._manual_ungroup_obs_ids if str(obs_id).strip()])
        settings.setValue(self._grouping_settings_key("manualUngroup"), ungrouped)
        settings.setValue(f"{prefix}/manualUngroupObsIds", ungrouped)

        manual_groups = []
        for obs_id, group_key in self._manual_group_key_by_obs_id.items():
            obs_text = str(obs_id).strip()
            group_text = str(group_key).strip()
            if obs_text and group_text:
                manual_groups.append(f"{obs_text}={group_text}")
        manual_groups.sort()
        settings.setValue(self._grouping_settings_key("manualGroups"), manual_groups)

        selected_obs = []
        for header_obs_id, selected_obs_id in self._selected_obs_id_by_header.items():
            header_text = str(header_obs_id).strip()
            selected_text = str(selected_obs_id).strip()
            if header_text and selected_text:
                selected_obs.append(f"{header_text}={selected_text}")
        selected_obs.sort()
        settings.setValue(self._grouping_settings_key("selectedObs"), selected_obs)

    def _restore_grouping_state(self) -> None:
        """Restore grouping state from QSettings."""
        settings = QSettings(SETTINGS_ORG, SETTINGS_APP)
        prefix = self._settings_key_prefix

        # Restore grouping enabled (default True for targets table)
        default_enabled = self._is_targets_table()
        val = settings.value(f"{prefix}/groupingEnabled", default_enabled)
        # QSettings may return string "true"/"false" instead of bool
        if isinstance(val, str):
            self._grouping_enabled = val.lower() == "true"
        else:
            self._grouping_enabled = bool(val)

        # Restore expanded groups
        expanded = settings.value(f"{prefix}/expandedGroups", [])
        if isinstance(expanded, list):
            self._expanded_groups = set(expanded)
        elif isinstance(expanded, str):
            self._expanded_groups = {expanded}
        else:
            self._expanded_groups = set()

        # Restore manual ungroup obs ids
        self._manual_ungroup_obs_ids = set()
        ungrouped = settings.value(
            self._grouping_settings_key("manualUngroup"),
            settings.value(f"{prefix}/manualUngroupObsIds", [])
        )
        if isinstance(ungrouped, str):
            ungrouped = [ungrouped]
        if isinstance(ungrouped, list):
            for obs_id in ungrouped:
                obs_text = str(obs_id).strip()
                if obs_text:
                    self._manual_ungroup_obs_ids.add(obs_text)

        self._manual_group_key_by_obs_id = {}
        manual_groups = settings.value(self._grouping_settings_key("manualGroups"), [])
        if isinstance(manual_groups, str):
            manual_groups = [manual_groups]
        if isinstance(manual_groups, list):
            for entry in manual_groups:
                text = str(entry).strip()
                if "=" not in text:
                    continue
                obs_id, group_key = text.split("=", 1)
                obs_text = obs_id.strip()
                group_text = group_key.strip()
                if obs_text and group_text:
                    self._manual_group_key_by_obs_id[obs_text] = group_text

        self._selected_obs_id_by_header = {}
        selected_obs_entries = settings.value(self._grouping_settings_key("selectedObs"), [])
        if isinstance(selected_obs_entries, str):
            selected_obs_entries = [selected_obs_entries]
        if isinstance(selected_obs_entries, list):
            for entry in selected_obs_entries:
                text = str(entry).strip()
                if "=" not in text:
                    continue
                header_obs_id, selected_obs_id = text.split("=", 1)
                header_text = header_obs_id.strip()
                selected_text = selected_obs_id.strip()
                if header_text and selected_text:
                    self._selected_obs_id_by_header[header_text] = selected_text

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
        """Launch bulk edit dialog for all targets in the current view scope."""
        try:
            from bulk_operations_dialog import BulkEditDialog
        except ImportError:
            QMessageBox.warning(
                self,
                "Feature Not Available",
                "Bulk edit dialog is not available."
            )
            return

        # C++ parity: bulk edit applies to all targets in current table scope
        # (current set + search filter), not just currently selected row(s).
        target_rows = self._get_bulk_edit_target_rows()
        if not target_rows:
            QMessageBox.warning(
                self,
                "No Targets",
                "No targets found to bulk edit."
            )
            return

        # Ask user which column to edit
        column_names = [
            col.name for col in self._columns
            if not col.is_primary and not col.is_auto_increment
        ]
        if not column_names:
            QMessageBox.warning(self, "Error", "No editable columns available.")
            return
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
            num_rows=len(target_rows)
        )

        if dialog.exec_() != QDialog.Accepted:
            return

        # Apply the bulk edit operation
        operation = dialog.get_operation()
        self._apply_bulk_edit(target_rows, column_name, operation)

    def _get_bulk_edit_target_rows(self) -> List[Dict[str, Any]]:
        """Fetch rows in the same scope as the visible targets table."""
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
        return self._db.fetch_rows(
            self._table_name,
            where=where,
            params=tuple(params),
            order_by=order_by,
        )

    def _apply_bulk_edit(self, target_rows: List[Dict[str, Any]], column_name: str, operation: Dict[str, Any]) -> None:
        """Apply bulk edit to all target rows, then refresh once."""
        if not self._db.is_open():
            QMessageBox.warning(self, "Error", "Database not connected")
            return

        try:
            primary_keys = [col.name for col in self._columns if col.is_primary]
            if not primary_keys:
                fallback_keys = []
                if "OBSERVATION_ID" in self._column_index:
                    fallback_keys.append("OBSERVATION_ID")
                if "SET_ID" in self._column_index:
                    fallback_keys.append("SET_ID")
                primary_keys = fallback_keys
            if not primary_keys:
                QMessageBox.warning(self, "Error", "Primary key column not found")
                return

            skip = object()

            def compute_new_value(current_value: Any) -> Any:
                op_type = str(operation.get("type", "")).strip().lower()
                if op_type == "set":
                    return operation.get("value")
                if op_type == "null":
                    return None
                if op_type == "append":
                    base = "" if current_value is None else str(current_value)
                    return base + str(operation.get("value", ""))
                if op_type == "prepend":
                    base = "" if current_value is None else str(current_value)
                    return str(operation.get("value", "")) + base
                if op_type == "add":
                    try:
                        return str(float(current_value) + float(operation.get("value")))
                    except (TypeError, ValueError):
                        return skip
                if op_type == "multiply":
                    try:
                        return str(float(current_value) * float(operation.get("value")))
                    except (TypeError, ValueError):
                        return skip
                return skip

            updates_to_apply: List[Tuple[Dict[str, Any], Any]] = []
            skipped_rows = 0
            for row_data in target_rows:
                key_values: Dict[str, Any] = {}
                for key in primary_keys:
                    if key not in row_data:
                        continue
                    val = row_data.get(key)
                    if val in ("", None):
                        continue
                    key_values[key] = val
                if len(key_values) != len(primary_keys):
                    skipped_rows += 1
                    continue

                new_value = compute_new_value(row_data.get(column_name))
                if new_value is skip:
                    skipped_rows += 1
                    continue
                updates_to_apply.append((key_values, new_value))

            if not updates_to_apply:
                QMessageBox.information(
                    self,
                    "Bulk Edit Complete",
                    "No rows were eligible for this operation."
                )
                return

            updates = 0
            self._db.start_transaction()
            try:
                for key_values, new_value in updates_to_apply:
                    self._db.update_record(self._table_name, key_values, {column_name: new_value})
                    updates += 1
                self._db.commit()
            except Exception:
                self._db.rollback()
                raise

            QMessageBox.information(
                self,
                "Bulk Edit Complete",
                f"Successfully updated {updates} rows."
                + (f"\nSkipped {skipped_rows} rows." if skipped_rows else "")
            )

            # Single refresh/emit after full batch so OTM reruns only once.
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
        self._otm_ui_locked = False
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
            self.targets_table.reorder_requested.connect(self._on_timeline_reorder)

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

            # Get targets for this set and build C++-parity OTM group context
            targets = self._get_targets_for_otm(set_id)
            if not targets:
                print(f"OTM: no targets found in set {set_id}")
                return
            otm_targets, members_by_science_obs, obs_order_by_obs_id = self._build_otm_group_context_for_set(
                set_id, targets
            )
            if not otm_targets:
                print(f"OTM: no grouped science targets found in set {set_id}")
                return

            # Create temporary files
            import tempfile
            temp_dir = tempfile.mkdtemp(prefix="ngps_otm_")
            input_csv = os.path.join(temp_dir, "otm_input.csv")
            output_csv = os.path.join(temp_dir, "otm_output.csv")
            timeline_json = os.path.join(temp_dir, "timeline.json")

            # Generate input CSV (filters targets outside observable DEC range)
            skipped = generate_otm_input_csv(otm_targets, input_csv)
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
            self._set_otm_ui_locked(True)
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
                    progress=None,
                    members_by_science_obs=members_by_science_obs,
                    obs_order_by_obs_id=obs_order_by_obs_id,
                )
            )
            self._otm_runner.finished.connect(self._otm_thread.quit)

            # Start
            self._otm_thread.start()

        except Exception as e:
            print(f"OTM: failed to run: {e}")
            if self._otm_running:
                self._finish_otm_pipeline(False)

    def _on_otm_finished(self, success: bool, message: str, input_csv: str, output_csv: str,
                         timeline_json: str, set_id: str, start_utc: str,
                         progress: QProgressDialog, silent: bool = False,
                         members_by_science_obs: Optional[Dict[str, List[str]]] = None,
                         obs_order_by_obs_id: Optional[Dict[str, int]] = None) -> None:
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
            self._import_otm_results(set_id, otm_results, members_by_science_obs=members_by_science_obs)

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
                ok, msg, json_path, len(otm_results), settings.airmass_max, progress, silent,
                obs_order_by_obs_id=obs_order_by_obs_id,
            )
        )
        self._timeline_runner.finished.connect(self._timeline_thread.quit)
        if progress:
            progress.canceled.connect(self._timeline_runner.cancel)
            progress.canceled.connect(self._timeline_thread.quit)

        self._timeline_thread.start()

    def _on_timeline_finished(self, success: bool, message: str, json_path: str,
                              num_results: int, airmass_limit: float,
                              progress: QProgressDialog, silent: bool = False,
                              obs_order_by_obs_id: Optional[Dict[str, int]] = None) -> None:
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

            # C++ parity: reapply table OBS_ORDER to timeline rows when available.
            if obs_order_by_obs_id and timeline_data and timeline_data.targets:
                ordered_pairs: List[Tuple[int, str]] = []
                for target in timeline_data.targets:
                    obs_id = str(getattr(target, "obs_id", "")).strip()
                    raw_order = obs_order_by_obs_id.get(obs_id, 10**9)
                    ordered_pairs.append((int(raw_order), obs_id))

                ordered_pairs.sort(key=lambda pair: (pair[0], pair[1]))
                seq_by_obs: Dict[str, int] = {}
                seq = 1
                for _, obs_id in ordered_pairs:
                    if obs_id and obs_id not in seq_by_obs:
                        seq_by_obs[obs_id] = seq
                        seq += 1

                for target in timeline_data.targets:
                    obs_id = str(getattr(target, "obs_id", "")).strip()
                    if obs_id in seq_by_obs:
                        target.obs_order = seq_by_obs[obs_id]

                def _parse_iso(ts: str):
                    text = str(ts or "").strip()
                    if not text:
                        return None
                    try:
                        from datetime import datetime, timezone
                        dt = datetime.fromisoformat(text.replace("Z", "+00:00"))
                        if dt.tzinfo is None:
                            dt = dt.replace(tzinfo=timezone.utc)
                        return dt
                    except Exception:
                        return None

                from datetime import datetime, timezone
                fallback_dt = datetime.max.replace(tzinfo=timezone.utc)
                timeline_data.targets = sorted(
                    timeline_data.targets,
                    key=lambda t: (
                        0 if int(getattr(t, "obs_order", 0) or 0) > 0 else 1,
                        int(getattr(t, "obs_order", 0) or 0) if int(getattr(t, "obs_order", 0) or 0) > 0 else 0,
                        _parse_iso(getattr(t, "start", "")) or fallback_dt,
                        str(getattr(t, "name", "") or ""),
                    ),
                )

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
        if self._otm_running or self._otm_ui_locked:
            print(f"DB write blocked ({source}): OTM pipeline running")
            return False
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
        self._set_otm_ui_locked(False)
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

    def _find_visible_row_for_obs_id(self, obs_id: str) -> int:
        """Return visible table row for OBSERVATION_ID, or -1 if not displayed."""
        table = self.targets_table.table
        obs_col = self.targets_table._column_index.get("OBSERVATION_ID")
        if obs_col is None:
            return -1
        target = str(obs_id).strip()
        for row in range(table.rowCount()):
            item = table.item(row, obs_col)
            if item and item.text().strip() == target:
                return row
        return -1

    def _find_group_header_row_for_obs_id(self, obs_id: str) -> int:
        """When grouped/collapsed, resolve member OBSERVATION_ID to its header row."""
        target = str(obs_id).strip()
        if not target:
            return -1
        for row, meta in enumerate(getattr(self.targets_table, "_row_metadata", [])):
            if not isinstance(meta, dict):
                continue
            members = meta.get("_GROUP_MEMBERS", [])
            if not isinstance(members, list):
                continue
            for member in members:
                member_obs_id = str(member.get("obs_id", "")).strip() if isinstance(member, dict) else ""
                if member_obs_id == target:
                    return row
        return -1

    def _select_obs_id_in_table(self, obs_id: str) -> None:
        """Select the row matching obs_id in the targets table."""
        table = self.targets_table.table
        row = self._find_visible_row_for_obs_id(obs_id)
        if row < 0:
            row = self._find_group_header_row_for_obs_id(obs_id)
        if row < 0:
            return

        table.selectRow(row)
        obs_col = self.targets_table._column_index.get("OBSERVATION_ID")
        item = table.item(row, obs_col) if obs_col is not None else table.item(row, 0)
        if item:
            table.scrollToItem(item)

        values = self.targets_table._current_row_values(row)
        if values:
            self._update_control_tab_fields(values)

    def _fetch_target_row_by_obs_id(self, obs_id: str) -> Dict[str, Any]:
        """Fetch a target row by OBSERVATION_ID from DB."""
        if not self._db.is_open():
            return {}
        target = str(obs_id).strip()
        if not target:
            return {}
        rows = self._db.fetch_rows(
            self._config.table_targets,
            where="`OBSERVATION_ID`=%s",
            params=(target,),
        )
        return rows[0] if rows else {}

    def _resolve_set_id_for_obs(self, obs_id: str) -> str:
        """Resolve SET_ID for an observation, falling back to current set."""
        row = self._fetch_target_row_by_obs_id(obs_id)
        if row:
            set_id = str(row.get("SET_ID", "")).strip()
            if set_id:
                return set_id
        if self._current_set_id is None:
            return ""
        return str(self._current_set_id).strip()

    def _fetch_ordered_rows_for_set(self, set_id: str) -> List[Dict[str, Any]]:
        """Fetch all rows for a set ordered by OBS_ORDER then OBSERVATION_ID."""
        if not self._db.is_open():
            return []
        sid = str(set_id).strip()
        if not sid:
            return []
        return self._db.fetch_rows(
            self._config.table_targets,
            where="`SET_ID`=%s",
            params=(sid,),
            order_by="OBS_ORDER, OBSERVATION_ID",
        )

    def _write_ordered_obs_ids(self, rows: List[Dict[str, Any]]) -> None:
        """Persist OBS_ORDER as the given OBSERVATION_ID list order (1..N)."""
        ordered_keys: List[Dict[str, Any]] = []
        for row in rows:
            obs_id = str(row.get("OBSERVATION_ID", "")).strip()
            if obs_id:
                ordered_keys.append({"OBSERVATION_ID": obs_id})
        if not ordered_keys:
            return
        self._db.update_obs_order(self._config.table_targets, ["OBSERVATION_ID"], ordered_keys)

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

        # Get targets and build grouped OTM context
        targets = self._get_targets_for_otm(set_id)
        if not targets:
            return
        otm_targets, members_by_science_obs, obs_order_by_obs_id = self._build_otm_group_context_for_set(
            set_id, targets
        )
        if not otm_targets:
            return

        # Generate input CSV
        import tempfile
        temp_dir = tempfile.mkdtemp(prefix="ngps_otm_auto_")
        input_csv = os.path.join(temp_dir, "otm_input.csv")
        output_csv = os.path.join(temp_dir, "otm_output.csv")
        timeline_json = os.path.join(temp_dir, "timeline.json")

        try:
            generate_otm_input_csv(otm_targets, input_csv)
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
        self._set_otm_ui_locked(True)
        self._cleanup_otm_threads()

        # Show status bar message
        parent_window = self.window()
        if hasattr(parent_window, 'statusBar'):
            parent_window.statusBar().showMessage("Auto-running OTM scheduler...", 5000)

        try:
            # Run OTM in thread (silent — no progress dialog)
            self._otm_thread = QThread()
            self._otm_runner = OtmRunner(settings, input_csv, output_csv, start_utc)
            self._otm_runner.moveToThread(self._otm_thread)

            self._otm_thread.started.connect(self._otm_runner.run)
            self._otm_runner.finished.connect(
                lambda success, msg: self._on_otm_finished(
                    success, msg, input_csv, output_csv, timeline_json, set_id, start_utc,
                    progress=None, silent=True,
                    members_by_science_obs=members_by_science_obs,
                    obs_order_by_obs_id=obs_order_by_obs_id,
                )
            )
            self._otm_runner.finished.connect(self._otm_thread.quit)

            self._otm_thread.start()
        except Exception as e:
            print(f"OTM: failed to auto-run: {e}")
            self._finish_otm_pipeline(True)

    def _set_otm_ui_locked(self, locked: bool) -> None:
        """Enable/disable user interaction with targets and timeline panels during OTM runs."""
        self._otm_ui_locked = bool(locked)

        if hasattr(self, "targets_table") and self.targets_table is not None:
            self.targets_table.setEnabled(not locked)

        if hasattr(self, "timeline_view") and self.timeline_view is not None:
            self.timeline_view.setEnabled(not locked)
            if WEBENGINE_AVAILABLE:
                try:
                    js = "document.body.style.pointerEvents='none';" if locked else "document.body.style.pointerEvents='auto';"
                    self.timeline_view.page().runJavaScript(js)
                except Exception:
                    pass

    def _build_otm_group_context_for_set(
        self,
        set_id: str,
        targets: List[Dict[str, Any]],
    ) -> Tuple[List[Dict[str, Any]], Dict[str, List[str]], Dict[str, int]]:
        """Build C++-parity OTM context: one input row per group science target.

        Returns:
            otm_targets: science/header rows sent to OTM (ordered)
            members_by_science_obs: header/science OBS_ID -> all group member OBS_IDs
            obs_order_by_obs_id: OBS_ID -> current table OBS_ORDER for timeline ordering
        """
        def _to_int(value: Any, default: int = 0) -> int:
            try:
                return int(float(value))
            except Exception:
                return default

        obs_order_by_obs_id: Dict[str, int] = {}
        for row in targets:
            obs_id = str(row.get("OBSERVATION_ID", "")).strip()
            if not obs_id:
                continue
            obs_order_by_obs_id[obs_id] = _to_int(row.get("OBS_ORDER", 0), 0)

        # C++ parity: when grouping is disabled, each row is its own OTM target.
        if not getattr(self.targets_table, "_grouping_enabled", False):
            otm_targets: List[Dict[str, Any]] = []
            members_by_science_obs: Dict[str, List[str]] = {}
            ordered = sorted(targets, key=lambda r: (_to_int(r.get("OBS_ORDER", 0), 0), str(r.get("OBSERVATION_ID", ""))))
            for row in ordered:
                obs_id = str(row.get("OBSERVATION_ID", "")).strip()
                if not obs_id:
                    continue
                otm_targets.append(dict(row))
                members_by_science_obs[obs_id] = [obs_id]
            return otm_targets, members_by_science_obs, obs_order_by_obs_id

        row_infos = self.targets_table._build_group_rows(targets)
        if not row_infos:
            return [], {}, obs_order_by_obs_id

        rows_by_key: Dict[str, List[Dict[str, Any]]] = {}
        for info in row_infos:
            rows_by_key.setdefault(str(info.get("group_key", "")), []).append(info)

        group_records: List[Tuple[int, str, Dict[str, Any], List[str]]] = []
        members_by_science_obs: Dict[str, List[str]] = {}
        for group_key, members in rows_by_key.items():
            ordered_members = sorted(members, key=lambda info: _to_int(info.get("obs_order", 0), 0))
            if not ordered_members:
                continue

            header_info = next((m for m in ordered_members if bool(m.get("is_science", False))), ordered_members[0])
            header_obs_id = str(header_info.get("obs_id", "")).strip()
            if not header_obs_id:
                continue

            member_obs_ids: List[str] = []
            for member in ordered_members:
                member_obs_id = str(member.get("obs_id", "")).strip()
                if member_obs_id and member_obs_id not in member_obs_ids:
                    member_obs_ids.append(member_obs_id)
            if not member_obs_ids:
                member_obs_ids = [header_obs_id]

            members_by_science_obs[header_obs_id] = member_obs_ids
            header_row = dict(header_info.get("row_data", {}))
            header_order = _to_int(header_info.get("obs_order", 0), 0)
            group_records.append((header_order, group_key, header_row, member_obs_ids))

        group_records.sort(key=lambda rec: (rec[0], str(rec[1])))
        otm_targets = [rec[2] for rec in group_records]
        return otm_targets, members_by_science_obs, obs_order_by_obs_id

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

    def _import_otm_results(
        self,
        set_id: str,
        results: List,
        members_by_science_obs: Optional[Dict[str, List[str]]] = None,
    ) -> None:
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
            group_members = members_by_science_obs or {}

            all_rows = self._db.fetch_rows(
                self._config.table_targets,
                where="`SET_ID`=%s",
                params=(set_id,),
                order_by="OBS_ORDER, OBSERVATION_ID",
            )
            row_by_obs_id: Dict[str, Dict[str, Any]] = {}
            order_by_obs_id: Dict[str, int] = {}
            for row in all_rows:
                obs_id_text = str(row.get("OBSERVATION_ID", "")).strip()
                if not obs_id_text:
                    continue
                row_by_obs_id[obs_id_text] = row
                try:
                    order_by_obs_id[obs_id_text] = int(float(row.get("OBS_ORDER", 0)))
                except Exception:
                    order_by_obs_id[obs_id_text] = 0

            def _parse_utc_iso(value: Any):
                text = str(value or "").strip()
                if not text:
                    return None
                try:
                    from datetime import datetime, timezone
                    dt = datetime.fromisoformat(text.replace("Z", "+00:00"))
                    if dt.tzinfo is None:
                        dt = dt.replace(tzinfo=timezone.utc)
                    return dt
                except Exception:
                    return None

            def _unique_preserve(values: List[str]) -> List[str]:
                out: List[str] = []
                for value in values:
                    text = str(value).strip()
                    if text and text not in out:
                        out.append(text)
                return out

            # Aggregate per science OBSERVATION_ID (C++ parity: last update wins,
            # but OTMexp_start keeps earliest and OTMexp_end keeps latest).
            agg_by_obs_id: Dict[str, Dict[str, Any]] = {}
            scheduled_science_order: List[str] = []
            for result in results:
                obs_id_text = str(getattr(result, "observation_id", "")).strip()
                if not obs_id_text:
                    continue
                if obs_id_text not in scheduled_science_order:
                    scheduled_science_order.append(obs_id_text)

                updates: Dict[str, Any] = {}
                for csv_col, db_col in OTM_CSV_TO_DB.items():
                    attr = csv_field_to_attr.get(csv_col)
                    if not attr:
                        continue
                    val = getattr(result, attr, '')
                    updates[db_col] = val if val else None

                if obs_id_text not in agg_by_obs_id:
                    agg_by_obs_id[obs_id_text] = {
                        "updates": updates,
                        "start_dt": None,
                        "start_txt": "",
                        "end_dt": None,
                        "end_txt": "",
                    }
                else:
                    agg_by_obs_id[obs_id_text]["updates"] = updates

                start_txt = str(updates.get("OTMexp_start") or "").strip()
                if start_txt:
                    start_dt = _parse_utc_iso(start_txt)
                    cur_start = agg_by_obs_id[obs_id_text]["start_dt"]
                    if start_dt is not None and (cur_start is None or start_dt < cur_start):
                        agg_by_obs_id[obs_id_text]["start_dt"] = start_dt
                        agg_by_obs_id[obs_id_text]["start_txt"] = start_txt

                end_txt = str(updates.get("OTMexp_end") or "").strip()
                if end_txt:
                    end_dt = _parse_utc_iso(end_txt)
                    cur_end = agg_by_obs_id[obs_id_text]["end_dt"]
                    if end_dt is not None and (cur_end is None or end_dt > cur_end):
                        agg_by_obs_id[obs_id_text]["end_dt"] = end_dt
                        agg_by_obs_id[obs_id_text]["end_txt"] = end_txt

            scheduled_obs_ids: Set[str] = set()

            # Apply aggregated OTM updates to all members of each scheduled group.
            for science_obs_id in scheduled_science_order:
                agg = agg_by_obs_id.get(science_obs_id)
                if not agg:
                    continue
                updates = dict(agg.get("updates", {}))
                if agg.get("start_txt"):
                    updates["OTMexp_start"] = agg["start_txt"]
                if agg.get("end_txt"):
                    updates["OTMexp_end"] = agg["end_txt"]

                members = _unique_preserve(group_members.get(science_obs_id, [science_obs_id]))
                members.sort(key=lambda obs: (order_by_obs_id.get(obs, 10**9), obs))
                for member_obs_id in members:
                    if member_obs_id not in row_by_obs_id:
                        continue
                    key_values = {"OBSERVATION_ID": member_obs_id, "SET_ID": set_id}
                    self._db.update_record(self._config.table_targets, key_values, updates)
                    scheduled_obs_ids.add(member_obs_id)

            # Keep scheduled groups contiguous in OTM order, then push excluded to end.
            if scheduled_science_order:
                next_order = 1

                for science_obs_id in scheduled_science_order:
                    members = _unique_preserve(group_members.get(science_obs_id, [science_obs_id]))
                    members.sort(key=lambda obs: (order_by_obs_id.get(obs, 10**9), obs))
                    for member_obs_id in members:
                        if member_obs_id not in row_by_obs_id:
                            continue
                        key_values = {"OBSERVATION_ID": member_obs_id, "SET_ID": set_id}
                        self._db.update_record(
                            self._config.table_targets,
                            key_values,
                            {"OBS_ORDER": next_order},
                        )
                        next_order += 1
                        scheduled_obs_ids.add(member_obs_id)

                excluded_rows: List[Dict[str, Any]] = []
                for row in all_rows:
                    obs_id_text = str(row.get("OBSERVATION_ID", "")).strip()
                    if not obs_id_text:
                        continue
                    if obs_id_text not in scheduled_obs_ids:
                        excluded_rows.append(row)

                for row in excluded_rows:
                    obs_id_text = str(row.get("OBSERVATION_ID", "")).strip()
                    if not obs_id_text:
                        continue
                    key_values = {"OBSERVATION_ID": obs_id_text, "SET_ID": set_id}
                    self._db.update_record(
                        self._config.table_targets,
                        key_values,
                        {"OBS_ORDER": next_order},
                    )
                    next_order += 1

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
        if self._otm_running or self._otm_ui_locked:
            return
        if not hasattr(self, 'targets_table') or not self.targets_table:
            return
        self._select_obs_id_in_table(obs_id)

    def _on_timeline_exptime_edit(self, obs_id: str) -> None:
        """Timeline double-click → edit exposure time dialog."""
        if self._otm_running or self._otm_ui_locked:
            return
        if not self._db.is_open():
            return

        row_values = self._fetch_target_row_by_obs_id(obs_id)
        if not row_values:
            return

        current_exptime = str(row_values.get("EXPTIME", "")).strip()

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

        set_id = str(row_values.get("SET_ID", "")).strip()

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
        if self._otm_running or self._otm_ui_locked:
            return
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
        if self._otm_running or self._otm_ui_locked:
            return
        if not self._db.is_open() or direction == 0:
            return

        set_id = self._resolve_set_id_for_obs(obs_id)
        if not set_id:
            return

        ordered_rows = self._fetch_ordered_rows_for_set(set_id)
        if len(ordered_rows) < 2:
            return

        from_idx = -1
        for idx, row in enumerate(ordered_rows):
            if str(row.get("OBSERVATION_ID", "")).strip() == str(obs_id).strip():
                from_idx = idx
                break
        if from_idx < 0:
            return

        to_idx = from_idx + direction
        if to_idx < 0 or to_idx >= len(ordered_rows):
            return

        moving_row = ordered_rows.pop(from_idx)
        ordered_rows.insert(to_idx, moving_row)

        try:
            self._write_ordered_obs_ids(ordered_rows)
        except Exception as e:
            print(f"DB write failed (timeline_move): {e}")
            return
        self._refresh_all_views()
        self._select_obs_id_in_table(obs_id)

    def _on_timeline_reorder(self, from_obs_id: str, after_obs_id: str) -> None:
        """Shared drag-reorder handler (timeline/table) → move target and renumber OBS_ORDER."""
        if self._otm_running or self._otm_ui_locked:
            return
        if not self._db.is_open() or not str(from_obs_id).strip():
            return

        set_id = self._resolve_set_id_for_obs(from_obs_id)
        if not set_id:
            return

        ordered_rows = self._fetch_ordered_rows_for_set(set_id)
        if len(ordered_rows) < 2:
            return

        from_idx = -1
        to_idx = -1
        from_obs = str(from_obs_id).strip()
        to_obs = str(after_obs_id).strip()
        for idx, row in enumerate(ordered_rows):
            row_obs_id = str(row.get("OBSERVATION_ID", "")).strip()
            if row_obs_id == from_obs:
                from_idx = idx
            if to_obs and row_obs_id == to_obs:
                to_idx = idx
        if from_idx < 0:
            return

        moving_row = ordered_rows.pop(from_idx)

        if not to_obs:
            insert_idx = 0
        else:
            if to_idx < 0:
                return
            if from_idx == to_idx:
                return
            if from_idx < to_idx:
                to_idx -= 1
            insert_idx = min(to_idx + 1, len(ordered_rows))

        ordered_rows.insert(insert_idx, moving_row)

        try:
            self._write_ordered_obs_ids(ordered_rows)
        except Exception as e:
            print(f"DB write failed (timeline_reorder): {e}")
            return
        self._refresh_all_views()

        # Re-select the dragged target so timeline highlights the correct row
        self._select_obs_id_in_table(from_obs_id)
