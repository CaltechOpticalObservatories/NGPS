/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package edu.caltech.palomar.telescopes.telemetry;

import java.time.Instant;
import java.time.ZonedDateTime;
import java.sql.Timestamp;
import nom.tam.fits.Header;
/**
 *
 * @author developer
 */
public class TelemetryObject extends Object{
   public long      ID;
   public Timestamp UTC_TIMESTAMP;
   public String UTC_DATE_TIME;
   public String LOCAL_DATE_TIME;
   public String UTCSTART;
   public long  HTM_LEVEL_7;
   public long  HTM_LEVEL_20;
   public float AIRMASS;
   public float AZIMUTH;
   public float CASSANG;
   public String DEC;
   public float  DECOFF;
   public float  DECTRAC;
   public float  DOMEAZI;
   public String DOMESHU;
   public float  EQUINOX;
   public String HA;
   public String INSTRUMNT;
   public float  LST;
   public String OBJECT;
   public String PUMPS;
   public String RA;
   public float RAOFF;
   public float RATEDEC;
   public float RATERA;
   public float TELFOCUS;
   public String TELID;
   public String TMOTION;
   public String WINDSCR;
   public float ZA;
   // The following parameters are calculated from the RA, DEC and time provided by the telescope. 
   public float ALTITUDE;
   public float ALTPARA;
   public double BARYJD;
   public String BARYVCOR;
   public String CONSTEL;
   public String ELEVHOR;
   public String ELEVSEA;
   public double JD;
   public String LAT;
   public String LONG;
   public float MOONALT;
   public float MOONANG;
   public float MOONAZ;
   public String MOONDEC;
   public String MOONLIG;
   public String MOONILM;
   public String MOONPH;
   public String MOONRA;
   public float PARA;
   public String PLANETW;
   public float SUNALT;
   public float SUNAZ;
   public String SUNDEC;
   public String SUNRA;
   public String ZTWILIGH;
//   public Header CURRENT_HEADER;
/*================================================================================================
/        TelemetryObject()
/=================================================================================================*/   
   public TelemetryObject(){       
   } 
/*================================================================================================
/       toString()
/=================================================================================================*/   
   public java.lang.String  headerString(){
       java.lang.String string_representation = new java.lang.String();
        string_representation ="UTC_TIMESTAMP" +","+
       "UTC_DATE_TIME" +","+
       "LOCAL_DATE_TIME" +","+
       "UTCSTART" +","+
       "RA" +","+
       "DEC" +","+
       "HTM_LEVEL_7" +","+
       "HTM_LEVEL_20" +","+               
       "AIRMASS" +","+ 
       "AZIMUTH" +","+ 
       "CASSANG" +","+ 
       "DECOFF" +","+ 
       "DECTRAC" +","+ 
       "DOMEAZI" +","+ 
       "DOMESHU" +","+ 
       "EQUINOX" +","+ 
       "HA" +","+ 
       "INSTRUMNT" +","+ 
       "LST" +","+ 
       "OBJECT" +","+ 
       "RAOFF" +","+ 
       "RATEDEC" +","+ 
       "RATERA" +","+ 
       "TELFOCUS" +","+ 
       "TELID" +","+ 
       "TMOTION" +","+ 
       "WINDSCR" +","+ 
       "ZA" +","+ 
       "ALTITUDE" +","+ 
       "ALTPARA" +","+ 
       "BARYJD" +","+ 
       "ZA" +","+ 
       "BARYVCOR" +","+ 
       "CONSTEL" +","+ 
       "JD" +","+ 
       "MOONALT" +","+ 
       "MOONANG" +","+ 
       "MOONAZ" +","+ 
       "MOONDEC" +","+ 
       "MOONLIG" +","+ 
       "MOONILM" +","+ 
       "MOONPH" +","+ 
       "MOONRA" +","+ 
       "PARA" +","+ 
       "PLANETW" +","+ 
       "SUNALT" +","+ 
       "SUNAZ" +","+ 
       "SUNDEC" +","+ 
       "SUNRA" +","+ 
       "ZTWILIGH";                                              
       return string_representation;
   }   
/*================================================================================================
/       toString()
/=================================================================================================*/   
   public java.lang.String  toString(){
       java.lang.String string_representation = new java.lang.String();
       string_representation = Long.toString(ID) +","+
       UTC_TIMESTAMP.toString() +","+
       UTC_DATE_TIME +","+
       LOCAL_DATE_TIME +","+
       UTCSTART +","+
       RA +","+
       DEC +","+
       Long.toString(HTM_LEVEL_7) +","+
       Long.toString(HTM_LEVEL_20) +","+               
       Float.toString(AIRMASS) +","+ 
       Float.toString(AZIMUTH) +","+ 
       Float.toString(CASSANG) +","+ 
       Float.toString(DECOFF) +","+ 
       Float.toString(DECTRAC) +","+ 
       Float.toString(DOMEAZI) +","+ 
       DOMESHU +","+ 
       Float.toString(EQUINOX) +","+ 
       HA +","+ 
       INSTRUMNT +","+ 
       Float.toString(LST) +","+ 
       OBJECT +","+ 
       Float.toString(RAOFF) +","+ 
       Float.toString(RATEDEC) +","+ 
       Float.toString(RATERA) +","+ 
       Float.toString(TELFOCUS) +","+ 
       TELID +","+ 
       TMOTION +","+ 
       WINDSCR +","+ 
       Float.toString(ZA) +","+ 
       Float.toString(ALTITUDE) +","+ 
       Float.toString(ALTPARA) +","+ 
       Double.toString(BARYJD) +","+ 
       Float.toString(ZA) +","+ 
       BARYVCOR +","+ 
       CONSTEL +","+ 
       Double.toString(JD) +","+ 
       Float.toString(MOONALT) +","+ 
       Float.toString(MOONANG) +","+ 
       Float.toString(MOONAZ) +","+ 
       MOONDEC +","+ 
       MOONLIG +","+ 
       MOONILM +","+ 
       MOONPH +","+ 
       MOONRA +","+ 
       Float.toString(PARA) +","+ 
       PLANETW +","+ 
       Float.toString(SUNALT) +","+ 
       Float.toString(SUNAZ) +","+ 
       SUNDEC +","+ 
       SUNRA +","+ 
       ZTWILIGH;                                              
       return string_representation;
   }
}

