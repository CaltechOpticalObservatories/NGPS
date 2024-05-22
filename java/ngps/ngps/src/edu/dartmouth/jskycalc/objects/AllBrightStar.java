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
import java.util.List.*;
import edu.dartmouth.jskycalc.coord.*;
/*================================================================================================
/
/=================================================================================================*/
public class AllBrightStar {
   // A class for storing info from bright star list.  These are to be read in from
   // bright_pmupdated.dat, which is a file of _all_ the 9096 stars from the bright
   // star catalog (excepting only a few novae).  bright_pmupdated is updated for
   // proper motion to 2010, and will need to be refreshed from time to time.
   // The lines look like this:
   //   hr9093   0 02 29.64  +8 29 07.5  2000.0  _32____Psc   5.630  F0V

   public String name;
   public Celest c;
   public String propername;
   public Float mag;
   public String spectraltype;
/*================================================================================================
/
/=================================================================================================*/
  public AllBrightStar(String instuff) {
          String [] fields;
          String [] fields2;
          String RAfield;
          String decfield;
       try {
          // System.out.printf("String:: %s\n",instuff);
          fields = instuff.split("\\s+");   // split on whitespace
          name = fields[0];
          // System.out.printf("%s\n",name);
          RAfield = fields[1] + ":" + fields[2] + ":" + fields[3]; 
          decfield = fields[4] + ":" + fields[5] + ":" + fields[6]; 
          // System.out.printf("%s %s\n",RAfield,decfield);
          c = new Celest(RAfield,decfield,fields[7]);   
          propername = fields[8];
          mag = Float.parseFloat(fields[9]);
          spectraltype = fields[10];
       } catch (Exception e)
          { System.out.printf("Unreadable line in bright star file input: %s\n",instuff); }
   }
}