#!/bin/bash

# Send a text message to camera GUI
# ./message.sh <camera> "<message>"   where <camera> is guider or slicev

camera=$1
msg=$2

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $SCRIPT_DIR/gui.config $camera

echo "`date -Iseconds`  $msg" | xpaset $id analysis text
