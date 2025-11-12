#!/bin/bash

# Start one of the ds9-based GUIs
# Usage: ./launch_gui.sh <camera>   where <camera> is guider or slicev

#region_cmd="image; compass 100 100 50 # compass=fk5 {N} {E} 0 0 color=yellow width=2 edit=0 move=0 rotate=0 delete=0"  #font=\"helvetica 12 bold roman\"

camera=$1   # guider or slicev

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $SCRIPT_DIR/gui.config $camera

# "$id" is set in $SCRIPT_DIR/gui.config $camera to GUIDER or SLICEVIEW
#
process_running=`ps aux | grep ds9 | grep " $id "`  # should be empty string if no matches

# If the ds9 window exists, show ds9 message
if xpaget $id xpa connect; then

  echo "ALREADY RUNNING"
  xpaset -p $id analysis message {$id is already running}  # This pop-up should activate the existing ds9 window

# If the process is running (string is not empty), show linux message
elif [ -n "$process_running" ]; then

  echo "PROCESS RUNNING"
  notify-send --urgency=normal -t 3000 "$id" "GUI (ds9) is launching.  Please wait longer..."

# Launch it
else

  echo LAUNCHING...
  notify-send --urgency=normal -t 3000 "$id" "GUI (ds9) is launching.  Please wait..."


  ds9 -png $startfile -title $id \
    -zoom to fit -cmap $cmap -scale linear -scale mode zscale -scale datasec yes \
    -preserve pan yes -preserve regions yes \
    -view filename no -view object no -view colorbar no -view frame no -view physical no \
    -prefs theme awbreezedark -geometry $geometry  \
    -analysis load $anstemplate -analysis task startsync &
  #  -region command "$region_cmd" \

fi

# wait for window to appear then move to preferred position

window_title=""
if [ "$camera" = "guider" ]; then
  window_title="SAOImage GUIDER"
elif [ "$camera" = "slicev" ]; then
  window_title="SAOImage SLICEVIEW"
fi

# wait up to 15 sec

timeout=15
elapsed=0

while !  wmctrl -l | grep -q "$window_title"; do
 sleep 0.5
 elapsed=$((elapsed + 1))
 if [ "$elapsed" -ge $((timeout *2)) ]; then
   echo "timeout waiting for $window_title window"
   exit 1
 fi
done

# position the window

if [ "$camera" = "guider" ]; then
  sleep 0.5
  wmctrl -r "SAOImage GUIDER" -e 0,110,50,625,880
elif [ "$camera" = "slicev" ]; then
  sleep 0.5
  wmctrl -r "SAOImage SLICEVIEW" -e 0,770,50,625,880
fi

# launch FIFO script
#
$SCRIPT_DIR/make_"$camera"_fifo.sh &
