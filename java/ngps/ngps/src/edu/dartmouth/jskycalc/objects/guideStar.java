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
/*================================================================================================
/
/=================================================================================================*/
public class guideStar {
   Celest orig;    // from catalog
   Celest pmcorr;  // J2000, corrected for PM only
   Celest current; // updated for precession and proper motion
   float mag;      // catalog
   double [] guiderXY;
   float pmra, pmdec;
   boolean legal;
/*================================================================================================
/
/=================================================================================================*/
   public guideStar(double rain, double decin, double eqin, double epin, double epnow,
                                           float pmrain, float pmdecin, float magin) {
      // pmra and pmdec are on sky (delta X and Y) in mas/year.
      double raep, decep, delta_ep, cosdec;
      pmra = pmrain;
      pmdec = pmdecin;
      orig = new Celest(rain, decin, eqin);
      cosdec = Math.cos(orig.Delta.radians());
      delta_ep = epnow - epin;
      // PM update tested carefully and found to be correct.
      raep = orig.Alpha.value + delta_ep * pmra / (cosdec * 54000000.);
      decep = orig.Delta.value + delta_ep *       pmdec /    3600000.;
      pmcorr = new Celest(raep, decep, eqin);
      current = pmcorr.precessed(epnow);
      guiderXY = new double[2]; // meaningless until there's a center.
      guiderXY[0] = 0.; guiderXY[1] = 0.;
      mag = magin;
   }
}

