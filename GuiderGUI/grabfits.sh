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

# xpa save and push commands are different for ACAM and SCAMs
if [[ "$camera" == "guider" ]]; then
fitstype="fits"
savetype=""
else
fitstype="mosaicimage wcs"
savetype="mosaicimage"
fi

# Save the FITS file
xpaset -p $id save $savetype $fname
echo $fname

# Quit after saving if --no-display
if [[ "$saveopt" == "--no-display" ]]; then
  exit 0
fi


process_running=`ps aux | grep ds9 | grep " $newID "`  # should be empty string if no matches

# If the ds9 window exists, make a new frame and load the new FITS file into it
if xpaget $newID xpa connect; then

	xpaset -p $newID frame new && error=1  # error=1 if there's an error
	cat $fname | xpaset $newID $fitstype #mosaicimage wcs

# If the process is running (string is not empty), show linux message
elif [ -n "$process_running" ]; then

	# Pop-up on linux OS
	zenity --notification --text "$newID GUI (ds9) is launching.  Please wait longer..."

# Otherwise, start new ds9 with the saved FITS file
else
#datasec hides the overlapping parts of SCAMs	
	nohup ds9 -$fitstype $fname -title $newID -mode none -tile yes \
	  -zoom $DEFzoom -scale linear -scale mode zscale -scale datasec yes \
	  -view object no -view colorbar no \
	   >&- 2>&- &  # this stuff "closes stdout" to make ds9 parent DS9 process stop waiting for child DS9 to finish
	   				# https://stackoverflow.com/questions/20338162/how-can-i-launch-a-new-process-that-is-not-a-child-of-the-original-process

	zenity --notification \
       --title "$camera frame grab" \
       --text "Launching a new DS9 window for grabbed frames..." \

fi
