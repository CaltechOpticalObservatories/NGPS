#!/bin/bash

# Save FITS file and push to another DS9 window

camera=$1  # guider or slicev

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $SCRIPT_DIR/gui.config $camera

newID=${id}_grabs
fname=${grabdir}/${id}_`date +"%Y%m%d-%H%M%S"`.fits
DEFzoom=.5

xpaset -p $id save mosaicimage $fname  # save the FITS file

xpaset -p $newID frame new && startup=1  # startup=1 if there's an error

if [ -z "$startup" ]; then  # if startup is 0 or null, start new ds9 with the FITS file

	nohup ds9 -mosaicimage wcs $fname -title $newID -mode none -tile yes \
	  -zoom $DEFzoom -scale linear -scale mode zscale \
	  -view object no -view colorbar no \
	   >&- 2>&- &  # this stuff "closes stdout" to make ds9 parent DS9 process stop waiting for child DS9 to finish
	   				# https://stackoverflow.com/questions/20338162/how-can-i-launch-a-new-process-that-is-not-a-child-of-the-original-process

	echo "Launching a new DS9 window for grabbed frames..."

else # load file into the new frame
	#xpaset -p $newID fits $fname
	cat $fname | xpaset $newID mosaicimage wcs
fi
