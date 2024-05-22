/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package edu.caltech.palomar.telescopes.telemetry;
import com.google.gson.Gson;
import edu.caltech.palomar.io.ClientSocket;
import static edu.caltech.palomar.telescopes.telemetry.P200WeatherObject.telemetryLogger;
import edu.caltech.palomar.util.general.CommandLogModel;
import java.beans.PropertyChangeListener;
import java.beans.PropertyChangeSupport;
import java.io.FileInputStream;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.Timestamp;
import java.time.ZoneId;
import java.time.ZonedDateTime;
import java.util.Properties;
import java.util.StringTokenizer;
import org.apache.log4j.BasicConfigurator;
import org.apache.log4j.DailyRollingFileAppender;
import org.apache.log4j.Layout;
import org.apache.log4j.Logger;
import org.apache.log4j.PatternLayout;
import edu.caltech.palomar.telescopes.telemetry.P48weather;
import static edu.caltech.palomar.telescopes.telemetry.P60WeatherObject.ERROR;
import nom.tam.fits.Header;
import nom.tam.fits.HeaderCard;
import nom.tam.fits.HeaderCardException;
import nom.tam.util.Cursor;
/**
 *
 * @author developer
 */
public class P48WeatherObject {
    transient private PropertyChangeSupport propertyChangeListeners = new PropertyChangeSupport(this);
    private java.lang.String     TERMINATOR                = new java.lang.String("\n");
    public P48weather            current                   = new P48weather();
//   ?WEATHER:\n
//   UTC=%04d:%03d:%02d:%02d:%04.1f\n
//   Windspeed_Avg_Threshold=%.1f\n
//   Gust_Speed_Threshold=%.1f\n
//   Alarm_Hold_Time=%.0f\n
//   Remaining_Hold_Time(s)=0.0\n
//   Outside_DewPt_Threshold=%.1f\n
//   Inside_DewPt_Threshold=%.1f\n
//   Wind_Dir_Current=%.0f\n
//   Windspeed_Current=%.1f\n
//   Windspeed_Average=%.1f\n
//   Outside_Air_Temp=%.1f\n
//   Outside_Rel_Hum=%d\n
//   Outside_DewPt=%.1f\n
//   Inside_Air_Temp=%.1f\n
//   Inside_Rel_Hum=%d\n
//   Inside_DewPt=%.1f\n
//   Wetness=%s\n
//   Weather_Status=%s\n\n
     public java.lang.String P48_UTC = new java.lang.String();
     public float            P48_Windspeed_Avg_Threshold;
     public float            P48_Gust_Speed_Threshold;
     public float            P48_Alarm_Hold_Time;
     public float            P48_Remaining_Hold_Time;
     public float            P48_Outside_DewPt_Threshold;
     public float            P48_Inside_DewPt_Threshold;
     public float            P48_Wind_Dir_Current;
     public float            P48_Windspeed_Current;
     public float            P48_Windspeed_Average;
     public float            P48_Outside_Air_Temp;
     public float            P48_Outside_Rel_Hum;
     public float            P48_Outside_DewPt;
     public float            P48_Inside_Air_Temp;
     public float            P48_Inside_Rel_Hum;
     public float            P48_Inside_DewPt;
     public java.lang.String P48_Wetness        = new java.lang.String();
     public java.lang.String P48_Weather_Status = new java.lang.String();
     public Timestamp        UTC_TIMESTAMP;
     public java.lang.String LOCAL_DATE_TIME    = new java.lang.String();
     public java.lang.String UTC_DATE_TIME      = new java.lang.String();
     public ClientSocket     myWeatherClientSocket;
     public PreparedStatement INSERT_PREP_STATEMENT;
     private boolean          polling;
/*     CREATE TABLE `telemetry
    `.`p48weather
    ` (
  `ID
    ` BIGINT NOT NULL AUTO_INCREMENT,
  `TIMESTAMP_UTC
    ` TIMESTAMP NOT NULL ,
  `UTCDATETIME

    ` VARCHAR(

    60) NULL,
  `LOCALDATETIME` VARCHAR(60) NULL,
  `WEATHER_TIMESTAMP` BIGINT NOT NULL,
  `WINDSPEED_AVG_THRESHOLD` FLOAT NULL,
  `GUST_WINDSPEED_THRESHOLD` FLOAT NULL,
  `ALARM_HOLDTIME` FLOAT NULL,
  `REMAINING_HOLDTIME` FLOAT NULL,
  `OUTSIDE_DEWPOINT_THRESHOLD` FLOAT NULL,
  `INSIDE_DEWPOINT_THRESHOLD` FLOAT NULL,
  `CURRENT_WIND_DIRECTION` FLOAT NULL,
  `CURRENT_WINDSPEED` FLOAT NULL,
  `WINDSPEED_AVERAGE` FLOAT NULL,
  `OUTSIDE_AIR_TEMPERATURE` FLOAT NULL,
  `OUTSIDE_RELATIVE_HUMIDITY` FLOAT NULL,
  `OUTSIDE_DEWPOINT` FLOAT NULL,
  `INSIDE_AIR_TEMPERATURE` FLOAT NULL,
  `INSIDE_RELATIVE_HUMIDITY` FLOAT NULL,
  `INSIDE_DEWPOINT` FLOAT NULL,
  `WETNESS` VARCHAR(45) NULL,
  `STATUS` VARCHAR(45) NULL,
  PRIMARY KEY (`ID`),
  INDEX `TIMESTAMP_UTC_INDEX` (`TIMESTAMP_UTC` ASC),
  INDEX `WEATHER_TIMESTAMP` USING BTREE (`WEATHER_TIMESTAMP` ASC));
*/
    public static int                DEBUG              = 0;
    public static int                INFO               = 1;
    public static int                WARN               = 2;
    public static int                ERROR              = 3;
    public static int                FATAL              = 4;    
    static Logger                    telemetryLogger    = Logger.getLogger(P48WeatherObject.class);
    public DailyRollingFileAppender  fileAppender       = new DailyRollingFileAppender();
    private Layout                   layout             = new PatternLayout("%-5p [%t]: %m%n");
    public java.lang.String          LOG_DIRECTORY      = new java.lang.String();
    private java.lang.String     SEP             = System.getProperty("file.separator");
    public java.lang.String      USERDIR         = System.getProperty("user.dir");
    private java.lang.String     CONFIG          = new java.lang.String("config");
    public  java.lang.String     DBMS_CONNECTION_PROPERTIES = "telemetry_dbms.ini";
    public int                   P48_WEATHER_POLLING;
    public Connection            conn               = null;
    public CommandLogModel       myCommandLogModel  = new CommandLogModel();
    public float                 MISSING_VALUE = -99;  
    public Header                CURRENT_FITS_HEADER;
    public java.lang.String      CURRENT_FITS_HEADER_STRING = new java.lang.String();
    public java.lang.String      CURRENT_GSON_TELEMETRY     = new java.lang.String();
/*=============================================================================================
/     initializeWeatherClient()
/=============================================================================================*/
     public P48WeatherObject(){
         initializeWeatherClient();
         initializeDBMS();
         boolean test =false;
         if(test){
             execute_process();
         }
     }
/*=============================================================================================
/     initializeWeatherClient()
/=============================================================================================*/
    private void initializeWeatherClient(){
         myWeatherClientSocket = new ClientSocket();
         myWeatherClientSocket.setServerName("pele.palomar.caltech.edu");
         myWeatherClientSocket.setServerPort(62000);
         myWeatherClientSocket.startConnection(ClientSocket.USE_HOSTNAME);
    } 
/*================================================================================================
/       execute_process()
/=================================================================================================*/
public void execute_process(){
      ExecuteProcessThread myExecuteProcessThread = new  ExecuteProcessThread();
      myExecuteProcessThread.start();
 }
/*=============================================================================================
/         initializeLogging()
/=============================================================================================*/
    public void process(){
        try{
           java.lang.String current = myWeatherClientSocket.sendReceiveCommand("?WEATHER"+TERMINATOR);
           parse(current);            
           executeInsertStatement();
           constructHeader();
        }catch(Exception e){
           System.out.println(e.toString());
        }       
    }
/*=============================================================================================
/         initializeLogging()
/=============================================================================================*/
 private void initializeLogging(){
     BasicConfigurator.configure();
     fileAppender.setFile(LOG_DIRECTORY + SEP +"P48Weather.log");        
     fileAppender.setDatePattern("'.'yyyy-MM-dd");
     fileAppender.setAppend(true);
     fileAppender.setLayout(layout);
     fileAppender.activateOptions();
     telemetryLogger.addAppender(fileAppender);
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
      LOG_DIRECTORY   = dbms_properties.getProperty("LOG_DIRECTORY");   
      P48_WEATHER_POLLING = java.lang.Integer.parseInt(dbms_properties.getProperty("P48_WEATHER_POLLING"));   
      Class.forName("com.mysql.jdbc.Driver").newInstance(); 
      conn = DriverManager.getConnection("jdbc:mysql://"+SYSTEM+SEP+DBMS+"?"+"user="+USERNAME+"&password="+PASSWORD);
      constructPreparedStatement();
      initializeLogging();
   }catch(Exception e){
//     System.out.println("Error loading the JDBC driver. "+e.toString()); 
     logMessage(ERROR,"Error loading the JDBC driver. "+e.toString());
   }     
}      
/*=============================================================================================
/     initializeWeatherClient()
/=============================================================================================*/
     public void parse(java.lang.String current_weather){

     java.lang.String UTC             = new java.lang.String();
     float Windspeed_Avg_Threshold    = MISSING_VALUE;
     float Gust_Speed_Threshold       = MISSING_VALUE;
     float Alarm_Hold_Time            = MISSING_VALUE;
     float Remaining_Hold_Time        = MISSING_VALUE;
     float Outside_DewPt_Threshold    = MISSING_VALUE;
     float Inside_DewPt_Threshold     = MISSING_VALUE;
     float Wind_Dir_Current           = MISSING_VALUE;
     float Windspeed_Current          = MISSING_VALUE;
     float Windspeed_Average          = MISSING_VALUE;
     float Outside_Air_Temp           = MISSING_VALUE;
     float Outside_Rel_Hum            = MISSING_VALUE;
     float Outside_DewPt              = MISSING_VALUE;
     float Inside_Air_Temp            = MISSING_VALUE;
     float Inside_Rel_Hum             = MISSING_VALUE;
     float Inside_DewPt               = MISSING_VALUE;
     java.lang.String Wetness         = new java.lang.String();
     java.lang.String Weather_Status  = new java.lang.String();
         
         
        ZonedDateTime now     = ZonedDateTime.now();
        ZonedDateTime now_utc = now.withZoneSameInstant(ZoneId.of("UTC"));
        Timestamp current_timestamp = new Timestamp(now_utc.getYear()-1900,now_utc.getMonthValue()-1,now_utc.getDayOfMonth(),
                                                    now_utc.getHour(),now_utc.getMinute(),now_utc.getSecond(),0);
         UTC_TIMESTAMP = current_timestamp;
         LOCAL_DATE_TIME = now.toString();
         UTC_DATE_TIME   = now_utc.toString();
         current.UTC_TIMESTAMP = current_timestamp;
         current.LOCAL_DATE_TIME = this.LOCAL_DATE_TIME;
         current.UTC_DATE_TIME   = this.UTC_DATE_TIME;
         System.out.println("Local Time = "+now.toString()+" UTC Time = "+now_utc.toString());         
         StringTokenizer st = new StringTokenizer(current_weather,TERMINATOR);
         int count = st.countTokens();
         java.lang.String command = st.nextToken();
         for(int i=0;i<count-1;i++){
             try{
                 java.lang.String current = st.nextToken();
                 current = current.trim();
                 if(current.length() != 0){
                     StringTokenizer  st2 = new StringTokenizer(current,"=");
                     java.lang.String description = st2.nextToken();
                     java.lang.String value       = st2.nextToken();
                     switch(i){
                         case 0:  UTC = value; break;
                         case 1:  Windspeed_Avg_Threshold = (Float.valueOf(value)).floatValue(); break;
                         case 2:  Gust_Speed_Threshold    = (Float.valueOf(value)).floatValue(); break;
                         case 3:  Alarm_Hold_Time         = (Float.valueOf(value)).floatValue(); break;
                         case 4:  Remaining_Hold_Time     = (Float.valueOf(value)).floatValue(); break;
                         case 5:  Outside_DewPt_Threshold = (Float.valueOf(value)).floatValue(); break;
                         case 6:  Inside_DewPt_Threshold  = (Float.valueOf(value)).floatValue(); break;
                         case 7:  Wind_Dir_Current        = (Float.valueOf(value)).floatValue(); break;
                         case 8:  Windspeed_Current       = (Float.valueOf(value)).floatValue(); break;
                         case 9:  Windspeed_Average       = (Float.valueOf(value)).floatValue(); break;
                         case 10: Outside_Air_Temp        = (Float.valueOf(value)).floatValue(); break;
                         case 11: Outside_Rel_Hum         = (Float.valueOf(value)).floatValue(); break;
                         case 12: Outside_DewPt           = (Float.valueOf(value)).floatValue(); break;
                         case 13: Inside_Air_Temp         = (Float.valueOf(value)).floatValue(); break;
                         case 14: Inside_Rel_Hum          = (Float.valueOf(value)).floatValue(); break;
                         case 15: Inside_DewPt            = (Float.valueOf(value)).floatValue(); break;
                         case 16: Wetness                 = value; break;
                         case 17: Weather_Status          = value; break;
                     }                               
                 System.out.print("Description = "+description);
                 System.out.println("  Value = "+value);
               }   
             }catch(Exception e){
                 this.logMessage(ERROR,"Error parsing response from the P48 TCS"+ e.toString());
             }
         }
         setP48Timestamp(UTC);
         setP48_Windspeed_Avg_Threshold(Windspeed_Avg_Threshold);
         setP48_Gust_Speed_Threshold(Gust_Speed_Threshold);
         setP48_Alarm_Hold_Time(Alarm_Hold_Time);
         setP48_Remaining_Hold_Time(Remaining_Hold_Time);         
         setP48_Outside_DewPt_Threshold(Outside_DewPt_Threshold);
         setP48_Inside_DewPt_Threshold(Inside_DewPt_Threshold);
         setP48_Wind_Dir_Current(Wind_Dir_Current);
         setP48_Windspeed_Current(Windspeed_Current);
         setP48_Windspeed_Average(Windspeed_Average);
         setP48_Outside_Air_Temp(Outside_Air_Temp);
         setP48_Outside_Rel_Hum(Outside_Rel_Hum);
         setP48_Outside_DewPt(Outside_DewPt);
         setP48_Inside_Air_Temp(Inside_Air_Temp);
         setP48_Inside_Rel_Hum(Inside_Rel_Hum);
         setP48_Inside_DewPt(Inside_DewPt);
         setP48_Wetness(Wetness);
         setP48_Weather_Status(Weather_Status);                        
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
public synchronized java.lang.String getCURRENT_GSON_WEATHER(){
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
/       weatherToGson(TelemetryObject current)
/=================================================================================================*/
public java.lang.String weatherToGson(P48weather current){
    Gson gson = new Gson();    
    java.lang.String gson_weather = gson.toJson(current);
    return gson_weather;
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
/       constructHeader(TelemetryObject current
/=================================================================================================*/
public java.lang.String constructHeader(){
    Header currentHeader = new Header(); 
    try{
       currentHeader.addLine(new HeaderCard("P4UTCTS",  UTC_TIMESTAMP.toString(),"P60 UTC timestamp"));
       currentHeader.addLine(new HeaderCard("P4UTCST",  UTC_DATE_TIME,"P60 UTC date-time string"));
       currentHeader.addLine(new HeaderCard("P4LDT",    LOCAL_DATE_TIME,"P60 Local date and time"));
       currentHeader.addLine(new HeaderCard("P4WTS",    this.P48_UTC.toString(),"P48 Time stamp for the weather record"));
       currentHeader.addLine(new HeaderCard("P4WINDS",  this.getP48_Windspeed_Avg_Threshold(),"P48 windspeed average threshold"));
       currentHeader.addLine(new HeaderCard("P4GWINDS", this.getP48_Gust_Speed_Threshold(),"P48 windspeed gust threshold"));
       currentHeader.addLine(new HeaderCard("P4ALRMHT", this.getP48_Alarm_Hold_Time(),"P48 alarm hold time"));
       currentHeader.addLine(new HeaderCard("P4REMHLD", this.getP48_Remaining_Hold_Time(),"P48 remaining hold time"));
       currentHeader.addLine(new HeaderCard("P4OTDEWT", this.getP48_Outside_DewPt_Threshold(),"P48 outside dewpoint threshold"));
       currentHeader.addLine(new HeaderCard("P4INDEWT", this.getP48_Inside_DewPt_Threshold(),"P48 inside dewpoint threshold"));
       currentHeader.addLine(new HeaderCard("P4WINDD",  this.getP48_Wind_Dir_Current(),"P48 wind direction"));
       currentHeader.addLine(new HeaderCard("P4WINDSP", this.getP48_Windspeed_Current(),"P48 windspeed current"));
       currentHeader.addLine(new HeaderCard("P4WINDAV", this.getP48_Windspeed_Average(),"P48 windspeed average"));
       currentHeader.addLine(new HeaderCard("P4OTAIRT", this.getP48_Outside_Air_Temp(),"P48 outside air temperature"));
       currentHeader.addLine(new HeaderCard("P4OTRHUM", this.getP48_Outside_Rel_Hum(),"P48"));
       currentHeader.addLine(new HeaderCard("P4OUTDEW", this.getP48_Outside_DewPt(),"P48 outside dewpoint"));
       currentHeader.addLine(new HeaderCard("P4INAIRT", this.getP48_Inside_Air_Temp(),"P48 inside air temperaturey"));
       currentHeader.addLine(new HeaderCard("P4INRHUM", this.getP48_Inside_Rel_Hum(),"P48 inside relative humidity"));
       currentHeader.addLine(new HeaderCard("P4INDEW",  this.getP48_Inside_DewPt(),"48 inside dewpoint"));
       currentHeader.addLine(new HeaderCard("P4WETNES", this.getP48_Wetness(),"P48 wetness?"));
       currentHeader.addLine(new HeaderCard("P4STATUS", this.getP48_Weather_Status(),"P48 status"));
       setCURRENT_HEADER(currentHeader);
       java.lang.String current_header_string =  headerToString(currentHeader);
       setCURRENT_FITS_HEADER_STRING(current_header_string);
//       current.CURRENT_HEADER = currentHeader;
       java.lang.String current_telemetry_string = weatherToGson(current);
       setCURRENT_GSON_TELEMETRY(current_telemetry_string);
       System.out.println(CURRENT_FITS_HEADER_STRING);
    }catch(HeaderCardException e){ 
       logMessage(ERROR,"Error in method constructHeader "+e.toString());
       System.out.println(e.toString()); 
    }
  return CURRENT_FITS_HEADER_STRING;   
}       
/*================================================================================================
/    setP48Timestamp(java.lang.String new_P48_UTC)
/=================================================================================================*/
  public void setP48Timestamp(java.lang.String new_P48_UTC){
     java.lang.String  old_P48_UTC = this.P48_UTC;
     this.P48_UTC = new_P48_UTC;
     current.P48_UTC = this.P48_UTC;
     propertyChangeListeners.firePropertyChange("P48_UTC", old_P48_UTC, P48_UTC);     
  }
  public java.lang.String getP48Timestamp(){
     return P48_UTC; 
  }
/*================================================================================================
/     setP48_Windspeed_Avg_Threshold(float new_P48_Windspeed_Avg_Threshold)
/=================================================================================================*/
  public void setP48_Windspeed_Avg_Threshold(float new_P48_Windspeed_Avg_Threshold) {
     float  old_P48_Windspeed_Avg_Threshold = this.P48_Windspeed_Avg_Threshold;
     this.P48_Windspeed_Avg_Threshold = new_P48_Windspeed_Avg_Threshold;
     current.P48_Windspeed_Average = this.P48_Windspeed_Average;
     propertyChangeListeners.firePropertyChange("P48_Windspeed_Avg_Threshold", Float.valueOf(old_P48_Windspeed_Avg_Threshold), Float.valueOf(P48_Windspeed_Avg_Threshold));
  }
  public float getP48_Windspeed_Avg_Threshold() {
     return P48_Windspeed_Avg_Threshold;
  } 
/*================================================================================================
/     setP48_Windspeed_Avg_Threshold(float new_P48_Windspeed_Avg_Threshold)
/=================================================================================================*/
  public void setP48_Gust_Speed_Threshold(float new_P48_Gust_Speed_Threshold) {
     float  old_P48_Gust_Speed_Threshold = this.P48_Gust_Speed_Threshold;
     this.P48_Gust_Speed_Threshold = new_P48_Gust_Speed_Threshold;
     current.P48_Gust_Speed_Threshold = P48_Gust_Speed_Threshold;
     propertyChangeListeners.firePropertyChange("P48_Gust_Speed_Threshold", Float.valueOf(old_P48_Gust_Speed_Threshold),Float.valueOf(P48_Gust_Speed_Threshold));
  }
  public float getP48_Gust_Speed_Threshold() {
     return P48_Gust_Speed_Threshold;
  } 
/*================================================================================================
/     setP48_Windspeed_Avg_Threshold(float new_P48_Windspeed_Avg_Threshold)
/=================================================================================================*/
  public void setP48_Alarm_Hold_Time(float new_P48_Alarm_Hold_Time) {
     float  old_P48_Alarm_Hold_Time = this.P48_Alarm_Hold_Time;
     this.P48_Alarm_Hold_Time = new_P48_Alarm_Hold_Time;
     current.P48_Alarm_Hold_Time = P48_Alarm_Hold_Time;
     propertyChangeListeners.firePropertyChange("P48_Alarm_Hold_Time",Float.valueOf(old_P48_Alarm_Hold_Time),Float.valueOf(P48_Alarm_Hold_Time));
  }
  public float getP48_Alarm_Hold_Time() {
     return P48_Alarm_Hold_Time;
  }   
/*================================================================================================
/     setP48_Windspeed_Avg_Threshold(float new_P48_Windspeed_Avg_Threshold)
/=================================================================================================*/
  public void setP48_Remaining_Hold_Time(float new_P48_Remaining_Hold_Time) {
     float  old_P48_Remaining_Hold_Time = this.P48_Remaining_Hold_Time;
     this.P48_Remaining_Hold_Time = new_P48_Remaining_Hold_Time;
     current.P48_Remaining_Hold_Time = P48_Remaining_Hold_Time;
     propertyChangeListeners.firePropertyChange("P48_Remaining_Hold_Time",Float.valueOf(old_P48_Remaining_Hold_Time),Float.valueOf(P48_Remaining_Hold_Time));
  }
  public float getP48_Remaining_Hold_Time() {
     return P48_Remaining_Hold_Time;
  } 
/*================================================================================================
/     setP48_Windspeed_Avg_Threshold(float new_P48_Windspeed_Avg_Threshold)
/=================================================================================================*/
  public void setP48_Outside_DewPt_Threshold(float new_P48_Outside_DewPt_Threshold) {
     float  old_P48_Outside_DewPt_Threshold = this.P48_Outside_DewPt_Threshold;
     this.P48_Outside_DewPt_Threshold = new_P48_Outside_DewPt_Threshold;
     current.P48_Outside_DewPt_Threshold = P48_Outside_DewPt_Threshold;
     propertyChangeListeners.firePropertyChange("P48_Outside_DewPt_Threshold",Float.valueOf(old_P48_Outside_DewPt_Threshold),Float.valueOf(P48_Outside_DewPt_Threshold));
  }
  public float getP48_Outside_DewPt_Threshold() {
     return P48_Outside_DewPt_Threshold;
  } 
/*================================================================================================
/     setP48_Windspeed_Avg_Threshold(float new_P48_Windspeed_Avg_Threshold)
/=================================================================================================*/
  public void setP48_Inside_DewPt_Threshold(float new_P48_Inside_DewPt_Threshold) {
     float  old_P48_Inside_DewPt_Threshold = this.P48_Inside_DewPt_Threshold;
     this.P48_Inside_DewPt_Threshold = new_P48_Inside_DewPt_Threshold;
     current.P48_Inside_DewPt_Threshold = P48_Inside_DewPt_Threshold;
     propertyChangeListeners.firePropertyChange("P48_Inside_DewPt_Threshold",Float.valueOf(old_P48_Inside_DewPt_Threshold),Float.valueOf(P48_Inside_DewPt_Threshold));
  }
  public float getP48_Inside_DewPt_Threshold() {
     return P48_Inside_DewPt_Threshold;
  }  
/*================================================================================================
/     setP48_Windspeed_Avg_Threshold(float new_P48_Windspeed_Avg_Threshold)
/=================================================================================================*/
  public void setP48_Wind_Dir_Current(float new_P48_Wind_Dir_Current) {
     float  old_P48_Wind_Dir_Current = this.P48_Wind_Dir_Current;
     this.P48_Wind_Dir_Current = new_P48_Wind_Dir_Current;
     current.P48_Wind_Dir_Current = P48_Wind_Dir_Current;
     propertyChangeListeners.firePropertyChange("P48_Wind_Dir_Current", Float.valueOf(old_P48_Wind_Dir_Current),Float.valueOf(P48_Wind_Dir_Current));
  }
  public float getP48_Wind_Dir_Current() {
     return P48_Wind_Dir_Current;
  } 
/*================================================================================================
/     setP48_Windspeed_Avg_Threshold(float new_P48_Windspeed_Avg_Threshold)
/=================================================================================================*/
  public void setP48_Windspeed_Current(float new_P48_Windspeed_Current) {
     float  old_P48_Windspeed_Current = this.P48_Windspeed_Current;
     this.P48_Windspeed_Current = new_P48_Windspeed_Current;
     current.P48_Windspeed_Current = P48_Windspeed_Current;
     propertyChangeListeners.firePropertyChange("P48_Windspeed_Current", Float.valueOf(old_P48_Windspeed_Current),Float.valueOf(P48_Windspeed_Current));
  }
  public float getP48_Windspeed_Current() {
     return P48_Windspeed_Current;
  }  
/*================================================================================================
/     setP48_Windspeed_Avg_Threshold(float new_P48_Windspeed_Avg_Threshold)
/=================================================================================================*/
  public void setP48_Windspeed_Average(float new_P48_Windspeed_Average) {
     float  old_P48_Windspeed_Average = this.P48_Windspeed_Average;
     this.P48_Windspeed_Average = new_P48_Windspeed_Average;
     current.P48_Windspeed_Average = this.P48_Windspeed_Average;
     propertyChangeListeners.firePropertyChange("P48_Windspeed_Average", Float.valueOf(old_P48_Windspeed_Average),Float.valueOf(P48_Windspeed_Average));
  }
  public float getP48_Windspeed_Average() {
     return P48_Windspeed_Average;
  } 
/*================================================================================================
/     setP48_Windspeed_Avg_Threshold(float new_P48_Windspeed_Avg_Threshold)
/=================================================================================================*/
  public void setP48_Outside_Air_Temp(float new_P48_Outside_Air_Temp) {
     float  old_P48_Outside_Air_Temp = this.P48_Outside_Air_Temp;
     this.P48_Outside_Air_Temp = new_P48_Outside_Air_Temp;
     current.P48_Outside_Air_Temp = P48_Outside_Air_Temp;
     propertyChangeListeners.firePropertyChange("P48_Outside_Air_Temp", Float.valueOf(old_P48_Outside_Air_Temp),Float.valueOf(P48_Outside_Air_Temp));
  }
  public float getP48_Outside_Air_Temp() {
     return P48_Outside_Air_Temp;
  }  
/*================================================================================================
/     setP48_Windspeed_Avg_Threshold(float new_P48_Windspeed_Avg_Threshold)
/=================================================================================================*/
  public void setP48_Outside_Rel_Hum(float new_P48_Outside_Rel_Hum) {
     float  old_P48_Outside_Rel_Hum = this.P48_Outside_Rel_Hum;
     this.P48_Outside_Rel_Hum = new_P48_Outside_Rel_Hum;
     current.P48_Outside_Rel_Hum = P48_Outside_Rel_Hum;
     propertyChangeListeners.firePropertyChange("P48_Outside_Rel_Hum", Float.valueOf(old_P48_Outside_Rel_Hum),Float.valueOf(P48_Outside_Rel_Hum));
  }
  public float getP48_Outside_Rel_Hum() {
     return P48_Outside_Rel_Hum;
  }  
/*================================================================================================
/     setP48_Windspeed_Avg_Threshold(float new_P48_Windspeed_Avg_Threshold)
/=================================================================================================*/
  public void setP48_Outside_DewPt(float new_P48_Outside_DewPt) {
     float  old_P48_Outside_DewPt  = this.P48_Outside_DewPt;
     this.P48_Outside_DewPt  = new_P48_Outside_DewPt;
     current.P48_Outside_DewPt = P48_Outside_DewPt;
     propertyChangeListeners.firePropertyChange("P48_Outside_DewPt", Float.valueOf(old_P48_Outside_DewPt),Float.valueOf(P48_Outside_DewPt));
  }
  public float getP48_Outside_DewPt() {
     return P48_Outside_DewPt;
  }  
/*================================================================================================
/     setP48_Windspeed_Avg_Threshold(float new_P48_Windspeed_Avg_Threshold)
/=================================================================================================*/
  public void setP48_Inside_Air_Temp(float new_P48_Inside_Air_Temp) {
     float  old_P48_Inside_Air_Temp  = this.P48_Inside_Air_Temp;
     this.P48_Inside_Air_Temp  = new_P48_Inside_Air_Temp;
     current.P48_Inside_Air_Temp = P48_Inside_Air_Temp;
     propertyChangeListeners.firePropertyChange("P48_Inside_Air_Temp", Float.valueOf(old_P48_Inside_Air_Temp),Float.valueOf(P48_Inside_Air_Temp));
  }
  public float getP48_Inside_Air_Temp() {
     return P48_Inside_Air_Temp;
  }  
 /*================================================================================================
/     setP48_Windspeed_Avg_Threshold(float new_P48_Windspeed_Avg_Threshold)
/=================================================================================================*/
  public void setP48_Inside_Rel_Hum(float new_P48_Inside_Rel_Hum) {
     float  old_P48_Inside_Rel_Hum  = this.P48_Inside_Rel_Hum;
     this.P48_Inside_Rel_Hum  = new_P48_Inside_Rel_Hum;
     current.P48_Inside_Rel_Hum = P48_Inside_Rel_Hum;
     propertyChangeListeners.firePropertyChange("P48_Inside_Rel_Hum", Float.valueOf(old_P48_Inside_Rel_Hum),Float.valueOf(P48_Inside_Rel_Hum));
  }
  public float getP48_Inside_Rel_Hum() {
     return P48_Inside_Rel_Hum;
  } 
 /*================================================================================================
/     setP48_Windspeed_Avg_Threshold(float new_P48_Windspeed_Avg_Threshold)
/=================================================================================================*/
  public void setP48_Inside_DewPt(float new_P48_Inside_DewPt) {
     float  old_P48_Inside_DewPt  = this.P48_Inside_DewPt;
     this.P48_Inside_DewPt  = new_P48_Inside_DewPt;
     current.P48_Inside_DewPt = P48_Inside_DewPt;
     propertyChangeListeners.firePropertyChange("P48_Inside_DewPt", Float.valueOf(old_P48_Inside_DewPt),Float.valueOf(P48_Inside_DewPt));
  }
  public float getP48_Inside_DewPt() {
     return P48_Inside_DewPt;
  }  
/*================================================================================================
/       setEquinox(String equinox_string)
/=================================================================================================*/
  public void setP48_Wetness(String new_P48_Wetness) {
    String  old_P48_Wetness = this.P48_Wetness;
    this.P48_Wetness = new_P48_Wetness;
    current.P48_Wetness = P48_Wetness;
    propertyChangeListeners.firePropertyChange("P48_Wetness", old_P48_Wetness, P48_Wetness);
  }
  public String getP48_Wetness() {
    return P48_Wetness;
  }  
/*================================================================================================
/       setEquinox(String equinox_string)
/=================================================================================================*/
  public void setP48_Weather_Status(String new_P48_Weather_Status) {
    String  old_P48_Weather_Status = this.P48_Weather_Status;
    this.P48_Weather_Status = new_P48_Weather_Status;
    current.P48_Weather_Status = P48_Weather_Status;
    propertyChangeListeners.firePropertyChange("P48_Weather_Status", old_P48_Weather_Status, P48_Weather_Status);
  }
  public String getP48_Weather_Status() {
    return P48_Weather_Status;
  }  
/*================================================================================================
/       TelescopeObject getTelescopeObject()
/=================================================================================================*/
public void executeInsertStatement(){
    try{
        INSERT_PREP_STATEMENT.clearParameters();        
        INSERT_PREP_STATEMENT.setTimestamp(1,this.UTC_TIMESTAMP);
        INSERT_PREP_STATEMENT.setString(2,this.UTC_DATE_TIME.trim());
        INSERT_PREP_STATEMENT.setString(3,this.LOCAL_DATE_TIME.trim());
        INSERT_PREP_STATEMENT.setString(4, this.P48_UTC);
        INSERT_PREP_STATEMENT.setFloat(5, this.P48_Windspeed_Avg_Threshold);
        INSERT_PREP_STATEMENT.setFloat(6,(float)this.P48_Gust_Speed_Threshold);
        INSERT_PREP_STATEMENT.setFloat(7,(float)this.P48_Alarm_Hold_Time);
        INSERT_PREP_STATEMENT.setFloat(8,(float)this.P48_Remaining_Hold_Time);
        INSERT_PREP_STATEMENT.setFloat(9,(float)this.P48_Outside_DewPt_Threshold);
        INSERT_PREP_STATEMENT.setFloat(10,(float)this.P48_Inside_DewPt_Threshold);
        INSERT_PREP_STATEMENT.setFloat(11,(float)this.P48_Wind_Dir_Current);
        INSERT_PREP_STATEMENT.setFloat(12,(float)this.P48_Windspeed_Current);
        INSERT_PREP_STATEMENT.setFloat(13,(float)this.P48_Windspeed_Average);
        INSERT_PREP_STATEMENT.setFloat(14,(float)this.P48_Outside_Air_Temp);
        INSERT_PREP_STATEMENT.setFloat(15,(float)this.P48_Outside_Rel_Hum);
        INSERT_PREP_STATEMENT.setFloat(16,(float)this.P48_Outside_DewPt);
        INSERT_PREP_STATEMENT.setFloat(17,(float)this.P48_Inside_Air_Temp);
        INSERT_PREP_STATEMENT.setFloat(18,(float)this.P48_Inside_Rel_Hum);
        INSERT_PREP_STATEMENT.setFloat(19,(float)this.P48_Inside_DewPt);
        INSERT_PREP_STATEMENT.setString(20,this.P48_Wetness);
        INSERT_PREP_STATEMENT.setString(21,this.P48_Weather_Status);
        
        logMessage(INFO,INSERT_PREP_STATEMENT.toString());
        // now that we have all the parameters set execute the update on the DBMS
        INSERT_PREP_STATEMENT.executeUpdate();
    }catch(Exception e){
       logMessage(ERROR,"Error in method executeInsertStatement "+e.toString());
//       System.out.println(e.toString()); 
    }  
}  
/*================================================================================================
/       constructPreparedStatement()
/=================================================================================================*/
public void constructPreparedStatement(){
    String query = "INSERT INTO p48weather ("
    + " TIMESTAMP_UTC,"
    + " UTCDATETIME,"
    + " LOCALDATETIME,"
    + " WEATHER_TIMESTAMP,"        
    + " WINDSPEED_AVG_THRESHOLD,"        
    + " GUST_WINDSPEED_THRESHOLD,"        
    + " ALARM_HOLDTIME,"        
    + " REMAINING_HOLDTIME,"        
    + " OUTSIDE_DEWPOINT_THRESHOLD,"        
    + " INSIDE_DEWPOINT_THRESHOLD,"        
    + " CURRENT_WIND_DIRECTION,"        
    + " CURRENT_WINDSPEED,"        
    + " WINDSPEED_AVERAGE,"        
    + " OUTSIDE_AIR_TEMPERATURE,"        
    + " OUTSIDE_RELATIVE_HUMIDITY,"        
    + " OUTSIDE_DEWPOINT,"        
    + " INSIDE_AIR_TEMPERATURE,"        
    + " INSIDE_RELATIVE_HUMIDITY,"        
    + " INSIDE_DEWPOINT,"        
    + " WETNESS,"        
    + " STATUS"        
    + " ) VALUES ("
    +  " ?, ?, ?, ?, ?, ?, ?, ?, ?, ?,"
    +  " ?, ?, ?, ?, ?, ?, ?, ?, ?, ?,"
    +  " ?"
    +  ")";
    try{
       INSERT_PREP_STATEMENT = conn.prepareStatement(query);
    }catch(Exception e){
        logMessage(ERROR,"Creating prepared statement. "+e.toString());
    }
}
/*================================================================================================
/       setP200_wind_direction
/=================================================================================================*/
  public synchronized void setPolling(boolean new_polling) {
     boolean  old_polling = this.polling;
     this.polling = new_polling;
     propertyChangeListeners.firePropertyChange("polling", Boolean.valueOf(old_polling),Boolean.valueOf(polling));
  }
  public boolean isPolling() {
     return polling;
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
    public class ExecuteProcessThread  implements Runnable{
    private Thread myThread;
 /*=============================================================================================
/         ExecuteMonitorConnectionThread()
/=============================================================================================*/
    private ExecuteProcessThread(){
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
/*=============================================================================================
/           run()
/=============================================================================================*/
    public void run(){
       setPolling(true);
       while(isPolling()){
           process();
           this.waitForResponseMilliseconds((int)P48_WEATHER_POLLING*1000);
       }       
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

    
/*================================================================================================
/     setP48_Windspeed_Avg_Threshold(float new_P48_Windspeed_Avg_Threshold)
/=================================================================================================*/
    
    
    
    /*================================================================================================
/      public static void main(String args[])
/=================================================================================================*/
      public static void main(String args[]) {
        /* Set the Nimbus look and feel */
       new P48WeatherObject(); //<editor-fold defaultstate="collapsed" desc=" Look and feel setting code (optional) ">
        /* If Nimbus (introduced in Java SE 6) is not available, stay with the default look and feel.
         * For details see http://download.oracle.com/javase/tutorial/uiswing/lookandfeel/plaf.html 
         */
        try {
            for (javax.swing.UIManager.LookAndFeelInfo info : javax.swing.UIManager.getInstalledLookAndFeels()) {
                if ("Nimbus".equals(info.getName())) {
                    javax.swing.UIManager.setLookAndFeel(info.getClassName());
                    break;
                }
            }
        } catch (ClassNotFoundException ex) {
            java.util.logging.Logger.getLogger(SFTPRetrieveWeather.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (InstantiationException ex) {
            java.util.logging.Logger.getLogger(SFTPRetrieveWeather.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (IllegalAccessException ex) {
            java.util.logging.Logger.getLogger(SFTPRetrieveWeather.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (javax.swing.UnsupportedLookAndFeelException ex) {
            java.util.logging.Logger.getLogger(SFTPRetrieveWeather.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        }
        //</editor-fold>
    }
}
