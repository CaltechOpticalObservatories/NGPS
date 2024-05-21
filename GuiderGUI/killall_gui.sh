#!/bin/bash

# Kill all ds9 windows for the GUIDER and SLITVIEW GUIs and restart xpans.
# Use this as a blunt instrument when GUIs aren't working

zenity --question --text="Are you sure you wish to kill all ds9 GUIs?" --width=300
yesno=$?  # YES->0 ; NO->1

echo YESNO $yesno

if [ "$yesno" == "0" ]; then

    echo Killing ds9 GUIs...

    pkill -9 -f "GUIDER";
    pkill -9 -f "SLICEVIEW";
    pkill -9 xpans;  # xpans will be restarted with next ds9 call

fi
