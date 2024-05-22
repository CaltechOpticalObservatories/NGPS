/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
 
package edu.caltech.palomar.telescopes.guider.catalog.ucac3;
import java.util.StringTokenizer; 
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
//    ucac3index  - holds index information from the UCAC3 catalog index file
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
//       DERIVED DIRECTLY FROM THE JSKYCALC by JOHN THORSTENSEN, Dartmouth University
//       Original Implementation : John Thorstensen copyright 2007,
//
//       December 7, 2010  Jennifer Milburn, refactor for use at the Palomar Observatory
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
/   class declaration ucac3index
/=================================================================================================*/
public class ucac3index {
   public int running_star_number = 0;
   public int number_stars_bin    = 0;
   public int zone_number         = 0;
   public int ra_bin_index        = 0;
   public double start_dec = 0;
   public double end_dec   = 0;
   public double start_ra  = 0;
   public double end_ra    = 0;
   public String           segment_name = new java.lang.String();
   private StringTokenizer stoken;
   public  String          currentLine = new java.lang.String();
/*================================================================================================
/      public constructor ucac3index(java.lang.String currentLine)
/=================================================================================================*/
    public ucac3index(java.lang.String currentLine){
        this.currentLine = currentLine;
        process();
    }
/*================================================================================================
/    private void process()
/=================================================================================================*/
    private void process(){
     try{
        stoken = new StringTokenizer(currentLine);
        running_star_number = (Integer.valueOf(stoken.nextToken())).intValue();
        number_stars_bin    = (Integer.valueOf(stoken.nextToken())).intValue();
        String zone_number_string  = stoken.nextToken();
        String ra_bin_index_string = stoken.nextToken();
        zone_number         = (Integer.valueOf(zone_number_string)).intValue();
        ra_bin_index        = (Integer.valueOf(ra_bin_index_string)).intValue();
        segment_name = zone_number_string + ":" + ra_bin_index_string;
        }catch(Exception e){
            System.out.println(e.toString());
        }
    }
/*================================================================================================
/   end of ucac3index class
/=================================================================================================*/
}
