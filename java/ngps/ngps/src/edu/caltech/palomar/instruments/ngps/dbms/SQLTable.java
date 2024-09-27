/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package edu.caltech.palomar.instruments.ngps.dbms;

import java.awt.Component;
import java.sql.ResultSet;
import java.sql.ResultSetMetaData;
import java.sql.SQLException;
import java.sql.Timestamp;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Vector;
import javax.swing.JTable;
import javax.swing.table.DefaultTableModel;
import javax.swing.table.TableCellRenderer;
import javax.swing.table.TableColumn;
import java.util.Arrays;
import java.util.List;
import java.util.ArrayList;

/**
 *
 * @author cshapiro
 */

public class SQLTable extends JTable{

        ResultSetMetaData metaData; // Table structure scraped from a mysql query
        
        @Override // Auto fit table to contents
        public Component prepareRenderer(TableCellRenderer renderer, int row, int column) {
           Component component = super.prepareRenderer(renderer, row, column);
           int rendererWidth = component.getPreferredSize().width;
           TableColumn tableColumn = getColumnModel().getColumn(column);
           tableColumn.setPreferredWidth(Math.max(rendererWidth + getIntercellSpacing().width, tableColumn.getPreferredWidth()));
           return component;
        }

        public SQLTable(ResultSet resultSet){

            super(); // JTable constructor

            try{
            this.setModel(buildTableModel(resultSet)); // 
            metaData = resultSet.getMetaData();

            // Hash and Print metadata - column names and types
            for (int i = 1; i <= metaData.getColumnCount(); i++) {
            System.out.println(String.valueOf(i) +"  "+
            metaData.getColumnName(i)+"---"+metaData.getColumnTypeName(i));
//            TableColumn tc = getNamedColumnModel("NAME");
//            tc.setMinWidth(200);
            this.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);
            }	
        }
        catch (Exception exception) {System.out.println(exception);}
        }

        public  DefaultTableModel buildTableModel(ResultSet rs) throws SQLException {
                // https://stackoverflow.com/questions/10620448/how-to-populate-jtable-from-resultset

            ResultSetMetaData metaData = rs.getMetaData();

            // names of columns
            Vector<String> columnNames = new Vector<String>();
            int columnCount = metaData.getColumnCount();
            for (int column = 1; column <= columnCount; column++) {
                columnNames.add(metaData.getColumnName(column));
            }

            // data of the table
            Vector<Vector<Object>> data = new Vector<Vector<Object>>();
            while (rs.next()) {
                Vector<Object> vector = new Vector<Object>();
                for (int columnIndex = 1; columnIndex <= columnCount; columnIndex++) {
                    vector.add(rs.getObject(columnIndex));
                }
                data.add(vector);
            }

            return new DefaultTableModel(data, columnNames);
        }

        public Object getValueAt(int row, String colname){
            /* OVERLOAD: Get a value using column name instead of index
            */
            int col = getColumn(colname).getModelIndex();
            return super.getValueAt(row, col);  // JTable.getValueAt(int,int)
        }

        public ArrayList<Object> getArray(String colname){
            /* Get all rows from named column
            */
            int col = getColumn(colname).getModelIndex();
            ArrayList<Object> colarray = new ArrayList<Object>();
            for(int row=0; row<this.getRowCount(); row++){
                colarray.add(super.getValueAt(row, col));
            }
        return colarray;
        }

        public Object getColumnType(String colname){
                /* Get data type of column from name
                */
                Object val = getValueAt(0, colname);
                return val.getClass();
        }
        public Object getColumnTypeString(String colname){
                /* Get data type of column from name
                */
                Object val = getValueAt(0, colname);
                return val.getClass().toString();
        }
        public TableColumn getNamedColumnModel(String colname){
                int col = getColumn(colname).getModelIndex();
                return getColumnModel().getColumn(col);
        }
        
   public static Timestamp stringToTimestamp(String dateString) { 
  
        // Define the date format of the input string 
        SimpleDateFormat dateFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss"); 
  
        try { 
            // Parse the input string to a java.util.Date object 
            java.util.Date parsedDate = dateFormat.parse(dateString); 
  
            // Convert java.util.Date to java.sql.Timestamp 
            Timestamp timestamp = new Timestamp(parsedDate.getTime()); 
            return timestamp;
    
        } catch (ParseException e) { 
            e.printStackTrace(); 
        }
        return null; // shouldn't get here 
    } 
           
}
