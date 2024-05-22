/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package edu.caltech.palomar.telescopes.telemetry;
import edu.caltech.palomar.io.ClientSocket;
import java.io.FileInputStream;
import java.sql.Connection;
import java.sql.DriverManager;
import java.util.Properties;
import java.util.Calendar;
import java.util.TimeZone;
import nom.tam.fits.Header;
import nom.tam.fits.HeaderCard;
import java.util.ArrayList;
import java.sql.ResultSet;
import edu.caltech.palomar.telescopes.telemetry.WeatherController;
import edu.caltech.palomar.telescopes.telemetry.P48WeatherObject;
/**
 *
 * @author developer
 */
public class TestClient {
    public ClientSocket      myClientSocket;
    public ClientSocket      myWeatherClientSocket;
    public WeatherController myWeatherController;
    
    private java.lang.String     TERMINATOR                = new java.lang.String("\n");
    private java.lang.String     SEP             = System.getProperty("file.separator");
    public java.lang.String      USERDIR         = System.getProperty("user.dir");
    private java.lang.String     CONFIG          = new java.lang.String("config");
    public  java.lang.String     DBMS_CONNECTION_PROPERTIES = "telemetry_dbms.ini";
    public Connection            conn = null;
    public TestClient(){
       initializeClient(); 
//       String result = getHeader();
//       String result = query_timestamp("2021-07-14 17:40:24");
       String result = query_timestamp_range("2021-07-14 17:40:24","2021-07-14 17:41:24");
 //      String result = getWeather();
//       String result = getWeatherJSON();
       System.out.println(result);
//       java.lang.String header_string = getWeatherJSON();
 //      System.out.println(header_string);
//       weather();
//       initializeWeatherClient();
//       initializeDBMS();
//       try{
//           
//          java.sql.ResultSet         rs       = queryTimestamp_Range("2019-08-26 08:14:07","2019-08-26 08:15:07");
//          java.sql.ResultSetMetaData metaData = rs.getMetaData();   
//          int column_count = metaData.getColumnCount();
//          rs.last();
//          int rowCount = rs.getRow();
//          rs.first();
//          System.out.println("Column Count = "+column_count+" Row Count = "+rowCount);
//       }catch(Exception e){
//           System.out.println(e.toString());
//       }
//       ArrayList results = query("2019-08-26 08:14:07");
//       System.out.println(results.toString());
//       poll(); 
    }
    private void weather(){
        myWeatherController = new WeatherController();
//        myWeatherController.poll();
    }
    private void poll(){
       boolean state = true;
       while(state){     
           java.lang.String current = myWeatherClientSocket.sendReceiveCommand("?WEATHER"+TERMINATOR);
//          java.lang.String current = getHeader();
//          java.lang.String current = query_timestamp("2019-08-28 20:40:04");
//            ResultSet current = queryTimestamp("2019-08-28 20:40:04");
            System.out.println(current);
       }        
    }
    private java.lang.String query_timestamp(java.lang.String start_timestamp_string){
       java.lang.String header = myClientSocket.sendCommand("QUERY "+start_timestamp_string+TERMINATOR,true);
       boolean state = true;
       while(state){
         java.lang.String line = myClientSocket.getNextResponse();
         header = header + line +TERMINATOR;
//         System.out.println(line.length());
         if(line.length() == 0){
             line = myClientSocket.getNextResponse();
             if(line.length() == 0){
               state = false;                
             }
         }
       }
       return header;
    }  
        private java.lang.String query_timestamp_range(java.lang.String start_timestamp_string,java.lang.String end_timestamp_string){
       java.lang.String header = myClientSocket.sendCommand("QUERYRANGE "+start_timestamp_string+","+end_timestamp_string+TERMINATOR,true);
       boolean state = true;
       while(state){
         java.lang.String line = myClientSocket.getNextResponse();
         header = header + line +TERMINATOR;
//         System.out.println(line.length());
         if(line.length() == 0){
             line = myClientSocket.getNextResponse();
             if(line.length() == 0){
               state = false;                
             }
         }
       }
       return header;
    }    

    private java.lang.String getHeader(){
       java.lang.String header = myClientSocket.sendCommand("REQUEST_JSON"+TERMINATOR,true);
       boolean state = true;
       int count = 0;
       while(state){
         java.lang.String line = myClientSocket.getNextResponse();
         header = header + line +TERMINATOR;
         count = count+1;
//         System.out.println(line.length());
         if(line.length() == 0){
             line = myClientSocket.getNextResponse();
             if(line.length() == 0){
               state = false;  
               System.out.println("Number of lines = "+count);
             }
         }
       }
       return header;
    }
    java.lang.String getHeader2(){
        java.lang.String header = myClientSocket.sendCommand("REQUEST_HEADER"+TERMINATOR,true);
        int NUM_CARDS = 52;
        for(int i=0;i<NUM_CARDS;i++){
         java.lang.String line = myClientSocket.getNextResponse();
         header = header + line +TERMINATOR;
         if(line.length() == 0){
             line = myClientSocket.getNextResponse();
             header = header + line +TERMINATOR;
         }
       }
      return header;
    }
    java.lang.String getWeather(){
        java.lang.String header = myClientSocket.sendCommand("WEATHER_FITS"+TERMINATOR,true);
        int NUM_CARDS =80;
        for(int i=0;i<NUM_CARDS;i++){
         java.lang.String line = myClientSocket.getNextResponse();
         header = header + line +TERMINATOR;
         if(line.length() == 0){
             line = myClientSocket.getNextResponse();
             header = header + line +TERMINATOR;
         }
       }
      return header;
    } 
    java.lang.String getWeatherJSON(){
        java.lang.String header = myClientSocket.sendCommand("WEATHER_JSON"+TERMINATOR,true);
        System.out.println(header);
      return header;
    }  
    private void initializeClient(){
        myClientSocket = new ClientSocket();
        myClientSocket.setServerName("lostgirl.palomar.caltech.edu");
        myClientSocket.setServerPort(TelemetryServer.DEFAULT_SERVER_PORT);
        myClientSocket.startConnection(ClientSocket.USE_HOSTNAME);
    }
    private void initializeWeatherClient(){
         myWeatherClientSocket = new ClientSocket();
         myWeatherClientSocket.setServerName("pele.palomar.caltech.edu");
         myWeatherClientSocket.setServerPort(62000);
         myWeatherClientSocket.startConnection(ClientSocket.USE_HOSTNAME);
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
      Class.forName("com.mysql.jdbc.Driver").newInstance(); 
      conn = DriverManager.getConnection("jdbc:mysql://"+SYSTEM+SEP+DBMS+"?"+"user="+USERNAME+"&password="+PASSWORD);
   }catch(Exception e){
     System.out.println("Error loading the JDBC driver. "+e.toString()); 
   }     
} 
/*================================================================================================
/       constructPreparedStatement()
/=================================================================================================*/
public java.sql.ResultSet queryTimestamp(java.lang.String timestamp){
    java.sql.ResultSet rs = null; 
    java.lang.String query = "SELECT * FROM telemetry.p200telemetry WHERE TIMESTAMP_UTC = ?"; 
     try{
       java.sql.Timestamp ts = java.sql.Timestamp.valueOf(timestamp) ;       
       java.sql.PreparedStatement st = conn.prepareStatement(query);
       st.setTimestamp(1,ts,Calendar.getInstance(TimeZone.getTimeZone("UTC")));       
       rs = st.executeQuery();
       java.sql.ResultSetMetaData metaData = rs.getMetaData();

     }catch(Exception e){
       System.out.println(e.toString());  
     }
     return rs;
}
public Header constructHeader(TelemetryObject current){
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
        
    }
   return currentHeader;
}
/*================================================================================================
/       constructPreparedStatement()
/=================================================================================================*/
public ArrayList query(java.lang.String timestamp){    
    java.util.ArrayList telemetry_array = new java.util.ArrayList();
    try{
       java.sql.ResultSet results = queryTimestamp(timestamp);
       while(results.next()){
          TelemetryObject current = new TelemetryObject();
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
          Header current_header = constructHeader(current);
//          current.CURRENT_HEADER = current_header;
          telemetry_array.add(current);
          System.out.println(current.UTC_TIMESTAMP);
       }
    }catch(Exception e){
       System.out.println(e.toString());
    }   
    return telemetry_array;
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
                                  rs       = st.executeQuery();  
       java.sql.ResultSetMetaData metaData = rs.getMetaData();
     }catch(Exception e){
       System.out.println(e.toString());  
     }
  return rs;
}

   public static void main(String args[]) {
    java.awt.EventQueue.invokeLater(new Runnable() {
        public void run() {
            new TestClient();
        }
    });
   }
}
