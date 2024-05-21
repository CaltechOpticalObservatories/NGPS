#!/bin/bash

# Start ds9 GUI and restart it if it ever dies (not if it quits normally)
# Usage: ./keepalive_gui.sh <camera>   where <camera> is guider or slicev

#region_cmd="image; compass 100 100 50 # compass=fk5 {N} {E} 0 0 color=yellow width=2 edit=0 move=0 rotate=0 delete=0"  #font=\"helvetica 12 bold roman\"

camera=$1   # guider or slicev

echo ""
echo "Starting the NGPS slice viewer GUI.  This may take up to a minute..."
echo ""

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $SCRIPT_DIR/gui.config $camera

until ds9 -png $startfile -title $id \
  -zoom to fit -cmap $cmap -scale linear -scale mode zscale \
  -preserve pan yes -preserve regions yes \
  -view filename no -view object no -view colorbar no \
  -prefs theme awbreezedark -geometry $geometry  \
  -analysis load $anstemplate -analysis task startsync; do
#  -region command "$region_cmd" \

    msg="$id GUI quit with exit code $?.  Please wait for respawn (may take a minute)."
    echo "$msg" >&2
    zenity --error --text="$msg" --title="DON'T PANIC" --width=250 2>/dev/null
    sleep 1

done
