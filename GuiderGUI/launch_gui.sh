#!/bin/bash

# Start one of the ds9-based GUIs
# Usage: ./launch_gui.sh <camera>   where <camera> is guider or slicev

#region_cmd="image; compass 100 100 50 # compass=fk5 {N} {E} 0 0 color=yellow width=2 edit=0 move=0 rotate=0 delete=0"  #font=\"helvetica 12 bold roman\"

camera=$1   # guider or slicev

if [ -z "$camera" ]; then
  echo "ERROR: camera not specified (use 'guider' or 'slicev')"
  exit 1
fi

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
CONFIG_FILE="$SCRIPT_DIR/gui.config"
if [ ! -f "$CONFIG_FILE" ]; then
  echo "ERROR: gui.config not found at $CONFIG_FILE"
  exit 1
fi
source "$CONFIG_FILE" "$camera"

# Clear any cached state so a fresh GUI launch redraws everything
region_cache="/tmp/ngps_${camera}_regions.key"
rm -f "$region_cache"

# Prefer the DS9 app binary (Homebrew wrapper drops args on macOS)
if [ -x "/Applications/SAOImageDS9.app/Contents/MacOS/ds9" ]; then
  DS9_BIN="/Applications/SAOImageDS9.app/Contents/MacOS/ds9"
else
  DS9_BIN="$(command -v ds9)"
fi

if [ -z "$DS9_BIN" ]; then
  echo "ERROR: ds9 not found in PATH"
  exit 1
fi

DEBUG="${NGPS_GUI_DEBUG:-0}"
if [ "$DEBUG" = "1" ]; then
  echo "DEBUG: camera=$camera"
  echo "DEBUG: SCRIPT_DIR=$SCRIPT_DIR"
  echo "DEBUG: CONFIG_FILE=$CONFIG_FILE"
  echo "DEBUG: id=$id"
  echo "DEBUG: startfile=$startfile"
  echo "DEBUG: anstemplate=$anstemplate"
  echo "DEBUG: ansfile=$ansfile"
  echo "DEBUG: fitstype=$fitstype"
  echo "DEBUG: DS9_BIN=$DS9_BIN"
  echo "DEBUG: PATH=$PATH"
  echo "DEBUG: xpaaccess=$(command -v xpaaccess || echo missing)"
  echo "DEBUG: xpaget=$(command -v xpaget || echo missing)"
  echo "DEBUG: xpaset=$(command -v xpaset || echo missing)"
  echo "DEBUG: xpaaccess -V $id:"
  xpaaccess -V "$id" 2>&1 || true
fi

# "$id" is set in $SCRIPT_DIR/gui.config $camera to GUIDER or SLICEVIEW
#
process_running=`ps aux | grep ds9 | grep " $id "`  # should be empty string if no matches

# If the ds9 window exists, show ds9 message
if xpaget $id xpa connect >/dev/null 2>&1; then

  echo "ALREADY RUNNING"
  xpaset -p $id analysis message {$id is already running}  # This pop-up should activate the existing ds9 window

# If the process is running (string is not empty), show linux message
elif [ -n "$process_running" ]; then

  echo "PROCESS RUNNING"
  if command -v notify-send >/dev/null 2>&1; then
    notify-send --urgency=normal -t 3000 "$id" "GUI (ds9) is launching.  Please wait longer..."
  fi

# Launch it
else

  echo LAUNCHING...
  if command -v notify-send >/dev/null 2>&1; then
    notify-send --urgency=normal -t 3000 "$id" "GUI (ds9) is launching.  Please wait..."
  fi

  LOGFILE="/tmp/ngps_${id}.ds9.log"
  "$DS9_BIN" -png "$startfile" -title "$id" -xpa "$id" \
    -zoom to fit -cmap $cmap -scale linear -scale mode zscale -scale datasec yes \
    -preserve pan yes -preserve regions yes \
    -view filename no -view object no -view colorbar no -view frame no -view physical no \
    -prefs theme awbreezedark -geometry $geometry  \
    -analysis load $anstemplate -analysis task startsync >"$LOGFILE" 2>&1 &
  if [ "$DEBUG" = "1" ]; then
    echo "DEBUG: ds9 log = $LOGFILE"
  fi
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

if command -v wmctrl >/dev/null 2>&1 && [ "$(uname -s)" != "Darwin" ]; then
  while ! wmctrl -l | grep -q "$window_title"; do
    sleep 0.5
    elapsed=$((elapsed + 1))
    if [ "$elapsed" -ge $((timeout *2)) ]; then
      echo "timeout waiting for $window_title window"
      exit 1
    fi
  done
fi

# verify XPA registration after launch
if [ "$DEBUG" = "1" ]; then
  sleep 1
  echo "DEBUG: xpaaccess -V $id (post-launch):"
  xpaaccess -V "$id" 2>&1 || true
fi

# position the window

if command -v wmctrl >/dev/null 2>&1 && [ "$(uname -s)" != "Darwin" ]; then
  if [ "$camera" = "guider" ]; then
    sleep 0.5
    wmctrl -r "SAOImage GUIDER" -e 0,110,50,625,880
  elif [ "$camera" = "slicev" ]; then
    sleep 0.5
    wmctrl -r "SAOImage SLICEVIEW" -e 0,770,50,625,880
  fi
fi

# launch FIFO script
#
$SCRIPT_DIR/make_"$camera"_fifo.sh &
