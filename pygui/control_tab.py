from PyQt5.QtWidgets import (
    QWidget, QVBoxLayout, QHBoxLayout, QPushButton, QLabel,
    QLineEdit, QFrame, QMessageBox, QDialog
)
from PyQt5.QtCore import Qt, QTimer
from PyQt5.QtMultimedia import QSound
from logic_service import LogicService
import subprocess


class ControlTab(QDialog):
    def __init__(self, parent):
        super().__init__(parent)
        self.parent = parent

        # state flags
        self.is_paused = False

        # build UI
        self.create_control_tab()

        # services
        self.logic_service = LogicService(self.parent)

    # -----------------------------
    # Style helpers (centralized)
    # -----------------------------
    def _style_enabled_green(self, btn: QPushButton):
        btn.setEnabled(True)
        btn.setStyleSheet("""
            QPushButton {
                background-color: #4CAF50;
                color: white;
                font-weight: bold;
                padding: 10px;
                border: none;
                border-radius: 5px;
            }
            QPushButton:hover   { background-color: #388E3C; }
            QPushButton:pressed { background-color: #2C6B2F; }
        """)

    def _style_disabled_gray(self, btn: QPushButton):
        btn.setEnabled(False)
        btn.setStyleSheet("""
            QPushButton {
                background-color: #D3D3D3;
                color: black;
                font-weight: bold;
                padding: 10px;
                border: none;
                border-radius: 5px;
            }
        """)

    def _style_black(self, btn: QPushButton):
        btn.setEnabled(True)
        btn.setStyleSheet("""
            QPushButton {
                background-color: #000000;
                border: none;
                color: white;
                font-weight: bold;
            }
            QPushButton:hover   { background-color: #333333; }
            QPushButton:pressed { background-color: #555555; }
        """)

    def _apply_startup_style(self):
        self.startup_shutdown_button.setText("Startup")
        self._style_enabled_green(self.startup_shutdown_button)

    def _apply_shutdown_style(self):
        self.startup_shutdown_button.setText("Shutdown")
        self._style_black(self.startup_shutdown_button)

    # -----------------------------
    # UI construction
    # -----------------------------
    def create_control_tab(self):
        control_layout = QVBoxLayout()

        control_layout.addWidget(self.create_row1())
        self.add_separator_line(control_layout)

        control_layout.addWidget(self.create_row2())
        self.add_separator_line(control_layout)

        control_layout.addWidget(self.create_row3())
        self.add_separator_line(control_layout)

        control_layout.addWidget(self.create_row4())
        self.add_separator_line(control_layout)

        control_layout.addWidget(self.create_row5())

        self.setLayout(control_layout)
        self.connect_input_fields()

    def create_row1(self):
        """Row 1: Target Name & RA/Dec (stacked)"""
        row1_layout = QVBoxLayout()
        row1_layout.setContentsMargins(0, 0, 0, 0)

        self.target_name_label = QLabel("Selected Target: Not Selected")
        self.target_name_label.setAlignment(Qt.AlignCenter)

        self.ra_dec_label = QLabel("RA: Not Set, Dec: Not Set")
        self.ra_dec_label.setAlignment(Qt.AlignCenter)

        row1_layout.addWidget(self.target_name_label)
        row1_layout.addWidget(self.ra_dec_label)

        row1_widget = QWidget()
        row1_widget.setLayout(row1_layout)
        return row1_widget

    def create_row2(self):
        """Row 2: Exposure, Slit Width/Angle, Binning, #Exposures, Confirm"""
        row2_layout = QVBoxLayout()
        row2_layout.setContentsMargins(0, 0, 0, 0)
        row2_layout.setSpacing(5)

        # Exposure time
        self.exposure_time_label = QLabel("Exposure Time:")
        self.exposure_time_box = QLineEdit()
        self.exposure_time_box.setPlaceholderText("Enter Exposure Time")
        self.exposure_time_box.setFixedWidth(120)
        exposure_time_layout = QHBoxLayout()
        exposure_time_layout.addWidget(self.exposure_time_label)
        exposure_time_layout.addWidget(self.exposure_time_box)

        # Slit width
        self.slit_width_label = QLabel("Slit Width:")
        self.slit_width_box = QLineEdit()
        self.slit_width_box.setPlaceholderText("Enter Slit Width")
        self.slit_width_box.setFixedWidth(120)
        slit_width_layout = QHBoxLayout()
        slit_width_layout.addWidget(self.slit_width_label)
        slit_width_layout.addWidget(self.slit_width_box)

        # Slit angle
        self.slit_angle_label = QLabel("Slit Angle:")
        self.slit_angle_box = QLineEdit()
        self.slit_angle_box.setPlaceholderText("Enter Slit Angle")
        self.slit_angle_box.setFixedWidth(120)
        slit_angle_layout = QHBoxLayout()
        slit_angle_layout.addWidget(self.slit_angle_label)
        slit_angle_layout.addWidget(self.slit_angle_box)

        # Binning (spectral/spatial)
        self.bin_spect_label = QLabel("Bin Spectral:")
        self.bin_spect_box = QLineEdit()
        self.bin_spect_box.setPlaceholderText("Enter Value")
        self.bin_spect_box.setFixedWidth(120)

        self.bin_spat_label = QLabel("Bin Spatial:")
        self.bin_spat_box = QLineEdit()
        self.bin_spat_box.setPlaceholderText("Enter Value")
        self.bin_spat_box.setFixedWidth(120)

        bin_spect_layout = QHBoxLayout()
        bin_spect_layout.addWidget(self.bin_spect_label)
        bin_spect_layout.addWidget(self.bin_spect_box)
        bin_spat_layout = QHBoxLayout()
        bin_spat_layout.addWidget(self.bin_spat_label)
        bin_spat_layout.addWidget(self.bin_spat_box)

        # Number of exposures
        self.num_of_exposures_label = QLabel("Number of Exposures:")
        self.num_of_exposures_box = QLineEdit()
        self.num_of_exposures_box.setPlaceholderText("Enter Number of Exposures")
        self.num_of_exposures_box.setFixedWidth(120)
        num_of_exposures_layout = QHBoxLayout()
        num_of_exposures_layout.addWidget(self.num_of_exposures_label)
        num_of_exposures_layout.addWidget(self.num_of_exposures_box)

        # Assemble
        row2_layout.addLayout(exposure_time_layout)
        row2_layout.addLayout(slit_width_layout)
        row2_layout.addLayout(slit_angle_layout)
        row2_layout.addLayout(bin_spat_layout)
        row2_layout.addLayout(bin_spect_layout)
        row2_layout.addLayout(num_of_exposures_layout)

        # Confirm button
        self.confirm_button = QPushButton("Confirm Changes")
        self._style_disabled_gray(self.confirm_button)
        self.confirm_button.clicked.connect(self.on_confirm_changes)
        row2_layout.addWidget(self.confirm_button)

        row2_widget = QWidget()
        row2_widget.setLayout(row2_layout)
        return row2_widget

    def create_row3(self):
        """Row 3: Go / Offset / Expose"""
        row3_layout = QHBoxLayout()
        row3_layout.setContentsMargins(0, 0, 0, 0)
        row3_layout.setSpacing(10)

        self.go_button = QPushButton("Go")
        self.go_button.clicked.connect(self.on_go_button_click)
        self._style_disabled_gray(self.go_button)

        self.offset_to_target_button = QPushButton("Offset")
        self.offset_to_target_button.clicked.connect(self.on_offset_to_target_click)
        self._style_disabled_gray(self.offset_to_target_button)

        self.continue_button = QPushButton("Expose")
        self.continue_button.clicked.connect(self.on_continue_button_click)
        self._style_disabled_gray(self.continue_button)

        row3_layout.addWidget(self.go_button)
        row3_layout.addWidget(self.offset_to_target_button)
        row3_layout.addWidget(self.continue_button)

        row3_widget = QWidget()
        row3_widget.setLayout(row3_layout)
        return row3_widget

    def create_row4(self):
        """Row 4: Repeat / Pause / Stop Now / Abort"""
        row4_layout = QHBoxLayout()
        row4_layout.setSpacing(10)

        self.repeat_button = QPushButton("Repeat")
        self.pause_button = QPushButton("Pause")
        self.abort_button = QPushButton("Abort")
        self.stop_now_button = QPushButton("Stop Now")

        self.repeat_button.clicked.connect(self.on_repeat_button_click)
        self.pause_button.clicked.connect(self.on_pause_button_click)
        self.abort_button.clicked.connect(self.on_abort_button_click)
        self.stop_now_button.clicked.connect(self.on_stop_now_button_click)

        row4_layout.addWidget(self.repeat_button)
        row4_layout.addWidget(self.pause_button)
        row4_layout.addWidget(self.stop_now_button)
        row4_layout.addWidget(self.abort_button)

        row4_widget = QWidget()
        row4_widget.setLayout(row4_layout)
        return row4_widget

    def create_row5(self):
        """Row 5: Binning / Headers & Calibration / Reset & Startup"""
        row5_layout = QHBoxLayout()
        row5_layout.setSpacing(10)

        binning_layout = QVBoxLayout()
        display_layout = QVBoxLayout()
        lamps_layout = QVBoxLayout()

        self.binning_button = QPushButton("Binning")

        self.etc_button = QPushButton("Run ETC")
        self.etc_button.clicked.connect(self.parent.open_etc_popup)

        self.headers_button = QPushButton("Headers")

        self.calibration_button = QPushButton("Calibration")
        self.calibration_button.clicked.connect(self.parent.open_calibration_gui)

        self.reset_button = QPushButton("Reset")
        self.reset_button.clicked.connect(self.on_reset_button_click)

        binning_layout.addWidget(self.binning_button)
        binning_layout.addWidget(self.etc_button)

        display_layout.addWidget(self.headers_button)
        display_layout.addWidget(self.calibration_button)

        lamps_layout.addWidget(self.reset_button)

        self.startup_shutdown_button = QPushButton("Startup")
        self._apply_startup_style()
        self.startup_shutdown_button.clicked.connect(self.toggle_startup_shutdown)
        lamps_layout.addWidget(self.startup_shutdown_button)

        row5_layout.addLayout(binning_layout)
        row5_layout.addLayout(display_layout)
        row5_layout.addLayout(lamps_layout)

        row5_widget = QWidget()
        row5_widget.setLayout(row5_layout)
        return row5_widget

    # -----------------------------
    # Utility wiring
    # -----------------------------
    def add_separator_line(self, layout):
        """Thin divider line between rows."""
        separator = QFrame()
        separator.setFrameShape(QFrame.HLine)
        separator.setFrameShadow(QFrame.Sunken)
        separator.setLineWidth(1)
        separator.setStyleSheet("background-color: lightgray;")
        layout.addWidget(separator)

    def connect_input_fields(self):
        """Enable Confirm when fields change."""
        self.exposure_time_box.textChanged.connect(self.on_input_changed)
        self.slit_width_box.textChanged.connect(self.on_input_changed)
        self.slit_angle_box.textChanged.connect(self.on_input_changed)
        self.num_of_exposures_box.textChanged.connect(self.on_input_changed)
        self.bin_spect_box.textChanged.connect(self.on_input_changed)
        self.bin_spat_box.textChanged.connect(self.on_input_changed)

    # -----------------------------
    # Button slots / actions
    # -----------------------------
    def on_repeat_button_click(self):
        print("Repeating now...")
        self.parent.send_command("repeat\n")

    def on_pause_button_click(self):
        if self.is_paused:
            self.is_paused = False
            self.pause_button.setText("Pause")
            print("Resuming action...")
            self.parent.send_command("resume\n")
        else:
            self.is_paused = True
            self.pause_button.setText("Resume")
            print("Pausing action...")
            self.parent.send_command("pause\n")
            self.parent.layout_service.update_system_status("paused")

    def on_stop_now_button_click(self):
        print("Stopping now...")
        self.parent.send_command("stop\n")
        self._style_disabled_gray(self.go_button)
        self._style_disabled_gray(self.offset_to_target_button)
        self._style_disabled_gray(self.continue_button)

    def on_reset_button_click(self):
        print("Reset button clicked!")
        self.logic_service.refresh_table()

    def toggle_startup_shutdown(self):
        current_text = self.startup_shutdown_button.text()
        if current_text == "Startup":
            self._apply_shutdown_style()
            print("Startup button clicked!")
            self.parent.send_command("startup\n")
        else:
            confirm = QMessageBox.question(
                self,
                "Confirm System Shutdown",
                "Are you sure you want to shut down the system?",
                QMessageBox.Yes | QMessageBox.No,
                QMessageBox.No
            )
            if confirm == QMessageBox.Yes:
                self._apply_startup_style()
                print("Shutdown button clicked!")
                self.parent.send_command("shutdown\n")
            else:
                print("Shutdown canceled.")

    def on_offset_to_target_click(self):
        print("Offset To Target button clicked!")
        cmd = "targetoffset\n"
        print(f"Sending command to SequencerService: {cmd}")
        self.parent.send_command(cmd)
        self._style_disabled_gray(self.offset_to_target_button)

    def on_input_changed(self):
        self._style_enabled_green(self.confirm_button)

    def on_go_button_click(self):
        """Send target start command; disable Go and show waiting popup."""
        if self.parent.current_observation_id is not None:
            self.parent.zmq_status_service.unsubscribe_from_topic("slitd")
            observation_id = self.parent.current_observation_id
            print(f"Sending command: seq startone {observation_id}")
            self.parent.layout_service.update_slit_info_fields()
            self.send_target_command(observation_id)
            QSound.play("sound/go_button_clicked.wav")
            self._style_disabled_gray(self.go_button)
            self.logic_service.set_active_target(observation_id)
            self.show_waiting_popup()
        else:
            print("No observation ID available.")

    def enable_continue_and_offset_button(self):
        self._style_enabled_green(self.continue_button)
        self._style_enabled_green(self.offset_to_target_button)

    def show_waiting_popup(self):
        """Show a popup message with a 'Close' button (auto-closes after 5s)."""
        msg_box = QMessageBox(self)
        msg_box.setIcon(QMessageBox.Information)
        msg_box.setText("Waiting for TCS Operator...")
        msg_box.setWindowTitle("Information")
        msg_box.setStandardButtons(QMessageBox.Ok)
        QTimer.singleShot(5000, msg_box.close)
        msg_box.exec_()

    def enable_go_button(self):
        """Re-enable 'Go' button (kept for external timer hooks if you re-enable later)."""
        print("Re-enabling 'Go' button.")
        self._style_enabled_green(self.go_button)

    def send_target_command(self, observation_id):
        if observation_id:
            command = f"startone {observation_id}\n"
            print(f"Sending command to SequencerService: {command}")
            self.parent.send_command(command)
            print(f"Command sent: {command}")
        else:
            print("No OBSERVATION_ID to send the command.")

    def on_continue_button_click(self):
        """Send 'usercontinue' and enable Expose button only if seq state shows USER."""
        print("On Continue button clicked!")
        self.offset_to_target_button.setEnabled(False)
        self.parent.zmq_status_service.subscribe_to_topic("slitd")
        self.parent.send_command("usercontinue\n")

        try:
            result = subprocess.check_output(['seq', 'state'], text=True)
            print(result)
            if 'USER' in result:
                self._style_enabled_green(self.continue_button)
            else:
                self._style_disabled_gray(self.continue_button)
        except subprocess.CalledProcessError as e:
            print(f"Error running command: {e}")
            self._style_disabled_gray(self.continue_button)

    def on_abort_button_click(self):
        """Abort sequence; re-enable/disable buttons appropriately."""
        print("Abort button clicked!")
        self.parent.send_command("abort\n")
        self._style_enabled_green(self.go_button)
        self._style_disabled_gray(self.offset_to_target_button)
        self._style_disabled_gray(self.continue_button)

    # -----------------------------
    # DB update helpers (unchanged logic)
    # -----------------------------
    def on_confirm_changes(self):
        """Confirm input changes and push updates; also enables Go button."""
        exposure_time = self.exposure_time_box.text()
        slit_width = self.slit_width_box.text()
        slit_angle = self.slit_angle_box.text()
        num_of_exposures = self.num_of_exposures_box.text()
        bin_spect = self.bin_spect_box.text()
        bin_spat = self.bin_spat_box.text()

        if exposure_time and slit_width and slit_angle and num_of_exposures and bin_spect and bin_spat:
            print(f"Confirmed Exposure Time: {exposure_time}, Slit Width: {slit_width}, "
                  f"Slit Angle: {slit_angle}, Number of Exposures: {num_of_exposures}")
            self.on_exposure_time_changed()
            self.on_slit_width_changed()
            self.on_slit_angle_changed()
            self.num_of_exposures_changed()
            self.bin_spect_changed()
            self.bin_spat_changed()
            self._style_disabled_gray(self.confirm_button)

        elif exposure_time and slit_width:
            print(f"Confirmed Exposure Time: {exposure_time}, Slit Width: {slit_width}, Slit Angle: {slit_angle}")
            self.on_exposure_time_changed()
            self.on_slit_width_changed()
            QSound.play("sound/exposure_slit_width_set.wav")
            self._style_disabled_gray(self.confirm_button)

        elif exposure_time:
            print(f"Confirmed Exposure Time: {exposure_time}")
            self.on_exposure_time_changed()
            QSound.play("sound/exposure_set.wav")
            self._style_disabled_gray(self.confirm_button)

        elif slit_width:
            print(f"Confirmed Slit Width: {slit_width}")
            self.on_slit_width_changed()
            QSound.play("sound/slit_width_set.wav")
            self._style_disabled_gray(self.confirm_button)

        elif slit_angle:
            print(f"Confirmed Slit Angle: {slit_angle}")
            self.on_slit_angle_changed()
            self._style_disabled_gray(self.confirm_button)

        else:
            print("Please enter valid values for all fields.")

        if getattr(self.parent, "current_target_list_name", None):
            print(f"Current target list: {self.parent.current_target_list_name}")
            self.logic_service.update_target_table_with_list(self.parent.current_target_list_name)

        self._style_enabled_green(self.go_button)

    def on_exposure_time_changed(self):
        exposure_time = self.exposure_time_box.text()
        if self.parent.current_observation_id:
            self.logic_service.send_update_to_db(self.parent.current_observation_id, "OTMexpt", exposure_time)
            self.logic_service.send_update_to_db(self.parent.current_observation_id, "exptime", "SET " + exposure_time)

    def on_slit_width_changed(self):
        slit_width = self.slit_width_box.text()
        if self.parent.current_observation_id:
            self.logic_service.send_update_to_db(self.parent.current_observation_id, "OTMslitwidth", slit_width)
            self.logic_service.send_update_to_db(self.parent.current_observation_id, "slitwidth", "SET " + slit_width)

    def on_slit_angle_changed(self):
        slit_angle = self.slit_angle_box.text()
        if slit_angle == "PA":
            slit_angle = self.logic_service.compute_parallactic_angle_astroplan(self.parent.current_ra, self.parent.current_dec)
            print(f"Parallactic Angle: {slit_angle}")
            self.slit_angle_box.setText(slit_angle)

        if self.parent.current_observation_id:
            self.logic_service.send_update_to_db(self.parent.current_observation_id, "OTMslitangle", slit_angle)
            self.logic_service.send_update_to_db(self.parent.current_observation_id, "slitangle", "SET " + slit_angle)

    def num_of_exposures_changed(self):
        num_of_exposures = self.num_of_exposures_box.text()
        if self.parent.current_observation_id:
            self.logic_service.send_update_to_db(self.parent.current_observation_id, "nexp", num_of_exposures)

    def bin_spect_changed(self):
        bin_spect = self.bin_spect_box.text()
        if self.parent.current_observation_id:
            self.logic_service.send_update_to_db(self.parent.current_observation_id, "BINSPECT", bin_spect)

    def bin_spat_changed(self):
        bin_spat = self.bin_spat_box.text()
        if self.parent.current_observation_id:
            self.logic_service.send_update_to_db(self.parent.current_observation_id, "BINSPAT", bin_spat)
