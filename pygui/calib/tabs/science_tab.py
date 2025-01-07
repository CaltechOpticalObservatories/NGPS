import subprocess
from PyQt5.QtWidgets import QWidget, QVBoxLayout, QLabel, QPushButton, QLineEdit, QFormLayout, QScrollArea

class ScienceTab(QWidget):
    def __init__(self):
        super().__init__()
        self.initUI()

    def initUI(self):
        layout = QVBoxLayout()

        # Label for the tab
        label = QLabel("Science Observations", self)
        layout.addWidget(label)

        # Form Layout for Commands
        form_layout = QFormLayout()

        # tcs coords <RA> <Dec> 2000 0 0
        self.ra_input = QLineEdit(self)
        self.ra_input.setPlaceholderText("Enter RA (Right Ascension)")
        form_layout.addRow("RA:", self.ra_input)

        self.dec_input = QLineEdit(self)
        self.dec_input.setPlaceholderText("Enter Dec (Declination)")
        form_layout.addRow("Dec:", self.dec_input)

        coords_button = QPushButton("Set tcs coords", self)
        coords_button.clicked.connect(self.set_tcs_coords)
        form_layout.addRow("", coords_button)

        # Slit Set Command <slitwidth>
        self.slitwidth_input = QLineEdit(self)
        self.slitwidth_input.setPlaceholderText("Enter slit width")
        form_layout.addRow("Slit Width:", self.slitwidth_input)

        slit_button = QPushButton("Set Slit Width", self)
        slit_button.clicked.connect(self.set_slit)
        form_layout.addRow("", slit_button)

        # Camera bin row <spatial binning>
        self.spatial_binning_input = QLineEdit(self)
        self.spatial_binning_input.setPlaceholderText("Enter spatial binning")
        form_layout.addRow("Spatial Binning:", self.spatial_binning_input)

        bin_button = QPushButton("Set Camera Bin", self)
        bin_button.clicked.connect(self.set_camera_bin)
        form_layout.addRow("", bin_button)

        # Acquiring with ACAM
        self.acam_ra_input = QLineEdit(self)
        self.acam_ra_input.setPlaceholderText("Enter RA for ACAM")
        form_layout.addRow("RA (ACAM):", self.acam_ra_input)

        self.acam_dec_input = QLineEdit(self)
        self.acam_dec_input.setPlaceholderText("Enter Dec for ACAM")
        form_layout.addRow("Dec (ACAM):", self.acam_dec_input)

        self.acam_ring_az_input = QLineEdit(self)
        self.acam_ring_az_input.setPlaceholderText("Enter Ring AZ for ACAM")
        form_layout.addRow("Ring AZ (ACAM):", self.acam_ring_az_input)

        acam_button = QPushButton("Acam Acquire", self)
        acam_button.clicked.connect(self.acam_acquire)
        form_layout.addRow("", acam_button)

        # Change Exposure Time for SCAM/ACAM
        self.exposure_time_input = QLineEdit(self)
        self.exposure_time_input.setPlaceholderText("Enter exposure time")
        form_layout.addRow("Exposure Time:", self.exposure_time_input)

        exposure_button = QPushButton("Change Exposure Time", self)
        exposure_button.clicked.connect(self.change_exposure_time)
        form_layout.addRow("", exposure_button)

        # Scam Avg Frames <num>
        self.avg_frames_input = QLineEdit(self)
        self.avg_frames_input.setPlaceholderText("Enter number of frames")
        form_layout.addRow("Avg Frames:", self.avg_frames_input)

        avg_frames_button = QPushButton("Set Scam Avg Frames", self)
        avg_frames_button.clicked.connect(self.set_avg_frames)
        form_layout.addRow("", avg_frames_button)

        # Zero Offsets (TCS z)
        zero_offsets_button = QPushButton("Zero Offsets", self)
        zero_offsets_button.clicked.connect(self.zero_offsets)
        form_layout.addRow("", zero_offsets_button)

        # Native Offset Direction
        self.native_direction_input = QLineEdit(self)
        self.native_direction_input.setPlaceholderText("Enter direction")
        form_layout.addRow("Native Direction:", self.native_direction_input)

        self.native_offset_input = QLineEdit(self)
        self.native_offset_input.setPlaceholderText("Enter offset in arcseconds")
        form_layout.addRow("Offset (arcsec):", self.native_offset_input)

        native_button = QPushButton("Set Native Direction", self)
        native_button.clicked.connect(self.set_native_direction)
        form_layout.addRow("", native_button)

        # Camera Exposure Time
        self.camera_exptime_input = QLineEdit(self)  # <-- Added input field for camera exposure time
        self.camera_exptime_input.setPlaceholderText("Enter Camera Exposure Time")
        form_layout.addRow("Camera Exposure Time:", self.camera_exptime_input)

        camera_exptime_button = QPushButton("Set Camera Exposure Time", self)
        camera_exptime_button.clicked.connect(self.set_camera_exptime)
        form_layout.addRow("", camera_exptime_button)

        # Exposen <n>
        self.exposen_input = QLineEdit(self)
        self.exposen_input.setPlaceholderText("Enter exposure number")
        form_layout.addRow("Exposen:", self.exposen_input)

        exposen_button = QPushButton("Set Exposen", self)
        exposen_button.clicked.connect(self.set_exposen)
        form_layout.addRow("", exposen_button)

        # Create a QWidget to hold the form layout
        form_widget = QWidget(self)
        form_widget.setLayout(form_layout)

        # Create a QScrollArea to make the form scrollable
        scroll_area = QScrollArea(self)
        scroll_area.setWidgetResizable(True)  # Ensures the content resizes as needed
        scroll_area.setWidget(form_widget)

        # Add the scroll area to the main layout
        layout.addWidget(scroll_area)

        # Set the layout for the ScienceTab
        self.setLayout(layout)

    def execute_command(self, command):
        """Runs the given command in the terminal"""
        try:
            print(f"Running command: {command}")
            subprocess.run(command, shell=True, check=True)
        except subprocess.CalledProcessError as e:
            print(f"Error executing command: {e}")

    def start_science_experiment(self):
        print("Science experiment started...")

    def set_tcs_coords(self):
        ra = self.ra_input.text()
        dec = self.dec_input.text()
        if ra and dec:
            command = f"tcs coords {ra} {dec} 2000 0 0"
            self.execute_command(command)
        else:
            print("Please provide valid RA and Dec.")

    def set_slit(self):
        slitwidth = self.slitwidth_input.text()
        if slitwidth:
            command = f"slit set {slitwidth}"
            self.execute_command(command)
        else:
            print("Please provide a valid slit width.")

    def set_camera_bin(self):
        spatial_binning = self.spatial_binning_input.text()
        if spatial_binning:
            command = f"camera bin row {spatial_binning}"
            self.execute_command(command)
        else:
            print("Please provide a valid spatial binning.")

    def acam_acquire(self):
        ra = self.acam_ra_input.text()
        dec = self.acam_dec_input.text()
        ring_az = self.acam_ring_az_input.text()
        if ra and dec and ring_az:
            command = f"acam acquire {ra} {dec} {ring_az}"
            self.execute_command(command)
        else:
            print("Please provide valid RA, Dec, and Ring AZ for ACAM.")

    def change_exposure_time(self):
        exposure_time = self.exposure_time_input.text()
        if exposure_time:
            command_sc = f"scam exptime {exposure_time}"
            command_ac = f"acam exptime {exposure_time}"
            self.execute_command(command_sc)
            self.execute_command(command_ac)
        else:
            print("Please provide a valid exposure time.")

    def set_avg_frames(self):
        avg_frames = self.avg_frames_input.text()
        if avg_frames:
            command = f"scam avgframes {avg_frames}"
            self.execute_command(command)
        else:
            print("Please provide a valid number of frames.")

    def zero_offsets(self):
        command = "tcs z"
        self.execute_command(command)

    def set_native_direction(self):
        direction = self.native_direction_input.text()
        offset = self.native_offset_input.text()
        if direction and offset:
            command = f"tcs native {direction} {offset}"
            self.execute_command(command)
        else:
            print("Please provide a valid direction and offset.")

    def set_camera_exptime(self):
        exposure_time = self.camera_exptime_input.text()
        if exposure_time:
            command = f"camera exptime {exposure_time}"
            self.execute_command(command)
        else:
            print("Please provide a valid exposure time.")

    def set_exposen(self):
        exposen = self.exposen_input.text()
        if exposen:
            command = f"exposen {exposen}"
            self.execute_command(command)
        else:
            print("Please provide a valid exposure number.")
