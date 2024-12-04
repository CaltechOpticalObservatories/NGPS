#!/bin/bash

camera=guider

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $SCRIPT_DIR/gui.config $camera

xpaset -p $id zoom to fit
xpaset -p $id zoom out
xpaset -p $id zoom out