from PyQt5.QtWidgets import (
    QDialog, QVBoxLayout, QHBoxLayout, QGridLayout,
    QLabel, QLineEdit, QComboBox, QCheckBox,
    QPushButton, QSizePolicy, QFrame
)
import re
import subprocess

class EtcPopup(QDialog):

    def __init__(self, parent=None):
        super().__init__(parent)

        self.FIELD_HEIGHT = 34
        self.FIELD_WIDTH = 220
        self.LABEL_WIDTH = 150
        self.INPUT_COLUMN_OFFSET = 190

        self.setWindowTitle("ETC")
        self.resize(900, 700)

        self.main_layout = QVBoxLayout(self)
        self.main_layout.setContentsMargins(20, 20, 20, 20)
        self.main_layout.setSpacing(14)
        
        self.channel_ranges = {
            "U": ("3250", "4330"),
            "G": ("4330", "5850"),
            "R": ("5850", "7600"),
            "I": ("7700", "9340"),
        }

        self.init_widgets()
        self.init_layout()

    def init_widgets(self):

        def line():
            w = QLineEdit()
            w.setMinimumHeight(self.FIELD_HEIGHT)
            w.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
            return w
        def combo(items):
            c = QComboBox()
            c.addItems(items)

            c.setMinimumHeight(self.FIELD_HEIGHT)
            c.setFixedWidth(self.FIELD_WIDTH)

            c.setMaxVisibleItems(4)  # exactly the number of items
            c.setSizeAdjustPolicy(QComboBox.AdjustToMinimumContentsLength)

            return c

        self.exptime_input = line()
        self.snr_input = line()

        self.snr_mode = QComboBox()
        self.snr_mode.addItems([
            "Fixed EXPTIME",
            "Solve for EXPTIME"
        ])
        self.snr_mode.setMinimumHeight(self.FIELD_HEIGHT)
        self.slit_width_input = line()
        self.resolution_input = line()
        self.res_mode = QComboBox()
        self.res_mode.addItems([
            "Fixed slit width",
            "RES",
            "AUTO"
        ])
        self.res_mode.setMinimumHeight(self.FIELD_HEIGHT)

        self.channel_dropdown = combo(["R", "I", "U", "G"])
        self.channel_dropdown.currentTextChanged.connect(self.update_channel_range)
        self.spatial_dropdown = combo(["1", "2", "4", "6"])
        self.spectral_dropdown = combo(["1", "2", "4", "6"])
        self.extract_dropdown = combo(["PSF", "2px", "4px", "6px", "8px", "10px"])

        self.range_start = line()
        self.range_end = line()

        self.no_slicer_checkbox = QCheckBox("No Slicer")
        self.no_slicer_checkbox.setChecked(True)
        self.no_slicer_checkbox.setMinimumHeight(self.FIELD_HEIGHT)

        self.seeing_input = line()
        self.seeing_wavelength = line()
        self.sky_mag_input = line()
        self.airmass_input = line()

        self.magnitude_input = line()
        self.abvega_dropdown = combo(["AB", "VEGA"])
        self.filter_dropdown = combo(["match", "U", "V", "R", "I"])

        self.extended_checkbox = QCheckBox("Extended Source")
        self.extended_checkbox.setMinimumHeight(self.FIELD_HEIGHT)

        self.expert_field = line()
        self.expert_field.setMaximumWidth(400)
        self.expert_field.setPlaceholderText("Advanced parameters (for power users)")

        self.run_button = QPushButton("Run ETC")
        self.save_button = QPushButton("Apply to Target")

        self.run_button.setFixedSize(160, 50)
        self.save_button.setFixedSize(160, 50)
        self.run_button.clicked.connect(self.run_etc)
        self.save_button.clicked.connect(self.save_etc)
        
        self.snr_mode.currentIndexChanged.connect(self.update_exptime_mode)
        self.res_mode.currentIndexChanged.connect(self.update_resolution_mode)
        self.extract_dropdown.currentTextChanged.connect(self.update_extract_mode)

        self.error_label = QLabel("")
        self.error_label.setStyleSheet("""
            QLabel {
                color: #cc0000;
                font-weight: bold;
            }
        """)
        self.error_label.setWordWrap(True)

        # initialize state
        self.update_exptime_mode()
        self.update_resolution_mode()
        
        # Default ETC values
        self.channel_dropdown.setCurrentText("R")
        self.filter_dropdown.setCurrentText("match")
        self.extract_dropdown.setCurrentText("8px")
        self.spatial_dropdown.setCurrentText("2")

        self.seeing_input.setText("1.5")
        self.seeing_wavelength.setText("6400")
        self.seeing_wavelength.setEnabled(False)
        self.airmass_input.setText("1")
        self.sky_mag_input.setText("21.4")
        self.magnitude_input.setText("18")

        self.update_channel_range(self.channel_dropdown.currentText())

    def label(self, text):
        l = QLabel(text)
        l.setFixedWidth(self.LABEL_WIDTH)
        return l

    def hline(self):
        line = QFrame()
        line.setFrameShape(QFrame.HLine)
        line.setStyleSheet("color:#555;margin-top:10px;margin-bottom:10px;")
        return line

    def etc_row(self, l_label=None, l_widget=None, r_label=None, r_widget=None):

        row = QHBoxLayout()
        row.setSpacing(10)

        def label(text):
            lab = QLabel(text)
            lab.setFixedWidth(self.INPUT_COLUMN_OFFSET)
            return lab

        if l_label:
            row.addWidget(label(l_label))

        if l_widget:
            row.addWidget(l_widget)

        if r_label:
            row.addSpacing(60)
            row.addWidget(label(r_label))

        if r_widget:
            row.addWidget(r_widget)

        row.addStretch()

        return row

    def init_layout(self):

        L = self.main_layout

        # EXPOSURE
        exp = QGridLayout()
        exp.setHorizontalSpacing(24)
        exp.setVerticalSpacing(14)

        exp.setColumnMinimumWidth(0, 180)
        exp.setColumnMinimumWidth(2, 140)

        exp.setColumnStretch(1, 1)
        exp.setColumnStretch(3, 1)

        # Row 1
        exp.addWidget(self.label("Exp. Time (s)"), 0, 0)
        exp.addWidget(self.exptime_input,          0, 1)

        exp.addWidget(self.label("SNR"),           0, 2)
        exp.addWidget(self.snr_input,              0, 3)
        exp.addWidget(self.snr_mode,               0, 4)

        # Row 2
        exp.addWidget(self.label("Slit width (arcsec)"), 1, 0)
        exp.addWidget(self.slit_width_input,             1, 1)

        exp.addWidget(self.label("Resolution"),          1, 2)
        exp.addWidget(self.resolution_input,             1, 3)
        exp.addWidget(self.res_mode,                     1, 4)

        L.addLayout(exp)
        L.addWidget(self.hline())

        # CHANNEL / CCD
        chan = QGridLayout()
        chan.setHorizontalSpacing(40)
        chan.setVerticalSpacing(16)

        chan.addWidget(QLabel("Channel"), 0, 0)
        chan.addWidget(self.channel_dropdown, 0, 1)

        snr = QHBoxLayout()
        snr.addWidget(self.range_start)
        snr.addWidget(self.range_end)

        chan.addWidget(QLabel("SNR Range (Å)"), 1, 0)
        chan.addLayout(snr, 1, 1)

        chan.addWidget(self.no_slicer_checkbox, 2, 0)

        chan.addWidget(QLabel("Bin Spatial"), 0, 2)
        chan.addWidget(self.spatial_dropdown, 0, 3)

        chan.addWidget(QLabel("Bin Spectral"), 1, 2)
        chan.addWidget(self.spectral_dropdown, 1, 3)

        chan.addWidget(QLabel("Extract Spatial"), 2, 2)
        chan.addWidget(self.extract_dropdown, 2, 3)

        L.addLayout(chan)
        L.addWidget(self.hline())

        # CONDITIONS
        cond = QGridLayout()
        cond.setHorizontalSpacing(40)
        cond.setVerticalSpacing(16)

        cond.addWidget(QLabel("Seeing (arcsec)"), 0, 0)
        cond.addWidget(self.seeing_input, 0, 1)

        cond.addWidget(QLabel("Seeing Pivot (Å)"), 0, 2)
        cond.addWidget(self.seeing_wavelength, 0, 3)

        cond.addWidget(QLabel("Sky (mag/arcsec²)"), 1, 0)
        cond.addWidget(self.sky_mag_input, 1, 1)

        cond.addWidget(QLabel("Airmass"), 1, 2)
        cond.addWidget(self.airmass_input, 1, 3)

        L.addLayout(cond)
        L.addWidget(self.hline())

        # TARGET MODEL
        target = QGridLayout()
        target.setHorizontalSpacing(40)
        target.setVerticalSpacing(16)

        target.addWidget(QLabel("Magnitude"), 0, 0)
        target.addWidget(self.magnitude_input, 0, 1)

        target.addWidget(QLabel("AB/Vega"), 0, 2)
        target.addWidget(self.abvega_dropdown, 0, 3)

        target.addWidget(QLabel("Filter"), 0, 4)
        target.addWidget(self.filter_dropdown, 0, 5)

        target.addWidget(self.extended_checkbox, 1, 0)

        target.addWidget(QLabel("Expert"), 1, 2)
        target.addWidget(self.expert_field, 1, 3, 1, 3)

        L.addLayout(target)
        L.addWidget(self.hline())

        L.addWidget(self.error_label)

        # BUTTONS
        btn = QHBoxLayout()
        btn.addStretch()
        btn.addWidget(self.run_button)
        btn.addSpacing(60)
        btn.addWidget(self.save_button)
        btn.addStretch()

        L.addLayout(btn)

    def update_exptime_mode(self):

        mode = self.snr_mode.currentText()

        if mode == "Fixed EXPTIME":
            self.set_field_state(self.exptime_input, True)
            self.set_field_state(self.snr_input, False)

        else:
            self.set_field_state(self.exptime_input, False)
            self.set_field_state(self.snr_input, True)
            
    def update_resolution_mode(self):

        mode = self.res_mode.currentText()

        if mode == "Fixed slit width":
            self.set_field_state(self.slit_width_input, True)
            self.set_field_state(self.resolution_input, False)

        elif mode == "RES":
            self.set_field_state(self.slit_width_input, False)
            self.set_field_state(self.resolution_input, True)

        else:  # AUTO
            self.set_field_state(self.slit_width_input, False)
            self.set_field_state(self.resolution_input, False)

    def set_field_state(self, field, enabled):

        field.setEnabled(enabled)

        if enabled:
            field.setStyleSheet("")
        else:
            field.clear()
            field.setStyleSheet("""
                QLineEdit {
                    border: 1px solid #cccccc;
                }
            """)

    def update_channel_range(self, channel):

        if channel in self.channel_ranges:

            start, end = self.channel_ranges[channel]

            self.range_start.setText(start)
            self.range_end.setText(end) 

    def update_extract_mode(self, mode):
        if mode != "PSF":
            self.no_slicer_checkbox.setChecked(False)
            
    def validate_inputs(self):
        """Validate numeric inputs and highlight invalid fields."""

        def is_valid_float(text):
            if text.strip() == "":
                return False
            try:
                float(text)
                return True
            except ValueError:
                return False

        fields = [
            self.magnitude_input,
            self.sky_mag_input,
            self.snr_input,
            self.slit_width_input,
            self.range_start,
            self.range_end,
            self.seeing_input,
            self.airmass_input,
        ]

        # clear previous errors
        for f in fields:
            f.setStyleSheet("")

        try:

            if not is_valid_float(self.magnitude_input.text()):
                self.magnitude_input.setStyleSheet("border:1px solid red;")
                raise ValueError("Magnitude must be a number")

            if not is_valid_float(self.sky_mag_input.text()):
                self.sky_mag_input.setStyleSheet("border:1px solid red;")
                raise ValueError("Sky Mag must be a number")

            if self.snr_input.isEnabled():
                if not is_valid_float(self.snr_input.text()):
                    self.snr_input.setStyleSheet("border:1px solid red;")
                    raise ValueError("SNR must be a number")

            if self.slit_width_input.isEnabled():
                if not is_valid_float(self.slit_width_input.text()):
                    self.slit_width_input.setStyleSheet("border:1px solid red;")
                    raise ValueError("Slit width must be a number")

            if not is_valid_float(self.range_start.text()):
                self.range_start.setStyleSheet("border:1px solid red;")
                raise ValueError("Range start must be a number")

            if not is_valid_float(self.range_end.text()):
                self.range_end.setStyleSheet("border:1px solid red;")
                raise ValueError("Range end must be a number")

            if self.resolution_input.isEnabled():
                if not is_valid_float(self.resolution_input.text()):
                    self.resolution_input.setStyleSheet("border:1px solid red;")
                    raise ValueError("Resolution must be a number")
        
            start = float(self.range_start.text())
            end = float(self.range_end.text())

            if start >= end:
                self.range_start.setStyleSheet("border:1px solid red;")
                self.range_end.setStyleSheet("border:1px solid red;")
                raise ValueError("Range start must be less than range end")

            self.save_button.setEnabled(True)
            return True

        except ValueError as e:
            print(f"Invalid input: {e}")
            return False

    def run_etc(self):

        if not self.validate_inputs():
            return
        self.error_label.setText("")

        channel = self.channel_dropdown.currentText()

        wrange_start = str(float(self.range_start.text()) / 10)
        wrange_end = str(float(self.range_end.text()) / 10)

        mag = self.magnitude_input.text()
        magsystem = self.abvega_dropdown.currentText()
        magfilter = self.filter_dropdown.currentText()

        sky_mag = self.sky_mag_input.text()
        seeing = self.seeing_input.text()
        airmass = self.airmass_input.text()

        # EXPTIME / SNR solve mode
        snr_mode = self.snr_mode.currentText()

        if snr_mode == "Fixed EXPTIME":
            solve_param = "EXPTIME"
            solve_value = self.exptime_input.text()
        else:
            solve_param = "SNR"
            solve_value = self.snr_input.text()

        # SLIT / RESOLUTION mode
        res_mode = self.res_mode.currentText()

        if res_mode == "Fixed slit width":
            slit_mode = ["-slit", "SET", self.slit_width_input.text()]

        elif res_mode == "RES":
            slit_mode = ["-slit", "RES", self.resolution_input.text()]

        else:
            slit_mode = ["-slit", "AUTO"]


        # Build command
        cmd = [
            "python3",
            "ETC/ETC_main.py",
            channel,
            wrange_start,
            wrange_end,
            solve_param,
            solve_value,
            *slit_mode,
            "-seeing", seeing, "640",
            "-airmass", airmass,
            "-skymag", sky_mag,
            "-mag", mag,
            "-magsystem", magsystem,
            "-magfilter", magfilter
        ]

        # slicer option
        if self.no_slicer_checkbox.isChecked():
            cmd.append("-noslicer")

        # binning options
        cmd.extend(["-binspect", self.spectral_dropdown.currentText()])
        cmd.extend(["-binspat", self.spatial_dropdown.currentText()])
        # extract aperture -> fastSNR option
        extract_mode = self.extract_dropdown.currentText()

        if extract_mode != "PSF":
            fastsnr_value = int(extract_mode.replace("px", "")) // 2
            cmd.extend(["-fastSNR", str(fastsnr_value)])

        # expert option
        expert = self.expert_field.text().strip()
        if expert:
            cmd.extend(expert.split())

        print("Running ETC command:")
        print(" ".join(cmd))

        try:

            result = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                check=True
            )

            output = result.stdout
            print(output)

            exptime_match = re.search(r"EXPTIME=([0-9.]+)", output)
            res_match = re.search(r"RESOLUTION=([0-9.]+)", output)
            snr_match = re.search(r"SNR=([0-9.]+)", output)
            slitwidth_match = re.search(r"SLITWIDTH=([0-9.]+)", output)

            if exptime_match:
                exptime = float(exptime_match.group(1))
                self.exptime_input.setText(str(exptime))

            if res_match:
                resolution = float(res_match.group(1))
                self.resolution_input.setText(str(resolution))

            if snr_match:
                snr = float(snr_match.group(1))
                self.snr_input.setText(str(snr))

            if slitwidth_match:
                slitwidth = float(slitwidth_match.group(1))
                self.slit_width_input.setText(str(slitwidth))

        except subprocess.CalledProcessError as e:

            stderr = e.stderr or ""

            error_line = ""

            for line in stderr.splitlines():
                if "ETC_main.py: error:" in line:
                    error_line = line
                    break

            if error_line:
                self.error_label.setText(error_line)
            else:
                self.error_label.setText("ETC failed. See terminal for details.")

            print("ETC failed")
            print(e.stdout)
            print(e.stderr)

        self.save_button.setEnabled(True)

    def save_etc(self):

        exptime = self.exptime_input.text()
        resolution = self.resolution_input.text()

        if self.parent.current_observation_id:

            self.logic_service.send_update_to_db(
                self.parent.current_observation_id, "OTMexpt", exptime
            )

            self.logic_service.send_update_to_db(
                self.parent.current_observation_id, "exptime", exptime
            )

            self.logic_service.send_update_to_db(
                self.parent.current_observation_id, "OTMres", resolution
            )

        self.save_button.setEnabled(True)
