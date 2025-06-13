import sys
import os
from PyQt5.QtWidgets import (
    QMainWindow, QApplication, QTabWidget, QDesktopWidget,
    QVBoxLayout, QTextEdit, QWidget, QPushButton
)
from PyQt5.QtCore import pyqtSignal

from calib.tabs.commands_tab import CommandsTab
from calib.tabs.afternoon_tab import AfternoonTab
from calib.tabs.focus_tab import FocusTab
from calib.tabs.science_tab import ScienceTab


class CalibrationGUI(QMainWindow):
    log_signal = pyqtSignal(str)

    def __init__(self):
        super().__init__()
        self.setWindowTitle("Calibrations")
        self.setGeometry(100, 100, 600, 400)
        self.initUI()

    def initUI(self):
        # Create a QTextEdit for logging
        self.log_text_edit = QTextEdit(self)
        self.log_text_edit.setReadOnly(True)
        self.log_text_edit.setPlaceholderText("Log messages will appear here...")
        self.log_text_edit.setMinimumHeight(150)

        # Connect signal to safe GUI update method
        self.log_signal.connect(self.append_log)

        # Define the thread-safe logging callback function
        def log_message(msg):
            self.log_signal.emit(msg)

        # Create the QTabWidget
        tab_widget = QTabWidget()

        # Create the tabs
        self.afternoon_tab = AfternoonTab(log_message_callback=self.log_signal.emit)
        self.commands_tab = CommandsTab()
        self.focus_tab = FocusTab(log_message_callback=self.log_signal.emit)
        self.science_tab = ScienceTab()

        # Add tabs to the tab widget
        tab_widget.addTab(self.afternoon_tab, "Afternoon")
        tab_widget.addTab(self.focus_tab, "Focus")
        tab_widget.addTab(self.science_tab, "Science")
        tab_widget.addTab(self.commands_tab, "Commands")

        # Set up layout and widgets
        main_layout = QVBoxLayout()
        main_layout.addWidget(tab_widget, stretch=3)
        main_layout.addWidget(self.log_text_edit, stretch=2)

        # Optionally, add a button to clear the log
        clear_log_button = QPushButton("Clear Log", self)
        clear_log_button.clicked.connect(self.clear_log)
        main_layout.addWidget(clear_log_button, stretch=0)

        # Create a QWidget and set the layout
        central_widget = QWidget(self)
        central_widget.setLayout(main_layout)
        self.setCentralWidget(central_widget)

        self.load_stylesheet("styles.qss")
        self.adjust_window_size()

    def append_log(self, message):
        """Safely append a log message to the QTextEdit widget from the main thread."""
        self.log_text_edit.append(message)

    def clear_log(self):
        """Clear the log messages."""
        self.log_text_edit.clear()

    def adjust_window_size(self):
        screen = QDesktopWidget().screenGeometry()
        width = screen.width()
        height = screen.height()

        new_width = int(width * 0.4)
        new_height = int(height * 0.6)
        self.setFixedSize(new_width, new_height)

    def resizeEvent(self, event):
        aspect_ratio = 4 / 3
        current_width = self.width()
        current_height = self.height()

        if current_width / current_height > aspect_ratio:
            new_width = int(current_height * aspect_ratio)
            new_height = current_height
        else:
            new_height = int(current_width / aspect_ratio)
            new_width = current_width

        self.resize(new_width, new_height)
        super().resizeEvent(event)

    def load_stylesheet(self, filename):
        if os.path.exists(filename):
            with open(filename, "r") as file:
                stylesheet = file.read()
                self.setStyleSheet(stylesheet)
        else:
            self.log_signal.emit(f"Stylesheet file {filename} not found.")


def main():
    app = QApplication(sys.argv)
    main_window = CalibrationGUI()
    main_window.show()
    main_window.log_signal.emit("Calibration started...")  # Thread-safe initial log
    sys.exit(app.exec_())


if __name__ == "__main__":
    main()
