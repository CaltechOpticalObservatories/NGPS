# Section 3.1: Spectral and Optical Performance

This page contains 16 requirements.

## Page summary

| Metric | Value |
|---|---:|
| Total requirements | 16 |
| Status: Compliant | 11 |
| Status: Planned Compliant | 3 |
| Status: Not Compliant | 1 |
| Status: Likely Compliant | 1 |
| Verification: Analysis | 7 |
| Verification: Design | 5 |
| Verification: Test | 4 |
| Result: PASS | 11 |
| Result: TBD | 4 |
| Result: FAIL / ACCEPT | 1 |

## Requirements

### REQ-100 — Wavelength coverage with to half power points

| Field | Value |
|---|---|
| Identifier | REQ-100 |
| Source | NGPS DRD |
| Source section | 3.1 |
| Category / request | Wavelength coverage with to half power points |
| Importance | Must Have |
| Feature | Yes |
| Status | Compliant |
| Verification method | Test |
| Owner | Chistoffer |
| Result | PASS |

**Requirement**

Wavelength coverage shall be at least 360-1000 nm at the half-power points. (goal is 330-1100 nm)

**Notes / verification evidence**

310-1050 achieved. arc data + domeflats

### REQ-101 — Total available wavelength coverage

| Field | Value |
|---|---|
| Identifier | REQ-101 |
| Source | NGPS DRD |
| Source section | 3.1 |
| Category / request | Total available wavelength coverage |
| Importance | Must Have |
| Feature | Yes |
| Status | Compliant |
| Verification method | Test |
| Owner | Christoffer |
| Result | PASS |

**Requirement**

Wavelength coverage shall be at least 310-1040 nm at 10% of peak throughput

### REQ-105 — At least one operational mode shall cover the entire wavelength coverage simult...

| Field | Value |
|---|---|
| Identifier | REQ-105 |
| Source | NGPS DRD |
| Source section | 3.1 |
| Category / request | At least one operational mode shall cover the entire wavelength coverage simultaneously. |
| Importance | Must Have |
| Feature | Yes |
| Status | Compliant |
| Verification method | Design |
| Owner | Christoffer |
| Result | PASS |

**Requirement**

At least one operational mode shall cover the entire wavelength coverage simultaneously.

**Notes / verification evidence**

All 4 channels can operate concurrently

### REQ-110 — Average resolution (R) shall be greater than 1800 with a 0.5 arcsec slit.

| Field | Value |
|---|---|
| Identifier | REQ-110 |
| Source | NGPS DRD |
| Source section | 3.1 |
| Category / request | Average resolution (R) shall be greater than 1800 with a 0.5 arcsec slit. |
| Importance | Must Have |
| Feature | Yes |
| Status | Compliant |
| Verification method | Design |
| Owner | Christoffer |
| Result | PASS |

**Requirement**

Average resolution (R) shall be greater than 1800 with a 0.5 arcsec slit.

**Notes / verification evidence**

The resolution requirements are lacking detail.

### REQ-115 — Resolution shall be greater than 3500 in the red part of the spectrum, to allow...

| Field | Value |
|---|---|
| Identifier | REQ-115 |
| Source | NGPS DRD |
| Source section | 3.1 |
| Category / request | Resolution shall be greater than 3500 in the red part of the spectrum, to allow sampling of continuum between OH emission lines. |
| Importance | Must Have |
| Feature | Yes |
| Status | Compliant |
| Verification method | Design |
| Owner | Christoffer |
| Result | PASS |

**Requirement**

Resolution shall be greater than 3500 at 800nm, to allow sampling of continuum between OH emission lines.

**Notes / verification evidence**

Measuring R>4300 for both channels at min slit width

### REQ-120 — Resolution over 4500 shall be achievable.

| Field | Value |
|---|---|
| Identifier | REQ-120 |
| Source | NGPS DRD |
| Source section | 3.1 |
| Category / request | Resolution over 4500 shall be achievable. |
| Importance | Must Have |
| Feature | Yes |
| Status | Compliant |
| Verification method | Analysis |
| Owner | Christoffer |
| Result | PASS |

**Requirement**

Resolution over 4500 shall be achievable.

### REQ-125 — Peak sky-to-electrons optical transmission shall be greater than 35%, including...

| Field | Value |
|---|---|
| Identifier | REQ-125 |
| Source | NGPS DRD |
| Source section | 3.1 |
| Category / request | Peak sky-to-electrons optical transmission shall be greater than 35%, including losses from sky, telescope, slit, instrument, and quantum efficiency. |
| Importance | Must Have |
| Feature | Yes |
| Status | Compliant |
| Verification method | Analysis |
| Owner | Christoffer |
| Result | PASS |

**Requirement**

At the wavelength of peak efficiency(880nm) sky-to-electrons optical transmission shall be greater than 35%, including losses from sky, telescope, instrument, and quantum efficiency.

**Notes / verification evidence**

Measured on sky

### REQ-130 — Average optical transmission of the instrument shall be greater than 65%, exclu...

| Field | Value |
|---|---|
| Identifier | REQ-130 |
| Source | NGPS DRD |
| Source section | 3.1 |
| Category / request | Average optical transmission of the instrument shall be greater than 65%, excluding slit losses |
| Importance | Must Have |
| Feature | Yes |
| Status | Compliant |
| Verification method | Analysis |
| Owner | Christoffer |
| Result | PASS |

**Requirement**

Average optical transmission of the instrument shall be greater than 65%, excluding slit losses

**Notes / verification evidence**

lab. curves show mean of 64% in 360-1000nm range. measured is 59 +-6%.

### REQ-135 — A variety of slit widths shall be available, from 0.35 arcsec to 10 arcsec.

| Field | Value |
|---|---|
| Identifier | REQ-135 |
| Source | NGPS DRD |
| Source section | 3.1 |
| Category / request | A variety of slit widths shall be available, from 0.35 arcsec to 10 arcsec. |
| Importance | Must Have |
| Feature | Yes |
| Status | Compliant |
| Verification method | Design |
| Owner | Matt |
| Result | PASS |

**Requirement**

The slit width shall be continuously adjustable from 0.35 arcsec to 10 arcsec.

**Notes / verification evidence**

Confirmed

### REQ-140 — Slit length shall be 60 arcseconds.

| Field | Value |
|---|---|
| Identifier | REQ-140 |
| Source | NGPS DRD |
| Source section | 3.1 |
| Category / request | Slit length shall be 60 arcseconds. |
| Importance | Must Have |
| Feature | Yes |
| Status | Not Compliant |
| Verification method | Design |
| Owner | Matt |
| Result | FAIL / ACCEPT |

**Requirement**

Slit length shall be 60 arcseconds.

**Notes / verification evidence**

Total slit ~ 50 arcsec. May be revised slightly

**Actions**

Potential future Slicer Upgrade

### REQ-145 — The FWHM of the delivered image quality shall be better than the minimum slit w...

| Field | Value |
|---|---|
| Identifier | REQ-145 |
| Source section | 3.1 |
| Category / request | The FWHM of the delivered image quality shall be better than the minimum slit width imaged onto the detector. |
| Importance | Must Have |
| Feature | Yes |
| Status | Likely Compliant |
| Verification method | Analysis |
| Owner | Matt |
| Result | TBD |

**Requirement**

The FWHM of the delivered image quality shall be better than the minimum slit width imaged onto the detector.

**Notes / verification evidence**

Include errors from focus and internal shifts. Requires error budget to allocate to subsystems

**Actions**

Run Analysis

### REQ-150 — Errors in guiding over 1 hr. within 60 deg. of zenith shall introduce wavelengt...

| Field | Value |
|---|---|
| Identifier | REQ-150 |
| Source | NGPS DRD |
| Source section | 3.1 |
| Category / request | Errors in guiding over 1 hr. within 60 deg. of zenith shall introduce wavelength error < 0.01 nm. |
| Importance | Must Have |
| Feature | Yes |
| Status | Planned Compliant |
| Verification method | Analysis |
| Owner | Christoffer |
| Result | TBD |

**Requirement**

Errors in guiding over 1 hr. within 60 deg. of zenith shall introduce wavelength error < 0.01 nm.

**Notes / verification evidence**

Testing planned for 5/18+19/2026 commissioning nights

**Actions**

Test on-sky

### REQ-155 — Errors in guiding over 1 hr. within 60 deg. of zenith shall introduce a spatial...

| Field | Value |
|---|---|
| Identifier | REQ-155 |
| Source | NGPS DRD |
| Source section | 3.1 |
| Category / request | Errors in guiding over 1 hr. within 60 deg. of zenith shall introduce a spatial blur < 0.1 arcsecond RMS. |
| Importance | Must Have |
| Feature | Yes |
| Status | Compliant |
| Verification method | Analysis |
| Owner | Christoffer |
| Result | PASS |

**Requirement**

Errors in guiding over 1 hr. within 60 deg. of zenith shall introduce a spatial blur < 0.1 arcsecond RMS.

**Notes / verification evidence**

tested using acam data from 2025-02-16 UT

### REQ-160 — The aquisition and guide system shall have a field of view shall be at least 2....

| Field | Value |
|---|---|
| Identifier | REQ-160 |
| Source | NGPS DRD |
| Source section | 3.1 |
| Category / request | The aquisition and guide system shall have a field of view shall be at least 2.5’ x 2.5’ (3’ x 3’ goal). |
| Importance | Must Have |
| Feature | Yes |
| Status | Compliant |
| Verification method | Analysis |
| Owner | Matt |
| Result | PASS |

**Requirement**

The acquisition and guide system shall have a field of view shall be at least 2.5’ x 2.5’ (3’ x 3’ goal).

**Notes / verification evidence**

Slicecams cover 1'x1'. acam covers 4.4'x4.4'

### REQ-220 — Including any flexure compensation, flexure over a 1-hour exposure within 60 de...

| Field | Value |
|---|---|
| Identifier | REQ-220 |
| Source | NGPS DRD |
| Source section | 3.1 |
| Category / request | Including any flexure compensation, flexure over a 1-hour exposure within 60 deg of zenith shall introduce a wavelength error of < 0.01nm. |
| Importance | Must Have |
| Feature | Yes |
| Status | Planned Compliant |
| Verification method | Test |
| Owner | Matt |
| Result | TBD |

**Requirement**

Including any flexure compensation, flexure over a 1-hour exposure within 60 deg of zenith shall introduce a wavelength error of < 0.01nm.

**Notes / verification evidence**

Testing planned for 6/16 & 7/25 commissioning nights

**Actions**

Test on-sky

### REQ-225 — Including any flexure compensation, flexure over a 1-hour exposure within 60 de...

| Field | Value |
|---|---|
| Identifier | REQ-225 |
| Source | NGPS DRD |
| Source section | 3.1 |
| Category / request | Including any flexure compensation, flexure over a 1-hour exposure within 60 deg of zenith shall introduce a spatial blur of less than 60 arcsec RMS. |
| Importance | Must Have |
| Feature | Yes |
| Status | Planned Compliant |
| Verification method | Test |
| Owner | Matt |
| Result | TBD |

**Requirement**

Including any flexure compensation, flexure over a 1-hour exposure within 60 deg of zenith shall introduce a spatial blur of less than 0.05 arcsec RMS.

**Notes / verification evidence**

Testing planned for 6/16 & 7/25 commissioning nights

**Actions**

Test on-sky
