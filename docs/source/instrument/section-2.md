# Section 2: Calibration and Data Reduction Performance

This page contains 4 requirements.

## Page summary

| Metric | Value |
|---|---:|
| Total requirements | 4 |
| Status: Compliant | 2 |
| Status: Partially Compliant | 1 |
| Status: N/A | 1 |
| Verification: Test | 3 |
| Verification: N/A | 1 |
| Result: PASS | 2 |
| Result: TBD | 1 |
| Result: Not Recorded | 1 |

## Requirements

### REQ-500 — Wavelength and spatial map stability

| Field | Value |
|---|---|
| Identifier | REQ-500 |
| Source | NGPS.CAL - DRD |
| Source section | 2 |
| Category / request | Wavelength and spatial map stability |
| Importance | Must Have |
| Feature | Yes |
| Status | Partially Compliant |
| Verification method | Test |
| Owner | Christoffer |
| Result | TBD |

**Requirement**

RMS wavelength calibration error for isolated unresolved emission or absorption lines shall be no larger than 0.1 pixel, which scales to 0.03 Angstroms or 2 km/s.

**Notes / verification evidence**

this needs flexure system operational. but using sky line corrections this can be achieved in post processing. confirmed using data on ngc2440

**Actions**

Test on-sky

### REQ-501 — Wavelength map accuracy

| Field | Value |
|---|---|
| Identifier | REQ-501 |
| Source | NGPS.CAL - DRD |
| Source section | 2 |
| Category / request | Wavelength map accuracy |
| Importance | Must Have |
| Feature | Yes |
| Status | Compliant |
| Verification method | Test |
| Owner | Christoffer |
| Result | PASS |

**Requirement**

Achieve a wavelength RMS for unresolved emission or absorption lines no larger than 0.05 pixel, which scales to 0.015 Angstroms or 1 km/s.

**Notes / verification evidence**

mesured RMS of arc line solutions is ~0.03-0.05 pixels.

### REQ-503 — High frequency instrumental response correction

| Field | Value |
|---|---|
| Identifier | REQ-503 |
| Source | NGPS.CAL - DRD |
| Source section | 2 |
| Category / request | High frequency instrumental response correction |
| Importance | Must Have |
| Feature | Yes |
| Status | Compliant |
| Verification method | Test |
| Owner | Christoffer |
| Result | PASS |

**Requirement**

Remove instrumental response variation to 2% RMS

**Notes / verification evidence**

What is the basis for 2%? in any case flat field data shows agreement on all channels for 0.4, 1.5, 10" slit widths. Held-out flat / master ratio: pixel RMS minus Poisson. NOTE: this requires spatial alignment of flat to science data by flexure system or DRP post processing

### REQ-504 — Low frequency instrument response correction

| Field | Value |
|---|---|
| Identifier | REQ-504 |
| Source | NGPS.CAL - DRD |
| Source section | 2 |
| Category / request | Low frequency instrument response correction |
| Importance | Must Have |
| Feature | No |
| Status | N/A |
| Verification method | N/A |
| Result | Not Recorded |

**Requirement**

Calibrate absolute flux from standard star to 2% RMS
