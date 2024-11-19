#!/bin/bash

# Get the celestial coordinates of the slit center and crosshairs as displayed on the sliceviewer GUI.
# Send to daemon in units of deg
# Daemon computes offsets (account for spherical geometry) in arcsec

camera=slicev

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $SCRIPT_DIR/gui.config $camera

# Get the slit region and crosshair coordinates from ds9
crosscenter=`xpaget $id crosshair wcs degrees`
slitcenter=`xpaget $id region -format xy -system wcs -group slitcenter -skyformat degrees`

# Define the slit region coordinates
hbin=`xpaget SLICEVIEW fits header keyword HBIN`
vbin=`xpaget SLICEVIEW fits header keyword VBIN`
xslit=$(( 693 / $hbin ))
yslit=$(( 512 / $vbin ))
xwidth=$(( 24 / $hbin ))
ywidth=$(( 200 / $vbin ))
slitreg="image; box($xslit,$yslit,$xwidth,$ywidth,0) # color=cyan edit=0 move=0 rotate=0 tag={slitcenter}"

# If the slit result is empty string, try loading the slit region file
if [ -z "$slitcenter" ]; then
  echo No slit! ;
  echo "$slitreg" | xpaset $id region
  slitcenter=`xpaget $id region -format xy -system wcs -group slitcenter -skyformat degrees`
fi

# If the slit region is still missing, give up
if [ -z "$slitcenter" ]; then
  echo No slit again! ;
  exit
fi

# Parse the returned (RA, DEC), formatted as "123.45 678.90" in decimal degrees
slitRA_deg=$(echo $slitcenter | cut -f1 -d ' ')
slitDEC_deg=$(echo $slitcenter | cut -f2 -d ' ')

crossRA_deg=$(echo $crosscenter | cut -f1 -d ' ')
crossDEC_deg=$(echo $crosscenter | cut -f2 -d ' ')

echo "(slitRA, slitDEC, crossRA, crossDEC)"
echo $slitRA_deg $slitDEC_deg $crossRA_deg $crossDEC_deg

# slice camera command
scam putonslit $slitRA_deg $slitDEC_deg $crossRA_deg $crossDEC_deg

# Use the result:
# import from FPoffsets:  solve_offset_deg(slitRA, slitDEC, crossRA, crossDEC)
# Returns (offsetRA, offsetDEC) in degrees
# You can convert to arcsec and use in PT i.e. PT offsetRA*3600 offsetDEC*3600
