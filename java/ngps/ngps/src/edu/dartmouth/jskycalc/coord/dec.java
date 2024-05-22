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
public class dec implements Cloneable {
/** declination.  */

   public double value;    // decimal degrees
   public sexagesimal sex;
/*================================================================================================
/
/=================================================================================================*/
   // overloaded constructors are simply wrappers
   public dec(double inval) {
      setDec(inval);
   }
/*================================================================================================
/
/=================================================================================================*/
   public dec(String s) {
      setDec(s);
   }
   // no good way to adjust out-of-range decs (it
   // doesn't wrap) so this is simpler than HA and RA
/*================================================================================================
/
/=================================================================================================*/
   public void setDec(double inval) {
      value = inval;
      sex = new sexagesimal(value);
   }
/*================================================================================================
/
/=================================================================================================*/
   public void setDec(String s) {
      sex = new sexagesimal(s);
      value = sex.value;
   }
/*================================================================================================
/
/=================================================================================================*/
   public dec clone() {
      try {
          dec copy = (dec) super.clone();
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
      return(value / Const.DEG_IN_RADIAN);
   }
/*================================================================================================
/
/=================================================================================================*/
   public double degrees() {
      return(value);   // duuhh - but may be good to have
   }
/*================================================================================================
/     RoundedDecString(int ndigits, String divider)
/=================================================================================================*/
   public String RoundedDecString(int ndigits, String divider) {
      // no need to wrap, so again simpler than HA and RA
      sexagesimal rounded;
      int secfieldwidth;
      double decimalMinute;
      String decformat = "";
      String outstr = "";
      String signout = "+";

      rounded = sex.roundsex(ndigits);

      if(rounded.sign > 0) signout = "+";
         else signout = "-";
      if(ndigits >= 0) {
         if(ndigits == 0) decformat =
           String.format(Locale.ENGLISH, "%s%%02d%s%%02d%s%%02.0f",signout,divider,divider);
         else {
           secfieldwidth=ndigits + 3;
           decformat =
             String.format(Locale.ENGLISH, "%s%%02d%s%%02d%s%%0%1d.%1df",signout,divider,divider,
                      secfieldwidth,ndigits);
         }
         outstr = String.format(Locale.ENGLISH, decformat,rounded.hour,rounded.minute,rounded.second);
      }
      else if (ndigits == -1) {
         decimalMinute = rounded.minute + rounded.second / 60.;
         outstr = String.format(Locale.ENGLISH, "%s%02d%s%04.1f",signout,rounded.hour,divider,decimalMinute);
      }
      else if (ndigits == -2) {
         outstr = String.format(Locale.ENGLISH, "%s%02d%s%02d",signout,rounded.hour,divider,rounded.minute);
      }
      return outstr;
   }
/*================================================================================================
/     RoundedDecStringNoSign(int ndigits, String divider)
/=================================================================================================*/
   public String RoundedDecStringNoSign(int ndigits, String divider) {
      // Exactly the same as above, but omits sign if it's positive.
      sexagesimal rounded;
      int secfieldwidth;
      double decimalMinute;
      String decformat = "";
      String outstr = "";
      String signout = "";

      rounded = sex.roundsex(ndigits);

      if(rounded.sign < 0) signout = "-";
         else signout = "";
      if(ndigits >= 0) {
         if(ndigits == 0) decformat =
           String.format(Locale.ENGLISH, "%s%%02d%s%%02d%s%%02.0f",signout,divider,divider);
         else {
           secfieldwidth=ndigits + 3;
           decformat =
             String.format(Locale.ENGLISH, "%s%%02d%s%%02d%s%%0%1d.%1df",signout,divider,divider,
                      secfieldwidth,ndigits);
         }
         outstr = String.format(Locale.ENGLISH, decformat,rounded.hour,rounded.minute,rounded.second);
      }
      else if (ndigits == -1) {
         decimalMinute = rounded.minute + rounded.second / 60.;
         outstr = String.format(Locale.ENGLISH, "%s%02d%s%04.1f",signout,rounded.hour,divider,decimalMinute);
      }
      else if (ndigits == -2) {
         outstr = String.format(Locale.ENGLISH, "%s%02d%s%02d",signout,rounded.hour,divider,rounded.minute);
      }
      return outstr;
   }

}
