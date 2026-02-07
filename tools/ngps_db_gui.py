import os
import sys
import re
import subprocess
import math
from dataclasses import dataclass, field
from typing import Any, Dict, List, Optional, Tuple, Set

import mysqlx
from PyQt5.QtCore import Qt, pyqtSignal, QSettings, QTimer
from PyQt5.QtWidgets import (
    QApplication,
    QDialog,
    QDialogButtonBox,
    QFormLayout,
    QHBoxLayout,
    QLabel,
    QLineEdit,
    QMainWindow,
    QMessageBox,
    QPushButton,
    QScrollArea,
    QTabWidget,
    QTableWidget,
    QTableWidgetItem,
    QVBoxLayout,
    QWidget,
    QMenu,
    QHeaderView,
)


# Defaults mirrored from tools/ngps_db_gui (C++).
DEFAULT_WRANGE_HALF_WIDTH_NM = 15.0
DEFAULT_EXPTIME = "SET 5"
DEFAULT_SLITWIDTH = "SET 1"
DEFAULT_SLITANGLE = "PA"
DEFAULT_MAGSYSTEM = "AB"
DEFAULT_MAGFILTER = "match"
DEFAULT_CHANNEL = "R"
DEFAULT_POINTMODE = "SLIT"
DEFAULT_CCDMODE = "default"
DEFAULT_NOTBEFORE = "1999-12-31T12:34:56"
DEFAULT_MAGNITUDE = 19.0
DEFAULT_AIRMASS_MAX = 4.0
DEFAULT_BIN = 1
DEFAULT_OTM_SLITWIDTH = 1.0
DEFAULT_TARGET_STATE = "pending"

SETTINGS_ORG = "NGPS"
SETTINGS_APP = "ngps_db_gui"


def channel_range_nm(channel: str) -> Optional[Tuple[float, float]]:
    """Get wavelength range for a channel."""
    ch = channel.strip().upper()
    if ch == "U":
        return (310.0, 436.0)
    if ch == "G":
        return (417.0, 590.0)
    if ch == "R":
        return (561.0, 794.0)
    if ch == "I":
        return (756.0, 1040.0)
    return None


def default_wrange_for_channel(channel: str) -> Tuple[float, float]:
    """Calculate default WRANGE_LOW/HIGH for a channel."""
    rng = channel_range_nm(channel) or channel_range_nm(DEFAULT_CHANNEL)
    min_nm, max_nm = rng
    center = 0.5 * (min_nm + max_nm)
    return (center - DEFAULT_WRANGE_HALF_WIDTH_NM, center + DEFAULT_WRANGE_HALF_WIDTH_NM)


def normalize_channel_value(text: str) -> str:
    """Normalize CHANNEL to U/G/R/I."""
    ch = text.strip().upper()
    if ch in ("U", "G", "R", "I"):
        return ch
    return DEFAULT_CHANNEL


def normalize_exptime_value(text: str, is_calib: bool = False) -> str:
    """Normalize EXPTIME to 'SET N' or 'SNR N' format."""
    val = text.strip()
    if not val:
        return DEFAULT_EXPTIME
    parts = val.split()
    if len(parts) == 1:
        try:
            num = float(parts[0])
            if num > 0:
                return f"SET {num:g}"
        except ValueError:
            pass
        return DEFAULT_EXPTIME
    key = parts[0].upper()
    if key == "EXPTIME":
        key = "SET"
    if key in ("SET", "SNR"):
        try:
            if len(parts) >= 2:
                num = float(parts[1])
                if num > 0:
                    return f"{key} {num:g}"
        except ValueError:
            pass
    return DEFAULT_EXPTIME


def normalize_slitwidth_value(text: str, is_calib: bool = False) -> str:
    """Normalize SLITWIDTH to 'SET/SNR/RES/LOSS N' or 'AUTO' format."""
    val = text.strip()
    if not val:
        return DEFAULT_SLITWIDTH
    parts = val.split()
    if len(parts) == 1:
        if parts[0].upper() == "AUTO":
            return "AUTO"
        try:
            num = float(parts[0])
            if num > 0:
                return f"SET {num:g}"
        except ValueError:
            pass
        return DEFAULT_SLITWIDTH
    key = parts[0].upper()
    if key == "AUTO":
        return "AUTO"
    if key in ("SET", "SNR", "RES", "LOSS"):
        try:
            if len(parts) >= 2:
                num = float(parts[1])
                if num > 0:
                    return f"{key} {num:g}"
        except ValueError:
            pass
    return DEFAULT_SLITWIDTH


def extract_set_numeric(text: str) -> Optional[float]:
    """Extract numeric value from 'SET N' format."""
    parts = text.strip().split()
    if len(parts) >= 2 and parts[0].upper() == "SET":
        try:
            num = float(parts[1])
            if num > 0:
                return num
        except ValueError:
            pass
    return None


def normalize_target_row(values: Dict[str, Any]) -> NormalizationResult:
    """
    Apply comprehensive normalization to a target row.
    Returns list of columns that were changed.
    """
    result = NormalizationResult()

    # Detect calibration target
    name = str(values.get("NAME", "")).strip()
    is_calib = name.upper().startswith("CAL_")

    def set_normalized(column: str, normalized: str):
        """Helper to set normalized value if changed."""
        current = str(values.get(column, ""))
        if current != normalized:
            result.changed_columns.append(column)
            values[column] = normalized

    # Normalize each field
    if "CHANNEL" in values:
        set_normalized("CHANNEL", normalize_channel_value(str(values["CHANNEL"])))

    if "EXPTIME" in values:
        set_normalized("EXPTIME", normalize_exptime_value(str(values["EXPTIME"]), is_calib))

    if "SLITWIDTH" in values:
        set_normalized("SLITWIDTH", normalize_slitwidth_value(str(values["SLITWIDTH"]), is_calib))

    # Auto-populate WRANGE_LOW/HIGH from CHANNEL if missing/invalid
    channel = values.get("CHANNEL", DEFAULT_CHANNEL)
    wrange_low, wrange_high = default_wrange_for_channel(str(channel))

    if "WRANGE_LOW" in values:
        try:
            val = float(str(values["WRANGE_LOW"]))
            if val <= 0:
                set_normalized("WRANGE_LOW", str(wrange_low))
        except (ValueError, TypeError):
            set_normalized("WRANGE_LOW", str(wrange_low))

    if "WRANGE_HIGH" in values:
        try:
            val = float(str(values["WRANGE_HIGH"]))
            if val <= 0:
                set_normalized("WRANGE_HIGH", str(wrange_high))
        except (ValueError, TypeError):
            set_normalized("WRANGE_HIGH", str(wrange_high))

    # Auto-populate OTMslitwidth from SLITWIDTH
    if "OTMslitwidth" in values and not str(values.get("OTMslitwidth", "")).strip():
        numeric = extract_set_numeric(str(values.get("SLITWIDTH", "")))
        if numeric is not None:
            set_normalized("OTMslitwidth", str(numeric))

    # Auto-populate OTMexpt from EXPTIME
    if "OTMexpt" in values and not str(values.get("OTMexpt", "")).strip():
        numeric = extract_set_numeric(str(values.get("EXPTIME", "")))
        if numeric is not None:
            set_normalized("OTMexpt", str(numeric))

    if result.changed_columns:
        result.message = f"Normalized: {', '.join(result.changed_columns)}"

    return result


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
        if isinstance(value, bytes):
            try:
                return value.decode()
            except Exception:
                return value.decode(errors="ignore")
        return str(value)

    @property
    def is_primary(self) -> bool:
        return "PRI" in self._to_str(self.key)

    @property
    def is_auto_increment(self) -> bool:
        return "auto_increment" in self._to_str(self.extra).lower()


@dataclass
class NormalizationResult:
    """Result of normalizing a target row."""
    changed_columns: List[str] = field(default_factory=list)
    message: str = ""


@dataclass
class ViewState:
    """Saved view state for scroll position and selection."""
    v_scroll: int = 0
    h_scroll: int = 0
    current_row: int = -1
    current_obs_id: str = ""


@dataclass
class DbConfig:
    host: str = ""
    port: int = 33060
    user: str = ""
    password: str = ""
    schema: str = ""
    table_targets: str = ""
    table_sets: str = ""

    def is_complete(self) -> bool:
        return all([self.host, self.user, self.schema, self.table_targets, self.table_sets])


def strip_inline_comment(line: str) -> str:
    idx = line.find("#")
    if idx >= 0:
        return line[:idx].strip()
    return line.strip()


def load_config_file(path: str) -> DbConfig:
    cfg = DbConfig()
    if not path or not os.path.exists(path):
        return cfg
    with open(path, "r") as f:
        for raw in f:
            line = raw.strip()
            if not line or line.startswith("#"):
                continue
            line = strip_inline_comment(line)
            if "=" not in line:
                continue
            key, value = [p.strip() for p in line.split("=", 1)]
            if key == "DB_HOST":
                cfg.host = value
            elif key == "DB_PORT":
                try:
                    cfg.port = int(value)
                except ValueError:
                    pass
            elif key == "DB_USER":
                cfg.user = value
            elif key == "DB_PASS":
                cfg.password = value
            elif key == "DB_SCHEMA":
                cfg.schema = value
            elif key == "DB_ACTIVE":
                cfg.table_targets = re.split(r"\s+", value)[0]
            elif key == "DB_SETS":
                cfg.table_sets = re.split(r"\s+", value)[0]
    return cfg


def detect_default_config_path() -> str:
    env_cfg = os.environ.get("NGPS_CONFIG")
    if env_cfg and os.path.exists(env_cfg):
        return env_cfg
    root = os.environ.get("NGPS_ROOT")
    if root:
        candidate = os.path.join(root, "Config", "sequencerd.cfg")
        if os.path.exists(candidate):
            return candidate
    # Walk up from this file location
    base = os.path.abspath(os.path.dirname(__file__))
    cur = base
    for _ in range(6):
        candidate = os.path.join(cur, "Config", "sequencerd.cfg")
        if os.path.exists(candidate):
            return candidate
        parent = os.path.dirname(cur)
        if parent == cur:
            break
        cur = parent
    # Try cwd
    cwd = os.getcwd()
    direct = os.path.join(cwd, "Config", "sequencerd.cfg")
    if os.path.exists(direct):
        return direct
    up_one = os.path.join(cwd, "..", "Config", "sequencerd.cfg")
    if os.path.exists(up_one):
        return up_one
    return ""


def infer_ngps_root_from_config(config_path: str) -> str:
    if not config_path:
        return ""
    config_dir = os.path.dirname(os.path.abspath(config_path))
    if os.path.basename(config_dir).lower() == "config":
        return os.path.dirname(config_dir)
    return os.path.dirname(config_path)


class DbClient:
    def __init__(self) -> None:
        self._session = None
        self._in_transaction = False

    def connect(self, cfg: DbConfig) -> None:
        self.close()
        self._session = mysqlx.get_session(
            {
                "host": cfg.host,
                "port": cfg.port,
                "user": cfg.user,
                "password": cfg.password,
                "compression": "DISABLED",
            }
        )
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
        """Begin a transaction."""
        if not self.is_open():
            raise RuntimeError("Database not connected")
        if not self._in_transaction:
            self._session.start_transaction()
            self._in_transaction = True

    def commit(self) -> None:
        """Commit current transaction."""
        if self._in_transaction:
            self._session.commit()
            self._in_transaction = False

    def rollback(self) -> None:
        """Rollback current transaction."""
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
        if not self._session or not self._session.is_open():
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
            cols.append(
                ColumnMeta(
                    name=ColumnMeta._to_str(row_dict.get("Field", "")),
                    type=ColumnMeta._to_str(row_dict.get("Type", "")),
                    nullable=ColumnMeta._to_str(row_dict.get("Null", "")).upper() == "YES",
                    key=ColumnMeta._to_str(row_dict.get("Key", "")),
                    default=row_dict.get("Default"),
                    extra=ColumnMeta._to_str(row_dict.get("Extra", "")),
                )
            )
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

    def insert_record(
        self,
        table: str,
        values: Dict[str, Any],
    ) -> None:
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

        if len(key_cols) == 2:
            k1, k2 = key_cols
            cases = []
            tuple_vals = []
            for idx, key_vals in enumerate(ordered_keys, start=1):
                cases.append(f"WHEN `{k1}`=%s AND `{k2}`=%s THEN %s")
                tuple_vals.append((key_vals[k1], key_vals[k2]))
            case_expr = " ".join(cases)
            in_clause = ", ".join(["(%s, %s)"] * len(tuple_vals))
            sql = (
                f"UPDATE `{table}` SET `OBS_ORDER` = CASE {case_expr} END "
                f"WHERE (`{k1}`, `{k2}`) IN ({in_clause})"
            )
            params: List[Any] = []
            for idx, key_vals in enumerate(ordered_keys, start=1):
                params.extend([key_vals[k1], key_vals[k2], idx])
            for tup in tuple_vals:
                params.extend(list(tup))
            self._execute(sql, tuple(params))
            return

        raise ValueError("Unsupported key column count for OBS_ORDER update.")


class AddRowDialog(QDialog):
    def __init__(
        self,
        parent: QWidget,
        columns: List[ColumnMeta],
        defaults: Dict[str, Any],
        exclude_columns: Optional[List[str]] = None,
        title: str = "Add Row",
    ) -> None:
        super().__init__(parent)
        self.setWindowTitle(title)
        self._columns = columns
        self._defaults = defaults
        self._exclude = {c.upper() for c in (exclude_columns or [])}
        self._inputs: Dict[str, QLineEdit] = {}

        layout = QVBoxLayout(self)
        scroll = QScrollArea(self)
        scroll.setWidgetResizable(True)
        inner = QWidget(scroll)
        form = QFormLayout(inner)

        for col in columns:
            if col.is_auto_increment:
                continue
            if col.name.upper() in self._exclude:
                continue
            inp = QLineEdit(inner)
            if col.name in defaults:
                inp.setText(str(defaults[col.name]))
            form.addRow(f"{col.name}:", inp)
            self._inputs[col.name] = inp

        scroll.setWidget(inner)
        layout.addWidget(scroll)

        buttons = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel, parent=self)
        buttons.accepted.connect(self.accept)
        buttons.rejected.connect(self.reject)
        layout.addWidget(buttons)

    def values(self) -> Dict[str, str]:
        out: Dict[str, str] = {}
        for name, widget in self._inputs.items():
            out[name] = widget.text().strip()
        return out


class DbTableWidget(QWidget):
    selection_changed = pyqtSignal(dict)

    def __init__(
        self,
        title: str,
        parent: QWidget,
        allow_reorder: bool = False,
        search_column: Optional[str] = None,
    ) -> None:
        super().__init__(parent)
        self._title = title
        self._db: Optional[DbClient] = None
        self._table_name: str = ""
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

        layout = QVBoxLayout(self)

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
        else:
            self.search_input = None

        top.addStretch()
        layout.addLayout(top)

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

        self.refresh_button.clicked.connect(self.refresh)
        self.add_button.clicked.connect(self._on_add_clicked)
        self.delete_button.clicked.connect(self._on_delete_clicked)
        if self.search_input:
            self.search_input.textChanged.connect(self.refresh)

    def set_database(self, db: DbClient, table_name: str) -> None:
        self._db = db
        self._table_name = table_name
        self._columns = db.fetch_columns(table_name)
        self._column_by_name = {c.name: c for c in self._columns}
        self._column_index = {c.name: idx for idx, c in enumerate(self._columns)}
        self._setup_table()

    def set_hidden_columns(self, columns: List[str]) -> None:
        self._hidden_columns = [c.upper() for c in columns]
        self._apply_hidden_columns()

    def set_order_by(self, column: str) -> None:
        self._order_by = column

    def set_fixed_filter(self, col: Optional[str], value: Optional[Any]) -> None:
        self._fixed_filter_col = col
        self._fixed_filter_val = value
        self.refresh()

    def has_column(self, name: str) -> bool:
        return name in self._column_by_name

    def current_row_values(self) -> Dict[str, Any]:
        row = self.table.currentRow()
        if row < 0:
            return {}
        values: Dict[str, Any] = {}
        for col_name, idx in self._column_index.items():
            item = self.table.item(row, idx)
            values[col_name] = item.text() if item else ""
        return values

    def _save_view_state(self) -> ViewState:
        """Save current scroll position and selection."""
        state = ViewState()
        state.v_scroll = self.table.verticalScrollBar().value()
        state.h_scroll = self.table.horizontalScrollBar().value()
        state.current_row = self.table.currentRow()

        # Try to save OBSERVATION_ID if available
        if state.current_row >= 0 and state.current_row < len(self._row_keys):
            key_vals = self._row_keys[state.current_row]
            state.current_obs_id = str(key_vals.get("OBSERVATION_ID", ""))

        return state

    def _restore_view_state(self, state: ViewState) -> None:
        """Restore scroll position and selection after data refresh."""
        # Try to restore selection by OBSERVATION_ID first
        if state.current_obs_id:
            for row_idx, key_vals in enumerate(self._row_keys):
                if str(key_vals.get("OBSERVATION_ID", "")) == state.current_obs_id:
                    self.table.setCurrentCell(row_idx, 0)
                    state.current_row = row_idx
                    break

        # Fallback to row index
        if state.current_row >= 0 and state.current_row < self.table.rowCount():
            self.table.setCurrentCell(state.current_row, 0)

        # Restore scroll position (must be done after table is populated)
        QTimer.singleShot(0, lambda: self._restore_scroll(state))

    def _restore_scroll(self, state: ViewState) -> None:
        """Restore scroll bars (called after table layout is complete)."""
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
        """Debounced save of column widths when user resizes."""
        # Restart timer to debounce rapid resize events
        self._column_width_save_timer.start(200)  # 200ms debounce

    def refresh(self) -> None:
        if not self._db or not self._db.is_open():
            return

        # Save view state before refresh
        view_state = self._save_view_state()

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

        self._loading = True
        self.table.setRowCount(0)
        self.table.setColumnCount(len(self._columns))
        self.table.setHorizontalHeaderLabels([c.name for c in self._columns])
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

        # Restore view state after refresh
        self._restore_view_state(view_state)

    def _setup_table(self) -> None:
        self.table.setColumnCount(len(self._columns))
        self.table.setHorizontalHeaderLabels([c.name for c in self._columns])
        self._apply_hidden_columns()

        # Connect column resize signal for auto-save
        header = self.table.horizontalHeader()
        header.sectionResized.connect(self._on_column_resized)

        # Restore column widths from settings
        self._restore_column_widths()

    def _apply_hidden_columns(self) -> None:
        for col_idx, col in enumerate(self._columns):
            hidden = col.name.upper() in self._hidden_columns
            self.table.setColumnHidden(col_idx, hidden)

    def _on_item_changed(self, item: QTableWidgetItem) -> None:
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
                self._show_error(f"{col_meta.name} cannot be NULL.")
                self._loading = True
                item.setText(str(old_text))
                self._loading = False
                return
            new_value = None
        else:
            new_value = new_text

        # Apply normalization if this is a target table
        if self._is_targets_table():
            row_values = self.current_row_values()
            row_values[col_meta.name] = new_value if new_value is not None else ""
            norm_result = normalize_target_row(row_values)

            if norm_result.changed_columns:
                # Show status message (less intrusive than dialog)
                parent_window = self.window()
                if hasattr(parent_window, 'status_bar'):
                    parent_window.status_bar.showMessage(norm_result.message, 5000)  # Show for 5 seconds
                # Update all changed columns
                try:
                    for changed_col in norm_result.changed_columns:
                        if changed_col in row_values:
                            update_val = row_values[changed_col]
                            key_vals = self._row_keys[row]
                            self._db.update_record(self._table_name, key_vals, {changed_col: update_val})
                            # Update display
                            if changed_col in self._column_index:
                                changed_col_idx = self._column_index[changed_col]
                                self._loading = True
                                self.table.item(row, changed_col_idx).setText(str(update_val))
                                self._loading = False
                except Exception as exc:
                    self._show_error(f"Normalization update failed: {exc}")
                return

        key_vals = self._row_keys[row]
        try:
            self._db.update_record(self._table_name, key_vals, {col_meta.name: new_value})
            item.setData(Qt.UserRole, "" if new_value is None else str(new_value))
        except Exception as exc:
            self._show_error(f"Update failed: {exc}")
            self._loading = True
            item.setText(str(old_text))
            self._loading = False

    def _on_selection_changed(self) -> None:
        values = self.current_row_values()
        if values:
            self.selection_changed.emit(values)

    def _on_add_clicked(self) -> None:
        if not self._db or not self._db.is_open():
            self._show_error("Not connected to database.")
            return

        defaults: Dict[str, Any] = {}
        exclude_cols: List[str] = []
        if self._is_targets_table():
            defaults.update(self._default_target_values())
            exclude_cols.append("OBSERVATION_ID")
            if self._fixed_filter_col and self._fixed_filter_col.upper() == "SET_ID":
                exclude_cols.append("SET_ID")
                defaults["SET_ID"] = self._fixed_filter_val
            exclude_cols.append("OBS_ORDER")

        dialog = AddRowDialog(self, self._columns, defaults, exclude_columns=exclude_cols, title=f"Add {self._title}")
        if dialog.exec() != QDialog.Accepted:
            return

        raw_values = dialog.values()
        values: Dict[str, Any] = {}
        exclude_upper = {c.upper() for c in exclude_cols}
        for col in self._columns:
            if col.is_auto_increment:
                continue
            if col.name.upper() in exclude_upper:
                if col.name in defaults:
                    values[col.name] = defaults[col.name]
                continue

            raw = raw_values.get(col.name, "").strip()
            if raw == "" or raw.upper() == "NULL":
                if col.nullable:
                    values[col.name] = None
                elif col.default is not None:
                    values[col.name] = col.default
                else:
                    self._show_error(f"{col.name} is required.")
                    return
            else:
                values[col.name] = raw

        if self._is_targets_table():
            values = self._apply_target_defaults(values)
            if values.get("NAME", "").strip() == "":
                self._show_error("NAME is required.")
                return
            if "OBS_ORDER" not in values and self._fixed_filter_val is not None:
                try:
                    values["OBS_ORDER"] = self._next_obs_order(self._fixed_filter_val)
                except Exception:
                    values["OBS_ORDER"] = 1

            # Apply normalization before insert
            norm_result = normalize_target_row(values)
            if norm_result.changed_columns:
                # Show status message
                parent_window = self.window()
                if hasattr(parent_window, 'status_bar'):
                    parent_window.status_bar.showMessage(
                        f"Added with normalization: {norm_result.message}", 5000
                    )
        else:
            if values.get("SET_NAME", "").strip() == "":
                self._show_error("SET_NAME is required.")
                return

        try:
            self._db.insert_record(self._table_name, values)
        except Exception as exc:
            self._show_error(f"Insert failed: {exc}")
            return
        self.refresh()

    def _on_delete_clicked(self) -> None:
        if not self._db or not self._db.is_open():
            self._show_error("Not connected to database.")
            return
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
            self._show_error(f"Delete failed: {exc}")
            return
        self.refresh()

    def _show_context_menu(self, pos) -> None:
        if self.search_input and self.search_input.text().strip():
            self._show_error("Reorder is disabled while search is active.")
            return
        row = self.table.currentRow()
        if row < 0:
            return
        menu = QMenu(self)
        move_up = menu.addAction("Move Up")
        move_down = menu.addAction("Move Down")
        move_top = menu.addAction("Move To Top")
        move_bottom = menu.addAction("Move To Bottom")
        action = menu.exec_(self.table.viewport().mapToGlobal(pos))
        if action == move_up:
            self._reorder_row(row, row - 1)
        elif action == move_down:
            self._reorder_row(row, row + 1)
        elif action == move_top:
            self._reorder_row(row, 0)
        elif action == move_bottom:
            self._reorder_row(row, len(self._row_keys) - 1)

    def _reorder_row(self, src: int, dst: int) -> None:
        if not self._allow_reorder or src < 0 or dst < 0:
            return
        if src >= len(self._row_keys) or dst >= len(self._row_keys):
            return
        if src == dst:
            return
        keys = list(self._row_keys)
        row_key = keys.pop(src)
        keys.insert(dst, row_key)
        key_cols = [c.name for c in self._columns if c.is_primary]
        if "OBS_ORDER" not in self._column_by_name:
            self._show_error("OBS_ORDER column not found.")
            return
        try:
            self._db.update_obs_order(self._table_name, key_cols, keys)
        except Exception as exc:
            self._show_error(f"Reorder failed: {exc}")
            return
        self.refresh()

    def _default_target_values(self) -> Dict[str, Any]:
        wrange_low, wrange_high = default_wrange_for_channel(DEFAULT_CHANNEL)
        return {
            "STATE": DEFAULT_TARGET_STATE,
            "TARGET_NUMBER": 1,
            "SEQUENCE_NUMBER": 1,
            "NAME": "",
            "RA": "0.0",
            "DECL": "0.0",
            "OFFSET_RA": "0.0",
            "OFFSET_DEC": "0.0",
            "SLITANGLE": DEFAULT_SLITANGLE,
            "SLITWIDTH": DEFAULT_SLITWIDTH,
            "EXPTIME": DEFAULT_EXPTIME,
            "NEXP": "1",
            "POINTMODE": DEFAULT_POINTMODE,
            "CCDMODE": DEFAULT_CCDMODE,
            "AIRMASS_MAX": str(DEFAULT_AIRMASS_MAX),
            "BINSPAT": str(DEFAULT_BIN),
            "BINSPECT": str(DEFAULT_BIN),
            "CHANNEL": DEFAULT_CHANNEL,
            "MAGNITUDE": str(DEFAULT_MAGNITUDE),
            "MAGSYSTEM": DEFAULT_MAGSYSTEM,
            "MAGFILTER": DEFAULT_MAGFILTER,
            "NOTBEFORE": DEFAULT_NOTBEFORE,
            "WRANGE_LOW": str(wrange_low),
            "WRANGE_HIGH": str(wrange_high),
            "OTMslitwidth": str(DEFAULT_OTM_SLITWIDTH),
        }

    def _apply_target_defaults(self, values: Dict[str, Any]) -> Dict[str, Any]:
        defaults = self._default_target_values()
        for key, val in defaults.items():
            if key not in self._column_by_name:
                continue
            if key in values:
                if values[key] is None or str(values[key]).strip() == "":
                    values[key] = val
            else:
                values[key] = val
        return values

    def _is_targets_table(self) -> bool:
        return self.has_column("OBSERVATION_ID") and self.has_column("SET_ID") and self.has_column("OBS_ORDER")

    def _next_obs_order(self, set_id: Any) -> int:
        if not self._db or not self._db.is_open():
            return 1
        where = "`SET_ID`=%s"
        rows = self._db.fetch_rows(self._table_name, where=where, params=(set_id,), order_by="`OBS_ORDER` DESC")
        if not rows:
            return 1
        top = rows[0].get("OBS_ORDER")
        try:
            return int(top) + 1
        except Exception:
            return 1

    def _show_error(self, msg: str) -> None:
        QMessageBox.critical(self, "Error", msg)


class MainWindow(QMainWindow):
    def __init__(self) -> None:
        super().__init__()
        self.setWindowTitle("NGPS Target Set Editor (PyQt5)")
        self.resize(1300, 800)

        self._db = DbClient()
        self._config_path = detect_default_config_path()
        self._config = load_config_file(self._config_path)
        self._ngps_root = infer_ngps_root_from_config(self._config_path)

        central = QWidget(self)
        layout = QVBoxLayout(central)

        top = QHBoxLayout()
        self.seq_start_btn = QPushButton("Seq Start", self)
        self.seq_abort_btn = QPushButton("Seq Abort", self)
        self.activate_set_btn = QPushButton("Activate target set", self)
        self.conn_status = QLabel("Not connected", self)
        top.addWidget(self.seq_start_btn)
        top.addWidget(self.seq_abort_btn)
        top.addWidget(self.activate_set_btn)
        top.addStretch()
        top.addWidget(self.conn_status)
        layout.addLayout(top)

        self.tabs = QTabWidget(self)
        self.sets_table = DbTableWidget("Target Sets", self)
        self.targets_table = DbTableWidget("Targets (Set View)", self, allow_reorder=True, search_column="NAME")
        self.tabs.addTab(self.sets_table, "Target Sets")
        self.tabs.addTab(self.targets_table, "Targets (Set View)")
        layout.addWidget(self.tabs, 1)

        self.setCentralWidget(central)

        # Add status bar
        self.status_bar = self.statusBar()
        self.status_bar.showMessage("Ready")

        self.seq_start_btn.clicked.connect(self.seq_start)
        self.seq_abort_btn.clicked.connect(self.seq_abort)
        self.activate_set_btn.clicked.connect(self.activate_set)
        self.sets_table.selection_changed.connect(self._on_set_selected)

        self._init_db()
        self._restore_window_state()

    def _init_db(self) -> None:
        if not self._config.is_complete():
            QMessageBox.critical(self, "Config", "sequencerd.cfg is missing DB settings.")
            return
        try:
            self._db.connect(self._config)
        except Exception as exc:
            QMessageBox.critical(self, "DB Connection", f"Failed to connect: {exc}")
            return

        self.conn_status.setText(
            f"Connected: {self._config.user}@{self._config.host}:{self._config.port}/{self._config.schema}"
        )

        self.sets_table.set_database(self._db, self._config.table_sets)
        self.targets_table.set_database(self._db, self._config.table_targets)

        self.sets_table.set_order_by("SET_ID")
        self.targets_table.set_order_by("OBS_ORDER")
        self.targets_table.set_hidden_columns(
            [
                "OBSERVATION_ID",
                "SET_ID",
                "OBS_ORDER",
                "TARGET_NUMBER",
                "SEQUENCE_NUMBER",
                "SLITOFFSET",
                "OBSMODE",
            ]
        )
        self.sets_table.refresh()
        self.targets_table.refresh()

    def _restore_window_state(self) -> None:
        """Restore window geometry and tab index from QSettings."""
        settings = QSettings(SETTINGS_ORG, SETTINGS_APP)

        # Restore window geometry
        geometry = settings.value("mainWindow/geometry")
        if geometry:
            self.restoreGeometry(geometry)

        # Restore window state (maximized, etc.)
        window_state = settings.value("mainWindow/windowState")
        if window_state:
            self.restoreState(window_state)

        # Restore last selected tab
        tab_index = settings.value("mainWindow/tabIndex", 0, type=int)
        if 0 <= tab_index < self.tabs.count():
            self.tabs.setCurrentIndex(tab_index)

    def _save_window_state(self) -> None:
        """Save window geometry and tab index to QSettings."""
        settings = QSettings(SETTINGS_ORG, SETTINGS_APP)
        settings.setValue("mainWindow/geometry", self.saveGeometry())
        settings.setValue("mainWindow/windowState", self.saveState())
        settings.setValue("mainWindow/tabIndex", self.tabs.currentIndex())

    def closeEvent(self, event) -> None:
        """Override close event to save state before exiting."""
        self._save_window_state()
        super().closeEvent(event)

    def _seq_command(self) -> Tuple[str, Dict[str, str], str]:
        cmd = "seq"
        if self._ngps_root:
            candidate = os.path.join(self._ngps_root, "run", "seq")
            if os.path.exists(candidate):
                cmd = candidate
        env = os.environ.copy()
        if self._config_path:
            env["SEQUENCERD_CONFIG"] = self._config_path
        if self._ngps_root:
            env["NGPS_ROOT"] = self._ngps_root
        workdir = self._ngps_root or None
        return cmd, env, workdir

    def _run_seq(self, args: List[str]) -> None:
        cmd, env, workdir = self._seq_command()
        try:
            subprocess.run([cmd] + args, env=env, cwd=workdir, check=False, capture_output=True, text=True)
        except Exception as exc:
            QMessageBox.critical(self, "Sequencer", f"Failed to run seq: {exc}")

    def seq_start(self) -> None:
        self._run_seq(["start"])
        self.status_bar.showMessage("Sequencer start command sent", 3000)

    def seq_abort(self) -> None:
        self._run_seq(["abort"])
        self.status_bar.showMessage("Sequencer abort command sent", 3000)

    def activate_set(self) -> None:
        values = self.sets_table.current_row_values()
        if not values:
            QMessageBox.information(self, "Target Sets", "Select a target set first.")
            return
        set_name = values.get("SET_NAME", "").strip()
        if set_name:
            self._run_seq(["targetset", set_name])
            self.status_bar.showMessage(f"Activated target set: {set_name}", 3000)
            return
        set_id = values.get("SET_ID", "").strip()
        if not set_id:
            QMessageBox.warning(self, "Target Sets", "SET_NAME/SET_ID not found.")
            return
        self._run_seq(["targetset", set_id])
        self.status_bar.showMessage(f"Activated target set ID: {set_id}", 3000)

    def _on_set_selected(self, values: Dict[str, Any]) -> None:
        set_id = values.get("SET_ID")
        if set_id is None or set_id == "":
            return
        self.targets_table.set_fixed_filter("SET_ID", set_id)


def main() -> int:
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    return app.exec_()


if __name__ == "__main__":
    sys.exit(main())
