#!/bin/bash

# Update the ds9 slice viewer display and menus with given arguments
# slice viewer camera daemon should call this script after changing anything displayed by the GUI
#
# USAGE:  ./push_settings_slicev.sh <update_menu> <exptime> <gain>
# exptime (float) = ACAM exposure time (s)
# gain (int) = ACAM gain
# update_menu (bool) = whether to update the GUI settings menu with current values

camera=slicev

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $SCRIPT_DIR/gui.config $camera

# settings from daemon or DS9 menu
update_menu=$1
exptime=$2
gain=$3

# create new analysis file from template and new parameters
sed -e "s/DEFexptime/$exptime/" \
    -e "s/DEFgain/$gain/" \
    $anstemplate > $ansfile

# load analysis file
if ${update_menu,,}; then  # bool must be lowercase
    xpaset -p $id analysis clear load $ansfile
fi

# update headsup display
width=`xpaget $id fits width`
XCENTER=$(($width/2)) # don't divide by 2 since there are 2 frames $(($width/2))
YCENTER=30

xpaset -p $id region delete all
echo "image; text $XCENTER $YCENTER # text={EXPTIME=${exptime}   GAIN=${gain}} \
  color=${headsup_fontcolor} width=2 edit=0 move=0 rotate=0 delete=1 font={helvetica ${headsup_fontsize} bold}" \
  | xpaset $id region 2>&1

