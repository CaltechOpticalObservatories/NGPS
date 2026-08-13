# Additional Instrument Requirements

This page contains 7 requirements.

## Page summary

| Metric | Value |
|---|---:|
| Total requirements | 7 |
| Status: Compliant | 4 |
| Status: In Progress | 1 |
| Status: Not Compliant | 1 |
| Status: Partially Compliant | 1 |
| Verification: Design | 7 |
| Result: PASS | 4 |
| Result: TBD | 1 |
| Result: FAIL | 1 |
| Result: ACCEPT | 1 |

## Requirements

### REQ-161 — offline calibration (in dome with work going on)

| Field | Value |
|---|---|
| Identifier | REQ-161 |
| Category / request | offline calibration (in dome with work going on) |
| Importance | Must Have |
| Feature | Yes |
| Status | In Progress |
| Verification method | Design |
| Owner | Matt |
| Result | TBD |

**Requirement**

There shall be the capability to do the calibration (including dark frames) in dome with the lights on.

**Actions**

Address light leaks and verify

### REQ-162 — inline calibration

| Field | Value |
|---|---|
| Identifier | REQ-162 |
| Category / request | inline calibration |
| Importance | Must Have |
| Feature | Yes |
| Status | Not Compliant |
| Verification method | Design |
| Owner | Matt |
| Result | FAIL |

**Requirement**

There shall be the capability to do wavelength calibration concurrent with science observation

**Notes / verification evidence**

Inline calibration not implemented

**Actions**

Initial design for in-line calibration was unsuccessful. No re-design currently planned.

### REQ-163 — slit/slice viewing cameras

| Field | Value |
|---|---|
| Identifier | REQ-163 |
| Category / request | slit/slice viewing cameras |
| Importance | Must Have |
| Feature | Yes |
| Status | Compliant |
| Verification method | Design |
| Owner | Matt |
| Result | PASS |

**Requirement**

There shall be the capability to image the slit/slicer to verify the fine positioning of the science target

**Notes / verification evidence**

Confirmed

### REQ-164 — Binning modes

| Field | Value |
|---|---|
| Identifier | REQ-164 |
| Category / request | Binning modes |
| Importance | Must have |
| Feature | Yes |
| Status | Partially Compliant |
| Verification method | Design |
| Owner | Matt |
| Result | ACCEPT |

**Requirement**

CCD binning shall be settable independenty for spatial and spectal directions, either to fixed values [1-6] to "auto" wherein the largest binning factor i consistent with PSF/LSF sampling requirement is selected.

**Notes / verification evidence**

FWHM in spatial direction, thus bin facor, depends on seeing averaged over exposure time, while in spectral direction binning depends on slit width.

### REQ-165 — FWHM of linespread function will be sampled by at least 2.7 pixels for 0.5" slit

| Field | Value |
|---|---|
| Identifier | REQ-165 |
| Category / request | FWHM of linespread function will be sampled by at least 2.7 pixels for 0.5" slit |
| Importance | Must Have |
| Feature | Yes |
| Status | Compliant |
| Verification method | Design |
| Owner | Christoffer |
| Result | PASS |

**Requirement**

FWHM of linespread function shall be sampled by at least 2.7 pixels for 0.5" slit

**Notes / verification evidence**

Check with final design, renegoiate as needed

### REQ-166 — Acquisition will center target on slit to within 0.05"

| Field | Value |
|---|---|
| Identifier | REQ-166 |
| Category / request | Acquisition will center target on slit to within 0.05" |
| Importance | Must Have |
| Feature | Yes |
| Status | Compliant |
| Verification method | Design |
| Owner | Christoffer |
| Result | PASS |

**Requirement**

Acquisition shall center target on slit to within 0.05"

**Notes / verification evidence**

That's about 20 um at the telescope focal plane, or about 1/3 of a detector unbinned pixel. guided offsets are accurate enough for this under normal obs. conditions, but the measured center of slit position is not more accurate than 0.1"

### REQ-210 — Co-locate electronics rack with instrument

| Field | Value |
|---|---|
| Identifier | REQ-210 |
| Source | NGPS DRD |
| Category / request | Co-locate electronics rack with instrument |
| Importance | Nice to Have |
| Feature | Yes |
| Status | Compliant |
| Verification method | Design |
| Owner | Reston |
| Result | PASS |

**Requirement**

The instrument electronics rack shall be mounted to the Cass rotator with the instrument
