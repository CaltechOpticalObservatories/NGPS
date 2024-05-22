/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */ 

package edu.caltech.palomar.telescopes.P200.gui.tables;
//=== File Prolog =============================================================
//	This code was developed by UCLA, Department of Physics and Astronomy,
//	for the Stratospheric Observatory for Infrared Astronomy (SOFIA) project.
//
//--- Contents ----------------------------------------------------------------
//	AstroObject
//           This class extends the Celest class and is designed
//      too hold the name, RA, Dec and tracking rates for an astronomical object
//
//--- Description -------------------------------------------------------------
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
//
//      8/10/10        Jennifer Milburn
//        Original Implementation
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
//	In no event shall UCLA be liable for any damages, including, but not
//	limited to direct, indirect, special or consequential damages, arising out
//	of, resulting from, or in any way connected with this software, whether or
//	not based upon warranty, contract, tort or otherwise, whether or not
//	injury was sustained by persons or property or otherwise, and whether or
//	not loss was sustained from or arose out of the results of, or use of,
//	their software or services provided hereunder.
//
//=== End File Prolog =========================================================
import edu.dartmouth.jskycalc.coord.Celest;
import edu.dartmouth.jskycalc.coord.sexagesimal;
import edu.dartmouth.jskycalc.coord.RA;
import edu.dartmouth.jskycalc.coord.dec;
/*================================================================================================
/          Class Declaration for AstroObject
/=================================================================================================*/
public class AstroObject extends edu.dartmouth.jskycalc.coord.Celest{
    public double      r_m;
    public double      d_m;
    public String      airmass;
    public boolean     m_flag;
    public int         download_status;
    sexagesimal        sexagesimal_object;
    public String      name;
    public String      comment  = new java.lang.String();
    public String      fileName = new java.lang.String();
/*================================================================================================
/          Constructors for AstroObject
/=================================================================================================*/
//  COORDS (five or six arguments):  supply a position to the TCS.  The arguments
//  in order are
//    1) RA in decimal hours,
//    2) declination in decimal degrees,
//    3) equinox of coordinates (value of zero indicates apparent equinox),
//    4) RA motion,
//    5) dec motion,
//    6) motion flag.  (Motion flag absent or 0:  arguments
//  4 and 5 are proper motion in RA units of .0001 sec/yr and dec units of .001
//  arcsec/yr.  Motion flag = 1:  arguments 4 and 5 are offset tracking rates in
//  arcsec/hr.  Motion flag = 2:  arguments 4 and 5 are offset tracking rates
//  with arg 4 in seconds/hr and arg 5 in arcsec/hr.)  NOTE: a name string can
//  now be provided, by adding an extra parameter enclosed in double quotes;
//  this will preferably be at the end of the command line, though it can be
//  inserted at any parameter position.
/*================================================================================================
/          Constructors for AstroObject
/=================================================================================================*/
    public AstroObject(){
    }
/*================================================================================================
/          Constructors for AstroObject
/=================================================================================================*/
    public AstroObject(String name,RA r, dec d, double e,double r_m,double d_m,boolean m_flag,String comment){
        this.name     = name;
        super.Alpha   =  r;
        super.Delta   =  d;
        super.Equinox = e;
        this.r_m      = r_m;
        this.d_m      = d_m;
        this.m_flag   = m_flag;
        this.comment  = comment;
        this.download_status = 0;
        this.fileName = "";
    }
/*================================================================================================
/          Constructors for AstroObject
/=================================================================================================*/
    public AstroObject(String name,RA r, dec d, double e,double r_m,double d_m,boolean m_flag,String comment,String fileName){
        this.name     = name;
        super.Alpha   =  r;
        super.Delta   =  d;
        super.Equinox = e;
        this.r_m      = r_m;
        this.d_m      = d_m;
        this.m_flag   = m_flag;
        this.comment  = comment;
        this.download_status = 0;
        this.fileName = fileName;
    }
/*================================================================================================
/          Constructors for AstroObject
/=================================================================================================*/
}
