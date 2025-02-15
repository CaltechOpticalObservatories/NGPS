#!/bin/bash

# Usage: ./guide_start.sh <HERE> <RA> <DEC>

acquire_timeout=30  # timeout in seconds when waiting for acquisition

HEREuser=${1-1}  # default to True
RAuser=$2		# acquire command can handle any units
DECuser=$3

if [ $# -eq 0 ]; then
    echo "No arguments supplied"
    exit 1
fi

# form "acam acquire RA DEC slitangle" OR "acam acquire here"
if [ $HEREuser -eq 1 ]; then
	coords="here"

elif [ $# -lt 3 ]; then 
	# Need the user's RA, DEC but first check they exist	
    echo "Didn't get valid RA and DEC.  Sent: $2  $3"  # print all args
    exit 1

else
	# Poll TCS for Cass angle, convert to slit angle
	casangle=`tcs getcass | cut -f1 -d ' '`
	slitcoords=`FPoffsets SCOPE SLIT 0 0 $casangle`
	slitangle=`echo $slitcoords | cut -f4 -d '='`

	# Use the user's RA, DEC
	coords="$RAuser $DECuser $slitangle"
fi


#echo "acam acquire $coords"
#exit 0

# # Acquire the target, acamd saves coordinates
 dummy=`acam acquire $coords`

# # Wait for acquisition success
 timeout $acquire_timeout listen -w "target acquired"

# # Keep guiding on acquired target
 acam acquire guide
