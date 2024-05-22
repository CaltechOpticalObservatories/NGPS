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
//	 Const
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
import java.util.Scanner;
import java.util.List.*;
/*================================================================================================
/        Const() Class Declaration
/=================================================================================================*/
public class sexagesimal implements Cloneable

{
   public int sign;
   public int hour, minute;
   public double second;
   public double value;

   // Overloaded constructor takes either a string or a double argument.
   // little public classes wrap the ones that do all the work.
/*================================================================================================
/
/=================================================================================================*/
   public sexagesimal(String s) {
       parseSexString(s);
   }
/*================================================================================================
/
/=================================================================================================*/
   public sexagesimal(double v) {
       tosex(v);
       value = v;
   }
/*================================================================================================
/
/=================================================================================================*/
   public sexagesimal clone() {
      try {
          sexagesimal copy = (sexagesimal) super.clone();
          copy.sign = sign;
          copy.hour= hour;
          copy.minute = minute;
          copy.second= second;
          copy.value = value;
          return copy;
      } catch (CloneNotSupportedException e) {
          throw new Error("This should never happen!");
      }
   }
/*================================================================================================
/
/=================================================================================================*/
   void tosex(double h) {  // decimal hour converter ...
       double m;
       // System.out.printf("h = %f\n",h);
       if(h >= 0.) sign = 1;
       else sign = -1;
       h = h * sign;
       hour = (int) h;
       m = 60. * (h - (double) hour);
       minute = (int) m;
       second = 60. * (m - (double) minute);
       // System.out.printf("sgn h m s %d  %d %d %f\n",sign,hour,minute,second);
   }
/*================================================================================================
/     parseSexString(String s)
/=================================================================================================*/
   void parseSexString(String s) {
       // converts a string, either colon or space separated, into a
       // sexagesimal. "minus-zero" problem is handled.
       double doubleMinute;
       String [] fields;
       if(s.contains(":")) {
          fields = s.split(":");      // colon-delimited
       }
       else {
          fields = s.split("\\s+");  // whitespace
       }
       if(fields.length == 1) {  // a string, but just a single number
           value = Double.parseDouble(fields[0]);
           tosex(value);
       }
       else {   // it's a triplet (or possibly doublet)
          // for (String ss : fields) {
            // System.out.printf("%s ... ",ss);  // colon-delimited
          // }
          if(fields[0].contains("-")) sign = -1;
          else sign = 1;
          fields[0] = fields[0].replace("+","");  // parseInt chokes on explicit "+"
          fields[0] = fields[0].replace(" ","");  // parseInt chokes on explicit " "
          hour = Integer.parseInt(fields[0]);
          if(hour < 0) hour = hour * -1;   // positive definite
          try {
             minute = Integer.parseInt(fields[1]);
          }
          catch ( NumberFormatException e ) { // decimal minute input
             doubleMinute = Double.parseDouble(fields[1]);
             minute = (int) doubleMinute;
             second = 60. * (double) (doubleMinute - (double) minute);
          }
          if(fields.length > 2) {  // seconds are there ...
             second = Double.parseDouble(fields[2]);
          }
       }
       // System.out.printf("%d  %d %d %f\n",sign,hour,minute,second);
       value = (double) hour + (double) minute / 60. + (double) second / 3600.;
       value = value * sign;
       // System.out.printf("value: %f\n",value);
   }
/*================================================================================================
/
/=================================================================================================*/
   public sexagesimal roundsex(int ndigits) {
       // returns a sexagesimal rounded off to ndigits
       // so that seconds and minutes are never 60.  Higher overflows
       // (e.g. 24 00 00 for a time of day) will be handled elsewhere.

       String teststr = "";
       String testformat = "";
       int hourback, minuteback;
       double secondback;
       int secondswidth;
       int tenthMinutes;
       double decimalMinute,decimalMinuteback;

       if(ndigits >= 0) {  // including seconds ...
         if(ndigits == 0) testformat =
              String.format(Locale.ENGLISH, "%%d %%d %%02.0f");
         if(ndigits > 0) {
            secondswidth = 3 + ndigits;
            testformat = String.format(Locale.ENGLISH, "%%d %%d %%0%1d.%1df",
               secondswidth,ndigits);
            // System.out.println(testformat);
         }

         // System.out.printf("In roundsex, testformat = %s\n",testformat);
         teststr = String.format(Locale.ENGLISH, testformat,hour,minute,second);
         Scanner readback = new Scanner( teststr ).useDelimiter("\\s");
         readback.useLocale(Locale.ENGLISH);
         // read back the result ...
         // System.out.printf("In roundsex, teststr = %s\n",teststr);
         hourback = readback.nextInt();
         if(hourback < 0.) hourback *= -1.;
         minuteback = readback.nextInt();
         secondback = readback.nextDouble();
         // System.out.printf("read back: %d %d %f\n",hourback,minuteback,secondback);
         if(secondback > 59.999999999) {  // klugy, but should be very safe
            secondback = 0.;
            minuteback++;
            if(minuteback == 60) {
               minuteback = 0;
               hourback++; // overflows to 24, etc need to be handled
                          // at the next level
            }
            teststr = String.format(Locale.ENGLISH, testformat,hourback,minuteback,secondback);
         }
       }
       else {  // -1 -> tenths of a minute, -2 -> whole minutes
         decimalMinute = minute + second / 60.;
         if(ndigits == -1)
           teststr = String.format(Locale.ENGLISH, "%d %4.1f",hour,decimalMinute);
         if(ndigits <= -2)
           teststr = String.format(Locale.ENGLISH, "%d %02.0f",hour,decimalMinute);
         Scanner readback = new Scanner (teststr);
         readback.useLocale(Locale.ENGLISH);
         hourback = readback.nextInt();
         decimalMinuteback = readback.nextDouble();
         if(decimalMinuteback > 59.99) {  // limited precision - this will be safe
            decimalMinuteback = 0.00001;
            hourback++;
         }
         minuteback = (int) decimalMinuteback;
         if(ndigits == -1) {
            tenthMinutes = (int)
              ((10. * (decimalMinuteback - minuteback) + 0.0001));
            teststr = String.format(Locale.ENGLISH, "%d %02d.%1d",hourback,minuteback,tenthMinutes);
         }
         else
            teststr = String.format(Locale.ENGLISH, "%d %02d",hourback,minuteback);
       }
       sexagesimal roundedsex =  new sexagesimal(teststr);
       roundedsex.sign = sign;
       return roundedsex;
   }
}