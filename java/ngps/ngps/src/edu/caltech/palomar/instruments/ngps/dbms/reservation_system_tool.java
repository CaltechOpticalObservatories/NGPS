/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package edu.caltech.palomar.instruments.ngps.dbms;

import static edu.caltech.palomar.instruments.ngps.dbms.NGPSdatabase.ERROR;
import java.beans.PropertyChangeSupport;
import java.io.FileInputStream;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.util.Properties;
import javax.swing.UIManager;

/**
 *
 * @author jennifermilburn
 */
public class reservation_system_tool {
    transient protected PropertyChangeSupport propertyChangeListeners = new PropertyChangeSupport(this);
    public PreparedStatement         QUERY_TARGET_PREP_STATEMENT;
    private java.lang.String         SEP             = System.getProperty("file.separator");
    public java.lang.String          USERDIR         = System.getProperty("user.dir");
    private java.lang.String         CONFIG          = new java.lang.String("config");
    public  java.lang.String         DBMS_CONNECTION_PROPERTIES = "reservations_dbms.ini";
    public Connection                conn = null;
    private boolean                  connected;
    public static int                DEBUG              = 0;
    public static int                INFO               = 1;
    public static int                WARN               = 2;
    public static int                ERROR              = 3;
    public static int                FATAL              = 4;    
/*================================================================================================
/      NGPSdatabase()    
/=================================================================================================*/
public reservation_system_tool(){
  boolean test = false;
  initializeDBMS();
  if(test){
  }
}       
/*================================================================================================
/     setOrder(int new_order)
/=================================================================================================*/
  public void setConnected(boolean new_connected) {
    boolean  old_connected = this.connected;
    connected = new_connected;
   propertyChangeListeners.firePropertyChange("connected", Boolean.valueOf(old_connected), Boolean.valueOf(new_connected));
  }
  public boolean isConnected() {
    return connected;
  } 
/*================================================================================================
/       initializeDBMS()
/=================================================================================================*/
public void initializeDBMS(){
   try{
      java.lang.String DBMS_CONNECTION_PROPERTIES_FILE = USERDIR + SEP + CONFIG + SEP + DBMS_CONNECTION_PROPERTIES;
      FileInputStream  dbms_properties_file            = new FileInputStream(DBMS_CONNECTION_PROPERTIES_FILE);
      Properties       dbms_properties                 = new Properties();
      dbms_properties.load(dbms_properties_file);
      dbms_properties_file.close();
      String SYSTEM             = dbms_properties.getProperty("SYSTEM");
      String DBMS               = dbms_properties.getProperty("DBMS");
      String USERNAME           = dbms_properties.getProperty("USERNAME");
      String PASSWORD           = dbms_properties.getProperty("PASSWORD");
      Class.forName("com.mysql.jdbc.Driver").newInstance(); 
      conn = DriverManager.getConnection("jdbc:mysql://"+SYSTEM+SEP+DBMS+"?"+"user="+USERNAME+"&password="+PASSWORD);
      setConnected(true);
      constructQueryTargetPreparedStatement();
   }catch(Exception e){
       setConnected(false);
       logMessage(ERROR,"Error loading the JDBC driver. "+e.toString());
   }     
} 
/*================================================================================================
/      constructQueryTargetPreparedStatement()
/=================================================================================================*/
public void constructQueryTargetPreparedStatement(){
   // example:    SELECT * FROM ngps.targets WHERE ngps.targets.NAME = 'ZTF22aaheakp' AND ngps.targets.OWNER = 'jennifer';
   String query = "SELECT * FROM ngps.targets WHERE ngps.targets.NAME = ? AND ngps.targets.OWNER = ?";
   try{
       QUERY_TARGET_PREP_STATEMENT = conn.prepareStatement(query);
    }catch(Exception e){
        logMessage(ERROR,"Creating query targets prepared statement. "+e.toString());
    }
}
/*=============================================================================================
/        logMessage(int code,java.lang.String message)
/=============================================================================================*/
public void logMessage(int code,java.lang.String message){
     System.out.println(message);
 }
 /*================================================================================================
/      add and remove Property Change Listeners
/=================================================================================================*/
      public static void main(String args[]) {
        /* Set the Nimbus look and feel */
        //<editor-fold defaultstate="collapsed" desc=" Look and feel setting code (optional) ">
        /* If Nimbus (introduced in Java SE 6) is not available, stay with the default look and feel.
         * For details see http://download.oracle.com/javase/tutorial/uiswing/lookandfeel/plaf.html 
         */ 
             try {
                 UIManager.setLookAndFeel(UIManager.getCrossPlatformLookAndFeelClassName());
                 java.lang.String test = UIManager.getSystemLookAndFeelClassName();
                 System.out.println(test);
               } catch(Exception e) {
                   e.printStackTrace();
               }
               new reservation_system_tool();
    } 
}
