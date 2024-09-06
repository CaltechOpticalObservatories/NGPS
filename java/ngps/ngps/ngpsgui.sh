#!/bin/bash

# USAGE: ./ngpsgui.sh [PLAN|OBSERVE]

java_exec=/home/developer/java/build_tools/jdk-14.0.2/bin/java

# Check number of arguments
if [ $# = 0 ]; then
    echo "USAGE: ./ngpsgui.sh [PLAN|OBSERVE]"
    exit 1
fi

opt=${1^^}  # OBSERVE or PLAN # ^^ forces uppercase

guipath=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )  # Directory of this file
cd $guipath

# Form the java command
cmd="$java_exec -jar ngps.jar $opt" 

# "Splash screen" to let users know something's happening
zenity --info \
  --title "Welcome to NGPS" \
  --text "The GUI will start in a moment..." \
  --width 300 \
  --timeout=3

# If OBSERVE, use file lock
if [ "$opt" = "OBSERVE" ]; then
    flock -n /tmp/ngps-obsserve-lock $cmd
    
    if [ $? != 0  ]; then
        zenity --warning \
         --title "Warning Message" \
         --width 300 \
         --height 100 \
         --text "NGPS-OBSERVE is already in use."
    fi

else
    $cmd
fi
