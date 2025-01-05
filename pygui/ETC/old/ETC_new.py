#!/usr/bin/env python
# Author: Chaz Shapiro (2022)
#
# Notes on SNR estimation: https://www.stsci.edu/instruments/wfpc2/Wfpc2_hand/HTML/W2_61.html

# TODOS: K-CORRECTION
#        USER SED, EXTENDED SOURCES (mag/arcsec**2), HOST GALAXY (mag/area)
#        MOON PHASE/POSITION, ZODIACAL LIGHT
#        Unique FILENAMES for plots?
#
# Clean up global-ish variables?
# Error handling
# Should background variances be 2x to acount for sky subtraction?

# import timer
# tt = timer.Timer()
# tt.start()  # Measures time until tt.stop()

from ETC.ETC_config import *
from ETC.ETC_arguments import *
from ETC.ETC_import import *
from numpy import array, arange, vstack, log

''' LOAD DATA that doesn't depend on user input '''

# Load sky background; eventually will need to interpolate a larger table
# background units dimensions are not same as other flux units
# File units = u.photon/u.s/u.nm/u.arcsec**2/u.m**2 ~ phot/s/wavelength  VS  nm
skySpec0 = SourceSpectrum.from_file(CSVdir+skybackground_file ,wave_unit='nm') #HARDCODED UNIT
# assumes units = phot/s/cm^2/Angstrom 

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

''' MAIN FUNCTION RETURNS DICT OF RESULTS '''
def main(args ,quiet=False):
    from ETC.ETC_config import channels

    # Check for valid inputs
    # If running as command line script, exit gracefully from a problem, otherwise raise an exception
    try:
        check_inputs_add_units(args)
    except Exception as e:
        if __name__ == "__main__": parser.error(e)
        else: raise e

    # Only bother with channels being used for SNR;  Saves ~0.3s
    if not args.plotSNR and not args.plotdiag: channels = [args.channel]

    # Load source spectrum model and normalize
    sourceSpectrum = makeSource(args)

    # Normalize the sky; normalization is wrong but proportional to phot/s/wavelength, same as file
    skySpec = skySpec0.normalize(args.skymag*uu.VEGAMAG ,band=skyFilter ,vegaspec=SourceSpectrum.from_vega() ) ###18.4ms
    # new units = VEGAMAG/arcsec^2 since skymag is really mag/arcsec^2

    # Load "throughput" for atmosphere
    throughput_atm = Extinction_atm(args.airmass)

    # Source is modulated by atm, sky is not
    sourceSpec_wTP = { k : sourceSpectrum * throughput_atm * TP[k] for k in channels }
    skySpec_wTP = { k : skySpec * TP[k] for k in channels }

    # Print the bins where the target wavelengths live; won't exactly match input range
    binCenters = makeBinCenters(args.binning[0])
    closest_bin_i = [abs(binCenters[args.channel]-wr).argmin() for wr in args.wrange]
    if not quiet:
        print( args.wrange, "-->", binCenters[args.channel][closest_bin_i].round(3) )

    '''  ALL SLIT DEPENDENCE BELOW THIS LINE '''

    # Setup the slicer function; cache saves time in loops where slit width doesn't change
    @cache
    def SSSfocalplane(w, chanlist):
        return applySlit(w, sourceSpec_wTP, skySpec_wTP, throughput_slicerOptics, args ,chanlist)

    # Solve this function to find slitwidth that gives 97% efficiency (3% slit loss)
    def efffunc(slitw_arcsec):
        eff = slitEfficiency(slitw_arcsec*u.arcsec ,slit_h ,args.seeing[0] ,pivot=args.seeing[1] ,optics=throughput_slicerOptics)
        if args.noslicer: eff = eff['center']
        else:             eff = eff['total']
        return eff(args.wrange).mean() - slit_efficiency_max

    '''  ALL EXPTIME DEPENDENCE BELOW THIS LINE '''

    # Unpack SNR or EXPTIME and set the other to None
    if args.ETCmode == 'SNR':       SNR_target, exptime = (args.ETCfixed, None)
    elif args.ETCmode == 'EXPTIME': SNR_target, exptime = (None, args.ETCfixed)
    else: raise Exception('Invalid ETC mode')

    if args.slit is not None: args.ETCmode += '..SLIT'  # makes this section more readable

    ''' Compute SNR or solve for EXPTIME or SLIT depending on what is fixed '''

    # EXPTIME and SLIT are fixed; just compute SNR
    if args.ETCmode == 'EXPTIME..SLIT':
        t = exptime
        slitw_result = args.slit
        SNR_result = computeSNR(t, args.slit, args, SSSfocalplane).astype('float16')

    # SNR and SLIT are fixed; solve for EXPTIME
    elif args.ETCmode == 'SNR..SLIT':
        from scipy import optimize

        slitw_result = args.slit
        SNR_result = SNR_target

        # Find root in log-log space where SNR(t) is ~linear; doesn't save much time but more likely to converge
        def SNRfunc(t_sec):
            return log( computeSNR(10**t_sec*u.s, args.slit, args, SSSfocalplane) / SNR_target)
            # return computeSNR(t_sec*u.s, args.slit, args, SSSfocalplane) - SNR_target

        #ans = optimize.root_scalar(SNRfunc ,x0=1 ,x1=100)  ### Very bright stars may not converge here; try log-log
        ans = optimize.root_scalar(SNRfunc ,x0=0 ,x1=3 ,xtol=.01)  ###52ms

        # Check for converged answer
        if ans.converged:
            # t = (ans.root).astype('float16')*u.s
            t = (10.**(ans.root).astype('float16'))*u.s
        else:
            raise RuntimeError('ETC calculation did not converge')
    # 52ms
    # EXPTIME is fixed; find SLIT that maximizes SNR
    elif args.ETCmode == 'EXPTIME':
        from scipy import optimize
        t = exptime

        ans = optimize.root_scalar(efffunc ,bracket=tuple(slit_w_range.to('arcsec').value) ,x0=args.seeing[0].to('arcsec').value)
        # Check for converged answer
        if ans.converged:
            slitw_max_arcsec = ans.root # unitless, in arcsec
        else:
            raise RuntimeError('Max slit efficiency calculation did not converge')

        # Solve for 'best' slitwidth (maximize SNR)
        def SNRfunc(slitw_arcsec):
            # print(slitw_arcsec)
            ret = computeSNR(t, slitw_arcsec*u.arcsec, args, SSSfocalplane)
            return -ret

        ans = optimize.minimize_scalar(SNRfunc ,bounds=slit_w_range.value 
                                        ,method='bounded' ,options={'xatol':0.02}) # absolute tolerance in slitw
        slitw_best_arcsec = ans.x

        if slitw_best_arcsec <= slitw_max_arcsec:
            SNR_result = -ans.fun
            slitw_result = slitw_best_arcsec*u.arcsec
        else:
            SNR_result = -SNRfunc(slitw_max_arcsec)
            slitw_result = slitw_max_arcsec*u.arcsec

        print('SLIT=%s  limit=%.2f  best=%.2f'%(slitw_result.round(2), slitw_max_arcsec, slitw_best_arcsec))

    # SNR is fixed; solve for EXPTIME; for each EXPTIME tried, optimize SLIT
    # elif args.ETCmode == 'SNR':

    t = t.round(3)

    if not quiet:
        print('slit efficiency= %.3f' % (efffunc(slitw_result.to('arcsec').value)+slit_efficiency_max) )
        print('SNR=%.2f   exptime=%s'%(SNR_result, t) )
        
    # tt.stop()

    # RETURN FROM MAIN()

    result = {'exptime':t, 'slitwidth':slitw_result, 'SNR':SNR_result}

    if args.plotSNR:
        result['plotSNR'] = computeSNR(t, slitw_result ,args, SSSfocalplane, allChans=True)

    return result

def runETC(row):
    '''Convert an astropy table row into an ETC command and run the ETC to get an exposure time'''
    cmd = formETCcommand(row)
    x = parser.parse_args(cmd.split())  ### Change this parser.error ? Should be OK since we check ETC cols at the beginning
    result = main(x ,quiet=True)  # Unitful (s)
    t = result['exptime']
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
    args = parser.parse_args()
    result = main(args ,quiet=False)
    print('')

    # Plot SNR vs. wavelength if requested
    if args.plotSNR or args.plotdiag:
        print('Plotting...')

        import matplotlib.pyplot as plt
        #matplotlib.rcParams.update({'font.size': 14})
        from astropy.visualization import astropy_mpl_style, quantity_support
        plt.style.use(astropy_mpl_style)
        quantity_support()

        SNR1 = result['plotSNR']

        fig, ax = plt.subplots(figsize=(15,4))

        binCenters = makeBinCenters(args.binning[0])
        for k in channels:
            ax.plot(binCenters[k], SNR1[k] ,color=channelColor[k] ,label=k)

        plt.ylabel('SNR / wavelength bin')
        plt.legend()
        ax.axvspan(args.wrange[0], args.wrange[1], alpha=0.2, color='grey') # shade user range
        plt.title('EXPTIME = '+str(result['exptime']))
        plt.savefig('plotSNR.png')
