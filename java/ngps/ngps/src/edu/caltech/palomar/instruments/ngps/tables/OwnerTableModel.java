/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package edu.caltech.palomar.instruments.ngps.tables;
//import edu.caltech.palomar.instruments.ngps.*;
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
import edu.caltech.palomar.instruments.ngps.object.Owner;
/**
 *
 * @author jennifermilburn
 */
public class OwnerTableModel extends AbstractTableModel{
/*================================================================================================
/          Class Declaration for AstroObjectTableModel
/=================================================================================================*/
   java.util.Vector    OwnerVector = new java.util.Vector();
   java.lang.String[]  columnNameArray   = new java.lang.String[3];
   int columncount = 3;
   int rowcount;
   private  javax.swing.JTable myOwnerTable;  
/*================================================================================================
/          Class Constructor for ObservationTableModel
/=================================================================================================*/
public OwnerTableModel(){
    jbInit();   
}
/*================================================================================================
/        setJTable(javax.swing.JTable newAstroObjectsTable)
/=================================================================================================*/
public void setJTable(javax.swing.JTable newAstroObjectsTable){
  myOwnerTable = newAstroObjectsTable;
}
public JTable getJTable(){
    return myOwnerTable;
}    
/*================================================================================================
/         clearTable()
/=================================================================================================*/
 public void clearTable(){
     OwnerVector.clear();
     rowcount = 0;
     fireTableDataChanged();
 }
/*================================================================================================
/          getVector()
/=================================================================================================*/
 public java.util.Vector getVector(){
     return OwnerVector;
 }
/*================================================================================================
/           jbInit() Initiaization Method
/=================================================================================================*/
    private void jbInit(){
       rowcount = 0;
       columnNameArray[0] = "OWNER_ID";
       columnNameArray[1] = "PASSWORD";
       columnNameArray[2] = "EMAIL";
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
 public void addRecord(Owner newRecord){
    OwnerVector.add(newRecord);
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
  public synchronized Owner getRecord(int recordNumber) {
     Owner myOwner =((Owner)(OwnerVector.get(recordNumber)));
    return myOwner;
  }
 /*================================================================================================
/        getRecordNumber(Owner selectedObject)
/=================================================================================================*/
 public int getRecordNumber(Owner selectedObject){
     int recordNumber = 0;
     java.util.ListIterator li = OwnerVector.listIterator();
     while(li.hasNext()){
        Owner currentObject = (Owner)(li.next());
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
    return editable;
}
/*================================================================================================
/         getValueAt() - Required Method for the Abstract Table Model Class
/=================================================================================================*/
  public Object getValueAt(int row, int col) {
    java.lang.Object returnObject = null;
    Owner selectedRow = (Owner)OwnerVector.elementAt(row);
    if(col == 0){
       returnObject = selectedRow.getOwner_ID();
    }
    if(col == 1){
       returnObject = selectedRow.getEncryptedPassword();
    }
    if(col == 2){
       returnObject = selectedRow.getEmail();
    }
    return returnObject;
  }
/*================================================================================================
/     setValueAt(Object value, int row, int col)
/=================================================================================================*/
 public void setValueAt(Object value, int row, int col) {
   Owner selectedRow;
   if(OwnerVector.size() >= row){
       selectedRow = (Owner)OwnerVector.elementAt(row);
    if(col == 0){
       selectedRow.setOwner_ID(((java.lang.String)value));
    }
    if(col == 1){
       selectedRow.setEncryptedPassword(((java.lang.String)value));
    }
    if(col == 2){
       selectedRow.setEmail(((java.lang.String)value));
    }    
    OwnerVector.setElementAt(selectedRow,row);
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
/        getColumnClass(int c)
/=================================================================================================*/
  public Class getColumnClass(int c) {
    java.lang.Class myClass = null;
     if(c == 0){
         myClass =  (new java.lang.String()).getClass();
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
    return myClass;
  }
/*=========================================================================================================
/       End of the AstroObjectTableModel Class
/=========================================================================================================*/
}
