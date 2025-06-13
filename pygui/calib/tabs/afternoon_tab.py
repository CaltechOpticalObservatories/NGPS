from PyQt5.QtWidgets import QWidget, QVBoxLayout, QLabel, QPushButton, QLineEdit, QFormLayout, QSizePolicy, QHBoxLayout, QScrollArea, QFrame
from calib.tabs.async_command_thread import AsyncCommandThread

class AfternoonTab(QWidget):
    def __init__(self, log_message_callback):
        super().__init__()
        self.log_message_callback = log_message_callback  # Set log_message callback from the parent
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

        self.thrufocus_button = QPushButton("Run thrufocus", self)
        self.thrufocus_button.setStyleSheet("""
        QPushButton {
                 background-color: #4CAF50;  /* Green color */
                 color: white;
                 border-radius: 8px;
                 padding: 10px;
                 border: none;
             }
             QPushButton:hover {
                 background-color: #45a049;  /* Slightly darker green on hover */
             }
             QPushButton:pressed {
                 background-color: #3e8e41;  /* Darker green when pressed */
             }
        """)   
        self.thrufocus_button.clicked.connect(self.run_thrufocus_script)
        self.thrufocus_button.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)  # Button expands horizontally
        form_layout.addRow("", self.thrufocus_button)  # Place button below the input
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

        # Add vertical spacing between sections
        form_layout.addRow("", QLabel())  # Empty row for spacing

        # Add white divider line before Slit Set
        divider = QFrame(self)
        divider.setFrameShape(QFrame.HLine)
        divider.setFrameShadow(QFrame.Sunken)
        divider.setStyleSheet('background-color: white;')  # Set divider color to white
        form_layout.addRow(divider)

        # Add vertical spacing between sections
        form_layout.addRow("", QLabel())  # Empty row for spacing

        # Slit Set Command Section (slit set <value>)
        self.slit_value_input = QLineEdit(self)
        self.slit_value_input.setPlaceholderText("Enter slit value")
        self.slit_value_input.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)  # Input expands horizontally
        form_layout.addRow("Slit Value:", self.slit_value_input)

        slit_button = QPushButton("Set Slit", self)
        slit_button.clicked.connect(self.set_slit)
        slit_button.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)  # Button expands horizontally
        form_layout.addRow("", slit_button)  # Place button below the input

        # Add vertical spacing between sections
        form_layout.addRow("", QLabel())  # Empty row for spacing

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

        # Focus Set Command Section (focus set <band> <value>)
        self.band_r_input = QLineEdit(self)
        self.band_r_input.setPlaceholderText("R")
        self.band_r_input.setReadOnly(True)
        self.band_r_input.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)  # Input expands horizontally
        form_layout.addRow("Band:", self.band_r_input)

        self.value_r_input = QLineEdit(self)
        self.value_r_input.setPlaceholderText("2.45")
        self.value_r_input.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)  # Input expands horizontally
        form_layout.addRow("Focus Value:", self.value_r_input)

        focus_button = QPushButton("Set Focus Value (R Band)", self)
        focus_button.clicked.connect(self.set_focus_r)
        focus_button.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)  # Button expands horizontally
        form_layout.addRow("", focus_button)  # Place button below the input

        # Add vertical spacing between sections
        form_layout.addRow("", QLabel())  # Empty row for spacing

        # Focus Set Command Section (focus set <band> <value>)
        self.band_i_input = QLineEdit(self)
        self.band_i_input.setPlaceholderText("I")
        self.band_i_input.setReadOnly(True)
        self.band_i_input.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)  # Input expands horizontally
        form_layout.addRow("Band:", self.band_i_input)

        self.value_input = QLineEdit(self)
        self.value_input.setPlaceholderText("4.85")
        self.value_input.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)  # Input expands horizontally
        form_layout.addRow("Focus Value:", self.value_input)

        focus_button = QPushButton("Set Focus Value (I Band)", self)
        focus_button.clicked.connect(self.set_focus_i)
        focus_button.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)  # Button expands horizontally
        form_layout.addRow("", focus_button)  # Place button below the input

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

    def set_slit(self):
        slit_value = self.slit_value_input.text()
        if slit_value:
            command = f"slit set {slit_value}"
            self.run_command_in_background(command)
        else:
            self.log_message("Please provide a valid slit value.")

    def set_spatial_binning(self):
        spatial_binning = self.spatial_binning_input.text()
        if spatial_binning:
            command_row = f"camera bin row {spatial_binning}"
            self.run_command_in_background(command_row)
        else:
            self.log_message("Please provide a spatial binning value.")

    def set_spectral_binning(self):
        spectral_binning = self.spectral_binning_input.text()
        if spectral_binning:
            command_col = f"camera bin col {spectral_binning}"
            self.run_command_in_background(command_col)
        else:
            self.log_message("Please provide a spectral binning value.")

    def run_thrufocus_script(self):
        self.thrufocus_button.setEnabled(False)
        self.thrufocus_button.setStyleSheet("""
                 QPushButton {
                     background-color: lightgray;
                 }
         """)    
 
 
        command = f"bash calib/thrufocus"
        self.run_command_in_background(command)
 
        self.thrufocus_button.setEnabled(True)
        self.thrufocus_button.setStyleSheet("""
             QPushButton {
                 background-color: #4CAF50;  /* Green color */
                 color: white;
                 border-radius: 8px;
                 padding: 10px;
                 border: none;
             }
             QPushButton:hover {
                 background-color: #45a049;  /* Slightly darker green on hover */
             }
             QPushButton:pressed {
                 background-color: #3e8e41;  /* Darker green when pressed */
             }
        """)  

    def set_focus_r(self):
        if self.value_r_input.placeholderText():
            value = self.value_r_input.placeholderText()
        else:
            value = self.value_r_input.text()
        if value:
            command = f"focus set R {value}"
            self.run_command_in_background(command)
        else:
            self.log_message("Please provide both band and value for the focus set command.")

    def set_focus_i(self):
        if self.value_input.placeholderText():
            value = self.value_input.placeholderText()
        else:
            value = self.value_input.text()
        if value:
            command = f"focus set I {value}"
            self.run_command_in_background(command)
        else:
            self.log_message("Please provide both band and value for the focus set command.")

    def run_getcalib(self):
        command = "bash calib/getcalib"
        self.run_command_in_background(command)

    def run_getcalib_flat(self):
        command = "bash calib/getcalib_flats"
        self.run_command_in_background(command)

    def run_command_in_background(self, command):
        """Run the command in a background thread."""
        self.thread = AsyncCommandThread(command, self.log_message_callback)
        self.thread.output_signal.connect(self.log_message_callback)
        self.thread.start()
