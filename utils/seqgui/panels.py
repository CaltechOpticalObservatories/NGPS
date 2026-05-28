from PyQt5.QtCore import Qt
from PyQt5.QtGui import QFont
from PyQt5.QtWidgets import (
    QFrame, QGridLayout, QHBoxLayout, QLabel, QSizePolicy, QVBoxLayout, QWidget
)

# Wait-state JSON keys (mirrors Sequencer::wait_state_names in sequence.h)
WAIT_ACAM     = "ACAM"
WAIT_CALIB    = "CALIB"
WAIT_CAMERA   = "CAMERA"
WAIT_FLEXURE  = "FLEXURE"
WAIT_FOCUS    = "FOCUS"
WAIT_POWER    = "POWER"
WAIT_SLICECAM = "SLICECAM"
WAIT_SLIT     = "SLIT"
WAIT_TCS      = "TCS"
WAIT_EXPOSE   = "EXPOSE"
WAIT_READOUT  = "READOUT"
WAIT_TCSOP    = "TCSOP"
WAIT_USER     = "USER"

# Subsystem display order: (display-name, daemon-key, wait-key).
# "sequencerd" has no wait-key (None) -- it shows online status only.
SUBSYSTEMS = [
    ("sequencerd",  "sequencerd", None),
    ("acam",        "acamd",      WAIT_ACAM),
    ("calib",       "calibd",     WAIT_CALIB),
    ("camera",      "camerad",    WAIT_CAMERA),
    ("flexure",     "flexured",   WAIT_FLEXURE),
    ("focus",       "focusd",     WAIT_FOCUS),
    ("power",       "powerd",     WAIT_POWER),
    ("slicecam",    "slicecamd",  WAIT_SLICECAM),
    ("slit",        "slitd",      WAIT_SLIT),
    ("tcs",         "tcsd",       WAIT_TCS),
]

# JSON key names -- keep in sync with common/message_keys.h
KEY_ACQUIRE_MODE        = "acquire_mode"
KEY_IS_ACQUIRED         = "is_acquired"
KEY_SEEING              = "seeing"
KEY_FINEACQUIRE_LOCKED  = "fineacquire_locked"
KEY_FINEACQUIRE_RUNNING = "fineacquire_running"
KEY_INREADOUT           = "inreadout"
KEY_EXPOSING            = "exposing"

# Color palette (dark theme)
COLOR_INACTIVE  = "#3a3a3a"     # very dim -- clearly off
COLOR_GRAY      = "#888888"     # secondary text / labels
COLOR_GREEN     = "#33cc33"     # active / ready / good
COLOR_GREEN_DIM = "#1a4a1a"     # blinking off-phase for green
COLOR_YELLOW    = "#cccc00"     # READY state
COLOR_RED       = "#cc3333"     # FAILED
COLOR_BLUE      = "#4488ff"     # STARTING / STOPPING
COLOR_CYAN      = "#00aaaa"     # PAUSED / guiding
COLOR_ORANGE    = "#ff9933"     # ABORTING
COLOR_BG_DARK   = "#1e1e1e"     # panel background
COLOR_BG_MID    = "#2a2a2a"     # widget background


def color_for_state(state):
    """ Return a CSS hex color for a seqstate label string. """
    if "FAILED"   in state: return COLOR_RED
    if "ABORTING" in state: return COLOR_ORANGE
    if "RUNNING"  in state: return COLOR_GREEN
    if "PAUSED"   in state: return COLOR_CYAN
    if "NOTREADY" in state: return COLOR_GRAY
    if "READY"    in state: return COLOR_YELLOW
    if "STARTING" in state or "STOPPING" in state: return COLOR_BLUE
    return COLOR_GRAY


def section_label(text):
    """ Return a small bold section-header label. """
    lbl = QLabel(text)
    lbl.setStyleSheet("font-weight: bold; color: #aaaaaa; font-size: 10pt;")
    return lbl


def hline():
    """ Return a thin horizontal separator. """
    line = QFrame()
    line.setFrameShape(QFrame.HLine)
    line.setFrameShadow(QFrame.Sunken)
    line.setStyleSheet("color: #333333;")
    line.setMaximumHeight(2)
    return line


def vline():
    """ Return a thin vertical separator for use in horizontal layouts. """
    line = QFrame()
    line.setFrameShape(QFrame.VLine)
    line.setFrameShadow(QFrame.Sunken)
    line.setStyleSheet("color: #333333;")
    line.setMaximumWidth(2)
    return line


class Badge(QLabel):
    """ Tri-state colored pill: not-ready (dim), ready (green), busy (blink). """

    def __init__(self, text, parent=None):
        super().__init__(text, parent)
        self.init_ui()

    def init_ui(self):
        self.setAlignment(Qt.AlignCenter)
        font = self.font()
        font.setBold(True)
        font.setPointSize(10)
        self.setFont(font)
        self.setMinimumWidth(82)
        self.setFixedHeight(28)
        self._set(COLOR_INACTIVE, COLOR_BG_MID)

    def _set(self, fg, bg):
        self.setStyleSheet(
            f"color: {fg}; background-color: {bg}; "
            "border: 1px solid #555; border-radius: 5px; padding: 0px 6px;"
        )

    def set_not_ready(self):
        """ Apply not-ready styling (dim). """
        self._set(COLOR_INACTIVE, COLOR_BG_MID)

    def set_ready(self):
        """ Apply ready styling (steady green). """
        self._set("#ffffff", COLOR_GREEN_DIM)

    def set_blink(self, phase):
        """ Apply busy styling for the current blink phase. """
        if phase:
            self._set("#000000", COLOR_GREEN)
        else:
            self._set("#cccccc", COLOR_GREEN_DIM)


class SubsystemPanel(QWidget):
    """ Tri-state grid of daemon badges plus a sequencerd online indicator. """

    def __init__(self, parent=None):
        super().__init__(parent)
        self.badges = {}
        self.daemon_ready = {}
        self.waiting = {}
        self.init_ui()

    def init_ui(self):
        layout = QVBoxLayout(self)
        layout.setContentsMargins(8, 6, 8, 6)
        layout.setSpacing(4)
        layout.addWidget(section_label("SUBSYSTEMS"))

        grid = QGridLayout()
        grid.setHorizontalSpacing(5)
        grid.setVerticalSpacing(5)

        for i, (name, _daemon, _wait) in enumerate(SUBSYSTEMS):
            badge = Badge(name)
            self.badges[name] = badge
            grid.addWidget(badge, i // 5, i % 5)

        layout.addLayout(grid)

        legend = QLabel("dim = not ready   green = ready   blink = busy")
        legend.setStyleSheet("color: #555555; font-size: 8pt;")
        layout.addWidget(legend)

    def set_sequencerd_online(self):
        """ Mark sequencerd badge online (called on any received seq message). """
        self.badges["sequencerd"].set_ready()

    def set_daemonstate(self, state):
        """ Update which daemons are initialized and refresh styling. """
        self.daemon_ready = state
        self._refresh_static()

    def set_waitstate(self, state):
        """ Update which subsystems are being waited on and refresh styling. """
        self.waiting = state
        self._refresh_static()

    def _refresh_static(self):
        """ Apply not-ready / ready styling; blink handled in blink_tick. """
        for name, daemon_key, wait_key in SUBSYSTEMS:
            if name == "sequencerd":
                continue    # driven by set_sequencerd_online only
            badge = self.badges[name]
            if wait_key and self.waiting.get(wait_key, False):
                continue    # leave for blink_tick
            if self.daemon_ready.get(daemon_key, False):
                badge.set_ready()
            else:
                badge.set_not_ready()

    def blink_tick(self, phase):
        """ Drive the busy-blink phase for any waiting subsystems. """
        for name, _daemon, wait_key in SUBSYSTEMS:
            if wait_key and self.waiting.get(wait_key, False):
                self.badges[name].set_blink(phase)


class StatePanel(QWidget):
    """ Left panel: STATE badge and (when active) TCSOP / USER alert. """

    def __init__(self, parent=None):
        super().__init__(parent)
        self.tcsop_active = False
        self.user_active = False
        self.init_ui()

    def init_ui(self):
        self.setMinimumWidth(160)
        self.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Expanding)

        layout = QVBoxLayout(self)
        layout.setContentsMargins(8, 6, 8, 6)
        layout.setSpacing(8)
        layout.addWidget(section_label("STATE"))

        self.state_label = QLabel("")
        self.state_label.setAlignment(Qt.AlignCenter)
        font = QFont()
        font.setPointSize(20)
        font.setBold(True)
        self.state_label.setFont(font)
        self.state_label.setFixedHeight(60)
        self.state_label.setStyleSheet(
            f"color: {COLOR_GRAY}; background-color: {COLOR_BG_MID}; "
            "border: 2px solid #444; border-radius: 8px;"
        )
        layout.addWidget(self.state_label)

        # TCSOP alert: hidden when inactive, appears with blink when active
        alert_font = QFont()
        alert_font.setPointSize(13)
        alert_font.setBold(True)

        self.tcsop_alert = QLabel("")
        self.tcsop_alert.setAlignment(Qt.AlignCenter)
        self.tcsop_alert.setFont(alert_font)
        self.tcsop_alert.setWordWrap(True)
        self.tcsop_alert.setVisible(False)
        layout.addWidget(self.tcsop_alert)

        # USER-continue alert: hidden when inactive, blinks when sequencer is
        # waiting for the user to issue 'usercontinue'
        self.user_alert = QLabel("")
        self.user_alert.setAlignment(Qt.AlignCenter)
        self.user_alert.setFont(alert_font)
        self.user_alert.setWordWrap(True)
        self.user_alert.setVisible(False)
        layout.addWidget(self.user_alert)

        layout.addStretch(1)

    def set_state(self, state):
        """ Update the STATE badge text and color from a seqstate string. """
        display = state.strip()
        color = color_for_state(display)
        self.state_label.setText(display)
        self.state_label.setStyleSheet(
            f"color: {color}; background-color: {COLOR_BG_MID}; "
            "border: 2px solid #444; border-radius: 8px;"
        )

    def set_tcsop_active(self, active):
        """ Mark the TCSOP-wait alert active or inactive. """
        self.tcsop_active = active
        if not active:
            self.tcsop_alert.setVisible(False)

    def set_user_active(self, active):
        """ Mark the USER-continue alert active or inactive. """
        self.user_active = active
        if not active:
            self.user_alert.setVisible(False)

    def blink_tick(self, phase):
        """ Drive blink phase for any active alert labels. """
        if self.tcsop_active:
            self.tcsop_alert.setVisible(True)
            self.tcsop_alert.setText("WAITING FOR\nTCS OPERATOR")
            if phase:
                self.tcsop_alert.setStyleSheet(
                    f"color: yellow; background-color: {COLOR_RED}; "
                    "border: 3px solid yellow; border-radius: 8px; padding: 4px;"
                )
            else:
                self.tcsop_alert.setStyleSheet(
                    f"color: {COLOR_RED}; background-color: yellow; "
                    "border: 3px solid red; border-radius: 8px; padding: 4px;"
                )
        if self.user_active:
            self.user_alert.setVisible(True)
            self.user_alert.setText("WAITING FOR\nUSER CONTINUE")
            if phase:
                self.user_alert.setStyleSheet(
                    "color: black; background-color: #33ff33; "
                    "border: 3px solid white; border-radius: 8px; padding: 4px;"
                )
            else:
                self.user_alert.setStyleSheet(
                    "color: #33ff33; background-color: #1a4a1a; "
                    "border: 3px solid #33ff33; border-radius: 8px; padding: 4px;"
                )


class CameraPanel(QWidget):
    """ Exposing / Reading out indicators. """

    def __init__(self, parent=None):
        super().__init__(parent)
        self.exposing_active = False
        self.readout_active = False
        self.init_ui()

    def init_ui(self):
        layout = QVBoxLayout(self)
        layout.setContentsMargins(8, 6, 8, 6)
        layout.setSpacing(4)
        layout.addWidget(section_label("CAMERA"))

        row = QHBoxLayout()
        row.setSpacing(8)
        self.lbl_exposing = self._make_indicator("Exposing")
        self.lbl_readout  = self._make_indicator("Reading out")
        row.addWidget(self.lbl_exposing)
        row.addWidget(self.lbl_readout)
        row.addStretch(1)
        layout.addLayout(row)

    @staticmethod
    def _make_indicator(text):
        lbl = QLabel(text)
        lbl.setAlignment(Qt.AlignCenter)
        font = lbl.font()
        font.setBold(True)
        font.setPointSize(10)
        lbl.setFont(font)
        lbl.setFixedHeight(28)
        lbl.setMinimumWidth(110)
        lbl.setStyleSheet(
            f"color: {COLOR_INACTIVE}; background-color: {COLOR_BG_MID}; "
            "border: 1px solid #444; border-radius: 5px; padding: 0 8px;"
        )
        return lbl

    @staticmethod
    def _set_idle(lbl):
        lbl.setStyleSheet(
            f"color: {COLOR_INACTIVE}; background-color: {COLOR_BG_MID}; "
            "border: 1px solid #444; border-radius: 5px; padding: 0 8px;"
        )

    @staticmethod
    def _set_blink(lbl, phase):
        if phase:
            lbl.setStyleSheet(
                f"color: #000000; background-color: {COLOR_GREEN}; "
                "border: 1px solid #22aa22; border-radius: 5px; padding: 0 8px;"
            )
        else:
            lbl.setStyleSheet(
                f"color: {COLOR_GREEN}; background-color: {COLOR_GREEN_DIM}; "
                "border: 1px solid #22aa22; border-radius: 5px; padding: 0 8px;"
            )

    def set_camerad(self, data):
        """ Update exposing / readout active flags from a camerad payload. """
        if KEY_EXPOSING in data:
            self.exposing_active = bool(data[KEY_EXPOSING])
            if not self.exposing_active:
                self._set_idle(self.lbl_exposing)
        if KEY_INREADOUT in data:
            self.readout_active = bool(data[KEY_INREADOUT])
            if not self.readout_active:
                self._set_idle(self.lbl_readout)

    def set_camerad_online(self, online):
        """ Clear indicators when camerad goes offline (defense-in-depth).
            Without this, the last-published exposing/readout state would
            stick on the UI after camerad stopped publishing. """
        if not online:
            self.exposing_active = False
            self.readout_active = False
            self._set_idle(self.lbl_exposing)
            self._set_idle(self.lbl_readout)

    def blink_tick(self, phase):
        """ Drive blink phase for any active camera indicators. """
        if self.exposing_active:
            self._set_blink(self.lbl_exposing, phase)
        if self.readout_active:
            self._set_blink(self.lbl_readout, phase)


class AcquisitionPanel(QWidget):
    """ ACAM + SLICECAM acquisition status display. """

    def __init__(self, parent=None):
        super().__init__(parent)
        self.acquiring_active = False
        self.locked_state = False
        self.running_state = False
        self.init_ui()

    def init_ui(self):
        layout = QVBoxLayout(self)
        layout.setContentsMargins(8, 6, 8, 6)
        layout.setSpacing(3)
        layout.addWidget(section_label("ACQUISITION"))

        # ACAM row
        acam_row = QHBoxLayout()
        acam_row.setSpacing(8)
        acam_row.addWidget(QLabel("ACAM:"))

        self.acam_mode_badge     = Badge("acquiring")
        self.acam_guiding_badge  = Badge("guiding")
        self.acam_acquired_badge = Badge("acquired")
        acam_row.addWidget(self.acam_mode_badge)
        acam_row.addWidget(self.acam_guiding_badge)
        acam_row.addWidget(self.acam_acquired_badge)

        acam_row.addSpacing(16)
        self.seeing = QLabel("seeing: --")
        self.seeing.setStyleSheet(
            f"color: {COLOR_BLUE}; font-weight: bold; font-size: 11pt;"
        )
        acam_row.addWidget(self.seeing)
        acam_row.addStretch(1)
        layout.addLayout(acam_row)

        # SLICECAM row
        slice_row = QHBoxLayout()
        slice_row.setSpacing(8)
        slice_row.addWidget(QLabel("SLICECAM:"))

        self.lbl_locked  = Badge("locked")
        self.lbl_running = Badge("running")
        slice_row.addWidget(self.lbl_locked)
        slice_row.addWidget(self.lbl_running)
        slice_row.addStretch(1)
        layout.addLayout(slice_row)

    def _apply_guiding_blue(self):
        """ Paint the guiding badge with a steady solid-blue style. """
        self.acam_guiding_badge.setStyleSheet(
            f"color: #ffffff; background-color: {COLOR_BLUE}; "
            "border: 1px solid #555; border-radius: 5px; padding: 0px 6px;"
        )

    def set_acamd(self, data):
        """ Update ACAM mode, acquired flag, and seeing from acamd payload. """
        if KEY_ACQUIRE_MODE in data:
            mode = str(data[KEY_ACQUIRE_MODE])
            if mode == "acquiring":
                self.acquiring_active = True
                self.acam_guiding_badge.set_not_ready()
                # acam_mode_badge styling driven by blink_tick
            elif mode == "guiding":
                self.acquiring_active = False
                self.acam_mode_badge.set_not_ready()
                self._apply_guiding_blue()
            else:
                self.acquiring_active = False
                self.acam_mode_badge.set_not_ready()
                self.acam_guiding_badge.set_not_ready()
        if KEY_IS_ACQUIRED in data:
            if bool(data[KEY_IS_ACQUIRED]):
                self.acam_acquired_badge.set_ready()
            else:
                self.acam_acquired_badge.set_not_ready()
        if KEY_SEEING in data:
            if data[KEY_SEEING] is None:
                self.seeing.setText("seeing: --")
            else:
                try:
                    self.seeing.setText(f"seeing: {float(data[KEY_SEEING]):.2f}\"")
                except (TypeError, ValueError):
                    self.seeing.setText("seeing: --")

    def set_slicecamd(self, data):
        """ Update SLICECAM locked / running flags from slicecamd payload. """
        if KEY_FINEACQUIRE_LOCKED in data:
            self.locked_state = bool(data[KEY_FINEACQUIRE_LOCKED])
            if self.locked_state:
                # locked = done; show steady green (not busy, not blinking)
                self.lbl_locked.set_ready()
            else:
                self.lbl_locked.set_not_ready()
        if KEY_FINEACQUIRE_RUNNING in data:
            self.running_state = bool(data[KEY_FINEACQUIRE_RUNNING])
            if not self.running_state:
                self.lbl_running.set_not_ready()

    def set_acamd_online(self, online):
        """ Clear ACAM indicators when acamd goes offline (defense-in-depth). """
        if not online:
            self.acquiring_active = False
            self.acam_mode_badge.set_not_ready()
            self.acam_guiding_badge.set_not_ready()
            self.acam_acquired_badge.set_not_ready()
            self.seeing.setText("seeing: --")

    def set_slicecamd_online(self, online):
        """ Clear SLICECAM indicators when slicecamd goes offline. """
        if not online:
            self.locked_state = False
            self.running_state = False
            self.lbl_locked.set_not_ready()
            self.lbl_running.set_not_ready()

    def blink_tick(self, phase):
        """ Drive blink phase for any active acquisition badges. """
        # locked is a stable "done" state -- never blink, stay steady green
        # when set (already applied in set_slicecamd).
        if self.acquiring_active:
            self.acam_mode_badge.set_blink(phase)
        if self.running_state:
            self.lbl_running.set_blink(phase)
