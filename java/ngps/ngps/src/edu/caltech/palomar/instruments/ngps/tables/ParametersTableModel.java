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
public class ParametersTableModel extends AbstractTableModel{
/*================================================================================================
/          Class Declaration for AstroObjectTableModel
/=================================================================================================*/
   java.util.Vector    parametersVector = new java.util.Vector();
   java.lang.String[]  columnNameArray   = new java.lang.String[2];
   Target              current;
   int columncount = 2;
   int rowcount = 14;
   private  javax.swing.JTable myParametersTable;  
   public edit_monitor my_edit_monitor;
/*================================================================================================
/          Class Constructor for ObservationTableModel
/=================================================================================================*/
public ParametersTableModel(){
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
     rowcount = 14;
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
       rowcount = 14;
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
           returnObject = current.sky.getRightAscension();
        }        
        if(row == 1){
           returnObject = current.sky.getDeclination();
        }        
        if(row == 2){
           returnObject = current.sky.getOFFSET_RA();
        } 
        if(row == 3){
           returnObject = current.sky.getOFFSET_DEC();
        }        
        if(row == 4){
           returnObject = current.instrument.getExposuretime();
        }        
        if(row == 5){
           returnObject = current.instrument.getSlitwidth();
        }        
        if(row == 6){
           returnObject = current.instrument.getSlitOffset();
        }        
        if(row == 7){
           returnObject = current.instrument.getOBSMODE();
        }        
        if(row == 8){
           returnObject = current.instrument.getBIN_SPEC();
        }        
        if(row == 9){
           returnObject = current.instrument.getBIN_SPACE();
        }           
        if(row == 10){
           returnObject = current.instrument.getRequestedSlitAngle();
        }
        if(row == 11){
           returnObject = current.sky.getAIRMASS_MAX();
        }        
        if(row == 12){
           returnObject = current.getCOMMENT();
        }
       }        
    }
    if(col == 0){
        if(row == 0){
           returnObject = "RA (hh:mm:ss.s)";
        }        
        if(row == 1){
           returnObject = "DEC (dd:mm:ss.s)";
        }        
        if(row == 2){
           returnObject = "RA offset (arcseconds)";
        } 
        if(row == 3){
           returnObject = "DEC offset (arcseconds)";
        }        
        if(row == 4){
           returnObject = "Exposure Time";
        }        
        if(row == 5){
           returnObject = "Slit Width (arcseconds)";
        }        
        if(row == 6){
           returnObject = "Slit offset (arcseconds)";
        }        
        if(row == 7){
           returnObject = "CCD Mode";
        }        
        if(row == 8){
           returnObject = "Bin Spectral (integer)";
        }        
        if(row == 9){
           returnObject =  "Bin Spatial (integer)";
        }  
        if(row == 10){
           returnObject =  "Cassegrain Angle (degrees)";
        } 
        if(row == 11){
           returnObject =  "AIRMASS_MAX";
        } 
        if(row == 12){
           returnObject =  "Comment";
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
           current.sky.setRightAscension((java.lang.String)value);
        }        
        if(row == 1){
           current.sky.setDeclination((java.lang.String)value);
        }        
        if(row == 2){
           try{
           current.sky.setOFFSET_RA(Double.valueOf((java.lang.String)value));
           }catch(Exception e){
           }
        } 
        if(row == 3){
           try{
           current.sky.setOFFSET_DEC(Double.valueOf((java.lang.String)value));
           }catch(Exception e){
           }        }        
        if(row == 4){
           current.instrument.setExposuretime((java.lang.String)value);
        }        
        if(row == 5){
           current.instrument.setSlitwidth_string((java.lang.String)value);
        }        
        if(row == 6){
           try{
           current.instrument.setSlitOffset(Double.valueOf((java.lang.String)value));
           }catch(Exception e){
           }               
        }        
        if(row == 7){
           current.instrument.setOBSMODE((java.lang.String)value);
        }        
        if(row == 8){
           try{
           current.instrument.setBIN_SPEC(Integer.valueOf((java.lang.String)value));
           }catch(Exception e){
           }               
        }        
        if(row == 9){
           try{
           current.instrument.setBIN_SPACE(Integer.valueOf((java.lang.String)value));
           }catch(Exception e){
           }               
        }           
        if(row == 10){
           current.instrument.setRequestedSlitAngle((java.lang.String)value);
        }  
        if(row == 11){
           current.sky.setAIRMASS_MAX((java.lang.String)value);
        }  
        if(row == 12){
           current.setCOMMENT((java.lang.String)value);
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

