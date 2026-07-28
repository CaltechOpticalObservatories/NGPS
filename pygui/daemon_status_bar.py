from PyQt5.QtCore import Qt, pyqtSignal, QPoint, QSize
from PyQt5.QtWidgets import (
    QWidget, QHBoxLayout, QPushButton, QMenu, QMessageBox,
    QSizePolicy, QFrame
)
from PyQt5.QtGui import QColor

class DaemonState:
    OK = "ok"         # green
    WARN = "warn"     # yellow
    ERROR = "error"   # red
    UNKNOWN = "unknown"  # grey

STATE_COLORS = {
    DaemonState.OK: QColor(46, 204, 113),
    DaemonState.WARN: QColor(241, 196, 15),
    DaemonState.ERROR: QColor(231, 76, 60),
    DaemonState.UNKNOWN: QColor(127, 140, 141),
}

def _css_rgba(qc: QColor, alpha=230):
    return f"rgba({qc.red()},{qc.green()},{qc.blue()},{alpha})"

class DaemonChip(QPushButton):
    """Rounded 'bubble' showing daemon health.
       Left-click: details. Right-click: commands."""
    detailsRequested = pyqtSignal(str)       # daemon_name
    commandRequested = pyqtSignal(str, str)  # daemon_name, command

    def __init__(self, name: str, commands=None, parent=None):
        super().__init__(name, parent)
        self.name = name
        self.state = DaemonState.UNKNOWN
        self.issue = "No status yet."
        self.commands = commands or ["Ping", "Restart", "Open Logs"]

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
        # QMessageBox.information(self, f"{self.name} status", f"State: {self.state}\n\n{self.issue}")
        # self.detailsRequested.emit(self.name)
        pass

    def _on_context_menu(self, pos: QPoint):
        menu = QMenu(self)
        for cmd in self.commands:
            menu.addAction(cmd)
        chosen = menu.exec_(self.mapToGlobal(pos))
        if chosen:
            self.commandRequested.emit(self.name, chosen.text())

    def set_state(self, state: str, issue: str = None):
        self.state = state
        if issue is not None:
            self.issue = issue
        self._apply_style()
        self.setToolTip(f"{self.name}: {self.state}\n{self.issue}")

    def _apply_style(self):
        base = STATE_COLORS.get(self.state, STATE_COLORS[DaemonState.UNKNOWN])
        fill = _css_rgba(base, 230)
        hover = _css_rgba(base, 255)
        self.setStyleSheet(f"""
            QPushButton {{
                background: {fill};
                color: white;
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
    """Bottom bar of clickable daemon chips with distributed spacing."""
    commandRequested = pyqtSignal(str, str)  # daemon_name, command
    detailsRequested = pyqtSignal(str)

    def __init__(self, daemons, per_daemon_commands=None, parent=None,
                 distribution: str = "around", base_spacing_px: int = 10):
        """
        distribution: "around" (space-around) or "between" (space-between)
        base_spacing_px: minimal pixel spacing around items (in addition to stretch)
        """
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

        self._chips = {}
        self._layout = QHBoxLayout(self)
        self._layout.setContentsMargins(12, 6, 12, 6)
        self._layout.setSpacing(base_spacing_px)

        # Build with stretch spacers so chips spread to fill width
        # (mimics CSS justify-content: space-around / space-between)
        n = len(daemons)
        if n == 0:
            return

        def add_stretch():
            self._layout.addStretch(1)

        if distribution not in ("around", "between"):
            distribution = "around"

        if distribution == "around":
            add_stretch()  # leading edge
            for name in daemons:
                cmds = (per_daemon_commands or {}).get(name)
                chip = DaemonChip(name, commands=cmds)
                chip.commandRequested.connect(self.commandRequested)
                chip.detailsRequested.connect(self.detailsRequested)
                self._layout.addWidget(chip)
                self._chips[name] = chip
                add_stretch()  # between + trailing edge
        else:  # "between": no edge stretch, only between chips; pushes ends to edges
            for i, name in enumerate(daemons):
                cmds = (per_daemon_commands or {}).get(name)
                chip = DaemonChip(name, commands=cmds)
                chip.commandRequested.connect(self.commandRequested)
                chip.detailsRequested.connect(self.detailsRequested)
                self._layout.addWidget(chip)
                self._chips[name] = chip
                if i != n - 1:
                    add_stretch()

    def set_daemon_state(self, name: str, state: str, issue: str = None):
        chip = self._chips.get(name)
        if chip:
            chip.set_state(state, issue)

    def bulk_update(self, updates: dict):
        for name, (state, issue) in updates.items():
            self.set_daemon_state(name, state, issue)
