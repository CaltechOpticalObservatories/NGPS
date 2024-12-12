#!/bin/bash

# Update the ds9 guider display and menus with given arguments
# acam daemon should call this script after changing anything displayed by the GUI
#
# USAGE:  ./sync_daemon.sh <update_menu> <exptime> <gain> <filter> <focus>
# exptime (float) = ACAM exposure time (s)
# gain (int) = ACAM gain
# filter (str) = ACAM filter wheel position
# focus (float) = Focus position of TCS
# update_menu (bool) = whether to update the GUI settings menu with current values

camera=guider

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $SCRIPT_DIR/gui.config $camera

# settings from daemon or DS9 menu
update_menu=$1
exptime=${2:-1}
gain=${3:-1}
filter=${4:-r}
navg=${5:-1}
# status=$6  # acquiring, guiding etc.  Not available in menu

# Get the focus setting from the P200 TCS
focus=`tcs getfocus | cut -f1 -d ' '` 

# create new analysis file from template and new parameters
sed -e "s/DEFexptime/$exptime/" \
    -e "s/DEFgain/$gain/" \
    -e "s/DEFfilter/$filter/" \
    -e "s/DEFnavg/$navg/" \
    -e "s/DEFfocus/$focus/" \
    $anstemplate > $ansfile

# load analysis file
if ${update_menu,,}; then  # bool must be lowercase
    xpaset -p $id analysis clear load $ansfile
fi
