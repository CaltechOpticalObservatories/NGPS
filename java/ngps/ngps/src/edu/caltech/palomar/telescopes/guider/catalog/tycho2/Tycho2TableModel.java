/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package edu.caltech.palomar.telescopes.guider.catalog.tycho2;
//=== File Prolog =============================================================
//	This code was developed by John Thorstensen of Dartmouth University
//
//      It has been refactored and modified by Jennifer Milburn for
//      use at the Palomar Observatory.
//
//      Caltech Optical Observatories, Palomar Observatory
//
//--- Contents ----------------------------------------------------------------
//    Tycho2TableModel
//    Jennifer Milburn June 23,2011
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
/*================================================================================================
/          Class Declaration for Tycho2TableModel
/=================================================================================================*/
public class Tycho2TableModel extends AbstractTableModel{
//   java.util.Hashtable   AstroObjectTable    = new java.util.Hashtable<java.lang.String, AstroObject>();
   java.util.Vector    AstroObjectVector = new java.util.Vector();
   java.lang.String[]  columnNameArray   = new java.lang.String[14];
   int columncount = 14;
   int rowcount;
   private  javax.swing.JTable myAstroObjectTable;
/*================================================================================================
/          public parameterless constructor for Tycho2TableModel
/=================================================================================================*/
  public Tycho2TableModel() { 
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
       columnNameArray[0] = "Object Name";
       columnNameArray[1] = "Spectral Type";
       columnNameArray[2] = "Magnitude";
       columnNameArray[3] = "RA";
       columnNameArray[4] = "Dec";
       columnNameArray[5] = "RA decimal degrees";
       columnNameArray[6] = "Dec decimal degrees";
       columnNameArray[7] = "Temperature Class";
       columnNameArray[8] = "Temperature Subclass";
       columnNameArray[9] = "Luminosity";
       columnNameArray[10] = "Temperature";
       columnNameArray[11] = "V magnitude";
       columnNameArray[12] = "B magnitude";
       columnNameArray[13] = "Alternate Name";
/*    columnVector.add(TYC);
    columnVector.add(TYC1);
    columnVector.add(TYC2);
    columnVector.add(TYC3);
    columnVector.add(ra);
    columnVector.add(dec);
    columnVector.add(Vmag);
    columnVector.add(Bmag);
    columnVector.add(spec_type_source);
    columnVector.add(name);
    columnVector.add(distance);
    columnVector.add(magnitude);
    columnVector.add(magnitude_type);
    columnVector.add(temperature_class);
    columnVector.add(temperature_subclass);
    columnVector.add(luminosity);
    columnVector.add(temperature);
    columnVector.add(spectral_type);
*/
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
 public void addRecord(tycho2star newRecord){
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
  public synchronized tycho2star getRecord(int recordNumber) {
     tycho2star myAstroObject =((tycho2star)(AstroObjectVector.get(recordNumber)));
    return myAstroObject;
  }
 /*================================================================================================
/        getRecordNumber(AstroObject selectedObject)
/=================================================================================================*/
 public int getRecordNumber(tycho2star selectedObject){
     int recordNumber = 0;
     java.util.ListIterator li = AstroObjectVector.listIterator();
     while(li.hasNext()){
        tycho2star currentObject = (tycho2star)(li.next());
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
    tycho2star selectedRow = (tycho2star)AstroObjectVector.elementAt(row);
    if(col == 0){
       returnObject = selectedRow.tycho_name;
    }
    if(col == 1){
       returnObject = selectedRow.spectral_type;
    }
    if(col == 2){
       returnObject = Double.valueOf(selectedRow.magnitude);
    }
    if(col == 3){
       returnObject = selectedRow.ra_string;
    }
    if(col == 4){
       returnObject = selectedRow.dec_string;
    }
    if(col == 5){
       returnObject = Double.valueOf(selectedRow.ra);
    }
    if(col == 6){
       returnObject = Double.valueOf(selectedRow.dec);
    }
    if(col == 7){
       returnObject = selectedRow.temperature_class;
    }
    if(col == 8){
       returnObject = selectedRow.temperature_subclass;
    }
    if(col == 9){
       returnObject = Integer.valueOf(selectedRow.luminosity);
    }
    if(col == 10){
       returnObject = Double.valueOf(selectedRow.temperature);
    }
    if(col == 11){
       returnObject = Double.valueOf(selectedRow.Vmag);
    }
    if(col == 12){
       returnObject = Double.valueOf(selectedRow.Bmag);
    }
    if(col == 13){
       returnObject = selectedRow.name;
    }
     return returnObject;
  }
/*================================================================================================
/     setValueAt(Object value, int row, int col)
/=================================================================================================*/
 public void setValueAt(Object value, int row, int col) {
   tycho2star selectedRow;
   if(AstroObjectVector.size() >= row){
       selectedRow = (tycho2star)AstroObjectVector.elementAt(row);
    if(col == 0){
       selectedRow.tycho_name = ((java.lang.String)value);
    }
    if(col == 1){
       selectedRow.spectral_type = ((java.lang.String)value);
    }
    if(col == 2){
       selectedRow.magnitude = ((java.lang.Double)value);
    }
    if(col == 3){
       selectedRow.ra_string = ((java.lang.String)value);
    }
    if(col == 4){
       selectedRow.dec_string = ((java.lang.String)value);
    }
    if(col == 5){
       selectedRow.ra = ((java.lang.Double)value).doubleValue();
    }
    if(col == 6){
       selectedRow.dec = ((java.lang.Double)value).doubleValue();
    }
    if(col == 7){
       selectedRow.temperature_class = ((java.lang.String)value);
    }
    if(col == 8){
       selectedRow.temperature_subclass = ((java.lang.String)value);
    }
    if(col == 9){
       selectedRow.luminosity = ((java.lang.Integer)value).intValue();
    }
    if(col == 10){
       selectedRow.temperature = ((java.lang.Double)value).doubleValue();
    }
    if(col == 11){
       selectedRow.Vmag = ((java.lang.Double)value).doubleValue();
    }
    if(col == 12){
       selectedRow.Bmag = ((java.lang.Double)value).doubleValue();
    }
    if(col == 13){
       selectedRow.name = ((java.lang.String)value);
    }
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
        myClass = java.lang.String.class;
        return myClass;
     }
    if(c == 1){
        myClass = java.lang.String.class;
        return myClass;
     }
     if(c == 2){
        myClass = java.lang.Double.class;
        return myClass;
     }
    if(c == 3){
        myClass = java.lang.String.class;
        return myClass;
     }
     if(c == 4){
        myClass =  java.lang.String.class;
        return myClass;
     }
     if(c == 5){
        myClass =  java.lang.Double.class;
        return myClass;
     }
     if(c == 6){
        myClass =  java.lang.Double.class;
        return myClass;
     }
     if(c == 7){
        myClass =  java.lang.String.class;
        return myClass;
     }
     if(c == 8){
        myClass =  java.lang.String.class;
        return myClass;
     }
      if(c == 9){
        myClass =  java.lang.Integer.class;
        return myClass;
     }
     if(c == 10){
        myClass = java.lang.Double.class;
        return myClass;
     }
     if(c == 11){
        myClass =  java.lang.Double.class;
        return myClass;
     }
     if(c == 12){
        myClass =  java.lang.Double.class;
        return myClass;
     }
     if(c == 13){
        myClass = java.lang.String.class;
        return myClass;
     }
    return myClass;
  }
/*=========================================================================================================
/       End of the Tycho2TableModel Class
/=========================================================================================================*/
}