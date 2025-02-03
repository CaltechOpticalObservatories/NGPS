import sys
from PyQt5.QtWidgets import QMainWindow, QApplication, QTabWidget, QDesktopWidget, QVBoxLayout, QTextEdit, QWidget, QPushButton
from calib.tabs.commands_tab import CommandsTab
from calib.tabs.afternoon_tab import AfternoonTab
from calib.tabs.focus_tab import FocusTab
from calib.tabs.science_tab import ScienceTab
import os

class CalibrationGUI(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Calibrations")
        self.setGeometry(100, 100, 600, 400)
        self.initUI()

    def initUI(self):
        
        # Create a QTextEdit for logging
        self.log_text_edit = QTextEdit(self)
        self.log_text_edit.setReadOnly(True)  # Make it read-only to prevent user editing
        self.log_text_edit.setPlaceholderText("Log messages will appear here...")
        
        # Define the logging callback function
        def log_message(msg):
            self.log_text_edit.append(msg)  # Add message to the QTextEdit
        
        # Create the QTabWidget
        tab_widget = QTabWidget()

        # Create the tabs
        self.afternoon_tab = AfternoonTab(log_message_callback=log_message)
        self.commands_tab = CommandsTab()
        self.focus_tab = FocusTab(log_message_callback=log_message)
        self.science_tab = ScienceTab()

        # Add tabs to the tab widget
        tab_widget.addTab(self.afternoon_tab, "Afternoon")
        tab_widget.addTab(self.focus_tab, "Focus")
        tab_widget.addTab(self.science_tab, "Science")
        tab_widget.addTab(self.commands_tab, "Commands")

        # Set the tab widget as the central widget of the main window
        main_layout = QVBoxLayout()
        main_layout.addWidget(tab_widget)

        # Add the QTextEdit to the layout
        main_layout.addWidget(self.log_text_edit)

        # Optionally, add a button to clear the log
        clear_log_button = QPushButton("Clear Log", self)
        clear_log_button.clicked.connect(self.clear_log)
        main_layout.addWidget(clear_log_button)

        # Create a QWidget and set the layout
        central_widget = QWidget(self)
        central_widget.setLayout(main_layout)
        self.setCentralWidget(central_widget)

        self.load_stylesheet("styles.qss")
        
        # Adjust window size based on screen resolution
        self.adjust_window_size()

    def log_message(self, message):
        """Append a log message to the QTextEdit widget."""
        self.log_text_edit.append(message)

    def clear_log(self):
        """Clear the log messages."""
        self.log_text_edit.clear()

    def adjust_window_size(self):
        screen = QDesktopWidget().screenGeometry()  # Get screen size
        width = screen.width()
        height = screen.height()

        # Set a default window size as a percentage of screen size (for example, 80% width and 60% height)
        new_width = int(width * 0.4)
        new_height = int(height * 0.6)

        self.setFixedSize(new_width, new_height)  # Set window size dynamically

    def resizeEvent(self, event):
        # Maintain 4:3 aspect ratio on window resizing
        aspect_ratio = 4 / 3
        current_width = self.width()
        current_height = self.height()

        # Calculate the new width/height to preserve aspect ratio
        if current_width / current_height > aspect_ratio:
            new_width = int(current_height * aspect_ratio)  # Ensure integer type
            new_height = current_height
        else:
            new_height = int(current_width / aspect_ratio)  # Ensure integer type
            new_width = current_width

        # Resize the window to maintain aspect ratio
        self.resize(new_width, new_height)

        # Call the original resizeEvent
        super().resizeEvent(event)

    def load_stylesheet(self, filename):
        """Load and apply the stylesheet from a .qss file"""
        if os.path.exists(filename):
            with open(filename, "r") as file:
                stylesheet = file.read()
                self.setStyleSheet(stylesheet)
        else:
            self.log_message(f"Stylesheet file {filename} not found.")

def main():
    app = QApplication(sys.argv)
    main_window = CalibrationGUI()
    main_window.show()
    # Example of logging a message
    main_window.log_message("Calibration started...")
    sys.exit(app.exec_())

if __name__ == "__main__":
    main()
