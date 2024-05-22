/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package edu.caltech.palomar.telescopes.telemetry;

import edu.caltech.palomar.telescopes.telemetry.TelescopeObject;
import edu.caltech.palomar.telescopes.P200.TelescopesIniReader;
import edu.caltech.palomar.io.ClientSocket;
import edu.dartmouth.jskycalc.JSkyCalcModel;
import java.beans.*;
import edu.caltech.palomar.telescopes.P200.gui.tables.AstroObject;
import edu.caltech.palomar.util.general.ExposureTimer;
import edu.jhu.htm.core.HTMindexImp;
import edu.jhu.htm.core.Vector3d;
import java.util.Vector;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;
import java.util.Properties;
import java.io.FileInputStream;
import java.sql.PreparedStatement;
import java.time.format.DateTimeFormatter;
import java.time.format.DateTimeFormatterBuilder;
import java.time.ZonedDateTime;
import java.time.ZoneId;
import java.time.temporal.ChronoField;
import java.sql.Timestamp;
import nom.tam.fits.Header;
import nom.tam.fits.HeaderCard;
import nom.tam.fits.HeaderCardException;
import nom.tam.util.Cursor;
import com.google.gson.Gson;
import java.util.Calendar;
import java.util.TimeZone;
import java.sql.Timestamp;
import java.sql.PreparedStatement;
import java.util.ArrayList;
import java.time.Instant;
import org.apache.log4j.BasicConfigurator;
import org.apache.log4j.DailyRollingFileAppender;
import org.apache.log4j.Layout;
import org.apache.log4j.Logger;
import org.apache.log4j.PatternLayout;
import edu.caltech.palomar.util.general.CommandLogModel;
//=== File Prolog =============================================================
//	This code was developed by California Institute of Technology
//      Caltech Optical Observatories, Palomar Observatory
//
//--- Contents ----------------------------------------------------------------
//	TelemetryController -  
//
//--- Description -------------------------------------------------------------
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
//
//      July 14, 2010 Jennifer Milburn
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
//	In no event shall Caltech be liable for any damages, including, but not
//	limited to direct, indirect, special or consequential damages, arising out
//	of, resulting from, or in any way connected with this software, whether or
//	not based upon warranty, contract, tort or otherwise, whether or not
//	injury was sustained by persons or property or otherwise, and whether or
//	not loss was sustained from or arose out of the results of, or use of,
//	their software or services provided hereunder.
//
//=== End File Prolog =========================================================
/*=============================================================================================
/      Class Definition  :   public class TelemetryController
/=============================================================================================*/
public class TelemetryController {
    transient private PropertyChangeSupport propertyChangeListeners = new PropertyChangeSupport(this);
    private java.lang.String     TERMINATOR                = new java.lang.String("\n");
    private java.lang.String     RETURN_CHAR               = new java.lang.String("\r");
    public TelescopeObject       myTelescopeObject         = new TelescopeObject();
    public ClientSocket          myClientSocket            = new ClientSocket();
    public TelescopesIniReader   myTelescopesIniReader     = new TelescopesIniReader();
    UpdateTelescopeStatusThread  myUpdateTelescopeStatusThread;
    UpdateEphemerisThread        myUpdateEphemerisThread;
    public Connection            conn = null;
//    JSkyCalcModel                myJSkyCalcModel           = new JSkyCalcModel();
    int                          pollingRate;
    public int                   DEFAULT_POLLING_RATE      = 500;
    private boolean              connected;
    private boolean              control_connected;
    private boolean              abort_connected;
    private boolean              moving;
    private boolean              polling;
    public AstroObject           currentAstroObject;
    public static int            MOVING_N = 1;
    public static int            MOVING_E = 2;
    public static int            MOVING_S = 3;
    public static int            MOVING_W = 4;
    public        long           SECOND_TO_MILLI_CONVERSION    = 1000;
    public        int            TIMER_DELAY                   = 1000;
    public        int            INT_TIMER_DELAY               = 1000;
    public        int            UPDATE_TIMELINE_DELAY         = 100;
    public static int            FOCUS_SET                     = 1;
    public static int            FOCUS_OFFSET                  = 2;
    private java.lang.String     coordMessage = new java.lang.String();
    private java.lang.String     SEP             = System.getProperty("file.separator");
    public java.lang.String      USERDIR         = System.getProperty("user.dir");
    private java.lang.String     CONFIG          = new java.lang.String("config");
    public  java.lang.String     DBMS_CONNECTION_PROPERTIES = "telemetry_dbms.ini";
    public PreparedStatement     INSERT_PREP_STATEMENT;
    public TelemetryObject       CURRENT_TELEMETRY;
    public Header                CURRENT_FITS_HEADER;
    public java.lang.String      CURRENT_FITS_HEADER_STRING = new java.lang.String();
    public java.lang.String      CURRENT_GSON_TELEMETRY     = new java.lang.String();
    public String[]              cards;
    private java.lang.String     start_timestamp;
    private java.lang.String     end_timestamp;
    private TelemetryTableModel  myTelemetryTableModel;
    static Logger                    telemetryLogger    = Logger.getLogger(TelemetryController.class);
    public DailyRollingFileAppender  fileAppender       = new DailyRollingFileAppender();
    private Layout                   layout             = new PatternLayout("%-5p [%t]: %m%n");
    public static int                DEBUG              = 0;
    public static int                INFO               = 1;
    public static int                WARN               = 2;
    public static int                ERROR              = 3;
    public static int                FATAL              = 4;    
    public java.lang.String          LOG_DIRECTORY      = new java.lang.String();
    public CommandLogModel           myCommandLogModel  = new CommandLogModel();
    private java.lang.String         output_directory   = new java.lang.String();
// `ELEVHOR` VARCHAR(45) NULL,
// `ELEVSEA` VARCHAR(45) NULL,
// `LAT` VARCHAR(45) NULL,
// `LONGITUDE` VARCHAR(45) NULL,
/*=============================================================================================
/      Class Definition  :   public class TelemetryController
/=============================================================================================*/
    public TelemetryController(){
       jbInit();     
       setConnected(false);
       setCoordinateStatusMessage("");
       DEFAULT_POLLING_RATE = (Integer.valueOf(myTelescopesIniReader.POLLING_RATE)).intValue(); 
       initializeDBMS();
       initializeTelemetryTableModel();
    }    
/*================================================================================================
/        jbInit() initialization method
/=================================================================================================*/
  private void jbInit(){
        setConnected(false);
        myClientSocket = new ClientSocket(myTelescopesIniReader.SERVERNAME_STRING,myTelescopesIniReader.SERVERPORT_INTEGER);
        myClientSocket.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
        public void propertyChange(java.beans.PropertyChangeEvent e) {
            myClientSocket_propertyChange(e);
         }
        });
        myUpdateTelescopeStatusThread = new UpdateTelescopeStatusThread(DEFAULT_POLLING_RATE);
        myUpdateEphemerisThread       = new UpdateEphemerisThread();
   }
  /*=============================================================================================
/         initializeLogging()
/=============================================================================================*/
 private void initializeLogging(){
     BasicConfigurator.configure();
     fileAppender.setFile(LOG_DIRECTORY + SEP +"TelemetryServer.log");        
     fileAppender.setDatePattern("'.'yyyy-MM-dd");
     fileAppender.setAppend(true);
     fileAppender.setLayout(layout);
     fileAppender.activateOptions();
     telemetryLogger.addAppender(fileAppender);
 }
 /*=============================================================================================
/        logMessage(int code,java.lang.String message)
/=============================================================================================*/
public void logMessage(int code,java.lang.String message){

     switch(code){
         case 0:  telemetryLogger.debug(message);  break;
         case 1:  telemetryLogger.info(message);   break;
         case 2:  telemetryLogger.warn(message);   break;
         case 3:  telemetryLogger.error(message);  break;
         case 4:  telemetryLogger.fatal(message);  break;
     }
     if((code ==WARN)|(code == ERROR)|(code ==FATAL)){
         myCommandLogModel.insertMessage(CommandLogModel.ERROR, message);
     }
     System.out.println(message);
 }
/*=============================================================================================
/     getCommandLogModel()
/=============================================================================================*/
public CommandLogModel getCommandLogModel(){
    return myCommandLogModel;
}
/*=============================================================================================
/     Property Change Listener for the _propertyChange
/=============================================================================================*/
  private void myClientSocket_propertyChange(PropertyChangeEvent e)  {
     java.lang.String propertyName = e.getPropertyName();
       if(propertyName == "connected"){
        boolean state = (java.lang.Boolean)e.getNewValue();
        setConnected(state);
        if(state){
            initializeTelescopeState();
        }
        if(!state){
            executeMonitorConnection();
        }
      }
  } 
/*================================================================================================
/     executeMonitorConnection()
/=================================================================================================*/
public void executeMonitorConnection(){
    ExecuteMonitorConnectionThread myExecuteMonitorConnectionThread = new ExecuteMonitorConnectionThread();
    myExecuteMonitorConnectionThread.start();
}
/*================================================================================================
/      monitorConnection()
/=================================================================================================*/
 public void monitorConnection(){
     while(!myClientSocket.isConnected()){
       waitForResponseMilliseconds(15000);
       myClientSocket.startConnection(ClientSocket.USE_HOSTNAME);
    }   
 }  
/*================================================================================================
/      initializeTelemetryTableModel()
/=================================================================================================*/
private void initializeTelemetryTableModel(){
    myTelemetryTableModel = new TelemetryTableModel();
}
public TelemetryTableModel getTelemetryTableModel(){
    return myTelemetryTableModel;
}
/*================================================================================================
/      executeTimestampQuery(java.lang.String new_start_timestamp)
/=================================================================================================*/
public void executeTimestampQuery(java.lang.String new_start_timestamp){
    ExecuteQueryThread myExecuteQueryThread = new ExecuteQueryThread(new_start_timestamp);
    myExecuteQueryThread.start();
}
/*================================================================================================
/      executeTimestampQuery(java.lang.String new_start_timestamp)
/=================================================================================================*/
public void executeRangeQuery(java.lang.String new_start_timestamp,java.lang.String new_end_timestamp){
    ExecuteQueryThread myExecuteQueryThread = new ExecuteQueryThread(new_start_timestamp,new_end_timestamp);
    myExecuteQueryThread.start();
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
      String SYSTEM   = dbms_properties.getProperty("SYSTEM");
      String DBMS     = dbms_properties.getProperty("DBMS");
      String USERNAME = dbms_properties.getProperty("USERNAME");
      String PASSWORD = dbms_properties.getProperty("PASSWORD");
      LOG_DIRECTORY = dbms_properties.getProperty("LOG_DIRECTORY");      
      Class.forName("com.mysql.jdbc.Driver").newInstance(); 
      conn = DriverManager.getConnection("jdbc:mysql://"+SYSTEM+SEP+DBMS+"?"+"user="+USERNAME+"&password="+PASSWORD);
      constructPreparedStatement();
      initializeLogging();
   }catch(Exception e){
//     System.out.println("Error loading the JDBC driver. "+e.toString()); 
     logMessage(ERROR,"Error loading the JDBC driver. "+e.toString());
   }     
} 
/*================================================================================================
/       constructPreparedStatement()
/=================================================================================================*/
public void constructPreparedStatement(){
    String query = "INSERT INTO p200telemetry ("
    + " TIMESTAMP_UTC,"
    + " UTCDATETIME,"
    + " LOCALDATETIME,"
    + " UTCSTART,"
    + " HTM_7,"        
    + " HTM_20,"        
    + " AIRMASS,"
    + " AZIMUTH,"
    + " CASSANG,"
    + " DECLINATION,"
    + " DECOFF,"
    + " DECTRAC,"
    + " DOMEAZI,"
    + " DOMESHU,"
    + " EQUINOX,"
    + " HA,"
    + " INSTRMNT,"
    + " LST,"
    + " OBJECT,"
    + " PUMPS,"
    + " RA,"
    + " RAOFF,"
    + " RATEDEC,"
    + " RATERA,"
    + " TELFOCUS,"
    + " TELID,"
    + " TMOTION,"
    + " WINDSCR,"
    + " ZA,"
    + " ALTITUDE,"
    + " ALTPARA,"
    + " BARYJD,"
    + " BARYVCOR,"
    + " CONSTEL,"
    + " JD,"
    + " MOONALT,"
    + " MOONANG,"
    + " MOONAZ,"
    + " MOONDEC,"
    + " MOONLIG,"
    + " MOONILM,"
    + " MOONPH,"
    + " MOONRA,"
    + " PARA,"
    + " PLANETW,"
    + " SUNALT,"
    + " SUNAZ,"
    + " SUNDEC,"
    + " SUNRA,"
    + " ZTWILIGH"
    + " ) VALUES ("
    +  " ?, ?, ?, ?, ?, ?, ?, ?, ?, ?,"
    +  " ?, ?, ?, ?, ?, ?, ?, ?, ?, ?,"
    +  " ?, ?, ?, ?, ?, ?, ?, ?, ?, ?,"
    +  " ?, ?, ?, ?, ?, ?, ?, ?, ?, ?,"
    +  " ?, ?, ?, ?, ?, ?, ?, ?, ?, ?"
    +  ")";
    try{
       INSERT_PREP_STATEMENT = conn.prepareStatement(query);
    }catch(Exception e){
        logMessage(ERROR,"Creating prepared statement. "+e.toString());
//        System.out.println(e.toString());
    }
}
/*================================================================================================
/       constructPreparedStatement()
/=================================================================================================*/
public java.lang.String queryToJSON(java.lang.String timestamp){
   java.lang.String json_string_array = new java.lang.String();
    try{
       ArrayList results = query(timestamp);
       int count = results.size();
       json_string_array = json_string_array + "[";
       for(int i=0;i<count;i++){
          TelemetryObject  current      = (TelemetryObject)results.get(i);
          java.lang.String current_json = telemetryToGson(current);
          if(i == 0){
             json_string_array = json_string_array+current_json+TERMINATOR; 
          }
          if(i != 0){
              json_string_array = json_string_array+","+current_json+TERMINATOR;
          }         
       }
       json_string_array = json_string_array + "]";
   }catch(Exception e){
      logMessage(ERROR,"Error in queryToJSON. "+e.toString());
      System.out.println(e.toString());
   } 
  return json_string_array;
}
/*================================================================================================
/       queryRangeToJSON(java.lang.String timestamp_start,java.lang.String timestamp_end)
/=================================================================================================*/
public java.lang.String queryRangeToJSON(java.lang.String timestamp_start,java.lang.String timestamp_end){
   java.lang.String json_string_array = new java.lang.String();
    try{
       ArrayList results = query_range(timestamp_start,timestamp_end);
       int count = results.size();
       System.out.println("RECORDS RECEIVED = "+count);
       for(int i=0;i<count;i++){
          TelemetryObject  current      = (TelemetryObject)results.get(i);
          java.lang.String current_json = telemetryToGson(current);                
          json_string_array = json_string_array+current_json+TERMINATOR;
       }
   }catch(Exception e){
      logMessage(ERROR,"Error in queryRangeToJSON "+e.toString());
//      System.out.println(e.toString());
   }
  return json_string_array;
}
/*================================================================================================
/       queryRangeToJSON(java.lang.String timestamp_start,java.lang.String timestamp_end)
/=================================================================================================*/
public java.lang.String queryRangeToJSON(java.io.BufferedWriter client,java.lang.String timestamp_start,java.lang.String timestamp_end){
   java.lang.String json_string_array = new java.lang.String();
    try{
       ArrayList results = query_range(timestamp_start,timestamp_end);
       int count = results.size();
       System.out.println("RECORDS RECEIVED = "+count);
       client.write("["+TERMINATOR);
       for(int i=0;i<count;i++){
          TelemetryObject  current      = (TelemetryObject)results.get(i);
          java.lang.String current_json = telemetryToGson(current);
          if(i < (count-1)){
              client.write(current_json+","); 
          }
          if(i == (count-1)){
              client.write(current_json); 
          }                          
          json_string_array = json_string_array+current_json+TERMINATOR;
       }
       json_string_array.lastIndexOf(",");
       client.write("]"+TERMINATOR);
       client.flush();
   }catch(Exception e){
      logMessage(ERROR,"Error in queryRangeToJSON "+e.toString());
//      System.out.println(e.toString());
   }
  return json_string_array;
}
/*================================================================================================
/       constructPreparedStatement()
/=================================================================================================*/
public TelemetryObject transformResultSetToTelemetryObject(java.sql.ResultSet results){
     TelemetryObject current = new TelemetryObject();
      try{  
          current.ID              = results.getLong("ID");
          current.UTC_TIMESTAMP   = results.getTimestamp("TIMESTAMP_UTC");
          current.UTC_DATE_TIME   = results.getString("UTCDATETIME");
          current.LOCAL_DATE_TIME = results.getString("LOCALDATETIME");
          current.UTCSTART        = results.getString("UTCSTART");
          current.HTM_LEVEL_7     = results.getLong("HTM_7");
          current.HTM_LEVEL_20    = results.getLong("HTM_20");
          current.AIRMASS         = results.getFloat("AIRMASS");
          current.ALTITUDE        = results.getFloat("ALTITUDE");
          current.AZIMUTH         = results.getFloat("AZIMUTH");
          current.ALTPARA         = results.getFloat("ALTPARA");
          current.BARYJD          = results.getDouble("BARYJD");
          current.BARYVCOR        = results.getString("BARYVCOR");
          current.CASSANG         = results.getFloat("CASSANG");
          current.CONSTEL         = results.getString("CONSTEL");
          current.DEC             = results.getString("DECLINATION");
          current.DECOFF          = results.getFloat("DECOFF");
          current.DECTRAC         = results.getFloat("DECTRAC");
          current.DOMEAZI         = results.getFloat("DOMEAZI");
          current.DOMESHU         = results.getString("DOMESHU");
          current.EQUINOX         = results.getFloat("EQUINOX");
          current.HA              = results.getString("HA");
          current.INSTRUMNT       = results.getString("INSTRMNT");
          current.JD              = results.getDouble("JD");
          current.LST             = results.getFloat("LST");
          current.MOONALT         = results.getFloat("MOONALT");
          current.MOONANG         = results.getFloat("MOONANG");
          current.MOONAZ          = results.getFloat("MOONAZ");
          current.MOONDEC         = results.getString("MOONDEC");
          current.MOONILM         = results.getString("MOONILM");
          current.MOONLIG         = results.getString("MOONLIG");
          current.MOONPH          = results.getString("MOONPH");
          current.MOONRA          = results.getString("MOONRA");
          current.OBJECT          = results.getString("OBJECT");
          current.PARA            = results.getFloat("PARA");
          current.PLANETW         = results.getString("PLANETW");
          current.PUMPS           = results.getString("PUMPS");
          current.RA              = results.getString("RA");
          current.RAOFF           = results.getFloat("RAOFF");
          current.RATEDEC         = results.getFloat("RATEDEC");
          current.RATERA          = results.getFloat("RATERA");
          current.SUNALT          = results.getFloat("SUNALT");
          current.SUNAZ           = results.getFloat("SUNAZ");
          current.SUNDEC          = results.getString("SUNDEC");
          current.SUNRA           = results.getString("SUNRA");
          current.TELFOCUS        = results.getFloat("TELFOCUS");
          current.TELID           = results.getString("TELID");
          current.TMOTION         = results.getString("TMOTION");
          current.WINDSCR         = results.getString("WINDSCR");
          current.ZA              = results.getFloat("ZA");
          current.ZTWILIGH        = results.getString("ZTWILIGH");
      }catch(Exception e){
         logMessage(ERROR,"Error in method transformResultSetToTelemetryObject "+e.toString());
//         System.out.println(e.toString());          
      }
  return current;
}
/*================================================================================================
/       constructPreparedStatement()
/=================================================================================================*/
public ArrayList query(java.lang.String timestamp){    
    java.util.ArrayList telemetry_array = new java.util.ArrayList();
    try{
       java.sql.ResultSet results = queryTimestamp(timestamp);
       while(results.next()){
          TelemetryObject current = transformResultSetToTelemetryObject(results);
          Header current_header = constructHeaderLocal(current);
//          current.CURRENT_HEADER = current_header;
          telemetry_array.add(current);
          System.out.println(current.UTC_TIMESTAMP);
       } 
    }catch(Exception e){
       logMessage(ERROR,"Error in method query "+e.toString());
//       System.out.println(e.toString());
    }   
    return telemetry_array;
}
/*================================================================================================
/       constructPreparedStatement()
/=================================================================================================*/
public ArrayList query_range(java.lang.String timestamp_start,java.lang.String timestamp_end){    
    java.util.ArrayList telemetry_array = new java.util.ArrayList();
    try{
       java.sql.ResultSet results = queryTimestamp_Range(timestamp_start,timestamp_end);
       while(results.next()){
          TelemetryObject current        = transformResultSetToTelemetryObject(results);
          Header          current_header = constructHeaderLocal(current);
//                  current.CURRENT_HEADER = current_header;
          telemetry_array.add(current);
          System.out.println(current.UTC_TIMESTAMP);
       } 
    }catch(Exception e){
       logMessage(ERROR,"Error in method query_range "+e.toString());
//       System.out.println(e.toString());
    }   
    return telemetry_array;
}
/*================================================================================================
/       constructPreparedStatement()
/=================================================================================================*/
public java.sql.ResultSet queryTimestamp(java.lang.String timestamp){
    java.sql.ResultSet rs    = null; 
    java.lang.String   query = "SELECT * FROM telemetry.p200telemetry WHERE TIMESTAMP_UTC = ?"; 
     try{
       java.sql.Timestamp         ts       = java.sql.Timestamp.valueOf(timestamp);       
       java.sql.PreparedStatement st       = conn.prepareStatement(query);
                                  st.setTimestamp(1,ts,Calendar.getInstance(TimeZone.getTimeZone("UTC")));       
                                  rs       = st.executeQuery();  
       java.sql.ResultSetMetaData metaData = rs.getMetaData();
     }catch(Exception e){
       logMessage(ERROR,"Error in method queryTimestamp "+e.toString());
//       System.out.println(e.toString());  
     }
  return rs;
}
/*================================================================================================
/       queryTimestamp_Range(java.lang.String timestamp)
/=================================================================================================*/
public java.sql.ResultSet queryTimestamp_Range(java.lang.String timestamp_start,java.lang.String timestamp_end){
    java.sql.ResultSet rs    = null; 
    java.lang.String   query = "SELECT * FROM telemetry.p200telemetry WHERE TIMESTAMP_UTC >= ? AND TIMESTAMP_UTC <= ?"; 
     try{
       java.sql.Timestamp         ts_start = java.sql.Timestamp.valueOf(timestamp_start);       
       java.sql.Timestamp         ts_end   = java.sql.Timestamp.valueOf(timestamp_end);       
       java.sql.PreparedStatement st       = conn.prepareStatement(query);
                                  st.setTimestamp(1,ts_start,Calendar.getInstance(TimeZone.getTimeZone("UTC")));       
                                  st.setTimestamp(2,ts_end,Calendar.getInstance(TimeZone.getTimeZone("UTC")));       
                                  long start = System.currentTimeMillis();
                                  rs       = st.executeQuery(); 
                                  long end = System.currentTimeMillis();
                                  long duration = end - start;
                                  System.out.println("Query Time = "+duration);
       java.sql.ResultSetMetaData metaData = rs.getMetaData();
     }catch(Exception e){
       logMessage(ERROR,"Error in method queryTimestamp_Range "+e.toString());
       System.out.println(e.toString());  
     }          
  return rs;
}
/*================================================================================================
/       TelescopeObject getTelescopeObject()
/=================================================================================================*/
public void executeInsertStatement(TelemetryObject current){
    try{
        INSERT_PREP_STATEMENT.clearParameters();        
        INSERT_PREP_STATEMENT.setTimestamp(1,current.UTC_TIMESTAMP);
        INSERT_PREP_STATEMENT.setString(2,current.UTC_DATE_TIME);
        INSERT_PREP_STATEMENT.setString(3,current.LOCAL_DATE_TIME);
        INSERT_PREP_STATEMENT.setString(4,current.UTCSTART);
        INSERT_PREP_STATEMENT.setLong(5, current.HTM_LEVEL_7);
        INSERT_PREP_STATEMENT.setLong(6, current.HTM_LEVEL_20);
        INSERT_PREP_STATEMENT.setFloat(7,current.AIRMASS);
        INSERT_PREP_STATEMENT.setFloat(8,current.AZIMUTH);
        INSERT_PREP_STATEMENT.setFloat(9,current.CASSANG);
        INSERT_PREP_STATEMENT.setString(10,current.DEC);
        INSERT_PREP_STATEMENT.setFloat(11,current.DECOFF);
        INSERT_PREP_STATEMENT.setFloat(12,current.DECTRAC);
        INSERT_PREP_STATEMENT.setFloat(13,current.DOMEAZI);
        INSERT_PREP_STATEMENT.setString(14,current.DOMESHU);
        INSERT_PREP_STATEMENT.setFloat(15,current.EQUINOX);
        INSERT_PREP_STATEMENT.setString(16,current.HA);
        INSERT_PREP_STATEMENT.setString(17,current.INSTRUMNT);
        INSERT_PREP_STATEMENT.setFloat(18,current.LST);
        INSERT_PREP_STATEMENT.setString(19,current.OBJECT.trim());
        INSERT_PREP_STATEMENT.setString(20,current.PUMPS.trim());
        INSERT_PREP_STATEMENT.setString(21,current.RA);
        INSERT_PREP_STATEMENT.setFloat(22,current.RAOFF);
        INSERT_PREP_STATEMENT.setFloat(23,current.RATEDEC);
        INSERT_PREP_STATEMENT.setFloat(24,current.RATERA);
        INSERT_PREP_STATEMENT.setFloat(25,current.TELFOCUS);
        INSERT_PREP_STATEMENT.setString(26,current.TELID);
        INSERT_PREP_STATEMENT.setString(27,current.TMOTION);
        INSERT_PREP_STATEMENT.setString(28,current.WINDSCR);
        INSERT_PREP_STATEMENT.setFloat(29,current.ZA);
        INSERT_PREP_STATEMENT.setFloat(30,current.ALTITUDE);
        INSERT_PREP_STATEMENT.setFloat(31,current.ALTPARA);
        INSERT_PREP_STATEMENT.setDouble(32,current.BARYJD);
        INSERT_PREP_STATEMENT.setString(33,current.BARYVCOR);
        INSERT_PREP_STATEMENT.setString(34,current.CONSTEL);
//        INSERT_PREP_STATEMENT.setString(33,current.ELEVHOR);
//        INSERT_PREP_STATEMENT.setString(34,current.ELEVSEA);
        INSERT_PREP_STATEMENT.setDouble(35,current.JD);
//        INSERT_PREP_STATEMENT.setString(36,current.LAT);
//        INSERT_PREP_STATEMENT.setString(37,current.LONG);
        INSERT_PREP_STATEMENT.setFloat(36,current.MOONALT);
        INSERT_PREP_STATEMENT.setFloat(37,current.MOONANG);
        INSERT_PREP_STATEMENT.setFloat(38,current.MOONAZ);
        INSERT_PREP_STATEMENT.setString(39,current.MOONDEC);
        INSERT_PREP_STATEMENT.setString(40,current.MOONLIG);
        INSERT_PREP_STATEMENT.setString(41,current.MOONILM);
        INSERT_PREP_STATEMENT.setString(42,current.MOONPH);
        INSERT_PREP_STATEMENT.setString(43,current.MOONRA);
        INSERT_PREP_STATEMENT.setFloat(44,current.PARA);
        INSERT_PREP_STATEMENT.setString(45,current.PLANETW);
        INSERT_PREP_STATEMENT.setFloat(46,current.SUNALT);
        INSERT_PREP_STATEMENT.setFloat(47,current.SUNAZ);
        INSERT_PREP_STATEMENT.setString(48,current.SUNDEC);
        INSERT_PREP_STATEMENT.setString(49,current.SUNRA);
        INSERT_PREP_STATEMENT.setString(50,current.ZTWILIGH);
        
        System.out.println(INSERT_PREP_STATEMENT.toString());
        // now that we have all the parameters set execute the update on the DBMS
        INSERT_PREP_STATEMENT.executeUpdate();
    }catch(Exception e){
       logMessage(ERROR,"Error in method executeInsertStatement "+e.toString());
//       System.out.println(e.toString()); 
    }  
}
/*================================================================================================
/       constructHeader(TelemetryObject current
/=================================================================================================*/
public Header constructHeaderLocal(TelemetryObject current){
    Header currentHeader = new Header(); 
    try{
       currentHeader.addLine(new HeaderCard("TIMESTMP",current.UTC_TIMESTAMP.toString(),"UTC timestamp"));
       currentHeader.addLine(new HeaderCard("LOCALDT",current.LOCAL_DATE_TIME,"Local date and time"));
       currentHeader.addLine(new HeaderCard("UTCDT",current.UTC_DATE_TIME,"UTC date and time"));
       currentHeader.addLine(new HeaderCard("UTCSTART",current.UTCSTART,"UTC timestamp from the P200 telescope"));
       currentHeader.addLine(new HeaderCard("HTM7",current.HTM_LEVEL_7,"HTM index, level 7"));
       currentHeader.addLine(new HeaderCard("HTM20",current.HTM_LEVEL_20,"HTM index, level 20"));                
       currentHeader.addLine(new HeaderCard("AIRMASS",current.AIRMASS,"Airmass"));
       currentHeader.addLine(new HeaderCard("ALTITUDE",current.ALTITUDE,"Altitude"));
       currentHeader.addLine(new HeaderCard("ALTPARA",current.ALTPARA,"Anti-Paralactic angle"));
       currentHeader.addLine(new HeaderCard("AZIMUTH",current.AZIMUTH,"Azimuth"));
       currentHeader.addLine(new HeaderCard("BARYJD",current.BARYJD,"Barycentric Julian Date"));
       currentHeader.addLine(new HeaderCard("BARYVCOR",current.BARYVCOR,"Barycentric velocity correction"));
       currentHeader.addLine(new HeaderCard("CASSANG",current.CASSANG,"Cassegrain angle"));
       currentHeader.addLine(new HeaderCard("CONSTEL",current.CONSTEL,"Constellation abbreviation"));
       currentHeader.addLine(new HeaderCard("DEC",current.DEC,"Declination"));
       currentHeader.addLine(new HeaderCard("DECOFF",current.DECOFF,"Declination offset"));
       currentHeader.addLine(new HeaderCard("DECTRAC",current.DECTRAC,"Declination tracking rate"));
       currentHeader.addLine(new HeaderCard("DOMEAZI",current.DOMEAZI,"Dome azimuth"));
       currentHeader.addLine(new HeaderCard("DOMESHU",current.DOMESHU,"Dome shutters"));
       currentHeader.addLine(new HeaderCard("ELEVHOR",current.ELEVHOR,"Elevation horizon - observatory"));
       currentHeader.addLine(new HeaderCard("ELEVSEA",current.ELEVSEA,"Elevation above sea level - observatory"));
       currentHeader.addLine(new HeaderCard("EQUINOX",current.EQUINOX,"Equinox"));
       currentHeader.addLine(new HeaderCard("HA",current.HA,"Hour angle"));
       currentHeader.addLine(new HeaderCard("INSTRMNT",current.INSTRUMNT,"Instrument"));
       currentHeader.addLine(new HeaderCard("JD",current.JD,"Julian Date"));
       currentHeader.addLine(new HeaderCard("LAT",current.LAT,"Latitude - observatory"));
       currentHeader.addLine(new HeaderCard("LONG",current.LONG,"Longitude -  observatory"));
       currentHeader.addLine(new HeaderCard("LST",current.LST,"Local Siderial Time"));
       currentHeader.addLine(new HeaderCard("MOONALT",current.MOONALT,"Moon altitude"));
       currentHeader.addLine(new HeaderCard("MOONANG",current.MOONANG,"Moon - object angle"));
       currentHeader.addLine(new HeaderCard("MOONAZ",current.MOONAZ,"Moon azimuth"));
       currentHeader.addLine(new HeaderCard("MOONDEC",current.MOONDEC,"Moon declination"));
       currentHeader.addLine(new HeaderCard("MOONILM",current.MOONILM,"Moon illumination"));
       currentHeader.addLine(new HeaderCard("MOONLIG",current.MOONLIG,"Moon light"));
       currentHeader.addLine(new HeaderCard("MOONPH",current.MOONPH,"Moon phase"));
       currentHeader.addLine(new HeaderCard("OBJECT",current.OBJECT,"Object"));
       currentHeader.addLine(new HeaderCard("PARA",current.PARA,"Paralactic angle"));
       currentHeader.addLine(new HeaderCard("PLANETW",current.PLANETW,"Planet Warning"));
//       currentHeader.addLine(new HeaderCard("PUMPS",current.PUMPS,"Pumps"));
       currentHeader.addLine(new HeaderCard("RA",current.RA,"Right Ascension"));
       currentHeader.addLine(new HeaderCard("RAOFF",current.RAOFF,"Right ascension offset"));
       currentHeader.addLine(new HeaderCard("RATEDEC",current.RATEDEC,"Declination tracking rate"));
       currentHeader.addLine(new HeaderCard("RATERA",current.RATERA,"Right Ascension tracking rate"));
       currentHeader.addLine(new HeaderCard("SUNALT",current.SUNALT,"Sun Altitude"));
       currentHeader.addLine(new HeaderCard("SUNAZ",current.SUNAZ,"Sun Azimuth"));
       currentHeader.addLine(new HeaderCard("SUNDEC",current.SUNDEC,"Sun Declination"));
       currentHeader.addLine(new HeaderCard("SUNRA",current.SUNRA,"Sun Right Ascension"));
       currentHeader.addLine(new HeaderCard("TELFOCUS",current.TELFOCUS,"Telescope focus"));
       currentHeader.addLine(new HeaderCard("TELID",current.TELID,"Telescope ID"));
       currentHeader.addLine(new HeaderCard("TMOTION",current.TMOTION,"Telescope motion flag"));
       currentHeader.addLine(new HeaderCard("WINDSCR",current.WINDSCR,"Wind screen"));
       currentHeader.addLine(new HeaderCard("ZA",current.ZA,"Zenith Angle"));
       currentHeader.addLine(new HeaderCard("ZTWILIGH",current.ZTWILIGH,"Zenith Twilight"));        
    }catch(Exception e){
       logMessage(ERROR,"Error in method constructHeaderLocal "+e.toString());
    }
   return currentHeader;
}
/*================================================================================================
/       constructHeader(TelemetryObject current
/=================================================================================================*/
public java.lang.String constructHeader(TelemetryObject current){
    Header currentHeader = new Header(); 
    try{
       currentHeader.addLine(new HeaderCard("TIMESTMP",current.UTC_TIMESTAMP.toString(),"UTC timestamp"));
       currentHeader.addLine(new HeaderCard("LOCALDT",current.LOCAL_DATE_TIME,"Local date and time"));
       currentHeader.addLine(new HeaderCard("UTCDT",current.UTC_DATE_TIME,"UTC date and time"));
       currentHeader.addLine(new HeaderCard("UTCSTART",current.UTCSTART,"UTC timestamp from the P200 telescope"));
       currentHeader.addLine(new HeaderCard("HTM7",current.HTM_LEVEL_7,"HTM index, level 7"));
       currentHeader.addLine(new HeaderCard("HTM20",current.HTM_LEVEL_20,"HTM index, level 20"));                
       currentHeader.addLine(new HeaderCard("AIRMASS",current.AIRMASS,"Airmass"));
       currentHeader.addLine(new HeaderCard("ALTITUDE",current.ALTITUDE,"Altitude"));
       currentHeader.addLine(new HeaderCard("ALTPARA",current.ALTPARA,"Anti-Paralactic angle"));
       currentHeader.addLine(new HeaderCard("AZIMUTH",current.AZIMUTH,"Azimuth"));
       currentHeader.addLine(new HeaderCard("BARYJD",current.BARYJD,"Barycentric Julian Date"));
       currentHeader.addLine(new HeaderCard("BARYVCOR",current.BARYVCOR,"Barycentric velocity correction"));
       currentHeader.addLine(new HeaderCard("CASSANG",current.CASSANG,"Cassegrain angle"));
       currentHeader.addLine(new HeaderCard("CONSTEL",current.CONSTEL,"Constellation abbreviation"));
       currentHeader.addLine(new HeaderCard("DEC",current.DEC,"Declination"));
       currentHeader.addLine(new HeaderCard("DECOFF",current.DECOFF,"Declination offset"));
       currentHeader.addLine(new HeaderCard("DECTRAC",current.DECTRAC,"Declination tracking rate"));
       currentHeader.addLine(new HeaderCard("DOMEAZI",current.DOMEAZI,"Dome azimuth"));
       currentHeader.addLine(new HeaderCard("DOMESHU",current.DOMESHU,"Dome shutters"));
       currentHeader.addLine(new HeaderCard("ELEVHOR",current.ELEVHOR,"Elevation horizon - observatory"));
       currentHeader.addLine(new HeaderCard("ELEVSEA",current.ELEVSEA,"Elevation above sea level - observatory"));
       currentHeader.addLine(new HeaderCard("EQUINOX",current.EQUINOX,"Equinox"));
       currentHeader.addLine(new HeaderCard("HA",current.HA,"Hour angle"));
       currentHeader.addLine(new HeaderCard("INSTRMNT",current.INSTRUMNT,"Instrument"));
       currentHeader.addLine(new HeaderCard("JD",current.JD,"Julian Date"));
       currentHeader.addLine(new HeaderCard("LAT",current.LAT,"Latitude - observatory"));
       currentHeader.addLine(new HeaderCard("LONG",current.LONG,"Longitude -  observatory"));
       currentHeader.addLine(new HeaderCard("LST",current.LST,"Local Siderial Time"));
       currentHeader.addLine(new HeaderCard("MOONALT",current.MOONALT,"Moon altitude"));
       currentHeader.addLine(new HeaderCard("MOONANG",current.MOONANG,"Moon - object angle"));
       currentHeader.addLine(new HeaderCard("MOONAZ",current.MOONAZ,"Moon azimuth"));
       currentHeader.addLine(new HeaderCard("MOONDEC",current.MOONDEC,"Moon declination"));
       currentHeader.addLine(new HeaderCard("MOONILM",current.MOONILM,"Moon illumination"));
       currentHeader.addLine(new HeaderCard("MOONLIG",current.MOONLIG,"Moon light"));
       currentHeader.addLine(new HeaderCard("MOONPH",current.MOONPH,"Moon phase"));
       currentHeader.addLine(new HeaderCard("OBJECT",current.OBJECT,"Object"));
       currentHeader.addLine(new HeaderCard("PARA",current.PARA,"Paralactic angle"));
       currentHeader.addLine(new HeaderCard("PLANETW",current.PLANETW,"Planet Warning"));
//       currentHeader.addLine(new HeaderCard("PUMPS",current.PUMPS,"Pumps"));
       currentHeader.addLine(new HeaderCard("RA",current.RA,"Right Ascension"));
       currentHeader.addLine(new HeaderCard("RAOFF",current.RAOFF,"Right ascension offset"));
       currentHeader.addLine(new HeaderCard("RATEDEC",current.RATEDEC,"Declination tracking rate"));
       currentHeader.addLine(new HeaderCard("RATERA",current.RATERA,"Right Ascension tracking rate"));
       currentHeader.addLine(new HeaderCard("SUNALT",current.SUNALT,"Sun Altitude"));
       currentHeader.addLine(new HeaderCard("SUNAZ",current.SUNAZ,"Sun Azimuth"));
       currentHeader.addLine(new HeaderCard("SUNDEC",current.SUNDEC,"Sun Declination"));
       currentHeader.addLine(new HeaderCard("SUNRA",current.SUNRA,"Sun Right Ascension"));
       currentHeader.addLine(new HeaderCard("TELFOCUS",current.TELFOCUS,"Telescope focus"));
       currentHeader.addLine(new HeaderCard("TELID",current.TELID,"Telescope ID"));
       currentHeader.addLine(new HeaderCard("TMOTION",current.TMOTION,"Telescope motion flag"));
       currentHeader.addLine(new HeaderCard("WINDSCR",current.WINDSCR,"Wind screen"));
       currentHeader.addLine(new HeaderCard("ZA",current.ZA,"Zenith Angle"));
       currentHeader.addLine(new HeaderCard("ZTWILIGH",current.ZTWILIGH,"Zenith Twilight"));
       setCURRENT_HEADER(currentHeader);
       java.lang.String current_header_string =  headerToString(currentHeader);
       setCURRENT_FITS_HEADER_STRING(current_header_string);
//       current.CURRENT_HEADER = currentHeader;
       java.lang.String current_telemetry_string = telemetryToGson(current);
       setCURRENT_GSON_TELEMETRY(current_telemetry_string);
       System.out.println(CURRENT_FITS_HEADER_STRING);
    }catch(HeaderCardException e){ 
       logMessage(ERROR,"Error in method constructHeader "+e.toString());
       System.out.println(e.toString()); 
    }
  return CURRENT_FITS_HEADER_STRING;   
}
/*================================================================================================
/       setCURRENT_FITS_HEADER_STRING(java.lang.String new_value)
/=================================================================================================*/
public synchronized void setCURRENT_FITS_HEADER_STRING(java.lang.String new_value){
   CURRENT_FITS_HEADER_STRING = new_value; 
}
public synchronized java.lang.String getCURRENT_FITS_HEADER_STRING(){
    return CURRENT_FITS_HEADER_STRING;
}
/*================================================================================================
/       setCURRENT_FITS_HEADER_STRING(java.lang.String new_value)
/=================================================================================================*/
public synchronized void setCURRENT_GSON_TELEMETRY(java.lang.String new_value){
   CURRENT_GSON_TELEMETRY = new_value; 
}
public synchronized java.lang.String getCURRENT_GSON_TELEMETRY(){
    return CURRENT_GSON_TELEMETRY;
}
/*================================================================================================
/       setCURRENT_FITS_HEADER_STRING(java.lang.String new_value)
/=================================================================================================*/
public synchronized void setCURRENT_HEADER(Header current){
   CURRENT_FITS_HEADER = current; 
}
public Header getCURRENT_HEADER(){
   return CURRENT_FITS_HEADER;
}
/*================================================================================================
/       telemetryToGson(TelemetryObject current)
/=================================================================================================*/
public java.lang.String telemetryToGson(TelemetryObject current){
    Gson gson = new Gson();    
    java.lang.String gson_telemetry = gson.toJson(current);
    return gson_telemetry;
}
/*================================================================================================
/       headerToString(Header current_header)
/=================================================================================================*/
public java.lang.String headerToString(Header current_header){
    java.lang.String completeHeader = new java.lang.String();
    int number_of_cards = current_header.getNumberOfCards();
    String[] cards = new String[number_of_cards];
    int i=0; 
    try{
       Cursor iter = current_header.iterator();
       while(iter.hasNext()){
           HeaderCard current_card = (HeaderCard)iter.next();
           cards[i] = current_card.toString();
           i++;
       }
       for(int j=0;j<number_of_cards;j++){
         completeHeader = completeHeader+cards[j].toString()+TERMINATOR;  
       }
    }catch(Exception e){
       logMessage(ERROR,"Error in method headerToString "+e.toString());
//        System.out.println(e.toString());
    }
//    completeHeader = completeHeader+TERMINATOR;
  return completeHeader;
}
/*================================================================================================
/       constructTelemetryObject()
/=================================================================================================*/
public TelemetryObject constructTelemetryObject(){
    TelemetryObject current = new TelemetryObject();
    try{
        ZonedDateTime now     = ZonedDateTime.now();
        ZonedDateTime now_utc = now.withZoneSameInstant(ZoneId.of("UTC"));
//        long       now_utc_instant_milli = now_utc.toInstant().toEpochMilli();
//        long    now_utc_instant_milli = Instant.now().toEpochMilli();
        Timestamp current_timestamp = new Timestamp(now_utc.getYear()-1900,now_utc.getMonthValue()-1,now_utc.getDayOfMonth(),
                                                    now_utc.getHour(),now_utc.getMinute(),now_utc.getSecond(),0);

        current.UTC_TIMESTAMP = current_timestamp;
        current.LOCAL_DATE_TIME = now.toString();
        current.UTC_DATE_TIME   = now_utc.toString();
        System.out.println("Local Time = "+now.toString()+" UTC Time = "+now_utc.toString()+" Telescope UTCSTART = "+myTelescopeObject.getUTCSTART());
//        java.time.LocalDateTime localDate = java.time.LocalDateTime.parse(utcstart_string, formatter);
        current.UTCSTART     = myTelescopeObject.getUTCSTART();
        current.HTM_LEVEL_7  = myTelescopeObject.getHTMIndex_Level7();
        current.HTM_LEVEL_20 = myTelescopeObject.getHTMIndex_Level20();
//        current.UTCSTART = myTelescopeObject.getUTCSTART();
        current.AIRMASS = (float)myTelescopeObject.getAIRMASS();
        current.AZIMUTH = (float)myTelescopeObject.getAzimuth();
        current.CASSANG = (float)myTelescopeObject.getCASS_RING_ANGLE();
        current.DEC     = myTelescopeObject.getDeclination();
        current.DECOFF  = (float)myTelescopeObject.getDEC_OFFSET();
        current.DECTRAC = (float)myTelescopeObject.getDEC_TRACK_RATE();
        current.DOMEAZI =  (float)myTelescopeObject.getDomeAzimuth();
        current.DOMESHU = myTelescopeObject.getDomeShuttersString();
        current.EQUINOX = (float)myTelescopeObject.getEQUINOX();
        current.HA      = myTelescopeObject.getHourAngle();
        current.INSTRUMNT = myTelescopeObject.getInstrument();
        current.LST     = (float)myTelescopeObject.getLST();
        current.OBJECT  = myTelescopeObject.getObjectName();
        current.PUMPS   = myTelescopeObject.getPumps().trim();
        current.RA      = myTelescopeObject.getRightAscension();
        current.RAOFF   = (float)myTelescopeObject.getRA_OFFSET();
        current.RATEDEC = (float)myTelescopeObject.getDEC_TRACK_RATE();
        current.RATERA  = (float)myTelescopeObject.getRA_TRACK_RATE();
        current.TELFOCUS = (float)myTelescopeObject.getFOCUS();
        current.TELID    = myTelescopeObject.getTelescopeID();
        current.TMOTION  = myTelescopeObject.getMotionStatusString();
        current.WINDSCR  = myTelescopeObject.getWindscreenPositionString();
        current.ZA       = (float)myTelescopeObject.getZenithAngle();
        current.ALTITUDE = (float)myTelescopeObject.myJSkyCalcModel.getAltitude();
        current.ALTPARA  = (float)myTelescopeObject.myJSkyCalcModel.getAltParallacticDouble();
        current.BARYJD   = Double.parseDouble(myTelescopeObject.myJSkyCalcModel.getBaryjd());
        current.BARYVCOR = myTelescopeObject.myJSkyCalcModel.geBarytcort();
        current.CONSTEL  = myTelescopeObject.myJSkyCalcModel.getConstel();
        current.ELEVHOR  = myTelescopeObject.myJSkyCalcModel.getElevhoriz();
        current.ELEVSEA  = myTelescopeObject.myJSkyCalcModel.getElevsea_string();
        current.JD       = myTelescopeObject.myJSkyCalcModel.getJulianDate();
        current.LAT      = myTelescopeObject.myJSkyCalcModel.getLatitude();
        current.LONG     = myTelescopeObject.myJSkyCalcModel.getLongitude();
        current.MOONALT  = Float.parseFloat(myTelescopeObject.myJSkyCalcModel.getMoonalt());
        current.MOONAZ   = Float.parseFloat(myTelescopeObject.myJSkyCalcModel.getMoonaz());
        current.MOONDEC  = myTelescopeObject.myJSkyCalcModel.getMoondec();
        current.MOONLIG  = myTelescopeObject.myJSkyCalcModel.getMoonlight();
        current.MOONILM  = myTelescopeObject.myJSkyCalcModel.getMoonIllum();
        current.MOONPH   = myTelescopeObject.myJSkyCalcModel.getMoonphase();
        current.MOONRA   = myTelescopeObject.myJSkyCalcModel.getMoonra();
        current.PARA     = (float)myTelescopeObject.myJSkyCalcModel.getParallacticDouble();
        current.PLANETW  = myTelescopeObject.myJSkyCalcModel.getPlanetproxim();
        current.SUNALT   = Float.parseFloat(myTelescopeObject.myJSkyCalcModel.getSunalt());
        current.SUNAZ    = Float.parseFloat(myTelescopeObject.myJSkyCalcModel.getSunaz());
        current.SUNDEC   = myTelescopeObject.myJSkyCalcModel.getSundec();
        current.SUNRA    = myTelescopeObject.myJSkyCalcModel.getSunra();
        current.ZTWILIGH = myTelescopeObject.myJSkyCalcModel.getZtwilight(); 
        String moon_angle  = myTelescopeObject.myJSkyCalcModel.getMoonobjang().replace(" deg","");
        current.MOONANG  = Float.parseFloat(moon_angle);
    }catch(Exception e){
       logMessage(ERROR,"Error in method constructTelemetryObject "+e.toString());
//        System.out.println(e.toString());
    }
   return current;
}
   // The following parameters are calculated from the RA, DEC and time provided by the telescope. 
/*================================================================================================
/       TelescopeObject getTelescopeObject()
/=================================================================================================*/
public TelescopeObject getTelescopeObject(){
    return myTelescopeObject;
}
/*================================================================================================
/      initializeTelescopeState()
/=================================================================================================*/
  public void initializeTelescopeState(){
    java.lang.String response0 = new java.lang.String();
    java.lang.String response1 = new java.lang.String();
    java.lang.String response2 = new java.lang.String();
    response0 = myClientSocket.sendReceiveCommand("REQPOS"+TERMINATOR);
    parseREQPOS(response0);
    waitForResponseMilliseconds(pollingRate);
    response1 = myClientSocket.sendReceiveCommand("REQSTAT"+TERMINATOR);
    parseREQSTAT(response1);
    waitForResponseMilliseconds(pollingRate);
//    response2 = myClientSocket.sendReceiveCommand("WEATHER"+TERMINATOR);
    response2 = myClientSocket.sendReceiveCommand("?WEATHER"+TERMINATOR);
    parseWEATHER(response2);
  }
/*================================================================================================
/       setCoordinateStatusMessage(java.lang.String newCoordMessage)
/=================================================================================================*/
  public void setCoordinateStatusMessage(java.lang.String newCoordMessage) {
    String  oldCoordMessage = this.coordMessage;
    this.coordMessage = newCoordMessage;
    propertyChangeListeners.firePropertyChange("coord_message", oldCoordMessage, coordMessage);
  }
  public java.lang.String getCoordinateStatusMessage() {
    return coordMessage;
  }
/*================================================================================================
/      setOutputDirectory(java.lang.String new_output_directory)
/=================================================================================================*/
  public void setOutputDirectory(java.lang.String new_output_directory) {
    String  old_output_directory = this.output_directory;
    this.output_directory = new_output_directory;
    propertyChangeListeners.firePropertyChange("output_directory", old_output_directory, new_output_directory);
  }
  public java.lang.String getOutputDirectory() {
    return output_directory;
  }
/*================================================================================================
/       setPolling(boolean new_polling)
/=================================================================================================*/
  public synchronized void setPolling(boolean new_polling) {
    boolean  old_polling = this.polling;
    this.polling = new_polling;
    propertyChangeListeners.firePropertyChange("polling", Boolean.valueOf(old_polling), Boolean.valueOf(new_polling));
  }
  public synchronized boolean isPolling() {
    return polling;
  }
/*================================================================================================
/       setConnected(boolean connected)
/=================================================================================================*/
  public void setConnected(boolean connected) {
    boolean  oldConnected = this.connected;
    this.connected = connected;
    propertyChangeListeners.firePropertyChange("connected",Boolean.valueOf(oldConnected),Boolean.valueOf(connected));
  }
  public boolean isConnected() {
    return connected;
  }
/*================================================================================================
/       setStartTimestamp(java.lang.String new_start_timestamp)
/=================================================================================================*/
  public void setStartTimestamp(java.lang.String new_start_timestamp) {
    java.lang.String  old_start_timestamp = this.start_timestamp;
    this.start_timestamp = new_start_timestamp;
    propertyChangeListeners.firePropertyChange("start_timestamp", old_start_timestamp,new_start_timestamp);
  }
  public java.lang.String getStartTimestamp() {
    return start_timestamp;
  }
/*================================================================================================
/       setStartTimestamp(java.lang.String new_start_timestamp)
/=================================================================================================*/
  public void setEndTimestamp(java.lang.String new_end_timestamp) {
    java.lang.String  old_end_timestamp = this.end_timestamp;
    this.end_timestamp = new_end_timestamp;
    propertyChangeListeners.firePropertyChange("end_timestamp", old_end_timestamp,new_end_timestamp);
  }
  public java.lang.String getEndTimestamp() {
    return end_timestamp;
  }   
/*================================================================================================
/        connect()
/=================================================================================================*/
  public void connect(){
      myClientSocket.startConnection(ClientSocket.USE_HOSTNAME);
  } 
/*================================================================================================
/        disconnect()
/=================================================================================================*/
  public void disconnect(){
     myClientSocket.closeConnection();
     myUpdateTelescopeStatusThread.setUpdating(false);
  }
/*================================================================================================
/      startPolling()
/=================================================================================================*/
 public void startPolling(){
     myUpdateTelescopeStatusThread.start();
 }
/*================================================================================================
/      stopPolling()
/=================================================================================================*/
public void stopPolling(){
    myUpdateTelescopeStatusThread.setUpdating(false);
}
/*================================================================================================
/      waitForResponse(int newDelay)
/=================================================================================================*/
public void waitForResponseMilliseconds(int newDelay){
     try{
        Thread.currentThread().sleep(newDelay);} // sleep for awhile to let the messages pile up
      catch (Exception e){
    }
}
/*================================================================================================
/     stripUnits(java.lang.String unitString, in newUnitLength)
/=================================================================================================*/
public java.lang.String stripUnits(java.lang.String unitString,int newUnitLength){
        int unitLength = newUnitLength;
 // NOT FINISHED THIS IS NOT IMPLEMENTED YET
       int length = unitString.length();
       java.lang.String result = unitString.substring(0, length - unitLength);
     return result;
}
/*================================================================================================
/     parseWEATHER(java.lang.String currentResponse)
/=================================================================================================*/
public java.lang.String stripLabel(java.lang.String labeledString){
       java.util.StringTokenizer tokenResponseString = new java.util.StringTokenizer(labeledString,"=");
       java.lang.String          result         = tokenResponseString.nextToken("=");
       result = tokenResponseString.nextToken();
    return result;
}
/*================================================================================================
/     parseResponse(java.lang.String currentResponse)
/=================================================================================================*/
 public java.lang.String parseResponse(java.lang.String currentResponse){
//	"0":  Successful completion.
//	"-1": Unrecognized command.
//	"-2": Invalid parameter(s).
//	"-3": Unable to execute at this time.
//	"-4": TCS Sparc host unavailable.
     java.lang.String message = new java.lang.String();
     if("0".regionMatches(0, currentResponse, 0, 1)){ // Request telescope position
        message = "Successful completion";
        myTelescopeObject.setStatusMessage(message);
//        System.out.println(message);
     }    
     if("-1".regionMatches(0, currentResponse, 0, 2)){ // Request telescope position
        message = "Unrecognized command";
        myTelescopeObject.setStatusMessage(message);
//        System.out.println(message);
     }
     if("-2".regionMatches(0, currentResponse, 0, 2)){ // Request telescope position
        message = "Invalid parameter(s)";
        myTelescopeObject.setStatusMessage(message);
//        System.out.println(message);
     }
     if("-3".regionMatches(0, currentResponse, 0, 2)){ // Request telescope position
        message = "Unable to execute at this time";
        myTelescopeObject.setStatusMessage(message);
//        System.out.println(message);
     }
     if("-4".regionMatches(0, currentResponse, 0, 2)){ // Request telescope position
        message = "TCS Sparc host unavailable";
        myTelescopeObject.setStatusMessage(message);
//        System.out.println(message);
     }
   return message;
 }
 /*================================================================================================
/     parseREQPOS(java.lang.String currentResponse)
/=================================================================================================*/
 public void parseREQPOS(java.lang.String currentResponse){
   // UTC = 216 04:34:38.1, LST = 17:37:45.4
   // RA = 17:57:57.69, DEC = +04:43:11.8, HA = E00:20:45.8
   // air mass =  1.143                                                                                                                                                  
      try{
       java.util.StringTokenizer tokenResponseString = new java.util.StringTokenizer(currentResponse,"=");
       java.lang.String          UTCString         = tokenResponseString.nextToken(",");
       java.lang.String          LSTString         = tokenResponseString.nextToken("\n");
       java.lang.String          RAString          = tokenResponseString.nextToken(",");
       java.lang.String          DECString         = tokenResponseString.nextToken(",");
       java.lang.String          HAString          = tokenResponseString.nextToken("\n");
       java.lang.String          AirMassString     = tokenResponseString.nextToken("\r");
       UTCString = stripLabel(UTCString);
       LSTString = stripLabel(LSTString);
       RAString = stripLabel(RAString);
       DECString = stripLabel(DECString);
       RAString  = RAString.trim();
       DECString = DECString.trim();
       HAString = stripLabel(HAString);
       AirMassString = stripLabel(AirMassString);
       AirMassString.substring(0, 5);
       myTelescopeObject.setUTCSTART(UTCString);
       myTelescopeObject.setSidereal(LSTString);
       myTelescopeObject.setRightAscension(RAString);
       myTelescopeObject.setDeclination(DECString);
       myTelescopeObject.setHourAngle(HAString);
//       myTelescopeObject.setAirMass(AirMassString);
//       double myRA = parseSexag(myRAString);
//       double myDec = parseSexag(myDecString);

     }catch(Exception e2){
       logMessage(ERROR,"Error parsing coordinates for a position. parseREQPOS"+e2.toString());
//       System.out.println("Error parsing coordinates for a position. parseREQPOS");
     }
 }
/*================================================================================================
/     parseREQSTAT(java.lang.String currentResponse)
/=================================================================================================*/
 public void parseREQSTAT(java.lang.String currentResponse){
              //  telescope ID = 200, focus = 36.71 mm, tube length = 22.11 mm
              //  offset RA =     137.3 arcsec, DEC =      95.8 arcsec
              //  rate RA =       0.0 arcsec/hr, DEC =       0.0 arcsec/hr
              //  Cass ring angle =  49.35                                        
     try{
       java.util.StringTokenizer tokenResponseString = new java.util.StringTokenizer(currentResponse,"=");
       java.lang.String          UTCString                 = tokenResponseString.nextToken("\n");
       java.lang.String          TelescopeIDString         = tokenResponseString.nextToken(",");
       java.lang.String          FocusString               = tokenResponseString.nextToken(",");
       java.lang.String          TubeLengthString          = tokenResponseString.nextToken("\n");
       java.lang.String          OffsetRAString            = tokenResponseString.nextToken(",");
       java.lang.String          OffsetDECString           = tokenResponseString.nextToken("\n");
       java.lang.String          RateRAString              = tokenResponseString.nextToken(",");
       java.lang.String          RateDECString             = tokenResponseString.nextToken("\n");
       java.lang.String          CassRingAngleString       = tokenResponseString.nextToken("\r");
//  Strip the label from the string
       TelescopeIDString   = stripLabel(TelescopeIDString);

       FocusString         = stripLabel(FocusString);
       FocusString         = stripUnits(FocusString,3);

       TubeLengthString    = stripLabel(TubeLengthString);
       TubeLengthString    = stripUnits(TubeLengthString,3);
       
       OffsetRAString      = stripLabel(OffsetRAString);
       OffsetRAString      = stripUnits(OffsetRAString,7);

       OffsetDECString     = stripLabel(OffsetDECString);
       OffsetDECString     = stripUnits(OffsetDECString,7);

       RateRAString        = stripLabel(RateRAString);
       RateRAString        = stripUnits(RateRAString,10);

       RateDECString       = stripLabel(RateDECString);
       RateDECString       = stripUnits(RateDECString,10);

       CassRingAngleString = stripLabel(CassRingAngleString);
       CassRingAngleString = CassRingAngleString.trim();
//       CassRingAngleString = CassRingAngleString.substring(0,6);

   // Update the Telescope Object
       myTelescopeObject.setTelescopeID(TelescopeIDString);
       myTelescopeObject.setTelescopeFocus(FocusString);
       myTelescopeObject.setTelescopeTubeLength(TubeLengthString);
       myTelescopeObject.setRightAscensionOffset(OffsetRAString);
       myTelescopeObject.setDeclinationOffset(OffsetDECString);
       myTelescopeObject.setRightAscensionTrackRate(RateRAString);
       myTelescopeObject.setDeclinationTrackRate(RateDECString);
       myTelescopeObject.setCassRingAngle(CassRingAngleString);
       myTelescopeObject.updateOffsetCoordinates();
//       double myRA = parseSexag(myRAString);
//       double myDec = parseSexag(myDecString);
       java.lang.String test = new java.lang.String();
     }catch(Exception e2){
       logMessage(ERROR,"Error parsing coordinates for a position. parseREQSTAT"+e2.toString());
//       System.out.println("Error parsing coordinates for a position. parseREQSTAT");
     }
 }
/*================================================================================================
/     parseWEATHER(java.lang.String currentResponse)
/=================================================================================================*/
 public void parseName(java.lang.String currentResponse){
        myTelescopeObject.setObjectName(currentResponse.trim());
 }
 /*================================================================================================
/     parseWEATHER(java.lang.String currentResponse)
/=================================================================================================*/
 public void parseWEATHER(java.lang.String currentResponse){
          // Example:
          // RA=17.97
          // Dec=+4.72
          // HA=-0.35
          // LST=17.63
          // Air mass=01.143
          // Azimuth=169.37
          // Zenith angle=29.00
          // Focus Point=36.71
          // Dome Azimuth=164.9
          // Dome shutters=1
          // Windscreen position=09.17
          // InstPos=1
          // Instrument=3SPEC
          // Pumps=0            
      try{
       java.util.StringTokenizer tokenResponseString = new java.util.StringTokenizer(currentResponse,"=");
       java.lang.String          RAString           = tokenResponseString.nextToken("\n");
       java.lang.String          DECString          = tokenResponseString.nextToken("\n");
       java.lang.String          HAString           = tokenResponseString.nextToken("\n");
       java.lang.String          LSTString          = tokenResponseString.nextToken("\n");
       java.lang.String          AirMassString      = tokenResponseString.nextToken("\n");
       java.lang.String          AzimuthString      = tokenResponseString.nextToken("\n");
       java.lang.String          ZenithAngleString  = tokenResponseString.nextToken("\n");
       java.lang.String          FocusPointString   = tokenResponseString.nextToken("\n");
       java.lang.String          DomeAzimuthString  = tokenResponseString.nextToken("\n");
       java.lang.String          DomeShuttersString = tokenResponseString.nextToken("\n");
       java.lang.String          WindScreenString   = tokenResponseString.nextToken("\n");
        java.lang.String         InstPosString      = tokenResponseString.nextToken("\n");
       java.lang.String          InstrumentString   = tokenResponseString.nextToken("\n");
       java.lang.String          PumpsString        = tokenResponseString.nextToken("\r");
       // Strip the labels from the values.
// These parameters are available in higher precision elsewhere and should not be use for updating the model
       RAString           = stripLabel(RAString);
       DECString          = stripLabel(DECString);
       HAString           = stripLabel(HAString);
       LSTString          = stripLabel(LSTString);
       AirMassString      = stripLabel(AirMassString);
       FocusPointString   = stripLabel(FocusPointString);
// The following parameters are only available from the WEATHER command and are used to update the model
       AzimuthString      = stripLabel(AzimuthString);
       ZenithAngleString  = stripLabel(ZenithAngleString);
       DomeAzimuthString  = stripLabel(DomeAzimuthString);
       DomeShuttersString = stripLabel(DomeShuttersString);
       WindScreenString   = stripLabel(WindScreenString);
       InstPosString      = stripLabel(InstPosString);
       InstrumentString   = stripLabel(InstrumentString);
       PumpsString        = stripLabel(PumpsString);
//   Set the actual parameters in the model
       myTelescopeObject.setAirMass(AirMassString);
       myTelescopeObject.setAzimuthString(AzimuthString);
       myTelescopeObject.setZenithAngleString(ZenithAngleString);
       myTelescopeObject.setDomeAzimuthString(DomeAzimuthString);
       myTelescopeObject.setDomeShuttersString(DomeShuttersString);
       myTelescopeObject.setWindscreenPositionString(WindScreenString);
       myTelescopeObject.setInstrumentPositionString(InstPosString);
       myTelescopeObject.setInstrument(InstrumentString);
       myTelescopeObject.setPumps(PumpsString);     // Not yet implemented on the telescope side
     }catch(Exception e2){
       logMessage(ERROR,"Error parsing coordinates for a position. parseWEATHER"+e2.toString());
//       System.out.println("Error parsing coordinates for a position. parseWEATHER");
     }
 }
/*=============================================================================================
/    parseMotion(java.lang.String currentResponse)
/=============================================================================================*/
 public int parseMotion(java.lang.String currentResponse){
     int motion = TelescopeObject.MOTION_UNKNOWN; 
     try{
         currentResponse = currentResponse.trim();
         motion = Integer.parseInt(currentResponse);
         myTelescopeObject.setMotionStatus(motion);
         if(motion == TelescopeObject.MOTION_STOPPED){
             myTelescopeObject.setMotionStatusString("STOPPED");
         }
         if(motion == TelescopeObject.MOTION_SLEWING){
             myTelescopeObject.setMotionStatusString("SLEWING");
         }
         if(motion == TelescopeObject.MOTION_OFFSETTING){
             myTelescopeObject.setMotionStatusString("OFFSETTING");
         }
         if(motion == TelescopeObject.MOTION_TRACKING_STABLY){
             myTelescopeObject.setMotionStatusString("TRACKING STABLY");
         }
         if(motion == TelescopeObject.MOTION_SETTLING){
             myTelescopeObject.setMotionStatusString("SETTLING");
         }      
     }catch(Exception e){
        myClientSocket.setConnected(false);
     }
  return motion;
 }
 /*=============================================================================================
/                            Log Message Methods
/      public void logErrorMessage(java.lang.String newMessage)
/=============================================================================================*/
  public void logErrorMessage(java.lang.String newMessage){
    System.out.println(newMessage);
  }// end of the logErrorMessage

/*================================================================================================
/      add and remove Property Change Listeners
/=================================================================================================*/
  public synchronized void removePropertyChangeListener(PropertyChangeListener l) {
    propertyChangeListeners.removePropertyChangeListener(l);
  }
  public synchronized void addPropertyChangeListener(PropertyChangeListener l) {
    propertyChangeListeners.addPropertyChangeListener(l);
  } 
/*=============================================================================================
/       INNER CLASS that polls the telescope for it's position and status
/=============================================================================================*/
    public class UpdateTelescopeStatusThread  implements Runnable{
    private Thread myThread;
    private int    pollingRate = 5;
    private boolean state      = false;
    private java.lang.String response = new java.lang.String();
/*================================================================================================
/      waitForResponse(int newDelay)
/=================================================================================================*/
public void waitForResponseMilliseconds(int newDelay){
     try{
        Thread.currentThread().sleep(newDelay);} // sleep for awhile to let the messages pile up
      catch (Exception e){
    }
}
public synchronized void setUpdating(boolean newState){
    state = newState;
    if(!state){
        setPolling(false);
    }
}
/*=============================================================================================
/          UpdateTelescopeStatusThread()
/=============================================================================================*/
    private UpdateTelescopeStatusThread(int newPollingRate){
       pollingRate = newPollingRate;
    }
/*=============================================================================================
/           run()
/=============================================================================================*/
    public void run(){
     state = true;
    initializeTelescopeState();    // Update the value of all the parameters before starting to poll the telescope
    int depth_7  = 7;
    int depth_20 = 20;
    Vector3d vec;
    HTMindexImp si_7  = new HTMindexImp(depth_7,5);
    HTMindexImp si_20 = new HTMindexImp(depth_20,5);

    while(state){
      setPolling(true);
       if(myClientSocket.isConnected()){
         long start = System.currentTimeMillis();
         response = myClientSocket.sendReceiveCommand("REQPOS"+TERMINATOR);
         parseREQPOS(response);
         response = myClientSocket.sendReceiveCommand("?MOTION"+TERMINATOR);
         parseMotion(response);
         response = myClientSocket.sendReceiveCommand("REQSTAT"+TERMINATOR);
         parseREQSTAT(response);
         response = myClientSocket.sendReceiveCommand("?NAME"+TERMINATOR);
         parseName(response);
         response = myClientSocket.sendReceiveCommand("?WEATHER"+TERMINATOR);     // For use with the real P200
         parseWEATHER(response);
          myTelescopeObject.getJSkyCalcModel().setTelescopePosition(myTelescopeObject.getRightAscension(), myTelescopeObject.getDeclination(), myTelescopeObject.getEquinox());
          if(myTelescopeObject.getJSkyCalcModel().getTimeSource() == JSkyCalcModel.NOW){
             myTelescopeObject.getJSkyCalcModel().SetToNow();           
          }
          if(myTelescopeObject.getJSkyCalcModel().getTimeSource() == JSkyCalcModel.SELECTED_TIME){
            java.lang.String[] selected_date_time = myTelescopeObject.getJSkyCalcModel().getSelectedDateTime();
            myTelescopeObject.getJSkyCalcModel().setToDate(selected_date_time[0], selected_date_time[1]);
          } 
          try{
             vec = new Vector3d(myTelescopeObject.getRA(),myTelescopeObject.getDEC());// initialize Vector3d from ra,dec
             long spt_ind7  =si_7.lookupId(vec);
             long spt_ind20 =si_20.lookupId(vec);  
             myTelescopeObject.setHTMIndex_Level7(spt_ind7);
             myTelescopeObject.setHTMIndex_Level20(spt_ind20);
          }catch(Exception e){
             logMessage(ERROR,"Error in UpdateTelescopeStatusThread error 1"+e.toString());
//             System.out.println(e.toString()); 
          } 
          CURRENT_TELEMETRY = constructTelemetryObject();
          constructHeader(CURRENT_TELEMETRY);
          executeInsertStatement(CURRENT_TELEMETRY);
          if(!state){
              setPolling(false);
              return;
          }
          waitForResponseMilliseconds(500);
          long end = System.currentTimeMillis();
          long duration = end - start;
          System.out.println("Update Time = "+duration);
       }  // if connected 
     }
      return;
    }
    public void start(){
     myThread = new Thread(this);
     myThread.start();
     }
    }// End of the UpdateTelescopeStatusThread Inner Class
/*=============================================================================================
/                           End of the UpdateTelescopeStatusThread Class
/=============================================================================================*/
/*=============================================================================================
/       INNER CLASS that polls the telescope for it's position and status
/=============================================================================================*/
    public class UpdateEphemerisThread  implements Runnable{
    private Thread myThread;
    private boolean state      = false;
    private java.lang.String response = new java.lang.String();
/*================================================================================================
/      waitForResponse(int newDelay)
/=================================================================================================*/
public void waitForResponseMilliseconds(int newDelay){
     try{
        Thread.currentThread().sleep(newDelay);} // sleep for awhile to let the messages pile up
      catch (Exception e){
    }
}
/*=============================================================================================
/          setUpdating(boolean newState)
/=============================================================================================*/
public synchronized void setUpdating(boolean newState){
    state = newState;
}
public synchronized boolean isUpdating(){
    return state;
}
/*=============================================================================================
/          UpdateEphemerisThread()
/=============================================================================================*/
    private UpdateEphemerisThread(){
    }
/*=============================================================================================
/           run()
/=============================================================================================*/
    public void run(){
       state = true;
       while(state){
         myTelescopeObject.getJSkyCalcModel().setTelescopePosition(myTelescopeObject.getRightAscension(), myTelescopeObject.getDeclination(), myTelescopeObject.getEquinox());
         if(myTelescopeObject.getJSkyCalcModel().getTimeSource() == JSkyCalcModel.NOW){
             myTelescopeObject.getJSkyCalcModel().SetToNow();           
         }
         if(myTelescopeObject.getJSkyCalcModel().getTimeSource() == JSkyCalcModel.SELECTED_TIME){
            java.lang.String[] selected_date_time = myTelescopeObject.getJSkyCalcModel().getSelectedDateTime();
            myTelescopeObject.getJSkyCalcModel().setToDate(selected_date_time[0], selected_date_time[1]);
         }
       }
       state = false;
       return;
    }
/*=============================================================================================
/         start()
/=============================================================================================*/
    public void start(){
     myThread = new Thread(this);
     myThread.start();
     }
    }// End of the UpdateEphemerisThread Inner Class
/*=============================================================================================
/                           End of the UpdateEphemerisThread Class
/=============================================================================================*/
/*=============================================================================================
/       INNER CLASS that polls the telescope for it's position and status
/=============================================================================================*/
    public class ExecuteQueryThread  implements Runnable{
    private Thread myThread;
    private int QUERY_MODE;
    private int QUERY_TIMESTAMP = 1;
    private int QUERY_RANGE     = 2;
    private java.lang.String start_timestamp;
    private java.lang.String end_timestamp;
/*=============================================================================================
/          ExecuteQueryThread()
/=============================================================================================*/
    private ExecuteQueryThread(java.lang.String new_start_timestamp){
        QUERY_MODE = QUERY_TIMESTAMP;
        start_timestamp = new_start_timestamp;
    }
    private ExecuteQueryThread(java.lang.String new_start_timestamp,java.lang.String new_end_timestamp){
        QUERY_MODE = QUERY_RANGE;
        start_timestamp = new_start_timestamp;
        end_timestamp   = new_end_timestamp;
    }
/*=============================================================================================
/           run()
/=============================================================================================*/
    public void run(){
        if(QUERY_MODE == QUERY_TIMESTAMP){
            myTelemetryTableModel.clearTable();
            ArrayList current = query(start_timestamp);
            int count = current.size();
            for(int i=0;i<count;i++){
               TelemetryObject current_telem = (TelemetryObject)current.get(i);
               myTelemetryTableModel.addRecord(current_telem);
            }
        }
        if(QUERY_MODE == QUERY_RANGE){
            myTelemetryTableModel.clearTable();
            ArrayList current = query_range(start_timestamp,end_timestamp);
            int count = current.size();
            for(int i=0;i<count;i++){
               TelemetryObject current_telem = (TelemetryObject)current.get(i);
               myTelemetryTableModel.addRecord(current_telem);
            }
        }
       return;
    }
/*=============================================================================================
/         start()
/=============================================================================================*/
    public void start(){
     myThread = new Thread(this);
     myThread.start();
     }
    }// End of the UpdateEphemerisThread Inner Class
/*=============================================================================================
/                           End of the UpdateEphemerisThread Class
/=============================================================================================*/
/*=============================================================================================
/       INNER CLASS that polls the telescope for it's position and status
/=============================================================================================*/
    public class ExecuteMonitorConnectionThread  implements Runnable{
    private Thread myThread;
 /*=============================================================================================
/         ExecuteMonitorConnectionThread()
/=============================================================================================*/
    private ExecuteMonitorConnectionThread(){
    }
/*=============================================================================================
/           run()
/=============================================================================================*/
    public void run(){
        monitorConnection();
    }
/*=============================================================================================
/         start()
/=============================================================================================*/
    public void start(){
     myThread = new Thread(this);
     myThread.start();
     }
    }// End of the UpdateEphemerisThread Inner Class
/*=============================================================================================
/                           End of the UpdateEphemerisThread Class
/=============================================================================================*/
}



/*
REMOTE COMMANDS FOR THE PALOMAR TELESCOPE CONTROL SYSTEMS (TCS)    10/07/08


OVERVIEW

Remote commands are those commands that can be issued by another computer via
serial link or stream socket to the Telescope Control System (TCS).  The
commands and their functions are listed below.

The command format is an ASCII string starting with the command name, followed
by the correct number of arguments and terminated with a '\r' (carriage-return
character, 015 octal).  Spaces are used to delimit fields.  Numeric arguments
may be integer or floating-point.  Commands may be entered in upper or lower
case.



REMOTE COMMANDS

(null command, '\r' only):  Get-header command.  (See also REQPOS and REQSTAT
commands below.)  Assembles current telescope data as native DEC (Digital
Equipment Corp) binary numbers and returns to the caller 100 8-bit bytes.
These are NOT ASCII characters, and must be interpreted as PDP11-format
numbers ("shorts" are two byte short integer, LSB first;  and "floats" are
modified IEEE format: MSB-1, MSB, LSB, LSB+1.  In other words, each 16-bit
word is byte-swapped).  A "C" declaration for the data structure that is
returned is provided below:

struct {				/* 100 BYTES TOTAL */ /*
        short 	ra_hours;		/* COORDINATES ALWAYS J2000.0 */
/*	short	ra_minutes;
	short	ra_seconds;
	short	ra_hundredth_seconds;
	short	dummyword_4;
	short	dec_degrees;
	short	dec_minutes;
	short	dec_seconds;
	short	dec_tenth_seconds;
	short	dec_sign;		/* 1 = POS, 0 = NEG */
/*	short	lst_hours;
	short	lst_minutes;
	short	lst_seconds;
	short	lst_tenth_seconds;
	short	dummyword_14;
	short	ha_hours;		/* HA IS APPARENT */
/*	short	ha_minutes;
	short	ha_seconds;
	short	ha_tenth_seconds;
	short	ha_direction;		/* 1 = EAST, 0 = WEST */
/*	short	ut_hours;
	short	ut_minutes;
	short	ut_seconds;
	short	ut_tenth_seconds;
	float	airmass;
	float	display_equinox;	/* J2000.0 */
/*	short	dummyword_28;
	short	dummyword_29;
	short	dummyword_30;
	short	dummyword_31;
	short	dummyword_32;
	short	dummyword_33;
	float	cass_ring_angle;
	float	telescope_focus_mm;
	float	telescope_tubelength_mm;
	float	ra_offset_arcsec;
	float	dec_offset_arcsec;
	float	ra_track_rate_arcsec;
	float	dec_track_rate_arcsec;
	short	dummyword_48;
	short	telescope_id;		/* 200 or 60 */
/*} telescope_data;

?NAME (no arguments): return the name of the currently observed position
(the last position for which a GO command has been issued).

?PARALLACTIC (no arguments):  return the current telescope parallactic
angle in ASCII format, as follows: "PARALLACTIC = %.2f\n".  This is intended
for the Double Spectrograph instrument.

?WEATHER (no arguments): does not provide weather information!  Was created
to provide telescope information to the P200 weather system, acting as a
client.  The format of the returned string is:
----------------------------------------------------------------------------
RA=%05.2f\nDec=%+.2f\nHA=%+.2f\nLST=%05.2f\nAir mass=%06.3f\nAzimuth=%06.2f\n\
Zenith angle=%05.2f\nFocus Point=%05.2f\nDome Azimuth=%05.1f\n\
Dome shutters=%1d\nWindscreen position=%05.2f\nInstPos=%1d\nInstrument=%.12s\n\
Pumps=%1d\n
----------------------------------------------------------------------------
Apart from low precision telescope position information, the command indicates
whether the dome shutters are closed (0) or open (1), dome and windscreen
positions, and instrument name.

COORDS (five or six arguments):  supply a position to the TCS.  The arguments
in order are  1) RA in decimal hours, 2) declination in decimal degrees,
3) equinox of coordinates (value of zero indicates apparent equinox), 4) RA
motion, 5) dec motion, 6) motion flag.  (Motion flag absent or 0:  arguments
4 and 5 are proper motion in RA units of .0001 sec/yr and dec units of .001
arcsec/yr.  Motion flag = 1:  arguments 4 and 5 are offset tracking rates in
arcsec/hr.  Motion flag = 2:  arguments 4 and 5 are offset tracking rates
with arg 4 in seconds/hr and arg 5 in arcsec/hr.)  NOTE: a name string can
now be provided, by adding an extra parameter enclosed in double quotes;
this will preferably be at the end of the command line, though it can be
inserted at any parameter position.

E (one argument):  move telescope east by number of arcseconds given in
argument.  Rate of move is set by first argument of MRATES - see below.
Move is precessed to display equinox.  Valid range is 0 to 6000 arcsec.

ES (one argument):  move telescope east by seconds of time given in
argument.  Rate of move is set by first argument of MRATES - see below.
Move is precessed to display equinox.  Valid range: 0 to 600 seconds.

F (no arguments):  move the telescope to the position established by the last
GO command (GO can only be issued from the Night Assistant's console).  This
move is done at 50 arcsec/sec.  Maximum move is 600 sec in RA and 6000 arcsec
in dec.  The telescope must be tracking stably.

FOCUSGO (one argument):  change telescope focus to the value given in the
argument.  Valid range: 1.00 to 74.00 mm. (P200 only)

FOCUSINC (one argument):  change telescope focus by the offset given in the
argument.  Valid range: -73.00 to 73.00 mm.  Limit checking is done.
(P200 only)

GHEAD (one argument):  for instruments with internal mechanisms that
affect the orientation of North on the video guider display, this command
provides a way for the instrument computer to notify the TCS of changes
in orientation.  Valid range: 0 to 360 degrees.

GUIDE (two arguments):  set the RA and dec rates for the handpaddle Guide
buttons.  Valid range: 0 to 50 arcsec/sec.

MRATES (two arguments):  set rate of RA and dec moves to values given in
first and second arguments, respectively.  Valid range: 0 to 50 arcsec/sec.
Note:  these rates apply to moves as described in this note and are
independent of rates for Night-Assistant-commanded moves.

N (one argument):  move telescope north by number of arcseconds given in
argument.  Rate of move is set by second argument of MRATES - see above.
Move is precessed to display equinox.  Valid range: 0 to 6000 arcsec.

NPS (two arguments): control power to lamps and LWIR camera in prime focus
cage.  First argument: 0 = turn off port given in second argument; 1 =
turn on port; 2 = get status of port ("OFF" or "ON ").  Special case:
NPS 0 0 turns off all lamps (ports 1, 2, 3).  Port allocations: 1 = low lamp;
2 = high lamp; 3 = arclamp; 7 = LWIR camera; 8 = LWIR camera shutter.

PT (two arguments):  move telescope in RA and dec simultaneously by
distance given in first and second arguments, respectively.  Distances are
in arcseconds.  Positive values move east and north.  Rates of moves are
set by MRATES - see above.  Moves are precessed to the display equinox.
Valid range: -6000 to 6000 arcsec in each axis.

PTS (two arguments):  move telescope in RA and dec simultaneously by
distance given in first and second arguments, respectively.  RA distance is
in seconds of time (range: -600 to 600 sec), dec is in arcseconds (range:
-6000 to 6000 arcsec).  Positive values move east and north.  Rates of moves
are set by MRATES - see above.  Moves are precessed to the display equinox.

R (no arguments):  turn on offset tracking rates (non-sidereal).  The rates to
be used may be entered at the Night Assistant's console or with the RATES or
RATESS commands - see below.

-R (no arguments):  turn off offset tracking rates.

RATES (two arguments):  define offsets to sidereal tracking rates for non-
sidereal objects.  First argument is RA rate in arcsec/hr, second argument
is declination rate in arcsec/hr.  Rates are enabled with the R command -
see above.

RATESS (two arguments):  define offsets to sidereal tracking rates for non-
sidereal objects.  First argument is RA rate in seconds/hr, second argument
is declination rate in arcsec/hr.  Rates are enabled with the R command -
see above.

RAWDEC (no arguments):  returns the encoder value of declination in ASCII
format, as follows: "RAW_DEC = %.4f\n".  This is intended for LGS operations.

RAWPOS (no arguments):  returns the encoder values of hour angle (in hours)
and declination (in degrees) in ASCII format, as follows:
"HA= %.6f\nDEC= %.5f\n".  This is intended for LGS operations.

RCLEAR (no arguments):  disable offset tracking rates and set offset rates
to zero.

REQPOS (no arguments):  obtain telescope position information in ASCII
format.  Format of returned string (dashed lines not included):
----------------------------------------------------------------------------
UTC = ddd hh:mm:ss.s, LST = hh:mm:ss.s\n
RA = hh:mm:ss.ss, DEC = [+/-]dd:mm:ss.s, HA = [W/E]hh:mm:ss.s\n
air mass = aa.aaa
----------------------------------------------------------------------------
In UTC, ddd is UT day of year.  Fields are fixed width.  RA and DEC are
J2000.  HA is apparent.  There is no terminating newline.  (See also Get-
header command above.)

REQSTAT (no arguments): obtain telescope status information in ASCII
format.  Format of returned string (dashed lines not included):
----------------------------------------------------------------------------
UTC = ddd hh:mm:ss.s\n
telescope ID = ttt, focus = ***** mm, tube length = tt.tt mm\n
offset RA = rrrrrrr.r arcsec, DEC = ddddddd.d arcsec\n
rate RA = rrrrrrr.r arcsec/hr, DEC = ddddddd.d arcsec/hr\n
Cass ring angle = ccc.cc
----------------------------------------------------------------------------
In UTC, ddd is UT day of year.  ID is 200 or 60.  At P200, focus format
is ff.ff mm;  at P60 the focus reading is fffff and is in arbitrary units
(not mm, although "mm" is displayed).  Tube length and Cass ring angle are
meaningless at P60.  Fields are fixed width.  There is no terminating
newline.  (See also Get-header command above.)

RET (no arguments):  move the telescope so the DRA and DDEC offsets on the
telescope status display go to zero.  (See Z command, below.)  Rate is
50 arcsec/sec.  Maximum move: 600 sec in RA, 6000 arcsec in dec.  The
telescope must be tracking stably.

RINGGO (one argument):  rotate the Cass ring to the position specified in
the argument, in degrees.  If the position is available in more than one
turn of the ring, the shortest move will be taken.  Valid range: 0 to
359.99... degrees.

RINGMOVE (one argument):  rotate the Cass ring by the displacement specified
in the argument, in degrees.  Valid range: depends on current position with
respect to limits.  If out of range, "-3" (unable to execute) is returned.

S (one argument):  move telescope south by number of arcseconds given in
argument.  Rate of move is set by second argument of MRATES - see above.
Move is precessed to display equinox.  Valid range: 0 to 6000 arcsec.

SET (two arguments):  set the RA and dec rates for the handpaddle Set
buttons.  Valid range: 0 to 50 arcsec/sec.

TX (no arguments):  define the current position of the telescope to be the
position established with the last GO command (operator command only).
Similar to X -- see below, but pointing offsets are not saved, and the
operator can restore the pointing offsets associated with the last X.
DISABLED-- not available to clients; telescope operator only.

W (one argument):  move telescope west by number of arcseconds given in
argument.  Rate of move is set by first argument of MRATES - see above.
Move is precessed to display equinox.  Valid range: 0 to 6000 arcsec.

WS (one argument):  move telescope west by seconds of time given in
argument.  Rate of move is set by first argument of MRATES - see above.
Move is precessed to display equinox.  Valid range: 0 to 600 seconds.

X (no arguments):  define the current position of the telescope to be the
position established with the last GO command (operator command only).
Normally done after GOing to a star with known good coordinates and centering
the star in the field, to optimize local pointing.  X zeroes the displayed
DRA and DDEC offsets.  Valid range:  change in pointing offsets less than
10 sec in RA and 300 arcsec in dec.  NOTE: this command must be used with
care -- if used at the wrong position, subsequent telescope pointing will be
bad.  DISABLED-- not available to clients; telescope operator only.

Z (no arguments):  zero the DRA and DDEC display offsets;  ie, make the
current position of the telescope a reference point.



RETURNS

The get-header command and the ?NAME, ?PARALLACTIC, ?WEATHER, RAWDEC, RAWPOS,
REQPOS, and REQSTAT commands return the requested information.  All other
commands return an ASCII string to show completion status.  (Motion commands
do not return until motion is complete.)  All ASCII strings have a null
character appended.  The meaning of return strings is as follows:

	"0":  Successful completion.
	"-1": Unrecognized command.
	"-2": Invalid parameter(s).
	"-3": Unable to execute at this time.
	"-4": TCS Sparc host unavailable.



COMMUNICATIONS PROTOCOLS

SERIAL PROTOCOL:  9600 bps, 1 start bit, 8 data bits, 1 stop bit, no parity.
No handshaking.

STREAM SOCKET PARAMETERS:
TCS IP address (P200): 198.202.125.194 (proxy address)
TCP service (port): 5004
[A sample client program, written in C, is available.]
*/