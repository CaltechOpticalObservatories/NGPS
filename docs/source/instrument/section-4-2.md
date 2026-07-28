# Section 4.2: Flexure and Calibration References

This page contains 2 requirements.

## Page summary

| Metric | Value |
|---|---:|
| Total requirements | 2 |
| Status: Likely Compliant | 1 |
| Status: Not Compliant | 1 |
| Verification: Test | 2 |
| Result: TBD | 1 |
| Result: FAIL | 1 |

## Requirements

### REQ-511 — Correction of flexure in wavelength to reference geometry

| Field | Value |
|---|---|
| Identifier | REQ-511 |
| Source | NGPS.CAL - DRD |
| Source section | 4.2 |
| Category / request | Correction of flexure in wavelength to reference geometry |
| Importance | Must Have |
| Status | Likely Compliant |
| Verification method | Test |
| Owner | Christoffer |
| Result | TBD |

**Requirement**

Provide reference wavelength lines over the range defined by the half-power points sufficient to calculate the wavelength offset in science observations relative to afternoon calibrations to 2% of the reference solution RMS

**Notes / verification evidence**

Testing planned for 6/16 & 7/25 commissioning nights

**Actions**

Expain terminology.

### REQ-550 — Flexure correction reference for observations

| Field | Value |
|---|---|
| Identifier | REQ-550 |
| Source | NGPS.CAL - DRD |
| Source section | 4.2 |
| Category / request | Flexure correction reference for observations |
| Importance | Must Have |
| Feature | Yes |
| Status | Not Compliant |
| Verification method | Test |
| Owner | Matt |
| Result | FAIL |

**Requirement**

Ability to tie concurrent calibrations obstained while observing to the afternoon calibration wavelength solution to an accuracy of 0.1 pixels or 0.03 Angstroms or 2 km/s at 5000 Angstroms

**Notes / verification evidence**

Is this rms error across all wavelengths or mean error per band? This seems quite loose inless it is the peak error per emission line and even then surely we can centroid to closer to 50th of a pixel for bright lines.

**Actions**

Initial design for in-line calibration was unsuccessful. No re-design currently planned.
