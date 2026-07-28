# Software

This page contains 5 requirements.

## Page summary

| Metric | Value |
|---|---:|
| Total requirements | 5 |
| Status: Compliant | 5 |
| Verification: Design | 5 |
| Result: PASS | 3 |
| Result: PASS (pypeit and quicklook) | 1 |
| Result: PASS (pypeit) | 1 |

## Requirements

### NGPS-SCI-REQ-0710 — The instrument shall be delivered with a data reduction pipeline.

| Field | Value |
|---|---|
| Identifier | NGPS-SCI-REQ-0710 |
| Source | NGPS.SCI.DRD.001.docx |
| Source section | 3.8 |
| Category / request | Software |
| Importance | Must Have |
| Status | Compliant |
| Verification method | Design |
| Result | PASS |

**Requirement**

The instrument shall be delivered with a data reduction pipeline.

### NGPS-SCI-REQ-0720 — The pipeline shall provide bias correction, flat fielding, wavelength calibrati...

| Field | Value |
|---|---|
| Identifier | NGPS-SCI-REQ-0720 |
| Source | NGPS.SCI.DRD.001.docx |
| Source section | 3.8 |
| Category / request | Software |
| Importance | Must Have |
| Status | Compliant |
| Verification method | Design |
| Result | PASS |

**Requirement**

The pipeline shall provide bias correction, flat fielding, wavelength calibration, sky subtraction, and spectral extraction.

**Actions**

Initial design for in-line calibration was unsuccessful. No re-design currently planned.

### NGPS-SCI-REQ-0730 — The pipeline shall be operable without user interaction. It may have optional i...

| Field | Value |
|---|---|
| Identifier | NGPS-SCI-REQ-0730 |
| Source | NGPS.SCI.DRD.001.docx |
| Source section | 3.8 |
| Category / request | Software |
| Importance | Must Have |
| Status | Compliant |
| Verification method | Design |
| Result | PASS (pypeit and quicklook) |

**Requirement**

The pipeline shall be operable without user interaction. It may have optional interactive modes.

### NGPS-SCI-REQ-0740 — The pipeline shall provide an object spectrum, variance spectrum, and sky spect...

| Field | Value |
|---|---|
| Identifier | NGPS-SCI-REQ-0740 |
| Source | NGPS.SCI.DRD.001.docx |
| Source section | 3.8 |
| Category / request | Software |
| Importance | Must Have |
| Status | Compliant |
| Verification method | Design |
| Result | PASS (pypeit) |

**Requirement**

The pipeline shall provide an object spectrum, variance spectrum, and sky spectrum.

**Actions**

consider adding variance and 1d sky output to ql data products

### NGPS-SCI-REQ-0750 — The pipeline shall provide flux calibration if the user provides observations o...

| Field | Value |
|---|---|
| Identifier | NGPS-SCI-REQ-0750 |
| Source | NGPS.SCI.DRD.001.docx |
| Source section | 3.8 |
| Category / request | Software |
| Importance | Must Have |
| Status | Compliant |
| Verification method | Design |
| Result | PASS |

**Requirement**

The pipeline shall provide flux calibration if the user provides observations of a common flux standard.
