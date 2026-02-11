#!/bin/bash

# Get acam settings and update the ds9 guider display and menus

camera=$1

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $SCRIPT_DIR/gui.config $camera

region_cache="/tmp/ngps_${camera}_regions.key"
rm -f "$region_cache"

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

# Reset DS9 zoom and cursor mode
xpaset -p $id zoom to fit;
xpaset -p $id mode crosshair;
xpaset -p $id cmap $cmap;
xpaset -p $id scale datasec yes


# if [[ "$camera" == "slicev" ]]; then
#     xpaset -p $id lock scalelimits yes  # equalize sides of slicer cams; not necessary for ACAM
# fi

# ACAM or SCAM demon will use push_settings*.sh
$synccommand
