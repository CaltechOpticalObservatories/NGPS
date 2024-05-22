/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
  
package edu.caltech.palomar.telescopes.guider.catalog.ucac3;
import edu.caltech.palomar.telescopes.guider.catalog.ucac3.ucac3Reader;
import edu.caltech.palomar.telescopes.guider.catalog.ucac3.ucac3Star;
import javax.swing.UIManager;
import edu.caltech.palomar.telescopes.P200.TelescopesIniReader;
//import jsky.catalog.astrocat.AstroCatTable;
import jsky.catalog.Catalog;
import jsky.catalog.MemoryCatalog;
//import jsky.catalog.astrocat.*;
import java.net.URL;
import java.io.IOException;
import jsky.coords.CoordinateRadius;
import jsky.coords.Coordinates;
import jsky.coords.ImageCoords;
import java.util.Enumeration;
import java.util.Vector;
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
//  A segment is one of the 86400 areas of the sky that is 0.5 degrees in
//  declination and 0.1 hours in size. (30 arcminutes by 90 arcminutes).
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
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
/*=============================================================================================
/        UCAC3Catalog Class Declaration
/=============================================================================================*/
public class UCAC3Catalog extends MemoryCatalog implements Catalog{

/*=============================================================================================
/        UCAC3Catalog Constructor
/=============================================================================================*/
   public UCAC3Catalog(){
     super();
     jbInit() ;
//     test();
   }
/*=============================================================================================
/        jbInit()
/=============================================================================================*/
   private void jbInit(){

   }




/*=============================================================================================
/         main method
/=============================================================================================*/
  //Main method
  public static void main(String[] args) {
    try {
      UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
    }
    catch(Exception e) {
      e.printStackTrace();
    }
    new UCAC3Catalog();
  }
}
