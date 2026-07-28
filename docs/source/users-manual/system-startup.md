# System Startup and VNC Sessions

## Instrument Setup

### VNC Sessions

Four VNC sessions are automatically started on boot for the following purposes. Do not run data analysis or web browsers on the instrument control computer. While the Instrument Control Software needs very little in terms of resources, other processes may cause sudden spikes in CPU usage, possibly starving the instrument software of resources and causing unexplained behaviors.

#### Screen :1

The TCS Operator should connect to this session. It is meant to display the “On-Target” GUI, which starts automatically with the VNC session.

#### Screen :2

The ACAM and Slicecam GUIs are started automatically in this session.

#### Screen :3

The main GUI is started automatically in this session.

#### Screen :4

This session is left open for running “ds9last” or other quick display tools.

### Instrument Control Software

Instrument control software starts automatically at boot but can be started manually if needed. The main, ACAM and Slicecam GUIs each have desktop icons. The icons appear on all desktops and there is no way to prevent them from running on any desktop, but for consistency it is advised to follow the default VNC session plan as described in the previous section.

Only one set of GUIs can be run at any given time.

The instrument control software daemons can be started by typing:

`ngps start`

in any terminal. Type ``ngps help` for other available options which include checking the status of the software.
