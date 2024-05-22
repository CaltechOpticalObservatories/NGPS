/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package edu.caltech.palomar.telescopes.telemetry;
import com.google.gson.Gson;
import static edu.caltech.palomar.telescopes.telemetry.P200WeatherObject.ERROR;
import edu.caltech.palomar.telescopes.telemetry.SFTPRetrieveWeather;
import edu.caltech.palomar.util.general.CommandLogModel;
import java.beans.PropertyChangeListener;
import java.beans.PropertyChangeSupport;
import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.FileReader;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.Timestamp;
import java.time.ZoneId;
import java.time.ZonedDateTime;
import java.util.ArrayList;
import java.util.Properties;
import org.apache.log4j.DailyRollingFileAppender;
import org.apache.log4j.Layout;
import org.apache.log4j.Logger;
import org.apache.log4j.PatternLayout;
import org.apache.log4j.BasicConfigurator;
import edu.caltech.palomar.telescopes.telemetry.P60weather;
import nom.tam.fits.Header;
import nom.tam.fits.HeaderCard;
import nom.tam.fits.HeaderCardException;
import nom.tam.util.Cursor;
       
/**
 *
 * @author developer
 */
public class P60WeatherObject {
    transient private PropertyChangeSupport propertyChangeListeners = new PropertyChangeSupport(this);
    public SFTPRetrieveWeather   mySFTPRetrieveWeather = new SFTPRetrieveWeather();
    private java.lang.String     TERMINATOR      = new java.lang.String("\n");
    public P60weather            current               = new P60weather();
    private java.lang.String     SEP             = System.getProperty("file.separator");
    public java.lang.String      USERDIR         = System.getProperty("user.dir");
    private java.lang.String     CONFIG          = new java.lang.String("config");
/*    public float                 OUTSIDE_TEMP;
    public float                 INSIDE_TEMP;
    public float                 FLOOR_TEMP;
    public float                 OUTSIDE_DEW_POINT;
    public float                 INSIDE_DEW_POINT;
    public float                 TUBE_AIR_TEMP;
    public float                 TUBE_TOP_TEMP;
    public float                 TUBE_MID_TEMP;
    public float                 TUBE_BOTTOM_TEMP;
    public float                 PRIMARY_MIRROR_TEMP;
    public float                 PRIMARY_CELL_TEMP;
    public float                 SECONDARY_CELL_TEMP;
    public int                   OUTSIDE_REL_HUMIDITY;
    public int                   INSIDE_REL_HUMIDITY;
    public float                 BAROMETRIC_PRESSURE;
    public boolean               RAINING;
    public int                   WIND_POS_ANGLE;
    public float                 WIND_GUST_MPH;
    public float                 WIND_AVERAGE_MPH;
*/    
    public java.lang.String      WEATHER_FILE_NAME = "WeatherData";
    public ArrayList             weather_arraylist = new ArrayList();
    public double P60_outside_air_temp;
    public double P60_dome_air_temp;
    public double P60_floor_temp;
    public double P60_outside_dewpoint;
    public double P60_inside_dewpoint;
    public double P60_tube_air_temperature;
    public double P60_tube_air_temperature_top;
    public double P60_tube_air_temperature_middle;
    public double P60_tube_air_temperature_bottom;
    public double P60_primary_mirror_temp;
    public double P60_primary_cell_temp;
    public double P60_secondary_cell_temp;
    public double P60_outside_relative_humidity;
    public double P60_inside_relative_humidity;
    public double P60_barometric_pressure;
    public boolean P60_raining;
    public double P60_wind_direction;    
    public double P60_windspeed_median;
    public double P60_windspeed_peak;
    public int    P60_YEAR;
    public int    P60_MONTH;
    public int    P60_DAY;
    public java.lang.String P40_TIME;
    static Logger                    weatherLogger    = Logger.getLogger(TelemetryController.class);
    public DailyRollingFileAppender  fileAppender       = new DailyRollingFileAppender();
    private Layout                   layout             = new PatternLayout("%-5p [%t]: %m%n");
    public static int                DEBUG              = 0;
    public static int                INFO               = 1;
    public static int                WARN               = 2;
    public static int                ERROR              = 3;
    public static int                FATAL              = 4;    
    public CommandLogModel           myCommandLogModel  = new CommandLogModel();
    public java.lang.String          LOG_DIRECTORY      = new java.lang.String();
    public Connection                conn               = null;
    public int                       WEATHER_POLLING;
    public  java.lang.String         DBMS_CONNECTION_PROPERTIES = "telemetry_dbms.ini";
    public double                    MISSING_VALUE = -99.9;
    public PreparedStatement         INSERT_PREP_STATEMENT;
    public ZonedDateTime             now;
    public ZonedDateTime             now_utc;
    public Timestamp                 current_timestamp;
    public Timestamp                 UTC_TIMESTAMP;
    public Timestamp                 P60_TIMESTAMP;
    public String                    UTC_DATE_TIME;
    public String                    LOCAL_DATE_TIME;
    private boolean                  polling;
    private long                     LAST_TIMESTAMP     = 0;
    private long                     CURRENT_TIMESTAMP;
    public Header                    CURRENT_FITS_HEADER;
    public java.lang.String          CURRENT_FITS_HEADER_STRING = new java.lang.String();
    public java.lang.String          CURRENT_GSON_TELEMETRY     = new java.lang.String();
    private java.lang.String         p60_utc_timestamp;
    
// CREATE TABLE SQL
/*    CREATE TABLE `telemetry`.`p60weather` (
  `ID` BIGINT NOT NULL AUTO_INCREMENT,
  `TIMESTAMP_UTC` TIMESTAMP NULL,
  `UTCDATETIME` VARCHAR(60) NULL,
  `LOCALDATETIME` VARCHAR(60) NULL,
  `P60TIMESTAMP` TIMESTAMP NULL,
  `OUTSIDE_TEMPERATURE` FLOAT NULL,
  `INSIDE_TEMPERATURE` FLOAT NULL,
  `OUTSIDE_DEWPOINT` FLOAT NULL,
  `INSIDE_DEWPOINT` FLOAT NULL,
  `TUBE_AIR_TEMPERATURE` FLOAT NULL,
  `TUBE_TOP_TEMPERATUE` FLOAT NULL,
  `TUBE_MIDDLE_TEMPERATURE` FLOAT NULL,
  `TUBE_BOTTOM_TEMPERATURE` FLOAT NULL,
  `PRIMARY_MIRROR_TEMPERATURE` FLOAT NULL,
  `PRIMARY_CELL_TEMPERATURE` FLOAT NULL,
  `SECONDARY_CELL_TEMPERATURE` FLOAT NULL,
  `OUTSIDE_RELATIVE_HUMIDITY` FLOAT NULL,
  `INSIDE_RELATIVE_HUMIDITY` FLOAT NULL,
  `BAROMETRIC_PRESSURE` FLOAT NULL,
  `RAINING` VARCHAR(45) NULL,
  `WIND_DIRECTION` FLOAT NULL,
  `WIND_GUSTS` FLOAT NULL,
  `WIND_MEDIAN` FLOAT NULL,
  PRIMARY KEY (`ID`),
  INDEX `TIMESTAMP_UTC_INDEX` (`TIMESTAMP_UTC` ASC),
  INDEX `TIMESTAMP_P60` USING BTREE (`P60TIMESTAMP` ASC));
*/
/*=============================================================================================
/     P60WeatherObject()
/=============================================================================================*/
    public P60WeatherObject(){
        initializeDBMS();
        boolean test = false;
        if(test){
            execute_process();
        }
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
     timestamp_measurements();
     readWeatherFile();
     if(CURRENT_TIMESTAMP == LAST_TIMESTAMP){
         logMessage(INFO,"Skipping update due to the same P60 timestamp!");
//         LAST_TIMESTAMP = CURRENT_TIMESTAMP;           
     }
     if(CURRENT_TIMESTAMP != LAST_TIMESTAMP){
         logMessage(INFO,"Updating DBMS with new P60 weather");
         executeInsertStatement();
         constructHeader();
         LAST_TIMESTAMP = CURRENT_TIMESTAMP; 
     }  

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
/         initializeLogging()
/=============================================================================================*/
 private void initializeLogging(){
     BasicConfigurator.configure();
     fileAppender.setFile(LOG_DIRECTORY + SEP +"P60Weather.log");        
     fileAppender.setDatePattern("'.'yyyy-MM-dd");
     fileAppender.setAppend(true);
     fileAppender.setLayout(layout);
     fileAppender.activateOptions();
     weatherLogger.addAppender(fileAppender);
 }      
 /*=============================================================================================
/        logMessage(int code,java.lang.String message)
/=============================================================================================*/
public void logMessage(int code,java.lang.String message){

     switch(code){
         case 0:  weatherLogger.debug(message);  break;
         case 1:  weatherLogger.info(message);   break;
         case 2:  weatherLogger.warn(message);   break;
         case 3:  weatherLogger.error(message);  break;
         case 4:  weatherLogger.fatal(message);  break;
     }
     if((code ==WARN)|(code == ERROR)|(code ==FATAL)){
         myCommandLogModel.insertMessage(CommandLogModel.ERROR, message);
     }
     System.out.println(message);
 }   
/*================================================================================================
/       retrieveWeatherFile()
/=================================================================================================*/
  public void readWeatherFile(){
     java.lang.String last_line = new java.lang.String(); 
      try{
         weather_arraylist.clear();
//         retrieveWeatherFile(); 
         BufferedReader br = new BufferedReader(new FileReader("/home/developer/p60weather"+SEP+WEATHER_FILE_NAME));
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
         parseFile();
      }catch(Exception e){
          System.out.println(e.toString());
      }
  } 
/*================================================================================================
/      parseFile()
/=================================================================================================*/
  public void parseFile(){
    int    YEAR;
    int    MONTH;
    int    DAY;
    int    HOURS;
    int    MINUTES;
    float  SECONDS;
    java.lang.String TIME;
    double outside_air_temp            = MISSING_VALUE;
    double dome_air_temp               = MISSING_VALUE;
    double floor_temp                  = MISSING_VALUE;
    double outside_dewpoint            = MISSING_VALUE;
    double inside_dewpoint             = MISSING_VALUE;
    double tube_air_temperature        = MISSING_VALUE;
    double tube_air_temperature_top    = MISSING_VALUE;
    double tube_air_temperature_middle = MISSING_VALUE;
    double tube_air_temperature_bottom = MISSING_VALUE;
    double primary_mirror_temp         = MISSING_VALUE;
    double primary_cell_temp           = MISSING_VALUE;
    double secondary_cell_temp         = MISSING_VALUE;
    double outside_relative_humidity   = MISSING_VALUE;
    double inside_relative_humidity    = MISSING_VALUE;
    double barometric_pressure         = MISSING_VALUE;
    boolean raining                    = false;
    double wind_direction              = MISSING_VALUE;    
    double windspeed_median            = MISSING_VALUE;
    double windspeed_peak              = MISSING_VALUE;   
      try{
          YEAR             = java.lang.Integer.parseInt((java.lang.String)weather_arraylist.get(0));
          MONTH            = java.lang.Integer.parseInt((java.lang.String)weather_arraylist.get(1));
          DAY              = java.lang.Integer.parseInt((java.lang.String)weather_arraylist.get(2));
          TIME             = (java.lang.String)weather_arraylist.get(3);
          java.util.StringTokenizer st = new java.util.StringTokenizer(TIME,":");
          HOURS            = java.lang.Integer.parseInt(st.nextToken());
          MINUTES          = java.lang.Integer.parseInt(st.nextToken());
          SECONDS          = java.lang.Float.parseFloat(st.nextToken());
          P60_TIMESTAMP    = new Timestamp(YEAR-1900,MONTH-1,DAY,HOURS,MINUTES,(int)SECONDS,0);  
          CURRENT_TIMESTAMP   = P60_TIMESTAMP.getTime();
          current.P60_TIMESTAMP = P60_TIMESTAMP;
          
          outside_air_temp = java.lang.Double.parseDouble((java.lang.String)weather_arraylist.get(4));
          dome_air_temp = java.lang.Double.parseDouble((java.lang.String)weather_arraylist.get(5));
          floor_temp = java.lang.Double.parseDouble((java.lang.String)weather_arraylist.get(6));
          outside_dewpoint = java.lang.Double.parseDouble((java.lang.String)weather_arraylist.get(7));
          inside_dewpoint = java.lang.Double.parseDouble((java.lang.String)weather_arraylist.get(8));
          tube_air_temperature = java.lang.Double.parseDouble((java.lang.String)weather_arraylist.get(9));
          tube_air_temperature_top = java.lang.Double.parseDouble((java.lang.String)weather_arraylist.get(10));
          tube_air_temperature_middle = java.lang.Double.parseDouble((java.lang.String)weather_arraylist.get(11));
          tube_air_temperature_bottom = java.lang.Double.parseDouble((java.lang.String)weather_arraylist.get(12));
          primary_mirror_temp = java.lang.Double.parseDouble((java.lang.String)weather_arraylist.get(13));
          primary_cell_temp = java.lang.Double.parseDouble((java.lang.String)weather_arraylist.get(14));
          secondary_cell_temp = java.lang.Double.parseDouble((java.lang.String)weather_arraylist.get(15));
          outside_relative_humidity = java.lang.Double.parseDouble((java.lang.String)weather_arraylist.get(16));
          inside_relative_humidity = java.lang.Double.parseDouble((java.lang.String)weather_arraylist.get(17));
          barometric_pressure = java.lang.Double.parseDouble((java.lang.String)weather_arraylist.get(18));          
          java.lang.String raining_string = (java.lang.String)weather_arraylist.get(19);
          if(raining_string.matches("YES")){
              raining = true;
          }else if(raining_string.matches("NO")){
              raining = false;
          }          
          wind_direction   = java.lang.Double.parseDouble((java.lang.String)weather_arraylist.get(20));
          windspeed_median = java.lang.Double.parseDouble((java.lang.String)weather_arraylist.get(21));
          windspeed_peak   = java.lang.Double.parseDouble((java.lang.String)weather_arraylist.get(22));
      }catch(Exception e){
          logMessage(ERROR,e.toString());
      }
      setP60_outside_air_temp(outside_air_temp);
      setP60_dome_air_ttemp(dome_air_temp);
      setP60_floor_temp(floor_temp);
      setP60_inside_dewpoint(inside_dewpoint);
      setP60_outside_dewpoint(outside_dewpoint);
      setP60_tube_air_temperature(tube_air_temperature);
      setP60_tube_air_temperature_top(tube_air_temperature_top);
      setP60_tube_air_temperature_middle(tube_air_temperature_middle);
      setP60_tube_air_temperature_bottom(tube_air_temperature_bottom);
      setP60_primary_mirror_temp(primary_mirror_temp);
      setP60_primary_cell_temp(primary_cell_temp);
      setP60_secondary_cell_temp(secondary_cell_temp);
      setP60_outside_relative_humidity(outside_relative_humidity);
      setP60_inside_relative_humidity(inside_relative_humidity);
      setP60_barometric_pressure(barometric_pressure);
      setP60_raining(raining);
      setP60_wind_direction(wind_direction);
      setP60_windspeed_median(windspeed_median);
      setP60_windspeed_peak(windspeed_peak);  
  }
/*================================================================================================
/       constructPreparedStatement()
/=================================================================================================*/
public void constructPreparedStatement(){
    String query = "INSERT INTO p60weather ("
    + " TIMESTAMP_UTC,"
    + " UTCDATETIME,"
    + " LOCALDATETIME,"
    + " TIMESTAMP_P60,"        
    + " OUTSIDE_TEMPERATURE,"        
    + " INSIDE_TEMPERATURE,"
    + " FLOOR_TEMPERATURE,"
    + " OUTSIDE_DEWPOINT," 
    + " INSIDE_DEWPOINT,"
    + " TUBE_AIR_TEMPERATURE,"
    + " TUBE_TOP_TEMPERATUE,"
    + " TUBE_MIDDLE_TEMPERATURE,"
    + " TUBE_BOTTOM_TEMPERATURE,"
    + " PRIMARY_MIRROR_TEMPERATURE,"
    + " PRIMARY_CELL_TEMPERATURE,"
    + " SECONDARY_CELL_TEMPERATURE,"
    + " OUTSIDE_RELATIVE_HUMIDITY,"
    + " INSIDE_RELATIVE_HUMIDITY,"
    + " BAROMETRIC_PRESSURE,"
    + " RAINING,"
    + " WIND_DIRECTION,"
    + " WIND_GUSTS,"
    + " WIND_MEDIAN"
    + " ) VALUES ("
    +  " ?, ?, ?, ?, ?, ?, ?, ?, ?, ?,"
    +  " ?, ?, ?, ?, ?, ?, ?, ?, ?, ?,"
    +  " ?, ?, ? "
    +  ")";
    try{
       INSERT_PREP_STATEMENT = conn.prepareStatement(query);
    }catch(Exception e){
        logMessage(ERROR,"Creating prepared statement. "+e.toString());
    }
}    
/*================================================================================================
/       timestamp_measurements()
/=================================================================================================*/
public void timestamp_measurements(){
   now     = ZonedDateTime.now();
   now_utc = now.withZoneSameInstant(ZoneId.of("UTC"));
   current_timestamp = new Timestamp(now_utc.getYear()-1900,now_utc.getMonthValue()-1,now_utc.getDayOfMonth(),
                                                    now_utc.getHour(),now_utc.getMinute(),now_utc.getSecond(),0);  
   this.UTC_TIMESTAMP      = current_timestamp;
   this.LOCAL_DATE_TIME    = now.toString();
   this.UTC_DATE_TIME      = now_utc.toString();
   current.UTC_TIMESTAMP   = current_timestamp;
   current.UTC_DATE_TIME   = UTC_DATE_TIME;
   current.LOCAL_DATE_TIME = LOCAL_DATE_TIME;
   setP60_utc_timestamp(UTC_DATE_TIME);
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
        INSERT_PREP_STATEMENT.setTimestamp(4,this.P60_TIMESTAMP);
        INSERT_PREP_STATEMENT.setFloat(5,(float)this.P60_outside_air_temp);
        INSERT_PREP_STATEMENT.setFloat(6,(float)this.P60_dome_air_temp);
        INSERT_PREP_STATEMENT.setFloat(7,(float)this.P60_floor_temp); 
        INSERT_PREP_STATEMENT.setFloat(8,(float)this.P60_outside_dewpoint);
        INSERT_PREP_STATEMENT.setFloat(9,(float)this.P60_inside_dewpoint);
        INSERT_PREP_STATEMENT.setFloat(10,(float)this.P60_tube_air_temperature);
        INSERT_PREP_STATEMENT.setFloat(11,(float)this.P60_tube_air_temperature_top);
        INSERT_PREP_STATEMENT.setFloat(12,(float)this.P60_tube_air_temperature_middle);
        INSERT_PREP_STATEMENT.setFloat(13,(float)this.P60_tube_air_temperature_bottom);
        INSERT_PREP_STATEMENT.setFloat(14,(float)this.P60_primary_mirror_temp);
        INSERT_PREP_STATEMENT.setFloat(15,(float)this.P60_primary_cell_temp);
        INSERT_PREP_STATEMENT.setFloat(16,(float)this.P60_secondary_cell_temp);
        INSERT_PREP_STATEMENT.setFloat(17,(float)this.P60_outside_relative_humidity);
        INSERT_PREP_STATEMENT.setFloat(18,(float)this.P60_inside_relative_humidity);
        INSERT_PREP_STATEMENT.setFloat(19,(float)this.P60_barometric_pressure);
        java.lang.String raining_string = new java.lang.String();
        if(P60_raining){
            raining_string = "YES";
        }else if(!P60_raining){
            raining_string = "NO";
        }
        INSERT_PREP_STATEMENT.setString(20,raining_string);
        INSERT_PREP_STATEMENT.setFloat(21,(float)this.P60_wind_direction);
        INSERT_PREP_STATEMENT.setFloat(22,(float)this.P60_windspeed_peak);
        INSERT_PREP_STATEMENT.setFloat(23,(float)this.P60_windspeed_median);        
        System.out.println(INSERT_PREP_STATEMENT.toString());
        // now that we have all the parameters set execute the update on the DBMS
        INSERT_PREP_STATEMENT.executeUpdate();
    }catch(Exception e){
       logMessage(ERROR,"Error in method executeInsertStatement "+e.toString());
    }  
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
public java.lang.String weatherToGson(P60weather current){
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
       currentHeader.addLine(new HeaderCard("P6UTCTS",  UTC_TIMESTAMP.toString(),"P60 UTC timestamp"));
       currentHeader.addLine(new HeaderCard("P6UTCST",  UTC_DATE_TIME,"P60 UTC date-time string"));
       currentHeader.addLine(new HeaderCard("P6LDT",    LOCAL_DATE_TIME,"P60 Local date and time"));
       currentHeader.addLine(new HeaderCard("P6TIMSTP", this.P60_TIMESTAMP.toString(),"P200 Time stamp for the weather record"));
       currentHeader.addLine(new HeaderCard("P6OUTEMP", this.getP60_outside_air_temp(),"P60 outside temperature"));
       currentHeader.addLine(new HeaderCard("P6INTEMP", this.getP60_dome_air_ttemp(),"P60 inside dome air temperature"));
       currentHeader.addLine(new HeaderCard("P6FLOORT", this.getP60_floor_temp(),"P60 floor temperature"));
       currentHeader.addLine(new HeaderCard("P6OUTDEW", this.getP60_outside_dewpoint(),"P60 outside dewpoint"));
       currentHeader.addLine(new HeaderCard("P6INDEW",  this.getP60_inside_dewpoint(),"P60 inside dewpoint"));
       currentHeader.addLine(new HeaderCard("P6TBAIRT", this.getP60_tube_air_temperature(),"P60 tube air temperature"));
       currentHeader.addLine(new HeaderCard("P6TUTOPT", this.getP60_tube_air_temperature_top(),"P60 tube top temperature"));
       currentHeader.addLine(new HeaderCard("P6TUBMDT", this.getP60_tube_air_temperature_middle(),"P60 tube middle temperature"));
       currentHeader.addLine(new HeaderCard("P6TUBBT",  this.getP60_tube_air_temperature_bottom(),"P60 tube bottom temperature"));
       currentHeader.addLine(new HeaderCard("P6PRMTMP", this.getP60_primary_mirror_temp(),"P60 primary mirror temperature"));
       currentHeader.addLine(new HeaderCard("P6CELLT",  this.getP60_primary_cell_temp(),"P60 primary cell temperature"));
       currentHeader.addLine(new HeaderCard("P6CELT",   this.getP60_secondary_cell_temp(),"P60 secondary cell temperature"));
       currentHeader.addLine(new HeaderCard("P6OTRHUM", this.getP60_outside_relative_humidity(),"P60 outside relative humidity"));
       currentHeader.addLine(new HeaderCard("P6INRHUM", this.getP60_inside_relative_humidity(),"P60 inside relative humidity"));
       currentHeader.addLine(new HeaderCard("P6BAROMP", this.getP60_barometric_pressure(),"P60 barometric pressure"));
       currentHeader.addLine(new HeaderCard("P6RAIN",   this.isP60_raining(),"P60 raining?"));
       currentHeader.addLine(new HeaderCard("P6WINDD",  this.getP60_wind_direction(),"P60 wind direction"));
       currentHeader.addLine(new HeaderCard("P6WINGUY",this.getP60_windspeed_peak(),"P60 wind gust/peak"));
       currentHeader.addLine(new HeaderCard("P6WINDMD", this.getP60_windspeed_median(),"P60 wind median"));
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
/     setP200_utc_timestamp(java.lang.String new_p200_utc_timestamp) 
/=================================================================================================*/
  public void setP60_utc_timestamp(java.lang.String new_p60_utc_timestamp) {
     java.lang.String  old_p60_utc_timestamp = this.p60_utc_timestamp;
     this.p60_utc_timestamp = new_p60_utc_timestamp;
     current.p60_utc_timestamp =p60_utc_timestamp;
     propertyChangeListeners.firePropertyChange("p60_utc_timestamp", old_p60_utc_timestamp, p60_utc_timestamp);
  }
  public java.lang.String getP60_utc_timestamp() {
     return p60_utc_timestamp;
  } 
/*================================================================================================
/       set
/=================================================================================================*/
  public void setP60_outside_air_temp(double new_P60_outside_air_temp) {
     double  old_P60_outside_air_temp = this.P60_outside_air_temp;
     this.P60_outside_air_temp = new_P60_outside_air_temp;
     current.P60_outside_air_temp = P60_outside_air_temp;
     propertyChangeListeners.firePropertyChange("P60_outside_air_temp", Double.valueOf(old_P60_outside_air_temp), Double.valueOf(P60_outside_air_temp));
  }
  public double getP60_outside_air_temp() {
     return P60_outside_air_temp;
  } 
/*================================================================================================
/       setP200_dome_air_ttemp
/=================================================================================================*/
  public void setP60_dome_air_ttemp(double new_P60_dome_air_ttemp) {
     double  old_P60_dome_air_ttemp = this.P60_dome_air_temp;
     this.P60_dome_air_temp = new_P60_dome_air_ttemp;
     current.P60_dome_air_temp = P60_dome_air_temp;
     propertyChangeListeners.firePropertyChange("P60_dome_air_ttemp", Double.valueOf(old_P60_dome_air_ttemp),Double.valueOf(P60_dome_air_temp));
  }
  public double getP60_dome_air_ttemp() {
     return P60_dome_air_temp;
  } 
/*================================================================================================
/       setP200_floor_temp
/=================================================================================================*/
  public void setP60_floor_temp(double new_P60_floor_temp) {
     double  old_P60_floor_temp = this.P60_floor_temp;
     this.P60_floor_temp = new_P60_floor_temp;
     current.P60_floor_temp = P60_floor_temp;
     propertyChangeListeners.firePropertyChange("P60_floor_temp", Double.valueOf(old_P60_floor_temp), Double.valueOf(P60_floor_temp));
  }
  public double getP60_floor_temp() {
     return P60_floor_temp;
  } 
/*================================================================================================
/       settP200_dewpoint
/=================================================================================================*/
  public void setP60_inside_dewpoint(double new_P60_inside_dewpoint) {
     double  old_P60_inside_dewpoint = this.P60_inside_dewpoint;
     this.P60_inside_dewpoint = new_P60_inside_dewpoint;
     current.P60_inside_dewpoint = P60_inside_dewpoint;
     propertyChangeListeners.firePropertyChange("P60_inside_dewpoint", Double.valueOf(old_P60_inside_dewpoint),Double.valueOf(P60_inside_dewpoint));
  }
  public double getP60_inside_dewpoint() {
     return P60_inside_dewpoint;
  } 
 /*================================================================================================
/       settP200_dewpoint
/=================================================================================================*/
  public void setP60_outside_dewpoint(double new_P60_outside_dewpoint) {
     double  old_P60_outside_dewpoint = this.P60_outside_dewpoint;
     this.P60_outside_dewpoint = new_P60_outside_dewpoint;
     current.P60_outside_dewpoint = P60_outside_dewpoint;
     propertyChangeListeners.firePropertyChange("P60_outside_dewpoint", Double.valueOf(old_P60_outside_dewpoint),Double.valueOf(P60_outside_dewpoint));
  }
  public double getP60_outside_dewpoint() {
     return P60_outside_dewpoint;
  } 
 /*================================================================================================
/       settP200_dewpoint
/=================================================================================================*/
  public void setP60_tube_air_temperature(double new_P60_tube_air_temperature) {
     double  old_P60_tube_air_temperature = this.P60_tube_air_temperature;
     this.P60_tube_air_temperature = new_P60_tube_air_temperature;
     current.P60_tube_air_temperature = P60_tube_air_temperature;
     propertyChangeListeners.firePropertyChange("P60_tube_air_temperature",Double.valueOf(old_P60_tube_air_temperature),Double.valueOf(P60_tube_air_temperature));
  }
  public double getP60_tube_air_temperature() {
     return P60_tube_air_temperature;
  }   
 /*================================================================================================
/       settP200_dewpoint
/=================================================================================================*/
  public void setP60_tube_air_temperature_top(double new_P60_tube_air_temperature_top) {
     double  old_P60_tube_air_temperature_top = this.P60_tube_air_temperature_top;
     this.P60_tube_air_temperature_top = new_P60_tube_air_temperature_top;
     current.P60_tube_air_temperature_top = P60_tube_air_temperature_top;
     propertyChangeListeners.firePropertyChange("P60_tube_air_temperature_top", Double.valueOf(old_P60_tube_air_temperature_top),Double.valueOf(P60_tube_air_temperature_top));
  }
  public double getP60_tube_air_temperature_top() {
     return P60_tube_air_temperature_top;
  }     
 /*================================================================================================
/       settP200_dewpoint
/=================================================================================================*/
  public void setP60_tube_air_temperature_middle(double new_P60_tube_air_temperature_middle) {
     double  old_P60_tube_air_temperature_middle = this.P60_tube_air_temperature_middle;
     this.P60_tube_air_temperature_middle = new_P60_tube_air_temperature_middle;
     current.P60_tube_air_temperature_middle = P60_tube_air_temperature_middle;
     propertyChangeListeners.firePropertyChange("P60_tube_air_temperature_middle", Double.valueOf(old_P60_tube_air_temperature_middle), Double.valueOf(P60_tube_air_temperature_middle));
  }
  public double getP60_tube_air_temperature_middle() {
     return P60_tube_air_temperature_middle;
  }   
 /*================================================================================================
/       settP200_dewpoint
/=================================================================================================*/
  public void setP60_tube_air_temperature_bottom(double new_P60_tube_air_temperature_bottom) {
     double  old_P60_tube_air_temperature_bottom = this.P60_tube_air_temperature_bottom;
     this.P60_tube_air_temperature_bottom = new_P60_tube_air_temperature_bottom;
     current.P60_tube_air_temperature_bottom = P60_tube_air_temperature_bottom;
     propertyChangeListeners.firePropertyChange("P60_tube_air_temperature_bottom", Double.valueOf(old_P60_tube_air_temperature_bottom), Double.valueOf(P60_tube_air_temperature_bottom));
  }
  public double getP60_tube_air_temperature_bottom() {
     return P60_tube_air_temperature_bottom;
  }   
/*================================================================================================
/       setP200_primary_mirror_temp
/=================================================================================================*/
  public void setP60_primary_mirror_temp(double new_P60_primary_mirror_temp) {
     double  old_P60_primary_mirror_temp = this.P60_primary_mirror_temp;
     this.P60_primary_mirror_temp = new_P60_primary_mirror_temp;
     current.P60_primary_mirror_temp = P60_primary_mirror_temp;
     propertyChangeListeners.firePropertyChange("P60_primary_mirror_temp", Double.valueOf(old_P60_primary_mirror_temp), Double.valueOf(P60_primary_mirror_temp));
  }
  public double getP60_primary_mirror_temp() {
     return P60_primary_mirror_temp;
  } 
/*================================================================================================
/       setP200_primary_mirror_temp
/=================================================================================================*/
  public void setP60_primary_cell_temp(double new_P60_primary_cell_temp) {
     double  old_P60_primary_cell_temp = this.P60_primary_cell_temp;
     this.P60_primary_cell_temp = new_P60_primary_cell_temp;
     current.P60_primary_cell_temp = P60_primary_cell_temp;
     propertyChangeListeners.firePropertyChange("P60_primary_cell_temp", Double.valueOf(old_P60_primary_cell_temp), Double.valueOf(P60_primary_cell_temp));
  }
  public double getP60_primary_cell_temp() {
     return P60_primary_cell_temp;
  }    
/*================================================================================================
/       setP200_primary_mirror_temp
/=================================================================================================*/
  public void setP60_secondary_cell_temp(double new_P60_secondary_cell_temp) {
     double  old_P60_secondary_cell_temp = this.P60_secondary_cell_temp;
     this.P60_secondary_cell_temp = new_P60_secondary_cell_temp;
     current.P60_secondary_cell_temp = P60_secondary_cell_temp;
     propertyChangeListeners.firePropertyChange("P60_secondary_cell_temp", Double.valueOf(old_P60_secondary_cell_temp), Double.valueOf(P60_secondary_cell_temp));
  }
  public double getP60_secondary_cell_temp() {
     return P60_secondary_cell_temp;
  }    
/*================================================================================================
/       setP60_outside_relative_humidity(double new_P60_outside_relative_humidity)
/=================================================================================================*/
  public void setP60_outside_relative_humidity(double new_P60_outside_relative_humidity) {
     double  old_P60_outside_relative_humidity = this.P60_outside_relative_humidity;
     this.P60_outside_relative_humidity = new_P60_outside_relative_humidity;
     current.P60_outside_relative_humidity = P60_outside_relative_humidity;
     propertyChangeListeners.firePropertyChange("P60_outside_relative_humidity", Double.valueOf(old_P60_outside_relative_humidity), Double.valueOf(P60_outside_relative_humidity));
  }
  public double getP60_outside_relative_humidity() {
     return P60_outside_relative_humidity;
  }   
/*================================================================================================
/       setP60_outside_relative_humidity(double new_P60_outside_relative_humidity)
/=================================================================================================*/
  public void setP60_inside_relative_humidity(double new_P60_inside_relative_humidity) {
     double  old_P60_inside_relative_humidity = this.P60_inside_relative_humidity;
     this.P60_inside_relative_humidity = new_P60_inside_relative_humidity;
     current.P60_inside_relative_humidity = P60_inside_relative_humidity;
     propertyChangeListeners.firePropertyChange("P60_inside_relative_humidity", Double.valueOf(old_P60_inside_relative_humidity), Double.valueOf(P60_inside_relative_humidity));
  }
  public double getP60_inside_relative_humidity() {
     return P60_inside_relative_humidity;
  }     
/*================================================================================================
/       setP60_barometric_pressure(double new_P60_barometric_pressure)
/=================================================================================================*/
  public void setP60_barometric_pressure(double new_P60_barometric_pressure) {
     double  old_P60_barometric_pressure = this.P60_barometric_pressure;
     this.P60_barometric_pressure = new_P60_barometric_pressure;
     current.P60_barometric_pressure = P60_barometric_pressure;
     propertyChangeListeners.firePropertyChange("P60_barometric_pressure", Double.valueOf(old_P60_barometric_pressure), Double.valueOf(P60_barometric_pressure));
  }
  public double getP60_barometric_pressure() {
     return P60_barometric_pressure;
  }    
/*================================================================================================
/       setP200_wind_direction
/=================================================================================================*/
  public void setP60_raining(boolean new_P60_raining) {
     boolean  old_P60_raining = this.P60_raining;
     this.P60_raining = new_P60_raining;
     current.P60_raining = P60_raining;
     propertyChangeListeners.firePropertyChange("raining", Boolean.valueOf(old_P60_raining), Boolean.valueOf(P60_raining));
  }
  public boolean isP60_raining() {
     return P60_raining;
  }  
/*================================================================================================
/       setP60_wind_direction
/=================================================================================================*/
  public void setP60_wind_direction(double new_P60_wind_direction) {
     double  old_P60_wind_direction = this.P60_wind_direction;
     this.P60_wind_direction = new_P60_wind_direction;
     current.P60_wind_direction = P60_wind_direction;
     propertyChangeListeners.firePropertyChange("P60_wind_direction", Double.valueOf(old_P60_wind_direction), Double.valueOf(P60_wind_direction));
  }
  public double getP60_wind_direction() {
     return P60_wind_direction;
  }   
  /*================================================================================================
/       setP60_windspeed_median
/=================================================================================================*/
  public void setP60_windspeed_median(double new_P60_windspeed_median) {
     double  old_P60_windspeed_median = this.P60_windspeed_median;
     this.P60_windspeed_median = new_P60_windspeed_median;
     current.P60_windspeed_median = P60_windspeed_median;
     propertyChangeListeners.firePropertyChange("P60_windspeed_median", Double.valueOf(old_P60_windspeed_median), Double.valueOf(P60_windspeed_median));
  }
  public double getP60_windspeed_median() {
     return P60_windspeed_median;
  } 
  /*================================================================================================
/       setP60_windspeed_peak
/=================================================================================================*/
  public void setP60_windspeed_peak(double new_P60_windspeed_peak) {
     double  old_P60_windspeed_peak = this.P60_windspeed_peak;
     this.P60_windspeed_peak = new_P60_windspeed_peak;
     current.P60_windspeed_peak = P60_windspeed_peak;
     propertyChangeListeners.firePropertyChange("P60_windspeed_peak", Double.valueOf(old_P60_windspeed_peak), Double.valueOf(P60_windspeed_peak));
  }
  public double getP60_windspeed_peak() {
     return P60_windspeed_peak;
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
       new P60WeatherObject(); //<editor-fold defaultstate="collapsed" desc=" Look and feel setting code (optional) ">
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






