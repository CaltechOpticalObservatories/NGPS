#!/bin/bash

# Get the celestial coordinates of the slit center as displayed on the sliceviewer GUI.
# Get the celestial coordinates of crosshairs from either GUI
# Send both to scam daemon in units of deg
# Daemon computes offsets (account for spherical geometry) in arcsec

camera=${1:-slicev}  # slicev or guider --> determines who's crosshairs to use
reverse=${2:-false}  # move target from slit to crosshairs

id_slicev=SLICEVIEW  ### HARDCODE

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $SCRIPT_DIR/gui.config $camera

# If going SLIT to ACAM crosshairs, reset the crosshairs to middle
if $reverse ; then
  width=`xpaget $id fits header keyword NAXIS1`
  height=`xpaget $id fits header keyword NAXIS2`
  xpaset -p $id crosshair $(($width / 2)) $(($height / 2)) image # set crosshair in image coords
fi

# Get the slit region and crosshair coordinates from ds9
crosscenter=`xpaget $id crosshair wcs degrees`

slitcenter=`xpaget $id_slicev region -format xy -system wcs -group slitcenter -skyformat degrees`  ### JUST USE HEADERS

# If the slit region is missing, give up
if [ -z "$slitcenter" ]; then
  echo No slit! ;
  exit 1
fi

# Parse the returned (RA, DEC), formatted as "123.45 678.90" in decimal degrees
slitRA_deg=$(echo $slitcenter | cut -f1 -d ' ')
slitDEC_deg=$(echo $slitcenter | cut -f2 -d ' ')

crossRA_deg=$(echo $crosscenter | cut -f1 -d ' ')
crossDEC_deg=$(echo $crosscenter | cut -f2 -d ' ')

echo "(slitRA, slitDEC, crossRA, crossDEC)"
echo $slitRA_deg $slitDEC_deg $crossRA_deg $crossDEC_deg

# slice camera command
if $reverse ; then
  timeout 5 scam putonslit $crossRA_deg $crossDEC_deg $slitRA_deg $slitDEC_deg 
else
  timeout 5 scam putonslit $slitRA_deg $slitDEC_deg $crossRA_deg $crossDEC_deg
fi

# Use the result:
# import from FPoffsets:  solve_offset_deg(slitRA, slitDEC, crossRA, crossDEC)
# Returns (offsetRA, offsetDEC) in degrees
# You can convert to arcsec and use in PT i.e. PT offsetRA*3600 offsetDEC*3600
