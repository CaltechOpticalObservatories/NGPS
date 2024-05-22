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
//    AirmassFrame   - Display panel for Airmass Displays
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
import edu.dartmouth.jskycalc.gui.AirmassDisplay;
import edu.dartmouth.jskycalc.JSkyCalcModel;
import edu.caltech.palomar.telescopes.P200.gui.tables.AstroObjectsModel;
import edu.dartmouth.jskycalc.objects.NightlyAlmanac;
import edu.caltech.palomar.telescopes.P200.TelescopesIniReader;
/*=============================================================================================
/     AirmassFrame extends JFrame
/=============================================================================================*/
public class AirmassFrame extends JFrame{
     JPanel               testPanel;
     AirmassDisplay       myAirmassDisplay = new AirmassDisplay();
     JSkyCalcModel        myJSkyCalcModel;
     AstroObjectsModel    myAstroObjectsModel;
     NightlyAlmanac       myNightly;
     ObjectSelectionPanel myObjectSelectionPanel = new ObjectSelectionPanel();
     UpdateAirmassGraphThread myUpdateAirmassGraphThread;
     TelescopesIniReader      myTelescopesIniReader = new TelescopesIniReader();
//     JFrame           parentFrame;
/*=============================================================================================
/     SkyDisplayFrame constructor
/=============================================================================================*/
    public AirmassFrame(){
        jbInit();
        this.setDefaultCloseOperation(JFrame.HIDE_ON_CLOSE);
    }
/*=============================================================================================
/    setModels(NightlyAlmanac newNightly,AstroObjectsModel newAstroObjectsModel, JSkyCalcModel newJSkyCalcModel)
/=============================================================================================*/
    public void setModels(NightlyAlmanac newNightly,AstroObjectsModel newAstroObjectsModel, JSkyCalcModel newJSkyCalcModel){
       myNightly       = newNightly;
       myJSkyCalcModel = newJSkyCalcModel;
       myAstroObjectsModel = newAstroObjectsModel;
       myAirmassDisplay.initializeAirmassDisplay(myNightly,myAstroObjectsModel, myJSkyCalcModel);
       myObjectSelectionPanel.setAstroObjectModel(myAstroObjectsModel);
       myObjectSelectionPanel.setAirmassFrame(this);
   }
/*=============================================================================================
/     resetAirmassDisplay()
/=============================================================================================*/
  public void resetAirmassDisplay(){
      stopUpdate();
      testPanel.remove(myAirmassDisplay);
      myAirmassDisplay = new AirmassDisplay();
      myAirmassDisplay.initializeAirmassDisplay(myNightly,myAstroObjectsModel, myJSkyCalcModel);  
      myObjectSelectionPanel.setAstroObjectModel(myAstroObjectsModel);
      testPanel.add(myAirmassDisplay,BorderLayout.CENTER);
      this.setVisible(false);
      this.setVisible(true);
      this.executeUpdate();
  }    
/*=============================================================================================
/     AirmassDisplay getAirmassDisplay()
/=============================================================================================*/
   public AirmassDisplay getAirmassDisplay(){
       return myAirmassDisplay;
   }
/*=============================================================================================
/     NightlyAlmanacy getNightlyAlmanac()
/=============================================================================================*/
   public NightlyAlmanac getNightlyAlmanac(){
       return myNightly;
   }
/*=============================================================================================
/     JSkyCalcModel getJSkyCalcModel()
/=============================================================================================*/
   public JSkyCalcModel getJSkyCalcModel(){
       return myJSkyCalcModel;
   }
/*=============================================================================================
/     AstroObjectsModel getAstroObjectsModel()
/=============================================================================================*/
   public AstroObjectsModel getAstroObjectsModel(){
       return myAstroObjectsModel;
   }
   public void initializeObjectSelection(){

   }
/*=============================================================================================
/      executeUpdate()
/=============================================================================================*/
 public void executeUpdate(){
   myUpdateAirmassGraphThread = new UpdateAirmassGraphThread();
   myUpdateAirmassGraphThread.start();
 }
/*=============================================================================================
/      executeUpdate()
/=============================================================================================*/
 public void stopUpdate(){
   myUpdateAirmassGraphThread.setUpdating(false);
 }
/*=============================================================================================
/     jbInit()
/=============================================================================================*/
    private void jbInit(){
      testPanel = new JPanel(new BorderLayout());
      testPanel.setBorder(BorderFactory.createLoweredBevelBorder());
      testPanel.add(myAirmassDisplay,BorderLayout.CENTER);
      testPanel.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(51, 153, 0), 5));
      XYLayout MainxYLayout                         = new XYLayout();
      this.setBackground(new java.awt.Color(0, 0, 0));
      this.setLayout(MainxYLayout);
      this.add(testPanel,new XYConstraints(  0,   0, 800,  500));
      this.add(myObjectSelectionPanel,new XYConstraints(  800,   0, 190,  500) );
      this.setSize(990, 520);
  }
/*=================================================================================================
/        positionFrame()
/=================================================================================================*/
  public void positionFrame(){
     this.setLocation(myTelescopesIniReader.AIRMASS_X, myTelescopesIniReader.AIRMASS_Y);
     this.setVisible(myTelescopesIniReader.AIRMASS_VIS);
     this.pack();
  }     
/*=============================================================================================
/       INNER CLASS that polls the telescope for it's position and status
/=============================================================================================*/
    public class UpdateAirmassGraphThread  implements Runnable{
    private Thread myThread;
    private boolean state      = false;
    private java.lang.String response = new java.lang.String();
/*================================================================================================
/      waitForResponse(int newDelay)
/=================================================================================================*/
public void waitForResponseMilliseconds(int newDelay){
     try{
        Thread.currentThread().sleep(newDelay);} // sleep for awhile to let the messages pile up
      catch (Exception e){
    }
}
/*=============================================================================================
/          setUpdating(boolean newState)
/=============================================================================================*/
public synchronized void setUpdating(boolean newState){
    state = newState;
}
public synchronized boolean isUpdating(){
    return state;
}
/*=============================================================================================
/          UpdateAirmassGraphThread()
/=============================================================================================*/
    private UpdateAirmassGraphThread(){
    }
/*=============================================================================================
/           run()
/=============================================================================================*/
    public void run(){
       state = true;
       while(state){
          myAirmassDisplay.Update();
 //         System.out.println("Updating the Airmass Display "+System.currentTimeMillis());
          waitForResponseMilliseconds(1000);
       }
   }
/*=============================================================================================
/         start()
/=============================================================================================*/
    public void start(){
     myThread = new Thread(this);
     myThread.start();
     }
    }// End of the UpdateEphemerisThread Inner Class
/*=============================================================================================
/                           End of the UpdateEphemerisThread Class
/=============================================================================================*/
}
