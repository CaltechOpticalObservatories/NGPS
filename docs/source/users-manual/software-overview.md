# Instrument Control Software

The instrument control software (ICS) consists of several independent daemons, one for each logical subsystem. An added daemon process orchestrates (or “sequences”) the operations needed to be performed by each subsystem to carry out an observation.

The user interacts with the ICS using three GUIs: a “main” GUI, an Acquisition & Guide (ACAM) GUI, and Slice Camera GUI. A command line interface is also available.

All daemon processes are started automatically when the computer boots.

A set of VNC servers are also started automatically, and the three GUIs are started automatically in the proper VNC session.

## Sequencer

The ICS is built around an Observation Sequencer, a state machine that orchestrates the sequence of commands to each subsystem necessary to carry out an observation. While the sequencer is the interface to the instrument – it is what drives every operation – the user is generally unaware of it. All commands pass through the sequencer; no direct communication with individual subsystem daemons is available during normal observing. The sequencer knows which operations are allowed in any given instrument state, which may be performed in parallel, and which must be run serially. Both the GUI and CLI communicate exclusively with the sequencer.

The observer prepares a target list file containing coordinates, exposure parameters, and instrument settings for each target. This target list is imported into a relational database via the GUI. The sequencer then reads one target at a time from the database, configures each subsystem (slit, calibrator, pointing, guiding, cameras, etc.) according to the database record, carries out the exposure, and advances to the next target. By default the entire list is processed automatically. An optional Single mode allows disabling this behavior and stops after each exposure, requiring the observer to initiate the next observation manually.

Because the sequencer reads only one target at a time, the observer is free to edit, reorder, or insert new targets – including Targets of Opportunity – into the pending portion of the list at any time, even while an observation is in progress. A Target of Opportunity is implemented simply by flagging the desired target as next in the database.

## Subsystems

The ICS consists of several independent daemon processes, each responsible for a specific subsystem. All daemons start automatically when the instrument computer boots. The observer does not interact with individual daemons directly; all commands are routed through the sequencer. The subsystem daemons are as follows:

### sequencerd

The observation sequencer. Orchestrates all subsystem operations required to carry out each observation, as described in the Sequencer section.

### camerad

Controls the four ARC (Leach) detector controllers, one per spectrographic channel (U,G,R,I). A single daemon communicates with all four controllers and supports commands to all controllers simultaneously or to individual controllers.

### slitd

Controls the slit slicer. Both width and offset are adjustable.

### focusd

Controls the focus mechanisms for science detectors. Nominal focus defaults are configurable parameters. Open loop focus corrections are applied using a temperature lookup table. [NOT YET IMPLEMENTED]

### flexured

Controls the flexure compensation system, which uses piezoelectric tip-tilt-piston actuators on each arm to align the spectra on the detectors. Open loop corrections are applied automatically, between observations. [NOT YET IMPLEMENTED]

### calibd

Controls the calibration system, including illumination sources, illumination source modulators, a dust cover, and a door to close off input from the calibration sources. Calibration observations (flats, arcs, darks) are controlled by the sequencer using special CAL_xxxx target names which define the settings needed to drive the calibration system.

### acamd

Controls the Acquisition & Guide Camera (ACAM): an : Andor iXon Ultra 888 camera, filter wheel, and dust cover. Provides target acquisition by solving the astrometry of guide camera images, computing pointing offsets, and sending corrections to the TCS. Also computes real-time seeing, extinction, and sky background measurements. After acquisition, it can transition to a guiding mode to maintain pointing.

### slicecamd

Controls the two EMCCD cameras that image the slicers. The two camera images are merged into a single display with a calculated gap representing the slit width and offset, allowing for precise placement of a target on the slit.

### tcsd

Interfaces with the P200 Telescope Control System and broadcasts TCS info to other daemons that need it for information.

### powerd

Controls four network power switches that provide remote AC and DC power control for instrument components. Any connected component can be powered on or off by command, as controlled by the sequencer.

### messaged

This is a broker for handling messages between daemons. All daemons publish and subscribe to this broker which ensures the messages are distributed to each subscriber.

## Configuration

All software modules have a configuration file that can be edited, which can change the behavior of the instrument. Many configuration values will likely not need to be modified, but some have potential use. A configuration tool allows the user to edit the configuration keys that are allowed to be modified. For example, this allows changing default calibration lamp modulation levels, nominal focus positions, or even disable certain parts of the system. A complete list of user-editable configuration values is given in the configuration reference.

To run the configuration tool type:

TBD

User configuration changes are per-night-only and will be reset to system defaults on the next UTC date, in order to ensure the system is in a known, stable state for each observer.
