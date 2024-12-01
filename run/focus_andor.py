#!/usr/bin/python3.9

'''Find best focus on ACAM or sliceviewer using a focus scan image series
No image processing; works with 1 image per focus position

USAGE:  ./NGPS_focus_andor.py regex1 regex2 ... regexN

e.g. ./NGPS_focus_andor.py /data/acam/20241114/focscan12*.fits

'''

import sys
flist = sys.argv[1:]

if (len(flist)==0 or '-h' in flist ):
    print('USAGE:  ./NGPS_focus_andor.py regex1 regex2 ... regexN')
    print('e.g. ./NGPS_focus_andor.py /data/acam/20241114/focscan12*.fits')
    print('')
    print('You can use multiple regex arguments to capture all the filenames, ')
    print('but the easiest thing is to move/link the files into a separate directory and use *.fits.')
    sys.exit(0)

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib import rcParams
rcParams.update({'font.size': 14})
# plt.rcParams['figure.figsize'] = [6, 3]

import numpy as np
import astropy.io.fits as pf
import astropy.io.ascii as ascii
import os.path
from pandas.api.types import is_numeric_dtype
# import pandas as pd

# NGPS imports
from FITS_tools.organize_headers import all_headers_to_df
from acam_skyinfo import settings as s

focuskey = 'TELFOCUS'  # FITS header used for focus
metrickey = 'FWHM_IMAGE'
# metrickey = 'FWHM_WORLD'

print(s.SEXTRACTOR_CONFIG)


### NEED SATURATION CUT?
### NEED SEPARATE CONFIG FILE FOR SLICEVIEW CAMS ?  SIMULATIONS?

def runsex_on_file(filename, ext=None, config=s.SEXTRACTOR_CONFIG):

    if os.path.exists(s.SEXTRACTOR_CATALOG): os.system('rm '+s.SEXTRACTOR_CATALOG)  ### define new path?

    cmd = "sex %s -c %s" % (filename, config)
    cmd += ''.join([' -%s %s' % (k,v) for k, v in s.SEparams.items()])  # add all SE options
    os.system(cmd)
    observation_raw = ascii.read(s.SEXTRACTOR_CATALOG)
    
    if ext: observation_raw = observation_raw[observation_raw["EXT_NUMBER"]==ext]

    if len(observation_raw) > 0:
        print('Extension '+str(ext),'SExtractor detections: %5i' % (len(observation_raw)))
    else:
        msg0 = 'Extension '+str(ext)+'  No SExtractor sources'
        print(msg0)
        raise RuntimeError(msg0)

    observation_raw.sort([s.FLUX_KEY])  # sort faintest to brightest
    
    observation_raw = observation_raw[observation_raw[metrickey]>0]  # discard sources with fwhm=0
    
    return observation_raw.to_pandas()

def DS9regions_from_sextable(df, regfilename):
    '''
    df:  SExtractor output table as Pandas DataFrame
    regfilename:  String with absolute output path/filename
    '''
    
    regfile = open(regfilename, "w") 
        
    regfile.write('# Region file format: DS9 version 4.1\n')
    regfile.write('global color=green dashlist=8 3 width=1 font="helvetica 10 normal roman"' +
                  'select=1 highlite=1 dash=0 fixed=0 edit=1 move=1 delete=1 include=1 source=1\n')
    regfile.write('image\n')
    
    regLine = 'circle(%s,%s,%s)\n'
    
    for i, row in df.iterrows():
        regTuple = (row["XWIN_IMAGE"], row["YWIN_IMAGE"], row["FWHM_IMAGE"])
        regfile.write(regLine % regTuple)
    

# Scrape all FITS headers, load into DataFrame
df = all_headers_to_df(flist)

# Count unique extensions, decide ACAM or SCAM
camname = 'acam' if len(df["EXTN"].unique())==1 else 'scam'

extkey = "EXTNAME" if "EXTNAME" in df.columns else "EXTN"

# Try to use filenames as focus key if focus key is missing 
dohack=False
if focuskey not in df.columns: dohack=True
elif not is_numeric_dtype(df[focuskey]): dohack=True

if dohack:
    df['IMNUM_HACK'] = [int(os.path.basename(fname).split('_')[1]) for fname in df['FILENAME']]
    focuskey = 'IMNUM_HACK'

print('Focus keyword = '+focuskey)

# Compute STDDEV of whole image; quick and dirty focus metric
df['STDDEV'] = df.apply(lambda row: pf.getdata(row['FILENAME'], ext=row['EXTN']).std() ,axis=1)
df['SUM'] = df.apply(lambda row: pf.getdata(row['FILENAME'], ext=row['EXTN']).sum() ,axis=1)

# SExtractor Loop over files
# sadly SExtractor always scans all extensions and compiles 1 table per file; identify by EXT_NUMBER
# So all df extensions initially have detections from all extensions

for ex in df["EXTN"].unique():
    
    df_ex = df[df["EXTN"]==ex]
    
    # plt.figure()
    # plt.title("Extension %s"%df_ex.iloc[0][extkey])
    
    for i, row in df_ex.iterrows():

        try:
            obstable = runsex_on_file(row["FILENAME"], ext=ex, config=s.SEXTRACTOR_CONFIG)
        except RuntimeError as e:
            continue
        
        obstable = obstable.sort_values(['FLUX_ISOCOR'])

        df.loc[i,metrickey] = np.median(obstable[metrickey])
        df.loc[i,metrickey+"_STD"] = np.std(obstable[metrickey])
        df.loc[i,"Ndetect"] = len(obstable)

    #     p = plt.hist(obstable[metrickey], bins=range(30), alpha=.5, label=str(row[focuskey]))

    # plt.legend()
    # plt.xlabel("FWHM (px)")


# PLOT focus metric vs. focus key


for ex in df[extkey].unique():

    df_ex = df[df[extkey]==ex]
    y = df_ex[metrickey]

    try:  # Fails if no sources detected

        # Fit fwhm vs focus to parabola
        pfit = np.polyfit(df_ex[focuskey], y, 2)  # p2, p1, p0  i.e. a, b, c
        vertex = -pfit[1]/2./pfit[0]  # -b/2a

    except:
        print('Problem fitting parabola to values; maybe has NaNs')

    dx=.1
    x = np.arange(df_ex[focuskey].min(), df_ex[focuskey].max()+dx, dx)

    plt.plot(df_ex[focuskey], df_ex[metrickey], marker='s', ls='none', label="Ext %s"%ex)
    plt.plot(x, np.polyval(pfit, x), ls='--')

    plt.xlabel(focuskey)
    plt.ylabel(metrickey)

    print('Ext '+str(ex)+'  '+metrickey+" Minimum at:", df_ex[df_ex[metrickey]==df_ex[metrickey].min()][focuskey].values , \
        "  Vertex fit at:", vertex)
    
plt.legend()
outname = 'focus_andor_FWHM_'+camname+'.png'
print(outname)
plt.savefig(outname)


# PLOT STDDEV vs. focus key

plt.figure()

for ex in df[extkey].unique():
    
    df_ex = df[df[extkey]==ex]
    
    y = df_ex['STDDEV']/df_ex['SUM']  # normalize by sum to account for flux variation
    y = y/y.mean()

    try:  # Fails if no sources detected

        # Fit fwhm vs focus to parabola
        pfit = np.polyfit(df_ex[focuskey], y, 2)  # p2, p1, p0  i.e. a, b, c
        vertex = -pfit[1]/2./pfit[0]  # -b/2a

    except:
        print('Problem fitting parabola to values; maybe has NaNs')

    dx=.1
    x = np.arange(df_ex[focuskey].min(), df_ex[focuskey].max()+dx, dx)

    plt.plot(df_ex[focuskey], y, marker='s', ls='none', label="Ext %s"%ex)
    plt.plot(x, np.polyval(pfit, x), ls='--')

    plt.ylabel("STDDEV (ADU)")

    print('Ext '+str(ex)+'  '+"STD Minimum at:", df_ex[df_ex['STDDEV']==df_ex['STDDEV'].min()][focuskey].values , \
        "  Vertex fit at:", vertex)
    
plt.legend()
plt.xlabel(focuskey)
plt.ylabel('STD/SUM')

outname = 'focus_andor_STD_'+camname+'.png'
print(outname)
plt.savefig(outname)



