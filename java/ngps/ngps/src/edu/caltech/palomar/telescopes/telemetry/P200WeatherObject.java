/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package edu.caltech.palomar.telescopes.telemetry;

import com.google.gson.Gson;
import java.beans.PropertyChangeListener;
import java.beans.PropertyChangeSupport;
import edu.caltech.palomar.telescopes.telemetry.SFTPRetrieveWeather;
import static edu.caltech.palomar.telescopes.telemetry.TelemetryController.ERROR;
import static edu.caltech.palomar.telescopes.telemetry.TelemetryController.telemetryLogger;
import edu.caltech.palomar.util.general.CommandLogModel;
import java.sql.Timestamp;
import java.time.ZoneId;
import java.time.ZonedDateTime;
import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.FileReader;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.util.ArrayList;
import java.util.Properties;
import org.apache.log4j.DailyRollingFileAppender;
import org.apache.log4j.Layout;
import org.apache.log4j.Logger;
import org.apache.log4j.PatternLayout;
import org.apache.log4j.BasicConfigurator;
import edu.caltech.palomar.telescopes.telemetry.P200weather;
import java.beans.PropertyChangeEvent;
import nom.tam.fits.Header;
import nom.tam.fits.HeaderCard;
import nom.tam.fits.HeaderCardException;
import nom.tam.util.Cursor;
/**
 *
 * @author developer
 */
public class P200WeatherObject {
    transient private PropertyChangeSupport propertyChangeListeners = new PropertyChangeSupport(this);
    private java.lang.String     TERMINATOR      = new java.lang.String("\n");
    private java.lang.String     SEP             = System.getProperty("file.separator");
    public java.lang.String      USERDIR         = System.getProperty("user.dir");
    private java.lang.String     CONFIG          = new java.lang.String("config");
    public long P200_raw_timestamp;
    public double P200_dewpoint;
    public double P200_outside_air_temp;
    public double P200_dome_air_temp;
    public double P200_primary_mirror_temp;
    public double P200_prime_focus_temp;
    public double P200_floor_temp;
    public double P200_primary_cell_temp;
    public double P200_windspeed_median;
    public double P200_windspeed_peak;
    public double P200_windspeed_stddev;
    public double P200_wind_direction;    
    public boolean P200_raining;
    public SFTPRetrieveWeather mySFTPRetrieveWeather = new SFTPRetrieveWeather();
    public ZonedDateTime now;
    public ZonedDateTime now_utc;
    public Timestamp     current_timestamp;
    public ArrayList     weather_arraylist     = new ArrayList();
    public ArrayList     temperature_arraylist = new ArrayList();
    public double        MISSING_VALUE = -99.9;
    public boolean       first_timestamp = true;
    public boolean       first_temperature_timestamp = true;
    public double        P200_temp_stairs0;
    public double        P200_temp_stairs20;
    public double        P200_temp_stairs40;
    public double        P200_temp_stairs60;
    public double        P200_temp_stairs75;
    public double        P200_temp_stairs90;
    public double        P200_temp_gantryUR;
    public double        P200_temp_gantryLR;
    public double        P200_temp_gantryUL;
    public double        P200_temp_gantryLL;
    public double        P200_temp_gantry_farR;
    public double        P200_temp_gantry_farL;
    public double        P200_temp_ArchL;
    public double        P200_temp_ArchR;
    public double        P200_temp_crane; 
    public double        P200_temp_exhaust;  
    public double        P200_temp_shutter;  
    public double        P200_temp_fanR;
    public double        P200_temp_fanL;        
    public double        P200_temp_catwalk;    
    public long          weather_reference_timestamp;
    public long          temperature_reference_timestamp;
    public PreparedStatement     INSERT_PREP_STATEMENT;
    public  java.lang.String     DBMS_CONNECTION_PROPERTIES = "telemetry_dbms.ini";
    static Logger                    telemetryLogger    = Logger.getLogger(TelemetryController.class);
    public DailyRollingFileAppender  fileAppender       = new DailyRollingFileAppender();
    private Layout                   layout             = new PatternLayout("%-5p [%t]: %m%n");
    public java.lang.String          LOG_DIRECTORY      = new java.lang.String();
    public Connection                conn               = null;
    public CommandLogModel           myCommandLogModel  = new CommandLogModel();
    public static int                DEBUG              = 0;
    public static int                INFO               = 1;
    public static int                WARN               = 2;
    public static int                ERROR              = 3;
    public static int                FATAL              = 4;    
    public Timestamp                 UTC_TIMESTAMP;
    public String                    UTC_DATE_TIME;
    public String                    LOCAL_DATE_TIME;
    private boolean                  polling;
    public int                       WEATHER_POLLING;
    public P200weather               current = new P200weather();
     public Header                   CURRENT_FITS_HEADER;
    public java.lang.String          CURRENT_FITS_HEADER_STRING = new java.lang.String();
    public java.lang.String          CURRENT_GSON_TELEMETRY     = new java.lang.String();
    private java.lang.String         p200_utc_timestamp;
/*================================================================================================
/     CREATE TABLE SQL STATEMENT
/=================================================================================================*/   
/*CREATE TABLE `p200weather` (
  `ID` bigint(20) NOT NULL AUTO_INCREMENT,
  `TIMESTAMP_UTC` timestamp NULL DEFAULT NULL,
  `UTCDATETIME` varchar(45) DEFAULT NULL,
  `LOCALDATETIME` varchar(45) DEFAULT NULL,
  `UTCSTART` varchar(45) DEFAULT NULL,
  `WEATHER_TIMESTAMP` bigint(20) DEFAULT NULL,
  `TEMPERATURE_TIMESTAMP` bigint(20) DEFAULT NULL,
  `AVERAGE_WINDSPEED` float DEFAULT NULL,
  `PEAK_WINDSPEED` float DEFAULT NULL,
  `GUST_WINDSPEED` float DEFAULT NULL,
  `WIND_DIRECTION` float DEFAULT NULL,
  `DEWPOINT` float DEFAULT NULL,
  `OUTSIDE_TEMPERATURE` float DEFAULT NULL,
  `INSIDE_TEMPERATURE` float DEFAULT NULL,
  `PRIMARY_MIRROR_TEMPERATURE` float DEFAULT NULL,
  `SECONDARY_MIRROR_TEMPERATURE` float DEFAULT NULL,
  `FLOOR_TEMPERATURE` float DEFAULT NULL,
  `PRIME_FOCUS_TEMPERATURE` float DEFAULT NULL,
  `RAINING` bit(1) DEFAULT NULL,
  `STAIRS_0` float DEFAULT NULL,
  `STAIRS_20` float DEFAULT NULL,
  `STAIRS_40` float DEFAULT NULL,
  `STAIRS_60` float DEFAULT NULL,
  `STAIRS_75` float DEFAULT NULL,
  `STAIRS_90` float DEFAULT NULL,
  `GANTRY_UR` float DEFAULT NULL,
  `GAINTRY_LR` float DEFAULT NULL,
  `GANTRY_UL` float DEFAULT NULL,
  `GANTRY_LL` float DEFAULT NULL,
  `CRANE` float DEFAULT NULL,
  `EXHAUST` float DEFAULT NULL,
  `SHUTTER` float DEFAULT NULL,
  `FAN_R` float DEFAULT NULL,
  `FAN_L` float DEFAULT NULL,
  `CATWALK` float DEFAULT NULL,
  `ARCH_R` float DEFAULT NULL,
  `ARCH_L` float DEFAULT NULL,
  `GANTRY_FAR_R` float DEFAULT NULL,
  `GANTRY_FAR_L` float DEFAULT NULL,
  PRIMARY KEY (`ID`),
  KEY `TIMESTAMP_UTC_INDEX` (`TIMESTAMP_UTC`) USING BTREE,
  KEY `WEATHER_UTC_INDEX` (`WEATHER_TIMESTAMP`),
  KEY `TEMPERATURE_UTC_INDEX` (`TEMPERATURE_TIMESTAMP`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
*/    
 /*================================================================================================
/      P200WeatherObject
/=================================================================================================*/
    public P200WeatherObject(){
       initializeDBMS(); 
       initializeValues();
       setFirstWeatherTimestamp(true);
       boolean test = false;
       if(test){
          execute_process();            
       }
    }
  /*=============================================================================================
/         initializeLogging()
/=============================================================================================*/
 private void initializeLogging(){
     BasicConfigurator.configure();
     fileAppender.setFile(LOG_DIRECTORY + SEP +"Weather.log");        
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
      WEATHER_POLLING = java.lang.Integer.parseInt(dbms_properties.getProperty("WEATHER_POLLING"));   
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
/      constructWeatherFilename()
/=================================================================================================*/
public java.lang.String constructWeatherFilename(){
    now     = ZonedDateTime.now();
    java.lang.String  filename = new java.lang.String();
    java.lang.Integer year  = Integer.valueOf(now.getYear());
    java.lang.Integer month = Integer.valueOf(now.getMonthValue());
    java.lang.String  month_string = new java.lang.String();
    if(month < 10){
       month_string = "0"+month.toString();
    }
    if(month >= 10){
        month_string = month.toString();
    }
    filename = "weather"+(year.toString()).substring(2,year.toString().length())+month_string+".txt";
    return filename;
}
/*================================================================================================
/      constructWeatherFilename()
/=================================================================================================*/
public java.lang.String constructTemperatureFilename(){
    now     = ZonedDateTime.now();
    java.lang.String  filename = new java.lang.String();
    java.lang.Integer year  = Integer.valueOf(now.getYear());
    java.lang.Integer month = Integer.valueOf(now.getMonthValue());
    java.lang.String  month_string = new java.lang.String();
    if(month < 10){
       month_string = "0"+month.toString();
    }
    filename = "dome"+(year.toString()).substring(2,year.toString().length())+month_string+".txt";
    return filename;
}
/*================================================================================================
/       timestamp_measurements()
/=================================================================================================*/
public void timestamp_measurements(){
   now     = ZonedDateTime.now();
   now_utc = now.withZoneSameInstant(ZoneId.of("UTC"));
   current_timestamp = new Timestamp(now_utc.getYear()-1900,now_utc.getMonthValue()-1,now_utc.getDayOfMonth(),
                                                    now_utc.getHour(),now_utc.getMinute(),now_utc.getSecond(),0);  
   this.UTC_TIMESTAMP = current_timestamp;
   this.LOCAL_DATE_TIME = now.toString();
   this.UTC_DATE_TIME   = now_utc.toString();
   current.current_timestamp = current_timestamp;
   current.now = now;
   current.now_utc = now_utc;
   setP200_utc_timestamp(UTC_DATE_TIME);
}
/*================================================================================================
/       retrieveWeatherFile()
/=================================================================================================*/
    public void retrieveWeatherFile(){
        java.lang.String current_filename = constructWeatherFilename();
        mySFTPRetrieveWeather.retrieveFileSCP(current_filename);
    }
/*================================================================================================
/       retrieveWeatherFile()
/=================================================================================================*/
    public void retrieveTemperatureFile(){
        java.lang.String current_filename = constructTemperatureFilename();
        mySFTPRetrieveWeather.retrieveFileSCP(current_filename);
    }     
/*================================================================================================
/       execute_process()
/=================================================================================================*/
public void execute_process(){
      ExecuteProcessThread myExecuteProcessThread = new  ExecuteProcessThread();
      myExecuteProcessThread.start();
 }   
/*================================================================================================
/       retrieveWeatherFile()
/=================================================================================================*/
 public void process(){
     boolean new_record = false;
     timestamp_measurements();
     new_record = readWeatherFile();
     readTemperatureFile();
     if(new_record){
         System.out.println("Inserting a new weather record");
         executeInsertStatement();
         constructHeader();
     }  
     if(!new_record){
         System.out.println("P200 weather skipping insert");         
     }
 }    
/*================================================================================================
/       retrieveWeatherFile()
/=================================================================================================*/
  public boolean readWeatherFile(){
      boolean status = false;
      java.lang.String last_line = new java.lang.String(); 
      try{
         weather_arraylist.clear();
         retrieveWeatherFile(); 
         BufferedReader br = new BufferedReader(new FileReader(mySFTPRetrieveWeather.LOCAL_ROOT_DIR+SEP+constructWeatherFilename()));
         boolean state = true;
         while(state){
           try{
             java.lang.String currentline = br.readLine();
             if(currentline != null){
                weather_arraylist.add(currentline);    
             } 
             if(currentline == null){
                state = false;
             }
           }catch(Exception e){
              state = false; 
           } 
         }
         br.close();
         int count = weather_arraylist.size();
         last_line = (java.lang.String)weather_arraylist.get(count-1);
         status = parse_weather(last_line);
      }catch(Exception e){
          System.out.println(e.toString());
      }
     return status;
  } 
/*================================================================================================
/       TelescopeObject getTelescopeObject()
/=================================================================================================*/
public void executeInsertStatement(){
    try{
        int raining = 0;
        if(this.isP200_raining()){
            raining = 1;
        }else if(!this.isP200_raining()){
            raining = 0;
        }
        INSERT_PREP_STATEMENT.clearParameters();        
        INSERT_PREP_STATEMENT.setTimestamp(1,this.UTC_TIMESTAMP);
        INSERT_PREP_STATEMENT.setString(2,this.UTC_DATE_TIME.trim());
        INSERT_PREP_STATEMENT.setString(3,this.LOCAL_DATE_TIME.trim());
        INSERT_PREP_STATEMENT.setLong(4, this.weather_reference_timestamp);
        INSERT_PREP_STATEMENT.setLong(5, this.temperature_reference_timestamp);
        INSERT_PREP_STATEMENT.setFloat(6,(float)this.P200_windspeed_median);
        INSERT_PREP_STATEMENT.setFloat(7,(float)this.P200_windspeed_peak);
        INSERT_PREP_STATEMENT.setFloat(8,(float)this.P200_windspeed_stddev);
        INSERT_PREP_STATEMENT.setFloat(9,(float)this.P200_wind_direction);
        INSERT_PREP_STATEMENT.setFloat(10,(float)this.P200_dewpoint);
        INSERT_PREP_STATEMENT.setFloat(11,(float)this.P200_outside_air_temp);
        INSERT_PREP_STATEMENT.setFloat(12,(float)this.P200_dome_air_temp);
        INSERT_PREP_STATEMENT.setFloat(13,(float)this.P200_primary_mirror_temp);
        INSERT_PREP_STATEMENT.setFloat(14,(float)this.P200_primary_cell_temp);
        INSERT_PREP_STATEMENT.setFloat(15,(float)this.P200_floor_temp);
        INSERT_PREP_STATEMENT.setFloat(16,(float)this.P200_prime_focus_temp);
        INSERT_PREP_STATEMENT.setInt(17,raining);
        INSERT_PREP_STATEMENT.setFloat(18,(float)this.P200_temp_stairs0);
        INSERT_PREP_STATEMENT.setFloat(19,(float)this.P200_temp_stairs20);
        INSERT_PREP_STATEMENT.setFloat(20,(float)this.P200_temp_stairs40);
        INSERT_PREP_STATEMENT.setFloat(21,(float)this.P200_temp_stairs60);
        INSERT_PREP_STATEMENT.setFloat(22,(float)this.P200_temp_stairs75);
        INSERT_PREP_STATEMENT.setFloat(23,(float)this.P200_temp_stairs90);
        INSERT_PREP_STATEMENT.setFloat(24,(float)this.P200_temp_gantryUR);
        INSERT_PREP_STATEMENT.setFloat(25,(float)this.P200_temp_gantryLR);
        INSERT_PREP_STATEMENT.setFloat(26,(float)this.P200_temp_gantryUL);
        INSERT_PREP_STATEMENT.setFloat(27,(float)this.P200_temp_gantryLL);
        INSERT_PREP_STATEMENT.setFloat(28,(float)this.P200_temp_crane);
        INSERT_PREP_STATEMENT.setFloat(29,(float)this.P200_temp_exhaust);
        INSERT_PREP_STATEMENT.setFloat(30,(float)this.P200_temp_shutter);
        INSERT_PREP_STATEMENT.setFloat(31,(float)this.P200_temp_fanR);
        INSERT_PREP_STATEMENT.setFloat(32,(float)this.P200_temp_fanL);
        INSERT_PREP_STATEMENT.setFloat(33,(float)this.P200_temp_catwalk);
        INSERT_PREP_STATEMENT.setFloat(34,(float)this.P200_temp_ArchR);
        INSERT_PREP_STATEMENT.setFloat(35,(float)this.P200_temp_ArchL);
        INSERT_PREP_STATEMENT.setFloat(36,(float)this.P200_temp_gantry_farR);
        INSERT_PREP_STATEMENT.setFloat(37,(float)this.P200_temp_gantry_farL);
        
        System.out.println(INSERT_PREP_STATEMENT.toString());
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
    String query = "INSERT INTO p200weather ("
    + " TIMESTAMP_UTC,"
    + " UTCDATETIME,"
    + " LOCALDATETIME,"
    + " WEATHER_TIMESTAMP,"        
    + " TEMPERATURE_TIMESTAMP,"        
    + " AVERAGE_WINDSPEED,"
    + " PEAK_WINDSPEED,"
    + " GUST_WINDSPEED,"
    + " WIND_DIRECTION,"
    + " DEWPOINT,"
    + " OUTSIDE_TEMPERATURE,"
    + " INSIDE_TEMPERATURE,"
    + " PRIMARY_MIRROR_TEMPERATURE,"
    + " SECONDARY_MIRROR_TEMPERATURE,"
    + " FLOOR_TEMPERATURE,"
    + " PRIME_FOCUS_TEMPERATURE,"
    + " RAINING,"
    + " STAIRS_0,"
    + " STAIRS_20,"
    + " STAIRS_40,"
    + " STAIRS_60,"
    + " STAIRS_75,"
    + " STAIRS_90,"
    + " GANTRY_UR,"
    + " GAINTRY_LR,"
    + " GANTRY_UL,"
    + " GANTRY_LL,"
    + " CRANE,"
    + " EXHAUST,"
    + " SHUTTER,"
    + " FAN_R,"
    + " FAN_L,"
    + " CATWALK,"
    + " ARCH_R,"
    + " ARCH_L,"
    + " GANTRY_FAR_R,"
    + " GANTRY_FAR_L"
    + " ) VALUES ("
    +  " ?, ?, ?, ?, ?, ?, ?, ?, ?, ?,"
    +  " ?, ?, ?, ?, ?, ?, ?, ?, ?, ?,"
    +  " ?, ?, ?, ?, ?, ?, ?, ?, ?, ?,"
    +  " ?, ?, ?, ?, ?, ?, ?"
    +  ")";
    try{
       INSERT_PREP_STATEMENT = conn.prepareStatement(query);
    }catch(Exception e){
        logMessage(ERROR,"Creating prepared statement. "+e.toString());
    }
}  
/*================================================================================================
/       retrieveWeatherFile()
/=================================================================================================*/
  public void readTemperatureFile(){
     java.lang.String last_line = new java.lang.String(); 
      try{
         temperature_arraylist.clear();
         retrieveTemperatureFile(); 
         BufferedReader br = new BufferedReader(new FileReader(mySFTPRetrieveWeather.LOCAL_ROOT_DIR+SEP+constructTemperatureFilename()));
         boolean state = true;
         while(state){
           try{
             java.lang.String currentline = br.readLine();
             if(currentline != null){
                temperature_arraylist.add(currentline);    
             } 
             if(currentline == null){
                state = false;
             }
           }catch(Exception e){
              state = false; 
           } 
         }
         int count = temperature_arraylist.size();
         last_line = (java.lang.String)temperature_arraylist.get(count-1);
         br.close();
         parse_temperatures(last_line);
      }catch(Exception e){
          System.out.println(e.toString());
      }
  }   
/*================================================================================================
/      parse(java.lang.String weather_string)
/=================================================================================================*/
  public boolean parse_weather(java.lang.String weather_string){
    boolean status             = false;
    long raw_timestamp         = 0;
    double dewpoint            = MISSING_VALUE;
    double outside_air_temp    = MISSING_VALUE;
    double dome_air_temp       = MISSING_VALUE;
    double primary_mirror_temp = MISSING_VALUE;
    double prime_focus_temp    = MISSING_VALUE;
    double floor_temp          = MISSING_VALUE;
    double primary_cell_temp   = MISSING_VALUE;
    double windspeed_median    = MISSING_VALUE;
    double windspeed_peak      = MISSING_VALUE;
    double windspeed_stddev    = MISSING_VALUE;
    double wind_direction      = MISSING_VALUE;    
    boolean raining            = false;
       
      try{
          java.util.StringTokenizer st = new java.util.StringTokenizer(weather_string," ");
          int count = 0;
          while(st.hasMoreElements()){
              java.lang.String current = st.nextToken();
              switch(count){
                  case 0:  raw_timestamp       = (Long.valueOf(current)).longValue(); break;
                  case 1:  windspeed_median    = (Double.valueOf(current)).doubleValue(); break;
                  case 2:  windspeed_peak      = (Double.valueOf(current)).doubleValue(); break;
                  case 3:  windspeed_stddev    = (Double.valueOf(current)).doubleValue(); break;
                  case 4:  wind_direction      = (Double.valueOf(current)).doubleValue(); break;
                  case 5:  dewpoint            = (Double.valueOf(current)).doubleValue(); break;
                  case 6:  outside_air_temp    = (Double.valueOf(current)).doubleValue(); break;
                  case 7:  dome_air_temp       = (Double.valueOf(current)).doubleValue(); break;
                  case 8:  primary_mirror_temp = (Double.valueOf(current)).doubleValue(); break;
                  case 9:  prime_focus_temp    = (Double.valueOf(current)).doubleValue(); break;
                  case 10: floor_temp          = (Double.valueOf(current)).doubleValue(); break;
                  case 11: primary_cell_temp   = (Double.valueOf(current)).doubleValue(); break;
                  case 12: raining             = (Boolean.valueOf(current)).booleanValue(); break;
              }
              count = count +1;
          }
          if(raw_timestamp > getP200_reference_weather_timestamp()){
              setP200_dewpoint(dewpoint);
              setP200_outside_air_temp(outside_air_temp);
              setP200_dome_air_ttemp(dome_air_temp);
              setP200_primary_mirror_temp(primary_mirror_temp);
              setP200_prime_focus_temp(prime_focus_temp);
              setP200_floor_temp(floor_temp);
              setP200_primary_cell_temp(primary_cell_temp);
              setP200_windspeed_median(windspeed_median);
              setP200_windspeed_peak(windspeed_peak);
              setP200_windspeed_stddev(windspeed_stddev);
              setP200_wind_direction(wind_direction);  
              setP200_raining(raining);
              // now set the timestamp to compare with the last record
              setP200_reference_weather_timestamp(raw_timestamp);
              status = true;
          }
          if(isFirstWeatherTimestamp()){
              setP200_reference_weather_timestamp(raw_timestamp);
              setFirstWeatherTimestamp(false);
          }
      }catch(Exception e){
          System.out.println("Error parsing the P200 weather record"+e.toString());
      }
      return status;
  }
 /*================================================================================================
/      parse(java.lang.String weather_string)
/=================================================================================================*/
  public void parse_temperatures(java.lang.String weather_string){
    long          raw_timestamp    = 0;
    double        temp_stairs0     = MISSING_VALUE;
    double        temp_stairs20    = MISSING_VALUE;
    double        temp_stairs40    = MISSING_VALUE;
    double        temp_stairs60    = MISSING_VALUE;
    double        temp_stairs75    = MISSING_VALUE;
    double        temp_stairs90    = MISSING_VALUE;
    double        temp_gantryUR    = MISSING_VALUE;
    double        temp_gantryLR    = MISSING_VALUE;
    double        temp_gantryUL    = MISSING_VALUE;
    double        temp_gantryLL    = MISSING_VALUE;
    double        temp_crane       = MISSING_VALUE; 
    double        temp_exhaust     = MISSING_VALUE;  
    double        temp_shutter     = MISSING_VALUE;  
    double        temp_fanR        = MISSING_VALUE;
    double        temp_fanL        = MISSING_VALUE;        
    double        temp_catwalk     = MISSING_VALUE;                        
    double        temp_gantry_farR = MISSING_VALUE;
    double        temp_gantry_farL = MISSING_VALUE;
    double        temp_ArchL       = MISSING_VALUE;
    double        temp_ArchR       = MISSING_VALUE;
      try{
          java.util.StringTokenizer st = new java.util.StringTokenizer(weather_string," ");
          int count = 0;
          while(st.hasMoreElements()){
              java.lang.String current = st.nextToken();
              switch(count){
                  case 0:  raw_timestamp    = (Long.valueOf(current)).longValue(); break;
                  case 1:  temp_stairs0     = (Double.valueOf(current)).doubleValue(); break;
                  case 2:  temp_stairs20    = (Double.valueOf(current)).doubleValue(); break;
                  case 3:  temp_stairs40    = (Double.valueOf(current)).doubleValue(); break;
                  case 4:  temp_stairs60    = (Double.valueOf(current)).doubleValue(); break;
                  case 5:  temp_stairs75    = (Double.valueOf(current)).doubleValue(); break;
                  case 6:  temp_stairs90    = (Double.valueOf(current)).doubleValue(); break;
                  case 7:  temp_gantryUR    = (Double.valueOf(current)).doubleValue(); break;
                  case 8:  temp_gantryLR    = (Double.valueOf(current)).doubleValue(); break;
                  case 9:  temp_gantryUL    = (Double.valueOf(current)).doubleValue(); break;
                  case 10: temp_gantryLL    = (Double.valueOf(current)).doubleValue(); break;
                  case 11: temp_crane       = (Double.valueOf(current)).doubleValue(); break;                      
                  case 12: temp_exhaust     = (Double.valueOf(current)).doubleValue(); break;
                  case 13: temp_shutter     = (Double.valueOf(current)).doubleValue(); break;
                  case 14: temp_fanR        = (Double.valueOf(current)).doubleValue(); break;
                  case 15: temp_fanL        = (Double.valueOf(current)).doubleValue(); break;
                  case 16: temp_catwalk     = (Double.valueOf(current)).doubleValue(); break;                      
                  case 17: temp_gantry_farR = (Double.valueOf(current)).doubleValue(); break;
                  case 18: temp_gantry_farL = (Double.valueOf(current)).doubleValue(); break;
                  case 19: temp_ArchL       = (Double.valueOf(current)).doubleValue(); break;
                  case 20: temp_ArchR       = (Double.valueOf(current)).doubleValue(); break;
              }
              count = count +1;
          }
          if(isFirstTemperatureTimestamp()){
              setP200_reference_temperature_timestamp(raw_timestamp);
              setFirstTemperatureTimestamp(false);
          }
          if(raw_timestamp >= getP200_reference_temperature_timestamp()){
              setP200_temp_stairs0(temp_stairs0);
              setP200_temp_stairs20(temp_stairs20);
              setP200_temp_stairs40(temp_stairs40);
              setP200_temp_stairs60(temp_stairs60);
              setP200_temp_stairs75(temp_stairs75);
              setP200_temp_stairs90(temp_stairs90);
              setP200_temp_gantryUR(temp_gantryUR);
              setP200_temp_gantryLR(temp_gantryLR);
              setP200_temp_gantryUL(temp_gantryUL);
              setP200_temp_gantryLL(temp_gantryLL);
              setP200_temp_crane(temp_crane);
              setP200_temp_exhaust(temp_exhaust);
              setP200_temp_shutter(temp_shutter);
              setP200_temp_fanR(temp_fanR);
              setP200_temp_fanL(temp_fanL);
              setP200_temp_catwalk(temp_catwalk);
              setP200_temp_gantry_farR(temp_gantry_farR);
              setP200_temp_gantry_farL(temp_gantry_farL);
              setP200_temp_ArchL(temp_ArchL);
              setP200_temp_ArchR(temp_ArchR);
              // now set the timestamp to compare with the last record
              setP200_reference_temperature_timestamp(raw_timestamp);
          }
      }catch(Exception e){
          System.out.println("Error parsing the P200 temperature record"+e.toString());
      }
  } 
/*================================================================================================
/       initializeValues()
/=================================================================================================*/
private void initializeValues(){
//  P200 DOME WEATHER PARAMETERS    
      setP200_dewpoint(MISSING_VALUE);
      setP200_outside_air_temp(MISSING_VALUE);
      setP200_dome_air_ttemp(MISSING_VALUE);
      setP200_primary_mirror_temp(MISSING_VALUE);
      setP200_prime_focus_temp(MISSING_VALUE);
      setP200_floor_temp(MISSING_VALUE);
      setP200_primary_cell_temp(MISSING_VALUE);
      setP200_windspeed_median(MISSING_VALUE);
      setP200_windspeed_peak(MISSING_VALUE);
      setP200_windspeed_stddev(MISSING_VALUE);
      setP200_wind_direction(MISSING_VALUE);  
      setP200_raining(false);
//   P200 DOME TEMPERATURES    
      setP200_temp_stairs0(MISSING_VALUE);
      setP200_temp_stairs20(MISSING_VALUE);
      setP200_temp_stairs40(MISSING_VALUE);
      setP200_temp_stairs60(MISSING_VALUE);
      setP200_temp_stairs75(MISSING_VALUE);
      setP200_temp_stairs90(MISSING_VALUE);
      setP200_temp_gantryUR(MISSING_VALUE);
      setP200_temp_gantryLR(MISSING_VALUE);
      setP200_temp_gantryUL(MISSING_VALUE);
      setP200_temp_gantryLL(MISSING_VALUE);
      setP200_temp_crane(MISSING_VALUE);
      setP200_temp_exhaust(MISSING_VALUE);
      setP200_temp_shutter(MISSING_VALUE);
      setP200_temp_fanR(MISSING_VALUE);
      setP200_temp_fanL(MISSING_VALUE);
      setP200_temp_catwalk(MISSING_VALUE);
      setP200_temp_gantry_farR(MISSING_VALUE);
      setP200_temp_gantry_farL(MISSING_VALUE);
      setP200_temp_ArchL(MISSING_VALUE);
      setP200_temp_ArchR(MISSING_VALUE);   
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
public java.lang.String weatherToGson(P200weather current){
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
       currentHeader.addLine(new HeaderCard("P2UTCTS",UTC_TIMESTAMP.toString(),"P200 UTC timestamp"));
       currentHeader.addLine(new HeaderCard("P2UTCST",UTC_DATE_TIME,"P200 UTC date-time string"));
       currentHeader.addLine(new HeaderCard("P2LDT",LOCAL_DATE_TIME,"Local date and time"));
       currentHeader.addLine(new HeaderCard("P2WEATIM",getP200_reference_weather_timestamp(),"P200 Time stamp for the weather record"));
       currentHeader.addLine(new HeaderCard("P2TEMPIM",getP200_reference_temperature_timestamp(),"P200 timestamp for the temperature record"));
       currentHeader.addLine(new HeaderCard("P2WAW",getP200_windspeed_median(),"P200 average windspeed"));                
       currentHeader.addLine(new HeaderCard("P2WPW",getP200_windspeed_peak(),"P200 peak windspeed"));
       currentHeader.addLine(new HeaderCard("P2WG@",getP200_windspeed_stddev(),"P200 gust windspeed"));
       currentHeader.addLine(new HeaderCard("P2WWD",getP200_wind_direction(),"P200 wind direction"));
       currentHeader.addLine(new HeaderCard("P2WDEWPT",this.getP200_dewpoint(),"P200 dewpoint"));
       currentHeader.addLine(new HeaderCard("P2WITEMP",this.getP200_dome_air_ttemp(),"P200 inside dome air temperature"));
       currentHeader.addLine(new HeaderCard("P2WOTEMP",this.getP200_outside_air_temp(),"P200 outside air temperature"));
       currentHeader.addLine(new HeaderCard("P2WPMT",this.getP200_primary_mirror_temp(),"P200 primary mirror temperature"));
       currentHeader.addLine(new HeaderCard("P2WSMTL",this.getP200_primary_cell_temp(),"P200 secondary mirror prime cell temperature"));
       currentHeader.addLine(new HeaderCard("P2WFT",this.getP200_floor_temp(),"P200 floor temperature"));
       currentHeader.addLine(new HeaderCard("P2WPRFT",this.getP200_prime_focus_temp(),"P200 prime focus temperature"));
       currentHeader.addLine(new HeaderCard("P2RAIN",this.isP200_raining(),"P200 raining?"));
       currentHeader.addLine(new HeaderCard("P2WS0TP",this.getP200_temp_stairs0(),"P200 stairs 0 temperature"));
       currentHeader.addLine(new HeaderCard("P2WS20TP",this.getP200_temp_stairs0(),"P200 stairs 20 temperature"));
       currentHeader.addLine(new HeaderCard("P2WS40TP",this.getP200_temp_stairs0(),"P200 stairs 40 temperature"));
       currentHeader.addLine(new HeaderCard("P2WS60TP",this.getP200_temp_stairs0(),"P200 stairs 60 temperature"));
       currentHeader.addLine(new HeaderCard("P2WS75TP",this.getP200_temp_stairs0(),"P200 stairs 75 temperature"));
       currentHeader.addLine(new HeaderCard("P2WS90TP",this.getP200_temp_stairs0(),"P200 stairs 90 temperature"));
       currentHeader.addLine(new HeaderCard("P2WGUR",this.getP200_temp_gantryUR(),"P200 gantry upper right"));
       currentHeader.addLine(new HeaderCard("P2WGLR",this.getP200_temp_gantryLR(),"P200 gantry lower right"));
       currentHeader.addLine(new HeaderCard("P2WGUL",this.getP200_temp_gantryUL(),"P200 gantry upper left"));
       currentHeader.addLine(new HeaderCard("P2WGLL",this.getP200_temp_gantryLL(),"P200 gantry lower left"));
       currentHeader.addLine(new HeaderCard("P2WCRAN",this.getP200_temp_crane(),"P200 crane temperature"));
       currentHeader.addLine(new HeaderCard("P2WEXHAU",this.getP200_temp_exhaust(),"P200 exhaust temperature"));
       currentHeader.addLine(new HeaderCard("P2WSHUT",this.getP200_temp_shutter(),"P200 shutter temperature"));
       currentHeader.addLine(new HeaderCard("P2WFANR",this.getP200_temp_fanR(),"P200 fan right temperature"));
       currentHeader.addLine(new HeaderCard("P2WFANL",this.getP200_temp_fanL(),"P200 fan left temperature"));
       currentHeader.addLine(new HeaderCard("P2WCATW",this.getP200_temp_catwalk(),"P200 catwalk temperature"));
       currentHeader.addLine(new HeaderCard("P2WARCR",this.getP200_temp_ArchR(),"P200 arch right temperature"));
       currentHeader.addLine(new HeaderCard("P2WARCL",this.getP200_temp_ArchL(),"P200 arch left temperature"));
       currentHeader.addLine(new HeaderCard("P2WGFR",this.getP200_temp_gantry_farR(),"P200 gantry far right temperature"));
       currentHeader.addLine(new HeaderCard("P2WGFL",this.getP200_temp_gantry_farL(),"P200 gantry far left temperature"));                                          
//       setCURRENT_HEADER(currentHeader);
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
/     setP200_utc_timestamp(java.lang.String new_p200_utc_timestamp) 
/=================================================================================================*/
  public void setP200_utc_timestamp(java.lang.String new_p200_utc_timestamp) {
     java.lang.String  old_p200_utc_timestamp = this.p200_utc_timestamp;
     this.p200_utc_timestamp = new_p200_utc_timestamp;
     current.p200_utc_timestamp =p200_utc_timestamp;
     propertyChangeListeners.firePropertyChange("p200_utc_timestamp", old_p200_utc_timestamp, p200_utc_timestamp);
  }
  public java.lang.String getP200_utc_timestamp() {
     return p200_utc_timestamp;
  }  
/*================================================================================================
/     setP200_reference_timestamp
/=================================================================================================*/
  public void setP200_reference_weather_timestamp(long new_weather_reference_timestamp) {
     long  old_reference_timestamp = this.weather_reference_timestamp;
     this.weather_reference_timestamp = new_weather_reference_timestamp;
     current.weather_reference_timestamp = weather_reference_timestamp;
     propertyChangeListeners.firePropertyChange("weather_reference_timestamp", Long.valueOf(old_reference_timestamp), Long.valueOf(weather_reference_timestamp));
  }
  public long getP200_reference_weather_timestamp() {
     return weather_reference_timestamp;
  }  
  /*================================================================================================
/     setP200_reference_timestamp
/=================================================================================================*/
  public void setP200_reference_temperature_timestamp(long new_temperature_reference_timestamp) {
     long  old_temperature_reference_timestamp = this.temperature_reference_timestamp;
     this.temperature_reference_timestamp = new_temperature_reference_timestamp;
     current.temperature_reference_timestamp = temperature_reference_timestamp;
     propertyChangeListeners.firePropertyChange("temperature_reference_timestamp", Long.valueOf(old_temperature_reference_timestamp), Long.valueOf(temperature_reference_timestamp));
  }
  public long getP200_reference_temperature_timestamp() {
     return temperature_reference_timestamp;
  } 
/*================================================================================================
/       settP200_dewpoint
/=================================================================================================*/
  public void setP200_dewpoint(double new_P200_dewpoint) {
     double  old_P200_dewpoint = this.P200_dewpoint;
     this.P200_dewpoint = new_P200_dewpoint;
     current.P200_dewpoint = P200_dewpoint;
     propertyChangeListeners.firePropertyChange("P200_dewpoint", Double.valueOf(old_P200_dewpoint), Double.valueOf(P200_dewpoint));
  }
  public double getP200_dewpoint() {
     return P200_dewpoint;
  } 
/*================================================================================================
/       set
/=================================================================================================*/
  public void setP200_outside_air_temp(double new_P200_outside_air_temp) {
     double  old_P200_outside_air_temp = this.P200_outside_air_temp;
     this.P200_outside_air_temp = new_P200_outside_air_temp;
     current.P200_outside_air_temp = P200_outside_air_temp;
     propertyChangeListeners.firePropertyChange("P200_outside_air_temperature", Double.valueOf(old_P200_outside_air_temp),Double.valueOf(P200_outside_air_temp));
  }
  public double getP200_outside_air_temp() {
     return P200_outside_air_temp;
  } 
  /*================================================================================================
/       setP200_dome_air_ttemp
/=================================================================================================*/
  public void setP200_dome_air_ttemp(double new_P200_dome_air_temp) {
     double  old_P200_dome_air_temp = this.P200_dome_air_temp;
     this.P200_dome_air_temp = new_P200_dome_air_temp;
     current.P200_dome_air_temp = P200_dome_air_temp;
     propertyChangeListeners.firePropertyChange("P200_dome_air_ttemp", Double.valueOf(old_P200_dome_air_temp),Double.valueOf(P200_dome_air_temp));
  }
  public double getP200_dome_air_ttemp() {
     return P200_dome_air_temp;
  } 
  /*================================================================================================
/       setP200_primary_mirror_temp
/=================================================================================================*/
  public void setP200_primary_mirror_temp(double new_P200_primary_mirror_temp) {
     double  old_P200_primary_mirror_temp = this.P200_primary_mirror_temp;
     this.P200_primary_mirror_temp = new_P200_primary_mirror_temp;
     current.P200_primary_mirror_temp = P200_primary_mirror_temp;
     propertyChangeListeners.firePropertyChange("P200_primary_mirror_temp", Double.valueOf(old_P200_primary_mirror_temp), Double.valueOf(P200_primary_mirror_temp));
  }
  public double getP200_primary_mirror_temp() {
     return P200_primary_mirror_temp;
  } 
  /*================================================================================================
/       setP200_prime_focus_temp
/=================================================================================================*/
  public void setP200_prime_focus_temp(double new_P200_prime_focus_temp) {
     double  old_P200_prime_focus_temp = this.P200_prime_focus_temp;
     this.P200_prime_focus_temp = new_P200_prime_focus_temp;
     current.P200_prime_focus_temp = P200_prime_focus_temp;
     propertyChangeListeners.firePropertyChange("P200_prime_focus_temp", Double.valueOf(old_P200_prime_focus_temp), Double.valueOf(P200_prime_focus_temp));
  }
  public double getP200_prime_focus_temp() {
     return P200_prime_focus_temp;
  } 
/*================================================================================================
/       setP200_floor_temp
/=================================================================================================*/
  public void setP200_floor_temp(double new_P200_floor_temp) {
     double  old_P200_floor_temp = this.P200_floor_temp;
     this.P200_floor_temp = new_P200_floor_temp;
     current.P200_floor_temp = P200_floor_temp;
     propertyChangeListeners.firePropertyChange("P200_floor_temp", Double.valueOf(old_P200_floor_temp), Double.valueOf(P200_floor_temp));
  }
  public double getP200_floor_temp() {
     return P200_floor_temp;
  } 
  /*================================================================================================
/       setP200_primary_cell_temp
/=================================================================================================*/
  public void setP200_primary_cell_temp(double new_P200_primary_cell_temp) {
     double  old_P200_primary_cell_temp = this.P200_primary_cell_temp;
     this.P200_primary_cell_temp = new_P200_primary_cell_temp;
     current.P200_primary_cell_temp = P200_primary_cell_temp;
     propertyChangeListeners.firePropertyChange("P200_primary_cell_temp", Double.valueOf(old_P200_primary_cell_temp),Double.valueOf(P200_primary_cell_temp));
  }
  public double getP200_primary_cell_temp() {
     return P200_primary_cell_temp;
  } 
  /*================================================================================================
/       setP200_windspeed_median
/=================================================================================================*/
  public void setP200_windspeed_median(double new_P200_windspeed_median) {
     double  old_P200_windspeed_median = this.P200_windspeed_median;
     this.P200_windspeed_median = new_P200_windspeed_median;
     current.P200_windspeed_median = P200_windspeed_median;
     propertyChangeListeners.firePropertyChange("P200_windspeed_median", Double.valueOf(old_P200_windspeed_median),Double.valueOf(P200_windspeed_median));
  }
  public double getP200_windspeed_median() {
     return P200_windspeed_median;
  } 
  /*================================================================================================
/       setP200_windspeed_peak
/=================================================================================================*/
  public void setP200_windspeed_peak(double new_P200_windspeed_peak) {
     double  old_P200_windspeed_peak = this.P200_windspeed_peak;
     this.P200_windspeed_peak = new_P200_windspeed_peak;
     current.P200_windspeed_peak = P200_windspeed_peak;
     propertyChangeListeners.firePropertyChange("P200_windspeed_peak", Double.valueOf(old_P200_windspeed_peak),Double.valueOf(P200_windspeed_peak));
  }
  public double getP200_windspeed_peak() {
     return P200_windspeed_peak;
  } 
  /*================================================================================================
/       setP200_windspeed_stddev
/=================================================================================================*/
  public void setP200_windspeed_stddev(double new_P200_windspeed_stddev) {
     double  old_P200_windspeed_stddev = this.P200_windspeed_stddev;
     this.P200_windspeed_stddev = new_P200_windspeed_stddev;
     current.P200_windspeed_stddev = P200_windspeed_stddev;
     propertyChangeListeners.firePropertyChange("P200_windspeed_stddev", Double.valueOf(old_P200_windspeed_stddev),Double.valueOf(P200_windspeed_stddev));
  }
  public double getP200_windspeed_stddev() {
     return P200_windspeed_stddev;
  } 
/*================================================================================================
/       setP200_wind_direction
/=================================================================================================*/
  public void setP200_wind_direction(double new_P200_wind_direction) {
     double  old_P200_wind_direction = this.P200_wind_direction;
     this.P200_wind_direction = new_P200_wind_direction;
     current.P200_wind_direction = P200_wind_direction;
     propertyChangeListeners.firePropertyChange("P200_wind_direction", Double.valueOf(old_P200_wind_direction),Double.valueOf(P200_wind_direction));
  }
  public double getP200_wind_direction() {
     return P200_wind_direction;
  } 
/*================================================================================================
/       setP200_wind_direction
/=================================================================================================*/
  public void setP200_raining(boolean new_P200_raining) {
     boolean  old_P200_raining = this.P200_raining;
     this.P200_raining = new_P200_raining;
     current.P200_raining = P200_raining;
     propertyChangeListeners.firePropertyChange("raining",Boolean.valueOf(old_P200_raining),Boolean.valueOf(P200_raining));
  }
  public boolean isP200_raining() {
     return P200_raining;
  }
/*================================================================================================
/       setP200_wind_direction
/=================================================================================================*/
  public void setFirstWeatherTimestamp(boolean new_first_timestamp) {
     boolean  old_first_timestamp = this.first_timestamp;
     this.first_timestamp = new_first_timestamp;
     propertyChangeListeners.firePropertyChange("first_weather_timestamp", Boolean.valueOf(old_first_timestamp),Boolean.valueOf(first_timestamp));
  }
  public boolean isFirstWeatherTimestamp() {
     return first_timestamp;
  } 
/*================================================================================================
/       setFirstTemperatureTimestamp(boolean new_first_temperature_timestamp)
/=================================================================================================*/
  public void setFirstTemperatureTimestamp(boolean new_first_temperature_timestamp) {
     boolean  old_first_temperature_timestamp = this.first_temperature_timestamp;
     this.first_temperature_timestamp = new_first_temperature_timestamp;
     propertyChangeListeners.firePropertyChange("first_temperature_timestamp", Boolean.valueOf(old_first_temperature_timestamp), Boolean.valueOf(first_temperature_timestamp));
  }
  public boolean isFirstTemperatureTimestamp() {
     return first_temperature_timestamp;
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
/*================================================================================================
/       setP200_temp_stairs0(double new_P200_temp_stairs0)
/=================================================================================================*/
  public void setP200_temp_stairs0(double new_P200_temp_stairs0) {
     double  old_P200_temp_stairs0 = this.P200_temp_stairs0;
     this.P200_temp_stairs0 = new_P200_temp_stairs0;
     current.P200_temp_stairs0 = P200_temp_stairs0;
     propertyChangeListeners.firePropertyChange("P200_temp_stairs0", Double.valueOf(old_P200_temp_stairs0), Double.valueOf(P200_temp_stairs0));
  }
  public double getP200_temp_stairs0() {
     return P200_temp_stairs0;
  }   
/*================================================================================================
/       setP200_temp_stairs0(double new_P200_temp_stairs0)
/=================================================================================================*/
  public void setP200_temp_stairs20(double new_P200_temp_stairs20) {
     double  old_P200_temp_stairs20 = this.P200_temp_stairs20;
     this.P200_temp_stairs20 = new_P200_temp_stairs20;
     current.P200_temp_stairs20 = P200_temp_stairs20;
     propertyChangeListeners.firePropertyChange("P200_temp_stairs20", Double.valueOf(old_P200_temp_stairs20), Double.valueOf(P200_temp_stairs20));
  }
  public double getP200_temp_stairs20() {
     return P200_temp_stairs20;
  }    
 /*================================================================================================
/       setP200_temp_stairs0(double new_P200_temp_stairs0)
/=================================================================================================*/
  public void setP200_temp_stairs40(double new_P200_temp_stairs40) {
     double  old_P200_temp_stairs40 = this.P200_temp_stairs40;
     this.P200_temp_stairs40 = new_P200_temp_stairs40;
     current.P200_temp_stairs40 = P200_temp_stairs40;
     propertyChangeListeners.firePropertyChange("P200_temp_stairs40", Double.valueOf(old_P200_temp_stairs40), Double.valueOf(P200_temp_stairs40));
  }
  public double getP200_temp_stairs40() {
     return P200_temp_stairs40;
  }   
 /*================================================================================================
/       setP200_temp_stairs0(double new_P200_temp_stairs0)
/=================================================================================================*/
  public void setP200_temp_stairs60(double new_P200_temp_stairs60) {
     double  old_P200_temp_stairs60 = this.P200_temp_stairs60;
     this.P200_temp_stairs60 = new_P200_temp_stairs60;
     current.P200_temp_stairs60 = P200_temp_stairs60;
     propertyChangeListeners.firePropertyChange("P200_temp_stairs60", Double.valueOf(old_P200_temp_stairs60), Double.valueOf(P200_temp_stairs60));
  }
  public double getP200_temp_stairs60() {
     return P200_temp_stairs60;
  }   
/*================================================================================================
/       setP200_temp_stairs0(double new_P200_temp_stairs0)
/=================================================================================================*/
  public void setP200_temp_stairs75(double new_P200_temp_stairs75) {
     double  old_P200_temp_stairs75 = this.P200_temp_stairs75;
     this.P200_temp_stairs75 = new_P200_temp_stairs75;
     current.P200_temp_stairs75 = P200_temp_stairs75;
     propertyChangeListeners.firePropertyChange("P200_temp_stairs75", Double.valueOf(old_P200_temp_stairs75), Double.valueOf(P200_temp_stairs75));
  }
  public double getP200_temp_stairs75() {
     return P200_temp_stairs75;
  }    
 /*================================================================================================
/       setP200_temp_stairs0(double new_P200_temp_stairs0)
/=================================================================================================*/
  public void setP200_temp_stairs90(double new_P200_temp_stairs90) {
     double  old_P200_temp_stairs90 = this.P200_temp_stairs90;
     this.P200_temp_stairs90 = new_P200_temp_stairs90;
     current.P200_temp_stairs90 = P200_temp_stairs90;
     propertyChangeListeners.firePropertyChange("P200_temp_stairs90", Double.valueOf(old_P200_temp_stairs90), Double.valueOf(P200_temp_stairs90));
  }
  public double getP200_temp_stairs90() {
     return P200_temp_stairs90;
  }  
 /*================================================================================================
/      setP200_temp_gantryUR(double new_P200_temp_gantryUR)
/=================================================================================================*/
  public void setP200_temp_gantryUR(double new_P200_temp_gantryUR) {
     double  old_P200_temp_gantryUR = this.P200_temp_gantryUR;
     this.P200_temp_gantryUR = new_P200_temp_gantryUR;
     current.P200_temp_gantryUR = P200_temp_gantryUR;
     propertyChangeListeners.firePropertyChange("P200_temp_gantryUR", Double.valueOf(old_P200_temp_gantryUR),Double.valueOf(P200_temp_gantryUR));
  }
  public double getP200_temp_gantryUR() {
     return P200_temp_gantryUR;
  }   
 /*================================================================================================
/      setP200_temp_gantryUR(double new_P200_temp_gantryUR)
/=================================================================================================*/
  public void setP200_temp_gantryLR(double new_P200_temp_gantryLR) {
     double  old_P200_temp_gantryLR = this.P200_temp_gantryLR;
     this.P200_temp_gantryLR = new_P200_temp_gantryLR;
     current.P200_temp_gantryLR = P200_temp_gantryLR;
     propertyChangeListeners.firePropertyChange("P200_temp_gantryLR", Double.valueOf(old_P200_temp_gantryLR),Double.valueOf(P200_temp_gantryLR));
  }
  public double getP200_temp_gantryLR() {
     return P200_temp_gantryLR;
  }
   /*================================================================================================
/      setP200_temp_gantryUR(double new_P200_temp_gantryUR)
/=================================================================================================*/
  public void setP200_temp_gantryUL(double new_P200_temp_gantryUL) {
     double  old_P200_temp_gantryUL = this.P200_temp_gantryUL;
     this.P200_temp_gantryUL = new_P200_temp_gantryUL;
     current.P200_temp_gantryUL = P200_temp_gantryUL;
     propertyChangeListeners.firePropertyChange("P200_temp_gantryUL", Double.valueOf(old_P200_temp_gantryUL), Double.valueOf(P200_temp_gantryUL));
  }
  public double getP200_temp_gantryUL() {
     return P200_temp_gantryUL;
  }
   /*================================================================================================
/      setP200_temp_gantryUR(double new_P200_temp_gantryUR)
/=================================================================================================*/
  public void setP200_temp_gantryLL(double new_P200_temp_gantryLL) {
     double  old_P200_temp_gantryLL = this.P200_temp_gantryLL;
     this.P200_temp_gantryLL = new_P200_temp_gantryLL;
     current.P200_temp_gantryLL = P200_temp_gantryLL;
     propertyChangeListeners.firePropertyChange("P200_temp_gantryLL", Double.valueOf(old_P200_temp_gantryLL),Double.valueOf(P200_temp_gantryLL));
  }
  public double getP200_temp_gantryLL() {
     return P200_temp_gantryLL;
  }
 /*================================================================================================
/      setP200_temp_gantryUR(double new_P200_temp_gantryUR)
/=================================================================================================*/
  public void setP200_temp_gantry_farR(double new_P200_temp_gantry_farR) {
     double  old_P200_temp_gantry_farR = this.P200_temp_gantry_farR;
     this.P200_temp_gantry_farR = new_P200_temp_gantry_farR;
     current.P200_temp_gantry_farR = P200_temp_gantry_farR;
     propertyChangeListeners.firePropertyChange("P200_temp_gantry_farR", Double.valueOf(old_P200_temp_gantry_farR), Double.valueOf(P200_temp_gantry_farR));
  }
  public double getP200_temp_gantry_farR() {
     return P200_temp_gantry_farR;
  }
/*================================================================================================
/      setP200_temp_gantryUR(double new_P200_temp_gantryUR)
/=================================================================================================*/
  public void setP200_temp_gantry_farL(double new_P200_temp_gantry_farL) {
     double  old_P200_temp_gantry_farL = this.P200_temp_gantry_farL;
     this.P200_temp_gantry_farL = new_P200_temp_gantry_farL;
     current.P200_temp_gantry_farL = P200_temp_gantry_farL;
     propertyChangeListeners.firePropertyChange("P200_temp_gantry_farL", Double.valueOf(old_P200_temp_gantry_farL),Double.valueOf(P200_temp_gantry_farL));
  }
  public double getP200_temp_gantry_farL() {
     return P200_temp_gantry_farL;
  }       
/*================================================================================================
/      setP200_temp_crane(double new_P200_temp_crane)
/=================================================================================================*/
  public void setP200_temp_crane(double new_P200_temp_crane) {
     double  old_P200_temp_crane = this.P200_temp_crane;
     this.P200_temp_crane = new_P200_temp_crane;
     current.P200_temp_crane = P200_temp_crane;
     propertyChangeListeners.firePropertyChange("P200_temp_crane", Double.valueOf(old_P200_temp_crane), Double.valueOf(P200_temp_crane));
  }
  public double getP200_temp_crane() {
     return P200_temp_crane;
  }  
/*================================================================================================
/      setP200_temp_exhaust(double new_P200_temp_exhaust)
/=================================================================================================*/
  public void setP200_temp_exhaust(double new_P200_temp_exhaust) {
     double  old_P200_temp_exhaust = this.P200_temp_exhaust;
     this.P200_temp_exhaust = new_P200_temp_exhaust;
     current.P200_temp_exhaust = P200_temp_exhaust;
     propertyChangeListeners.firePropertyChange("P200_temp_exhaust", Double.valueOf(old_P200_temp_exhaust), Double.valueOf(P200_temp_exhaust));
  }
  public double getP200_temp_exhaust() {
     return P200_temp_exhaust;
  }    
/*================================================================================================
/      setP200_temp_shutter(double new_P200_temp_shutter)
/=================================================================================================*/
  public void setP200_temp_shutter(double new_P200_temp_shutter) {
     double  old_P200_temp_shutter = this.P200_temp_shutter;
     this.P200_temp_shutter = new_P200_temp_shutter;
     current.P200_temp_shutter = P200_temp_shutter;
     propertyChangeListeners.firePropertyChange("P200_temp_shutter", Double.valueOf(old_P200_temp_shutter), Double.valueOf(P200_temp_shutter));
  }
  public double getP200_temp_shutter() {
     return P200_temp_shutter;
  }  
/*================================================================================================
/      setP200_temp_fanR(double new_P200_temp_fanR)
/=================================================================================================*/
  public void setP200_temp_fanR(double new_P200_temp_fanR) {
     double  old_P200_temp_fanR = this.P200_temp_fanR;
     this.P200_temp_fanR = new_P200_temp_fanR;
     current.P200_temp_fanR = P200_temp_fanR;
     propertyChangeListeners.firePropertyChange("P200_temp_fanR", Double.valueOf(old_P200_temp_fanR), Double.valueOf(P200_temp_fanR));
  }
  public double getP200_temp_fanR() {
     return P200_temp_fanR;
  }  
/*================================================================================================
/      setP200_temp_fanL(double new_P200_temp_fanL)
/=================================================================================================*/
  public void setP200_temp_fanL(double new_P200_temp_fanL) {
     double  old_P200_temp_fanL = this.P200_temp_fanL;
     this.P200_temp_fanL = new_P200_temp_fanL;
     current.P200_temp_fanL = P200_temp_fanL;
     propertyChangeListeners.firePropertyChange("P200_temp_fanL", Double.valueOf(old_P200_temp_fanL), Double.valueOf(P200_temp_fanL));
  }
  public double getP200_temp_fanL() {
     return P200_temp_fanL;
  }    
/*================================================================================================
/      setP200_temp_catwalk(double new_P200_temp_catwalk)
/=================================================================================================*/
  public void setP200_temp_catwalk(double new_P200_temp_catwalk) {
     double  old_P200_temp_catwalk = this.P200_temp_catwalk;
     this.P200_temp_catwalk = new_P200_temp_catwalk;
     current.P200_temp_catwalk = P200_temp_catwalk;
     propertyChangeListeners.firePropertyChange("P200_temp_catwalk", Double.valueOf(old_P200_temp_catwalk), Double.valueOf(P200_temp_catwalk));
  }
  public double getP200_temp_catwalk() {
     return P200_temp_catwalk;
  }         
/*================================================================================================
/      setP200_temp_gantryUR(double new_P200_temp_gantryUR)
/=================================================================================================*/
  public void setP200_temp_ArchR(double new_P200_temp_ArchR) {
     double  old_P200_temp_ArchR = this.P200_temp_ArchR;
     this.P200_temp_ArchR = new_P200_temp_ArchR;
     current.P200_temp_ArchR = P200_temp_ArchR;
     propertyChangeListeners.firePropertyChange("P200_temp_ArchR", Double.valueOf(old_P200_temp_ArchR), Double.valueOf(P200_temp_ArchR));
  }
  public double getP200_temp_ArchR() {
     return P200_temp_ArchR;
  }
/*================================================================================================
/      setP200_temp_gantryUR(double new_P200_temp_gantryUR)
/=================================================================================================*/
  public void setP200_temp_ArchL(double new_P200_temp_ArchL) {
     double  old_P200_temp_ArchL = this.P200_temp_ArchL;
     this.P200_temp_ArchL = new_P200_temp_ArchL;
     current.P200_temp_ArchL = P200_temp_ArchL;
     propertyChangeListeners.firePropertyChange("P200_temp_ArchL", Double.valueOf(old_P200_temp_ArchL),Double.valueOf(P200_temp_ArchL));
  }
  public double getP200_temp_ArchL() {
     return P200_temp_ArchL;
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
           this.waitForResponseMilliseconds((int)WEATHER_POLLING*1000);
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
/      public static void main(String args[])
/=================================================================================================*/
      public static void main(String args[]) {
        /* Set the Nimbus look and feel */
       new P200WeatherObject(); //<editor-fold defaultstate="collapsed" desc=" Look and feel setting code (optional) ">
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
/*     propertyChangeListeners.firePropertyChange("weather_reference_timestamp", new Long(old_reference_timestamp), new Long(weather_reference_timestamp));
     propertyChangeListeners.firePropertyChange("temperature_reference_timestamp", new Long(old_temperature_reference_timestamp), new Long(temperature_reference_timestamp));
     propertyChangeListeners.firePropertyChange("first_weather_timestamp", new Boolean(old_first_timestamp), new Boolean(first_timestamp));
     propertyChangeListeners.firePropertyChange("first_temperature_timestamp", new Boolean(old_first_temperature_timestamp), new Boolean(first_temperature_timestamp));

     propertyChangeListeners.firePropertyChange("P200_dewpoint", new Double(old_P200_dewpoint), new Double(P200_dewpoint));
     propertyChangeListeners.firePropertyChange("P200_outside_air_temperature", new Double(old_P200_outside_air_temp), new Double(P200_outside_air_temp));
     propertyChangeListeners.firePropertyChange("P200_dome_air_ttemp", new Double(old_P200_dome_air_temp), new Double(P200_dome_air_temp));
     propertyChangeListeners.firePropertyChange("P200_primary_mirror_temp", new Double(old_P200_primary_mirror_temp), new Double(P200_primary_mirror_temp));
     propertyChangeListeners.firePropertyChange("P200_prime_focus_temp", new Double(old_P200_prime_focus_temp), new Double(P200_prime_focus_temp));
     propertyChangeListeners.firePropertyChange("P200_floor_temp", new Double(old_P200_floor_temp), new Double(P200_floor_temp));
     propertyChangeListeners.firePropertyChange("P200_primary_cell_temp", new Double(old_P200_primary_cell_temp), new Double(P200_primary_cell_temp));
     propertyChangeListeners.firePropertyChange("P200_windspeed_median", new Double(old_P200_windspeed_median), new Double(P200_windspeed_median));
     propertyChangeListeners.firePropertyChange("P200_windspeed_peak", new Double(old_P200_windspeed_peak), new Double(P200_windspeed_peak));
     propertyChangeListeners.firePropertyChange("P200_windspeed_stddev", new Double(old_P200_windspeed_stddev), new Double(P200_windspeed_stddev));
     propertyChangeListeners.firePropertyChange("P200_wind_direction", new Double(old_P200_wind_direction), new Double(P200_wind_direction));
     propertyChangeListeners.firePropertyChange("raining", new Boolean(old_P200_raining), new Boolean(P200_raining));
     propertyChangeListeners.firePropertyChange("P200_temp_stairs0", new Double(old_P200_temp_stairs0), new Double(P200_temp_stairs0));
     propertyChangeListeners.firePropertyChange("P200_temp_stairs20", new Double(old_P200_temp_stairs20), new Double(P200_temp_stairs20));
     propertyChangeListeners.firePropertyChange("P200_temp_stairs40", new Double(old_P200_temp_stairs40), new Double(P200_temp_stairs40));
     propertyChangeListeners.firePropertyChange("P200_temp_stairs60", new Double(old_P200_temp_stairs60), new Double(P200_temp_stairs60));
     propertyChangeListeners.firePropertyChange("P200_temp_stairs75", new Double(old_P200_temp_stairs75), new Double(P200_temp_stairs75));
     propertyChangeListeners.firePropertyChange("P200_temp_stairs90", new Double(old_P200_temp_stairs90), new Double(P200_temp_stairs90));
     propertyChangeListeners.firePropertyChange("P200_temp_gantryUR", new Double(old_P200_temp_gantryUR), new Double(P200_temp_gantryUR));
     propertyChangeListeners.firePropertyChange("P200_temp_gantryLR", new Double(old_P200_temp_gantryLR), new Double(P200_temp_gantryLR));
     propertyChangeListeners.firePropertyChange("P200_temp_gantryUL", new Double(old_P200_temp_gantryUL), new Double(P200_temp_gantryUL));
     propertyChangeListeners.firePropertyChange("P200_temp_gantryLL", new Double(old_P200_temp_gantryLL), new Double(P200_temp_gantryLL));
     propertyChangeListeners.firePropertyChange("P200_temp_gantry_farR", new Double(old_P200_temp_gantry_farR), new Double(P200_temp_gantry_farR));
     propertyChangeListeners.firePropertyChange("P200_temp_gantry_farL", new Double(old_P200_temp_gantry_farL), new Double(P200_temp_gantry_farL));
     propertyChangeListeners.firePropertyChange("P200_temp_crane",       new Double(old_P200_temp_crane), new Double(P200_temp_crane));
     propertyChangeListeners.firePropertyChange("P200_temp_exhaust",     new Double(old_P200_temp_exhaust), new Double(P200_temp_exhaust));
     propertyChangeListeners.firePropertyChange("P200_temp_shutter",     new Double(old_P200_temp_shutter), new Double(P200_temp_shutter));
     propertyChangeListeners.firePropertyChange("P200_temp_fanR",        new Double(old_P200_temp_fanR), new Double(P200_temp_fanR));
     propertyChangeListeners.firePropertyChange("P200_temp_fanL",        new Double(old_P200_temp_fanL), new Double(P200_temp_fanL));
     propertyChangeListeners.firePropertyChange("P200_temp_catwalk", new Double(old_P200_temp_catwalk), new Double(P200_temp_catwalk));
     propertyChangeListeners.firePropertyChange("P200_temp_ArchR", new Double(old_P200_temp_ArchR), new Double(P200_temp_ArchR));
     propertyChangeListeners.firePropertyChange("P200_temp_ArchL", new Double(old_P200_temp_ArchL), new Double(P200_temp_ArchL));
     propertyChangeListeners.firePropertyChange("polling", new Boolean(old_polling), new Boolean(polling));


     propertyChangeListeners.firePropertyChange("P60_outside_air_temp", new Double(old_P60_outside_air_temp), new Double(P60_outside_air_temp));
     propertyChangeListeners.firePropertyChange("P60_dome_air_ttemp", new Double(old_P60_dome_air_ttemp), new Double(P60_dome_air_temp));
     propertyChangeListeners.firePropertyChange("P60_floor_temp", new Double(old_P60_floor_temp), new Double(P60_floor_temp));
     propertyChangeListeners.firePropertyChange("P60_inside_dewpoint", new Double(old_P60_inside_dewpoint), new Double(P60_inside_dewpoint));
     propertyChangeListeners.firePropertyChange("P60_outside_dewpoint", new Double(old_P60_outside_dewpoint), new Double(P60_outside_dewpoint));
     propertyChangeListeners.firePropertyChange("P60_tube_air_temperature", new Double(old_P60_tube_air_temperature), new Double(P60_tube_air_temperature));
     propertyChangeListeners.firePropertyChange("P60_tube_air_temperature_top", new Double(old_P60_tube_air_temperature_top), new Double(P60_tube_air_temperature_top));
     propertyChangeListeners.firePropertyChange("P60_tube_air_temperature_middle", new Double(old_P60_tube_air_temperature_middle), new Double(P60_tube_air_temperature_middle));
     propertyChangeListeners.firePropertyChange("P60_tube_air_temperature_bottom", new Double(old_P60_tube_air_temperature_bottom), new Double(P60_tube_air_temperature_bottom));
     propertyChangeListeners.firePropertyChange("P60_primary_mirror_temp", new Double(old_P60_primary_mirror_temp), new Double(P60_primary_mirror_temp));
     propertyChangeListeners.firePropertyChange("P60_primary_cell_temp", new Double(old_P60_primary_cell_temp), new Double(P60_primary_cell_temp));
     propertyChangeListeners.firePropertyChange("P60_secondary_cell_temp", new Double(old_P60_secondary_cell_temp), new Double(P60_secondary_cell_temp));
     propertyChangeListeners.firePropertyChange("P60_outside_relative_humidity", new Double(old_P60_outside_relative_humidity), new Double(P60_outside_relative_humidity));
     propertyChangeListeners.firePropertyChange("P60_inside_relative_humidity", new Double(old_P60_inside_relative_humidity), new Double(P60_inside_relative_humidity));
     propertyChangeListeners.firePropertyChange("P60_barometric_pressure", new Double(old_P60_barometric_pressure), new Double(P60_barometric_pressure));
     propertyChangeListeners.firePropertyChange("raining", new Boolean(old_P60_raining), new Boolean(P60_raining));
     propertyChangeListeners.firePropertyChange("P60_wind_direction", new Double(old_P60_wind_direction), new Double(P60_wind_direction));
     propertyChangeListeners.firePropertyChange("P60_windspeed_median", new Double(old_P60_windspeed_median), new Double(P60_windspeed_median));
     propertyChangeListeners.firePropertyChange("P60_windspeed_peak", new Double(old_P60_windspeed_peak), new Double(P60_windspeed_peak));
    
     propertyChangeListeners.firePropertyChange("P48_UTC", old_P48_UTC, P48_UTC);     
     propertyChangeListeners.firePropertyChange("P48_Windspeed_Avg_Threshold", new Float(old_P48_Windspeed_Avg_Threshold), new Float(P48_Windspeed_Avg_Threshold));
     propertyChangeListeners.firePropertyChange("P48_Gust_Speed_Threshold", new Float(old_P48_Gust_Speed_Threshold), new Float(P48_Gust_Speed_Threshold));
     propertyChangeListeners.firePropertyChange("P48_Alarm_Hold_Time", new Float(old_P48_Alarm_Hold_Time), new Float(P48_Alarm_Hold_Time));
     propertyChangeListeners.firePropertyChange("P48_Remaining_Hold_Time", new Float(old_P48_Remaining_Hold_Time), new Float(P48_Remaining_Hold_Time));
     propertyChangeListeners.firePropertyChange("P48_Outside_DewPt_Threshold", new Float(old_P48_Outside_DewPt_Threshold), new Float(P48_Outside_DewPt_Threshold));
     propertyChangeListeners.firePropertyChange("P48_Inside_DewPt_Threshold", new Float(old_P48_Inside_DewPt_Threshold), new Float(P48_Inside_DewPt_Threshold));
     propertyChangeListeners.firePropertyChange("P48_Wind_Dir_Current", new Float(old_P48_Wind_Dir_Current), new Float(P48_Wind_Dir_Current));
     propertyChangeListeners.firePropertyChange("P48_Windspeed_Current", new Float(old_P48_Windspeed_Current), new Float(P48_Windspeed_Current));
     propertyChangeListeners.firePropertyChange("P48_Windspeed_Average", new Float(old_P48_Windspeed_Average), new Float(P48_Windspeed_Average));
     propertyChangeListeners.firePropertyChange("P48_Outside_Air_Temp", new Float(old_P48_Outside_Air_Temp), new Float(P48_Outside_Air_Temp));
     propertyChangeListeners.firePropertyChange("P48_Outside_Rel_Hum", new Float(old_P48_Outside_Rel_Hum), new Float(P48_Outside_Rel_Hum));
     propertyChangeListeners.firePropertyChange("P48_Outside_DewPt", new Float(old_P48_Outside_DewPt), new Float(P48_Outside_DewPt));
     propertyChangeListeners.firePropertyChange("P48_Inside_Air_Temp", new Float(old_P48_Inside_Air_Temp), new Float(P48_Inside_Air_Temp));
     propertyChangeListeners.firePropertyChange("P48_Inside_Rel_Hum", new Float(old_P48_Inside_Rel_Hum), new Float(P48_Inside_Rel_Hum));
     propertyChangeListeners.firePropertyChange("P48_Inside_DewPt", new Float(old_P48_Inside_DewPt), new Float(P48_Inside_DewPt));
     propertyChangeListeners.firePropertyChange("P48_Wetness", old_P48_Wetness, P48_Wetness);
     propertyChangeListeners.firePropertyChange("P48_Weather_Status", old_P48_Weather_Status, P48_Weather_Status);
 

     propertyChangeListeners.firePropertyChange("P48_UTC", old_P48_UTC, P48_UTC);     
     propertyChangeListeners.firePropertyChange("", new Float(old_P48_Windspeed_Avg_Threshold), new Float(P48_Windspeed_Avg_Threshold));
     propertyChangeListeners.firePropertyChange("", new Float(old_P48_Gust_Speed_Threshold), new Float(P48_Gust_Speed_Threshold));
     propertyChangeListeners.firePropertyChange("", new Float(old_P48_Alarm_Hold_Time), new Float(P48_Alarm_Hold_Time));
     propertyChangeListeners.firePropertyChange("", new Float(old_P48_Remaining_Hold_Time), new Float(P48_Remaining_Hold_Time));
     propertyChangeListeners.firePropertyChange("", new Float(old_P48_Outside_DewPt_Threshold), new Float(P48_Outside_DewPt_Threshold));
     propertyChangeListeners.firePropertyChange("", new Float(old_P48_Inside_DewPt_Threshold), new Float(P48_Inside_DewPt_Threshold));
     propertyChangeListeners.firePropertyChange("", new Float(old_P48_Wind_Dir_Current), new Float(P48_Wind_Dir_Current));
     propertyChangeListeners.firePropertyChange("", new Float(old_P48_Windspeed_Current), new Float(P48_Windspeed_Current));
     propertyChangeListeners.firePropertyChange("", new Float(old_P48_Windspeed_Average), new Float(P48_Windspeed_Average));
     propertyChangeListeners.firePropertyChange("", new Float(old_P48_Outside_Air_Temp), new Float(P48_Outside_Air_Temp));
     propertyChangeListeners.firePropertyChange("", new Float(old_P48_Outside_Rel_Hum), new Float(P48_Outside_Rel_Hum));
     propertyChangeListeners.firePropertyChange("", new Float(old_P48_Outside_DewPt), new Float(P48_Outside_DewPt));
     propertyChangeListeners.firePropertyChange("", new Float(old_P48_Inside_Air_Temp), new Float(P48_Inside_Air_Temp));
     propertyChangeListeners.firePropertyChange("", new Float(old_P48_Inside_Rel_Hum), new Float(P48_Inside_Rel_Hum));
     propertyChangeListeners.firePropertyChange("", new Float(old_P48_Inside_DewPt), new Float(P48_Inside_DewPt));
     propertyChangeListeners.firePropertyChange("", old_P48_Wetness, P48_Wetness);
     propertyChangeListeners.firePropertyChange("", old_P48_Weather_Status, P48_Weather_Status);
*/
