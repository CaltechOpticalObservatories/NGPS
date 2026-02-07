"""
Coordinate Utilities for NGPS Database GUI

Provides gnomonic (tangent plane) projection and angular separation
calculations for target grouping.

Based on C++ implementation in tools/ngps_db_gui/main.cpp
"""

import math
from typing import Tuple, Optional
from dataclasses import dataclass


# Constants
DEG_TO_RAD = math.pi / 180.0
RAD_TO_DEG = 180.0 / math.pi
ARCSEC_TO_DEG = 1.0 / 3600.0
DEG_TO_ARCSEC = 3600.0

# Default grouping tolerance (arcseconds)
DEFAULT_GROUP_TOLERANCE_ARCSEC = 1.0


@dataclass
class SkyCoordinate:
    """Sky coordinate in degrees."""
    ra_deg: float  # Right Ascension in degrees [0, 360)
    dec_deg: float  # Declination in degrees [-90, 90]


def parse_sexagesimal_ra(ra_str: str) -> Optional[float]:
    """
    Parse sexagesimal RA string to degrees.

    Formats supported:
    - HH:MM:SS.sss
    - HH MM SS.sss
    - Decimal degrees

    Args:
        ra_str: RA string

    Returns:
        RA in degrees [0, 360), or None if parse fails
    """
    try:
        # Try decimal degrees first
        ra_deg = float(ra_str)
        return ra_deg % 360.0
    except ValueError:
        pass

    # Try sexagesimal
    try:
        # Replace spaces with colons
        ra_str = ra_str.strip().replace(" ", ":")
        parts = ra_str.split(":")

        if len(parts) != 3:
            return None

        hours = float(parts[0])
        minutes = float(parts[1])
        seconds = float(parts[2])

        # Convert to degrees (15 degrees per hour)
        ra_deg = (hours + minutes / 60.0 + seconds / 3600.0) * 15.0

        return ra_deg % 360.0

    except (ValueError, IndexError):
        return None


def parse_sexagesimal_dec(dec_str: str) -> Optional[float]:
    """
    Parse sexagesimal DEC string to degrees.

    Formats supported:
    - +DD:MM:SS.sss
    - +DD MM SS.sss
    - Decimal degrees

    Args:
        dec_str: DEC string

    Returns:
        DEC in degrees [-90, 90], or None if parse fails
    """
    try:
        # Try decimal degrees first
        dec_deg = float(dec_str)
        return max(-90.0, min(90.0, dec_deg))
    except ValueError:
        pass

    # Try sexagesimal
    try:
        # Replace spaces with colons
        dec_str = dec_str.strip().replace(" ", ":")

        # Determine sign
        sign = 1.0
        if dec_str.startswith("-"):
            sign = -1.0
            dec_str = dec_str[1:]
        elif dec_str.startswith("+"):
            dec_str = dec_str[1:]

        parts = dec_str.split(":")

        if len(parts) != 3:
            return None

        degrees = float(parts[0])
        arcmin = float(parts[1])
        arcsec = float(parts[2])

        # Convert to degrees
        dec_deg = sign * (degrees + arcmin / 60.0 + arcsec / 3600.0)

        return max(-90.0, min(90.0, dec_deg))

    except (ValueError, IndexError):
        return None


def gnomonic_projection(
    ra0_deg: float,
    dec0_deg: float,
    offset_ra_arcsec: float,
    offset_dec_arcsec: float
) -> Tuple[float, float]:
    """
    Apply gnomonic (tangent plane) projection to compute science coordinates.

    Given a base position (RA0, DEC0) and offsets in arcseconds, compute the
    projected science target position.

    This is used for multi-position observations where the target has an
    offset from the base coordinates.

    Args:
        ra0_deg: Base RA in degrees
        dec0_deg: Base DEC in degrees
        offset_ra_arcsec: RA offset in arcseconds (positive East)
        offset_dec_arcsec: DEC offset in arcseconds (positive North)

    Returns:
        Tuple of (ra1_deg, dec1_deg) - projected coordinates in degrees
    """
    # Convert to radians
    ra0 = ra0_deg * DEG_TO_RAD
    dec0 = dec0_deg * DEG_TO_RAD
    xi = offset_ra_arcsec * ARCSEC_TO_DEG * DEG_TO_RAD  # RA offset in radians
    eta = offset_dec_arcsec * ARCSEC_TO_DEG * DEG_TO_RAD  # DEC offset in radians

    # Gnomonic projection formulas
    sin_dec0 = math.sin(dec0)
    cos_dec0 = math.cos(dec0)

    denom = cos_dec0 - eta * sin_dec0

    # Projected RA
    ra1 = ra0 + math.atan2(xi, denom)

    # Projected DEC
    numerator = sin_dec0 + eta * cos_dec0
    denominator = math.sqrt(denom * denom + xi * xi)
    dec1 = math.atan2(numerator, denominator)

    # Convert back to degrees
    ra1_deg = (ra1 * RAD_TO_DEG) % 360.0
    dec1_deg = dec1 * RAD_TO_DEG

    # Clamp DEC to [-90, 90]
    dec1_deg = max(-90.0, min(90.0, dec1_deg))

    return ra1_deg, dec1_deg


def angular_separation_arcsec(
    ra1_deg: float,
    dec1_deg: float,
    ra2_deg: float,
    dec2_deg: float
) -> float:
    """
    Compute angular separation between two sky positions using haversine formula.

    Args:
        ra1_deg: RA of first position (degrees)
        dec1_deg: DEC of first position (degrees)
        ra2_deg: RA of second position (degrees)
        dec2_deg: DEC of second position (degrees)

    Returns:
        Angular separation in arcseconds
    """
    # Convert to radians
    ra1 = ra1_deg * DEG_TO_RAD
    dec1 = dec1_deg * DEG_TO_RAD
    ra2 = ra2_deg * DEG_TO_RAD
    dec2 = dec2_deg * DEG_TO_RAD

    # Haversine formula
    # cos(d) = sin(dec1) * sin(dec2) + cos(dec1) * cos(dec2) * cos(ra1 - ra2)
    cos_d = (math.sin(dec1) * math.sin(dec2) +
             math.cos(dec1) * math.cos(dec2) * math.cos(ra1 - ra2))

    # Clamp to [-1, 1] to avoid numerical errors
    cos_d = max(-1.0, min(1.0, cos_d))

    # Angular separation in radians
    sep_rad = math.acos(cos_d)

    # Convert to arcseconds
    sep_arcsec = sep_rad * RAD_TO_DEG * DEG_TO_ARCSEC

    return sep_arcsec


def compute_coordinate_key(ra_deg: float, dec_deg: float, tolerance_arcsec: float = 1.0) -> str:
    """
    Generate a coordinate key rounded to the specified tolerance.

    This is used for grouping targets by proximity. Targets within the
    tolerance will have the same key.

    Args:
        ra_deg: RA in degrees
        dec_deg: DEC in degrees
        tolerance_arcsec: Rounding tolerance in arcseconds (default 1.0)

    Returns:
        String key in format "RA_deg:DEC_deg" (6 decimal places)
    """
    # Convert tolerance to degrees
    tolerance_deg = tolerance_arcsec * ARCSEC_TO_DEG

    # Round to tolerance
    ra_rounded = round(ra_deg / tolerance_deg) * tolerance_deg
    dec_rounded = round(dec_deg / tolerance_deg) * tolerance_deg

    # Format as string with 6 decimal places
    return f"{ra_rounded:.6f}:{dec_rounded:.6f}"


def find_nearest_center(
    target_ra_deg: float,
    target_dec_deg: float,
    centers: list[Tuple[str, float, float]],
    max_sep_arcsec: float = DEFAULT_GROUP_TOLERANCE_ARCSEC
) -> Optional[str]:
    """
    Find the nearest center within the maximum separation.

    Args:
        target_ra_deg: Target RA in degrees
        target_dec_deg: Target DEC in degrees
        centers: List of (key, ra_deg, dec_deg) tuples for group centers
        max_sep_arcsec: Maximum separation in arcseconds

    Returns:
        Key of nearest center, or None if no center within max_sep_arcsec
    """
    nearest_key = None
    min_sep = float('inf')

    for key, center_ra, center_dec in centers:
        sep = angular_separation_arcsec(target_ra_deg, target_dec_deg, center_ra, center_dec)

        if sep <= max_sep_arcsec and sep < min_sep:
            min_sep = sep
            nearest_key = key

    return nearest_key


def test_coordinate_utils():
    """Test coordinate utility functions."""
    print("Testing Coordinate Utilities...")

    # Test sexagesimal parsing
    print("\n1. Sexagesimal Parsing:")
    ra_str = "12:34:56.78"
    ra_deg = parse_sexagesimal_ra(ra_str)
    print(f"  RA '{ra_str}' -> {ra_deg:.6f} deg")

    dec_str = "+45:30:20.1"
    dec_deg = parse_sexagesimal_dec(dec_str)
    print(f"  DEC '{dec_str}' -> {dec_deg:.6f} deg")

    # Test gnomonic projection
    print("\n2. Gnomonic Projection:")
    ra0, dec0 = 188.5, 45.0  # Base position
    offset_ra, offset_dec = 5.0, -3.0  # Offsets in arcsec
    ra1, dec1 = gnomonic_projection(ra0, dec0, offset_ra, offset_dec)
    print(f"  Base: RA={ra0:.6f}, DEC={dec0:.6f}")
    print(f"  Offset: dRA={offset_ra} arcsec, dDEC={offset_dec} arcsec")
    print(f"  Projected: RA={ra1:.6f}, DEC={dec1:.6f}")

    # Test angular separation
    print("\n3. Angular Separation:")
    sep = angular_separation_arcsec(ra0, dec0, ra1, dec1)
    print(f"  Separation: {sep:.3f} arcsec")
    expected = math.sqrt(offset_ra**2 + offset_dec**2)
    print(f"  Expected: {expected:.3f} arcsec")

    # Test coordinate key generation
    print("\n4. Coordinate Key:")
    key = compute_coordinate_key(188.5, 45.0, tolerance_arcsec=1.0)
    print(f"  Key for RA=188.5, DEC=45.0: {key}")

    # Test grouping
    print("\n5. Nearest Center:")
    centers = [
        ("center1", 188.5, 45.0),
        ("center2", 189.0, 45.0),
    ]
    nearest = find_nearest_center(188.50001, 45.00001, centers, max_sep_arcsec=2.0)
    print(f"  Nearest center to (188.50001, 45.00001): {nearest}")

    print("\nAll tests completed!")


if __name__ == "__main__":
    test_coordinate_utils()
