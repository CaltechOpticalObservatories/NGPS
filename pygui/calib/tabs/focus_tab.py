import datetime
from PyQt5.QtWidgets import QWidget, QVBoxLayout, QLabel, QPushButton, QLineEdit, QFormLayout, QFrame, QScrollArea, QSizePolicy, QHBoxLayout, QSpacerItem
from PyQt5.QtCore import Qt, pyqtSignal
import subprocess
from calib.tabs.async_command_thread import AsyncCommandThread

class FocusTab(QWidget):
    output_signal = pyqtSignal(str)
    def __init__(self, log_message_callback):
        super().__init__()
        self.log_message_callback = log_message_callback  # Set log_message callback from the parent
        self.initUI()


    def initUI(self):
        main_layout = QVBoxLayout()
        
        main_layout.setSpacing(10)  # Set spacing between sections to make it readable

        # Create a horizontal layout for the label to center it
        label_layout = QHBoxLayout()
        label_layout.setContentsMargins(0, 0, 0, 0)  # Remove outer margins for the label layout

        # Afternoon Tab Label
        label = QLabel("Focus", self)
        label.setSizePolicy(QSizePolicy.Minimum, QSizePolicy.Fixed)  # Keep label fixed in height
        label_layout.addWidget(label)

        main_layout.addLayout(label_layout)  # Add label layout to the main layout

        # Scroll Area Widget to make layout scrollable
        scroll_area_widget = QWidget()  # Create a widget that will be scrolled
        scroll_area_layout = QFormLayout()  # Use a QFormLayout to organize the sections
        
        scroll_area_layout.setContentsMargins(15, 15, 15, 15)  # Inner margins around form
        scroll_area_layout.setSpacing(10)  # Spacing between rows (increased for better readability)
        
        # Run Focus Button (Before Band of Interest Section)
        # self.run_focus_button = QPushButton("Run Focus", self)
        # self.run_focus_button.setStyleSheet("""
        #    QPushButton {
        #       background-color: #4CAF50;  /* Green color */
        #        color: white;
        #        border-radius: 8px;
        #        padding: 10px;
        #        border: none;
        #    }
        #    QPushButton:hover {
        #        background-color: #45a049;  /* Slightly darker green on hover */
        #    }
        #    QPushButton:pressed {
        #        background-color: #3e8e41;  /* Darker green when pressed */
        #    }
        # """)        
        # self.run_focus_button.clicked.connect(self.run_focus)
        # self.run_focus_button.setFixedHeight(45)
        # self.run_focus_button.setFixedWidth(300)
        # self.run_focus_button_layout = QHBoxLayout()
        # self.run_focus_button_layout.addWidget(self.run_focus_button)
        # self.run_focus_button_layout.setAlignment(self.run_focus_button, Qt.AlignCenter)  # Center the button
        # scroll_area_layout.addRow(self.run_focus_button_layout)

        # Run ACAM Focus Button (Before Band of Interest Section)
        self.run_acam_focus_button = QPushButton("Run ACAM Focus", self)
        self.run_acam_focus_button.setStyleSheet("""
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
        self.run_acam_focus_button.clicked.connect(self.run_focus_acam)
        self.run_acam_focus_button.setFixedHeight(45)
        self.run_acam_focus_button.setFixedWidth(325)
        self.run_acam_focus_button_layout = QHBoxLayout()
        self.run_acam_focus_button_layout.addWidget(self.run_acam_focus_button)
        self.run_acam_focus_button_layout.setAlignment(self.run_acam_focus_button, Qt.AlignCenter)  # Center the button
        scroll_area_layout.addRow(self.run_acam_focus_button_layout)


        # Camstep Focus Command
        scroll_area_layout.addRow(QLabel("camstep Focus"))

        self.focus_value_input = QLineEdit(self)
        self.focus_value_input.setPlaceholderText("1")
        self.focus_upper_input = QLineEdit(self)
        self.focus_upper_input.setPlaceholderText("Upper bound")
        self.focus_lower_input = QLineEdit(self)
        self.focus_lower_input.setPlaceholderText("Lower bound")
        self.focus_step_input = QLineEdit(self)
        self.focus_step_input.setPlaceholderText("0.2")
        
        camstep_button = QPushButton("camstep Focus (General)", self)
        camstep_button.clicked.connect(self.camstep_focus)
        camstep_button.setFixedHeight(45)
        camstep_button.setFixedWidth(325)  # Half-width button
        camstep_button_layout = QHBoxLayout()
        camstep_button_layout.addWidget(camstep_button)
        camstep_button_layout.setAlignment(camstep_button, Qt.AlignCenter)  # Right-align the button
        scroll_area_layout.addRow("Image Number:", self.focus_value_input)
        scroll_area_layout.addRow("Upper Bound:", self.focus_upper_input)
        scroll_area_layout.addRow("Lower Bound:", self.focus_lower_input)
        scroll_area_layout.addRow("Step:", self.focus_step_input)
        scroll_area_layout.addRow(camstep_button_layout)

        # Camstep Focus Button (ACAM)
        camstep_acam_button = QPushButton("camstep Focus (ACAM)", self)
        camstep_acam_button.clicked.connect(self.camstep_focus_acam)
        camstep_acam_button.setFixedHeight(45)
        camstep_acam_button.setFixedWidth(325)  # Half-width button
        camstep_acam_button_layout = QHBoxLayout()
        camstep_acam_button_layout.addWidget(camstep_acam_button)
        camstep_acam_button_layout.setAlignment(camstep_acam_button, Qt.AlignCenter)  # Right-align the button
        scroll_area_layout.addRow(camstep_acam_button_layout)
        
        # Band of Interest Section
        scroll_area_layout.addRow(QLabel("Band of Interest (R)"))
        
        # R Band Form Layout
        self.channel_r_input = QLineEdit(self)
        self.channel_r_input.setPlaceholderText("R")
        self.channel_r_input.setReadOnly(True)
        self.skip_rows_r_input = QLineEdit(self)
        self.skip_rows_r_input.setPlaceholderText("400")
        self.rows_r_input = QLineEdit(self)
        self.rows_r_input.setPlaceholderText("200")

        # Add R Band inputs and spacer
        spacer = QSpacerItem(20, 40, QSizePolicy.Minimum, QSizePolicy.Expanding)
        scroll_area_layout.addRow("Channel (R):", self.channel_r_input)
        scroll_area_layout.addRow("Skip Rows (R):", self.skip_rows_r_input)
        scroll_area_layout.addRow("Rows to Read (R):", self.rows_r_input)
        scroll_area_layout.addItem(spacer)

        # BOI Activation Button for R (right-aligned)
        boi_r_button = QPushButton("Activate BOI (R)", self)
        boi_r_button.clicked.connect(self.activate_boi_r)
        boi_r_button.setFixedHeight(45)
        boi_r_button.setFixedWidth(325)  # Set fixed width (adjust as needed)
        boi_r_button_layout = QHBoxLayout()
        boi_r_button_layout.addWidget(boi_r_button)
        boi_r_button_layout.setAlignment(boi_r_button, Qt.AlignCenter)  # Right-align the button
        scroll_area_layout.addRow(boi_r_button_layout)

        # Band of Interest Section for "I" band
        scroll_area_layout.addRow(QLabel("Band of Interest (I)"))

        # I Band Form Layout
        self.channel_i_input = QLineEdit(self)
        self.channel_i_input.setPlaceholderText("I")
        self.channel_i_input.setReadOnly(True)
        self.skip_rows_i_input = QLineEdit(self)
        self.skip_rows_i_input.setPlaceholderText("580")
        self.rows_i_input = QLineEdit(self)
        self.rows_i_input.setPlaceholderText("200")

        # Add I Band inputs and spacer
        spacer = QSpacerItem(20, 40, QSizePolicy.Minimum, QSizePolicy.Expanding)
        scroll_area_layout.addRow("Channel (I):", self.channel_i_input)
        scroll_area_layout.addRow("Skip Rows (I):", self.skip_rows_i_input)
        scroll_area_layout.addRow("Rows to Read (I):", self.rows_i_input)
        scroll_area_layout.addItem(spacer)

        # BOI Activation Button for I (right-aligned)
        boi_i_button = QPushButton("Activate BOI (I)", self)
        boi_i_button.clicked.connect(self.activate_boi_i)
        boi_i_button.setFixedHeight(45)
        boi_i_button.setFixedWidth(325)  # Set fixed width (adjust as needed)
        boi_i_button_layout = QHBoxLayout()
        boi_i_button_layout.addWidget(boi_i_button)
        boi_i_button_layout.setAlignment(boi_i_button, Qt.AlignCenter)  # Right-align the button
        scroll_area_layout.addRow(boi_i_button_layout)

        # Add white divider line
        divider = QFrame(self)
        divider.setFrameShape(QFrame.HLine)
        divider.setFrameShadow(QFrame.Sunken)
        divider.setStyleSheet('background-color: white;')  # Set divider color to white
        scroll_area_layout.addRow(divider)
        
        # Add white divider line
        divider2 = QFrame(self)
        divider2.setFrameShape(QFrame.HLine)
        divider2.setFrameShadow(QFrame.Sunken)
        divider2.setStyleSheet('background-color: white;')  # Set divider color to white

        # Add white divider line
        divider3 = QFrame(self)
        divider3.setFrameShape(QFrame.HLine)
        divider3.setFrameShadow(QFrame.Sunken)
        divider3.setStyleSheet('background-color: white;')  # Set divider color to white
        
        # Add white divider line
        divider4 = QFrame(self)
        divider4.setFrameShape(QFrame.HLine)
        divider4.setFrameShadow(QFrame.Sunken)
        divider4.setStyleSheet('background-color: white;')  # Set divider color to white

        # Camera and Focus Section
        scroll_area_layout.addRow(QLabel("Camera and Focus Commands"))
        
        # Camera Bin Command Form (Row Bin)
        self.axis_input_row = QLineEdit(self)
        self.axis_input_row.setPlaceholderText("row")
        self.axis_input_row.setReadOnly(True)
        self.binfactor_input_row = QLineEdit(self)
        self.binfactor_input_row.setPlaceholderText("4")

        row_bin_button = QPushButton("Activate Row Bin", self)
        row_bin_button.clicked.connect(self.activate_row_bin)
        row_bin_button.setFixedHeight(50)
        row_bin_button.setFixedWidth(325)  # Half-width button
        row_bin_button_layout = QHBoxLayout()
        row_bin_button_layout.addWidget(row_bin_button)
        row_bin_button_layout.setAlignment(row_bin_button, Qt.AlignCenter)  # Center the button

        # Add Row Bin Form and button to layout
        scroll_area_layout.addRow("Axis (Row Bin):", self.axis_input_row)
        scroll_area_layout.addRow("Row Bin Factor:", self.binfactor_input_row)
        scroll_area_layout.addRow(row_bin_button_layout)

        # Camera Bin Command Form (Col Bin)
        self.axis_input_col = QLineEdit(self)
        self.axis_input_col.setPlaceholderText("col")
        self.axis_input_col.setReadOnly(True)
        self.binfactor_input_col = QLineEdit(self)
        self.binfactor_input_col.setPlaceholderText("1")

        col_bin_button = QPushButton("Activate Col Bin", self)
        col_bin_button.clicked.connect(self.activate_col_bin)
        col_bin_button.setFixedHeight(45)
        col_bin_button.setFixedWidth(325)  # Half-width button
        col_bin_button_layout = QHBoxLayout()
        col_bin_button_layout.addWidget(col_bin_button)
        col_bin_button_layout.setAlignment(col_bin_button, Qt.AlignCenter)  # Center the button

        # Add Col Bin Form and button to layout
        scroll_area_layout.addRow("Axis (Col Bin):", self.axis_input_col)
        scroll_area_layout.addRow("Col Bin Factor:", self.binfactor_input_col)
        scroll_area_layout.addRow(col_bin_button_layout)

        scroll_area_layout.addRow(divider2)

        # Exposure Time Command
        self.exptime_input = QLineEdit(self)
        self.exptime_input.setPlaceholderText("10000")
        exptime_button = QPushButton("Set Camera Exposure Time", self)
        exptime_button.clicked.connect(self.set_exptime)
        exptime_button.setFixedHeight(45)
        exptime_button.setFixedWidth(325)  # Half-width button
        exptime_button_layout = QHBoxLayout()
        exptime_button_layout.addWidget(exptime_button)
        exptime_button_layout.setAlignment(exptime_button, Qt.AlignCenter)  # Right-align the button
        scroll_area_layout.addRow("Exposure Time (ms):", self.exptime_input)
        scroll_area_layout.addRow(exptime_button_layout)
        scroll_area_layout.addRow(divider3)

        # Slit Set Command
        self.slit_width_input = QLineEdit(self)
        self.slit_width_input.setPlaceholderText("5")
        self.slit_offset_input = QLineEdit(self)
        self.slit_offset_input.setPlaceholderText("3")
        slit_button = QPushButton("Set Slit", self)
        slit_button.clicked.connect(self.set_slit)
        slit_button.setFixedHeight(45)
        slit_button.setFixedWidth(325)  # Half-width button
        slit_button_layout = QHBoxLayout()
        slit_button_layout.addWidget(slit_button)
        slit_button_layout.setAlignment(slit_button, Qt.AlignCenter)  # Right-align the button
        scroll_area_layout.addRow("Slit Width:", self.slit_width_input)
        scroll_area_layout.addRow("Slit Offset:", self.slit_offset_input)
        scroll_area_layout.addRow(slit_button_layout)

        # TCS Set Focus Command
        self.tcs_focus_value_input = QLineEdit(self)
        self.tcs_focus_value_input.setPlaceholderText("Set TCS focus value")
        tcs_button = QPushButton("Set TCS Focus", self)
        tcs_button.clicked.connect(self.set_tcs_focus)
        tcs_button.setFixedHeight(45)
        tcs_button.setFixedWidth(325)  # Half-width button
        tcs_button_layout = QHBoxLayout()
        tcs_button_layout.addWidget(tcs_button)
        tcs_button_layout.setAlignment(tcs_button, Qt.AlignCenter)  # Right-align the button
        scroll_area_layout.addRow("TCS Focus Value:", self.tcs_focus_value_input)
        scroll_area_layout.addRow(tcs_button_layout)

        scroll_area_layout.addRow(divider4)

        # Band of Interest Section
        scroll_area_layout.addRow(QLabel("Full BOI"))
        # Full BOI Section
        full_boi_button = QPushButton("Activate Full BOI R Band", self)
        full_boi_button.clicked.connect(self.activate_boi_r_full)
        full_boi_button.setFixedHeight(45)
        full_boi_button.setFixedWidth(325)  # Half-width button
        full_boi_button_layout = QHBoxLayout()
        full_boi_button_layout.addWidget(full_boi_button)
        full_boi_button_layout.setAlignment(full_boi_button, Qt.AlignCenter) 
        scroll_area_layout.addRow(full_boi_button_layout)
        
        # Full BOI Section
        full_boi_i_button = QPushButton("Activate Full BOI I Band", self)
        full_boi_i_button.clicked.connect(self.activate_boi_i_full)
        full_boi_i_button.setFixedHeight(45)
        full_boi_i_button.setFixedWidth(325)  # Half-width button
        full_boi_i_button_layout = QHBoxLayout()
        full_boi_i_button_layout.addWidget(full_boi_i_button)
        full_boi_i_button_layout.setAlignment(full_boi_i_button, Qt.AlignCenter) 
        scroll_area_layout.addRow(full_boi_i_button_layout)
        
        # Add a button to run the focus_andor.py command
        run_focus_andor_button = QPushButton("Analyze Focus", self)     
        run_focus_andor_button.clicked.connect(self.run_focus_andor)
        run_focus_andor_button.setFixedHeight(45)
        run_focus_andor_button.setFixedWidth(325)
        run_focus_andor_button_layout = QHBoxLayout()
        run_focus_andor_button_layout.addWidget(run_focus_andor_button)
        run_focus_andor_button_layout.setAlignment(run_focus_andor_button, Qt.AlignCenter)  # Center the button
        scroll_area_layout.addRow(run_focus_andor_button_layout)

        # Add a button to spectrograph open images with eog
        open_images_button = QPushButton("Open Spectrograph Focus Images", self)
        open_images_button.setStyleSheet("""
            QPushButton {
                background-color: #007BFF;  /* Blue color */
                color: white;
                border-radius: 8px;
                padding: 10px;
                border: none;
            }
            QPushButton:hover {
                background-color: #0056b3;  /* Slightly darker blue on hover */
            }
            QPushButton:pressed {
                background-color: #004085;  /* Darker blue when pressed */
            }
        """)
        open_images_button.clicked.connect(self.open_focus_images)
        open_images_button.setFixedHeight(45)
        open_images_button.setFixedWidth(325)
        open_images_button_layout = QHBoxLayout()
        open_images_button_layout.addWidget(open_images_button)
        open_images_button_layout.setAlignment(open_images_button, Qt.AlignCenter)  # Center the button
        scroll_area_layout.addRow(open_images_button_layout)

        # Add a button to acam open images with eog
        acam_open_images_button = QPushButton("Open ACAM Focus Images", self)
        acam_open_images_button.setStyleSheet("""
            QPushButton {
                background-color: #007BFF;  /* Blue color */
                color: white;
                border-radius: 8px;
                padding: 10px;
                border: none;
            }
            QPushButton:hover {
                background-color: #0056b3;  /* Slightly darker blue on hover */
            }
            QPushButton:pressed {
                background-color: #004085;  /* Darker blue when pressed */
            }
        """)
        acam_open_images_button.clicked.connect(self.open_focus_acam_images)
        acam_open_images_button.setFixedHeight(45)
        acam_open_images_button.setFixedWidth(325)
        acam_open_images_layout = QHBoxLayout()
        acam_open_images_layout.addWidget(acam_open_images_button)
        acam_open_images_layout.setAlignment(acam_open_images_button, Qt.AlignCenter)  # Center the button
        scroll_area_layout.addRow(acam_open_images_layout)


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

        # Final stretch to force scrolling if content exceeds visible area
        scroll_area_widget.setSizePolicy(QSizePolicy.Minimum, QSizePolicy.Expanding)
    
    def activate_boi_r(self):
        # Get the user input or use the placeholder (default values)
        channel = self.channel_r_input.text() or self.channel_r_input.placeholderText()
        skip_rows = self.skip_rows_r_input.text() or self.skip_rows_r_input.placeholderText()
        rows = self.rows_r_input.text() or self.rows_r_input.placeholderText()

        if channel and skip_rows and rows:
            command = f"camera boi {channel} {skip_rows} {rows}"
            self.run_command_in_background(command)
        else:
            print("Please provide valid input for channel, rows to skip, and rows to read.")

    def activate_boi_i(self):
        # Use placeholder text if no input provided
        channel = self.channel_i_input.text() or self.channel_i_input.placeholderText()
        skip_rows = self.skip_rows_i_input.text() or self.skip_rows_i_input.placeholderText()
        rows = self.rows_i_input.text() or self.rows_i_input.placeholderText()

        if channel and skip_rows and rows:
            command = f"camera boi {channel} {skip_rows} {rows}"
            self.run_command_in_background(command)
        else:
            print("Please provide valid input for channel, rows to skip, and rows to read.")

    def activate_boi_r_full(self):
        command_r = f"camera boi R full"
        self.run_command_in_background(command_r)

    def activate_boi_i_full(self):
        command_i = f"camera boi I full"
        self.run_command_in_background(command_i)

    def activate_row_bin(self):
        # Use placeholder text if no input provided
        axis = self.axis_input_row.text() or self.axis_input_row.placeholderText()
        binfactor = self.binfactor_input_row.text() or self.binfactor_input_row.placeholderText()

        if axis and binfactor:
            command = f"camera bin {axis} {binfactor}"
            self.run_command_in_background(command)
        else:
            print("Please provide valid input for axis and bin factor.")

    def activate_col_bin(self):
        # Use placeholder text if no input provided
        axis = self.axis_input_col.text() or self.axis_input_col.placeholderText()
        binfactor = self.binfactor_input_col.text() or self.binfactor_input_col.placeholderText()

        if axis and binfactor:
            command = f"camera bin {axis} {binfactor}"
            self.run_command_in_background(command)
        else:
            print("Please provide valid input for axis and bin factor.")

    def set_exptime(self):
        # Use placeholder text if no input provided
        exptime = self.exptime_input.text() or self.exptime_input.placeholderText()

        if exptime:
            command = f"camera exptime {exptime}"
            self.run_command_in_background(command)
        else:
            print("Please provide a valid input for exposure time.")

    def set_slit(self):
        # Use placeholder text if no input provided
        width = self.slit_width_input.text() or self.slit_width_input.placeholderText()
        offset = self.slit_offset_input.text() or self.slit_offset_input.placeholderText()

        if width and offset:
            command = f"slit set {width} {offset}"
            self.run_command_in_background(command)
        else:
            print("Please provide valid input for slit width and offset.")

    def camstep_focus(self):
        # Use placeholder text if no input provided
        value = self.focus_value_input.text() or self.focus_value_input.placeholderText()
        upper = self.focus_upper_input.text() or self.focus_upper_input.placeholderText()
        lower = self.focus_lower_input.text() or self.focus_lower_input.placeholderText()
        step = self.focus_step_input.text() or self.focus_step_input.placeholderText()

        if value and upper and lower and step:
            timestamp = datetime.datetime.now().strftime("%Y%m%d-%H%M%S")
            label = f"focusloop_{timestamp}"
            command = f"camstep focus all {label} {value} {upper} {lower} {step}"
            self.run_command_in_background(command)
        else:
            print("Please provide valid input for focus loop parameters.")

    def camstep_focus_acam(self):
        # Use placeholder text if no input provided
        value = self.focus_value_input.text() or self.focus_value_input.placeholderText()
        upper = self.focus_upper_input.text() or self.focus_upper_input.placeholderText()
        lower = self.focus_lower_input.text() or self.focus_lower_input.placeholderText()
        step = self.focus_step_input.text() or self.focus_step_input.placeholderText()

        if value and upper and lower and step:
            timestamp = datetime.datetime.now().strftime("%Y%m%d-%H%M%S")
            label = f"focusloop_{timestamp}"
            command = f"camstep focus acam {label} {value} {upper} {lower} {step}"
            self.run_command_in_background(command)
        else:
            print("Please provide valid input for ACAM focus loop parameters.")

    def set_tcs_focus(self):
        # Use placeholder text if no input provided
        value = self.tcs_focus_value_input.text() or self.tcs_focus_value_input.placeholderText()

        if value:
            command = f"tcs setfocus {value}"
            self.run_command_in_background(command)
        else:
            print("Please provide a valid input for the TCS focus.")

    def set_basename(self, basename="ngps"):
        """Set the basename."""
        if basename:
            command = f"camera basename {basename}"
            self.run_command_in_background(command)

    def run_focus_andor(self):
        """Run the focus_andor.py script with specified arguments."""
        command = "bash calib/andor.sh"
        self.run_command_in_background(command)

    def open_focus_images(self):
        """Run the exact eog command to open images."""
        command = "eog /home/observer/focus/focus_spec_I.png /home/observer/focus/focus_spec_R.png"
        
        try:
            # Run the command exactly as it is
            subprocess.run(command, shell=True, check=True)
            print("Command executed successfully.")
        except subprocess.CalledProcessError as e:
            print(f"Error occurred: {e.stderr}")

    def open_focus_acam_images(self):
        """Run the exact eog command to open images."""
        command = "eog /home/observer/focus/focus_andor_FWHM_acam.png"

        try:
            # Run the command exactly as it is
            subprocess.run(command, shell=True, check=True)
            print("Command executed successfully.")
        except subprocess.CalledProcessError as e:
            print(f"Error occurred: {e.stderr}")

    # Event handler methods for R and I bands
    def run_focus(self):
        # This method will run all the other buttons when clicked
        print("Running Focus...")


        self.run_focus_button.setEnabled(False)
        self.run_focus_button.setStyleSheet("""
                 QPushButton {
                     background-color: lightgray;
                 }
            """)        
 
        # Set basename
        command = f"camera basename focus"
        self.run_command(command)
        # Activate BOI R
        channel = self.channel_r_input.text() or self.channel_r_input.placeholderText()
        skip_rows = self.skip_rows_r_input.text() or self.skip_rows_r_input.placeholderText()
        rows = self.rows_r_input.text() or self.rows_r_input.placeholderText()
 
        if channel and skip_rows and rows:
            command = f"camera boi {channel} {skip_rows} {rows}"
            self.run_command(command)
        else:
            print("Please provide valid input for channel, rows to skip, and rows to read.")
 
        # Activate BOI I
        channel = self.channel_i_input.text() or self.channel_i_input.placeholderText()
        skip_rows = self.skip_rows_i_input.text() or self.skip_rows_i_input.placeholderText()
        rows = self.rows_i_input.text() or self.rows_i_input.placeholderText()
 
        if channel and skip_rows and rows:
            command = f"camera boi {channel} {skip_rows} {rows}"
            self.run_command(command)
 
        # Activate Row BIN
        axis = self.axis_input_row.text() or self.axis_input_row.placeholderText()
        binfactor = self.binfactor_input_row.text() or self.binfactor_input_row.placeholderText()
 
        if axis and binfactor:
            command = f"camera bin {axis} {binfactor}"
            self.run_command(command)
        else:
            print("Please provide valid input for axis and bin factor.")
 
        # Activate COL BIN
        axis = self.axis_input_col.text() or self.axis_input_col.placeholderText()
        binfactor = self.binfactor_input_col.text() or self.binfactor_input_col.placeholderText()
 
        if axis and binfactor:
            command = f"camera bin {axis} {binfactor}"
            self.run_command(command)
        else:
            print("Please provide valid input for axis and bin factor.")
 
        # Set EXPTime
        exptime = self.exptime_input.text() or self.exptime_input.placeholderText()
 
        if exptime:
            command = f"camera exptime {exptime}"
            self.run_command(command)
        else:
            print("Please provide a valid input for exposure time.")
            
        # Set slitwidth
        width = self.slit_width_input.text() or self.slit_width_input.placeholderText()
        offset = self.slit_offset_input.text() or self.slit_offset_input.placeholderText()
 
        if width and offset:
            command = f"slit set {width} {offset}"
            self.run_command(command)
        else:
            print("Please provide valid input for slit width and offset.")
 
 
        # Set CAM focus CAMSTEP
        value = self.focus_value_input.text() or self.focus_value_input.placeholderText()
        upper = self.focus_upper_input.text() or self.focus_upper_input.placeholderText()
        lower = self.focus_lower_input.text() or self.focus_lower_input.placeholderText()
        step = self.focus_step_input.text() or self.focus_step_input.placeholderText()

        if value and upper and lower and step:
            command = f"camstep focus all focusloop {value} {upper} {lower} {step}"
            self.run_command(command)
        else:
            print("Please provide valid input for focus loop parameters.")
 
 
        # Set CAM focus ACAM
        value = self.focus_value_input.text() or self.focus_value_input.placeholderText()
        upper = self.focus_upper_input.text() or self.focus_upper_input.placeholderText()
        lower = self.focus_lower_input.text() or self.focus_lower_input.placeholderText()
        step = self.focus_step_input.text() or self.focus_step_input.placeholderText()
 
        if value and upper and lower and step:
            command = f"camstep focus acam focusloop {value} {upper} {lower} {step}"
            self.run_command(command)
        else:
            print("Please provide valid input for ACAM focus loop parameters.")


        command = f"camera basename ngps"
        self.run_command(command)
         
        command_r = f"camera boi R full"
        self.run_command(command_r)
 
        command_i = f"camera boi I full"
        self.run_command(command_i)
         
        # Run the focus_andor.py script with specified arguments. 
        command = "bash calib/andor.sh"
        self.run_command(command)
        
        self.open_focus_images()
     
        self.run_focus_button.setEnabled(True)
        self.run_focus_button.setStyleSheet("""
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

    def run_focus_acam(self):
        """Run the full ACAM focus sequence in the background (like thrufocus)."""

        # Disable + gray the button while running
        self.run_acam_focus_button.setEnabled(False)
        self.run_acam_focus_button.setStyleSheet("""
            QPushButton {
                background-color: lightgray;
                color: black;
                border-radius: 8px;
                padding: 10px;
                border: none;
            }
        """)

        self.log_message_callback("Running ACAM focus sequence...\n")

        # Read parameters (use placeholder if field is empty)
        value = self.focus_value_input.text() or self.focus_value_input.placeholderText()
        upper = self.focus_upper_input.text() or self.focus_upper_input.placeholderText()
        lower = self.focus_lower_input.text() or self.focus_lower_input.placeholderText()
        step  = self.focus_step_input.text()  or self.focus_step_input.placeholderText()

        if not (value and upper and lower and step):
            self.log_message_callback("Please provide valid input for ACAM focus loop parameters.\n")
            # restore button immediately since we didn't start anything
            self.run_acam_focus_button.setEnabled(True)
            self.run_acam_focus_button.setStyleSheet("""
                QPushButton {
                    background-color: #4CAF50;  /* Green */
                    color: white;
                    border-radius: 8px;
                    padding: 10px;
                    border: none;
                }
                QPushButton:hover   { background-color: #45a049; }
                QPushButton:pressed { background-color: #3e8e41; }
            """)
            return

        # Build label and combined shell command (all steps in one go)
        timestamp = datetime.datetime.now().strftime("%Y%m%d-%H%M%S")
        label = f"focusloop_{timestamp}"

        command = (
            f"camera basename focus && "
            f"camstep focus acam {label} {value} {upper} {lower} {step} && "
            f"camera basename ngps && "
            f"bash calib/andor.sh {label}"
        )

        # Start async command, logging output to the calibration GUI
        self.thread_acam_focus = AsyncCommandThread(command, self.log_message_callback)
        self.thread_acam_focus.output_signal.connect(self.log_message_callback)

        # Restore UI + open image when the background task ends
        def _restore_acam_focus():
            self.run_acam_focus_button.setEnabled(True)
            self.run_acam_focus_button.setStyleSheet("""
                QPushButton {
                    background-color: #4CAF50;  /* Green */
                    color: white;
                    border-radius: 8px;
                    padding: 10px;
                    border: none;
                }
                QPushButton:hover   { background-color: #45a049; }
                QPushButton:pressed { background-color: #3e8e41; }
            """)
            # Show the resulting ACAM focus plot (still uses eog)
            self.open_focus_acam_images()

        self.thread_acam_focus.finished.connect(_restore_acam_focus)
        try:
            # If AsyncCommandThread also emits terminated, hook that too (like afternoon_tab)
            self.thread_acam_focus.terminated.connect(_restore_acam_focus)
        except Exception:
            pass

        self.thread_acam_focus.start()


    def run_command_in_background(self, command):
        """Run the command in a background thread."""
        self.thread = AsyncCommandThread(command, self.log_message_callback)
        self.thread.output_signal.connect(self.log_message_callback)
        self.thread.start()

    def run_command(self, command):
        """Run a command synchronously, sending all output to the GUI log."""
        # Make sure the signal is connected once
        try:
            self.output_signal.disconnect()
        except TypeError:
            # Wasn't connected yet – that's fine
            pass

        self.output_signal.connect(self.log_message_callback)

        try:
            self.output_signal.emit(f"$ {command}\n")

            result = subprocess.run(
                command,
                shell=True,
                check=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
            )

            if result.stdout:
                self.output_signal.emit(result.stdout)
            if result.stderr:
                self.output_signal.emit(result.stderr)

            self.output_signal.emit("Command executed successfully.\n")

        except subprocess.CalledProcessError as e:
            msg = ""
            if e.stdout:
                msg += e.stdout
            if e.stderr:
                msg += e.stderr
            if not msg:
                msg = f"Error occurred while running: {command}\n"
            self.output_signal.emit(msg)

