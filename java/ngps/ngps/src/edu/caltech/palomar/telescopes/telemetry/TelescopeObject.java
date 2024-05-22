package edu.caltech.palomar.telescopes.telemetry;   
//=== File Prolog =============================================================
//	This code was developed by California Institute of Technology
//      Caltech Optical Observatories, Palomar Observatory
//
//--- Contents ----------------------------------------------------------------
//	 TelescopeObject- This is the object model for a basic telescope
// 
//--- Description -------------------------------------------------------------
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
//
//      July 14, 2010 Jennifer Milburn
//        Original Implementation
//
// Change log:  changed minimum move rate to 1 arcsec per second to slow things down for WIRC guiding
// Jennifer Milburn May 26,2011
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
import edu.caltech.palomar.telescopes.P200.*;
import java.beans.*;
import java.util.Date;
import jsky.science.Coordinates;
import jsky.science.CoordinatesOffset;
import jsky.coords.HMS;
import jsky.coords.DMS;
import edu.dartmouth.jskycalc.JSkyCalcModel;
import java.lang.Math;
/*================================================================================================
/       TelescopeObject Class Declaration
/=================================================================================================*/
public class TelescopeObject {

  transient private PropertyChangeSupport propertyChangeListeners = new PropertyChangeSupport(this);
  // String representations of the telescope orientation and setup parameters
  private String right_ascension;
  private String declination;
  private String temp_declination       = new String();
  private String right_ascension_last_GO;
  private String declination_last_GO;
  private String equinox_string;
  private String hourangle_string;
  private String airmass_string;
  private String azimuth;
  private String sidereal;
  private String universal_time_date;
  private String universal_time;
  private String local_siderial_time;
  private String right_ascension_offset;
  private String declination_offset;
  private String right_ascension_track_rate;
  private String declination_track_rate;
  private String telescope_focus;
  private String telescope_tube_length;
  private String cass_ring_angle;
  private String telescope_ID;
  private String objectname;
  private java.lang.String objectA        = new java.lang.String();
  private java.lang.String objectT        = new java.lang.String();
  private java.lang.String UTCSTART       = new java.lang.String();
  private java.lang.String UTCEND         = new java.lang.String();
  private java.lang.String DATE_OBS       = new java.lang.String();
  private java.lang.String DATE           = new java.lang.String();
// decimal representation of the telescope orientation and setup parameters
  private double RA;
  private double DEC;
  private double EQUINOX;
  private double HA;
  private double AIRMASS;
  private double LST;
  private double RA_OFFSET;
  private double DEC_OFFSET;
  private double RA_TRACK_RATE;
  private double DEC_TRACK_RATE;
  private double RA_GUIDE_RATE;
  private double DEC_GUIDE_RATE;
  private int    RA_SET_RATE;
  private int    DEC_SET_RATE;
  private double RAMoveRate;
  private double DecMoveRate;
  private double RAMoveRateManual;
  private double DecMoveRateManual;
  private boolean manualMoveState;
  private double FOCUS;
  private double TUBE_LENGTH;
  private double CASS_RING_ANGLE;
  private double ZENITH_ANGLE;
  private String zenith_angle;
  private double AZIMUTH;
  // status parameters
  private boolean connected;
  private boolean lock;
  private boolean updating;
  private boolean moving;
  private int progress;
  private int offset;
  private int tracking_rates_units;
  private double DOME_AZIMUTH;
  private String dome_azimuth;
  private double  WINDSCREEN_POSITION;
  private String  windscreen_position;
  private double  DOME_SHUTTERS;
  private String dome_shutters;
  private int  INSTRUMENT_POSITION;
  private String instrument_position;
  private boolean tracking;
  private java.lang.String INSTRUMENT = new java.lang.String();
  private java.lang.String PUMPS      = new java.lang.String();
  private java.lang.String caption    = new java.lang.String();
  /* The B1950 equinox**/
  public static final int B1950_EQUINOX = 1;
  public static final String B1950_EQUINOX_LABEL = "B1950";
  /*** The J2000 equinox**/
  public static final int J2000_EQUINOX = 2;
  public static final String J2000_EQUINOX_LABEL = "J2000";
  private Coordinates       myCoordinates       = new Coordinates();
  private CoordinatesOffset myCoordinatesOffset = new CoordinatesOffset();
//  Ephemeris related variables
  public JSkyCalcModel      myJSkyCalcModel = new JSkyCalcModel();
// Each line contains parameters of a single site, and consists of 9
// comma-separated parameters, which must be in this order:
// 1) Name, in quotation marks; keep it succinct; must be unique.
// 2) Longitude, expressed in DECIMAL HOURS, POSITIVE WESTWARD (dammit!)
// 3) Latitude, in decimal degrees, positive northward
// 4) Offset of standard time from UT, in decimal hours with decimal included
//    positive -> west of Greenwich, negative -> east
// 5) Integer code for the convention used for daylight savings time;
//    0 -> off, 1 -> USA (keyed to year, includes 1987 revisions),
//    2 -> a Spanish convention (I think), -1 -> Chilean (negative sign for
//    southern hemisphere logic), -2 -> Australian.
// 6) Name of the time zone in quotation marks
// 7) A 1- or 2-character code for the timezone (e.g. "C" for central)
// 8) The elevation of the observatory above sea level, in meters
// 9) An estimate of the elevation of the obs. around the terrain that forms its
//    horizon, used for adjusting rise-set times.  THIS CANNOT BE NEGATIVE
//    (negative values cause a square-root domain error and are ignored).
// The following are codes for the direction the telescope is moving and whether it is moving
    public static int NOT_MOVING = 0;
    public static int MOVING_N = 1;
    public static int MOVING_E = 2;
    public static int MOVING_S = 3;
    public static int MOVING_W = 4;
    public static int MOVING_NE = 10;
    public static int MOVING_SE = 11;
    public static int MOVING_SW = 12;
    public static int MOVING_NW = 13;
    public static int MOVING_UNKNOWN = -1;

    private int          telescopeMotion;
    private int          focusMotion;
    public static int    FOCUS_NOT_MOVING  = 0;
    public static int    FOCUS_MOVING_UP   = 1;
    public static int    FOCUS_MOVING_DOWN = 2;

    public static double MAX_RA_MOVE_RATE      = 45.0;
    public static double MAX_DEC_MOVE_RATE     = 45.0;
    public  double       MIN_RA_MOVE_RATE      = 0.7;
    public  double       MIN_DEC_MOVE_RATE     = 0.7;
    public  double TELESCOPE_SETTLE_TIME = 2.0;  // this is the default value that can be programmatically rest
    public  double FOCUS_MOVE_RATE       = 0.1;  // this is the default value that can be programmatically rest
    private double totalElapsedTime;
    private double percentCompleted;
    public String     statusMessage = new String();
    public static double MAX_OFFSET         = 6000.0;
    public static double MIN_RA_OFFSET      = -6000.0;
    public static double MAX_RA_OFFSET      = 6000.0;
    public static double MIN_DEC_OFFSET     = -6000.0;
    public static double MAX_DEC_OFFSET     = 6000.0;
    public static int    SEC_ARCSEC_YEAR    = 0;
    public static int    ARCSEC_HOUR        = 1;
    public static int    SEC__ARCSEC_HOUR   = 2;
    private boolean      abortArmed         = false;
    public int           MOUNT;
    public static int    CASSEGRAIN = 100;
    public static int    PRIME      = 200;
    public static int    COUDE      = 300;
    public static int    MOTION_STOPPED         = 0;
    public static int    MOTION_SLEWING         = 1;
    public static int    MOTION_OFFSETTING      = 2;
    public static int    MOTION_TRACKING_STABLY = 3;
    public static int    MOTION_SETTLING        = -1;
    public static int    MOTION_UNKNOWN         = -2;
    public int           motion_status          = MOTION_STOPPED;
    public java.lang.String motion_status_string = new java.lang.String();
    public static int    LOW_LAMP_PORT            = 1;
    public static int    HIGH_LAMP_PORT           = 2;
    public static int    ARC_LAMP_PORT            = 3;
    public static int    LWIR_CAMERA_PORT         = 7;
    public static int    LWIR_CAMERA_SHUTTER_PORT = 8;
    public static int    LAMP_OFF                 = 0;
    public static int    LAMP_ON                  = 1;
    public static int    LAMP_STATUS              = 2;
    private boolean      low_lamp;
    private boolean      high_lamp;
    private boolean      arc_lamp;   
    private long         htm_index_level7;
    private long         htm_index_level20;
/*================================================================================================
/       TelescopeObject parameterless constructor
/=================================================================================================*/
  public TelescopeObject() {
    jbInit();
  }
/*================================================================================================
/        jbInit() initialization method
/=================================================================================================*/
  private void jbInit(){
     setTelescopeMotionDirection(NOT_MOVING);
     setFocusMotionDirection(FOCUS_NOT_MOVING);
     setUTCSTART("000 00:00:00.0");
     setSidereal("00:00:00.0");
     setRightAscension("00:00:00.00");
     setDeclination("+00:00:00.0");
     setHourAngle("E00:00:00.0");
     setTelescopeFocus("0.0");
     setAirMass("1.000                                                                                                                                                  ");
     setAzimuthString("000.00");
     setZenithAngleString("00.00");
     setDomeAzimuthString("000.0");
     setWindscreenPositionString("00.00");
     setTelescopeFocus("00.00");
     setRightAscensionOffset("000.0");
     setDeclinationOffset("00.0");
     setCassRingAngle("0.00                                        ");
     setEquinox("2000");
     setMoving(false);
     setUpdating(false);
     setTracking(false);
     setRightAscensionTrackRate("0.0");
     setDeclinationTrackRate("0.0");
     setTelescopeTubeLength("00.00");
     setTelescopeMotionDirection(MOVING_UNKNOWN);
     setRightAscensionTrackRate("0.0");
     setDeclinationTrackRate("0.0");
     setObjectName("");
     this.setTrackingRatesUnits(this.ARCSEC_HOUR);
     setLowLamp(false);
     setHighLamp(false);
     setArcLamp(false);
   // UTC = 216 04:34:38.1, LST = 17:37:45.4
   // RA = 17:57:57.69, DEC = +04:43:11.8, HA = E00:20:45.8
   // air mass =  1.143                                                                                                                                                  
// REQSTAT Example:
//  telescope ID = 200, focus = 36.71 mm, tube length = 22.11 mm
//  offset RA =     137.3 arcsec, DEC =      95.8 arcsec
//  rate RA =       0.0 arcsec/hr, DEC =       0.0 arcsec/hr
//  Cass ring angle =  49.35                                        
// WEATHER Example
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
  }
/*================================================================================================
/        jbInit() initialization method
/=================================================================================================*/
  public void intializeSimulatorValues(){
     setUTCSTART("216 04:34:38.1");
     setSidereal("17:37:45.4");
     setRightAscension("17:57:57.69");
     setDeclination("+04:43:11.8");
     setRightAscensionLastGO("17:57:57.69");
     setDeclinationLastGO("+04:43:11.8");
     setHourAngle("E00:20:45.8");
     setTelescopeFocus("0.0");
     setAirMass("1.143 ");
     setAzimuthString("169.37");
     setZenithAngleString("29.00");
     setDomeAzimuthString("164.9");
     setWindscreenPositionString("09.17");
     setTelescopeFocus("36.71");
     setRightAscensionOffset("137.3");
     setDeclinationOffset("95.8");
     setCassRingAngle("0.00");
     setEquinox("2000");
     setMoving(false);
     setUpdating(false);
     setTracking(false);    
     setRightAscensionTrackRate("0.0");
     setDeclinationTrackRate("0.0");
     setTelescopeTubeLength("22.11");
  }
/*================================================================================================
/        initialJSkyCalcModel()
/=================================================================================================*/
  public void initialJSkyCalcModel(){
      myJSkyCalcModel.setObjectName("Hale 200' Telescope Position");
      myJSkyCalcModel.setTelescopePosition(right_ascension,declination,equinox_string);
      myJSkyCalcModel.SetToNow();;
  } 
/*================================================================================================
/         getJSkyCalcModel()
/=================================================================================================*/
  public JSkyCalcModel getJSkyCalcModel(){
    return myJSkyCalcModel; 
}
/*================================================================================================
/       setHTMIndex_Level7(long new_htm_index_level7)
/=================================================================================================*/
   public void setHTMIndex_Level7(long new_htm_index_level7) {
    long  old_htm_index_level7 = this.htm_index_level7;
    this.htm_index_level7 = new_htm_index_level7;
    propertyChangeListeners.firePropertyChange("htm_index_level7", Long.valueOf(old_htm_index_level7),Long.valueOf(new_htm_index_level7));
  }
  public long getHTMIndex_Level7() {
    return htm_index_level7;
  }
/*================================================================================================
/       setHTMIndex_Level7(long new_htm_index_level7)
/=================================================================================================*/
   public void setHTMIndex_Level20(long new_htm_index_level20) {
    long  old_htm_index_level20 = this.htm_index_level20;
    this.htm_index_level20 = new_htm_index_level20;
    propertyChangeListeners.firePropertyChange("htm_index_level20", Long.valueOf(old_htm_index_level20), Long.valueOf(new_htm_index_level20));
  }
  public long getHTMIndex_Level20() {
    return htm_index_level20;
  }
/*=============================================================================================
/    setAbortArmed(boolean newAbortArmed)
/=============================================================================================*/
  public void setAbortArmed(boolean newAbortArmed){
    boolean  oldAbortArmed = this.abortArmed;
    this.abortArmed = newAbortArmed;
    propertyChangeListeners.firePropertyChange("abortArmed",Boolean.valueOf(oldAbortArmed),Boolean.valueOf(abortArmed));
  }
  public boolean getAbortArmed(){
      return abortArmed;
  }
/*================================================================================================
/          setCoordinates(double newRA,double newDEC)
/=================================================================================================*/
 public void setCoordinates(double newRA,double newDEC){
    myCoordinates.setRa(newRA);
    myCoordinates.setDec(newDEC);
 }
 public Coordinates getCoordinates(){
    return myCoordinates;
 }
 public void updateCoordinates(){
    setRightAscension(myCoordinates.raToString());
    setDeclination(myCoordinates.decToString());
 }
 /*================================================================================================
/          setCoordinates(double newRA,double newDEC)
/=================================================================================================*/
 public void setCoordinatesOffset(double newRAOffset,double newDECOffset){
    myCoordinatesOffset.set(newRAOffset, newDECOffset);
 }
 public CoordinatesOffset getCoordinatesOffset(){
    return myCoordinatesOffset;
 }
/*================================================================================================
/          updateOffsetCoordinates()
/=================================================================================================*/
 public void updateOffsetCoordinates(){
     if(declination.contains(" +")){
          temp_declination = declination.substring(2,declination.length());
     }
     if(!declination.contains(" +")){
          temp_declination = declination;
     }
     myCoordinates.setSeparatorStyle(Coordinates.COLON_SEPARATOR_STYLE);
     myCoordinates = Coordinates.valueOf(right_ascension, temp_declination);
     myCoordinatesOffset.set(-(Double.valueOf(right_ascension_offset).doubleValue())/3600, -(Double.valueOf(declination_offset).doubleValue())/3600);
     myCoordinates.translate(myCoordinatesOffset); 
     setRightAscensionLastGO(myCoordinates.raToString());
     setDeclinationLastGO(myCoordinates.decToString());     
 }
/*=============================================================================================
/             setMotionStatus(int new_motion_status)
/=============================================================================================*/
public synchronized void setMotionStatus(int new_motion_status){
    int  old_motion_status = this.motion_status;
    this.motion_status = new_motion_status;
    propertyChangeListeners.firePropertyChange("motion_status", Integer.valueOf(old_motion_status),Integer.valueOf(new_motion_status));
}
  public int getMotionStatus() {
    return motion_status;
  }
/*=============================================================================================
/           setMotionStatusString(java.lang.String new_motion_status_string)
/=============================================================================================*/
public synchronized void setMotionStatusString(java.lang.String new_motion_status_string){
    java.lang.String  old_motion_status_string = this.motion_status_string;
    this.motion_status_string = new_motion_status_string;
    propertyChangeListeners.firePropertyChange("motion_status_string", old_motion_status_string, new_motion_status_string);
}
  public java.lang.String getMotionStatusString() {
    return motion_status_string;
  }  
/*=============================================================================================
/                 setTelescopeMotionDirection(int newTelescopeMotion)
/=============================================================================================*/
public synchronized void setTelescopeMotionDirection(int newTelescopeMotion){
    int  oldTelescopeMotion = this.telescopeMotion;
    this.telescopeMotion = newTelescopeMotion;
    propertyChangeListeners.firePropertyChange("telescopeMotion", Integer.valueOf(oldTelescopeMotion), Integer.valueOf(newTelescopeMotion));
}
  public int getTelescopeMotionDirection() {
    return telescopeMotion;
  }
/*=============================================================================================
/                 setFocusMotionDirection(int newFocusMotion)
/=============================================================================================*/
public synchronized void setFocusMotionDirection(int newFocusMotion){
    int  oldFocusMotion = this.focusMotion;
    this.focusMotion = newFocusMotion;
    propertyChangeListeners.firePropertyChange("focusMotion", Integer.valueOf(oldFocusMotion), Integer.valueOf(newFocusMotion));
}
  public int getFocusMotionDirection() {
    return focusMotion;
  }
/*================================================================================================
/      setRightAscensionLastGO
/=================================================================================================*/
  public void setRightAscensionLastGO(String right_ascension_last_GO) {
     String  oldright_ascension_last_GO = this.right_ascension_last_GO;
    this.right_ascension_last_GO = right_ascension_last_GO;
    propertyChangeListeners.firePropertyChange("right_ascension_last_GO", oldright_ascension_last_GO, right_ascension_last_GO);
  }
  public String getRightAscensionLastGO() {
    return right_ascension_last_GO;
  }
/*================================================================================================
/      setDeclinationLastGO
/=================================================================================================*/
  public void setDeclinationLastGO(String declination_last_GO) {
    String  oldDeclination_last_GO = this.declination_last_GO;
    this.declination_last_GO = declination_last_GO;
    propertyChangeListeners.firePropertyChange("declination_last_GO", oldDeclination_last_GO, declination_last_GO);
  }
  public String getDeclinationLastGO() {
    return declination_last_GO;
  }
/*================================================================================================
/          STRING REPRESENTATIONS OF THE TELESCOPE ORIENTATION
/=================================================================================================*/
/*================================================================================================
/      setRightAscension
/=================================================================================================*/
  public void setRightAscension(String right_ascension) {
     HMS myHMS = new HMS(right_ascension);
//     double newRA = Coordinates.valueOf(right_ascension, declination).getRa();
     setRA(myHMS.getVal());
     String  oldright_ascension = this.right_ascension;
    this.right_ascension = right_ascension;
    propertyChangeListeners.firePropertyChange("right_ascension", oldright_ascension, right_ascension);
  }
  public String getRightAscension() {
    return right_ascension;
  }
/*================================================================================================
/      setDeclination
/=================================================================================================*/
  public void setDeclination(String new_declination) {
      DMS myDMS = new DMS(new_declination);
 //     double newDEC = Coordinates.valueOf(right_ascension, declination).getDec();
    setDEC(myDMS.getVal());
    String  oldDeclination = this.declination;
    this.declination = new_declination;
    propertyChangeListeners.firePropertyChange("declination", oldDeclination, new_declination);
  }
  public String getDeclination() {
    return declination;
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
/       setHourAngle(String hourAngle)
/=================================================================================================*/
  public void setHourAngle(String hourangle_string) {
//     HMS myHMS = new HMS(hourangle_string);
//     setHA(myHMS.getVal());
    String  oldhourangle_string = this.hourangle_string;
    this.hourangle_string = hourangle_string;
    propertyChangeListeners.firePropertyChange("hourangle_string", oldhourangle_string, hourangle_string);
  }
  public String getHourAngle() {
    return hourangle_string;
  }
/*================================================================================================
/       setAirMass(String airmass_string)
/=================================================================================================*/
  public void setAirMass(String airmass_string) {
   try{
         setAIRMASS((Double.valueOf(airmass_string)).doubleValue());
      }catch(Exception e){
          System.out.println("Error Parsing the airmass to a number");
      }
    String  oldairmass_string = this.airmass_string;
    this.airmass_string = airmass_string;
    propertyChangeListeners.firePropertyChange("airmass_string", oldairmass_string, airmass_string);
  }
  public String getAirMass() {
    return airmass_string;
  }
/*================================================================================================
/   setSidereal(java.lang.String newsidereal)
/=================================================================================================*/
      public java.lang.String getSidereal() {
         return sidereal;
      }
      public void setSidereal(java.lang.String newsidereal) {
         java.lang.String  oldsidereal = this.sidereal;
         this.sidereal = newsidereal;
         propertyChangeListeners.firePropertyChange("sidereal", oldsidereal, newsidereal);
      }
//  UTCSTART 	UTC of exposure start any [hh:mm:ss.s] [str] [] ['09:30:01.00'] absolute :
//  UTCEND 	UTC of exposure end any [hh:mm:ss.s] [str] [] ['09:30:01.00'] absolute :
/*================================================================================================
/   setUTCSTART(java.lang.String newUTCSTART)
/=================================================================================================*/
    public java.lang.String getUTCSTART() {
      return UTCSTART;
    }
    public void setUTCSTART(java.lang.String newUTCSTART) {
      java.lang.String  oldUTCSTART = this.UTCSTART;
      this.UTCSTART = newUTCSTART;
      propertyChangeListeners.firePropertyChange("UTCSTART", oldUTCSTART, newUTCSTART);
    }
/*================================================================================================
/   setUTCEND(java.lang.String newUTCEND)
/=================================================================================================*/
    public java.lang.String getUTCEND() {
          return UTCEND;
    }
    public void setUTCEND(java.lang.String newUTCEND) {
       java.lang.String  oldUTCEND = this.UTCEND;
       this.UTCEND = newUTCEND;
       propertyChangeListeners.firePropertyChange("UTCEND", oldUTCEND, newUTCEND);
    }
/*================================================================================================
/   setDATE_OBS(java.lang.String newDATE_OBS)
/=================================================================================================*/
   public java.lang.String getDATE_OBS() {
      return DATE_OBS;
   }
   public void setDATE_OBS(java.lang.String newDATE_OBS) {
      java.lang.String  oldDATE_OBS = this.DATE_OBS;
      this.DATE_OBS = newDATE_OBS;
      propertyChangeListeners.firePropertyChange("DATE_OBS", oldDATE_OBS, newDATE_OBS);
   }
/*================================================================================================
/   setDATE(java.lang.String newDATE)
/=================================================================================================*/
      public java.lang.String getDATE() {
         return DATE;
      }
      public void setDATE(java.lang.String newDATE) {
         java.lang.String  oldDATE = this.DATE;
         this.DATE = newDATE;
         propertyChangeListeners.firePropertyChange("DATE", oldDATE, newDATE);
      }
//  MISSING THE DATE AND TIME RELATED METHODS

/*================================================================================================
/      setRightAscensionOffset
/=================================================================================================*/
  public void setRightAscensionOffset(String right_ascension_offset) {
    setRA_OFFSET((Double.valueOf(right_ascension_offset)).doubleValue());
    String  oldright_ascension_offset = this.right_ascension_offset;
    this.right_ascension_offset = right_ascension_offset;
    propertyChangeListeners.firePropertyChange("right_ascension_offset", oldright_ascension_offset, right_ascension_offset);
  }
  public String getRightAscensionOffset() {
    return right_ascension_offset;
  }
/*================================================================================================
/      setDeclination
/=================================================================================================*/
  public void setDeclinationOffset(String declination_offset) {
    setDEC_OFFSET((Double.valueOf(declination_offset)).doubleValue());
    String  olddeclination_offset = this.declination_offset;
    this.declination_offset = declination_offset;
    propertyChangeListeners.firePropertyChange("declination_offset", olddeclination_offset, declination_offset);
  }
  public String getDeclinationOffset() {
    return declination_offset;
  }
/*================================================================================================
/      setRightAscensionTrackRate
/=================================================================================================*/
  public void setRightAscensionTrackRate(String right_ascension_track_rate) {
    setRA_TRACK_RATE((Double.valueOf(right_ascension_track_rate)).doubleValue());
    String  oldright_ascension_track_rate = this.right_ascension_track_rate;
    this.right_ascension_track_rate = right_ascension_track_rate;
    propertyChangeListeners.firePropertyChange("right_ascension_track_rate", oldright_ascension_track_rate, right_ascension_track_rate);
  }
  public String getRightAscensionTrackRate() {
    return right_ascension_track_rate;
  }
/*================================================================================================
/      setDeclinationTrackRate
/=================================================================================================*/
  public void setDeclinationTrackRate(String declination_track_rate) {
    setDEC_TRACK_RATE((Double.valueOf(declination_track_rate)).doubleValue());
    String  olddeclination_track_rate = this.declination_track_rate;
    this.declination_track_rate = declination_track_rate;
    propertyChangeListeners.firePropertyChange("declination_track_rate", olddeclination_track_rate, declination_track_rate);
  }
  public String getDeclinationTrackRate() {
    return declination_track_rate;
  }
/*================================================================================================
/      setTelescopeFocus(String telescope_focus)
/=================================================================================================*/
  public void setTelescopeFocus(String telescope_focus) {
    setFOCUS((Double.valueOf(telescope_focus)).doubleValue());
    String  oldtelescope_focus = this.telescope_focus;
    this.telescope_focus = telescope_focus;
    propertyChangeListeners.firePropertyChange("telescope_focus", oldtelescope_focus, telescope_focus);
  }
  public String getTelescopeFocus() {
    return telescope_focus;
  }
/*================================================================================================
/      setTelescopeTubeLength(String telescope_tube_length)
/=================================================================================================*/
  public void setTelescopeTubeLength(String telescope_tube_length) {
    setTUBE_LENGTH(( Double.valueOf(telescope_tube_length)).doubleValue());
    String  oldtelescope_tube_length = this.telescope_tube_length;
    this.telescope_tube_length = telescope_tube_length;
    propertyChangeListeners.firePropertyChange("telescope_tube_length", oldtelescope_tube_length, telescope_tube_length);
  }
  public String getTelescopeTubeLength() {
    return telescope_tube_length;
  }
/*================================================================================================
/      setCassRingAngle(String cass_ring_angle)
/=================================================================================================*/
  public void setCassRingAngle(String cass_ring_angle) {
    setCASS_RING_ANGLE((Double.valueOf(cass_ring_angle)).doubleValue());
    String  oldcass_ring_angle = this.cass_ring_angle;
    this.cass_ring_angle = cass_ring_angle;
    propertyChangeListeners.firePropertyChange("cass_ring_angle", oldcass_ring_angle, cass_ring_angle);
  }
  public String getCassRingAngle() {
    return cass_ring_angle;
  }
/*================================================================================================
/      setCassRingAngle(String cass_ring_angle)
/=================================================================================================*/
  public void setTelescopeID(String telescope_ID) {
    String  oldtelescope_ID = this.telescope_ID;
    this.telescope_ID = telescope_ID;
    propertyChangeListeners.firePropertyChange("telescope_ID", oldtelescope_ID, telescope_ID);
  }
  public String getTelescopeID() {
    return cass_ring_angle;
  }
/*================================================================================================
/          DECIMAL REPRESENTATIONS OF THE TELESCOPE ORIENTATION STRINGS
/=================================================================================================*/
/*================================================================================================
/      setRA(double newRA)
/=================================================================================================*/
  public void setRA(double newRA) {
    double  oldRA = this.RA;
    this.RA = newRA;
    myCoordinates.setRa(newRA);
   propertyChangeListeners.firePropertyChange("RA", Double.valueOf(oldRA), Double.valueOf(newRA));
  }
  public double getRA() {
    return RA;
  }
/*================================================================================================
/      setDEC(double newDEC)
/=================================================================================================*/
  public void setDEC(double newDEC) {
    double  oldDEC = this.DEC;
    this.DEC = newDEC;
    myCoordinates.setDec(newDEC);
    propertyChangeListeners.firePropertyChange("DEC", Double.valueOf(oldDEC), Double.valueOf(newDEC));
  }
  public double getDEC() {
    return DEC;
  }
/*================================================================================================
/      setEQUINOX(double newEQUINOX)
/=================================================================================================*/
  public void setEQUINOX(double newEQUINOX) {
    double  oldEQUINOX = this.EQUINOX;
    this.EQUINOX = newEQUINOX;
    propertyChangeListeners.firePropertyChange("EQUINOX", Double.valueOf(oldEQUINOX), Double.valueOf(newEQUINOX));
  }
  public double getEQUINOX() {
    return EQUINOX;
  }
/*================================================================================================
/      setHA(double newHA)
/=================================================================================================*/
  public void setHA(double newHA) {
    double  oldHA = this.HA;
    this.HA = newHA;
    propertyChangeListeners.firePropertyChange("HA", Double.valueOf(oldHA), Double.valueOf(newHA));
  }
  public double getHA() {
    return HA;
  }
/*================================================================================================
/      setAIRMASS(double newAIRMASS)
/=================================================================================================*/
  public void setAIRMASS(double newAIRMASS) {
    double  oldAIRMASS = this.AIRMASS;
    this.AIRMASS = newAIRMASS;
    propertyChangeListeners.firePropertyChange("AIRMASS", Double.valueOf(oldAIRMASS), Double.valueOf(newAIRMASS));
  }
  public double getAIRMASS() {
    return AIRMASS;
  }
/*================================================================================================
/      setRA_OFFSET(double newRA_OFFSET)
/=================================================================================================*/
  public void setRA_OFFSET(double newRA_OFFSET) {
    double  oldRA_OFFSET = this.RA_OFFSET;
    this.RA_OFFSET = newRA_OFFSET;
    propertyChangeListeners.firePropertyChange("RA_OFFSET",Double.valueOf(oldRA_OFFSET),Double.valueOf(newRA_OFFSET));
  }
  public double getRA_OFFSET() {
    return RA_OFFSET;
  }
/*================================================================================================
/      setDEC_OFFSET(double newDEC_OFFSET)
/=================================================================================================*/
  public void setDEC_OFFSET(double newDEC_OFFSET) {
    double  oldDEC_OFFSET = this.DEC_OFFSET;
    this.DEC_OFFSET = newDEC_OFFSET;
    propertyChangeListeners.firePropertyChange("DEC_OFFSET", Double.valueOf(oldDEC_OFFSET),Double.valueOf(newDEC_OFFSET));
  }
  public double getDEC_OFFSET() {
    return DEC_OFFSET;
  }
/*================================================================================================
/      setRA_TRACK_RATE(double newRA_TRACK_RATE)
/=================================================================================================*/
  public void setRA_TRACK_RATE(double newRA_TRACK_RATE) {
    double  oldRA_TRACK_RATE = this.RA_TRACK_RATE;
    this.RA_TRACK_RATE = newRA_TRACK_RATE;
    propertyChangeListeners.firePropertyChange("RA_TRACK_RATE", Double.valueOf(oldRA_TRACK_RATE),Double.valueOf(newRA_TRACK_RATE));
  }
  public double getRA_TRACK_RATE() {
    return RA_TRACK_RATE;
  }
/*================================================================================================
/      setDEC_TRACK_RATE(double newDEC_TRACK_RATE)
/=================================================================================================*/
  public void setDEC_TRACK_RATE(double newDEC_TRACK_RATE) {
    double  oldDEC_TRACK_RATE = this.DEC_TRACK_RATE;
    this.DEC_TRACK_RATE = newDEC_TRACK_RATE;
    propertyChangeListeners.firePropertyChange("DEC_TRACK_RATE", Double.valueOf(oldDEC_TRACK_RATE),Double.valueOf(newDEC_TRACK_RATE));
  }
  public double getDEC_TRACK_RATE() {
    return DEC_TRACK_RATE;
  }
/*================================================================================================
/      setRA_GUIDE_RATE(double newRA_GUIDE_RATE)
/=================================================================================================*/
  public void setRA_GUIDE_RATE(double newRA_GUIDE_RATE) {
    double  oldRA_GUIDE_RATE = this.RA_GUIDE_RATE;
    this.RA_GUIDE_RATE = newRA_GUIDE_RATE;
    propertyChangeListeners.firePropertyChange("RA_GUIDE_RATE", Double.valueOf(oldRA_GUIDE_RATE),Double.valueOf(newRA_GUIDE_RATE));
  }
  public double getRA_GUIDE_RATE() {
    return RA_GUIDE_RATE;
  }
/*================================================================================================
/      setDEC_GUIDE_RATE(double newDEC_GUIDE_RATE)
/=================================================================================================*/
  public void setDEC_GUIDE_RATE(int newDEC_GUIDE_RATE) {
    double oldDEC_GUIDE_RATE = this.DEC_GUIDE_RATE;
    this.DEC_GUIDE_RATE = newDEC_GUIDE_RATE;
    propertyChangeListeners.firePropertyChange("DEC_GUIDE_RATE", Double.valueOf(oldDEC_GUIDE_RATE), Double.valueOf(newDEC_GUIDE_RATE));
  }
  public double getDEC_GUIDE_RATE() {
    return DEC_GUIDE_RATE;
  }
/*================================================================================================
/      setRA_SET_RATE(int newRA_SET_RATE)
/=================================================================================================*/
  public void setRA_SET_RATE(int newRA_SET_RATE) {
    int  oldRA_SET_RATE = this.RA_SET_RATE;
    this.RA_SET_RATE = newRA_SET_RATE;
    propertyChangeListeners.firePropertyChange("RA_SET_RATE",Integer.valueOf(oldRA_SET_RATE),Integer.valueOf(newRA_SET_RATE));
  }
  public int getRA_SET_RATE() {
    return RA_SET_RATE;
  }
/*================================================================================================
/      setMinRAMoveRate(int newRAMoveRate)
/=================================================================================================*/
  public void setMinRAMoveRate(double newMinRAMoveRate) {
    double  oldMinRAMoveRate = this.MIN_RA_MOVE_RATE;
    this.MIN_RA_MOVE_RATE = newMinRAMoveRate;
    propertyChangeListeners.firePropertyChange("MinRAMoveRate", Double.valueOf(oldMinRAMoveRate), Double.valueOf(newMinRAMoveRate));
  }
  public double getMinRAMoveRate() {
    return MIN_RA_MOVE_RATE;
  }
/*================================================================================================
/      setMinRAMoveRate(int newRAMoveRate)
/=================================================================================================*/
  public void setMinDECMoveRate(double newMinDECMoveRate) {
    double  oldMinDECMoveRate = this.MIN_DEC_MOVE_RATE;
    this.MIN_DEC_MOVE_RATE = newMinDECMoveRate;
    propertyChangeListeners.firePropertyChange("MinDECMoveRate", Double.valueOf(oldMinDECMoveRate),Double.valueOf(newMinDECMoveRate));
  }
  public double getMinDECMoveRate() {
    return MIN_DEC_MOVE_RATE;
  }
/*================================================================================================
/      setRAMoveRate(int newRAMoveRate)
/=================================================================================================*/
  public void setRAMoveRate(double newRAMoveRate) {
    double  oldRAMoveRate = this.RAMoveRate;
    this.RAMoveRate = newRAMoveRate;
    propertyChangeListeners.firePropertyChange("RAMoveRate",Double.valueOf(oldRAMoveRate),Double.valueOf(newRAMoveRate));
  }
  public double getRAMoveRate() {
    return RAMoveRate;
  }
/*================================================================================================
/      setDecMoveRate(int newDecMoveRate)
/=================================================================================================*/
  public void setDecMoveRate(double newDecMoveRate) {
    double  oldDecMoveRate = this.DecMoveRate;
    this.DecMoveRate = newDecMoveRate;
    propertyChangeListeners.firePropertyChange("DecMoveRate", Double.valueOf(oldDecMoveRate), Double.valueOf(newDecMoveRate));
  }
  public double getDecMoveRate() {
    return DecMoveRate;
  }
/*================================================================================================
/      setRAMoveRateManual(int newRAMoveRateManual)
/=================================================================================================*/
  public void setRAMoveRateManual(double newRAMoveRateManual) {
    double  oldRAMoveRateManual = this.RAMoveRateManual;
    this.RAMoveRateManual = newRAMoveRateManual;
    propertyChangeListeners.firePropertyChange("RAMoveRateManual", Double.valueOf(oldRAMoveRateManual),Double.valueOf(newRAMoveRateManual));
  }
  public double getRAMoveRateManual() {
    return RAMoveRateManual;
  }
/*================================================================================================
/      setDecMoveRateManual(int newDecMoveRateManual)
/=================================================================================================*/
  public void setDecMoveRateManual(double newDecMoveRateManual) {
    double  oldDecMoveRateManual = this.DecMoveRateManual;
    this.DecMoveRateManual = newDecMoveRateManual;
    propertyChangeListeners.firePropertyChange("DecMoveRate",Double.valueOf(oldDecMoveRateManual), Double.valueOf(newDecMoveRateManual));
  }
  public double getDecMoveRateManual() {
    return DecMoveRateManual;
  }
/*================================================================================================
/      setManualMoveRateState(boolean newState)
/=================================================================================================*/
  public void setManualMoveRateState(boolean newState){
     manualMoveState = newState;
  }
  public boolean isManualMoveRate(){
      return manualMoveState;
  }
/*================================================================================================
/      setDEC_SET_RATE(int newDEC_SET_RATE)
/=================================================================================================*/
  public void setDEC_SET_RATE(int newDEC_SET_RATE) {
    int  oldDEC_SET_RATE = this.DEC_SET_RATE;
    this.DEC_SET_RATE = newDEC_SET_RATE;
    propertyChangeListeners.firePropertyChange("DEC_SET_RATE", Integer.valueOf(oldDEC_SET_RATE),Integer.valueOf(newDEC_SET_RATE));
  }
  public int getDEC_SET_RATE() {
    return DEC_SET_RATE;
  }
/*================================================================================================
/      setFOCUS(double newFOCUS)
/=================================================================================================*/
  public void setFOCUS(double newFOCUS) {
    double  oldFOCUS = this.FOCUS;
    this.FOCUS = newFOCUS;
    propertyChangeListeners.firePropertyChange("FOCUS", Double.valueOf(oldFOCUS), Double.valueOf(newFOCUS));
  }
  public double getFOCUS() {
    return FOCUS;
  }
/*================================================================================================
/      setTUBE_LENGTH(double newTUBE_LENGTH)
/=================================================================================================*/
  public void setTUBE_LENGTH(double newTUBE_LENGTH) {
    double  oldTUBE_LENGTH = this.TUBE_LENGTH;
    this.TUBE_LENGTH = newTUBE_LENGTH;
    propertyChangeListeners.firePropertyChange("TUBE_LENGTH", Double.valueOf(oldTUBE_LENGTH),Double.valueOf(newTUBE_LENGTH));
  }
  public double getTUBE_LENGTH() {
    return TUBE_LENGTH;
  }
/*================================================================================================
/      setCASS_RING_ANGLE(double newCASS_RING_ANGLE)
/=================================================================================================*/
  public void setCASS_RING_ANGLE(double newCASS_RING_ANGLE) {
    double  oldCASS_RING_ANGLE = this.CASS_RING_ANGLE;
    this.CASS_RING_ANGLE = newCASS_RING_ANGLE;
    propertyChangeListeners.firePropertyChange("CASS_RING_ANGLE",Double.valueOf(oldCASS_RING_ANGLE),Double.valueOf(newCASS_RING_ANGLE));
  }
  public double getCASS_RING_ANGLE() {
    return CASS_RING_ANGLE;
  }
/*================================================================================================
/       setLST(double LST)
/=================================================================================================*/
  public void setLST(double newLST) {
     double  oldLST = this.LST;
     this.LST = newLST;
     propertyChangeListeners.firePropertyChange("LST", Double.valueOf(oldLST), Double.valueOf(newLST));
  }
  public double getLST() {
     return LST;
  }
/*================================================================================================
/       setAzimuth(String azimuth)
/=================================================================================================*/
  public void setAzimuth(double newAZIMUTH) {
     double  oldAZIMUTH = this.AZIMUTH;
     this.AZIMUTH = newAZIMUTH;
     propertyChangeListeners.firePropertyChange("AZIMUTH", Double.valueOf(oldAZIMUTH),Double.valueOf(AZIMUTH));
  }
  public double getAzimuth() {
     return AZIMUTH;
  }
/*================================================================================================
/   setAzimuthString(java.lang.String newAzimuth)
/=================================================================================================*/
  public void setAzimuthString(java.lang.String new_azimuth) {
    setAzimuth((Double.valueOf(new_azimuth)).doubleValue());
    java.lang.String  old_azimuth = this.azimuth;
    this.azimuth = new_azimuth;
    propertyChangeListeners.firePropertyChange("azimuth", old_azimuth, new_azimuth);
   }
  public java.lang.String getAzimuthString() {
    return azimuth;
  }
/*================================================================================================
/       setZenithAngle(String zenithAngle)
/=================================================================================================*/
  public void setZenithAngle(double newZENITH_ANGLE) {
    double  oldZENITH_ANGLE = this.ZENITH_ANGLE;
    this.ZENITH_ANGLE = newZENITH_ANGLE;
    propertyChangeListeners.firePropertyChange("zenithAngle", Double.valueOf(oldZENITH_ANGLE), Double.valueOf(ZENITH_ANGLE));
  }
  public double getZenithAngle() {
    return ZENITH_ANGLE;
  }
/*================================================================================================
/       setZenithAngle(String zenithAngle)
/=================================================================================================*/
  public void setZenithAngleString(String new_zenith_angle) {
    setZenithAngle((Double.valueOf(new_zenith_angle)).doubleValue());
    String  old_zenith_angle = this.zenith_angle;
    this.zenith_angle = new_zenith_angle;
    propertyChangeListeners.firePropertyChange("zenith_angle", old_zenith_angle, new_zenith_angle);
  }
  public String getZenithAngleString() {
    return zenith_angle;
  }

/*================================================================================================
/       setDOME_AZIMUTH(String DOME_AZIMUTH)
/=================================================================================================*/
  public void setDomeAzimuth(double newDOME_AZIMUTH) {
    double  oldDOME_AZIMUTH = this.DOME_AZIMUTH;
    this.DOME_AZIMUTH = newDOME_AZIMUTH;
    propertyChangeListeners.firePropertyChange("DOME_AZIMUTH", Double.valueOf(oldDOME_AZIMUTH), Double.valueOf(DOME_AZIMUTH));
  }
  public double getDomeAzimuth() {
    return DOME_AZIMUTH;
  }
 /*================================================================================================
/       setDomeAzimuthString(String new_dome_azimuth)
/=================================================================================================*/
  public void setDomeAzimuthString(String new_dome_azimuth) {
    setDomeAzimuth((Double.valueOf(new_dome_azimuth)).doubleValue());
    String  old_dome_azimuth = this.dome_azimuth;
    this.dome_azimuth = new_dome_azimuth;
    propertyChangeListeners.firePropertyChange("dome_azimuth", old_dome_azimuth, dome_azimuth);
  }
  public java.lang.String getDomeAzimuthString() {
    return dome_azimuth;
  }
/*================================================================================================
/       setWindscreenPosition(String WINDSCREEN_POSITION)
/=================================================================================================*/
  public void setWindscreenPosition(double newWINDSCREEN_POSITION) {
    double  oldWINDSCREEN_POSITION = this.WINDSCREEN_POSITION;
    this.WINDSCREEN_POSITION = newWINDSCREEN_POSITION;
    propertyChangeListeners.firePropertyChange("WINDSCREEN_POSITION", Double.valueOf(oldWINDSCREEN_POSITION),Double.valueOf(WINDSCREEN_POSITION));
  }
  public double getWindscreenPosition() {
    return WINDSCREEN_POSITION;
  }
/*================================================================================================
/       setWindscreenPositionString(String new_windscreen_position)
/=================================================================================================*/
  public void setWindscreenPositionString(String new_windscreen_position) {
     setWindscreenPosition((Double.valueOf(new_windscreen_position)).doubleValue());
     String  old_windscreen_position = this.windscreen_position;
    this.windscreen_position = new_windscreen_position;
    propertyChangeListeners.firePropertyChange("windscreen_position", old_windscreen_position, new_windscreen_position);
  }
  public String getWindscreenPositionString() {
    return windscreen_position;
  }
/*================================================================================================
/       setDomeShutters(String DOME_SHUTTERS)
/=================================================================================================*/
  public void setDomeShutters(double newDOME_SHUTTERS) {
    double  oldDOME_SHUTTERS = this.DOME_SHUTTERS;
    this.DOME_SHUTTERS = newDOME_SHUTTERS;
    propertyChangeListeners.firePropertyChange("DOME_SHUTTERS", Double.valueOf(oldDOME_SHUTTERS), Double.valueOf(DOME_SHUTTERS));
  }
  public double getDomeShutters() {
    return DOME_SHUTTERS;
  }
/*================================================================================================
/       setDomeShutters(String DOME_SHUTTERS)
/=================================================================================================*/
  public void setDomeShuttersString(String new_dome_shutters) {
    setDomeShutters((Double.valueOf(new_dome_shutters)).doubleValue());
    String  old_dome_shutters = this.dome_shutters;
    this.dome_shutters = new_dome_shutters;
    propertyChangeListeners.firePropertyChange("dome_shutters", old_dome_shutters, new_dome_shutters);
  }
  public String getDomeShuttersString() {
    return dome_shutters;
  }
/*================================================================================================
/       setInstrumentPosition(String INSTRUMENT_POSITION)
/=================================================================================================*/
  public void setInstrumentPosition(int newINSTRUMENT_POSITION) {
    int  oldINSTRUMENT_POSITION = this.INSTRUMENT_POSITION;
    this.INSTRUMENT_POSITION = newINSTRUMENT_POSITION;
    propertyChangeListeners.firePropertyChange("INSTRUMENT_POSITION", Integer.valueOf(oldINSTRUMENT_POSITION), Integer.valueOf(INSTRUMENT_POSITION));
  }
  public int getInstrumentPosition() {
    return INSTRUMENT_POSITION;
  }
/*================================================================================================
/      setInstrumentPositionString(String new_instrument_position)
/=================================================================================================*/
  public void setInstrumentPositionString(String new_instrument_position) {
    setInstrumentPosition((Integer.valueOf(new_instrument_position)).intValue());
    String  old_instrument_position = this.instrument_position;
    this.instrument_position = new_instrument_position;
    propertyChangeListeners.firePropertyChange("instrument_position", old_instrument_position, new_instrument_position);
  }
  public String getInstrumentPositionString() {
    return instrument_position;
  }
/*================================================================================================
/       setTrackingRatesUnits(int new_tracking_rates_units)
/=================================================================================================*/
   public void setTrackingRatesUnits(int new_tracking_rates_units) {
    int  old_tracking_rates_units = this.tracking_rates_units;
    this.tracking_rates_units = new_tracking_rates_units;
    propertyChangeListeners.firePropertyChange("tracking_rates_units", Integer.valueOf(old_tracking_rates_units), Integer.valueOf(new_tracking_rates_units));
  }
  public int getTrackingRatesUnits() {
    return tracking_rates_units;
  }
/*================================================================================================
/       setInstrument(String INSTRUMENT)
/=================================================================================================*/
  public void setInstrument(java.lang.String newINSTRUMENT) {
    String  oldINSTRUMENT = this.INSTRUMENT;
    this.INSTRUMENT = newINSTRUMENT;
    propertyChangeListeners.firePropertyChange("INSTRUMENT", oldINSTRUMENT, INSTRUMENT);
  }
  public java.lang.String getInstrument() {
    return INSTRUMENT;
  }
/*================================================================================================
/       setPumps(java.lang.String PUMPS)   - NOT YET IMPLEMENTED ON THE TELESCOPE SIDE
/=================================================================================================*/
  public void setPumps(java.lang.String newPUMPS) {
    String  oldPUMPS = this.PUMPS;
    this.PUMPS = newPUMPS;
    propertyChangeListeners.firePropertyChange("PUMPS", oldPUMPS, PUMPS);
  }
  public java.lang.String getPumps() {
    return PUMPS;
  }
/*================================================================================================
/       setConnected(boolean connected)
/=================================================================================================*/
  public void setConnected(boolean connected) {
    boolean  oldConnected = this.connected;
    this.connected = connected;
    propertyChangeListeners.firePropertyChange("connected", Boolean.valueOf(oldConnected), Boolean.valueOf(connected));
  }
  public boolean isConnected() {
    return connected;
  }
/*================================================================================================
/       setMoving(boolean newMoving)
/=================================================================================================*/
  public void setMoving(boolean newMoving) {
    boolean  oldMoving = newMoving;
    this.moving = newMoving;
    propertyChangeListeners.firePropertyChange("moving", Boolean.valueOf(oldMoving), Boolean.valueOf(newMoving));
  }
  public boolean isMoving() {
    return moving;
  }
/*================================================================================================
/       setUpdating(boolean newUpdating)
/=================================================================================================*/
  public void setUpdating(boolean newUpdating) {
    boolean  oldUpdating = newUpdating;
    this.updating = newUpdating;
    propertyChangeListeners.firePropertyChange("updating", Boolean.valueOf(oldUpdating), Boolean.valueOf(newUpdating));
  }
  public boolean isUpdating() {
    return updating;
  }
/*================================================================================================
/       setConnected(boolean connected)
/=================================================================================================*/
  public void setLock(boolean newLock) {
    boolean  oldLock = this.lock;
    this.lock = newLock;
    propertyChangeListeners.firePropertyChange("lock", Boolean.valueOf(oldLock), Boolean.valueOf(newLock));
  }
  public boolean isLocked() {
    return lock;
  }
/*================================================================================================
/       setConnected(boolean connected)
/=================================================================================================*/
  public void setTracking(boolean newTracking) {
    boolean  oldTracking = this.tracking;
    this.tracking = newTracking;
    propertyChangeListeners.firePropertyChange("tracking", Boolean.valueOf(oldTracking),Boolean.valueOf(newTracking));
  }
  public boolean isTracking() {
    return tracking;
  }
/*================================================================================================
/       setProgress(int progress)
/=================================================================================================*/
  public void setProgress(int progress) {
    int  oldProgress = this.progress;
    this.progress = progress;
    propertyChangeListeners.firePropertyChange("progress",Integer.valueOf(oldProgress),Integer.valueOf(progress));
  }
  public int getProgress() {
    return progress;
  }
/*=============================================================================================
/     Setting and Getting the Percent Completed Variable
/=============================================================================================*/
  public void setMOUNT(int newMOUNT) {
    int  oldMOUNT =  MOUNT;
    MOUNT = newMOUNT;
    propertyChangeListeners.firePropertyChange("MOUNT",Integer.valueOf(oldMOUNT),Integer.valueOf(newMOUNT));
  }
  public int getMOUNT() {
    return MOUNT;
  } 
/*================================================================================================
/       setCaption(int progress)
/=================================================================================================*/
  public void setCaption(java.lang.String newCaption) {
    java.lang.String  oldCaption = this.caption;
    this.caption = newCaption;
    propertyChangeListeners.firePropertyChange("caption", oldCaption, newCaption);
  }
  public java.lang.String getCaption() {
    return caption;
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
/*================================================================================================
/     setObjectA(java.lang.String new_objectA)   - Astronomical Object Name from the GUI
/=================================================================================================*/
  public void setObjectNameA(java.lang.String new_objectA) {
    java.lang.String  old_objectA = this.objectA;
    this.objectA = new_objectA;
   propertyChangeListeners.firePropertyChange("objectA", old_objectA, new_objectA);
  }
  public java.lang.String getObjectNameA() {
    return objectA;
  }  
/*================================================================================================
/     setObjectA(java.lang.String new_objectA)  - object name derived from the telescope interface
/=================================================================================================*/
  public void setObjectNameT(java.lang.String new_objectT) {
    java.lang.String  old_objectT = this.objectT;
    this.objectT = new_objectT;
   propertyChangeListeners.firePropertyChange("objectT", old_objectT, new_objectT);
  }
  public java.lang.String getObjectNameT() {
    return objectT;
  }     
/*================================================================================================
/     setTotalExposureTime(double newTotalExposureTime)
/     setTotalElapsedTime(double newTotalElapsedTime)
/=================================================================================================*/
  public void setTotalElapsedTime(double newTotalElapsedTime) {
    double  oldTotalElapsedTime = totalElapsedTime;
    totalElapsedTime = newTotalElapsedTime;
    propertyChangeListeners.firePropertyChange("totalElapsedTime",Double.valueOf(oldTotalElapsedTime),Double.valueOf(newTotalElapsedTime));
  }

  public double getTotalElapsedTime() {
    return totalElapsedTime;
  }
/*=============================================================================================
/     Setting and Getting the Percent Completed Variable
/=============================================================================================*/
  public void setPercentCompleted(double newPercentCompleted) {
    double  oldPercentCompleted =  percentCompleted;
    percentCompleted = newPercentCompleted;
    propertyChangeListeners.firePropertyChange("percentCompleted",Double.valueOf(oldPercentCompleted), Double.valueOf(newPercentCompleted));
  }

  public double getPercentCompleted() {
    return percentCompleted;
  }
/*================================================================================================
/      setLowLamp(boolean new_low_lamp)
/=================================================================================================*/
  public void setLowLamp(boolean new_low_lamp) {
    boolean  old_low_lamp = this.low_lamp;
    this.low_lamp = new_low_lamp;
    propertyChangeListeners.firePropertyChange("low_lamp",Boolean.valueOf(old_low_lamp),Boolean.valueOf(new_low_lamp));
  }
  public boolean isLowLamp() {
    return low_lamp;
  }  
/*================================================================================================
/      setHighLamp(boolean new_high_lamp)
/=================================================================================================*/
  public void setHighLamp(boolean new_high_lamp) {
    boolean  old_high_lamp = this.high_lamp;
    this.high_lamp = new_high_lamp;
    propertyChangeListeners.firePropertyChange("high_lamp",Boolean.valueOf(old_high_lamp),Boolean.valueOf(new_high_lamp));
  }
  public boolean isHighLamp() {
    return high_lamp;
  } 
/*================================================================================================
/      setHighLamp(boolean new_high_lamp)
/=================================================================================================*/
  public void setArcLamp(boolean new_arc_lamp) {
    boolean  old_arc_lamp = this.arc_lamp;
    this.arc_lamp = new_arc_lamp;
    propertyChangeListeners.firePropertyChange("arc_lamp",Boolean.valueOf(old_arc_lamp),Boolean.valueOf(new_arc_lamp));
  }
  public boolean isArcLamp() {
    return arc_lamp;
  }   
/*================================================================================================
/   setStatusMessage(java.lang.String newStatusMessage)
/=================================================================================================*/
      public void setStatusMessage(java.lang.String newStatusMessage) {
         java.lang.String  oldStatusMessage = this.statusMessage;
         this.statusMessage = newStatusMessage;
         propertyChangeListeners.firePropertyChange("statusMessage", oldStatusMessage, newStatusMessage);
      }
      public java.lang.String getStatusMessage() {
         return statusMessage;
      }
/*================================================================================================
/      CalculateTelescopeMoveTime()
/=================================================================================================*/
public double CalculateTelescopeMoveTime(double RAoffset,double Decoffset){
   // set the rate of motion to 1/2 the greater of the greater of the two offsets
   double RAMoveRate = MAX_RA_MOVE_RATE;
   double DecMoveRate = MAX_DEC_MOVE_RATE;
   double MoveTimeEstimate = 0.0;
   RAoffset =Math.abs(RAoffset);    // take the absolute value of the move so the time isn't dependent on sign
   Decoffset =Math.abs(Decoffset);

   if(RAoffset <= 2*MAX_RA_MOVE_RATE){
      RAMoveRate = RAoffset/2.0;
      if(RAMoveRate < MIN_RA_MOVE_RATE){
          RAMoveRate = MIN_RA_MOVE_RATE;
      }
   }
   if(RAoffset > 2*MAX_RA_MOVE_RATE){
      RAMoveRate = MAX_RA_MOVE_RATE;
   }
   if(Decoffset <= 2*MAX_DEC_MOVE_RATE){
      DecMoveRate = Decoffset/2.0;
      if(DecMoveRate < MIN_DEC_MOVE_RATE){
          DecMoveRate = MIN_DEC_MOVE_RATE;
      }
   }
   if(Decoffset > 2*MAX_DEC_MOVE_RATE){
      DecMoveRate = MAX_DEC_MOVE_RATE;
   }
   setRAMoveRate(RAMoveRate);
   setDecMoveRate(DecMoveRate);
   if(isManualMoveRate()){
     RAMoveRate  = RAMoveRateManual;
     DecMoveRate = DecMoveRateManual;
     setRAMoveRate(RAMoveRateManual);
     setDecMoveRate(DecMoveRateManual);
   }
   MoveTimeEstimate = (RAoffset/RAMoveRate) + (Decoffset/DecMoveRate) + TELESCOPE_SETTLE_TIME;
//   execute_move_flag_monitor(MoveTimeEstimate);
   // method returns the estimated move time in seconds, multiply by 1000 for milliseconds
  return MoveTimeEstimate;
}
/*================================================================================================
/      execute_move_flag_monitor(double new_move_time)
/=================================================================================================*/
public void execute_move_flag_monitor(double new_move_time){
    MotionStatusThread myMotionStatusThread = new MotionStatusThread(new_move_time);
    myMotionStatusThread.start();
}
/*================================================================================================
/      move_flag_monitor(double move_time_estimate)
/=================================================================================================*/
public void move_flag_monitor(double move_time_estimate){
   setMotionStatus(MOTION_OFFSETTING);
   long move_time_long = (long)move_time_estimate*1000;
   long start_time = System.currentTimeMillis();
   long elapsed_time = 0;
   while(elapsed_time < move_time_long){
       elapsed_time = System.currentTimeMillis() - start_time;
   }
   setMotionStatus(MOTION_TRACKING_STABLY);   
}
/*================================================================================================
/      CalculateTelescopeMoveTimeMaxRate(double RAoffset,double Decoffset)
/=================================================================================================*/
public double CalculateTelescopeMoveTimeMaxRate(double RAoffset,double Decoffset){
   // set the rate of motion to 1/2 the greater of the greater of the two offsets
   double RAMoveRate = MAX_RA_MOVE_RATE;
   double DecMoveRate = MAX_DEC_MOVE_RATE;
   double MoveTimeEstimate = 0.0;
   RAoffset =Math.abs(RAoffset);    // take the absolute value of the move so the time isn't dependent on sign
   Decoffset =Math.abs(Decoffset);

   MoveTimeEstimate = (RAoffset/RAMoveRate) + (Decoffset/DecMoveRate)+ TELESCOPE_SETTLE_TIME;
   // method returns the estimated move time in seconds, multiply by 1000 for milliseconds
//   execute_move_flag_monitor(MoveTimeEstimate);
  return MoveTimeEstimate;
}
/*================================================================================================
/      evaluateMoveDirections(double RAoffset,double Decoffset)
/=================================================================================================*/
 public void evaluateMoveDirections(double RAoffset,double Decoffset){
      if((RAoffset > 0)&(Decoffset > 0)){
          setTelescopeMotionDirection(TelescopeObject.MOVING_NE);
       }
       if((RAoffset > 0)&(Decoffset < 0)){
          setTelescopeMotionDirection(TelescopeObject.MOVING_SE);
       }
       if((RAoffset < 0)&(Decoffset < 0)){
          setTelescopeMotionDirection(TelescopeObject.MOVING_SW);
       }
       if((RAoffset < 0)&(Decoffset > 0)){
          setTelescopeMotionDirection(TelescopeObject.MOVING_NW);
       }
       if((RAoffset == 0)&(Decoffset > 0)){
          setTelescopeMotionDirection(TelescopeObject.MOVING_N);
       }
       if((RAoffset == 0)&(Decoffset < 0)){
          setTelescopeMotionDirection(TelescopeObject.MOVING_S);
       }
       if((RAoffset < 0)&(Decoffset == 0)){
          setTelescopeMotionDirection(TelescopeObject.MOVING_W);
       }
       if((RAoffset > 0)&(Decoffset == 0)){
          setTelescopeMotionDirection(TelescopeObject.MOVING_E);
       }
 }
/*================================================================================================
/      CalculateFocusMoveTime(double currentFocusOffset)
/=================================================================================================*/
public double CalculateFocusMoveTime(double currentFocusOffset){
     currentFocusOffset = Math.abs(currentFocusOffset);
     int direction = getFocusMotionDirection();
     double FocusMoveTimeEstimate = 0.0;
     double FOCUS_RATE_CUTOFF = 0.2;
     double calibration =10.0;
     if(currentFocusOffset <= FOCUS_RATE_CUTOFF){
//        FOCUS_MOVE_RATE = calibration*0.05;
        FOCUS_MOVE_RATE = calibration*6.0;
     }
    if((currentFocusOffset > FOCUS_RATE_CUTOFF)){
//       FOCUS_MOVE_RATE = calibration*0.18;
//        FOCUS_MOVE_RATE = calibration*0.18;
         FOCUS_MOVE_RATE = calibration;
      }
     if(direction == FOCUS_MOVING_UP){
      FocusMoveTimeEstimate = currentFocusOffset*FOCUS_MOVE_RATE;
     }
     if(direction == FOCUS_MOVING_DOWN){
      FocusMoveTimeEstimate = 1.5*(currentFocusOffset*FOCUS_MOVE_RATE);
     }
   return FocusMoveTimeEstimate;
}
/*================================================================================================
/      evaluateFocusMoveDirection(double targetFocusValue)
/=================================================================================================*/
public void evaluateFocusMoveDirection(double targetFocusValue){
    double focusDelta = targetFocusValue - FOCUS;
    if(focusDelta > 0.0){
        setFocusMotionDirection(FOCUS_MOVING_UP);
    }
    if(focusDelta < 0.0){
        setFocusMotionDirection(FOCUS_MOVING_DOWN);

    }
}
/*================================================================================================
/       setCaption(int progress)
/=================================================================================================*/
public static final byte[] intToByteArray(int value) {
        return new byte[] {
                (byte)(value >>> 24),
                (byte)(value >>> 16),
                (byte)(value >>> 8),
                (byte)value};
}
/*================================================================================================
/       setCaption(int progress)
/=================================================================================================*/
public static final byte[] shortToByteArray(short value) {
        return new byte[] {
                (byte)(value >>> 8),
                (byte)value};
}
/*================================================================================================
/       setCaption(int progress)
/=================================================================================================*/
public static short byteArrayToShort(byte[] data, int offset) {
	return (short) (((data[offset] << 8)) | ((data[offset + 1] & 0xff)));
}
/*================================================================================================
/       setCaption(int progress)
/=================================================================================================*/
public static final int byteArrayToInt(byte [] b) {
        return (b[0] << 24)
                + ((b[1] & 0xFF) << 16)
                + ((b[2] & 0xFF) << 8)
                + (b[3] & 0xFF);
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
/       Ephemeris Related Information
/=================================================================================================*/
/*================================================================================================
/        parseSexag(String sexagStr)
/=================================================================================================*/
  /**
      * Given a sexag string (from MCS subscribe response) parse
      * it and return the double value. Do not know place of
      * separators (since not padded) nor the separators themselves.
      *
      * @param sexagStr String in sexag format; do not assume the first
      * element is padded, do assume the second and third are; i.e., may
      * have 4h04m04s but not 4h4m4s.
      */
     public static double parseSexag(String sexagStr) {

         // What to default to??
         double d = 0;

         boolean isNeg = sexagStr.charAt(0) == ('-');
         int idx = (isNeg) ? 1 : 0;

         // Find first separator
         for (; idx < sexagStr.length(); idx++) {
            if (!Character.isDigit(sexagStr.charAt(idx)) ) {
               break;
            }
         }

         // Get numeric values for element, assume second element
         // and non decimal part of third element are always 2 chars
         // long.
         String se[] = new String[3];
         se[0] = sexagStr.substring(0, idx);
         se[1] = sexagStr.substring(idx+1, idx+3);
         se[2] = sexagStr.substring(idx+4, sexagStr.length()-1);
         double de[] = new double[3];
         try {
            de[2] = Double.parseDouble(se[2]) / 60.0d;
         } catch(NumberFormatException nfe) {
            de[2] = 0;
         }
         try {
            de[1] = (Double.parseDouble(se[1]) + de[2])/60.0d;
         } catch(NumberFormatException nfe) {
            de[1] = 0;
         }
         try {
            d = Math.abs(Double.parseDouble(se[0])) + de[1];
         } catch(NumberFormatException nfe) {
         }
         if (isNeg) {
            d *= -1.0;
         }
         return d;
     }
/*=============================================================================================
/     Property Change Listener for the _propertyChange
/=============================================================================================*/
  private void Celestial_propertyChange(PropertyChangeEvent e)  {
     java.lang.String propertyName = e.getPropertyName();
/*=============================================================================================
/
/=============================================================================================*/
 /*   if(propertyName == "right_ascension"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "declination"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
   if(propertyName == "objectname_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "equinox_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "hourangle_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "airmass_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "universal_time_date"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "universal_time"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "local_siderial_time"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "right_ascension_offset"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "declination_offset"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "right_ascension_track_rate"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "declination_track_rate"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "telescope_focus"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "telescope_tube_length"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "cass_ring_angle"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "telescope_ID"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
  */
  }
/*=============================================================================================
/       INNER CLASS that sends a set of coordinates to the telescope
/=============================================================================================*/
    public class MotionStatusThread  implements Runnable{
    private Thread myThread;
    private double move_time;
/*=============================================================================================
/          SendCoordinatesThread()
/=============================================================================================*/
    private MotionStatusThread(double new_move_time){
        move_time = new_move_time;
    }
/*=============================================================================================
/           run()
/=============================================================================================*/
    public void run(){
        move_flag_monitor(move_time);
    }
/*=============================================================================================
/         start()
/=============================================================================================*/
    public void start(){
     myThread = new Thread(this);
     myThread.start();
     }
    }// End of the SendCommandThread Inner Class
/*=============================================================================================
/                           End of the SendCommandThread Class
/=============================================================================================*/  
}
/*================================================================================================
/      End of the TelescopeObject Class
/=================================================================================================*/
/*struct {				/ 100 BYTES TOTAL /
	short 	ra_hours;		/ COORDINATES ALWAYS J2000.0 /
	short	ra_minutes;
	short	ra_seconds;
	short	ra_hundredth_seconds;

 short	dummyword_4;

        short	dec_degrees;
	short	dec_minutes;
	short	dec_seconds;
	short	dec_tenth_seconds;
	short	dec_sign;		/ 1 = POS, 0 = NEG /

        short	lst_hours;
	short	lst_minutes;
	short	lst_seconds;
	short	lst_tenth_seconds;

        short	dummyword_14;

        short	ha_hours;		/ HA IS APPARENT /
	short	ha_minutes;
	short	ha_seconds;
	short	ha_tenth_seconds;
	short	ha_direction;		/ 1 = EAST, 0 = WEST /

        short	ut_hours;
	short	ut_minutes;
	short	ut_seconds;
	short	ut_tenth_seconds;

        float	airmass;

        float	display_equinox;	/ J2000.0 /

        short	dummyword_28;
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

        short	telescope_id;		/ 200 or 60 /
} telescope_data;*/

/*================================================================================================
/         Summary of Property Change Events propagated by the TelescopeObject Class
/=================================================================================================*/
/*  propertyChangeListeners.firePropertyChange("right_ascension", oldright_ascension, right_ascension);
    propertyChangeListeners.firePropertyChange("declination", oldDeclination, declination);
    propertyChangeListeners.firePropertyChange("equinox_string", oldequinox_string, equinox_string);
    propertyChangeListeners.firePropertyChange("hourangle_string", oldhourangle_string, hourangle_string);
    propertyChangeListeners.firePropertyChange("airmass_string", oldairmass_string, airmass_string);
    propertyChangeListeners.firePropertyChange("UTCSTART", oldUTCSTART, newUTCSTART);
    propertyChangeListeners.firePropertyChange("UTCEND", oldUTCEND, newUTCEND);
    propertyChangeListeners.firePropertyChange("DATE_OBS", oldDATE_OBS, newDATE_OBS);
    propertyChangeListeners.firePropertyChange("DATE", oldDATE, newDATE);
    propertyChangeListeners.firePropertyChange("right_ascension_offset", oldright_ascension_offset, right_ascension_offset);
    propertyChangeListeners.firePropertyChange("declination_offset", olddeclination_offset, declination_offset);
    propertyChangeListeners.firePropertyChange("right_ascension_track_rate", oldright_ascension_track_rate, right_ascension_track_rate);
    propertyChangeListeners.firePropertyChange("declination_track_rate", olddeclination_track_rate, declination_track_rate);
    propertyChangeListeners.firePropertyChange("telescope_focus", oldtelescope_focus, telescope_focus);
    propertyChangeListeners.firePropertyChange("telescope_focus", oldtelescope_tube_length, telescope_tube_length);
    propertyChangeListeners.firePropertyChange("cass_ring_angle", oldcass_ring_angle, cass_ring_angle);
    propertyChangeListeners.firePropertyChange("telescope_ID", oldtelescope_ID, telescope_ID);
    propertyChangeListeners.firePropertyChange("RA", new Double(oldRA), new Double(newRA));
    propertyChangeListeners.firePropertyChange("DEC", new Double(oldDEC), new Double(newDEC));
    propertyChangeListeners.firePropertyChange("EQUINOX", new Double(oldEQUINOX), new Double(newEQUINOX));
    propertyChangeListeners.firePropertyChange("HA", new Double(oldHA), new Double(newHA));
    propertyChangeListeners.firePropertyChange("AIRMASS", new Double(oldAIRMASS), new Double(newAIRMASS));
    propertyChangeListeners.firePropertyChange("RA_OFFSET", new Integer(oldRA_OFFSET), new Integer(newRA_OFFSET));
    propertyChangeListeners.firePropertyChange("DEC_OFFSET", new Integer(oldDEC_OFFSET), new Integer(newDEC_OFFSET));
    propertyChangeListeners.firePropertyChange("RA_TRACK_RATE", new Integer(oldRA_TRACK_RATE), new Integer(newRA_TRACK_RATE));
    propertyChangeListeners.firePropertyChange("DEC_TRACK_RATE", new Integer(oldDEC_TRACK_RATE), new Integer(newDEC_TRACK_RATE));
    propertyChangeListeners.firePropertyChange("RA_GUIDE_RATE", new Integer(oldRA_GUIDE_RATE), new Integer(newRA_GUIDE_RATE));
    propertyChangeListeners.firePropertyChange("DEC_GUIDE_RATE", new Integer(oldDEC_GUIDE_RATE), new Integer(newDEC_GUIDE_RATE));
    propertyChangeListeners.firePropertyChange("RA_SET_RATE", new Integer(oldRA_SET_RATE), new Integer(newRA_SET_RATE));
    propertyChangeListeners.firePropertyChange("DEC_SET_RATE", new Integer(oldDEC_SET_RATE), new Integer(newDEC_SET_RATE));
    propertyChangeListeners.firePropertyChange("FOCUS", new Double(oldFOCUS), new Double(newFOCUS));
    propertyChangeListeners.firePropertyChange("TUBE_LENGTH", new Double(oldTUBE_LENGTH), new Double(newTUBE_LENGTH));
    propertyChangeListeners.firePropertyChange("CASS_RING_ANGLE", new Double(oldCASS_RING_ANGLE), new Double(newCASS_RING_ANGLE));
    propertyChangeListeners.firePropertyChange("LST", new Double(oldLST), new Double(LST));
    propertyChangeListeners.firePropertyChange("azimuth", new Double(oldAZIMUTH), new Double(AZIMUTH));
    propertyChangeListeners.firePropertyChange("zenithAngle", new Double(oldZENITH_ANGLE), new Double(ZENITH_ANGLE));
    propertyChangeListeners.firePropertyChange("DOME_AZIMUTH", new Double(oldDOME_AZIMUTH), new Double(DOME_AZIMUTH));
    propertyChangeListeners.firePropertyChange("WINDSCREEN_POSITION", new Double(oldWINDSCREEN_POSITION), new Double(WINDSCREEN_POSITION));
    propertyChangeListeners.firePropertyChange("DOME_SHUTTERS", new Integer(oldDOME_SHUTTERS), new Integer(DOME_SHUTTERS));
    propertyChangeListeners.firePropertyChange("INSTRUMENT_POSITION", new Integer(oldINSTRUMENT_POSITION), new Integer(INSTRUMENT_POSITION));
    propertyChangeListeners.firePropertyChange("INSTRUMENT", oldINSTRUMENT, INSTRUMENT);
    propertyChangeListeners.firePropertyChange("PUMPS", oldPUMPS, PUMPS);
    propertyChangeListeners.firePropertyChange("connected", new Boolean(oldConnected), new Boolean(connected));
    propertyChangeListeners.firePropertyChange("moving", new Boolean(oldMoving), new Boolean(newMoving));
    propertyChangeListeners.firePropertyChange("updating", new Boolean(oldUpdating), new Boolean(newUpdating));
    propertyChangeListeners.firePropertyChange("lock", new Boolean(oldLock), new Boolean(newLock));
    propertyChangeListeners.firePropertyChange("tracking", new Boolean(oldTracking), new Boolean(newTracking));
    propertyChangeListeners.firePropertyChange("progress", new Integer(oldProgress), new Integer(progress));
    propertyChangeListeners.firePropertyChange("caption", oldCaption, newCaption);
*/