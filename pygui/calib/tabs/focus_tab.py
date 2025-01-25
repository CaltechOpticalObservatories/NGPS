from PyQt5.QtWidgets import QWidget, QVBoxLayout, QLabel, QPushButton, QLineEdit, QFormLayout, QFrame, QScrollArea, QSizePolicy, QHBoxLayout, QSpacerItem
from PyQt5.QtCore import Qt
import subprocess

class FocusTab(QWidget):
    def __init__(self):
        super().__init__()
        self.initUI()

    def initUI(self):
        main_layout = QVBoxLayout()

        # Scroll Area Widget to make layout scrollable
        scroll_area_widget = QWidget()  # Create a widget that will be scrolled
        scroll_area_layout = QVBoxLayout()  # Layout for the scrollable widget
        
        # Turn on Band of Interest Section
        boi_label = QLabel("Turn on Band of Interest", self)
        scroll_area_layout.addWidget(boi_label)

        # Form Layout for Parameterized BOI
        boi_form_layout = QFormLayout()

        # Channel input field
        self.channel_input = QLineEdit(self)
        self.channel_input.setPlaceholderText("R")
        self.channel_input.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        self.channel_input.setMinimumHeight(35)
        boi_form_layout.addRow("Channel:", self.channel_input)
        spacer = QSpacerItem(20, 40, QSizePolicy.Minimum, QSizePolicy.Expanding)
        boi_form_layout.addItem(spacer)

        # Skip Rows input field
        self.skip_rows_input = QLineEdit(self)
        self.skip_rows_input.setPlaceholderText("400")
        self.skip_rows_input.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        self.skip_rows_input.setMinimumHeight(35)
        boi_form_layout.addRow("Skip Rows:", self.skip_rows_input)
        boi_form_layout.addItem(spacer)

        # Rows to Read input field
        self.rows_input = QLineEdit(self)
        self.rows_input.setPlaceholderText("200")
        self.rows_input.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        self.rows_input.setMinimumHeight(35)
        boi_form_layout.addRow("Rows to Read:", self.rows_input)
        boi_form_layout.addItem(spacer)

        scroll_area_layout.addLayout(boi_form_layout)

        # Camera BOI Button with parameters (Centered)
        boi_button = QPushButton("Activate BOI", self)
        boi_button.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)

        # Create a horizontal layout for the button to center it
        button_layout = QHBoxLayout()
        button_layout.addWidget(boi_button)
        button_layout.setAlignment(boi_button, Qt.AlignCenter)  # Center the button

        boi_button.clicked.connect(self.activate_boi)
        scroll_area_layout.addLayout(button_layout)

        # Full BOI Section
        self.full_channel_input = QLineEdit(self)
        self.full_channel_input.setPlaceholderText("Enter channel")
        self.full_channel_input.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        scroll_area_layout.addWidget(QLabel("Enter channel for full BOI:"))
        scroll_area_layout.addWidget(self.full_channel_input)

        # Full BOI Button (Centered)
        boi_full_button = QPushButton("Activate BOI (Full)", self)
        boi_full_button.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        full_button_layout = QHBoxLayout()
        full_button_layout.addWidget(boi_full_button)
        full_button_layout.setAlignment(boi_full_button, Qt.AlignCenter)  # Center the button

        boi_full_button.clicked.connect(self.activate_boi_full)
        scroll_area_layout.addLayout(full_button_layout)

        # Divider (horizontal line) to separate sections
        divider = QFrame(self)
        divider.setFrameShape(QFrame.HLine)
        divider.setFrameShadow(QFrame.Sunken)
        scroll_area_layout.addWidget(divider)

        # Camera and Focus Section
        commands_label = QLabel("Camera and Focus Commands", self)
        scroll_area_layout.addWidget(commands_label)

        # Form Layout for Camera Bin Command
        bin_form_layout = QFormLayout()
        self.axis_input = QLineEdit(self)
        self.axis_input.setPlaceholderText("Enter axis (for camera bin)")
        self.axis_input.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        bin_form_layout.addRow("Axis:", self.axis_input)

        self.binfactor_input = QLineEdit(self)
        self.binfactor_input.setPlaceholderText("Bin factor")
        self.binfactor_input.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        bin_form_layout.addRow("Bin Factor:", self.binfactor_input)

        scroll_area_layout.addLayout(bin_form_layout)
        # Divider (horizontal line) to separate sections

        # Camera Bin Button (Centered)
        bin_button = QPushButton("Activate Camera Bin", self)
        bin_button.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        bin_button_layout = QHBoxLayout()
        bin_button_layout.addWidget(bin_button)
        bin_button_layout.setAlignment(bin_button, Qt.AlignCenter)  # Center the button

        bin_button.clicked.connect(self.activate_bin)
        scroll_area_layout.addLayout(bin_button_layout)
        scroll_area_layout.addWidget(divider)
        
        # Camera Exptime Command Input Fields
        exptime_form_layout = QFormLayout()
        self.exptime_input = QLineEdit(self)
        self.exptime_input.setPlaceholderText("10000")
        self.exptime_input.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        exptime_form_layout.addRow("Exposure Time (ms):", self.exptime_input)
        scroll_area_layout.addLayout(exptime_form_layout)

        # Exposure Time Button (Centered)
        exptime_button = QPushButton("Set Camera Exposure Time", self)
        exptime_button.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        exptime_button_layout = QHBoxLayout()
        exptime_button_layout.addWidget(exptime_button)
        exptime_button_layout.setAlignment(exptime_button, Qt.AlignCenter)  # Center the button

        exptime_button.clicked.connect(self.set_exptime)
        scroll_area_layout.addLayout(exptime_button_layout)

        # Slit Set Command Input Fields
        slit_form_layout = QFormLayout()
        self.slit_width_input = QLineEdit(self)
        self.slit_width_input.setPlaceholderText("5")
        self.slit_width_input.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        slit_form_layout.addRow("Slit Width:", self.slit_width_input)

        self.slit_offset_input = QLineEdit(self)
        self.slit_offset_input.setPlaceholderText("3")
        self.slit_offset_input.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        slit_form_layout.addRow("Slit Offset:", self.slit_offset_input)

        scroll_area_layout.addLayout(slit_form_layout)

        # Slit Button (Centered)
        slit_button = QPushButton("Set Slit", self)
        slit_button.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        slit_button_layout = QHBoxLayout()
        slit_button_layout.addWidget(slit_button)
        slit_button_layout.setAlignment(slit_button, Qt.AlignCenter)  # Center the button

        slit_button.clicked.connect(self.set_slit)
        scroll_area_layout.addLayout(slit_button_layout)
        scroll_area_layout.addWidget(divider)
        
        # Camstep Focus Command Input Fields (General)
        camstep_form_layout = QFormLayout()
        self.focus_value_input = QLineEdit(self)
        self.focus_value_input.setPlaceholderText("Image Number")
        self.focus_value_input.setMinimumHeight(35)
        self.focus_value_input.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        camstep_form_layout.addRow("Image Number:", self.focus_value_input)
        camstep_form_layout.addItem(spacer)
        camstep_form_layout.addItem(spacer)

        self.focus_upper_input = QLineEdit(self)
        self.focus_upper_input.setPlaceholderText("Upper bound")
        self.focus_upper_input.setMinimumHeight(35)
        self.focus_upper_input.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        camstep_form_layout.addRow("Upper Bound:", self.focus_upper_input)
        camstep_form_layout.addItem(spacer)
        camstep_form_layout.addItem(spacer)

        self.focus_lower_input = QLineEdit(self)
        self.focus_lower_input.setPlaceholderText("Lower bound")
        self.focus_lower_input.setMinimumHeight(35)
        self.focus_lower_input.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        camstep_form_layout.addRow("Lower Bound:", self.focus_lower_input)
        camstep_form_layout.addItem(spacer)
        camstep_form_layout.addItem(spacer)

        self.focus_step_input = QLineEdit(self)
        self.focus_step_input.setPlaceholderText("Step")
        self.focus_step_input.setMinimumHeight(35)
        self.focus_step_input.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        camstep_form_layout.addRow("Step:", self.focus_step_input)

        scroll_area_layout.addLayout(camstep_form_layout)
        camstep_form_layout.addItem(spacer)
        camstep_form_layout.addItem(spacer)

        # Camstep Focus Button (General)
        camstep_button = QPushButton("Camstep Focus (General)", self)
        camstep_button.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        camstep_button_layout = QHBoxLayout()
        camstep_button_layout.addWidget(camstep_button)
        camstep_button_layout.setAlignment(camstep_button, Qt.AlignCenter)  # Center the button

        camstep_button.clicked.connect(self.camstep_focus)
        scroll_area_layout.addLayout(camstep_button_layout)
    
        # Camstep Focus Button (ACAM)
        camstep_acam_button = QPushButton("Camstep Focus (ACAM)", self)
        camstep_acam_button_layout = QHBoxLayout()
        camstep_acam_button_layout.addWidget(camstep_acam_button)
        camstep_acam_button_layout.setAlignment(camstep_acam_button, Qt.AlignCenter)  # Center the button

        camstep_acam_button.clicked.connect(self.camstep_focus_acam)
        scroll_area_layout.addLayout(camstep_acam_button_layout)

        # TCS Set Focus Command Input Fields (tcs setfocus <value>)
        tcs_form_layout = QFormLayout()
        self.tcs_focus_value_input = QLineEdit(self)
        self.tcs_focus_value_input.setPlaceholderText("Set TCS focus value")
        self.tcs_focus_value_input.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        tcs_form_layout.addRow("TCS Focus Value:", self.tcs_focus_value_input)
        scroll_area_layout.addLayout(tcs_form_layout)

        # TCS Set Focus Button (Centered)
        tcs_button = QPushButton("Set TCS Focus", self)
        tcs_button.setFixedWidth(200)  # Set a fixed width for the button
        tcs_button_layout = QHBoxLayout()
        tcs_button_layout.addWidget(tcs_button)
        tcs_button_layout.setAlignment(tcs_button, Qt.AlignCenter)  # Center the button

        tcs_button.clicked.connect(self.set_tcs_focus)
        scroll_area_layout.addLayout(tcs_button_layout)


        # Set scrollable widget layout
        scroll_area_widget.setLayout(scroll_area_layout)

        # Create scroll area and add the scrollable widget
        scroll_area = QScrollArea(self)
        scroll_area.setWidget(scroll_area_widget)
        scroll_area.setWidgetResizable(True)  # Allow the scroll area to resize with window

        # Add scroll area to main layout
        main_layout.addWidget(scroll_area)

        # Set the layout of the main window
        self.setLayout(main_layout)

        # Set window properties to ensure it maintains aspect ratio
        self.setMinimumSize(800, 600)  # Minimum window size (adjust as needed)
        self.setWindowTitle("Focus Tab")

    def run_command(self, command_list):
        """Helper function to run terminal command and handle errors"""
        try:
            result = subprocess.run(command_list, check=True, text=True, capture_output=True)
            print(f"Command output: {result.stdout}")
        except subprocess.CalledProcessError as e:
            print(f"Command failed with error: {e.stderr}")
    
    def activate_boi(self):
        channel = self.channel_input.text()
        skip_rows = self.skip_rows_input.text()
        rows = self.rows_input.text()

        if channel and skip_rows and rows:
            command = f"camera boi {channel} {skip_rows} {rows}"
            self.run_command(command.split())
        else:
            print("Please provide valid input for channel, rows to skip, and rows to read.")

    def activate_boi_full(self):
        full_channel = self.full_channel_input.text()

        if full_channel:
            command = f"camera boi {full_channel} full"
            self.run_command(command.split())
        else:
            print("Please provide a valid input for the channel.")

    def activate_bin(self):
        axis = self.axis_input.text()
        binfactor = self.binfactor_input.text()

        if axis and binfactor:
            command = f"camera bin {axis} {binfactor}"
            self.run_command(command.split())
        else:
            print("Please provide valid input for axis and bin factor.")

    def set_exptime(self):
        exptime = self.exptime_input.text()

        if exptime:
            command = f"camera exptime {exptime}"
            self.run_command(command.split())
        else:
            print("Please provide a valid input for exposure time.")

    def set_slit(self):
        width = self.slit_width_input.text()
        offset = self.slit_offset_input.text()

        if width and offset:
            command = f"slit set {width} {offset}"
            self.run_command(command.split())
        else:
            print("Please provide valid input for slit width and offset.")

    def camstep_focus(self):
        value = self.focus_value_input.text()
        upper = self.focus_upper_input.text()
        lower = self.focus_lower_input.text()
        step = self.focus_step_input.text()

        if value and upper and lower and step:
            command = f"camstep focus all focusloop {value} {upper} {lower} {step}"
            self.run_command(command.split())
        else:
            print("Please provide valid input for focus loop parameters.")

    def camstep_focus_acam(self):
        value = self.focus_value_input.text()
        upper = self.focus_upper_input.text()
        lower = self.focus_lower_input.text()
        step = self.focus_step_input.text()

        if value and upper and lower and step:
            command = f"camstep focus acam focusloop {value} {upper} {lower} {step}"
            self.run_command(command.split())
        else:
            print("Please provide valid input for ACAM focus loop parameters.")

    def set_tcs_focus(self):
        value = self.tcs_focus_value_input.text()

        if value:
            command = f"tcs setfocus {value}"
            self.run_command(command.split())
        else:
            print("Please provide a valid input for the TCS focus.")
