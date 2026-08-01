# Calibration

```{figure} assets/calibrations-01.png
:alt: NGPS manual figure
:width: 90%
```

NGPS calibrations are tied to the detector binning and slit-width configurations used for science observations. The basic rule is: take arcs and biases for every binning mode used, and take dome flats for every slit-width plus binning combination used. It is generally acceptable to use the same slit widths on internal cals as what will later be used for science if the slit width is picked to track typical seeing conditions (2" or narrower). If very wide slits will be used for science, use a narrower slit to take internal cals (so that arc lines can be fit for the wavelength solutions). Configurations that are not used for science or standards do not need to be calibrated.

Calibration observations are executed through the normal NGPS sequencer. Observers should use the GUI to generate calibration target lists for the desired internal calibrations or dome-flat sequences, then run those target lists with **Go**. Internal calibration target lists are generated for a selected binning mode. Dome-flat target lists are generated for a selected binning mode and slit width. This replaces the older Calibration GUI workflow and the separate getcalib_cf scripts used for long U and G calibrations.

A complete internal calibration set for a binning mode consists of the required arc and bias frames for each active spectrograph channel. The standard requirements are 3 ThAr arcs, 3 FeAr arcs, and 7 bias frames per channel and binning mode. Arc frames should show clear, unsaturated lines. Bias frames should have normal low count levels and no obvious detector problems. Internal flats may be acquired as part of some calibration sequences, but the key required products tracked for wavelength calibration and bias subtraction are the arcs and biases.

Dome flats are required for each slit width and binning combination used for science or standards. A complete dome-flat set consists of at least 5 dome flats per channel for each setup (7-10 are preferred for U and G channels). Before taking dome flats, confirm with the Support Astronomer or Telescope Operator that the dome is dark, the telescope and mirror-cover configuration are appropriate, and the high lamp can be turned on. Set the ACAM and SCAM exposure times to zero before using the high lamp. Dome flats should have useful high counts without saturation.

Calibration completeness can be checked from a terminal with the terminal command ngps_cals (this is normally started by SAs and should already be running when observers start taking cals).

ngps_cals opens the NGPS calibration-status display. The top table, "Arcs/Bias by binning," groups internal calibrations by detector binning, listed as BINSPAT x BINSPEC. For each channel, the THAR, FEAR, and BIAS columns show the number of valid frames found compared with the number required. For example, 3/3 means that all required frames of that type are present. The SLITW<=2 columns indicate how many of the arc frames satisfy the slit-width criterion used for wavelength calibration. The SCI column shows how many science frames were found for that channel and binning setup.

The lower table, "Dome flats by setup," groups dome flats by detector binning and slit width. For each setup and channel, the DOMEFLAT column shows the number of valid dome flats found compared with the required number. Green entries indicate complete calibration sets; red entries indicate missing or incomplete sets. The summary lines give the number of complete setups compared with the number of required setups for each channel.

The status display can be left open while calibrations are being taken. If a science setup appears in red, generate and run the missing calibration target list before relying on that configuration for science-quality reductions. If only partial data is missing, the calibration target list can be modified by setting NEXP to 0 for frames that are not needed, and then re-running the calibration list with the **Go** button. If a new binning or slit width is introduced during the night, rerun ngps_cals and obtain the missing calibrations for that setup.

## Wavelength Calibration

The internal NGPS arc lamps provide usable calibration lines only at wavelengths longer than approximately 3500 Å. Consequently, the extrapolated wavelength solution near the blue edge of the U-channel detector, around 3200 Å, may drift by a few Å.

He dome-lamp exposures can be obtained to anchor the wavelength solution to approximately 3200 Å using the He I 3187 Å line. This line is detected near the blue edge of the **central U-channel slice**. The exposures can be included in the regular dome-flat calibration target list by setting NEXP > 0 for the CAL_DOMEHE entries in the automatically generated calibration target list. Both the automatic quicklook reduction and PypeIt will use these calibration data when they are available. If He dome arcs are obtained, an RMS of < 0.1 Å can be expected at 3200 Å.

For an accurate wavelength solution to 3200 Å on the **U-channel side slices**, we recommend observing a bright planetary nebula during twilight. The planetary-nebula spectrum provides the same He I 3187 Å line on all slices, allowing the wavelength solution to be anchored independently across the full image slicer. The table below lists suitable planetary nebulae visible from Palomar near the start of the night at different times of year. If the side slices will not be used for science, this data is of course not necessary.

| Target | RA (J2000) | Dec (J2000) | Approx. evening twilight window | Published He I 3187 detection |
| --- | --- | --- | --- | --- |
| NGC 2392 | 07:29:10.77 | +20:54:42.5 | February-May | He I λ3187 detected; log observed flux = -11.96 (quality c). Aller & Keyes (1980/1981). |
| NGC 6210 | 16:44:29.52 | +23:47:59.4 | June-October | He I λ3187.74 measured directly; reddening-corrected flux ratio I(3187.7)/I(3444.1) = 2.189. O'Dell & Miller (1992). |
| NGC 6818 | 19:43:57.77 | -14:09:13.4 | August-November | Observed at 3187.11 Å and identified as He I λ3187.74; dereddened intensity = 2.20 for Hβ = 100. Tsamis et al. (2003). |
| NGC 7009 | 21:04:10.82 | -11:21:48.6 | September-December | He I λ3187.74 measured directly; observed intensity = 0.742 relative to He I λ4471 = 1. Fang & Liu (2013). |
| NGC 7027 | 21:07:01.57 | +42:14:10.5 | August-January | He I λ3187.74 measured directly; reddening-corrected flux ratio I(3187.7)/I(3444.1) = 0.086. O'Dell & Miller (1992). |
| NGC 7662 | 23:25:53.83 | +42:32:05.8 | September-February | He I λ3187.74 measured directly; reddening-corrected flux ratio I(3187.7)/I(3444.1) = 0.094. O'Dell & Miller (1992). |

## Inter-Target Calibration

Calibration measurements can be automatically performed during the night, between science targets. To do this, you can include any one or more of the calibration targets along with your science targets when you prepare your target list, using one of the reserved calibration target names listed below. Along with the special target name, set your desired binning, slit width, and exposure time. When the sequencer loads this target, it will configure all subsystems (lamps, calibration covers, etc.) as needed for that type of calibration. No telescope moves will take place. When the system loads your next science target, the instrument will be automatically reconfigured, and science observations will continue.

**Calibration Targets:**

- CAL_THAR
- CAL_THAR_UG
- CAL_FEAR
- CAL_FEAR_UG
- CAL_CONTR
- CAL_CONTB
- CAL_DOME
- CAL_DOME_UG
- CAL_DOMEHE
- CAL_DOMEHE_UG
- CAL_BIAS
- CAL_DARK

Calibration targets with the "_UG" suffix will use only those channels -- channels I,R are disabled.
