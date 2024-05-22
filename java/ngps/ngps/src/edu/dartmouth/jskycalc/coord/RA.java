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
/* Now come the classes that handle celestial coordinates ...  */

public class RA implements Cloneable {
/** Right Ascension.  */

   public double value;    // decimal hours
   public sexagesimal sex;
/*================================================================================================
/
/=================================================================================================*/
   double adjra(double inval) {
      // System.out.printf("input inval %f ... ",inval);
      inval = inval % 24.;
      while(inval < 0.) inval += 24.;
      while(inval > 24.) inval -= 24.;
      // System.out.printf("returning inval = %f\n",inval);
      return inval;
   }
/*================================================================================================
/
/=================================================================================================*/
   // overloaded constructors are simply wrappers
   public RA(double inval) {
      value = adjra(inval);
      sex = new sexagesimal(value);
   }
 /*================================================================================================
/
/=================================================================================================*/
  public RA(String s) {
      sex = new sexagesimal(s);
      value = adjra(sex.value);
      sex.tosex(value);
   }
/*================================================================================================
/
/=================================================================================================*/
   public void setRA(double inval) {
      value = adjra(inval);
//      System.out.printf("Setting start %d %d %5.2f  %f  -> ",
//        sex.hour,sex.minute,sex.second,value);
      sex.tosex(value);
//      System.out.printf("%d %d %5.2f ... \n",sex.hour,sex.minute,sex.second);
   }
/*================================================================================================
/
/=================================================================================================*/
   public void setRA(String s) {
      sex.parseSexString(s);
      value = adjra(sex.value);
      sex.tosex(value);
   }
/*================================================================================================
/
/=================================================================================================*/
   public RA clone() {
      try {
          RA copy = (RA) super.clone();
          copy.value = value;
          copy.sex = (sexagesimal) sex.clone();
          return copy;
      } catch (CloneNotSupportedException e) {
          throw new Error("This should never happen!");
      }
   }
/*================================================================================================
/
/=================================================================================================*/
   public double radians() {
      return(value / Const.HRS_IN_RADIAN);
   }
/*================================================================================================
/
/=================================================================================================*/
   public double degrees() {
      return(value * 15.);
   }
/*================================================================================================
/
/=================================================================================================*/
   public String RoundedRAString(int ndigits, String divider) {

   /** Returns a rounded sexagesimal RA, with the cut imposed at 24 h */

      sexagesimal rounded;
      int secfieldwidth;
      double decimalMinute;
      String raformat = "";
      String outstr = "";

      rounded = sex.roundsex(ndigits);
      if(rounded.hour == 24) {
         rounded.hour = 0;
         rounded.minute = 0;
         rounded.second = 0.;
      }
      if(ndigits >= 0) {
         if(ndigits == 0) raformat =
           String.format(Locale.ENGLISH, "%%02d%s%%02d%s%%02.0f",divider,divider);
         else {
           secfieldwidth=ndigits + 3;
           raformat =
             String.format(Locale.ENGLISH, "%%02d%s%%02d%s%%0%1d.%1df",divider,divider,
                      secfieldwidth,ndigits);
         }
         outstr = String.format(Locale.ENGLISH, raformat,rounded.hour,rounded.minute,rounded.second);
      }
      else if (ndigits == -1) {
         decimalMinute = rounded.minute + rounded.second / 60.;
         outstr = String.format(Locale.ENGLISH, "%02d%s%04.1f",rounded.hour,divider,decimalMinute);
      }
      else if (ndigits == -2) {
         outstr = String.format(Locale.ENGLISH, "%02d%s%02d",rounded.hour,divider,rounded.minute);
      }
      return outstr;
   }
}
