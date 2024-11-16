import sys
import subprocess
import shutil
from PyQt6.QtWidgets import QApplication, QWidget, QPushButton, QVBoxLayout, QLabel, QTextEdit, QMessageBox
from PyQt6.QtCore import QTimer


class NgpsGui(QWidget):
    def __init__(self):
        super().__init__()
        
        self.setWindowTitle('NGPS Command GUI')
        self.setGeometry(100, 100, 400, 400)  # Set window size and position
        
        # Layout to arrange buttons and status area
        self.layout = QVBoxLayout()

        # Create a label for status
        self.status_label = QLabel("Select a command to execute", self)
        self.layout.addWidget(self.status_label)
        
        # List of commands
        self.commands = ['acam', 'scam', 'calib', 'camera', 'power', 'seq', 'slit', 'tcs', 'thermal']
        
        # Create a button for each command
        for cmd in self.commands:
            button = QPushButton(cmd, self)
            button.clicked.connect(self.create_button_handler(cmd))  # Connect each button to the command handler
            self.layout.addWidget(button)
        
        # Text area for displaying the NGPS status
        self.status_text_area = QTextEdit(self)
        self.status_text_area.setReadOnly(True)  # Make it read-only
        self.layout.addWidget(self.status_text_area)
        
        # Set the layout for the main window
        self.setLayout(self.layout)

        # Set up a timer to poll for the NGPS status every 5 seconds (5000 ms)
        self.status_timer = QTimer(self)
        self.status_timer.timeout.connect(self.update_ngps_status)
        self.status_timer.start(5000)  # Poll every 5 seconds

    def create_button_handler(self, cmd):
        """Return a handler for executing the selected command."""
        def handler():
            # Update status
            self.status_label.setText(f"Executing: {cmd}")
            self.execute_command(cmd)
        return handler

    def execute_command(self, cmd):
        """Execute the command in the system shell."""
        try:
            # Run the command using subprocess
            result = subprocess.run([cmd], capture_output=True, text=True, check=True)
            self.status_label.setText(f"Command '{cmd}' executed successfully.")
            print(result.stdout)  # Print the command output to console
        except subprocess.CalledProcessError as e:
            self.status_label.setText(f"Error executing {cmd}: {e}")
            print(e.stderr)  # Print the error to the console

    def update_ngps_status(self):
        """Poll for the NGPS status and update the status text area."""
        if not self.is_ngps_available():
            self.show_ngps_not_available_popup()
            return

        try:
            # Run the 'ngps status' command to get the status of running processes
            result = subprocess.run(['ngps', 'status'], capture_output=True, text=True, check=True)
            status_output = result.stdout
            self.status_text_area.setPlainText(status_output)

            # Check if the daemon is running (you can modify the check to match your actual daemon names)
            if "telemd" not in status_output or "acamd" not in status_output:
                self.show_daemon_not_running_popup()

        except subprocess.CalledProcessError as e:
            self.status_text_area.setPlainText(f"Error fetching status: {e}")
            print(e.stderr)  # Print the error to the console

    def is_ngps_available(self):
        """Check if 'ngps' command is available in the system."""
        return shutil.which("ngps") is not None

    def show_ngps_not_available_popup(self):
        """Show a popup message if the ngps command is not available."""
        msg = QMessageBox(self)
        msg.setIcon(QMessageBox.Icon.Warning)
        msg.setWindowTitle("NGPS Command Not Found")
        msg.setText("The 'ngps' command is not available on this system.\nPlease ensure it is installed and accessible.")
        msg.setStandardButtons(QMessageBox.StandardButton.Ok)

        # Connect the Ok button to the reset function
        msg.buttonClicked.connect(self.on_ngps_popup_button_clicked)
        
        msg.exec()

    def on_ngps_popup_button_clicked(self, button):
        """Handle the button click in the ngps unavailable popup."""
        if button.text() == "&Ok":
            self.reset_and_try_again()

    def reset_and_try_again(self):
        """Attempt to check if 'ngps' is now available and retry."""
        if self.is_ngps_available():
            self.status_label.setText("NGPS command is available. Checking status...")
            self.update_ngps_status()  # Try to check the status again
        else:
            self.status_label.setText("NGPS command is still unavailable. Please try again later.")

    def show_daemon_not_running_popup(self):
        """Show a popup message if a daemon is not running."""
        msg = QMessageBox(self)
        msg.setIcon(QMessageBox.Icon.Warning)
        msg.setWindowTitle("Daemon Not Running")
        msg.setText("The NGPS daemon is not running.\nWould you like to reset and try again?")
        msg.setStandardButtons(QMessageBox.StandardButton.Ok | QMessageBox.StandardButton.Cancel)

        # Connect the Ok button to the reset function
        msg.buttonClicked.connect(self.on_popup_button_clicked)
        
        msg.exec()

    def on_popup_button_clicked(self, button):
        """Handle the button click in the popup."""
        if button.text() == "&Ok":
            self.reset_daemon_and_check()

    def reset_daemon_and_check(self):
        """Attempt to reset the daemon and check status again."""
        try:
            # Try restarting the daemon (you can replace this with your actual reset command)
            subprocess.run(['ngps', 'reset'], capture_output=True, text=True, check=True)
            self.status_label.setText("Daemon reset. Checking status...")
            self.update_ngps_status()  # Recheck status after reset
        except subprocess.CalledProcessError as e:
            self.status_label.setText(f"Error resetting daemon: {e}")
            print(e.stderr)  # Print the error to the console


if __name__ == '__main__':
    app = QApplication(sys.argv)
    window = NgpsGui()
    window.show()
    sys.exit(app.exec())
