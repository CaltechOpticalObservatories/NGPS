# Observing

## Choosing Slit Width and Detector Binning

The slit width sets the approximate spectral resolution and should be chosen based on the science goal, seeing, and required signal-to-noise ratio. The detector binning should normally be chosen to match the slit width and seeing. The recommended values below are given as BINSPAT x BINSPEC, i.e. spatial binning by spectral binning.

| Seeing (FWHM) | 0.5" slit | 1.0" slit | 1.5" slit | 2.0" slit |
| --- | --- | --- | --- | --- |
| 1.0" | 2x1 | 2x2 | 2x3 | 2x4 |
| 1.5" | 2x1 | 2x2 | 2x3 | 2x4 |
| >2.0" | 4x1 | 4x2 | 4x3 | 4x4 |

*Table 1: Recommended NGPS detector binning configurations. Entries are given as BINSPAT x BINSPEC, i.e. spatial binning by spectral binning.*

For most observations, spatial binning of 2 is adequate. Use spatial binning of 4 mainly in poor seeing. Spectral binning should usually scale with slit width: 1 for a 0.5" slit, 2 for a 1.0" slit, 3 for a 1.5" slit, and 4 for a 2.0" slit. Configurations with binning 1-4 in both spatial and spectral directions have been tested. Slit widths between 0.37" and 10" have been tested.

## Notes on Guiding

The NGPS observing system will automatically enter guiding mode as part of the target acquisition loop when the system has completed the astrometric solution and acquired the target close to the slit position. Once the system is guiding, offsets can be applied by the user to perfect the target centering on the slit graphic. The science offset can also be applied while the system is guiding. In normal operations there is typically no reason to disable guiding at any step.

The method NGPS uses to guide is tied to the astrometric solver, and requires >4 stars detected on ACAM. During clear observing conditions this allows >99% of fields to be solved (based on catalog cross-matching and astrometric solver simulations and the ACAM's field size). However, in bright time or bad seeing conditions, it is important to compensate with longer ACAM exposure times. Exposure times up to 30s have been tested and can maintain guiding.

The drift while guiding has been measured using the slice viewer cameras to be < 0.05 arcsec, with an uncertainty on the measurement of 0.06 arcsec over 30 minutes of guiding (this is consistent with zero drift). If the system drops out of guiding due to very bad seeing or intermittent cloud coverage, it is still possible for the 200-inch to maintain a target on slit for typical 1"-1.5" slits for a 15 minute exposure. The measured un-guided drift is < 0.5" in 15 minutes. This performance may degrade with increased zenith distance but the behavior is not fully characterized (basic tests do not show an obvious correlation).
