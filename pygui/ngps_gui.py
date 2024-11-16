# ngps_gui.py

import sys
from PyQt5.QtWidgets import QApplication, QMainWindow, QWidget, QVBoxLayout, QPushButton, QLabel, QTextEdit, QMessageBox, QMenuBar, QMenu, QAction, QGroupBox
from PyQt5.QtCore import QTimer
from ngps_viewmodel import NgpsViewModel

class NgpsGui(QMainWindow):
    def __init__(self):
        super().__init__()

        self.setWindowTitle('NGPS Command GUI')
        self.setGeometry(100, 100, 600, 500)

        # Initialize the ViewModel
        self.view_model = NgpsViewModel()

        # Central widget and layout for buttons and status area
        central_widget = QWidget(self)
        self.setCentralWidget(central_widget)
        self.layout = QVBoxLayout(central_widget)

        # Create a label for the command execution status
        self.status_label = QLabel("Select a command to execute", self)
        self.layout.addWidget(self.status_label)

        # List of commands
        self.commands = ['acam', 'scam', 'calib', 'camera', 'power', 'seq', 'slit', 'tcs', 'thermal']

        # Create a button for each command
        for cmd in self.commands:
            button = QPushButton(cmd, self)
            button.clicked.connect(self.create_button_handler(cmd))
            self.layout.addWidget(button)

        # Text area for displaying the NGPS status
        self.status_text_area = QTextEdit(self)
        self.status_text_area.setReadOnly(True)  # Make it read-only
        self.layout.addWidget(self.status_text_area)

        # Instrument Status Section
        self.create_instrument_status_section()

        # Set up the menu bar
        self.create_menu()

        # Set up a timer to poll for the NGPS status every 5 seconds (5000 ms)
        self.status_timer = QTimer(self)
        self.status_timer.timeout.connect(self.update_ngps_status)
        self.status_timer.start(5000)  # Poll every 5 seconds

    def create_instrument_status_section(self):
        """Create a section to display instrument status."""
        self.status_group = QGroupBox("Instrument Status", self)
        self.layout.addWidget(self.status_group)

        status_layout = QVBoxLayout(self.status_group)

        self.camera_status_label = QLabel("Camera: Unknown", self)
        self.power_status_label = QLabel("Power: Unknown", self)
        self.thermal_status_label = QLabel("Thermal: Unknown", self)

        status_layout.addWidget(self.camera_status_label)
        status_layout.addWidget(self.power_status_label)
        status_layout.addWidget(self.thermal_status_label)

    def create_menu(self):
        """Create the menu bar with File, Tools, and Help menus."""
        menubar = self.menuBar()

        # File menu
        file_menu = menubar.addMenu("File")
        exit_action = QAction("Exit", self)
        exit_action.triggered.connect(self.close)
        file_menu.addAction(exit_action)

        # Tools menu
        tools_menu = menubar.addMenu("Tools")
        check_status_action = QAction("Check NGPS Status", self)
        check_status_action.triggered.connect(self.update_ngps_status)
        tools_menu.addAction(check_status_action)

        reset_daemon_action = QAction("Reset Daemon", self)
        reset_daemon_action.triggered.connect(self.reset_daemon)
        tools_menu.addAction(reset_daemon_action)

        # Help menu
        help_menu = menubar.addMenu("Help")
        about_action = QAction("About", self)
        about_action.triggered.connect(self.show_about)
        help_menu.addAction(about_action)

    def create_button_handler(self, cmd):
        """Return a handler for executing the selected command."""
        def handler():
            self.status_label.setText(f"Executing: {cmd}")
            result = self.view_model.execute_command(cmd)
            self.status_label.setText(result)
        return handler

    def update_ngps_status(self):
        """Poll for the NGPS status and update the status text area."""
        if not self.view_model.check_ngps_status():
            self.show_ngps_not_available_popup()
            return

        status = self.view_model.fetch_ngps_status()
        self.status_text_area.setPlainText(status)

        # Update the Instrument Status section based on the status output
        self.update_instrument_status(status)

    def update_instrument_status(self, status_output):
        """Update the instrument status section based on the output."""
        if "camera" in status_output:
            self.camera_status_label.setText("Camera: Running")
        else:
            self.camera_status_label.setText("Camera: Not Running")

        if "power" in status_output:
            self.power_status_label.setText("Power: On")
        else:
            self.power_status_label.setText("Power: Off")

        if "thermal" in status_output:
            self.thermal_status_label.setText("Thermal: Active")
        else:
            self.thermal_status_label.setText("Thermal: Inactive")

    def reset_daemon(self):
        """Reset the NGPS daemon."""
        result = self.view_model.reset_ngps_daemon()
        self.status_label.setText(result)

    def show_about(self):
        """Show the about dialog."""
        QMessageBox.about(self, "About NGPS Command GUI", "This is a GUI for interacting with the NGPS system.\nVersion 1.0")

    def show_ngps_not_available_popup(self):
        """Show a popup message if the ngps command is not available."""
        msg = QMessageBox(self)
        msg.setIcon(QMessageBox.Icon.Warning)
        msg.setWindowTitle("NGPS Command Not Found")
        msg.setText("The 'ngps' command is not available on this system.\nPlease ensure it is installed and accessible.")
        msg.setStandardButtons(QMessageBox.StandardButton.Ok)
        msg.exec_()


if __name__ == '__main__':
    app = QApplication(sys.argv)
    window = NgpsGui()
    window.show()
    sys.exit(app.exec_())
