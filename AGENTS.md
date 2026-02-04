# AGENTS.md

This file provides guidance to WARP (warp.dev) when working with code in this repository.

## Project Overview

NGPS (Next Generation Palomar Spectrograph) is a telescope/observatory instrument control system. It uses a distributed daemon architecture where independent C++ daemons control different hardware subsystems and communicate via TCP sockets and ZeroMQ messaging.

## Build Commands

```bash
# Build all components (from repository root)
cd build && cmake .. && make

# Build with verbose output
make VERBOSE=1

# The build system auto-generates .cfg files from .cfg.in templates using port definitions in Config/portdefs
```

## Running the Software

```bash
# Manage all daemons
ngps start                    # Start all daemons
ngps stop                     # Stop all daemons
ngps status                   # Show running daemons
ngps list                     # List available modules

# Manage individual daemons
ngps start <daemon>           # e.g., ngps start sequencerd
ngps stop <daemon>

# Communicate with running daemons via CLI
acam <command>                # ACAM camera daemon
camera <command>              # Science camera daemon
seq <command>                 # Observation sequencer
tcs <command>                 # Telescope control
calib <command>               # Calibration unit
slit <command>                # Slit control
power <command>               # Power control
focus <command>               # Focus control
thermal <command>             # Thermal monitoring
flexure <command>             # Flexure compensation

# Get help for a daemon
seq help                      # or: seq ?

# Monitor logs and async messages
colorlog                      # Monitor today's log files
listen                        # Listen to async messages
```

## Hardware Emulators

Emulators allow testing without physical hardware:

```bash
emulator start all            # Start all emulators
emulator start <name>         # Start specific: acam, calib, flexure, focus, power, slit, tcs
emulator stop all
emulator status
```

## Architecture

### Daemon Structure

Each subsystem follows a consistent pattern:
- `<name>d.cpp` - Main daemon entry point, handles daemonization and socket setup
- `<name>_server.cpp/.h` - Server class handling client connections and command dispatch
- `<name>_interface.cpp/.h` - Interface to actual hardware or lower-level systems
- `<name>.cpp/.h` - Core business logic

**Key daemons:**
- `sequencerd` - Central orchestrator, coordinates observations by commanding other daemons
- `camerad` - Science camera control (ARC controllers)
- `acamd` - Acquisition camera (Andor)
- `slicecamd` - Slicer cameras (Andor)
- `tcsd` - Telescope Control System interface
- `messaged` - ZeroMQ message broker for pub/sub telemetry

### Communication

- **TCP Sockets**: Each daemon exposes blocking (`*_BLK_PORT`) and non-blocking (`*_NB_PORT`) ports
- **ZeroMQ Pub/Sub**: Telemetry published through message broker (messaged) on ports 5555/5556
- **UDP Multicast**: Async notifications on port 1300, group 239.1.1.234
- **Commands**: Text-based protocol, daemons return status codes (NO_ERROR=0, ERROR=1, BUSY=2, TIMEOUT=3)

### Shared Libraries

- `common/` - Shared classes: PubSub, Header (FITS), Queue, state management
- `utils/` - Networking (TcpSocket, UdpSocket), logging, config parsing, database access, utilities
- `common/*_commands.h` - Command string constants for each daemon

### Configuration

- `Config/*.cfg.in` - Configuration templates (CMake processes these into .cfg files)
- `Config/portdefs` - Master port number definitions (CMake variables)
- Ports are set via CMake to avoid conflicts across daemons

### Hardware Abstraction

- `Andor/` - Andor camera SDK wrapper
- `ARC/` - ARC CCD controller interface
- `PI/` - PI motion controller interface
- `LKS/` - Lakeshore temperature controller
- `WTI/` - WTI network power switch

### GUIs

- `java/ngps/ngps/` - Main Java observing GUI (built with NetBeans/Ant)
- `pygui/` - Python Qt-based GUI

## Code Conventions

- C++17 standard, compiled with g++
- Error codes: `NO_ERROR`, `ERROR`, `BUSY`, `TIMEOUT` defined in `common/common.h`
- Logging via `logwrite(function_name, message)` - logs go to `/data/logs/`
- JSON handling uses nlohmann/json library
- FITS files use CCFits/CFITSIO

## Database

MySQL database `ngps` accessed via MySQL Connector/C++ X DevAPI:
- `targets` - Active observation targets
- `completed_obs` - Completed observations
- `target_sets` - Target groupings

Connection config in sequencerd.cfg: host=localhost, port=33060, user=obseq

## Deployment

See `HOW_TO_DEPLOY.TXT` for deployment procedures. Key points:
- Push code from ngps-devel, pull to ngps deployment machine
- Java GUI: build in NetBeans, run `java/ngps/ngps/deploy` script
- Database schema changes: use skeema tool in `mysql/` directory
