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
import java.util.List.*;
/*================================================================================================
/
/=================================================================================================*/
public class Topo {  // topocentric correction stuff

  static final double FLATTEN = 0.003352813;  // flattening, 1/298.257
  static final double EQUAT_RAD = 6378137.;   // equatorial radius, meters
/*================================================================================================
/
/=================================================================================================*/
 public static double [] Geocent(double longitin, double latitin, double height) {
      // XYZ coordinates given geographic.  Declared static because it will often
      // be used with lst in place of longitude.
      // input is decimal hours, decimal degrees, and meters.
      // See 1992 Astr Almanac, p. K11.

      double denom, C_geo, S_geo;
      double geolat, coslat, sinlat, geolong, sinlong, coslong;
      double [] retval = {0.,0.,0.};


      //System.out.printf("lat long %f %f\n",latitin,longitin);
      geolat = latitin / Const.DEG_IN_RADIAN;
      geolong = longitin / Const.HRS_IN_RADIAN;
      //System.out.printf("radians %f %f \n",geolat,geolong);
      coslat = Math.cos(geolat);  sinlat = Math.sin(geolat);
      coslong = Math.cos(geolong); sinlong = Math.sin(geolong);

      denom = (1. - FLATTEN) * sinlat;
      denom = coslat * coslat + denom * denom;
      C_geo = 1./ Math.sqrt(denom);
      S_geo = (1. - FLATTEN) * (1. - FLATTEN) * C_geo;
      C_geo = C_geo + height / EQUAT_RAD;
      S_geo = S_geo + height / EQUAT_RAD;
      retval[0] = C_geo * coslat * coslong;
      retval[1] = C_geo * coslat * sinlong;
      retval[2] = S_geo * sinlat;

      return retval;
  }
/*================================================================================================
/
/=================================================================================================*/
 public static Celest topocorr(Celest geopos, InstantInTime when, Site where, double sidereal) {
      // The geopos to which this is being applied needs to have its
      // distance set.

      double x, y, z, x_geo, y_geo, z_geo, topodist;
      double [] retvals;

      x = Math.cos(geopos.Alpha.radians()) *
          Math.cos(geopos.Delta.radians()) * geopos.distance;
      y = Math.sin(geopos.Alpha.radians()) *
          Math.cos(geopos.Delta.radians()) * geopos.distance;
      z = Math.sin(geopos.Delta.radians()) * geopos.distance;

      retvals = Geocent(sidereal,where.lat.value,
        where.elevsea);
      x_geo = retvals[0] / Const.EARTHRAD_IN_AU;
      y_geo = retvals[1] / Const.EARTHRAD_IN_AU;
      z_geo = retvals[2] / Const.EARTHRAD_IN_AU;

      x = x - x_geo;
      y = y - y_geo;
      z = z - z_geo;

      topodist = Math.sqrt(x*x + y*y + z*z);

      x /= topodist;
      y /= topodist;
      z /= topodist;

      return new Celest(Math.atan2(y,x) * Const.HRS_IN_RADIAN,
                        Math.asin(z) * Const.DEG_IN_RADIAN,
                        when.JulianEpoch(), topodist);
   }
}
