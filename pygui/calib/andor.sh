#!/bin/bash

# Move to the working directory
cd /home/observer/focus || exit 1

prefix=$(find /data/latest/acam -maxdepth 1 -type f -name 'focusloop_*_*.fits' -print0 \
  | xargs -0 -n1 basename \
  | sed -E 's/^focusloop_([0-9]{8}-[0-9]{6})_.*/\1/' \
  | sort | tail -1) \
&& /home/developer/Software/run/focus_andor.py -fk TELFOCUS "/data/latest/acam/focusloop_${prefix}_*.fits"

