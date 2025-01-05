import subprocess
from PyQt5.QtWidgets import QWidget, QVBoxLayout, QLabel, QPushButton, QLineEdit, QScrollArea

class FocusCmdTab(QWidget):
    def __init__(self):
        super().__init__()
        self.initUI()

    def initUI(self):
        layout = QVBoxLayout()

        # Label for the Focus Tab
        label = QLabel("Focus Control Tab", self)
        layout.addWidget(label)

        # Command: close
        close_button = QPushButton("Execute focus close", self)
        close_button.clicked.connect(self.execute_focus_close)
        layout.addWidget(close_button)

        # Command: default
        default_button = QPushButton("Execute focus default", self)
        default_button.clicked.connect(self.execute_focus_default)
        layout.addWidget(default_button)

        # Command: get <chan>
        self.get_chan_input = QLineEdit(self)
        self.get_chan_input.setPlaceholderText("Enter channel for 'get' command")
        get_button = QPushButton("Execute focus get <chan>", self)
        get_button.clicked.connect(self.execute_focus_get)
        layout.addWidget(self.get_chan_input)
        layout.addWidget(get_button)

        # Command: home <chan>
        self.home_chan_input = QLineEdit(self)
        self.home_chan_input.setPlaceholderText("Enter channel for 'home' command")
        home_button = QPushButton("Execute focus home <chan>", self)
        home_button.clicked.connect(self.execute_focus_home)
        layout.addWidget(self.home_chan_input)
        layout.addWidget(home_button)

        # Command: ishome
        ishome_button = QPushButton("Execute focus ishome", self)
        ishome_button.clicked.connect(self.execute_focus_ishome)
        layout.addWidget(ishome_button)

        # Command: isopen
        isopen_button = QPushButton("Execute focus isopen", self)
        isopen_button.clicked.connect(self.execute_focus_isopen)
        layout.addWidget(isopen_button)

        # Command: native <chan> <cmd>
        self.native_chan_input = QLineEdit(self)
        self.native_chan_input.setPlaceholderText("Enter channel for 'native' command")
        self.native_cmd_input = QLineEdit(self)
        self.native_cmd_input.setPlaceholderText("Enter command for 'native' command")
        native_button = QPushButton("Execute focus native <chan> <cmd>", self)
        native_button.clicked.connect(self.execute_focus_native)
        layout.addWidget(self.native_chan_input)
        layout.addWidget(self.native_cmd_input)
        layout.addWidget(native_button)

        # Command: open
        open_button = QPushButton("Execute focus open", self)
        open_button.clicked.connect(self.execute_focus_open)
        layout.addWidget(open_button)

        # Command: set <chan> { <pos> | nominal }
        self.set_chan_input = QLineEdit(self)
        self.set_chan_input.setPlaceholderText("Enter channel for 'set' command")
        self.set_pos_input = QLineEdit(self)
        self.set_pos_input.setPlaceholderText("Enter position for 'set' command")
        set_button = QPushButton("Execute focus set <chan> { <pos> | nominal }", self)
        set_button.clicked.connect(self.execute_focus_set)
        layout.addWidget(self.set_chan_input)
        layout.addWidget(self.set_pos_input)
        layout.addWidget(set_button)

        # Command: sendtelem
        sendtelem_button = QPushButton("Execute focus sendtelem", self)
        sendtelem_button.clicked.connect(self.execute_focus_sendtelem)
        layout.addWidget(sendtelem_button)

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

    # Function to execute a command in the terminal
    def execute_command(self, command: str):
        """Executes the given command in the terminal."""
        try:
            print(f"Executing command: {command}")
            result = subprocess.run(command, shell=True, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            print(f"Output: {result.stdout.decode()}")
        except subprocess.CalledProcessError as e:
            print(f"Error executing command: {e}")
            print(f"stderr: {e.stderr.decode()}")
    
    # Focus command functions (now using execute_command)
    def execute_focus_close(self):
        """Execute the focus close command."""
        command = "focus close"
        self.execute_command(command)

    def execute_focus_default(self):
        """Execute the focus default command."""
        command = "focus default"
        self.execute_command(command)

    def execute_focus_get(self):
        """Execute the focus get <chan> command."""
        chan = self.get_chan_input.text()
        if chan:
            command = f"focus get {chan}"
            self.execute_command(command)
        else:
            print("Please provide a channel for 'get' command.")

    def execute_focus_home(self):
        """Execute the focus home <chan> command."""
        chan = self.home_chan_input.text()
        if chan:
            command = f"focus home {chan}"
            self.execute_command(command)
        else:
            print("Please provide a channel for 'home' command.")

    def execute_focus_ishome(self):
        """Execute the focus ishome command."""
        command = "focus ishome"
        self.execute_command(command)

    def execute_focus_isopen(self):
        """Execute the focus isopen command."""
        command = "focus isopen"
        self.execute_command(command)

    def execute_focus_native(self):
        """Execute the focus native <chan> <cmd> command."""
        chan = self.native_chan_input.text()
        cmd = self.native_cmd_input.text()
        if chan and cmd:
            command = f"focus native {chan} {cmd}"
            self.execute_command(command)
        else:
            print("Please provide both channel and command.")

    def execute_focus_open(self):
        """Execute the focus open command."""
        command = "focus open"
        self.execute_command(command)

    def execute_focus_set(self):
        """Execute the focus set <chan> { <pos> | nominal } command."""
        chan = self.set_chan_input.text()
        pos = self.set_pos_input.text()
        if chan and pos:
            command = f"focus set {chan} {pos}"
            self.execute_command(command)
        else:
            print("Please provide both channel and position for 'set' command.")

    def execute_focus_sendtelem(self):
        """Execute the focus sendtelem command."""
        command = "focus sendtelem"
        self.execute_command(command)
