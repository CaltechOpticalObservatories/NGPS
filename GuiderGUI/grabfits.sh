#!/bin/bash

# Save FITS file and push to another DS9 window
# Use --no-display to skip pushing to DS9

camera=$1  # guider or slicev
saveopt=$2 # push display or save only

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $SCRIPT_DIR/gui.config $camera

newID=${id}_grabs
fname=${grabdir}/${id}_`date +"%Y%m%d-%H%M%S"`.fits
DEFzoom=.5

# Prefer the DS9 app binary on macOS
if [ -x "/Applications/SAOImageDS9.app/Contents/MacOS/ds9" ]; then
  DS9_BIN="/Applications/SAOImageDS9.app/Contents/MacOS/ds9"
else
  DS9_BIN="$(command -v ds9)"
fi
if [ -z "$DS9_BIN" ]; then
  echo "ERROR: ds9 not found in PATH" >&2
  exit 1
fi

# Ensure grab directory exists
mkdir -p "$grabdir"

# xpa save commands are different for ACAM and SCAMs
if [[ "$camera" == "guider" ]]; then
fitstype="fits"
savetype=""
else
fitstype="mosaicimage wcs"
savetype="mosaicimage"
fi

# Save the FITS file
if [ -n "$savetype" ]; then
  xpaset -p $id save $savetype $fname
else
  xpaset -p $id save $fname
fi
echo $fname

if [ ! -s "$fname" ]; then
  echo "ERROR: grab fits failed (no output at $fname)" >&2
  xpaset -p $id analysis message "ERROR: grab fits failed (no output)" 2>/dev/null
  exit 1
fi

# Quit after saving if --no-display
if [[ "$saveopt" == "--no-display" ]]; then
  exit 0
fi


process_running=`ps aux | grep ds9 | grep " $newID "`  # should be empty string if no matches

# If the ds9 window exists, replace frames and load the new FITS file into it
if xpaget $newID xpa connect >/dev/null 2>&1; then

	# Replace any existing frames to avoid leaving an empty frame behind
	xpaset -p $newID frame delete all
	xpaset -p $newID file "$fname"

# If the process is running (string is not empty), show linux message
elif [ -n "$process_running" ]; then

	# Pop-up on linux OS
	zenity --notification --text "$newID GUI (ds9) is launching.  Please wait longer..."

# Otherwise, start new ds9 with the saved FITS file
else
#datasec hides the overlapping parts of SCAMs	
	LOGFILE="/tmp/ngps_${newID}.ds9.log"
	nohup "$DS9_BIN" -$fitstype "$fname" -title "$newID" -mode none -tile yes \
	  -zoom $DEFzoom -scale linear -scale mode zscale -scale datasec yes \
	  -view object no -view colorbar no \
	   >"$LOGFILE" 2>&1 &  # log for debugging and avoid parent wait
	   				# https://stackoverflow.com/questions/20338162/how-can-i-launch-a-new-process-that-is-not-a-child-of-the-original-process

	if [ "${NGPS_GUI_DEBUG:-0}" = "1" ]; then
	  echo "DEBUG: ds9 log = $LOGFILE"
	fi

	if command -v zenity >/dev/null 2>&1; then
	  zenity --notification \
         --title "$camera frame grab" \
         --text "Launching a new DS9 window for grabbed frames..."
	fi

	# No XPA load here; file is already displayed on launch

fi
