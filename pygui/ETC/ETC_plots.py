from ETC_main import *

def plotSNR_vs_slit(args, plt):

    w_arcsec = arange(.2,4.5,.2)

    ### HARDCODED UNITS
    fwhm_dlam = [makeLSFkernel_slicer(wi ,args.seeing[0] ,args.channel ,pivot=args.seeing[1], kernel_upsample=10.)[1:] for wi in w_arcsec*u.arcsec]

    fwhm =  array([x[0] for x in fwhm_dlam]).T*fwhm_dlam[0][0].unit  # 2 x Nwi
    dlambda =  array([x[1].value for x in fwhm_dlam]).T * fwhm_dlam[1][0].unit  # 2 x Nwi

    R = array(args.wrange).mean()*u.nm/fwhm
    R = R.to(1).value

    slicer = [False, True]
    colors = ['orange','blue']
    labels = ['slit only','with slicer']

    fig, ax1 = plt.subplots(figsize=(10,6))

    for s, c, l in zip(slicer, colors, labels):

        # Compute SNR for all slit widths; find max SNR

        args.noslicer = not s
        result, efffunc, SNRfunc = main(args ,quiet=True, ETCextras=True)  #SNRfunc will be SNR vs slit in arcsec

        # Check that main() returned a function we can loop over for plotting
        if SNRfunc is None:
            from sys import exit
            exit('No SNR function for plotting generated in this mode')

        snrs = -array(list(map(SNRfunc,w_arcsec)))
        #ans = optimize.root_scalar(efffunc ,bracket=(w_arcsec.min(),w_arcsec.max()) ,x0=args.seeing[0])
        #w97 = ans.root

        #Plot SNR for each case with vertical markers
        ax1.plot(w_arcsec, snrs,color=c ,label=l)
        if s: ll = 'max SNR'
        else: ll = None
        ax1.axvline(w_arcsec[snrs.argmax()] ,color=c ,ls='--',label=ll)

    # Add labels for SNR plots
    ax1.set_xlabel('Slit width (arcsec)')
    ax1.set_ylabel('SNR')
    ax1.set_title( '%s   mag=%s   exptime=%s   seeing=%s"' % (result['wrange'], args.mag, result['exptime'].round(3), args.seeing[0].round(2).value) )
    ax1.legend(loc='center right')

    # Add plot and 2nd y-axis
    ax2 = ax1.twinx()
    ax2.plot(w_arcsec, R[0], label='R (center)' ,color='green')
    ax2.plot(w_arcsec, R[1], label='R (side)' ,color='green', ls='dashed')
    ax2.set_ylabel('R', color='green')
    ax2.tick_params(axis ='y', labelcolor = 'green') 
    ax2.tick_params(axis='y', direction='in', length=6, width=2, colors='g')#, grid_color='r', grid_alpha=0.5)
    ax2.grid(False)
    ax2.legend(loc='lower right')

    plt.savefig('SNR_vs_slit.png')
    print('Wrote', 'SNR_vs_slit.png')


def plotSNR_vs_slit_mags(args, plt):

    # w_arcsec = arange(.2,4.5,.1)
    w_arcsec = arange(.2,4,.05)
    # x = w_arcsec*u.arcsec/args.seeing[0]
    x = w_arcsec

    mags = [15,17,19,21,23]
    cmap=plt.cm.get_cmap('cool')
    colors = (arange(len(mags))+1)/len(mags)

    ### HARDCODED UNITS
    fwhm = [makeLSFkernel(wi ,args.seeing[0] ,args.channel ,pivot=args.seeing[1], kernel_upsample=100.)[1] for wi in w_arcsec*u.arcsec]
    fwhm = array(fwhm)*fwhm[0].unit
    R = array(args.wrange).mean()*u.nm/fwhm
    R = R.to(1).value

    s = True #slicer
    c = 'blue' #color
    l = None #label

    fig, ax1 = plt.subplots(figsize=(10,6.5))

    # Compute SNR for all slit widths; find max SNR and where slit encloses 97% of PSF

    print('Computing curves...')

    args.noslicer = not s

    snr_m = []
    for m in mags:
        args.mag = m
        result, efffunc, SNRfunc = main(args ,quiet=True, ETCextras=True)  #SNRfunc will be SNR vs slit in arcsec

        # Check that main() returned a function we can loop over for plotting
        if SNRfunc is None:
            from sys import exit
            exit('No SNR function for plotting generated in this mode')

        snrs = -array(list(map(SNRfunc,w_arcsec)))
        # snr_m.append(snrs/snrs.max()-m/20.)
        snr_m.append(snrs/snrs.max())
    snr_m = array(snr_m)

    # breakpoint()

    print('Rendering plot...')

    from matplotlib.ticker import (MultipleLocator, AutoMinorLocator)

    #Plot SNR for each case with vertical markers
    offset = 0.0
    for i, m, snrs, c in zip(range(len(mags)), mags,snr_m, colors):
        ax1.plot(x, snrs-i*offset ,label=str(m) ,color=cmap(c) )
        # ax1.semilogy(x, snrs ,label=str(m) ,color=cmap(c) )
        plt.plot(x[snrs.argmax()], snrs.max()-i*offset, marker="o" ,markersize=10 ,color=cmap(c))

    # Add labels for SNR plots
    ax1.set_xlabel('Slit width (arcscec)')
    ax1.set_ylabel('SNR/max(SNR)')
    ax1.yaxis.set_major_locator(MultipleLocator(.1))
    ax1.yaxis.set_major_formatter('{x:.1f}')
    ax1.set_ylim(.2,1.05)

    # Add plot and 2nd y-axis
    ax2 = ax1.twinx()
    ax2.plot(x, R, label='R' ,color='green')
    ax2.set_ylabel('R', color='green')
    ax2.tick_params(axis ='y', labelcolor = 'green') 
    ax2.tick_params(axis='y', direction='in', length=6, width=2, colors='g')#, grid_color='r', grid_alpha=0.5)
    ax2.grid(False)
    ax2.set_ylim(800,6800)

    # Add legend last so it's on top
    ax1.legend(loc='lower left' ,title='Mag')

    def w_arcsec2see(w):
        return w*u.arcsec/args.seeing[0]

    def see2w_arcsec(x):
        return x*args.seeing[0]/u.arcsec

    secax = ax1.secondary_xaxis('top', functions=(w_arcsec2see, see2w_arcsec))
    secax.set_xlabel('Slit width / seeing')
    secax.tick_params(axis='x', direction='in', length=6, width=2, colors='black')#, grid_color='r', grid_alpha=0.5)

    fig.suptitle( '%s   skymag=%s   exptime=%s   seeing=%s"' % (result['wrange'], args.skymag, result['exptime'].round(3), args.seeing[0].round(2).value) )


    plt.savefig('SNR_vs_slit.png')
    print('Wrote', 'SNR_vs_slit.png')


    fig, ax1 = plt.subplots(figsize=(10,5))

    #Plot SNR for each case with vertical markers
    for i, m, snrs, c in zip(range(len(mags)), mags,snr_m, colors):
        imax = snrs.argmax()
        ax1.plot(R[:imax+1], snrs[:imax+1] ,label=str(m) ,color=cmap(c) )

    ax1.legend(loc='lower left' ,title='Mag')
    ax1.set_xlabel('R')
    ax1.set_ylabel('SNR/max(SNR)')
    ax1.set_xlim(800,6800)
    ax1.set_ylim(.2,1.03)

    fig.suptitle( '%s   skymag=%s   exptime=%s   seeing=%s"' % (result['wrange'], args.skymag, result['exptime'].round(3), args.seeing[0].round(2).value) )

    plt.savefig('SNR_v_R.png')
