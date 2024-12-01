#!/usr/bin/python3

# Convert TCS "getcoords" output to decimal degrees

import sys

if len(sys.argv)==1 or '-h' in sys.argv:
    print('Convert TCS "getcoords" output to decimal degrees')
    print('RA and DEC are in sexagesimal hours and degrees, respectively')
    print('USAGE:  ./radec.py hh:mm:ss.ss dd:mm:ss')
    sys.exit(1)

ra_string = sys.argv[1]
dec_string = sys.argv[2]

ra_hr, ra_min, ra_sec = ra_string.split(':')
dec_deg, dec_arcmin, dec_arcsec = dec_string.split(':')

ra_deg = (int(ra_hr) + int(ra_min)/60. +float(ra_sec)/3600.)*15.
dec_deg = int(dec_deg) + int(dec_arcmin)/60. +float(dec_arcsec)/3600.

print(ra_deg, dec_deg)
