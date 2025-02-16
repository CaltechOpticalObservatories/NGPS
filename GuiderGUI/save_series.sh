#!/bin/bash

# GUI interface to "acam saveframes"
# acam saveframes $Nsave $Nskip --> save Nsave frames then skip Nskip frames; repeat forever
# Nsave=0 --> STOP

startstop=$1  # Start or stop saving images
Nperiod=$2    # Save every Nth frame

if [ "$startstop" = "START" ]; then
	Nsave=1
else
	Nsave=0
fi

# Stop if the period doesn't make sense
if [[ $Nperiod -lt 1 ]]; then
	Nsave=0
fi

Nskip=$(($Nperiod - 1))

acam saveframes $Nsave $Nskip 