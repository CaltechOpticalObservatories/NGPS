/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package edu.caltech.palomar.telescopes.telemetry;

import java.sql.Timestamp;

/**
 *
 * @author developer
 */
public class P48weather {
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
/*================================================================================================
/     P48weather() Constructor
/=================================================================================================*/
    public P48weather(){
        
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
       "WINDSPEED_AVG_THRESHOLD" +","+
       "GUST_WINDSPEED_THRESHOLD" +","+
       "ALARM_HOLDTIME" +","+               
       "REMAINING_HOLDTIME" +","+ 
       "OUTSIDE_DEWPOINT_THRESHOLD" +","+ 
       "INSIDE_DEWPOINT_THRESHOLD" +","+ 
       "CURRENT_WIND_DIRECTION" +","+ 
       "CURRENT_WINDSPEED" +","+ 
       "WINDSPEED_AVERAGE" +","+ 
       "OUTSIDE_AIR_TEMPERATURE" +","+ 
       "OUTSIDE_RELATIVE_HUMIDITY" +","+ 
       "OUTSIDE_DEWPOINT" +","+ 
       "INSIDE_AIR_TEMPERATURE" +","+ 
       "INSIDE_RELATIVE_HUMIDITY" +","+ 
       "INSIDE_DEWPOINT";
       return string_representation;
   }   
/*================================================================================================
/       toString()
/=================================================================================================*/   
   public java.lang.String  toString(){
       java.lang.String string_representation = new java.lang.String();
       string_representation = UTC_TIMESTAMP.toString() +","+
       UTC_DATE_TIME +","+
       LOCAL_DATE_TIME +","+
       P48_UTC +","+
       Float.toString((float)P48_Windspeed_Avg_Threshold) +","+
       Float.toString((float)P48_Gust_Speed_Threshold) +","+ 
       Float.toString((float)P48_Alarm_Hold_Time) +","+ 
       Float.toString((float)P48_Remaining_Hold_Time) +","+ 
       Float.toString((float)P48_Outside_DewPt_Threshold) +","+ 
       Float.toString((float)P48_Inside_DewPt_Threshold) +","+ 
       Float.toString((float)P48_Wind_Dir_Current) +","+ 
       Float.toString((float)P48_Windspeed_Current) +","+ 
       Float.toString((float)P48_Windspeed_Average) +","+ 
       Float.toString((float)P48_Outside_Air_Temp) +","+ 
       Float.toString((float)P48_Outside_Rel_Hum) +","+ 
       Float.toString((float)P48_Outside_DewPt) +","+ 
       Float.toString((float)P48_Inside_Air_Temp) +","+ 
       Float.toString((float)P48_Inside_Rel_Hum) +","+ 
       Float.toString((float)P48_Inside_DewPt) +","+ 
       P48_Wetness +","+ 
       P48_Weather_Status;                             
       return string_representation;
   }  
}
