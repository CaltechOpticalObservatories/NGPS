# Calibrations

Figure 1: The ngps_cals terminal window keeps track of the required calibration data

for the setups where either science data has been taken (IMGTYPE=SCI) or where

partial calibration data has started to be taken

NGPS calibrations are tied to the detector binning and slit-width configurations used for science observations. The basic rule is: take arcs and biases for every binning mode used, and take dome flats for every slit-width plus binning combination used. It is generally acceptable to use the same slit widths

```{figure} assets/page_008_image_01.png
:alt: NGPS manual figure
:width: 90%

```

on internal cals as what will later be used for science if the slit width is picked to track typical seeing conditions (2” or narrower). If very wide slits will be used for science, use a narrower slit to take internal cals (so that arc lines can be fit for the wavelength solutions). Configurations that are not used for science or standards do not need to be calibrated.

Calibration observations are executed through the normal NGPS sequencer. Observers should use the GUI to generate calibration target lists for the desired internal calibrations or dome-flat sequences, then run those target lists with Go. Internal calibration target lists are generated for a selected binning mode. Dome-flat target lists are generated for a selected binning mode and slit width. This replaces the older Calibration GUI workflow and the separate getcalib_cf scripts used for long U and G calibrations.

A complete internal calibration set for a binning mode consists of the required arc and bias frames for each active spectrograph channel. The standard requirements are 3 ThAr arcs, 3 FeAr arcs, and 7 bias frames per channel and binning mode. Arc frames should show clear, unsaturated lines. Bias frames should have normal low count levels and no obvious detector problems. Internal flats may be acquired as part of some calibration sequences, but the key required products tracked for wavelength calibration and bias subtraction are the arcs and biases.

Dome flats are required for each slit width and binning combination used for science or standards. A complete dome-flat set consists of at least 5 dome flats per channel for each setup (7-10 are preferred for U and G channels). Before taking dome flats, confirm with the Support Astronomer or Telescope Operator that the dome is dark, the telescope and mirror-cover configuration are appropriate, and the high lamp can be turned on. Set the ACAM and SCAM exposure times to zero before using the high lamp. Dome flats should have useful high counts without saturation.

Calibration completeness can be checked from a terminal with the terminal command ngps_cals (this is normally started by SAs and should be already running when observers start taking cals.

ngps_cals opens the NGPS calibration-status display shown in Figure 3-1. The top table, “Arcs/Bias by binning,” groups internal calibrations by detector binning, listed as BINSPAT x BINSPEC. For each channel, the THAR, FEAR, and BIAS columns show the number of valid frames found compared with the number required. For example, 3/3 means that all required frames of that type are present. The SLITW<=2 columns indicate how many of the arc frames satisfy the slit-width criterion used for wavelength calibration. The SCI column shows how many science frames were found for that channel and binning setup.

The lower table, “Dome flats by setup,” groups dome flats by detector binning and slit width. For each setup and channel, the DOMEFLAT column shows the number of valid dome flats found compared with the required number. Green entries indicate complete calibration sets; red entries indicate missing or incomplete sets. The summary lines give the number of complete setups compared with the number of required setups for each channel.

The status display can be left open while calibrations are being taken. If a science setup appears in red, generate and run the missing calibration target list before relying on that configuration for science-quality reductions. If only partial data is missing, the calibration target list can be modified by setting NEXP to 0 for frames that are not needed, and then re-running the calibration list with the Go button. If a new binning or slit width is introduced during the night, rerun ngps_cals and obtain the missing calibrations for that setup.

Add example ThAr exposure figure. Discuss.

Add example FeAr exposure figure. Discuss.

Add example dome flat exposure(s) figure. Discuss.

## Inter-Target Calibration

Calibration measurements can be automatically performed during the night, between science targets. To do this, you can include any one or more of the calibration targets along with your science targets when you prepare your target list, using one of the reserved calibration target names listed in the table below. Along with the special target name, set your desired binning, slit width, and exposure time. When the sequencer loads this target, it will configure all subsystems (lamps, calibration covers, etc.) as needed for that type of calibration. No telescope moves will take place. When the system loads your next science target, the instrument will be automatically reconfigured, and science observations will continue.

Calibration Targets:

CAL_THAR CAL_THAR_UG CAL_FEAR CAL_FEAR_UG CAL_CONTR CAL_CONTB CAL_DOME CAL_DOME_UG CAL_BIAS CAL_DARK

Calibration targets with the “_UG” suffix will use only those channels -- channels I,R are disabled.
