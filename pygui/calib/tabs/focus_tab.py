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
        scroll_area_widget = QWidget()
        scroll_area_layout = QVBoxLayout()

        # Add Band of Interest Section
        self.add_boi_section(scroll_area_layout)

        # Divider (horizontal line) to separate sections
        self.add_divider(scroll_area_layout)

        # Add Camera and Focus Section
        self.add_camera_focus_section(scroll_area_layout)

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

    def add_boi_section(self, layout):
        # Turn on Band of Interest Section
        layout.addWidget(QLabel("Turn on Band of Interest", self))

        # Form Layout for Parameterized BOI
        boi_form_layout = QFormLayout()

        self.channel_input = self.create_input_field("Enter channel (for parameterized BOI)")
        boi_form_layout.addRow("Channel:", self.channel_input)

        self.skip_rows_input = self.create_input_field("Rows to skip")
        boi_form_layout.addRow("Skip Rows:", self.skip_rows_input)

        self.rows_input = self.create_input_field("Rows to read")
        boi_form_layout.addRow("Rows to Read:", self.rows_input)

        layout.addLayout(boi_form_layout)

        # BOI Activate Button (Centered)
        boi_button = self.create_centered_button("Activate BOI", self.activate_boi)
        layout.addLayout(boi_button)

        # Full BOI Section
        self.full_channel_input = self.create_input_field("Enter channel for full BOI:")
        layout.addWidget(QLabel("Enter channel for full BOI:"))
        layout.addWidget(self.full_channel_input)

        # Full BOI Button (Centered)
        boi_full_button = self.create_centered_button("Activate BOI (Full)", self.activate_boi_full)
        layout.addLayout(boi_full_button)

    def add_divider(self, layout):
        divider = QFrame(self)
        divider.setFrameShape(QFrame.HLine)
        divider.setFrameShadow(QFrame.Sunken)
        layout.addWidget(divider)

    def add_camera_focus_section(self, layout):
        # Camera and Focus Commands
        layout.addWidget(QLabel("Camera and Focus Commands", self))

        # Camera Bin Command Inputs
        self.add_input_fields_to_layout(layout, [("Axis", "Enter axis (for camera bin)"), 
                                                  ("Bin Factor", "Bin factor")])

        # Camera Bin Button
        bin_button = self.create_centered_button("Activate Camera Bin", self.activate_bin)
        layout.addLayout(bin_button)

        # Exposure Time Command
        self.add_input_fields_to_layout(layout, [("Exposure Time (ms)", "Exposure time (in msec)")])

        # Exposure Time Button
        exptime_button = self.create_centered_button("Set Camera Exposure Time", self.set_exptime)
        layout.addLayout(exptime_button)

        # Slit Command
        self.add_input_fields_to_layout(layout, [("Slit Width", "Slit width"), 
                                                  ("Slit Offset", "Slit offset")])

        # Slit Button
        slit_button = self.create_centered_button("Set Slit", self.set_slit)
        layout.addLayout(slit_button)

        # Camstep Focus Command Inputs (General)
        self.add_input_fields_to_layout(layout, [("Focus Value", "Focus loop value"),
                                                  ("Upper Bound", "Upper bound"),
                                                  ("Lower Bound", "Lower bound"),
                                                  ("Focus Step", "Focus step")])

        # Camstep Focus Button (General)
        camstep_button = self.create_centered_button("Camstep Focus (General)", self.camstep_focus)
        layout.addLayout(camstep_button)

        # Camstep Focus Button (ACAM)
        camstep_acam_button = self.create_centered_button("Camstep Focus (ACAM)", self.camstep_focus_acam)
        layout.addLayout(camstep_acam_button)

        # TCS Set Focus Command Inputs
        self.add_input_fields_to_layout(layout, [("TCS Focus Value", "Set TCS focus value")])

        # TCS Set Focus Button
        tcs_button = self.create_centered_button("Set TCS Focus", self.set_tcs_focus)
        layout.addLayout(tcs_button)

    def create_input_field(self, placeholder):
        input_field = QLineEdit(self)
        input_field.setPlaceholderText(placeholder)
        input_field.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        input_field.setMinimumWidth(150)  # Minimum width for the input
        input_field.setMinimumHeight(35)  # Minimum height for the input
        return input_field

    def add_input_fields_to_layout(self, layout, field_data):
        form_layout = QFormLayout()
        for label, placeholder in field_data:
            input_field = self.create_input_field(placeholder)
            form_layout.addRow(label + ":", input_field)
        layout.addLayout(form_layout)

    def create_centered_button(self, text, action):
        button = QPushButton(text, self)
        button.setFixedWidth(200)  # Set a fixed width for the button
        button_layout = QHBoxLayout()
        button_layout.addWidget(button)
        button_layout.setAlignment(button, Qt.AlignCenter)  # Center the button
        button.clicked.connect(action)
        return button_layout

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
