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

import java.util.*;
import java.util.List.*;
/*================================================================================================
/         Class Declaration
/=================================================================================================*/
public class InstantInTime implements Cloneable
/** The instance variable jd is the true jd (always), and there are
    two GenericCalDat instances, UTDate and localDate.  Instance
    variables stdz and useDST are the std zone offset (hours west) and an
    integer encoding which daylight savings convention is used.
    FindDSTBounds figures out whether DST is actually in effect.
**/

{
   public double jd;   /* the real JD, always. */

   public GenericCalDat UTDate;
   public GenericCalDat localDate;
   double localjd;   /* used transiently ... */
   double stdz;   // hours west
   int useDST;
   public boolean dstInEffect = false;
   double [] DSTBounds = {0.,0};

   static double TheEpoch = 2440587.50000;  // Jan 1, 1970 0h UT
/*================================================================================================
/
/=================================================================================================*/
   public InstantInTime (double stdzin, int use_dst) {
        // default constructor sets to system time ...
        stdz = stdzin;  useDST = use_dst;
        SetInstant( stdzin, use_dst );
   }
/*================================================================================================
/
/=================================================================================================*/
   public InstantInTime( String s, double stdzin,
           int use_dst, boolean is_ut ) {
        /* allowed formats are "yyyy mo dd hh mm ss" and "mo" can
           be a three-letter abbreviation. */
        stdz = stdzin;  useDST = use_dst;
        SetInstant(s, stdzin, use_dst, is_ut);
   }
/*================================================================================================
/
/=================================================================================================*/
   public InstantInTime( double jdin, double stdzin,
           int use_dst, boolean is_ut ) {
        /* allowed formats are "yyyy mo dd hh mm ss" and "mo" can
           be a three-letter abbreviation. */
        stdz = stdzin;  useDST = use_dst;
        SetInstant(jdin, stdzin, use_dst, is_ut);
   }
/*================================================================================================
/
/=================================================================================================*/
   public void SetInstant( double stdzin, int use_dst) {
        // when no time specified, sets to system time.
        long milliseconds;
        double jdnow;

        stdz = stdzin;  useDST = use_dst;
        milliseconds = System.currentTimeMillis();
        jdnow = TheEpoch + milliseconds / 86400000.;
        SetInstant(jdnow, stdzin, use_dst, true);
   }
/*================================================================================================
/
/=================================================================================================*/
   public InstantInTime clone() {
       try {
         InstantInTime copy = (InstantInTime) super.clone();
            copy.jd = jd;
            copy.UTDate = (GenericCalDat) UTDate.clone();
            copy.localDate = (GenericCalDat) localDate.clone();
            copy.localjd = localjd;
            copy.stdz = stdz;
            copy.useDST = useDST;
            copy.dstInEffect = dstInEffect;
            copy.DSTBounds = DSTBounds;
            return copy;
       } catch (CloneNotSupportedException e) {
          throw new Error("This should never happen!");
       }
   }
/*================================================================================================
/
/=================================================================================================*/
  public void SetInstant( String s, double stdzin, int use_dst, boolean is_ut) {

      useDST = use_dst;

      if(is_ut) {
          UTDate = new GenericCalDat(s);
          jd = UTDate.Cal2JD();
          // System.out.printf("Setting to UT, s = %s,jd = %f\n",s,jd);
          // The DST calculation is not sensitive to year if dstInEffect is
          // computed using the same year as used in findDSTBounds.
          if(use_dst == 0) dstInEffect = false;
          else {  // determine if DST is in effect ...
             DSTBounds = findDSTBounds(UTDate.year, stdzin, use_dst);
             if(use_dst > 0)  {  // northern hemisphere logic
                if(jd > DSTBounds[0] & jd < DSTBounds[1]) dstInEffect = true;
                else dstInEffect=false;
             }
             if(use_dst < 0) {  // southern hemisphere logic
                if(jd < DSTBounds[0] | jd > DSTBounds[1]) dstInEffect = true;
                else dstInEffect = false;
             }
             // System.out.printf("use_dst %d jd %f Bounds[0,1] = %f %f\n",
               //   use_dst,jd,DSTBounds[0],DSTBounds[1]);
            // if (dstInEffect) System.out.printf("DST is in effect.\n");
            // else System.out.printf("DST is NOT in effect.\n");
          }

          if(dstInEffect) {
              localjd = jd - (stdzin - 1.) / 24.;
            //  System.out.printf("setting localjd using DST\n");
          }
          else {
              localjd = jd - (stdzin / 24.);
            //  System.out.printf("Setting localjd using std time\n");
          }
          localDate = new GenericCalDat(localjd);

      }
      else {  // input string is local date and time.
          localDate = new GenericCalDat(s);  // by definition ..
          localjd = localDate.Cal2JD();
          // System.out.printf("using localjd = %f\n",localjd);
          if(use_dst == 0) dstInEffect = false;
          else { // dst is used if applicable ... use local-time limits of
                 // applicability {DSTBounds [2] and [3] instead of [0] and [1])
             DSTBounds = findDSTBounds(localDate.year, stdzin, use_dst);
             if(use_dst > 0)  {  // northern hemisphere logic
                if(localjd > DSTBounds[2] & localjd < DSTBounds[3]) dstInEffect = true;
                else dstInEffect = false;
             }
             if(use_dst < 0) {  // southern hemisphere logic
                if(localjd < DSTBounds[2] | localjd > DSTBounds[3]) dstInEffect = true;
                else dstInEffect = false;
             }
          }

          if(dstInEffect) jd = localjd + (stdzin - 1.) / 24.;
          else jd = localjd + stdzin / 24.;
          // System.out.printf("Setting jd to %f\n",jd);
          UTDate = new GenericCalDat(jd);
       }
   }
/*================================================================================================
/
/=================================================================================================*/
  public void AdvanceTime(String s, double stdzin, int use_dst_in) {
      String [] fields ;
      String lastfield;
      double inputdelta, delta;
      int nf;

      stdz = stdzin;
      useDST = use_dst_in;

      fields = s.split("\\s+");  // whitespace

      nf = fields.length;
      inputdelta = Double.parseDouble(fields[0]);
      if(nf > 1)
          lastfield = fields[nf - 1].toLowerCase();
      else lastfield = "h";   // defaults to hours

      // use first character to tell us what the unit is
//      System.out.printf("last field %s  lastfield.substring(0,1) :%s:\n",
 //          lastfield,lastfield.substring(0,1));
      if(lastfield.startsWith("h")) delta = inputdelta / 24.;
      else if(lastfield.startsWith("m")) delta = inputdelta / 1440.;
      else if(lastfield.startsWith("s")) delta = inputdelta / 86400.;
      else if(lastfield.startsWith("d")) delta = inputdelta;
      else if(lastfield.startsWith("t")) delta = inputdelta / 1.0027379093;
           // 1 sidereal day
      else if(lastfield.startsWith("l")) delta = 29.5307 * inputdelta;
           // 1 lunation
      else if(lastfield.startsWith("y")) delta = 365. * inputdelta;
      else delta = inputdelta / 24.;

      SetInstant(jd + delta, stdz, useDST, true);
         // jd is always UT, so use_dst is true

   }
/*================================================================================================
/
/=================================================================================================*/
  public void AdvanceTime(String s, double stdzin, int use_dst_in, boolean forward) {
      String [] fields ;
      String lastfield;
      double inputdelta, delta;
      int nf;

      stdz = stdzin;
      useDST = use_dst_in;

      fields = s.split("\\s+");  // whitespace

      nf = fields.length;
      inputdelta = Double.parseDouble(fields[0]);
      if(nf > 1)
          lastfield = fields[nf - 1].toLowerCase();
      else lastfield = "h";   // defaults to hours

      // use first character to tell us what the unit is
//      System.out.printf("last field %s  lastfield.substring(0,1) :%s:\n",
 //          lastfield,lastfield.substring(0,1));
      if(lastfield.startsWith("h")) delta = inputdelta / 24.;
      else if(lastfield.startsWith("m")) delta = inputdelta / 1440.;
      else if(lastfield.startsWith("s")) delta = inputdelta / 86400.;
      else if(lastfield.startsWith("d")) delta = inputdelta;
      else if(lastfield.startsWith("t")) delta = inputdelta / 1.0027379093;
           // 1 sidereal day
      else if(lastfield.startsWith("l")) delta = 29.5307 * inputdelta;
           // 1 lunation
      else if(lastfield.startsWith("y")) delta = 365. * inputdelta;
      else delta = inputdelta / 24.;
/*      System.out.println("AdvanceTime, delta = " + String.format(Locale.ENGLISH, "%f",delta) +
               "forward = " + forward); */

      if(forward) SetInstant(jd + delta, stdz, useDST, true);
         // jd is always UT, so use_dst is true
      else {
         SetInstant(jd - delta, stdz, useDST, true);
        // System.out.printf("AdvanceTime: JD, delta %f %f  stdz %f useDST %d\n",
         //          jd,delta,stdz,useDST);
      }
   }
/*================================================================================================
/
/=================================================================================================*/
  public void SetInstant( double jdin, double stdzin, int use_dst, boolean is_ut) {
      // Silly to repeat all that code just to change datatype ...
      // but it'll work correctly at least.

      useDST = use_dst;

      if(is_ut) {
          jd = jdin;  // by definition, it's the JD
          UTDate = new GenericCalDat(jd);
          // The DST calculation is not sensitive to year if dstInEffect is
          // computed using the same year as used in findDSTBounds.
          if(use_dst == 0) dstInEffect = false;
          else {  // determine if DST is in effect ...
             DSTBounds = findDSTBounds(UTDate.year, stdzin, use_dst);
             if(use_dst > 0)  {  // northern hemisphere logic
                if(jd > DSTBounds[0] & jd < DSTBounds[1]) dstInEffect = true;
                else dstInEffect=false;
             }
             if(use_dst < 0) {  // southern hemisphere logic
                if(jd < DSTBounds[0] | jd > DSTBounds[1]) dstInEffect = true;
                else dstInEffect = false;
             }
             // System.out.printf("use_dst %d jd %f Bounds[0,1] = %f %f\n",
               //   use_dst,jd,DSTBounds[0],DSTBounds[1]);
             // if (dstInEffect) System.out.printf("DST is in effect.\n");
             // else System.out.printf("DST is NOT in effect.\n");
          }

          if(dstInEffect) localjd = jd - (stdzin - 1.) / 24.;
          else localjd = jd - (stdzin / 24.);
          localDate = new GenericCalDat(localjd);
      }
      else {  // input string is local date and time.
          localDate = new GenericCalDat(jdin);  // by definition ..
          localjd = localDate.Cal2JD();
          if(use_dst == 0) dstInEffect = false;
          else { // dst is used if applicable ... use local-time limits of
                 // applicability (DSTBounds [2] and [3] instead of [0] and [1])
             DSTBounds = findDSTBounds(localDate.year, stdzin, use_dst);
             if(use_dst > 0)  {  // northern hemisphere logic
                if(localjd > DSTBounds[2] & localjd < DSTBounds[3]) dstInEffect = true;
                else dstInEffect = false;
             }
             if(use_dst < 0) {  // southern hemisphere logic
                if(localjd < DSTBounds[2] | localjd > DSTBounds[3]) dstInEffect = true;
                else dstInEffect = false;
             }
          }

          if(dstInEffect) jd = localjd + (stdzin - 1.) / 24.;
          else jd = localjd + stdzin / 24.;
          UTDate = new GenericCalDat(jd);
       }
   }
/*================================================================================================
/
/=================================================================================================*/
  public double [] findDSTBounds(int year, double stdz, int use_dst) {
      /** returns jdb and jde, first and last dates for dst in year.
          [0] and [1] are true JD, [2] and [3] are reckoned wrt local time.
          This proved to be useful later.
         The parameter use_dst allows for a number
            of conventions, namely:
                0 = don't use it at all (standard time all the time)
                1 = use USA convention (1st Sun in April to
                     last Sun in Oct 1986 - 2006; last Sun in April before;
                     2nd sunday in March to first in Nov from 2007 on.)
                2 = use Spanish convention (for Canary Islands)
                -1 = use Chilean convention (CTIO).
                -2 = Australian convention (for AAT).
            Negative numbers denote sites in the southern hemisphere,
            where jdb and jde are beginning and end of STANDARD time for
            the year.
            It's assumed that the time changes at 2AM local time; so
            when clock is set ahead, time jumps suddenly from 2 to 3,
            and when time is set back, the hour from 1 to 2 AM local
            time is repeated.  This could be changed in code if need be.
         Straight translation of skycalc c routine.
      **/
       int nSundays = 0;
       int nSaturdays = 0;  // for Chile, keep descriptive name
       int trialday = 1;
       int trialdow = 0;
       String trialstring;
       double [] JDBoundary = {0.,0.,0.,0.};
       double jdtmp;

       GenericCalDat trial = new GenericCalDat("2000 1 1 1 1 1");
       if (use_dst == 0) {  // if DST is not used this shouldn't matter.
          return JDBoundary;  // this is a common case, get outta here fast.
       }

       else if (use_dst == 1) {
          // USA conventions from mid-60s (?) onward.
          // No attempt to account for energy-crisis measures (Nixon admin.)
          if(year < 1986) {  // last sunday in April
             trialday = 30;
             while(trialdow != 6) {
                trialstring = String.format(Locale.ENGLISH, "%d 4 %d 2 0 0",year,trialday);
                trial.CalFromString(trialstring);
                trialdow = trial.DayOfWeek();
                trialday--;
             }
          }
          else if(year <= 2006) {  // first Sunday in April
             trialday = 1;
             trialdow = 0;
             while(trialdow != 6) {
                trialstring = String.format(Locale.ENGLISH, "%d 4 %d 2 0 0",year,trialday);
                trial.CalFromString(trialstring);
                trialdow = trial.DayOfWeek();
                trialday++;
             }
          }
          else {  // 2007 and after, it's 2nd Sunday in March ....
             nSundays = 0;
             trialday = 1;
             trialdow = 0;
             while(nSundays < 2) {
                trialstring = String.format(Locale.ENGLISH, "%d 3 %d 2 0 0",year,trialday);
                trial.CalFromString(trialstring);
                trialdow = trial.DayOfWeek();
                if(trialdow == 6) nSundays++;
                trialday++;
             }
          }
          jdtmp = trial.Cal2JD();
          JDBoundary[0] = jdtmp + stdz / 24.;  // true JD of change.
          JDBoundary[2] = jdtmp;  // local-time version


          trialdow = 0;  // for next round

          if(year < 2007) {  // last Sunday in October
             trialday = 31;
             while(trialdow != 6) {
                trialstring = String.format(Locale.ENGLISH, "%d 10 %d 2 0 0",year,trialday);
                trial.CalFromString(trialstring);
                trialdow = trial.DayOfWeek();
                trialday--;
             }
          }
          else {   // first Sunday in November, didn't change in 1986.
             trialday = 1;
             trialdow = 0;
             while(trialdow != 6) {
                trialstring = String.format(Locale.ENGLISH, "%d 11 %d 2 0 0",year,trialday);
                trial.CalFromString(trialstring);
                trialdow = trial.DayOfWeek();
                trialday++;
             }
          }
          jdtmp = trial.Cal2JD();
          JDBoundary[1] = jdtmp + (stdz - 1.) / 24.; // true JD
          JDBoundary[3] = jdtmp;  // local-time version
       }

       else if(use_dst == 2) { // EU convention
           trialday = 31; // start last Sunday in March at 1 AM
           while(trialdow != 6) {
              trialstring = String.format(Locale.ENGLISH, "%d 3 %d 1 0 0",year,trialday);
              trial.CalFromString(trialstring);
              trialdow = trial.DayOfWeek();
              trialday--;
           }
           jdtmp = trial.Cal2JD();
           JDBoundary[0] = jdtmp + stdz / 24.;
           JDBoundary[2] = jdtmp;

           trialday = 30; // end last Sunday in October at 1 AM
           trialdow = 0;
           while(trialdow != 6) {
              trialstring = String.format(Locale.ENGLISH, "%d 10 %d 1 0 0",year,trialday);
              trial.CalFromString(trialstring);
              trialdow = trial.DayOfWeek();
              trialday--;
           }
           jdtmp = trial.Cal2JD();
           JDBoundary[1] = jdtmp + (stdz - 1.) / 24.;
           JDBoundary[3] = jdtmp;
       }
       else if(use_dst == -1) { // Chile - negative -> southern
           // In the south, [0] and [2] -> March-ish -> END of DST
           //               [1] and [3] -> Octoberish -> BEGIN DST

           trialday = 1;  // starts at 24h on 2nd Sat. of Oct.
           nSaturdays = 0;
           while(nSaturdays != 2) {
              trialstring = String.format(Locale.ENGLISH, "%d 10 %d 23 0 0",year,trialday);
                  // use 11 pm for day calculation to avoid any ambiguity
              trial.CalFromString(trialstring);
              trialdow = trial.DayOfWeek();
              if(trialdow == 5) nSaturdays++;
              trialday++;
           }
           jdtmp = trial.Cal2JD();
           JDBoundary[1] = jdtmp + (stdz + 1.) / 24.;
           JDBoundary[3] = jdtmp;
              // add the hour back here (DST start)

           nSaturdays = 0;
           trialday = 1;
           while(nSaturdays != 2) { // end on the 2nd Sat in March
              trialstring = String.format(Locale.ENGLISH, "%d 3 %d 23 0 0",year,trialday);
                  // use 11 pm for day calculation to avoid any ambiguity
              trial.CalFromString(trialstring);
              trialdow = trial.DayOfWeek();
              if(trialdow == 5) nSaturdays++;
              trialday++;
           }
           jdtmp = trial.Cal2JD();
           JDBoundary[0] = jdtmp + stdz / 24.;
           JDBoundary[2] = jdtmp;
              // no need to add the hour back, DST is ending now.
       }

       else if(use_dst == -2) {  // Australia (NSW)
           trialday = 31;
           trialdow = 0;
           while(trialdow != 6) { // DST begins at 2 AM, Last sun in Oct
              trialstring = String.format(Locale.ENGLISH, "%d 10 %d 2 0 0",year,trialday);
              trial.CalFromString(trialstring);
              trialdow = trial.DayOfWeek();
              trialday--;
           }
           jdtmp = trial.Cal2JD();
           JDBoundary[1] = jdtmp + stdz / 24.;
           JDBoundary[3] = jdtmp;

           trialday = 31;
           trialdow = 0;
           while(trialdow != 6) { // DST ends at 3 AM, Last sun in March
              trialstring = String.format(Locale.ENGLISH, "%d 3 %d 3 0 0",year,trialday);
              trial.CalFromString(trialstring);
              trialdow = trial.DayOfWeek();
              trialday--;
           }
           jdtmp = trial.Cal2JD();
           JDBoundary[0] = jdtmp + (stdz + 1.) / 24.;
           JDBoundary[2] = jdtmp;
       }
       return JDBoundary;
   }
/*================================================================================================
/
/=================================================================================================*/
 public double JulianEpoch() {
       return 2000. + (jd - Const.J2000) / 365.25;   // as simple as that ...
   }
}

