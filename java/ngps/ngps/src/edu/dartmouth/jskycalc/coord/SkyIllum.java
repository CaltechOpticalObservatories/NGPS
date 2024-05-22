package edu.dartmouth.jskycalc.coord;   
//=== File Prolog =============================================================
//	This code was developed by John Thorstensen of Dartmouth University
//
//      It has been refactored and modified by Jennifer Milburn for
//      use at the Palomar Observatory.
//
//      Caltech Optical Observatories, Palomar Observatory
//
//--- Contents ----------------------------------------------------------------
//
//
//--- Description -------------------------------------------------------------
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
//       DERIVED DIRECTLY FROM THE JSKYCALC by JOHN THORSTENSEN, Dartmouth University
//       Original Implementation : John Thorstensen copyright 2007,
//
//       July 24, 2010  Jennifer Milburn, refactor for use at the Palomar Observatory
//
//
//   JSkyCalc24m.java -- 2009 January.  I liked the 1.3m so much I decided
//   to do something similar for the 2.4m.  This has the capability of reading
//   the telescope coords and commenting on them, but it doesn't slew the
//   telescope.  It'll be nice to automatically have the telescope coords there,
//   though. */
//
// JSkyCalc13m.java -- 2009 January.  This version adds an interface to
// * Dick Treffers' 1.3m telescope control server, which communicates via
// * the BAIT protocol.  The target-selection facilities here can then be
// * used. */
//
// JSkyCalc.java -- copyright 2007, John Thorstensen, Dartmouth College. */
//
// TERMS OF USE --
// Anyone is free to use this software for any purpose, and to
// modify it for their own purposes, provided that credit is given to the author
// in a prominent place.  For the present program that means that the green
// title and author banner appearing on the main window must not be removed,
// and may not be altered without premission of the author.
//
//--- DISCLAIMER---------------------------------------------------------------
//
//	This software is provided "as is" without any warranty of any kind, either
//	express, implied, or statutory, including, but not limited to, any
//	warranty that the software will conform to specification, any implied
//	warranties of merchantability, fitness for a particular purpose, and
//	freedom from infringement, and any warranty that the documentation will
//	conform to the program, or any warranty that the software will be error
//	free.
//
//	In no event shall Caltech be liable for any damages, including, but not
//	limited to direct, indirect, special or consequential damages, arising out
//	of, resulting from, or in any way connected with this software, whether or
//	not based upon warranty, contract, tort or otherwise, whether or not
//	injury was sustained by persons or property or otherwise, and whether or
//	not loss was sustained from or arose out of the results of, or use of,
//	their software or services provided hereunder.
//
//=== End File Prolog =========================================================
import java.util.List.*;
/*================================================================================================
/
/=================================================================================================*/
public class SkyIllum {
/** container for the ztwilight and krisciunas/schaefer routines */
/*================================================================================================
/
/=================================================================================================*/
   public static double ztwilight(double alt) {

/* evaluates a polynomial expansion for the approximate brightening
   in magnitudes of the zenith in twilight compared to its
   value at full night, as function of altitude of the sun (in degrees).
   To get this expression I looked in Meinel, A.,
   & Meinel, M., "Sunsets, Twilight, & Evening Skies", Cambridge U.
   Press, 1983; there's a graph on p. 38 showing the decline of
   zenith twilight.  I read points off this graph and fit them with a
   polynomial; I don't even know what band there data are for! */
/* Comparison with Ashburn, E. V. 1952, JGR, v.57, p.85 shows that this
   is a good fit to his B-band measurements.  */

        double y, val;

        if(alt > 0.) return 99.;  // guard
        if(alt < -18.) return 0.;

        y = (-1.* alt - 9.0) / 9.0;  /* my polynomial's argument...*/
	val = ((2.0635175 * y + 1.246602) * y - 9.4084495)*y + 6.132725;
	return(val);
   }
/*================================================================================================
/
/=================================================================================================*/
   public static double lunskybright(double alpha,double rho,double kzen,
		double altmoon, double alt, double moondist)  {

/* Evaluates predicted LUNAR part of sky brightness, in
   V magnitudes per square arcsecond, following K. Krisciunas
   and B. E. Schaeffer (1991) PASP 103, 1033.

   alpha = separation of sun and moon as seen from earth,
   converted internally to its supplement,
   rho = separation of moon and object,
   kzen = zenith extinction coefficient,
   altmoon = altitude of moon above horizon,
   alt = altitude of object above horizon
   moondist = distance to moon, in earth radii

   all are in decimal degrees. */

    double istar,Xzm,Xo,Z,Zmoon,Bmoon,fofrho,rho_rad,test;

    rho_rad = rho/Const.DEG_IN_RADIAN;
    alpha = (180. - alpha);
    Zmoon = (90. - altmoon)/Const.DEG_IN_RADIAN;
    Z = (90. - alt)/Const.DEG_IN_RADIAN;
    moondist = Const.EARTHRAD_IN_AU * moondist/(60.27);
        /* distance arrives in AU, want it normalized to mean distance,
            60.27 earth radii. */

    istar = -0.4*(3.84 + 0.026*Math.abs(alpha) + 4.0e-9*Math.pow(alpha,4.)); /*eqn 20*/
    istar =  Math.pow(10.,istar)/(moondist * moondist);
    if(Math.abs(alpha) < 7.)   /* crude accounting for opposition effect */
	istar = istar * (1.35 - 0.05 * Math.abs(istar));
	/* 35 per cent brighter at full, effect tapering linearly to
	   zero at 7 degrees away from full. mentioned peripherally in
	   Krisciunas and Scheafer, p. 1035. */
    fofrho = 229087. * (1.06 + Math.cos(rho_rad)*Math.cos(rho_rad));
    if(Math.abs(rho) > 10.)
       fofrho=fofrho + Math.pow(10.,(6.15 - rho/40.));            /* eqn 21 */
    else if (Math.abs(rho) > 0.25)
       fofrho= fofrho+ 6.2e7 / (rho*rho);   /* eqn 19 */
    else fofrho = fofrho+9.9e8;  /*for 1/4 degree -- radius of moon! */
    Xzm = Math.sqrt(1.0 - 0.96*Math.sin(Zmoon)*Math.sin(Zmoon));
    if(Xzm != 0.) Xzm = 1./Xzm;
	  else Xzm = 10000.;
    Xo = Math.sqrt(1.0 - 0.96*Math.sin(Z)*Math.sin(Z));
    if(Xo != 0.) Xo = 1./Xo;
	  else Xo = 10000.;
    Bmoon = fofrho * istar * Math.pow(10.,(-0.4*kzen*Xzm))
	  * (1. - Math.pow(10.,(-0.4*kzen*Xo)));   /* nanoLamberts */
    if(Bmoon > 0.001)
      return(22.50 - 1.08574 * Math.log(Bmoon/34.08)); /* V mag per sq arcs-eqn 1 */
    else return(99.);
  }
}
