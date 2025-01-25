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
        scroll_area_layout = QFormLayout()  # Use a QFormLayout to organize the sections
        
        # Band of Interest Section
        scroll_area_layout.addRow(QLabel("Turn on Band of Interest"))
        
        # Band of Interest Form Layout
        self.channel_input = QLineEdit(self)
        self.channel_input.setPlaceholderText("R")
        self.skip_rows_input = QLineEdit(self)
        self.skip_rows_input.setPlaceholderText("400")
        self.rows_input = QLineEdit(self)
        self.rows_input.setPlaceholderText("200")

        # Add BOI inputs and spacer
        spacer = QSpacerItem(20, 40, QSizePolicy.Minimum, QSizePolicy.Expanding)
        scroll_area_layout.addRow("Channel:", self.channel_input)
        scroll_area_layout.addRow("Skip Rows:", self.skip_rows_input)
        scroll_area_layout.addRow("Rows to Read:", self.rows_input)
        scroll_area_layout.addItem(spacer)

        # BOI Activation Button
        boi_button = QPushButton("Activate BOI", self)
        boi_button.clicked.connect(self.activate_boi)
        scroll_area_layout.addRow(boi_button)

        # Full BOI Section
        self.full_channel_input = QLineEdit(self)
        self.full_channel_input.setPlaceholderText("Enter channel")
        full_boi_button = QPushButton("Activate BOI (Full)", self)
        full_boi_button.clicked.connect(self.activate_boi_full)
        scroll_area_layout.addRow("Enter channel for full BOI:", self.full_channel_input)
        scroll_area_layout.addRow(full_boi_button)

        # Divider line
        divider = QFrame(self)
        divider.setFrameShape(QFrame.HLine)
        divider.setFrameShadow(QFrame.Sunken)
        scroll_area_layout.addWidget(divider)

        # Camera and Focus Section
        scroll_area_layout.addRow(QLabel("Camera and Focus Commands"))
        
        # Camera Bin Command Form
        self.axis_input = QLineEdit(self)
        self.axis_input.setPlaceholderText("Enter axis (for camera bin)")
        self.binfactor_input = QLineEdit(self)
        self.binfactor_input.setPlaceholderText("Bin factor")
        bin_button = QPushButton("Activate Camera Bin", self)
        bin_button.clicked.connect(self.activate_bin)
        
        scroll_area_layout.addRow("Axis:", self.axis_input)
        scroll_area_layout.addRow("Bin Factor:", self.binfactor_input)
        scroll_area_layout.addRow(bin_button)

        # Exposure Time Command
        self.exptime_input = QLineEdit(self)
        self.exptime_input.setPlaceholderText("10000")
        exptime_button = QPushButton("Set Camera Exposure Time", self)
        exptime_button.clicked.connect(self.set_exptime)
        scroll_area_layout.addRow("Exposure Time (ms):", self.exptime_input)
        scroll_area_layout.addRow(exptime_button)

        # Slit Set Command
        self.slit_width_input = QLineEdit(self)
        self.slit_width_input.setPlaceholderText("5")
        self.slit_offset_input = QLineEdit(self)
        self.slit_offset_input.setPlaceholderText("3")
        slit_button = QPushButton("Set Slit", self)
        slit_button.clicked.connect(self.set_slit)

        scroll_area_layout.addRow("Slit Width:", self.slit_width_input)
        scroll_area_layout.addRow("Slit Offset:", self.slit_offset_input)
        scroll_area_layout.addRow(slit_button)

        # Camstep Focus Command
        self.focus_value_input = QLineEdit(self)
        self.focus_value_input.setPlaceholderText("Image Number")
        self.focus_upper_input = QLineEdit(self)
        self.focus_upper_input.setPlaceholderText("Upper bound")
        self.focus_lower_input = QLineEdit(self)
        self.focus_lower_input.setPlaceholderText("Lower bound")
        self.focus_step_input = QLineEdit(self)
        self.focus_step_input.setPlaceholderText("Step")
        
        camstep_button = QPushButton("Camstep Focus (General)", self)
        camstep_button.clicked.connect(self.camstep_focus)
        
        scroll_area_layout.addRow("Image Number:", self.focus_value_input)
        scroll_area_layout.addRow("Upper Bound:", self.focus_upper_input)
        scroll_area_layout.addRow("Lower Bound:", self.focus_lower_input)
        scroll_area_layout.addRow("Step:", self.focus_step_input)
        scroll_area_layout.addRow(camstep_button)

        # Camstep Focus Button (ACAM)
        camstep_acam_button = QPushButton("Camstep Focus (ACAM)", self)
        camstep_acam_button.clicked.connect(self.camstep_focus_acam)
        scroll_area_layout.addRow(camstep_acam_button)

        # TCS Set Focus Command
        self.tcs_focus_value_input = QLineEdit(self)
        self.tcs_focus_value_input.setPlaceholderText("Set TCS focus value")
        tcs_button = QPushButton("Set TCS Focus", self)
        tcs_button.clicked.connect(self.set_tcs_focus)
        scroll_area_layout.addRow("TCS Focus Value:", self.tcs_focus_value_input)
        scroll_area_layout.addRow(tcs_button)

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
