#!/bin/bash

# Update the ds9 slice viewer display and menus with given arguments
# slice viewer camera daemon should call this script after changing anything displayed by the GUI
#
# USAGE:  ./push_settings_slicev.sh <update_menu> <exptime> <gain>
# exptime (float) = exposure time (s)
# gain (int) = camera gain
# bin (int) = on-chip binning (px)
# update_menu (bool) = whether to update the GUI settings menu with current values

camera=slicev

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $SCRIPT_DIR/gui.config $camera

# settings from daemon or DS9 menu
update_menu=$1
exptime=${2:-1}  # ${argN:-default}
gain=${3:-1}
bin=${4:-4}
navg=${5:-1}

# Get the jog size parameter which is just 1 number saved in a file
# The jog settings menu writes to this file
JOGSIZE_ARCSEC=`cat $SCRIPT_DIR/jog.ini`

# create new analysis file from template and new parameters
sed -e "s/DEFexptime/$exptime/" \
    -e "s/DEFgain/$gain/" \
    -e "s/DEFbin/$bin/" \
    -e "s/DEFnavg/$navg/" \
    -e "s/DEFjog/$JOGSIZE_ARCSEC/" \
    $anstemplate > $ansfile

# load analysis file
if ${update_menu,,}; then  # bool must be lowercase
    xpaset -p $id analysis clear load $ansfile
fi
