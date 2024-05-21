#!/bin/bash

# Get the celestial coordinates of the slit center and crosshairs as displayed on the sliceviewer GUI.
# Subtract and send offsets to a daemon in units of arcsec

camera=slicev

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $SCRIPT_DIR/gui.config $camera

# Get the slit region and crosshair coordinates from ds9
crosscenter=`xpaget $id crosshair wcs degrees`
slitcenter=`xpaget $id region -format xy -system wcs -group slitcenter -skyformat degrees`

#echo $crosscenter
#echo $slitcenter

# If the slit result is empty string, try loading the slit region file
if [ -z "$slitcenter" ]; then
  echo No slit! ;
  cat $slit_regionfile | xpaset $id region -format ds9 -system physical ;
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

# Arithmetic
# dRA=`echo "($crossRA_deg - $slitRA_deg)*3600."  | bc -l`
# dDEC=`echo "($crossDEC_deg - $slitDEC_deg)*3600."  | bc -l`

# Use the result
# echo $dRA $dDEC arcsec # offsets in arcsec
