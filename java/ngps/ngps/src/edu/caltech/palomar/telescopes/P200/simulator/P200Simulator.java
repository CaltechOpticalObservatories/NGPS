/*
 * To change this template, choose Tools | Templates 
 * and open the template in the editor.
 */

package edu.caltech.palomar.telescopes.P200.simulator;
//=== File Prolog =============================================================
//	This code was developed by California Institute of Technology
//      Caltech Optical Observatories, Palomar Observatory
//
//--- Contents ----------------------------------------------------------------
//	 P200Simulator- This is the TCP/IP server for the P200 Simulator
//
//--- Description -------------------------------------------------------------
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
//
//      July 14, 2010 Jennifer Milburn
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
//	In no event shall Caltech be liable for any damages, including, but not
//	limited to direct, indirect, special or consequential damages, arising out
//	of, resulting from, or in any way connected with this software, whether or
//	not based upon warranty, contract, tort or otherwise, whether or not
//	injury was sustained by persons or property or otherwise, and whether or
//	not loss was sustained from or arose out of the results of, or use of,
//	their software or services provided hereunder.
//
//=== End File Prolog =========================================================
import javax.swing.UIManager;
import java.awt.*;
import edu.caltech.palomar.telescopes.P200.simulator.P200SimulatorFrame2;
import edu.caltech.palomar.telescopes.P200.TelescopesIniReader;
/*=============================================================================================
/         P200Simulator Class Declaration
/=============================================================================================*/
public class P200Simulator {
 boolean packFrame = false;
 P200SimulatorFrame2 frame;
 TelescopesIniReader myTelescopesIniReader = new TelescopesIniReader();
 //Construct the application
   public static int              DEFAULT_SERVERPORT = 49200;
/*=============================================================================================
/         P200Simulator constructor
/=============================================================================================*/
 public P200Simulator() {    
     frame = new P200SimulatorFrame2();
//     frame.componentsTesting();
     frame.getServerSocket().setServerPort(myTelescopesIniReader.SERVERPORT_INTEGER);
     frame.startServer();
    //Validate frames that have preset sizes
    //Pack frames that have useful preferred size info, e.g. from their layout
    if (packFrame) {
      frame.pack();
    }
    else {
      frame.validate();
    }
    //Center the window
    Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
    Dimension frameSize = frame.getSize();
    if (frameSize.height > screenSize.height) {
      frameSize.height = screenSize.height;
    }
    if (frameSize.width > screenSize.width) {
      frameSize.width = screenSize.width;
    }
    frame.setLocation((screenSize.width - frameSize.width) / 2, (screenSize.height - frameSize.height) / 2);
    frame.setVisible(true);
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
    new P200Simulator();
  }
}

