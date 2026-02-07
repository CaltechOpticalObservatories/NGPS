"""
OTM Settings Dialog for NGPS GUI

Provides UI for configuring OTM scheduler parameters.
"""

import os
from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import (
    QDialog, QVBoxLayout, QHBoxLayout, QFormLayout, QLabel,
    QLineEdit, QPushButton, QCheckBox, QMessageBox, QFileDialog,
    QDialogButtonBox, QGroupBox, QSpinBox, QDoubleSpinBox
)

from otm_integration import OtmSettings, detect_otm_scripts


class OtmSettingsDialog(QDialog):
    """Dialog for configuring OTM scheduler settings."""

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowTitle("OTM Scheduler Settings")
        self.setMinimumWidth(500)

        # Load existing settings
        self.settings = OtmSettings.load()

        # Build UI
        self._init_ui()

        # Populate fields
        self._load_settings()

    def _init_ui(self):
        """Initialize UI components."""
        layout = QVBoxLayout(self)

        # Observation conditions group
        conditions_group = QGroupBox("Observation Conditions")
        conditions_layout = QFormLayout()

        self.seeing_fwhm_spin = QDoubleSpinBox()
        self.seeing_fwhm_spin.setRange(0.1, 10.0)
        self.seeing_fwhm_spin.setSingleStep(0.1)
        self.seeing_fwhm_spin.setDecimals(2)
        self.seeing_fwhm_spin.setSuffix(" arcsec")
        conditions_layout.addRow("Seeing FWHM:", self.seeing_fwhm_spin)

        self.seeing_pivot_spin = QDoubleSpinBox()
        self.seeing_pivot_spin.setRange(300.0, 1000.0)
        self.seeing_pivot_spin.setSingleStep(10.0)
        self.seeing_pivot_spin.setDecimals(1)
        self.seeing_pivot_spin.setSuffix(" nm")
        conditions_layout.addRow("Seeing Pivot λ:", self.seeing_pivot_spin)

        self.airmass_max_spin = QDoubleSpinBox()
        self.airmass_max_spin.setRange(1.0, 6.0)
        self.airmass_max_spin.setSingleStep(0.1)
        self.airmass_max_spin.setDecimals(1)
        conditions_layout.addRow("Max Airmass:", self.airmass_max_spin)

        self.alt_twilight_spin = QDoubleSpinBox()
        self.alt_twilight_spin.setRange(-18.0, 0.0)
        self.alt_twilight_spin.setSingleStep(1.0)
        self.alt_twilight_spin.setDecimals(1)
        self.alt_twilight_spin.setSuffix(" deg")
        conditions_layout.addRow("Twilight Altitude:", self.alt_twilight_spin)

        conditions_group.setLayout(conditions_layout)
        layout.addWidget(conditions_group)

        # Sky simulation group
        sky_group = QGroupBox("Sky Background")
        sky_layout = QVBoxLayout()

        self.use_sky_sim_check = QCheckBox("Use Rubin Observatory Sky Simulation")
        self.use_sky_sim_check.setToolTip(
            "Enable to use detailed sky background model.\n"
            "Disable for faster scheduling with default sky magnitudes."
        )
        sky_layout.addWidget(self.use_sky_sim_check)

        sky_group.setLayout(sky_layout)
        layout.addWidget(sky_group)

        # Timeline generation group
        timeline_group = QGroupBox("Timeline Generation")
        timeline_layout = QFormLayout()

        self.step_minutes_spin = QDoubleSpinBox()
        self.step_minutes_spin.setRange(1.0, 60.0)
        self.step_minutes_spin.setSingleStep(1.0)
        self.step_minutes_spin.setDecimals(1)
        self.step_minutes_spin.setSuffix(" min")
        self.step_minutes_spin.setToolTip("Time interval for airmass samples in timeline")
        timeline_layout.addRow("Sample Interval:", self.step_minutes_spin)

        timeline_group.setLayout(timeline_layout)
        layout.addWidget(timeline_group)

        # Script paths group
        paths_group = QGroupBox("Script Paths")
        paths_layout = QVBoxLayout()

        # Python command
        python_layout = QHBoxLayout()
        python_layout.addWidget(QLabel("Python Command:"))
        self.python_cmd_edit = QLineEdit()
        self.python_cmd_edit.setPlaceholderText("python3")
        python_layout.addWidget(self.python_cmd_edit, 1)
        paths_layout.addLayout(python_layout)

        # OTM script path
        otm_layout = QHBoxLayout()
        otm_layout.addWidget(QLabel("OTM Script:"))
        self.otm_script_edit = QLineEdit()
        self.otm_script_edit.setPlaceholderText("/path/to/Python/OTM/OTM.py")
        otm_layout.addWidget(self.otm_script_edit, 1)
        self.otm_browse_button = QPushButton("Browse...")
        self.otm_browse_button.clicked.connect(self._browse_otm_script)
        otm_layout.addWidget(self.otm_browse_button)
        paths_layout.addLayout(otm_layout)

        # Timeline script path
        timeline_layout = QHBoxLayout()
        timeline_layout.addWidget(QLabel("Timeline Script:"))
        self.timeline_script_edit = QLineEdit()
        self.timeline_script_edit.setPlaceholderText("/path/to/Python/OTM/otm_timeline.py")
        timeline_layout.addWidget(self.timeline_script_edit, 1)
        self.timeline_browse_button = QPushButton("Browse...")
        self.timeline_browse_button.clicked.connect(self._browse_timeline_script)
        timeline_layout.addWidget(self.timeline_browse_button)
        paths_layout.addLayout(timeline_layout)

        # Auto-detect button
        auto_detect_button = QPushButton("Auto-Detect Scripts")
        auto_detect_button.clicked.connect(self._auto_detect_scripts)
        paths_layout.addWidget(auto_detect_button)

        paths_group.setLayout(paths_layout)
        layout.addWidget(paths_group)

        # Dialog buttons
        button_box = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        button_box.accepted.connect(self.accept)
        button_box.rejected.connect(self.reject)
        layout.addWidget(button_box)

    def _load_settings(self):
        """Load settings into UI fields."""
        self.seeing_fwhm_spin.setValue(self.settings.seeing_fwhm)
        self.seeing_pivot_spin.setValue(self.settings.seeing_pivot)
        self.airmass_max_spin.setValue(self.settings.airmass_max)
        self.alt_twilight_spin.setValue(self.settings.alt_twilight)
        self.use_sky_sim_check.setChecked(self.settings.use_sky_sim)
        self.step_minutes_spin.setValue(self.settings.step_minutes)
        self.python_cmd_edit.setText(self.settings.python_cmd)
        self.otm_script_edit.setText(self.settings.otm_script_path)
        self.timeline_script_edit.setText(self.settings.timeline_script_path)

    def _save_settings(self):
        """Save UI fields to settings."""
        self.settings.seeing_fwhm = self.seeing_fwhm_spin.value()
        self.settings.seeing_pivot = self.seeing_pivot_spin.value()
        self.settings.airmass_max = self.airmass_max_spin.value()
        self.settings.alt_twilight = self.alt_twilight_spin.value()
        self.settings.use_sky_sim = self.use_sky_sim_check.isChecked()
        self.settings.step_minutes = self.step_minutes_spin.value()
        self.settings.python_cmd = self.python_cmd_edit.text()
        self.settings.otm_script_path = self.otm_script_edit.text()
        self.settings.timeline_script_path = self.timeline_script_edit.text()

    def _browse_otm_script(self):
        """Browse for OTM script."""
        current_path = self.otm_script_edit.text()
        if not current_path:
            current_path = os.path.expanduser("~")

        path, _ = QFileDialog.getOpenFileName(
            self,
            "Select OTM Script",
            current_path,
            "Python Scripts (*.py);;All Files (*)"
        )

        if path:
            self.otm_script_edit.setText(path)

    def _browse_timeline_script(self):
        """Browse for timeline script."""
        current_path = self.timeline_script_edit.text()
        if not current_path:
            current_path = os.path.expanduser("~")

        path, _ = QFileDialog.getOpenFileName(
            self,
            "Select Timeline Script",
            current_path,
            "Python Scripts (*.py);;All Files (*)"
        )

        if path:
            self.timeline_script_edit.setText(path)

    def _auto_detect_scripts(self):
        """Auto-detect OTM script paths."""
        try:
            otm_script, timeline_script = detect_otm_scripts()

            if os.path.exists(otm_script):
                self.otm_script_edit.setText(otm_script)
            if os.path.exists(timeline_script):
                self.timeline_script_edit.setText(timeline_script)

            if os.path.exists(otm_script) and os.path.exists(timeline_script):
                QMessageBox.information(
                    self,
                    "Scripts Found",
                    f"Found OTM scripts:\n\n{otm_script}\n{timeline_script}"
                )
            else:
                missing = []
                if not os.path.exists(otm_script):
                    missing.append(f"OTM.py not found at: {otm_script}")
                if not os.path.exists(timeline_script):
                    missing.append(f"otm_timeline.py not found at: {timeline_script}")

                QMessageBox.warning(
                    self,
                    "Scripts Not Found",
                    "\n".join(missing) + "\n\nPlease browse manually."
                )

        except Exception as e:
            QMessageBox.critical(
                self,
                "Error",
                f"Failed to auto-detect scripts: {e}"
            )

    def accept(self):
        """Validate and save settings before closing."""
        # Validate script paths
        otm_path = self.otm_script_edit.text()
        timeline_path = self.timeline_script_edit.text()

        if not otm_path or not os.path.exists(otm_path):
            QMessageBox.warning(
                self,
                "Invalid Path",
                "Please specify a valid OTM script path."
            )
            return

        if not timeline_path or not os.path.exists(timeline_path):
            QMessageBox.warning(
                self,
                "Invalid Path",
                "Please specify a valid timeline script path."
            )
            return

        # Save settings
        self._save_settings()
        self.settings.save()

        super().accept()

    def get_settings(self) -> OtmSettings:
        """Get the current settings."""
        return self.settings
