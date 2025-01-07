
#TODO: remove asserts
#TODO: rewite LSF convolution using astropy.convolution?

#import numpy as np
from numpy import array, arange, pi, hstack, vstack
from pickle import load as pload  # Only needed in point source case
from synphot import SourceSpectrum, SpectralElement  # SLOW IMPORT!
from synphot.models import Empirical1D, Gaussian1D, Box1D, ConstFlux1D
import astropy.units as u
import synphot.units as uu  # used in main code, not this file
from functools import cache
from copy import deepcopy
from scipy.signal import convolve, peak_widths

from ETC_config import *

vegaspec = SourceSpectrum.from_vega()

# Setup paths to data that comes with the ETC package
import ETC_config as p
from os import path
ETCdir = path.dirname(p.__file__)
sourcesdir = ETCdir+'/sources/'
if CSVdir is None: CSVdir = ETCdir+'/CSV/'
PSFsum2DFile = ETCdir+'/PSFsum2D.pkl' #pre-tabulated integral of PSF over slit and side slices

# Check config file inputs are valid and make some derived parameters

# Unit equivalence
plate_scale = { k : u.pixel_scale(platescale[k]) for k in channels }

dispersion_scale_nobin={}  # Unit equivalence
for k in channels:
    lambdamin,lambdamax = channelRange[k]
    dispersion_scale_nobin[k]=u.pixel_scale( (lambdamax-lambdamin)/(Npix_dispers[k]*u.pix) )

# Min and max wavelength of all channels together
totalRange = u.Quantity([v for v in channelRange.values()])
totalRange = u.Quantity([totalRange.min(),totalRange.max()])

telescope_Area = (1. - Obscuration**2)*pi*(telescope_D/2.)**2 #Collecting area 


# Function definitions

#@cache  # saves only ms
def makeBinCenters(binspec, chanlist=channels, wrange=None):
    '''Compute the center wavelength of each pixel along the spectral direction, accounting for binning

    binspec: binning in spectral direction (int)
    chanlist: loop over these channels
    wrange: min and max wavelengths (Quantity)

    Returns: dictionary of arrays (Quantity) of bin center wavelengths; keys = chanlist
    '''

    binCenters={}
    for k in chanlist:
        lambdamin,lambdamax = channelRange[k]
        binCenters[k] = rangeQ(lambdamin,lambdamax, binspec*(lambdamax-lambdamin)/Npix_dispers[k])
        # dispersion_scale[k]=u.pixel_scale( args.binspect*(lambdamax-lambdamin)/(Npix_dispers[k]*u.pix) )

        if wrange is not None:
            closest_bin_i = [abs(binCenters[k]-wr).argmin() for wr in wrange]
            binCenters[k] = binCenters[k][closest_bin_i[0]:closest_bin_i[1]+1]

    return binCenters

def rangeQ(q0, q1, dq=None):
    '''Construct an evenly spaced array using unitful Quantities'''
    
    assert isinstance(q0, u.Quantity) and isinstance(q1, u.Quantity), "Inputs must be Quantities"
    
    unit = q0.unit
    if dq is None: dq = 1.*unit
    else: assert isinstance(dq, u.Quantity), "Inputs must be Quantities"

    # Make sure all input units are same dimension
    assert (unit.physical_type == q1.unit.physical_type) and (unit.physical_type == dq.unit.physical_type), \
        "All inputs must have same physical dimensions (different units are OK)"

    v0 = q0.value
    v1 = q1.to_value(unit)    
    dv = dq.to_value(unit)
        
    return arange(v0,v1,dv)*unit


def LoadCSVSpec(filename ,CSVdir=CSVdir):
    '''Helper to load throughput/spectrum element from CSV file'''
    return SpectralElement.from_file(CSVdir+filename ,wave_unit=default_waveunit)

def seeingLambda(w ,FWHM ,pivot=500.*u.nm):
    '''Seeing law scaled to wavelength'''
    assert u.get_physical_type(w) == 'length', "w must have units of length"
    #pivot = 500.*u.nm
    return FWHM*(w/pivot)**0.2

def makeSource(args):
    ''' Load the source model, mix with astrophysics, normalize.
    Returns a spectrum object for the source, normalized at the top of the atmosphere
    '''
    needNorm = True  # Save time by keeping track of whether we really need to normalize

    # Load source spectrum model
    if args.model[0].lower() == 'constant':
        label = 'constant'
        sourceSpectrum = SourceSpectrum(ConstFlux1D ,amplitude=args.mag*u.ABmag) # start with AB mag
        # If spectrum is FLAT and mag is AB, then we are now normalized by definition
        if args.magsystem.upper() == 'AB': needNorm = False  

    elif args.model[0].lower() == 'template':
        label = args.srctemp.split('/')[-1]  #used for plot labels
        sourceSpectrum = SourceSpectrum.from_file(args.srctemp)
    elif args.model[0].lower() == 'blackbody':
        from synphot.models import BlackBodyNorm1D
        label = 'blackbody '+str(args.tempK)
        sourceSpectrum = SourceSpectrum(BlackBodyNorm1D, temperature=args.tempK)

    else: raise Exception('Invalid source model')

    # Redshift it
    sourceSpectrum.z = args.z

    # Galactic extinction
    # https://pysynphot.readthedocs.io/en/latest/spectrum.html#pysynphot-extinction
    if args.E_BV != 0:
        from synphot import ReddeningLaw
        redlaw = ReddeningLaw.from_extinction_model(args.extmodel)
        sourceSpectrum *= SpectralElement(redlaw.extinction_curve(args.E_BV))
        needNorm = True  # Spectrum is no longer flat

    if needNorm:
        # Load bandpass for normalization
        if args.magfilter.upper() in ['USER','MATCH']: # MATCH is preferred, keeping USER as legacy
            # Use the wavelength range from command line
            from synphot.models import Box1D
            norm_band = SpectralElement(Box1D, amplitude=1, x_0=args.wrange.mean(), 
                                        width=(args.wrange[1]-args.wrange[0]) )
        else:
            norm_band = SpectralElement.from_filter('johnson_'+args.magfilter.lower())

        # Normalize source - this is done after all astrophysical adjustments and before the atmosphere
        # So we are fixing the magnitude "at the top of the atmosphere"
        if args.magsystem.upper() == 'AB':
            sourceSpectrum = sourceSpectrum.normalize(args.mag*u.ABmag ,band=norm_band )
                                                      #,wavelengths=sourceSpectrum.waveset)
        elif args.magsystem.upper() == 'VEGA':
            sourceSpectrum = sourceSpectrum.normalize(args.mag*uu.VEGAMAG ,band=norm_band 
                                                      ,vegaspec=vegaspec)

    return sourceSpectrum

def make_empirical(spec, lams):
    '''convert spectrum model to lookup table'''
    return SourceSpectrum(Empirical1D, points=lams, lookup_table=spec(lams), keep_neg=True)

bandpass_atm = LoadCSVSpec(throughputFile_atm)
def Extinction_atm(airmass):
    '''Compute Transmission vs. wavelength modulated by airmass.  Returns bandpass object'''
    bandpass = deepcopy(bandpass_atm)
    bandpass.model.lookup_table = bandpass.model.lookup_table**airmass
    return bandpass

def makeLSFkernel(slit_w ,seeing ,ch ,kernel_upsample=10. ,kernel_range_factor=4. ,pivot=500*u.nm):
    '''Placeholder until we have LSF data'''
    '''
    Approximates seeing for each channel as Gaussian with scale at channel center wavelength
    Final kernel is INSTRUMENT * (SLIT X SEEING)  (*=convolve)
    TODO: vary LSF in dispersion and/or spatial directions
    TODO: Different LSFs for center and side slices

    slit_w: slit width (unitful; angular size on sky)
    seeing: FWHM of seeing profile at pivot (unitful)
    ch: channel name
    kernel_upsample: factor by which to upsample kernel scale
    kernel_range_factor: factor of LSF FWHM by which to extend range of kernel sampling
    pivot: wavelength where seeing FWHM is defined (unitful)

    RETURNS: Kernel (array, unitless) with which to convolve the spectrum
             fwhm (unitful scalar) -- width of LSF kernel ("dlambda" in denominator of R=lambda/dlambda)
             dlambda (unitful scalar) -- kernel sampling step
    '''
    
    assert isinstance(slit_w,u.Quantity), "slit_w needs units"
    
    # Set seeing to that of center wavelength of channel
    ### TODO: let seeing vary across channel?
    midlam = channelRange[ch].mean()
    sigma_seeing = seeingLambda(midlam ,seeing ,pivot=pivot)/2.35

    # Convert seeing and slit scales to wavelength
    # LSFsigma is already in wavelength units
    sigma_seeing_lam = sigma_seeing.to('pix', equivalencies=plate_scale[ch]) \
                                    .to(u.AA ,equivalencies=dispersion_scale_nobin[ch])
    slit_w_lam = slit_w.to('pix', equivalencies=plate_scale[ch]) \
                                    .to(u.AA ,equivalencies=dispersion_scale_nobin[ch])

    # Sample kernel out to slit width plus extra for instrument PSF
    # Not ideal if slit width >> seeing but should work
    kernel_range = slit_w_lam/2 + LSFsigma[ch]*kernel_range_factor

    # MIN of seeing and slit sets the slit scale; MAX of slit and instument sets total scale
    dlambda = max(min(sigma_seeing_lam, slit_w_lam/2.) ,LSFsigma[ch])/kernel_upsample

    # Make wavelength array for sampling kernel
    ## KERNEL RANGE MUST HAVE 0 IN THE EXACT CENTER (need odd array length)
    xk = rangeQ(0.*dlambda,kernel_range+dlambda,dlambda)
    xk = hstack((-xk[::-1],xk[1:]))  # Symmetrizes range, e.g. [-2,-1,0,1,2]

    # Unitless arrays for use with convolve; BEWARE of boundary behavior
    # convolve(mode=same) matches size of 1st argument
    ### Need to offset the tophat for side slices
    slitLSF = Gaussian1D(stddev=sigma_seeing_lam)*Box1D(width=slit_w_lam) # Multiply seeing profile with slit "tophat"
    instLSF = Gaussian1D(stddev=LSFsigma[ch])
    kernel1 = slitLSF(xk).value
    kernel2 = instLSF(xk).value

    # Total kernel
    kernel = convolve(kernel1, kernel2, mode='same', method='auto')

    # Width of final kernel 
    fwhm=peak_widths(kernel, [int((kernel.shape[0]+1)/2)-1] )[0] * dlambda
    fwhm=fwhm[0]
    # R = (midlam/fwhm).to(1).value

    return kernel, fwhm, dlambda

def makeLSFkernel_slicer(slit_w ,seeing ,ch ,kernel_upsample=10. ,kernel_range_factor=4. ,pivot=500*u.nm, centeronly=False):
    '''Placeholder until we have LSF data'''
    '''
    Approximates seeing for each channel as Gaussian with scale at channel center wavelength
    Final kernel is INSTRUMENT * (SLIT X SEEING)  (*=convolve)
    TODO: vary LSF in dispersion and/or spatial directions
    TODO: Different LSFs for center and side slices

    slit_w: slit width (unitful; angular size on sky)
    seeing: FWHM of seeing profile at pivot (unitful)
    ch: channel name
    kernel_upsample: factor by which to upsample kernel scale
    kernel_range_factor: factor of LSF FWHM by which to extend range of kernel sampling
    pivot: wavelength where seeing FWHM is defined (unitful)
    centeronly: If true, only compute LSF for the center slit, not slices

    RETURNS: Kernel (array, unitless) with which to convolve the spectrum
             fwhm (unitful scalar) -- width of LSF kernel ("dlambda" in denominator of R=lambda/dlambda)
             dlambda (unitful scalar) -- kernel sampling step
    '''
    
    assert isinstance(slit_w,u.Quantity), "slit_w needs units"
    
    # Set seeing to that of center wavelength of channel
    ### TODO: let seeing vary across channel?
    midlam = channelRange[ch].mean()
    sigma_seeing = seeingLambda(midlam ,seeing ,pivot=pivot)/2.35

    # Convert seeing and slit scales to wavelength
    # LSFsigma is already in wavelength units
    sigma_seeing_lam = sigma_seeing.to('pix', equivalencies=plate_scale[ch]) \
                                    .to(u.AA ,equivalencies=dispersion_scale_nobin[ch])
    slit_w_lam = slit_w.to('pix', equivalencies=plate_scale[ch]) \
                                    .to(u.AA ,equivalencies=dispersion_scale_nobin[ch])

    # Sample kernel out to slit width plus extra for instrument PSF
    # Not ideal if slit width >> seeing but should work
    kernel_range = slit_w_lam/2 + LSFsigma[ch]*kernel_range_factor

    # MIN of seeing and slit sets the slit scale; MAX of slit and instument sets total scale
    dlambda = max(min(sigma_seeing_lam, slit_w_lam/2.) ,LSFsigma[ch])/kernel_upsample

    # Make wavelength array for sampling kernel
    ## KERNEL RANGE MUST HAVE 0 IN THE EXACT CENTER (need odd array length)
    xk = rangeQ(0.*dlambda,kernel_range+dlambda,dlambda)
    xk = hstack((-xk[::-1],xk[1:]))  # Symmetrizes range, e.g. [-2,-1,0,1,2]

    # Unitless arrays for use with convolve; BEWARE of boundary behavior
    # convolve(mode=same) matches size of 1st argument

    LSF = {}
    w_offsets = {'center':0.}
    if not centeronly: w_offsets['side'] = slit_w_lam

    for s, w_offset in w_offsets.items(): # slit offset=0, slice offset = slid width

        # -w_offset in slitLSF gives the shape for the slice on the "righthand" side, i.e. decreasing to the right/higher lambda

        slitLSF = Gaussian1D(mean=-w_offset, stddev=sigma_seeing_lam)*Box1D(width=slit_w_lam) # Multiply seeing profile with slit "tophat"
        instLSF = Gaussian1D(stddev=LSFsigma[ch])
        kernel1 = slitLSF(xk).value
        kernel2 = instLSF(xk).value

        # Total kernel
        _kernel = convolve(kernel1, kernel2, mode='same', method='auto')
        # kernel.append(_kernel)

        # Width of final kernel 
        _fwhm=peak_widths(_kernel, [_kernel.argmax()] )[0] * dlambda
        # fwhm.append( _fwhm[0] )

        LSF[s] = {
            'kernel' : _kernel,
            'fwhm' : _fwhm[0],
            'dlambda' : dlambda,
            'xk' : xk
        }

    # fwhm = [x.value for x in fwhm]*fwhm[0].unit

    if not centeronly:
        LSF['side2'] = LSF['side']
        LSF['side2']['kernel'] = LSF['side2']['kernel'][::-1]  ### Reverse direction for the opposite slice

    return LSF

def convolveLSF_old(spectrum, slit_w ,seeing ,ch ,kernel_upsample=10. ,kernel_range_factor=4. ,pivot=500*u.nm ,wrange_=None):
    '''Convolve spectrum at focal plane with LSF'''
    '''
    Approximates seeing for each channel as Gaussian with scale at channel center wavelength
    Final kernel is INSTRUMENT * (SLIT X SEEING)  (*=convolve)
    TODO: vary LSF in dispersion and/or spatial directions

    spectrum: synphot Spectrum object
    slit_w: slit width (angular size on sky)
    seeing: FWHM of seeing profile at pivot
    ch: channel name
    kernel_upsample: factor by which to upsample kernel scale
    kernel_range_factor: factor by which to extend range of kernel sampling
    pivot: wavelength where seeing FWHM is defined
    wrange_: min and max wavelength range (Quantity) over which to convolve spectra

    RETURNS: Spectrum object (Emprical1D) after LSF convolution
    '''
    
    assert isinstance(slit_w,u.Quantity), "slit_w needs units"
    assert isinstance(spectrum ,(SourceSpectrum,SpectralElement)), \
        "Input spectrum must be SourceSpectrum or SpectralElement class"

    # Make the convolution kernel
    kernel, fwhm , dlambda = makeLSFkernel(slit_w ,seeing ,ch ,kernel_upsample ,kernel_range_factor ,pivot)

    # Make wavelength array for sampling spectrum; same spacing, larger range
    if wrange_ is None: wrange_ = channelRange[ch]
    x = rangeQ(wrange_[0]-kernel_range_factor*fwhm ,wrange_[1]+kernel_range_factor*fwhm ,dlambda)

    # Convolved spectrum as unitless array
    spec2 = convolve(spectrum(x).value, kernel, mode='same', method='auto')/kernel.sum()

    # Convert array to spectrum class; Model will be Empirical1D even if input was compound model
    if isinstance(spectrum ,SourceSpectrum):
        newspec = SourceSpectrum(Empirical1D, points=x, lookup_table=spec2*spectrum(1).unit, keep_neg=True)
    elif isinstance(spectrum ,SpectralElement):
        newspec = SpectralElement(Empirical1D, points=x, lookup_table=spec2, keep_neg=True)
    else:
        raise Exception("Unsupported input spectrum class")
        
    return newspec

def convolveLSF(LSF, spectrum ,ch ,kernel_range_factor=4. ,wrange_=None):
    '''Convolve spectrum at focal plane with LSF'''
    '''
    Approximates seeing for each channel as Gaussian with scale at channel center wavelength
    Final kernel is INSTRUMENT * (SLIT X SEEING)  (*=convolve)
    TODO: vary LSF in dispersion and/or spatial directions

    spectrum: synphot Spectrum object
    ch: channel name
    kernel_range_factor: factor by which to extend range of kernel sampling
    wrange_: min and max wavelength range (Quantity) over which to convolve spectra

    RETURNS: Spectrum object (Emprical1D) after LSF convolution
    '''
    
    assert isinstance(spectrum ,(SourceSpectrum,SpectralElement)), \
        "Input spectrum must be SourceSpectrum or SpectralElement class"

    # Make the convolution kernel
    # kernel, fwhm , dlambda = makeLSFkernel(slit_w ,seeing ,ch ,kernel_upsample ,kernel_range_factor ,pivot)
    kernel = LSF['kernel']
    fwhm = LSF['fwhm']
    dlambda = LSF['dlambda']

    # Make wavelength array for sampling spectrum; same spacing, larger range
    if wrange_ is None: wrange_ = channelRange[ch]
    x = rangeQ(wrange_[0]-kernel_range_factor*fwhm ,wrange_[1]+kernel_range_factor*fwhm ,dlambda)

    # Convolved spectrum as unitless array
    spec2 = convolve(spectrum(x).value, kernel, mode='same', method='auto')/kernel.sum()

    # Convert array to spectrum class; Model will be Empirical1D even if input was compound model
    if isinstance(spectrum ,SourceSpectrum):
        newspec = SourceSpectrum(Empirical1D, points=x, lookup_table=spec2*spectrum(1).unit, keep_neg=True)
    elif isinstance(spectrum ,SpectralElement):
        newspec = SpectralElement(Empirical1D, points=x, lookup_table=spec2, keep_neg=True)
    else:
        raise Exception("Unsupported input spectrum class")
        
    return newspec
def Moffat_scalefree(x,y ,beta=moffat_beta):
    '''Moffat PSF profile; x and y are dimensionless; not normalized here - we do that after tabulating'''
    return (1.+x**2+y**2)**(-beta)

# TABULATED IN PSF-profile-scratch.ipynb
PSFsum2D = pload(open(PSFsum2DFile ,'rb'))
from scipy.interpolate import dfitpack

from scipy.special import gamma
def sharpess_scalefree(theta, x=0., beta=moffat_beta):
    ''' 1D sharpness of seeing-limited PSF (Moffat) along spatial direction
    x = horizontal offset; changes y projection for e.g. side image slice
    '''

    # Normalization of PSF
    integral1 = 3.14159**.5 * theta**(2*beta) * (theta**2+x**2)**(0.5-beta) * gamma(beta-0.5) / gamma(beta)

    # Integral over PSF squared (sharpness)
    integral2 = 3.14159**.5 * theta**(4*beta) * (theta**2+x**2)**(0.5-2*beta) * gamma(2*beta-0.5) / gamma(2*beta)

    return integral2/integral1**2

def evaluate2Dinterp(f, x, y):
    '''Trick for quickly evaluating 2D interpolation on (x,y) pairs, not a 2D grid'''
    # https://stackoverflow.com/questions/47087109/evaluate-the-output-from-scipy-2d-interpolation-along-a-curve
    return dfitpack.bispeu(f.tck[0], f.tck[1], f.tck[2], f.tck[3], f.tck[4], x, y)[0]

def slitFractions(lam, w ,h ,FWHM ,pivot=500.*u.nm):
    '''Compute fraction of PSF passing through slit and side slices. Assumes Moffat PSF'''
    
    ts = moffat_theta_factor * seeingLambda(lam ,FWHM ,pivot=pivot)  #specific to Moffat PSF
    #if ts.isscalar: ts=[ts]

    wts=(w/ts).value
    hts=(h/ts).value

    if hasattr(hts, '__iter__'):
        wts=array([wts]*len(hts))  # copy into a list if h is a list
        
    centerFrac = evaluate2Dinterp(PSFsum2D, wts/2., hts/2.)
    totalFrac = evaluate2Dinterp(PSFsum2D, 3.*wts/2., hts/2.)

    sideFrac = (totalFrac-centerFrac)/2.
    return {'total':totalFrac, 'center':centerFrac, 'side':sideFrac}

def slitEfficiency(w ,h ,FWHM ,pivot=500.*u.nm ,optics=None):
    '''Compute fraction of PSF passing through slit and side slices, assuming Moffat PSF'''
    '''
    w: slit width (unitful; angular projection on sky)
    h: slit length (height) (unitful; angular projection on sky)
    FWHM: seeing at pivot (unitful)
    optics: Bandpass object for slicer side optics

    RETURNS: Dictionary of bandpass objects for 'center', 'side', and 'total'
    '''

    # Slow function of wavelength so choose 10nm sampling
    lams = rangeQ(totalRange[0],totalRange[1],10*u.nm)

    ts = moffat_theta_factor * seeingLambda(lams ,FWHM ,pivot=pivot)  #specific to Moffat PSF
    #if ts.isscalar: ts=[ts]
        
    centerFrac = evaluate2Dinterp(PSFsum2D, w/ts/2., h/ts/2.)
    totalFrac = evaluate2Dinterp(PSFsum2D, 3.*w/ts/2., h/ts/2.)

    sideFrac = (totalFrac-centerFrac)/2.

    if optics is not None:
        sideFrac *= optics(lams).value  # Bandpass object is dimensionless Quantity
        totalFrac = centerFrac + sideFrac*2

    throughput_slicer = {}
    throughput_slicer['center'] = SpectralElement(Empirical1D, points=lams, lookup_table=centerFrac)
    throughput_slicer['side'] = SpectralElement(Empirical1D, points=lams, lookup_table=sideFrac)
    throughput_slicer['total'] = SpectralElement(Empirical1D, points=lams, lookup_table=totalFrac)

    return throughput_slicer

def profileOnDetector(channel ,slit_w ,seeing ,pivot ,lams ,spatial_range=None ,bin_spatial=1):
    ''' USE TABULATED MOFFAT INTEGRAL TO BACK OUT PIXELIZED SPATIAL PROFILES '''
    '''

    channel: single spectrograph channel to compute for
    slit_w:  width of slit in arcsec
    seeing:  FWHM of seeing profile at wavelength pivot (unitful)
    pivot:  pivot wavelength at which seeing is defined (unitful)
    lams: wavelengths over which to sample the spatial profile (Quantity)
    spatial_range:  how far to compute profile in the spatial direction (unitful; angular projection)
    bin_spatial:  step size (number of spatial pixels) to sample spatial profile; simulates CCD binning

    returns: profile_slit{'center' , side' } - for each key, a numpy array of the pixelized slit profile with shape (Npix,Nlambda)
             Npix is the number of pixels in the spatial direction counting half the profile from the center out
             Output is normalized to total flux entering slit section for each lambda;
             spatial direction should sum to 0.5 since we return only 1/2 the symmetric profile
    '''

    # Profile is a slow function of wavelength

    if spatial_range is None: spatial_range = 3*seeing

    # Step through integer pixel widths from 0 (profile center) to some max spatial height
    assert bin_spatial - int(bin_spatial) == 0, "binning must be an integer"
    dx = (1*u.pix).to(u.arcsec ,equivalencies=plate_scale[channel]) * bin_spatial
    x=rangeQ(0*u.arcsec ,spatial_range ,dx)

    # For each wavelength, integrate over the slit width and from origin to each spatial pixel boundary
    # slitFractions() halves w and h in its integrals so double the pixel height argument
    # This returns Npix+1 dicts containing arrays of length Nlambda
    # PSFsums=[slitFractions(lams, slit_w, 2*xi ,seeing) for xi in x]  #SLOW?
    PSFsums=slitFractions(lams, slit_w, 2*x ,seeing)

    # Normalize to sum over full slit
    # This sums only from 0 to slit height, so double it below to get full slit value
    profile_norm=slitFractions(lams, slit_w, slit_h ,seeing)  #shape is (Nlambda)

    # Subtract sums at adjacent pixel boundaries to get sum in each pixel
    profile_slit={}
    for k in ['center','side']:
        profile_slit[k]=(PSFsums[k][1:]-PSFsums[k][:-1])/2/profile_norm[k]
        # NB doubled normalization cf. above note

    # shapes are (Npix, Nlambda)

    return profile_slit

def applySlit(slitw, source_at_slit, sky_at_slit, throughput_slicerOptics, args ,chanlist ,kernel_range_factor=4.):
    '''Convert source and sky spectra at slit entrance to spectra at focal plane.
    Applies throughput of slit and slicer, convolves with LSF, computes sharpness parameters
    1/sharpness is effective spatial extent of profile in (possibly binned) pixels 
    Assumes seeing-limited PSF

    INPUTS
    slitw:  slit width (unitful)
    source_at_slit: dict of source spectra for each channel, including effects of atm and all throughputs except slit/slicer
    sky_at_slit:  dict of sky spectra for each channel, including all throughputs except slit/slicer
    throughput_slicerOptics: throughput of slicer optics (bandpass object)
    args:  argparse object with all the command line arguments

    RETURNS
    sourceSpectrumFPA[k][s]: dict of dict of spectra for channel k and slice s
    skySpectrumFPA[k][s]: dict of dict of spectra for channel k and slice s
    sharpness: dict of dict of arrays for channel k and slice s
    '''

    POINTSOURCE = (args.extended is None)

    # if not POINTSOURCE:
    #     return applySlit_extended(slitw, source_at_slit, sky_at_slit, throughput_slicerOptics, args ,chanlist)

    binCenters = args.binCenters_
    if args.noslicer or args.fastSNR:   slicer_paths = ['center']
    else:                               slicer_paths = ['center','side']

    if POINTSOURCE:
        # Combine slit fractions arrays with Optics to make throughput elements
        throughput_slicer = slitEfficiency(slitw ,slit_h ,args.seeing[0] ,pivot=args.seeing[1] ,optics=throughput_slicerOptics)

        # Compute pixelized spatial profiles for a flat spectrum
        # Multiplying spectra by these profiles "distributes" counts over pixels in spatial direction
        # THIS IS ONLY HALF THE (symmetric) PROFILE, so it is normalized to 0.5
        # profile_slit[k][lightpath] sums to 0.5 in each w bin and each path individually
        # profile_slit[k][lightpath] shape is (Nspatial, Nspectral)

        profile_slit = { k: profileOnDetector(k ,slitw ,args.seeing[0] ,args.seeing[1] ,binCenters[k].mean() ### If not using mean, must change slitFractions() to allow lambda arrays
                                                ,spatial_range=None ,bin_spatial=args.binspat)
                        for k in chanlist }
        Npix_spatial = None

    else:
        Npix_spatial = args.extended

    if args.fastSNR: Npix_spatial = 2  # overrides extended source size

    sharpness = { k : { s: 
        1./array([Npix_spatial]*len(binCenters[k])) if Npix_spatial is not None # 1/sharpness = [N, N, N...]
        else 2*(profile_slit[k][s]**2).sum(0)
        for s in slicer_paths }  for k in chanlist }

    # Multiply source spectrum by all throughputs, atmosphere, slit loss, and convolve with LSF
    # These are the flux densities at the focal plane array (FPA)
    # Side slice throughputs are for a SINGLE side slice

    sourceSpectrumFPA={}  #Total flux summed over all spatial pixels and used slices  ### true for extended?
    skySpectrumFPA={}     #Flux PER spatial pixel, depends on slice bc of slicer optics

    # background flux/pixel ~ slit_width * spatial_pixel_height; we'll scale sky flux by this later
    for k in chanlist:
        # area of sky projected onto 1 pixel in units of arcsec^2
        bg_pix_area = slitw * (1*u.pix).to('arcsec' ,equivalencies=plate_scale[k])
        bg_pix_area = bg_pix_area.to('arcsec2').value

        sourceSpectrumFPA[k]={}
        skySpectrumFPA[k]={}

        if args.hires: 
            # Make the convolution kernel for each slice
            LSF = makeLSFkernel_slicer(slitw ,args.seeing[0] ,k ,kernel_range_factor=kernel_range_factor ,pivot=args.seeing[1])

        for s in slicer_paths:
            if POINTSOURCE:
                spec = source_at_slit[k] * throughput_slicer[s]  # This applies slit loss and optics

                # scale signal down to 2 center pixels
                if args.fastSNR:
                    spec *= SpectralElement(Empirical1D, points=binCenters[k], lookup_table=2*profile_slit[k][s][0])

            else:
                # extended source is normalized to mag/arcsec^2, so multiplying by arcsec^2/px gives signal in 1 pixel
                spec = source_at_slit[k] * bg_pix_area * Npix_spatial * args.binspat  # signal per pixel * N_pixels
                if s == 'side': spec *= throughput_slicerOptics

            if args.hires: 
                # specold = convolveLSF_old(spec, slitw ,args.seeing[0] ,k ,pivot=args.seeing[1] ,wrange_=args.wrange_)
                spec = convolveLSF(LSF[s], spec ,k ,kernel_range_factor=kernel_range_factor ,wrange_=args.wrange_)

            sourceSpectrumFPA[k][s] = spec 

        for s in slicer_paths:
            # Doesn't include atmosphere or slitloss for sky flux
            # Scale sky flux by effective area: slit_width*pixel_height
            spec = sky_at_slit[k] * bg_pix_area
            if s == 'side': spec *= throughput_slicerOptics
            if args.hires: 
                # specold = convolveLSF_old(spec, slitw ,args.seeing[0] ,k ,pivot=args.seeing[1] ,wrange_=args.wrange_)
                spec = convolveLSF(LSF[s], spec ,k ,kernel_range_factor=kernel_range_factor ,wrange_=args.wrange_)

            skySpectrumFPA[k][s] = spec

    return sourceSpectrumFPA, skySpectrumFPA, sharpness

def applySlit_extended(slitw, source_at_slit, sky_at_slit, throughput_slicerOptics, args ,chanlist ,kernel_range_factor=4.):
    '''same as applySlit for extended source'''

    binCenters = args.binCenters_
    if args.noslicer or args.fastSNR:   slicer_paths = ['center']
    else:                               slicer_paths = ['center','side']

    if args.fastSNR: Npix_spatial = 2
    elif args.extended != None: Npix_spatial = args.extended
    else: Npix_spatial = None

    sharpness = { k : { s: 
        1./array([Npix_spatial]*len(binCenters[k]))  #1/sharpness = [N, N, N...]
        for s in slicer_paths }  for k in chanlist }

    # Multiply source spectrum by all throughputs, atmosphere, slit loss, and convolve with LSF
    # These are the flux densities at the focal plane array (FPA)
    # Side slice throughputs are for a SINGLE side slice

    sourceSpectrumFPA={}  #Total flux summed over all spatial pixels and used slices
    skySpectrumFPA={}     #Flux PER spatial pixel, depends on slice bc of slicer optics

    # background flux/pixel ~ slit_width * spatial_pixel_height; we'll scale sky flux by this later
    for k in chanlist:
        # area of sky projected onto 1 pixel in units of arcsec^2
        bg_pix_area = slitw * (1*u.pix).to('arcsec' ,equivalencies=plate_scale[k])
        bg_pix_area = bg_pix_area.to('arcsec2').value

        sourceSpectrumFPA[k]={}
        skySpectrumFPA[k]={}

        if args.hires: 
            # Make the convolution kernel for each slice
            LSF = makeLSFkernel_slicer(slitw ,args.seeing[0] ,k ,kernel_range_factor=kernel_range_factor ,pivot=args.seeing[1])

        for s in slicer_paths:
            spec = source_at_slit[k] * bg_pix_area * Npix_spatial  # signal per pixel * N_pixels
            if s == 'side': spec *= throughput_slicerOptics
            if args.hires: 
                # specold = convolveLSF_old(spec, slitw ,args.seeing[0] ,k ,pivot=args.seeing[1] ,wrange_=args.wrange_)
                spec = convolveLSF(LSF[s], spec ,k ,kernel_range_factor=kernel_range_factor ,wrange_=args.wrange_)

            sourceSpectrumFPA[k][s] = spec 

            # Doesn't include atmosphere or slitloss for sky flux
            # Scale sky flux by effective area: slit_width*pixel_height
            spec = sky_at_slit[k] * bg_pix_area
            if s == 'side': spec *= throughput_slicerOptics
            if args.hires: 
                # specold = convolveLSF_old(spec, slitw ,args.seeing[0] ,k ,pivot=args.seeing[1] ,wrange_=args.wrange_)
                spec = convolveLSF(LSF[s], spec ,k ,kernel_range_factor=kernel_range_factor ,wrange_=args.wrange_)

            skySpectrumFPA[k][s] = spec

    return sourceSpectrumFPA, skySpectrumFPA, sharpness

def computeSNR(exptime, slitw, args, SSSfocalplane, allChans=False):
    '''Compute SNR from inputs'''
    '''
    exptime: exposure time (unitful)
    slitw: slit width (unitful)
    args: argparse object containing user inputs
    SSSfocalplane: function that returns source spectra, sky spectra, and sharpness at the FPA
    allChans: boolean; if True, loop over all channels; otherwise, compute only for target SNR channel
    
    RETURNS:
        allChans --> dictionary of SNR/wavelength bin for all channels
        not allChans --> single SNR value; average over target SNR range
    '''

    # Only loop over channels we need
    chanlist = tuple(args.binCenters_.keys())

    if args.noslicer or args.fastSNR:   slicer_paths = ['center']
    else:                               slicer_paths = ['center','side']

    signal, bgvar, sharpness = SSSfocalplane(slitw)  #Cached

    SNR2={}
    SNR={}

    for k in chanlist:
        SNR2[k] = {}

        for s in slicer_paths:
            SIGNAL = signal[k][s]*exptime 

            #account for more background per pixel if binning
            NOISE2 = (bgvar[k][s]*exptime + darkcurrent[k]*exptime*u.pix) * args.binspat
            NOISE2 += (readnoise[k]*u.pix)**2/u.ct  #read noise per read pixel is unchanged                                
            NOISE2 = SIGNAL + NOISE2/sharpness[k][s]  #add shot noise

            SNR2[k][s] = (SIGNAL**2/NOISE2).to('ct').value

        if args.noslicer or args.fastSNR: SNR[k] = SNR2[k]['center']**0.5
        else: SNR[k] = (SNR2[k]['center']+2*SNR2[k]['side'])**0.5  # SNR combines in quadrature

    # Return SNR data for each channel
    if allChans: return SNR

    # Return 1 number - the SNR caclulated from user input
    else: return SNR[args.channel].mean()

def plotAllChannels(spec ,lambda_range=None ,binned=False ,spec_allchan=None ,binCenters=None):
    '''
    spec: dictionary with a spectrum for each channel
    lambda_range: shade a wavelength range of interest
    binned:  if true, calculate flux in counts; otherwise use flux density
    spec_allchan: multiply spectra in all channels by this spectrum (bandpass)
    x: wavelength values to use for plot (good to use binCenters)
    '''

    import matplotlib.pyplot as plt

    fig, ax = plt.subplots(figsize=(12,4))
    for k in spec.keys():
        if binCenters: x = binCenters[k]
        else: x = spec[k].waveset

        y = spec[k]
        if spec_allchan is not None: y *= spec_allchan

        if binned:
            y = y(x ,flux_unit='count', area=telescope_Area)
            ax.plot(x ,y ,drawstyle='steps-mid', label=k ,color=channelColor[k])
        else:
            y = y(x)
            ax.plot(x, y ,label=k ,color=channelColor[k])

    ax.set_ylabel('Flux (%s)' % y[0].unit ) #evaluate the spectrum once to get units
    ax.set_xlim(totalRange)
    ax.legend()    
    
    if lambda_range is not None:
        ax.axvspan(lambda_range[0], lambda_range[1], alpha=0.2, color='grey') # shade user range
