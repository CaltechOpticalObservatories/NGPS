# Target Lists

## Target List File

The target list (TL) is an ASCII text file in comma-separated values (CSV) format. It contains all of the information needed to drive observations. A single header row defines the meaning of each column; each subsequent row describes the parameters for one target.

The following formatting rules apply:

- Commas are the only recognized delimiter.
- White space is allowed within strings (between commas) and is ignored before or after numeric values.
- Quotes are read literally.
- Leading and trailing whitespace is trimmed.
- Every data row must have the same number of commas as the header row.
- Completely blank lines are ignored.
- The observer may add extra columns for personal use; unknown column headers are ignored by the software.

The nexp column allows users to specify repeated exposures of the same target. When the target list is loaded into the database, each row with nexp > 1 is expanded into N identical rows, so that each exposure is represented as a separate database entry. This allows the sequencer to process each exposure independently.

Example target list:

```text
name, ra, decl, slitwidth, exptime, nexp
alp_ori, 05:55:10.30, +07:24:25.43, 1.5, 1800, 1
alp_tau, , , 1.5, 900, 3
```

Note that empty fields (RA, DECL in line 2) are left blank between commas. It is required that the header row contain at least one recognized keyword. The recognized keywords also appear as FITS header keywords (see Appendix 9.2).

## Target List Header Keywords

The following table lists all recognized target list column headers. Default values are used when a column is omitted or left empty.

Table 1. Target List Header Keywords

Table 2: Target List Header Keywords

Column Default Description

nexp 1 Number of exposures (integer, 1 to 16,777,216). When nexp > 1, the loader expands the row into N identical database entries.

name Test Target name (alphanumeric). Reserved names: "dark" (dark frame), "bias" (bias frame), "calib" (calibration).

RA 0 Right ascension of slit center (J2000). Format: HH:MM:SS.S or HH MM SS.S. Range: 0 to 24 hours.

DECL -90:00:00 Declination of slit center (J2000). Format: ±DD:MM:SS or ±DD MM SS. Range: −90 to +90 degrees.

REF_OFFSET_RA 00:00:00 RA offset of reference star from target (same format as RA). Used for indirect acquisition.

REF_OFFSET_DECL 00:00:00 DECL offset of reference star from target (same format as DECL). Used for indirect acquisition.

AcqMode Direct Acquisition mode. Direct: slew to place target on slit. Indirect: slew to reference star, measure position, offset to target.

slitwidth SET 1.3 Slit width with optional mode code. SET X or X: fixed arcsec (0 < X ≤ 10). PSF X: capture X% of PSF. SNR X: optimize for SNR.

slitoffset 0 Slit offset in arcseconds (float ≥ 0).

exptime SET 10 Exposure time with optional mode code. SET X or X: fixed seconds (0.001 to 16,777.216). SNR X: achieve SNR = X.

ccdmode TBD Reserved for pre-defined CCD modes (clocking speed, amplifier selection). Applies to all cameras.

binspect 1 Binning in the spectral direction (integer).

binspat 1 Binning in the spatial direction (integer).

slitangle 0 Slit orientation. Numeric: fixed angle in degrees (−180 to 200). PA: parallactic angle at exposure midpoint.

airmass_max 3 Maximum acceptable airmass (float > 1). Sequencer waits for target to drop below this limit.

acamfilter none Acquisition camera filter selection.

wrange 500:510 Wavelength range for ETC (Angstroms).

channel G Spectrograph channel for ETC.

mag 19 Target magnitude for ETC.

magsystem AB Magnitude system (e.g. AB).

magfilter G Magnitude filter band.

source (optional) Source spectral template for ETC.

ShortComment (empty) Up to 24 characters, displayed at right in target list table.

Comment (empty) Up to 1024 characters. Displayed in Detail Pane when target is selected. Recorded in observing log.

Three keywords (exptime, slitangle, slitwidth) support mode codes that allow the OTM/ETC to compute optimal values. When a fixed value is desired, simply enter the numeric value or precede it with SET. When optimization is requested (e.g. SNR X for exptime), additional ETC columns may be required in the target list.

## Target Database

NGPS is database-driven. The instrument is controlled entirely by what is stored in the database. The observer prepares one or more target list files in advance (see earlier section) prior to arriving at the telescope. A target list file is read and stored in a database table, and the sequencer processes targets in the order in which they appear. Target order may be rearranged using the GUI.
