from PyQt5.QtWidgets import QWidget, QVBoxLayout, QLabel, QPushButton, QLineEdit, QComboBox, QHBoxLayout, QScrollArea
import subprocess

class CalibTab(QWidget):
    def __init__(self):
        super().__init__()
        self.initUI()

    def initUI(self):
        layout = QVBoxLayout()

        # Label for the Calib Tab
        label = QLabel("Calibration Control Tab", self)
        layout.addWidget(label)

        # List of basic calib commands
        calib_commands = [
            "exit", "home", "ishome", "sendtelem"
        ]

        # Add buttons for basic calib commands
        for command in calib_commands:
            button = QPushButton(f"Execute calib {command}", self)
            button.clicked.connect(lambda checked, cmd=command: self.execute_calib_command(cmd))
            layout.addWidget(button)

        # Add dropdown (ComboBox) and button for commands with motion/lampmod options
        self.motion_lampmod_combo = QComboBox(self)
        self.motion_lampmod_combo.addItems(["motion", "lampmod"])

        # Command: calib open [motion | lampmod]
        self.open_button = QPushButton("Execute calib open", self)
        self.open_button.clicked.connect(self.execute_open_command)
        layout.addWidget(self.open_button)
        layout.addWidget(self.motion_lampmod_combo)

        # Command: calib isopen [motion | lampmod]
        self.isopen_button = QPushButton("Execute calib isopen", self)
        self.isopen_button.clicked.connect(self.execute_isopen_command)
        layout.addWidget(self.isopen_button)
        layout.addWidget(self.motion_lampmod_combo)

        # Command: calib close [motion | lampmod]
        self.close_button = QPushButton("Execute calib close", self)
        self.close_button.clicked.connect(self.execute_close_command)
        layout.addWidget(self.close_button)
        layout.addWidget(self.motion_lampmod_combo)

        # Command: calib get <actuator>
        self.get_input = QLineEdit(self)
        self.get_input.setPlaceholderText("Enter actuator for 'get' command")
        get_button = QPushButton("Execute calib get <actuator>", self)
        get_button.clicked.connect(self.execute_get_command)
        layout.addWidget(self.get_input)
        layout.addWidget(get_button)

        # Command: calib native <addr> <cmd>
        self.addr_input = QLineEdit(self)
        self.addr_input.setPlaceholderText("Enter address for 'native' command")
        self.cmd_input = QLineEdit(self)
        self.cmd_input.setPlaceholderText("Enter command for 'native' command")
        native_button = QPushButton("Execute calib native <addr> <cmd>", self)
        native_button.clicked.connect(self.execute_native_command)
        layout.addWidget(self.addr_input)
        layout.addWidget(self.cmd_input)
        layout.addWidget(native_button)

        # Command: calib set <actuator>=open|close
        self.set_input = QLineEdit(self)
        self.set_input.setPlaceholderText("Enter actuator and state (e.g., actuator=open)")
        set_button = QPushButton("Execute calib set <actuator>=open|close", self)
        set_button.clicked.connect(self.execute_set_command)
        layout.addWidget(self.set_input)
        layout.addWidget(set_button)

        # Lampmod command dropdowns
        self.lampmod_actions_combo = QComboBox(self)
        self.lampmod_actions_combo.addItems(["open", "close", "reconnect", "default"])

        self.lampmod_onoff_combo = QComboBox(self)
        self.lampmod_onoff_combo.addItems(["on", "off"])

        # Command: calib lampmod [open | close | reconnect | default] [on | off]
        self.lampmod_button = QPushButton("Execute calib lampmod", self)
        self.lampmod_button.clicked.connect(self.execute_lampmod_command)
        layout.addWidget(self.lampmod_actions_combo)
        layout.addWidget(self.lampmod_onoff_combo)
        layout.addWidget(self.lampmod_button)

        # Now, create a QWidget to contain all the layout
        form_widget = QWidget(self)
        form_widget.setLayout(layout)

        # Create a QScrollArea and set the QWidget containing the layout as its widget
        scroll_area = QScrollArea(self)
        scroll_area.setWidgetResizable(True)  # Ensures the content resizes as needed
        scroll_area.setWidget(form_widget)

        # Set the scrollable area as the main layout
        final_layout = QVBoxLayout()
        final_layout.addWidget(scroll_area)
        self.setLayout(final_layout)

    def execute_command(self, command: str):
        """Executes the given command in the terminal."""
        try:
            print(f"Running command: {command}")
            subprocess.run(command, shell=True, check=True)  # Execute the command in the terminal
        except subprocess.CalledProcessError as e:
            print(f"Error executing command: {e}")

    def execute_calib_command(self, command: str):
        """Execute the basic calib command (e.g., calib open, calib close)."""
        motion_lampmod = self.motion_lampmod_combo.currentText()
        if command == "open" or command == "close" or command == "isopen":
            full_command = f"calib {command} {motion_lampmod}"
        else:
            full_command = f"calib {command}"
        self.execute_command(full_command)

    def execute_open_command(self):
        """Execute the calib open command with motion/lampmod."""
        motion_lampmod = self.motion_lampmod_combo.currentText()
        full_command = f"calib open {motion_lampmod}"
        self.execute_command(full_command)

    def execute_isopen_command(self):
        """Execute the calib isopen command with motion/lampmod."""
        motion_lampmod = self.motion_lampmod_combo.currentText()
        full_command = f"calib isopen {motion_lampmod}"
        self.execute_command(full_command)

    def execute_close_command(self):
        """Execute the calib close command with motion/lampmod."""
        motion_lampmod = self.motion_lampmod_combo.currentText()
        full_command = f"calib close {motion_lampmod}"
        self.execute_command(full_command)

    def execute_get_command(self):
        """Execute the calib get <actuator> command."""
        actuator = self.get_input.text()
        if actuator:
            full_command = f"calib get {actuator}"
            self.execute_command(full_command)
        else:
            print("Please provide an actuator.")

    def execute_native_command(self):
        """Execute the calib native <addr> <cmd> command."""
        addr = self.addr_input.text()
        cmd = self.cmd_input.text()
        if addr and cmd:
            full_command = f"calib native {addr} {cmd}"
            self.execute_command(full_command)
        else:
            print("Please provide both address and command.")

    def execute_set_command(self):
        """Execute the calib set <actuator>=open|close command."""
        set_input = self.set_input.text()
        if set_input:
            full_command = f"calib set {set_input}"
            self.execute_command(full_command)
        else:
            print("Please provide actuator and state.")

    def execute_lampmod_command(self):
        """Execute the calib lampmod command with selected parameters."""
        action = self.lampmod_actions_combo.currentText()
        onoff = self.lampmod_onoff_combo.currentText()
        full_command = f"calib lampmod {action} {onoff}"
        self.execute_command(full_command)
