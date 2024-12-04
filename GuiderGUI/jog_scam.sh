#!/bin/bash

# Hacky use of "scam putonslit" to jog (offset) the telescope in slit coordinate frame
# Uses the DS9 crosshiars to select 2 points forming an offset in the jog direction
# Sends x1, y1, x2, y2 to scam daemon in units of deg
# Daemon computes offsets (account for spherical geometry) in arcsec

camera=slicev

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $SCRIPT_DIR/gui.config $camera


JOG_DIRECTION=$1  # bash argument 1
echo $JOG_DIRECTION

# Get the jog size parameter which is just 1 number saved in a file
# The jog settings menu writes to this file
JOGSIZE_ARCSEC=`cat $jogfile`
echo JOGSIZE_ARCSEC $JOGSIZE_ARCSEC

# Get the slit region and crosshair coordinates from ds9
crosscenter_orig=`xpaget $id crosshair`
echo $crosscenter_orig

pixscale=`xpaget $id fits header keyword PIXSCALE`
JOGSIZE_PX=$(echo "scale=4; $JOGSIZE_ARCSEC / $pixscale" | bc)

# Use cross to get 0 0 coords
# Parse the returned (RA, DEC), formatted as "123.45 678.90" in decimal degrees
xpaset -p $id crosshair 0 0 image
crosscenter=`xpaget $id crosshair wcs degrees`
X0_deg=$(echo $crosscenter | cut -f1 -d ' ')
Y0_deg=$(echo $crosscenter | cut -f2 -d ' ')

case ${JOG_DIRECTION^^} in
  LEFT)
    xpaset -p $id crosshair $JOGSIZE_PX 0 image
    ;;
  RIGHT)
    xpaset -p $id crosshair -$JOGSIZE_PX 0 image
    ;;
  UP)
    xpaset -p $id crosshair 0 -$JOGSIZE_PX image
    ;;
  DOWN)
    xpaset -p $id crosshair 0 $JOGSIZE_PX image
    ;;
  *)
    ;;
esac

crosscenter=`xpaget $id crosshair wcs degrees`
X1_deg=$(echo $crosscenter | cut -f1 -d ' ')
Y1_deg=$(echo $crosscenter | cut -f2 -d ' ')

# reset cross to original position
xpaset -p $id crosshair $crosscenter_orig

echo "(X0, Y0, X1, Y1)"
echo $X0_deg $Y0_deg $X1_deg $Y1_deg

# slice camera command handles the spherical geometry - don't just compute the difference
timeout 5 scam putonslit $X0_deg $Y0_deg $X1_deg $Y1_deg
