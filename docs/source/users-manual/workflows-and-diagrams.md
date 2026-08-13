# Workflows and Diagrams


## Observing Workflow

```{mermaid}
flowchart TD
    A[Prepare target list CSV] --> B[Connect to Palomar observing systems]
    B --> C[Load target list into NGPS GUI]
    C --> D[Choose slit width and binning]
    D --> E[Generate and run calibration target lists]
    E --> F[Focus telescope with ACAM]
    F --> G{Sequencer mode}
    G -->|Single| H[Select target and click Go]
    G -->|All| I[Process target list automatically]
    H --> J[TCS operator slews and clicks On Target]
    I --> J
    J --> K[Acquire target with ACAM/SCAM]
    K --> L[Guide if conditions allow]
    L --> M[Expose]
    M --> N[Inspect data with Quicklook DRP]
    N --> O[Repeat or continue to next target]
```

## Instrument Control Software Overview

```{mermaid}
flowchart LR
    Observer[Observer] --> GUI[Main GUI]
    Observer --> ACAMGUI[ACAM / Slice Viewer GUIs]
    Observer --> CLI[seq CLI]
    GUI --> Sequencer[sequencerd]
    CLI --> Sequencer
    ACAMGUI --> Sequencer

    Sequencer --> Camera[camerad]
    Sequencer --> Slit[slitd]
    Sequencer --> Calib[calibd]
    Sequencer --> ACAM[acamd]
    Sequencer --> Slice[slicecamd]
    Sequencer --> TCS[tcsd]
    Sequencer --> Power[powerd]
    Sequencer --> Focus[focusd]
    Sequencer --> Flexure[flexured]

    Message[messaged broker] <--> Sequencer
    Message <--> Camera
    Message <--> Slit
    Message <--> Calib
    Message <--> ACAM
    Message <--> Slice
    Message <--> TCS
    Message <--> Power
```

## Calibration Workflow

```{mermaid}
flowchart TD
    A[Decide science slit widths and binning modes] --> B[Internal calibrations by binning]
    A --> C[Dome flats by slit width plus binning]
    B --> D[Generate calibration target list in GUI]
    C --> D
    D --> E[Run through sequencer with Go]
    E --> F[Inspect raw frames]
    F --> G{Complete and valid?}
    G -->|Yes| H[Continue observing]
    G -->|No| I[Modify or regenerate target list]
    I --> E
    H --> J[Check status with ngps_cals]
```
