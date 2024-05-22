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
public class longitude implements Cloneable {

/** longitude, +- 12 hours.  Includes some special hooks for
    interpreting inmput strings .*/

   public double value;   // internally, hours west.
   public sexagesimal sex;
/*================================================================================================
/
/=================================================================================================*/
   public double adjlongit(double inval) {
      inval = inval % 24.;
      while(inval < -12.) inval += 24.;
      while(inval > 12.) inval -= 24.;
      return inval;
   }
/*================================================================================================
/
/=================================================================================================*/
   public longitude(double x) {
     setFromDouble(x);
   }
/*================================================================================================
/
/=================================================================================================*/
   public longitude(String s) {
     setFromString(s);
   }
/*================================================================================================
/
/=================================================================================================*/
   public longitude clone() {
      try {
         longitude copy = (longitude) super.clone();
         copy.value = value;
         copy.sex = (sexagesimal) sex.clone();
         return copy;
      } catch (CloneNotSupportedException e) {
         throw new Error ("This should never happen!");
      }
   }
/*================================================================================================
/
/=================================================================================================*/
   public void setFromDouble(double x) {
     value = adjlongit(x);  // force value to +- 12 h on input.
     sex = new sexagesimal(value);
   }
/*================================================================================================
/
/=================================================================================================*/
   public void setFromString(String s) {
      String [] fields;
      String unitfield;
      String directionfield;
      boolean indegrees = false, positiveEast = false;

      int nf, i;

// THIS NEEDS WORK to avoid colliding with rules used in the
// interpretation of sexagesimals.  Need to test for letters in last
// couple of pieces of the string, then feed the sexagesimal converter
// with only the remaining pieces.

      // check for a label at the end ...
      fields = s.split("\\s+");  // whitespace
      nf = fields.length;
      unitfield = fields[nf - 2].toLowerCase();
     // System.out.printf("last field %s\n",lastfield);
      i = unitfield.indexOf("h");
      if(i > -1) {
         indegrees = false;
        // System.out.println("h found ... ");
      }
      i = unitfield.indexOf("d");
      if(i > -1) {
         indegrees = true;
        // System.out.println("d found ... ");
      }
      directionfield = fields[nf - 1].toLowerCase();
      i = directionfield.indexOf("e");
      if(i > -1) {
         positiveEast = true;
        // System.out.println("e found ... ");
      }
      i = directionfield.indexOf("w");
      if(i > -1) {
         positiveEast = false;
        // System.out.println("w found ... ");
      }
      sex = new sexagesimal(s);
      value = sex.value;
      if(indegrees) value = value / 15.;
      if(positiveEast) value *= -1.;
     // System.out.printf("Value = %f\n",value);
      value = adjlongit(value);
      sex.tosex(value);
   }
/*================================================================================================
/
/=================================================================================================*/
   public double radiansWest() {
      return (value / Const.HRS_IN_RADIAN);
   }
/*================================================================================================
/
/=================================================================================================*/
   public double hoursWest() {
      return value;
   }
   public double degreesEast() {
      return value * -15.;
   }
/*================================================================================================
/
/=================================================================================================*/
   public String RoundedLongitString(int ndigits, String divider,
         boolean inDegrees) {

      sexagesimal outvalsex, rounded;
      double outval;
      int secfieldwidth;
      double decimalMinute;
      String raformat = "";
      String outstr = "";

      if(inDegrees) outval = value * 15.;
      else outval = value;

      outvalsex = new sexagesimal(outval);
      rounded = outvalsex.roundsex(ndigits);

      // System.out.printf("rounded value is %f\n",rounded.value);

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

      if(inDegrees) outstr = outstr + " D";
      else outstr = outstr + " H";
      if(rounded.sign == 1) outstr = outstr + " W";
      else outstr = outstr + " E";

      return outstr;
   }
}
