package edu.dartmouth.jskycalc.objects;      
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

import edu.dartmouth.jskycalc.coord.Celest;
import edu.dartmouth.jskycalc.coord.Const;
import edu.dartmouth.jskycalc.coord.HA;
import edu.dartmouth.jskycalc.coord.SkyIllum;
import edu.dartmouth.jskycalc.coord.Spherical;
import java.util.List.*;
/*================================================================================================
/
/=================================================================================================*/
public class Observation implements Cloneable {

   public WhenWhere w;
   public Celest c;
   public Celest current;
   public HA ha;
   public double altitude, azimuth, parallactic;
   public double airmass;
   public double barytcor, baryvcor;   // time and velocity corrections to barycenter
   public double baryjd;

   // I'd been hauling around a separate sun and moon, but these are now
   // expunged ... they're in the WhenWhere.  But moonlight, moonobj, and sunobj
   // depend on the object coordinates so they are here:

   public double moonlight;
   public double moonobj, sunobj;    // angular sepn of moon from obj and sun from obj
/*================================================================================================
/
/=================================================================================================*/
   public Observation(WhenWhere wIn, Celest celIn) {
      w = (WhenWhere) wIn.clone();
      c = (Celest) celIn.clone();
      ha = new HA(0.);
      ComputeSky();
   }
/*================================================================================================
/
/=================================================================================================*/
   public void ComputeSky() {   // bare-bones updater
      // assumes WhenWhere w has been updated.
      current = c.precessed(w.when.JulianEpoch());
      ha.setHA(w.sidereal - current.Alpha.value);
      double [] altazpar = altit(current.Delta.value, ha.value, w.where.lat.value);
      altitude = altazpar[0];
      azimuth = altazpar[1];
      parallactic = altazpar[2];
      airmass = Spherical.true_airmass(altitude);
   }
/*================================================================================================
/
/=================================================================================================*/
   // Split off the sun, moon, barycenter etc. to save time -- they're
   // not always needed in every instance.

   public Observation clone() {  // override clone method to make it public
      try {
         Observation copy = (Observation) super.clone();   // this needs to be try/catch to make it work
         copy.w = (WhenWhere) w.clone();
         copy.c = (Celest) c.clone();
         copy.current = (Celest) current.clone();
         copy.ha = (HA) ha.clone();
         copy.altitude = altitude;
         copy.azimuth = azimuth;
         copy.parallactic = parallactic;
         copy.barytcor = barytcor;
         copy.baryvcor = baryvcor;
         copy.baryjd = baryjd;
         copy.moonlight = moonlight;
         copy.moonobj = moonobj;
         copy.sunobj = sunobj;
         return copy;
      } catch (CloneNotSupportedException e) {
        throw new Error("This should never happen!\n");
      }
   }
/*================================================================================================
/
/=================================================================================================*/
   public void ComputeSunMoon() {


      current = c.precessed(w.when.JulianEpoch());
      //System.out.printf("ComputeSunMoon %s (%f)\n",
      //       current.Alpha.RoundedRAString(2," "),current.Equinox);
      w.ComputeSunMoon();   // the non-object related parts are all done here

      sunobj = Const.DEG_IN_RADIAN * Spherical.subtend(w.sun.topopos,current);
      moonobj = Const.DEG_IN_RADIAN * Spherical.subtend(w.moon.topopos,current);

      moonlight = SkyIllum.lunskybright(w.sunmoon,moonobj,0.172,w.altmoon,
                altitude,w.moon.topopos.distance);

   }
/*================================================================================================
/
/=================================================================================================*/
   public static double [] altit(double dec, double hrangle, double lat) {
    // returns altitiude (degr), azimuth (degrees), parallactic angle (degr)
      double x,y,z;
      double sinp, cosp;   // sine and cosine of parallactic angle
      double cosdec, sindec, cosha, sinha, coslat, sinlat;
      double retvals [] = {0.,0.,0.};
      double az, parang;

      dec = dec / Const.DEG_IN_RADIAN;
      hrangle = hrangle / Const.HRS_IN_RADIAN;
      lat = lat / Const.DEG_IN_RADIAN;
      cosdec = Math.cos(dec); sindec = Math.sin(dec);
      cosha = Math.cos(hrangle); sinha = Math.sin(hrangle);
      coslat = Math.cos(lat); sinlat = Math.sin(lat);
      x = Const.DEG_IN_RADIAN * Math.asin(cosdec*cosha*coslat + sindec*sinlat);
            // x is the altitude.
      y =  sindec*coslat - cosdec*cosha*sinlat; /* due N comp. */
      z =  -1. * cosdec*sinha; /* due east comp. */
      az = Math.atan2(z,y);

      if(cosdec != 0.) { // protect from divide by zero
         sinp = -1. * Math.sin(az) * coslat / cosdec;
              /* spherical law of sines ... cosdec = sine of codec,
                    coslat = sine of colatitude */
         cosp = -1. * Math.cos(az) * cosha - Math.sin(az) * sinha * sinlat;
              /* spherical law of cosines ... also transformed to local
                    available variables. */
         parang = Math.atan2(sinp, cosp) * Const.DEG_IN_RADIAN;
              /* library function gets the quadrant. */
      }
      else { // you're on the pole ...
         if(lat >= 0.) parang = 180.;
         else parang = 0.;
      }

      az *= Const.DEG_IN_RADIAN;
      while(az < 0.) az += 360.;
      while(az >= 360.) az -= 360.;

      retvals[0] = x;
      retvals[1] = az;
      retvals[2] = parang;

      return retvals;
   }
//
//   void computesun() {
//      sun = new Sun(w);
//   }
//
//   void computemoon() {
//      moon = new Moon(w);
//   }
//
/*================================================================================================
/
/=================================================================================================*/
   public void computebary(Planets p) {

      w.baryxyzvel(p, w.sun);  /* find the position and velocity of the
                                 observing site wrt the solar system barycent */
      double [] unitvec = current.cel_unitXYZ();
//      System.out.printf("Current: %s\n",current.checkstring());
      barytcor = 0.; baryvcor = 0.;
//      System.out.printf("Bary xyz %f %f %f \n",w.barycenter[0],
//             w.barycenter[1],w.barycenter[2]);
      for(int i = 0; i < 3; i++) {
         barytcor += unitvec[i] * w.barycenter[i];
         baryvcor += unitvec[i] * w.barycenter[i+3];
      }
//      System.out.printf("Bary %f sec %f km/s ...\n",
//          barytcor, baryvcor);
      baryjd = w.when.jd + barytcor / 86400.;
   }
}