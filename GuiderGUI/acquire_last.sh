#!/bin/bash

radec=`tcs getcoords`
ra=`echo $radec | cut -f1 -d ' '`   # RA in sexagesimal
dec=`echo $radec | cut -f2 -d ' '`  # DEC in sexagesimal
cass=`tcs getcass | cut -f1 -d ' '`	# Cass angle in degrees

FPslit=`FPoffsets SCOPE SLIT 0 0 $cass | grep XANGLE`  # FPoffsets prints a few lines
slitangle=`echo $FPslit | cut -f4 -d '='`  # Looks like this: SLIT center: RA=08:34:47.69  DEC=+22:20:16.8  XANGLE=0.000000

acam acquire $ra $dec $slitangle