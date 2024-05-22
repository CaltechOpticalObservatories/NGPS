package edu.caltech.palomar.telescopes.P200.gui;
//=== File Prolog =============================================================
//	This code was developed by John Thorstensen of Dartmouth University
//
//      It has been refactored and modified by Jennifer Milburn for
//      use at the Palomar Observatory.
//
//      Caltech Optical Observatories, Palomar Observatory
//
//--- Contents ----------------------------------------------------------------
//    P200DesignerForm   - Display panel for the principle telescope pointing
//                         and status display parameters.
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
import javax.swing.JFrame;
import javax.swing.JPanel;
import java.util.*;
import java.util.List.*;
import java.awt.*;
import java.awt.event.*;
import java.awt.geom.*;
import javax.swing.*;
import com.borland.jbcl.layout.*;
import edu.caltech.palomar.telescopes.P200.TelescopesIniReader;
import edu.dartmouth.jskycalc.gui.SkyDisplaySimple;
import edu.dartmouth.jskycalc.JSkyCalcModel;
/*=============================================================================================
/     SkyDisplayFrame extends JFrame
/=============================================================================================*/
public class SkyDisplayFrame extends JFrame{
     JPanel           testPanel;
   public  SkyDisplaySimple mySkyDisplaySimple = new SkyDisplaySimple();
     JSkyCalcModel    myJSkyCalcModel;
//     JFrame           parentFrame;

   private  java.lang.String      TERMINATOR        = new java.lang.String("\n");
   public   java.lang.String      USERDIR           = System.getProperty("user.dir");
   public   java.lang.String      SEP               = System.getProperty("file.separator");
   public   java.lang.String      IMAGE_CACHE       = new java.lang.String(SEP + "images" + SEP);
   public   java.lang.String      CONFIG            = new java.lang.String(SEP + "config" + SEP);
   private  JToolBar              toolBar           = new JToolBar();
   public  TelescopesIniReader    myTelescopesIniReader = new TelescopesIniReader();
/*=============================================================================================
/     SkyDisplayFrame constructor
/=============================================================================================*/
    public SkyDisplayFrame(){
//        this.parentFrame = parentFrame;
      this.setDefaultCloseOperation(JFrame.HIDE_ON_CLOSE);
    }
/*=================================================================================================
/        positionFrame()
/=================================================================================================*/
  public void positionFrame(){
     this.setLocation(myTelescopesIniReader.SKY_X, myTelescopesIniReader.SKY_Y);
     this.setVisible(myTelescopesIniReader.SKY_VIS);
     this.pack();
  }     
/*=============================================================================================
/     setJSkyCalcModel(JSkyCalcModel newJSkyCalcModel)
/=============================================================================================*/
    public void setJSkyCalcModel(JSkyCalcModel newJSkyCalcModel){
       myJSkyCalcModel = newJSkyCalcModel;
       mySkyDisplaySimple.initializeDisplay(800,800,this,myJSkyCalcModel);
       jbInit();
   }
/*=============================================================================================
/     SkyDisplaySimple getSkyDisplay()
/=============================================================================================*/
   public SkyDisplaySimple getSkyDisplay(){
       return mySkyDisplaySimple;
   }
/*=============================================================================================
/     jbInit()
/=============================================================================================*/
    private void jbInit(){
      JPanel testPanel = new JPanel(new BorderLayout());
      testPanel.setBorder(BorderFactory.createLoweredBevelBorder());
      testPanel.add(mySkyDisplaySimple,BorderLayout.CENTER);
      testPanel.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(51, 153, 0), 5));

      XYLayout MainxYLayout                         = new XYLayout();
      this.setBackground(new java.awt.Color(0, 0, 0));

      this.setLayout(MainxYLayout);
      this.add(testPanel,new XYConstraints(  0,   0, 800,  800));
      this.setSize(800, 820);
  }
}
