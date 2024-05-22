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
import edu.dartmouth.jskycalc.coord.Const;
import java.util.*;
import java.util.List.*;
/*================================================================================================
/
/=================================================================================================*/
public class Seasonal {
   double [] jdnew;
   double [] jdfull;   // new and full moon dates bracketing the present
   double ha_at_center;       // hour angle at night center
   public Object [][] tabledata;
   static Observation lastcomputedobs;
   boolean wasupdated = false;
/*================================================================================================
/
/=================================================================================================*/
   public Seasonal(Observation obs) {  // just sets up arrays
      int i;
      jdnew = new double [8];
      jdfull = new double [8];
      tabledata = new Object [16][11];
      lastcomputedobs = (Observation) obs.clone();
      this.Update(obs);  // for constructor, go ahead and do it ...
   }
/*================================================================================================
/
/=================================================================================================*/
   public void Update(Observation obs) {
      int lun, i;
      double [] xy;
      double min_alt, max_alt;
      String [] hoursup = {" "," "," "};

      // check to see if anything has changed significantly, otherwise skip it.
      if(Math.abs(lastcomputedobs.w.when.jd - obs.w.when.jd) > 5. ||
        !obs.c.equals(lastcomputedobs.c) || !obs.w.where.equals(lastcomputedobs.w.where))  {

         Observation oseason = (Observation) obs.clone();
         NightlyAlmanac ng = new NightlyAlmanac(oseason.w);

         xy = Spherical.min_max_alt(oseason.w.where.lat.value, oseason.current.Delta.value);
         min_alt = xy[0];
         max_alt = xy[1];

         // run computations for new and full moon in a roughly +- 3-month interval

         int lunstart = Moon.lunation(oseason.w.when.jd) - 3;
         for(i = 0; i < 8; i++) {

            lun = lunstart + i;

            jdnew[i] = Moon.flmoon(lun,0);
            oseason.w.ChangeWhen(jdnew[i]);

            ng.Update(oseason.w);
            tabledata[2 * i][0] =  " New ";
            // tabulated date is EVENING DATE of the night closest to the instant of phase ...
            tabledata[2 * i][1] = (Object) ng.sunset.when.localDate.RoundedCalString(3,0);
            // System.out.printf("New : %s  ",oseason.w.when.localDate.RoundedCalString(1,0));

            // System.out.printf(" sunrise: %s\n",ng.sunrise.when.localDate.RoundedCalString(1,0));

            oseason.w.ChangeWhen(ng.eveningTwilight.when.jd);
            oseason.ComputeSky();
            tabledata[2 * i][2] = oseason.ha.RoundedHAString(-2,":");
            tabledata[2 * i][3] = airmassstring(oseason.altitude,oseason.airmass);

            oseason.w.ChangeWhen(ng.nightcenter.when.jd);
            oseason.ComputeSky();
            tabledata[2 * i][4] = oseason.ha.RoundedHAString(-2,":");
            tabledata[2 * i][5] = airmassstring(oseason.altitude,oseason.airmass);
            ha_at_center = oseason.ha.value;  // store for later

            oseason.w.ChangeWhen(ng.morningTwilight.when.jd);
            oseason.ComputeSky();
            tabledata[2 * i][6] = oseason.ha.RoundedHAString(-2,":");
            tabledata[2 * i][7] = airmassstring(oseason.altitude,oseason.airmass);

            hoursup = NightHoursAboveAirmass(oseason, ng, min_alt, max_alt);
            tabledata[2 * i][8] = hoursup[0];
            tabledata[2 * i][9] = hoursup[1];
            tabledata[2 * i][10] = hoursup[2];

            jdfull[i] = Moon.flmoon(lun,2);
            oseason.w.ChangeWhen(jdfull[i]);

            ng.Update(oseason.w);
            tabledata[2 * i + 1][0] = " Full";
            tabledata[2 * i + 1][1] = ng.sunset.when.localDate.RoundedCalString(3,0);
            // System.out.printf("Full: %s  ",oseason.w.when.localDate.RoundedCalString(1,0));

            ng.Update(oseason.w);
            // System.out.printf(" sunrise: %s\n",ng.sunrise.when.localDate.RoundedCalString(1,0));

            oseason.w.ChangeWhen(ng.eveningTwilight.when.jd);
            oseason.ComputeSky();
            tabledata[2 * i + 1][2] = oseason.ha.RoundedHAString(-2,":");
            tabledata[2 * i + 1][3] = airmassstring(oseason.altitude,oseason.airmass);

            oseason.w.ChangeWhen(ng.nightcenter.when.jd);
            oseason.ComputeSky();
            tabledata[2 * i + 1][4] = oseason.ha.RoundedHAString(-2,":");
            tabledata[2 * i + 1][5] = airmassstring(oseason.altitude,oseason.airmass);
            ha_at_center = oseason.ha.value;  // store for later

            oseason.w.ChangeWhen(ng.morningTwilight.when.jd);
            oseason.ComputeSky();
            tabledata[2 * i + 1][6] = oseason.ha.RoundedHAString(-2,":");
            tabledata[2 * i + 1][7] = airmassstring(oseason.altitude,oseason.airmass);

            hoursup = NightHoursAboveAirmass(oseason, ng, min_alt, max_alt);
            tabledata[2 * i + 1][8] = hoursup[0];
            tabledata[2 * i + 1][9] = hoursup[1];
            tabledata[2 * i + 1][10] = hoursup[2];

         }
         wasupdated = true;
         lastcomputedobs = obs.clone();  // copy computed obs for use in skip condition
         // dump();
      }
      else {
         wasupdated = false ;
         // System.out.printf("no seasonal update.\n");  // diagn. used to check skip condition.
      }
   }
/*================================================================================================
/
/=================================================================================================*/
   public String airmassstring(double altitude, double airmass) {
      // this happens so many times that it's worth writing a method.
      if(altitude < 0.) return "(down)";
      else if (airmass > 10.) return "> 10.";
      else return String.format(Locale.ENGLISH, "%5.2f",airmass);
   }
/*================================================================================================
/
/=================================================================================================*/
  public double hrs_up(double jdup, double jddown, double jdeve, double jdmorn) {
     /* an object comes up past a given point at jdup, and goes down at jddown,
        with twilight jdeve and jdmorn.  Computes how long object is up *and* it's
        dark.  Transcribed without significant modification from _skysub.c  ... */

      double jdup2, jddown0;

      if(jdup < jdeve) {
        if(jddown >= jdmorn)   /* up all night */
           return ((jdmorn - jdeve) * 24.);
        else if(jddown >= jdeve)  {
           /* careful here ... circumpolar objects can come back *up*
              a second time before morning.  jdup and jddown are
              the ones immediately preceding and following the upper
              culmination nearest the center of the night, so "jdup"
              can be on the previous night rather than the one we want. */
          jdup2 = jdup + 1.0/Const.SID_RATE;
          if(jdup2 > jdmorn)  /* the usual case ... doesn't rise again */
              return ((jddown - jdeve) * 24.);
          else return(((jddown - jdeve) + (jdmorn - jdup2)) * 24.);
        }
        else return(0.);
      }
      else if(jddown > jdmorn) {
        if(jdup >= jdmorn) return(0.);
        else {
           /* again, a circumpolar object can be up at evening twilight
               and come 'round again in the morning ... */
           jddown0 = jddown - 1.0/Const.SID_RATE;
           if(jddown0 < jdeve) return((jdmorn - jdup) * 24.);
           else return(((jddown0 - jdeve) + (jdmorn - jdup)) * 24.);
        }
      }
      else return((jddown - jdup)*24.);  /* up & down the same night ...
         might happen a second time in pathological cases, but this will
         be extremely rare except at very high latitudes.  */
   }
/*================================================================================================
/
/=================================================================================================*/
   public void dump() {
      int i, j;
      for(i = 0; i < 16; i++) {
         for(j = 0; j < 11; j++) {
            System.out.printf("%8s ",tabledata[i][j]);
         }
         System.out.printf("\n");
      }
   }
/*================================================================================================
/
/=================================================================================================*/
   public String [] NightHoursAboveAirmass(Observation obs, NightlyAlmanac ng, double min_alt, double max_alt) {
      double [] critical_alt = {19.2786, 29.8796, 41.7592};
          // altitudes above horizon at which true airmass = 1.5, 2, and 3 respectively.
      String [] retvals = {" "," "," "};   // night hours above critical altitude
      double jdtrans;   // jd of transit closest to midnight
      double [] dt = {0.,0.,0.};
      double [] jdup = {0.,0.,0.};
      double [] jddown = {0.,0.,0.};
      int i;

      WhenWhere diagn = obs.w.clone();

      // jd of transit nearest midnight

      jdtrans = ng.nightcenter.when.jd - ha_at_center / (24. * Const.SID_RATE);

//      diagn.ChangeWhen(jdtrans);
//      System.out.printf("diagnostic: transit at local time %s\n",
//             diagn.when.localDate.RoundedCalString(0,0));

      for(i = 0; i < 3; i++) {
          if((min_alt < critical_alt[i]) && (max_alt > critical_alt[i])) { // passes this altitude
             dt[i] = Spherical.ha_alt(obs.current.Delta.value,obs.w.where.lat.value,critical_alt[i]) /
                          (Const.SID_RATE * 24.);
             jdup[i] = jdtrans - dt[i];
             jddown[i] = jdtrans + dt[i];
          }
          else {
             jdup[i] = 0.; jddown[i] = 0.;
             if(min_alt < critical_alt[i]) dt[i] = 0.;
             else dt[i] = 12.;
          }

          // let's just hope twilight occurs for now ...

          if(jdup[i] != 0.)
             // passes the relevant airmass
             retvals[i] = String.format(Locale.ENGLISH, "%4.1f",hrs_up(jdup[i],jddown[i],ng.eveningTwilight.when.jd,
                               ng.morningTwilight.when.jd));
             // always remains above the relevant altitude
          else if (min_alt > critical_alt[i])
             retvals[i] = String.format(Locale.ENGLISH, "%4.1f",
                      24. * (ng.morningTwilight.when.jd - ng.eveningTwilight.when.jd));
             // never rises above the relevant altitude
          else retvals[i] = String.format(Locale.ENGLISH, "0.0");

      }

      return retvals;

   }
}
