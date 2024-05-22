package edu.dartmouth.jskycalc.gui;
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
import java.util.List.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import edu.dartmouth.jskycalc.objects.NightlyAlmanac;
import java.beans.PropertyChangeListener;
import java.beans.PropertyChangeSupport;
/*================================================================================================
/
/=================================================================================================*/
  public class NightlyWindow extends JFrame {
    transient protected PropertyChangeSupport propertyChangeListeners = new PropertyChangeSupport(this);
     /** Displays phenomena (sunset, moonrise, etc) */

      JTextField [] phenfield ;
      JLabel [] phenlabel;
      String [] labeltext = {"Sunset","Twilight Ends","LST Eve. Twi.","Night Center","Twilight Begins",
        "LST Morn. Twi.","Sunrise","Moonrise","Moonset","UTC Twilight"};
      NightlyAlmanac Nightly;
      private java.lang.String UTC_twilight = new java.lang.String();
/*================================================================================================
/
/=================================================================================================*/
    public NightlyWindow(NightlyAlmanac Nightly) {
          this.Nightly = Nightly;
         int i;

         JPanel container = new JPanel();

         JPanel tablepanel = new JPanel();
         tablepanel.setLayout(new GridBagLayout());
         GridBagConstraints tableconstraints = new GridBagConstraints();

	 phenfield = new JTextField [10];
         phenlabel = new JLabel [10];
         for (i = 0; i < 10; i++) {
            tableconstraints.gridy = i;
            tableconstraints.gridx = 0;
            phenlabel[i] = new JLabel(labeltext[i]);
            tablepanel.add(phenlabel[i],tableconstraints);
            tableconstraints.gridx = 1;
            phenfield[i] = new JTextField(11);
            tablepanel.add(phenfield[i],tableconstraints);
         }

         JButton hider = new JButton("Hide Nightly Almanac");
         hider.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                setVisible(false);
//   Commented out to allow subset compilation
//                nightlyframevisible = false;
             }
         });
         container.add(tablepanel);
         container.add(hider);
         this.setSize(270,360);
//         this.setSize(280,270);
         this.setLocation(400,100);
         this.setContentPane(container);
      }
 /*================================================================================================
/     setOTMpa(double new_OTMpa)
/=================================================================================================*/
  public void setUTCTwilight(java.lang.String new_UTC_twilight) {
    java.lang.String  old_UTC_twilight = this.UTC_twilight;
    UTC_twilight = new_UTC_twilight;
   propertyChangeListeners.firePropertyChange("UTC_twilight", old_UTC_twilight, new_UTC_twilight);
  }
  public java.lang.String getUTCTwilight() {
    return UTC_twilight;
  }    
/*================================================================================================
/
/=================================================================================================*/
    public void UpdateDisplay() {  // assumes Nightly has been updated separately.

         phenfield[0].setText(Nightly.sunset.when.localDate.RoundedCalString(2,0));
         phenfield[1].setText(Nightly.eveningTwilight.when.localDate.RoundedCalString(2,0));
         phenfield[2].setText(Nightly.eveningTwilight.siderealobj.RoundedRAString(-2," "));
         phenfield[3].setText(Nightly.nightcenter.when.localDate.RoundedCalString(2,0));
         phenfield[4].setText(Nightly.morningTwilight.when.localDate.RoundedCalString(2,0));
         phenfield[5].setText(Nightly.morningTwilight.siderealobj.RoundedRAString(-2," "));
         phenfield[6].setText(Nightly.sunrise.when.localDate.RoundedCalString(2,0));

         if(Nightly.moonrise.when.jd < Nightly.moonset.when.jd) {
            phenfield[7].setText(Nightly.moonrise.when.localDate.RoundedCalString(2,0));
            phenlabel[7].setText("Moonrise");
            phenfield[8].setText(Nightly.moonset.when.localDate.RoundedCalString(2,0));
            phenlabel[8].setText("Moonset");
         }
         else {
            phenfield[7].setText(Nightly.moonset.when.localDate.RoundedCalString(2,0));
            phenlabel[7].setText("Moonset");
            phenfield[8].setText(Nightly.moonrise.when.localDate.RoundedCalString(2,0));
            phenlabel[8].setText("Moonrise");
         }
         phenlabel[9].setText("UTC Evening Twilight");
         phenfield[9].setText(Nightly.eveningTwilight.when.UTDate.RoundedCalString(11,4));
         setUTCTwilight(Nightly.eveningTwilight.when.UTDate.RoundedCalString(11,4));
         System.out.println(Nightly.eveningTwilight.when.UTDate.RoundedCalString(11,4));
      }
 /*================================================================================================
/      add and remove Property Change Listeners
/=================================================================================================*/
  public synchronized void removePropertyChangeListener(PropertyChangeListener l) {
    propertyChangeListeners.removePropertyChangeListener(l);
  }
  public synchronized void addPropertyChangeListener(PropertyChangeListener l) {
    propertyChangeListeners.addPropertyChangeListener(l);
  }     
   }