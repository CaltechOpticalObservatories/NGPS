
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
public class ETCParameterTableModel extends AbstractTableModel{
/*================================================================================================
/          Class Declaration for AstroObjectTableModel
/=================================================================================================*/
   java.util.Vector    parametersVector = new java.util.Vector();
   java.lang.String[]  columnNameArray   = new java.lang.String[2];
   Target              current;
   int columncount = 2;
   int rowcount = 8;
   private  javax.swing.JTable myParametersTable;  
   public edit_monitor my_edit_monitor;
/*================================================================================================
/          Class Constructor for ObservationTableModel
/=================================================================================================*/
public ETCParameterTableModel(){
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
     rowcount = 8;
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
       rowcount = 10;
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
   timestamp = null;
}
return timestamp;
}
public java.lang.String timestamp_to_string(java.sql.Timestamp current){
//    java.sql.Timestamp timestamp = java.sql.Timestamp.valueOf("2018-12-12 01:02:03.123456789");
     String timestampAsString = new java.lang.String();
     try{
          java.time.format.DateTimeFormatter formatter = java.time.format.DateTimeFormatter.ISO_LOCAL_DATE_TIME;
          timestampAsString = formatter.format(current.toLocalDateTime());       
     }catch(Exception e){
        System.out.println(e.toString());
     }
//    assertEquals("2018-12-12T01:02:03.123456789", timestampAsString);
return timestampAsString;
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
     java.lang.String my_parameter_value =((java.lang.String)(parametersVector.get(recordNumber)));
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
        editable = true;
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
           returnObject = "";
        }   
        if(row == 1){
           returnObject = current.etc.getWRANGE_LOW();
        }   
        if(row == 2){
           returnObject = current.etc.getWRANGE_HIGH();
        }   
        if(row == 3){
           returnObject = current.etc.getChannel();
        }   
        if(row == 4){
           returnObject = current.etc.getMagnitude();
        }
        if(row == 5){
           returnObject = current.etc.getMagref_system();
        }   
        if(row == 6){
           returnObject = current.etc.getMagref_filter();
        }   
        if(row == 7){
           returnObject = current.etc.getSrcmodel();
        }   
        if(row == 8){
           returnObject = timestamp_to_string(current.otm.getOTMnotbefore());
        } 
        if(row == 9){
           returnObject = current.otm.getOTMpointmode();
        } 
        }        
    }
    if(col == 0){

        if(row == 0){
           returnObject = "Exposure Time Calculator (ETC) parameters";
        }                 
        if(row == 1){
           returnObject = "Wavelength Range LOW (nanometers)";
        }           
        if(row == 2){
           returnObject = "Wavelength Range HIGH (nanometers)";
        }  
        if(row == 3){
           returnObject = "Channel (filter)";
        }  
        if(row == 4){
           returnObject = "Magnitude";
        }  
        if(row == 5){
           returnObject = "Magnitude Reference System";
        }  
        if(row == 6){
           returnObject = "Magnitude Filter";
        }  
        if(row == 7){
           returnObject = "SRCMODEL parameters";
        }  
        if(row == 8){
           returnObject = "NOT BEFORE";
        }  
        if(row == 9){
           returnObject = "POINTMODE";
        }  
    }
    return returnObject;
  }
/*================================================================================================
/     setValueAt(Object value, int row, int col)
/=================================================================================================*/
 public void setValueAt(Object value, int row, int col) {
    if(col == 1){
        if(row == 0){
           
        }         
        if(row == 1){
           try{
           current.etc.setWRANGE_LOW(Integer.valueOf((java.lang.String)value));
           }catch(Exception e){
           }               
        }  
        if(row == 2){
           try{
           current.etc.setWRANGE_HIGH(Integer.valueOf((java.lang.String)value));
           }catch(Exception e){
           }               
        }  
        if(row == 3){
           current.etc.setChannel((java.lang.String)value);
        } 
        if(row == 4){
           try{
           current.etc.setMagnitude(Double.valueOf((java.lang.String)value));
           }catch(Exception e){
           }               
        }           
        if(row == 5){
           current.etc.setMagref_system((java.lang.String)value);
        } 
        if(row == 6){
           current.etc.setMagref_filter((java.lang.String)value);
        } 
        if(row == 7){
           current.etc.setSrcmodel((java.lang.String)value);
        } 
        if(row == 8){
            try{
               current.otm.setOTMnotbefore(string_to_timestamp((java.lang.String)value));
            }catch(Exception e){
              current.otm.setOTMnotbefore(null);     
            }
        } 
        if(row == 9){
           current.otm.setOTMpointmode((java.lang.String)value);
        } 
    }
   fireTableCellUpdated(row, col);
   fireTableDataChanged();
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

