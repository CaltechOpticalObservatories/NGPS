#!/bin/bash

# Save FITS file and use it to calibrate ACAM geometry

camera="guider"  # guider or slicev

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $SCRIPT_DIR/gui.config $camera

tag=CALIBRATE_`date +"%Y%m%d-%H%M%S"`
basename=${grabdir}/${id}_$tag
fname=${basename}.fits
fname2=${basename}_FPfix.fits

# Save the FITS file
xpaset -p $id save $fname
echo $fname

# THE MAIN FUNCTION -- Run astrometry solver with FPoffsets model tweaker
# Iterates the focal plane model until it can predict the ACAM image from TCS coordinates
# Verified by the astrometric solver
WCSfix $fname -FP

# If successful (or maybe don't need to check), load FPOffsets calibration file FROM ACAMD COMMAND
# "acam loadfpcal"

# Display the image with a catalog superimposed
newID=CALIBRATE  #$tag
catalog=gaia
zoom=0.5

# If our DS9 is running, push calibrated image to existing window; otherwise start DS9
if xpaget $newID xpa connect; then
	xpaset -p $newID frame new
	xpaset -p $newID fits $fname2
	xpaset -p $newID catalog $catalog
	sleep 1  # Seems to crash if you don't wait for the catalog to load
	xpaset -p $newID catalog close 
	xpaset -p $newID zoom $zoom
else
	nohup ds9 $fname2 -title $newID -tile yes -tile grid -catalog $catalog -catalog close -zoom $zoom >&- 2>&- & 
	echo "Calibration result will appear in a new DS9..."
	# -zoom $zoom -geometry 1000x500 -view layout basic
	# catalog filter '\$Gmag<21'
fi
						# "">&- 2>&- &"" stuff "closes stdout" to make ds9 parent DS9 process stop waiting for child DS9 to finish
	   				# https://stackoverflow.com/questions/20338162/how-can-i-launch-a-new-process-that-is-not-a-child-of-the-original-process


# Now display the image with headers before calibration; first make sure ds9 is running
# while true; do
# 	sleep 1
# 	if xpaget $newID xpa connect; then

# 		sleep 1
# 		xpaset -p $newID frame new
# 		xpaset -p $newID fits $fname
# 		# xpaset -p $newID frame last
# 		xpaset -p $newID catalog $catalog
# 		sleep 1  # Seems to crash if you don't wait for the catalog to load
# 		xpaset -p $newID catalog symbol color red
# 		xpaset -p $newID catalog close 

# 		xpaset -p $newID zoom to fit
# 		xpaset -p $newID frame prev
# 		xpaset -p $newID zoom to fit

# 		break
# 	fi
# done
