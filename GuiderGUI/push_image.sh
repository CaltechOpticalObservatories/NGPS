#!/bin/bash

# Update the ds9 slice viewer gui display with a 2-extension FITS file

camera=$1
fname=$2

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $SCRIPT_DIR/gui.config $camera

# Push the image to frame 1
xpaset -p $id frame 1
cat $fname | xpaset $id $fitstype

# Force crosshair mode
xpaset -p $id mode crosshair;

# update headsup display
if [[ "$camera" == "slicev" ]]; then

	# Get info from FITS headers
	vbin=`xpaget $id fits header 1 keyword VBIN`
	gain=`xpaget $id fits header 1 keyword GAIN`
	exptime=`xpaget $id fits header 1 keyword EXPTIME`
	exptime=`printf "%'.3f\n" $exptime`
	slitw=`xpaget $id fits header 1 keyword SLITW`
	slitw=`printf "%'.3f\n" $slitw`

	# horizontally center text on slit (CRPIX1)
	XCENTER=`xpaget $id fits header 1 keyword CRPIX1`
	YCENTER=$((24/$vbin))
	headsup_fontsize=$((${headsup_fontsize}-$vbin))

	# clear and create the text region
	xpaset -p $id region delete all
	echo "image; text $XCENTER $YCENTER # text={EXPTIME=${exptime}   GAIN=${gain}   BIN=${vbin}   SLIT=${slitw}\"} \
	  color=${headsup_fontcolor} width=2 edit=0 move=0 rotate=0 delete=1 font={helvetica ${headsup_fontsize} bold}" \
	  | xpaset $id region 2>&1

	# Slit graphic
	# Define the slit region coordinates
	xslit=`xpaget $id fits header keyword CRPIX1`  # defaults to Left camera?
	yslit=`xpaget $id fits header keyword CRPIX2`
	slitw_arcsec=`xpaget $id fits header keyword SLITW`
	pixscale=`xpaget $id fits header keyword PIXSCALE`

	xwidth=$(echo "scale=2; $slitw_arcsec / $pixscale" | bc)
	ywidth=$(echo "scale=2; 10 / $pixscale" | bc)
	slitreg="image; box($xslit,$yslit,$xwidth,$ywidth,0) # color=cyan edit=0 move=0 rotate=0 tag={slitcenter}"

	#Load the slit
	echo "$slitreg" | xpaset $id region

	# Slice labels
	YCENTER=`xpaget $id fits header 1 keyword NAXIS2`
	YCENTER=$(($YCENTER-8))
	echo "image; text $xslit $YCENTER # text={     TOP SLICE-->           <--BOTTOM SLICE} \
	  color=${headsup_fontcolor} width=2 edit=0 move=0 rotate=0 delete=1 font={helvetica ${headsup_fontsize} bold}" \
	 | xpaset $id region
fi
