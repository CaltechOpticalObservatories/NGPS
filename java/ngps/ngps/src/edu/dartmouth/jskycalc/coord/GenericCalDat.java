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
/*================================================================================================
/         Class Declaration
/=================================================================================================*/
/* The Java calendar API is pretty nasty, and the DST rules in
   particular are buried very deeply.  The application I have in
   mind requires me to have explicit control over these rules in the
   past and the future.  Given that I've already
   written my own date handlers in other languages, I'm thinking
   it may be be easier and more robust for me to just go ahead and
   reproduce that functionality from scratch.  */
/*================================================================================================
/
/=================================================================================================*/
public class GenericCalDat implements Cloneable
/** GenericCalDat - handles conversions back and forth to JD, and
    formatting and rounding of dates.  Rounding of times is handled in the
    sexagesimal class, and then continued rounding of dates is handled
    here.  GenericCalDat is blind to whether the date represented is
    UT or local - that's handled by InstantInTime. */
{
   public int year, month, day;
   public sexagesimal timeofday;
   String [] months = {"","January","February","March","April",
           "May","June","July","August","September","October","November",
           "December"};
   String monthtest = "janfebmaraprmayjunjulaugsepoctnovdec";
   String [] dayname = {"Mon","Tue","Wed","Thu","Fri","Sat","Sun"};

   // Polymorphism! Overload constructor class to
   // accept either a string or a double.
/*================================================================================================
/
/=================================================================================================*/
   public GenericCalDat( String s ) {
      // "yyyy mm dd hh mm ss" ... or JD as character string.
      CalFromString(s);
   }
/*================================================================================================
/
/=================================================================================================*/
   // overload the constructor method to take a JD number.
   public GenericCalDat(double jdin) {
      CalFromJD(jdin);
   }
/*================================================================================================
/
/=================================================================================================*/
   public GenericCalDat clone() {
      try {
          GenericCalDat copy = (GenericCalDat) super.clone();
          copy.year= year;
          copy.month = month;
          copy.day = day;
          copy.timeofday = (sexagesimal) timeofday.clone();
          return copy;
      } catch (CloneNotSupportedException e) {
          throw new Error("This should never happen!");
      }
   }
/*================================================================================================
/
/=================================================================================================*/
   public void CalFromJD (double jd) {
       /* sets the calendar date using the current values of jd -- can
          be either a local or UT date */

       /* Adapted from J. Meeus,  Astronomical Formulae for Calculators,
           published by Willman-Bell Inc.
           Avoids a copyrighted routine from Numerical Recipes.
           Tested and works properly from the beginning of the
            calendar era (1583) to beyond 3000 AD. */

       double tmp;
       long alpha;
       long Z, A, B, C, D, E;
       double F, x;
       int dow;
       double jdin;
       int rounded_ok = 0;

       jdin = jd;


       while(rounded_ok == 0) {
          tmp = jdin + 0.5;
          Z = (long) tmp;
          x = Z/7. + 0.01;
          dow = (int) (7.* Math.floor(x));

          F = tmp - Z;
          if (Z < 2299161) A = Z;
          else {
             alpha = (long) ((Z - 1867216.25) / 36524.25);
             A = Z + 1 + alpha - (long) (alpha / 4);
          }

          B = A + 1524;
          C = (long)((B - 122.1) / 365.25);
          D = (long)(365.25 * C);
          E = (long)((B - D) / 30.6001);

          day = (int)(B - D - (long)(30.6001 * E));
          if(E < 13.5) month = (int)(E - 1);
          else month = (int)(E - 13);
          if(month > 2.5) year = (int)(C - 4716);
          else year = (int) (C - 4715);
             timeofday = new sexagesimal(24. * F);

          if(timeofday.hour == 24) {
             jdin = jdin + 1.0e-7; // near to the resolution of the double
          }
          else rounded_ok = 1;
      }
   }
/*================================================================================================
/
/=================================================================================================*/
   public void CalFromString(String s) {
      // "yyyy mm dd hh mm ss" ... or just a string "JD".
      // "yyyy mm dd" or "yyyy mm dd hh mm" work ok too.
      // "2006 Jul 14" works, as does "2006 July 14 18:00"
      String [] fields = s.split("\\s+");   // whitespace

      if(fields.length == 1) { // presumably only a JD ...
         try{ 
         double jdin = Double.parseDouble(fields[0]);
         CalFromJD(jdin);
         return;
         }catch(Exception e){
            System.out.println("Error parsing the date string: " + s+ "  "+e.toString()); 
         }
       }
      year = Integer.parseInt(fields[0]);
      try {
         month = Integer.parseInt(fields[1]);
      }
      catch (NumberFormatException e) {
         // System.out.println("Catching exception .. ");
         String ss = fields[1].toLowerCase();
         ss = ss.substring(0,3);
         int ind = monthtest.indexOf(ss);
         if(ind > -1) {
            month =  ((ind / 3) + 1) ;
            // System.out.printf("ss %s ind %d month %d\n",ss,ind,month);
         }
         else {
            // System.out.println("Undecipherable month, set to 1000.\n");
            month = 1000;
         }
      }
      day = Integer.parseInt(fields[2]);
      if(fields.length == 4) { // colon-sep time included
         timeofday = new sexagesimal(fields[3]);
      }
      if(fields.length == 5) {
         timeofday = new sexagesimal(String.format(Locale.ENGLISH, "%s:%s",fields[3],fields[4]));
      }
      if(fields.length > 5) {
         timeofday =
          new sexagesimal(String.format(Locale.ENGLISH, "%s:%s:%s",fields[3],fields[4],fields[5]));
      }
   }
/*================================================================================================
/
/=================================================================================================*/
   public double Cal2JD() {

       int y, m;
       long A, B;
       double jdout;

       // System.out.printf("%d %d %d  %d %d %f\n",
          // year,month,day,timeofday.hour,timeofday.minute,timeofday.second);

       if(month <= 2) {
           y = year - 1;
           m = month + 12;
       }
       else {
           y = year;
           m = month;
       }

       A = (long) ((double) (y+0.00000001) / 100.);
       B = 2 - A + (long) ((double) (A + 0.000000001) / 4.);

       jdout =  (long)(365.25 * (double) y) + (long) (30.6001 * (m + 1)) + day +
           1720994.5;
       //System.out.println(jd);
       jdout += (double) timeofday.hour / 24. + (double) timeofday.minute / 1440. +
              timeofday.second / 86400.;
       //System.out.println(jd);

       if(year > 1583) return (jdout + (double) B);
       else return jdout;
   }
/*================================================================================================
/
/=================================================================================================*/
   public int DayOfWeek() {
   /** Adapted straight from skycalc, returns 0=Mon throuh 6=Sun **/

       double jd;
       long i;
       double x;
       int d;

       jd = Cal2JD() + 0.5;
       i = (long) jd;
       x = i/7. + 0.01;
       d = (int) (7.*(x - (long) x));
       return d;
   }
/*================================================================================================
/
/=================================================================================================*/
   public void quickprint() {
       System.out.printf("%d %02d %02d  %02d %02d %f",
          year,month,day,timeofday.hour,timeofday.minute,timeofday.second);
       System.out.printf(" -> %s\n",dayname[DayOfWeek()]);
   }
/*================================================================================================
/
/=================================================================================================*/
   public String RoundedCalString(int style, int digits) {
   /** Returns a descriptive string; rather than writing a flexible
       format I'll code a number of options.  Much sturm und drang here
       because of the need to round the day along with everything else.
       These styles follow cooclasses.py; I'll write more as needed. :

       style 0 -> 2006 8 12  10 11 12
       style 1 -> 2005 Aug 12  10 11
       style 2 -> Fri Aug 12  10:11
       style 3 -> 2005 Aug 12
       style 4 -> Fri Aug 12
       style 5 -> 2005 6 12
       style 6 -> 2006 Aug 12 Tue
       style 7 -> Fri 2006 Aug 12  10:11

       style 10 -> (time only) 10 11 12.0
       style 11 -> (time only) 10:11:12.0
       style 12 -> (time only) 10:11

       These are included here to force correct rounding when printing
          only the time.

        */
       String result;
       String outputFormat;
       int timeDigits;
       double jdtemp;
       int printYear, printMonth, printDay;
       int printHour, printMinute;
       double printSecond;
       int printDOW;


       if(style == 0)  digits = 0;
       if(style == 1 | style == 2 | style == 12)  digits = -2;

       sexagesimal Rounded = timeofday.roundsex(digits);
       // round the date upward ...
       if(Rounded.hour == 24) {
          // System.out.println("Oops, gotta round day upward.\n");
          jdtemp = Cal2JD() + 0.4; // this will always round upward
                                   // and never screw the day of week
          GenericCalDat tempcal = new GenericCalDat(jdtemp);
          printYear = tempcal.year;
          printMonth = tempcal.month;
          printDay = tempcal.day;
          printHour = 0;
          printMinute = 0;
          printSecond = 0.;
          printDOW = tempcal.DayOfWeek();
       }
       else {
          printYear = year;
          printMonth = month;
          printDay = day;
          printHour = Rounded.hour;
          printMinute = Rounded.minute;
          printSecond = Rounded.second;
          printDOW = DayOfWeek();
       }
       String monthAbr = months[printMonth].substring(0,3);
       switch(style) {
         case 0 :
           // System.out.println("*** 0 ***");
           result = String.format(Locale.ENGLISH, "%4d %02d %02d  %02d %02d %02.0f",
             printYear, printMonth, printDay, printHour, printMinute,
             printSecond);
         break;
         case 1:
           // System.out.println("*** 1 ***");
           result = String.format(Locale.ENGLISH, "%4d %s %02d  %02d %02d",
             printYear, monthAbr, printDay, printHour, printMinute);
           break;
         case 2 :
           // System.out.println("*** 2 ***");
           result = String.format(Locale.ENGLISH, "%s %s %02d  %02d:%02d",
             dayname[printDOW],monthAbr,printDay,printHour,printMinute);
           break;
         case 3 :
           // System.out.println("*** 3 ***");
           result = String.format(Locale.ENGLISH, "%4d %s %02d",printYear,monthAbr,
              printDay);
           break;
         case 4 :
           // System.out.println("*** 4 ***");
           result = String.format(Locale.ENGLISH, "%s %s %02d",dayname[printDOW],
             monthAbr,printDay);
           break;
         case 5 :
           // System.out.println("*** 5 ***");
           result = String.format(Locale.ENGLISH, "%4d %02d %02d",printYear,printMonth,
                 printDay);
           break;
         case 6 :
           // System.out.println("*** 6 ***");
           result = String.format(Locale.ENGLISH, "%4d %s %02d  %s",printYear,monthAbr,
              printDay,dayname[printDOW]);
           break;
         case 7 :
           // System.out.println("*** 2 ***");
           result = String.format(Locale.ENGLISH, "%s  %4d %s %02d  %02d:%02d",
             dayname[printDOW],printYear,monthAbr,printDay,printHour,printMinute);
           break;

         case 11 :
           // System.out.println("*** 11 ***");
           result = String.format(Locale.ENGLISH, "%02d %02d %04.1f",printHour,printMinute,
                 printSecond);
           break;
         case 12 :
           result = String.format(Locale.ENGLISH, "%02d:%02d",printHour,printMinute);
           break;
         case 13:  
             //yyyy-mm-dd hh:mm:ss.fffffffff
             result = String.format(Locale.ENGLISH, "%4d-%02d-%02d %02d:%02d:%02.0f",printYear, printMonth, printDay, printHour, printMinute,printSecond);
         break;  
         default :
           // System.out.println("*** Default ***");
           result = String.format(Locale.ENGLISH, "%02d %02d %02.0f",
             printYear, monthAbr, printDay, printHour, printMinute,
             printSecond);
       }
       return result;
   }
}