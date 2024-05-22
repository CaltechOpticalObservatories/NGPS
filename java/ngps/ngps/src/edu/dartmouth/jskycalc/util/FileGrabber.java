package edu.dartmouth.jskycalc.util;
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
import java.io.*;
import java.util.List.*;
import javax.swing.*;
/*================================================================================================
/
/=================================================================================================*/
public class FileGrabber extends JFrame {
// similar utility class for opening an infile, pops a file chooser.
    public File infile;
    public BufferedReader br = null;
    public FileReader fr = null;
    public String fname;
    public String pathname;

    public FileGrabber(String inPath) {
       if (inPath == null) {   // not a previously opened file - use file dialog
          JFileChooser chooser = new JFileChooser();
          int result = chooser.showOpenDialog(this);
          if(result != JFileChooser.CANCEL_OPTION) {
             try {
                 infile = chooser.getSelectedFile();
                 fname = chooser.getName(infile);
                 pathname = infile.getPath();
    //	      System.out.printf("fname = %s, curdirstr = %s\n",fname,pathname);
                 br = null;
                 fr = new FileReader(infile);
             } catch (Exception e) {
                 System.out.println("File opening error of some kind.");
             }
             if(fr != null)  br = new BufferedReader(fr);
          }
       }
       else {   // previously opened file path supplied.
          try {
//             System.out.printf("About to try to open %s\n",inPath);
               infile = new File(inPath);
//             ClassLoader cl = this.getClass().getClassLoader();
//             System.out.printf("Here 1 ... \n");
//             InputStream is = cl.getResourceAsStream(inPath);
//             System.out.printf("Here 2 ... \n");
//             br = new BufferedReader(new InputStreamReader(is));
               br = null;
               fr = new FileReader(infile);
//             System.out.printf("Here 3 ... \n");
          } catch (Exception e) 
              { System.out.printf("Problem opening %s for input.\n",inPath); }
          if(fr != null)  br = new BufferedReader(fr);
       }
    }
/*================================================================================================
/
/=================================================================================================*/
  public void closer() {
       try {
           fr.close();
       } catch(IOException e) {
           System.out.println("File reader didn't close.");
       }
    }
}