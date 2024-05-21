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
exptime=$2
gain=$3
filter=$4
focus=$5

# create new analysis file from template and new parameters
sed -e "s/DEFexptime/$exptime/" \
    -e "s/DEFgain/$gain/" \
    -e "s/DEFfilter/$filter/" \
    -e "s/DEFfocus/$focus/" \
    $anstemplate > $ansfile

# load analysis file
if ${update_menu,,}; then  # bool must be lowercase
    xpaset -p $id analysis clear load $ansfile
fi

# update headsup display
width=`xpaget $id fits width`
XCENTER=$(($width/2))
YCENTER=30

xpaset -p $id region delete all
echo "image; text $XCENTER $YCENTER # text={EXPTIME=${exptime}   GAIN=${gain}   FILTER=${filter}   FOCUS=${focus}} \
  color=${headsup_fontcolor} width=2 edit=0 move=0 rotate=0 delete=1 font={helvetica ${headsup_fontsize} normal}" \
  | xpaset $id region 2>&1

