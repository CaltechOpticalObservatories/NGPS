#!/bin/bash

# Move to the working directory
cd /home/observer/focus || exit 1

/home/developer/Software/run/focus_andor.py -fk TELFOCUS /data/latest/acam/*focus*.fits
