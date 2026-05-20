import csv
import io
import os
import re


CAL_HEADER = [
    "NAME",
    "RA",
    "DECL",
    "OFFSET_RA",
    "OFFSET_DEC",
    "COMMENT",
    "PRIORITY",
    "BINSPAT",
    "BINSPECT",
    "SLITANGLE",
    "SLITWIDTH",
    "AIRMASS_MAX",
    "WRANGE_LOW",
    "WRANGE_HIGH",
    "CHANNEL",
    "MAGNITUDE",
    "MAGFILTER",
    "EXPTIME",
    "NEXP",
]


def _fmt(value):
    """Format values so 20.0 becomes '20', but 2.5 stays '2.5'."""
    if isinstance(value, float) and value.is_integer():
        return str(int(value))
    return str(value)


def _cal_row(name, comment, bin_spat, bin_spec, slitwidth, exptime, nexp):
    """
    Build one calibration target row using the same field layout as make_cals.

    Bash equivalent:
        print_line $name "$comment" $xbin $ybin $slitwidth $exptime $N_exp
    """
    row = {key: "" for key in CAL_HEADER}

    row["NAME"] = name
    row["COMMENT"] = comment
    row["BINSPAT"] = _fmt(bin_spat)
    row["BINSPECT"] = _fmt(bin_spec)
    row["SLITWIDTH"] = _fmt(slitwidth)
    row["EXPTIME"] = _fmt(exptime)
    row["NEXP"] = _fmt(nexp)

    return row


def make_calibration_targets(slitwidth, xbin, ybin):
    """
    Generate NGPS calibration target rows.

    Parameters match the old bash script:
        slitwidth = $1
        xbin      = $2
        ybin      = $3

    Current mapping preserves the bash output:
        BINSPAT  <- xbin
        BINSPECT <- ybin
    """
    slitwidth = float(slitwidth)
    xbin = int(xbin)
    ybin = int(ybin)

    if slitwidth <= 0:
        raise ValueError("slitwidth must be greater than 0.")

    if xbin <= 0 or ybin <= 0:
        raise ValueError("binning values must be greater than 0.")

    # Nominal exposure times, seconds.
    t_thar_nom = 20
    t_fear_nom = 21
    t_thar_nom_ug = 200
    t_fear_nom_ug = 210
    t_cont_nom = 20
    t_contb_nom = 30
    t_etalon_nom = 3
    t_dome_nom = 90
    t_dome_nom_ug = 400
    t_bias = 0
    t_dark = 1200

    # Nominal counts.
    n_thar = 3
    n_fear = 3
    n_cont = 3
    n_etalon = 0
    n_dome = 0
    n_dome_ug = 0
    n_bias = 5
    n_dark = 0

    arc_multiplier = 1.0 / (xbin * ybin)
    cont_multiplier = (1.0 / (xbin * ybin)) * (0.5 / slitwidth)

    t_thar = round(t_thar_nom * arc_multiplier, 2)
    t_fear = int(t_fear_nom * arc_multiplier)
    t_cont = int(t_cont_nom * cont_multiplier)
    t_dome = int(t_dome_nom * cont_multiplier)
    t_thar_ug = int(t_thar_nom_ug * arc_multiplier)
    t_fear_ug = int(t_fear_nom_ug * arc_multiplier)
    t_contb = int(t_contb_nom * cont_multiplier)
    t_dome_ug = int(t_dome_nom_ug * cont_multiplier)
    t_etalon = int(t_etalon_nom * cont_multiplier)

    rows = []

    if n_bias > 0:
        rows.append(
            _cal_row(
                "CAL_BIAS",
                "ugri bias",
                xbin,
                ybin,
                slitwidth,
                t_bias,
                n_bias,
            )
        )

    if n_thar > 0:
        rows.append(
            _cal_row(
                "CAL_THAR",
                "ugri thar",
                xbin,
                ybin,
                slitwidth,
                t_thar,
                n_thar,
            )
        )

    if n_fear > 0:
        rows.append(
            _cal_row(
                "CAL_FEAR",
                "ugri fear",
                xbin,
                ybin,
                slitwidth,
                t_fear,
                n_fear,
            )
        )

    if n_cont > 0:
        rows.append(
            _cal_row(
                "CAL_CONTR",
                "ugri red continuum",
                xbin,
                ybin,
                slitwidth,
                t_cont,
                n_cont,
            )
        )

    if n_cont > 0:
        rows.append(
            _cal_row(
                "CAL_CONTB",
                "ugri blue continuum",
                xbin,
                ybin,
                slitwidth,
                t_contb,
                n_cont,
            )
        )

    if n_thar > 0:
        rows.append(
            _cal_row(
                "CAL_THAR_UG",
                "ug thar",
                xbin,
                ybin,
                slitwidth,
                t_thar_ug,
                n_thar,
            )
        )

    if n_fear > 0:
        rows.append(
            _cal_row(
                "CAL_FEAR_UG",
                "ugri fear",
                xbin,
                ybin,
                slitwidth,
                t_fear_ug,
                n_fear,
            )
        )

    if n_etalon > 0:
        rows.append(
            _cal_row(
                "CAL_ETALON",
                "ugri etalon",
                xbin,
                ybin,
                slitwidth,
                t_etalon,
                n_etalon,
            )
        )

    if n_dome > 0:
        rows.append(
            _cal_row(
                "CAL_DOME",
                "ugri dome",
                xbin,
                ybin,
                slitwidth,
                t_dome,
                n_dome,
            )
        )

    if n_dome_ug > 0:
        rows.append(
            _cal_row(
                "CAL_DOME_UG",
                "ug dome",
                xbin,
                ybin,
                slitwidth,
                t_dome_ug,
                n_dome_ug,
            )
        )

    if n_dark > 0:
        rows.append(
            _cal_row(
                "CAL_DARK",
                "ugri dark",
                xbin,
                ybin,
                slitwidth,
                t_dark,
                n_dark,
            )
        )

    return rows


def make_calibration_csv_text(slitwidth, xbin, ybin):
    """Return the generated calibration procedure as CSV text."""
    rows = make_calibration_targets(slitwidth, xbin, ybin)

    output = io.StringIO()
    writer = csv.DictWriter(output, fieldnames=CAL_HEADER)
    writer.writeheader()
    writer.writerows(rows)

    return output.getvalue()

def make_dome_flat_targets(slitwidth, xbin, ybin):
    """
    Generate NGPS dome-flat calibration target rows.

    This is the Python equivalent of the make_domes bash recipe:
        make_domes <slitw> <bin_spat> <bin_spec>

    The exposure-time constants follow the same convention used by
    make_calibration_targets() above. In other words, the bash values
    90000 and 400000 are represented here as 90 and 400.

    Current mapping preserves the bash output:
        BINSPAT  <- xbin
        BINSPECT <- ybin
    """
    slitwidth = float(slitwidth)
    xbin = int(xbin)
    ybin = int(ybin)

    if slitwidth <= 0:
        raise ValueError("slitwidth must be greater than 0.")

    if xbin <= 0 or ybin <= 0:
        raise ValueError("binning values must be greater than 0.")

    # Nominal exposure times, seconds.
    # Bash make_domes equivalent values:
    #   T_dome_nom    = 90000
    #   T_dome_nom_ug = 400000
    t_dome_nom = 90
    t_dome_nom_ug = 400

    # Nominal counts from make_domes.
    n_dome = 7
    n_dome_ug = 7

    cont_multiplier = (1.0 / (xbin * ybin)) * (0.5 / slitwidth)

    t_dome = int(t_dome_nom * cont_multiplier)
    t_dome_ug = int(t_dome_nom_ug * cont_multiplier)

    rows = []

    if n_dome > 0:
        rows.append(
            _cal_row(
                "CAL_DOME",
                "ugri dome",
                xbin,
                ybin,
                slitwidth,
                t_dome,
                n_dome,
            )
        )

    if n_dome_ug > 0:
        rows.append(
            _cal_row(
                "CAL_DOME_UG",
                "ug dome",
                xbin,
                ybin,
                slitwidth,
                t_dome_ug,
                n_dome_ug,
            )
        )

    return rows


def make_dome_flat_csv_text(slitwidth, xbin, ybin):
    """Return the generated dome-flat procedure as CSV text."""
    rows = make_dome_flat_targets(slitwidth, xbin, ybin)

    output = io.StringIO()
    writer = csv.DictWriter(output, fieldnames=CAL_HEADER)
    writer.writeheader()
    writer.writerows(rows)

    return output.getvalue()


def save_calibration_csv(rows, target_list_name, output_dir="generated_target_lists/calibrations"):
    """
    Save generated calibration rows to a CSV file.

    The file name is based on the target list name, for example:
        CAL_2026-05-02_12-54-30_slit0.5_bin1x1.csv
    """
    if not rows:
        raise ValueError("No calibration rows to save.")

    os.makedirs(output_dir, exist_ok=True)

    safe_name = re.sub(r"[^A-Za-z0-9_.-]+", "_", target_list_name)
    csv_path = os.path.abspath(
        os.path.join(output_dir, f"{safe_name}.csv")
    )

    with open(csv_path, "w", newline="") as file:
        writer = csv.DictWriter(file, fieldnames=CAL_HEADER)
        writer.writeheader()
        writer.writerows(rows)

    return csv_path