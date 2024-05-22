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
import edu.dartmouth.jskycalc.coord.Site;
import edu.dartmouth.jskycalc.gui.JSkyCalcWindow;
import java.io.*;
import java.util.List.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.util.HashMap;
import java.util.Vector;
/*================================================================================================
/         class SiteWindow
/=================================================================================================*/
class SiteWindow extends JFrame {
      // shoves this large block of code into its own subclass.
private transient Vector actionListeners;
 // having problems with the getResourceAsStream function so I'm trying to get the file directly
  public   java.lang.String USERDIR                = System.getProperty("user.dir");
  public   java.lang.String SEP                    = System.getProperty("file.separator");
  public   java.lang.String CONFIG                 = SEP + "config" + SEP;
  public   java.lang.String RESOURCES              = "resources" + SEP;
  public   java.lang.String PATH                   = USERDIR + CONFIG + RESOURCES;
  public   static HashMap<String, Site> siteDict   = new HashMap<String, Site>();
  public   static final int SETTOJD                = 1000;
  public   static final int COLOR_USER_INPUT_TRUE  = 1002;
  public   static final int COLOR_USER_INPUT_FALSE = 1003;

  public ButtonGroup SiteButtons;
      String [] sitekeys;
      int nsites;
      static String st = null;
/*================================================================================================
/         class SiteWindow
/=================================================================================================*/
     public SiteWindow() {
         int iy = 0;
         int i = 0;

         ReadSites();    // see below.

         JPanel sitepan = new JPanel();
         sitepan.setLayout(new GridBagLayout());
         GridBagConstraints constr = new GridBagConstraints();
         SiteButtons = new ButtonGroup();
         JRadioButton radioButton;  // need a generic one here

         constr.anchor = GridBagConstraints.LINE_START;

         constr.gridx = 0; constr.gridy = iy;
         sitepan.add(radioButton = new JRadioButton("Enable User Input",false),constr);
         radioButton.setActionCommand("x");  // we'll wait before refreshing.
         radioButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                ColorUserInput(true);  // change site to input color
            }
         });

         SiteButtons.add(radioButton);

         for(i = 0; i < nsites; i++) {
            String name = sitekeys[i];

            iy++; constr.gridx = 0; constr.gridy = iy;
            sitepan.add(radioButton = new JRadioButton(name,true),constr);
            radioButton.setActionCommand(name);
            radioButton.addActionListener(new ActionListener() {  // ugly to do it to all...
            public void actionPerformed(ActionEvent e) {
                   setToJD();
                   ColorUserInput(false);
               }
            });
            SiteButtons.add(radioButton);
         }

         iy++; constr.gridx = 0; constr.gridy = iy;
         JButton sitehider = new JButton("Hide Site Chooser");
         sitehider.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                setVisible(false);
                siteframevisible(false);
            }
         });
         sitepan.add(sitehider,constr);

         Container sitecontainer = this.getContentPane();
         sitecontainer.add(sitepan);
         this.setSize(180,580);
//         this.setSize(210,640);
         // this.pack();
         this.setLocation(585,20);
         this.setContentPane(sitepan);
      }
    public void setToJD(){
      fireActionPerformed(new java.awt.event.ActionEvent(this,SETTOJD,"setToJD"));
 //       jsc.setToJD();
    }
    public void siteframevisible(boolean vis){
 //      jsc.siteframevisible = vis;
    }
    public void ColorUserInput(boolean state){
      int actionFlag = COLOR_USER_INPUT_TRUE;
        if(state){
           actionFlag = COLOR_USER_INPUT_TRUE;
        }
        if(!state){
           actionFlag = COLOR_USER_INPUT_FALSE;
        }
      fireActionPerformed(new java.awt.event.ActionEvent(this,actionFlag,"ColorUserInput(actionFlag)"));

    }


      public Site firstSite() {
         // just pops out first site on list for initilization ...
         return siteDict.get(sitekeys[0]);
      }

     public void ReadSites() {

         BufferedReader br = null;
         boolean inquote = false;
         String [] fields = new String[14];  // too many but that's ok
         int i,j;

         sitekeys = new String[40];         // hard-coded, sorry
         nsites = 0;

         try {
//             ClassLoader cl = getClass().getClassLoader();
//             InputStream is = cl.getResourceAsStream("/resources/skycalcsites.txt");
             FileInputStream fis = new FileInputStream(PATH + "skycalcsites.txt");
             br = new BufferedReader(new InputStreamReader(fis));
         } catch (Exception e) {
             System.out.printf("Problem opening skycalcsites.dat for input.\n");
         }

         // read the site info character-by-character to preserve quoted values.
         // there's undoubtedly a better way, but this works well enough.

         try {
            while((st = br.readLine()) != null) {
               // System.out.printf("read: %s\n",st);
               if(st.length() > 0) {
                  if(st.charAt(0) != '#') {
                     j = 0;   // field counter
                     fields[j] = "";
                     for(i = 0; i < st.length(); i++) {
                        char [] thischar = {st.charAt(i)};
                        if(st.charAt(i) == '"') {
                            if(inquote) inquote = false;
                            else inquote = true;
                        }
                        else {
                           if(inquote) fields[j] = fields[j] + new String(thischar);
                           else {
                              if (st.charAt(i) == ',') {
                                 j = j + 1;
                                 fields[j] = "";
                              }
                              else fields[j] = fields[j] + new String(thischar);
                           }
                        }
                     }
                     siteDict.put(fields[0],new Site(fields));
                     sitekeys[nsites] = fields[0];  // so they'll come out in order ...
                     nsites++;
      //               for(j = 0; j < fields.length; j++) System.out.printf("%s ",fields[j]);
       //              System.out.printf("\n");
                  }
               }
            }
         } catch (IOException e) {System.out.printf("IO exception\n");}
      }
/*=============================================================================================
/   ActionEvent Handling Code - used to informing registered listeners that a FITS
/      file is ready to be retrieved from the server
/  removeActionListener()
/  addActionListener()
/  fireActionPerformed()
/=============================================================================================*/
/*=============================================================================================
/     synchronized void removeActionListener(ActionListener l)
/=============================================================================================*/
  public synchronized void removeActionListener(ActionListener l) {
    if (actionListeners != null && actionListeners.contains(l)) {
      Vector v = (Vector) actionListeners.clone();
      v.removeElement(l);
      actionListeners = v;
    }
  }
/*=============================================================================================
/    synchronized void addActionListener(ActionListener l)
/=============================================================================================*/
  public synchronized void addActionListener(ActionListener l) {
    Vector v = actionListeners == null ? new Vector(2) : (Vector) actionListeners.clone();
    if (!v.contains(l)) {
      v.addElement(l);
      actionListeners = v;
    }
  }
/*=============================================================================================
/     protected void fireActionPerformed(ActionEvent e)
/=============================================================================================*/
  public void fireActionPerformed(ActionEvent e) {
    if (actionListeners != null) {
      Vector listeners = actionListeners;
      int count = listeners.size();
      for (int i = 0; i < count; i++) {
        ((ActionListener) listeners.elementAt(i)).actionPerformed(e);
      }
    }
}
}






