#!/bin/bash

# Start one of the ds9-based GUIs
# Usage: ./launch_gui.sh <camera>   where <camera> is guider or slicev

#region_cmd="image; compass 100 100 50 # compass=fk5 {N} {E} 0 0 color=yellow width=2 edit=0 move=0 rotate=0 delete=0"  #font=\"helvetica 12 bold roman\"

camera=$1   # guider or slicev

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $SCRIPT_DIR/gui.config $camera

process_running=`ps aux | grep ds9 | grep " $id "`  # should be empty string if no matches

# If the ds9 window exists, show ds9 message
if xpaget $id xpa connect; then

  echo "ALREADY RUNNING"
  xpaset -p $id analysis message {$id is already running}  # This pop-up should activate the existing ds9 window

# If the process is running (string is not empty), show linux message
elif [ -n "$process_running" ]; then

  echo "PROCESS RUNNING"
  zenity --notification --text "$id GUI (ds9) is launching.  Please wait longer..."  # Pop-up on linux OS

# Launch it
else

  echo LAUNCHING...
  zenity --notification --text "$id GUI (ds9) is launching.  Please wait..."  # Pop-up on linux OS

  ds9 -png $startfile -title $id \
    -zoom to fit -cmap $cmap -scale linear -scale mode zscale \
    -preserve pan yes -preserve regions yes \
    -view filename no -view object no -view colorbar no \
    -prefs theme awbreezedark -geometry $geometry  \
    -analysis load $anstemplate -analysis task startsync
  #  -region command "$region_cmd" \

fi
