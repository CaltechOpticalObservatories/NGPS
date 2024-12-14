from PyQt5.QtWidgets import QWidget, QVBoxLayout, QHBoxLayout, QPushButton, QLabel, QLineEdit, QFrame, QMessageBox
from PyQt5.QtCore import Qt, QTimer
from PyQt5.QtMultimedia import QSound
from logic_service import LogicService
import astropy.units as u

class ControlTab(QWidget):
    def __init__(self, parent):
        super().__init__(parent)
        self.parent = parent
        self.create_control_tab()
        self.logic_service = LogicService(self.parent)

    def create_control_tab(self):
        # Create the main layout for the Control tab
        control_layout = QVBoxLayout()

        # Row 1: Target Name Label and Refresh Button
        row1_widget = self.create_row1()
        control_layout.addWidget(row1_widget)

        # Add a thin gray line between rows
        self.add_separator_line(control_layout)

        # Row 2: Exposure Time, Slit Width, and Confirm Button
        row2_widget = self.create_row2()
        control_layout.addWidget(row2_widget)

        # Add a thin gray line between rows
        self.add_separator_line(control_layout)

        # Row 3: Go Button
        row3_widget = self.create_row3()
        control_layout.addWidget(row3_widget)

        # Add a thin gray line between rows
        self.add_separator_line(control_layout)

        # Row 4: Pause, Stop Now, and Expose Buttons
        row4_widget = self.create_row4()
        control_layout.addWidget(row4_widget)

        # Add a thin gray line between rows
        self.add_separator_line(control_layout)

        # Row 5: Binning, Headers, Display, Temp, Lamps, and Startup Buttons
        row5_widget = self.create_row5()
        control_layout.addWidget(row5_widget)

        # Set the layout for the control tab
        self.setLayout(control_layout)

        # Connect the input fields to methods for handling changes
        self.connect_input_fields()

    def create_row1(self):
        """Create Row 1 layout with Target Name Label and Refresh Button"""
        row1_layout = QHBoxLayout()
        row1_layout.setContentsMargins(0, 0, 0, 0)

        # Create the QLabel with default text
        self.target_name_label = QLabel("Selected Target: Not Selected")
        self.target_name_label.setAlignment(Qt.AlignCenter)

        row1_layout.addWidget(self.target_name_label)

        row1_widget = QWidget()
        row1_widget.setLayout(row1_layout)
        return row1_widget

    def create_row2(self):
        """Create Row 2 layout with Exposure Time, Slit Width, and Confirm Button"""
        row2_layout = QVBoxLayout()
        row2_layout.setContentsMargins(0, 0, 0, 0)
        row2_layout.setSpacing(5)

        # Exposure Time and Slit Width fields
        self.exposure_time_label = QLabel("Exposure Time:")
        self.exposure_time_box = QLineEdit()
        self.exposure_time_box.setPlaceholderText("Enter Exposure Time")
        self.exposure_time_box.setFixedWidth(120)

        exposure_time_layout = QHBoxLayout()
        exposure_time_layout.addWidget(self.exposure_time_label)
        exposure_time_layout.addWidget(self.exposure_time_box)

        self.slit_width_label = QLabel("Slit Width:")
        self.slit_width_box = QLineEdit()
        self.slit_width_box.setPlaceholderText("Enter Slit Width")
        self.slit_width_box.setFixedWidth(120)

        slit_width_layout = QHBoxLayout()
        slit_width_layout.addWidget(self.slit_width_label)
        slit_width_layout.addWidget(self.slit_width_box)

        self.slit_angle_label = QLabel("Slit Angle:")
        self.slit_angle_box = QLineEdit()
        self.slit_angle_box.setPlaceholderText("Enter Slit Width")
        self.slit_angle_box.setFixedWidth(120)

        slit_angle_layout = QHBoxLayout()
        slit_angle_layout.addWidget(self.slit_angle_label)
        slit_angle_layout.addWidget(self.slit_angle_box)


        row2_layout.addLayout(exposure_time_layout)
        row2_layout.addLayout(slit_width_layout)
        row2_layout.addLayout(slit_angle_layout)

        # Confirm Button
        self.confirm_button = QPushButton("Confirm Changes")
        self.confirm_button.setEnabled(False)
        self.confirm_button.clicked.connect(self.on_confirm_changes)
        self.confirm_button.setStyleSheet("""
                QPushButton {
                    background-color: #D3D3D3;  /* Light gray when disabled */
                    color: black;
                    font-weight: bold;
                    padding: 10px;
                    border: none;
                    border-radius: 5px;  /* Optional: Round corners */
                }
                QPushButton:hover {
                    background-color: #D3D3D3;  /* No hover effect when disabled */
                }
                QPushButton:pressed {
                    background-color: #D3D3D3;  /* No pressed effect when disabled */
                }
            """)
        row2_layout.addWidget(self.confirm_button)

        row2_widget = QWidget()
        row2_widget.setLayout(row2_layout)
        return row2_widget

    def create_row3(self):
        """Create Row 3 layout with Go Button and Offset To Target Button"""
        row3_layout = QHBoxLayout()
        row3_layout.setContentsMargins(0, 0, 0, 0)
        row3_layout.setSpacing(10)

        # Go Button
        self.go_button = QPushButton("Go")
        self.go_button.clicked.connect(self.on_go_button_click)
        self.go_button.setEnabled(False)

        # Offset To Target Button
        self.offset_to_target_button = QPushButton("Offset")
        self.offset_to_target_button.clicked.connect(self.on_offset_to_target_click)
        self.offset_to_target_button.setEnabled(False)

        # Offset To Target Button
        self.continue_button = QPushButton("Continue")
        self.continue_button.clicked.connect(self.on_continue_button_click)
        self.continue_button.setEnabled(False)

        row3_layout.addWidget(self.go_button)
        row3_layout.addWidget(self.offset_to_target_button)
        row3_layout.addWidget(self.continue_button)

        row3_widget = QWidget()
        row3_widget.setLayout(row3_layout)
        return row3_widget

    def create_row4(self):
        """Create Row 4 layout with Pause, Stop Now, and Expose Buttons"""
        row4_layout = QHBoxLayout()
        row4_layout.setSpacing(10)

        # Buttons
        self.pause_button = QPushButton("Pause")
        self.stop_now_button = QPushButton("Stop Now")

        # Connect the buttons to their corresponding slots
        self.pause_button.clicked.connect(self.on_pause_button_click)
        self.stop_now_button.clicked.connect(self.on_stop_now_button_click)

        row4_layout.addWidget(self.pause_button)
        row4_layout.addWidget(self.stop_now_button)

        row4_widget = QWidget()
        row4_widget.setLayout(row4_layout)
        return row4_widget

    def on_pause_button_click(self):
        """Toggle between Pause and Resume when the Pause button is clicked."""
        if hasattr(self, 'is_paused') and self.is_paused:
            # If currently paused, resume and change button text to "Pause"
            self.is_paused = False
            self.pause_button.setText("Pause")
            print("Resuming action...")
            # Add resume functionality here
        else:
            # If not paused, pause and change button text to "Resume"
            self.is_paused = True
            self.pause_button.setText("Resume")
            print("Pausing action...")
            # Add pause functionality here

    def on_stop_now_button_click(self):
        """Handle Stop Now button click."""
        print("Stopping now...")

    def create_row5(self):
        """Create Row 5 layout with Binning, Headers, Display, Temp, Lamps, and Startup Buttons"""
        row5_layout = QHBoxLayout()
        row5_layout.setSpacing(10)

        # Create vertical layouts for each pair of buttons
        binning_layout = QVBoxLayout()
        headers_layout = QVBoxLayout()
        display_layout = QVBoxLayout()
        temp_layout = QVBoxLayout()
        lamps_layout = QVBoxLayout()

        # Create the buttons
        self.binning_button = QPushButton("Binning")
        self.headers_button = QPushButton("Headers")
        self.display_button = QPushButton("Display")
        self.temp_button = QPushButton("Temp")
        self.startup_button = QPushButton("Startup")
        self.shutdown_button = QPushButton("Shutdown")
        self.startup_button.clicked.connect(self.on_startup_button_click)
        self.shutdown_button.clicked.connect(self.on_shutdown_button_click)

        # Add buttons to each vertical layout
        binning_layout.addWidget(self.binning_button)
        binning_layout.addWidget(self.headers_button)

        display_layout.addWidget(self.display_button)
        display_layout.addWidget(self.temp_button)

        lamps_layout.addWidget(self.startup_button)
        lamps_layout.addWidget(self.shutdown_button)

        # Add the vertical layouts to the main row layout
        row5_layout.addLayout(binning_layout)
        row5_layout.addLayout(display_layout)
        row5_layout.addLayout(lamps_layout)

        row5_widget = QWidget()
        row5_widget.setLayout(row5_layout)
        return row5_widget


    def add_separator_line(self, layout):
        """ Helper method to add a thin light gray line (separator) between rows. """
        separator = QFrame()
        separator.setFrameShape(QFrame.HLine)
        separator.setFrameShadow(QFrame.Sunken)
        separator.setLineWidth(1)
        separator.setStyleSheet("background-color: lightgray;")
        layout.addWidget(separator)

    def connect_input_fields(self):
        """Connect input fields (Exposure Time and Slit Width) to change methods"""
        self.exposure_time_box.textChanged.connect(self.on_input_changed)
        self.slit_width_box.textChanged.connect(self.on_input_changed)
        self.slit_angle_box.textChanged.connect(self.on_input_changed)

    def on_continue_button_click(self):
        """Handle the 'Expose' button click"""
        print("Startup button clicked!")
        command = f"userexpose\n"
        self.parent.send_command(command)
        self.continue_button.setEnabled(False)
        self.continue_button.setStyleSheet("""
                QPushButton {
                    background-color: #D3D3D3;  /* Light gray when disabled */
                    color: black;
                    font-weight: bold;
                    padding: 10px;
                    border: none;
                    border-radius: 5px;  /* Optional: Round corners */
                }
                QPushButton:hover {
                    background-color: #D3D3D3;  /* No hover effect when disabled */
                }
                QPushButton:pressed {
                    background-color: #D3D3D3;  /* No pressed effect when disabled */
                }
        """)

    def on_startup_button_click(self):
        """Handle the 'Startup' button click"""
        print("Startup button clicked!")
        command = f"startup\n"
        self.parent.send_command(command)

    def on_shutdown_button_click(self):
        """Handle the 'Startup' button click"""
        print("Startup button clicked!")
        command = f"shutdown\n"
        self.parent.send_command(command) 

    def on_offset_to_target_click(self):
        """Handle the Offset To Target button click event"""
        print("Offset To Target button clicked!")
        command = f"targetoffset\n"
        print(f"Sending command to SequencerService: {command}")  # Print the command being sent
        # Call send_command method from SequencerService
        self.parent.send_command(command)
        self.offset_to_target_button.setEnabled(False)
        self.offset_to_target_button.setStyleSheet("""
                QPushButton {
                    background-color: #D3D3D3;  /* Light gray when disabled */
                    color: black;
                    font-weight: bold;
                    padding: 10px;
                    border: none;
                    border-radius: 5px;  /* Optional: Round corners */
                }
                QPushButton:hover {
                    background-color: #D3D3D3;  /* No hover effect when disabled */
                }
                QPushButton:pressed {
                    background-color: #D3D3D3;  /* No pressed effect when disabled */
                }
        """)

    def on_input_changed(self):
        """Enable the Confirm button when the user modifies input fields"""
        self.confirm_button.setEnabled(True)
        self.confirm_button.setStyleSheet("""
            QPushButton {
                background-color: #4CAF50;  /* Green when enabled */
                color: white;
                font-weight: bold;
                padding: 10px;
                border: none;
                border-radius: 5px;
            }
            QPushButton:hover {
                background-color: #388E3C;  /* Darker green when hovered */
            }
            QPushButton:pressed {
                background-color: #2C6B2F;  /* Even darker green when pressed */
            }
        """)

    def on_go_button_click(self):
        """Slot to handle 'Go' button click and send the target command."""
        if hasattr(self, 'current_observation_id'):
            observation_id = self.parent.current_observation_id
            print(f"Sending command: seq startone {observation_id}")
            self.send_target_command(observation_id)
            QSound.play("sound/go_button_clicked.wav")
            self.go_button.setEnabled(False)
            # Disable the button immediately after the user clicks it
            self.go_button.setStyleSheet("""
                QPushButton {
                    background-color: #D3D3D3;  /* Light gray when disabled */
                    color: black;
                    font-weight: bold;
                    padding: 10px;
                    border: none;
                    border-radius: 5px;  /* Optional: Round corners */
                }
                QPushButton:hover {
                    background-color: #D3D3D3;  /* No hover effect when disabled */
                }
                QPushButton:pressed {
                    background-color: #D3D3D3;  /* No pressed effect when disabled */
                }
            """)

            # Show a popup message
            self.show_waiting_popup()
            self.offset_to_target_button.setEnabled(True)
            self.offset_to_target_button.setStyleSheet("""
            QPushButton {
                background-color: #4CAF50;  /* Green when enabled */
                color: white;
                font-weight: bold;
                padding: 10px;
                border: none;
                border-radius: 5px;
            }
            QPushButton:hover {
                background-color: #388E3C;  /* Darker green when hovered */
            }
            QPushButton:pressed {
                background-color: #2C6B2F;  /* Even darker green when pressed */
            }
        """)            
            self.continue_button.setEnabled(True)
            self.continue_button.setStyleSheet("""
            QPushButton {
                background-color: #4CAF50;  /* Green when enabled */
                color: white;
                font-weight: bold;
                padding: 10px;
                border: none;
                border-radius: 5px;
            }
            QPushButton:hover {
                background-color: #388E3C;  /* Darker green when hovered */
            }
            QPushButton:pressed {
                background-color: #2C6B2F;  /* Even darker green when pressed */
            }
        """)

            # Start a QTimer to re-enable the button after 60 seconds
            # self.timer = QTimer(self)
            # self.timer.setSingleShot(True)  # Ensure the timer only runs once
            # self.timer.timeout.connect(self.enable_go_button)
            # self.timer.start(60000)  # Timeout after 60 seconds (60000 ms)

        else:
            print("No observation ID available.")

    def show_waiting_popup(self):
        """Show a popup message that disappears after 5 seconds."""
        msg_box = QMessageBox(self)
        msg_box.setIcon(QMessageBox.Information)
        msg_box.setText("Waiting for TCS Operator...")
        msg_box.setWindowTitle("Information")
        msg_box.setStandardButtons(QMessageBox.NoButton)  # No buttons, just a message

        # Set a QTimer to close the message box after 5 seconds
        QTimer.singleShot(5000, msg_box.close)
        
        msg_box.exec_()
        
    def enable_go_button(self):
        """Method to re-enable the 'Go' button after 60 seconds."""
        print("60 seconds have passed. Re-enabling 'Go' button.")
        
        # Re-enable the button and reset its appearance
        self.go_button.setEnabled(True)
        self.go_button.setStyleSheet("""
            QPushButton {
                background-color: #4CAF50;  /* Green when enabled */
                color: white;
                font-weight: bold;
                padding: 10px;
                border: none;
                border-radius: 5px;  /* Optional: Round corners */
            }
            QPushButton:hover {
                background-color: #388E3C;  /* Darker green when hovered */
            }
            QPushButton:pressed {
                background-color: #2C6B2F;  /* Even darker green when pressed */
            }
        """)
            
    def send_target_command(self, observation_id):
        """ Method to send the command to the SequencerService """
        if observation_id:
            # Build the command string
            command = f"startone {observation_id}\n"
            print(f"Sending command to SequencerService: {command}")  # Print the command being sent
            # Call send_command method from SequencerService
            self.parent.send_command(command)
            print(f"Command sent: {command}")  # Print confirmation of command sent
        else:
            print("No OBSERVATION_ID to send the command.")  # Print if no observation ID is found


    def on_confirm_changes(self):
        """Handle the confirmation of changes made to the input fields"""
        exposure_time = self.exposure_time_box.text()
        slit_width = self.slit_width_box.text()
        slit_angle = self.slit_angle_box.text()
        
        if exposure_time and slit_width and slit_angle:
            # Handle the confirmed changes, e.g., update internal state or UI
            print(f"Confirmed Exposure Time: {exposure_time}, Slit Width: {slit_width}, Slit Angle: {slit_angle}")
            self.on_exposure_time_changed()
            self.on_slit_width_changed()
            self.on_slit_angle_changed()  # Handle slit angle change as well
            
            # Disable the button again after confirmation
            self.confirm_button.setEnabled(False)
            self.confirm_button.setStyleSheet("""
                QPushButton {
                    background-color: lightgray;
                }
            """)
        elif exposure_time and slit_width:
            # Handle the confirmed changes, e.g., update internal state or UI
            print(f"Confirmed Exposure Time: {exposure_time}, Slit Width: {slit_width}, Slit Angle: {slit_angle}")
            self.on_exposure_time_changed()
            self.on_slit_width_changed()
            QSound.play("sound/exposure_slit_width_set.wav")
            
            # Disable the button again after confirmation
            self.confirm_button.setEnabled(False)
            self.confirm_button.setStyleSheet("""
                QPushButton {
                    background-color: lightgray;
                }
            """)            
        elif exposure_time:
            # Handle the confirmed changes for exposure time
            print(f"Confirmed Exposure Time: {exposure_time}")
            self.on_exposure_time_changed()
            QSound.play("sound/exposure_set.wav")
            
            # Disable the button again after confirmation
            self.confirm_button.setEnabled(False)
            self.confirm_button.setStyleSheet("""
                QPushButton {
                    background-color: lightgray;
                }
            """)

        elif slit_width:
            # Handle the confirmed changes for slit width
            print(f"Confirmed Slit Width: {slit_width}")
            self.on_slit_width_changed()
            QSound.play("sound/slit_width_set.wav")
            
            # Disable the button again after confirmation
            self.confirm_button.setEnabled(False)
            self.confirm_button.setStyleSheet("""
                QPushButton {
                    background-color: lightgray;
                }
            """)

        elif slit_angle:
            # Handle the confirmed changes for slit angle
            print(f"Confirmed Slit Angle: {slit_angle}")
            self.on_slit_angle_changed()
            
            # Disable the button again after confirmation
            self.confirm_button.setEnabled(False)
            self.confirm_button.setStyleSheet("""
                QPushButton {
                    background-color: lightgray;
                }
            """)  

        else:
            # Handle the case where one or more fields are empty
            print("Please enter valid values for all fields.")


    def on_exposure_time_changed(self):
        # Retrieve the exposure time and send the query to the database
        exposure_time = self.exposure_time_box.text()
        if (self.parent.current_observation_id):
            self.logic_service.send_update_to_db(self.parent.current_observation_id, "OTMexpt", "SET " + exposure_time)
            self.exposure_time_box.clear()

    def on_slit_width_changed(self):
        # Retrieve the slit width and send the query to the database
        slit_width = self.slit_width_box.text()
        if (self.parent.current_observation_id):
            self.logic_service.send_update_to_db(self.parent.current_observation_id, "OTMslitwidth", "SET " + slit_width)
            self.slit_width_box.clear()

    def on_slit_angle_changed(self):
        # Retrieve the slit width and send the query to the database
        slit_angle = self.slit_angle_box.text()
        if(slit_angle == "PA"):
            slit_angle = self.logic_service.compute_parallactic_angle_astroplan(self.logic_service, self.parent.current_ra, self.parent.current_dec)
            print(f"Parallactic Angle: {slit_angle.to(u.deg):.2f}")
            self.slit_angle_box.setText(slit_angle)

        if (self.parent.current_observation_id):
            self.logic_service.send_update_to_db(self.parent.current_observation_id, "OTMslitangle", slit_angle)
            self.slit_angle_box.clear()
