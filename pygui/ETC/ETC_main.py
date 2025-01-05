#!/usr/bin/python3
# Author: Chaz Shapiro (2022)
#
# Notes on SNR estimation: https://www.stsci.edu/instruments/wfpc2/Wfpc2_hand/HTML/W2_61.html

# TODOS: 
### HOST GALAXY (mag/area)
### USER SED
### K-CORRECTION
# Clean up global-ish variables?

from ETC_config import *
from ETC_arguments import *
from ETC_import import *
from numpy import array, arange, vstack, log, where
from scipy import optimize

''' LOAD DATA that doesn't depend on user input '''

# Load sky default background; used if rubin_sim spectrum is not provided
# background units dimensions are not same as other flux units
# File units = u.photon/u.s/u.nm/u.arcsec**2/u.m**2 ~ phot/s/wavelength  VS  nm
skySpec0 = SourceSpectrum.from_file(CSVdir+skybackground_file ,wave_unit='nm') #HARDCODED UNIT
# assumes units = phot/s/cm^2/Angstrom 

skySpec0 = skySpec0*1.575e-17 # * 10.**((21.4-args.skymag)/2.5) ### HARDCODE NORMALIZED TO VEGA mag 21.4 JOHNSON V
skymag0 = 21.4

# Don't need to match mag reference and filter to source; will use whatever data we have
skyFilter = SpectralElement.from_filter('johnson_v')

# Load telescope throughput
throughput_telescope = LoadCSVSpec(throughputFile_telescope)

# Load throughputs and detector QE for all spectrograph channels
throughput_spectrograph = { k : LoadCSVSpec(throughputFile_spectrograph[k]) for k in channels }
QE = { k : LoadCSVSpec(QEFile[k]) for k in channels }

# Combine spectra with all throughputs except for slit/slicer
TP = { k : throughput_spectrograph[k]*QE[k]*throughput_telescope for k in channels }

# Load throughput for slicer optics (either side of slit)
throughput_slicerOptics = LoadCSVSpec(throughputFile_slicer)

# tt.stop('Setup')

''' MAIN FUNCTION RETURNS DICT OF RESULTS '''
def main(args ,quiet=False ,ETCextras=False ,plotSNR=False ,plotslit=False, skyspec=None):
    '''
    skyspec = 2xN array tabulating a sky background spectrum; units = (nm , ergs/s/cm^2/Å )
              Generated in OTM using SkyModel.return_wave_spec() from rubin_sim package
              This currently overrides args.skymag which is still required by argparse. 
    '''

    from ETC_config import channels

    # Only bother with channels being used for SNR to save time
    if plotSNR or plotslit:
        args.wrange_ = None
    else:
        channels = [args.channel]
        args.wrange_ = args.wrange

    binCenters = makeBinCenters(args.binspect ,chanlist=channels ,wrange=args.wrange_)
    args.binCenters_ = binCenters

    # # Print the bins where the target wavelengths live; won't exactly match input range
    # if not quiet:
    #     print( args.wrange, "-->", binCenters[args.channel][[0,-1]].round(3) )

    if args.noslicer or args.fastSNR:   slicer_paths = ['center']
    else:                               slicer_paths = ['center','side']

    # Load source spectrum model and normalize
    sourceSpectrum = makeSource(args)

    # Use provided sky spectrum; if not provided use and normalize template
    if skyspec: 
        skySpec = SourceSpectrum(Empirical1D, keep_neg=True, 
                                  points=skyspec[0]*u.nm, lookup_table=skyspec[1][0]*uu.FLAM) # (nm , ergs/s/cm^2/Å )

    else:
    # Normalize the sky; normalization is wrong but proportional to phot/s/wavelength, same as file
    # skySpec = skySpec0.normalize(args.skymag*uu.VEGAMAG ,band=skyFilter ,vegaspec=vegaspec ) 
        skySpec = skySpec0 * 10.**((skymag0-args.skymag)/2.5) ###
    # new units = VEGAMAG/arcsec^2 since skymag is really mag/arcsec^2

    # Load "throughput" for atmosphere
    throughput_atm = Extinction_atm(args.airmass)

    # Source is modulated by atm, sky is not
    if args.hires:
        sourceSpec_wTP = { k : sourceSpectrum * throughput_atm * TP[k] for k in channels }
        skySpec_wTP = { k : skySpec * TP[k] for k in channels }
    else:
        # Replace analytic models with lookup tables  ### just as fast to make tables hi-res??
        sourceSpec_wTP = { k : make_empirical(sourceSpectrum*throughput_atm*TP[k], binCenters[k]) for k in channels }
        skySpec_wTP = { k : make_empirical(skySpec*TP[k], binCenters[k]) for k in channels }


    '''  ALL SLIT DEPENDENCE BELOW THIS LINE '''

    # Setup the slicer function; cache saves time in loops where slit width doesn't change
    @cache
    def SSSfocalplane(w):

        sourceSpectrumFPA, skySpectrumFPA, sharpness = \
            applySlit(w, sourceSpec_wTP, skySpec_wTP, throughput_slicerOptics, args ,channels)

        # Convert signal and background to counts (total per wavelength), including noise
        # flux_unit='count' actually makes Spectrum object return counts/second,
        #   so adjust units so we can scale by unitfull exptime

        if args.hires:
            # Integrate spectrum over each bin
            signal = {k: {s: sourceSpectrumFPA[k][s](binCenters[k] ,flux_unit='count' ,area=telescope_Area)/u.s
                           for s in slicer_paths} for k in channels }

            bgvar = {k: {s: skySpectrumFPA[k][s](binCenters[k] ,flux_unit='count' ,area=telescope_Area)/u.s
                           for s in slicer_paths} for k in channels }

        else:
            # Slightly faster approximation for smooth functions; assume spectrum is constant across bins
            # binspect is the width of the bin in pixels
            signal = {k: {s: (sourceSpectrumFPA[k][s](binCenters[k])*telescope_Area/u.ph*u.ct
                            *(args.binspect*u.pix).to('nm',  equivalencies=dispersion_scale_nobin[k])).decompose()
                            for s in slicer_paths} for k in channels }

            bgvar = {k: {s: (skySpectrumFPA[k][s](binCenters[k])*telescope_Area/u.ph*u.ct
                            *(args.binspect*u.pix).to('nm',  equivalencies=dispersion_scale_nobin[k])).decompose()
                            for s in slicer_paths} for k in channels }

        return signal, bgvar, sharpness

    slit_bracket = tuple(slit_w_range.to('arcsec').value)
    SNRfunc = res_R = res_slitloss = None

    # Solve this function to find slitwidth that gives requested slit loss
    def efffunc(slitw_arcsec, loss=1.):
        eff = slitEfficiency(slitw_arcsec*u.arcsec ,slit_h ,args.seeing[0] ,pivot=args.seeing[1] ,optics=throughput_slicerOptics)
        if args.noslicer: eff = eff['center']
        else:             eff = eff['total']
        return eff(args.wrange).mean() - (1-loss)  # solver will set this line to 0

    # Solve this function to find slitwidth that gives requested R
    def Rfunc(slitw_arcsec, Rgoal=0, seeing=args.seeing[0], pivot=args.seeing[1]):
        # Compute R = lambda/fwhm;  approximate fwhm at center of channel, lambda at center of wrange
        # For FWHM, use kernel result or spectral bin width, whichever is larger
        # kernel, kernel_fwhm, kernel_dlambda = makeLSFkernel(slitw_arcsec*u.arcsec ,seeing ,args.channel ,pivot=pivot)
        LSF = makeLSFkernel_slicer(slitw_arcsec*u.arcsec ,seeing ,args.channel ,pivot=pivot, kernel_upsample=100., centeronly=True)  ### sensitive to kernel_upsample 
        kernel_fwhm = LSF['center']['fwhm']
        binres = (args.binspect*u.pix).to('nm',  equivalencies=dispersion_scale_nobin[args.channel])
        Res = (args.wrange.mean()/max(binres, kernel_fwhm)).to(1).value
        return Res - Rgoal  # solver will set this line to 0

    if args.slitmode == 'LOSS':
        res_slitloss = args.slit/100.
        # solve for where slit efficiency = 100% - loss
        ans = optimize.root_scalar(efffunc ,args=(res_slitloss,) ,bracket=slit_bracket ,x0=args.seeing[0].to('arcsec').value)
        if not ans.converged: raise RuntimeError('Slit solution for slit loss did not converge')
        args.slit = ans.root * u.arcsec

    if args.slitmode == 'RES':
        res_R = args.slit
        Rbounds = [Rfunc(w) for w in slit_w_range.to('arcsec').value]
        if res_R > max(Rbounds): raise RuntimeError('Cannot solve for slit width, R is too high')
        ### R is lower limited by PSF for point sources, and we currently only support point sources
        ### If very low R is requested, approximate extended source by setting seeing very high
        EXTENDED_SOURCE = (res_R < min(Rbounds)) or args.extended
        _seeing = 100*u.arcsec if EXTENDED_SOURCE else args.seeing[0]
        ans = optimize.root_scalar(Rfunc ,args=(res_R,_seeing) ,bracket=slit_bracket ,x0=args.seeing[0].to('arcsec').value)
        if not ans.converged: raise RuntimeError('Slit solution for R did not converge')
        args.slit = ans.root * u.arcsec

    '''  ALL EXPTIME DEPENDENCE BELOW THIS LINE '''

    ''' Compute SNR or solve for EXPTIME or SLIT depending on what is fixed '''

    SOLVE_EXPTIME = (args.ETCmode == 'SNR')  # makes this section more readable
    SOLVE_SLIT    = (args.slitmode == 'SNR')

    # Unpack fixed values
    if SOLVE_EXPTIME:  res_SNR     = args.ETCfixed
    else:              res_exptime = args.ETCfixed
    if not SOLVE_SLIT: res_slitw   = args.slit

    # EXPTIME and SLIT are fixed; just compute SNR
    if (not SOLVE_EXPTIME) and (not SOLVE_SLIT):
        res_SNR = computeSNR(res_exptime, res_slitw, args, SSSfocalplane)

    # SNR and SLIT are fixed; solve for EXPTIME
    elif SOLVE_EXPTIME and (not SOLVE_SLIT):

        # Find root in log-log space where SNR(t) is ~linear; doesn't save much time but more likely to converge
        def SNRfunc(t_sec):
            return log( computeSNR(10**t_sec*u.s, res_slitw, args, SSSfocalplane) / res_SNR)
            # return computeSNR(t_sec*u.s, res_slitw, args, SSSfocalplane) - res_SNR

        #ans = optimize.root_scalar(SNRfunc ,x0=1 ,x1=100)  # Very bright stars may not converge here; try log-log
        ans = optimize.root_scalar(SNRfunc ,x0=0 ,x1=3 ,xtol=.01)

        # Check for converged answer
        if not ans.converged:  raise RuntimeError('ETC calculation did not converge')
        # t = (ans.root).astype('float16')*u.s
        res_exptime = (10.**(ans.root).astype('float16'))*u.s


    # EXPTIME is fixed; find SLIT that is X% below maximum SNR
    elif SOLVE_SLIT and (not SOLVE_EXPTIME):

        def SNRfunc(slitw_arcsec, SNRgoal=0):
            ret = computeSNR(res_exptime, slitw_arcsec*u.arcsec, args, SSSfocalplane)
            return -(ret-SNRgoal)

        # First solve for 'best' slitwidth (maximize SNR)
        ans = optimize.minimize_scalar(SNRfunc ,bounds=slit_bracket 
                                        ,method='bounded' ,options={'xatol':0.05}) # absolute tolerance in slitw

        res_SNR = -ans.fun  # above function can minimize not maximize so we returned -1*SNR
        slit_bracket = (slit_bracket[0], ans.x)  # slit width at maximum SNR
        print('max SNR = %f   slit w = %f arcsec' % (-ans.fun, ans.x))

        # Find slit width where SNR = X% of max; args.slit is percent of max SNR
        if args.slit < 100:
            res_SNR *= args.slit/100.
            ans = optimize.root_scalar(SNRfunc, args=(res_SNR,) ,bracket=slit_bracket ,x0=args.seeing[0].to('arcsec').value, rtol=0.01)
            res_slitw = ans.root*u.arcsec
        else:
            res_slitw = ans.x*u.arcsec

        # if not quiet:
        # slitw_max_arcsec = slit_bracket[-1]
        #     print('SLIT=%s  limit=%.2f  best=%.2f'%(slitw_result.round(2), slitw_max_arcsec, slitw_result.value))

    # SNR is fixed; solve for EXPTIME; for each EXPTIME tried, optimize SLIT
    elif SOLVE_EXPTIME and SOLVE_SLIT:

        def SNRfunc(log_t_sec, slit_bracket=slit_bracket, SNRgoal=res_SNR):

            t = (10**log_t_sec)*u.s

            if args._slitGoodEnough:

                _slitw = args._slitw

            else: 

                def SNRfunc_W(slitw_arcsec, SNRgoal=0):
                    ret = computeSNR(t, slitw_arcsec*u.arcsec, args, SSSfocalplane)
                    return -(ret-SNRgoal)

                # First solve for 'best' slitwidth (maximize SNR)
                ans = optimize.minimize_scalar(SNRfunc_W ,bounds=slit_bracket 
                                                ,method='bounded' ,options={'xatol':0.05}) # absolute tolerance in slitw

                _SNR = -ans.fun  # above function can minimize not maximize so we returned -1*SNR
                slit_bracket = (slit_bracket[0], ans.x)  # slit width at maximum SNR
                # print('max SNR = %f   slit w = %f arcsec' % (-ans.fun, ans.x))

                # Find slit width where SNR = X% of max; args.slit is percent of max SNR
                if args.slit < 100:
                    _SNR *= args.slit/100.
                    ans = optimize.root_scalar(SNRfunc_W, args=(_SNR,) ,bracket=slit_bracket ,x0=args.seeing[0].to('arcsec').value, rtol=0.01)
                    _slitw = ans.root*u.arcsec
                else:
                    _slitw = ans.x*u.arcsec

            dSNR = log( computeSNR(t, _slitw, args, SSSfocalplane) / SNRgoal)
            # print(10**log_t_sec, _slitw, _SNR, dSNR)
            # Use "global" args to store results
            if (not args.hires_solve) and (abs(dSNR) <= .2):  # 10**0.2 = 1.58 i.e. SNR is now within factor of 2 of target
                args._slitGoodEnough = True  # Once SNR is in the ballpark, stop varying the slit and just solve for EXPTIME
            args._slitw = _slitw
            args._SNR = SNRgoal * 10.**dSNR
            return dSNR

        args._slitw = 0.
        args._slitGoodEnough = False
        ans = optimize.root_scalar(SNRfunc ,x0=0 ,x1=3 ,xtol=.01)
        # print(args._slitw)

        if not ans.converged:  raise RuntimeError('ETC calculation did not converge')
        res_exptime = (10.**(ans.root).astype('float16'))*u.s
        res_slitw = args._slitw
        res_SNR = args._SNR

    # PACK UP AND RETURN FROM MAIN()

    result = {
        'wrange' : binCenters[args.channel][[0,-1]].round(3), # Binned wavelength range won't exactly match input range
        'exptime' : res_exptime,
        'slitwidth' : res_slitw,
        'SNR' : res_SNR,
        'resolution' : res_R if res_R else Rfunc(res_slitw.to('arcsec').value),
        'slitloss' : res_slitloss if res_slitloss else  1. - efffunc(res_slitw.to('arcsec').value)
    }

    for k in result: result[k] *= u.Unit(1)  # Convert all to Quantity

    if not quiet:
        print(  '  '.join(['%s=%s' % (k.upper(),v.round(3)) for (k,v) in result.items()])  )

    if args.plotSNR:
        result['plotSNR'] = computeSNR(t, slitw_result ,args, SSSfocalplane, allChans=True)

    # return extra functions and data for plotting
    if ETCextras: return result, efffunc, SNRfunc
    else:         return result


def runETC(row ,check=False, skyspec=None):
    '''Convert an astropy table row into an ETC command and run the ETC to get an exposure time'''
    cmd = formETCcommand(row)
    args = parser.parse_args(cmd.split())  ### Change this parser.error ? Should be OK since we check ETC cols at the beginning
    check_inputs_add_units(args) # Check for valid inputs; append units

    # skyspec should be a tuple of arrays:  (Nlambda, (Nspec, Nlambda)) where Nspec > 1 for multiple filters
    if skyspec:
        try:
            assert len(skyspec) == 2
            for ss in skyspec[1]:
                assert len(skyspec[0]) == len(ss)
        except:
            raise Exception('skyspec must be a 2-item tuple; 0: wavelength samples, 1: N arrays of spectrum values ')

    # If only checking input, we're done
    if check: return True

    result = main(args ,quiet=True, skyspec=skyspec)  # Unitful (s)
    t = result['exptime']
    assert isinstance(t,u.Quantity), "Got invalid result from ETC"
    return result



if __name__ == "__main__":

    args = parser.parse_args()

    # Check for valid inputs; append units
    try: check_inputs_add_units(args)
    except Exception as e: parser.error(e)  # Exit gracefully

    if args.timer:
        from timer import Timer
        tt = Timer()
        tt.start()

    # Run the main program
    result = main(args ,quiet=False)
    print('')

    if args.timer: tt.stop()

    # Plot SNR vs. wavelength if requested
    if args.plotSNR or args.plotslit:
        print('Plotting...')

        from ETC_plots import *
        import matplotlib.pyplot as plt
        #matplotlib.rcParams.update({'font.size': 14})
        from astropy.visualization import astropy_mpl_style, quantity_support
        plt.style.use(astropy_mpl_style)
        quantity_support()

    if args.plotSNR:
        args.ETCmode = 'EXPTIME'
        args.ETCfixed = result['exptime']
        args.slitmode = 'SET'
        args.slit = result['slitwidth']
        result_plot = main(args ,quiet=True ,plotSNR=True)
        SNR1 = result_plot['plotSNR']

        fig, ax = plt.subplots(figsize=(15,4))

        binCenters = makeBinCenters(args.binspect)
        for k in channels:
            ax.plot(binCenters[k], SNR1[k] ,color=channelColor[k] ,label=k)

        plt.ylabel('SNR / wavelength bin')
        plt.legend()
        ax.axvspan(args.wrange[0], args.wrange[1], alpha=0.2, color='grey') # shade user range
        plt.title('EXPTIME = '+str(result['exptime'].round(3)))
        plt.savefig('plotSNR.png')
        print('Wrote', 'plotSNR.png')

    if args.plotslit:
        # Plot SNR and resolution (R) vs. slit width for this target
        args.ETCmode = 'EXPTIME'
        args.ETCfixed = result['exptime']
        args.slitmode = 'SNR'
        args.slit = 90  # dummy value
        plotSNR_vs_slit(args, plt)
