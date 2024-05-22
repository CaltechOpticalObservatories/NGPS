/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package edu.caltech.palomar.instruments.ngps.tables;

//import edu.caltech.palomar.telescopes.P200.gui.tables.AstroObject;
import edu.dartmouth.jskycalc.coord.RA;
import edu.dartmouth.jskycalc.coord.dec;
import javax.swing.JTable;
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
/**
 *
 * @author jennifermilburn
 */
public class OTMTableModel extends AbstractTableModel{
//=== File Prolog =============================================================
//	This code was developed by John Thorstensen of Dartmouth University
//
//      It has been refactored and modified by Jennifer Milburn for
//      use at the Palomar Observatory.
//
//      Caltech Optical Observatories, Palomar Observatory
//
//--- Contents ----------------------------------------------------------------
//   ObservationTableModel()  -
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
   java.util.Vector    OTMVector = new java.util.Vector();
   java.lang.String[]  columnNameArray   = new java.lang.String[20];
   int columncount = 20;
   int rowcount;
   private  javax.swing.JTable myObservationsTable;  
/*================================================================================================
/          Class Constructor for ObservationDBMSTableModel
/=================================================================================================*/
public OTMTableModel(){
    jbInit();   
}
/*================================================================================================
/        setJTable(javax.swing.JTable newAstroObjectsTable)
/=================================================================================================*/
public void setJTable(javax.swing.JTable newAstroObjectsTable){
  myObservationsTable = newAstroObjectsTable;
}
public JTable getJTable(){
    return myObservationsTable;
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
public java.lang.String timestamp_to_string(java.sql.Timestamp current){
//    java.sql.Timestamp timestamp = java.sql.Timestamp.valueOf("2018-12-12 01:02:03.123456789");
    java.time.format.DateTimeFormatter formatter = java.time.format.DateTimeFormatter.ISO_LOCAL_DATE_TIME;
    String timestampAsString = formatter.format(current.toLocalDateTime());
//    assertEquals("2018-12-12T01:02:03.123456789", timestampAsString);
return timestampAsString;
}
/*================================================================================================
/         clearTable()
/=================================================================================================*/
 public void clearTable(){
     OTMVector.clear();
     rowcount = 0;
     fireTableDataChanged();
 }
/*================================================================================================
/          getVector()
/=================================================================================================*/
 public java.util.Vector getVector(){
     return OTMVector;
 }
/*================================================================================================
/           jbInit() Initiaization Method
/=================================================================================================*/
    private void jbInit(){
       rowcount = 0;
//       columnNameArray[0] = "OBS_ORDER";
//       columnNameArray[1] = "NAME";
//       columnNameArray[2] = "pa";
//       columnNameArray[3] = "cass";
//       columnNameArray[4] = "wait";
//       columnNameArray[5] = "flag";
//       columnNameArray[6] = "slewgo";
//       columnNameArray[7] = "last";
//       columnNameArray[8] = "slew";
//       columnNameArray[9] = "dead";
//       columnNameArray[10] = "start";
//       columnNameArray[11] = "end";
//       columnNameArray[12] = "AIRMASS";
//       columnNameArray[13] = "SKYMAG";
//       columnNameArray[14] = "expt";
//       columnNameArray[15] = "slit";
       columnNameArray[0] = "OBS_ORDER";
       columnNameArray[1] = "NAME";
       columnNameArray[2] = "expt";
       columnNameArray[3] = "slit";       
       columnNameArray[4] = "cass";
       columnNameArray[5] = "AIRMASS_start";
       columnNameArray[6] = "AIRMASS_end";
       columnNameArray[7] = "SKYMAG";      
       columnNameArray[8] = "dead";
       columnNameArray[9] = "slewgo";
       columnNameArray[10] = "start";
       columnNameArray[11] = "end";
       columnNameArray[12] = "pa";            
       columnNameArray[13] = "wait";
       columnNameArray[14] = "flag";
       columnNameArray[15] = "last";
       columnNameArray[16] = "slew";
       columnNameArray[17] = "moon";
       columnNameArray[18] = "notbefore";
       columnNameArray[19] = "pointmode";
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
 public void addRecord(Target newRecord){
    OTMVector.add(newRecord);
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
  public synchronized Target getRecord(int recordNumber) {
     Target myAstroObject =((Target)(OTMVector.get(recordNumber)));
    return myAstroObject;
  }
 /*================================================================================================
/        getRecordNumber(AstroObject selectedObject)
/=================================================================================================*/
 public int getRecordNumber(Target selectedObject){
     int recordNumber = 0;
     java.util.ListIterator li = OTMVector.listIterator();
     while(li.hasNext()){
        Target currentObject = (Target)(li.next());
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
    if(vColIndex == 7){
        editable = true;
    }
    if(vColIndex == 8){
        editable = true;
    }
    if(vColIndex == 9){
        editable = true;
    }
    if(vColIndex == 10){
        editable = true;
    }
    if(vColIndex == 11){
        editable = true;
    }
    if(vColIndex == 12){
        editable = true;
    }
    if(vColIndex == 13){
        editable = true;
    }
    if(vColIndex == 14){
        editable = true;
    }
    if(vColIndex == 15){
        editable = true;
    }
    if(vColIndex == 16){
        editable = true;
    }
    if(vColIndex == 17){
        editable = true;
    }
    if(vColIndex == 18){
        editable = true;
    }
    if(vColIndex == 19){
        editable = true;
    }
    return editable;
}
/*================================================================================================
/         getValueAt() - Required Method for the Abstract Table Model Class
/=================================================================================================*/
  public Object getValueAt(int row, int col) {
    java.lang.Object returnObject = null;
    Target selectedRow = (Target)OTMVector.elementAt(row);
    if(col == 0){
       returnObject = selectedRow.getOrder();
    }   
    if(col == 1){
       returnObject = selectedRow.name;
    }
    if(col == 2){
       returnObject = selectedRow.otm.getOTMexpt();
    } 
    if(col == 3){
       returnObject = selectedRow.otm.getOTMslitwidth();
    }     
    if(col == 4){
       returnObject = selectedRow.otm.getOTMcass();
    }  
    if(col == 5){
       returnObject = selectedRow.otm.getOTMAirmass_start();
    } 
    if(col == 6){
       returnObject = selectedRow.otm.getOTMAirmass_end();
    } 
    if(col == 7){
       returnObject = selectedRow.otm.getSkymag();
    } 
    if(col == 8){
       returnObject = selectedRow.otm.getOTMdead();
    }
   if(col == 9){
       returnObject = timestamp_to_string(selectedRow.otm.getOTMslewgo());
    }
    if(col == 10){
       returnObject = timestamp_to_string(selectedRow.otm.getOTMstart());
    }
    if(col == 11){
       returnObject = timestamp_to_string(selectedRow.otm.getOTMend());
    }        
    if(col == 12){
       returnObject = selectedRow.otm.getOTMpa();
    }
    if(col == 13){
       returnObject = selectedRow.otm.getOTMwait();
    }
    if(col == 14){
        java.lang.String flag = selectedRow.otm.getOTMflag();
        if(flag.contains("ALT")){
           returnObject = "Target's altitude is out of range" ;
        }else if(flag.contains("AIR")){
            returnObject = "Target's airmass is out of range" ;
        }else if(flag.contains("HA")){
            returnObject = "Target's hour angle is out of range" ;
        }else if(flag.contains("DAY")){
            returnObject = "Obervation is happening during daylight" ;
        }else{
            returnObject = selectedRow.otm.getOTMflag();
        }
//        
//       returnObject = selectedRow.otm.getOTMflag();
    }
    if(col == 15){
       returnObject = selectedRow.otm.getOTMlast();
    }
    if(col == 16){
       returnObject = selectedRow.otm.getOTMslew();
    }
    if(col == 17){
       returnObject = selectedRow.otm.getOTMmoon();
    }
    if(col == 18){
       returnObject = timestamp_to_string(selectedRow.otm.getOTMnotbefore());
    }
    if(col == 19){
       returnObject = selectedRow.otm.getOTMpointmode();
    }
    return returnObject;
  }
/*================================================================================================
/     setValueAt(Object value, int row, int col)
/=================================================================================================*/
 public void setValueAt(Object value, int row, int col) {
   Target selectedRow;
   if(OTMVector.size() >= row){
       selectedRow = (Target)OTMVector.elementAt(row);
    if(col == 0){
       selectedRow.setOrder(((java.lang.Integer)value).intValue());
    } 
    if(col == 1){
       selectedRow.name = ((java.lang.String)value);
    }
    if(col == 2){
       selectedRow.otm.setOTMexpt(((java.lang.Double)value).doubleValue());
    } 
    if(col == 3){
       selectedRow.otm.setOTMslitwidth(((java.lang.Double)value).doubleValue());
    }     
    if(col == 4){
       selectedRow.otm.setOTMcass(((java.lang.Double)value).doubleValue());
    }
    if(col == 5){
       selectedRow.otm.setOTMAirmass_start(((java.lang.Double)value).doubleValue());
    }      
    if(col == 6){
       selectedRow.otm.setOTMAirmass_end(((java.lang.Double)value).doubleValue());
    } 
    if(col == 7){
       selectedRow.otm.setSkymag(((java.lang.Double)value).doubleValue());
    }     
    if(col == 8){
       selectedRow.otm.setOTMdead(((java.lang.Double)value).doubleValue());
    }   
    if(col == 9){        
       selectedRow.otm.setOTMslewgo(string_to_timestamp((java.lang.String)value));
    }    
    if(col == 10){
       selectedRow.otm.setOTMstart(string_to_timestamp((java.lang.String)value));
    }  
    if(col == 11){
       selectedRow.otm.setOTMend(string_to_timestamp((java.lang.String)value));
    }         
    if(col == 12){
       selectedRow.otm.setOTMpa(((java.lang.Double)value).doubleValue());
    }
    if(col == 13){
       selectedRow.otm.setOTMwait(((java.lang.Integer)value).intValue());
    }
    if(col == 14){
       selectedRow.otm.setOTMflag(((java.lang.String)value));
    }
    if(col == 15){
       selectedRow.otm.setOTMlast(((java.lang.String)value));
    }
    if(col == 16){
       selectedRow.otm.setOTMslew(((java.lang.Double)value).doubleValue());
    }   
    if(col == 17){
       selectedRow.otm.setOTMmoon(((java.lang.String)value));
    } 
    if(col == 18){
       selectedRow.otm.setOTMnotbefore(string_to_timestamp((java.lang.String)value));
    } 
    if(col == 19){
       selectedRow.otm .setOTMpointmode(((java.lang.String)value));
    } 
    OTMVector.setElementAt(selectedRow,row);
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
         myClass =  (Integer.valueOf(0)).getClass();
         //        myClass =  (new Double(((AstroObject)AstroObjectVector.firstElement()).Delta.value)).getClass();
        return myClass;
     }
    if(c == 1){
         myClass =  (new String()).getClass();
         //        myClass =  (new Double(((AstroObject)AstroObjectVector.firstElement()).Delta.value)).getClass();
        return myClass;
     }
    if(c == 2){
         myClass =  (Double.valueOf(0)).getClass();
         //        myClass =  (new Double(((AstroObject)AstroObjectVector.firstElement()).Delta.value)).getClass();
        return myClass;
     } 
    if(c == 3){
         myClass =  (Double.valueOf(0)).getClass();
         //        myClass =  (new Double(((AstroObject)AstroObjectVector.firstElement()).Delta.value)).getClass();
        return myClass;
     }        
    if(c == 4){
         myClass =  (Double.valueOf(0)).getClass();
//         myClass = ((AstroObject)AstroObjectVector.firstElement()).name.getClass();
        return myClass;
     }
    if(c == 5){
       myClass =  (Double.valueOf(0)).getClass();
       //        myClass = ((AstroObject)AstroObjectVector.firstElement()).Alpha.RoundedRAString(2, ":").getClass();
        return myClass;
     }
    if(c == 6){
       myClass =  (Double.valueOf(0)).getClass();
       //        myClass = ((AstroObject)AstroObjectVector.firstElement()).Alpha.RoundedRAString(2, ":").getClass();
        return myClass;
     }    
     if(c == 7){
      myClass =  (Double.valueOf(0)).getClass();
 //        myClass = ((AstroObject)AstroObjectVector.firstElement()).Delta.RoundedDecString(2, ":").getClass();
        return myClass;
     }
     if(c == 8){
         myClass =  (Double.valueOf(0)).getClass();
//        myClass =  (new Double(((AstroObject)AstroObjectVector.firstElement()).Alpha.value)).getClass();
        return myClass;
     }
     if(c == 9){
         myClass =  (new java.lang.String()).getClass();
//        myClass =  (new Double(((AstroObject)AstroObjectVector.firstElement()).Alpha.value)).getClass();
        return myClass;
     }
     if(c == 10){
         myClass =  (new java.lang.String()).getClass();
//         myClass =  (new Double(((AstroObject)AstroObjectVector.firstElement()).r_m)).getClass();
        return myClass;
     }
     if(c == 11){
         myClass =  (new java.lang.String()).getClass();
        return myClass;
     }
     if(c == 12){
        myClass = (Double.valueOf(0)).getClass();
        return myClass;
     }
     if(c == 13){
        myClass =    (Integer.valueOf(0)).getClass();
        return myClass;
     }
     if(c == 14){
        myClass =    (new java.lang.String()).getClass();
        return myClass;
     }
     if(c == 15){
         myClass =    (new java.lang.String()).getClass();
        return myClass;
     } 
     if(c == 16){
         myClass =   (Double.valueOf(0)).getClass();
        return myClass;
     }
     if(c == 17){
         myClass =   (Double.valueOf(0)).getClass();
        return myClass;
     }
     if(c == 18){
         myClass =    (new java.lang.String()).getClass();
        return myClass;
     } 
     if(c == 19){
         myClass =    (new java.lang.String()).getClass();
        return myClass;
     } 
     return myClass;
  }
/*=========================================================================================================
/       End of the AstroObjectTableModel Class
/=========================================================================================================*/
}

