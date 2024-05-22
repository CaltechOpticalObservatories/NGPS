/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package edu.caltech.palomar.instruments.ngps.tables;

import javax.swing.JTable;
import javax.swing.table.AbstractTableModel;
import javax.swing.table.DefaultTableModel;
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
import edu.caltech.palomar.instruments.ngps.util.Reorderable;
import edu.caltech.palomar.instruments.ngps.util.Reorderable;
import java.util.HashMap;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.Iterator;
/**
//=== File Prolog =============================================================
//	This code was developed by John Thorstensen of Dartmouth University
//
//      It has been refactored and modified by Jennifer Milburn for
//      use at the Palomar Observatory.
//
//      Caltech Optical Observatories, Palomar Observatory
//
//--- Contents ----------------------------------------------------------------
//   ActiveTargetTableModel ()  -
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
 public class ActiveTargetTableModel  extends DefaultTableModel implements Reorderable{
  boolean[] editable_array = new boolean[16];
  public ArrayList  myArrayList  = new ArrayList();
//  public LinkedList myLinkedList = new LinkedList();
  
  public ActiveTargetTableModel (){  
      super();
      initializeTableColumns();
  }
/*================================================================================================
/        initializeTableColumns()
/=================================================================================================*/
private void initializeTableColumns(){
     setRowCount(0);
     addColumn("STATE");
     addColumn("OBSERVATION_ID");
     addColumn("TARGET_NUMBER");
     addColumn("SEQUENCE_NUMBER");
     addColumn("NAME");
     addColumn("RA");
     addColumn("DECL");
     addColumn("OFFSET_RA");
     addColumn("OFFSET_DEC");    
     addColumn("EXPTIME");
     addColumn("SLITW");
     addColumn("SLITOFFSET");
     addColumn("CCDMODE");
     addColumn("BINSPECT");
     addColumn("BINSPAT");
     addColumn("CASSANGLE");
     editable_array[0] = true;
     editable_array[1] = true;
     editable_array[2] = true;
     editable_array[3] = true;
     editable_array[4] = true;
     editable_array[5] = true;
     editable_array[6] = true;
     editable_array[7] = true;
     editable_array[8] = true;
     editable_array[9] = true;
     editable_array[10] = true;
     editable_array[11] = true;
     editable_array[12] = true;
     editable_array[13] = true;
     editable_array[14] = true;
     editable_array[15] = true;
//     editable_array[16] = true;
   }
/*================================================================================================
/      reorder(int fromIndex, int toIndex)
/=================================================================================================*/
public void reorder(int fromIndex, int toIndex){
    moveRow(fromIndex, fromIndex, toIndex);
    reorder_table();
    fireTableDataChanged();
 }
/*================================================================================================
/      reorder(int fromIndex, int toIndex)
/=================================================================================================*/
public void reorder(int[] fromIndex, int toIndex){
//    int count = fromIndex.length;
    moveRow(fromIndex[0], fromIndex[1], toIndex);    
    reorder_table();
    fireTableDataChanged();
 }
/*================================================================================================
/         clearTable()
/=================================================================================================*/
 public void reorder_table(){
     int count = 0;
     Iterator it = getDataVector().iterator();
     while(it.hasNext()){
        Vector my_vector = (Vector)it.next();
        my_vector.set(2,count);
        count = count+1;
     }
 }
/*================================================================================================
/         clearTable()
/=================================================================================================*/
 public void clearTable(){
     getDataVector().clear();
     myArrayList.clear();
     setRowCount(0);
     fireTableDataChanged();
 }
/*================================================================================================
/      addTarget(Target current)
/=================================================================================================*/
 public void addTarget(Target current){
     int size = myArrayList.size();
     current.setOrder(size);
     current.setIndex(size);
     myArrayList.add(current);
     addRow(target_to_vector(current));
 }
/*================================================================================================
/     insert(int index,Target current)
/=================================================================================================*/
 public void insert(int index,Target current){
    int next_index = myArrayList.size();
    current.setIndex(next_index);
    Vector target_vector = target_to_vector(current);
    myArrayList.add(current);
    insertRow(index,target_vector);
    reorder_table();
 }
/*================================================================================================
/     delete(int index)
/=================================================================================================*/
 public void delete(int index){
    removeRow(index);
    reorder_table();
 } 
/*================================================================================================
/       target_to_vector(Target current)
/=================================================================================================*/
 public Vector target_to_vector(Target current){
     java.util.Vector target_vector = new java.util.Vector();
     target_vector.add(current.getSTATE());
     target_vector.add(current.getObservationID());
     target_vector.add(current.getTarget_Number());
     target_vector.add(current.getSequence_Number());     
     target_vector.add(current.getName());
     target_vector.add(current.sky.getRightAscension());
     target_vector.add(current.sky.getDeclination());
     target_vector.add(current.sky.getOFFSET_RA());
     target_vector.add(current.sky.getOFFSET_DEC());     
     target_vector.add(current.instrument.getExposuretime());
     target_vector.add(current.instrument.getSlitwidth_string());
     target_vector.add(current.instrument.getSlitOffset());
     target_vector.add(current.instrument.getOBSMODE());
     target_vector.add(current.instrument.getBIN_SPEC());
     target_vector.add(current.instrument.getBIN_SPACE());
     target_vector.add(current.instrument.getRequestedSlitAngle());     
     target_vector.add(current);
     return target_vector;       
 } 
/*================================================================================================
/       vector_to_target(Vector current)
/=================================================================================================*/
 public Target vector_to_target(int row,Vector current){
     int size = current.size();
     Target current_target = new Target();
//     myArrayList.get(size)
//     Target current_target = vector_to_target(row,getDataVector().get(row)); 
//     Target current_target = (Target)myArrayList.get(row);
     if(current_target != null){
     current_target.setSTATE((String)current.get(0));
     current_target.setObservationID((Integer)current.get(1));
     current_target.setTarget_Number((Integer)current.get(2));
     current_target.setSequence_Number((Integer)current.get(3));
     current_target.setName((String)current.get(4));
     current_target.sky.setRightAscension((String)current.get(5));
     current_target.sky.setDeclination((String)current.get(6));
     current_target.sky.setOFFSET_RA((Double)current.get(7));
     current_target.sky.setOFFSET_DEC((Double)current.get(8));
     current_target.instrument.setExposuretime((String)current.get(10));
     current_target.instrument.setSlitwidth_string((String)current.get(11));
     current_target.instrument.setSlitOffset((Double)current.get(12));
     current_target.instrument.setOBSMODE((String)current.get(13));
     current_target.instrument.setBIN_SPEC((Integer)current.get(14));
     current_target.instrument.setBIN_SPACE((Integer)current.get(15));
     current_target.instrument.setRequestedSlitAngle((String)current.get(16));
     }
   return current_target;       
 } 
/*================================================================================================
/         getRecord(int recordNumber)
/=================================================================================================*/
  public synchronized Target getRecord(int recordNumber) {
     Target current = vector_to_target(recordNumber,getDataVector().get(recordNumber));
    return current;
  }
/*================================================================================================
/         getValueAt() - Required Method for the Abstract Table Model Class
/=================================================================================================*/
  public Object getValueAt(int row, int col) {
    java.lang.Object returnObject = null;
    Target selectedRow = vector_to_target(row,getDataVector().get(row));   
    if(col == 0){
       returnObject = selectedRow.getSTATE();
    }
    if(col == 1){
       returnObject = selectedRow.getObservationID();
    }
    if(col == 2){
       returnObject = selectedRow.getTarget_Number();
    }
    if(col == 3){
       returnObject = selectedRow.getSequence_Number();
    }    
    if(col == 4){
       returnObject = selectedRow.name;
    }  
    if(col == 5){
       returnObject = selectedRow.sky.getRightAscension();
    }
    if(col == 6){
       returnObject = selectedRow.sky.getDeclination();
    }
    if(col == 7){
       returnObject = selectedRow.sky.getOFFSET_RA();
    }
    if(col == 8){
       returnObject = selectedRow.sky.getOFFSET_DEC();
    }     
    if(col == 9){
       returnObject = selectedRow.instrument.getExposuretime();
    }
    if(col == 10){
       returnObject = selectedRow.instrument.getSlitwidth_string();
    }
    if(col == 11){
       returnObject = selectedRow.instrument.getSlitOffset();
    }
    if(col == 12){
       returnObject = selectedRow.instrument.getOBSMODE();
    }
    if(col == 13){
       returnObject = selectedRow.instrument.getBIN_SPEC();
    }
    if(col == 14){
       returnObject = selectedRow.instrument.getBIN_SPACE();
    }
    if(col == 15){
       returnObject = selectedRow.instrument.getRequestedSlitAngle();
    }
    return returnObject;
  } 
/*================================================================================================
/     setValueAt(Object value, int row, int col)
/=================================================================================================*/
 public void setValueAt(Object value, int row, int col) {
   Target selectedRow = vector_to_target(row,getDataVector().get(row));  
    if(col == 0){
       selectedRow.setSTATE((java.lang.String)value);
    }
    if(col == 1){
       selectedRow.setObservationID(((java.lang.Integer)value).intValue());
    }
    if(col == 2){
       selectedRow.setTarget_Number(((java.lang.Integer)value).intValue());
    }
    if(col == 3){
       selectedRow.setSequence_Number(((java.lang.Integer)value).intValue());
    }
    if(col == 4){
       selectedRow.name = ((java.lang.String)value);
    }
    if(col == 5){
       selectedRow.sky.setRightAscension((java.lang.String)value);
    }
    if(col == 6){
       selectedRow.sky.setDeclination(((java.lang.String)value));
    }
    if(col == 7){
       selectedRow.sky.setOFFSET_RA(((java.lang.Double)value).doubleValue());
    }    
    if(col == 8){
       selectedRow.sky.setOFFSET_DEC(((java.lang.Double)value).doubleValue());
    }    
    if(col == 9){
       selectedRow.instrument.setExposuretime(((java.lang.String)value));
    }
    if(col == 10){
       selectedRow.instrument.setSlitwidth_string(((java.lang.String)value));
    }
    if(col == 11){
       selectedRow.instrument.setSlitOffset(((java.lang.Double)value).doubleValue());
    }
    if(col == 12){
       selectedRow.instrument.setOBSMODE(((java.lang.String)value));
    }
    if(col == 13){
       selectedRow.instrument.setBIN_SPEC(((java.lang.Integer)value).intValue());
    }
    if(col == 14){
       selectedRow.instrument.setBIN_SPACE(((java.lang.Integer)value).intValue());
    }  
    if(col == 15){
       selectedRow.instrument.setRequestedSlitAngle(((java.lang.String)value));
    }     
    getDataVector().setElementAt(target_to_vector(selectedRow),row);
    fireTableCellUpdated(row, col);
    fireTableDataChanged();
 }
  /*================================================================================================
/         Required Methods for the Abstract Table Model Class
/    getColumnCount()
/    getRowCount()
/    getValueAt(int row, int col)
/    OPTIONAL METHOD  setValueAt(Object value, int row, int col)
/=================================================================================================*/
public boolean isCellEditable(int rowIndex, int vColIndex) {   
    boolean editable = editable_array[vColIndex];
    return editable;
}
/*================================================================================================
/        getColumnClass(int c)
/=================================================================================================*/
  public Class getColumnClass(int c) {
    java.lang.Class myClass = null;
     if(c == 0){
        myClass =  (new java.lang.String()).getClass(); //STATE
        return myClass;
     }
     if(c == 1){
        myClass =   ((Integer.valueOf(0)).getClass());//OBSERVATION_ID
        return myClass;
     }
     if(c == 2){
        myClass =   ((Integer.valueOf(0)).getClass());//TARGET_NUMBER
        return myClass;
     }
     if(c == 3){
        myClass =   ((Integer.valueOf(0)).getClass());//SEQUENCE_NUMBER
        return myClass;
     }
     if(c == 4){
        myClass =  (new java.lang.String()).getClass();//NAME
        return myClass;
     }
     if(c == 5){
        myClass =  (new java.lang.String()).getClass();//RA
        return myClass;
     }
     if(c == 6){
        myClass =  (new java.lang.String()).getClass();//DEC
        return myClass;
     }
     if(c == 7){
        myClass =  (Double.valueOf(0.0)).getClass();//OFFSET_RA
        return myClass;
     }    
      if(c == 8){
        myClass =  (Double.valueOf(0.0)).getClass();//OFFSET_DEC
        return myClass;
     }    
     if(c == 9){
        myClass =  (Double.valueOf(0.0)).getClass();//EXPTIME
        return myClass;
     }       
     if(c == 10){
        myClass =  (Double.valueOf(0.0)).getClass();//SLITW
        return myClass;
     }        
     if(c == 11){
        myClass =  (Double.valueOf(0.0)).getClass();//SLITOFFSET
        return myClass;
     }
     if(c == 12){
        myClass =   (new java.lang.String()).getClass();//CCDMODE
        return myClass;
     }
     if(c == 13){
        myClass =   ((Integer.valueOf(0)).getClass());//BINSPECT
        return myClass;
     }        
     if(c == 14){
        myClass =  ((Integer.valueOf(0)).getClass());//BINSPAT
        return myClass;
     }
     if(c == 15){
        myClass =  (new java.lang.String()).getClass();//CASSANGLE
        return myClass;
     }     
    return myClass;
  }
/*=========================================================================================================
/       End of the AstroObjectTableModel Class
/=========================================================================================================*/
}


