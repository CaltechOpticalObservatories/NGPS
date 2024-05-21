#!/bin/bash

# Update the ds9 slice viewer gui display with a 2-extension FITS file

camera=$1
fname=$2

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $SCRIPT_DIR/gui.config $camera

xpaset -p $id frame 1
cat $fname | xpaset $id $fitstype

xpaset -p $id mode crosshair;

