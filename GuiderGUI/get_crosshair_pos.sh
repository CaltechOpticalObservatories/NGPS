#!/bin/bash

# Defines the ds9 task for the "offset guider" button

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $SCRIPT_DIR/guider.config

xpaget $id crosshair $1  # returns X Y (px) ;  use $1=wcs to get sky coords

# For slice viewer, need to compute dX relative to slit center (off the edge of image)
