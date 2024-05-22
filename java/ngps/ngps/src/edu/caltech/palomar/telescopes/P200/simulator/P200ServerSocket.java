/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */ 

package edu.caltech.palomar.telescopes.P200.simulator;
//=== File Prolog =============================================================
//	This code was developed by California Institute of Technology
//      Caltech Optical Observatories, Palomar Observatory
//
//--- Contents ----------------------------------------------------------------
//	 P200ServerSocket- This is the TCP/IP server socket for the P200 Simulator
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
 import javax.swing.UIManager;
 import jsky.science.Coordinates;
 import jsky.science.CoordinatesOffset;
 import java.util.StringTokenizer;
 import java.beans.*;
 import edu.caltech.palomar.telescopes.P200.TelescopeObject;
 import com.sharkysoft.printf.Printf;
 import com.sharkysoft.printf.PrintfData;
 import com.sharkysoft.printf.PrintfTemplate;
 import com.sharkysoft.printf.PrintfTemplateException;
 import java.util.*;
 import edu.caltech.palomar.util.general.TimeUtilities;
 import edu.dartmouth.jskycalc.JSkyCalcModel;
/*=============================================================================================
/         P200ServerSocket Class Declaration
/=============================================================================================*/
 public class P200ServerSocket extends edu.caltech.palomar.io.PalomarServerSocket{

  public static java.lang.String SUCCESS               = "0";                     //Successful completion
  public static java.lang.String UNRECOGNIZED          = "-1";                    //Unrecognized command.
  public static java.lang.String INVALID_PARAMETER     = "-2";                    //Invalid parameter(s).
  public static java.lang.String UNABLE_TO_EXECUTE     = "-3";                    //Unable to execute at this time.
  public static java.lang.String TCS_HOST_UNAVAILABLE  = "-4";                    //TCS Sparc host unavailable.
  public static java.lang.String RA_LABEL              = "RA = ";                 //RA label
  public static java.lang.String DEC_LABEL             = "DEC = ";                //DEC label
  public static java.lang.String HA_LABEL              = "HA = ";                 //HA label
  public static java.lang.String AIRMASS_LABEL         = "air mass = ";           //AIRMASS label
  public static java.lang.String TELESCOPE_ID_LABEL    = "telescope ID = ";       //Telescope ID label
  public static java.lang.String FOCUS_LABEL           = "focus = ";              //Focus label
  public static java.lang.String TUBE_LENGTH_LABEL     = "tube length = ";        //Tube Length label
  public static java.lang.String OFFSET_LABEL          = "offset ";               //Offset ID label
  public static java.lang.String RATE_LABEL            = "rate ";                 //Rate label
  public static java.lang.String CASS_RING_ANGLE_LABEL = "Cass ring angle = ";    //Cass Ring Angle label
  
   private double azimuth;
   private double hourAngle;
   private double zenithAngle;
   private double airMass;
   private java.lang.String rightAscension;
   private java.lang.String declination;
   private double DEC;
   private double RA;

   public static int SIMULATED_ZA       = 1;
   public static int SIMULATED_HA       = 2;
   public static int SIMULATED_AIRMASS  = 3;
   public static int SIMULATED_AZ       = 4;
   public static int SIMULATED_RA       = 5;
   public static int SIMULATED_DEC      = 6;

   private double    simulatorZA        = 0;
   private double    simulatorZADelta   = 0.01;
   private double    simulatorZARange   = 90.0;
   private double    simulatorZAOffset  = 45.0;

   private double    simulatorHA        = 0;
   private double    simulatorHADelta   = 0.01;
   private double    simulatorHARange   = 24.0;
   private double    simulatorHAOffset  = 0.0;

   private double    simulatorAIRMASS        = 0;
   private double    simulatorAIRMASSDelta   = 0.005;
   private double    simulatorAIRMASSRange   = 1.0;
   private double    simulatorAIRMASSOffset  = 1.0;

   private double    simulatorAZ        = 0;
   private double    simulatorAZDelta   = 0.01;
   private double    simulatorAZRange   = 360.0;
   private double    simulatorAZOffset  = 45.0;

   public TelescopeObject                  myTelescopeObject;
   private java.lang.String                TERMINATOR_N              = "\n";
   private java.lang.String                TERMINATOR_R              = "\r";
   java.lang.String RAString                  = new java.lang.String();
   java.lang.String DECString                 =  new java.lang.String();
   java.lang.String HAString                  =  new java.lang.String();
   java.lang.String LSTString                 =  new java.lang.String();
   java.lang.String AIRMASSString             =  new java.lang.String();
   java.lang.String AZIMUTHString             =  new java.lang.String();
   java.lang.String ZENITH_ANGLEString        =  new java.lang.String();
   java.lang.String FOCUSString               =  new java.lang.String();
   java.lang.String DOME_AZIMUTHString        =  new java.lang.String();
   java.lang.String DOME_SHUTTERSString       =  new java.lang.String();
   java.lang.String WINDSCREEN_POSITIONString =  new java.lang.String();
   java.lang.String INSTRUMENT_POSITIONString =  new java.lang.String();
   java.lang.String INSTRUMENT_String         =  new java.lang.String();
   java.lang.String PUMPS_String              =  new java.lang.String();
   java.lang.String PalomarUTC                = new java.lang.String();
   java.lang.String PalomarLST                = new java.lang.String();
    TimeUtilities   myTimeUtilities           = new TimeUtilities();
   java.lang.String response                  = new java.lang.String();
   JSkyCalcModel    myJSkyCalcModel;
    public    long SECOND_TO_MILLI_CONVERSION    = 1000;
/*=============================================================================================
/                                Constructors
/   public P200ServerSocket()
/   public P200ServerSocket(int newServerPort)
/=============================================================================================*/
/*=============================================================================================
/         P200ServerSocket()
/=============================================================================================*/
  public  P200ServerSocket() {
     super();
     setServerPort(DEFAULT_SERVERPORT);
     setTelescopeObject(new TelescopeObject());
     myTelescopeObject.intializeSimulatorValues();
     setLogging(true);
     RESPONSE_STRING = "OK";
  }
/*=============================================================================================
/          P200ServerSocket(int newServerPort) Constructors
/=============================================================================================*/
  public  P200ServerSocket(int newServerPort) {
     super(newServerPort); 
     setServerPort(newServerPort);
     setLogging(true);
     RESPONSE_STRING = "OK";
  }
/*=============================================================================================
/             P200ServerSocket(int newServerPort,TelescopeObject newTelescopeObject)
/=============================================================================================*/
  public  P200ServerSocket(int newServerPort,TelescopeObject newTelescopeObject) {
     super(newServerPort); 
     setServerPort(newServerPort);
     setTelescopeObject(newTelescopeObject);
     myTelescopeObject.intializeSimulatorValues();
     setLogging(true);
     RESPONSE_STRING = "OK";
  }
/*=============================================================================================
/             setTelescopeObject(TelescopeObject newTelescopeObject)
/=============================================================================================*/
  public  void setTelescopeObject(TelescopeObject newTelescopeObject) {
     myTelescopeObject = newTelescopeObject;
     myTelescopeObject.intializeSimulatorValues();
     setJSkyCalcModel(myTelescopeObject.getJSkyCalcModel());
  }
/*=============================================================================================
/            TelescopeObject getTelescopeObject()
/=============================================================================================*/
  public  TelescopeObject getTelescopeObject() {
     return myTelescopeObject;
  }
/*=============================================================================================
/            TelescopeObject getTelescopeObject()
/=============================================================================================*/
 public void updateSimulationValues(){
     myJSkyCalcModel.setTelescopePosition(myTelescopeObject.getRightAscension(), myTelescopeObject.getDeclination(), myTelescopeObject.getEquinox());
     myJSkyCalcModel.SetToNow();
 }
/*================================================================================================
/     setJSkyCalcModel(JSkyCalcModel newJSkyCalcModel)
/=================================================================================================*/
 public void setJSkyCalcModel(JSkyCalcModel newJSkyCalcModel){
     myJSkyCalcModel = newJSkyCalcModel;
     myJSkyCalcModel.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
      public void propertyChange(java.beans.PropertyChangeEvent e) {
          Ephemeris_propertyChange(e);
       }
     });
    }
/*=============================================================================================
/     Property Change Listener for the Ephemeris_propertyChange
/=============================================================================================*/
  private void Ephemeris_propertyChange(PropertyChangeEvent e)  {
     java.lang.String propertyName = e.getPropertyName();
/*=============================================================================================
/    PARAMETERS FROM THE JSKYCALCMODEL
/=============================================================================================*/
     if(propertyName == "siderialobj_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        myTelescopeObject.setSidereal(newValue);
    }
    if(propertyName == "ha_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        myTelescopeObject.setHourAngle(newValue);
    }
    if(propertyName == "airmass_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        myTelescopeObject.setAirMass(newValue);
    }
    if(propertyName == "ra_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
//        RAfield.setText(newValue);
    }
    if(propertyName == "dec_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
//        decfield.setText(newValue);
    }
    if(propertyName == "equinox_string"){
 //       java.lang.String newValue = (java.lang.String)e.getNewValue();
//        equinoxfield.setText(newValue);
    }
    if(propertyName == "azimuth_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        myTelescopeObject.setAzimuthString(newValue);
    }
    if(propertyName == "UTDateDate"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
//        utdatefield.setText(newValue);
    }
    if(propertyName == "UTDateTime"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
//        uttimefield.setText(newValue);
    }
    if(propertyName == "localDateDate"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
//        localdatefield.setText(newValue);
    }
    if(propertyName == "localDateTime"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
//        localtimefield.setText(newValue);
    }
    if(propertyName == "obsname"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        myTelescopeObject.setObjectName(newValue);
    }
  }
/*=============================================================================================
/          java.lang.String REQPOS() method
/=============================================================================================*/
 public java.lang.String REQPOS(){
      java.lang.String response = new java.lang.String();
      updateSimulationValues();
            /*----------------------------------------------------------------------------
            UTC = ddd hh:mm:ss.s, LST = hh:mm:ss.s\n
            RA = hh:mm:ss.ss, DEC = [+/-]dd:mm:ss.s, HA = [W/E]hh:mm:ss.s\n
            air mass = aa.aaa
            ---------------------------------------------------------------------------- */
               PalomarUTC      = myTimeUtilities.constructPalomarUTC();
               PalomarLST      = myTelescopeObject.getSidereal();
               System.out.println(PalomarLST);
               response = PalomarUTC + ", " + "LST = "+ PalomarLST + TERMINATOR_N
                       + RA_LABEL + myTelescopeObject.getRightAscension() + ", " + DEC_LABEL + myTelescopeObject.getDeclination() + ", " + HA_LABEL + myTelescopeObject.getHourAngle() + TERMINATOR_N
                       + AIRMASS_LABEL + myTelescopeObject.getAirMass();
             /*----------------------------------------------------------------------------
Example    // UTC = 216 04:34:38.1, LST = 17:37:45.4
           // RA = 17:57:57.69, DEC = +04:43:11.8, HA = E00:20:45.8
           // air mass =  1.143                                                                                                                                                  
            UTC = ddd hh:mm:ss.s,
            LST = hh:mm:ss.s\n
            RA = hh:mm:ss.ss,       myTelescopeObject
            DEC = [+/-]dd:mm:ss.s,
            HA = [W/E]hh:mm:ss.s\n
            air mass = aa.aaa
            ----------------------------------------------------------------------------*/
   return response;
 }
/*=============================================================================================
/          java.lang.String RESTAT() method
/=============================================================================================*/
 public java.lang.String RESTAT(){
      java.lang.String response = new java.lang.String();
             /*----------------------------------------------------------------------------
            UTC = ddd hh:mm:ss.s\n
            telescope ID = ttt, focus = ***** mm, tube length = tt.tt mm\n
            offset RA = rrrrrrr.r arcsec, DEC = ddddddd.d arcsec\n
            rate RA = rrrrrrr.r arcsec/hr, DEC = ddddddd.d arcsec/hr\n
            Cass ring angle = ccc.cc
            ----------------------------------------------------------------------------*/
            java.lang.String CASS_ANGLE = "0.000000";
            PalomarUTC      = myTimeUtilities.constructPalomarUTC();
            response = PalomarUTC + TERMINATOR_N
                    + TELESCOPE_ID_LABEL + myTelescopeObject.getTelescopeID() + ", "
                    + FOCUS_LABEL + myTelescopeObject.getTelescopeFocus() + " mm, " + TUBE_LENGTH_LABEL + myTelescopeObject.getTelescopeTubeLength() + " mm" + TERMINATOR_N
                    + OFFSET_LABEL  + RA_LABEL + myTelescopeObject.getRightAscensionOffset() + " arcsec, " + DEC_LABEL + myTelescopeObject.getDeclinationOffset() + " arcsec" + TERMINATOR_N
                    + RATE_LABEL + RA_LABEL + myTelescopeObject.getRightAscensionTrackRate() + " arcsec/hr, " + DEC_LABEL + myTelescopeObject.getDeclinationTrackRate() + " arcsec/hr" + TERMINATOR_N
                    + CASS_RING_ANGLE_LABEL + myTelescopeObject.getCassRingAngle();
            /*----------------------------------------------------------------------------
 Example:
            //  telescope ID = 200, focus = 36.71 mm, tube length = 22.11 mm
            //  offset RA =     137.3 arcsec, DEC =      95.8 arcsec
            //  rate RA =       0.0 arcsec/hr, DEC =       0.0 arcsec/hr
            //  Cass ring angle =  49.35                                        

            UTC = ddd hh:mm:ss.s\n
            telescope ID = ttt,
            focus = *****,
            tube length = tt.tt
            offset RA = rrrrrrr.r ,
            DEC = ddddddd.d
            rate RA = rrrrrrr.r ,
            DEC = ddddddd.d
            Cass ring angle = ccc.cc
            ----------------------------------------------------------------------------*/
   return response;
 }
/*=============================================================================================
/          java.lang.String WEATHER() method
/=============================================================================================*/
 public java.lang.String WEATHER(){
      java.lang.String response = new java.lang.String();
            /*?WEATHER (no arguments): does not provide weather information!  Was created
            to provide telescope information to the P200 weather system, acting as a
            client.  The format of the returned string is:
            ----------------------------------------------------------------------------
            RA=%05.2f\nDec=%+.2f\nHA=%+.2f\nLST=%05.2f\nAir mass=%06.3f\nAzimuth=%06.2f\n\
            Zenith angle=%05.2f\nFocus Point=%05.2f\nDome Azimuth=%05.1f\n\
            Dome shutters=%1d\nWindscreen position=%05.2f\nInstPos=%1d\nInstrument=%.12s\n\
            Pumps=%1d\n
            ----------------------------------------------------------------------------
            Apart from low precision telescope position information, the command indicates
            whether the dome shutters are closed (0) or open (1), dome and windscreen
            positions, and instrument name.
/           ------------------------------------------------------------------------------
            RA=                   %05.2f   \n
            Dec=                  %+.2f    \n
            HA=                   %+.2f    \n
            LST=                  %05.2f   \n
            Air mass=             %06.3f   \n
            Azimuth=              %06.2f   \n\
            Zenith angle=         %05.2f   \n
            Focus Point=          %05.2f   \n
            Dome Azimuth=         %05.1f   \n\
            Dome shutters=        %1d      \n
            Windscreen position=  %05.2f   \n
            InstPos=              %1d      \n
            Instrument=           %.12s    \n\
            Pumps=                %1d      \n
*           ---------------------------------------------------------------------------------*/
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

            RAString                  = Printf.format("%05.2f", new PrintfData().add(myTelescopeObject.getRA()));
            DECString                 = Printf.format("%+.2f",  new PrintfData().add(myTelescopeObject.getDEC()));
            HAString                  = Printf.format("%+.2f",  new PrintfData().add(myTelescopeObject.getHA()));
            LSTString                 = Printf.format("%s",     new PrintfData().add(myTelescopeObject.getSidereal()));
            AIRMASSString             = Printf.format("%06.3f", new PrintfData().add(myTelescopeObject.getAIRMASS()));
            AZIMUTHString             = Printf.format("%06.3f", new PrintfData().add(myTelescopeObject.getAzimuth()));
            ZENITH_ANGLEString        = Printf.format("%06.2f", new PrintfData().add(myTelescopeObject.getZenithAngle()));
            FOCUSString               = Printf.format("%05.2f", new PrintfData().add(myTelescopeObject.getFOCUS()));
            DOME_AZIMUTHString        = Printf.format("%05.1f", new PrintfData().add(myTelescopeObject.getDomeAzimuth()));
            DOME_SHUTTERSString       = Printf.format("%1d",    new PrintfData().add(myTelescopeObject.getDomeShutters()));
            WINDSCREEN_POSITIONString = Printf.format("%05.2f", new PrintfData().add(myTelescopeObject.getWindscreenPosition()));
            INSTRUMENT_POSITIONString = Printf.format("%1d",    new PrintfData().add(myTelescopeObject.getInstrumentPosition()));
            INSTRUMENT_String         = Printf.format("%.12s",  new PrintfData().add(myTelescopeObject.getInstrument()));
            PUMPS_String              = Printf.format("%.12s",  new PrintfData().add(myTelescopeObject.getPumps()));
            response = "RA="                 + RAString                  + TERMINATOR_N +
                       "Dec="                + DECString                 + TERMINATOR_N +
                       "HA="                 + HAString                  + TERMINATOR_N +
                       "LST="                + LSTString                 + TERMINATOR_N +
                       "Air mass="           + AIRMASSString             + TERMINATOR_N +
                       "Azimuth="            + AZIMUTHString             + TERMINATOR_N +
                       "Zenith angle="       + ZENITH_ANGLEString        + TERMINATOR_N +
                       "Focus Point="        + FOCUSString               + TERMINATOR_N +
                       "Dome Azimuth="       + DOME_AZIMUTHString        + TERMINATOR_N +
                       "Dome shutters="      + DOME_SHUTTERSString       + TERMINATOR_N +
                       "Windscreen position="+ WINDSCREEN_POSITIONString + TERMINATOR_N +
                       "InstPos="            + INSTRUMENT_POSITIONString + TERMINATOR_N +
                       "Instrument="         + INSTRUMENT_POSITIONString + TERMINATOR_N +
                       "Pumps="              + PUMPS_String + TERMINATOR_N;
             //The mean or apparent sidereal time locally is found by obtaining the local longitude in degrees,
            //converting it to hours by dividing by 15, and then adding it to or subtracting it from the
            //Greenwich time depending on whether the local position is east (add) or west (subtract) of Greenwich.
    return response;
 }
/*=============================================================================================
/          java.lang.String COORDS() method
/=============================================================================================*/
 public java.lang.String COORDS(String param1,String param2,String param3,String param4,String param5,String param6,String param7, String param8){
      response = SUCCESS;
   return response;
 }
/*=============================================================================================
/          java.lang.String F() method
/=============================================================================================*/
 public java.lang.String F(){
      response = SUCCESS;
   return response;
 }
 /*=============================================================================================
/          java.lang.String R() method
/=============================================================================================*/
 public java.lang.String R(){
      myTelescopeObject.setTracking(true);
      response = SUCCESS;
    return response;
 }
 /*=============================================================================================
/          java.lang.String mR() method
/=============================================================================================*/
 public java.lang.String mR(){
       myTelescopeObject.setTracking(false);
       response = SUCCESS;
   return response;
 }
 /*=============================================================================================
/          java.lang.String RCLEAR() method
/=============================================================================================*/
 public java.lang.String RCLEAR(){
      myTelescopeObject.setRA_TRACK_RATE(0);
      myTelescopeObject.setDEC_TRACK_RATE(0);
     response = SUCCESS;
   return response;
 }
 /*=============================================================================================
/          java.lang.String X() method
/=============================================================================================*/
 public java.lang.String X(){
      myTelescopeObject.setRA_OFFSET(0);
      myTelescopeObject.setDEC_OFFSET(0);
    response = SUCCESS;
   return response;
 }
/*=============================================================================================
/          java.lang.String TX() method
/=============================================================================================*/
 public java.lang.String TX(){
      response = SUCCESS;
   return response;
 }
/*=============================================================================================
/          java.lang.String Z() method
/=============================================================================================*/
 public java.lang.String Z(){
       myTelescopeObject.setRA_OFFSET(0);
       myTelescopeObject.setDEC_OFFSET(0);
       myTelescopeObject.setDeclinationOffset("0.0");
       myTelescopeObject.setRightAscensionOffset("0.0");
       response = SUCCESS;
   return response;
 }
/*=============================================================================================
/          java.lang.String RET() method
/=============================================================================================*/
 public java.lang.String RET(){
     double waitTime = myTelescopeObject.CalculateTelescopeMoveTimeMaxRate(-myTelescopeObject.getRA_OFFSET(),-myTelescopeObject.getDEC_OFFSET());
     System.out.println(waitTime);
     wait((int)(waitTime*SECOND_TO_MILLI_CONVERSION));
     myTelescopeObject.getCoordinates().translate(-myTelescopeObject.getRA_OFFSET(),-myTelescopeObject.getDEC_OFFSET());
     myTelescopeObject.updateCoordinates();
     response = SUCCESS;
   return response;
 }
/*================================================================================================
/      wait(int newDelay)
/=================================================================================================*/
public void wait(int newDelay){
     try{ 
        Thread.currentThread().sleep(newDelay);} // sleep for awhile to let the messages pile up
      catch (Exception e){
    }
}
/*=============================================================================================
/          java.lang.String E() method
/=============================================================================================*/
 public java.lang.String E(java.lang.String parameter1){
     java.lang.Double myDouble = Double.valueOf(parameter1);
     double param1 = myDouble.doubleValue();
     if(param1 < 0 | param1 > 6000){
        response = INVALID_PARAMETER;
        return response;
     }  // since it's an offset in the eastern direction it is a positive offset
     double waitTime = myTelescopeObject.CalculateTelescopeMoveTime(param1,0.0);
     System.out.println(waitTime);
     wait((int)(waitTime*SECOND_TO_MILLI_CONVERSION));
     myTelescopeObject.getCoordinates().translate(param1, 0);
     myTelescopeObject.updateCoordinates();
   response = SUCCESS;
   return response;
 }
/*=============================================================================================
/          java.lang.String W() method
/=============================================================================================*/
 public java.lang.String W(java.lang.String parameter1){
    java.lang.Double myDouble = Double.valueOf(parameter1);
     double param1 = myDouble.doubleValue();
     if(param1 < 0 | param1 > 6000){
        response = INVALID_PARAMETER;
        return response;
     }  // since it's an offset in the eastern direction it is a positive offset
     double waitTime = myTelescopeObject.CalculateTelescopeMoveTime(param1,0.0);
     System.out.println(waitTime);
     wait((int)(waitTime*SECOND_TO_MILLI_CONVERSION));
     myTelescopeObject.getCoordinates().translate(-param1, 0);
     myTelescopeObject.updateCoordinates();
  response = SUCCESS;
 return response;
 }
/*=============================================================================================
/          java.lang.String N(java.lang.String parameter1) method
/=============================================================================================*/
 public java.lang.String N(java.lang.String parameter1){
    java.lang.Double myDouble = Double.valueOf(parameter1);
     double param1 = myDouble.doubleValue();
     if(param1 < 0 | param1 > 6000){
        response = INVALID_PARAMETER;
        return response;
     }  // since it's an offset in the eastern direction it is a positive offset
     double waitTime = myTelescopeObject.CalculateTelescopeMoveTime(param1,0.0);
     System.out.println(waitTime);
     wait((int)(waitTime*SECOND_TO_MILLI_CONVERSION));
    myTelescopeObject.getCoordinates().translate(0, param1);
    myTelescopeObject.updateCoordinates();
    response = SUCCESS;
  return response;
}
/*=============================================================================================
/          java.lang.String S(java.lang.String parameter1) method
/=============================================================================================*/
 public java.lang.String S(java.lang.String parameter1){
    java.lang.Double myDouble = Double.valueOf(parameter1);
     double param1 = myDouble.doubleValue();
     if(param1 < 0 | param1 > 6000){
        response = INVALID_PARAMETER;
        return response;
     }  // since it's an offset in the eastern direction it is a positive offset
     double waitTime = myTelescopeObject.CalculateTelescopeMoveTime(param1,0.0);
     System.out.println(waitTime);
     wait((int)(waitTime*SECOND_TO_MILLI_CONVERSION));
      myTelescopeObject.getCoordinates().translate(0, -param1);
      myTelescopeObject.updateCoordinates();
    response = SUCCESS;
  return response;
 }
 /*=============================================================================================
/          java.lang.String ES() method
/=============================================================================================*/
 public java.lang.String ES(java.lang.String parameter1){
       java.lang.Integer myInt = Integer.valueOf(parameter1);
       int param1 = myInt.intValue();
       param1 = param1*10;
       if(param1 < 0 | param1 > 6000){
          response = INVALID_PARAMETER;
         return response;
       }  // since it's an offset in the eastern direction it is a positive offset
     double waitTime = myTelescopeObject.CalculateTelescopeMoveTime(param1,0.0);
     System.out.println(waitTime);
     wait((int)(waitTime*SECOND_TO_MILLI_CONVERSION));
     myTelescopeObject.getCoordinates().translate(param1, 0);
     myTelescopeObject.updateCoordinates();
     response = SUCCESS;
  return response;
 }
/*=============================================================================================
/          java.lang.String WS() method
/=============================================================================================*/
 public java.lang.String WS(java.lang.String parameter1){
     java.lang.Integer myInt = Integer.valueOf(parameter1);
     int param1 = myInt.intValue();
         param1 = param1*10;
         if(param1 < 0 | param1 > 6000){
           response = INVALID_PARAMETER;
         return response;
         }  // since it's an offset in the western direction it is a negative offset
     double waitTime = myTelescopeObject.CalculateTelescopeMoveTime(param1,0.0);
     System.out.println(waitTime);
     wait((int)(waitTime*SECOND_TO_MILLI_CONVERSION));
      myTelescopeObject.getCoordinates().translate(-param1, 0);
      myTelescopeObject.updateCoordinates();
    response = SUCCESS;
   return response;
 }
/*=============================================================================================
/          java.lang.String FOCUSGO() method
/=============================================================================================*/
 public java.lang.String FOCUSGO(java.lang.String parameter1){
     java.lang.Double myDouble = Double.valueOf(parameter1);
     double param1 = myDouble.doubleValue();
     if(param1 < 0.0 | param1 > 74.0){
        response = INVALID_PARAMETER;
     return response;
     }  // This is a fixed focus value to go to not an offset
     myTelescopeObject.setFOCUS(param1);
     response = SUCCESS;
    return response;
}
/*=============================================================================================
/          java.lang.String FOCUSINC(java.lang.String parameter1) method
/=============================================================================================*/
 public java.lang.String FOCUSINC(java.lang.String parameter1){
       java.lang.Double myDouble = Double.valueOf(parameter1);
       double param1 = myDouble.doubleValue();
       // first check that the parameter is within the valid range for the mechanism
       if(param1 < -73.0 | param1 > 73.0){
         response = INVALID_PARAMETER;
         return response;
       }
       // now do a limit check to make sure that the offset won't put the focus beyond the physical limits
       double currentPosition = myTelescopeObject.getFOCUS();
       param1 = param1+currentPosition;
       if(param1 < 0.0 | param1 > 74.0){
         response = INVALID_PARAMETER;
         return response;
       }  // This is a fixed focus value to go to not an offset
    myTelescopeObject.setFOCUS(param1);
    response = SUCCESS;
  return response;
 }
 /*=============================================================================================
/          java.lang.String GHEAD(java.lang.String parameter1) method
/=============================================================================================*/
 public java.lang.String GHEAD(java.lang.String parameter1){
     java.lang.Double myDouble = Double.valueOf(parameter1);
     double param1 = myDouble.doubleValue();
     if(param1 < 0.0 | param1 > 360.0){
       response = INVALID_PARAMETER;
       return response;
     }  // This is a fixed focus value to go to not an offset
   response = SUCCESS;
   return response;
 }
/*=============================================================================================
/          java.lang.String PT() method
/=============================================================================================*/
 public java.lang.String PT(java.lang.String parameter1,java.lang.String parameter2){
               java.lang.Integer myInteger1 = Integer.valueOf(parameter1);
               java.lang.Integer myInteger2 = Integer.valueOf(parameter2);
               int param1 = myInteger1.intValue();
               int param2 = myInteger2.intValue();
               if(param1 < -6000 | param1 > 6000 | param2 < -6000 | param2 > 6000){
                  response = INVALID_PARAMETER;
                  return response;
               }  // This is a fixed focus value to go to not an offset
               myTelescopeObject.getCoordinates().translate(param1,param2);
               myTelescopeObject.updateCoordinates();
               response = SUCCESS;
   return response;
 }
 /*=============================================================================================
/          java.lang.String PTS() method
/=============================================================================================*/
 public java.lang.String PTS(java.lang.String parameter1,java.lang.String parameter2){
     java.lang.Integer myInteger1 = Integer.valueOf(parameter1);
     java.lang.Integer myInteger2 = Integer.valueOf(parameter2);
     int param1 = myInteger1.intValue();
     int param2 = myInteger2.intValue();
     if(param1 < -600 | param1 > 600 | param2 < -600 | param2 > 600){
        response = INVALID_PARAMETER;
        return response;
     }  // This is a fixed focus value to go to not an offset
     myTelescopeObject.getCoordinates().translate(param1*10,param2*10);
     myTelescopeObject.updateCoordinates();
     response = SUCCESS;
   return response;
 }
/*=============================================================================================
/          java.lang.String RATES() method
/=============================================================================================*/
 public java.lang.String RATES(java.lang.String parameter1,java.lang.String parameter2){
     java.lang.Integer myInteger1 = Integer.valueOf(parameter1);
     java.lang.Integer myInteger2 = Integer.valueOf(parameter2);
     int param1 = myInteger1.intValue();
     int param2 = myInteger2.intValue();
     if(param1 < 0 | param1 > 50 | param2 < 0 | param2 > 50){
        response = INVALID_PARAMETER;
        return response;
     }
   myTelescopeObject.setRA_TRACK_RATE(param1);
   myTelescopeObject.setDEC_TRACK_RATE(param2);
   response = SUCCESS;
  return response;
 }
/*=============================================================================================
/          java.lang.String RATESS() method
/=============================================================================================*/
 public java.lang.String RATESS(java.lang.String parameter1,java.lang.String parameter2){
     java.lang.Integer myInteger1 = Integer.valueOf(parameter1);
     java.lang.Integer myInteger2 = Integer.valueOf(parameter2);
     int param1 = myInteger1.intValue();
     int param2 = myInteger2.intValue();
     if(param1 < 0 | param1 > 5 | param2 < 0 | param2 > 5){
        response = INVALID_PARAMETER;
        return response;
     }
     myTelescopeObject.setRA_TRACK_RATE(param1*10);
     myTelescopeObject.setDEC_TRACK_RATE(param2*10);
   response = SUCCESS;
   return response;
 }
 /*=============================================================================================
/          java.lang.String MRATES() method
/=============================================================================================*/
 public java.lang.String MRATES(java.lang.String parameter1,java.lang.String parameter2){
   java.lang.Integer myDouble1 = Integer.valueOf(parameter1);
   java.lang.Integer myDouble2 = Integer.valueOf(parameter2);
   double param1 = myDouble1.doubleValue();
   double param2 = myDouble2.doubleValue();
   if(param1 < 0 | param1 > TelescopeObject.MAX_RA_MOVE_RATE | param2 < 0 | param2 > TelescopeObject.MAX_DEC_MOVE_RATE){
      response = INVALID_PARAMETER;
      return response;
   }
   myTelescopeObject.setRAMoveRate(param1*10);
   myTelescopeObject.setDecMoveRate(param2*10);
   response = SUCCESS;
  return response;
 }
 /*=============================================================================================
/          java.lang.String GUIDE() method
/=============================================================================================*/
 public java.lang.String GUIDE(java.lang.String parameter1,java.lang.String parameter2){
     java.lang.Integer myInteger1 = Integer.valueOf(parameter1);
     java.lang.Integer myInteger2 = Integer.valueOf(parameter2);
     int param1 = myInteger1.intValue();
     int param2 = myInteger2.intValue();
     if(param1 < 0 | param1 > 50 | param2 < 0 | param2 > 50){
        response = INVALID_PARAMETER;
        return response;
     }
     myTelescopeObject.setRA_GUIDE_RATE(param1*10);
     myTelescopeObject.setDEC_GUIDE_RATE(param2*10);
     response = SUCCESS;
   return response;
 }
/*=============================================================================================
/          java.lang.String SET() method
/=============================================================================================*/
 public java.lang.String SET(java.lang.String parameter1,java.lang.String parameter2){
    java.lang.Integer myInteger1 = Integer.valueOf(parameter1);
    java.lang.Integer myInteger2 = Integer.valueOf(parameter2);
    int param1 = myInteger1.intValue();
    int param2 = myInteger2.intValue();
    if(param1 < 0 | param1 > 50 | param2 < 0 | param2 > 50){
         response = INVALID_PARAMETER;
         return response;
    }
    myTelescopeObject.setRA_SET_RATE(param1*10);
    myTelescopeObject.setDEC_SET_RATE(param2*10);
  response = SUCCESS;
  return response;
 }
/*=============================================================================================
/          ABORT() Method Not yet implemented on the actual P200 Telescope
/=============================================================================================*/
 public java.lang.String ABORT(){
   response = SUCCESS;
  return response;
 }
/*=============================================================================================
/         MOTION()
/=============================================================================================*/
 public java.lang.String MOTION(){
     java.lang.String response = new java.lang.String();
     int motion = myTelescopeObject.getMotionStatus();
     if(motion == TelescopeObject.MOTION_STOPPED){
         response = java.lang.Integer.toString(TelescopeObject.MOTION_STOPPED) + TERMINATOR_N;
     }
     if(motion == TelescopeObject.MOTION_SLEWING){
         response = java.lang.Integer.toString(TelescopeObject.MOTION_SLEWING) + TERMINATOR_N;
     }
     if(motion == TelescopeObject.MOTION_OFFSETTING){
         response = java.lang.Integer.toString(TelescopeObject.MOTION_OFFSETTING) + TERMINATOR_N;
     }
     if(motion == TelescopeObject.MOTION_TRACKING_STABLY){
         response = java.lang.Integer.toString(TelescopeObject.MOTION_TRACKING_STABLY) + TERMINATOR_N;
     }
     if(motion == TelescopeObject.MOTION_SETTLING){
         response = java.lang.Integer.toString(TelescopeObject.MOTION_SETTLING) + TERMINATOR_N;
     }
  return response;  
 }
/*=============================================================================================
/               prepareResponse(java.lang.String commandString) method
/=============================================================================================*/
  public java.lang.String prepareResponse(java.lang.String commandString){
      // This is the method that is over-ridden by classes that extend the Server Socket
      // This method is called when a command string is received from a client and this is
      // where the parsing of the command string and the preparation of the response takes place

      java.lang.String response   = new java.lang.String();
      java.lang.String params     = new java.lang.String();
      java.lang.String parameter1 = new java.lang.String();
      java.lang.String parameter2 = new java.lang.String();
      java.lang.String parameter3 = new java.lang.String();
      java.lang.String parameter4 = new java.lang.String();
      java.lang.String parameter5 = new java.lang.String();
      java.lang.String parameter6 = new java.lang.String();
      java.lang.String parameter7 = new java.lang.String();
      java.lang.String parameter8 = new java.lang.String();
      java.lang.String command    = new java.lang.String();
      java.util.StringTokenizer st = new java.util.StringTokenizer(commandString);
      int tokenCount = st.countTokens();
          command    = st.nextToken(" "); // This places the first token in "command"
          command.toUpperCase();
          // this makes sure that we are dealing with the capitalized version
      // The following section parses the incoming command and constructs the response
      switch(tokenCount){
      case 0:{
          // This is the error case where this is an invalid string

          break;}
      case 1:{
           if(command.startsWith("?")){
             command = command.substring(1,command.length());
           }
          //This case contains all of the commands that have no arguements
           if("REQPOS".matches(command)){ // Request telescope position
               response = REQPOS();
           }
           if("REQSTAT".matches(command)){ // Request telescope status
               response = RESTAT();
           }
           if("WEATHER".matches(command)){ // Request telescope weather
               response = WEATHER();
           }
           if("F".matches(command)){ // Moves telescope to HOME, the last position established with GO
               response = F();
           }
           if("R".matches(command)){ // Turns on Rate Tracking
               response = R();
           }
           if("-R".matches(command)){ // Turns off Rate Tracking
               response = mR();
           }
            if("RCLEAR".matches(command)){ // Turns off Rate Tracking and sets offset rates to zero
              response = RCLEAR();
           }
           if("X".matches(command)){ // Sets the current position to be HOME
              response = X();
           }
           if("TX".matches(command)){ // Temporarily sets the current position to HOME
              response = TX();
           }
           if("Z".matches(command)){ // Sets the offset to zero
               response = Z();
          }
           if("RET".matches(command)){ // Moves the telescope so that the offsets are zero, same as going home
               response = RET();
           }
           if("ABORT".matches(command)){ // Moves the telescope so that the offsets are zero, same as going home
               response = ABORT();
           }
           if("MOTION".matches(command)){
               response = MOTION();
           }
           if("NAME".matches(command)){
               response = myTelescopeObject.getObjectName() + TERMINATOR_N;
           }
           break;}
      case 2:{
          params = st.nextToken();
          // This is the case containing all of the commands with 1 arguement
           if("E".matches(command)){ // Moves the telescope east by the amount of the next token in arcseconds
               response = E(params);
           }
           if("W".matches(command)){ // Moves the telescope east by the amount of the next token in arcseconds
               response = W(params);
           }
           if("N".matches(command)){ // Moves the telescope north by the amount of the next token in arcseconds
               response = N(params);
           }
           if("S".matches(command)){ // Moves the telescope south by the amount of the next token in arcseconds
               response = S(params);
           }
           if("ES".matches(command)){ // Moves the telescope east by the amount of the next token in seconds rather than arcseconds
               response = ES(params);
           }
           if("WS".matches(command)){ // Moves the telescope east by the amount of the next token in seconds rather than arcseconds
               response = WS(params);
           }
           if("FOCUSGO".matches(command)){ // This changes the focus to the value of the included parameter
               response = FOCUSGO(params);
           }
           if("FOCUSINC".matches(command)){ // This changes the focus to the value of the included parameter
               response = FOCUSINC(params);
           }
           if("GHEAD".matches(command)){ // WARNING THIS PARAMETER ISN"T IN THE TELESCOPE OBJECT MODEL
               response = FOCUSINC(params);
           }
           if("COORDS".matches(command)){ // WARNING THIS PARAMETER ISN"T IN THE TELESCOPE OBJECT MODEL
                  java.util.StringTokenizer st2 = new java.util.StringTokenizer(params,",");
                  parameter1 = st2.nextToken();
                  parameter2 = st2.nextToken();
                  parameter3 = st2.nextToken();
                  parameter4 = st2.nextToken();
                  parameter5 = st2.nextToken();
                  parameter6 = st2.nextToken();
                  parameter7 = st2.nextToken();
               response = COORDS(parameter1,parameter2,parameter3,parameter4,parameter5,parameter6,parameter7,parameter8);
           }
          break;}
      case 3:{
          // This is the case containing all of the commands with 2 arguements
          parameter1 = st.nextToken();
          parameter2 = st.nextToken();
           if("PT".matches(command)){ // WARNING THIS PARAMETER ISN"T IN THE TELESCOPE OBJECT MODEL
               response = PT(parameter1,parameter2);
           }
           if("PTS".matches(command)){ // 
               response = PTS(parameter1,parameter2);
           }
           if("RATES".matches(command)){ // NOTE! The tracking rate limits are not recorded in the file. These are made up
           }
           if("RATESS".matches(command)){ // NOTE! The tracking rate limits are not recorded in the file. These are made up
           }
           if("MRATES".matches(command)){ // NOTE! The tracking rate limits are not recorded in the file. These are made up
            }
           if("GUIDE".matches(command)){ // NOTE! The tracking rate limits are not recorded in the file. These are made up
           }
           if("SET".matches(command)){ // NOTE! The tracking rate limits are not recorded in the file. These are made up
           }
          break;}
    }
   return response;
  }
/*=============================================================================================
/               simulationUpdate() method
/=============================================================================================*/
  public void simulationUpdate(){
      // This is the method that is over-ridden by classes that extend the Server Socket
      // This method is called within the SimulationThread and is used to allow internal
      // parameters to be changed and update in a systematic fashion.  Place any slowly
      // varying values that are not deterministic to be updated in this method.  For example
      // the airmass, zenith angle, hour angle can be gradually changed so they cover the
      // entire range of possible values returned by a server.
  }
/*================================================================================================
/       getCurrentCoordinates(double newRA,double newDec)
/=================================================================================================*/
      public Coordinates getCurrentCoordinates(double newRA,double newDec){
                Coordinates      myCoordinate  = null;
                Coordinates      configuredCoordinates = new Coordinates();
                // construct the command
                    try{
                        configuredCoordinates.setRa(newRA);
                        configuredCoordinates.setDec(newDec);
                        configuredCoordinates.setEquinox(Coordinates.J2000_EQUINOX);
                        myCoordinate = configuredCoordinates;
                    }catch(NumberFormatException nfe){
                       logErrorMessage("Number Format Exception creating a Coordinate Object Method");
                    }
          return myCoordinate;
      }
/*================================================================================================
/       evaluateMoveAccuracy()
/=================================================================================================*/
          public void evaluateMoveAccuracy(double x,double y,Coordinates startCoordinates,Coordinates endCoordinates){
            double maxRAerror  = 5.00;
            double maxDecerror = 5.00;
            if((startCoordinates != null)&(endCoordinates != null)){
            CoordinatesOffset myCoordinatesOffset = endCoordinates.subtract(startCoordinates);
                double myDeltaRA  = myCoordinatesOffset.getRa(Coordinates.DEGREE);
                double myDeltaDec = myCoordinatesOffset.getDec(Coordinates.DEGREE);
                double errorRA    = (double)x - myDeltaRA;
                double errorDec   = (double)y - myDeltaDec;
            logErrorMessage("Change in RA measured = "  + myDeltaRA  + " Error in move = " + errorRA);
            logErrorMessage("Change in Dec measured = " + myDeltaDec + " Error in move = " + errorDec);
            boolean errorFlag = false;
                if(x != 0){
                  if(Math.abs(errorRA) > maxRAerror){
                    errorFlag = true;
                  }
                }
                if(y != 0){
                  if(Math.abs(errorDec) > maxDecerror){
                    errorFlag = true;
                  }
                }
              if(errorFlag){
                logErrorMessage("Telescope Move was unsuccessful");
              }// end of firing the error report action
            }// end of evaluation if the coordinates are NOT null
          }// end of the method
/*================================================================================================
/        java.lang.String doubleToRAString(double newDoubleRA)
/=================================================================================================*/
      public java.lang.String doubleToRAString(double newDoubleRA){
        java.lang.String myRAString = new java.lang.String();
        Coordinates      configuredCoordinates = new Coordinates();
                         configuredCoordinates.setSeparatorStyle(Coordinates.COLON_SEPARATOR_STYLE);
                         configuredCoordinates.setRa(newDoubleRA);
               myRAString = configuredCoordinates.raToString();
        return myRAString;
      }
/*================================================================================================
/        java.lang.String doubleToDECString(double newDoubleDEC)
/=================================================================================================*/
        public java.lang.String doubleToDECString(double newDoubleDEC){
          java.lang.String myDECString = new java.lang.String();
          Coordinates      configuredCoordinates = new Coordinates();
                           configuredCoordinates.setSeparatorStyle(Coordinates.COLON_SEPARATOR_STYLE);
                           configuredCoordinates.setDec(newDoubleDEC);
                myDECString = configuredCoordinates.decToString();
          return myDECString;
        }
/*================================================================================================
/     double telescopeSimulator(int telescopeParameter)
/=================================================================================================*/
      public double telescopeSimulator(int telescopeParameter){
               double simulator = 0;
               if(telescopeParameter ==  SIMULATED_ZA){
                 simulatorZA = (simulatorZA + simulatorZADelta);
                 simulator  = (Math.sin(simulatorZA))*simulatorZARange + simulatorZAOffset;
               }
               if(telescopeParameter ==  SIMULATED_HA){
                 simulatorHA = (simulatorHA + simulatorHADelta);
                 simulator  = (Math.sin(simulatorHA))*simulatorHARange + simulatorHAOffset;
               }
               if(telescopeParameter ==  SIMULATED_AIRMASS){
                 simulatorAIRMASS = (simulatorAIRMASS + simulatorAIRMASSDelta);
                 simulator  = (Math.sin(simulatorAIRMASS))*simulatorAIRMASSRange + simulatorAIRMASSOffset;
               }
               if(telescopeParameter ==  SIMULATED_AZ){
                 simulatorAZ = (simulatorAZ + simulatorAZDelta);
                 simulator  = (Math.sin(simulatorAZ))*simulatorAZRange + simulatorAZOffset;
               }
             return simulator;
        }
/*================================================================================================
/       Documentation on the TCP/IP interface for the P200 Hale Telescope Control System
/=================================================================================================*//*

 REMOTE COMMANDS FOR THE PALOMAR TELESCOPE CONTROL SYSTEMS (TCS)    10/07/08


OVERVIEW

Remote commands are those commands that can be issued by another computer via
serial link or stream socket to the Telescope Control System (TCS).  The
commands and their functions are listed below.

The command format is an ASCII string starting with the command name, followed
by the correct number of arguments and terminated with a '\r' (carriage-return
character, 015 octal).  Spaces are used to delimit fields.  Numeric arguments
may be integer or floating-point.  Commands may be entered in upper or lower
case.



REMOTE COMMANDS

(null command, '\r' only):  Get-header command.  (See also REQPOS and REQSTAT
commands below.)  Assembles current telescope data as native DEC (Digital
Equipment Corp) binary numbers and returns to the caller 100 8-bit bytes.
These are NOT ASCII characters, and must be interpreted as PDP11-format
numbers ("shorts" are two byte short integer, LSB first;  and "floats" are
modified IEEE format: MSB-1, MSB, LSB, LSB+1.  In other words, each 16-bit
word is byte-swapped).  A "C" declaration for the data structure that is
returned is provided below:

struct {				/ 100 BYTES TOTAL /
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
} telescope_data;

?NAME (no arguments): return the name of the currently observed position
(the last position for which a GO command has been issued).

?PARALLACTIC (no arguments):  return the current telescope parallactic
angle in ASCII format, as follows: "PARALLACTIC = %.2f\n".  This is intended
for the Double Spectrograph instrument.

?WEATHER (no arguments): does not provide weather information!  Was created
to provide telescope information to the P200 weather system, acting as a
client.  The format of the returned string is:
----------------------------------------------------------------------------
RA=%05.2f\nDec=%+.2f\nHA=%+.2f\nLST=%05.2f\nAir mass=%06.3f\nAzimuth=%06.2f\n\
Zenith angle=%05.2f\nFocus Point=%05.2f\nDome Azimuth=%05.1f\n\
Dome shutters=%1d\nWindscreen position=%05.2f\nInstPos=%1d\nInstrument=%.12s\n\
Pumps=%1d\n
----------------------------------------------------------------------------
Apart from low precision telescope position information, the command indicates
whether the dome shutters are closed (0) or open (1), dome and windscreen
positions, and instrument name.

COORDS (five or six arguments):  supply a position to the TCS.  The arguments
in order are  1) RA in decimal hours, 2) declination in decimal degrees,
3) equinox of coordinates (value of zero indicates apparent equinox), 4) RA
motion, 5) dec motion, 6) motion flag.  (Motion flag absent or 0:  arguments
4 and 5 are proper motion in RA units of .0001 sec/yr and dec units of .001
arcsec/yr.  Motion flag = 1:  arguments 4 and 5 are offset tracking rates in
arcsec/hr.  Motion flag = 2:  arguments 4 and 5 are offset tracking rates
with arg 4 in seconds/hr and arg 5 in arcsec/hr.)  NOTE: a name string can
now be provided, by adding an extra parameter enclosed in double quotes;
this will preferably be at the end of the command line, though it can be
inserted at any parameter position.

E (one argument):  move telescope east by number of arcseconds given in
argument.  Rate of move is set by first argument of MRATES - see below.
Move is precessed to display equinox.  Valid range is 0 to 6000 arcsec.

ES (one argument):  move telescope east by seconds of time given in
argument.  Rate of move is set by first argument of MRATES - see below.
Move is precessed to display equinox.  Valid range: 0 to 600 seconds.

F (no arguments):  move the telescope to the position established by the last
GO command (GO can only be issued from the Night Assistant's console).  This
move is done at 50 arcsec/sec.  Maximum move is 600 sec in RA and 6000 arcsec
in dec.  The telescope must be tracking stably.

FOCUSGO (one argument):  change telescope focus to the value given in the
argument.  Valid range: 1.00 to 74.00 mm. (P200 only)

FOCUSINC (one argument):  change telescope focus by the offset given in the
argument.  Valid range: -73.00 to 73.00 mm.  Limit checking is done.
(P200 only)

GHEAD (one argument):  for instruments with internal mechanisms that
affect the orientation of North on the video guider display, this command
provides a way for the instrument computer to notify the TCS of changes
in orientation.  Valid range: 0 to 360 degrees.

GUIDE (two arguments):  set the RA and dec rates for the handpaddle Guide
buttons.  Valid range: 0 to 50 arcsec/sec.

MRATES (two arguments):  set rate of RA and dec moves to values given in
first and second arguments, respectively.  Valid range: 0 to 50 arcsec/sec.
Note:  these rates apply to moves as described in this note and are
independent of rates for Night-Assistant-commanded moves.

N (one argument):  move telescope north by number of arcseconds given in
argument.  Rate of move is set by second argument of MRATES - see above.
Move is precessed to display equinox.  Valid range: 0 to 6000 arcsec.

NPS (two arguments): control power to lamps and LWIR camera in prime focus
cage.  First argument: 0 = turn off port given in second argument; 1 =
turn on port; 2 = get status of port ("OFF" or "ON ").  Special case:
NPS 0 0 turns off all lamps (ports 1, 2, 3).  Port allocations: 1 = low lamp;
2 = high lamp; 3 = arclamp; 7 = LWIR camera; 8 = LWIR camera shutter.

PT (two arguments):  move telescope in RA and dec simultaneously by
distance given in first and second arguments, respectively.  Distances are
in arcseconds.  Positive values move east and north.  Rates of moves are
set by MRATES - see above.  Moves are precessed to the display equinox.
Valid range: -6000 to 6000 arcsec in each axis.

PTS (two arguments):  move telescope in RA and dec simultaneously by
distance given in first and second arguments, respectively.  RA distance is
in seconds of time (range: -600 to 600 sec), dec is in arcseconds (range:
-6000 to 6000 arcsec).  Positive values move east and north.  Rates of moves
are set by MRATES - see above.  Moves are precessed to the display equinox.

R (no arguments):  turn on offset tracking rates (non-sidereal).  The rates to
be used may be entered at the Night Assistant's console or with the RATES or
RATESS commands - see below.

-R (no arguments):  turn off offset tracking rates.

RATES (two arguments):  define offsets to sidereal tracking rates for non-
sidereal objects.  First argument is RA rate in arcsec/hr, second argument
is declination rate in arcsec/hr.  Rates are enabled with the R command -
see above.

RATESS (two arguments):  define offsets to sidereal tracking rates for non-
sidereal objects.  First argument is RA rate in seconds/hr, second argument
is declination rate in arcsec/hr.  Rates are enabled with the R command -
see above.

RAWDEC (no arguments):  returns the encoder value of declination in ASCII
format, as follows: "RAW_DEC = %.4f\n".  This is intended for LGS operations.

RAWPOS (no arguments):  returns the encoder values of hour angle (in hours)
and declination (in degrees) in ASCII format, as follows:
"HA= %.6f\nDEC= %.5f\n".  This is intended for LGS operations.

RCLEAR (no arguments):  disable offset tracking rates and set offset rates
to zero.

REQPOS (no arguments):  obtain telescope position information in ASCII
format.  Format of returned string (dashed lines not included):
----------------------------------------------------------------------------
UTC = ddd hh:mm:ss.s, LST = hh:mm:ss.s\n
RA = hh:mm:ss.ss, DEC = [+/-]dd:mm:ss.s, HA = [W/E]hh:mm:ss.s\n
air mass = aa.aaa
----------------------------------------------------------------------------
In UTC, ddd is UT day of year.  Fields are fixed width.  RA and DEC are
J2000.  HA is apparent.  There is no terminating newline.  (See also Get-
header command above.)

REQSTAT (no arguments): obtain telescope status information in ASCII
format.  Format of returned string (dashed lines not included):
----------------------------------------------------------------------------
UTC = ddd hh:mm:ss.s\n
telescope ID = ttt, focus = ***** mm, tube length = tt.tt mm\n
offset RA = rrrrrrr.r arcsec, DEC = ddddddd.d arcsec\n
rate RA = rrrrrrr.r arcsec/hr, DEC = ddddddd.d arcsec/hr\n
Cass ring angle = ccc.cc
----------------------------------------------------------------------------
In UTC, ddd is UT day of year.  ID is 200 or 60.  At P200, focus format
is ff.ff mm;  at P60 the focus reading is fffff and is in arbitrary units
(not mm, although "mm" is displayed).  Tube length and Cass ring angle are
meaningless at P60.  Fields are fixed width.  There is no terminating
newline.  (See also Get-header command above.)

RET (no arguments):  move the telescope so the DRA and DDEC offsets on the
telescope status display go to zero.  (See Z command, below.)  Rate is
50 arcsec/sec.  Maximum move: 600 sec in RA, 6000 arcsec in dec.  The
telescope must be tracking stably.

RINGGO (one argument):  rotate the Cass ring to the position specified in
the argument, in degrees.  If the position is available in more than one
turn of the ring, the shortest move will be taken.  Valid range: 0 to
359.99... degrees.

RINGMOVE (one argument):  rotate the Cass ring by the displacement specified
in the argument, in degrees.  Valid range: depends on current position with
respect to limits.  If out of range, "-3" (unable to execute) is returned.

S (one argument):  move telescope south by number of arcseconds given in
argument.  Rate of move is set by second argument of MRATES - see above.
Move is precessed to display equinox.  Valid range: 0 to 6000 arcsec.

SET (two arguments):  set the RA and dec rates for the handpaddle Set
buttons.  Valid range: 0 to 50 arcsec/sec.

TX (no arguments):  define the current position of the telescope to be the
position established with the last GO command (operator command only).
Similar to X -- see below, but pointing offsets are not saved, and the
operator can restore the pointing offsets associated with the last X.
DISABLED-- not available to clients; telescope operator only.

W (one argument):  move telescope west by number of arcseconds given in
argument.  Rate of move is set by first argument of MRATES - see above.
Move is precessed to display equinox.  Valid range: 0 to 6000 arcsec.

WS (one argument):  move telescope west by seconds of time given in
argument.  Rate of move is set by first argument of MRATES - see above.
Move is precessed to display equinox.  Valid range: 0 to 600 seconds.

X (no arguments):  define the current position of the telescope to be the
position established with the last GO command (operator command only).
Normally done after GOing to a star with known good coordinates and centering
the star in the field, to optimize local pointing.  X zeroes the displayed
DRA and DDEC offsets.  Valid range:  change in pointing offsets less than
10 sec in RA and 300 arcsec in dec.  NOTE: this command must be used with
care -- if used at the wrong position, subsequent telescope pointing will be
bad.  DISABLED-- not available to clients; telescope operator only.

Z (no arguments):  zero the DRA and DDEC display offsets;  ie, make the
current position of the telescope a reference point.



RETURNS

The get-header command and the ?NAME, ?PARALLACTIC, ?WEATHER, RAWDEC, RAWPOS,
REQPOS, and REQSTAT commands return the requested information.  All other
commands return an ASCII string to show completion status.  (Motion commands
do not return until motion is complete.)  All ASCII strings have a null
character appended.  The meaning of return strings is as follows:

	"0":  Successful completion.
	"-1": Unrecognized command.
	"-2": Invalid parameter(s).
	"-3": Unable to execute at this time.
	"-4": TCS Sparc host unavailable.



COMMUNICATIONS PROTOCOLS

SERIAL PROTOCOL:  9600 bps, 1 start bit, 8 data bits, 1 stop bit, no parity.
No handshaking.

STREAM SOCKET PARAMETERS:
TCS IP address (P200): 198.202.125.194 (proxy address)
TCP service (port): 5004 or 49200
[A sample client program, written in C, is available.]
*/
/*================================================================================================
/       End of the P200ServerSocket Class
/=================================================================================================*/
 }
