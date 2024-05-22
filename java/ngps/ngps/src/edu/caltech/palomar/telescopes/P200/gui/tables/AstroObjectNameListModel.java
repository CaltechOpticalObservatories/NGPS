package edu.caltech.palomar.telescopes.P200.gui.tables; 
//=== File Prolog =============================================================
//	This code was developed by John Thorstensen of Dartmouth University
//
//      It has been refactored and modified by Jennifer Milburn for
//      use at the Palomar Observatory.
//
//      Caltech Optical Observatories, Palomar Observatory
//
//--- Contents ----------------------------------------------------------------
//    AstroObjectNameListModel  -
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
/*================================================================================================
/          Class Declaration for AstroObjectTableModel
/=================================================================================================*/
public class AstroObjectNameListModel extends AbstractTableModel{
//   java.util.Hashtable   AstroObjectTable    = new java.util.Hashtable<java.lang.String, AstroObject>();
   java.util.Vector    AstroObjectVector = new java.util.Vector();
   java.lang.String[]  columnNameArray   = new java.lang.String[10];
   int columncount = 1;
   int rowcount;
/*================================================================================================
/          public parameterless constructor for InstrumentDataTableModel
/=================================================================================================*/
  public AstroObjectNameListModel() {
       jbInit();
  }
/*================================================================================================
/          getVector()
/=================================================================================================*/
 public java.util.Vector getVector(){
     return AstroObjectVector;
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
/           jbInit() Initiaization Method
/=================================================================================================*/
    private void jbInit(){
       rowcount = 0;
       columnNameArray[0] = "Object Name";
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
/       addRecord(String name,double r, double d, double e,double r_m,double d_m,boolean m_flag)
/=================================================================================================*/
 public void addRecord(String name){
    String newRecord = new String();
    newRecord = name;
    AstroObjectVector.add(newRecord);
    rowcount = rowcount + 1;
    fireTableDataChanged();
 }
/*================================================================================================
/       addRecord(AstroObject newRecord)
/=================================================================================================*/
 public void addRecord(AstroObject newRecord){
    AstroObjectVector.add(newRecord.name);
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
  public synchronized String getRecord(int recordNumber) {
     String myRecord =((String)(AstroObjectVector.get(recordNumber)));
    return myRecord;
  }
/*================================================================================================
/         getValueAt() - Required Method for the Abstract Table Model Class
/=================================================================================================*/
  public Object getValueAt(int row, int col) {
    java.lang.Object returnObject = null;
    String selectedRow = (String)AstroObjectVector.elementAt(row);
    if(col == 0){
       returnObject = selectedRow;
    }
    return returnObject;
  }
/*================================================================================================
/     setValueAt(Object value, int row, int col)
/=================================================================================================*/
 public void setValueAt(Object value, int row, int col) {
   String selectedRow;
   if(AstroObjectVector.size() >= row){
       selectedRow = (String)AstroObjectVector.elementAt(row);
    if(col == 0){
       selectedRow = ((java.lang.String)value);
    }
     AstroObjectVector.setElementAt(selectedRow,row);
   }
   fireTableCellUpdated(row, col);
 }
/*================================================================================================
/            Optional Utility Methods for the Abstract Table Model Class
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
        myClass = ((String)AstroObjectVector.firstElement()).getClass();
        return myClass;
     }
    return myClass;
  }
/*=========================================================================================================
/       End of the AstroObjectTableModel Class
/=========================================================================================================*/
}
