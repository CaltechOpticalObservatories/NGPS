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

# clear all regions
xpaset -p $id region delete all


# Update headsup display for GUIDER
if [[ "$camera" == "guider" ]]; then

	gain=`xpaget $id fits header keyword CCDGAIN`
	filter=`xpaget $id fits header keyword FILTER`
	exptime=`xpaget $id fits header keyword EXPTIME`
	exptime=`printf "%'.3f\n" $exptime`
	focus=`xpaget $id fits header keyword TELFOCUS`
	focus=`printf "%'.2f\n" $focus`
	navg=`xpaget $id fits header keyword NAVG`
  navg=`printf "%'.1f\n" $navg`
	vbin=`xpaget $id fits header 1 keyword VBIN`
	status=`xpaget $id fits header keyword STATUS`

	NAXIS1=`xpaget $id fits header keyword NAXIS1`
	NAXIS2=`xpaget $id fits header keyword NAXIS2`
	XCENTER=$(($NAXIS1 / 2))
	pixscale=`xpaget $id fits header keyword PIXSCALE`

	# Camera settings
	YCENTER=$((30/$vbin))
	echo "image; text $XCENTER $YCENTER # text={EXPTIME=${exptime}   EMGAIN=${gain}   FILTER=${filter}   N_AVG=${navg}} \
	  color=${headsup_fontcolor} width=2 $notouch font={helvetica ${headsup_fontsize} bold} group=headsup" \
	  | xpaset $id region 2>&1

	# Camera status
	YCENTER_STATUS=$(($NAXIS2 - $YCENTER))
	xpaset -p $id region group status new
	echo "image; text $XCENTER $YCENTER_STATUS # text={STATUS=${status}          FOCUS=${focus}} \
	  color=${headsup_fontcolor} width=2 $notouch font={helvetica ${headsup_fontsize} bold}" \
	  | xpaset $id region 2>&1

	# Slit/Scale graphic
	scale_arcsec=30
	slitw_arcsec=1
	slit_angle=18 # deg

	YCENTER=$(($NAXIS2 / 2))
	slitl=`awk "BEGIN { print ($scale_arcsec/$pixscale) }"`
	slitw=`awk "BEGIN { print ($slitw_arcsec/$pixscale) }"`

	scalereg="image; box($XCENTER,$YCENTER,4,$slitl,$slit_angle) # color=${headsup_fontcolor} fill=1 \
		$notouch text={$scale_arcsec\"   }"
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

	# Get info from FITS headers
	vbin=`xpaget $id fits header 1 keyword VBIN`
	gain=`xpaget $id fits header 1 keyword CCDGAIN`
	exptime=`xpaget $id fits header 1 keyword EXPTIME`
	exptime=`printf "%'.3f\n" $exptime`
	slitw_arcsec=`xpaget $id fits header 1 keyword SLITW`
	slitw_arcsec=`printf "%'.3f\n" $slitw_arcsec`
	navg=`xpaget $id fits header keyword NAVG`
  navg=`printf "%'.1f\n" $navg`

	xslit=`xpaget $id fits header 1 keyword CRPIX1`  # DS9 coordinate origin is on Left camera?
	yslit=`xpaget $id fits header 1 keyword CRPIX2`
	pixscale=`xpaget $id fits header keyword PIXSCALE`
	NAXIS2=`xpaget $id fits header 1 keyword NAXIS2`

	# Camera settings
	YCENTER=$((40/$vbin))
	fontsize=$((${headsup_fontsize}-$vbin))
	font="{$headsup_font $fontsize $headsup_fontstyle}"   ### NOT WORKING
	echo $font

	reg="image; text $xslit $YCENTER # text={EXPTIME=${exptime}   EMGAIN=${gain}   BIN=${vbin}   N_AVG=${navg}    } \
	  color=${headsup_fontcolor} width=2 $notouch font={helvetica ${headsup_fontsize} bold}"
	echo "$reg" | xpaset $id region 2>&1

	# Slit graphic
	slitw_px=`awk "BEGIN { print ($slitw_arcsec/$pixscale) }"`
	slitl_px=`awk "BEGIN { print ( 10 / $pixscale ) }"` # constant 10" high
	reg="image; box($xslit,$yslit,$slitw_px,$slitl_px,0) # color=$graphic_color $notouch tag={slitcenter}"
	echo "$reg" | xpaset $id region

	# circle around slit
	radius=`awk "BEGIN { print ($slitw_px*1.5) }"`
	reg="image; circle($xslit,$yslit,$radius) # color=$graphic_color $notouch"
	echo "$reg" | xpaset $id region

	# Slit/Slice labels
  YCENTER=$(($NAXIS2 - 36/$vbin))
	reg="image; text $xslit $YCENTER # text={TOP SLICE        SLIT=${slitw_arcsec}\"       BOTTOM SLICE   } \
	  color=${headsup_fontcolor} width=2 $notouch font={helvetica ${headsup_fontsize} bold}"
	echo "$reg" | xpaset $id region

	# Scale graphic
	scale_arcsec=5
	scalex=$((240/$vbin))
	scaley=$((NAXIS2/2))
	scalel=`awk "BEGIN { print ($scale_arcsec/$pixscale) }"`
	reg="image; box($scalex,$scaley,1,$scalel,0) # color=${headsup_fontcolor} fill=1 \
		$notouch text={$scale_arcsec\"}"
	echo "$reg" | xpaset $id region

	# TCS marker
	TELRA_hr=`xpaget $id fits header 1 keyword TELRA`
	TELRA_deg=`awk "BEGIN { print ($TELRA_hr*15) }"`
	TELDEC_deg=`xpaget $id fits header 1 keyword TELDEC`
	reg="icrs; point($TELRA_deg, $TELDEC_deg) # point=x 20 color=$graphic_color $notouch text={    TCS}"
	echo "$reg" | xpaset $id region

fi
