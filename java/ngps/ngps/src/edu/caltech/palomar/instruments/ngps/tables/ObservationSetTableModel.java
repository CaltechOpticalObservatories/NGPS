/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package edu.caltech.palomar.instruments.ngps.tables;

//import edu.caltech.palomar.instruments.ngps.*;
import javax.swing.JTable;
import javax.swing.table.AbstractTableModel;
//import edu.caltech.palomar.telescopes.P200.gui.tables.AstroObject;
import edu.dartmouth.jskycalc.coord.RA;
import edu.dartmouth.jskycalc.coord.dec;
import javax.swing.JTable;
import javax.swing.table.AbstractTableModel;
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
import edu.caltech.palomar.instruments.ngps.object.Target;
import edu.caltech.palomar.instruments.ngps.object.ObservationSet;

public class ObservationSetTableModel extends AbstractTableModel{
//=== File Prolog =============================================================
//	This code was developed by John Thorstensen of Dartmouth University
//
//      It has been refactored and modified by Jennifer Milburn for
//      use at the Palomar Observatory.
//
//      Caltech Optical Observatories, Palomar Observatory
//
//--- Contents ----------------------------------------------------------------
//   ObservationSetTableModel  -
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
/*================================================================================================
/          Class Declaration for AstroObjectTableModel
/=================================================================================================*/
   java.util.Vector    ObservationSetVector = new java.util.Vector();
   java.lang.String[]  columnNameArray   = new java.lang.String[7];
   int columncount = 7;
   int rowcount;
   private  javax.swing.JTable myObservationsSetTable;  
/*================================================================================================
/          Class Constructor for ObservationTableModel
/=================================================================================================*/
public ObservationSetTableModel(){
    jbInit();   
}
/*================================================================================================
/        setJTable(javax.swing.JTable newAstroObjectsTable)
/=================================================================================================*/
public void setJTable(javax.swing.JTable newObjectsTable){
  myObservationsSetTable = newObjectsTable;
}
public JTable getJTable(){
    return myObservationsSetTable;
}  
/*===============================================================================================
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
/         clearTable()
/=================================================================================================*/
 public void clearTable(){
     ObservationSetVector.clear();
     rowcount = 0;
     fireTableDataChanged();
 }
/*================================================================================================
/          getVector()
/=================================================================================================*/
 public java.util.Vector getVector(){
     return ObservationSetVector;
 }
/*================================================================================================
/           jbInit() Initiaization Method
/=================================================================================================*/
    private void jbInit(){
       rowcount = 0;
       columnNameArray[0] = "SET_ID";
       columnNameArray[1] = "SET_NAME";
       columnNameArray[2] = "OWNER";
       columnNameArray[3] = "STATE";
       columnNameArray[4] = "NUM_OBSERVATIONS";
       columnNameArray[5] = "SET_CREATION_TIMESTAMP";
       columnNameArray[6] = "LAST_UPDATE_TIMESTAMP";
    }
/*================================================================================================
/     timestamp_to_string(java.sql.Timestamp current)
/=================================================================================================*/
public java.lang.String timestamp_to_string(java.sql.Timestamp current){
//    java.sql.Timestamp timestamp = java.sql.Timestamp.valueOf("2018-12-12 01:02:03.123456789");
    java.time.format.DateTimeFormatter formatter = java.time.format.DateTimeFormatter.ISO_LOCAL_DATE_TIME;
    String timestampAsString = formatter.format(current.toLocalDateTime());
//    assertEquals("2018-12-12T01:02:03.123456789", timestampAsString);
return timestampAsString;   
}
/*================================================================================================
/      main(String args[])
/=================================================================================================*/
public java.sql.Timestamp string_to_timestamp(java.lang.String current_datetime){
 java.sql.Timestamp timestamp = new java.sql.Timestamp(System.currentTimeMillis());
    try {
    java.text.SimpleDateFormat dateFormat = new java.text.SimpleDateFormat("yyyy-MM-dd'T'hh:mm:ss.SSS");
    java.util.Date parsedDate = dateFormat.parse(current_datetime);
    timestamp = new java.sql.Timestamp(parsedDate.getTime());
    return timestamp;
} catch(Exception e) { //this generic but you can control another types of exception
   System.out.println(e.toString());
}
return timestamp;
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
/       addRecord(AstroObject newRecord)
/=================================================================================================*/
 public void addRecord(ObservationSet newRecord){
    ObservationSetVector.add(newRecord);
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
  public synchronized ObservationSet getRecord(int recordNumber) {
     ObservationSet myObservationSet =((ObservationSet)(ObservationSetVector.get(recordNumber)));
    return myObservationSet;
  }
 /*================================================================================================
/        getRecordNumber(ObservationSet selectedObject)
/=================================================================================================*/
 public int getRecordNumber(ObservationSet selectedObject){
     int recordNumber = 0;
     java.util.ListIterator li = ObservationSetVector.listIterator();
     while(li.hasNext()){
        ObservationSet currentObject = (ObservationSet)(li.next());
        if(currentObject.equals(selectedObject)){
           recordNumber = li.previousIndex();
        }
     }
     return recordNumber;
 }
 /*================================================================================================
/         Required Methods for the Abstract Table Model Class
/    getColumnCount()
/    getRowCount()
/    getValueAt(int row, int col)
/    OPTIONAL METHOD  setValueAt(Object value, int row, int col)
/=================================================================================================*/
public boolean isCellEditable(int rowIndex, int vColIndex) {
    boolean editable = false;
    if(vColIndex == 0){
        editable = true;
    }
    if(vColIndex == 1){
        editable = true;
    }
    if(vColIndex == 2){
        editable = true;
    }
    if(vColIndex == 3){
        editable = true;
    }
    if(vColIndex == 4){
        editable = true;
    }
    if(vColIndex == 5){
        editable = true;
    }
    if(vColIndex == 6){
        editable = true;
    }
    return editable;
}
/*================================================================================================
/         getValueAt() - Required Method for the Abstract Table Model Class
/=================================================================================================*/
  public Object getValueAt(int row, int col) {
    java.lang.Object returnObject = null;
    ObservationSet selectedRow = (ObservationSet)ObservationSetVector.elementAt(row);
    if(col == 0){
       returnObject = selectedRow.getSET_ID();
    }
    if(col == 1){
       returnObject = selectedRow.getSET_NAME();
    }
    if(col == 2){
       returnObject = selectedRow.getOWNER();
    }
    if(col == 3){
       returnObject = selectedRow.getSTATE();
    }   
    if(col == 4){
       returnObject = selectedRow.getNUM_OBSERVATIONS();
    }
    if(col == 5){
       returnObject = timestamp_to_string(selectedRow.getCreation_Timestamp());
    }
    if(col == 6){
       returnObject = timestamp_to_string(selectedRow.getLastUpdate_Timestamp());
    }
    return returnObject;
  }
/*================================================================================================
/     setValueAt(Object value, int row, int col)
/=================================================================================================*/
 public void setValueAt(Object value, int row, int col) {
   ObservationSet selectedRow;
   if(ObservationSetVector.size() >= row){
       selectedRow = (ObservationSet)ObservationSetVector.elementAt(row);
    if(col == 0){
       selectedRow.setSET_ID(((java.lang.Integer)value));
    }
    if(col == 1){
       selectedRow.setSET_NAME(((java.lang.String)value));
    }
    if(col == 2){
       selectedRow.setOWNER((java.lang.String)value);
    }
    if(col == 3){
       selectedRow.setSTATE((java.lang.String)value);
    }
    if(col == 4){
       selectedRow.setNUM_OBSERVATIONS(((java.lang.Integer)value));
    }    
    if(col == 5){
       selectedRow.setCreation_Timestamp(string_to_timestamp(((java.lang.String)value)));
    }
    if(col == 6){
       selectedRow.setLastUpdate_Timestamp(string_to_timestamp(((java.lang.String)value)));
    }
    ObservationSetVector.setElementAt(selectedRow,row);
   }
   fireTableCellUpdated(row, col);
   fireTableDataChanged();
 }
/*================================================================================================
/            Optional Utility Methods for the Abstract Table Model Class
/  public String getColumnName(int col) {
/  public Class  getColumnClass(int c) {
/  public void   setRecordAt(int row,FilterRecord newFilterRecord)
/=================================================================================================*/
/*================================================================================================
/        getColumnClass(int c)
/=================================================================================================*/
  public Class getColumnClass(int c) {
    java.lang.Class myClass = null;
     if(c == 0){
         myClass =  (Integer.valueOf(0)).getClass();
//         myClass = ((AstroObject)AstroObjectVector.firstElement()).name.getClass();
        return myClass;
     }
    if(c == 1){
       myClass =  (new java.lang.String()).getClass();
       //        myClass = ((AstroObject)AstroObjectVector.firstElement()).Alpha.RoundedRAString(2, ":").getClass();
        return myClass;
     }
     if(c == 2){
      myClass =  (new java.lang.String()).getClass();
 //        myClass = ((AstroObject)AstroObjectVector.firstElement()).Delta.RoundedDecString(2, ":").getClass();
        return myClass;
     }
     if(c == 3){
         myClass =  (new java.lang.String()).getClass();
//        myClass =  (new Double(((AstroObject)AstroObjectVector.firstElement()).Alpha.value)).getClass();
        return myClass;
     }
     if(c == 4){
         myClass =  (Integer.valueOf(0)).getClass();
//        myClass =  (new Double(((AstroObject)AstroObjectVector.firstElement()).Alpha.value)).getClass();
        return myClass;
     }
     if(c == 5){
         myClass =  (Integer.valueOf(0)).getClass();
         //        myClass =  (new Double(((AstroObject)AstroObjectVector.firstElement()).Delta.value)).getClass();
        return myClass;
     }
     if(c == 6){
         myClass =  (new java.lang.String()).getClass();
//         myClass =  (new Double(((AstroObject)AstroObjectVector.firstElement()).r_m)).getClass();
        return myClass;
     }
     if(c == 7){
         myClass =  (new java.lang.String()).getClass();
        return myClass;
     }
    return myClass;
  }
/*=========================================================================================================
/       End of the AstroObjectTableModel Class
/=========================================================================================================*/
}

