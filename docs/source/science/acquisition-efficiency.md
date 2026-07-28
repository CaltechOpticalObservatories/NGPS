# Acquisition Efficiency

This page contains 3 requirements.

## Page summary

| Metric | Value |
|---|---:|
| Total requirements | 3 |
| Status: Compliant | 3 |
| Verification: Design | 3 |
| Result: PASS | 3 |

## Requirements

### NGPS-SCI-REQ-0510 — Elapsed time from the end of telescope slew until initiation of a science integ...

| Field | Value |
|---|---|
| Identifier | NGPS-SCI-REQ-0510 |
| Source | NGPS.SCI.DRD.001.docx |
| Source section | 3.6 |
| Category / request | Acquisition Efficiency |
| Importance | Must Have |
| Status | Compliant |
| Verification method | Design |
| Result | PASS |

**Requirement**

Elapsed time from the end of telescope slew until initiation of a science integration shall be less than 120 seconds with a goal of 90 seconds.

**Notes / verification evidence**

90s req PASS in auto acq. mode

### NGPS-SCI-REQ-0520 — In order to support NGPS-SCI-REQ-0510, the spectrograph shall have acquisition...

| Field | Value |
|---|---|
| Identifier | NGPS-SCI-REQ-0520 |
| Source | NGPS.SCI.DRD.001.docx |
| Source section | 3.6 |
| Category / request | Acquisition Efficiency |
| Importance | Must Have |
| Status | Compliant |
| Verification method | Design |
| Result | PASS |

**Requirement**

In order to support NGPS-SCI-REQ-0510, the spectrograph shall have acquisition software capable of recognizing the star field of any area of the sky, calculate an astrometric solution, and apply offsets to the telescope pointing to center the desired target on the slit.

**Notes / verification evidence**

Two step method using both acam astrometry and scam feedback for fine tuning of centering

### NGPS-SCI-REQ-0530 — The acquisition and guide system shall have a field of view shall be at least 2...

| Field | Value |
|---|---|
| Identifier | NGPS-SCI-REQ-0530 |
| Source | NGPS.SCI.DRD.001.docx |
| Source section | 3.6 |
| Category / request | Acquisition Efficiency |
| Importance | Must Have |
| Status | Compliant |
| Verification method | Design |
| Result | PASS |

**Requirement**

The acquisition and guide system shall have a field of view shall be at least 2.5’ x 2.5’ (3’ x 3’ goal).

**Notes / verification evidence**

Slicecams cover 1'x1'. acam covers 4.4'x4.4'
