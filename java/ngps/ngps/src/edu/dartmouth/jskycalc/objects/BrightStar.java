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
import java.io.*;
import java.net.*; // needed for sockets.
import java.util.*;
import java.util.Scanner;
import java.util.regex.*;
import java.util.List.*;
import java.awt.*;
import java.awt.print.*;
import java.awt.event.*;
import java.awt.geom.*;
import java.awt.font.*;
// import java.awt.PointerInfo.*;
// import java.awt.MouseInfo.*;
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.table.*;
import javax.swing.text.html.*;
import javax.swing.border.*;
/*================================================================================================
/
/=================================================================================================*/
public class BrightStar {
   // A class for storing info from bright star list.  These are to be read in from
   // brightest.dat, which is a file of the about 900 stars with m < 4.5
   // These are the ones for display, which carry color info etc.  The ones for
   // pointing updates are separate.
   public String name;
   public Celest c;
   public double m;   // apparent magnitude.
   public Color col;  // color used for plotting the star, based on spectral type.
/*================================================================================================
/     BrightStar()
/=================================================================================================*/
  public BrightStar(){

   }
/*================================================================================================
/     BrightStar(String instuff)
/=================================================================================================*/
   public BrightStar(String instuff) {
       // constructor reads from a file with lines like:
       // 3.158278  44.85722  3.80 " 27Kap Per"
      // try {
          String [] fields;
          String [] fields2;
       try {
          fields = instuff.split("\"");   // split out the quoted name
          name = fields[1];
          fields[0] = fields[0].trim();
          fields2 = fields[0].split("\\s+");  // and the rest
          c = new Celest(Double.parseDouble(fields2[0]),Double.parseDouble(fields2[1]),2000.);
          m = Double.parseDouble(fields2[2]);
          col = new Color(Integer.parseInt(fields2[3]),Integer.parseInt(fields2[4]),
                         Integer.parseInt(fields2[5]));
       } catch (Exception e)
          { System.out.printf("Unreadable line in bright star file input: %s\n",instuff); }
   }

   // time to rationalize the precession in the display map ... which means we need:
}
