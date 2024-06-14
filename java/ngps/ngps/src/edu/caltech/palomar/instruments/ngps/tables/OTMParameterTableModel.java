/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package edu.caltech.palomar.instruments.ngps.tables;

import edu.caltech.palomar.instruments.ngps.dbms.edit_monitor;
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
/**
 *
 * @author jennifermilburn
 */
public class OTMParameterTableModel extends AbstractTableModel{
/*================================================================================================
/          Class Declaration for AstroObjectTableModel
/=================================================================================================*/
   java.util.Vector    parametersVector = new java.util.Vector();
   java.lang.String[]  columnNameArray   = new java.lang.String[2];
   Target              current;
   int columncount = 2;
   int rowcount = 19;
   private  javax.swing.JTable myParametersTable;  
   public edit_monitor my_edit_monitor;
/*================================================================================================
/          Class Constructor for ObservationTableModel
/=================================================================================================*/
public OTMParameterTableModel(){
    jbInit();   
    setEdited(false);
}
/*================================================================================================
/        setJTable(javax.swing.JTable newAstroObjectsTable)
/=================================================================================================*/
public void setJTable(javax.swing.JTable newAstroObjectsTable){
  myParametersTable = newAstroObjectsTable;
}
public JTable getJTable(){
    return myParametersTable;
}  
public void set_edit_monitor(edit_monitor new_edit_monitor){
   my_edit_monitor = new_edit_monitor; 
}
public void setEdited(boolean new_edited){
  if(my_edit_monitor != null){
      my_edit_monitor.setEdited(new_edited);
  }  
}
/*================================================================================================
/         clearTable()
/=================================================================================================*/
 public void clearTable(){
     parametersVector.clear();
     rowcount = 19;
     fireTableDataChanged();
 }
/*================================================================================================
/          getVector()
/=================================================================================================*/
 public java.util.Vector getVector(){
     return parametersVector;
 }
/*================================================================================================
/           jbInit() Initiaization Method
/=================================================================================================*/
    private void jbInit(){
       rowcount = 19;
       columnNameArray[0] = "PARAMETER";
       columnNameArray[1] = "VALUE";
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
public void setTarget(Target current_target){
   current =  current_target;
   fireTableDataChanged();
}
public Target getTarget(){
    return current;
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
  public synchronized java.lang.String getRecord(int recordNumber) {
     java.lang.String my_parameter_value = new java.lang.String();
      try{
         my_parameter_value =((java.lang.String)(parametersVector.get(recordNumber)));
      }catch(Exception e){
          System.out.println(e.toString());
      }
    return my_parameter_value;
  }
 /*================================================================================================
/        getRecordNumber(Owner selectedObject)
/=================================================================================================*/
 public int getRecordNumber(java.lang.String selectedObject){
     int recordNumber = 0;
     java.util.ListIterator li = parametersVector.listIterator();
     while(li.hasNext()){
        java.lang.String currentObject = (java.lang.String)(li.next());
        if(currentObject.equals(selectedObject)){
           recordNumber = li.previousIndex();
        }
     }
     return recordNumber;
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
public boolean isCellEditable(int rowIndex, int vColIndex) {
    boolean editable = false;
    if(vColIndex == 0){
        editable = false;
    }
    if(vColIndex == 1){
        editable = false;
    }
    return editable;
}
/*================================================================================================
/         getValueAt() - Required Method for the Abstract Table Model Class
/=================================================================================================*/
  public Object getValueAt(int row, int col) {
    java.lang.Object returnObject = null;
    if(col == 1){
        if(current != null){
        if(row == 0){
           returnObject = current.otm.getOTMexpt();
        }        
        if(row == 1){
           returnObject = current.otm.getOTMslitwidth();
        }        
        if(row == 2){
           returnObject = current.otm.getOTMcass();
        } 
        if(row == 3){
           returnObject = current.otm.getOTMAirmass_start();
        } 
        if(row == 4){
           returnObject = current.otm.getOTMAirmass_end();
        }
        if(row == 5){
           returnObject = current.otm.getSkymag();
        } 
        if(row == 6){
           returnObject = timestamp_to_string(current.otm.getOTMslewgo());
        }        
        if(row == 7){
           returnObject = timestamp_to_string(current.otm.getOTMstart());
        }        
        if(row == 8){
           returnObject = timestamp_to_string(current.otm.getOTMend());
        }        
        if(row == 9){
           returnObject = current.otm.getOTMpa();
        }        
        if(row == 10){
           returnObject = current.otm.getOTMwait();
        }        
        if(row == 11){
//            java.lang.String flag = current.otm.getOTMflag();
//            if(flag.contains("ALT")){
//               returnObject = "Altitude is out of range" ;
//            }else if(flag.contains("AIR")){
//                returnObject = "Airmass is out of range" ;
//            }else if(flag.contains("HA")){
//                returnObject = "Hour angle is out of range" ;
//            }else if(flag.contains("DAY")){
//                returnObject = "Obervation during daylight" ;
//            }else{
//                returnObject = current.otm.getOTMflag();
//            }                        
           returnObject = current.otm.getOTMflag();
        }           
        if(row == 12){
           returnObject = current.otm.getOTMlast();
        }
        if(row == 13){
           returnObject = current.otm.getOTMslew();
        }  
        if(row == 14){
           returnObject = current.otm.getOTMmoon();
        }  
         if(row == 15){
           returnObject = current.otm.getOTMslitangle();
        }  
        if(row == 16){
           returnObject = current.otm.getOTMSNR();
        }  
        if(row == 17){
           returnObject = current.otm.getOTMres();
        }  
        if(row == 18){
           returnObject = current.otm.getOTMseeing();
        } 
       }        
    }
    if(col == 0){
        if(row == 0){
           returnObject = "Exposure Time";
        }       
        if(row == 1){
           returnObject = "Slit Width";
        }        
        if(row == 2){
           returnObject = "Slit Angle";
        } 
        if(row == 3){
           returnObject = "Airmass start";
        }        
        if(row == 4){
           returnObject = "Airmass end";
        }  
        if(row == 5){
           returnObject = "Sky Magnitude";
        } 
        if(row == 6){
           returnObject = "Slew GO time";
        }        
        if(row == 7){
           returnObject = "Exposure Start Time";
        }        
        if(row == 8){
           returnObject = "Exposure End Time";
        }        
        if(row == 9){
           returnObject = "OTMpa";
        }        
        if(row == 10){
           returnObject = "OTMwait";
        }        
        if(row == 11){
           returnObject = "OTMflag";
        }  
        if(row == 12){
           returnObject = "OTMlast";
        }   
        if(row == 13){
           returnObject = "OTMslew";
        }                 
        if(row == 14){
           returnObject = "OTMmoon";
        }                 
        if(row == 15){
           returnObject = "OTMslitangle";
        }  
        if(row == 16){
           returnObject = "OTMSNR";
        }   
        if(row == 17){
           returnObject = "OTMres";
        }  
        if(row == 18){
           returnObject = "OTMseeing (arcsec)";
        }  
    }
    return returnObject;
  }
/*================================================================================================
/     setValueAt(Object value, int row, int col)
/=================================================================================================*/
 public void setValueAt(Object value, int row, int col) {
    setEdited(true);
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
    return myClass;
  }
/*=========================================================================================================
/       End of the AstroObjectTableModel Class
/=========================================================================================================*/
}


