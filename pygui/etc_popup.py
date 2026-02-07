from PyQt5.QtWidgets import QDialog, QVBoxLayout, QHBoxLayout, QLabel, QLineEdit, QComboBox, QCheckBox, QPushButton, QSpacerItem, QSizePolicy, QFrame, QMessageBox
from PyQt5.QtCore import Qt
import subprocess
import re
from typing import Dict, Any, List, Optional

class EtcPopup(QDialog):
    def __init__(self, parent=None, logic_service=None, initial_values: Optional[Dict[str, Any]] = None,
                 observation_ids: Optional[List[str]] = None):
        """
        Initialize ETC dialog.

        Args:
            parent: Parent widget
            logic_service: Logic service for database updates
            initial_values: Dictionary of initial field values to pre-fill
            observation_ids: List of observation IDs to update (for bulk operations)
        """
        super().__init__(parent)

        self.logic_service = logic_service
        self.observation_ids = observation_ids or []
        self.initial_values = initial_values or {}

        # Set window title based on mode
        if len(self.observation_ids) > 1:
            self.setWindowTitle(f"ETC - Calculate for {len(self.observation_ids)} targets")
        elif len(self.observation_ids) == 1:
            self.setWindowTitle("ETC - Calculate Exposure Time")
        else:
            self.setWindowTitle("ETC")

        self.setFixedSize(600, 600)  # Increased width for better alignment

        # Main layout for the dialog
        self.main_layout = QVBoxLayout()
        self.main_layout.setSpacing(12)
        self.main_layout.setContentsMargins(10, 10, 10, 10)

        # Initialize the form components
        self.init_widgets()
        self.init_layout()
        self.setLayout(self.main_layout)

        # Populate initial values if provided
        if self.initial_values:
            self.set_values(self.initial_values)

    def init_widgets(self):
        """Initialize all the widgets needed for the form."""
        # Create input fields and widgets
        self.magnitude_input = QLineEdit()
        self.filter_dropdown = QComboBox()
        self.filter_dropdown.addItems(["U", "G", "R", "I"])
        self.system_field = QLineEdit("AB")
        self.system_field.setReadOnly(True)

        self.sky_mag_input = QLineEdit()
        self.snr_input = QLineEdit()

        self.slit_width_input = QLineEdit()
        self.slit_dropdown = QComboBox()
        self.slit_dropdown.addItems(["SET", "LOSS", "SNR", "RES", "AUTO"])

        self.range_input_start = QLineEdit()
        self.range_input_end = QLineEdit()
        self.no_slicer_checkbox = QCheckBox("No Slicer")

        self.seeing_input = QLineEdit()
        self.airmass_input = QLineEdit()

        self.exptime_input = QLineEdit()
        self.resolution_input = QLineEdit()

        # Buttons
        self.run_button = QPushButton("Run ETC")
        self.save_button = QPushButton("Save")
        self.save_button.setEnabled(False)  # Initially disable the Save button

    def set_values(self, values: Dict[str, Any]):
        """
        Populate form fields from database values.

        Expected keys:
        - MAGNITUDE: Target magnitude
        - CHANNEL: Filter (U, G, R, I)
        - WRANGE_LOW: Wavelength range start
        - WRANGE_HIGH: Wavelength range end
        - AIRMASS_MAX: Maximum airmass
        - SEEING: Seeing value
        - SKY_MAG: Sky magnitude
        - SNR: Signal-to-noise ratio
        - SLITWIDTH: Slit width
        """
        # Set magnitude
        if "MAGNITUDE" in values and values["MAGNITUDE"]:
            self.magnitude_input.setText(str(values["MAGNITUDE"]))

        # Set filter/channel
        if "CHANNEL" in values and values["CHANNEL"]:
            channel = str(values["CHANNEL"]).upper()
            index = self.filter_dropdown.findText(channel)
            if index >= 0:
                self.filter_dropdown.setCurrentIndex(index)

        # Set wavelength range
        if "WRANGE_LOW" in values and values["WRANGE_LOW"]:
            self.range_input_start.setText(str(values["WRANGE_LOW"]))
        if "WRANGE_HIGH" in values and values["WRANGE_HIGH"]:
            self.range_input_end.setText(str(values["WRANGE_HIGH"]))

        # Set airmass
        if "AIRMASS_MAX" in values and values["AIRMASS_MAX"]:
            self.airmass_input.setText(str(values["AIRMASS_MAX"]))

        # Set seeing
        if "SEEING" in values and values["SEEING"]:
            self.seeing_input.setText(str(values["SEEING"]))

        # Set sky mag
        if "SKY_MAG" in values and values["SKY_MAG"]:
            self.sky_mag_input.setText(str(values["SKY_MAG"]))

        # Set SNR (default to 10 if not provided)
        if "SNR" in values and values["SNR"]:
            self.snr_input.setText(str(values["SNR"]))
        elif not self.snr_input.text():
            self.snr_input.setText("10")

        # Set slit width
        if "SLITWIDTH" in values and values["SLITWIDTH"]:
            self.slit_width_input.setText(str(values["SLITWIDTH"]))

        # Set slit option (default to SET if not provided)
        if "SLIT_MODE" in values and values["SLIT_MODE"]:
            mode = str(values["SLIT_MODE"]).upper()
            index = self.slit_dropdown.findText(mode)
            if index >= 0:
                self.slit_dropdown.setCurrentIndex(index)

    def init_layout(self):
        """Add widgets to the layout."""
        # Add input rows for each section
        self.main_layout.addLayout(self.create_input_row("Magnitude:", self.magnitude_input, self.filter_dropdown, self.system_field))
        
        # Call create_sky_mag_snr_layout for Sky Mag and SNR fields
        self.main_layout.addLayout(self.create_sky_mag_snr_layout())
        
        self.main_layout.addLayout(self.create_input_row("Slit Width:", self.slit_width_input, self.slit_dropdown))
        self.main_layout.addLayout(self.create_range_layout())

        # Modified the "Seeing" and "Airmass" row
        self.main_layout.addLayout(self.create_seeing_airmass_layout())

        # Modified the "Exp Time" and "Resolution" row
        self.main_layout.addLayout(self.create_exptime_resolution_layout())

        # Add a divider line
        divider_line = QFrame()
        divider_line.setFrameShape(QFrame.HLine)
        divider_line.setFrameShadow(QFrame.Sunken)
        self.main_layout.addWidget(divider_line)

        # Add buttons
        self.add_buttons()

        # Add a spacer to ensure widgets aren't squished
        spacer = QSpacerItem(20, 30, QSizePolicy.Minimum, QSizePolicy.Expanding)
        self.main_layout.addItem(spacer)

    def create_input_row(self, label_text, *widgets):
        """Create a horizontal layout with a label and widgets."""
        row_layout = QHBoxLayout()
        label = self.create_aligned_label(label_text)
        row_layout.addWidget(label)
        for widget in widgets:
            row_layout.addWidget(widget)
            widget.setFixedHeight(35)
            widget.setFixedWidth(110)
            widget.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Fixed)
        return row_layout

    def create_sky_mag_snr_layout(self):
        """Create a layout for Sky Mag and SNR with labels next to the input fields."""
        layout = QHBoxLayout()

        # Add Sky Mag label and input
        sky_mag_label = self.create_aligned_label("Sky Mag:")
        layout.addWidget(sky_mag_label)
        layout.addWidget(self.sky_mag_input)

        # Add SNR label and input next to it
        snr_label = self.create_aligned_label("SNR:")
        layout.addWidget(snr_label)
        layout.addWidget(self.snr_input)

        self.sky_mag_input.setFixedHeight(35)
        self.sky_mag_input.setFixedWidth(110)
        self.snr_input.setFixedHeight(35)
        self.snr_input.setFixedWidth(110)

        return layout

    def create_seeing_airmass_layout(self):
        """Create a layout for 'Seeing' and 'Airmass' next to each other."""
        layout = QHBoxLayout()
        
        # Add Seeing label and input
        seeing_label = self.create_aligned_label("Seeing:")
        layout.addWidget(seeing_label)
        layout.addWidget(self.seeing_input)
        
        # Add Airmass label and input next to it
        airmass_label = self.create_aligned_label("Airmass:")
        layout.addWidget(airmass_label)
        layout.addWidget(self.airmass_input)
        
        self.seeing_input.setFixedHeight(35)
        self.seeing_input.setFixedWidth(110)
        self.airmass_input.setFixedHeight(35)
        self.airmass_input.setFixedWidth(110)

        return layout

    def create_exptime_resolution_layout(self):
        """Create a layout for 'Exp Time' and 'Resolution' next to each other."""
        layout = QHBoxLayout()
        
        # Add Exp Time label and input
        exptime_label = self.create_aligned_label("Exp Time:")
        layout.addWidget(exptime_label)
        layout.addWidget(self.exptime_input)
        
        # Add Resolution label and input next to it
        resolution_label = self.create_aligned_label("Resolution:")
        layout.addWidget(resolution_label)
        layout.addWidget(self.resolution_input)
        
        self.exptime_input.setFixedHeight(35)
        self.exptime_input.setFixedWidth(110)
        self.resolution_input.setFixedHeight(35)
        self.resolution_input.setFixedWidth(110)

        return layout

    def create_range_layout(self):
        """Create the range row layout."""
        range_layout = QHBoxLayout()
        range_layout.setSpacing(10)
        range_layout.addWidget(self.create_aligned_label("Range:"))
        range_layout.addWidget(self.range_input_start)
        range_layout.addWidget(QLabel("-"))
        range_layout.addWidget(self.range_input_end)
        range_layout.addWidget(self.no_slicer_checkbox)
        return range_layout

    def create_aligned_label(self, text):
        """Create a label with right alignment."""
        label = QLabel(text)
        label.setFixedWidth(110)
        label.setAlignment(Qt.AlignRight | Qt.AlignVCenter)
        label.setStyleSheet("font-size: 14pt;")
        return label

    def add_buttons(self):
        """Add buttons to the layout."""
        button_row_layout = QHBoxLayout()
        button_row_layout.setSpacing(10)

        self.run_button.setFixedSize(110, 45)
        self.run_button.clicked.connect(self.run_etc)

        self.save_button.setFixedSize(100, 45)
        self.save_button.clicked.connect(self.save_etc)

        button_row_layout.addWidget(self.run_button)
        button_row_layout.addWidget(self.save_button)

        # Add a spacer after the buttons to create margin below
        spacer = QSpacerItem(20, 40, QSizePolicy.Minimum, QSizePolicy.Expanding)
        button_row_layout.addItem(spacer)

        self.main_layout.addLayout(button_row_layout)

    def validate_inputs(self):
        """Validates user inputs in the ETC tab and highlights invalid fields."""
        
        # Helper function to check if the input is a valid float and not empty
        def is_valid_float(text):
            if text.strip() == '':  # Check if the text is empty
                return False
            try:
                float(text)  # Try to convert to float
                return True
            except ValueError:
                return False  # Return False if the conversion fails

        # Reset all fields to default state (clear previous error highlighting)
        self.magnitude_input.setStyleSheet("")
        self.sky_mag_input.setStyleSheet("")
        self.snr_input.setStyleSheet("")
        self.slit_width_input.setStyleSheet("")
        self.range_input_start.setStyleSheet("")
        self.range_input_end.setStyleSheet("")
        
        try:
            # Check if all numeric inputs are valid
            if not is_valid_float(self.magnitude_input.text()):
                self.magnitude_input.setStyleSheet("border: 1px solid red;")  # Highlight invalid field
                raise ValueError("Magnitude must be a valid number.")
            magnitude = float(self.magnitude_input.text())

            if not is_valid_float(self.sky_mag_input.text()):
                self.sky_mag_input.setStyleSheet("border: 1px solid red;")  # Highlight invalid field
                raise ValueError("Sky Mag must be a valid number.")
            sky_mag = float(self.sky_mag_input.text())

            if not is_valid_float(self.snr_input.text()):
                self.snr_input.setStyleSheet("border: 1px solid red;")  # Highlight invalid field
                raise ValueError("SNR must be a valid number.")
            snr = float(self.snr_input.text())

            if not is_valid_float(self.slit_width_input.text()):
                self.slit_width_input.setStyleSheet("border: 1px solid red;")  # Highlight invalid field
                raise ValueError("Slit Width must be a valid number.")
            slit_width = float(self.slit_width_input.text())

            if not is_valid_float(self.range_input_start.text()):
                self.range_input_start.setStyleSheet("border: 1px solid red;")  # Highlight invalid field
                raise ValueError("Range Start must be a valid number.")
            range_start = float(self.range_input_start.text())

            if not is_valid_float(self.range_input_end.text()):
                self.range_input_end.setStyleSheet("border: 1px solid red;")  # Highlight invalid field
                raise ValueError("Range End must be a valid number.")
            range_end = float(self.range_input_end.text())
            
            # Ensure range_start is less than range_end
            if range_start >= range_end:
                self.range_input_start.setStyleSheet("border: 1px solid red;")  # Highlight invalid field
                self.range_input_end.setStyleSheet("border: 1px solid red;")  # Highlight invalid field
                raise ValueError("Range start must be less than range end.")

            # Check for valid values (you can adjust this for your specific needs)
            if magnitude <= 0 or sky_mag <= 0 or snr <= 0 or slit_width <= 0:
                raise ValueError("Magnitude, Sky Mag, SNR, and Slit Width must be positive values.")
            
            self.save_button.setEnabled(True)
            return True  # All inputs are valid

        except ValueError as e:
            # Show error message
            error_msg = f"Invalid input: {str(e)}"
            print(error_msg)
            return False  # Input is invalid


    def run_etc(self):
        """Handles the logic for the 'Run ETC' button."""
        
        # Validate inputs before running the command
        if not self.validate_inputs():
            return  # If inputs are invalid, do not proceed
        
        # Collecting all necessary data from input fields
        filter_value = self.filter_dropdown.currentText()  # e.g., "G"
        magnitude_value = self.magnitude_input.text()  # e.g., "18.0"
        sky_mag_value = self.sky_mag_input.text()  # e.g., "21.4"
        snr_value = self.snr_input.text()  # e.g., "10"
        slit_width_value = self.slit_width_input.text()  # e.g., "0.5"
        slit_option = self.slit_dropdown.currentText()  # e.g., "SET X"
        seeing_value = self.seeing_input.text()
        airmass_value = self.airmass_input.text()
        mag_system_value = self.system_field.text()  # e.g., "AB"
        mag_filter_value = "match"  # e.g., "match"
        
        # Handling the range inputs
        range_start_value = self.range_input_start.text()
        range_end_value = self.range_input_end.text()

        # Construct the command string
        command = f"python3 ETC/ETC_main.py {filter_value} {range_start_value} {range_end_value} SNR {snr_value} " \
                f"-slit {slit_option} {slit_width_value} -seeing {seeing_value} 500 -airmass {airmass_value} " \
                f"-skymag {sky_mag_value} -mag {magnitude_value} -magsystem {mag_system_value} -magfilter {mag_filter_value}"

        # Print the command for debugging
        print(f"Running command: {command}")
        
        # Run the command and capture the output
        try:
            result = subprocess.run(command, shell=True, capture_output=True, text=True)
            output = result.stdout.strip()  # Get the output from the command
            print(f"Command output: {output}")

            # Extract EXPTIME and RESOLUTION from the output using regex
            exptime_match = re.search(r"EXPTIME=([0-9.]+) s", output)
            resolution_match = re.search(r"RESOLUTION=([0-9.]+)", output)

            if exptime_match:
                exptime = float(exptime_match.group(1))
                exptime_rounded = round(exptime)  # Round EXPTIME to the nearest integer
                self.exptime_input.setText(str(exptime_rounded))  # Update GUI field with rounded EXPTIME

            if resolution_match:
                resolution = float(resolution_match.group(1))
                resolution_rounded = round(resolution)  # Round RESOLUTION to the nearest integer
                self.resolution_input.setText(str(resolution_rounded))  # Update GUI field with rounded RESOLUTION

        except subprocess.CalledProcessError as e:
            # Handle errors if the command fails
            print(f"Error running ETC: {e}")
        
        # Display the result in the results display (GUI)
        result_text = f"Running ETC with the following parameters:\n{command}\n\n" \
                    f"EXPTIME: {self.exptime_input.text()}\n" \
                    f"RESOLUTION: {self.resolution_input.text()}"
        print(result_text)
        self.save_button.setEnabled(True)

    def save_etc(self):
        """Save ETC results to database for all selected observation IDs."""
        if not self.logic_service:
            QMessageBox.warning(
                self,
                "No Logic Service",
                "Cannot save: Logic service not available."
            )
            return

        exptime = self.exptime_input.text()
        resolution = self.resolution_input.text()

        if not exptime or not resolution:
            QMessageBox.warning(
                self,
                "Missing Results",
                "Please run ETC calculation first."
            )
            return

        if not self.observation_ids:
            QMessageBox.warning(
                self,
                "No Target Selected",
                "No observation IDs to update."
            )
            return

        try:
            # Update all selected observation IDs
            for obs_id in self.observation_ids:
                self.logic_service.send_update_to_db(obs_id, "OTMexpt", exptime)
                self.logic_service.send_update_to_db(obs_id, "exptime", exptime)
                self.logic_service.send_update_to_db(obs_id, "OTMres", resolution)

            # Show success message
            if len(self.observation_ids) > 1:
                msg = f"Successfully updated {len(self.observation_ids)} targets with:\nEXPTIME={exptime}s\nRESOLUTION={resolution}"
            else:
                msg = f"Successfully updated target with:\nEXPTIME={exptime}s\nRESOLUTION={resolution}"

            QMessageBox.information(self, "ETC Results Saved", msg)
            self.save_button.setEnabled(False)

            # Close dialog after successful save
            self.accept()

        except Exception as e:
            QMessageBox.critical(
                self,
                "Save Failed",
                f"Error updating database:\n{e}"
            )
