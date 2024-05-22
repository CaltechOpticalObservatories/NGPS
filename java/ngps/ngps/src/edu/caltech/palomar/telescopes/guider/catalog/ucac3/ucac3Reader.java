package edu.caltech.palomar.telescopes.guider.catalog.ucac3;  
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
import edu.dartmouth.jskycalc.util.Conv;
import java.io.*;
import java.util.List.*;
import java.util.Hashtable;
import java.util.ArrayList;
import java.util.Stack;
import java.util.Enumeration;
import edu.dartmouth.jskycalc.util.filewriter;
import edu.caltech.palomar.telescopes.P200.TelescopesIniReader;
import java.util.Vector;
import edu.caltech.palomar.telescopes.guider.catalog.ucac3.UCAC3QueryResult;
import java.beans.*;
import edu.caltech.palomar.telescopes.guider.catalog.server.CatalogIniReader;
/*================================================================================================
/     ucac3Reader
/=================================================================================================*/
public class ucac3Reader {
   private transient PropertyChangeSupport propertyChangeListeners = new PropertyChangeSupport(this);
   public ucac3Star ustars[];
   public int nstars_read = 0;
   int nselected;
   byte stardat[];
   static int star_record_length = 84;
//   static int index_file_bytes = 345600;
   static int index_file_bytes = 691200;
   static int index_file_lines = 86400;
   static int max_n_stars = 70000;  // roughly the max in any 53 arcmin sq
   static byte [] indexdat;
//   static String u3path = "/home/ucac3";  // HARD-CODED -- change to move.
   public   java.lang.String USERDIR           = System.getProperty("user.dir");
   public   java.lang.String SEP               = System.getProperty("file.separator");
   public   java.lang.String CONFIG            = SEP + "config" + SEP;
   public   java.lang.String RESOURCES         = "resources" + SEP;
   public   java.lang.String CATALOG           = "ucac3-index" + SEP;
   public   java.lang.String PATH              = USERDIR + CONFIG + RESOURCES + CATALOG;
   public   String u3path;  // HARD-CODED -- change to move.
   public   Hashtable indexHash    = new Hashtable();
   public   Vector    objectVector = new Vector();
   public   ArrayList segmentList  = new ArrayList();
   public   Stack     searchStack  = new Stack();
   public static java.lang.String  tempDir        = "temp";
   public static java.lang.String  catalogDir     = "catalog";
   public static java.lang.String  filePrefix     = "UCAC3";
   public static java.lang.String  fileSuffix     = ".csv";
   private       java.lang.String  downloadFileName;
   public TelescopesIniReader      myTelescopesIniReader;
   public CatalogIniReader         myCatalogIniReader;
   public boolean    status;
   public boolean    print;
/*================================================================================================
/      ucac3Reader()
/=================================================================================================*/
   public ucac3Reader(TelescopesIniReader     newTelescopesIniReader) {
       this.myTelescopesIniReader = newTelescopesIniReader;
       u3path = myTelescopesIniReader.UCAC3PATH;
       setPrint(false);
       initialize();
   }
/*================================================================================================
/      ucac3Reader()
/=================================================================================================*/
   public ucac3Reader(CatalogIniReader     newCatalogIniReader) {
       this.myCatalogIniReader = newCatalogIniReader;
       u3path = myCatalogIniReader.UCAC3PATH;
       setPrint(false);
       initialize();
   }
/*================================================================================================
/     initialize()
/=================================================================================================*/
public void initialize(){
      execute_initializeIndex();
      nselected = 0;
      stardat   = new byte [star_record_length];
      ustars    = new ucac3Star[max_n_stars];
//     indexdat = new byte[index_file_bytes];
}
/*================================================================================================
/      setInitialized(boolean state)
/=================================================================================================*/
public void setInitialized(boolean newInitialized){
    boolean  oldInitialized = status;
    status = newInitialized;
    propertyChangeListeners.firePropertyChange("initialized", Boolean.valueOf(oldInitialized),Boolean.valueOf(newInitialized));
}
public boolean isInitialized(){
    return status;
}
/*=============================================================================================
/     addPropertyChangeListener(PropertyChangeListener l)
/=============================================================================================*/
  public synchronized void removePropertyChangeListener(PropertyChangeListener l) {
    propertyChangeListeners.removePropertyChangeListener(l);
  }
  public synchronized void addPropertyChangeListener(PropertyChangeListener l) {
    propertyChangeListeners.addPropertyChangeListener(l);
  }
/*================================================================================================
/    testQuery()
/=================================================================================================*/
 public void testQuery(){
//    double RA     = 202.48219583;
//    double DEC    = 47.23150889;
    double RA     = 0;
    double DEC    = 0;
    double BOX    = 15.0;
    int    MAX    = 500;
    long start    = 0;
    long end      = 0;
    long delta    = 0;

    for(int i=0;i<MAX;i++){
       RA = 360.0*Math.random();
       DEC = 90*Math.random();
       start = System.currentTimeMillis();
       queryCatalog(RA, DEC, BOX);
       end   = System.currentTimeMillis();
       delta = end - start;
//       System.out.println(RA+","+DEC+","+delta);
       System.out.println(delta);
       waitMilliseconds(20);
    }
}
/*================================================================================================
/      waitForResponse(int newDelay)
/=================================================================================================*/
public void waitMilliseconds(int newDelay){
     try{
        Thread.currentThread().sleep(newDelay);} // sleep for awhile to let the messages pile up
      catch (Exception e){
    }
}/*================================================================================================
/     queryCatalog(double racent, double deccent, double arcminbox)
/=================================================================================================*/
 public Vector queryCatalog(double racent, double deccent, double arcminbox){
       objectVector.clear();
       SearchBox sb = new SearchBox(racent, deccent,arcminbox);
       sb.print();
       constructTestSegmentsList(racent,deccent);
       evaluateSegmentsList(racent,deccent,arcminbox);
       while(!searchStack.empty()){
            ucac3index current_index = (ucac3index)searchStack.pop();
            readSegment(sb,current_index);
       }
       printObjectList();
    return objectVector;
 }
/*================================================================================================
/     setPrint(boolean newPrint)
/=================================================================================================*/
public void setPrint(boolean newPrint){
    print = newPrint;
}
public boolean isPrint(){
    return print;
}
/*================================================================================================
/    printObjectList()
/=================================================================================================*/
 public void printObjectList(){
//   setPrint(true);
   if(print){
     java.lang.String currentStarString = new java.lang.String();
     java.lang.String fileName = constructTemporaryFileName();
     filewriter myfilewriter = new filewriter(fileName);
     Enumeration e = objectVector.elements();
      while(e.hasMoreElements()){
      Vector current_star = (Vector)e.nextElement();
      currentStarString = (Double)current_star.elementAt(0)*15.0+","+(Double)current_star.elementAt(1);
      System.out.println(currentStarString);
      myfilewriter.pw.println(currentStarString);
    }
    myfilewriter.pw.flush();
    myfilewriter.pw.close();
   }
//   setPrint(false);
  }
 /*=========================================================================================================
 /     constructTemporaryFileName()
 /=========================================================================================================*/
 protected java.lang.String constructTemporaryFileName(){
    java.lang.String tempFileName = new java.lang.String();
    java.lang.String randomString = new java.lang.String();
    java.security.SecureRandom  myRandom = new java.security.SecureRandom();
    int    randomNumber    = myRandom.nextInt();
    Integer myRandomInteger = Integer.valueOf(Math.abs(randomNumber));
    randomString    = myRandomInteger.toString();

    java.lang.String tempFile     = filePrefix + randomString + fileSuffix;
    tempFileName = USERDIR + SEP + tempDir + SEP + catalogDir + SEP + tempFile;
    return tempFileName;
 }
/*================================================================================================
/     test()
/=================================================================================================*/
 protected void test(){
     // setup an RA and DEC (M51) that can be used to test some functions
    double RA     = 202.48219583;
    double DEC    = 47.23150889;
//    double RA     = 0;
//   double DEC    = -89.9;
    double BOX    = 60.0;
    constructTestSegmentsList(RA,DEC);
    evaluateSegmentsList(RA,DEC,BOX);
    testSearchBox(RA,DEC,BOX);
    testPositives(1000);
    testNegatives(1000);
    testLimits(RA, DEC);
    testLimits(0.0,   0.0);
    testLimits(0.0,  90.0);
    testLimits(0.0, -90.0);
    testLimits(0.0,  45.0);
    testLimits(0.0, -45.0);
    testLimits(0.0, 45);
    testLimits(90,  45);
    testLimits(180, 45);
    testLimits(270, 45);
    testLimits(360, 45);
    testLimits(0.0, -45);
    testLimits(90,  -45);
    testLimits(180, -45);
    testLimits(270, -45);
    testLimits(360, -45);
    // test the retrieval of all the records in a zone
//    readZone(RA,DEC);
 }    
/*================================================================================================
/     testSearchBox(double testRA, double testDec, double arcminbox)
/=================================================================================================*/
protected void testSearchBox(double testRA, double testDec, double arcminbox){
    SearchBox sb = new SearchBox(testRA, testDec,arcminbox);
    sb.print();
}
/*================================================================================================
/     initializeIndex()
/=================================================================================================*/
 protected boolean testLimits(double testRA, double testDec){
     boolean test = false;
     // test the retrieval of the zone and bin number return functions
    int    zone   = zone_number(testDec);
    int    ra_bin = ra_bin_index(testRA);
    // test the retrieval of the correct index record
    String bin_index_name = String.format("%s:%s", zone,ra_bin);
          ucac3index selectedIndex = (ucac3index)indexHash.get(bin_index_name);
    System.out.println("Target RA  = "+testRA);
    System.out.println("Target DEC = "+ testDec);
    System.out.println("Segment RA bin = "+ selectedIndex.ra_bin_index);
    System.out.println("Segment RA:  start = "+ selectedIndex.start_ra + " end = " + selectedIndex.end_ra);
    System.out.println("Segment Dec: start = "+ selectedIndex.start_dec + " end = " + selectedIndex.end_dec);

    test = isPointInSegment(selectedIndex,testRA,testDec);
    System.out.println("Point is in segment test = "+test);

     return test;
 }
/*================================================================================================
/     testNegatives()
/=================================================================================================*/
 protected void testNegatives(int number_tests){
     int MAX    = number_tests;
     int failures = 0;
     System.out.println("Testing randomly for negatives: Number of Cycles = " + MAX);
     int zone   = 10;
     int ra_bin = 10;
     double ratest;
     double dectest;
     String bin_index_name = String.format("%s:%s", zone,ra_bin);
          ucac3index selectedIndex = (ucac3index)indexHash.get(bin_index_name);
     for(int i=0;i<MAX;i++){
       ratest = 360.0*Math.random();
       dectest = 90*Math.random();
       boolean test = isPointInSegment(selectedIndex,ratest,dectest);
        if(!test){
           failures = failures +1;
       }
      System.out.println("Point is in segment test = "+test);
     }
     System.out.println("Failures = " + failures);
 }
/*================================================================================================
/     testNegatives()
/=================================================================================================*/
 protected void testPositives(int number_tests){
     int MAX = number_tests;
     int failures = 0;
     System.out.println("Testing randomly for positives: Number of Cycles = " + MAX);
     int zone   = 0;
     int ra_bin = 0;
     double ratest;
     double dectest;
      for(int i=0;i<MAX;i++){
       ratest = 360.0*Math.random();
       dectest = 90*Math.random();

       zone   = zone_number(dectest);
       ra_bin = ra_bin_index(ratest);
       // test the retrieval of the correct index record
       String bin_index_name = String.format("%s:%s", zone,ra_bin);
          ucac3index selectedIndex = (ucac3index)indexHash.get(bin_index_name);
       boolean test = isPointInSegment(selectedIndex,ratest,dectest);
       if(!test){
           failures = failures +1;
       }
 //    System.out.println("Target RA  = "+ratest);
  //   System.out.println("Target DEC = "+ dectest);
 //    System.out.println("Segment RA bin = "+ selectedIndex.ra_bin_index);
 //    System.out.println("Segment RA:  start = "+ selectedIndex.start_ra + " end = " + selectedIndex.end_ra);
 //    System.out.println("Segment Dec: start = "+ selectedIndex.start_dec + " end = " + selectedIndex.end_dec);
      System.out.println("Point is in segment test = " + test);
     }
     System.out.println("Failures = " + failures);
  }
/*================================================================================================
/     initializeIndex()
/=================================================================================================*/
 public void initializeIndex(){
     setInitialized(false);
       BufferedInputStream indstr = null;
       try {
               BufferedReader bufread = new BufferedReader(new FileReader(u3path + SEP +"u3index.asc"));
               int     lineNumber = 0;
               java.lang.String currentLine = new java.lang.String();
               while(lineNumber < index_file_lines){
                  try{
                                 currentLine = bufread.readLine();
                    ucac3index   currentindex = new ucac3index(currentLine);
                    calculateBinLimits(currentindex);
                                 indexHash.put(currentindex.segment_name, currentindex);
                                 lineNumber = lineNumber +1;
//                                 System.out.println("Line Number = " + lineNumber);
                  }catch(Exception e){
                      System.out.println("Error reading a line in the index file. LineNumber = " + lineNumber + " Error Message = "+e.toString());
                   }
               }
               bufread.close();
       } catch (Exception e) {
         System.out.printf("Index file not found!!\n");
      }
    setInitialized(true);
 }
/*================================================================================================
/     execute_initializeIndex()
/=================================================================================================*/
public void execute_initializeIndex(){
    InitializeIndexThread myInitializeIndexThread = new InitializeIndexThread();
    myInitializeIndexThread.start();
 }
/*================================================================================================
/     isPointInSegment(ucac3index index,double RA,double DEC)
/=================================================================================================*/
public boolean isPointInSegment(ucac3index index,double RA,double DEC){
   boolean isInside = false;
   try{
   if(RA >= index.start_ra & RA <= index.end_ra){
      if(DEC >= index.start_dec & DEC <= index.end_dec){
        isInside = true;
      }     
   }
   } catch (Exception e){
       System.out.println("Error checking is point is in the segment");
   }
   return isInside;
}
/*================================================================================================
/     isPointInSearchBox(SearchBox sb,double RA,double DEC)
/=================================================================================================*/
public boolean isPointInSearchBox(SearchBox sb,double RA,double DEC){
   boolean isInside = false;
      if(RA >= sb.ra_min & RA <= sb.ra_max){
      if(DEC >= sb.dec_min & DEC <= sb.dec_max){
        isInside = true;
      }
   }
   return isInside;
}
/*================================================================================================
/     printSegmentLimits(ucac3index segment)
/=================================================================================================*/
public void printSegmentLimits(ucac3index segment){
if(print){
    System.out.println(segment.start_dec+","+segment.start_ra);
    System.out.println(segment.start_dec+","+segment.end_ra);
    System.out.println(segment.end_dec+","+segment.end_ra);
    System.out.println(segment.end_dec+","+segment.start_ra);

    java.lang.String fileName = constructTemporaryFileName();
     filewriter myfilewriter = new filewriter(fileName);

      myfilewriter.pw.println(segment.start_dec+","+segment.start_ra);
      myfilewriter.pw.println(segment.start_dec+","+segment.end_ra);
      myfilewriter.pw.println(segment.end_dec+","+segment.end_ra);
      myfilewriter.pw.println(segment.end_dec+","+segment.start_ra);

    myfilewriter.pw.flush();
    myfilewriter.pw.close();
  }
}
/*================================================================================================
/     isBoxInSegment(double testRA, double testDec, double arcminbox,ucac3index segment)
/=================================================================================================*/
public boolean isBoxInSegment(double testRA, double testDec, double arcminbox,ucac3index segment){
    boolean status = false;
    SearchBox sb = new SearchBox(testRA, testDec,arcminbox);
    // Test each of the corners to see if any of the corners is in the segment
    boolean testcenter = isPointInSegment(segment,sb.center.ra,sb.center.dec);
    boolean testUL     = isPointInSegment(segment,sb.UL.ra,sb.UL.dec);
    boolean testLL     = isPointInSegment(segment,sb.LL.ra,sb.LL.dec);
    boolean testUR     = isPointInSegment(segment,sb.UR.ra,sb.UR.dec);
    boolean testLR     = isPointInSegment(segment,sb.LR.ra,sb.LR.dec);
    boolean testU      = isPointInSegment(segment,sb.U.ra,sb.U.dec);
    boolean testD      = isPointInSegment(segment,sb.D.ra,sb.D.dec);
    boolean testL      = isPointInSegment(segment,sb.L.ra,sb.L.dec);
    boolean testR      = isPointInSegment(segment,sb.R.ra,sb.R.dec);
    printSegmentLimits(segment);
    if(testcenter | testUL | testLL| testUR | testLR | testU | testD | testL | testR){
        status = true;
    }
   return status;
}
/*================================================================================================
/     isBoxInSegment(double testRA, double testDec, double arcminbox,ucac3index segment)
/=================================================================================================*/
 public void evaluateSegmentsList(double testRA, double testDec, double arcminbox){
   boolean test0 = isBoxInSegment(testRA,testDec,arcminbox,(ucac3index)segmentList.get(0));
   boolean test1 = isBoxInSegment(testRA,testDec,arcminbox,(ucac3index)segmentList.get(1));
   boolean test2 = isBoxInSegment(testRA,testDec,arcminbox,(ucac3index)segmentList.get(2));
   boolean test3 = isBoxInSegment(testRA,testDec,arcminbox,(ucac3index)segmentList.get(3));
   boolean test4 = isBoxInSegment(testRA,testDec,arcminbox,(ucac3index)segmentList.get(4));
   boolean test5 = isBoxInSegment(testRA,testDec,arcminbox,(ucac3index)segmentList.get(5));
   boolean test6 = isBoxInSegment(testRA,testDec,arcminbox,(ucac3index)segmentList.get(6));
   boolean test7 = isBoxInSegment(testRA,testDec,arcminbox,(ucac3index)segmentList.get(7));
   boolean test8 = isBoxInSegment(testRA,testDec,arcminbox,(ucac3index)segmentList.get(8));
//   System.out.println("Segments Evaluation = " + test0 + " " + test1 + " "+test2 + " "+test3+ " "+test4 +" "+test5 +" "+test6 +" "+test7+ " "+test8 +" ");
   if(test8){
       searchStack.add((ucac3index)segmentList.get(8));
   }
    if(test7){
       searchStack.add((ucac3index)segmentList.get(7));
   }
    if(test6){
       searchStack.add((ucac3index)segmentList.get(6));
  }
    if(test5){
       searchStack.add((ucac3index)segmentList.get(5));
  }
   if(test4){
       searchStack.add((ucac3index)segmentList.get(4));
  }
   if(test3){
       searchStack.add((ucac3index)segmentList.get(3));
  }
   if(test2){
       searchStack.add((ucac3index)segmentList.get(2));
  }
   if(test1){
       searchStack.add((ucac3index)segmentList.get(1));
  }
  if(test0){
       searchStack.add((ucac3index)segmentList.get(0));
  }
}
/*================================================================================================
/     isPointInSegment(ucac3index index,double RA,double DEC)
/=================================================================================================*/
public void constructTestSegmentsList(double testRA, double testDec){
    // first we need to find the primary segment that contains the testRA and testDec
    // we do this using the zone_number and ra_bin_index methods.  Once we have the\
    // first of the segments we need to get the set of 8 additional segments that surround it.
    int    zone   = zone_number(testDec);
    int    ra_bin = ra_bin_index(testRA);

     String pindex   = String.format("%s:%s", zone,ra_bin);
     String pindexM1 = String.format("%s:%s", zone,testWrapRA(ra_bin-1));   // primary index in the same zone but -1 in ra_bin
     String pindexP1 = String.format("%s:%s", zone,testWrapRA(ra_bin+1));   // primary index in the same zone but +1 in ra_bin
     String lindex   = String.format("%s:%s", testWrapDec(zone-1),ra_bin);   // left index in the same zone but +1 in ra_bin
     String lindexM1 = String.format("%s:%s", testWrapDec(zone-1),testWrapRA(ra_bin-1)); // left index in the same zone but +1 in ra_bin
     String lindexP1 = String.format("%s:%s", testWrapDec(zone-1),testWrapRA(ra_bin+1)); // left index in the same zone but +1 in ra_bin
     String rindex   = String.format("%s:%s", testWrapDec(zone+1),ra_bin);   // right index in the same zone but +1 in ra_bin
     String rindexM1 = String.format("%s:%s", testWrapDec(zone+1),testWrapRA(ra_bin-1)); // right index in the same zone but +1 in ra_bin
     String rindexP1 = String.format("%s:%s", testWrapDec(zone+1),testWrapRA(ra_bin+1)); // right index in the same zone but +1 in ra_bin
// Now retrieve the indexes in the primary zone
    ucac3index P0  = (ucac3index)indexHash.get(pindex);
    ucac3index PM1 = (ucac3index)indexHash.get(pindexM1);
    ucac3index PP1 = (ucac3index)indexHash.get(pindexP1);
 // Now retrieve the indexes to the left of the primary zone
    ucac3index L0  = (ucac3index)indexHash.get(lindex);
    ucac3index LM1 = (ucac3index)indexHash.get(lindexM1);
    ucac3index LP1 = (ucac3index)indexHash.get(lindexP1);
 // Now retrieve the indexes to the right of the primary zone
    ucac3index R0  = (ucac3index)indexHash.get(rindex);
    ucac3index RM1 = (ucac3index)indexHash.get(rindexM1);
    ucac3index RP1 = (ucac3index)indexHash.get(rindexP1);
   // now add these to the ArrayList data structure
    segmentList.add(0, P0);
    segmentList.add(1, PM1);
    segmentList.add(2, PP1);
    segmentList.add(3, L0);
    segmentList.add(4, LM1);
    segmentList.add(5, LP1);
    segmentList.add(6, R0);
    segmentList.add(7, RM1);
    segmentList.add(8, RP1);
}
/*================================================================================================
/     testWrapRA(int test_value)
/=================================================================================================*/
protected int testWrapRA(int test_value){
  // this function simply tests to see if the number is less than zero or greater than 240 and returns the
  // adjacent number
    if(test_value > 240){
        test_value = test_value - 240;
    }
    if(test_value < 0){
        test_value = 240 - test_value;
    }
    return test_value;
}
/*================================================================================================
/     testWrapDec(int test_value)
/=================================================================================================*/
protected int testWrapDec(int test_value){
  // this function simply tests to see if the number is less than zero or greater than 240 and returns the
  // adjacent number
    if(test_value > 360){
        test_value = test_value - 360;
    }
    if(test_value < 0){
        test_value = 360 - test_value;
    }
    return test_value;
}
/*================================================================================================
/     readZone(ucac3index index)
/=================================================================================================*/
public void readSegment(SearchBox sb,ucac3index index){
//  A segment is one of the 86400 areas of the sky that is 0.5 degrees in
//  declination and 0.1 hours in size. (30 arcminutes by 90 arcminutes). 
   readSegment(sb,index.zone_number,index.ra_bin_index);
}
/*================================================================================================
/     readZone(int zone,int ra_bin)
/=================================================================================================*/
 public void readSegment(SearchBox sb,int zone,int ra_bin){
 //  A segment is one of the 86400 areas of the sky that is 0.5 degrees in
//  declination and 0.1 hours in size. (30 arcminutes by 90 arcminutes).
    boolean check = false;
    long byteskip = 0L;
    long starskip = 0L;
    int  n_read   = 0;
    String bin_index_name = String.format("%s:%s", zone,ra_bin);
    ucac3index selectedIndex = (ucac3index)indexHash.get(bin_index_name);
               starskip = selectedIndex.running_star_number;
               byteskip = starskip * star_record_length;

      // open file for this zone -- separately for every wrap pass.
      String fname = String.format("%s/z%03d",u3path,zone);
      // System.out.printf("opening %s\n",fname);
      try {
         BufferedInputStream  instream = new BufferedInputStream(new FileInputStream(fname));
         // any skipping needs to be in try-catch.
        //       System.out.printf("Skipping %d stars or %d bytes\n",starskip,byteskip);
        instream.skip(byteskip);
        int recordNumber = 0;
           while(recordNumber < selectedIndex.number_stars_bin) {
                 // read stars one by one for now
                 n_read = instream.read(stardat,0,star_record_length);
                 recordNumber = recordNumber + 1;
                  if(n_read == 84) {
                     ucac3Star ust = new ucac3Star();  // empty instance
                               ust.fill(stardat);
                               check = isPointInSearchBox(sb,ust.ra,ust.dec);
                               if(check){
                                  objectVector.add(ust.columnVector);
                               }
                  }
          }       
         // System.out.printf("Opened ok.\n");
        instream.close();
      } catch (Exception e) {
         System.out.printf("File %s not found!!\n",fname);
      }
 }
/*================================================================================================
/     calculateBinLimits()
/=================================================================================================*/
public void calculateBinLimits(ucac3index index){
    index.start_ra = bin_degrees_start(index.ra_bin_index);
    index.end_ra = bin_degrees_end(index.ra_bin_index);
    index.start_dec = zone_degrees_start(index.zone_number);
    index.end_dec   =  zone_degrees_end(index.zone_number);
}
/*================================================================================================
/    zone_degrees_end(int zoneNumber)
/=================================================================================================*/
public double zone_degrees_end(int zoneNumber){
  double z   = (double)zoneNumber;
  double dec = (z-1.0)/2 - 90.0 + 0.5;
  return dec;
}
/*================================================================================================
/    zone_degrees_start(int zoneNumber)
/=================================================================================================*/
public double zone_degrees_start(int zoneNumber){
  double z   = (double)zoneNumber;
  double dec = (z-1.0)/2 - 90.0;
  return dec;
}
/*================================================================================================
/    bin_degrees_start(int binNumber)
/=================================================================================================*/
public double bin_degrees_start(int binNumber){
   double bin = (double)binNumber;
   double ra  = 1.5*bin-1.5;
 return ra;
}
/*================================================================================================
/    bin_degrees_end(int binNumber)
/=================================================================================================*/
public double bin_degrees_end(int binNumber){
   double bin = (double)binNumber;
   double ra  = 1.5*bin;
 return ra;
}
  // Methods to figure out where to start reading the binary data.
  // index file is in Fortran order, so first 360 entries are zero --
  // i.e., cumulative number of stars in zone at start of zone; next
  // 360 go from a few up to quite a few back down (number in first 0.1 h,
  // as a function of dec); and so on.
/*================================================================================================
/    int zone_number(double dec)
/=================================================================================================*/
   public int zone_number(double dec) {  // 360 zones from S pole to N.
       int z = 1 + (int) (2. * (90. + dec));
       if(z < 1) z = 1;
       if(z > 360) z = 360;
       return z;
   }
/*================================================================================================
/    ra_bin_index(double ra)
/=================================================================================================*/
   public int ra_bin_index(double ra){
       // First turn the ra from degrees to hours
       int z = (int)Math.ceil(ra/1.5);
 //      int z = (int)Math.round(ra_hr/0.1);
       if(z==0){
           z=1;
       }
       if(z>360){
           z=360;
       }
       return z;
   }
/*================================================================================================
/    zone_offset(int zone, double ra)
/=================================================================================================*/
   public int zone_offset(int zone, double ra) {
      // looks up running number of star at start of 0.1-hr subzone, in
      // dec zone 'zone'.
      int ra_ind = (int) (ra * 10.);
      return(Conv.getint32(indexdat,(4 * (zone - 1) + 1440 * ra_ind)));
   }
/*================================================================================================
/       zone_offset(double ra, double dec)
/=================================================================================================*/
   int zone_offset(double ra, double dec) { // overloaded.
      // as above, but for ra and dec.
      int zone = zone_number(dec);
      int ra_ind = (int) (ra * 10.);
      return(Conv.getint32(indexdat,(4 * (zone - 1) + 1440 * ra_ind)));
   }
/*=============================================================================================
/     degreesToRadians(double degrees)
/=============================================================================================*/
 public double degreesToRadians(double degrees){
   double radians = degrees*(Math.PI/180.00);
   return radians;
 }
/*=============================================================================================
/       INNER CLASS Point contains the x-y coordinates of a point
/=============================================================================================*/
public class Point extends java.lang.Object{
    public double ra;
    public double dec;
    public Point(double x,double y){
        this.ra = x;
        this.dec = y;       
    }
}
/*=============================================================================================
/       INNER CLASS Point contains the x-y coordinates of a point
/=============================================================================================*/
public class SearchBox extends java.lang.Object{
    public Point  UL;
    public Point  LL;
    public Point  UR;
    public Point  LR;
    public Point  L;
    public Point  R;
    public Point  U;
    public Point  D;
    public Point  center;
    public double arcmin_box_degree;
    public double ra_min;
    public double ra_max;
    public double dec_min;
    public double dec_max;
    public double delta_box;
    public double delta_box_ra;
/*=============================================================================================
/         SearchBox(double racent, double deccent, double arcminbox)
/=============================================================================================*/
    public SearchBox(double racent, double deccent, double arcminbox){
       this.arcmin_box_degree = arcminbox/60.0;
       this.delta_box         = (arcmin_box_degree/2.0);
       this.delta_box_ra      = delta_box/Math.cos(degreesToRadians(deccent));
       center = new Point(racent,deccent);
       construct();
    }
/*=============================================================================================
/       construct()
/=============================================================================================*/
   private void construct(){
    // get the box size in degrees to make the calculations easier
    // test for the simple case of no wrapping in ra

    if((center.ra > (delta_box_ra))&(center.ra < (360.0-delta_box_ra))){
      ra_min = center.ra - (delta_box_ra);
      ra_max = center.ra + (delta_box_ra);
    }
    if(Math.abs(center.dec) < (90.0 - delta_box)){
      dec_min = center.dec - (delta_box);
      dec_max = center.dec + (delta_box);
    }
    // Now deal with the case where
    if(center.ra < delta_box_ra){
      // this is case where the RA is close to zero and it wraps to t number close to 360
        ra_min = 360+(center.ra -delta_box_ra);
        ra_max = center.ra + (delta_box_ra);
    }
    if(center.ra > (360.0-delta_box_ra)){
      // this is case where the RA is close to 360 and wraps around to something close to zero
        ra_max = (center.ra + delta_box_ra) - 360;
        ra_min = center.ra - (delta_box_ra);
   }
    if(Math.abs(center.dec) > (90.0 - delta_box)){
        // this is the case where the declination is near the north or the south pole and the box will cause it to wrap around
        if(center.dec > 0.0){
           // if we are close to the north pole
           dec_max = 90+(90-(center.dec + delta_box));
           dec_min = center.dec - (delta_box);
       }
        if(center.dec < 0.0){
           dec_max = -(90+(90-(Math.abs(center.dec) + delta_box)));
           dec_min = -(Math.abs(center.dec) - (delta_box));
       }
    }
    UL = new Point(ra_min,dec_max);
    LL = new Point(ra_min,dec_min);
    UR = new Point(ra_max,dec_max);
    LR = new Point(ra_max,dec_min);
    L  = new Point(ra_min,center.dec);
    R  = new Point(ra_max,center.dec);
    U  = new Point(center.ra,dec_max);
    D  = new Point(center.ra,dec_min);
    }
 /*=============================================================================================
/       print()
/=============================================================================================*/
public void print(){
// setPrint(true);
 if(print){
    System.out.println("Center = ," + center.ra + "," + center.dec);
     System.out.println("UL = ," + UL.ra + "," + UL.dec);
     System.out.println("LL = ," + LL.ra + "," + LL.dec);
     System.out.println("UR = ," + UR.ra + "," + UR.dec);
     System.out.println("LR = ," + LR.ra + "," + LR.dec);
     System.out.println("U = ," + U.ra + "," + U.dec);
     System.out.println("D = ," + D.ra + "," + D.dec);
     System.out.println("L = ," + L.ra + "," + L.dec);
     System.out.println("R = ," + R.ra + "," + R.dec); 

     java.lang.String fileName = constructTemporaryFileName();
     filewriter myfilewriter = new filewriter(fileName);

      myfilewriter.pw.println("Center = ," + center.ra + "," + center.dec);
      myfilewriter.pw.println("UL = ," + UL.ra + "," + UL.dec);
      myfilewriter.pw.println("LL = ," + LL.ra + "," + LL.dec);
      myfilewriter.pw.println("UR = ," + UR.ra + "," + UR.dec);
      myfilewriter.pw.println("LR = ," + LR.ra + "," + LR.dec);
      myfilewriter.pw.println("U = ," + U.ra + "," + U.dec);
      myfilewriter.pw.println("D = ," + D.ra + "," + D.dec);
      myfilewriter.pw.println("L = ," + L.ra + "," + L.dec);
      myfilewriter.pw.println("R = ," + R.ra + "," + R.dec);
      
    myfilewriter.pw.flush();
    myfilewriter.pw.close();
  }
}
}
/*=============================================================================================
/       INNER CLASS that initializes the index in a separate thread
/=============================================================================================*/
    public class InitializeIndexThread  implements Runnable{
    private Thread myThread;
/*=============================================================================================
/          InitializeIndexThread()
/=============================================================================================*/
    private InitializeIndexThread(){
    }
/*=============================================================================================
/           run()
/=============================================================================================*/
    public void run(){
       System.out.println("Start Reading the UCAC3 index file");
       initializeIndex();
       System.out.println("Finished Reading the UCAC3 index file");
    }
/*=============================================================================================
/         start()
/=============================================================================================*/
    public void start(){
     myThread = new Thread(this);
     myThread.start();
     }
    }// End of the InitializeIndexThread Inner Class
/*=============================================================================================
/                           End of the InitializeIndexThread Class
/=============================================================================================*/
}

