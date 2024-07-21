/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package edu.caltech.palomar.instruments.ngps.tables;

import edu.caltech.palomar.instruments.ngps.util.Reorderable;
import edu.caltech.palomar.instruments.ngps.object.Target;
import java.io.Serializable;
import java.util.Iterator;   
import java.util.Vector;
import javax.swing.event.TableModelEvent;
import javax.swing.table.AbstractTableModel;
/*
 * Copyright (c) 1997, 2013, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.  Oracle designates this
 * particular file as subject to the "Classpath" exception as provided
 * by Oracle in the LICENSE file that accompanied this code.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 */



import java.io.Serializable;
import java.util.Vector;
import javax.swing.event.TableModelEvent;
import javax.swing.table.AbstractTableModel;
import edu.caltech.palomar.instruments.ngps.object.Target;
import edu.caltech.palomar.instruments.ngps.tables.EditTableInterface;
import java.beans.PropertyChangeListener;
import java.beans.PropertyChangeSupport;
import java.util.Iterator;
import javax.swing.table.TableModel;
import edu.caltech.palomar.instruments.ngps.dbms.edit_monitor;


/**
 * This is an implementation of <code>TableModel</code> that
 * uses a <code>Vector</code> of <code>Vectors</code> to store the
 * cell value objects.
 * <p>
 * <strong>Warning:</strong> <code>DefaultTableModel</code> returns a
 * column class of <code>Object</code>.  When
 * <code>DefaultTableModel</code> is used with a
 * <code>TableRowSorter</code> this will result in extensive use of
 * <code>toString</code>, which for non-<code>String</code> data types
 * is expensive.  If you use <code>DefaultTableModel</code> with a
 * <code>TableRowSorter</code> you are strongly encouraged to override
 * <code>getColumnClass</code> to return the appropriate type.
 * <p>
 * <strong>Warning:</strong>
 * Serialized objects of this class will not be compatible with
 * future Swing releases. The current serialization support is
 * appropriate for short term storage or RMI between applications running
 * the same version of Swing.  As of 1.4, support for long term storage
 * of all JavaBeans&trade;
 * has been added to the <code>java.beans</code> package.
 * Please see {@link java.beans.XMLEncoder}.
 *
 * @author Philip Milne
 *
 * @see TableModel
 * @see #getDataVector
 */
public class MasterDBTableModel extends AbstractTableModel implements Reorderable, EditTableInterface, Serializable {

//
// Instance Variables
//

    /**
     * The <code>Vector</code> of <code>Vectors</code> of
     * <code>Object</code> values.
     */
    protected Vector    dataVector;

    /** The <code>Vector</code> of column identifiers. */
    protected Vector    columnIdentifiers;
    boolean[]           extended_editable_array     = new boolean[45];
    boolean[]           minimal_editable_array      = new boolean[15];
    public static int   EXTENDED_TABLE = 1;
    public static int   MINIMAL_TABLE  = 2;
    public        int   configuration;
    public edit_monitor my_edit_monitor;
//
// Constructors
//

    /**
     *  Constructs a default <code>DefaultTableModel</code>
     *  which is a table of zero columns and zero rows.
     */
    public MasterDBTableModel(int newConfiguration) {
        this(0, 0);
        configuration = newConfiguration;
        setEdited(false);
        if(configuration == EXTENDED_TABLE){
           initializeTableColumnsAll();
        }
        if(configuration == MINIMAL_TABLE){
           initializeTableColumnsMinimal();
        }
    }

    private static Vector newVector(int size) {
        Vector v = new Vector(size);
        v.setSize(size);
        return v;
    }
    private static Target newTarget(){
        Target v = new Target();
        return v;
    }
    /**
     *  Constructs a <code>DefaultTableModel</code> with
     *  <code>rowCount</code> and <code>columnCount</code> of
     *  <code>null</code> object values.
     *
     * @param rowCount           the number of rows the table holds
     * @param columnCount        the number of columns the table holds
     *
     * @see #setValueAt
     */
    public MasterDBTableModel(int rowCount, int columnCount) {
        this(newVector(columnCount), rowCount);
        setEdited(false);
    }

    /**
     *  Constructs a <code>DefaultTableModel</code> with as many columns
     *  as there are elements in <code>columnNames</code>
     *  and <code>rowCount</code> of <code>null</code>
     *  object values.  Each column's name will be taken from
     *  the <code>columnNames</code> vector.
     *
     * @param columnNames       <code>vector</code> containing the names
     *                          of the new columns; if this is
     *                          <code>null</code> then the model has no columns
     * @param rowCount           the number of rows the table holds
     * @see #setDataVector
     * @see #setValueAt
     */
    public MasterDBTableModel(Vector columnNames, int rowCount) {
        setDataVector(newVector(rowCount), columnNames);
        setEdited(false);
    }

    /**
     *  Constructs a <code>DefaultTableModel</code> with as many
     *  columns as there are elements in <code>columnNames</code>
     *  and <code>rowCount</code> of <code>null</code>
     *  object values.  Each column's name will be taken from
     *  the <code>columnNames</code> array.
     *
     * @param columnNames       <code>array</code> containing the names
     *                          of the new columns; if this is
     *                          <code>null</code> then the model has no columns
     * @param rowCount           the number of rows the table holds
     * @see #setDataVector
     * @see #setValueAt
     */
//    public DefaultTableModel2(Object[] columnNames, int rowCount) {
//        this(convertToVector(columnNames), rowCount);
//    }

    /**
     *  Constructs a <code>DefaultTableModel</code> and initializes the table
     *  by passing <code>data</code> and <code>columnNames</code>
     *  to the <code>setDataVector</code> method.
     *
     * @param data              the data of the table, a <code>Vector</code>
     *                          of <code>Vector</code>s of <code>Object</code>
     *                          values
     * @param columnNames       <code>vector</code> containing the names
     *                          of the new columns
     * @see #getDataVector
     * @see #setDataVector
     */
//    public DefaultTableModel2(Vector data, Vector columnNames) {
//        setDataVector(data, columnNames);
//    }
public void set_edit_monitor(edit_monitor new_edit_monitor){
   my_edit_monitor = new_edit_monitor; 
}
public void setEdited(boolean new_edited){
  if(my_edit_monitor != null){
      my_edit_monitor.setEdited(new_edited);
  }  
}
    /**
     *  Constructs a <code>DefaultTableModel</code> and initializes the table
     *  by passing <code>data</code> and <code>columnNames</code>
     *  to the <code>setDataVector</code>
     *  method. The first index in the <code>Object[][]</code> array is
     *  the row index and the second is the column index.
     *
     * @param data              the data of the table
     * @param columnNames       the names of the columns
     * @see #getDataVector
     * @see #setDataVector
     */
  public void setMode(int new_configuration){
      columnIdentifiers.clear();
      configuration = new_configuration;
        if(configuration == EXTENDED_TABLE){
           initializeTableColumnsAll();
        }
        if(configuration == MINIMAL_TABLE){
           initializeTableColumnsMinimal();
        }
     fireTableStructureChanged();
  }
    
    
  public void initializeTableColumnsMinimal(){
     addColumn("");
//     addColumn("OBS"+'\n'+"ORDER");
     addColumn("STATE");
     addColumn("NAME");
     addColumn("RA");
     addColumn("DEC");
     addColumn("EXPTIME\nRequest");
     addColumn("Exposure\nTime (s)"); //6
     addColumn("SLITWIDTH\nRequest");
     addColumn("Slit Width\n(arcsec)");//8
     addColumn("AIRMASS");
     addColumn("OTMSNR");//10
     addColumn("NOTE");
     minimal_editable_array[0] = true;
     minimal_editable_array[1] = true;
     minimal_editable_array[2] = true;
     minimal_editable_array[3] = true;
     minimal_editable_array[4] = true;
     minimal_editable_array[5] = true;
     minimal_editable_array[6] = false;
     minimal_editable_array[7] = true;
     minimal_editable_array[8] = false;
     minimal_editable_array[9] = false;
     minimal_editable_array[10] = false;
     minimal_editable_array[11] = true;
     minimal_editable_array[12] = true;
  }    
    
    
  public void initializeTableColumnsAll(){
     addColumn("ACTIVE");
     addColumn("OBSERVATION_ID");
     addColumn("STATE");
     addColumn("TARGET_NUMBER");
     addColumn("SEQUENCE NUMBER");
     addColumn("NAME");
     addColumn("OBS_ORDER");
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
     addColumn("AIRMASS_MAX");
     addColumn("WRANGE_START");
     addColumn("WRANGE_END");
     addColumn("CHANNEL");
     addColumn("MAGNITUDE");
     addColumn("MAGSYSTEM");
     addColumn("MAGFILTER");
     addColumn("SRCMODEL");
     addColumn("OTMexpt");
     addColumn("OTMslit");
     addColumn("OTMcass");
     addColumn("OTMairmass_start");
     addColumn("OTMairmass_end");
     addColumn("SKYMAG");
     addColumn("OTMslewgo");
     addColumn("OTMstart");
     addColumn("OTMend");
     addColumn("OTMpa");
     addColumn("OTMwait");
     addColumn("OTMflag");
     addColumn("OTMlast");
     addColumn("OTMslew");
     addColumn("OTMmoon");
     addColumn("OTMslitangle");    
     addColumn("OTMSNR");
     addColumn("NOTE");
     addColumn("COMMENT");   
     extended_editable_array[0] = true;
     extended_editable_array[1] = false;
     extended_editable_array[2] = true;
     extended_editable_array[3] = true;
     extended_editable_array[4] = true;
     extended_editable_array[5] = true;
     extended_editable_array[6] = true;
     extended_editable_array[7] = true;
     extended_editable_array[8] = true;
     extended_editable_array[9] = true;
     extended_editable_array[10] = true;
     extended_editable_array[11] = true;
     extended_editable_array[12] = true;
     extended_editable_array[13] = true;
     extended_editable_array[14] = true;
     extended_editable_array[15] = true;
     extended_editable_array[16] = true;
     extended_editable_array[17] = true;
     extended_editable_array[18] = true;
     extended_editable_array[19] = true;
     extended_editable_array[20] = true;
     extended_editable_array[21] = true;
     extended_editable_array[22] = true;  
     extended_editable_array[23] = true;  
     extended_editable_array[24] = true;  
     extended_editable_array[25] = false;  
     extended_editable_array[26] = false;  
     extended_editable_array[27] = false;  
     extended_editable_array[28] = false;  
     extended_editable_array[29] = false;  
     extended_editable_array[30] = false;  
     extended_editable_array[31] = false;  
     extended_editable_array[32] = false;  
     extended_editable_array[33] = false;  
     extended_editable_array[34] = false;  
     extended_editable_array[35] = false;  
     extended_editable_array[36] = false;  
     extended_editable_array[37] = false;  
     extended_editable_array[38] = false;  
     extended_editable_array[39] = false;  
     extended_editable_array[40] = false;  
     extended_editable_array[41] = false;  
     extended_editable_array[42] = false;  
     extended_editable_array[43] = true;  
     extended_editable_array[44] = true;  
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
    int count = fromIndex.length;
    moveRow(fromIndex[0], fromIndex[count-1], toIndex);    
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
        Target my_target = (Target)it.next();
        my_target.setOrder(count);
        count = count+1;
     }
 }
/*================================================================================================
/         clearTable()
/=================================================================================================*/
 public void clearTable(){
     getDataVector().clear();
     setRowCount(0);
     fireTableDataChanged();
 }
/*================================================================================================
/      addTarget(Target current)
/=================================================================================================*/
 public void addTarget(Target current){
     int size = getDataVector().size();
     current.setOrder(size);
     current.setIndex(size);
     addRow(current);
 }
/*================================================================================================
/     insert(int index,Target current)
/=================================================================================================*/
 public void insert(int index,Target current){
    int next_index = getDataVector().size();
    current.setIndex(next_index);
    insertRow(index,current);
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
/         getRecord(int recordNumber)
/=================================================================================================*/
  public synchronized Target getRecord(int recordNumber) {
     Target current = (Target)getDataVector().get(recordNumber);
    return current;
  }
    /**
     *  Returns the <code>Vector</code> of <code>Vectors</code>
     *  that contains the table's
     *  data values.  The vectors contained in the outer vector are
     *  each a single row of values.  In other words, to get to the cell
     *  at row 1, column 5: <p>
     *
     *  <code>((Vector)getDataVector().elementAt(1)).elementAt(5);</code>
     *
     * @return  the vector of vectors containing the tables data values
     *
     * @see #newDataAvailable
     * @see #newRowsAdded
     * @see #setDataVector
     */
    public Vector getDataVector() {
        return dataVector;
    }

    private static Vector nonNullVector(Vector v) {
        return (v != null) ? v : new Vector();
    }

    /**
     *  Replaces the current <code>dataVector</code> instance variable
     *  with the new <code>Vector</code> of rows, <code>dataVector</code>.
     *  Each row is represented in <code>dataVector</code> as a
     *  <code>Vector</code> of <code>Object</code> values.
     *  <code>columnIdentifiers</code> are the names of the new
     *  columns.  The first name in <code>columnIdentifiers</code> is
     *  mapped to column 0 in <code>dataVector</code>. Each row in
     *  <code>dataVector</code> is adjusted to match the number of
     *  columns in <code>columnIdentifiers</code>
     *  either by truncating the <code>Vector</code> if it is too long,
     *  or adding <code>null</code> values if it is too short.
     *  <p>Note that passing in a <code>null</code> value for
     *  <code>dataVector</code> results in unspecified behavior,
     *  an possibly an exception.
     *
     * @param   dataVector         the new data vector
     * @param   columnIdentifiers     the names of the columns
     * @see #getDataVector
     */
    public void setDataVector(Vector dataVector, Vector columnIdentifiers) {
        this.dataVector = nonNullVector(dataVector);
        this.columnIdentifiers = nonNullVector(columnIdentifiers);
        justifyRows(0, getRowCount());
        fireTableStructureChanged();
    }
  
    /**
     *  Replaces the value in the <code>dataVector</code> instance
     *  variable with the values in the array <code>dataVector</code>.
     *  The first index in the <code>Object[][]</code>
     *  array is the row index and the second is the column index.
     *  <code>columnIdentifiers</code> are the names of the new columns.
     *
     * @param dataVector                the new data vector
     * @param columnIdentifiers the names of the columns
     * @see #setDataVector(Vector, Vector)
     */
//    public void setDataVector(Object[][] dataVector, Object[] columnIdentifiers) {
//        setDataVector(convertToVector(dataVector), convertToVector(columnIdentifiers));
//    }

    /**
     *  Equivalent to <code>fireTableChanged</code>.
     *
     * @param event  the change event
     *
     */
    public void newDataAvailable(TableModelEvent event) {
        fireTableChanged(event);
    }

//
// Manipulating rows
//

    private void justifyRows(int from, int to) {
        // Sometimes the DefaultTableModel is subclassed
        // instead of the AbstractTableModel by mistake.
        // Set the number of rows for the case when getRowCount
        // is overridden.
        dataVector.setSize(getRowCount());

        for (int i = from; i < to; i++) {
            if (dataVector.elementAt(i) == null) {
                dataVector.setElementAt(new Vector(), i);
            }
//            ((Vector)dataVector.elementAt(i)).setSize(getColumnCount());
        }
    }

    /**
     *  Ensures that the new rows have the correct number of columns.
     *  This is accomplished by  using the <code>setSize</code> method in
     *  <code>Vector</code> which truncates vectors
     *  which are too long, and appends <code>null</code>s if they
     *  are too short.
     *  This method also sends out a <code>tableChanged</code>
     *  notification message to all the listeners.
     *
     * @param e         this <code>TableModelEvent</code> describes
     *                           where the rows were added.
     *                           If <code>null</code> it assumes
     *                           all the rows were newly added
     * @see #getDataVector
     */
    public void newRowsAdded(TableModelEvent e) {
        justifyRows(e.getFirstRow(), e.getLastRow() + 1);
        fireTableChanged(e);
        setEdited(true); 
    }

    /**
     *  Equivalent to <code>fireTableChanged</code>.
     *
     *  @param event the change event
     *
     */
    public void rowsRemoved(TableModelEvent event) {
        fireTableChanged(event);
        setEdited(true); 
    }

    /**
     * Obsolete as of Java 2 platform v1.3.  Please use <code>setRowCount</code> instead.
     */
    /*
     *  Sets the number of rows in the model.  If the new size is greater
     *  than the current size, new rows are added to the end of the model
     *  If the new size is less than the current size, all
     *  rows at index <code>rowCount</code> and greater are discarded.
     *
     * @param   rowCount   the new number of rows
     * @see #setRowCount
     */
    public void setNumRows(int rowCount) {
        int old = getRowCount();
        if (old == rowCount) {
            return;
        }
        dataVector.setSize(rowCount);
        if (rowCount <= old) {
            fireTableRowsDeleted(rowCount, old-1);
        }
        else {
            justifyRows(old, rowCount);
            fireTableRowsInserted(old, rowCount-1);
        }
    }

    /**
     *  Sets the number of rows in the model.  If the new size is greater
     *  than the current size, new rows are added to the end of the model
     *  If the new size is less than the current size, all
     *  rows at index <code>rowCount</code> and greater are discarded.
     *
     *  @see #setColumnCount
     * @since 1.3
     */
    public void setRowCount(int rowCount) {
        setNumRows(rowCount);
    }

    /**
     *  Adds a row to the end of the model.  The new row will contain
     *  <code>null</code> values unless <code>rowData</code> is specified.
     *  Notification of the row being added will be generated.
     *
     * @param   rowData          optional data of the row being added
     */
    public void addRow(Target rowData) {
        insertRow(getRowCount(), rowData);
        setEdited(true); 
    }

    /**
     *  Adds a row to the end of the model.  The new row will contain
     *  <code>null</code> values unless <code>rowData</code> is specified.
     *  Notification of the row being added will be generated.
     *
     * @param   rowData          optional data of the row being added
     */
//    public void addRow(Object[] rowData) {
//        addRow(convertToVector(rowData));
//    }

    /**
     *  Inserts a row at <code>row</code> in the model.  The new row
     *  will contain <code>null</code> values unless <code>rowData</code>
     *  is specified.  Notification of the row being added will be generated.
     *
     * @param   row             the row index of the row to be inserted
     * @param   rowData         optional data of the row being added
     * @exception  ArrayIndexOutOfBoundsException  if the row was invalid
     */
    public void insertRow(int row, Target rowData) {
        dataVector.insertElementAt(rowData, row);
        justifyRows(row, row+1);
        fireTableRowsInserted(row, row);
        setEdited(true); 
    }

    /**
     *  Inserts a row at <code>row</code> in the model.  The new row
     *  will contain <code>null</code> values unless <code>rowData</code>
     *  is specified.  Notification of the row being added will be generated.
     *
     * @param   row      the row index of the row to be inserted
     * @param   rowData          optional data of the row being added
     * @exception  ArrayIndexOutOfBoundsException  if the row was invalid
     */
//    public void insertRow(int row, Object[] rowData) {
//        insertRow(row, convertToVector(rowData));
 //   }

    private static int gcd(int i, int j) {
        return (j == 0) ? i : gcd(j, i%j);
    }

    private static void rotate(Vector v, int a, int b, int shift) {
        int size = b - a;
        int r = size - shift;
        int g = gcd(size, r);
        for(int i = 0; i < g; i++) {
            int to = i;
            Object tmp = v.elementAt(a + to);
            for(int from = (to + r) % size; from != i; from = (to + r) % size) {
                v.setElementAt(v.elementAt(a + from), a + to);
                to = from;
            }
            v.setElementAt(tmp, a + to);
        }
    }

    /**
     *  Moves one or more rows from the inclusive range <code>start</code> to
     *  <code>end</code> to the <code>to</code> position in the model.
     *  After the move, the row that was at index <code>start</code>
     *  will be at index <code>to</code>.
     *  This method will send a <code>tableChanged</code> notification
       message to all the listeners.
     *
     *  <pre>
     *  Examples of moves:
     *
     *  1. moveRow(1,3,5);
     *          a|B|C|D|e|f|g|h|i|j|k   - before
     *          a|e|f|g|h|B|C|D|i|j|k   - after
     *
     *  2. moveRow(6,7,1);
     *          a|b|c|d|e|f|G|H|i|j|k   - before
     *          a|G|H|b|c|d|e|f|i|j|k   - after
     *  </pre>
     *
     * @param   start       the starting row index to be moved
     * @param   end         the ending row index to be moved
     * @param   to          the destination of the rows to be moved
     * @exception  ArrayIndexOutOfBoundsException  if any of the elements
     * would be moved out of the table's range
     *
     */
    public void moveRow(int start, int end, int to) {
        int shift = to - start;
        int first, last;
        if (shift < 0) {
            first = to;
            last = end;
        }
        else {
            first = start;
            last = to + end - start;
        }
        System.out.println("Start = "+start+" End = "+end+" To = "+to);
        int length = dataVector.size();
        if(to < length){
            System.out.println("CASE 1 Start = "+start+" End = "+end+" To = "+to);
           rotate(dataVector, first, last + 1, shift); 
        }
        if(to >= length){
           System.out.println("CASE 2 Start = "+start+" End = "+end+" To = "+to); 
//           rotate(dataVector, first, last + 1, shift); 
        }
        fireTableRowsUpdated(first, last);
        setEdited(true); 
    }

    /**
     *  Removes the row at <code>row</code> from the model.  Notification
     *  of the row being removed will be sent to all the listeners.
     *
     * @param   row      the row index of the row to be removed
     * @exception  ArrayIndexOutOfBoundsException  if the row was invalid
     */
    public void removeRow(int row) {
        dataVector.removeElementAt(row);
        fireTableRowsDeleted(row, row);
         setEdited(true); 
   }

//
// Manipulating columns
//

    /**
     * Replaces the column identifiers in the model.  If the number of
     * <code>newIdentifier</code>s is greater than the current number
     * of columns, new columns are added to the end of each row in the model.
     * If the number of <code>newIdentifier</code>s is less than the current
     * number of columns, all the extra columns at the end of a row are
     * discarded.
     *
     * @param   columnIdentifiers  vector of column identifiers.  If
     *                          <code>null</code>, set the model
     *                          to zero columns
     * @see #setNumRows
     */
    public void setColumnIdentifiers(Vector columnIdentifiers) {
        setDataVector(dataVector, columnIdentifiers);
    }

    /**
     * Replaces the column identifiers in the model.  If the number of
     * <code>newIdentifier</code>s is greater than the current number
     * of columns, new columns are added to the end of each row in the model.
     * If the number of <code>newIdentifier</code>s is less than the current
     * number of columns, all the extra columns at the end of a row are
     * discarded.
     *
     * @param   newIdentifiers  array of column identifiers.
     *                          If <code>null</code>, set
     *                          the model to zero columns
     * @see #setNumRows
     */
    public void setColumnIdentifiers(Object[] newIdentifiers) {
        setColumnIdentifiers(convertToVector(newIdentifiers));
    }

    /**
     *  Sets the number of columns in the model.  If the new size is greater
     *  than the current size, new columns are added to the end of the model
     *  with <code>null</code> cell values.
     *  If the new size is less than the current size, all columns at index
     *  <code>columnCount</code> and greater are discarded.
     *
     *  @param columnCount  the new number of columns in the model
     *
     *  @see #setColumnCount
     * @since 1.3
     */
    public void setColumnCount(int columnCount) {
        columnIdentifiers.setSize(columnCount);
        justifyRows(0, getRowCount());
        fireTableStructureChanged();
    }

    /**
     *  Adds a column to the model.  The new column will have the
     *  identifier <code>columnName</code>, which may be null.  This method
     *  will send a
     *  <code>tableChanged</code> notification message to all the listeners.
     *  This method is a cover for <code>addColumn(Object, Vector)</code> which
     *  uses <code>null</code> as the data vector.
     *
     * @param   columnName the identifier of the column being added
     */
    public void addColumn(Object columnName) {
        columnIdentifiers.addElement(columnName);
    }

    /**
     *  Adds a column to the model.  The new column will have the
     *  identifier <code>columnName</code>, which may be null.
     *  <code>columnData</code> is the
     *  optional vector of data for the column.  If it is <code>null</code>
     *  the column is filled with <code>null</code> values.  Otherwise,
     *  the new data will be added to model starting with the first
     *  element going to row 0, etc.  This method will send a
     *  <code>tableChanged</code> notification message to all the listeners.
     *
     * @param   columnName the identifier of the column being added
     * @param   columnData       optional data of the column being added
     */
/*    public void addColumn(Object columnName, Vector columnData) {
        columnIdentifiers.addElement(columnName);
        if (columnData != null) {
            int columnSize = columnData.size();
            if (columnSize > getRowCount()) {
                dataVector.setSize(columnSize);
            }
            justifyRows(0, getRowCount());
            int newColumn = getColumnCount() - 1;
            for(int i = 0; i < columnSize; i++) {
                  Vector row = (Vector)dataVector.elementAt(i);
                  row.setElementAt(columnData.elementAt(i), newColumn);
            }
        }
        else {
            justifyRows(0, getRowCount());
        }
        fireTableStructureChanged();
    }*/

    /**
     *  Adds a column to the model.  The new column will have the
     *  identifier <code>columnName</code>.  <code>columnData</code> is the
     *  optional array of data for the column.  If it is <code>null</code>
     *  the column is filled with <code>null</code> values.  Otherwise,
     *  the new data will be added to model starting with the first
     *  element going to row 0, etc.  This method will send a
     *  <code>tableChanged</code> notification message to all the listeners.
     *
     * @see #addColumn(Object, Vector)
     */
//    public void addColumn(Object columnName, Object[] columnData) {
//        addColumn(columnName, convertToVector(columnData));
//    }

//
// Implementing the TableModel interface
//

    /**
     * Returns the number of rows in this data table.
     * @return the number of rows in the model
     */
    public int getRowCount() {
        return dataVector.size();
    }

    /**
     * Returns the number of columns in this data table.
     * @return the number of columns in the model
     */
    public int getColumnCount() {
        return columnIdentifiers.size();
    }

    /**
     * Returns the column name.
     *
     * @return a name for this column using the string value of the
     * appropriate member in <code>columnIdentifiers</code>.
     * If <code>columnIdentifiers</code> does not have an entry
     * for this index, returns the default
     * name provided by the superclass.
     */
    public String getColumnName(int column) {
        Object id = null;
        // This test is to cover the case when
        // getColumnCount has been subclassed by mistake ...
        if (column < columnIdentifiers.size() && (column >= 0)) {
            id = columnIdentifiers.elementAt(column);
        }
        return (id == null) ? super.getColumnName(column)
                            : id.toString();
    }

    /**
     * Returns true regardless of parameter values.
     *
     * @param   row             the row whose value is to be queried
     * @param   column          the column whose value is to be queried
     * @return                  true
     * @see #setValueAt
     */
public boolean isCellEditable(int rowIndex, int vColIndex) {   
boolean editable = false;
    if(configuration == EXTENDED_TABLE){
         editable = extended_editable_array[vColIndex];   
         Target selectedRow = (Target)dataVector.elementAt(rowIndex);
         java.lang.String state = selectedRow.getSTATE();
         if(state.matches("COMPLETED")){
            editable = false;
         }
         if(state.matches("EXPOSING")){
            editable = false;
         }
         if(state.matches("INACTIVE")){
            editable = minimal_editable_array[vColIndex];
         }
        if(state.matches("PENDING")){
            editable = extended_editable_array[vColIndex];  
        }      
    }
    if(configuration == MINIMAL_TABLE){
        editable = minimal_editable_array[vColIndex];   
         Target selectedRow = (Target)dataVector.elementAt(rowIndex);
         java.lang.String state = selectedRow.getSTATE();
         if(state.matches("COMPLETED")){
            editable = false;
         }
         if(state.matches("EXPOSING")){
            editable = false;
         }
         if(state.matches("INACTIVE")){
            editable = minimal_editable_array[vColIndex];
         }
        if(state.matches("PENDING")){
            editable = minimal_editable_array[vColIndex];  
        }            
    }
    return editable;
}

    /**
     * Returns an attribute value for the cell at <code>row</code>
     * and <code>column</code>.
     *
     * @param   row             the row whose value is to be queried
     * @param   column          the column whose value is to be queried
     * @return                  the value Object at the specified cell
     * @exception  ArrayIndexOutOfBoundsException  if an invalid row or
     *               column was given
     */
//    public Object getValueAt(int row, int column) {
 //       Vector rowVector = (Vector)dataVector.elementAt(row);
//        return rowVector.elementAt(column);
//    }

    /**
     * Sets the object value for the cell at <code>column</code> and
     * <code>row</code>.  <code>aValue</code> is the new value.  This method
     * will generate a <code>tableChanged</code> notification.
     *
     * @param   aValue          the new value; this can be null
     * @param   row             the row whose value is to be changed
     * @param   column          the column whose value is to be changed
     * @exception  ArrayIndexOutOfBoundsException  if an invalid row or
     *               column was given
     */
//    public void setValueAt(Object aValue, int row, int column) {
//        Vector rowVector = (Vector)dataVector.elementAt(row);
//        rowVector.setElementAt(aValue, column);
//        fireTableCellUpdated(row, column);
//    }
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
/         getValueAt() - Required Method for the Abstract Table Model Class
/=================================================================================================*/
  public Object getValueAt(int row, int col) {
    java.lang.Object returnObject = null;
    Target selectedRow = (Target)dataVector.elementAt(row);
//    Target selectedTarget = vector_to_target(row,getDataVector().get(row));  
    if(configuration == EXTENDED_TABLE){
        if(col == 0){
            returnObject = selectedRow.isActive();
        }
//        if(col == 1){
//           returnObject = selectedRow.getObservationID();
//        }
        if(col == 1){
           returnObject = selectedRow.getSTATE();
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
           returnObject = selectedRow.getOrder();
        }    
        if(col == 6){
           returnObject = selectedRow.sky.getRightAscension();
        }
        if(col == 7){
           returnObject = selectedRow.sky.getDeclination();
        }
        if(col == 8){
           returnObject = selectedRow.sky.getOFFSET_RA();
        }
        if(col == 9){
           returnObject = selectedRow.sky.getOFFSET_DEC();
        }     
        if(col == 10){
           returnObject = selectedRow.instrument.getExposuretime();
        }
        if(col == 11){
           returnObject = selectedRow.instrument.getSlitwidth_string();
        }
        if(col == 12){
           returnObject = selectedRow.instrument.getSlitOffset();
        }
        if(col == 13){
           returnObject = selectedRow.instrument.getOBSMODE();
        }
        if(col == 14){
           returnObject = selectedRow.instrument.getBIN_SPEC();
        }
        if(col == 15){
           returnObject = selectedRow.instrument.getBIN_SPACE();
        }
        if(col == 16){
           returnObject = selectedRow.instrument.getRequestedSlitAngle();
        }
        if(col == 17){
           returnObject = selectedRow.etc.getWRANGE_LOW();
        } 
        if(col == 18){
           returnObject = selectedRow.etc.getWRANGE_HIGH();
        } 
        if(col == 19){
           returnObject = selectedRow.etc.getChannel();
        } 
        if(col == 20){
           returnObject = selectedRow.etc.getMagnitude();
        } 
        if(col == 21){
           returnObject = selectedRow.etc.getMagref_system();
        }  
        if(col == 22){
           returnObject = selectedRow.etc.getMagref_filter();
        }   
        if(col == 23){
           returnObject = selectedRow.etc.getSrcmodel();
        } 
        if(col == 24){
           returnObject = selectedRow.otm.getOTMexpt();
        } 
        if(col == 25){
           returnObject = selectedRow.otm.getOTMslitwidth();
        } 
        if(col == 26){
           returnObject = selectedRow.otm.getOTMcass();
        } 
        if(col == 27){
            returnObject = selectedRow.otm.getOTMSNR();
        }
        if(col == 28){
           returnObject = selectedRow.otm.getOTMAirmass_start();
        } 
        if(col == 29){
           returnObject = selectedRow.otm.getOTMAirmass_end();
        } 
        if(col == 30){
           returnObject = selectedRow.otm.getSkymag();
        } 
        if(col == 31){
           returnObject = selectedRow.otm.getOTMslewgo();
        } 
        if(col == 32){
           returnObject = selectedRow.otm.getOTMstart();
        }    
        if(col == 33){
           returnObject = selectedRow.otm.getOTMend();
        } 
        if(col == 34){
           returnObject = selectedRow.otm.getOTMdPadt();
        }
        if(col == 35){
           returnObject = selectedRow.otm.getOTMwait();
        }
        if(col == 36){
           returnObject = selectedRow.otm.getOTMflag();
        }   
        if(col == 37){
           returnObject = selectedRow.otm.getOTMlast();
        } 
        if(col == 38){
           returnObject = selectedRow.otm.getOTMslew();
        }     
        if(col == 39){
           returnObject = selectedRow.otm.getOTMmoon();
        }        
        if(col == 40){
           returnObject = selectedRow.otm.getOTMslitangle();
        } 
        if(col == 41){
           returnObject = selectedRow.otm.getOTMSNR();
        }  
        if(col == 42){
           returnObject = selectedRow.getNOTE();
        }        
        if(col == 43){
           returnObject = selectedRow.getCOMMENT();
        }        
    }
    if(configuration == MINIMAL_TABLE){
        if(col == 0){
            returnObject = selectedRow.isActive();
        }
//        if(col == 1){
//           returnObject = selectedRow.getOrder();
//        }    
        if(col == 1){
           returnObject = selectedRow.getSTATE();
        }
        if(col == 2){
           returnObject = selectedRow.name;
        }
        if(col == 3){
           returnObject = selectedRow.sky.getRightAscension();
        }
        if(col == 4){
           returnObject = selectedRow.sky.getDeclination();
        }
        if(col == 5){
           returnObject = selectedRow.instrument.getExposuretime();
        }
        if(col == 6){
           returnObject = selectedRow.otm.getOTMexpt();
        } 
        if(col == 7){
           returnObject = selectedRow.instrument.getSlitwidth_string();
        }
        if(col == 8){
           returnObject = selectedRow.otm.getOTMslitwidth();
        } 
        if(col == 9){
           returnObject = selectedRow.otm.getOTMAirmass_start();
        } 
        if(col == 10){
           returnObject = selectedRow.otm.getOTMSNR();
        }         
        if(col == 11){
           returnObject = selectedRow.getNOTE();
        }
    }
    return returnObject;
  } 
/*================================================================================================
/     setValueAt(Object value, int row, int col)
/=================================================================================================*/
 public void setValueAt(Object value, int row, int col) {
//   Target selectedTarget = vector_to_target(row,getDataVector().get(row));  
   Target selectedTarget = (Target)getDataVector().get(row);  
    if(configuration == EXTENDED_TABLE){
        if(col == 0){
            selectedTarget.setActive(((java.lang.Boolean)value).booleanValue());
        }
//        if(col == 1){
//           selectedTarget.setObservationID(((java.lang.Integer)value).intValue()); 
//        }
        if(col == 1){
           selectedTarget.setSTATE(((java.lang.String)value)); 
        }
        if(col == 2){
           selectedTarget.setTarget_Number(((java.lang.Integer)value).intValue()); 
        }    
        if(col == 3){
           selectedTarget.setSequence_Number(((java.lang.Integer)value).intValue()); 
        }   
        if(col == 4){
           selectedTarget.name = ((java.lang.String)value);
        }
        if(col == 5){
           selectedTarget.setOrder(((java.lang.Integer)value).intValue()); 
        }
        if(col == 6){
           selectedTarget.sky.setRightAscension((java.lang.String)value);
        }
        if(col == 7){
           selectedTarget.sky.setDeclination(((java.lang.String)value));
        }
        if(col == 8){
           selectedTarget.sky.setOFFSET_RA(((java.lang.Double)value).doubleValue());
        }    
        if(col == 9){
           selectedTarget.sky.setOFFSET_DEC(((java.lang.Double)value).doubleValue());
        }    
        if(col == 10){
           selectedTarget.instrument.setExposuretime(((java.lang.String)value));
        }
        if(col == 11){
           selectedTarget.instrument.setSlitwidth_string(((java.lang.String)value));
        }
        if(col == 12){
           selectedTarget.instrument.setSlitOffset(((java.lang.Double)value).doubleValue());
        }
        if(col == 13){
           selectedTarget.instrument.setOBSMODE(((java.lang.String)value));
        }
        if(col == 14){
           selectedTarget.instrument.setBIN_SPEC(((java.lang.Integer)value).intValue());
        }
        if(col == 15){
           selectedTarget.instrument.setBIN_SPACE(((java.lang.Integer)value).intValue());
        }  
        if(col == 16){
           selectedTarget.instrument.setRequestedSlitAngle(((java.lang.String)value));
        } 
        if(col == 17){
            selectedTarget.sky.setAIRMASS_MAX((java.lang.String)value);
        }
        if(col == 18){
           selectedTarget.etc.setWRANGE_LOW(((java.lang.Integer)value).intValue());
        }      
        if(col == 19){
           selectedTarget.etc.setWRANGE_HIGH(((java.lang.Integer)value).intValue());
        }     
        if(col == 20){
           selectedTarget.etc.setChannel(((java.lang.String)value));
        }     
        if(col == 21){
           selectedTarget.etc.setMagnitude(((java.lang.Double)value).doubleValue());
        }     
        if(col == 22){
           selectedTarget.etc.setMagref_system(((java.lang.String)value));
        } 
        if(col == 23){
           selectedTarget.etc.setMagref_filter(((java.lang.String)value));
        } 
        if(col == 24){
           selectedTarget.etc.setSrcmodel(((java.lang.String)value));
        } 
        if(col == 25){
           selectedTarget.otm.setOTMexpt(((java.lang.Double)value).doubleValue());
        } 
        if(col == 26){
           selectedTarget.otm.setOTMslitwidth(((java.lang.Double)value).doubleValue());
        }     
        if(col == 27){
           selectedTarget.otm.setOTMcass(((java.lang.Double)value).doubleValue());
        }
        if(col == 28){
           selectedTarget.otm.setOTMAirmass_start(((java.lang.Double)value).doubleValue());
        }      
        if(col == 29){
           selectedTarget.otm.setOTMAirmass_end(((java.lang.Double)value).doubleValue());
        }      
        if(col == 30){
           selectedTarget.otm.setSkymag(((java.lang.Double)value).doubleValue());
        }      
        if(col == 31){        
           selectedTarget.otm.setOTMslewgo(string_to_timestamp((java.lang.String)value));
        }    
        if(col == 32){
           selectedTarget.otm.setOTMstart(string_to_timestamp((java.lang.String)value));
        }  
        if(col == 33){
           selectedTarget.otm.setOTMend(string_to_timestamp((java.lang.String)value));
        }         
        if(col == 34){
           selectedTarget.otm.setOTMpa(((java.lang.Double)value).doubleValue());
        }
        if(col == 35){
           selectedTarget.otm.setOTMwait(((java.lang.Double)value).doubleValue());
        }
        if(col == 36){
           selectedTarget.otm.setOTMflag(((java.lang.String)value));
        }
        if(col == 37){
           selectedTarget.otm.setOTMlast(((java.lang.String)value));
        }
        if(col == 38){
           selectedTarget.otm.setOTMslew(((java.lang.Double)value).doubleValue());
        }   
        if(col == 39){
           selectedTarget.otm.setOTMmoon(((java.lang.String)value));
        }  
        if(col == 40){
           selectedTarget.otm.setOTMslitangle(((java.lang.Double)value));
        } 
        if(col == 41){
           selectedTarget.otm.setOTMSNR(((java.lang.String)value));
        }  
        if(col == 42){
           selectedTarget.setNOTE(((java.lang.String)value));
        }  
        if(col == 43){
           selectedTarget.setCOMMENT(((java.lang.String)value));
        }  
    }    
    if(configuration == MINIMAL_TABLE){
        if(col == 0){
            selectedTarget.setActive(((java.lang.Boolean)value).booleanValue());
        }
//        if(col == 1){
//           selectedTarget.setOrder(((java.lang.Integer)value).intValue()); 
//       }
        if(col == 1){
           selectedTarget.setSTATE(((java.lang.String)value)); 
        }
        if(col == 2){
           selectedTarget.name = ((java.lang.String)value);
        }
        if(col == 3){
           selectedTarget.sky.setRightAscension((java.lang.String)value);
        }
        if(col == 4){
           selectedTarget.sky.setDeclination(((java.lang.String)value));
        }
        if(col == 5){
           selectedTarget.instrument.setExposuretime(((java.lang.String)value));
        }
        if(col == 6){
           selectedTarget.otm.setOTMexpt(((java.lang.Double)value).doubleValue());
        }  
        if(col == 7){
           selectedTarget.instrument.setSlitwidth_string(((java.lang.String)value));
        }
        if(col == 8){
           selectedTarget.otm.setOTMslitwidth(((java.lang.Double)value).doubleValue());
        }     
        if(col == 9){
           selectedTarget.otm.setOTMAirmass_start(((java.lang.Double)value).doubleValue());
        }      
        if(col == 10){
           selectedTarget.otm.setOTMSNR(((java.lang.String)value));
        }      
        if(col == 11){
           selectedTarget.setNOTE(((java.lang.String)value));
        }
    }
    getDataVector().setElementAt(selectedTarget,row);
    fireTableCellUpdated(row, col);
    fireTableDataChanged();   
    setEdited(true); 
   }
/*================================================================================================
/        getColumnClass(int c)
/=================================================================================================*/
  public Class getColumnClass(int c) {
    java.lang.Class myClass = null;
    myClass =  (new java.lang.String()).getClass();
    if(configuration == EXTENDED_TABLE){
        if(c == 0){
            myClass =   ((Boolean.valueOf("true")).getClass()); //OBSERVATION_ID
            return myClass;
         }
//        if(c == 1){
//            myClass =   ((new Integer(0)).getClass()); //OBSERVATION_ID
//            return myClass;
//         }
        if(c == 1){
            myClass =  (new java.lang.String()).getClass(); // STATE
            return myClass;
         }
         if(c == 2){
            myClass =   ((Integer.valueOf(0)).getClass()); // TARGET_NUMBER
            return myClass;
         }
         if(c == 3){
            myClass =   ((Integer.valueOf(0)).getClass()); // SEQUENCE_NUMBER
            return myClass;
         }     
         if(c == 4){
            myClass =  (new java.lang.String()).getClass(); // NAME
            return myClass;
         }
         if(c == 5){
            myClass =  ((Integer.valueOf(0)).getClass()); // OBS_ORDER
            return myClass;
         }
          if(c == 6){
            myClass =  (new java.lang.String()).getClass(); // RA
            return myClass;
         }    
          if(c == 7){
            myClass =  (new java.lang.String()).getClass(); // DEC
            return myClass;
         }          
         if(c == 8){
            myClass =  (Double.valueOf(0.0)).getClass(); // OFFSET_RA
            return myClass;
         }
         if(c == 9){
            myClass =  (Double.valueOf(0.0)).getClass(); // OFFSET_DEC
            return myClass;
         }
//         if(c ==10){
//            myClass =  (new java.lang.String()).getClass(); //EPOCH
//            return myClass;
//         }
         if(c == 10){
            myClass =  (new java.lang.String()).getClass(); // EXPTIME
            return myClass;
         }
         if(c == 11){
            myClass =  (new java.lang.String()).getClass(); // SLIT WIDTH
            return myClass;
         }
         if(c == 12){
            myClass =  (Double.valueOf(0.0)).getClass(); // SLIT OFFSET
            return myClass;
         }
         if(c == 13){
            myClass =  (new java.lang.String()).getClass(); // OBS_MODE
            return myClass;
         }
         if(c == 14){
            myClass =  (Integer.valueOf(0)).getClass(); // BIN_SPEC
            return myClass;
         }
         if(c == 15){
            myClass =  (Integer.valueOf(0)).getClass(); // BIN_SPACE
            return myClass;
         }
         if(c == 16){
            myClass =  (new java.lang.String()).getClass(); // CASS_RING_ANGLE
            return myClass;
         }
         if(c == 17){
            myClass =  (new String()).getClass(); //WRANGE_LOW
            return myClass;
         }          
         if(c == 18){
            myClass =  (Integer.valueOf(0)).getClass(); //WRANGE_LOW
            return myClass;
         }
         if(c == 19){
             myClass =  (Integer.valueOf(0)).getClass(); // WRANGE_HIGH
            return myClass;
         }   
         if(c == 20){
             myClass =  (new java.lang.String()).getClass(); // CHANNEL
            return myClass;
         }     
         if(c == 21){
             myClass =  (Double.valueOf(0.0)).getClass(); // MAGNITUDE
            return myClass;
         }     
         if(c == 22){
             myClass =  (new java.lang.String()).getClass();   //Magref_system
            return myClass;
         }     
         if(c == 23){
             myClass =  (new java.lang.String()).getClass();  //Magref_filter
            return myClass;
         }     
         if(c == 24){
             myClass =  (new java.lang.String()).getClass(); // SRC_MODEL
         } 
         if(c == 25){
             myClass =  (Double.valueOf(0)).getClass();  // OTMexpt
            return myClass;
         }        
        if(c == 26){
             myClass =  (Double.valueOf(0)).getClass(); // OTMslit
            return myClass;
         }
        if(c == 27){
           myClass =  (Double.valueOf(0)).getClass(); // OTMcass
            return myClass;
         }
         if(c == 28){
          myClass =  (Double.valueOf(0)).getClass(); // Airmass_start
            return myClass;
         }
         if(c == 29){
          myClass =  (Double.valueOf(0)).getClass(); // Airmass_end
            return myClass;
         }
         if(c == 30){
             myClass =  (Double.valueOf(0)).getClass();  // Skymag
            return myClass;
         }    
         if(c == 31){
             myClass =  (new java.lang.String()).getClass();  // OTMslewgo
            return myClass;
         }
         if(c == 32){
             myClass =  (new java.lang.String()).getClass();  //OTMstart
            return myClass;
         }
         if(c == 33){
             myClass =  (new java.lang.String()).getClass();  //OTMend
            return myClass;
         }
         if(c == 34){
            myClass = (Double.valueOf(0)).getClass(); //OTMpa
            return myClass;
         }
         if(c == 35){
            myClass =    (Double.valueOf(0)).getClass();  // OTMwait
            return myClass;
         }
         if(c == 36){
            myClass =    (new java.lang.String()).getClass(); //OTMflag
            return myClass;
         }
         if(c == 37){
             myClass =    (new java.lang.String()).getClass(); //OTMlast
            return myClass;
         } 
         if(c == 38){
             myClass =   (Double.valueOf(0)).getClass(); // OTMslew
            return myClass;
         }
         if(c == 39){
             myClass =   (new java.lang.String()).getClass(); // OTMmoon
            return myClass;
         }
         if(c == 40){
             myClass =   (Double.valueOf(0)).getClass(); // OTMmoon
            return myClass;
         }
         if(c == 41){
             myClass =   (new java.lang.String()).getClass(); // OTMmoon
            return myClass;
         }
         if(c == 42){
             myClass =   (new java.lang.String()).getClass(); // OTMmoon
            return myClass;
         }
         if(c == 43){
             myClass =   (new java.lang.String()).getClass(); // OTMmoon
            return myClass;
         }
    }    
    if(configuration == MINIMAL_TABLE){
        if(c == 0){
            myClass =   ((Boolean.valueOf("true")).getClass()); //OBSERVATION_ID
            return myClass;
         }
//         if(c == 1){
//            myClass =  ((new Integer(0)).getClass()); // OBS_ORDER
//            return myClass;
//         }
        if(c == 1){
            myClass =  (new java.lang.String()).getClass(); // STATE
            return myClass;
         }
         if(c == 2){
            myClass =  (new java.lang.String()).getClass(); // NAME
            return myClass;
         }
          if(c == 3){
            myClass =  (new java.lang.String()).getClass(); // RA
            return myClass;
         }    
          if(c == 4){
            myClass =  (new java.lang.String()).getClass(); // DEC
            return myClass;
         }          
         if(c == 5){
            myClass =  (new java.lang.String()).getClass(); // EXPTIME
            return myClass;
         }
         if(c == 6){
             myClass =  (Double.valueOf(0)).getClass();  // OTMexpt
            return myClass;
         }           
         if(c == 7){
            myClass =  (new java.lang.String()).getClass(); // SLIT WIDTH
            return myClass;
         }
        if(c == 8){
             myClass =  (Double.valueOf(0)).getClass(); // OTMslit
            return myClass;
         }
/*        if(c == 9){
            myClass =  (new java.lang.String()).getClass(); // slitangle
            return myClass;
         }
        if(c == 10){
           myClass =  (Double.valueOf(0)).getClass(); // OTMslitangle
            return myClass;
         }
         if(c == 11){
          myClass =  (Double.valueOf(0)).getClass(); // Airmass_start
            return myClass;
         }
         if(c == 12){
          myClass = (new String()).getClass(); // SNR
            return myClass;
         }
         if(c == 13){
          myClass =  (new String()).getClass(); // note
            return myClass;
         }  */
         if(c == 9){
          myClass =  (Double.valueOf(0)).getClass(); // Airmass_start
            return myClass;
         }
         if(c == 10){
          myClass = (new String()).getClass(); // SNR
            return myClass;
         }
         if(c == 11){
          myClass =  (new String()).getClass(); // note
            return myClass;
         }
    }    
     return myClass;
  }
/*=========================================================================================================
/       End of the AstroObjectTableModel Class
/=========================================================================================================*/
    /**
     * Returns a vector that contains the same objects as the array.
     * @param anArray  the array to be converted
     * @return  the new vector; if <code>anArray</code> is <code>null</code>,
     *                          returns <code>null</code>
     */
    protected static Vector convertToVector(Object[] anArray) {
        if (anArray == null) {
            return null;
        }
        Vector<Object> v = new Vector<Object>(anArray.length);
        for (Object o : anArray) {
            v.addElement(o);
        }
        return v;
    }

    /**
     * Returns a vector of vectors that contains the same objects as the array.
     * @param anArray  the double array to be converted
     * @return the new vector of vectors; if <code>anArray</code> is
     *                          <code>null</code>, returns <code>null</code>
     */
    protected static Vector convertToVector(Object[][] anArray) {
        if (anArray == null) {
            return null;
        }
        Vector<Vector> v = new Vector<Vector>(anArray.length);
        for (Object[] o : anArray) {
            v.addElement(convertToVector(o));
        }
        return v;
    }

} // End of class DefaultTableModel

