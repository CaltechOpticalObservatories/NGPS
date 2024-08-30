#!/bin/bash

# USAGE: ./ngpsgui.sh [PLAN|OBSERVE]

java_exec=/home/developer/java/build_tools/jdk-14.0.2/bin/java

opt=$1  # OBSERVE or PLAN

guipath=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )  # Directory of this file

cd $guipath

$java_exec -jar ngps.jar ${opt^^}
