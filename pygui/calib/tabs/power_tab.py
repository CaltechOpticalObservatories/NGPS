import subprocess
from PyQt5.QtWidgets import QWidget, QVBoxLayout, QLabel, QPushButton, QLineEdit, QComboBox, QHBoxLayout, QScrollArea

class PowerTab(QWidget):
    def __init__(self):
        super().__init__()
        self.initUI()

    def initUI(self):
        layout = QVBoxLayout()

        # Label for the Power Tab
        label = QLabel("Power Control Tab", self)
        layout.addWidget(label)

        # List of subcommands for Power (from previous example)
        power_commands = [
            "close", "isopen", "list", "open", 
            "reopen", "status", "sendtelem"
        ]

        # Create a button for each Power subcommand (close, isopen, etc.)
        for command in power_commands:
            button = QPushButton(f"Execute power {command}", self)
            button.clicked.connect(lambda checked, cmd=command: self.execute_power_command(cmd))
            layout.addWidget(button)

        # Add the new command widgets for <unit#> <plug#> [ON | OFF | BOOT]
        # Unit and Plug number inputs
        unit_layout = QHBoxLayout()
        self.unit_input = QLineEdit(self)
        self.unit_input.setPlaceholderText("Enter unit#")
        self.plug_input = QLineEdit(self)
        self.plug_input.setPlaceholderText("Enter plug#")
        unit_layout.addWidget(self.unit_input)
        unit_layout.addWidget(self.plug_input)
        
        # Combo box for ON, OFF, BOOT options (for unit and plug)
        self.state_combo = QComboBox(self)
        self.state_combo.addItems(["ON", "OFF", "BOOT"])

        # Button for executing the "power unit plug [ON/OFF/BOOT]" command
        execute_unit_plug_button = QPushButton("Execute power <unit#> <plug#> [ON/OFF/BOOT]", self)
        execute_unit_plug_button.clicked.connect(self.execute_unit_plug_command)

        # Add widgets for unit, plug and state to layout
        layout.addLayout(unit_layout)
        layout.addWidget(self.state_combo)
        layout.addWidget(execute_unit_plug_button)

        # Add the widgets for the plugname and state combo box
        self.plugname_input = QLineEdit(self)
        self.plugname_input.setPlaceholderText("Enter plugname")

        # Combo box for ON, OFF, BOOT options (for plugname)
        self.state_combo_plugname = QComboBox(self)
        self.state_combo_plugname.addItems(["ON", "OFF", "BOOT"])

        # Button for executing the "power <plugname> [ON/OFF/BOOT]" command
        execute_plugname_button = QPushButton("Execute power <plugname> [ON/OFF/BOOT]", self)
        execute_plugname_button.clicked.connect(self.execute_plugname_command)

        # Add plugname input, combo box, and button to layout
        layout.addWidget(self.plugname_input)
        layout.addWidget(self.state_combo_plugname)
        layout.addWidget(execute_plugname_button)

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
            print(f"Executing command: {command}")
            result = subprocess.run(command, shell=True, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            print(f"Output: {result.stdout.decode()}")
        except subprocess.CalledProcessError as e:
            print(f"Error executing command: {e}")
            print(f"stderr: {e.stderr.decode()}")

    def execute_power_command(self, command: str):
        """Execute the basic power command (e.g., power close, power isopen)."""
        full_command = f"power {command}"
        self.execute_command(full_command)

    def execute_unit_plug_command(self):
        """Execute a power command with <unit#> <plug#> [ON/OFF/BOOT]."""
        unit = self.unit_input.text()
        plug = self.plug_input.text()
        state = self.state_combo.currentText()

        if unit and plug:
            full_command = f"power {unit} {plug} {state}"
            self.execute_command(full_command)
        else:
            print("Please provide both unit# and plug#.")

    def execute_plugname_command(self):
        """Execute a power command with <plugname> [ON/OFF/BOOT]."""
        plugname = self.plugname_input.text()
        state = self.state_combo_plugname.currentText()

        if plugname:
            full_command = f"power {plugname} {state}"
            self.execute_command(full_command)
        else:
            print("Please provide a plugname.")
