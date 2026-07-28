# Technical Specifications

This page summarizes the NGPS technical specifications from the May 14, 2026 Version 2 specification sheet.

## General Specifications

| Category | Specification |
| --- | --- |
| Instrument type | 4-channel UV-NIR spectrograph |
| Wavelength coverage | 3050 to 10400 |
| Telescope focus | Cassegrain focus at P200, F/13.6, 2.54 arcsec/mm |
| Slit / IFU | 3-slit adjustable-width IFU |
| Slice length | 50 arcsec long |
| Slice width | 0.36 to 10 arcsec wide per slice |
| Spectral resolution | R > 4000 for the narrowest slit setting |
| Exposure timing | Common exposure time |
| Detector binning | Independently binned spatial and spectral directions |
| Readout mode | BOI mode |

## Channel Performance

| Channel | Bandpass | Max Efficiency, Slit | Spectral Resolution at 0.4 arcsec | Spatial Resolution |
| --- | ---: | ---: | ---: | --- |
| U | 3050 to 4430 | 65% | 4100 | Seeing limited |
| G | 4250 to 5960 | 60% | 4200 | Seeing limited |
| R | 5620 to 7950 | 74% | 4300 | Seeing limited |
| I | 7530 to 10400 | 74% | 4500 | Seeing limited |

## Detector and Readout Properties

| Channel | Plate Scale (arcsec/pixel) | Dispersion (A/pixel) | Read Noise (e rms) | Dark Current (e/pixel/hour) |
| --- | ---: | ---: | ---: | ---: |
| U | 0.193 | 0.340 | 2.8 | < 5.6 TBC |
| G | 0.189 | 0.435 | 7.8 | < 1.7 |
| R | 0.186 | 0.577 | 3.7 | < 1.2 |
| I | 0.186 | 0.713 | 4.6 | < 1.0 |

## Detector Format and Cosmic-Ray Rate

| Channel | Active Detector Region | Pixel Size | Cosmic-Ray Rate (% pixel/hour) | 2x1 Bin Readout Time (s) |
| --- | --- | --- | ---: | ---: |
| U | 1024 x 4102 | 15 um square pixels | 1.1 | 64 |
| G | 1020 x 4114 | 15 um square pixels | 2.8 | 31 |
| R | 1020 x 4114 | 15 um square pixels | 3.2 | 55 |
| I | 1020 x 4114 | 15 um square pixels | 2.1 | 55 |

## Calibration System

| Area | Specification |
| --- | --- |
| Calibration input | Telescope-mimicking f/14 telecentric input beam |
| Arc lamps | ThAr, FeAr |
| Continuum sources | Quartz halogen continuum, red; laser-pumped continuum, blue |
| Additional calibration sources | UV LEDs, etalon spectra |
| Calibration control | Concurrent calibration, individual lamp shutters, mechanical PWM control |
| Dome lamps | Palomar dome lamps: low, high, ultrahigh, He |

## Acquisition and Guidance

| Area | Specification |
| --- | --- |
| Acquisition and guidance system | Offset guider |
| Field of view | 4.44 x 4.15 |
| Pixel scale | 0.26 arcsec/pixel |
| Astrometry | Full-field solution |
| Filters | Photometric filters: g', r', i', clear |
| Camera mode | EMCCD mode |

## Slice Viewing Cameras

| Area | Specification |
| --- | --- |
| Configuration | Two fields of view adjacent to outer slices |
| Field of view | 21 x 50 arcsec each |
| Pixel scale | 0.060 arcsec/pixel |

## Additional Features

| Feature |
| --- |
| Exposure time calculator |
| Automated target acquisition |
| Guided afternoon calibration |
| Quick-look pipeline |
| Flexure compensation |
| Focus compensation |
| Observation Timeline Modeler (OTM) |
| Automatic binning adjustment |
| Frame transfer capability |

