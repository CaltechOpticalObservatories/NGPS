from PyQt5.QtWidgets import QWidget, QVBoxLayout, QHBoxLayout, QLabel, QCheckBox, QProgressBar, QGroupBox, QFormLayout, QComboBox, QTextEdit, QSizePolicy, QSpacerItem

class InstrumentStatusTab(QWidget):
    def __init__(self, parent):
        super().__init__(parent)
        self.parent = parent
        self.init_ui()

    def init_ui(self):
        # Main layout for the InstrumentStatusTab
        layout = QVBoxLayout()

        # Cal cover, cal door, and acam cover status (open/close)
        self.create_status_section(layout)

        # Lamp Status (FeAr, ThAr, RedCont, BlueCont) on|off
        self.create_lamp_status_section(layout)

        # Lamp Modulator Status (same as above + 2 more)
        self.create_lamp_modulator_status_section(layout)

        # Exposure status (exptime progress, readout progress, shutter open/close)
        self.create_exposure_status_section(layout)

        # Slit width offset (currently just display the status)
        self.create_slit_width_offset_section(layout)

        # Binning info (spectral & spatial)
        self.create_binning_info_section(layout)

        # Other remarks or notes
        self.create_remarks_section(layout)

        # Set the layout for this widget
        self.setLayout(layout)

    def create_status_section(self, layout):
        """Create a section to show the status of the covers and doors (open/close)"""
        status_group = QGroupBox("Cover and Door Status")
        form_layout = QFormLayout()

        self.cal_cover_checkbox = QCheckBox()
        self.cal_cover_checkbox.setEnabled(False)  # Read-only status
        form_layout.addRow("Cal Cover", self.cal_cover_checkbox)

        self.cal_door_checkbox = QCheckBox()
        self.cal_door_checkbox.setEnabled(False)  # Read-only status
        form_layout.addRow("Cal Door", self.cal_door_checkbox)

        self.acam_cover_checkbox = QCheckBox()
        self.acam_cover_checkbox.setEnabled(False)  # Read-only status
        form_layout.addRow("ACAM Cover", self.acam_cover_checkbox)

        status_group.setLayout(form_layout)
        layout.addWidget(status_group)

    def create_lamp_status_section(self, layout):
        """Create a section to show lamp status with labels and checkboxes aligned horizontally and columns side by side."""
        lamp_group = QGroupBox("Lamp Status")
        form_layout = QFormLayout()

        # Create the first vertical layout for FeAr Lamp and RedCont Lamp
        first_column_layout = QVBoxLayout()

        # First checkbox for FeAr Lamp
        first_row_layout = QHBoxLayout()
        self.lamp_FeAr_checkbox = QCheckBox()
        self.lamp_FeAr_checkbox.setEnabled(False)  # Read-only status
        self.lamp_FeAr_checkbox.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Fixed)  # Ensure consistent size
        first_row_layout.addWidget(QLabel("FeAr Lamp"))
        first_row_layout.addWidget(self.lamp_FeAr_checkbox)
        first_column_layout.addLayout(first_row_layout)

        # Spacer for space between FeAr Lamp and RedCont Lamp
        first_column_layout.addSpacerItem(QSpacerItem(20, 0, QSizePolicy.Minimum, QSizePolicy.Minimum))

        # Second checkbox for RedCont Lamp
        second_row_layout = QHBoxLayout()
        self.lamp_RedCont_checkbox = QCheckBox()
        self.lamp_RedCont_checkbox.setEnabled(False)  # Read-only status
        self.lamp_RedCont_checkbox.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Fixed)  # Ensure consistent size
        second_row_layout.addWidget(QLabel("RedCont Lamp"))
        second_row_layout.addWidget(self.lamp_RedCont_checkbox)
        first_column_layout.addLayout(second_row_layout)

        # Create the second vertical layout for ThAr Lamp and BlueCont Lamp
        second_column_layout = QVBoxLayout()

        # First checkbox for ThAr Lamp
        third_row_layout = QHBoxLayout()
        self.lamp_ThAr_checkbox = QCheckBox()
        self.lamp_ThAr_checkbox.setEnabled(False)  # Read-only status
        self.lamp_ThAr_checkbox.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Fixed)  # Ensure consistent size
        third_row_layout.addWidget(QLabel("ThAr Lamp"))
        third_row_layout.addWidget(self.lamp_ThAr_checkbox)
        second_column_layout.addLayout(third_row_layout)

        # Spacer for space between ThAr Lamp and BlueCont Lamp
        second_column_layout.addSpacerItem(QSpacerItem(20, 0, QSizePolicy.Minimum, QSizePolicy.Minimum))

        # Second checkbox for BlueCont Lamp
        fourth_row_layout = QHBoxLayout()
        self.lamp_BlueCont_checkbox = QCheckBox()
        self.lamp_BlueCont_checkbox.setEnabled(False)  # Read-only status
        self.lamp_BlueCont_checkbox.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Fixed)  # Ensure consistent size
        fourth_row_layout.addWidget(QLabel("BlueCont Lamp"))
        fourth_row_layout.addWidget(self.lamp_BlueCont_checkbox)
        second_column_layout.addLayout(fourth_row_layout)

        # Add both columns to a horizontal layout (side-by-side)
        row_layout = QHBoxLayout()
        row_layout.addLayout(first_column_layout)
        row_layout.addSpacerItem(QSpacerItem(30, 0, QSizePolicy.Minimum, QSizePolicy.Minimum))  # Optional spacer between columns
        row_layout.addLayout(second_column_layout)

        # Add the row layout to the form layout
        form_layout.addRow(row_layout)

        # Set the form layout to the lamp group
        lamp_group.setLayout(form_layout)

        # Add the lamp group to the main layout
        layout.addWidget(lamp_group)

    def create_lamp_modulator_status_section(self, layout):
        """Create a section to show lamp modulator status"""
        modulator_group = QGroupBox("Lamp Modulator Status")
        form_layout = QFormLayout()

        self.modulator_FeAr_checkbox = QCheckBox()
        self.modulator_FeAr_checkbox.setEnabled(False)
        form_layout.addRow("FeAr Modulator", self.modulator_FeAr_checkbox)

        self.modulator_ThAr_checkbox = QCheckBox()
        self.modulator_ThAr_checkbox.setEnabled(False)
        form_layout.addRow("ThAr Modulator", self.modulator_ThAr_checkbox)

        self.modulator_RedCont_checkbox = QCheckBox()
        self.modulator_RedCont_checkbox.setEnabled(False)
        form_layout.addRow("RedCont Modulator", self.modulator_RedCont_checkbox)

        self.modulator_BlueCont_checkbox = QCheckBox()
        self.modulator_BlueCont_checkbox.setEnabled(False)
        form_layout.addRow("BlueCont Modulator", self.modulator_BlueCont_checkbox)

        # Two more modulator statuses can be added here as needed
        self.modulator_Extra1_checkbox = QCheckBox()
        self.modulator_Extra1_checkbox.setEnabled(False)
        form_layout.addRow("Extra1 Modulator", self.modulator_Extra1_checkbox)

        self.modulator_Extra2_checkbox = QCheckBox()
        self.modulator_Extra2_checkbox.setEnabled(False)
        form_layout.addRow("Extra2 Modulator", self.modulator_Extra2_checkbox)

        modulator_group.setLayout(form_layout)
        layout.addWidget(modulator_group)

    def create_exposure_status_section(self, layout):
        """Create a section to show exposure time and readout progress"""
        exposure_group = QGroupBox("Exposure Status")
        form_layout = QFormLayout()

        self.exposure_progress_bar = QProgressBar()
        self.exposure_progress_bar.setRange(0, 100)
        self.exposure_progress_bar.setValue(0)
        form_layout.addRow("Exposure Time Progress", self.exposure_progress_bar)

        self.readout_progress_bar = QProgressBar()
        self.readout_progress_bar.setRange(0, 100)
        self.readout_progress_bar.setValue(0)
        form_layout.addRow("Readout Progress", self.readout_progress_bar)

        self.shutter_status_checkbox = QCheckBox()
        self.shutter_status_checkbox.setEnabled(False)  # Read-only status
        form_layout.addRow("Shutter", self.shutter_status_checkbox)

        exposure_group.setLayout(form_layout)
        layout.addWidget(exposure_group)

    def create_slit_width_offset_section(self, layout):
        """Create a section for slit width offset display"""
        slit_group = QGroupBox("Slit Width Offset")
        form_layout = QFormLayout()

        self.slit_offset_label = QLabel("N/A")
        form_layout.addRow("Slit Offset", self.slit_offset_label)

        slit_group.setLayout(form_layout)
        layout.addWidget(slit_group)

    def create_binning_info_section(self, layout):
        """Create a section for spectral or spatial binning"""
        binning_group = QGroupBox("Binning Information")
        form_layout = QFormLayout()

        # ComboBox to choose binning type (Spectral or Spatial)
        self.binning_type_combo = QComboBox()

        # Label to display the current binning information
        self.binning_info_label = QLabel("Spatial")  # Default to Spatial binning
        form_layout.addRow("Binning Info", self.binning_info_label)

        binning_group.setLayout(form_layout)
        layout.addWidget(binning_group)

    def update_binning_info(self):
        """Update the binning info based on selected binning type"""
        selected_binning = self.binning_type_combo.currentText()

        if selected_binning == "Spectral":
            self.binning_info_label.setText("Spectral Binning: 1x1")  # Default spectral binning value
        elif selected_binning == "Spatial":
            self.binning_info_label.setText("Spatial Binning: 1x1")  # Default spatial binning value

    def create_remarks_section(self, layout):
        """Create a section for additional remarks or notes"""
        remarks_group = QGroupBox("Remarks")
        form_layout = QFormLayout()

        self.remarks_text_edit = QTextEdit()
        self.remarks_text_edit.setPlaceholderText("Enter additional remarks here...")
        self.remarks_text_edit.setReadOnly(True)  # Read-only by default
        form_layout.addRow("Additional Remarks", self.remarks_text_edit)

        remarks_group.setLayout(form_layout)
        layout.addWidget(remarks_group)
