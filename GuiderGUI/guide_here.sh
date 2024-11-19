#!/bin/bash

acquire_timeout=30  # timeout in seconds when waiting for acquisition

# Poll TCS for Cass angle, convert to slit angle
casangle=`tcs getcass | cut -f1 -d ' '`
slitcoords=`FPoffsets SCOPE SLIT 0 0 $casangle`
slitangle=`echo $slitcoords | cut -f4 -d '='`

# Poll TCS for RA, DEC of boresight, assume equal to RA, DEC of slit for now
radec=`tcs getcoords`
ra=`echo $radec | cut -f1 -d ' '`
dec=`echo $radec | cut -f2 -d ' '`

#echo DISABLED!
#exit 0


# Acquire the target, acamd saves coordinates
dummy=`acam acquire $ra $dec $slitangle`

# Wait for acquisition success
timeout $acquire_timeout listen -w "target acquired"

# Keep guiding on acquired target
acam acquire guide
