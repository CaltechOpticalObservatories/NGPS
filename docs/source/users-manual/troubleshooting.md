# Troubleshooting

## Known Hardware Issues

The following are the known outstanding hardware issues. While an attempt at completeness was made, there may be items missing. Ideally, this list would be periodically updated.

### R channel intermittent behavior [May 2026]

This problem has occurred twice since commissioning in late 2024 (as of this writing, May 2026). Some clocks on the R channel were not being transmitted to the device and the images showed horizontal banding. The issue is clearly tied to the connectors on the outside of the detector enclosure (air side). Reseating the connectors resolved the issue both times. A prior investigation of the connectors did not reveal any obvious defects.

We have spare connectors and are assessing whether to debug existing cables or build a new harness.

The repair here, while not strictly invasive, would need to be planned to not interfere with observations.

### Flexure compensation [May 2026]

NGPS has flexure-compensating tip/tilt stages. These are fully functional. We need acquire collimator tip/tilt to detector shift mapping, and need to acquire data to measure the flexure shifts with telescope/Cass ring angle position. The alignment of R and I channels changed recently. U and G take much longer to acquire due to long arc lamp exposure times; once U channel BOI mode is fully functional we’ll be in a position to collect the requisite data.

Intend to implement by 2026B. Should not impact observers negatively.

### U Channel Binning with BOI mode [May 2026]

The U channel detector is an E2V device (vs. LBNL for the other channels) and the controller firmware is different. BOI with binning mode implementation is in progress.

Intend to implement by 2026B

The U channel spectrum falls at the center of the CCD. Above functionality will allow us to save on readout time.

Requires modification of the detector firmware. Should not impact observers negatively, can always roll back to current version of the software.

### A&G hatch [May 2026]

The hatch on the A&G system is not operational. We suspect a faulty limit switch.

Intend to repair by 2026B

Should not affect observers.

### U focus automation [May 2026]

The U channel focus control electronics were a late change. The stage is operational, but the software interface layer needed to command it from the main ICS is not yet available.

Intend to complete by 2026B

This is a key feature that is missing. Work associated with implementing this will not affect observers.

### Improve Spectrograph Focus [May 2026]

The current performance of the 4 channels is adequate. Due to late installation of the U and G channels (without the benefit of some of the lab alignment tools) and recent retrofit work on R and I channels (and need for slight subsequent realignment) we do see room for improvement.

Desire to complete by 2026B

Turnaround time for an adjustment for a single camera is not long and does not require a thermal cycle. After the adjustment, the spectra may be in a slightly different place on the detector than they are currently, so there will be some pipeline work and extra effort for those programs that wish to combine pre- and post- adjustment data.

As part of this work we would also rotate the U detector to ensure better alignment of the dispersion direction with spectrograph pixels

### NPS power up issue [May 2026]

There are four network power supplies in the NGPS electronics rack. When the instrument is first powered up, sometimes a fraction of these NPSes do not show up on the network. A power cycle generally resolves the issue. This workaround is not ideal, as all of the hardware attached to the NPS being cycled is cycled as well. Typically only occurs on days when the instrument is installed or removed from the telescope.

We don’t have a clear solution here; some hardware may need to be added to resolve (e.g., NPSes on an external “always-up” network, instead of the private instrument network.

Work on this should be transparent to the observers.

### Light leaks [May 2026]

The instrument enclosure is not yet fully light tight. Need to run a in-Cass cage test to locate the issue.

Intend to complete by 2026B

Work on this should be transparent to the observers and should benefit them.

### G channel read noise

The G channel read noise is still slightly elevated from where we would like it. After initial deployment of the channel we were seeing 25 e rms which we have subsequently reduced to 8 e rms via minor hardware adjustments. Further reduction will likely involve modification of the DSP code to reduce gain and slow down the readout.

We would like to complete by 2026B

Work on this will require modifications to the detector firmware. This can be done in a manner that does not affect observers (we can always roll back to a previously working version).

### Image slicer rotation

There appears to be a ~< 1 skew between the slicer spatial direction and dispersion direction. We can correct for this by rotating the image slicer structure on top of the instrument. The gain from implementing this helps with reducing spectral resolution loss when binning.

Considering implementing by 2026B

This work is somewhat invasive and would cause minor pipeline work. Pre-and post work data would need to be reduced differently.

### Frame transfer for G, R, and I

Frame transfer mode has not been tested for G, R, and I. It is not possible for U given the detector. If there are science cases where U is not needed and frame transfer is desired for G, R, and I, we could deploy this.

Intend to wait for U channel upgrade in the next 1-2 yrs to implement frame transfer on other channels

The work would not be invasive or affect observers.

### Image slicer non-compliance and contamination [May 2026]

The image slicer optics were accepted without meeting full set of requirements due to schedule and funding constraints. A future slicer upgrade is planned to address the shortcomings.

The instrument does not have an entrance window, we see dust accumulation on the slicer optics. The main slit is unaffected. The optics can be cleaned, but that requires a partial disassembly of the instrument front end (removal of the calibration system). We would like to do this at the same time as installing an instrument window. To accomplish this, need to identify suitable full-bandpass window and may need to revisit guider-IFU confocality alignment.

It is likely that future addition of SIGHT will protect the instrument sufficiently, such that an instrument window is not needed.

This work is invasive and needs to be scheduled. Some risk to the image slicer prisms with mechanical cleaning.

### In-Line calibration unit non-compliance [May 2026]

The initial design called for inline calibration fibers in conjunction with etalon light or arc lamp light. These were intended to help with flexure compensation.

These fibers were installed late and we found they introduced scattered and ghost light into the spectra. Additionally, their spectral signatures change and move with slit width adjustment.

As we are optimistically anticipating a revision of the slicer assembly, we intend to include an optimized version of this spectral reference in that work.

The work to get this going “as is” is not invasive. Any rework would be.

## Issue Reporting

Issue reporting is easy – from any terminal type:

gecko –u ‘yourname’ -m ‘brief descriptive message’

It is important to report all issues as soon as they occur because the time stamps generated by the gecko reporting tool will be used to help diagnose the issue.

Reports will be managed and responded to by the COO software group.
