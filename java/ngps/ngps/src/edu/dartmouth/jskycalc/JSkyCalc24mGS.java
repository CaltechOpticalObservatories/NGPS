package edu.dartmouth.jskycalc;
/* JSkyCalc24m.java -- 2009 January.  I liked the 1.3m so much I decided
   to do something similar for the 2.4m.  This has the capability of reading
   the telescope coords and commenting on them, but it doesn't slew the 
   telescope.  It'll be nice to automatically have the telescope coords there,
   though. */

/* JSkyCalc13m.java -- 2009 January.  This version adds an interface to 
 * Dick Treffers' 1.3m telescope control server, which communicates via
 * the BAIT protocol.  The target-selection facilities here can then be
 * used. */

/* JSkyCalc.java -- copyright 2007, John Thorstensen, Dartmouth College. */
/** TERMS OF USE -- Anyone is free to use this software for any purpose, and to
modify it for their own purposes, provided that credit is given to the author
in a prominent place.  For the present program that means that the green
title and author banner appearing on the main window must not be removed,
and may not be altered without premission of the author. */

import edu.dartmouth.jskycalc.gui.JSkyCalcWindow;

import java.util.List.*;
import javax.swing.*;

public class JSkyCalc24mGS {
   public static void main(String [] args) {
   //   Locale locale = Locale.getDefault();  ... fixes the problems with German locale,
   //   Locale.setDefault(Locale.ENGLISH);    ... but, breaks the webstart!!

      try {
	    // Set cross-platform Java L&F (also called "Metal")
        UIManager.setLookAndFeel(
            UIManager.getCrossPlatformLookAndFeelClassName());
      } 
      catch (UnsupportedLookAndFeelException e) {
         System.out.printf("UnsupportedLookAndFeelException ... \n");
      }
      catch (ClassNotFoundException e) {
         System.out.printf("Class not found while trying to set look-and-feel ...\n");
      }
      catch (InstantiationException e) {
         System.out.printf("Instantiation exception while trying to set look-and-feel ...\n");
      }
      catch (IllegalAccessException e) {
         System.out.printf("Illegal access exception while trying to set look-and-feel ...\n");
      }

      JSkyCalcWindow MainWin = new JSkyCalcWindow() ;

   }
}










  