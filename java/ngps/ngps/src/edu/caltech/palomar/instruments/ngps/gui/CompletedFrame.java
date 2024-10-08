/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/GUIForms/JFrame.java to edit this template
 */
package edu.caltech.palomar.instruments.ngps.gui;
import edu.caltech.palomar.instruments.ngps.dbms.NGPSdatabase;
import edu.caltech.palomar.instruments.ngps.dbms.SQLTable;

import java.sql.*;
import java.text.ParseException; 
import java.text.SimpleDateFormat; 
import javax.swing.JFrame;
import javax.swing.JScrollPane;
/**
 *
 * @author Chaz Shapiro, JPL
 */
public class CompletedFrame extends javax.swing.JFrame {
    public NGPSdatabase      dbms;
    public SQLTable          completed_obs;
    public String            MYSQLDB;
    public String            COMPLETED_TABLE;

    /**
     * Creates new Frame
     */
    public CompletedFrame(NGPSdatabase db) {
        dbms = db;
        MYSQLDB = dbms.MYSQLDB;
        COMPLETED_TABLE = dbms.COMPLETED_TABLE;
        setTitle("COMPLETED OBSERVATIONS");
        setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);       
        setVisible(true);

        // CONNECT TO DB
//		try {
//                    // below two lines are used for connectivity.
//                    Class.forName("com.mysql.cj.jdbc.Driver").newInstance();
//                    connection = DriverManager.getConnection(
//                            "jdbc:mysql://localhost:3306/ngps",
//                            "gui", "Write1Spec2.!");
//		}
//		catch (Exception exception) {
//			System.out.println(exception);
//		}

        try{
            // GET TARGET TABLE FROM MYSQL QUERY
            Statement statement = dbms.conn.createStatement();
            ResultSet resultSet = statement.executeQuery("SELECT * FROM completed_obs;"); //HARDCODE

            // Construct table from query results
            completed_obs = new SQLTable(resultSet);
            resultSet.close();
            statement.close();

//            // Select some columns
//            String[] printCols = new String[]{"NAME","RA","SLEW_START","BINSPECT","EXPTIME"};
//
//            // Print header row
//            System.out.println("------------------");            
//            System.out.println("");
//            System.out.println(Arrays.toString(printCols));
//
//            // Print selected cols for each row
//            for (int row = 0; row < completed_obs.getRowCount(); row++){
//                // System.out.println(resultSet.getString("Name"));
//                String outstring = "";
//                for(String col : printCols){
//                    outstring = outstring + completed_obs.getValueAt(row, col) + "  ";
//                }
//                System.out.println(outstring);				
//            }

        }
        catch (Exception exception) { System.out.println(exception); }

        initComponents(); // Loads JTable/SQLtable into frame
    }
    
    /**
     * This method is called from within the constructor to initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is always
     * regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        jScrollPane1 = new javax.swing.JScrollPane(completed_obs, JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED, JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addComponent(jScrollPane1, javax.swing.GroupLayout.PREFERRED_SIZE, 1449, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addGap(0, 0, Short.MAX_VALUE))
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(jScrollPane1, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, 433, Short.MAX_VALUE)
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents

    /**
     * @param args the command line arguments
     */

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JScrollPane jScrollPane1;
    // End of variables declaration//GEN-END:variables
}
