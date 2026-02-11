#!/bin/bash

# Update the ds9 slice viewer gui display with a 2-extension FITS file

# bash cannot do floating point arithmetic, so awk is sometimes used below for calculations

camera=$1
fname=$2

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $SCRIPT_DIR/gui.config $camera

region_key_version=1
region_cache="/tmp/ngps_${camera}_regions.key"
xpaset -p $id preserve regions yes >/dev/null 2>&1

# Preserve crosshair position before loading new image (DS9 resets it on load).
crosshair_img=""
mode=$(xpaget $id mode 2>/dev/null)
crosshair_img=$(xpaget $id crosshair image 2>/dev/null)

# Temporarily hide crosshair while loading to avoid center flash
if [ "$mode" = "crosshair" ]; then
	xpaset -p $id mode none
fi

# Push the image to frame 1
xpaset -p $id frame 1
cat $fname | xpaset $id $fitstype

# Restore crosshair position if we were able to capture it
if [ -n "$crosshair_img" ]; then
	xpaset -p $id crosshair $crosshair_img image 2>/dev/null
elif [ "$mode" = "crosshair" ]; then
	# Keep crosshair mode if it was active but position was unavailable
	xpaset -p $id mode crosshair
fi

# parameters to prevent editing the regions
notouch="edit=0 move=0 rotate=0 delete=1" 

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
	if ! [[ "$NAXIS1" =~ ^[0-9]+$ && "$NAXIS2" =~ ^[0-9]+$ ]]; then
		echo "WARN: missing/invalid NAXIS in FITS ($NAXIS1,$NAXIS2)" >&2
		exit 0
	fi
	XCENTER=$(($NAXIS1 / 2))
	pixscale=`xpaget $id fits header keyword PIXSCALE`

	# Slit/Scale graphic
	scale_arcsec=30
	slitw_arcsec=1
	slit_angle=18 # deg

	# TCS marker
	TELRA_hr=`xpaget $id fits header keyword TELRA`
	TELRA_deg=`awk "BEGIN { print ($TELRA_hr*15) }"`
	TELDEC_deg=`xpaget $id fits header keyword TELDEC`

	regions_key=$(printf "v%s|guider|gain=%s|filter=%s|exptime=%s|focus=%s|navg=%s|status=%s|vbin=%s|NAXIS1=%s|NAXIS2=%s|pixscale=%s|scale_arcsec=%s|slitw_arcsec=%s|slit_angle=%s|TELRA_deg=%s|TELDEC_deg=%s" \
		"$region_key_version" "$gain" "$filter" "$exptime" "$focus" "$navg" "$status" "$vbin" "$NAXIS1" "$NAXIS2" "$pixscale" \
		"$scale_arcsec" "$slitw_arcsec" "$slit_angle" "$TELRA_deg" "$TELDEC_deg")

	cached_key=""
	if [ -s "$region_cache" ]; then
		cached_key="$(cat "$region_cache")"
	fi

	if [ "$regions_key" != "$cached_key" ]; then
		xpaset -p $id region delete all

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
		YCENTER=$(($NAXIS2 / 2))
		slitl=`awk "BEGIN { print ($scale_arcsec/$pixscale) }"`
		slitw=`awk "BEGIN { print ($slitw_arcsec/$pixscale) }"`

		scalereg="image; box($XCENTER,$YCENTER,4,$slitl,$slit_angle) # color=${headsup_fontcolor} fill=1 \
			$notouch text={$scale_arcsec\"   }"
		echo "$scalereg" | xpaset $id region

		# TCS marker
		markerreg="icrs; point $TELRA_deg $TELDEC_deg # point=x 20 $notouch text={TCS}"
		echo "$markerreg" | xpaset $id region

		printf "%s" "$regions_key" > "$region_cache"
	fi
fi

# Update headsup display for SLICEVIEW
if [[ "$camera" == "slicev" ]]; then

	# Get info from FITS headers
	vbin=`xpaget $id fits header 1 keyword VBIN`
	if ! [[ "$vbin" =~ ^[0-9]+$ && "$vbin" -gt 0 ]]; then
		echo "WARN: missing/invalid VBIN in FITS ($vbin)" >&2
		exit 0
	fi
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
	# Use DS9-reported display size when available (mosaic coords can differ from ext header)
	NAXIS2=""
	ds9_size=$(xpaget $id fits size 2>/dev/null)
	if [[ "$ds9_size" =~ ^[0-9]+[[:space:]]+[0-9]+$ ]]; then
		NAXIS2=${ds9_size##* }
	else
		NAXIS2=`xpaget $id fits header 1 keyword NAXIS2`
	fi
	if ! [[ "$NAXIS2" =~ ^[0-9]+$ && "$NAXIS2" -gt 0 ]]; then
		echo "WARN: missing/invalid NAXIS2 in FITS ($NAXIS2)" >&2
		exit 0
	fi

	# TCS marker
	TELRA_hr=`xpaget $id fits header 1 keyword TELRA`
	TELRA_deg=`awk "BEGIN { print ($TELRA_hr*15) }"`
	TELDEC_deg=`xpaget $id fits header 1 keyword TELDEC`

	slitw_px=`awk "BEGIN { print ($slitw_arcsec/$pixscale) }"`
	slitl_px=`awk "BEGIN { print ( 10 / $pixscale ) }"` # constant 10" high
	radius=`awk "BEGIN { print ($slitw_px*1.5) }"`

	scale_arcsec=5
	scalex=$((240/$vbin))
	scaley=$((NAXIS2/2))
	scalel=`awk "BEGIN { print ($scale_arcsec/$pixscale) }"`

	regions_key=$(printf "v%s|slicev|gain=%s|exptime=%s|slitw_arcsec=%s|navg=%s|vbin=%s|xslit=%s|yslit=%s|pixscale=%s|NAXIS2=%s|TELRA_deg=%s|TELDEC_deg=%s|slitw_px=%s|slitl_px=%s|radius=%s|scalex=%s|scaley=%s|scalel=%s|graphic_color=%s|headsup_fontsize=%s|headsup_fontcolor=%s" \
		"$region_key_version" "$gain" "$exptime" "$slitw_arcsec" "$navg" "$vbin" "$xslit" "$yslit" "$pixscale" "$NAXIS2" "$TELRA_deg" "$TELDEC_deg" \
		"$slitw_px" "$slitl_px" "$radius" "$scalex" "$scaley" "$scalel" "$graphic_color" "$headsup_fontsize" "$headsup_fontcolor")

	cached_key=""
	if [ -s "$region_cache" ]; then
		cached_key="$(cat "$region_cache")"
	fi

	if [ "$regions_key" != "$cached_key" ]; then
		xpaset -p $id region delete all

		# Camera settings
		YCENTER=$((40/$vbin))
		fontsize=$((${headsup_fontsize}-$vbin))
		font="{$headsup_font $fontsize $headsup_fontstyle}"   ### NOT WORKING
		echo $font

		reg="image; text $xslit $YCENTER # text={EXPTIME=${exptime}   EMGAIN=${gain}   BIN=${vbin}   N_AVG=${navg}    } \
		  color=${headsup_fontcolor} width=2 $notouch font={helvetica ${headsup_fontsize} bold}"
		echo "$reg" | xpaset $id region 2>&1

		# Slit graphic
		reg="image; box($xslit,$yslit,$slitw_px,$slitl_px,0) # color=$graphic_color $notouch tag={slitcenter}"
		echo "$reg" | xpaset $id region

		# circle around slit
		reg="image; circle($xslit,$yslit,$radius) # color=$graphic_color $notouch"
		echo "$reg" | xpaset $id region

		# Slit/Slice labels
	  YCENTER=$(($NAXIS2 - 36/$vbin))
		reg="image; text $xslit $YCENTER # text={TOP SLICE        SLIT=${slitw_arcsec}\"       BOTTOM SLICE   } \
		  color=${headsup_fontcolor} width=2 $notouch font={helvetica ${headsup_fontsize} bold}"
		echo "$reg" | xpaset $id region

		# Scale graphic
		reg="image; box($scalex,$scaley,1,$scalel,0) # color=${headsup_fontcolor} fill=1 \
			$notouch text={$scale_arcsec\"}"
		echo "$reg" | xpaset $id region

		# TCS marker
		reg="icrs; point($TELRA_deg, $TELDEC_deg) # point=x 20 color=$graphic_color $notouch text={    TCS}"
		echo "$reg" | xpaset $id region

		printf "%s" "$regions_key" > "$region_cache"
	fi

fi
