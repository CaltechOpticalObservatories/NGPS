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
import edu.dartmouth.jskycalc.objects.Observation;
import java.util.List.*;
import edu.dartmouth.jskycalc.objects.*;
/*================================================================================================
/
/=================================================================================================*/
public class Ecliptic {
  /* Holds some static methods for rotating from ecliptic to equatorial and back ... */

  public static double [] eclrot(double jd, double x, double y, double z) {
  /** rotates x,y,z coordinates to equatorial x,y,z; all are
      in equinox of date. Returns [0] = x, [1] = y, [2] = z */
     double incl;
     double T;
     double [] retval = {0.,0.,0.};

     T = (jd - Const.J2000) / 36525;
     incl = (23.439291 + T * (-0.0130042 - 0.00000016 * T))/Const.DEG_IN_RADIAN;
             /* 1992 Astron Almanac, p. B18, dropping the
               cubic term, which is 2 milli-arcsec! */
     // System.out.printf("T incl %f %f\n",T,incl);
     retval[1] = Math.cos(incl) * y - Math.sin(incl) * z;
     retval[2] = Math.sin(incl) * y + Math.cos(incl) * z;
     retval[0] = x;
     return(retval);
  }
/*================================================================================================
/
/=================================================================================================*/
  public static double [] Cel2Ecl(Observation o) {
  /** rotates celestial coords to equatorial at
      equinox of jd. Returns [0] = x, [1] = y, [2] = z */
     double incl;
     double T;
     double julep;
     double [] retval = {0.,0.};   // ecliptic longitude and latitude
     double [] equat = {0.,0.,0.}; // equatorial unit vector
     double [] eclipt = {0.,0.,0.}; // ecliptic unit vector

     T = (o.w.when.jd - Const.J2000) / 36525;
     incl = (23.439291 + T * (-0.0130042 - 0.00000016 * T))/Const.DEG_IN_RADIAN;
             /* 1992 Astron Almanac, p. B18, dropping the
               cubic term, which is 2 milli-arcsec! */
     // System.out.printf("T incl %f %f\n",T,incl);

     equat = o.current.cel_unitXYZ();

     eclipt[1] = Math.cos(incl) * equat[1] + Math.sin(incl) * equat[2];
     eclipt[2] = -1. * Math.sin(incl) * equat[1] + Math.cos(incl) * equat[2];
     eclipt[0] = equat[0];

     retval[0] = Math.atan2(eclipt[1],eclipt[0]) * Const.DEG_IN_RADIAN;
     while(retval[0] < 0.) retval[0] += 360.;
     while(retval[0] >= 360.) retval[0] -= 360.;
     retval[1] = Math.asin(eclipt[2]) * Const.DEG_IN_RADIAN;

     return(retval);
  }
}
