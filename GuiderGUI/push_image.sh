#!/bin/bash

# Update the ds9 slice viewer gui display with a 2-extension FITS file

# bash cannot do floating point arithmetic, so awk is sometimes used below for calculations

camera=$1
fname=$2

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $SCRIPT_DIR/gui.config $camera

# Push the image to frame 1
xpaset -p $id frame 1
cat $fname | xpaset $id $fitstype

# Force crosshair mode
xpaset -p $id mode crosshair;

# parameters to prevent editing the regions; need delete to refresh them
notouch="edit=0 move=0 rotate=0 delete=1" 


# Update headsup display for GUIDER
if [[ "$camera" == "guider" ]]; then

	# clear all regions
	xpaset -p $id region delete all

	width=`xpaget $id fits header keyword NAXIS1`
	height=`xpaget $id fits header keyword NAXIS2`
	XCENTER=$(($width / 2))
	YCENTER=30
	YCENTER_STATUS=$(($height - $YCENTER))

	gain=`xpaget $id fits header keyword CCDGAIN`
	filter=`xpaget $id fits header keyword FILTER`
	exptime=`xpaget $id fits header keyword EXPTIME`
	exptime=`printf "%'.3f\n" $exptime`
	focus=`xpaget $id fits header keyword TELFOCUS`
	focus=`printf "%'.2f\n" $focus`
	navg=`xpaget $id fits header keyword NAVG`

	# Camera settings
	echo "image; text $XCENTER $YCENTER # text={EXPTIME=${exptime}   EMGAIN=${gain}   FILTER=${filter}   N_AVG=${navg}} \
	  color=${headsup_fontcolor} width=2 $notouch font={helvetica ${headsup_fontsize} bold}" \
	  | xpaset $id region 2>&1

	# Camera status
	status=`xpaget $id fits header keyword STATUS`
	# status=`acam acquire`  ### placeholder until above header exists
	xpaset -p $id region group status new
	echo "image; text $XCENTER $YCENTER_STATUS # text={STATUS=${status}          FOCUS=${focus}} \
	  color=${headsup_fontcolor} width=2 $notouch font={helvetica ${headsup_fontsize} bold}" \
	  | xpaset $id region 2>&1

	# Scale graphic
	scale_arcsec=20

	XCENTER=32
	YCENTER=128
	pixscale=`xpaget $id fits header keyword PIXSCALE`
	ywidth=`awk "BEGIN { print ($scale_arcsec/$pixscale) }"`

	scalereg="image; box($XCENTER,$YCENTER,4,$ywidth,0) # color=${headsup_fontcolor} fill=1 \
		$notouch text={$scale_arcsec\"}"
	echo "$scalereg" | xpaset $id region

	# TCS marker
	TELRA_hr=`xpaget $id fits header keyword TELRA`
	TELRA_deg=`awk "BEGIN { print ($TELRA_hr*15) }"`
	TELDEC_deg=`xpaget $id fits header keyword TELDEC`
	markerreg="icrs; point $TELRA_deg $TELDEC_deg # point=x 20 $notouch text={TCS}"
	echo "$markerreg" | xpaset $id region
fi

# Update headsup display for SLICEVIEW
if [[ "$camera" == "slicev" ]]; then

	# clear all regions
	xpaset -p $id region delete all

	# Get info from FITS headers
	vbin=`xpaget $id fits header 1 keyword VBIN`
	gain=`xpaget $id fits header 1 keyword CCDGAIN`
	exptime=`xpaget $id fits header 1 keyword EXPTIME`
	exptime=`printf "%'.3f\n" $exptime`
	slitw=`xpaget $id fits header 1 keyword SLITW`
	slitw=`printf "%'.3f\n" $slitw`
	navg=`xpaget $id fits header keyword NAVG`

	# horizontally center text on slit (CRPIX1)
	XCENTER=`xpaget $id fits header 1 keyword CRPIX1`
	YCENTER=$((36/$vbin))
	fontsize=$((${headsup_fontsize}-$vbin))
	font="{$headsup_font $fontsize $headsup_fontstyle}"

	textreg="image; text $XCENTER $YCENTER # text={EXPTIME=${exptime}   EMGAIN=${gain}   BIN=${vbin}   N_AVG=${navg}} \
	  color=${headsup_fontcolor} width=2 $notouch font=$font"
	echo "$textreg" | xpaset $id region 2>&1

	# Slit graphic
	xslit=`xpaget $id fits header keyword CRPIX1`  # defaults to Left camera?
	yslit=`xpaget $id fits header keyword CRPIX2`
	slitw_arcsec=`xpaget $id fits header keyword SLITW`
	pixscale=`xpaget $id fits header keyword PIXSCALE`

	xwidth=`awk "BEGIN { print ($slitw_arcsec/$pixscale) }"`
	ywidth=`awk "BEGIN { print ( 10 / $pixscale ) }"` # constant 10" high
	slitreg="image; box($xslit,$yslit,$xwidth,$ywidth,0) # color=$graphic_color $notouch tag={slitcenter}"
	echo "$slitreg" | xpaset $id region

	# circle around slit
	radius=`awk "BEGIN { print ($xwidth*1.5) }"`
	slitreg="image; circle($xslit,$yslit,$radius) # color=$graphic_color $notouch"
	echo "$slitreg" | xpaset $id region

	# Slice labels
	YCENTER=`xpaget $id fits header 1 keyword NAXIS2`
	YCENTER=$(($YCENTER-8))
	slicereg="image; text $xslit $YCENTER # text={     TOP SLICE        SLIT=${slitw}\"        BOTTOM SLICE} \
	  color=${headsup_fontcolor} width=2 $notouch font=$font"
	echo "$slicereg" | xpaset $id region

	# Scale graphic
	scale_arcsec=5

	datasec=`xpaget $id fits header 1 keyword DATASEC`
	XCENTER=`echo $datasec | cut -f1 -d ':' | cut -f2 -d '['`
	XCENTER=$((XCENTER+10))
	YCENTER=32
	ywidth=`awk "BEGIN { print ($scale_arcsec/$pixscale) }"`

	scalereg="image; box($XCENTER,$YCENTER,1,$ywidth,0) # color=${headsup_fontcolor} fill=1 \
		$notouch text={$scale_arcsec\"}"
	echo "$scalereg" | xpaset $id region

	# TCS marker
	TELRA_hr=`xpaget $id fits header 1 keyword TELRA`
	TELRA_deg=`awk "BEGIN { print ($TELRA_hr*15) }"`
	TELDEC_deg=`xpaget $id fits header 1 keyword TELDEC`
	markerreg="icrs; point($TELRA_deg, $TELDEC_deg) # point=x 20 color=$graphic_color $notouch text={    TCS}"
	echo "$markerreg" | xpaset $id region

fi
