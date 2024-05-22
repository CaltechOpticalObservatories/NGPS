/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package edu.caltech.palomar.telescopes.telemetry;

import java.sql.Timestamp;
import java.time.ZonedDateTime;

/**
 *
 * @author developer
 */
public class P60weather {
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
    public ZonedDateTime             now;
    public ZonedDateTime             now_utc;
    public Timestamp                 current_timestamp;
    public Timestamp                 UTC_TIMESTAMP;
    public Timestamp                 P60_TIMESTAMP;
    public String                    UTC_DATE_TIME;
    public String                    LOCAL_DATE_TIME;
    public String                    p60_utc_timestamp;
/*================================================================================================
/     P60weather()
/=================================================================================================*/   
  public  P60weather(){   
  }
 /*================================================================================================
/       toString()
/=================================================================================================*/   
   public java.lang.String  headerString(){
       java.lang.String string_representation = new java.lang.String();
        string_representation ="TIMESTAMP_UTC" +","+
       "UTCDATETIME" +","+
       "LOCALDATETIME" +","+
       "TIMESTAMP_P60" +","+
       "OUTSIDE_TEMPERATURE" +","+
       "INSIDE_TEMPERATURE" +","+
       "FLOOR_TEMPERATURE" +","+               
       "OUTSIDE_DEWPOINT" +","+ 
       "INSIDE_DEWPOINT" +","+ 
       "TUBE_AIR_TEMPERATURE" +","+ 
       "TUBE_TOP_TEMPERATURE" +","+ 
       "TUBE_MIDDLE_TEMPERATURE" +","+ 
       "TUBE_BOTTOM_TEMPERATURE" +","+ 
       "PRIMARY_MIRROR_TEMPERATURE" +","+ 
       "PRIMARY_CELL_TEMPERATURE" +","+ 
       "SECONDARY_CELL_TEMPERATURE" +","+ 
       "OUTSIDE_RELATIVE_HUMIDITY" +","+ 
       "INSIDE_RELATIVE_HUMIDITY" +","+ 
       "BAROMETRIC_PRESSURE" +","+ 
       "RAINING" +","+ 
       "WIND_DIRECTION" +","+ 
       "WIND_GUSTS" +","+ 
       "WIND_MEDIAN";
       return string_representation;
   }   
/*================================================================================================
/       toString()
/=================================================================================================*/   
   public java.lang.String  toString(){
       java.lang.String string_representation = new java.lang.String();
       string_representation = current_timestamp.toString() +","+
       now_utc.toString() +","+
       now.toString() +","+
       P60_TIMESTAMP.toString() +","+
       Float.toString((float) P60_outside_air_temp) +","+
       Float.toString((float)P60_dome_air_temp) +","+ 
       Float.toString((float)P60_floor_temp) +","+ 
       Float.toString((float)P60_outside_dewpoint) +","+ 
       Float.toString((float)P60_inside_dewpoint) +","+ 
       Float.toString((float)P60_tube_air_temperature) +","+ 
       Float.toString((float)P60_tube_air_temperature_top) +","+ 
       Float.toString((float)P60_tube_air_temperature_middle) +","+ 
       Float.toString((float)P60_tube_air_temperature_bottom) +","+ 
       Float.toString((float)P60_primary_mirror_temp) +","+ 
       Float.toString((float)P60_primary_cell_temp) +","+ 
       Float.toString((float)P60_secondary_cell_temp) +","+ 
       Float.toString((float)P60_outside_relative_humidity) +","+ 
       Float.toString((float)P60_inside_relative_humidity) +","+ 
       Float.toString((float)P60_barometric_pressure) +","+ 
       Boolean.toString(P60_raining) +","+ 
       Float.toString((float)P60_wind_direction) +","+ 
       Float.toString((float)P60_windspeed_peak) +","+ 
       Float.toString((float)P60_windspeed_median);                             
       return string_representation;
   }     
}
