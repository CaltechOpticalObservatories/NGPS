/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package edu.caltech.palomar.telescopes.telemetry;

import java.sql.Timestamp;
import java.time.ZonedDateTime;
import java.util.ArrayList;

/**
 *
 * @author developer
 */
public class P200weather {
    public long          ID = -1;
    public ZonedDateTime now;
    public ZonedDateTime now_utc;
    public Timestamp     current_timestamp;
    public long          P200_raw_timestamp;
    public long          weather_reference_timestamp;
    public long          temperature_reference_timestamp;
    public double        P200_dewpoint;
    public double        P200_outside_air_temp;
    public double        P200_dome_air_temp;
    public double        P200_primary_mirror_temp;
    public double        P200_prime_focus_temp;
    public double        P200_floor_temp;
    public double        P200_primary_cell_temp;
    public double        P200_windspeed_median;
    public double        P200_windspeed_peak;
    public double        P200_windspeed_stddev;
    public double        P200_wind_direction;    
    public boolean       P200_raining;
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
    public java.lang.String p200_utc_timestamp;
    public P200weather(){
        
    }
/*================================================================================================
/       toString()
/=================================================================================================*/   
   public java.lang.String  headerString(){
       java.lang.String string_representation = new java.lang.String();
        string_representation ="TIMESTAMP_UTC" +","+
       "UTCDATETIME" +","+
       "LOCALDATETIME" +","+
       "WEATHER_TIMESTAMP" +","+
       "TEMPERATURE_TIMESTAMP" +","+
       "AVERAGE_WINDSPEED" +","+
       "PEAK_WINDSPEED" +","+               
       "GUST_WINDSPEED" +","+ 
       "WIND_DIRECTION" +","+ 
       "DEWPOINT" +","+ 
       "OUTSIDE_TEMPERATURE" +","+ 
       "INSIDE_TEMPERATURE" +","+ 
       "PRIMARY_MIRROR_TEMPERATURE" +","+ 
       "SECONDARY_MIRROR_TEMPERATURE" +","+ 
       "FLOOR_TEMPERATURE" +","+ 
       "PRIME_FOCUS_TEMPERATURE" +","+ 
       "RAINING" +","+ 
       "STAIRS_0" +","+ 
       "STAIRS_20" +","+ 
       "STAIRS_40" +","+ 
       "STAIRS_60" +","+ 
       "STAIRS_75" +","+ 
       "STAIRS_90" +","+ 
       "GANTRY_UR" +","+ 
       "GANTRY_LR" +","+ 
       "GANTRY_UL" +","+ 
       "GANTRY_LL" +","+ 
       "CRANE" +","+ 
       "EXHAUST" +","+ 
       "SHUTTER" +","+ 
       "FAN_R" +","+ 
       "FAN_L" +","+ 
       "CATWALK" +","+ 
       "ARCH_R" +","+ 
       "ARCH_L" +","+ 
       "GANTRY_FAR_R" +","+ 
       "GANTRY_FAR_L";
       return string_representation;
   }   
/*================================================================================================
/       toString()
/=================================================================================================*/   
   public java.lang.String  toString(){
       java.lang.String string_representation = new java.lang.String();
       string_representation = Long.toString(ID) +","+
       current_timestamp.toString() +","+
       now_utc.toString() +","+
       now.toString() +","+
       weather_reference_timestamp +","+
       temperature_reference_timestamp +","+
       Float.toString((float)P200_windspeed_median) +","+
       Float.toString((float)P200_windspeed_peak) +","+ 
       Float.toString((float)P200_windspeed_stddev) +","+ 
       Float.toString((float)P200_wind_direction) +","+ 
       Float.toString((float)P200_dewpoint) +","+ 
       Float.toString((float)P200_outside_air_temp) +","+ 
       Float.toString((float)P200_dome_air_temp) +","+ 
       Float.toString((float)P200_primary_mirror_temp) +","+ 
       Float.toString((float)P200_primary_cell_temp) +","+ 
       Float.toString((float)P200_floor_temp) +","+ 
       Float.toString((float)P200_prime_focus_temp) +","+ 
       Boolean.toString(P200_raining) +","+ 
       Float.toString((float)P200_temp_stairs0) +","+ 
       Float.toString((float)P200_temp_stairs20) +","+ 
       Float.toString((float)P200_temp_stairs40) +","+ 
       Float.toString((float)P200_temp_stairs60) +","+ 
       Float.toString((float)P200_temp_stairs75) +","+ 
       Float.toString((float)P200_temp_stairs90) +","+ 
       Float.toString((float)P200_temp_crane) +","+ 
       Float.toString((float)P200_temp_exhaust) +","+ 
       Float.toString((float)P200_temp_shutter) +","+ 
       Float.toString((float)P200_temp_fanR) +","+ 
       Float.toString((float)P200_temp_fanL) +","+ 
       Float.toString((float)P200_temp_catwalk) +","+ 
       Float.toString((float)P200_temp_ArchR) +","+ 
       Float.toString((float)P200_temp_ArchL) +","+ 
       Float.toString((float)P200_temp_gantry_farR) +","+ 
       Float.toString((float)P200_temp_gantry_farL);                             
       return string_representation;
   }    
}
