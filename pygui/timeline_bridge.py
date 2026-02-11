"""
QWebChannel bridge for Plotly timeline ↔ Python communication.

Receives JS events from the Plotly timeline HTML and emits PyQt signals
that database_tab.py connects to for target selection, editing, reordering, etc.
"""

from PyQt5.QtCore import QObject, pyqtSignal, pyqtSlot


class TimelineBridge(QObject):
    """Bridge between Plotly timeline JS and Python via QWebChannel."""

    # Signals emitted when JS calls bridge methods
    targetSelected = pyqtSignal(str)                  # obs_id
    targetDoubleClicked = pyqtSignal(str)             # obs_id
    flagClicked = pyqtSignal(str, str)                # obs_id, flag_text
    contextMenuRequested = pyqtSignal(str, int, int)  # obs_id, screen_x, screen_y
    targetReorderRequested = pyqtSignal(str, str)     # from_obs_id, after_obs_id

    @pyqtSlot(str)
    def onTargetClicked(self, obs_id):
        """Called from JS when a target row/bar is clicked."""
        self.targetSelected.emit(obs_id)

    @pyqtSlot(str)
    def onTargetDoubleClicked(self, obs_id):
        """Called from JS when a target is double-clicked."""
        self.targetDoubleClicked.emit(obs_id)

    @pyqtSlot(str, str)
    def onFlagClicked(self, obs_id, flag_text):
        """Called from JS when a flag annotation is clicked."""
        self.flagClicked.emit(obs_id, flag_text)

    @pyqtSlot(str, int, int)
    def onContextMenu(self, obs_id, screen_x, screen_y):
        """Called from JS on right-click context menu."""
        self.contextMenuRequested.emit(obs_id, screen_x, screen_y)

    @pyqtSlot(str, str)
    def onReorder(self, from_obs_id, after_obs_id):
        """Called from JS when a target is drag-reordered."""
        self.targetReorderRequested.emit(from_obs_id, after_obs_id)
