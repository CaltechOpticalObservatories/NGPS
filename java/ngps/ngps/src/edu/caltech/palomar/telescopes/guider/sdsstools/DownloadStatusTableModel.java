package edu.caltech.palomar.telescopes.guider.sdsstools; 
//=== File Prolog =============================================================
//	This code was developed by John Thorstensen of Dartmouth University
//
//      It has been refactored and modified by Jennifer Milburn for
//      use at the Palomar Observatory.
//
//      Caltech Optical Observatories, Palomar Observatory
//
//--- Contents ----------------------------------------------------------------
//    DownloadStatusTableModel  - Same as the AstroObjectTableModel but with a boolean
//    that indicates the download state of the image for the guider display
//    January 25, 2011 Jennifer Milburn
//
//--- Description -------------------------------------------------------------
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
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
import javax.swing.table.AbstractTableModel;
// Imports from the java.util package
import java.util.Vector;
import java.util.Comparator;
import java.util.List;
import java.util.Date;
import java.util.Iterator;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Enumeration;
import java.util.GregorianCalendar;
import java.util.Calendar;
import java.util.SimpleTimeZone;
import java.util.TimeZone;
import javax.swing.text.StyleContext;
import javax.swing.text.Style;
import javax.swing.text.StyleConstants;
import edu.dartmouth.jskycalc.coord.RA;
import edu.dartmouth.jskycalc.coord.dec;
import edu.dartmouth.jskycalc.coord.sexagesimal;
import edu.dartmouth.jskycalc.coord.Celest;
import java.util.HashMap;
import javax.swing.JTable;
import edu.caltech.palomar.telescopes.P200.gui.tables.*;
/*================================================================================================
/          Class Declaration for DownloadStatusTableModel
/=================================================================================================*/
public class DownloadStatusTableModel extends AbstractTableModel{
//   java.util.Hashtable   AstroObjectTable    = new java.util.Hashtable<java.lang.String, AstroObject>();
   java.util.Vector    AstroObjectVector = new java.util.Vector();
   java.lang.String[]  columnNameArray   = new java.lang.String[11];
   int columncount = 11;
   int rowcount;
   public static int NOT_DOWNLOADED = 0;
   public static int DOWNLOADING    = 1;
   public static int DOWNLOADED     = 2;
   private  javax.swing.JTable myAstroObjectTable;
/*================================================================================================
/          public parameterless constructor for InstrumentDataTableModel
/=================================================================================================*/
  public DownloadStatusTableModel() {
       jbInit();
  }
/*================================================================================================
/        setJTable(javax.swing.JTable newAstroObjectsTable)
/=================================================================================================*/
public void setJTable(javax.swing.JTable newAstroObjectsTable){
  myAstroObjectTable = newAstroObjectsTable;
}
public JTable getJTable(){
    return myAstroObjectTable;
}
/*================================================================================================
/         clearTable()
/=================================================================================================*/
 public void clearTable(){
     AstroObjectVector.clear();
     rowcount = 0;
     fireTableDataChanged();
 }
/*================================================================================================
/          getVector()
/=================================================================================================*/
 public java.util.Vector getVector(){
     return AstroObjectVector;
 }
/*================================================================================================
/           jbInit() Initiaization Method
/=================================================================================================*/
    private void jbInit(){
       rowcount = 0;
       columnNameArray[0] = "Downloaded";
       columnNameArray[1] = "Object Name";
       columnNameArray[2] = "RA";
       columnNameArray[3] = "Dec";
       columnNameArray[4] = "RA decimal hours";
       columnNameArray[5] = "Dec decimal degrees";
       columnNameArray[6] = "RA motion";
       columnNameArray[7] = "Dec motion";
       columnNameArray[8] = "Equinox";
       columnNameArray[9] = "Motion Flag";
       columnNameArray[10] = "File Name";
//       columnNameArray[9] = "Comments";
    }
/*================================================================================================
/         Required Methods for the Abstract Table Model Class
/    getColumnCount()
/    getRowCount()
/    getValueAt(int row, int col)
/    OPTIONAL METHOD  setValueAt(Object value, int row, int col)
/=================================================================================================*/
/*================================================================================================
/         getColumnCount() - Required Method for the Abstract Table Model Class
/=================================================================================================*/
 public void clearDownloadStatus(){
     for(int i=0;i<rowcount;i++){
         ((AstroObject)(AstroObjectVector.get(i))).download_status = NOT_DOWNLOADED;
     }
     fireTableDataChanged();
 }
/*================================================================================================
/         getColumnCount() - Required Method for the Abstract Table Model Class
/=================================================================================================*/
  public int getColumnCount() {
      return columncount;
  }
/*================================================================================================
/         setColumnCount() -
/=================================================================================================*/
  public void setColumnCount(int newColumnCount){
      columncount = newColumnCount;
  }
 //  addRecord Method for constructing the AstroObject
/*================================================================================================
/       addRecord(String name,double r, double d, double e,double r_m,double d_m,boolean m_flag)
/=================================================================================================*/
 public void addRecord(String name,RA r, dec d, double e,double r_m,double d_m,boolean m_flag,String comment,String fileName){
    AstroObject newRecord = new AstroObject(name,r,d,e,r_m,d_m,m_flag,comment,fileName);
    AstroObjectVector.add(newRecord);
    rowcount = rowcount + 1;
    fireTableDataChanged();
 }
/*================================================================================================
/       addRecord(AstroObject newRecord)
/=================================================================================================*/
 public void addRecord(AstroObject newRecord){
    AstroObjectVector.add(newRecord);
    rowcount = rowcount + 1;
    fireTableDataChanged();
 }
/*================================================================================================
/         getRowCount() - Required Method for the Abstract Table Model Class
/=================================================================================================*/
  public int getRowCount() {
        return rowcount;
  }
/*================================================================================================
/         getRecord(int recordNumber)
/=================================================================================================*/
  public synchronized AstroObject getRecord(int recordNumber) {
     AstroObject myAstroObject =((AstroObject)(AstroObjectVector.get(recordNumber)));
    return myAstroObject;
  }
 /*================================================================================================
/        getRecordNumber(AstroObject selectedObject)
/=================================================================================================*/
 public int getRecordNumber(AstroObject selectedObject){
     int recordNumber = 0;
     java.util.ListIterator li = AstroObjectVector.listIterator();
     while(li.hasNext()){
        AstroObject currentObject = (AstroObject)(li.next());
        if(currentObject.equals(selectedObject)){
           recordNumber = li.previousIndex();
        }
     }
     return recordNumber;
 }
/*================================================================================================
/         getValueAt() - Required Method for the Abstract Table Model Class
/=================================================================================================*/
  public Object getValueAt(int row, int col) {
    java.lang.Object returnObject = null;
    AstroObject selectedRow = (AstroObject)AstroObjectVector.elementAt(row);
    if(col == 0){
       returnObject = Integer.valueOf(selectedRow.download_status);
    }
    if(col == 1){
       returnObject = selectedRow.name;
    }
    if(col == 2){
       returnObject = selectedRow.Alpha.RoundedRAString(2, ":");
    }
    if(col == 3){
       returnObject = selectedRow.Delta.RoundedDecString(2, ":");
    }
    if(col == 4){
       returnObject = Double.valueOf(selectedRow.Alpha.degrees());
    }
    if(col == 5){
       returnObject = Double.valueOf(selectedRow.Delta.degrees());
    }
    if(col == 6){
       returnObject = Double.valueOf(selectedRow.r_m);
    }
    if(col == 7){
       returnObject = Double.valueOf(selectedRow.d_m);
    }
    if(col == 8){
       returnObject = Double.valueOf(selectedRow.Equinox);
    }
    if(col == 9){
       returnObject = Boolean.valueOf(selectedRow.m_flag);
    }
    if(col == 10){
       returnObject = selectedRow.fileName;
    }

//    if(col == 9){
//       returnObject = selectedRow.comment;
//    }
    return returnObject;
  }
/*================================================================================================
/     setValueAt(Object value, int row, int col)
/=================================================================================================*/
 public void setValueAt(Object value, int row, int col) {
   AstroObject selectedRow;
   if(AstroObjectVector.size() >= row){
       selectedRow = (AstroObject)AstroObjectVector.elementAt(row);
    if(col == 0){
       selectedRow.download_status = ((java.lang.Integer)value).intValue();
    }
    if(col == 1){
       selectedRow.name = ((java.lang.String)value);
    }
    if(col == 2){
       selectedRow.Alpha.setRA(((java.lang.String)value));
    }
    if(col == 3){
       selectedRow.Delta.setDec((java.lang.String)value);
    }
    if(col == 4){
       selectedRow.Alpha.value = ((java.lang.Double)value).doubleValue();
    }
    if(col == 5){
       selectedRow.Delta.value = ((java.lang.Double)value).doubleValue();
    }
    if(col == 6){
       selectedRow.r_m = ((java.lang.Double)value).doubleValue();
    }
    if(col == 7){
       selectedRow.d_m = ((java.lang.Double)value).doubleValue();
    }
    if(col == 8){
       selectedRow.Equinox = ((java.lang.Double)value).doubleValue();
    }
    if(col == 9){
       selectedRow.m_flag = ((java.lang.Boolean)value).booleanValue();
    }
    if(col == 10){
       selectedRow.fileName = ((java.lang.String)value);
    }

//    if(col == 9){
//       selectedRow.comment = ((java.lang.String)value);
//    }
    AstroObjectVector.setElementAt(selectedRow,row);
   }
   fireTableCellUpdated(row, col);
 }
/*================================================================================================
/            Optional Utility Methods for the Abstract Table Model Class
/  public String getColumnName(int col) {
/  public Class  getColumnClass(int c) {
/  public void   setRecordAt(int row,FilterRecord newFilterRecord)
/=================================================================================================*/
/*================================================================================================
/     setColumnName(int col)
/=================================================================================================*/
 public void setColumnName(int col,java.lang.String newColumnName){
     columnNameArray[col] = newColumnName;
 }
/*================================================================================================
/     getColumnName(int col)
/=================================================================================================*/
  public String getColumnName(int col) {
    return columnNameArray[col];
  }
/*================================================================================================
/        getColumnClass(int c)
/=================================================================================================*/
  public Class getColumnClass(int c) {
    java.lang.Class myClass = null;
     if(c == 0){
        myClass = (Integer.valueOf(((AstroObject)AstroObjectVector.firstElement()).download_status)).getClass();
        return myClass;
     }
    if(c == 1){
        myClass = ((AstroObject)AstroObjectVector.firstElement()).name.getClass();
        return myClass;
     }
    if(c == 2){
        myClass = ((AstroObject)AstroObjectVector.firstElement()).Alpha.RoundedRAString(2, ":").getClass();
        return myClass;
     }
     if(c == 3){
        myClass = ((AstroObject)AstroObjectVector.firstElement()).Delta.RoundedDecString(2, ":").getClass();
        return myClass;
     }
     if(c == 4){
        myClass =  (Double.valueOf(((AstroObject)AstroObjectVector.firstElement()).Alpha.value)).getClass();
        return myClass;
     }
     if(c == 5){
        myClass =  (Double.valueOf(((AstroObject)AstroObjectVector.firstElement()).Delta.value)).getClass();
        return myClass;
     }
     if(c == 6){
        myClass =  (Double.valueOf(((AstroObject)AstroObjectVector.firstElement()).r_m)).getClass();
        return myClass;
     }
     if(c == 7){
        myClass =  (Double.valueOf(((AstroObject)AstroObjectVector.firstElement()).d_m)).getClass();
        return myClass;
     }
     if(c == 8){
        myClass =  (Double.valueOf(((AstroObject)AstroObjectVector.firstElement()).Equinox)).getClass();
        return myClass;
     }
     if(c == 9){
        myClass =  (Boolean.valueOf(((AstroObject)AstroObjectVector.firstElement()).m_flag)).getClass();
        return myClass;
     }
     if(c == 10){
        myClass = ((AstroObject)AstroObjectVector.firstElement()).fileName.getClass();
        return myClass;
     }

//     if(c == 8){
//        myClass =  ((AstroObject)AstroObjectVector.firstElement()).comment.getClass();
//        return myClass;
//     }
    return myClass;
  }
/*=========================================================================================================
/       End of the DownloadStatusTableModel Class
/=========================================================================================================*/
}