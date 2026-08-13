# Detector and Command Reference

## CCDs

This appendix describes the available CCD amplifiers, their naming conventions, and other related details.

### G,R,I Channels

```{figure} assets/detector-and-command-reference-01.png
:alt: NGPS manual figure
:width: 90%
```

```{figure} assets/detector-and-command-reference-02.png
:alt: NGPS manual figure
:width: 90%
```

### U Channel

## CLI Commands

This section contains the recognized command line interface (CLI) commands.

*[Not yet implemented. Incomplete list.]*

| Command | Description |
|---|---|
| `move_to_target [ ra=<hh:mm:ss.s> dec=<dd:mm:ss.s> ]` | Move to specified coordinates. Omit coordinates to move to the currently loaded database target. If coordinates are specified, then both are required. |
| `expose` | Triggers exposure and waits for completion |
| `trigger_exposure` | Triggers exposure and returns (no wait) |
| `repeat_exposure` | Repeat the last exposure |
| `stop_exposure` | Stops the exposure delay in progress (not the readout) |
| `abort` | Sends abort signal to all processes |
| `sequence_start` | Starts the sequencer list processing |
| `calib_set cal=<name>` | Configures the calibrator for the specified target where `<name>` is one of: `THAR`, `FEAR`, `THAR_UG`, `FEAR_UG`, `CONTR`, `CONTB`, `DOME`, `DOME_UG`, `DOMEHE`, `DOMEHE_UG`, `BIAS`, `DARK` |
| `camera_init [ chan=<n>,... ]` | Initialize connection to science cameras. Default is all cameras. Optionally specify one or more channels with comma-separated list, where `<n>` is one of: `U`, `G`, `R`, `I`. Examples: `camera_init chan=I`, `camera_init chan=U,G` |
| `camera_shutdown` | Shut down camera system |
| `camera_set exptime=<t> binspec=<n> binspat=<n>` | Configure science camera settings |
| `slit_set width=<n> offset=<n>` | Set slit width and offset. Offset is optional |
| `focus_set <chan>=<value>` | `<chan>` is one of `U`, `G`, `R`, `I` |
| `flexure_set` | |
| `acam_init` | |
| `acam_shutdown` | |
| `acam_set acquire=<option>` | Options include: `start`, `<ra,dec,angle>` |
