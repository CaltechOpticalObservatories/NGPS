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
/
/=================================================================================================*/
public class TimeInterval {
/** A little class for expressing and formatting time intervals. */

   double interval;   // interval, in decimal days.  This is the "root" variable.
   int sign;          // + if t2 is later than t1.
   int days;          // if needed
   sexagesimal s;     // hh mm ss.
/*================================================================================================
/
/=================================================================================================*/
   TimeInterval(InstantInTime t1, InstantInTime t2) {
      // convention:  t1 = now ...
      interval = t2.jd - t1.jd;
      if(interval >= 0.) sign = 1;
      else sign = -1;
      interval = interval * sign;  // force positive
      days = (int) (interval - Math.floor(interval));
      double hrs = 24. * (interval - days);
      s = new sexagesimal(hrs);
   }
/*================================================================================================
/
/=================================================================================================*/
   TimeInterval(double dt) {   // dt, in days; if > 0, it's later than now.
      interval = dt;
      if(interval >= 0.) sign = 1;
      else sign = -1;
      interval = interval * sign;  // force positive
      days = (int) (interval - Math.floor(interval));
      double hrs = 24. * (interval - days);
      s = new sexagesimal(hrs);
   }
/*================================================================================================
/
/=================================================================================================*/
   void reset(InstantInTime t1, InstantInTime t2) {
      interval = t2.jd - t1.jd;
      if(interval >= 0.) sign = 1;
      else sign = -1;
      interval = interval * sign;  // force positive
      days = (int) (interval - Math.floor(interval));
      double hrs = 24. * (interval - days);
      s = new sexagesimal(hrs);
   }
/*================================================================================================
/
/=================================================================================================*/
   void reset(double dt) {
      interval = dt;
      if(interval >= 0.) sign = 1;
      else sign = -1;
      interval = interval * sign;  // force positive
      days = (int) (interval - Math.floor(interval));
      double hrs = 24. * (interval - days);
      s = new sexagesimal(hrs);
   }
/*================================================================================================
/
/=================================================================================================*/
   public String intervalDescr(int ndigits, boolean IsNow) {
      // puts out a nice string description.

      String signdescr;
      String outstr, timestr, daystr;
      String timeformat;
      int runaway = 0;

      if(IsNow) {
         if(sign == 1) signdescr = " from now.";
         else signdescr = " ago.";
      }
      else {
         if(sign == 1) signdescr = " later.";
         else signdescr = " earlier.";
      }

      int daysout = days;
      sexagesimal sexout = s.roundsex(ndigits);

      while(sexout.hour >= 24 && runaway < 3) {
         sexout.hour -= 24;
         daysout += 1;
         runaway++;   // this really shouldn't run away, but catch it if it happens
      }
      if(runaway >= 3) System.out.printf("Unexpected runaway in intervalDescr.\n");

      if(daysout > 0) daystr = String.format("%d days, ",daysout);
      else daystr = "";
      if(ndigits >= 0) { // including seconds ...
          if(ndigits == 0) timeformat = String.format(Locale.ENGLISH,"%%d %%d %%02.0f");
          else {
              int secondswidth = 3 + ndigits;
              timeformat = String.format(Locale.ENGLISH, "%%d hr %%d min %%0%1d.%1df sec",
                  secondswidth,ndigits);
          }
          timestr = String.format(Locale.ENGLISH,timeformat,sexout.hour,
                        sexout.minute,sexout.second);
      }
      else if (ndigits == -1)  {
          double decimalmin = sexout.minute + sexout.second / 60.;
          timestr = String.format(Locale.ENGLISH,"%d hr %4.1f min",sexout.hour,decimalmin);
      }
      else {
          timestr = String.format(Locale.ENGLISH,"%d hr %02d min",sexout.hour,
            sexout.minute);
      }

      return( daystr + timestr + signdescr );
   }
}
