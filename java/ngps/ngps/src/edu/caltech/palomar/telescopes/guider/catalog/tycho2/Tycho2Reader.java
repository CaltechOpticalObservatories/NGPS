/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package edu.caltech.palomar.telescopes.guider.catalog.tycho2;
//=== File Prolog =============================================================
//	This code was developed by California Institute of Technology

import java.util.Vector;
import jsky.coords.CoordinateRadius;
import jsky.science.Coordinates;
import jsky.coords.WorldCoords;

//      Caltech Optical Observatories, Palomar Observatory
//
//--- Contents ----------------------------------------------------------------
//	 Tycho2Reader
//
//--- Description -------------------------------------------------------------
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
//
//      June 21, 2011 Jennifer Milburn
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
import java.io.File;
import java.io.FileInputStream;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.FileOutputStream;
import java.io.BufferedWriter;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.Enumeration;
import com.sharkysoft.printf.Printf;
import com.sharkysoft.printf.PrintfData;
import com.sharkysoft.printf.PrintfTemplate;
import com.sharkysoft.printf.PrintfTemplateException;
import java.util.ArrayList;
import java.nio.channels.FileChannel;
import java.beans.PropertyChangeListener;
import java.beans.PropertyChangeSupport;
import edu.caltech.palomar.telescopes.guider.catalog.tycho2.Tycho2TableModel;
/*================================================================================================
/     Tycho2Reader() Class Declaration
/=================================================================================================*/
public class Tycho2Reader {
    transient private PropertyChangeSupport propertyChangeListeners = new PropertyChangeSupport(this);
    public java.util.Vector    catalog_vector  = new java.util.Vector();
    public java.util.Hashtable index_vector    = new java.util.Hashtable();
    public java.util.ArrayList index_arraylist = new java.util.ArrayList();
    public java.util.Vector    results_vector  = new java.util.Vector();
    public java.util.Vector    search_vector   = new java.util.Vector();
    private BufferedReader  br;
    private BufferedWriter  bw;
    private java.lang.String catalog_file_name;
    private java.lang.String index_file_name;
    public    java.lang.String      TERMINATOR        = "\n";
    public static int               RECORD_LENGTH     = 124;
    public static int               CATALOG_LENGTH    = 351864;
    public static int               ZONE_LENGTH       = 9536;
    public static double            ARCMIN_DEGREE     = 60.0;
    private double                  RA;
    private double                  DEC;
    private double                  selectedRA;
    private double                  selectedDEC;
    private java.lang.String        selectedName = new java.lang.String();
    private Tycho2TableModel        myTycho2TableModel;
    private boolean                 status;
/*================================================================================================
/     Tycho2Reader() Constructor
/=================================================================================================*/
   public Tycho2Reader(){ 
      init();
//     executeReadIndex();
//      test();
   }
/*================================================================================================
/     init()
/=================================================================================================*/
 private void init(){
     myTycho2TableModel = new Tycho2TableModel();
 }
/*================================================================================================
/     test()
/=================================================================================================*/
 private void test(){
//     setCatalogFile("/rdata/ucac/tycho2/spectype_catalog/catalog.dat");
     setCatalogFile("/rdata/ucac/tycho2/spectype_catalog/catalog2.dat");
     setIndexFile("/rdata/ucac/tycho2/spectype_catalog/spectype_index.csv");
//     setIndexFile("/rdata/ucac/tycho2/spectype_catalog/spec_index.csv");
     readIndex();
//     double RA     = 4.0;
//     double DEC    = 1.0;
     double RA     = 202.48219583;
     double DEC    = 47.23150889;
     double BOX    = 600.0;
     query(RA,DEC,BOX);
     printResults();
//     writeCatalog();
     readCatalog();
//     constructIndex();
//     writeNewIndex();
 }
/*================================================================================================
/     setCatalogFile(java.lang.String new_catalog_file_name)
/=================================================================================================*/
public void setCatalogFile(java.lang.String new_catalog_file_name){
    catalog_file_name = new_catalog_file_name;
}
public java.lang.String getCatalogFile(){
    return catalog_file_name;
}
/*================================================================================================
/     setIndexFile(java.lang.String new_catalog_file_name)
/=================================================================================================*/
public void setIndexFile(java.lang.String new_index_file_name){
    index_file_name = new_index_file_name;
}
public java.lang.String getIndexFile(){
    return index_file_name;
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
/*================================================================================================
/          DECIMAL REPRESENTATIONS OF THE TELESCOPE ORIENTATION STRINGS
/=================================================================================================*/
/*================================================================================================
/      setRA(double newRA)
/=================================================================================================*/
  public void setRA(double newRA) {
    double  oldRA = this.RA;
    this.RA = newRA;
   propertyChangeListeners.firePropertyChange("RA", Double.valueOf(oldRA), Double.valueOf(newRA));
  }
  public double getRA() {
    return RA;
  }
/*================================================================================================
/      setDEC(double newDEC)
/=================================================================================================*/
  public void setDEC(double newDEC) {
    double  oldDEC = this.DEC;
    this.DEC = newDEC;
    propertyChangeListeners.firePropertyChange("DEC", Double.valueOf(oldDEC), Double.valueOf(newDEC));
  }
  public double getDEC() {
    return DEC;
  }
/*================================================================================================
/      setRA(double newRA)
/=================================================================================================*/
  public void setSelectedRA(double newSelectedRA) {
    double  oldSelectedRA = this.selectedRA;
    this.selectedRA = newSelectedRA;
   propertyChangeListeners.firePropertyChange("selectedRA", Double.valueOf(oldSelectedRA), Double.valueOf(newSelectedRA));
  }
  public double getSelectedRA() {
    return selectedRA;
  }
/*================================================================================================
/      setDEC(double newDEC)
/=================================================================================================*/
  public void setSelectedDEC(double newSelectedDEC) {
    double  oldSelectedDEC = this.selectedDEC;
    this.selectedDEC = newSelectedDEC;
    propertyChangeListeners.firePropertyChange("selectedDEC", Double.valueOf(oldSelectedDEC), Double.valueOf(newSelectedDEC));
  }
  public double getSelectedDEC() {
    return selectedDEC;
  }
 /*================================================================================================
/      add and remove Property Change Listeners
/=================================================================================================*/
 public void setSelectedObjectName(java.lang.String newSelectedName){
    java.lang.String  oldSelectedName = this.selectedName;
    this.selectedName = newSelectedName;
    propertyChangeListeners.firePropertyChange("selectedName", oldSelectedName, newSelectedName);
  }
  public java.lang.String getSelectedObjectName(){
      return selectedName;
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
/*================================================================================================
/     constructIndex()
/=================================================================================================*/
 public void constructIndex(){
     java.lang.String min_ra_string  = new java.lang.String();
     java.lang.String min_dec_string = new java.lang.String();
     java.lang.String max_ra_string  = new java.lang.String();
     java.lang.String max_dec_string = new java.lang.String();
     java.lang.String record         = new java.lang.String();
    int size = catalog_vector.size();
     int              last_index  = 1;
     java.lang.String last_TYC1   = "0";
     java.lang.String current_TYC1 = new java.lang.String();
     int current_index = 0;
     int count = 0;
     for(int i=0;i<size;i++){
        tycho2star current_tycho2star = (tycho2star)catalog_vector.get(i);
        count = count+1;
        current_index = Integer.parseInt(current_tycho2star.TYC1);
        if(current_index != last_index){
           
           int test = Integer.parseInt(last_TYC1);
           last_TYC1 = Integer.toString(test);
           tycho2index currentTycho2index = ((tycho2index)index_vector.get(last_TYC1));
           currentTycho2index.last_index = i;
           currentTycho2index.length      = count;
           currentTycho2index.TYC1        = Integer.toString(test);
              min_ra_string  = Printf.format("%6.2f", new PrintfData().add(currentTycho2index.ra_min));
              max_ra_string  = Printf.format("%6.2f", new PrintfData().add(currentTycho2index.ra_max));
              min_dec_string = Printf.format("%6.2f", new PrintfData().add(currentTycho2index.dec_min));
              max_dec_string = Printf.format("%6.2f", new PrintfData().add(currentTycho2index.dec_max));
              record = currentTycho2index.TYC1 + ","+currentTycho2index.last_index + ","+currentTycho2index.length+
                       ","+ min_ra_string +","+ max_ra_string+","+ min_dec_string+","+ max_dec_string + TERMINATOR;
              System.out.println(record);
           count = 0;
        }
        last_index = current_index;
        last_TYC1  = current_tycho2star.TYC1;
     }
 }
/*================================================================================================
/     constructIndex()
/=================================================================================================*/
 public void writeNewIndex(){
     java.lang.String min_ra_string  = new java.lang.String();
     java.lang.String min_dec_string = new java.lang.String();
     java.lang.String max_ra_string  = new java.lang.String();
     java.lang.String max_dec_string = new java.lang.String();
     java.lang.String record         = new java.lang.String();
        try{
          FileOutputStream fos = new FileOutputStream("/rdata/ucac/tycho2/newindex6.dat");
          bw = new BufferedWriter(new OutputStreamWriter(fos));
          boolean status = true;
          java.lang.String current_key = new java.lang.String();
          int     index  = 0;
          Enumeration en = index_vector.elements();
          while(en.hasMoreElements()){
              tycho2index currentTycho2index = ((tycho2index)en.nextElement());
              min_ra_string  = Printf.format("%6.2f", new PrintfData().add(currentTycho2index.ra_min));
              max_ra_string  = Printf.format("%6.2f", new PrintfData().add(currentTycho2index.ra_max));
              min_dec_string = Printf.format("%6.2f", new PrintfData().add(currentTycho2index.dec_min));
              max_dec_string = Printf.format("%6.2f", new PrintfData().add(currentTycho2index.dec_max));
              record = currentTycho2index.TYC1 + ","+currentTycho2index.last_index + ","+currentTycho2index.length+
                       ","+ min_ra_string +","+ max_ra_string+","+ min_dec_string+","+ max_dec_string + TERMINATOR;
              index_arraylist.add(record);
          }
          java.util.Collections.sort(index_arraylist);
          java.util.Iterator myIter = index_arraylist.iterator();
          while(myIter.hasNext()){
            record = (java.lang.String)myIter.next();
            bw.write(record);
          }
          bw.flush();
          bw.close();
       }catch(Exception e){

       }
 }
/*================================================================================================
/     readCatalog()
/=================================================================================================*/
   public void readCatalog(){
       java.lang.String currentLine = new java.lang.String();
       try{
          FileInputStream fis = new FileInputStream(catalog_file_name);
          br = new BufferedReader(new InputStreamReader(fis));
          boolean status = true;
          int     index  = 0;
          while(status){
             currentLine = br.readLine();
             System.out.println("Length = " +currentLine.length());
             if(currentLine != null){
                tycho2star current_tycho2star = new tycho2star(currentLine);
                catalog_vector.add(current_tycho2star);
                System.out.println("Reading Tycho-2 Spectral Type Catalog Line Number = "+index+ "  TYC"+current_tycho2star.TYC1+"-"+current_tycho2star.TYC2+"-"+current_tycho2star.TYC3);
                index = index + 1;
//                if(index == 100){
//                    status = false;
//                }
             }
             if(currentLine == null){
                 status = false;
             }
          }               
       }catch(Exception e){
           
       }
   }
/*================================================================================================
/     readCatalog()
/=================================================================================================*/
   public void writeCatalog(){
       java.lang.String currentLine = new java.lang.String();
       try{
          FileInputStream fis = new FileInputStream(catalog_file_name);
          br = new BufferedReader(new InputStreamReader(fis));
          bw = new BufferedWriter(new java.io.FileWriter("/rdata/ucac/tycho2/catalog2.dat"));
          boolean status = true;
          int     index  = 0;
          while(status){
             currentLine = br.readLine();
             currentLine = padLine(currentLine);
             bw.write(currentLine);
             System.out.println("Processed Line = "+index);
             index = index+1;
          }
       }catch(Exception e){

       }
   }
/*================================================================================================
/     readCatalog()
/=================================================================================================*/
 public java.lang.String padLine(java.lang.String line){
     line = line.replace(TERMINATOR, "");
     int length = line.length();
     int padLength = 123-length;
      if(padLength != 0){
         for(int i=0;i<padLength;i++){
            line = line + " ";
         }
     }
    line = line + TERMINATOR;
    return line;
 }
/*================================================================================================
/     executeReadIndex()
/=================================================================================================*/
 public void executeReadIndex(){
      InitializeIndexThread myInitializeIndexThread = new InitializeIndexThread();
      myInitializeIndexThread.start();
  }
/*================================================================================================
/     readIndex()
/=================================================================================================*/
  public void readIndex(){
        java.lang.String currentLine = new java.lang.String();
       try{
          FileInputStream fis = new FileInputStream(index_file_name);
          br = new BufferedReader(new InputStreamReader(fis));
          boolean status = true;
          int     index  = 1;
          while(status){
             currentLine = br.readLine();

             if(currentLine != null){
//                tycho2index current_tycho2index = new tycho2index(index,currentLine);
                tycho2index current_tycho2index = new tycho2index(currentLine);
                index_vector.put(current_tycho2index.TYC1,current_tycho2index);
                System.out.println("Reading Tycho-2 Index File Line Number = "+index+ "  TYC = "+current_tycho2index.TYC1+
                                   " RA min = "+current_tycho2index.ra_min+" RA max ="+current_tycho2index.ra_max
                                   + " Dec min = "+current_tycho2index.dec_min + " max = "+current_tycho2index.dec_max);
                index = index + 1;
             }
             if(currentLine == null){
                 status = false;
             }
          }
       }catch(Exception e){

       }
  }
/*================================================================================================
/     readCatalog()
/=================================================================================================*/
   public void readZone(int zone,SearchBox searchBox){
       java.lang.String currentLine = new java.lang.String();
       java.lang.String zone_index = Integer.toString(zone);

       tycho2index currentIndex = (tycho2index)index_vector.get(zone_index);
       int starting_position = currentIndex.last_index+1 - currentIndex.length;
       try{
          java.io.RandomAccessFile      raf   = new java.io.RandomAccessFile(catalog_file_name,"r");
          java.nio.channels.FileChannel fchan = raf.getChannel();
          java.nio.ByteBuffer           buf   = java.nio.ByteBuffer.allocate(RECORD_LENGTH);
          int bytesRead = 0;
//          fchan.position(starting_position);
          for(int i=0;i<currentIndex.length-1;i++){
             fchan.position(starting_position*RECORD_LENGTH+i*RECORD_LENGTH);
             bytesRead = fchan.read(buf);
             currentLine = new java.lang.String(buf.array());
             tycho2star current_tycho2star = new tycho2star(currentLine);
             boolean inside = isStarInBox2(current_tycho2star,searchBox);
             if(inside){
                results_vector.add(current_tycho2star);
             }
             buf.clear();
//             System.out.print(currentLine);
          }
 }catch(Exception e){
    System.out.println("An error occurred extracting information from zone: "+zone);
 }
 }
/*=============================================================================================
/     query(double testRA, double testDec, double arcminbox)
/=============================================================================================*/
 public java.util.Vector query(double testRA, double testDec, double arcminbox){
     int zone = 0;
     testRA = testRA*15.0;
     SearchBox sb = new SearchBox(testRA, testDec,arcminbox);
     myTycho2TableModel.clearTable();
     results_vector.clear();
     constructZoneList(testRA,testDec,arcminbox);
     Enumeration en = search_vector.elements();
     while(en.hasMoreElements()){
         tycho2index current_tycho2index = (tycho2index)en.nextElement();
         zone = Integer.parseInt(current_tycho2index.TYC1);
         readZone(zone,sb);
     }
     updateTableModel();
     return results_vector;
 }
/*=============================================================================================
/     getTycho2TableModel()
/=============================================================================================*/
 public Tycho2TableModel getTycho2TableModel(){
     return myTycho2TableModel;
 }
/*=============================================================================================
/     updateTableModel()
/=============================================================================================*/
 public void updateTableModel(){
   Enumeration en =  results_vector.elements();
   while(en.hasMoreElements()){
       tycho2star current_tycho2star = (tycho2star)en.nextElement();
       if(current_tycho2star != null){
          myTycho2TableModel.addRecord(current_tycho2star);
       }
   }
   myTycho2TableModel.fireTableDataChanged();
 }
/*=============================================================================================
/     query(double testRA, double testDec, double arcminbox)
/=============================================================================================*/
  public void printResults(){
     Enumeration en = results_vector.elements();
     while(en.hasMoreElements()){
         tycho2star current_tycho2star = (tycho2star)en.nextElement();
         System.out.println("  TYC"+current_tycho2star.TYC1+"-"+current_tycho2star.TYC2+"-"+current_tycho2star.TYC3+
                            " RA= "+ current_tycho2star.ra+" DEC= "+current_tycho2star.dec+" Spectral Type = "+current_tycho2star.spectral_type);
     }
  }
/*=============================================================================================
/     constructZoneList(double testRA, double testDec, double arcminbox)
/=============================================================================================*/
public void constructZoneList(double testRA, double testDec, double arcminbox){
    java.lang.String zone = new java.lang.String();
    boolean inside = false;
    search_vector.clear();
    for(int i=1;i<=ZONE_LENGTH;i++){
       zone = Integer.toString(i); 
       tycho2index current_tycho2index = (tycho2index)index_vector.get(zone);
       inside = isBoxInSegment(testRA,testDec,arcminbox,current_tycho2index);
       if(inside){
          search_vector.add(current_tycho2index); 
       }
    }   
} 
/*=============================================================================================
/     degreesToRadians(double degrees)
/=============================================================================================*/
 public double degreesToRadians(double degrees){
   double radians = degrees*(Math.PI/180.00);
   return radians;
 }
/*================================================================================================
/     printSegmentLimits(ucac3index segment)
/=================================================================================================*/
public void printSegmentLimits(tycho2index segment){
    System.out.println(segment.dec_min+","+segment.ra_min);
    System.out.println(segment.dec_min+","+segment.ra_max);
    System.out.println(segment.dec_max+","+segment.ra_max);
    System.out.println(segment.dec_max+","+segment.ra_min);
}
/*================================================================================================
/     isPointInSegment(ucac3index index,double RA,double DEC)
/=================================================================================================*/
public boolean isPointInSegment(tycho2index index,double RA,double DEC){
   boolean isInside = false;
   try{
   if(RA >= index.ra_min  & RA <= index.ra_max){
      if(DEC >= index.dec_min  & DEC <= index.dec_max){
        isInside = true;
      }     
   }
   } catch (Exception e){
       System.out.println("Error checking is point is in the segment");
   }
   return isInside;
}
/*================================================================================================
/     isStarInBox(tycho2star star,SearchBox searchBox)
/=================================================================================================*/
public boolean isStarInBox(tycho2star star,SearchBox searchBox){
   boolean isInside = false;
   try{
   if(star.ra >= searchBox.ra_min  & star.ra <= searchBox.ra_max){
      if(star.dec >= searchBox.dec_min  & star.dec <= searchBox.dec_max){
        isInside = true;
      }
   }
   } catch (Exception e){
       System.out.println("Error checking if star is in search box");
   }
   return isInside;
}
/*================================================================================================
/     isStarInBox(tycho2star star,SearchBox searchBox)
/=================================================================================================*/
public boolean isStarInBox2(tycho2star star,SearchBox searchBox){
   boolean isInside = false;
   try{
   WorldCoords      currentCoordinates  = new WorldCoords(star.ra,star.dec,2000);
   WorldCoords      currentSearchCenter = new WorldCoords(searchBox.center.ra,searchBox.center.dec);
//   CoordinateRadius cradius             = new CoordinateRadius(currentSearchCenter,searchBox.arcmin_box_degree*ARCMIN_DEGREE);
   double           distance            = currentSearchCenter.dist(currentCoordinates);
   System.out.println("Distance = "+distance);
   if(distance <= searchBox.arcmin_box_degree*ARCMIN_DEGREE){
       isInside = true;
   }
//                   isInside             = cradius.contains(currentCoordinates);
 //  if(star.ra >= searchBox.ra_min  & star.ra <= searchBox.ra_max){
 //     if(star.dec >= searchBox.dec_min  & star.dec <= searchBox.dec_max){
 //       isInside = true;
 //     }
 //  }
   } catch (Exception e){
       System.out.println("Error checking if star is in search box");
   }
   return isInside;
}
/*================================================================================================
/     isBoxInSegment(double testRA, double testDec, double arcminbox,ucac3index segment)
/=================================================================================================*/
public boolean isBoxInSegment(double testRA, double testDec, double arcminbox,tycho2index segment){
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
//    printSegmentLimits(segment);
    if(testcenter | testUL | testLL| testUR | testLR | testU | testD | testL | testR){
        status = true;
    }
   return status;
}
/*================================================================================================
/     isBoxInSegment(double testRA, double testDec, double arcminbox,ucac3index segment)
/=================================================================================================*/
public boolean isBoxInSegment(SearchBox sb,tycho2index segment){
    boolean status = false;
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
//    printSegmentLimits(segment);
    if(testcenter | testUL | testLL| testUR | testLR | testU | testD | testL | testR){
        status = true;
    }
   return status;
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
       setInitialized(false);
       readIndex();
       setInitialized(true);
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
/*=============================================================================================
/         main method
/=============================================================================================*/
  //Main method
  public static void main(String[] args) {
    try {
    }
    catch(Exception e) {
      e.printStackTrace();
    }
    new Tycho2Reader();
  }
}
