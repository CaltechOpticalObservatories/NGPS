package edu.dartmouth.jskycalc;  
//=== File Prolog =============================================================
//	This code was developed by John Thorstensen of Dartmouth University
//
//      It has been refactored and modified by Jennifer Milburn for
//      use at the Palomar Observatory.
//
//      Caltech Optical Observatories, Palomar Observatory
//
//--- Contents ----------------------------------------------------------------
//
//    JSkyCalcModel
//--- Description -------------------------------------------------------------
//       This is a summary model that performs all of the ephemeris calculations
//     supported by JSkyCalc
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
//       DERIVED DIRECTLY FROM THE JSKYCALC by JOHN THORSTENSEN, Dartmouth University
//       Original Implementation : John Thorstensen copyright 2007,
//
//       August 4, 2010  Jennifer Milburn, refactor for use at the Palomar Observatory
//
//
//   JSkyCalc24m.java -- 2009 January.  I liked the 1.3m so much I decided
//   to do something similar for the 2.4m.  This has the capability of reading
//   the telescope coords and commenting on them, but it doesn't slew the
//   telescope.  It'll be nice to automatically have the telescope coords there,
//   though. */
//
// JSkyCalc13m.java -- 2009 January.  This version adds an interface to
// * Dick Treffers' 1.3m telescope control server, which communicates via
// * the BAIT protocol.  The target-selection facilities here can then be
// * used. */
//
// JSkyCalc.java -- copyright 2007, John Thorstensen, Dartmouth College. */
//
// TERMS OF USE --
// Anyone is free to use this software for any purpose, and to
// modify it for their own purposes, provided that credit is given to the author
// in a prominent place.  For the present program that means that the green
// title and author banner appearing on the main window must not be removed,
// and may not be altered without premission of the author.
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
import java.io.*;
import java.util.List.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.util.HashMap;
import java.util.Vector;
import java.util.*;
import edu.dartmouth.jskycalc.coord.Celest;
import edu.dartmouth.jskycalc.coord.Const;
import edu.dartmouth.jskycalc.coord.Ecliptic;
import edu.dartmouth.jskycalc.coord.GenericCalDat;
import edu.dartmouth.jskycalc.coord.HA;
import edu.dartmouth.jskycalc.coord.InstantInTime;
import edu.dartmouth.jskycalc.coord.RA;
import edu.dartmouth.jskycalc.coord.Site;
import edu.dartmouth.jskycalc.coord.SkyIllum;
import edu.dartmouth.jskycalc.coord.Spherical;
import edu.dartmouth.jskycalc.coord.TimeInterval;
import edu.dartmouth.jskycalc.coord.Topo;
import edu.dartmouth.jskycalc.coord.dec;
import edu.dartmouth.jskycalc.coord.deltaT;
import edu.dartmouth.jskycalc.coord.latitude;
import edu.dartmouth.jskycalc.coord.longitude;
import edu.dartmouth.jskycalc.coord.sexagesimal;
import edu.dartmouth.jskycalc.objects.Observation;
import edu.dartmouth.jskycalc.objects.AllBrightStar;
import edu.dartmouth.jskycalc.objects.AstrObj;
import edu.dartmouth.jskycalc.objects.BrightStar;
import edu.dartmouth.jskycalc.objects.Constel;
import edu.dartmouth.jskycalc.objects.Moon;
import edu.dartmouth.jskycalc.objects.NightlyAlmanac;
import edu.dartmouth.jskycalc.objects.Observation;
import edu.dartmouth.jskycalc.objects.Planets;
import edu.dartmouth.jskycalc.objects.Seasonal;
import edu.dartmouth.jskycalc.objects.Sun;
import edu.dartmouth.jskycalc.objects.WhenWhere;
import edu.dartmouth.jskycalc.objects.guideStar;
import java.beans.*;
/*================================================================================================
/          JSkyCalcModel Class Definition
/=================================================================================================*/
public class JSkyCalcModel {
 transient private PropertyChangeSupport propertyChangeListeners = new PropertyChangeSupport(this);  
 private transient Vector  actionListeners;
 public   static final int SYNCH_OUTPUT       = 1000;
 public   static final int ADVANCE_TIME       = 1001;
 public   static final int ADVANCE_TIME_TRUE  = 1002;
 public   static final int ADVANCE_TIME_FALSE = 1003;
 public   static final int MODEL_UPDATED      = 1010;
 public   static final int SITE_UPDATED       = 1020;
 public static String [] palomarsite = {"Palomar Mountain",  "7.79089",  "33.35667",  "8.",  "1",  "Pacific",  "P",  "1706.",  "1706."};
 public static HashMap<String, Site> siteDict   = new HashMap<String, Site>();
 public AllBrightStar[]              allbrights;
 public BrightStar[]                 bs;

 public   java.lang.String USERDIR          = System.getProperty("user.dir");
 public   java.lang.String SEP              = System.getProperty("file.separator");
 public   java.lang.String CONFIG           = new java.lang.String("config");
 public   java.lang.String RESOURCES        = new java.lang.String("resources");
   //  Definition of JSkyCalc coordinate and object classes
 public WhenWhere     w;
 public InstantInTime i;
 public Site          s;
 public Observation   o;
 public Planets       p;
   // Individual Model Parameters
 String ha_string;
 String siderialobj_string;
 String altitude_string;
 String airmass_string;
 String azimuth_string;
 String baryjd_string;
 String barytcor_string;
 String sunra_string;
 String sundec_string;
 String moonra_string;
 String moondec_string;
 String altsun_string;
 String azsun_string;
 String altmoon_string;
 String azmoon_string;
 String moonillum_string;
 String moonobj_string;
 String baryvcor_string;
 String jd_string;
 String ztwilight_string;
 String UTDateDate;
 String UTDateTime;
 String localDateDate;
 String localDateTime;
 String constellation;
 String moonlight_string;
 String moonphasedescription;
 String ra_string;
 String dec_string;
 String equinox_string;
 String planetwarning;
 String obsname;
 String longitude_string;
 String latitude_string;
 String stdz_string;
 String use_dst_string;
 String zonename;
 String elevsea_string;
 String elevhoriz_string;
 String parallactic_string;
 String altparallactic_string;
 String objectname;
// double precision representation of parameters
 double ha;
 double siderialobj;
 double altitude;
 double airmass;
 double moonlight;
 double azimuth;
 double parallactic;
 double parallactic_double;
 double altparallactic;
 double altparallactic_double;
 double baryjd;
 double barytcor;
 double sunra;
 double sundec;
 double moonra;
 double moondec;
 double altsun;
 double azsun;
 double altmoon;
 double azmoon;
 double moonillum;
 double moonobj;
 double baryvcor;
 double jd;
 double ztwilight;
 double ra;
 double dec;
 double equinox;
 //  Variables used by the read sites related methods
 String [] sitekeys;
 int nsites;
 static String st = null;
 // Site definition parameters
 double longitude;
 double latitude;
 double stdz;
 int    use_dst;
 double elevsea;
 double elevhoriz;
// Housekeeping parameters
 boolean is_UT;
 String dateTimeString    = new java.lang.String();
 String dateString        = new java.lang.String();
 String timeString        = new java.lang.String();
 double JulianDate;
 String timeStep          = new java.lang.String();
 String nearestBrightStar = new java.lang.String();
 private java.lang.String[] date_time_array = new java.lang.String[2];
 private int                time_source;
 public static int         NOW           = 100;
 public static int         SELECTED_TIME = 200;
/*================================================================================================
/          JSkyCalc Constructor
/=================================================================================================*/
    public JSkyCalcModel(){
        jbInit();
    }
/*================================================================================================
/          jbInit() method
/=================================================================================================*/
   private void jbInit(){
     initializeValues();
     initializeObservatory();
     this.setTimeSource(NOW); 
    }
/*================================================================================================
/     initializeValues()
/=================================================================================================*/
 public void initializeValues(){
     setRightAscension( "00:00:00.0");
     setDeclination(    "00:00:00.0");
     setHAString(       " ");
     setSidereal(       " ");
     setSunra(          " ");
     setSundec(         " ");
     setMoonra(         " ");
     setMoondec(        " ");
     setEquinox(        " ");
     setAzimuthString(  " ");
     setSunalt(         " ");
     setSunaz(          " ");
     setMoonalt(        " ");
     setMoonaz(         " ");
     setMoonIllum(      " ");
     setMoonobjang(     " ");
     setBaryvcor(       " ");
     setBaryjd(         " ");
     setBarytcor(       " ");
     setJd(             " ");
     setAltitudeString( " ");
     setParallactic(    " ");
     setAltParallactic( " ");
// STATIC METHODS THAT RETURN TEXT STRINGS
     setConstel(        " ");
     setMoonphase(      " ");
 // moonPhaseDescription
     setUTDateDate(     " ");
     setUTDateTime(     " ");
     setlocalDateDate(  " ");
     setlocalDateTime(  " ");  
     setAirmass(        " ");
     setZtwilight(      " ");
     setMoonlight(      " ");
     setObjectName(     " ");
 }
/*================================================================================================
/          JSkyCalc Initializations initializeObservatory() - use only at Palomar
/=================================================================================================*/
  public void initializeObservatory(){
      LoadAllBright();
      LoadBright();
      s = new Site(palomarsite);
      i = new InstantInTime(s.stdz,s.use_dst);
      w = new WhenWhere(i,s);
      o = new Observation(w,w.zenith2000());
      p = new Planets(w);
      o.ComputeSky(); 
      o.w.MakeLocalSun();
      o.w.MakeLocalMoon();
      o.ComputeSunMoon();
      o.computebary(p);
}
/*================================================================================================
/          JSkyCalc Initializations initializeObservatory() - use if this is NOT Palomar
/=================================================================================================*/
  public void initializeObservatory(java.lang.String siteKey){
      ReadSites();
      LoadAllBright();
      LoadBright();
      s = siteDict.get(siteKey);
      i = new InstantInTime(s.stdz,s.use_dst);
      w = new WhenWhere(i,s);
      o = new Observation(w,w.zenith2000());
      p = new Planets(w);
      o.ComputeSky();
      o.w.MakeLocalSun();
      o.w.MakeLocalMoon();
      o.ComputeSunMoon();
      o.computebary(p);
}
/*================================================================================================
/      setSelectedDateTime(java.lang.String[] new_date_time_array)
/=================================================================================================*/
 public void setSelectedDateTime(java.lang.String[] new_date_time_array)  {
     date_time_array = new_date_time_array;
 }
 public java.lang.String[] getSelectedDateTime(){
     return date_time_array;
 }
/*================================================================================================
/      setTimeSource(int new_time_source)
/=================================================================================================*/
public void setTimeSource(int new_time_source){
     time_source = new_time_source;
 }
 public int getTimeSource(){
     return time_source;
 }
/*================================================================================================
/      setTimeSource(int new_time_source)
/=================================================================================================*/
 public void setBrightStars(BrightStar[] new_bs){
     bs = new_bs;
 }
/*================================================================================================
/      SetToNow()
/=================================================================================================*/
   public void SetToNow() {
      // Get site and UT options
      synchSite();
      o.w.SetToNow();
      o.w.ComputeSunMoon();
      synchOutput();
   }
/*================================================================================================
/     setDateTime(java.lang.String newDateString, java.lang.String newTimeString);
/=================================================================================================*/
  public void setDateTime(java.lang.String newDateString, java.lang.String newTimeString){
    try{  
     dateString = newDateString;
     timeString = newTimeString;
     String [] datepieces = dateString.split("\\s+");
     dateTimeString = String.format(Locale.ENGLISH, "%s %s %s %s",datepieces[0],datepieces[1], datepieces[2],timeString);
    }catch(Exception e){
        System.out.println(e.toString());
    }
  }
/*================================================================================================
/     setJulianDate(double newJD)
/=================================================================================================*/
  public void setJulianDate(double newJD){
      JulianDate = newJD;
  }
  public double getJulianDate(){
      return JulianDate;
  }
/*================================================================================================
/     setTimeStep(String newTimeStep)
/=================================================================================================*/
  public void setTimeStep(String newTimeStep){
      timeStep = newTimeStep;
  }
  public String getTimeStep(){
      return timeStep;
  }
/*================================================================================================
/     setToDate()
/=================================================================================================*/
  public void setToDate(java.lang.String newDateString, java.lang.String newTimeString) {
      setDateTime(newDateString,newTimeString);
      setToDate();
   }
/*================================================================================================
/     setToDate()
/=================================================================================================*/
  public void setToDate() {
      // Get site and UT options
      synchSite();
      // set the actual time ...
      o.w.ChangeWhen(dateTimeString, is_UT);
      o.w.ComputeSunMoon();
      synchOutput();
  }
/*================================================================================================
/       public void setToJD
/=================================================================================================*/
  public void setToJD() {
      // Get site and UT options, and set them ...
      synchSite();
      // change the time using current options
      o.w.ChangeWhen(JulianDate);
      o.w.ComputeSunMoon();
      synchOutput();
  }
/*================================================================================================
/     advanceTime()
/=================================================================================================*/
   public void advanceTime(String newTimeStep) {
      setTimeStep(newTimeStep);
      advanceTime();
   }
/*================================================================================================
/     advanceTime()
/=================================================================================================*/
  public void advanceTime(String newTimeStep,boolean forward) {
      setTimeStep(newTimeStep);
      advanceTime(forward);
   }
/*================================================================================================
/     advanceTime()
/=================================================================================================*/
  public void advanceTime() {
      synchSite();
      o.w.AdvanceWhen(timeStep);
      o.w.ComputeSunMoon();
      synchOutput();
   }
/*================================================================================================
/     advanceTime(boolean forward)
/=================================================================================================*/
  public void advanceTime(boolean forward) {
      synchSite();
      o.w.AdvanceWhen(timeStep, forward);
      o.w.ComputeSunMoon();
      synchOutput();
   }
/*================================================================================================
/       void updateParameters()
/=================================================================================================*/
  public void updateParameters(){
 // Double precision versions of all the applicable parameters
     ra             = o.c.Alpha.degrees();
     dec            = o.c.Delta.degrees();
     equinox        = o.c.Equinox;
     altitude       = o.altitude;
     ha             = o.ha.degrees();
     siderialobj    = o.w.siderealobj.degrees();
     azimuth        = o.azimuth;
     sunra          = o.w.sun.topopos.Alpha.degrees();
     sundec         = o.w.sun.topopos.Delta.degrees();
     moonra         = o.w.moon.topopos.Alpha.degrees();
     moondec        = o.w.moon.topopos.Delta.degrees();
     altsun         = o.w.altsun;
     azsun          = o.w.azsun;
     altmoon        = o.w.altmoon;
     azmoon         = o.w.azmoon;
     moonillum      = o.w.moonillum;
     moonobj        = o.moonobj;
     baryvcor       = o.baryvcor;
     baryjd         = o.baryjd;
     barytcor       = o.barytcor;
     jd             = o.w.when.jd;
     airmass        = o.airmass;
     ztwilight      = o.w.twilight;
     moonlight      = o.moonlight;
     parallactic    = o.parallactic;
// PARALLACTIC CALCULATION CONSTRUCTION
      while(parallactic < -180.) parallactic += 360.;
      while(parallactic >= 180.) parallactic -= 360.;
            altparallactic = parallactic + 180.;
      while(altparallactic < -180.) altparallactic += 360.;
      while(altparallactic >= 180.) altparallactic -= 360.;
// STRING REPRESENTATIONS OF THE EPHEMERIS PARAMETERS
     setRightAscension( o.c.Alpha.RoundedRAString(2," "));
     setDeclination(    o.c.Delta.RoundedDecString(1," "));
     setHAString(       o.ha.RoundedHAString(0," "));
     setSidereal(       o.w.siderealobj.RoundedRAString(0,":"));
     setSunra(          o.w.sun.topopos.Alpha.RoundedRAString(1," "));
     setSundec(         o.w.sun.topopos.Delta.RoundedDecString(0," "));
     setMoonra(         o.w.moon.topopos.Alpha.RoundedRAString(1," "));
     setMoondec(        o.w.moon.topopos.Delta.RoundedDecString(0," "));
     setEquinox(        String.format(Locale.ENGLISH, "%7.2f",     equinox));
     setAzimuthString(  String.format(Locale.ENGLISH, "%6.1f",     azimuth));
     setSunalt(         String.format(Locale.ENGLISH,"%5.1f",      altsun));
     setSunaz(          String.format(Locale.ENGLISH, "%6.1f",     azsun));
     setMoonalt(        String.format(Locale.ENGLISH, "%5.1f",     altmoon));
     setMoonaz(         String.format(Locale.ENGLISH, "%6.1f",     azmoon));
     setMoonIllum(      String.format(Locale.ENGLISH, "%5.3f",     moonillum));
     setMoonobjang(     String.format(Locale.ENGLISH, "%5.1f deg", moonobj));
     setBaryvcor(       String.format(Locale.ENGLISH, "%6.2f km/s",baryvcor));
     setBaryjd(         String.format(Locale.ENGLISH, "%13.5f",    baryjd));
     setBarytcor(       String.format(Locale.ENGLISH, "[%6.1f s]", barytcor));
     setJd(             String.format(Locale.ENGLISH, "%15.6f",    jd));
     setAltitudeString( String.format(Locale.ENGLISH, "%5.1f",     altitude));
     setParallactic(    String.format(Locale.ENGLISH, "%5.1f degr.",parallactic));
     setAltParallactic( String.format(Locale.ENGLISH, "[%5.1f] degr.",altparallactic));
     setParallacticDouble(parallactic);
     setAltParallacticDouble(altparallactic);
// STATIC METHODS THAT RETURN TEXT STRINGS
     setConstel(  Constel.getconstel(o.c));
     setMoonphase(     o.w.moon.MoonPhaseDescr(o.w.when.jd));
 // moonPhaseDescription
     setUTDateDate(    o.w.when.UTDate.RoundedCalString(6,0));
     setUTDateTime(    o.w.when.UTDate.RoundedCalString(11,1));
     setlocalDateDate( o.w.when.localDate.RoundedCalString(6,0));
     setlocalDateTime( o.w.when.localDate.RoundedCalString(11,1));
//  AIRMASS FORMATING FIELD
//    AIRMASS SPECIAL STRING CONSTRUCTION - NOT USED SINCE TELESCOPE AIRMASS IS DOMINATE
      if(altitude < 0.)       setAirmass("(down.)");
      else if (airmass > 10.) setAirmass( "> 10.");
      else                      setAirmass( String.format(Locale.ENGLISH, "%6.3f",airmass));
// ZTWILIGHT CONSTRUCTION
      if(altsun > 0.)
         setZtwilight("(Daytime.)");
      else if(altsun < -18.)
         setZtwilight("No twilight.");
      else {
         setZtwilight( String.format(Locale.ENGLISH, "%5.1f mag (blue)",ztwilight));
      }
// SKY LUMMINOCITY CONSTRUCTION
      if (     o.w.altmoon < -2.)
          setMoonlight("Moon is down.");
      else if (o.altitude < 0.)
          setMoonlight( "Target is down.");
      else if( o.w.altsun < -12.) {
          setMoonlight( String.format(Locale.ENGLISH, "%5.1f V mag/sq arcsec",o.moonlight));}
      else if (o.w.altsun < 0.)
          setMoonlight( "(Bright twilight.)");
      else
          setMoonlight( "(Daytime.)");
  }
/*================================================================================================
/     checkPlanets()
/=================================================================================================*/
      void checkPlanets() {
      int i;
      double sepn;
      String planetwarning_temp = new String();
      String warningtext = "";
      int warninglevel = 0;  // save worst warning ... 0 none, 1 orange, 2 red.

      for(i = 0; i < 9; i++) {
         if (i != 2) {  // skip earth ....
            sepn = Spherical.subtend(o.c, p.PlanetObs[i].c) * Const.DEG_IN_RADIAN;
            if(sepn < 3.) {
               if(i > 0 & i < 6) {  // Venus through Saturn
                  planetwarning_temp = planetwarning_temp + String.format(Locale.ENGLISH, "%s - %4.2f deg ",p.names[i],sepn);
                  if(sepn < 1. ) {
                     warninglevel = 2;
                  }
                  else if (warninglevel < 2) warninglevel = 1;
               }
               else {   // the rest of the planets
                  if(sepn < 1.) {
                     planetwarning_temp = planetwarning_temp + String.format(Locale.ENGLISH, "%s - %4.2f deg ",p.names[i],sepn);
                     if (warninglevel < 1) warninglevel = 1;
                  }
               }
            }
         }
      }
      if (warningtext.equals("")) {
         planetwarning_temp =  " --- ";
      }
      setPlanetproxim(planetwarning_temp);
   }
/*================================================================================================
/        synchOutput()
/=================================================================================================*/
 public void setTelescopePosition(java.lang.String new_ra_string,java.lang.String new_dec_string, java.lang.String new_equinox_string){
    setRightAscension(new_ra_string);
    setDeclination(new_dec_string);
    setEquinox(new_equinox_string);
 }
/*================================================================================================
/        synchOutput()
/=================================================================================================*/
 public void synchOutput(java.lang.String new_ra_string,java.lang.String new_dec_string, java.lang.String new_equinox_string){
      setTelescopePosition(new_ra_string,new_dec_string,new_equinox_string);
      synchOutput();
 }
/*================================================================================================
/        synchOutput()
/=================================================================================================*/
 public void synchOutput(){
    if(ra_string.length() != 0){
       o.c.UpdateFromStrings(ra_string,dec_string,equinox_string);
    }
   // This function simply gets the RA and Dec from the GUI. Replace this with the RA, Dec and Equinox of the telescope
   // UPDATE POSITION FROM TELESCOPE  - do it this way for directly from the double values
   // or update from the string directly retrieved from the telescope
   // PERFORM THE ACTUAL CALCULATIONS BY COMPUTING THE SKY, SUN, MOON, PLANETS
   o.ComputeSky();
   o.ComputeSunMoon();
   o.computebary(p);
   checkPlanets();
   updateParameters();
   fireActionPerformed(new java.awt.event.ActionEvent(this,SYNCH_OUTPUT,"synchOutput()"));
//   UPDATE THE RA,DEC, EQUINOX AGAIN - MAY NOT BE NECESSARY
//   o.c.UpdateFromStrings(ra_string,dec_string,equinox_string);
// SETTING THE BACKGROUNDS
//      Color mooncolor;  // used for several fields
//      mooncolor = MoonWarningColor(o.w.altmoon,o.altitude,o.w.altsun,o.moonobj,o.moonlight);
//      airmassfield.setBackground(AirMassWarningColor(o.altitude,o.airmass));
//      ztwilightfield.setBackground(TwilightWarningColor(o.w.altsun,o.w.twilight));
//      HAfield.setBackground(HAWarningColor(o.ha.value));
//      lunskyfield.setBackground(           mooncolor);
//      moonobjangfield.setBackground(       mooncolor);
     }
/*================================================================================================
/        synchSite()
/=================================================================================================*/
  public void synchSite() {
  // I have removed any support for simply editing the site parameters since this is intended
  // as a GUI and control system for observatories that don't really move :), not for
  // amateur astronomers in their back yard.  If we want to support that in the future it would
  // be good to add an interface to a GPS unit or the GPS in a mobile phone to update this
  // information automatically and remove the possibility of editing the values incorrectly.
   longitude   = o.w.where.longit.value;
   latitude    = o.w.where.lat.degrees();
   stdz        = o.w.where.stdz;
   use_dst     = o.w.where.use_dst;
   elevsea     = o.w.where.elevsea;
   elevhoriz   = o.w.where.elevhoriz;
   setObsname(   o.w.where.name);
   setLongitude( o.w.where.longit.RoundedLongitString(1,":",false));
   setLatitude(  o.w.where.lat.RoundedDecString(0,":"));
   setZonename(  o.w.where.timezone_name);
   setElevhoriz( String.format(Locale.ENGLISH, "%4.0f m",elevhoriz));
   setElevsea(   String.format(Locale.ENGLISH, "%4.0f m",elevsea));
   setStdz(      String.format(Locale.ENGLISH, "%5.2f",stdz));
   setUse_dst(   String.format(Locale.ENGLISH, "%d",use_dst));
   fireActionPerformed(new java.awt.event.ActionEvent(this,SITE_UPDATED,"synchSite()"));
   }
/*================================================================================================
/        ReadSites()
/=================================================================================================*/
     public void ReadSites() {

         BufferedReader br = null;
         boolean inquote = false;
         String [] fields = new String[14];  // too many but that's ok
         int i,j;

         sitekeys = new String[40];         // hard-coded, sorry
         nsites = 0;
         try {
             FileInputStream fis = new FileInputStream(USERDIR + SEP + CONFIG + SEP + RESOURCES + "skycalcsites.txt");
             br = new BufferedReader(new InputStreamReader(fis));
         } catch (Exception e) {
             System.out.printf("Problem opening skycalcsites.dat for input.\n");
         }

         // read the site info character-by-character to preserve quoted values.
         // there's undoubtedly a better way, but this works well enough.

         try {
            while((st = br.readLine()) != null) {
               // System.out.printf("read: %s\n",st);
               if(st.length() > 0) {
                  if(st.charAt(0) != '#') {
                     j = 0;   // field counter
                     fields[j] = "";
                     for(i = 0; i < st.length(); i++) {
                        char [] thischar = {st.charAt(i)};
                        if(st.charAt(i) == '"') {
                            if(inquote) inquote = false;
                            else inquote = true;
                        }
                        else {
                           if(inquote) fields[j] = fields[j] + new String(thischar);
                           else {
                              if (st.charAt(i) == ',') {
                                 j = j + 1;
                                 fields[j] = "";
                              }
                              else fields[j] = fields[j] + new String(thischar);
                           }
                        }
                     }
                     siteDict.put(fields[0],new Site(fields));
                     sitekeys[nsites] = fields[0];  // so they'll come out in order ...
                     nsites++;
      //               for(j = 0; j < fields.length; j++) System.out.printf("%s ",fields[j]);
       //              System.out.printf("\n");
                  }
               }
            }
         } catch (IOException e) {System.out.printf("IO exception\n");}
      }
/*================================================================================================
/     getAllBrightStars()
/=================================================================================================*/
public AllBrightStar[] getAllBrightStars(){
    return allbrights;
}
/*================================================================================================
/     getBrightStars()
/=================================================================================================*/
public BrightStar[] getBrightStars(){
    return bs;
}
/*================================================================================================
/     LoadAllBright()
/=================================================================================================*/
  public void LoadAllBright() {

       int i = 0;
       allbrights = new AllBrightStar [9096];
       File infile = null;
       FileReader fr = null;
       BufferedReader br = null;
       String st;
       try {
//           ClassLoader cl = this.getClass().getClassLoader();
//    	   InputStream is = cl.getResourceAsStream("bright_pmupdated.dat");
//           br = new BufferedReader(new InputStreamReader(is));
           System.out.println(USERDIR + SEP + CONFIG + SEP + RESOURCES + SEP + "bright_pmupdated.dat");
           FileInputStream fis = new FileInputStream(USERDIR + SEP + CONFIG + SEP + RESOURCES + SEP + "bright_pmupdated.dat");
           br = new BufferedReader(new InputStreamReader(fis));
       } catch (Exception e)
         { System.out.printf("Problem opening bright_pmupdated.dat for input.\n"); }

       try {
          while((st = br.readLine()) != null) {
              allbrights[i] = new AllBrightStar(st); 
              i++;
          }
       } catch (IOException e) { System.out.println(e); }

       //infile.close(); 

       System.out.printf("%d bright stars read (entire catlog).\n",allbrights.length); 
  }
/*================================================================================================
/        LoadBright()
/=================================================================================================*/
    public void LoadBright() {
          int i = 0;
          bs = new BrightStar [904];
          File infile = null;
          FileReader fr = null;
          BufferedReader br = null;
          String st;

          try {
//	      ClassLoader cl = this.getClass().getClassLoader();
//            InputStream is = cl.getResourceAsStream("brightest.dat");
//            br = new BufferedReader(new InputStreamReader(is));
               FileInputStream fis = new FileInputStream(USERDIR + SEP + CONFIG + SEP + RESOURCES + SEP + "brightest.dat");
               br = new BufferedReader(new InputStreamReader(fis));

              // infile = new File("brightest.dat");
            //  fr = new FileReader(infile);
          } catch (Exception e)
            { System.out.printf("Problem opening brightest.dat for input.\n"); }

          try {
             while((st = br.readLine()) != null) {
                 bs[i] = new BrightStar(st);
                 i++;
             }
          } catch (IOException e) { System.out.println(e); }

          //infile.close();

          System.out.printf("%d bright stars read for display.\n",bs.length);
       }
/*================================================================================================
/     NearestBright(Celest incel)
/=================================================================================================*/
  public void NearestBright(Celest incel) {
       /* Find the bright star nearest to the given coordinates,
       from the _full_ Bright Star List and load it into the display. */
       double decband = 10.;     // degrees
       double decin;
       Celest objcel;
       double sep, minsep = 1000000000000.;
       int i, minindex = 0;
       decin = incel.Delta.value;

       System.out.printf("Looking up bright star nearest to %s ... \n",incel.checkstring());

       for(i = 0; i < allbrights.length; i++) {
           if(Math.abs(decin - allbrights[i].c.Delta.value) < decband) {  // guard expensive subtend
               sep = Spherical.subtend(incel,allbrights[i].c);
               if (sep < minsep) {
                   minsep = sep;
                   minindex = i;
               }
          }
       }
       String outname = String.format("%s_V=%4.1f",allbrights[minindex].name,allbrights[minindex].mag);
       setNearestBrightStar(outname);
//   I'm not sure where to display this data since we DON'T want to set the RA and DEC of the system
//   automatically to this value.  Defer implementation for now.
//       objnamefield.setText(outname);
//       RAfield.setText(allbrights[minindex].c.Alpha.RoundedRAString(2," "));
//       decfield.setText(allbrights[minindex].c.Delta.RoundedDecString(1," "));
//       equinoxfield.setText(String.format(Locale.ENGLISH, "%7.2f",allbrights[minindex].c.Equinox));
   }
/*================================================================================================
/        setNearestBrightStar(java.lang.String newNearestBrightStar)
/=================================================================================================*/
  public void setNearestBrightStar(java.lang.String new_NearestBrightStar){
    java.lang.String  old_NearestBrightStar = this.nearestBrightStar;
    this.nearestBrightStar = new_NearestBrightStar;
    propertyChangeListeners.firePropertyChange("nearestBrightStar", old_NearestBrightStar, new_NearestBrightStar);
  }
  public java.lang.String getNearestBrightStar(){
    return nearestBrightStar;
  }
/*================================================================================================
/         Site getSite()
/=================================================================================================*/
  public Site getSite(){
      return s;
  }
/*================================================================================================
/         getObsevation()
/=================================================================================================*/
  public Observation getObsevation(){
      return o;
  }
/*================================================================================================
/         getWhenWhere()
/=================================================================================================*/
  public WhenWhere getWhenWhere(){
      return w;
  }
/*================================================================================================
/         getPlanets()
/=================================================================================================*/
  public Planets getPlanets(){
      return p;
  }
/*================================================================================================
/       setUT(boolean new_is_UT)
/=================================================================================================*/
  public void setUT(boolean new_is_UT){
     is_UT = new_is_UT; 
  }
  public boolean isUT(){
     return is_UT;
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
/   ActionEvent Handling Code - used to informing registered listeners that a FITS
/      file is ready to be retrieved from the server
/  removeActionListener()
/  addActionListener()
/  fireActionPerformed()
/=============================================================================================*/
/*=============================================================================================
/     synchronized void removeActionListener(ActionListener l)
/=============================================================================================*/
  public synchronized void removeActionListener(ActionListener l) {
    if (actionListeners != null && actionListeners.contains(l)) {
      Vector v = (Vector) actionListeners.clone();
      v.removeElement(l);
      actionListeners = v;
    }
  }
/*=============================================================================================
/    synchronized void addActionListener(ActionListener l)
/=============================================================================================*/
  public synchronized void addActionListener(ActionListener l) {
    Vector v = actionListeners == null ? new Vector(2) : (Vector) actionListeners.clone();
    if (!v.contains(l)) {
      v.addElement(l);
      actionListeners = v;
    }
  }
/*=============================================================================================
/     protected void fireActionPerformed(ActionEvent e)
/=============================================================================================*/
  public void fireActionPerformed(ActionEvent e) {
    if (actionListeners != null) {
      Vector listeners = actionListeners;
      int count = listeners.size();
      for (int i = 0; i < count; i++) {
        ((ActionListener) listeners.elementAt(i)).actionPerformed(e);
      }
    }
}
/*================================================================================================
/       Ephemeris Related Parameters
================================================================================================*/
/*================================================================================================
/   setBaryjd(java.lang.String newBaryjd)
/=================================================================================================*/
      public java.lang.String getJd() {
         return jd_string;
      }
      public void setJd(java.lang.String new_jd) {
         java.lang.String  old_jd = this.jd_string;
         this.jd_string = new_jd;
         propertyChangeListeners.firePropertyChange("jd_string", old_jd, new_jd);
      }
/*================================================================================================
/   setMoonobjang
/=================================================================================================*/
      public java.lang.String getMoonobjang() {
         return moonobj_string;
      }
      public void setMoonobjang(java.lang.String newMoonobjang) {
         java.lang.String  oldMoonobjang = this.moonobj_string;
         this.moonobj_string = newMoonobjang;
         propertyChangeListeners.firePropertyChange("moonobj_string", oldMoonobjang, newMoonobjang);
      }
/*================================================================================================
/   setBaryjd(java.lang.String newBaryjd)
/=================================================================================================*/
      public java.lang.String getBaryjd() {
         return baryjd_string;
      }
      public void setBaryjd(java.lang.String newBaryjd) {
         java.lang.String  oldBaryjd = this.baryjd_string;
         this.baryjd_string = newBaryjd;
         propertyChangeListeners.firePropertyChange("baryjd_string", oldBaryjd, newBaryjd);
      }       
/*================================================================================================
/   setBaryvcor(java.lang.String newBaryvcor)
/=================================================================================================*/
      public java.lang.String geBaryvcort() {
         return baryvcor_string;
      }
      public void setBaryvcor(java.lang.String newBaryvcor) {
         java.lang.String  oldBaryvcor = this.baryvcor_string;
         this.baryvcor_string = newBaryvcor;
         propertyChangeListeners.firePropertyChange("baryvcor_string", oldBaryvcor, newBaryvcor);
      }
/*================================================================================================
/   setBarytcor(java.lang.String newBarytcor)
/=================================================================================================*/
      public java.lang.String geBarytcort() {
         return barytcor_string;
      }
      public void setBarytcor(java.lang.String newBarytcor) {
         java.lang.String  oldBarytcor = this.barytcor_string;
         this.barytcor_string = newBarytcor;
         propertyChangeListeners.firePropertyChange("barytcor_string", oldBarytcor, newBarytcor);
      }
/*================================================================================================
/   setConstel(java.lang.String newconstel)
/=================================================================================================*/
      public java.lang.String getConstel() {
         return constellation;
      }
      public void setConstel(java.lang.String newConstel) {
         java.lang.String  oldConstel = this.constellation;
         this.constellation = newConstel;
         propertyChangeListeners.firePropertyChange("constellation", oldConstel, newConstel);
      }        
/*================================================================================================
/   setPlanetproxim(java.lang.String newPlanetproxim)
/=================================================================================================*/
      public java.lang.String getPlanetproxim() {
         return planetwarning;
      }
      public void setPlanetproxim(java.lang.String newPlanetproxim) {
         java.lang.String  oldPlanetproxim = this.planetwarning;
         this.planetwarning = newPlanetproxim;
         propertyChangeListeners.firePropertyChange("planetwarning", oldPlanetproxim, newPlanetproxim);
      }               
/*================================================================================================
/   setMoonradec(java.lang.String newMoonradec) 
/=================================================================================================*/
      public java.lang.String getMoonra() {
         return moonra_string;
      }
      public void setMoonra(java.lang.String newMoonra) {
         java.lang.String  oldMoonra = this.moonra_string;
         this.moonra_string = newMoonra;
         propertyChangeListeners.firePropertyChange("moonra_string", oldMoonra, newMoonra);
      }
/*================================================================================================
/   setMoonradec(java.lang.String newMoonradec)
/=================================================================================================*/
      public java.lang.String getMoondec() {
         return moondec_string;
      }
      public void setMoondec(java.lang.String newMoondec) {
         java.lang.String  oldMoondec = this.moondec_string;
         this.moondec_string = newMoondec;
         propertyChangeListeners.firePropertyChange("moondec_string", oldMoondec, newMoondec);
      }
/*================================================================================================
/   setMoonalt(java.lang.String newMoonalt)
/=================================================================================================*/
      public java.lang.String getMoonalt() {
         return altmoon_string;
      }
      public void setMoonalt(java.lang.String newMoonalt) {
         java.lang.String  oldMoonalt = this.altmoon_string;
         this.altmoon_string = newMoonalt;
         propertyChangeListeners.firePropertyChange("altmoon_string", oldMoonalt, newMoonalt);
      }  
/*================================================================================================
/   setMoonaz(java.lang.String newMoonaz)
/=================================================================================================*/
      public java.lang.String getMoonaz() {
         return azmoon_string;
      }
      public void setMoonaz(java.lang.String newMoonaz) {
         java.lang.String  oldMoonaz = this.azmoon_string;
         this.azmoon_string = newMoonaz;
         propertyChangeListeners.firePropertyChange("azmoon_string", oldMoonaz, newMoonaz);
      }        
/*================================================================================================
/   setSunra(java.lang.String newSunra)
/=================================================================================================*/
      public java.lang.String getSunra() {
         return sunra_string;
      }
      public void setSunra(java.lang.String newSunra) {
         java.lang.String  oldSunra = this.sunra_string;
         this.sunra_string = newSunra;
         propertyChangeListeners.firePropertyChange("sunra_string", oldSunra, newSunra);
      }
/*================================================================================================
/   setSundec(java.lang.String newSundec)
/=================================================================================================*/
      public java.lang.String getSundec() {
         return sundec_string;
      }
      public void setSundec(java.lang.String newSundec) {
         java.lang.String  oldSundec = this.sundec_string;
         this.sundec_string = newSundec;
         propertyChangeListeners.firePropertyChange("sundec_string", oldSundec, newSundec);
      }
/*================================================================================================
/   setSunalt(java.lang.String newSunalt)
/=================================================================================================*/
      public java.lang.String getSunalt() {
         return altsun_string;
      }
      public void setSunalt(java.lang.String newSunalt) {
         java.lang.String  oldSunalt = this.altsun_string;
         this.altsun_string = newSunalt;
         propertyChangeListeners.firePropertyChange("altsun_string", oldSunalt, newSunalt);
      }
/*================================================================================================
/   setSunaz(java.lang.String newSunaz)
/=================================================================================================*/
      public java.lang.String getSunaz() {
         return azsun_string;
      }
      public void setSunaz(java.lang.String newSunaz) {
         java.lang.String  oldSunaz = this.azsun_string;
         this.azsun_string = newSunaz;
         propertyChangeListeners.firePropertyChange("azsun_string", oldSunaz, newSunaz);
      }
/*================================================================================================
/   setZtwilight(java.lang.String newZtwilight) 
/=================================================================================================*/
      public java.lang.String getZtwilight() {
         return ztwilight_string;
      }
      public void setZtwilight(java.lang.String newZtwilight) {
         java.lang.String  oldZtwilight = this.ztwilight_string;
         this.ztwilight_string = newZtwilight;
         propertyChangeListeners.firePropertyChange("ztwilight_string", oldZtwilight, newZtwilight);
      }        
/*================================================================================================
/   setMoonphase(java.lang.String newMoonphase)
/=================================================================================================*/
      public java.lang.String getMoonphase() {
         return moonphasedescription;
      }
      public void setMoonphase(java.lang.String newMoonphase) {
         java.lang.String  oldMoonphase = this.moonphasedescription;
         this.moonphasedescription = newMoonphase;
         propertyChangeListeners.firePropertyChange("moonphasedescription", oldMoonphase, newMoonphase);
      }        
/*================================================================================================
/   setSidereal(java.lang.String newsidereal)
/=================================================================================================*/
      public java.lang.String getSidereal() {
         return siderialobj_string;
      }
      public void setSidereal(java.lang.String newsidereal) {
         java.lang.String  oldsidereal = this.siderialobj_string;
         this.siderialobj_string = newsidereal;
         propertyChangeListeners.firePropertyChange("siderialobj_string", oldsidereal, newsidereal);
      }
/*================================================================================================
/   setHAString(java.lang.String newHA)
/=================================================================================================*/
      public java.lang.String getHAString() {
         return ha_string;
      }
      public void setHAString(java.lang.String newHA) {
         java.lang.String  oldHA = this.ha_string;
         this.ha_string = newHA;
         propertyChangeListeners.firePropertyChange("ha_string", oldHA, newHA);
      }
/*================================================================================================
/   setAirmass(java.lang.String newAirmass)
/=================================================================================================*/
      public java.lang.String getAirmass() {
         return airmass_string;
      }
      public void setAirmass(java.lang.String newAirmass) {
         java.lang.String  oldAirmass = this.airmass_string;
         this.airmass_string = newAirmass;
         propertyChangeListeners.firePropertyChange("airmass_string", oldAirmass, newAirmass);
      }
/*================================================================================================
/   setParallactic(java.lang.String newParallactic)
/=================================================================================================*/
      public java.lang.String getParallactic() {
         return parallactic_string;
      }
      public void setParallactic(java.lang.String newParallactic) {
         java.lang.String  oldParallactic = this.parallactic_string;
         this.parallactic_string = newParallactic;
         propertyChangeListeners.firePropertyChange("parallactic_string", oldParallactic, newParallactic);
      }
/*================================================================================================
/   setParallactic(double newParallactic)
/=================================================================================================*/
      public double getParallacticDouble() {
         return parallactic_double;
      }
      public void setParallacticDouble(double newParallactic) {
         double  oldParallactic = this.parallactic_double;
         this.parallactic_double = newParallactic;
         propertyChangeListeners.firePropertyChange("parallactic", Double.valueOf(oldParallactic), Double.valueOf(newParallactic));
      }
/*================================================================================================
/   setAltParallactic(java.lang.String newAltParallactic)
/=================================================================================================*/
      public java.lang.String getAltParallactic() {
         return altparallactic_string;
      }
      public void setAltParallactic(java.lang.String newAltParallactic) {
         java.lang.String  oldAltParallactic = this.altparallactic_string;
         this.altparallactic_string = newAltParallactic;
         propertyChangeListeners.firePropertyChange("altparallactic_string", oldAltParallactic, newAltParallactic);
      }
 /*================================================================================================
/   setAltParallactic(java.lang.String newAltParallactic)
/=================================================================================================*/
      public double getAltParallacticDouble() {
         return altparallactic_double;
      }
      public void setAltParallacticDouble(double newAltParallactic) {
         double  oldAltParallactic = this.altparallactic_double;
         this.altparallactic_double = newAltParallactic;
         propertyChangeListeners.firePropertyChange("altparallactic_double", Double.valueOf(oldAltParallactic), Double.valueOf(newAltParallactic));
      }
/*================================================================================================
/   setMoonlight(java.lang.String newMoonlight)
/=================================================================================================*/
      public java.lang.String getMoonlight() {
         return moonlight_string;
      }
      public void setMoonlight(java.lang.String newMoonlight) {
         java.lang.String  oldMoonlight = this.moonlight_string;
         this.moonlight_string = newMoonlight;
         propertyChangeListeners.firePropertyChange("moonlight_string", oldMoonlight, newMoonlight);
      }
/*================================================================================================
/   setMoonIllum(java.lang.String newMoonIllum)
/=================================================================================================*/
      public java.lang.String getMoonIllum() {
         return moonillum_string;
      }
      public void setMoonIllum(java.lang.String newMoonIllum) {
         java.lang.String  oldMoonIllum = this.moonillum_string;
         this.moonillum_string = newMoonIllum;
         propertyChangeListeners.firePropertyChange("moonillum_string", oldMoonIllum, newMoonIllum);
      }
/*================================================================================================
/      setRightAscension
/=================================================================================================*/
  public void setRightAscension(String right_ascension) {
     String  oldright_ascension = this.ra_string;
    this.ra_string = right_ascension;
    propertyChangeListeners.firePropertyChange("ra_string", oldright_ascension, right_ascension);
  }
  public String getRightAscension() {
    return ra_string;
  }
/*================================================================================================
/      setDeclination
/=================================================================================================*/
  public void setDeclination(String declination) {
    String  oldDeclination = this.dec_string;
    this.dec_string = declination;
    propertyChangeListeners.firePropertyChange("dec_string", oldDeclination, declination);
  }
  public String getDeclination() {
    return dec_string;
  }
/*================================================================================================
/       setEquinox(String equinox_string)
/=================================================================================================*/
  public void setEquinox(String equinox_string) {
      // We are expecting one of the following strings "J2000" or "B1950"
    String  oldequinox_string = this.equinox_string;
    this.equinox_string = equinox_string;
    propertyChangeListeners.firePropertyChange("equinox_string", oldequinox_string, equinox_string);
  }
  public String getEquinox() {
    return equinox_string;
  }
/*================================================================================================
/   setAzimuthString(java.lang.String newAzimuth)
/=================================================================================================*/
  public void setAzimuthString(java.lang.String new_azimuth) {
    java.lang.String  old_azimuth = this.azimuth_string;
    this.azimuth_string= new_azimuth;
    propertyChangeListeners.firePropertyChange("azimuth_string", old_azimuth, new_azimuth);
   }
  public java.lang.String getAzimuthString() {
    return azimuth_string;
  }
/*================================================================================================
/   setAltitudeString(java.lang.String newAltitude)
/=================================================================================================*/
  public void setAltitudeString(java.lang.String new_Altitude) {
    java.lang.String  old_Altitude = this.altitude_string;
    this.altitude_string = new_Altitude;
    propertyChangeListeners.firePropertyChange("altitude_string", old_Altitude, new_Altitude);
   }
  public java.lang.String getAltitudeString() {
    return altitude_string;
  }
/*================================================================================================
/   setAltitudeString(java.lang.String newAltitude)
/=================================================================================================*/
  public void setAltitude(double new_Altitude) {
    double  old_Altitude = this.altitude;
    altitude = new_Altitude;
    propertyChangeListeners.firePropertyChange("altitude", Double.valueOf(old_Altitude), Double.valueOf(new_Altitude));
   }
  public double getAltitude() {
    return altitude;
  }
/*================================================================================================
/       setUTDateDate(String _string)
/=================================================================================================*/
  public void setUTDateDate(String new_UTDateDate) {
    String  old_UTDateDate = this.UTDateDate;
    this.UTDateDate = new_UTDateDate;
    propertyChangeListeners.firePropertyChange("UTDateDate", old_UTDateDate, new_UTDateDate);
  }
  public String getUTDateDate() {
    return UTDateDate;
  }
/*================================================================================================
/       setUTDateTime(String new_UTDateTime)
/=================================================================================================*/
  public void setUTDateTime(String new_UTDateTime) {
    String  old_UTDateTime = this.UTDateTime;
    this.UTDateTime = new_UTDateTime;
    propertyChangeListeners.firePropertyChange("UTDateTime", old_UTDateTime, new_UTDateTime);
  }
  public String getUTDateTime() {
    return UTDateTime;
  }
/*================================================================================================
/       setlocalDateDate(String _string)
/=================================================================================================*/
  public void setlocalDateDate(String new_localDateDate) {
    String  old_localDateDate = this.localDateDate;
    this.localDateDate = new_localDateDate;
    propertyChangeListeners.firePropertyChange("localDateDate", old_localDateDate, new_localDateDate);
  }
  public String getlocalDateDate() {
    return localDateDate;
  } 
/*================================================================================================
/       setlocalDateTime(String new_localDateTime)
/=================================================================================================*/
  public void setlocalDateTime(String new_localDateTime) {
    String  old_localDateTime = this.localDateTime;
    this.localDateTime = new_localDateTime;
    propertyChangeListeners.firePropertyChange("localDateTime", old_localDateTime, new_localDateTime);
  }
  public String getlocalDateTime() {
    return localDateTime;
  } 
 //    OBSERVATORY SITE RELATED PARAMETERS
/*================================================================================================
/       setobsname(String obsname_string)
/=================================================================================================*/
  public void setObsname(String new_obsname) {
    String  old_obsname = this.obsname;
    this.obsname = new_obsname;
    propertyChangeListeners.firePropertyChange("obsname", old_obsname, new_obsname);
  }
  public String getObsname() {
    return obsname;
  }
/*================================================================================================
/       setLongitude(String longitude_string)
/=================================================================================================*/
  public void setLongitude(String new_longitude_string) {
    String  old_longitude_string = this.longitude_string;
    this.longitude_string = new_longitude_string;
    propertyChangeListeners.firePropertyChange("longitude_string", old_longitude_string, new_longitude_string);
  }
  public String getLongitude() {
    return longitude_string;
  }
/*================================================================================================
/       setLatitude(String new_latitude_string)
/=================================================================================================*/
  public void setLatitude(String new_latitude_string) {
    String  old_latitude_string = this.latitude_string;
    this.latitude_string = new_latitude_string;
    propertyChangeListeners.firePropertyChange("latitude_string", old_latitude_string, new_latitude_string);
  }
  public String getLatitude() {
    return latitude_string;
  }
/*================================================================================================
/       setElevsea_string(String elevsea__string)
/=================================================================================================*/
  public void setElevsea(String new_elevsea_string) {
    String  old_elevsea_string = this.elevsea_string;
    this.elevsea_string = new_elevsea_string;
    propertyChangeListeners.firePropertyChange("elevsea_string", old_elevsea_string, new_elevsea_string);
  }
  public String getElevsea_string() {
    return elevsea_string;
  }/*================================================================================================
/       setelevhoriz_string(String _string)
/=================================================================================================*/
  public void setElevhoriz(String new_elevhoriz_string) {
    String  old_elevhoriz_string = this.elevhoriz_string;
    this.elevhoriz_string = new_elevhoriz_string;
    propertyChangeListeners.firePropertyChange("elevhorizon_string", old_elevhoriz_string, new_elevhoriz_string);
  }
  public String getElevhoriz() {
    return elevhoriz_string;
  }
/*================================================================================================
/       setStdz(String new_stdz_string)
/=================================================================================================*/
  public void setStdz(String new_stdz_string) {
    String  old_stdz_string = this.stdz_string;
    this.stdz_string = new_stdz_string;
    propertyChangeListeners.firePropertyChange("stdz_string", old_stdz_string, new_stdz_string);
  }
  public String getStdz() {
    return stdz_string;
  }
/*================================================================================================
/       setUse_dst(String new_use_dst_string)
/=================================================================================================*/
  public void setUse_dst(String new_use_dst_string) {
    String  old_use_dst_string = this.use_dst_string;
    this.use_dst_string = new_use_dst_string;
    propertyChangeListeners.firePropertyChange("use_dst_string", old_use_dst_string, new_use_dst_string);
  }
  public String getUse_dst() {
    return use_dst_string;
  }
/*================================================================================================
/       setZonename(String zonename_string)
/=================================================================================================*/
  public void setZonename(String new_zonename_string) {
    String  old_zonename_string = this.zonename;
    this.zonename = new_zonename_string;
    propertyChangeListeners.firePropertyChange("zonename_string", old_zonename_string, new_zonename_string);
  }
  public String getZonename() {
    return zonename;
  }
/*================================================================================================
/       setObjectName(String new_objectname_string)
/=================================================================================================*/
  public void setObjectName(String new_objectname_string) {
    String  old_objectname_string = this.objectname;
    this.objectname = new_objectname_string;
    propertyChangeListeners.firePropertyChange("objectname_string", old_objectname_string, new_objectname_string);
  }
  public String getObjectName() {
    return objectname;
  }
/*=============================================================================================
/     Property Change Listener for the Ephemeris_propertyChange
/=============================================================================================*/
  private void Ephemeris_propertyChange(PropertyChangeEvent e)  {
     java.lang.String propertyName = e.getPropertyName();
/*=============================================================================================
/
/=============================================================================================*/
/*    if(propertyName == "jd_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "moonobj_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "baryjd_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "baryvcor_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "barytcor_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "constellation"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "planetwarning"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "moonra_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "moondec_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "altmoon_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "azmoon_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "sunra_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "sundec_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "altsun_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "sunaz_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "ztwilight_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "moonphasedescription"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "siderialobj_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "ha_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "airmass_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "parallactic_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "altparallactic_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "moonlight_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "moonillum_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "ra_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
     if(propertyName == "dec_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "equinox_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "azimuth_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "altitude_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "UTDateDate"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "UTDateTime"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "localDateDate"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "localDateTime"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "obsname"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "longitude_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "latitude_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "elevsea_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "elevhorizon_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "stdz_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "use_dst_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "zonename_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "nearestBrightStar"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "objectname_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
 */
  }
}
