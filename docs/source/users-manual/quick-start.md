# Quick Start Guide

This section gives the minimum workflow needed to begin observing with NGPS. More detailed descriptions of the individual GUIs, target-list format, calibrations, acquisition tools, and data reduction are given in the later sections of this manual.

Before the run, prepare a target list in CSV format. At minimum, include target name, RA, and DECL; for efficient observing also include the intended exposure time, slit width, number of exposures, binning, and slit angle where known. The .csv must have a header present (e.g. NAME, RA, DECL). Use PA for the slit angle when the target should be observed at the parallactic angle. Split long integrations into exposures of 900 s or less to reduce cosmic-ray impact.

Connect to the Palomar observing systems from an approved network, normally through the Caltech VPN or the Caltech remote observing room. The NGPS VNC sessions are started automatically: the TCS operator uses the On-Target GUI, the observer uses the ACAM/SCAM acquisition GUIs and the main NGPS GUI, and an additional VNC session is available for ds9 or quick-look inspection. The instrument control software normally starts automatically; if needed, it can be started from a terminal with ngps start. Do not run browsers or heavy data analysis on the instrument-control computer.

Load the target list into the NGPS GUI. NGPS is database-driven: the sequencer observes targets in the order stored in the target database, although pending targets can be edited, reordered, or inserted during the night.

Choose the observing setup before calibrations (your target list should already define what slit width and binning settings you want). Typical binning choices are (spatial x spectral) 2x1 for a 0.4 arcsec slit, 2x2 for a 1.0 arcsec slit, 2x3 for a 1.5 arcsec slit, and 4x4 for a 2.0 arcsec slit or poor seeing. In most conditions, spatial binning of 2 is adequate; use larger spatial binning only when the seeing is very poor. Take calibrations for every binning mode used and take dome flats for every slit-width plus binning combination used for science.

Calibration workflow note: some calibration workflows are changing. The older observing manual describes the Calibration GUI and separate getcalib_cf scripts for long U and G calibrations. The intended workflow is instead to generate calibration target lists from the GUI: choose the desired internal calibration or dome-flat sequence, choose the relevant binning and/or slit width, generate the calibration target list, and run it through the normal sequencer by clicking Go. Internal calibrations are normally selected by binning; dome flats are selected by both slit width and binning. Inspect the resulting frames: arcs should show unsaturated lines, biases should have low counts, and dome flats should be roughly in the useful high-count regime (ideally >10,000 counts) without saturation in the brightest parts.

The Support Astronomer will usually perform internal spectrograph focus during daytime setup. Before science observing, configure the instrument for science, open the dome at the appropriate time, and focus the telescope on a suitable bright star using ACAM. Observe at least one spectrophotometric

standard star near the beginning and one near the end of the night, using the same slit-width and binning configurations used for science. Avoid saturation in standards and aim for enough counts to generate a reliable sensitivity function.

## Observing

There are two observing modes selectable in the main GUI. Single and All. Use Single mode for manual observing or while learning the system; use All mode only when the target list and observing conditions are well defined. The user can also choose between automatic and manual acquisition modes in the main GUI.

In the All mode, if automatic acquisition is enabled, the system will proceed automatically through the target list with the settings (slit width, binning, exposure times) defined by the list without waiting for user input or confirmation with the exception of the TCS operator needing to initiate the actual move and acknowledge when the TCS is ready to observe (see the On-Target GUI page).

In Single mode: For each target, select the row in the target list, confirm the exposure time, slit width, slit angle, binning, and number of exposures, then click Go. The TCS operator must approve and perform the slew; once the telescope is ready, the operator clicks On Target and NGPS proceeds. After the slew, use the SLICEVIEW and GUIDER GUIs to acquire the target and turn on guiding. The SLICEVIEW GUI shows the slit-viewing field and a virtual slit marker; use acquire last slew, put on slit, and small jogs to place the target accurately on the virtual slit. For faint targets, offset-star acquisition may be required. Guiding should be on if sky conditions allow before any offset is applied since this results in a more accurate offset compared to an unguided tcs offset.

When the target is acquired and guiding is stable, click Expose. Repeat may be used for another exposure at the same location. The target is reloaded from the database and camera parameters are updated; therefore, you can use a different exposure time and/or binning if you update the database before clicking repeat.

During the night, use the Quicklook DRP to inspect new data as they arrive. Confirm that the trace is in the expected place, check the 2D frames for obvious acquisition or calibration problems, inspect the extracted 1D spectrum, and use stacking/SNR estimates to decide whether additional exposure time is needed.

At the end of the night, make sure the raw and reduced data have been transferred or archived according to the current run policy. Raw files are stored under /data/YYYYMMDD/. Then use Shutdown from the NGPS menu in the main GUI; this closes dust covers and powers off the relevant subsystems while leaving the control software infrastructure available.
