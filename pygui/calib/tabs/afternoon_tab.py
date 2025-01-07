import subprocess
from PyQt5.QtWidgets import QWidget, QVBoxLayout, QLabel, QPushButton, QLineEdit, QFormLayout, QSizePolicy, QHBoxLayout, QScrollArea

class AfternoonTab(QWidget):
    def __init__(self):
        super().__init__()
        self.initUI()

    def initUI(self):
        layout = QVBoxLayout()
        layout.setSpacing(10)  # Set spacing between sections to make it readable

        # Create a horizontal layout for the label to center it
        label_layout = QHBoxLayout()
        label_layout.setContentsMargins(0, 0, 0, 0)  # Remove outer margins for the label layout

        # Afternoon Tab Label
        label = QLabel("Afternoon Calibration", self)
        label.setSizePolicy(QSizePolicy.Minimum, QSizePolicy.Fixed)  # Keep label fixed in height
        label_layout.addWidget(label)

        layout.addLayout(label_layout)  # Add label layout to the main layout

        # Create a single QFormLayout to combine all sections (Slit Set, Camera Bin, Thrufocus Script, Focus Set)
        form_layout = QFormLayout()
        form_layout.setContentsMargins(10, 10, 10, 10)  # Inner margins around form
        form_layout.setSpacing(10)  # Spacing between rows (increased for better readability)

        # Slit Set Command Section (slit set <value>)
        self.slit_value_input = QLineEdit(self)
        self.slit_value_input.setPlaceholderText("Enter slit value")
        self.slit_value_input.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)  # Input expands horizontally
        form_layout.addRow("Slit Value:", self.slit_value_input)

        slit_button = QPushButton("Set Slit", self)
        slit_button.clicked.connect(self.set_slit)
        slit_button.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)  # Button expands horizontally
        form_layout.addRow("", slit_button)  # Place button below the input

        # Camera Bin Section (spatial binning and spectral binning)
        self.spatial_binning_input = QLineEdit(self)
        self.spatial_binning_input.setPlaceholderText("Enter spatial binning value")
        self.spatial_binning_input.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)  # Input expands horizontally
        form_layout.addRow("Spatial Binning:", self.spatial_binning_input)

        spatial_binning_button = QPushButton("Set Spatial Binning", self)
        spatial_binning_button.clicked.connect(self.set_spatial_binning)
        spatial_binning_button.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)  # Button expands horizontally
        form_layout.addRow("", spatial_binning_button)  # Place button below spatial binning input

        self.spectral_binning_input = QLineEdit(self)
        self.spectral_binning_input.setPlaceholderText("Enter spectral binning value")
        self.spectral_binning_input.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)  # Input expands horizontally
        form_layout.addRow("Spectral Binning:", self.spectral_binning_input)

        spectral_binning_button = QPushButton("Set Spectral Binning", self)
        spectral_binning_button.clicked.connect(self.set_spectral_binning)
        spectral_binning_button.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)  # Button expands horizontally
        form_layout.addRow("", spectral_binning_button)  # Place button below spectral binning input

        # Add vertical spacing between sections
        form_layout.addRow("", QLabel())  # Empty row for spacing

        # Thrufocus Script Section (./thrufocus | tee <output log file>)
        self.log_file_input = QLineEdit(self)
        self.log_file_input.setPlaceholderText("Enter output log file path")
        self.log_file_input.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)  # Input expands horizontally
        form_layout.addRow("Log File Path:", self.log_file_input)

        thrufocus_button = QPushButton("Run thrufocus", self)
        thrufocus_button.clicked.connect(self.run_thrufocus_script)
        thrufocus_button.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)  # Button expands horizontally
        form_layout.addRow("", thrufocus_button)  # Place button below the input

        # Add vertical spacing between sections
        form_layout.addRow("", QLabel())  # Empty row for spacing

        # Focus Set Command Section (focus set <band> <value>)
        self.band_input = QLineEdit(self)
        self.band_input.setPlaceholderText("Enter band (e.g., R, I)")
        self.band_input.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)  # Input expands horizontally
        form_layout.addRow("Band:", self.band_input)

        self.value_input = QLineEdit(self)
        self.value_input.setPlaceholderText("Enter focus value")
        self.value_input.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)  # Input expands horizontally
        form_layout.addRow("Focus Value:", self.value_input)

        focus_button = QPushButton("Set Focus Value", self)
        focus_button.clicked.connect(self.set_focus)
        focus_button.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)  # Button expands horizontally
        form_layout.addRow("", focus_button)  # Place button below the input

        # Add vertical spacing between sections
        form_layout.addRow("", QLabel())  # Empty row for spacing

        # Getcalib Command Section (./getcalib)
        getcalib_button = QPushButton("Get Calibration", self)
        getcalib_button.clicked.connect(self.run_getcalib)
        getcalib_button.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)  # Button expands horizontally
        form_layout.addRow("", getcalib_button)  # Place button below the input

        # Getcalib_flat Command Section (./getcalib_flat)
        getcalib_flat_button = QPushButton("Get Calibration Flats", self)
        getcalib_flat_button.clicked.connect(self.run_getcalib_flat)
        getcalib_flat_button.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)  # Button expands horizontally
        form_layout.addRow("", getcalib_flat_button)  # Place button below the input

        # Create a QWidget to hold the form layout
        form_widget = QWidget(self)
        form_widget.setLayout(form_layout)

        # Create a QScrollArea to make the form scrollable
        scroll_area = QScrollArea(self)
        scroll_area.setWidgetResizable(True)  # Ensures the content resizes as needed
        scroll_area.setWidget(form_widget)

        # Add the scroll area to the main layout
        layout.addWidget(scroll_area)

        # Set the main layout
        self.setLayout(layout)

    def execute_command(self, command):
        """Runs the given command in the terminal"""
        try:
            print(f"Running command: {command}")
            subprocess.run(command, shell=True, check=True)
        except subprocess.CalledProcessError as e:
            print(f"Error executing command: {e}")

    def start_afternoon_session(self):
        print("Afternoon session started...")

    def set_slit(self):
        # Get input value for the slit set command
        slit_value = self.slit_value_input.text()

        # Ensure the slit value is provided
        if slit_value:
            command = f"slit set {slit_value}"
            self.execute_command(command)
        else:
            print("Please provide a valid slit value.")

    def set_spatial_binning(self):
        # Get input value for spatial binning
        spatial_binning = self.spatial_binning_input.text()

        # Ensure the spatial binning value is provided
        if spatial_binning:
            command_row = f"camera bin row {spatial_binning}"
            self.execute_command(command_row)
        else:
            print("Please provide a spatial binning value.")

    def set_spectral_binning(self):
        # Get input value for spectral binning
        spectral_binning = self.spectral_binning_input.text()

        # Ensure the spectral binning value is provided
        if spectral_binning:
            command_col = f"camera bin col {spectral_binning}"
            self.execute_command(command_col)
        else:
            print("Please provide a spectral binning value.")

    def run_thrufocus_script(self):
        # Get the output log file path
        log_file = self.log_file_input.text()

        # Ensure the log file path is provided
        if log_file:
            command = f"bash thrufocus | tee {log_file}"
            self.execute_command(command)
        else:
            print("Please provide a valid log file path.")

    def set_focus(self):
        # Get input values for the focus set command
        band = self.band_input.text()
        value = self.value_input.text()

        # Ensure both band and value are provided
        if band and value:
            command = f"focus set {band} {value}"
            self.execute_command(command)
        else:
            print("Please provide both band and value for the focus set command.")

    def run_getcalib(self):
        """Runs the getcalib command"""
        command = "bash getcalib"
        self.execute_command(command)

    def run_getcalib_flat(self):
        """Runs the getcalib_flat command"""
        command = "bash getcalib_flat"
        self.execute_command(command)
