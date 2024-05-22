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
public class deltaT {
  /* holds only a static method that returns a rough ephemeris time
     correction */
/*================================================================================================
/
/=================================================================================================*/
  public static double etcorr(double jd) {

	/* Given a julian date in 1900-2100, returns the correction
           delta t which is:
		TDT - UT (after 1983 and before 1998)
		ET - UT (before 1983)
		an extrapolated guess  (after 2001).

	For dates in the past (<= 2001 and after 1900) the value is linearly
        interpolated on 5-year intervals; for dates after the present,
        an extrapolation is used, because the true value of delta t
	cannot be predicted precisely.  Note that TDT is essentially the
	modern version of ephemeris time with a slightly cleaner
	definition.

	Where the algorithm shifts there will be a small (< 0.1 sec)
        discontinuity.  Also, the 5-year linear interpolation scheme can
        lead to errors as large as 0.5 seconds in some cases, though
 	usually rather smaller.   One seldom has actual UT to work with anyway,
	since the commonly-used UTC is tied to TAI within an integer number
	of seconds.  */

	double jd1900 = 2415019.5;
	double [] dates = {1900.,1905.,1910.,1915.,1920.,1925.,1930.,
          1935.,1940.,1945.,1950.,1955.,1960.,1965.,1970.,1975.,1980.,
          1985.,1990.,1995.,2000.,2004.};
          // 2004 is the last one tabulated in the 2006 almanac
	double [] delts = { -2.72, 3.86, 10.46, 17.20, 21.16, 23.62,
	24.02,  23.93, 24.33, 26.77,  29.15, 31.07, 33.15,  35.73, 40.18,
	45.48,  50.54, 54.34, 56.86,  60.78, 63.83, 64.57};
	double year, delt = 0.;
	int i;

	year = 1900. + (jd - jd1900) / 365.25;

	if(year < 2004. && year >= 1900.) {
		i = (int) ((year - 1900) / 5);
		delt = delts[i] +
		 ((delts[i+1] - delts[i])/(dates[i+1] - dates[i])) * (year - dates[i]);
	}

	else if (year >= 2004. && year < 2100.)
		delt = 31.69 + (2.164e-3) * (jd - 2436935.4);  /* rough extrapolation */
                /* the 31.69 is adjusted to give 64.09 sec at the start of 2001. */
	else if (year < 1900) {
		// printf("etcorr ... no ephemeris time data for < 1900.\n");
       		delt = 0.;
	}

	else if (year >= 2100.) {
		// printf("etcorr .. very long extrapolation in delta T - inaccurate.\n");
		delt = 180.; /* who knows? */
	}

	return(delt);
  }
}