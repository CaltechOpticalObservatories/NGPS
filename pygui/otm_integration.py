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
import logging
import tempfile
from dataclasses import dataclass, field
from typing import List, Dict, Any, Optional, Tuple
from datetime import datetime, timedelta

from coordinate_utils import parse_sexagesimal_dec

logger = logging.getLogger(__name__)

# Palomar Observatory declination limits (degrees)
PALOMAR_DEC_MIN = -40.0
PALOMAR_DEC_MAX = 89.0

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
    etc_flags: str = "-noslicer -fastSNR"  # Extra flags passed to ETC

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
        settings.setValue("etc_flags", self.etc_flags)

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
            step_minutes=float(settings.value("step_minutes", 5.0)),
            etc_flags=settings.value("etc_flags", "-noslicer -fastSNR")
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
    """OTM output data for a single target.

    Field names match OTM output CSV columns. Use csv_to_db_mapping()
    to get the mapping from these fields to database column names.
    """
    observation_id: str
    otmstart: str = ""        # ISO timestamp → DB: OTMexp_start
    otmend: str = ""          # ISO timestamp → DB: OTMexp_end
    otmslewgo: str = ""       # ISO timestamp → DB: OTMslewgo
    otmflag: str = ""         # Status flag   → DB: OTMflag
    otmlast: str = ""         # Last flag     → DB: OTMlast
    otmwait: str = ""         # Wait time     → DB: OTMwait
    otmdead: str = ""         # Dead time     → DB: OTMdead
    otmslew: str = ""         # Slew time     → DB: OTMslew
    otmexptime: str = ""      # Exposure time → DB: OTMexpt
    otmslitwidth: str = ""    # Slit width    → DB: OTMslitwidth
    otmpa: str = ""           # Position angle→ DB: OTMpa
    otmslitangle: str = ""    # Slit angle    → DB: OTMslitangle
    otmcass: str = ""         # Cass angle    → DB: OTMcass
    otmsnr: str = ""          # SNR           → DB: OTMSNR
    otmres: str = ""          # Resolution    → DB: OTMres
    otmseeing: str = ""       # Seeing        → DB: OTMseeing
    otmairmass_start: str = ""  # Airmass start → DB: OTMairmass_start
    otmairmass_end: str = ""    # Airmass end   → DB: OTMairmass_end
    otmsky: str = ""          # Sky mag       → DB: OTMsky
    otmmoon: str = ""         # Moon          → DB: OTMmoon
    obs_order: int = 0        # New order     → DB: OBS_ORDER


# Mapping from OTM output CSV column names to database column names.
# Based on C++ addUpdate() calls in main.cpp lines 7269-7288.
OTM_CSV_TO_DB = {
    "OTMstart":        "OTMexp_start",
    "OTMend":          "OTMexp_end",
    "OTMexptime":      "OTMexpt",
    "OTMslitwidth":    "OTMslitwidth",
    "OTMpa":           "OTMpa",
    "OTMslitangle":    "OTMslitangle",
    "OTMcass":         "OTMcass",
    "OTMwait":         "OTMwait",
    "OTMflag":         "OTMflag",
    "OTMlast":         "OTMlast",
    "OTMslewgo":       "OTMslewgo",
    "OTMslew":         "OTMslew",
    "OTMdead":         "OTMdead",
    "OTMairmass_start":"OTMairmass_start",
    "OTMairmass_end":  "OTMairmass_end",
    "OTMsky":          "OTMsky",
    "OTMmoon":         "OTMmoon",
    "OTMSNR":          "OTMSNR",
    "OTMres":          "OTMres",
    "OTMseeing":       "OTMseeing",
}


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

            if self.settings.etc_flags and self.settings.etc_flags.strip():
                args.append("-etc_flags=" + self.settings.etc_flags.strip())

            self.progress.emit(f"Executing: {' '.join(args)}")

            # Run subprocess
            self._process = subprocess.Popen(
                args,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True
            )

            # Capture all stdout for error reporting
            stdout_lines = []
            while True:
                line = self._process.stdout.readline()
                if not line:
                    break
                stdout_lines.append(line.strip())
                self.progress.emit(line.strip())

            # Wait for completion
            self._process.wait()

            # Check result
            if self._process.returncode == 0:
                self.progress.emit("OTM scheduler completed successfully")
                self.finished.emit(True, "OTM scheduler completed")
            else:
                stderr = self._process.stderr.read()
                stdout_tail = "\n".join(stdout_lines[-20:]) if stdout_lines else "(no stdout)"
                error_msg = (
                    f"OTM scheduler failed with exit code {self._process.returncode}\n\n"
                    f"--- stdout (last 20 lines) ---\n{stdout_tail}\n\n"
                    f"--- stderr ---\n{stderr}"
                )
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


def _normalize_slitwidth(val: str) -> str:
    """Normalize slitwidth value (matches C++ normalizeSlitwidthValue).

    Bare numbers become 'SET <num>'. Valid modes: SET, SNR, RES, LOSS, AUTO.
    """
    v = val.strip()
    if not v:
        return "SET 1.0"
    parts = v.split()
    if len(parts) == 1:
        upper = parts[0].upper()
        if upper == "AUTO":
            return "AUTO"
        try:
            num = float(parts[0])
            if num > 0:
                return f"SET {num}"
        except ValueError:
            pass
        return "SET 1.0"
    key = parts[0].upper()
    if key == "AUTO":
        return "AUTO"
    if key in ("SET", "SNR", "RES", "LOSS"):
        try:
            num = float(parts[1])
            if num > 0:
                return f"{key} {num}"
        except (ValueError, IndexError):
            pass
    return "SET 1.0"


def _normalize_exptime(val: str) -> str:
    """Normalize exptime value (matches C++ normalizeExptimeValue).

    Bare numbers become 'SET <num>'. 'EXPTIME' keyword treated as 'SET'.
    """
    v = val.strip()
    if not v:
        return "SET 5"
    parts = v.split()
    if len(parts) == 1:
        try:
            num = float(parts[0])
            if num > 0:
                return f"SET {num}"
        except ValueError:
            pass
        return "SET 5"
    key = parts[0].upper()
    if key == "EXPTIME":
        key = "SET"
    if key in ("SET", "SNR"):
        try:
            num = float(parts[1])
            if num > 0:
                return f"{key} {num}"
        except (ValueError, IndexError):
            pass
    return "SET 5"


def _normalize_magsystem(val: str) -> str:
    """Normalize magsystem value (matches C++ normalizeMagsystemValue)."""
    v = val.strip().upper()
    if v in ("AB", "VEGA"):
        return v
    return "AB"


def _normalize_magfilter(val: str) -> str:
    """Normalize magfilter value (matches C++ normalizeMagfilterValue)."""
    v = val.strip()
    if not v:
        return "match"
    upper = v.upper()
    if upper in ("G", "MATCH", "--"):
        return "match"
    if upper == "USER":
        return "user"
    if upper in ("U", "B", "V", "R", "I", "J", "K"):
        return upper
    return "match"


# Default wavelength range center by channel (nm)
_CHANNEL_CENTER_NM = {"U": 380.0, "G": 475.0, "R": 635.0, "I": 830.0}
_DEFAULT_WRANGE_HALF_NM = 15.0  # +/- 150 Angstrom


def _default_wrange(channel: str) -> Tuple[float, float]:
    """Get default wavelength range for a channel."""
    center = _CHANNEL_CENTER_NM.get(channel.strip().upper(), _CHANNEL_CENTER_NM["R"])
    return (center - _DEFAULT_WRANGE_HALF_NM, center + _DEFAULT_WRANGE_HALF_NM)


def generate_otm_input_csv(targets: List[Dict[str, Any]], output_path: str,
                           dec_min: float = PALOMAR_DEC_MIN,
                           dec_max: float = PALOMAR_DEC_MAX) -> List[str]:
    """
    Generate OTM input CSV from target data.

    Targets with declinations outside the observable range are filtered out.
    Column names and value normalization match the C++ implementation.

    Args:
        targets: List of target dictionaries from database
        output_path: Path to write CSV file
        dec_min: Minimum declination in degrees (default: Palomar limit)
        dec_max: Maximum declination in degrees (default: Palomar limit)

    Returns:
        List of warning messages for skipped targets
    """
    # OTM CSV header - must match what OTM.py expects
    header = [
        'OBSERVATION_ID', 'name', 'RA', 'DECL', 'slitangle', 'slitwidth',
        'exptime', 'notbefore', 'pointmode', 'ccdmode', 'airmass_max',
        'binspat', 'binspect', 'channel', 'wrange', 'mag', 'magsystem',
        'magfilter', 'srcmodel'
    ]

    rows = []
    skipped = []
    for target in targets:
        # Extract values with defaults matching C++ implementation
        obs_id = target.get('OBSERVATION_ID', '')
        name = target.get('NAME', '')
        ra = target.get('RA', '')
        decl = target.get('DECL', '')
        slitangle = target.get('SLITANGLE', '') or 'PA'
        slitwidth = _normalize_slitwidth(str(target.get('SLITWIDTH', '') or ''))
        exptime = _normalize_exptime(str(target.get('EXPTIME', '') or ''))
        notbefore = target.get('NOTBEFORE', '') or '1999-12-31T12:34:56'
        pointmode = target.get('POINTMODE', '') or 'SLIT'
        ccdmode = target.get('CCDMODE', '') or 'default'
        airmass_max = target.get('AIRMASS_MAX', '') or '4.0'
        binspat = target.get('BINSPAT', '') or '1'
        binspect = target.get('BINSPECT', '') or '1'
        channel = target.get('CHANNEL', '') or 'R'
        srcmodel = target.get('SRCMODEL', '') or ''

        # Filter by declination range
        dec_deg = parse_sexagesimal_dec(str(decl)) if decl else None
        if dec_deg is not None and (dec_deg < dec_min or dec_deg > dec_max):
            msg = f"Skipped {name} (obs {obs_id}): DEC {decl} ({dec_deg:.2f} deg) outside [{dec_min}, {dec_max}]"
            logger.warning(msg)
            skipped.append(msg)
            continue

        # Combine WRANGE_LOW and WRANGE_HIGH into single "wrange" column
        try:
            low = float(target.get('WRANGE_LOW', 0) or 0)
            high = float(target.get('WRANGE_HIGH', 0) or 0)
            if low <= 0 or high <= 0 or high <= low:
                low, high = _default_wrange(channel)
        except (ValueError, TypeError):
            low, high = _default_wrange(channel)
        wrange = f"{low} {high}"

        # Normalize magnitude fields to match OTM expected names/values
        mag_raw = str(target.get('MAGNITUDE', '') or '')
        try:
            mag = str(float(mag_raw)) if mag_raw else '19.0'
        except ValueError:
            mag = '19.0'
        magsystem = _normalize_magsystem(str(target.get('MAGSYSTEM', '') or ''))
        magfilter = _normalize_magfilter(str(target.get('MAGFILTER', '') or ''))

        row = [
            obs_id, name, ra, decl, slitangle, slitwidth, exptime,
            notbefore, pointmode, ccdmode, airmass_max, binspat, binspect,
            channel, wrange, mag, magsystem, magfilter, srcmodel
        ]
        rows.append(row)

    # Write CSV
    with open(output_path, 'w', newline='') as f:
        writer = csv.writer(f)
        writer.writerow(header)
        writer.writerows(rows)

    return skipped


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
                # Build case-insensitive lookup
                row_ci = {k.strip(): v.strip() if v else '' for k, v in row.items()}
                # Also build lowercase version for fallback
                row_lower = {k.lower(): v for k, v in row_ci.items()}

                def get(key):
                    """Get value by key, trying exact case first then lowercase."""
                    val = row_ci.get(key, '')
                    if not val:
                        val = row_lower.get(key.lower(), '')
                    if val and val.lower() == 'none':
                        return ''
                    return val

                result = OtmResult(
                    observation_id=get('OBSERVATION_ID'),
                    otmstart=get('OTMstart'),
                    otmend=get('OTMend'),
                    otmslewgo=get('OTMslewgo'),
                    otmflag=get('OTMflag'),
                    otmlast=get('OTMlast'),
                    otmwait=get('OTMwait'),
                    otmdead=get('OTMdead'),
                    otmslew=get('OTMslew'),
                    otmexptime=get('OTMexptime'),
                    otmslitwidth=get('OTMslitwidth'),
                    otmpa=get('OTMpa'),
                    otmslitangle=get('OTMslitangle'),
                    otmcass=get('OTMcass'),
                    otmsnr=get('OTMSNR'),
                    otmres=get('OTMres'),
                    otmseeing=get('OTMseeing'),
                    otmairmass_start=get('OTMairmass_start'),
                    otmairmass_end=get('OTMairmass_end'),
                    otmsky=get('OTMsky'),
                    otmmoon=get('OTMmoon'),
                    obs_order=idx,
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
