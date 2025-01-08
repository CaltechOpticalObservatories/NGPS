import sys
from PyQt5.QtWidgets import QMainWindow, QApplication, QTabWidget, QVBoxLayout, QLabel, QPushButton, QWidget, QDesktopWidget
from tabs.commands_tab import CommandsTab
from tabs.afternoon_tab import AfternoonTab
from tabs.focus_tab import FocusTab
from tabs.science_tab import ScienceTab
import os

class CalibrationGUI(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Calibrations")
        self.setGeometry(100, 100, 600, 400)
        self.initUI()

    def initUI(self):
        # Create the QTabWidget
        tab_widget = QTabWidget()

        # Create the tabs
        commands_tab = CommandsTab()
        afternoon_tab = AfternoonTab()
        focus_tab = FocusTab()
        science_tab = ScienceTab()

        # Add tabs to the tab widget
        tab_widget.addTab(focus_tab, "Focus")
        tab_widget.addTab(afternoon_tab, "Afternoon")
        tab_widget.addTab(science_tab, "Science")
        tab_widget.addTab(commands_tab, "Commands")

        # Set the tab widget as the central widget of the main window
        self.setCentralWidget(tab_widget)
        
        self.load_stylesheet("styles.qss")
        
        # Adjust window size based on screen resolution
        self.adjust_window_size()

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
            print(f"Stylesheet file {filename} not found.")

def main():
    app = QApplication(sys.argv)
    main_window = CalibrationGUI()
    main_window.show()
    sys.exit(app.exec_())

if __name__ == "__main__":
    main()
