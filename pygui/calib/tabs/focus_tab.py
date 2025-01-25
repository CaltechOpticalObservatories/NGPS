from PyQt5.QtWidgets import QWidget, QVBoxLayout, QLabel, QPushButton, QLineEdit, QFormLayout, QFrame, QScrollArea, QSizePolicy, QHBoxLayout, QSpacerItem
from PyQt5.QtCore import Qt
import subprocess
from PyQt5.QtCore import QThread, pyqtSignal

# Worker thread to run the command
class CommandThread(QThread):
    # Signal to pass the result back to the main thread
    result_signal = pyqtSignal(str)

    def __init__(self, command_list):
        super().__init__()
        self.command_list = command_list

    def run(self):
        try:
            # Running the command in subprocess
            result = subprocess.run(self.command_list, check=True, text=True, capture_output=True)
            # Emit the result to update the UI
            self.result_signal.emit(result.stdout)
        except subprocess.CalledProcessError as e:
            self.result_signal.emit(f"Error: {e.stderr}")


class FocusTab(QWidget):
    def __init__(self):
        super().__init__()
        self.initUI()

    def initUI(self):
        main_layout = QVBoxLayout()

        # Scroll Area Widget to make layout scrollable
        scroll_area_widget = QWidget()  # Create a widget that will be scrolled
        scroll_area_layout = QFormLayout()  # Use a QFormLayout to organize the sections
        
        scroll_area_layout.setContentsMargins(15, 15, 15, 15)  # Inner margins around form
        scroll_area_layout.setSpacing(10)  # Spacing between rows (increased for better readability)
        
        # Run Focus Button (Before Band of Interest Section)
        run_focus_button = QPushButton("Run Focus", self)
        run_focus_button.setStyleSheet("""
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
        run_focus_button.clicked.connect(self.run_focus)
        run_focus_button.setFixedHeight(45)
        run_focus_button.setFixedWidth(300)
        run_focus_button_layout = QHBoxLayout()
        run_focus_button_layout.addWidget(run_focus_button)
        run_focus_button_layout.setAlignment(run_focus_button, Qt.AlignCenter)  # Center the button
        scroll_area_layout.addRow(run_focus_button_layout)
        
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
        boi_r_button.setFixedWidth(300)  # Set fixed width (adjust as needed)
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
        boi_i_button.setFixedWidth(300)  # Set fixed width (adjust as needed)
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
        row_bin_button.setFixedWidth(300)  # Half-width button
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
        col_bin_button.setFixedWidth(300)  # Half-width button
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
        exptime_button.setFixedWidth(300)  # Half-width button
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
        slit_button.setFixedWidth(300)  # Half-width button
        slit_button_layout = QHBoxLayout()
        slit_button_layout.addWidget(slit_button)
        slit_button_layout.setAlignment(slit_button, Qt.AlignCenter)  # Right-align the button
        scroll_area_layout.addRow("Slit Width:", self.slit_width_input)
        scroll_area_layout.addRow("Slit Offset:", self.slit_offset_input)
        scroll_area_layout.addRow(slit_button_layout)

        # Camstep Focus Command
        self.focus_value_input = QLineEdit(self)
        self.focus_value_input.setPlaceholderText("1")
        self.focus_upper_input = QLineEdit(self)
        self.focus_upper_input.setPlaceholderText("Upper bound")
        self.focus_lower_input = QLineEdit(self)
        self.focus_lower_input.setPlaceholderText("Lower bound")
        self.focus_step_input = QLineEdit(self)
        self.focus_step_input.setPlaceholderText(".2")
        
        camstep_button = QPushButton("Camstep Focus (General)", self)
        camstep_button.clicked.connect(self.camstep_focus)
        camstep_button.setFixedHeight(45)
        camstep_button.setFixedWidth(300)  # Half-width button
        camstep_button_layout = QHBoxLayout()
        camstep_button_layout.addWidget(camstep_button)
        camstep_button_layout.setAlignment(camstep_button, Qt.AlignCenter)  # Right-align the button
        scroll_area_layout.addRow("Image Number:", self.focus_value_input)
        scroll_area_layout.addRow("Upper Bound:", self.focus_upper_input)
        scroll_area_layout.addRow("Lower Bound:", self.focus_lower_input)
        scroll_area_layout.addRow("Step:", self.focus_step_input)
        scroll_area_layout.addRow(camstep_button_layout)

        # Camstep Focus Button (ACAM)
        camstep_acam_button = QPushButton("Camstep Focus (ACAM)", self)
        camstep_acam_button.clicked.connect(self.camstep_focus_acam)
        camstep_acam_button.setFixedHeight(45)
        camstep_acam_button.setFixedWidth(300)  # Half-width button
        camstep_acam_button_layout = QHBoxLayout()
        camstep_acam_button_layout.addWidget(camstep_acam_button)
        camstep_acam_button_layout.setAlignment(camstep_acam_button, Qt.AlignCenter)  # Right-align the button
        scroll_area_layout.addRow(camstep_acam_button_layout)

        # TCS Set Focus Command
        self.tcs_focus_value_input = QLineEdit(self)
        self.tcs_focus_value_input.setPlaceholderText("Set TCS focus value")
        tcs_button = QPushButton("Set TCS Focus", self)
        tcs_button.clicked.connect(self.set_tcs_focus)
        tcs_button.setFixedHeight(45)
        tcs_button.setFixedWidth(300)  # Half-width button
        tcs_button_layout = QHBoxLayout()
        tcs_button_layout.addWidget(tcs_button)
        tcs_button_layout.setAlignment(tcs_button, Qt.AlignCenter)  # Right-align the button
        scroll_area_layout.addRow("TCS Focus Value:", self.tcs_focus_value_input)
        scroll_area_layout.addRow(tcs_button_layout)

        scroll_area_layout.addRow(divider4)

        # Band of Interest Section
        scroll_area_layout.addRow(QLabel("Full BOI"))
        # Full BOI Section
        full_boi_button = QPushButton("Activate Full BOI", self)
        full_boi_button.clicked.connect(self.activate_boi_full)
        full_boi_button.setFixedHeight(45)
        full_boi_button.setFixedWidth(300)  # Half-width button
        full_boi_button_layout = QHBoxLayout()
        full_boi_button_layout.addWidget(full_boi_button)
        full_boi_button_layout.setAlignment(full_boi_button, Qt.AlignCenter) 
        scroll_area_layout.addRow(full_boi_button_layout)

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
        """Run the command in a separate thread to prevent blocking the UI."""
        self.worker = CommandThread(command_list)
        # Connect the result signal to handle the command output
        self.worker.result_signal.connect(self.handle_command_result)
        # Start the worker thread
        self.worker.start()
    
    def activate_boi_r(self):
        # Get the user input or use the placeholder (default values)
        channel = self.channel_r_input.text() or self.channel_r_input.placeholderText()
        skip_rows = self.skip_rows_r_input.text() or self.skip_rows_r_input.placeholderText()
        rows = self.rows_r_input.text() or self.rows_r_input.placeholderText()

        if channel and skip_rows and rows:
            command = f"camera boi {channel} {skip_rows} {rows}"
            self.run_command(command.split())
        else:
            print("Please provide valid input for channel, rows to skip, and rows to read.")

    def activate_boi_r(self):
        # Use placeholder text if no input provided
        channel = self.channel_r_input.text() or self.channel_r_input.placeholderText()
        skip_rows = self.skip_rows_r_input.text() or self.skip_rows_r_input.placeholderText()
        rows = self.rows_r_input.text() or self.rows_r_input.placeholderText()

        if channel and skip_rows and rows:
            command = f"camera boi {channel} {skip_rows} {rows}"
            self.run_command(command.split())
        else:
            print("Please provide valid input for channel, rows to skip, and rows to read.")

    def activate_boi_i(self):
        # Use placeholder text if no input provided
        channel = self.channel_i_input.text() or self.channel_i_input.placeholderText()
        skip_rows = self.skip_rows_i_input.text() or self.skip_rows_i_input.placeholderText()
        rows = self.rows_i_input.text() or self.rows_i_input.placeholderText()

        if channel and skip_rows and rows:
            command = f"camera boi {channel} {skip_rows} {rows}"
            self.run_command(command.split())
        else:
            print("Please provide valid input for channel, rows to skip, and rows to read.")

    def activate_boi_full(self):
        command_r = f"camera boi R full"
        command_i = f"camera boi I full"
        self.run_command(command_r.split())
        self.run_command(command_i.split())

    def activate_row_bin(self):
        # Use placeholder text if no input provided
        axis = self.axis_input_row.text() or self.axis_input_row.placeholderText()
        binfactor = self.binfactor_input_row.text() or self.binfactor_input_row.placeholderText()

        if axis and binfactor:
            command = f"camera bin {axis} {binfactor}"
            self.run_command(command.split())
        else:
            print("Please provide valid input for axis and bin factor.")

    def activate_col_bin(self):
        # Use placeholder text if no input provided
        axis = self.axis_input_col.text() or self.axis_input_col.placeholderText()
        binfactor = self.binfactor_input_col.text() or self.binfactor_input_col.placeholderText()

        if axis and binfactor:
            command = f"camera bin {axis} {binfactor}"
            self.run_command(command.split())
        else:
            print("Please provide valid input for axis and bin factor.")

    def set_exptime(self):
        # Use placeholder text if no input provided
        exptime = self.exptime_input.text() or self.exptime_input.placeholderText()

        if exptime:
            command = f"camera exptime {exptime}"
            self.run_command(command.split())
        else:
            print("Please provide a valid input for exposure time.")

    def set_slit(self):
        # Use placeholder text if no input provided
        width = self.slit_width_input.text() or self.slit_width_input.placeholderText()
        offset = self.slit_offset_input.text() or self.slit_offset_input.placeholderText()

        if width and offset:
            command = f"slit set {width} {offset}"
            self.run_command(command.split())
        else:
            print("Please provide valid input for slit width and offset.")

    def camstep_focus(self):
        # Use placeholder text if no input provided
        value = self.focus_value_input.text() or self.focus_value_input.placeholderText()
        upper = self.focus_upper_input.text() or self.focus_upper_input.placeholderText()
        lower = self.focus_lower_input.text() or self.focus_lower_input.placeholderText()
        step = self.focus_step_input.text() or self.focus_step_input.placeholderText()

        if value and upper and lower and step:
            command = f"camstep focus all focusloop {value} {upper} {lower} {step}"
            self.run_command(command.split())
        else:
            print("Please provide valid input for focus loop parameters.")

    def camstep_focus_acam(self):
        # Use placeholder text if no input provided
        value = self.focus_value_input.text() or self.focus_value_input.placeholderText()
        upper = self.focus_upper_input.text() or self.focus_upper_input.placeholderText()
        lower = self.focus_lower_input.text() or self.focus_lower_input.placeholderText()
        step = self.focus_step_input.text() or self.focus_step_input.placeholderText()

        if value and upper and lower and step:
            command = f"camstep focus acam focusloop {value} {upper} {lower} {step}"
            self.run_command(command.split())
        else:
            print("Please provide valid input for ACAM focus loop parameters.")

    def set_tcs_focus(self):
        # Use placeholder text if no input provided
        value = self.tcs_focus_value_input.text() or self.tcs_focus_value_input.placeholderText()

        if value:
            command = f"tcs setfocus {value}"
            self.run_command(command.split())
        else:
            print("Please provide a valid input for the TCS focus.")

            
    # Event handler methods for R and I bands
    def run_focus(self):
        # This method will run all the other buttons when clicked
        print("Running Focus...")

        # Call each button's functionality
        self.activate_boi_r()
        self.activate_boi_i()
        self.activate_row_bin()
        self.activate_col_bin()
        self.set_exptime()
        self.set_slit()
        self.camstep_focus()
        self.camstep_focus_acam()

    def handle_command_result(self, result):
        """Handle the result from the command and update the UI."""
        print(result)  # You can update the UI here, for example, show result in a QLabel

