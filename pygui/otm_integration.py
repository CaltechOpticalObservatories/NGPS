"""
OTM (Optimal Target scheduler) Integration for NGPS GUI

Provides integration with Python/OTM/ scripts for:
- Generating OTM input CSV from database targets
- Running OTM scheduler as subprocess
- Parsing OTM output and timeline JSON
- Importing OTM results back to database

Based on C++ implementation in tools/ngps_db_gui/main.cpp
"""

import os
import sys
import subprocess
import json
import csv
import tempfile
from dataclasses import dataclass, field
from typing import List, Dict, Any, Optional, Tuple
from datetime import datetime, timedelta

from PyQt5.QtCore import Qt, QObject, pyqtSignal, QThread, QSettings
from PyQt5.QtWidgets import (
    QDialog, QVBoxLayout, QHBoxLayout, QFormLayout, QLabel,
    QLineEdit, QPushButton, QCheckBox, QMessageBox, QProgressDialog,
    QApplication, QDialogButtonBox, QGroupBox, QSpinBox, QDoubleSpinBox
)


# Settings
SETTINGS_ORG = "NGPS"
SETTINGS_APP = "ngps_gui_otm"


@dataclass
class OtmSettings:
    """OTM scheduler configuration settings."""
    seeing_fwhm: float = 1.1  # FWHM in arcsec at zenith
    seeing_pivot: float = 500.0  # Reference wavelength in nm
    airmass_max: float = 4.0  # Maximum airmass limit
    alt_twilight: float = -12.0  # Sun altitude defining twilight (degrees)
    use_sky_sim: bool = True  # Use Rubin sky background simulation
    python_cmd: str = "python3"  # Python executable path
    otm_script_path: str = ""  # Path to OTM.py
    timeline_script_path: str = ""  # Path to otm_timeline.py
    step_minutes: float = 5.0  # Minutes per airmass sample

    def save(self) -> None:
        """Save settings to QSettings."""
        settings = QSettings(SETTINGS_ORG, SETTINGS_APP)
        settings.setValue("seeing_fwhm", self.seeing_fwhm)
        settings.setValue("seeing_pivot", self.seeing_pivot)
        settings.setValue("airmass_max", self.airmass_max)
        settings.setValue("alt_twilight", self.alt_twilight)
        settings.setValue("use_sky_sim", self.use_sky_sim)
        settings.setValue("python_cmd", self.python_cmd)
        settings.setValue("otm_script_path", self.otm_script_path)
        settings.setValue("timeline_script_path", self.timeline_script_path)
        settings.setValue("step_minutes", self.step_minutes)

    @staticmethod
    def load() -> 'OtmSettings':
        """Load settings from QSettings."""
        settings = QSettings(SETTINGS_ORG, SETTINGS_APP)
        return OtmSettings(
            seeing_fwhm=float(settings.value("seeing_fwhm", 1.1)),
            seeing_pivot=float(settings.value("seeing_pivot", 500.0)),
            airmass_max=float(settings.value("airmass_max", 4.0)),
            alt_twilight=float(settings.value("alt_twilight", -12.0)),
            use_sky_sim=settings.value("use_sky_sim", True, type=bool),
            python_cmd=settings.value("python_cmd", "python3"),
            otm_script_path=settings.value("otm_script_path", ""),
            timeline_script_path=settings.value("timeline_script_path", ""),
            step_minutes=float(settings.value("step_minutes", 5.0))
        )


@dataclass
class OtmTarget:
    """Target data for OTM input CSV."""
    observation_id: str
    name: str
    ra: str  # Sexagesimal format (HH:MM:SS.ss)
    decl: str  # Sexagesimal format (+DD:MM:SS.ss)
    slitangle: str  # Degrees or "PA" for parallactic angle
    slitwidth: str  # Format: "SET 1.5" or "SNR 10" or "AUTO"
    exptime: str  # Format: "SET 300" or "SNR 5"
    notbefore: str = ""  # ISO format timestamp
    pointmode: str = "SLIT"  # "SLIT" or "ACAM"
    ccdmode: str = "default"
    airmass_max: float = 4.0
    binspat: int = 1
    binspect: int = 1
    channel: str = "R"
    wrange_low: float = 650.0
    wrange_high: float = 680.0
    magnitude: float = 18.0
    magfilter: str = "G"
    magsystem: str = "AB"
    srcmodel: str = ""
    nexp: int = 1  # Number of exposures (repeat row this many times)


@dataclass
class OtmResult:
    """OTM output data for a single target."""
    observation_id: str
    otmstart: str  # ISO timestamp
    otmend: str  # ISO timestamp
    otmslewgo: str  # ISO timestamp
    otmflag: str  # Status flag (e.g., "DAY-0", "DAY-1", "SKY", etc.)
    otmwait: float  # Wait time in seconds
    otmdead: float  # Dead time in seconds
    otmslew: float  # Slew time in seconds
    obs_order: int  # New order based on schedule


@dataclass
class TimelineTarget:
    """Timeline visualization data for a single target."""
    obs_id: str
    name: str
    ra: str
    dec: str
    start: str  # ISO timestamp
    end: str  # ISO timestamp
    slew_go: str  # ISO timestamp
    flag: str
    severity: int  # 0=none, 1=warning, 2=error
    wait_sec: float
    observed: bool
    airmass: List[float] = field(default_factory=list)
    obs_order: int = 0


@dataclass
class TimelineData:
    """Complete timeline visualization data."""
    times_utc: List[str] = field(default_factory=list)  # ISO timestamps for airmass grid
    twilight_evening_16: str = ""
    twilight_evening_12: str = ""
    twilight_morning_12: str = ""
    twilight_morning_16: str = ""
    targets: List[TimelineTarget] = field(default_factory=list)
    airmass_limit: float = 4.0
    delay_slew_sec: float = 0.0
    idle_intervals: List[Tuple[str, str]] = field(default_factory=list)  # (start, end) tuples


class OtmRunner(QObject):
    """Worker thread for running OTM scheduler."""

    progress = pyqtSignal(str)  # Progress message
    finished = pyqtSignal(bool, str)  # Success flag, message

    def __init__(self, settings: OtmSettings, input_csv: str, output_csv: str, start_utc: str):
        super().__init__()
        self.settings = settings
        self.input_csv = input_csv
        self.output_csv = output_csv
        self.start_utc = start_utc
        self._process = None

    def run(self) -> None:
        """Run OTM scheduler subprocess."""
        try:
            self.progress.emit("Starting OTM scheduler...")

            # Build command
            args = [
                self.settings.python_cmd,
                self.settings.otm_script_path,
                self.input_csv,
                self.start_utc,
                "-seeing", str(self.settings.seeing_fwhm), str(self.settings.seeing_pivot),
                "-airmass_max", str(self.settings.airmass_max),
                "-alt_twilight", str(self.settings.alt_twilight),
                "-out", self.output_csv
            ]

            if not self.settings.use_sky_sim:
                args.append("-noskysim")

            self.progress.emit(f"Executing: {' '.join(args)}")

            # Run subprocess
            self._process = subprocess.Popen(
                args,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True
            )

            # Stream output
            while True:
                line = self._process.stdout.readline()
                if not line:
                    break
                self.progress.emit(line.strip())

            # Wait for completion
            self._process.wait()

            # Check result
            if self._process.returncode == 0:
                self.progress.emit("OTM scheduler completed successfully")
                self.finished.emit(True, "OTM scheduler completed")
            else:
                stderr = self._process.stderr.read()
                error_msg = f"OTM scheduler failed with exit code {self._process.returncode}\n{stderr}"
                self.progress.emit(error_msg)
                self.finished.emit(False, error_msg)

        except Exception as e:
            error_msg = f"Error running OTM scheduler: {e}"
            self.progress.emit(error_msg)
            self.finished.emit(False, error_msg)

    def cancel(self) -> None:
        """Cancel the running process."""
        if self._process and self._process.poll() is None:
            self._process.terminate()
            self.progress.emit("OTM scheduler cancelled")


class OtmTimelineRunner(QObject):
    """Worker thread for generating timeline JSON."""

    progress = pyqtSignal(str)
    finished = pyqtSignal(bool, str, str)  # Success, message, JSON path

    def __init__(self, settings: OtmSettings, input_csv: str, output_csv: str,
                 json_path: str, start_utc: str):
        super().__init__()
        self.settings = settings
        self.input_csv = input_csv
        self.output_csv = output_csv
        self.json_path = json_path
        self.start_utc = start_utc
        self._process = None

    def run(self) -> None:
        """Run timeline generation subprocess."""
        try:
            self.progress.emit("Generating timeline JSON...")

            # Build command
            args = [
                self.settings.python_cmd,
                self.settings.timeline_script_path,
                "--input", self.input_csv,
                "--output", self.output_csv,
                "--json", self.json_path,
                "--step", str(self.settings.step_minutes)
            ]

            if self.start_utc:
                args.extend(["--start", self.start_utc])

            self.progress.emit(f"Executing: {' '.join(args)}")

            # Run subprocess
            self._process = subprocess.Popen(
                args,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True
            )

            # Wait for completion
            stdout, stderr = self._process.communicate()

            # Check result
            if self._process.returncode == 0:
                self.progress.emit("Timeline JSON generated successfully")
                self.finished.emit(True, "Timeline generated", self.json_path)
            else:
                error_msg = f"Timeline generation failed: {stderr}"
                self.progress.emit(error_msg)
                self.finished.emit(False, error_msg, "")

        except Exception as e:
            error_msg = f"Error generating timeline: {e}"
            self.progress.emit(error_msg)
            self.finished.emit(False, error_msg, "")

    def cancel(self) -> None:
        """Cancel the running process."""
        if self._process and self._process.poll() is None:
            self._process.terminate()
            self.progress.emit("Timeline generation cancelled")


def generate_otm_input_csv(targets: List[Dict[str, Any]], output_path: str) -> None:
    """
    Generate OTM input CSV from target data.

    Args:
        targets: List of target dictionaries from database
        output_path: Path to write CSV file
    """
    # Define CSV header
    header = [
        'OBSERVATION_ID', 'name', 'RA', 'DECL', 'slitangle', 'slitwidth',
        'exptime', 'notbefore', 'pointmode', 'ccdmode', 'airmass_max',
        'binspat', 'binspect', 'channel', 'wrange_low', 'wrange_high',
        'magnitude', 'magfilter', 'magsystem', 'srcmodel'
    ]

    rows = []
    for target in targets:
        # Extract values with defaults
        obs_id = target.get('OBSERVATION_ID', '')
        name = target.get('NAME', '')
        ra = target.get('RA', '')
        decl = target.get('DECL', '')
        slitangle = target.get('SLITANGLE', 'PA')
        slitwidth = target.get('SLITWIDTH', 'SET 1.5')
        exptime = target.get('EXPTIME', 'SET 300')
        notbefore = target.get('NOTBEFORE', '')
        pointmode = target.get('POINTMODE', 'SLIT')
        ccdmode = 'default'
        airmass_max = target.get('AIRMASS_MAX', '4.0')
        binspat = target.get('BINSPAT', '1')
        binspect = target.get('BINSPECT', '1')
        channel = target.get('CHANNEL', 'R')
        wrange_low = target.get('WRANGE_LOW', '650')
        wrange_high = target.get('WRANGE_HIGH', '680')
        magnitude = target.get('MAGNITUDE', '18.0')
        magfilter = target.get('MAGFILTER', 'G')
        magsystem = target.get('MAGSYSTEM', 'AB')
        srcmodel = target.get('SRCMODEL', '')

        row = [
            obs_id, name, ra, decl, slitangle, slitwidth, exptime,
            notbefore, pointmode, ccdmode, airmass_max, binspat, binspect,
            channel, wrange_low, wrange_high, magnitude, magfilter, magsystem, srcmodel
        ]
        rows.append(row)

    # Write CSV
    with open(output_path, 'w', newline='') as f:
        writer = csv.writer(f)
        writer.writerow(header)
        writer.writerows(rows)


def parse_otm_output_csv(output_path: str) -> List[OtmResult]:
    """
    Parse OTM output CSV and extract results.

    Args:
        output_path: Path to OTM output CSV file

    Returns:
        List of OtmResult objects
    """
    results = []

    try:
        with open(output_path, 'r', newline='') as f:
            reader = csv.DictReader(f)
            for idx, row in enumerate(reader):
                # Normalize keys (case-insensitive)
                row_lower = {k.strip().lower(): v for k, v in row.items()}

                result = OtmResult(
                    observation_id=row_lower.get('observation_id', ''),
                    otmstart=row_lower.get('otmstart', ''),
                    otmend=row_lower.get('otmend', ''),
                    otmslewgo=row_lower.get('otmslewgo', ''),
                    otmflag=row_lower.get('otmflag', ''),
                    otmwait=float(row_lower.get('otmwait', 0.0) or 0.0),
                    otmdead=float(row_lower.get('otmdead', 0.0) or 0.0),
                    otmslew=float(row_lower.get('otmslew', 0.0) or 0.0),
                    obs_order=idx
                )
                results.append(result)

    except Exception as e:
        raise RuntimeError(f"Failed to parse OTM output CSV: {e}")

    return results


def parse_timeline_json(json_path: str) -> TimelineData:
    """
    Parse timeline JSON file.

    Args:
        json_path: Path to timeline JSON file

    Returns:
        TimelineData object
    """
    try:
        with open(json_path, 'r') as f:
            data = json.load(f)

        timeline = TimelineData()
        timeline.times_utc = data.get('times_utc', [])
        timeline.airmass_limit = data.get('airmass_limit', 4.0)
        timeline.delay_slew_sec = data.get('delay_slew_sec', 0.0)

        # Parse twilight times
        twilight = data.get('twilight', {})
        timeline.twilight_evening_16 = twilight.get('evening_16', '')
        timeline.twilight_evening_12 = twilight.get('evening_12', '')
        timeline.twilight_morning_12 = twilight.get('morning_12', '')
        timeline.twilight_morning_16 = twilight.get('morning_16', '')

        # Parse targets
        for target_data in data.get('targets', []):
            flag = target_data.get('flag', '')
            severity = otm_flag_severity(flag)

            target = TimelineTarget(
                obs_id=target_data.get('obs_id', ''),
                name=target_data.get('name', ''),
                ra=target_data.get('ra', ''),
                dec=target_data.get('dec', ''),
                start=target_data.get('start', ''),
                end=target_data.get('end', ''),
                slew_go=target_data.get('slew_go', ''),
                flag=flag,
                severity=severity,
                wait_sec=target_data.get('wait_sec', 0.0),
                observed=target_data.get('observed', False),
                airmass=target_data.get('airmass', []),
                obs_order=target_data.get('obs_order', 0)
            )
            timeline.targets.append(target)

        # Parse idle intervals
        for interval in data.get('idle_intervals', []):
            timeline.idle_intervals.append((interval.get('start', ''), interval.get('end', '')))

        return timeline

    except Exception as e:
        raise RuntimeError(f"Failed to parse timeline JSON: {e}")


def otm_flag_severity(flag: str) -> int:
    """
    Determine severity level of OTM flag.

    Args:
        flag: OTM flag string

    Returns:
        0 = no flag (ok)
        1 = warning
        2 = error
    """
    if not flag or flag.strip() == "":
        return 0

    flag_upper = flag.strip().upper()

    # Severity 2 (error): DAY-0-1, DAY-1, or flags ending with "1"
    if "DAY-0-1" in flag_upper or "DAY-1" in flag_upper:
        return 2
    if flag_upper.endswith("1"):
        return 2

    # Severity 1 (warning): Any other non-empty flag
    return 1


def otm_flag_color(severity: int) -> str:
    """
    Get color for OTM flag based on severity.

    Args:
        severity: Flag severity (0, 1, or 2)

    Returns:
        HTML color code
    """
    if severity == 2:
        return "#DC4646"  # Red
    elif severity == 1:
        return "#FFA500"  # Orange
    else:
        return "#000000"  # Black


def detect_otm_scripts(ngps_root: Optional[str] = None) -> Tuple[str, str]:
    """
    Auto-detect OTM script paths.

    Args:
        ngps_root: NGPS root directory (or None to use $NGPS_ROOT)

    Returns:
        Tuple of (otm_script_path, timeline_script_path)
    """
    if ngps_root is None:
        ngps_root = os.environ.get('NGPS_ROOT', '')

    if not ngps_root:
        # Try to detect from current file location
        ngps_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

    otm_script = os.path.join(ngps_root, "Python", "OTM", "OTM.py")
    timeline_script = os.path.join(ngps_root, "Python", "OTM", "otm_timeline.py")

    return otm_script, timeline_script
