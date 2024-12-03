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

# Update headsup display for SLICEVIEWER
if [[ "$camera" == "slicev" ]]; then

	# parameters to prevent editing the regions; need delete to refresh them
	notouch="edit=0 move=0 rotate=0 delete=1" 

	# clear all regions
	xpaset -p $id region delete all

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
	fontsize=$((${headsup_fontsize}-$vbin))
	font="{$headsup_font $fontsize $headsup_fontstyle}"

	textreg="image; text $XCENTER $YCENTER # text={EXPTIME=${exptime}   GAIN=${gain}   BIN=${vbin}   SLIT=${slitw}\"} \
	  color=${headsup_fontcolor} width=2 $notouch font=$font"
	echo "$textreg" | xpaset $id region 2>&1

	# Slit graphic
	xslit=`xpaget $id fits header keyword CRPIX1`  # defaults to Left camera?
	yslit=`xpaget $id fits header keyword CRPIX2`
	slitw_arcsec=`xpaget $id fits header keyword SLITW`
	pixscale=`xpaget $id fits header keyword PIXSCALE`

	xwidth=$(echo "scale=2; $slitw_arcsec / $pixscale" | bc) # "scale" sets decimal places in bc
	ywidth=$(echo "scale=2; 10 / $pixscale" | bc)
	slitreg="image; box($xslit,$yslit,$xwidth,$ywidth,0) # color=$graphic_color $notouch tag={slitcenter}"
	echo "$slitreg" | xpaset $id region

	# Slice labels
	YCENTER=`xpaget $id fits header 1 keyword NAXIS2`
	YCENTER=$(($YCENTER-8))
	slicereg="image; text $xslit $YCENTER # text={     TOP SLICE-->           <--BOTTOM SLICE} \
	  color=${headsup_fontcolor} width=2 $notouch font=$font"
	echo "$slicereg" | xpaset $id region

	# Scale graphic
	scale_arcsec=5
	xwidth=$(echo "scale=2; $scale_arcsec / $pixscale" | bc)
	YCENTER=$(($YCENTER-16))  # slightly below slice labels
	scalereg="image; box($xslit,$YCENTER,$xwidth,1,0) # color=${headsup_fontcolor} fill=1 \
		$notouch text={$scale_arcsec\"}"
	echo "$scalereg" | xpaset $id region

	# TCS marker
	TELRA_hr=`xpaget $id fits header 1 keyword TELRA`
	TELRA_deg=$(echo "scale=6; $TELRA_hr * 15 " | bc)
	TELDEC_deg=`xpaget $id fits header 1 keyword TELDEC`
	markerreg="wcs; point $TELRA_deg $TELDEC_deg # point=x 20 color=$graphic_color $notouch text={TCS-test}"
	echo "$markerreg" | xpaset $id region

fi
