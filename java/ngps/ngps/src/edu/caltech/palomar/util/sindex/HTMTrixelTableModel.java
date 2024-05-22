package edu.caltech.palomar.util.sindex; 
//=== File Prolog =============================================================
//	This code was developed by UCLA, Department of Physics and Astronomy,
//	for the Stratospheric Observatory for Infrared Astronomy (SOFIA) project.
//
//--- Contents ----------------------------------------------------------------
//	HTMTrixelTableModel  -  A Table Model for containing a description of
//           a HTM triangle
//--- Description -------------------------------------------------------------
//
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
//
//      07/07/04        J. Milburn
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
//	In no event shall UCLA be liable for any damages, including, but not
//	limited to direct, indirect, special or consequential damages, arising out
//	of, resulting from, or in any way connected with this software, whether or
//	not based upon warranty, contract, tort or otherwise, whether or not
//	injury was sustained by persons or property or otherwise, and whether or
//	not loss was sustained from or arose out of the results of, or use of,
//	their software or services provided hereunder.
//
//=== End File Prolog =========================================================
import javax.swing.table.AbstractTableModel;
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
import edu.caltech.palomar.util.sindex.TriangleDataModel;
/*================================================================================================
/      class HTMTrixelTableModel
/=================================================================================================*/
public class HTMTrixelTableModel   extends AbstractTableModel {
  java.util.Vector HTMVector = new java.util.Vector();
  java.lang.String[]  columnNameArray = new java.lang.String[11];
  int columncount = 10;
  int rowcount;
  public static java.lang.String columnName1  = "HTM Index ID";
  public static java.lang.String columnName2  = "HTM Node Name";
  public static java.lang.String columnName3  = "RA decimal hours";
  public static java.lang.String columnName4  = "Dec decimal degrees";
  public static java.lang.String columnName5  = "RA hh:mm:ss";
  public static java.lang.String columnName6  = "Dec dd:mm:ss";
  public static java.lang.String columnName7  = "X vector";
  public static java.lang.String columnName8  = "Y vector";
  public static java.lang.String columnName9  = "Z vector";
  public static java.lang.String columnName10 = "Triangle Area x 10+6";
/*================================================================================================
/      constructor for the HTMTrixelTableModel
/=================================================================================================*/
  public HTMTrixelTableModel() {
     initializeColumns();
  }
/*================================================================================================
/         initializeColumns()
/=================================================================================================*/
   public void initializeColumns(){
      columnNameArray[0]  = columnName1;
      columnNameArray[1]  = columnName2;
      columnNameArray[2]  = columnName3;
      columnNameArray[3]  = columnName4;
      columnNameArray[4]  = columnName5;
      columnNameArray[5]  = columnName6;
      columnNameArray[6]  = columnName7;
      columnNameArray[7]  = columnName8;
      columnNameArray[8]  = columnName9;
      columnNameArray[9]  = columnName10;
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
/*================================================================================================
/         addRecord()
/=================================================================================================*/
   //  addRecord Method f
   public void addRecord(TriangleDataModel newTriangleModel){
      HTMVector.addElement(newTriangleModel);
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
    public synchronized TriangleDataModel getRecord(int recordNumber) {
       TriangleDataModel myTriangleDataModel =((TriangleDataModel)(HTMVector.get(recordNumber)));
      return myTriangleDataModel;
    }
/*================================================================================================
/         getValueAt() - Required Method for the Abstract Table Model Class
/=================================================================================================*/
    public Object getValueAt(int row, int col) {
      java.lang.Object returnObject = null;
      TriangleDataModel selectedRow = (TriangleDataModel)HTMVector.elementAt(row);
      returnObject = selectedRow.getValue(col);
     return returnObject;
    }
/*================================================================================================
/
/=================================================================================================*/
   public void setValueAt(Object value, int row, int col) {
     TriangleDataModel selectedRow;
     if(HTMVector.size() >= row){
         selectedRow = (TriangleDataModel)HTMVector.elementAt(row);
         selectedRow.setValue(col,value);
      HTMVector.setElementAt(selectedRow,row);
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
/     clearTable()
/=================================================================================================*/
  public void clearTable(){
    rowcount = 0;
    HTMVector.clear();
    fireTableDataChanged();
  }
/*================================================================================================
/     setColumnName(int col)
/=================================================================================================*/
   public void setColumnName(int col,java.lang.String newColumnName){
       columnNameArray[col] = newColumnName;
   }
/*================================================================================================
/     setColumnName( )
/=================================================================================================*/
   public void setColumnNames(java.lang.String newColumnName1,java.lang.String newColumnName2,
                              java.lang.String newColumnName3,java.lang.String newColumnName4,
                              java.lang.String newColumnName5,java.lang.String newColumnName6,
                              java.lang.String newColumnName7,java.lang.String newColumnName8,
                              java.lang.String newColumnName9,java.lang.String newColumnName10){
       columnNameArray[0]  = newColumnName1;
       columnNameArray[1]  = newColumnName2;
       columnNameArray[2]  = newColumnName3;
       columnNameArray[3]  = newColumnName4;
       columnNameArray[4]  = newColumnName5;
       columnNameArray[5]  = newColumnName6;
       columnNameArray[6]  = newColumnName7;
       columnNameArray[7]  = newColumnName8;
       columnNameArray[8]  = newColumnName9;
       columnNameArray[9]  = newColumnName10;
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
         myClass = (((TriangleDataModel)HTMVector.firstElement()).getValue(c).getClass());
      return myClass;
    }
/*================================================================================================
/      End of the class HTMTrixelTableModel
/=================================================================================================*/
}