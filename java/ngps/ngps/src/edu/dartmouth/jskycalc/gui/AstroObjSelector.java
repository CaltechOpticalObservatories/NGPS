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
import edu.dartmouth.jskycalc.coord.Spherical;
import edu.dartmouth.jskycalc.util.FileGrabber;
import edu.dartmouth.jskycalc.coord.Celest;
import edu.dartmouth.jskycalc.objects.AstrObj;
import java.io.*;
import java.util.*;
import java.util.List.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.event.*;
/*================================================================================================
/      class AstroObjSelector
/=================================================================================================*/
 public class AstroObjSelector extends JFrame {
   static String [] RASelectors;  // real names, sorted by ra
   static String [] objListNames;  // real names, sorted by ra
   static int max_lists = 20;
   static int num_lists = 0;
   SkyDisplay SkyDisp;
   static String st = null;
   static AstrObj obj;
   public HashMap<String, AstrObj> byname = new HashMap<String, AstrObj>();
   public HashMap<Double, AstrObj> byra   = new HashMap<Double, AstrObj>();
   static HashMap<String, String> objListFilesDict = new HashMap<String,String>();
   static Double rakey;
   static String [] NameSelectors;  // real names, sorted by dec.
   HashMap<String, AstrObj> presenterKey;
       JList selectorList;
       JList infilesList;
   public boolean objselwinvisible = true;  // for this version ...
   AirmassDisplay myAirmassDisplay;

//   The following modifications of text fields directly in this code creates a serious dependency
//   the text fields have been commented out to eliminate the dependency but the functionality has not been replaced
//                         objnamefield.setText(sel);
//                         RAfield.setText(presenterKey.get(sel).c.Alpha.RoundedRAString(3," "));
//                         decfield.setText(presenterKey.get(sel).c.Delta.RoundedDecString(2," "));
//                         equinoxfield.setText(String.format(Locale.ENGLISH, "%7.2f",presenterKey.get(sel).c.Equinox));
//              objnamefield.setText(RASelectors[minindex]);
//              RAfield.setText(objcel.Alpha.RoundedRAString(2," "));
//              decfield.setText(objcel.Delta.RoundedDecString(1," "));
//              equinoxfield.setText(String.format(Locale.ENGLISH, "%7.2f",objcel.Equinox));
//   MULTIPLE CALLS TO LOAD A LIST FOR AN INSTANCE OF THE OBJECT - A SYMANTIC NO-NO  REPLACED
//       ObjSelWin.LoadList();
//       AirmSelWin.LoadList();
//     REPLACED WITH   LoadList();

/*================================================================================================
/      class AstroObjSelector
/=================================================================================================*/
     public  AstroObjSelector(boolean is_single,SkyDisplay SkyDisp,AirmassDisplay newAirmassDisplay,HashMap<String, AstrObj> presenterKey) {
            this.myAirmassDisplay = newAirmassDisplay;
            this.presenterKey = presenterKey;
            this.SkyDisp = SkyDisp;
            RASelectors = new String[max_lists];   // fixed max size.
            RASelectors[0] = " "; // this too
            objListNames = new String[max_lists];   // fixed max size.
            objListNames[0] = " "; // this too

            JPanel outer = new JPanel();
            selectorList = new JList(RASelectors);
            infilesList  = new JList(objListNames);
            infilesList.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);

            if (is_single) {
               selectorList.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
               this.setTitle("Object Selector");
            }
            else {
               selectorList.setSelectionMode(
                  ListSelectionModel.MULTIPLE_INTERVAL_SELECTION);
               this.setTitle("Airmass Graph Sel.");
               outer.setBackground(new Color(100,100,100));  // color this differently
            }

            selectorList.setPrototypeCellValue("xxxxxxxxxxxxxxxxxxxx");
            outer.add(new JScrollPane(selectorList));
            infilesList.setPrototypeCellValue("xxxxxxxxxxxxxxxxxxxx");
            JScrollPane infilePane = new JScrollPane(infilesList);

            outer.add(infilePane);
            super.add(outer);

            JButton objselbutton = new JButton("Load New Object List");
            objselbutton.addActionListener(new ActionListener() {
               public void actionPerformed(ActionEvent e) {
                   LoadAstrObjs(null);
                   repaintSkyDisplay();  // cause them to appear on display.
               }
            });
            outer.add(objselbutton);

            JButton sortbyra = new JButton("Sort by RA");
            sortbyra.addActionListener(new ActionListener() {
               public void actionPerformed(ActionEvent e) {
                  selectorList.setListData(RASelectors);
               }
            });
            outer.add(sortbyra);

            JButton byname = new JButton("Alphabetical Order");
            byname.addActionListener(new ActionListener() {
               public void actionPerformed(ActionEvent e) {
                  selectorList.setListData(NameSelectors);
               //   for(int i = 0; i < NameSelectors.length; i++)
                //    System.out.printf("%s\n",NameSelectors[i]);
               }
            });
            outer.add(byname);

            JButton clearbutton = new JButton("Clear list");
            clearbutton.addActionListener(new ActionListener() {
               public void actionPerformed(ActionEvent e) {
                  ClearAstrObjs();
                  repaintSkyDisplay();
               }
            });
            outer.add(clearbutton);

            if(!is_single) {

               JButton plotairmasses = new JButton("Plot airmasses");
               plotairmasses.addActionListener(new ActionListener() {
                  public void actionPerformed(ActionEvent e) {
//                     myAirmassDisplay.airmassPlotSelections =  selectorList.getSelectedValues();
//                     System.out.printf("%d objects selected\n",myAirmassDisplay.airmassPlotSelections.length);
                     synchOutput();
                  }
               });
               outer.add(plotairmasses);

               JButton deselector = new JButton("Deselect all");
               deselector.addActionListener(new ActionListener() {
                  public void actionPerformed(ActionEvent e) {
                     selectorList.clearSelection();
                  }
               });
               outer.add(deselector);
            }

            JButton hider = new JButton("Hide Window");
            hider.addActionListener(new ActionListener() {
               public void actionPerformed(ActionEvent e) {
                  setVisible(false);
                  objselwinvisible = false;
               }
            });
            outer.add(hider);

            this.add(outer);
//            this.setSize(180,340);
            // leave more room for the airmass selector ...
            if(is_single) this.setSize(210,480);
            else this.setSize(210,530);
            if(is_single) this.setLocation(10,270);
            else this.setLocation(700,50);

            if(is_single) {
               selectorList.addListSelectionListener(new ListSelectionListener() {
                   public void valueChanged(ListSelectionEvent l) {
                      /* Changing the list fires a selection event that can
                         generate bad data.  Catch the resulting exceptions. */
                      try {
                         String sel = (String) selectorList.getSelectedValues()[0];
      /*                    System.out.printf("%d %s %s %s\n",i,
                           Selectors[i],
                          presenterKey.get(Selectors[i]).name,
                          presenterKey.get(Selectors[i]).c.checkstring()
                          ); */
//                         objnamefield.setText(sel);
//                         RAfield.setText(presenterKey.get(sel).c.Alpha.RoundedRAString(3," "));
//                         decfield.setText(presenterKey.get(sel).c.Delta.RoundedDecString(2," "));
//                         equinoxfield.setText(String.format(Locale.ENGLISH, "%7.2f",presenterKey.get(sel).c.Equinox));
                         synchOutput();
                      } catch (ArrayIndexOutOfBoundsException e) {}
                   }
               });
            }

	    // reload an object list if selected in infilesList

            infilesList.addListSelectionListener(new ListSelectionListener() {
// for some reason this executes 2x every time the value is changed ... !
// but later I guard against loading the same object 2x, so it seems to work??
                public void valueChanged(ListSelectionEvent l) {
                    try {
                        String sel = (String) infilesList.getSelectedValues()[0];
                        String path_to_try = objListFilesDict.get(sel);
//			System.out.printf("Calling LoadAstrObjs(%s)\n",path_to_try);
                        LoadAstrObjs(path_to_try);
		        repaintSkyDisplay();
                    } catch (ArrayIndexOutOfBoundsException e) {}
                }
            });
       }
     void repaintSkyDisplay(){
       SkyDisp.repaint();
     }

     void LoadList() {
           selectorList.setListData(RASelectors);
       }

       void LoadInfList() {
           infilesList.setListData(objListNames);
       }


   void LoadAstrObjs(String inPath) {

       FileGrabber ff = new FileGrabber(inPath);

       if(ff == null) {
          System.out.printf("No objects loaded.\n");
          return;
       }

       try {
// I cannot seem to actually kill the byra has with the "clear" command, no matter
// what I do.  Then, when I go to read another list, the rakey gets filled in twice
// because of the "tie-breaker" below.  So I'll re-code the tie breaker so as not to
// add an object that actually exists (same ra and dec).
// diagnostic
//          Iterator keyiterator = byra.keySet().iterator();
//          System.out.printf("If 'ra key:' lines appear below, ra keys are not empty.\n");
//          while(keyiterator.hasNext()) {
//              Object ss = keyiterator.next();
//              AstrObj yy = (AstrObj) byra.get(ss);
//              System.out.printf("ra key: %s radec %s\n",ss,yy.c.checkstring());
//          }
// end diagnostic
          while((st = ff.br.readLine()) != null) {
              obj = new AstrObj(st);
              if (obj.name != null & obj.c != null) {
                  byname.put(obj.name.toLowerCase(),obj);
                  rakey = (Double) obj.c.Alpha.value;
		  double decval = (Double) obj.c.Delta.value;
//                  System.out.printf("%s\n",obj.c.checkstring());
                  // ensure unique RA keys by inserting small tie-breaker offset
                  if(byra.keySet().contains(rakey) && byra.get(rakey).c.Delta.value != obj.c.Delta.value) {
                     System.out.printf("incrementing rakey from %f for %s\n",rakey,obj.name);
                     rakey = rakey + 0.00001;
                  }
                  byra.put(rakey,obj);
              }
          }
       } catch (IOException e) { System.out.println(e); }

       ff.closer();

       java.util.List<String> namekeys = new ArrayList<String>(byname.keySet());
       java.util.List<Double> rakeys = new ArrayList<Double>(byra.keySet());

       Collections.sort(namekeys);
       Collections.sort(rakeys);

       RASelectors = new String[rakeys.size()];

       Iterator raiterator = rakeys.iterator();
       int i = 0;
       while(raiterator.hasNext()) {
          Object key =  raiterator.next();
          AstrObj tempobj = byra.get(key);
          presenterKey.put(tempobj.name,tempobj);
          RASelectors[i] = tempobj.name;
          i++;
       }

       NameSelectors = new String[namekeys.size()];

       Iterator nameiterator = namekeys.iterator();
       i = 0;
       while(nameiterator.hasNext()) {
          Object key =  nameiterator.next();
          AstrObj tempobj = byname.get(key);
          // presenterkey is alreary loaded ... but the NameSelector array will
          // be in alphabetical order.
          NameSelectors[i] = tempobj.name;
          i++;
       }
       // AstrObjSelector objsel = new AstrObjSelector();
//       ObjSelWin.LoadList();
//       AirmSelWin.LoadList();
         LoadList();
       // See if the filename that was used is new, and if so add it to
       // the filename selector.
       String fname = ff.fname;
       String pathname = ff.pathname;
//     System.out.printf("at end of Load, fname = %s, pathname = %s\n",fname,pathname);

       if(fname != null) {
          boolean inf_never_used = true;
          int ifile = 0;
          while(inf_never_used && ifile < num_lists) {
              if(objListNames[ifile].equals(fname)) {
   			inf_never_used = false;
                           //System.out.printf("List already opened.\n");
   	   }
              ifile++;
          }
          if(inf_never_used) {
//            System.out.printf("num_lists = %d\n",num_lists);
              objListNames[num_lists] = fname;
              objListFilesDict.put(fname,pathname);
              num_lists++;
          }
//          ObjSelWin.LoadInfList();
//          AirmSelWin.LoadInfList();
         LoadInfList();

       }
//     else {
//        System.out.printf("fname is null, not checking.\n");
//     }

      // for(i = 0; i < num_lists; i++) {
      //    System.out.printf("%s\n",objListNames[i]);
      //}

   }

   void ClearAstrObjs() {

       NameSelectors = new String[1];
       NameSelectors[0] = "null";
       RASelectors = new String[1];
       RASelectors[0] =   "null";

       java.util.List<String> namekeys = new ArrayList<String>(byname.keySet());
       java.util.List<Double> rakeys = new ArrayList<Double>(byra.keySet());


       //Iterator<Map.Entry<String,AstrObj>> byraiterator = byra.EntrySet.iterator();
       //while(byraiterator.hasNext()) {
       //   Map.Entry<String,AstrObj> entry = byraiterator.getNext();
       //   String k = entry.getKey();
//	  byraiterator.remove();
 //         System.out.printf("Killed key %s\n",k);
  //     }
       Iterator raiterator = rakeys.iterator();
       while(raiterator.hasNext()) {
          Object key =  raiterator.next();
          AstrObj tempobj = byra.get(key);
          byra.remove(key);
          presenterKey.remove(tempobj.name);
       }
//       byra.keySet().removeAll();
       byra.clear();
//       byra.keySet().clear();
//       presenterKey.clear();
/*
       java.util.List<Double> diagnostic_list = new ArrayList<Double>(byra.keySet());
       Iterator it2 = diagnostic_list.iterator();
       System.out.printf("After removal, ra keys are ... \n");
       while(it2.hasNext()) {
          Object key =  it2.next();
          AstrObj tempobj = byra.get(key);
          byra.remove(key);
          presenterKey.remove(tempobj.name);
         System.out.printf("key %s\n",key);
      }
 */
       Iterator nameiterator = namekeys.iterator();
       while(nameiterator.hasNext()) {
          Object key =  nameiterator.next();
          byname.remove(key);
       }

       Celest nullc = new Celest(0.,0.,2000.);
       AstrObj nullobj = new AstrObj("null",nullc);
       byra.put(0., nullobj);
       byname.put("null", nullobj);
       presenterKey.put("null", nullobj);

       // AstrObjSelector objsel = new AstrObjSelector();
//       ObjSelWin.LoadList();   // which is now empty.
//       AirmSelWin.LoadList();
       LoadList();
   }

   void SelObjByPos(Celest incel) {
    /* get input from the graphical display, or wherever, and get the nearest
       object on the list within a tolerance.  Precession is ignored. */
       double tolerance = 0.1;  // radians
       double decband = 6.;     // degrees
       double decin;
       Celest objcel;
       double sep, minsep = 1000000000000.;
       int i, minindex = 0;

       if(presenterKey.size() > 0) {
          decin = incel.Delta.value;
          for(i = 0; i < RASelectors.length; i++) {
              objcel = presenterKey.get(RASelectors[i]).c;
              if(Math.abs(decin - objcel.Delta.value) < decband) {  // guard expensive subtend
                  sep = Spherical.subtend(incel,objcel);
                  if (sep < minsep) {
                      minsep = sep;
                      minindex = i;
                  }
             }
          }

          if(minsep < tolerance) {
              objcel = presenterKey.get(RASelectors[minindex]).c;
//              objnamefield.setText(RASelectors[minindex]);
//              RAfield.setText(objcel.Alpha.RoundedRAString(2," "));
//              decfield.setText(objcel.Delta.RoundedDecString(1," "));
//              equinoxfield.setText(String.format(Locale.ENGLISH, "%7.2f",objcel.Equinox));
          }
      }
   }
/*================================================================================================
/      public void synchOutput()
/=================================================================================================*/
    public void synchOutput(){
        // This should call the synchOutput() method of the calling class JSkyCalcWindow
        // it is segregated here so that we only need to either make the reference or propagate
        // the synchOutput event from one location.
    }
/*================================================================================================
/      class AstroObjSelector
/=================================================================================================*/

 }

   