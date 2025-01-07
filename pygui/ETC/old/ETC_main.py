#!/usr/bin/env python
# Author: Chaz Shapiro (2022)

### THIS FILE NOW DEPRECATED; It computes SNR using a more complicated method with covariance matrices ###

# TODOS: K-CORRECTION
#        USER SED, EXTENDED SOURCES (mag/arcsec**2), HOST GALAXY (mag/area)
#        MOON PHASE/POSITION, ZODIACAL LIGHT
#        Unique FILENAMES for plots?
#
# Change -SNR_pix behavior
# Clean up global-ish variables?
# Error handling

# import timer
# tt = timer.Timer()
# tt.start()  # Measures time until tt.stop()

from ETC.ETC_config import *
from ETC.ETC_arguments import *
from ETC.ETC_import import *
from numpy import array, arange, vstack, log

def main(etcargs ,quiet=False):
    from ETC.ETC_config import channels
    args = etcargs

    # Check for valid inputs
    # If running as command line script, exit gracefully from a problem, otherwise raise an exception
    try:
        check_inputs_add_units(args)
    except Exception as e:
        if __name__ == "__main__": parser.error(e)
        else: raise e

    # Unpack SNR or EXPTIME and set the other to None
    if args.ETCmode == 'SNR':
        SNR_target, exptime = (args.ETCfixed, None)
    elif args.ETCmode == 'EXPTIME':
        SNR_target, exptime = (None, args.ETCfixed)
    else: raise Exception('Invalid ETC mode')

    # Only bother with channels being used for SNR;  Saves ~0.3s
    if not args.plotSNR and not args.plotdiag: channels = [args.channel]

    # Some derived parameters; accounts for wavelength binning
    # TODO: SHOULD WE MOVE THESE TO IMPORT FILE?
    binCenters={}
    dispersion_scale={}
    for k in channels:
        lambdamin,lambdamax = channelRange[k]
        dispersion_scale[k]=u.pixel_scale( args.binning[0]*(lambdamax-lambdamin)/(Npix_dispers[k]*u.pix) )
        binCenters[k] = rangeQ(lambdamin,lambdamax, args.binning[0]*(lambdamax-lambdamin)/Npix_dispers[k])

    # Find the bins where the target wavelengths live; won't exactly match input range
    bc = binCenters[args.channel]
    closest_bin_i = [abs(bc-wr).argmin() for wr in args.wrange]
    target_slice = slice(closest_bin_i[0],closest_bin_i[1]+1,None)
    if not quiet:
        print( args.wrange, "-->", bc[closest_bin_i[0]:closest_bin_i[1]+1:closest_bin_i[1]-closest_bin_i[0]] )

    # Only bother with wavelength range used for SNR; negligible time saved
    #if ~args.plotSNR and ~args.plotdiag: binCenters[args.channel] = bc[target_slice]


    # Load source spectrum model
    if args.srcmodel[0].lower() == 'template':
        label = args.srctemp.split('/')[-1]  #used for plot labels
        sourceSpectrum = SourceSpectrum.from_file(args.srctemp)
    elif args.srcmodel[0].lower() == 'blackbody':
        from synphot.models import BlackBodyNorm1D
        label = 'blackbody '+str(args.tempK)
        sourceSpectrum = SourceSpectrum(BlackBodyNorm1D, temperature=args.tempK)
    elif args.srcmodel[0].lower() == 'constant':
        from astropy.modeling.models import Const1D
        label = 'constant'
        sourceSpectrum = SourceSpectrum(Const1D)

    else: raise Exception('Invalid source model')

    # Redshift it
    sourceSpectrum.z = args.z

    # Galactic extinction
    # https://pysynphot.readthedocs.io/en/latest/spectrum.html#pysynphot-extinction
    if args.E_BV != 0:
    	from synphot import ReddeningLaw
    	redlaw = ReddeningLaw.from_extinction_model(args.extmodel)
    	sourceSpectrum *= SpectralElement(redlaw.extinction_curve(args.E_BV))

    # Load bandpass for normalization
    if args.magref[1].lower() == 'user':
    	# Use the wavelength range from command line
    	from synphot.models import Box1D
    	norm_band = SpectralElement(Box1D, amplitude=1, x_0=args.wrange.mean(), 
    		                        width=(args.wrange[1]-args.wrange[0]) )
    else:
    	norm_band = SpectralElement.from_filter('johnson_'+args.magref[1].lower())

    # Normalize source - this is done after all astrophysical adjustments and before the atmosphere
    # So we are fixing the magnitude "at the top of the atmosphere"
    if args.magref[0].upper() == 'AB': magunit=u.ABmag
    elif args.magref[0].upper() == 'VEGA': magunit=uu.VEGAMAG

    if magunit is uu.VEGAMAG:
        sourceSpectrum = sourceSpectrum.normalize(args.mag*magunit ,band=norm_band 
                                                  ,vegaspec=SourceSpectrum.from_vega())
    else:
        sourceSpectrum = sourceSpectrum.normalize(args.mag*magunit ,band=norm_band )
                                                  #,wavelengths=sourceSpectrum.waveset)


    # Load sky background; eventually will need to interpolate a larger table
    # background units dimensions are not same as other flux units
    # File units = u.photon/u.s/u.nm/u.arcsec**2/u.m**2 ~ phot/s/wavelength  VS  nm

    skySpec = SourceSpectrum.from_file(CSVdir+skybackground_file ,wave_unit='nm') #HARDCODED UNIT
    # assumes units = phot/s/cm^2/Angstrom 
    # normalization is wrong but proportional to phot/s/wavelength, same as file

    # Don't need to match mag reference and filter to source; will use whatever data we have
    skyFilter = SpectralElement.from_filter('johnson_v')
    skySpec = skySpec.normalize(args.skymag*uu.VEGAMAG ,band=skyFilter ,vegaspec=SourceSpectrum.from_vega() )

    # new units = VEGAMAG/arcsec^2 since skymag is really mag/arcsec^2

    # background flux/pixel ~ slit_width * spatial_pixel_height; we'll scale sky flux by this later
    bg_pix_area={}
    for ch in channels:
    	bg_pix_area[ch] = args.slit *(1*u.pix).to('arcsec' ,equivalencies=plate_scale[ch])
    	# Make dimensionless in units of arcsec^2
    	bg_pix_area[ch] = (bg_pix_area[ch]/u.arcsec**2).to(u.dimensionless_unscaled).value

    # Load "throughput" for atmosphere
    throughput_atm = Extinction_atm(args.airmass)

    # Load throughputs for all spectrograph channels
    throughput_spectrograph={}
    for k in channels:
        throughput_spectrograph[k] = LoadCSVSpec(throughputFile_spectrograph[k])

    # Load detector QE for all channels
    QE={}
    for k in channels:
        QE[k] = LoadCSVSpec(QEFile[k])
        
    # Load telescope throughput
    throughput_telescope = LoadCSVSpec(throughputFile_telescope)

    # Load throughput for slicer optics (either side of slit)
    throughput_slicerOptics = LoadCSVSpec(throughputFile_slicer)

    TP={}
    for k in channels:
        TP[k] = throughput_spectrograph[k]*QE[k]*throughput_telescope

    # Compute transmission thru slit/slicer sections as function of slit width and seeing(wavelength)
    # Assumes seeing-limited PSF

    # Combine slit fractions arrays with Optics to make throughput elements
    throughput_slicer = slitEfficiency(args.slit ,slit_h ,args.seeing[0] ,pivot=args.seeing[1])
    throughput_slicer['side'] *= throughput_slicerOptics

    # Compute pixelized spatial profiles for a flat spectrum
    # Multiplying spectra by these profiles "distributes" counts over pixels in spatial direction
    # THIS IS ONLY HALF THE (symmetric) PROFILE, so it is normalized to 0.5 #

    profile_slit = {}
    totalSlicer={}        #Total slicer throughput including all slices and optics

    # profile_slit[k][lightpath] sums to 0.5 in each w bin and each path individually
    # we want it to be 0.5 when summed over all paths, with the correct relative path efficiencies
    for k in channels:
        profile_slit[k] = profileOnDetector(k ,args.slit ,args.seeing[0] ,args.seeing[1] ,binCenters[k]
                                            ,spatial_range=None ,bin_spatial=args.binning[1])
        # Shape is (Npix, Nlambda)
        for lightpath in ['center','side']:
            profile_slit[k][lightpath] *= throughput_slicer[lightpath](binCenters[k]).value

        if args.noslicer: renorm = (profile_slit[k]['center']).sum(0)
        else:             renorm = (profile_slit[k]['center'] + 2*profile_slit[k]['side']).sum(0)

        totalSlicer[k] = 2*SpectralElement(Empirical1D, points=binCenters[k], lookup_table=renorm)

        for lightpath in profile_slit[k].keys():
            profile_slit[k][lightpath] *= 0.5/renorm

        # totalSlicer[k] is the total throughput of signal through all slice(s) being used for SNR, so...
        # ... totalSlicer*profile_slit distributes the used light over the used slice half-profiles
        # This separation makes the covariance matrix calculation easier later on


    # Multiply source spectrum by all throughputs, atmosphere, slit loss, and convolve with LSF
    # These are the flux densities at the focal plane array (FPA)
    # Side slice throughputs are for a SINGLE side slice

    sourceSpectrumFPA={}  #Total flux summed over all spatial pixels and used slices
    skySpectrumFPA={}     #Flux PER spatial pixel, depends on slice bc of slicer optics

    for k in channels:
        spec = sourceSpectrum * throughput_atm * TP[k] * totalSlicer[k]
        sourceSpectrumFPA[k] = convolveLSF(spec, args.slit ,args.seeing[0] ,k ,pivot=args.seeing[1])

        # Doesn't include atmosphere or slitloss for sky flux
        # Scale sky flux by effective area: slit_width*pixel_height
        skySpectrumFPA[k]={}    
        for lightpath in ['center','side']:
            spec = skySpec * TP[k] * bg_pix_area[k]
            if lightpath == 'side': spec *= throughput_slicerOptics
            skySpectrumFPA[k][lightpath] = convolveLSF(spec, args.slit ,args.seeing[0] ,k ,pivot=args.seeing[1])

    # Create workhorse function for solving for EXPTIME
    def SNR_from_exptime(exptime, wave_range=None, ch=None ,Np=None):
        '''Compute SNR from inputs'''
        '''
        exptime: exposure time (unitful)
        wave_range: compute average SNR for this wavelength range (unitful), requires ch
        ch: compute SNR only for this channel
        Np: Number of spatial pixels on either side of profile center to estimate SNR;
            If none, assume profile fit to each channel and lightpath
        
        RETURNS:
            wave_range --> single SNR
            ch, no wave_range --> SNR/wavelength bin for whole channel
            no keywords --> dictionary of SNR/wavelength bin for all channels
        '''
        
        # Find the bins where the target wavelengths live
        if wave_range is not None:
            closest_bin_i = [abs(binCenters[ch]-wr).argmin() for wr in wave_range]

        # Only loop over channels we need
        if ch is not None: chanlist = ch
        else: chanlist = channels

        # Convert signal and background to counts (total per wavelength), including noise
        # flux_unit='count' actually makes Spectrum object return counts/second,
        #   so adjust units so we can scale by unitfull exptime

        signal={}  #Total fluence (counts) summed over all spatial pixels
        bgvar={}   #Variance (counts) PER spatial pixel read

        for sigpath in ['center','side']:
            signal[sigpath] = {}
            for k in chanlist:
                signal[sigpath][k] = sourceSpectrumFPA[k](binCenters[k] 
                                        ,flux_unit='count' ,area=telescope_Area)/u.s*exptime

            # TODO: Should background variances be 2x to acount for sky subtraction?
            bgvar[sigpath] = {}
            for k in chanlist:
                bgvar[sigpath][k] = skySpectrumFPA[k][sigpath](binCenters[k] 
                                        ,flux_unit='count' ,area=telescope_Area)/u.s*exptime
                bgvar[sigpath][k] += darkcurrent[k]*exptime*u.pix 

                bgvar[sigpath][k] *= args.binning[1]  #account for more background per pixel if binning

                bgvar[sigpath][k] += (readnoise[k]*u.pix)**2/u.ct  #read noise per read pixel is unchanged
                
        SNR = {}

        # Compute variance for each spatial pixel and wavelength bin; inverse sum over spatial
        for k in chanlist:
            if Np is None:
                cv = profile_slit[k]['center']**2/(profile_slit[k]['center']*signal['center'][k]+bgvar['center'][k])

                # Keep adding pixels to the spatial sum -- normalization was handled above
                if not args.noslicer:
                    cv += 2*profile_slit[k]['side']**2/(profile_slit[k]['side']*signal['side'][k]+bgvar['side'][k])

                cv = 2.*cv.sum(0)  # Spatial sum; doubled since using symmetric half-profile
                cv = 1./cv

            # SNR assuming flux is extracted from profile fit
            if Np is None:
                SIGNAL = signal['center'][k]
                NOISE2 = cv
                SNR[k] = SIGNAL/NOISE2**.5

            # Repeat for peak profile pixels only
            else:
                SIGNAL = (signal['center'][k]*profile_slit[k]['center'][:Np] + 2*signal['side'][k]*profile_slit[k]['side'][:Np]).sum(0)
                NOISE2 = Np*(bgvar['center'][k] + 2*bgvar['side'][k]) + SIGNAL  # SIGNAL here adds shot noise
                SNR[k] = 2*SIGNAL/(2*NOISE2)**.5  # Use both sides of symmetric profile --> x2
                
            # Ensure SNR is dimensionless
            SNR[k] = (SNR[k]/u.ct**.5).to(u.dimensionless_unscaled).value

        if wave_range is not None:        return (SNR[ch][closest_bin_i[0]:closest_bin_i[1]+1]).mean()
        elif ch is not None:              return SNR[ch]
        else:                             return SNR


    # Solve for exptime or SNR depending on what is fixed

    if SNR_target is not None:
        from scipy import optimize

        # Find root in log-log space where SNR(t) is ~linear; doesn't save much time but more likely to converge
        def SNRfunc(t_sec):
            return log( SNR_from_exptime(10**t_sec*u.s, wave_range=args.wrange, ch=args.channel ,Np=None) / SNR_target) #,Np=args.SNR_pix
            # return SNR_from_exptime(t_sec*u.s, wave_range=args.wrange, ch=args.channel ,Np=args.SNR_pix) - SNR_target

        #ans = optimize.root_scalar(SNRfunc ,x0=1 ,x1=100)  ### Very bright stars may not converge here; try log-log
        ans = optimize.root_scalar(SNRfunc ,x0=0 ,x1=3)

        # Check for converged answer
        if ans.converged:
            # t = (ans.root).astype('float16')*u.s
            t = (10.**(ans.root).astype('float16'))*u.s
        else:
            raise RuntimeError('ETC calculation did not converge')

        # print(ans)
        if not quiet:
            print('SNR=%s   exptime=%s'%(SNR_target, t))
        
    else:
        t = exptime
        SNR_t = SNR_from_exptime(t, wave_range=args.wrange, ch=args.channel ,Np=None).astype('float16') #,Np=args.SNR_pix
        if not quiet:
            print('SNR=%s   exptime=%s'%(SNR_t, t))
        
    # END MAIN CALCULATION
    # tt.stop()

    # Plot SNR vs. wavelength if requested
    if args.plotSNR or args.plotdiag:

    	import matplotlib.pyplot as plt
    	#matplotlib.rcParams.update({'font.size': 14})
    	from astropy.visualization import astropy_mpl_style, quantity_support
    	plt.style.use(astropy_mpl_style)
    	quantity_support()

    	SNR1 = SNR_from_exptime(t)

    	fig, ax = plt.subplots(figsize=(15,4))

    	for k in channels:
    	    ax.plot(binCenters[k], SNR1[k] ,color=channelColor[k] ,label=k)

    	plt.ylabel('SNR / wavelength bin')
    	plt.legend()
    	ax.axvspan(args.wrange[0], args.wrange[1], alpha=0.2, color='grey') # shade user range
    	plt.title('EXPTIME = '+str(t))
    	plt.savefig('plotSNR.png')

    if args.plotdiag:

    	signal = {}
    	bgsky = {}
    	for k in channels:
    		signal[k] = sourceSpectrumFPA[k] + 2*sourceSpectrumFPA[k]
    		#signal[k] = sourceSpectrum
    		bgsky[k] = skySpectrumFPA[k]['center'] + 2*skySpectrumFPA[k]['side']

    	# Display the source spectrum
    	plotAllChannels(signal ,lambda_range=args.wrange ,binned=True ,binCenters=binCenters)
    	plt.title('Source counts/s (%s)'%label) # + '\n'+ str(args.mag*magunit) +' '+ args.magref[1])
    	plt.xlim(totalRange)
    	plt.savefig('source.png')

    	# Display the sky spectrum
    	plotAllChannels(bgsky ,lambda_range=args.wrange ,binned=True ,binCenters=binCenters)
    	plt.xlim(totalRange)
    	plt.savefig('sky.png')

    # RETURN EXPTIME
    return t

def runETC(row):
    '''Convert an astropy table row into an ETC command and run the ETC to get an exposure time'''
    cmd = formETCcommand(row)
    x = parser.parse_args(cmd.split())  ### Change this parser.error ? Should be OK since we check ETC cols at the beginning
    t = main(x ,quiet=True)  # Unitful (s)
    assert isinstance(t,u.Quantity), "Got invalid result from ETC"
    return result

def checkETCargs(row):
    '''Convert an astropy table row into an ETC command and check it using the argument parser'''
    cmd = formETCcommand(row)
    #print(cmd)
    x = parser.parse_args(cmd.split())
    check_inputs_add_units(x)
    return True


if __name__ == "__main__":
    etcargs = parser.parse_args()
    main(etcargs ,quiet=False)
