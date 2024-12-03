#!/bin/bash

# Update the ds9 slice viewer display and menus with given arguments
# slice viewer camera daemon should call this script after changing anything displayed by the GUI
#
# USAGE:  ./push_settings_slicev.sh <update_menu> <exptime> <gain>
# exptime (float) = exposure time (s)
# gain (int) = camera gain
# bin (int) = on-chip binning (px)
# update_menu (bool) = whether to update the GUI settings menu with current values

camera=slicev

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $SCRIPT_DIR/gui.config $camera

# settings from daemon or DS9 menu
update_menu=$1
exptime=$2
gain=$3
bin=$4

# Get the jog size parameter which is just 1 number saved in a file
# The jog settings menu writes to this file
JOGSIZE_ARCSEC=`cat $SCRIPT_DIR/jog.ini`

# create new analysis file from template and new parameters
sed -e "s/DEFexptime/$exptime/" \
    -e "s/DEFgain/$gain/" \
    -e "s/DEFbin/$bin/" \
    -e "s/DEFjog/$JOGSIZE_ARCSEC/" \
    $anstemplate > $ansfile

# load analysis file
if ${update_menu,,}; then  # bool must be lowercase
    xpaset -p $id analysis clear load $ansfile
fi

# # update headsup display
# # put text center at xmax from DATASEC="[xmin:xmax,ymin:ymax]" (right edge of cropped left image)
# qq=`xpaget $id fits header 1 keyword DATASEC | cut -f2 -d ':'`
# width=`echo $qq | cut -f1 -d ','`  # get X max from DATASEC="[xmin:xmax,ymin:ymax]"
# #width=700  ### HARDCODE

# #hbin=`xpaget $id fits header 1 keyword HBIN`
# vbin=`xpaget $id fits header 1 keyword VBIN`
# XCENTER=$(($width)) 
# YCENTER=$((24/$vbin))
# headsup_fontsize=$((${headsup_fontsize}-$vbin))

# xpaset -p $id region delete all
# echo "image; text $XCENTER $YCENTER # text={EXPTIME=${exptime}   GAIN=${gain}   BIN=${bin}} \
#   color=${headsup_fontcolor} width=2 edit=0 move=0 rotate=0 delete=1 font={helvetica ${headsup_fontsize} bold}" \
#   | xpaset $id region 2>&1

# # Slit graphic
# # Define the slit region coordinates
# xslit=`xpaget $id fits header keyword CRPIX1`  # defaults to Left camera?
# yslit=`xpaget $id fits header keyword CRPIX2`
# slitw_arcsec=`xpaget $id fits header keyword SLITW`
# pixscale=`xpaget $id fits header keyword PIXSCALE`

# xwidth=$(echo "scale=2; $slitw_arcsec / $pixscale" | bc)
# ywidth=$(echo "scale=2; 10 / $pixscale" | bc)
# slitreg="image; box($xslit,$yslit,$xwidth,$ywidth,0) # color=cyan edit=0 move=0 rotate=0 tag={slitcenter}"

# #Load the slit
# echo "$slitreg" | xpaset $id region
