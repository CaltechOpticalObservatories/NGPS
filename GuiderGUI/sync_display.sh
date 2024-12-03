#!/bin/bash

# Get acam settings and update the ds9 guider display and menus

camera=$1

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $SCRIPT_DIR/gui.config $camera

frames=`xpaget $id frame all`  # list of frame numbers

# Delete all frames except #1
for f in $frames; do
    if [ "$f" != "1"  ]; then
        xpaset -p $id frame delete $f
    fi
done

# Load image into frame #1 if it is empty
xpaset -p $id frame 1
hasfits=`xpaget $id frame has fits`
if [ "$hasfits" == "no" ]; then
    cat $startfile | xpaset $id fits;
fi

xpaset -p $id zoom to fit;
xpaset -p $id mode crosshair;

if [[ "$camera" == "slicev" ]]; then
    xpaset -p $id lock scalelimits yes  # equalize sides of slicer cams; not necessary for ACAM
fi

# request acamd to print parameters to stdout
#params=`acam guideset | awk '{$NF="";sub(/[ \t]+$/,"")}1'` # awk removes final status word ("ERROR"/"DONE")

#$SCRIPT_DIR/sync_daemon.sh true $params # 1st arg is true to update menu

$synccommand
