#!/bin/bash

# Defines the ds9 task for the "offset guider" button

camera=guider

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $SCRIPT_DIR/gui.config $camera

crosscenter=`xpaget $id crosshair wcs`  # returns X Y (px) ;  use wcs to get sky coords

# For slice viewer, need to compute dX relative to slit center (off the edge of image)

crossRA_deg=$(echo $crosscenter | cut -f1 -d ' ')
crossDEC_deg=$(echo $crosscenter | cut -f2 -d ' ')

echo $crossRA_deg $crossDEC_deg
acam putonslit $crossRA_deg $crossDEC_deg

