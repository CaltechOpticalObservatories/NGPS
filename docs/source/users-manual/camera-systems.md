# Camera Systems

## Optical Design

The G, R, and I spectrograph cameras use Petzval-type designs. This design family has a positive group in the front, a positive group in the middle, and a negative group near the image. The Petzval is a common design family for spectrograph cameras because it can accommodate an external aperture stop and can achieve relatively high performance and loose tolerances for fields of a few degrees and apertures of F/2-3. Designs are constrained to have no vignetting.

```{figure} assets/page_048_image_01.png
:alt: NGPS manual figure
:width: 90%

```

## Focus

Internal spectrograph focus for the G, R, and I channels (reimaging of the instrument entrance slit onto the detector) is accomplished by actuating the second to last element in the camera (element before the field flattener=dewar window). The mechanisms are PI M-D01.11S actuators with PI C-663.12 controllers. The stages need to be homed and that is done by the instrument software when the system is initialized. The nominal stage range is 0 mm to 10 mm. The 0 mm position is closest to the center of the NGPS instrument, 10 mm position would be furthest out, however, the mechanical design does not allow motion out to the full 10 mm. The stage encounters a hard stop prior to that reading and, as it is a stepper stage and there is no limit switch, running into that hard stop will cause skipped steps and the stage reading to not be accurate. A component shutdown and daemon restart/rehome will resolve this. The focusd daemon does not permit commanding of the stage outside of its real physical range. The table below lists the nominal focus and max value for each of the G, R, and I focus axes.

Focus Axis Typical Max

G 3.3 7.8

R 2.7 6.7

I 5.1 7.6

As implemented, the mechanical focus change reading corresponds almost exactly to a shift in focus position at the detector.

Note that for the U channel, the focus axis is at the collimator and that functionality is discussed there.

## U Camera

The U channel is a Schmidt design, including a transmissive aspherical corrector plane. This was chosen to minimize UV losses and improve imaging performance.

Documentation on the U channel design can be found in NGP.SPC.TEC.020.

## Detector Systems

NGPS has four detector systems, they are referred to as U, G, R, and I channels. To reduce risk and cost, the instrument has repurposed the two Oxford SWIFT detector systems [REFERENCE] and the two detector systems from Double Spectrograph [REFERENCE].

Our documentation for the OSWIFT systems can be found here.

Our documentation for the DoubleSpec systems can be found here.

We have retained the original readout electronics and associated harnessing.

All front ends (instrument mounting interface, entrance windows) were rebuilt. Dewars were modified to include ½ tank depth stinger tubes to effectively eliminate liquid nitrogen spillage as the telescope slews on sky.

G, R, and I detector mounts include independent in-plane adjustments and tip tilt adjustments.

G, R, and I channels are now equipped with frame transfer masks that cover ½ of the physical detector. The other ½ is where light from the spectrograph is collected. In principle, these devices could now be

used in frame transfer mode. Unfortunately, the U device has the vertical and horizontal clocks running in directions that do not permit this, so a full 4 channel frame transfer mode is not possible.

### U Channel

The U channel cryostat is the former Double Spectrograph Blue detector system. Not many changes to this system. We refabricated the thermal sensing and control harness. Drawing can be found at NGPS_Temperature_Sensor_Cables.pdf with pinout.

The cold link on the U channel is slightly undersized. The operating temperature is safe, but slightly elevated compared to what we would ideally want.

### G Channel

The G channel cryostat is the former Double Spectrograph Red detector system. We have replaced the dewar with a very similar (but longer) design to improve hold time. We have retained the same electrical interface to the tank thermal sensors and heater. We refabricated the thermal sensing and control harness. Drawing can be found at NGPS_Temperature_Sensor_Cables.pdf with pinout. We replaced the detector, as the original DBSP R detector was damaged by excessive illumination.

### R Channel

This is one of the OSWIFT detectors.

We have seen an intermittent connection issue on the clocks for this system. This exhibits by lack of structure in the serial direction, including lack of overscan, and banding in the parallel direction. The issue has occurred twice since December 2024. A reseating of the connectors at the detector head resolved it both times. An inspection of the harness has not revealed an obvious defect. We are evaluating options as of this writing (May 2026).

### I Channel

This is one of the OSWIFT detectors.

Chan Device Physical Used Gain (e/DN) RN (e rms) DC (e/px/h)

U E2V CCD44-82 BI 2048x4096 1028x4096 0.755 2.8 TBC

G LBNL SNAP CCD 2040x4114 1020x4114 2.8 7.8 TBC

R LBNL SNAP CCD 2040x4114 1020x4114 0.9 3.7 TBC

I LBNL SNAP CCD 2040x4114 1020x4114 0.86 4.6 TBC

Table of read-out times?

### Lakeshores

NGPS has two Lakeshore 336 temperature controllers. Channel assignments are as follows:

Lakeshore U-R Description Typical value (setpoint) [K]

A U CCD 168* (163)

B U Dewar 75.3

C R CCD 128 (128)

D R Dewar 76.5

Lakeshore G-I Description Typical value (setpoint) [K]

A G CCD 128 (128)

B G Dewar 82.6

C I CCD 130 (130)

D I Dewar 76.5

### Pressure gauges

Each cryostat has an Edwards D147010000 wide range gauge and they are read out in pairs by two Edwards ADC dual channel standard controllers.

Typical operating values when cold, are between few x 10-7 to few x 10-6 Torr. Generally, during a cooldown, it takes a cryostat several days to reach 10-7 Torr levels.

### Cooling guidance

All four dewars are equipped with active charcoal getters. When preparing to cool, we generally like to pump till we get down to 10-4 to 10-3 Torr range then close the pump valve as we start filling. When possible allow for an extended warm pump down to clear out any accumulated moisture, etc.

The cooldown time for all dewars is several hours. It’s generally OK to take images with G, R, and I once the temperature comes down to below 160K. U can be exposed a bit warmer.

The hold times for the dewars are approximately:

U: 24h

G: 18h

R: 17h

I: 17h

Figure 13: NGPS G Channel cooldown curve. Other cooldown curves are similar. The spike in the blue and red traces at 15:25 corresponds to a fill.
