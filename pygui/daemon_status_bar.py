from PyQt5.QtCore import Qt, pyqtSignal, QPoint, QSize
from PyQt5.QtWidgets import (
    QWidget, QHBoxLayout, QPushButton, QMenu,
    QSizePolicy, QFrame
)
from PyQt5.QtGui import QColor


# Wait-state JSON keys from sequencer telemetry.
WAIT_ACAM = "ACAM"
WAIT_CALIB = "CALIB"
WAIT_CAMERA = "CAMERA"
WAIT_FLEXURE = "FLEXURE"
WAIT_FOCUS = "FOCUS"
WAIT_POWER = "POWER"
WAIT_SLICECAM = "SLICECAM"
WAIT_SLIT = "SLIT"
WAIT_TCS = "TCS"


# label  -> what the GUI displays
# daemon -> key used by seq_daemonstate
# wait   -> key used by seq_waitstate; None means no busy/blink state
DAEMON_SUBSYSTEMS = [
    {"label": "ACAM",      "daemon": "acamd",      "wait": WAIT_ACAM},
    {"label": "CALIB",     "daemon": "calibd",     "wait": WAIT_CALIB},
    {"label": "CAMERA",    "daemon": "camerad",    "wait": WAIT_CAMERA},
    {"label": "FLEXURE",   "daemon": "flexured",   "wait": WAIT_FLEXURE},
    {"label": "FOCUS",     "daemon": "focusd",     "wait": WAIT_FOCUS},
    {"label": "POWER",     "daemon": "powerd",     "wait": WAIT_POWER},
    {"label": "SEQUENCER", "daemon": "sequencerd", "wait": None},
    {"label": "SLICECAM",  "daemon": "slicecamd",  "wait": WAIT_SLICECAM},
    {"label": "SLIT",      "daemon": "slitd",      "wait": WAIT_SLIT},
    {"label": "TCS",       "daemon": "tcsd",       "wait": WAIT_TCS},
]


class DaemonState:
    OK = "ok"              # ready / initialized
    WARN = "warn"          # warning state, manual use
    ERROR = "error"        # error state, manual use
    UNKNOWN = "unknown"    # not ready / no telemetry yet
    BUSY = "busy"          # sequencer is waiting on this subsystem; blink


STATE_COLORS = {
    DaemonState.OK: QColor(46, 204, 113),
    DaemonState.WARN: QColor(241, 196, 15),
    DaemonState.ERROR: QColor(231, 76, 60),
    DaemonState.UNKNOWN: QColor(80, 84, 88),
}

# seqgui-like busy colors
BUSY_ON = QColor(51, 204, 51)
BUSY_OFF = QColor(26, 74, 26)


def _css_rgba(qc: QColor, alpha=230):
    return f"rgba({qc.red()},{qc.green()},{qc.blue()},{alpha})"


class DaemonChip(QPushButton):
    """Rounded daemon/subsystem chip.

    Left-click: details signal.
    Right-click: command menu.

    The displayed label can differ from the daemon key.  For example,
    label="ACAM" while daemon_key="acamd".
    """

    detailsRequested = pyqtSignal(str)       # daemon_key
    commandRequested = pyqtSignal(str, str)  # daemon_key, command

    def __init__(self, label: str, daemon_key: str = None, commands=None, parent=None):
        super().__init__(label, parent)
        self.label = label
        self.daemon_key = daemon_key or label
        self.state = DaemonState.UNKNOWN
        self.issue = "No status yet."
        self.commands = commands or ["Ping", "Restart", "Open Logs"]
        self._blink_phase = False

        self.setCursor(Qt.PointingHandCursor)
        self.setFlat(True)
        self.setFocusPolicy(Qt.NoFocus)
        self.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Fixed)
        self.setMinimumHeight(26)
        self._apply_style()

        self.clicked.connect(self._on_left_click)
        self.setContextMenuPolicy(Qt.CustomContextMenu)
        self.customContextMenuRequested.connect(self._on_context_menu)

    def sizeHint(self) -> QSize:
        base = super().sizeHint()
        return QSize(max(base.width(), 76), 26)

    def _on_left_click(self):
        self.detailsRequested.emit(self.daemon_key)

    def _on_context_menu(self, pos: QPoint):
        menu = QMenu(self)
        for cmd in self.commands:
            menu.addAction(cmd)
        chosen = menu.exec_(self.mapToGlobal(pos))
        if chosen:
            self.commandRequested.emit(self.daemon_key, chosen.text())

    def set_state(self, state: str, issue: str = None):
        self.state = state
        if state != DaemonState.BUSY:
            self._blink_phase = False
        if issue is not None:
            self.issue = issue
        self._apply_style()
        self.setToolTip(
            f"{self.label} ({self.daemon_key})\n"
            f"State: {self.state}\n"
            f"{self.issue}"
        )

    def set_busy_phase(self, phase: bool, issue: str = None):
        self.state = DaemonState.BUSY
        self._blink_phase = bool(phase)
        if issue is not None:
            self.issue = issue
        self._apply_style()
        self.setToolTip(
            f"{self.label} ({self.daemon_key})\n"
            f"State: busy\n"
            f"{self.issue}"
        )

    def _apply_style(self):
        if self.state == DaemonState.BUSY:
            base = BUSY_ON if self._blink_phase else BUSY_OFF
            fill = _css_rgba(base, 255)
            hover = _css_rgba(BUSY_ON, 255)
            text_color = "#000000" if self._blink_phase else "#cccccc"
        else:
            base = STATE_COLORS.get(self.state, STATE_COLORS[DaemonState.UNKNOWN])
            fill = _css_rgba(base, 230)
            hover = _css_rgba(base, 255)
            text_color = "white"

        self.setStyleSheet(f"""
            QPushButton {{
                background: {fill};
                color: {text_color};
                border: 0px;
                border-radius: 13px;
                padding: 4px 12px;
                font-weight: 600;
            }}
            QPushButton:hover {{
                background: {hover};
            }}
        """)


class DaemonStatusBar(QFrame):
    """Bottom bar of daemon chips with seqgui-style behavior.

    seq_daemonstate drives ready/not-ready.
    seq_waitstate drives busy/blink.
    Process polling can still call bulk_update() as a fallback before telemetry
    arrives.
    """

    commandRequested = pyqtSignal(str, str)  # daemon_key, command
    detailsRequested = pyqtSignal(str)       # daemon_key

    def __init__(self, daemons=None, per_daemon_commands=None, parent=None,
                 distribution: str = "around", base_spacing_px: int = 10):
        super().__init__(parent)
        self.setObjectName("DaemonStatusBar")
        self.setFrameShape(QFrame.StyledPanel)
        self.setFrameShadow(QFrame.Plain)
        self.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        self.setStyleSheet("""
        #DaemonStatusBar {
            border-top: 1px solid #444;
            background: #202225;
        }""")

        self._chips_by_label = {}
        self._chips_by_daemon = {}
        self._subsystems = [self._normalize_subsystem(d) for d in (daemons or DAEMON_SUBSYSTEMS)]
        self.daemon_ready = {}
        self.waiting = {}
        self._blink_phase = False

        self._layout = QHBoxLayout(self)
        self._layout.setContentsMargins(12, 6, 12, 6)
        self._layout.setSpacing(base_spacing_px)

        n = len(self._subsystems)
        if n == 0:
            return

        def add_stretch():
            self._layout.addStretch(1)

        if distribution not in ("around", "between"):
            distribution = "around"

        if distribution == "around":
            add_stretch()

        for i, subsystem in enumerate(self._subsystems):
            label = subsystem["label"]
            daemon_key = subsystem["daemon"]

            cmds = None
            if per_daemon_commands:
                cmds = (
                    per_daemon_commands.get(daemon_key)
                    or per_daemon_commands.get(label)
                    or per_daemon_commands.get(label.lower())
                )

            chip = DaemonChip(label, daemon_key=daemon_key, commands=cmds)
            chip.commandRequested.connect(self.commandRequested)
            chip.detailsRequested.connect(self.detailsRequested)
            self._layout.addWidget(chip)
            self._chips_by_label[label] = chip
            self._chips_by_daemon[daemon_key] = chip

            if distribution == "around":
                add_stretch()
            elif i != n - 1:
                add_stretch()

    @staticmethod
    def _normalize_subsystem(item):
        if isinstance(item, dict):
            return {
                "label": str(item.get("label") or item.get("name") or item.get("daemon")),
                "daemon": str(item.get("daemon") or item.get("key") or item.get("label")),
                "wait": item.get("wait"),
            }

        if isinstance(item, (tuple, list)) and len(item) >= 2:
            return {
                "label": str(item[0]),
                "daemon": str(item[1]),
                "wait": item[2] if len(item) > 2 else None,
            }

        text = str(item)
        return {"label": text, "daemon": text, "wait": None}

    @staticmethod
    def _bool_dict(state: dict):
        if not isinstance(state, dict):
            return {}
        return {
            str(k): bool(v)
            for k, v in state.items()
            if isinstance(v, bool)
        }

    def set_daemon_ready_state(self, state: dict):
        """Apply seq_daemonstate: {"acamd": true, ...}."""
        self.daemon_ready = self._bool_dict(state)
        self._refresh_static()

    def set_wait_state(self, state: dict):
        """Apply seq_waitstate: {"ACAM": true, ...}."""
        self.waiting = self._bool_dict(state)
        self._refresh_static()

    def set_sequencerd_online(self):
        """Mark sequencerd ready when any sequencer status arrives."""
        self.daemon_ready["sequencerd"] = True
        self._refresh_static()

    def _refresh_static(self):
        """Apply not-ready / ready styling; blinking is handled by blink_tick."""
        for subsystem in self._subsystems:
            label = subsystem["label"]
            daemon_key = subsystem["daemon"]
            wait_key = subsystem.get("wait")
            chip = self._chips_by_label.get(label)
            if chip is None:
                continue

            if wait_key and self.waiting.get(wait_key, False):
                # Leave active wait states to blink_tick().
                continue

            if self.daemon_ready.get(daemon_key, False):
                chip.set_state(DaemonState.OK, f"{daemon_key} is initialized/ready.")
            else:
                chip.set_state(DaemonState.UNKNOWN, f"{daemon_key} is not ready or has not reported yet.")

    def blink_tick(self, phase: bool):
        """Drive busy blinking for wait-state-active subsystems."""
        self._blink_phase = bool(phase)
        for subsystem in self._subsystems:
            label = subsystem["label"]
            daemon_key = subsystem["daemon"]
            wait_key = subsystem.get("wait")
            chip = self._chips_by_label.get(label)
            if chip is None:
                continue
            if wait_key and self.waiting.get(wait_key, False):
                chip.set_busy_phase(
                    self._blink_phase,
                    f"Sequencer is waiting on {wait_key} ({daemon_key})."
                )

    def set_daemon_state(self, name: str, state: str, issue: str = None):
        """Manual/fallback update by daemon key or display label."""
        chip = self._chips_by_daemon.get(name) or self._chips_by_label.get(name)
        if not chip:
            return

        # Keep daemon_ready coherent for fallback updates.
        if state == DaemonState.OK:
            self.daemon_ready[chip.daemon_key] = True
        elif state in (DaemonState.UNKNOWN, DaemonState.ERROR):
            self.daemon_ready[chip.daemon_key] = False

        chip.set_state(state, issue)

    def bulk_update(self, updates: dict):
        for name, value in updates.items():
            if isinstance(value, tuple):
                state, issue = value[0], value[1] if len(value) > 1 else None
            else:
                state, issue = value, None
            self.set_daemon_state(name, state, issue)
