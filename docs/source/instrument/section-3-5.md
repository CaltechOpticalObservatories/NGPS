# Section 3.5: Instrument Constraints and Reliability

This page contains 28 requirements.

## Page summary

| Metric | Value |
|---|---:|
| Total requirements | 28 |
| Status: Likely Compliant | 13 |
| Status: Compliant | 11 |
| Status: Not Set | 4 |
| Verification: Design | 21 |
| Verification: Test | 4 |
| Verification: Inspection | 2 |
| Verification: N/A | 1 |
| Result: ACCEPT | 13 |
| Result: PASS | 11 |
| Result: Not Recorded | 4 |

## Requirements

### REQ-245 — On recovery from a power loss, NGPS shall return to a safe state.

| Field | Value |
|---|---|
| Identifier | REQ-245 |
| Source | NGPS DRD |
| Source section | 3.5 |
| Category / request | On recovery from a power loss, NGPS shall return to a safe state. |
| Importance | Must Have |
| Feature | Yes |
| Status | Compliant |
| Verification method | Test |
| Owner | Dave |
| Result | PASS |

**Requirement**

On recovery from a power loss, NGPS shall return to a safe state for the instrument, to include protection from overillumination of the detectors

**Notes / verification evidence**

The line power for each hardware component (cameras, actuators, etc.) is independently controlled via a network power switch. If the instrument were to lose, then regain power, the default states for the outlets on the NPS are configured such that sensitive components remain in a power-off condition until the proper checks have been made and/or sequences carried out, while outlets that need to be powered are powered-on at startup.

### REQ-255 — Remove heat via liquid cooling to meet 300W Cass cage requirement per the P200 ICD

| Field | Value |
|---|---|
| Identifier | REQ-255 |
| Source | P200 thermal ICD |
| Source section | 3.5 |
| Category / request | Remove heat via liquid cooling to meet 300W Cass cage requirement per the P200 ICD |
| Importance | Must Have |
| Feature | Yes |
| Status | Compliant |
| Verification method | Design |
| Owner | Matt |
| Result | PASS |

**Requirement**

Remove heat via liquid cooling to meet 300W Cass cage requirement per the P200 ICD

**Notes / verification evidence**

Test on-sky

### REQ-260 — The instrument operational lifetime shall be 10 years with preventive maintenan...

| Field | Value |
|---|---|
| Identifier | REQ-260 |
| Source | NGPS DRD |
| Source section | 3.5 |
| Category / request | The instrument operational lifetime shall be 10 years with preventive maintenance by Palomar Observatory staff |
| Importance | Nice to Have |
| Feature | No |
| Status | Not Set |
| Verification method | Design |
| Result | Not Recorded |

**Requirement**

The instrument operational lifetime shall be 25 years with preventive maintenance by Palomar Observatory staff, with the exception of electronics（adhesive,coating）

### REQ-265 — The instrument shall meet all relevant local and US standards for worker safety...

| Field | Value |
|---|---|
| Identifier | REQ-265 |
| Source | NGPS DRD |
| Source section | 3.5 |
| Category / request | The instrument shall meet all relevant local and US standards for worker safety and occupational hazards, |
| Importance | Must Have |
| Feature | Yes |
| Status | Compliant |
| Verification method | Design |
| Owner | Reston |
| Result | PASS |

**Requirement**

The instrument shall meet all relevant local and US standards for worker safety and occupational hazards,

**Notes / verification evidence**

Add safety rail to lift

### REQ-270 — The instrument shall be movable to the Cass Cage using the existing freight ele...

| Field | Value |
|---|---|
| Identifier | REQ-270 |
| Source | NGPS DRD |
| Source section | 3.5 |
| Category / request | The instrument shall be movable to the Cass Cage using the existing freight elevator. |
| Importance | Nice to Have |
| Feature | No |
| Status | Compliant |
| Verification method | Design |
| Owner | Matt |
| Result | PASS |

**Requirement**

Nice to Have

**Notes / verification evidence**

Lift points available

### REQ-275 — The instrument shall allow full 360-degree rotation of the Cass rotator without...

| Field | Value |
|---|---|
| Identifier | REQ-275 |
| Source | NGPS DRD |
| Source section | 3.5 |
| Category / request | The instrument shall allow full 360-degree rotation of the Cass rotator without mechanical interference. |
| Importance | Must Have |
| Feature | Yes |
| Status | Compliant |
| Verification method | Test |
| Owner | Matt |
| Result | PASS |

**Requirement**

The instrument shall allow full 360-degree rotation of the Cass rotator without mechanical interference.

**Notes / verification evidence**

Works in CAD. Test at POMO

### REQ-280 — The instrument shall allow full 360-degree rotation of the Cass rotator without...

| Field | Value |
|---|---|
| Identifier | REQ-280 |
| Source | NGPS DRD |
| Source section | 3.5 |
| Category / request | The instrument shall allow full 360-degree rotation of the Cass rotator without optical vignetting of the technical field of view |
| Importance | Nice to Have |
| Feature | No |
| Status | Not Set |
| Verification method | Test |
| Result | Not Recorded |

**Requirement**

The instrument shall allow at least 180-degree rotation of the Cass rotator without optical vignetting of the acquisition field of view

### REQ-281 — The instrument shall allow full 360-degree rotation of the Cass rotator without...

| Field | Value |
|---|---|
| Identifier | REQ-281 |
| Source | NGPS DRD |
| Source section | 3.5 |
| Category / request | The instrument shall allow full 360-degree rotation of the Cass rotator without optical vignetting of the technical field of view |
| Feature | No |
| Status | Not Set |
| Verification method | N/A |
| Result | Not Recorded |

**Requirement**

The instrument shall allow full 360-degree rotation of the Cass rotator without optical vignetting of the science field of view

**Notes / verification evidence**

280 covers needs

### REQ-285 — The instrument shall have a total on-telescope mass less than the amount specif...

| Field | Value |
|---|---|
| Identifier | REQ-285 |
| Source | P200 ICD |
| Source section | 3.5 |
| Category / request | The instrument shall have a total on-telescope mass less than the amount specified in the P200 ICD including electronics racks |
| Importance | Must Have |
| Feature | Yes |
| Status | Compliant |
| Verification method | Inspection |
| Owner | Reston |
| Result | PASS |

**Requirement**

The instrument shall have a total on-telescope mass less than the amount specified in the P200 ICD including electronics racks, upper bound 3 tons

**Notes / verification evidence**

Verified loading is less than P3K

### REQ-290 — The instrument shall have a total instrument weight on the Cass rotator of less...

| Field | Value |
|---|---|
| Identifier | REQ-290 |
| Source | P200 ICD |
| Source section | 3.5 |
| Category / request | The instrument shall have a total instrument weight on the Cass rotator of less than the amount specified in the P200 ICD, including cable loading. |
| Importance | Must Have |
| Feature | Yes |
| Status | Not Set |
| Verification method | Inspection |
| Result | Not Recorded |

**Requirement**

flexur

### REQ-295 — The instrument shall require no more than XX watts of power, per P200 ICD

| Field | Value |
|---|---|
| Identifier | REQ-295 |
| Source | P200 ICD |
| Source section | 3.5 |
| Category / request | The instrument shall require no more than XX watts of power, per P200 ICD |
| Importance | Must Have |
| Feature | Yes |
| Status | Compliant |
| Verification method | Test |
| Owner | Matt |
| Result | PASS |

**Requirement**

The instrument shall require no more than 30 amps at 110V AC, per P200 ICD

**Notes / verification evidence**

13A theoretical max, Instrument not blowing fuses but should quantify

**Actions**

Measure and report current draw

### REQ-300 — The instrument shall meet all performance specifications at any Cass rotator angle

| Field | Value |
|---|---|
| Identifier | REQ-300 |
| Source | NGPS DRD |
| Source section | 3.5 |
| Category / request | The instrument shall meet all performance specifications at any Cass rotator angle |
| Importance | Must Have |
| Feature | Yes |
| Status | Compliant |
| Verification method | Design |
| Owner | Matt |
| Result | PASS |

**Requirement**

The instrument shall meet all performance specifications at any Cass rotator angle

**Notes / verification evidence**

Design includes flexure compensation

### REQ-305 — The instrument shall not preclude the addition of a rapid response capability f...

| Field | Value |
|---|---|
| Identifier | REQ-305 |
| Source | NGPS DRD |
| Source section | 3.5 |
| Category / request | The instrument shall not preclude the addition of a rapid response capability for use on an interrupt basis |
| Importance | Must Have |
| Feature | Yes |
| Status | Compliant |
| Verification method | Design |
| Result | PASS |

**Requirement**

The instrument shall not preclude the addition of a rapid response capability for use on an interrupt basis

**Notes / verification evidence**

Operations can be interrupted manually

### REQ-310 — The instrument shall meet all performance specifications in an ambient temperat...

| Field | Value |
|---|---|
| Identifier | REQ-310 |
| Source | NGPS DRD |
| Source section | 3.5 |
| Category / request | The instrument shall meet all performance specifications in an ambient temperature range of -5 C to 20 C (-10 C to 25 C goal) |
| Importance | Must Have |
| Feature | Yes |
| Status | Compliant |
| Verification method | Design |
| Result | PASS |

**Requirement**

The instrument shall meet all performance specifications in an ambient temperature range of -5 C to 25 C (-10 C to 30 C goal)

**Notes / verification evidence**

Component selection

### REQ-315 — The instrument shall meet all performance specifications in a maximum temperatu...

| Field | Value |
|---|---|
| Identifier | REQ-315 |
| Source | Temperature ranges |
| Source section | 3.5 |
| Category / request | The instrument shall meet all performance specifications in a maximum temperature gradient of 3 C per hour (5 C per hour goal) |
| Importance | Must Have |
| Feature | Yes |
| Status | Likely Compliant |
| Verification method | Design |
| Result | ACCEPT |

**Requirement**

The instrument shall meet all performance specifications in the variation of the temperature environment defined by previous data.

**Notes / verification evidence**

No test Planned

### REQ-320 — The instrument shall meet all performance specifications in an ambient relative...

| Field | Value |
|---|---|
| Identifier | REQ-320 |
| Source | NGPS DRD |
| Source section | 3.5 |
| Category / request | The instrument shall meet all performance specifications in an ambient relative humidity of 5% to 80%, non-condensing |
| Importance | Must Have |
| Feature | Yes |
| Status | Likely Compliant |
| Verification method | Design |
| Result | ACCEPT |

**Requirement**

The instrument shall meet all performance specifications in an ambient relative humidity of 5% to 80%, non-condensing

**Notes / verification evidence**

No test Planned

### REQ-325 — The instrument shall meet all performance specifications in an ambient pressure...

| Field | Value |
|---|---|
| Identifier | REQ-325 |
| Source | NGPS DRD |
| Source section | 3.5 |
| Category / request | The instrument shall meet all performance specifications in an ambient pressure range of 1676 m altitude +- 10% |
| Importance | Must Have |
| Feature | Yes |
| Status | Likely Compliant |
| Verification method | Design |
| Result | ACCEPT |

**Requirement**

The instrument shall meet all performance specifications in an ambient pressure range of 1676 m altitude +- 10%

**Notes / verification evidence**

No test Planned

### REQ-340 — The instrument shall meet all performance specifications in a gravitational fie...

| Field | Value |
|---|---|
| Identifier | REQ-340 |
| Source | NGPS DRD |
| Source section | 3.5 |
| Category / request | The instrument shall meet all performance specifications in a gravitational field of <60° zenith angle during normal operations |
| Importance | Must Have |
| Feature | Yes |
| Status | Likely Compliant |
| Verification method | Design |
| Result | ACCEPT |

**Requirement**

The instrument shall meet all performance specifications in a gravitational field of <60° zenith angle during normal operations

**Notes / verification evidence**

No test Planned

### REQ-345 — The instrument shall meet all functional specifications in a gravitational fiel...

| Field | Value |
|---|---|
| Identifier | REQ-345 |
| Source | NGPS DRD |
| Source section | 3.5 |
| Category / request | The instrument shall meet all functional specifications in a gravitational field of over 4 ster. in engineering mode |
| Importance | Must Have |
| Feature | Yes |
| Status | Likely Compliant |
| Verification method | Design |
| Result | ACCEPT |

**Requirement**

The instrument shall meet all functional specifications in a gravitational field of over 4pi ster. in engineering mode

**Notes / verification evidence**

No test Planned

### REQ-350 — The instrument shall meet all performance specifications with background light...

| Field | Value |
|---|---|
| Identifier | REQ-350 |
| Source | NGPS DRD |
| Source section | 3.5 |
| Category / request | The instrument shall meet all performance specifications with background light from being >30° from the Full Moon |
| Importance | Must Have |
| Feature | Yes |
| Status | Likely Compliant |
| Verification method | Design |
| Owner | Christoffer |
| Result | ACCEPT |

**Requirement**

The instrument shall meet all performance specifications with background light from being >30° from the Full Moon

**Notes / verification evidence**

No test Planned

### REQ-355 — The instrument shall meet all performance specifications with background light...

| Field | Value |
|---|---|
| Identifier | REQ-355 |
| Source | NGPS DRD |
| Source section | 3.5 |
| Category / request | The instrument shall meet all performance specifications with background light of the Sun >18° below horizon (>12° goal) |
| Importance | Must Have |
| Feature | Yes |
| Status | Likely Compliant |
| Verification method | Design |
| Owner | Christoffer |
| Result | ACCEPT |

**Requirement**

The instrument shall meet all performance specifications with background light of the Sun >18° below horizon (>12° goal)

**Notes / verification evidence**

No test Planned

### REQ-360 — The instrument shall survive (defined as become once again operable, meeting al...

| Field | Value |
|---|---|
| Identifier | REQ-360 |
| Source | NGPS DRD |
| Source section | 3.5 |
| Category / request | The instrument shall survive (defined as become once again operable, meeting all performance specifications after 24 hours) through an ambient temperature range of -20 C to 30 C |
| Importance | Must Have |
| Feature | Yes |
| Status | Likely Compliant |
| Verification method | Design |
| Owner | Matt |
| Result | ACCEPT |

**Requirement**

The instrument shall survive (defined as become once again operable, meeting all performance specifications after 24 hours) through an ambient temperature range of -20 C to 30 C

**Notes / verification evidence**

Per component selection, datasheets, etc

### REQ-365 — The instrument shall survive (defined as become once again operable, meeting al...

| Field | Value |
|---|---|
| Identifier | REQ-365 |
| Source | NGPS DRD |
| Source section | 3.5 |
| Category / request | The instrument shall survive (defined as become once again operable, meeting all performance specifications after 24 hours) through a maximum temperature gradient of 10 C per hour |
| Importance | Must Have |
| Feature | Yes |
| Status | Likely Compliant |
| Verification method | Design |
| Owner | Matt |
| Result | ACCEPT |

**Requirement**

The instrument shall survive (defined as become once again operable, meeting all performance specifications after 24 hours) through a maximum temperature gradient of 10 C per hour

**Notes / verification evidence**

Covers shipping/storage

### REQ-370 — The instrument shall survive (defined as become once again operable, meeting al...

| Field | Value |
|---|---|
| Identifier | REQ-370 |
| Source | NGPS DRD |
| Source section | 3.5 |
| Category / request | The instrument shall survive (defined as become once again operable, meeting all performance specifications after 24 hours) through an ambient relative humidity of 0% to 100%, condensing |
| Importance | Must Have |
| Feature | Yes |
| Status | Likely Compliant |
| Verification method | Design |
| Owner | Matt |
| Result | ACCEPT |

**Requirement**

The instrument shall survive (defined as become once again operable, meeting all performance specifications after 24 hours) through an ambient relative humidity of 0% to 100%, condensing

**Notes / verification evidence**

No test Planned

### REQ-375 — The instrument shall survive (defined as become once again operable, meeting al...

| Field | Value |
|---|---|
| Identifier | REQ-375 |
| Source | NGPS DRD |
| Source section | 3.5 |
| Category / request | The instrument shall survive (defined as become once again operable, meeting all performance specifications after 24 hours) through an ambient pressure range of 700 hPa to 1100 hPa |
| Importance | Must Have |
| Feature | Yes |
| Status | Compliant |
| Verification method | Design |
| Owner | Matt |
| Result | PASS |

**Requirement**

The instrument shall survive (defined as become once again operable, meeting all performance specifications after 24 hours) through an ambient pressure range of 800 hPa to 1100 hPa (operating at sea level to 5500ft)

### REQ-380 — The instrument shall survive (defined as become once again operable, meeting al...

| Field | Value |
|---|---|
| Identifier | REQ-380 |
| Source | NGPS DRD |
| Source section | 3.5 |
| Category / request | The instrument shall survive (defined as become once again operable, meeting all performance specifications after 24 hours) through a ground vibration of 0.5 g acceleration vertical |
| Importance | Must Have |
| Feature | Yes |
| Status | Likely Compliant |
| Verification method | Design |
| Owner | Matt |
| Result | ACCEPT |

**Requirement**

The instrument shall survive (defined as become once again operable, meeting all performance specifications after 24 hours) through a ground vibration of 0.5 g acceleration vertical

**Notes / verification evidence**

No test Planned

### REQ-385 — The instrument shall survive (defined as become once again operable, meeting al...

| Field | Value |
|---|---|
| Identifier | REQ-385 |
| Source | NGPS DRD |
| Source section | 3.5 |
| Category / request | The instrument shall survive (defined as become once again operable, meeting all performance specifications after 24 hours) through a gravitational field of 4 ster |
| Importance | Must Have |
| Feature | Yes |
| Status | Likely Compliant |
| Verification method | Design |
| Owner | Matt |
| Result | ACCEPT |

**Requirement**

The instrument shall survive (defined as become once again operable, meeting all performance specifications after 24 hours) through a gravitational field of 4pi ster

**Notes / verification evidence**

No test Planned

### REQ-390 — The instrument shall survive (defined as become once again operable, meeting al...

| Field | Value |
|---|---|
| Identifier | REQ-390 |
| Source | NGPS DRD |
| Source section | 3.5 |
| Category / request | The instrument shall survive (defined as become once again operable, meeting all performance specifications after 24 hours) direct pointing at the Full Moon |
| Importance | Must Have |
| Feature | Yes |
| Status | Likely Compliant |
| Verification method | Design |
| Owner | Christoffer |
| Result | ACCEPT |

**Requirement**

The instrument shall survive (defined as become once again operable, meeting all performance specifications after 24 hours) direct pointing at the Full Moon

**Notes / verification evidence**

No test Planned
