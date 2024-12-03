#!/bin/bash

# mini script to write 1 number to the jog settings file, because I can't get this to work natively in the DS9 analysis menu

camera=slicev

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $SCRIPT_DIR/gui.config $camera

jog_arcsec=$1
echo $jog_arcsec > $jogfile  ### If we can get just this line in the DS9 menu it would simplify things
