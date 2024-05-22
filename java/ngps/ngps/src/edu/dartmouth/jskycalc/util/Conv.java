package edu.dartmouth.jskycalc.util;
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
/*================================================================================================
/
/=================================================================================================*/
public class Conv {
   /** static classes to translate the raw bytes into ints. */
   // These assume a certain byte order.  I don't know if Java specifies this.
/*================================================================================================
/
/=================================================================================================*/
   public static float getfloat32(byte [] by, int startpos) {
      float out2;
      int   out;
      out = ((by[startpos + 3] & 0xFF) << 24) | ((by[startpos + 2] & 0xFF) << 16) | ((by[startpos + 1] & 0xFF) << 8) | by[startpos] & 0xFF;
      out2 = (float)out;
      return out;
   }
/*================================================================================================
/
/=================================================================================================*/
   public static int getint32(byte [] by, int startpos) {
      int out;
      out = ((by[startpos + 3] & 0xFF) << 24) | ((by[startpos + 2] & 0xFF) << 16) | ((by[startpos + 1] & 0xFF) << 8) | by[startpos] & 0xFF;
      return out;
   }

   // fake it out -- make a 32-bit integer by throwing garbage into the higher
   // bytes and and-ing them with zeros.  Otherwise java objects to
   // loss of precision.  Probably a more elegant way around this, but life is
   // short.
/*================================================================================================
/
/=================================================================================================*/
   public static int getint16(byte [] by, int startpos) {
      int out;
      out = ((by[startpos + 0] & 0x00) << 24) | ((by[startpos + 1] & 0x00) << 16) | ((by[startpos + 1] & 0xFF) << 8) | by[startpos] & 0xFF;
      return out;
   }
/*============================================================================================
/
/=================================================================================================*/
   public static int getint16_short(byte [] by, int startpos) {
      int out;
      out = (int)((short)(((by[startpos + 1] & 0xFF) << 8) | by[startpos] & 0xFF));
      return out;
   }
/*================================================================================================
/
/=================================================================================================*/
   // same garbage-blanked-by-zeros trick used here.
   public static int getint08(byte [] by, int startpos) {
      int out;
      out = ((by[startpos] & 0x00) << 24) | ((by[startpos] & 0x00) << 16) | ((by[startpos] & 0x00) << 8) | by[startpos] & 0xFF;
      return out;
   }
}



