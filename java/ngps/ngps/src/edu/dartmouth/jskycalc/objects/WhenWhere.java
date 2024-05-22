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
import edu.dartmouth.jskycalc.coord.InstantInTime;
import edu.dartmouth.jskycalc.coord.RA;
import edu.dartmouth.jskycalc.coord.Site;
import edu.dartmouth.jskycalc.coord.SkyIllum;
import edu.dartmouth.jskycalc.coord.Spherical;
import edu.dartmouth.jskycalc.coord.Topo;
import java.util.*;
import java.util.List.*;
/*================================================================================================
/       class WhenWhere
/=================================================================================================*/
public class WhenWhere implements Cloneable {
   public InstantInTime when;
   public Site where;
   public double sidereal;
   public RA siderealobj;   // for output
   public double [] barycenter = {0.,0.,0.,0.,0.,0.};
     // Barycentric coords in epoch of date; 0-2 are XYZ, 3-5 are velocity.

   public Sun sun;
   public HA hasun;
   public double altsun, azsun;
   public double twilight;

   public Moon moon;    // mostly for use in rise-set calculations.
   public HA hamoon;
   public double altmoon, azmoon;

   public double sunmoon;
   public double moonillum;
   public double cusppa;
/*================================================================================================
/
/=================================================================================================*/
   public void dump() {
      System.out.printf("jd %f  Local %d %d %d  %d %d  UT %d %d %d  %d %d\n",
         when.jd,when.localDate.year,when.localDate.month,when.localDate.day,
         when.localDate.timeofday.hour,when.localDate.timeofday.minute,
         when.UTDate.year,when.UTDate.month,when.UTDate.day,when.UTDate.timeofday.hour,
         when.UTDate.timeofday.minute);
      System.out.printf("lst %f\n",sidereal);
   }
/*================================================================================================
/
/=================================================================================================*/
   public WhenWhere(InstantInTime t, Site loc) {
      when = t;
      where = loc;
      sidereal = lstcalc(when.jd,where.longit.value);
      siderealobj = new RA(sidereal);
      MakeLocalSun();
      MakeLocalMoon();   // these always need instantiation to avoid trouble later.

   }
/*================================================================================================
/
/=================================================================================================*/
   public WhenWhere(double jdin, Site loc) {
      when = new InstantInTime(jdin,loc.stdz,loc.use_dst,true);
      where = loc;
      sidereal = lstcalc(when.jd,where.longit.value);
      siderealobj = new RA(sidereal);
      MakeLocalSun();
      MakeLocalMoon();   // these always need instantiation to avoid trouble later.
   }
/*================================================================================================
/
/=================================================================================================*/
   public void ChangeWhen(double jdin) {
      when.SetInstant(jdin, where.stdz, where.use_dst, true);
      sidereal = lstcalc(when.jd,where.longit.value);
      siderealobj.setRA(sidereal);
   }
/*================================================================================================
/
/=================================================================================================*/
   public void ChangeWhen(String s, boolean is_ut) {
      when.SetInstant(s, where.stdz, where.use_dst,is_ut);
      sidereal = lstcalc(when.jd,where.longit.value);
      siderealobj.setRA(sidereal);
   }
/*================================================================================================
/
/=================================================================================================*/
   public void SetToNow() {
      when.SetInstant(where.stdz,where.use_dst);
      sidereal = lstcalc(when.jd,where.longit.value);
      siderealobj.setRA(sidereal);
   }
/*================================================================================================
/
/=================================================================================================*/
   public void ChangeSite(HashMap<String,Site> hash, String s) {
      // changing site involves synching sidereal and local time
      // so test to see if these are needed.
      Site ss = hash.get(s);
      if(where.equals(ss) == false) {  // there's an equals method for this now ...
         // System.out.printf("Changing site ... .\n");
         where = ss;
         // System.out.printf("Site changed, stdz = %f\n",where.stdz);
         sidereal = lstcalc(when.jd,where.longit.value);
         when.SetInstant(when.jd, where.stdz, where.use_dst, true);
         siderealobj.setRA(sidereal);
      }
      // otherwise do nothing.
      // else System.out.printf("Not changing site ... \n");
   }
/*================================================================================================
/
/=================================================================================================*/
   public void AdvanceWhen(String s) {
      when.AdvanceTime(s, where.stdz, where.use_dst);
      sidereal = lstcalc(when.jd,where.longit.value);
      siderealobj.setRA(sidereal);
   }
/*================================================================================================
/
/=================================================================================================*/
   public void AdvanceWhen(String s, boolean forward) {
      when.AdvanceTime(s, where.stdz, where.use_dst, forward);
      sidereal = lstcalc(when.jd,where.longit.value);
      siderealobj.setRA(sidereal);
   }
/*================================================================================================
/
/=================================================================================================*/
   public Celest zenith2000() {
      Celest c = new Celest(sidereal,where.lat.value,when.JulianEpoch());
      c.selfprecess(2000.);
      return c;
   }
/*================================================================================================
/
/=================================================================================================*/
   public WhenWhere clone() {  // override clone method to make it public
      try {
         WhenWhere copy = (WhenWhere) super.clone();   // this needs to be try/catch to make it work
         copy.when = (InstantInTime) when.clone();
         copy.where = (Site) where.clone();
         copy.sidereal = sidereal;
         copy.siderealobj = (RA) siderealobj.clone();
         copy.barycenter = (double []) barycenter;
         copy.sun = (Sun) sun.clone();
         copy.hasun = (HA) hasun.clone();
         copy.altsun = altsun;
         copy.azsun = azsun;
         copy.twilight = twilight;
         copy.moon = (Moon) moon.clone();
         copy.hamoon = (HA) hamoon.clone();
         copy.altmoon = altmoon;
         copy.azmoon = azmoon;
         copy.sunmoon = sunmoon;
         copy.moonillum = moonillum;
         copy.cusppa = cusppa;
         return copy;
      } catch (CloneNotSupportedException e) {
        throw new Error("This should never happen!\n");
      }
   }
/*================================================================================================
/
/=================================================================================================*/
   public static double lstcalc(double jdin, double longitin) {
      double tt, ut, jdmid, jdint, jdfrac, sid_g, sid;
      long jdintt, sid_int;

      jdintt = (long) jdin;
      jdfrac = jdin - jdintt;
      if (jdfrac < 0.5) {
         jdmid = jdintt - 0.5;
         ut = jdfrac + 0.5;
      }
      else {
         jdmid = jdintt + 0.5;
         ut = jdfrac - 0.5;
      }
      tt = (jdmid - Const.J2000) / 36525;
      sid_g = (24110.54841+8640184.812866*tt+0.093104*tt*tt-6.2e-6*tt*tt*tt)/86400.;
      sid_int = (long) sid_g;
      sid_g = sid_g - (double) sid_int;
      sid_g = sid_g + 1.0027379093 * ut - longitin / 24.;
      sid_int = (long) sid_g;
      sid_g = (sid_g - (double) sid_int) * 24.;
      if(sid_g < 0.) sid_g = sid_g + 24.;
      return sid_g;
   }
/*================================================================================================
/
/=================================================================================================*/
   // Pass in a previously-computed planets and sun for this ...
   // Planets and sun are assumed up-to-date.

   public void baryxyzvel(Planets p, Sun s) {

       int i;
       double [] geopos;

       // need sunvel now so get it ...

 //      System.out.printf("into baryxyzvel, jd = %f\n",when.jd);
       s.sunvel(when.jd);  // compute sun velocity  ...

//       System.out.printf("Helio Vxyz sun: %f %f %f  %f\n",
 //         s.xyzvel[0] * Const.KMS_AUDAY,
  //        s.xyzvel[1] * Const.KMS_AUDAY,
   //       s.xyzvel[2] * Const.KMS_AUDAY, when.jd);

       p.ComputeBaryCor(); // compute offset of barycenter from heliocenter

//       System.out.printf("sun xyz  %f %f %f\n",s.xyz[0],s.xyz[1],s.xyz[2]);
//       System.out.printf(" baryc   %f %f %f\n",p.barycor[0],p.barycor[1],p.barycor[2]);
       for(i = 0; i < 3; i++) {
          barycenter[i] = (-1. * s.xyz[i] - p.barycor[i]) *
                     Const.LIGHTSEC_IN_AU;
          barycenter[i+3] = (-1. * s.xyzvel[i] - p.barycor[i+3]) *
                      Const.KMS_AUDAY;
//          System.out.printf("pre-topo: %d   %f   %f \n",i,
//              barycenter[i] / Const.LIGHTSEC_IN_AU,
//                                                barycenter[i+3]);
       }

       // add in the topocentric velocity ... note use of sidereal for longit

       geopos = Topo.Geocent(sidereal, where.lat.value, where.elevsea);

//       System.out.printf("Geopos: %f %f %f\n",geopos[0],geopos[1],geopos[2]);
//       System.out.printf("topo corrn vx vy %f %f\n",
//          Const.OMEGA_EARTH * geopos[1] * Const.EARTHRAD_IN_KM,
//          Const.OMEGA_EARTH * geopos[0] * Const.EARTHRAD_IN_KM);

       // rotation vel is vector omega crossed into posn vector

       barycenter[3] -= Const.OMEGA_EARTH * geopos[1] * Const.EARTHRAD_IN_KM ;
       barycenter[4] += Const.OMEGA_EARTH * geopos[0] * Const.EARTHRAD_IN_KM ;

   }
/*================================================================================================
/
/=================================================================================================*/
   public void MakeLocalSun() {
       //System.out.printf("Making a new sun, jd = %f\n",when.jd);
       sun = new Sun(this);
       //System.out.printf("Made sun, sidereal = %f\n",sidereal);
       hasun = new HA(sidereal - sun.topopos.Alpha.value);
       double [] altazpar = Observation.altit(sun.topopos.Delta.value, hasun.value,
              where.lat.value);
       altsun = altazpar[0];
       azsun = altazpar[1];
       twilight = SkyIllum.ztwilight(altsun);
       //System.out.printf("Made a new sun: %s alt %f\n",sun.topopos.checkstring(),altazpar[0]);
   }
/*================================================================================================
/
/=================================================================================================*/
   public void UpdateLocalSun() {
       sun.update(when,where,sidereal);
       hasun = new HA(sidereal - sun.topopos.Alpha.value);
       double [] altazpar = Observation.altit(sun.topopos.Delta.value, hasun.value,
              where.lat.value);
       altsun = altazpar[0];
       azsun = altazpar[1];
       twilight = SkyIllum.ztwilight(altsun);
//       System.out.printf("Updated sun: %s %f\n",sun.topopos.checkstring(),altazpar[0]);
   }
/*================================================================================================
/
/=================================================================================================*/
   public void MakeLocalMoon() {
      moon = new Moon(this);
      hamoon = new HA(sidereal - moon.topopos.Alpha.value);
      double [] altazpar = Observation.altit(moon.topopos.Delta.value, hamoon.value,
              where.lat.value);
      altmoon = altazpar[0];
      azmoon = altazpar[1];
//      sunmoon = Spherical.subtend(sun.topopos,moon.topopos);
//      moonillum = 0.5 * (1. - Math.cos(sunmoon));
//      sunmoon *= Const.DEG_IN_RADIAN;
//      System.out.printf("Made a new moon: %s HA %s alt %f\n",moon.topopos.checkstring(),
//          hamoon.RoundedHAString(0,":"),altazpar[0]);
   }
/*================================================================================================
/
/=================================================================================================*/
   public void UpdateLocalMoon() {
      moon.update(when,where,sidereal);
      hamoon = new HA(sidereal - moon.topopos.Alpha.value);
      double [] altazpar = Observation.altit(moon.topopos.Delta.value, hamoon.value,
              where.lat.value);
      altmoon = altazpar[0];
      azmoon = altazpar[1];
      // call this after updating the sun ...
//      sunmoon = Spherical.subtend(sun.topopos,moon.topopos);
//      moonillum = 0.5 * (1. - Math.cos(sunmoon));
//      sunmoon *= Const.DEG_IN_RADIAN;

//      System.out.printf("Updated the moon: %s HA %s alt %f\n",moon.topopos.checkstring(),
//          hamoon.RoundedHAString(0,":"),altazpar[0]);
   }
/*================================================================================================
/
/=================================================================================================*/
   public void ComputeSunMoon() {
      double retvals[];

      UpdateLocalSun();
      UpdateLocalMoon();
      retvals = Spherical.CuspPA(sun.topopos,moon.topopos);
      sunmoon = retvals[1];
      moonillum = 0.5 * (1. - Math.cos(sunmoon));
      sunmoon *= Const.DEG_IN_RADIAN;
      cusppa = retvals[0];  // radians ...
   }
}