#!/usr/bin/python3.9

'''
Script to find best focus position from series of NGPS spectrum images.
Generates plots for each channel for which a region file is given.
Assumes regions are labeled in the format "SLICE_FEATURE" e.g. "A_1" or "CENTER_6550"

No image processing - assumes 1 FITS file per focus position
'''

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib import rcParams
rcParams.update({'font.size': 14})

import argparse, sys
import numpy as np
import astropy.io.fits as pf
from pandas.api.types import is_numeric_dtype

from FITS_tools.organize_headers import all_headers_to_df
from FITS_tools.regions import make_region_dict


help = 'Script to plot the focus metric from a series of NGPS spectrum images.'
epilog = 'Example: focus_spec.py focus_001*.fits -R focusR.reg -I focusI.reg -fk FOCUS  \n'

parser = argparse.ArgumentParser(  # Make printed help text wider
  formatter_class=lambda prog: argparse.HelpFormatter(prog,max_help_position=40) ,description=help ,epilog=epilog)

help = 'List of filenames or regexes (multi-extension FITS)'
parser.add_argument('flist', type=str ,nargs='*' ,help=help)

help = 'Focus keyword in FITS header that we are optimizing (FOCUS, TELFOCUS, IMNUM, IMNUM_HACK)'
parser.add_argument('-fk','--focuskey', type=str, default='IMNUM' ,help=help)

help = 'U channel DS9 region file'
parser.add_argument('-U', type=str ,help=help)

help = 'G channel DS9 region file'
parser.add_argument('-G', type=str ,help=help)

help = 'R channel DS9 region file'
parser.add_argument('-R', type=str ,help=help)

help = 'I channel DS9 region file'
parser.add_argument('-I', type=str ,help=help)

help = 'Sort plots by wavelength feature instead of by image slice'
parser.add_argument('-gf', '--groupby-feature', action='store_true' ,help=help)

help = 'Override region height (px)'
parser.add_argument('-dy', type=int ,help=help)

help = 'File number range'
parser.add_argument('--range', type=int, nargs=2 ,help=help)


#print(args)

## PARSE INPUTS
args=parser.parse_args()

# Organize region files
regfiles = {}
if args.U: regfiles['U'] = args.U
if args.G: regfiles['G'] = args.G
if args.R: regfiles['R'] = args.R 
if args.I: regfiles['I'] = args.I
if len(regfiles.keys())==0: sys.exit('No region files specified')

regdict = {k:make_region_dict(v, DY=args.dy) for k,v in regfiles.items()}

GROUPBY_SLICE = not args.groupby_feature  # Group plots by slice (true) or spectrum feature (false)

# Identify all files
df = all_headers_to_df(args.flist)
focuskey = args.focuskey
imnumkey = 'IMNUM'

# Make the hacky image number column just in case needed
# image number is before the dot, after the last '_': basename_00000.fits
df['IMNUM_HACK'] = [int(fname.split('.')[0].split('_')[-1]) for fname in df['FILENAME']]

if focuskey not in df.columns: sys.exit('Focus keyword not found: '+focuskey)
if not is_numeric_dtype(df[focuskey]): sys.exit('Focus keyword has missing/non-numeric values: '+focuskey)
if df[focuskey].isnull().values.any(): sys.exit('Focus keyword has missing/non-numeric values: '+focuskey)

# Filter by image number
if args.range:
    if 'IMNUM' not in df.columns: imnumkey = 'IMNUM_HACK'

    myFilter = (df[imnumkey]>=args.range[0]) & (df["IMNUM_HACK"]<=args.range[1])
    df = df[myFilter]

# Analysis functions
def med_reduce(img, project=True):
    # divide out median to account for flux changes, then project out columns to get 1D spectrum
    ans = img/np.median(img)
    if project: ans = np.median(ans,axis=0)
    return ans 

def get_stds_from_row(row, project=True):
    '''Normalize ROI, then get STDDEV of 2D area or projected 1D spectrum'''
    # Uses global roi_dict
        
    img = pf.getdata(row['FILENAME'],row['EXTN'])
    ch = row['SPEC_ID']
        
    stds = {f: med_reduce(img[roi], project=True).std() for f,roi in regdict[ch].items() }
    
    return stds    # stds[feature] = float

# Compute STDs for all rows
df['std_dict'] = df.apply(lambda row: get_stds_from_row(row), axis=1)


# PLOTTING -- 1 file per channel, 1 axis per group

for ch in regdict:

    plt.figure()
        
    df_ch = df[df['SPEC_ID']==ch]
    ROInames = list(regdict[ch].keys())
    sliceTags = np.unique([rn.split('_')[0] for rn in ROInames])
    featureTags = np.unique([rn.split('_')[1] for rn in ROInames])
    
    if GROUPBY_SLICE: 
        groupTags, curveTags, curve_i = (sliceTags, featureTags, 1)
        plotTag, legendTitle = ('Slice','Feature')        
    else:
        groupTags, curveTags, curve_i = (featureTags, sliceTags, 0)
        plotTag, legendTitle = ('Feature','Slice')

    fig, axes_g = plt.subplots(len(groupTags), sharex=True, sharey=False, figsize=(6, 8))  # sharey=True makes limits the same

    if len(groupTags)==1: axes_g = [axes_g]  # subplots doesn't return a list if it's just 1

    # g=group, c=curve
    for ax, g in zip(axes_g, groupTags):
        for rn in ROInames:
            if g not in rn: continue
            c = rn.split('_')[curve_i]
            ax.text(1.02, 0.1, "%s %s"%(plotTag,g), transform=ax.transAxes, fontsize=10,
            verticalalignment='top')

            x = df_ch[focuskey]
            y = df_ch.apply(lambda row: row['std_dict'][rn], axis=1)
            ax.plot(x, y/y.max(), label="%s"%(c))

            ax.tick_params(top=True, labeltop=True, bottom=True, labelbottom=True)
            ax.tick_params(axis="x",direction="in", pad=-17)

            
    axes_g[0].set_title("%s Channel"%(ch))
    axes_g[-1].set_xlabel(focuskey)
    plt.xticks(x[::2])
    fig.supylabel('Scaled STDDEV')
    axes_g[0].legend(bbox_to_anchor=(1.02, 1.02), loc='upper left', borderaxespad=0, prop={'size': 10}, title=legendTitle)
    plt.tight_layout()

    outname = 'focus_spec_%s.png'%ch
    plt.savefig(outname)
    print(outname)
