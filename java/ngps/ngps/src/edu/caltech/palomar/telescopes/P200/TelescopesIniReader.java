package edu.caltech.palomar.telescopes.P200;  
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
import java.lang.Object;
import java.io.File;
import com.ibm.inifile.IniFile;
/*=============================================================================================
/      class TelescopesIniReader
/=============================================================================================*/
public class TelescopesIniReader extends java.lang.Object{
  // Static Strings that represent the sections of the INI file.
  public static java.lang.String CONNECTION_PARAMETERS            = new java.lang.String("CONNECTION_PARAMETERS");
  public static java.lang.String TELESCOPE_SECTION                = new java.lang.String("TELESCOPE_SECTION");
  public static java.lang.String CATALOG_SECTION                  = new java.lang.String("CATALOG_SECTION");
  public static java.lang.String XYSTAGE_CONNECTION_PARAMETERS    = new java.lang.String("XYSTAGE_CONNECTION_PARAMETERS");
  public static java.lang.String ALLSKY_SECTION                   = new java.lang.String("ALLSKY_SECTION");
      // Initialization File Bean
  public IniFile myIniFile;
  private String iniFileName;
  private java.io.File iniFile;
  public   java.lang.String USERDIR          = System.getProperty("user.dir");
  public   java.lang.String SEP              = System.getProperty("file.separator");
  public   java.lang.String CONFIG           = new java.lang.String("config");
  public   java.lang.String TELESCOPE        = new java.lang.String("telescope.cfg");
  public   java.lang.String INIFILE          = new java.lang.String();
/*=============================================================================================
/      Keyword Definitions for World Coordinate System Cards
/=============================================================================================*/
    public static java.lang.String  SERVERNAME         = new java.lang.String("SERVERNAME");   //
    public static java.lang.String  SERVERPORT         = new java.lang.String("SERVERPORT");   //
    public static java.lang.String  XYSTAGE_SERVERNAME = new java.lang.String("XYSTAGE_SERVERNAME");   //
    public static java.lang.String  XYSTAGE_SERVERPORT = new java.lang.String("XYSTAGE_SERVERPORT");   //
    public static java.lang.String  LOGIN_COMMAND      = new java.lang.String("LOGIN_COMMAND");   //
    public static java.lang.String  SIMULATION_STATE   = new java.lang.String("SIMULATION_STATE");   //
    public static java.lang.String  XYSTAGE_SIMULATION_STATE   = new java.lang.String("XYSTAGE_SIMULATION_STATE");   //
    public static java.lang.String  DEPLOYMENT         = new java.lang.String("DEPLOY");   //
    public static java.lang.String  CREATOR            = new java.lang.String("CREATOR");   //
    public static java.lang.String  WVCOLDEN           = new java.lang.String("WVCOLDEN");   //
    public static java.lang.String  TEMP_OUT           = new java.lang.String("TEMP_OUT");   //
    public static java.lang.String  TEMPR1             = new java.lang.String("TEMPR1");   //
    public static java.lang.String  TEMPR2             = new java.lang.String("TEMPR2");   //
    public static java.lang.String  TEMPR3             = new java.lang.String("TEMPR3");   //
    public static java.lang.String  TEMPSEC1           = new java.lang.String("TEMPSEC1");   //
    public static java.lang.String  AC_ALT             = new java.lang.String("AC_ALT");   //
    public static java.lang.String  AIRSPEED           = new java.lang.String("AIRSPEED");   //
    public static java.lang.String  GRDSPEED           = new java.lang.String("GRDSPEED");   //
    public static java.lang.String  AC_LAT             = new java.lang.String("AC_LAT");   //
    public static java.lang.String  AC_LONG            = new java.lang.String("AC_LONG");   //
    public static java.lang.String  HEADING            = new java.lang.String("HEADING");   //
    public static java.lang.String  HELIO_COR          = new java.lang.String("HELIO_COR");   //
    public static java.lang.String  DELTKRA            = new java.lang.String("DELTKRA");   //
    public static java.lang.String  DELTKDEC           = new java.lang.String("DELTKDEC");   //
    public static java.lang.String  TELRA              = new java.lang.String("TELRA");   //
    public static java.lang.String  TELDEC             = new java.lang.String("TELDEC");   //
    public static java.lang.String  AIRMASS            = new java.lang.String("AIRMASS");   //
    public static java.lang.String  EQUINOX            = new java.lang.String("EQUINOX");   //
    public static java.lang.String  ZA                 = new java.lang.String("ZA");   //
    public static java.lang.String  HA                 = new java.lang.String("HA");   //
    public static java.lang.String  SUNANGL            = new java.lang.String("SUNANGL");   //
    public static java.lang.String  MOONANGL           = new java.lang.String("MOONANGL");   //
    public static java.lang.String  ROT_ANGL           = new java.lang.String("ROT_ANGL");   //
    public static java.lang.String  LSR_COR            = new java.lang.String("LSR_COR");   //
    public static java.lang.String  TELFOCUS           = new java.lang.String("TELFOCUS");   //
    public static java.lang.String  TELTCS             = new java.lang.String("TELTCS");   //
    public static java.lang.String  TRACMODE           = new java.lang.String("TRACMODE");   //
    public static java.lang.String  TRACGDST           = new java.lang.String("TRACGDST");   //
    public static java.lang.String  ROF_FRZ            = new java.lang.String("ROF_FRZ");   //
    public static java.lang.String  PLANID             = new java.lang.String("PLANID");   //
    public static java.lang.String  SCHEDBLK           = new java.lang.String("SCHEDBLK");   //
    public static java.lang.String  FLIGHTNO           = new java.lang.String("FLIGHTNO");   //
    public static java.lang.String  FLIGHTLG           = new java.lang.String("FLIGHTLG");   //
    public static java.lang.String  TIME_FLIGHT_LEG    = new java.lang.String("TIME_FLIGHT_LEG");   //
    public static java.lang.String  TIME_ROT_REWIND    = new java.lang.String("TIME_ROT_REWIND");   //
    public static java.lang.String  QUERY_COMMAND      = new java.lang.String("QUERYCOMMAND");   //
    public static java.lang.String  TELESCOP           = new java.lang.String("TELESCOP");   //
    public static java.lang.String  OBSERVAT           = new java.lang.String("OBSERVAT");   //
    public static java.lang.String  OPERATOR           = new java.lang.String("OPERATOR");   //
    public static java.lang.String  HOUSEKEEPING1      = new java.lang.String("HOUSEKEEPING1");   //
    public static java.lang.String  HOUSEKEEPING2      = new java.lang.String("HOUSEKEEPING2");   //
    public static java.lang.String  HOUSEKEEPING3      = new java.lang.String("HOUSEKEEPING3");   //
    public static java.lang.String  TELESCOPEMONITOR1  = new java.lang.String("TELESCOPEMONITOR1");   //
    public static java.lang.String  TELESCOPEMONITOR2  = new java.lang.String("TELESCOPEMONITOR2");   //
    public static java.lang.String  TELESCOPEMONITOR3  = new java.lang.String("TELESCOPEMONITOR3");   //
    public static java.lang.String  TELESCOPESTATE1    = new java.lang.String("TELESCOPESTATE1");   //
    public static java.lang.String  TELESCOPESTATE2    = new java.lang.String("TELESCOPESTATE2");   //
    public static java.lang.String  TELESCOPESTATE3    = new java.lang.String("TELESCOPESTATE3");   //
    public static java.lang.String  POSITIONLIST       = new java.lang.String("POSITIONLIST");   //
    public        java.lang.String  SERVERNAME_STRING               = new java.lang.String();   //
    public        java.lang.String  SERVERPORT_STRING               = new java.lang.String();   //
    public        java.lang.String  SIMULATION_STATE_STRING         = new java.lang.String();   //
    public        java.lang.String  XYSTAGE_SERVERNAME_STRING       = new java.lang.String();   //
    public        java.lang.String  XYSTAGE_SERVERPORT_STRING       = new java.lang.String();   //
    public        java.lang.String  XYSTAGE_SIMULATION_STATE_STRING = new java.lang.String();   //
    public        java.lang.String  SDSS_SERVER_ADDRESS             = new java.lang.String();
    //   decimal representations of WCS default values
    public   int     SERVERPORT_INTEGER;
    public   boolean SIMULATION_STATE_BOOLEAN;
    public   int     XYSTAGE_SERVERPORT_INTEGER;
    public   boolean XYSTAGE_SIMULATION_STATE_BOOLEAN;

    public  java.lang.String  DEFAULT_OFFSET          = new java.lang.String();   //
    public  java.lang.String  DEFAULT_RA_OFFSET       = new java.lang.String();   //
    public  java.lang.String  DEFAULT__DEC_OFFSET     = new java.lang.String();   //
    public  java.lang.String  DEFAULT__RA_MOVE_RATE   = new java.lang.String();   //
    public  java.lang.String  DEFAULT__DEC_MOVE_RATE  = new java.lang.String();   //
    public  java.lang.String  DEFAULT__FOCUS_OFFSET   = new java.lang.String();   //
    public  java.lang.String  FOCUS_MOVE_RATE         = new java.lang.String();
    public  java.lang.String  TELESCOPE_SETTLE_TIME   = new java.lang.String();
    public  java.lang.String  POLLING_RATE            = new java.lang.String();
    public  java.lang.String  TYCHO2_INDEX            = new java.lang.String();
    public  java.lang.String  TYCHO2_CATALOG          = new java.lang.String();
    public  java.lang.String  UCAC3PATH               = new java.lang.String();
    public  java.lang.String  DEFAULT_OBSERVER_DIR    = new java.lang.String();
    public  java.lang.String  SDSS_IMAGE_PATH         = new java.lang.String();
    public  double            XYSTAGE_STEPS_PER_SECOND_X;
    public  double            XYSTAGE_STEPS_PER_SECOND_Y;
    public  int               XYSTAGE_CALIBRATION_X;
    public  int               XYSTAGE_CALIBRATION_Y;
    public  int               RAW_MASK_DIAMETER;
    public  int               OVERLAY_MASK_DIAMETER;
    public  double            DECENTER_X ;
    public  double            DECENTER_Y;
    public  double            COEFFICIENT_A;
    public  double            COEFFICIENT_B;
    public  double            COEFFICIENT_C;
    public  double            ROTATION;
    public  java.lang.String  ALLSKY_HOST              = new java.lang.String();
    public  java.lang.String  ALLSKY_PATH              = new java.lang.String();
    public  java.lang.String  ALLSKY_IMAGE_NAME        = new java.lang.String();
    public  int               ALLSKY_PORT;
    public  int               ALLSKY_POLLING_TIME;
    public   int FACSUMFORM_X ;
    public   int FACSUMFORM_Y;
    public   boolean FACSUMFORM_VIS;
    public   int AIRMASS_X;
    public   int AIRMASS_Y;
    public   boolean AIRMASS_VIS;
    public   int EPHEMERIS_X;
    public   int EPHEMERIS_Y ;
    public   boolean EPHEMERIS_VIS;
    public   int SKY_X;
    public   int SKY_Y;
    public   boolean SKY_VIS;
    public   int ASTRO_OBJECT_X;
    public   int ASTRO_OBJECT_Y;
    public   boolean ASTRO_OBJECT_VIS;
/*=================================================================================================
/        TelescopeIniReader Default Constructor
/=================================================================================================*/
  public TelescopesIniReader() {
    INIFILE = USERDIR + SEP + CONFIG + SEP + TELESCOPE;
    setIniFileName(INIFILE);
    initializeINIFile();
  }
/*================================================================================================
/        TelescopeIniReader Constructor that initializes and reads the INI File
/=================================================================================================*/
  public TelescopesIniReader(java.lang.String newIniFileName,boolean newInitialize) {
     setIniFileName(newIniFileName);     
     if(newInitialize){
        initializeINIFile();
     }
  }
/*================================================================================================
/        initializeINIFile() - This method reads the initialization file.
/=================================================================================================*/
  public void initializeINIFile(){
    try{
      // Now open the IniFile as and INI File
      myIniFile = new com.ibm.inifile.IniFile(getIniFileName());
      myIniFile.setSectionized(true);
      initializeConnectionParameters();
      initializeTelescopeCommands();
      initializeCatalog();
      initializeXYStageConnectionParameters();
      initializeAllSkyImage();
      initializeFramePositions();
    }
    catch(Exception e){
      logMessage("An error occurred while reading parameters from the Initialization File. "+e);
    }
  }
/*================================================================================================
/        rereadINIFile() - This method reads the initialization file.
/=================================================================================================*/
  public void rereadINIFile(){
    try{
      // Now open the IniFile as and INI File
      myIniFile.reRead();
     }
    catch(Exception e){
      logMessage("An error occurred while rereading parameters from the Initialization File. "+e);
    }
  }
/*================================================================================================
/        initializeConnectionParameters() - This method initializes the properties for the
/=================================================================================================*/
  public void initializeConnectionParameters(){
    java.lang.String sectionName = new java.lang.String();
    sectionName = CONNECTION_PARAMETERS;
    try{  // WCS Properties
          SERVERNAME_STRING         = myIniFile.getProperty(SERVERNAME,sectionName);
          SERVERPORT_STRING         = myIniFile.getProperty(SERVERPORT,sectionName);
          SIMULATION_STATE_STRING   = myIniFile.getProperty(SIMULATION_STATE,sectionName);
          // Construct the integer version of the values retrieved from the initialization file
          SERVERPORT_INTEGER        = (Integer.valueOf(SERVERPORT_STRING)).intValue();
          SIMULATION_STATE_BOOLEAN  = (Boolean.valueOf(SIMULATION_STATE_STRING)).booleanValue();
    }
    catch(Exception e){
      logMessage("An error occurred while reading the SOFIA Simulator Connection Parameters from the Initialization File. "+e);
    }
}
 /*================================================================================================
/        initializeConnectionParameters() - This method initializes the properties for the
/=================================================================================================*/
  public void initializeXYStageConnectionParameters(){
    java.lang.String sectionName = new java.lang.String();
    sectionName = XYSTAGE_CONNECTION_PARAMETERS;
    try{  // WCS Properties
          XYSTAGE_SERVERNAME_STRING         = myIniFile.getProperty(XYSTAGE_SERVERNAME,sectionName);
          XYSTAGE_SERVERPORT_STRING         = myIniFile.getProperty(XYSTAGE_SERVERPORT,sectionName);
          XYSTAGE_SIMULATION_STATE_STRING   = myIniFile.getProperty(XYSTAGE_SIMULATION_STATE,sectionName);
          XYSTAGE_STEPS_PER_SECOND_X        = (Double.valueOf(myIniFile.getProperty("XYSTAGE_STEPS_PER_SECOND_X",sectionName))).doubleValue();
          XYSTAGE_STEPS_PER_SECOND_Y        = (Double.valueOf(myIniFile.getProperty("XYSTAGE_STEPS_PER_SECOND_Y",sectionName))).doubleValue();
          // Construct the integer version of the values retrieved from the initialization file
          XYSTAGE_SERVERPORT_INTEGER        = (Integer.valueOf(XYSTAGE_SERVERPORT_STRING)).intValue();
          XYSTAGE_SIMULATION_STATE_BOOLEAN  = (Boolean.valueOf(XYSTAGE_SIMULATION_STATE_STRING)).booleanValue();
          SDSS_SERVER_ADDRESS               = myIniFile.getProperty("SDSS_SERVER_ADDRESS",sectionName);
          SDSS_IMAGE_PATH                   = myIniFile.getProperty("SDSS_IMAGE_PATH",sectionName);
          XYSTAGE_CALIBRATION_X             = (Integer.valueOf(myIniFile.getProperty("XYSTAGE_CALIBRATION_X",sectionName))).intValue();
          XYSTAGE_CALIBRATION_Y             = (Integer.valueOf(myIniFile.getProperty("XYSTAGE_CALIBRATION_Y",sectionName))).intValue();
    }
    catch(Exception e){
      logMessage("An error occurred while reading the SOFIA Simulator Connection Parameters from the Initialization File. "+e);
    }
}
/*================================================================================================
/        initializeTelescopeCommands() - This method initializes the properties for the
/=================================================================================================*/
    public void initializeTelescopeCommands(){
      java.lang.String sectionName = new java.lang.String();
      sectionName = TELESCOPE_SECTION;
      try{  // WCS Properties
         DEFAULT_OFFSET          = myIniFile.getProperty("DEFAULT_OFFSET",sectionName);
         DEFAULT_RA_OFFSET       = myIniFile.getProperty("DEFAULT_RA_OFFSET",sectionName);
         DEFAULT__DEC_OFFSET     = myIniFile.getProperty("DEFAULT__DEC_OFFSET",sectionName);
         DEFAULT__RA_MOVE_RATE   = myIniFile.getProperty("DEFAULT__RA_MOVE_RATE",sectionName);
         DEFAULT__DEC_MOVE_RATE  = myIniFile.getProperty("DEFAULT__DEC_MOVE_RATE",sectionName);
         DEFAULT__FOCUS_OFFSET   = myIniFile.getProperty("DEFAULT__FOCUS_OFFSET",sectionName);
         FOCUS_MOVE_RATE         = myIniFile.getProperty("FOCUS_MOVE_RATE",sectionName);
         TELESCOPE_SETTLE_TIME   = myIniFile.getProperty("TELESCOPE_SETTLE_TIME",sectionName);
         POLLING_RATE            = myIniFile.getProperty("POLLING_RATE",sectionName);
      }
      catch(Exception e){
        logMessage("An error occurred while reading the Telescope Command Parameters from the Initialization File. "+e);
      }
  }
/*================================================================================================
/        initializeCatalog()
/=================================================================================================*/
  public void initializeCatalog(){
    java.lang.String sectionName = new java.lang.String();
    sectionName = CATALOG_SECTION;
    try{  // WCS Properties
          UCAC3PATH            = myIniFile.getProperty("UCAC3PATH",      sectionName);
          TYCHO2_INDEX         = myIniFile.getProperty("TYCHO2_INDEX",   sectionName);
          TYCHO2_CATALOG       = myIniFile.getProperty("TYCHO2_CATALOG", sectionName);
          DEFAULT_OBSERVER_DIR = myIniFile.getProperty("DEFAULT_OBSERVER_DIR", sectionName);
      }
    catch(Exception e){
      logMessage("An error occurred while reading the UCAC3 catalog location from the Initialization File. "+e);
    }
}
/*================================================================================================
/        initializeAllSkyImage()
/=================================================================================================*/
  public void initializeAllSkyImage(){
    java.lang.String sectionName = new java.lang.String();
    sectionName = ALLSKY_SECTION;
    try{  // WCS Properties
          RAW_MASK_DIAMETER    = (Integer.valueOf(myIniFile.getProperty("RAW_MASK_DIAMETER", sectionName))).intValue();
          OVERLAY_MASK_DIAMETER= (Integer.valueOf(myIniFile.getProperty("OVERLAY_MASK_DIAMETER", sectionName))).intValue();
          DECENTER_X           = (Double.valueOf(myIniFile.getProperty("DECENTER_X",    sectionName))).doubleValue();
          DECENTER_Y           = (Double.valueOf(myIniFile.getProperty("DECENTER_Y",    sectionName))).doubleValue();
          COEFFICIENT_A        = (Double.valueOf(myIniFile.getProperty("COEFFICIENT_A", sectionName))).doubleValue();
          COEFFICIENT_B        = (Double.valueOf(myIniFile.getProperty("COEFFICIENT_B", sectionName))).doubleValue();
          COEFFICIENT_C        = (Double.valueOf(myIniFile.getProperty("COEFFICIENT_C", sectionName))).doubleValue();
          ROTATION             = (Double.valueOf(myIniFile.getProperty("ROTATION",      sectionName))).doubleValue();
          ALLSKY_HOST          = myIniFile.getProperty(                      "ALLSKY_HOST",      sectionName);
          ALLSKY_PORT          = (Integer.valueOf(myIniFile.getProperty("ALLSKY_PORT", sectionName))).intValue();
          ALLSKY_PATH          = myIniFile.getProperty(                      "ALLSKY_PATH",      sectionName);
          ALLSKY_IMAGE_NAME    = myIniFile.getProperty(                      "ALLSKY_IMAGE_NAME",      sectionName);
          ALLSKY_POLLING_TIME  = (Integer.valueOf(myIniFile.getProperty("ALLSKY_POLLING_TIME", sectionName))).intValue();
     }
    catch(Exception e){
      logMessage("An error occurred while reading the AllSky calibration parameters from the Initialization File. "+e);
    }
}
/*================================================================================================
/         initializeFramePositions()
/=================================================================================================*/
  public void initializeFramePositions(){
     java.lang.String sectionName = new java.lang.String();
     sectionName = "POSITIONS";
     try{
         FACSUMFORM_X   = (Integer.valueOf(myIniFile.getProperty("FACSUMFORM_X", sectionName))).intValue();
         FACSUMFORM_Y   = (Integer.valueOf(myIniFile.getProperty("FACSUMFORM_Y", sectionName))).intValue();
         FACSUMFORM_VIS = (Boolean.valueOf(myIniFile.getProperty("FACSUMFORM_VIS", sectionName))).booleanValue();
         AIRMASS_X      = (Integer.valueOf(myIniFile.getProperty("AIRMASS_X", sectionName))).intValue();
         AIRMASS_Y      = (Integer.valueOf(myIniFile.getProperty("AIRMASS_Y", sectionName))).intValue();
         AIRMASS_VIS    = (Boolean.valueOf(myIniFile.getProperty("AIRMASS_VIS", sectionName))).booleanValue();
         EPHEMERIS_X    = (Integer.valueOf(myIniFile.getProperty("EPHEMERIS_X", sectionName))).intValue();
         EPHEMERIS_Y    = (Integer.valueOf(myIniFile.getProperty("EPHEMERIS_Y", sectionName))).intValue();
         EPHEMERIS_VIS  = (Boolean.valueOf(myIniFile.getProperty("EPHEMERIS_VIS", sectionName))).booleanValue();
         SKY_X          = (Integer.valueOf(myIniFile.getProperty("SKY_X", sectionName))).intValue();
         SKY_Y          = (Integer.valueOf(myIniFile.getProperty("SKY_Y", sectionName))).intValue();
         SKY_VIS        = (Boolean.valueOf(myIniFile.getProperty("SKY_VIS", sectionName))).booleanValue();
         ASTRO_OBJECT_X = (Integer.valueOf(myIniFile.getProperty("ASTRO_OBJECT_X", sectionName))).intValue();
         ASTRO_OBJECT_Y = (Integer.valueOf(myIniFile.getProperty("ASTRO_OBJECT_Y", sectionName))).intValue();
         ASTRO_OBJECT_VIS = (Boolean.valueOf(myIniFile.getProperty("ASTRO_OBJECT_VIS", sectionName))).booleanValue(); 
      }catch(Exception e){
      logMessage("An error occurred while reading the AllSky calibration parameters from the Initialization File. "+e);
    }
  }  
/*================================================================================================
/         IniFile Name
/=================================================================================================*/
  public void setIniFileName(String newIniFileName) {
    this.iniFileName = newIniFileName;
  }
  public String getIniFileName() {
    return iniFileName;
  }
/*=================================================================================
/          Set and Get Methods for the Initialization File
/=================================================================================*/
  public void setIniFile(java.io.File newIniFile) {
    iniFile = newIniFile;
   }
  public java.io.File getIniFile() {
    return iniFile;
  }
/*=================================================================================
/          logMessage Method  - not yet implemented
/=================================================================================*/
  public void logMessage(java.lang.String myMessage){
    System.out.println(myMessage);
  }
/*=================================================================================
/         finalize() method
/=================================================================================*/
  protected void finalize() throws Throwable {
      super.finalize();
  }
}
