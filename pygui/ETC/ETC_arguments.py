import argparse
from sys import path
from numpy.ma import is_masked
from ETC_import import sourcesdir
from ETC_config import slitmodes

# Hack to avoid exiting the program when there's a parser error
class ArgumentParserError(Exception): pass
def raiseerror(self, message): raise ArgumentParserError(message)
def noQuitETCparser():
	'''Change behavior of ArgumentParser to raise an exception instead of quitting on an error'''
	argparse.ArgumentParser.error = classmethod(raiseerror)

help = 'Run the Exposure Time Calculator.  Outputs are SNR, EXPTIME, wavelength range, and optional plots. '
help += 'The model assumes that signals from 3 image slicer paths are summed for the SNR calculation.'
epilog = 'Example minimum argument set:\n'
epilog += './ETC_main.py G 500 510 SNR 10 -slit SET .5 -seeing 1 500 -airmass 1 -skymag 21.4 -mag 18. -magsystem AB -magfilter match'

parser = argparse.ArgumentParser(  # Make printed help text wider
  formatter_class=lambda prog: argparse.HelpFormatter(prog,max_help_position=40) ,description=help ,epilog=epilog)

# Define some extra types to enforce values for arguments

def posfloat(value): # require > 0
    fvalue = float(value)
    if fvalue <= 0:
        raise argparse.ArgumentTypeError("%s is an invalid positive float" % value)
    return fvalue

def nonegfloat(value): # require >= 0
    fvalue = float(value)
    if fvalue < 0:
        raise argparse.ArgumentTypeError("%s is an invalid non-negative float" % value)
    return fvalue

def posint(value): # require > 0
    ivalue = int(value)
    if ivalue <= 0:
        raise argparse.ArgumentTypeError("%s is an invalid positive int" % value)
    return ivalue

def slitfloat(value): # require slit in slit_w_range
	from ETC_config import slit_w_range
	fvalue = float(value)
	slitmin, slitmax = slit_w_range.to('arcsec').value
	if (fvalue < slitmin) or (fvalue > slitmax):
	    raise argparse.ArgumentTypeError("slitwidth must be in %s" % (slit_w_range))
	return fvalue

SNRparam = parser.add_argument_group('SNR parameters')

from ETC_config import channels
help = 'Spectrograph channel used for SNR'
parser.add_argument('channel', type=str, choices=channels ,help=help)

help = 'Min and max wavelength (nm) for SNR avg, e.g. "500 510". Will be rounded up to a whole number of bins'
parser.add_argument('wrange', type=posfloat ,nargs=2 ,help=help )

help = 'Fix SNR or EXPTIME and calculate the other'
parser.add_argument('ETCmode', type=str, choices=['SNR','EXPTIME','SET'], help=help)

help = 'Value of the fixed parameter: SNR (dimensionless) or EXPTIME (s)'
parser.add_argument('ETCfixed', type=posfloat, help=help)

help = 'On-chip binning along dispersion/spectral axis'
parser.add_argument('-binspect','-bindisp', type=posint, default=1, help=help)

help = 'On-chip binning along spatial axis'
parser.add_argument('-binspat', type=posint, default=1, help=help)

help = 'Only use flux from the center slit, not side slices'
parser.add_argument('-noslicer', action='store_true', help=help)

help = 'Assume astronomer only uses 2 brightest pixels in center slice for SNR'
parser.add_argument('-fastSNR', action='store_true', help=help)

help = 'Plot SNR vs. wavelength for the solution'
parser.add_argument('-plotSNR', action='store_true', help=help)

help = 'Make diagnostic plots'
parser.add_argument('-plotslit', action='store_true', help=help)

help = 'Print timing info'
parser.add_argument('-timer', action='store_true', help=help)

help = 'Use hi-res spectra calculations to improve SNR accuracy'
parser.add_argument('-hires', action='store_true', help=help)

help = 'Increase accuracy when solving for slit width and exptime simultaneously. (slower)'
parser.add_argument('-hires_solve', action='store_true', help=help)

obsparam = parser.add_argument_group('REQUIRED Observation conditions')

help = 'Mode of setting the slit width (string) and value for that mode (float).  '
help += 'Valid modes are: %s' % str(slitmodes.keys())
obsparam.add_argument('-slit', '-slitwidth', required=True, nargs='*',  metavar=('MODE','VALUE'), help=help)

help = 'Seeing FWHM (arcsec) defined at zenith and at pivot wavelength (nm)'
obsparam.add_argument('-seeing', type=posfloat, nargs=2, metavar=('SEEING','PIVOT'), required=True, help=help)

help = 'Airmass (dimensionless)'
obsparam.add_argument('-airmass', type=float, required=True, help=help)

help = 'Sky brightness magnitude per arcsec^2 (VEGA, johnson_v)'
obsparam.add_argument('-skymag', type=float, required=True, help=help)

sourceparam_req = parser.add_argument_group('REQUIRED Source parameters')

help = 'Source magnitude (observed at top of atmosphere)'
sourceparam_req.add_argument('-mag', type=float, required=True, help=help)

help = '''Reference system (AB or VEGA) for source magnitude'''
choices=['AB','VEGA','Vega']
sourceparam_req.add_argument('-magsystem', type=str, choices=choices, required=True, help=help)

help = '''Johnson filter (UBVRIJK) to define source magnitude. Use FILTER="match" to normalize to the WRANGE input'''
choices = [c for c in 'UBVRIJK'] + ['user','USER','User'] + ['match','MATCH','Match']
sourceparam_req.add_argument('-magfilter', type=str, choices=choices, required=True, help=help)

sourceparam_add = parser.add_argument_group('Additional source parameters')

help = 'Astronomical source model.  Examples: "constant" (default), "blackbody 5000", "template spiral_001".  \
The "constant" model ignores other parameters in this group.'
sourceparam_add.add_argument('-model', nargs='+', required=False, default=['constant'], help=help)

help = 'Redshift'
sourceparam_add.add_argument('-z', type=nonegfloat, default=0., help=help)

help = 'Selective Extinction E(B-V); default=0'
sourceparam_add.add_argument('-E_BV', type=float, default=0., help=help)

help = 'Extinction model; default="mwavg" (Diffuse Milky Way, R_V=3.1)'
sourceparam_add.add_argument('-extmodel', type=str, default='mwavg', help=help)

help = 'Assume an extended source with constant surface brightness; -mag will be interpreted as mag/arcsec^2; '
help += 'user chooses an integer number of spatial pixels for which to extract signal (same for all slices).'
sourceparam_add.add_argument('-extended', type=posint, default=None, help=help)

# ETC parameter summary for external modules
etc_args = ['channel', 'wrange','exptime'] # Order is important  #, 'SNR' now coded in exptime
etc_kwargs = ['slitwidth', 'airmass', 'skymag','seeing', 'mag', 'magsystem','magfilter']
etc_optargs = ['srcmodel']
etc_optkwargs = ['binspect','binspat'] #, 'fast_SNR', -noslicer ; these take no arguments  ### -binspat ,-binspect

def formETCcommand(row):  ### Maybe make command as list not a big string  ### standardize 1 col per kw; make OTM reformat
	'''Form the ETC command line string from a dict created from astropy table row'''
	cmd = '%s %s %s ' % tuple([row[k] for k in etc_args])  # e.g. U 400 410 SNR 5
	cmd_kwargs = [ '-%s %s'%(k,row[k]) for k in etc_kwargs if not is_masked(row[k]) ]

	# These columns are optional, so first check if they exist
	cols_exist = (set(etc_optargs) & set(row.keys()))
	cmd_optargs = [ row[k] for k in cols_exist if not is_masked(row[k]) ]

	cols_exist = (set(etc_optkwargs) & set(row.keys()))
	cmd_optkwargs = [ '-%s %s'%(k,row[k]) for k in cols_exist if not is_masked(row[k]) ]	

	return cmd + ' '.join(cmd_kwargs+cmd_optkwargs+cmd_optargs)

# Check that inputs are valid and append units where applicable
def check_inputs_add_units(args):

	model = args.model[0]

	# Number of expected arguments after command line option
	choices = {'blackbody':2,'template':2,'constant':1}

	if model.lower() not in choices: parser.error('-model first argument must be in '+str(choices.keys()))

	if len(args.model) != choices[model]: parser.error('-model "%s" requires exactly %i arguments'%(model,choices[model]))

	# Check for valid template if using a template model
	if model.lower()=='template':

		args.srctemp = args.model[1]  # copy template name to new attribute

		# Automatically go looking for the template in the sources directory
		from os import walk
		from os.path import join as joinpath
		foundTemplate = False
		for root, dirs, files in walk(sourcesdir):
			if not foundTemplate:
				for name in files:
					if name == args.srctemp+".fits":
						dir_and_name = '/'.join(joinpath(root, name).split('/')[-2:])
						#print("Found template: "+dir_and_name)
						args.srctemp = joinpath(root, name)
						foundTemplate = True
						continue

		if not foundTemplate: parser.error("Could not find source template: "+args.srctemp+'.fits')

	# Add/check temperature if using blackbody model
	if model.lower()=='blackbody':
		try: args.tempK = posfloat(args.model[1])  # copy blackbody temperature to new attribute
		except: parser.error("-model blackbody TEMPK requires TEMPK to be a positive float")

	# Valid extinction model
	choices = ('lmc30dor', 'lmcavg', 'mwavg', 'mwdense', 'mwrv21', 'mwrv40', 'smcbar', 'xgalsb')
	if args.extmodel not in choices: parser.error('-extmodel must be in '+str(choices))

	## Check for Valid slit mode ##
	# Check mode string
	args.slitmode = args.slit[0].upper() # move the code to new parameter
	if args.slitmode not in slitmodes.keys(): parser.error('-slitwidth MODE must be in '+str(list(slitmodes.keys())))
	# Convert AUTO to SNR
	if args.slitmode == 'AUTO':
		args.slitmode = 'SNR'
		args.slit = slitmodes['AUTO'][0]
	# Check for extra parameter
	elif len(args.slit)>1:
		args.slit = posfloat(args.slit[1]) # this parameter now just a float
	else:
		parser.error('-slitwidth %s needs a parameter X in range %s' % (args.slitmode, str(slitmodes[args.slitmode])))
	# Check parameter range
	if (args.slit > max(slitmodes[args.slitmode])) or (args.slit < min(slitmodes[args.slitmode])):
		parser.error('-slitwidth %s X requires X be in range %s' % (args.slitmode, str(slitmodes[args.slitmode])))
	
	# Append units to inputs where applicable
	import astropy.units as u

	if args.slitmode == 'SET':
		args.slit = slitfloat(args.slit)*u.arcsec  ### checks for valid range -- should simplify

	args.wrange *= u.nm
	if args.ETCmode in ['EXPTIME','SET']:
		args.ETCfixed *= u.s
		args.ETCmode = 'EXPTIME'
	args.seeing[0] *= u.arcsec
	args.seeing[1] *= u.nm
	if hasattr(args, 'tempK'): args.tempK*=u.K

	# Modulate seeing at zenith to seeing at airmass >1
	args.seeing[0] *= args.airmass**0.6

	if args.extended: args.seeing[0] = 100.*u.arcsec  # override observed source "size" with something very big

	# Check wavelength range is (min, max) and within specified channel
	from ETC_config import channelRange
	if args.wrange[0] >= args.wrange[1]: parser.error("Wavelength range must be in form [min, max]")
	if args.wrange[0] < channelRange[args.channel][0]:
		parser.error("Wavelength range %s not in channel %s"%(str(args.wrange),args.channel))
	if args.wrange[1] > channelRange[args.channel][1]:
		parser.error("Wavelength range %s not in channel %s"%(str(args.wrange),args.channel))

