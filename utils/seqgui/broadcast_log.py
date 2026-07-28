from datetime import datetime
from PyQt5.QtGui import QColor, QFont, QTextCharFormat, QTextCursor
from PyQt5.QtWidgets import QPlainTextEdit

from panels import COLOR_RED, COLOR_YELLOW


class BroadcastLog(QPlainTextEdit):
    """ Scrolling color-coded broadcast message log. """

    def __init__(self, parent=None):
        super().__init__(parent)
        self.init_ui()

    def init_ui(self):
        self.setReadOnly(True)
        self.setMaximumBlockCount(2000)
        # Don't wrap long lines; show a horizontal scrollbar instead (as needed),
        # mirroring the vertical one.
        self.setLineWrapMode(QPlainTextEdit.NoWrap)
        font = QFont("Monospace")
        font.setStyleHint(QFont.TypeWriter)
        font.setPointSize(9)
        self.setFont(font)

    def append(self, severity, source, message):
        """ Append one colorized broadcast line with timestamp and source. """
        if severity == "ERROR":
            color = QColor(COLOR_RED)
        elif severity == "WARNING":
            color = QColor(COLOR_YELLOW)
        else:
            color = QColor("#cccccc")

        ts = datetime.now().strftime("%H:%M:%S")
        prefix = f"[{ts}] [{source}] "
        if severity and severity != "NOTICE":
            prefix += f"{severity}: "

        fmt = QTextCharFormat()
        fmt.setForeground(color)
        cursor = self.textCursor()
        cursor.movePosition(QTextCursor.End)
        cursor.insertText(prefix + message + "\n", fmt)
        self.setTextCursor(cursor)
        self.ensureCursorVisible()
