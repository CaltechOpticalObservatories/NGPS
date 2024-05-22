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
import edu.dartmouth.jskycalc.coord.Spherical;
import edu.dartmouth.jskycalc.coord.Site;
import edu.dartmouth.jskycalc.coord.InstantInTime;
import edu.dartmouth.jskycalc.coord.Const;
import java.util.List.*;

/*================================================================================================
/
/=================================================================================================*/
public class NightlyAlmanac {
   /** For finding timing of various phenomena, esp sun and moon rise and set. */

   public WhenWhere midnight;
   public WhenWhere sunrise;
   public WhenWhere sunset;
   public WhenWhere moonrise;
   public WhenWhere moonset;
   public WhenWhere eveningTwilight;
   public WhenWhere morningTwilight;
   public WhenWhere nightcenter;
/*================================================================================================
/
/=================================================================================================*/
   public static double jd_sun_alt(double alt, WhenWhere wIn) {
   /**  finds the jd at which the sun is at altitude alt, given initial guess handed
        in with a whenwhere.  */
      double jdguess, lastjd;
      double deriv, err, del = 0.002;
      double alt2, alt3;
      int i = 0;

      WhenWhere w = (WhenWhere) wIn.clone();

   /* Set up calculation, then walk in with Newton-Raphson scheme (guess and
      check using numerical derivatives). */

      jdguess = w.when.jd;
//      System.out.printf("Before makelocalsun, w.when.jd %f\n",w.when.jd);
      w.MakeLocalSun();
      alt2 = w.altsun;
//      System.out.printf("after alt2: w.when.jd %f alt2 %f\n",
 //                  w.when.jd,alt2);
      jdguess = jdguess + del;
      w.ChangeWhen(jdguess);
      w.UpdateLocalSun();
      alt3 = w.altsun;
      err = alt3 - alt;
      deriv = (alt3 - alt2) / del;
//      System.out.printf("alt2 alt3 %f %f  err %f deriv %f\n",
 //             alt2,alt3,err,deriv);
      while((Math.abs(err) > 0.02) && (i < 10)) {
          lastjd = jdguess;
          alt2 = alt3;       // save last guess
          jdguess = jdguess - err/deriv;
          w.ChangeWhen(jdguess);
          w.UpdateLocalSun();
          alt3 = w.altsun;
//          System.out.printf("alt3 %f jdguess %f\n",alt3,jdguess);
          err = alt3 - alt;
          i++;
          deriv = (alt3 - alt2) / (jdguess - lastjd);

          if(i == 9) System.out.printf("jd_sun_alt not converging.\n");

      }
      if (i >= 9) jdguess = -1000.;
//      System.out.printf("Leaving sun w/ wIn %f w %f\n",wIn.when.jd,w.when.jd);
      return jdguess;
   }
/*================================================================================================
/
/=================================================================================================*/
   public static double jd_moon_alt(double alt, WhenWhere wIn) {
   /**  finds the jd at which the moon is at altitude alt, given initial guess handed
        in with a whenwhere.  */
      double jdguess, lastjd;
      double deriv, err, del = 0.002;
      double alt2, alt3;
      int i = 0;

   /* Set up calculation, then walk in with Newton-Raphson scheme (guess and
      check using numerical derivatives). */

      WhenWhere w =  (WhenWhere) wIn.clone();

      // System.out.printf("Into jd_moon_alt, target = %f\n",alt);
      jdguess = w.when.jd;
      w.MakeLocalMoon();
      alt2 = w.altmoon;
      // System.out.printf("after alt2: w.when.jd %f alt2 %f\n",
      //              w.when.jd,alt2);
      jdguess = jdguess + del;
      w.ChangeWhen(jdguess);
      w.UpdateLocalMoon();
      alt3 = w.altmoon;
      err = alt3 - alt;
      deriv = (alt3 - alt2) / del;
      // System.out.printf("alt2 alt3 %f %f  err %f deriv %f\n",
      //        alt2,alt3,err,deriv);
      while((Math.abs(err) > 0.02) && (i < 10)) {
          lastjd = jdguess;
          alt2 = alt3;       // save last guess
          jdguess = jdguess - err/deriv;
          w.ChangeWhen(jdguess);
          w.UpdateLocalMoon();
          alt3 = w.altmoon;
          // System.out.printf("alt3 %f jdguess %f ",alt3,jdguess,deriv);
          err = alt3 - alt;
          i++;
          deriv = (alt3 - alt2) / (jdguess - lastjd);
          // System.out.printf(" err %f deriv %f\n",err,deriv);

          if(i == 9) System.out.printf("jd_moon_alt not converging.\n");

      }
      if (i >= 9) jdguess = -1000.;
      // System.out.printf("Exiting with jdguess = %f\n\n",jdguess);
      return jdguess;
   }
/*================================================================================================
/      NightlyAlmanac(WhenWhere wIn)
/=================================================================================================*/
   public NightlyAlmanac(WhenWhere wIn) {
   /** Computes rise, set, and twilight for the night nearest in time to wIn.when */

      WhenWhere w = (WhenWhere) wIn.clone();

      midnight = new WhenWhere(w.when.clone(),w.where.clone());   // instantiate these ...
      // midnight.MakeLocalSun();
      // midnight.MakeLocalMoon();

      // AAACK!  Have to clone EVERY WHEN, or they are all the SAME WHEN forever.

      sunrise = new WhenWhere(w.when.clone(), w.where.clone());
      sunset = new WhenWhere(w.when.clone(), w.where.clone());
      moonrise = new WhenWhere(w.when.clone(), w.where.clone());
      moonset = new WhenWhere(w.when.clone(), w.where.clone());
      eveningTwilight = new WhenWhere(w.when.clone(), w.where.clone());
      morningTwilight = new WhenWhere(w.when.clone(), w.where.clone());
      nightcenter = new WhenWhere(w.when.clone(), w.where.clone());

      Update(w);
   }
/*================================================================================================
/
/=================================================================================================*/
   public void Update(WhenWhere wIn) {
     /** Computes rise, set, and twilight for the night nearest in time to wIn.when */
     double horiz;    // depression of the horizon in degrees
     double rise_set_alt;
     double dtrise, dtset;
     double jdtemp, jdnoon;
     double twilight_alt = -18.;  // the standard choice for solar altitude at twilight

     // be sure the site infor is up to date

     WhenWhere w = (WhenWhere) wIn.clone();
     w.when = (InstantInTime)  wIn.when.clone();
     w.where = (Site) wIn.where.clone();
     midnight.where = (Site) w.where.clone();
     midnight.when = (InstantInTime) w.when.clone();
     sunrise.where = (Site) w.where.clone();
     sunset.where = (Site) w.where.clone();
     moonrise.where = (Site) w.where.clone();
     moonset.where = (Site) w.where.clone();
     eveningTwilight.where = (Site) w.where.clone();
     morningTwilight.where = (Site) w.where.clone();
     nightcenter.where = (Site) w.where.clone();

     // approx means you can't use a negative elevation for the obs to
     // compensate for higher terrain.

     horiz = Const.DEG_IN_RADIAN *
              Math.sqrt(2. * w.where.elevhoriz / (1000. * Const.EARTHRAD_IN_KM));
     rise_set_alt = -(0.83 + horiz);
        // upper limb of sun and moon rise and set when center of disk is about 50 arcmin
        // below horizon, mostly because of refraction.  Sun and moon are almost the same
        // angular size, and variation in refraction is much larger than ang. size variations.


     // Establish and set the nearest midnight -- previous if before local noon,
     // next midnight if after local noon.

      midnight.when = (InstantInTime) w.when.clone();
//      System.out.printf("Entering almanac with local  %s\n",midnight.when.localDate.RoundedCalString(0,0));
      if(midnight.when.localDate.timeofday.hour >= 12) {
         midnight.when.localDate.timeofday.hour = 23;
         midnight.when.localDate.timeofday.minute = 59;
         midnight.when.localDate.timeofday.second = 59.9;
      }
      else {
         midnight.when.localDate.timeofday.hour = 0;
         midnight.when.localDate.timeofday.minute = 0;
         midnight.when.localDate.timeofday.second = 0.0;
      }
      jdtemp = midnight.when.localDate.Cal2JD();
     // System.out.printf("jdtemp (local) = %f\n",jdtemp);
      midnight.when.SetInstant(jdtemp, w.where.stdz, w.where.use_dst, false);
     // System.out.printf("translates to midnight.jd %f\n",midnight.when.jd);
      double jdmid = midnight.when.jd;   // the real JD
      midnight.ChangeWhen(jdmid);        // to synch sidereal etc.
//      System.out.printf("Midnight set to %s\n", midnight.when.UTDate.RoundedCalString(0,1));
//      System.out.printf("lst at midnight = %f\n",midnight.sidereal);

      midnight.UpdateLocalSun();
      midnight.UpdateLocalMoon();

      // See if the sun rises or sets ...
      double hasunrise = Spherical.ha_alt(midnight.sun.topopos.Delta.value,
              midnight.where.lat.value, rise_set_alt);
//      System.out.printf("hourangle sunrise: %f\n",hasunrise);

      if(hasunrise < 11.8 && hasunrise > 0.2) {
         // if sun grazes horizon, small changes in dec may affect whether it actually
         // rises or sets.  Since dec is computed at midnight, put a little pad on to
         // avoid non-convergent calculations.
        // sunrise = new WhenWhere(jdmid + hasunrise/24.,w.where);

         dtrise = midnight.sun.topopos.Alpha.value - hasunrise -
              midnight.sidereal;

         while (dtrise >= 12.) dtrise -= 24.;
         while (dtrise < -12.) dtrise += 24.;

         dtset = midnight.sun.topopos.Alpha.value + hasunrise -
              midnight.sidereal;
         while (dtset >= 12) dtset -= 24.;
         while (dtset < -12.) dtset += 24.;

//         System.out.printf("going to jd_sun_alt with est sunrise = %f\n",jdmid + dtrise/24.);
         sunrise.ChangeWhen(jdmid + dtrise / 24.);
         sunrise.UpdateLocalSun();
//         System.out.printf("sunrise.when.jd %f, sunrise.altsun %f\n",sunrise.when.jd,
//             sunrise.altsun);
         jdtemp = jd_sun_alt(rise_set_alt,sunrise);
//         System.out.printf("out, sunrise = %f\n",jdtemp);
         sunrise.ChangeWhen(jdtemp);

//         System.out.printf("going to jd_sun_alt with est sunset = %f\n",jdmid + dtset/24.);
         sunset.ChangeWhen(jdmid + dtset/24.);
         sunset.UpdateLocalSun();
         jdtemp = jd_sun_alt(rise_set_alt,sunset);
//         System.out.printf("out, sunset = %f\n",jdtemp);
         sunset.ChangeWhen(jdtemp);
//         System.out.printf("In NightlyAlmanac.Update, sunset set to:\n");
//         sunset.dump();
         nightcenter.ChangeWhen((sunset.when.jd + sunrise.when.jd) / 2.);
      }
      else if (hasunrise < 0.2) {  // may not rise ... set sunrise to noontime to flag.
         if(midnight.when.localDate.timeofday.hour == 23)
            jdnoon = jdmid - 0.5;
         else jdnoon = jdmid + 0.5;
         sunrise.ChangeWhen(jdnoon);
         sunset.ChangeWhen(jdnoon);
         nightcenter.ChangeWhen(jdnoon);
      }
      else if (hasunrise >= 11.8) { // may not set ... set sunset to midnight to flag.
         sunrise.ChangeWhen(jdmid);
         sunset.ChangeWhen(jdmid);
         nightcenter.ChangeWhen(jdmid);
      }

      // Now let's do the same thing for twilight ...
      double hatwilight = Spherical.ha_alt(midnight.sun.topopos.Delta.value,
              midnight.where.lat.value, twilight_alt);
      // System.out.printf("hourangle sunrise: %f\n",hasunrise);
      if(hatwilight < 11.8 && hatwilight > 0.2) {

         dtrise = midnight.sun.topopos.Alpha.value - hatwilight -
              midnight.sidereal;

         while (dtrise >= 12.) dtrise -= 24.;
         while (dtrise < -12.) dtrise += 24.;

         dtset = midnight.sun.topopos.Alpha.value + hatwilight -
              midnight.sidereal;
         while (dtset >= 12) dtset -= 24.;
         while (dtset < -12.) dtset += 24.;

         eveningTwilight.ChangeWhen(jdmid + dtset / 24.);
         eveningTwilight.UpdateLocalSun();
         jdtemp = jd_sun_alt(twilight_alt,eveningTwilight);
         eveningTwilight.ChangeWhen(jdtemp);

         morningTwilight.ChangeWhen(jdmid + dtrise / 24.);
         morningTwilight.UpdateLocalSun();
         jdtemp = jd_sun_alt(twilight_alt,morningTwilight);
         morningTwilight.ChangeWhen(jdtemp);

      }
      else if (hatwilight < 0.2) {  // twilight may not begin ... set to noon to flag.
         if(midnight.when.localDate.timeofday.hour == 23)  // this case will be rare.
            jdnoon = jdmid - 0.5;
         else jdnoon = jdmid + 0.5;
         morningTwilight.ChangeWhen(jdnoon);
         eveningTwilight.ChangeWhen(jdnoon);

      }
      else if (hatwilight >= 11.8) { // twilight may not end (midsummer at high lat).
         morningTwilight.ChangeWhen(jdmid);
         eveningTwilight.ChangeWhen(jdmid);
           // flag -- set twilight to exactly midn.
      }

      // now we tackle the moon ... which is a bit harder.

      double hamoonrise = Spherical.ha_alt(midnight.moon.topopos.Delta.value,
              midnight.where.lat.value, rise_set_alt);

      if(hamoonrise < 11. && hamoonrise > 1.) {
         // The moon moves faster than the sun, so set more conservative limits for
         // proceeding with the calculation.

         dtrise = midnight.moon.topopos.Alpha.value - hamoonrise -
              midnight.sidereal;

         while (dtrise >= 12.) dtrise -= 24.;
         while (dtrise < -12.) dtrise += 24.;

         dtset = midnight.moon.topopos.Alpha.value + hamoonrise -
              midnight.sidereal;
         while (dtset >= 12) dtset -= 24.;
         while (dtset < -12.) dtset += 24.;
/*
         System.out.printf("ra moon midn    %s\n",midnight.moon.topopos.Alpha.RoundedRAString(0,":"));
         System.out.printf("lst at midnight %f\n",midnight.sidereal);
         System.out.printf("rise-set HA %f\n",hamoonrise);
         System.out.printf("dtrise %f dtset %f\n",dtrise,dtset);
*/

         moonrise.ChangeWhen(jdmid + dtrise / 24.);
         moonrise.UpdateLocalMoon();
         jdtemp = jd_moon_alt(rise_set_alt,moonrise);
         moonrise.ChangeWhen(jdtemp);

         moonset.ChangeWhen(jdmid + dtset/24.);
         moonset.UpdateLocalMoon();
         jdtemp = jd_moon_alt(rise_set_alt,moonset);
         moonset.ChangeWhen(jdtemp);
      }
   }
}