package edu.caltech.palomar.telescopes.P200; 
//=== File Prolog =============================================================
//	This code was developed by California Institute of Technology
//      Caltech Optical Observatories, Palomar Observatory
//
//--- Contents ----------------------------------------------------------------
//	 P200Component- This is the primary component used to interact with the
//                      P200 telescope. Includes TCP/IP socket support, telescope
//                      object model and specific P200 command syntax 
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
import edu.caltech.palomar.telescopes.P200.TelescopeObject;
import edu.caltech.palomar.io.ClientSocket;
import edu.dartmouth.jskycalc.JSkyCalcModel;
import java.beans.*;
import edu.caltech.palomar.telescopes.P200.gui.tables.AstroObject;
import edu.caltech.palomar.util.general.ExposureTimer; 
import java.util.Vector;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
/*=============================================================================================
/      Class Definition  :   public class P200Component
/=============================================================================================*/
public class P200Component extends java.lang.Object {
    transient private PropertyChangeSupport propertyChangeListeners = new PropertyChangeSupport(this);
    private transient Vector actionListeners;
    private java.lang.String      TERMINATOR               = new java.lang.String("\n");
    TelescopeObject              myTelescopeObject         = new TelescopeObject();
    ClientSocket                 myClientSocket            = new ClientSocket();
    ClientSocket                 myControlSocket           = new ClientSocket();
    ClientSocket                 myAbortSocket             = new ClientSocket();
    public TelescopesIniReader   myTelescopesIniReader     = new TelescopesIniReader();
    public     UpdateTimeLine    myUpdateTimeLine          = new UpdateTimeLine();
    UpdateTelescopeStatusThread  myUpdateTelescopeStatusThread;
    UpdateEphemerisThread        myUpdateEphemerisThread;
//    JSkyCalcModel                myJSkyCalcModel           = new JSkyCalcModel();
    int                          pollingRate;
    public int                   DEFAULT_POLLING_RATE      = 500;
    private boolean              connected;
    private boolean              control_connected;
    private boolean              abort_connected;
    private boolean              moving;
    private boolean              polling;
    public AstroObject           currentAstroObject;
    public static int MOVING_N = 1;
    public static int MOVING_E = 2;
    public static int MOVING_S = 3;
    public static int MOVING_W = 4;
    public ExposureTimer GOTimer;
    public ExposureTimer FOCUSTimer;
    public    long SECOND_TO_MILLI_CONVERSION    = 1000;
    public    int  TIMER_DELAY                   = 1000;
    public    int  INT_TIMER_DELAY               = 1000;
    public  int    UPDATE_TIMELINE_DELAY   = 100;
    public static int FOCUS_SET    = 1;
    public static int FOCUS_OFFSET = 2;
    private java.lang.String   coordMessage = new java.lang.String();
    private boolean            focus_moving;
    private boolean            change_move_rates;
    
/*=============================================================================================
/      Constructors :  P200Component
/=============================================================================================*/
   public P200Component(){
       jbInit();     
       setConnected(false);
       setControlConnected(false);
       setMoving(false);
       setCoordinateStatusMessage("");
       setFocusMoving(false);
       setChangeMoveRates(true);
       DEFAULT_POLLING_RATE = (Integer.valueOf(myTelescopesIniReader.POLLING_RATE)).intValue();
   }
/*================================================================================================
/        jbInit() initialization method
/=================================================================================================*/
  private void jbInit(){
        setConnected(false);
        myClientSocket = new ClientSocket(myTelescopesIniReader.SERVERNAME_STRING,myTelescopesIniReader.SERVERPORT_INTEGER);
        myClientSocket.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
        public void propertyChange(java.beans.PropertyChangeEvent e) {
            myClientSocket_propertyChange(e);
         }
        });
        myControlSocket = new ClientSocket(myTelescopesIniReader.SERVERNAME_STRING,myTelescopesIniReader.SERVERPORT_INTEGER);
        myControlSocket.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
        public void propertyChange(java.beans.PropertyChangeEvent e) {
            myControlSocket_propertyChange(e);
         }
        });
        myAbortSocket = new ClientSocket(myTelescopesIniReader.SERVERNAME_STRING,myTelescopesIniReader.SERVERPORT_INTEGER);
        myAbortSocket.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
        public void propertyChange(java.beans.PropertyChangeEvent e) {
            myAbortSocket_propertyChange(e);
         }
        });       
        myUpdateTelescopeStatusThread = new UpdateTelescopeStatusThread(DEFAULT_POLLING_RATE);
        myUpdateEphemerisThread       = new UpdateEphemerisThread();
        registerTimer();
        registerFocusTimer();
  }
/*================================================================================================
/       TelescopeObject getTelescopeObject()
/=================================================================================================*/
public TelescopeObject getTelescopeObject(){
    return myTelescopeObject;
}
/*=============================================================================================
/     Property Change Listener for the _propertyChange
/=============================================================================================*/
  private void myClientSocket_propertyChange(PropertyChangeEvent e)  {
     java.lang.String propertyName = e.getPropertyName();
       if(propertyName == "connected"){
        boolean state = (java.lang.Boolean)e.getNewValue();
        setConnected(state);
        if(state){
            initializeTelescopeState();
        }
      }
  }
/*=============================================================================================
/     Property Change Listener for the _propertyChange
/=============================================================================================*/
  private void myControlSocket_propertyChange(PropertyChangeEvent e)  {
     java.lang.String propertyName = e.getPropertyName();
       if(propertyName == "connected"){
        boolean state = (java.lang.Boolean)e.getNewValue();
        setControlConnected(state);
        if(state){
          myTelescopeObject.setTelescopeMotionDirection(TelescopeObject.NOT_MOVING);
        }
      }
  }
/*=============================================================================================
/     Property Change Listener for the _propertyChange
/=============================================================================================*/
  private void myAbortSocket_propertyChange(PropertyChangeEvent e)  {
     java.lang.String propertyName = e.getPropertyName();
       if(propertyName == "connected"){
        boolean state = (java.lang.Boolean)e.getNewValue();
        setAbortConnected(state);
        if(state){
        }
      }
  }  
/*================================================================================================
/       setCoordinateStatusMessage(java.lang.String newCoordMessage)
/=================================================================================================*/
  public void setCoordinateStatusMessage(java.lang.String newCoordMessage) {
    String  oldCoordMessage = this.coordMessage;
    this.coordMessage = newCoordMessage;
    propertyChangeListeners.firePropertyChange("coord_message", oldCoordMessage, coordMessage);
  }
  public java.lang.String getCoordinateStatusMessage() {
    return coordMessage;
  }
/*================================================================================================
/       setConnected(boolean connected)
/=================================================================================================*/
  public synchronized void setMoving(boolean new_moving) {
    boolean  old_moving = this.moving;
    this.moving = new_moving;
    propertyChangeListeners.firePropertyChange("moving", Boolean.valueOf(old_moving), Boolean.valueOf(new_moving));
  }
  public synchronized boolean isMoving() {
    return moving;
  }
/*================================================================================================
/       setConnected(boolean connected)
/=================================================================================================*/
  public synchronized void setFocusMoving(boolean new_focus_moving) {
    boolean  old_focus_moving = this.focus_moving;
    this.focus_moving = new_focus_moving;
    propertyChangeListeners.firePropertyChange("focus_moving", Boolean.valueOf(old_focus_moving), Boolean.valueOf(new_focus_moving));
  }
  public synchronized boolean isFocusMoving() {
    return focus_moving;
  }  
/*================================================================================================
/       setPolling(boolean new_polling)
/=================================================================================================*/
  public synchronized void setPolling(boolean new_polling) {
    boolean  old_polling = this.polling;
    this.polling = new_polling;
    propertyChangeListeners.firePropertyChange("polling", Boolean.valueOf(old_polling), Boolean.valueOf(new_polling));
  }
  public synchronized boolean isPolling() {
    return polling;
  }
/*================================================================================================
/       setConnected(boolean connected)
/=================================================================================================*/
  public void setConnected(boolean connected) {
    boolean  oldConnected = this.connected;
    this.connected = connected;
    propertyChangeListeners.firePropertyChange("connected", Boolean.valueOf(oldConnected),Boolean.valueOf(connected));
  }
  public boolean isConnected() {
    return connected;
  }
/*================================================================================================
/       setControlConnected(boolean connected)
/=================================================================================================*/
  public void setControlConnected(boolean control_connected) {
    boolean  oldControlConnected = this.control_connected;
    this.control_connected = control_connected;
    propertyChangeListeners.firePropertyChange("control_connected", Boolean.valueOf(oldControlConnected),Boolean.valueOf(control_connected));
  }
  public boolean isControlConnected() {
    return control_connected;
  }
/*================================================================================================
/       setAbortConnected(boolean connected)
/=================================================================================================*/
  public void setAbortConnected(boolean abort_connected) {
    boolean  oldAbortConnected = this.abort_connected;
    this.abort_connected = abort_connected;
    propertyChangeListeners.firePropertyChange("abort_connected", Boolean.valueOf(oldAbortConnected),Boolean.valueOf(abort_connected));
  }
  public boolean isAbortConnected() {
    return abort_connected;
  }
  public void setChangeMoveRates(boolean new_change_move_rates){
      boolean old_change_move_rates = change_move_rates;
      this.change_move_rates = new_change_move_rates;
     propertyChangeListeners.firePropertyChange("change_move_rates", Boolean.valueOf(old_change_move_rates), Boolean.valueOf(new_change_move_rates));     
  }
  public boolean isChangeMoveRates(){
      return change_move_rates;
  }
/*================================================================================================
/        connect()
/=================================================================================================*/
  public void connect(){
      myClientSocket.startConnection(ClientSocket.USE_HOSTNAME);
  }
/*================================================================================================
/        connect_control()
/=================================================================================================*/
  public void connect_control(){
      myControlSocket.startConnection(ClientSocket.USE_HOSTNAME);
  }
/*================================================================================================
/        connect_abort()
/=================================================================================================*/
  public void connect_abort(){
      myAbortSocket.startConnection(ClientSocket.USE_HOSTNAME);
  }  
/*================================================================================================
/        disconnect()
/=================================================================================================*/
  public void disconnect(){
     myClientSocket.closeConnection();
     myUpdateTelescopeStatusThread.setUpdating(false);
  }
/*================================================================================================
/       disconnect_control()
/=================================================================================================*/
  public void disconnect_control(){
     myControlSocket.closeConnection();
   }
/*================================================================================================
/       disconnect_abort()
/=================================================================================================*/
  public void disconnect_abort(){
     myAbortSocket.closeConnection();
   }
/*================================================================================================
/      startPolling()
/=================================================================================================*/
 public void startPolling(){
     myUpdateTelescopeStatusThread.start();
 }
/*================================================================================================
/      stopPolling()
/=================================================================================================*/
public void stopPolling(){
    myUpdateTelescopeStatusThread.setUpdating(false);
}
/*================================================================================================
/      reinitializeControlSocket()
/=================================================================================================*/
public void reinitializeControlSocket(){
   myControlSocket.forcecloseConnection();
 //  myControlSocket.startConnection(ClientSocket.USE_HOSTNAME);
}
/*================================================================================================
/      sentCoordinates(AstroObject currentAstroObject)
/=================================================================================================*/
public void sentCoordinates(AstroObject currentAstroObject){
    SendCoordinatesThread mySendCoordinatesThread = new SendCoordinatesThread(currentAstroObject);
    mySendCoordinatesThread.start();
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
/*================================================================================================
/      initializeTelescopeState()
/=================================================================================================*/
  public void initializeTelescopeState(){
    java.lang.String response0 = new java.lang.String();
    java.lang.String response1 = new java.lang.String();
    java.lang.String response2 = new java.lang.String();
    response0 = myClientSocket.sendReceiveCommand("REQPOS"+TERMINATOR);
    parseREQPOS(response0);
    waitForResponseMilliseconds(pollingRate);
    response1 = myClientSocket.sendReceiveCommand("REQSTAT"+TERMINATOR);
    parseREQSTAT(response1);
    waitForResponseMilliseconds(pollingRate);
//    response2 = myClientSocket.sendReceiveCommand("WEATHER"+TERMINATOR);
    response2 = myClientSocket.sendReceiveCommand("?WEATHER"+TERMINATOR);
    parseWEATHER(response2);
  }
/*================================================================================================
/     stripUnits(java.lang.String unitString, in newUnitLength)
/=================================================================================================*/
public java.lang.String stripUnits(java.lang.String unitString,int newUnitLength){
        int unitLength = newUnitLength;
 // NOT FINISHED THIS IS NOT IMPLEMENTED YET
       int length = unitString.length();
       java.lang.String result = unitString.substring(0, length - unitLength);
     return result;
}
/*================================================================================================
/     parseWEATHER(java.lang.String currentResponse)
/=================================================================================================*/
public java.lang.String stripLabel(java.lang.String labeledString){
       java.util.StringTokenizer tokenResponseString = new java.util.StringTokenizer(labeledString,"=");
       java.lang.String          result         = tokenResponseString.nextToken("=");
       result = tokenResponseString.nextToken();
    return result;
}
/*================================================================================================
/     parseResponse(java.lang.String currentResponse)
/=================================================================================================*/
 public java.lang.String parseResponse(java.lang.String currentResponse){
//	"0":  Successful completion.
//	"-1": Unrecognized command.
//	"-2": Invalid parameter(s).
//	"-3": Unable to execute at this time.
//	"-4": TCS Sparc host unavailable.
     java.lang.String message = new java.lang.String();
     if("0".regionMatches(0, currentResponse, 0, 1)){ // Request telescope position
        message = "Successful completion";
        myTelescopeObject.setStatusMessage(message);
//        System.out.println(message);
     }
     if("-1".regionMatches(0, currentResponse, 0, 2)){ // Request telescope position
        message = "Unrecognized command";
        myTelescopeObject.setStatusMessage(message);
//        System.out.println(message);
     }
     if("-2".regionMatches(0, currentResponse, 0, 2)){ // Request telescope position
        message = "Invalid parameter(s)";
        myTelescopeObject.setStatusMessage(message);
//        System.out.println(message);
     }
     if("-3".regionMatches(0, currentResponse, 0, 2)){ // Request telescope position
        message = "Unable to execute at this time";
        myTelescopeObject.setStatusMessage(message);
//        System.out.println(message);
     }
     if("-4".regionMatches(0, currentResponse, 0, 2)){ // Request telescope position
        message = "TCS Sparc host unavailable";
        myTelescopeObject.setStatusMessage(message);
//        System.out.println(message);
     }
   return message;
 }
/*================================================================================================
/     parseREQPOS(java.lang.String currentResponse)
/=================================================================================================*/
 public void parseREQPOS(java.lang.String currentResponse){
   // UTC = 216 04:34:38.1, LST = 17:37:45.4
   // RA = 17:57:57.69, DEC = +04:43:11.8, HA = E00:20:45.8
   // air mass =  1.143                                                                                                                                                  
      try{
       java.util.StringTokenizer tokenResponseString = new java.util.StringTokenizer(currentResponse,"=");
       java.lang.String          UTCString         = tokenResponseString.nextToken(",");
       java.lang.String          LSTString         = tokenResponseString.nextToken("\n");
       java.lang.String          RAString          = tokenResponseString.nextToken(",");
       java.lang.String          DECString         = tokenResponseString.nextToken(",");
       java.lang.String          HAString          = tokenResponseString.nextToken("\n");
       java.lang.String          AirMassString     = tokenResponseString.nextToken("\r");
       UTCString = stripLabel(UTCString);
       LSTString = stripLabel(LSTString);
       RAString = stripLabel(RAString);
       DECString = stripLabel(DECString);
       RAString  = RAString.trim();
       DECString = DECString.trim();
       HAString = stripLabel(HAString);
       AirMassString = stripLabel(AirMassString);
       AirMassString.substring(0, 5);
       myTelescopeObject.setUTCSTART(UTCString);
       myTelescopeObject.setSidereal(LSTString);
       myTelescopeObject.setRightAscension(RAString);
       myTelescopeObject.setDeclination(DECString);
       myTelescopeObject.setHourAngle(HAString);
//       myTelescopeObject.setAirMass(AirMassString);
//       double myRA = parseSexag(myRAString);
//       double myDec = parseSexag(myDecString);

     }catch(Exception e2){
       System.out.println("Error parsing coordinates for a position. parseREQPOS");
     }
 }
/*================================================================================================
/     parseREQSTAT(java.lang.String currentResponse)
/=================================================================================================*/
 public void parseREQSTAT(java.lang.String currentResponse){
              //  telescope ID = 200, focus = 36.71 mm, tube length = 22.11 mm
              //  offset RA =     137.3 arcsec, DEC =      95.8 arcsec
              //  rate RA =       0.0 arcsec/hr, DEC =       0.0 arcsec/hr
              //  Cass ring angle =  49.35                                        
     try{
       java.util.StringTokenizer tokenResponseString = new java.util.StringTokenizer(currentResponse,"=");
       java.lang.String          UTCString                 = tokenResponseString.nextToken("\n");
       java.lang.String          TelescopeIDString         = tokenResponseString.nextToken(",");
       java.lang.String          FocusString               = tokenResponseString.nextToken(",");
       java.lang.String          TubeLengthString          = tokenResponseString.nextToken("\n");
       java.lang.String          OffsetRAString            = tokenResponseString.nextToken(",");
       java.lang.String          OffsetDECString           = tokenResponseString.nextToken("\n");
       java.lang.String          RateRAString              = tokenResponseString.nextToken(",");
       java.lang.String          RateDECString             = tokenResponseString.nextToken("\n");
       java.lang.String          CassRingAngleString       = tokenResponseString.nextToken("\r");
//  Strip the label from the string
       TelescopeIDString   = stripLabel(TelescopeIDString);

       FocusString         = stripLabel(FocusString);
       FocusString         = stripUnits(FocusString,3);

       TubeLengthString    = stripLabel(TubeLengthString);
       TubeLengthString    = stripUnits(TubeLengthString,3);
       
       OffsetRAString      = stripLabel(OffsetRAString);
       OffsetRAString      = stripUnits(OffsetRAString,7);

       OffsetDECString     = stripLabel(OffsetDECString);
       OffsetDECString     = stripUnits(OffsetDECString,7);

       RateRAString        = stripLabel(RateRAString);
       RateRAString        = stripUnits(RateRAString,10);

       RateDECString       = stripLabel(RateDECString);
       RateDECString       = stripUnits(RateDECString,10);

       CassRingAngleString = stripLabel(CassRingAngleString);
       CassRingAngleString = CassRingAngleString.trim();
//       CassRingAngleString = CassRingAngleString.substring(0,6);

   // Update the Telescope Object
       myTelescopeObject.setTelescopeID(TelescopeIDString);
       myTelescopeObject.setTelescopeFocus(FocusString);
       myTelescopeObject.setTelescopeTubeLength(TubeLengthString);
       myTelescopeObject.setRightAscensionOffset(OffsetRAString);
       myTelescopeObject.setDeclinationOffset(OffsetDECString);
       myTelescopeObject.setRightAscensionTrackRate(RateRAString);
       myTelescopeObject.setDeclinationTrackRate(RateDECString);
       myTelescopeObject.setCassRingAngle(CassRingAngleString);
       myTelescopeObject.updateOffsetCoordinates();
//       double myRA = parseSexag(myRAString);
//       double myDec = parseSexag(myDecString);
       java.lang.String test = new java.lang.String();
     }catch(Exception e2){
       System.out.println("Error parsing coordinates for a position. parseREQSTAT");
     }
 }
/*================================================================================================
/     parseWEATHER(java.lang.String currentResponse)
/=================================================================================================*/
 public void parseName(java.lang.String currentResponse){
        myTelescopeObject.setObjectName(currentResponse.trim());
 }
/*================================================================================================
/     parseWEATHER(java.lang.String currentResponse)
/=================================================================================================*/
 public void parseWEATHER(java.lang.String currentResponse){
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
      try{
       java.util.StringTokenizer tokenResponseString = new java.util.StringTokenizer(currentResponse,"=");
       java.lang.String          RAString           = tokenResponseString.nextToken("\n");
       java.lang.String          DECString          = tokenResponseString.nextToken("\n");
       java.lang.String          HAString           = tokenResponseString.nextToken("\n");
       java.lang.String          LSTString          = tokenResponseString.nextToken("\n");
       java.lang.String          AirMassString      = tokenResponseString.nextToken("\n");
       java.lang.String          AzimuthString      = tokenResponseString.nextToken("\n");
       java.lang.String          ZenithAngleString  = tokenResponseString.nextToken("\n");
       java.lang.String          FocusPointString   = tokenResponseString.nextToken("\n");
       java.lang.String          DomeAzimuthString  = tokenResponseString.nextToken("\n");
       java.lang.String          DomeShuttersString = tokenResponseString.nextToken("\n");
       java.lang.String          WindScreenString   = tokenResponseString.nextToken("\n");
        java.lang.String         InstPosString      = tokenResponseString.nextToken("\n");
       java.lang.String          InstrumentString   = tokenResponseString.nextToken("\n");
       java.lang.String          PumpsString        = tokenResponseString.nextToken("\r");
       // Strip the labels from the values.
// These parameters are available in higher precision elsewhere and should not be use for updating the model
       RAString           = stripLabel(RAString);
       DECString          = stripLabel(DECString);
       HAString           = stripLabel(HAString);
       LSTString          = stripLabel(LSTString);
       AirMassString      = stripLabel(AirMassString);
       FocusPointString   = stripLabel(FocusPointString);
// The following parameters are only available from the WEATHER command and are used to update the model
       AzimuthString      = stripLabel(AzimuthString);
       ZenithAngleString  = stripLabel(ZenithAngleString);
       DomeAzimuthString  = stripLabel(DomeAzimuthString);
       DomeShuttersString = stripLabel(DomeShuttersString);
       WindScreenString   = stripLabel(WindScreenString);
       InstPosString      = stripLabel(InstPosString);
       InstrumentString   = stripLabel(InstrumentString);
       PumpsString        = stripLabel(PumpsString);
//   Set the actual parameters in the model
       myTelescopeObject.setAirMass(AirMassString);
       myTelescopeObject.setAzimuthString(AzimuthString);
       myTelescopeObject.setZenithAngleString(ZenithAngleString);
       myTelescopeObject.setDomeAzimuthString(DomeAzimuthString);
       myTelescopeObject.setDomeShuttersString(DomeShuttersString);
       myTelescopeObject.setWindscreenPositionString(WindScreenString);
       myTelescopeObject.setInstrumentPositionString(InstPosString);
       myTelescopeObject.setInstrument(InstrumentString);
       myTelescopeObject.setPumps(PumpsString);     // Not yet implemented on the telescope side
     }catch(Exception e2){
       System.out.println("Error parsing coordinates for a position. parseWEATHER");
     }
 }
/*=============================================================================================
/    parseMotion(java.lang.String currentResponse)
/=============================================================================================*/
 public int parseMotion(java.lang.String currentResponse){
     currentResponse = currentResponse.trim();
     int motion = Integer.parseInt(currentResponse);
     myTelescopeObject.setMotionStatus(motion);
     if(motion == TelescopeObject.MOTION_STOPPED){
         myTelescopeObject.setMotionStatusString("STOPPED");
     }
     if(motion == TelescopeObject.MOTION_SLEWING){
         myTelescopeObject.setMotionStatusString("SLEWING");
     }
     if(motion == TelescopeObject.MOTION_OFFSETTING){
         myTelescopeObject.setMotionStatusString("OFFSETTING");
     }
     if(motion == TelescopeObject.MOTION_TRACKING_STABLY){
         myTelescopeObject.setMotionStatusString("TRACKING STABLY");
     }
     if(motion == TelescopeObject.MOTION_SETTLING){
         myTelescopeObject.setMotionStatusString("SETTLING");
     }
     return motion;
 }
/*=============================================================================================
/                            Log Message Methods
/      public void logErrorMessage(java.lang.String newMessage)
/=============================================================================================*/
  public void logErrorMessage(java.lang.String newMessage){
    System.out.println(newMessage);
  }// end of the logErrorMessage
/*=============================================================================================
/    returnToGO()
/=============================================================================================*/
  public void returnToGO(){
      //  If the telescope has been parked (i.e. not tracking) then the return to GO may be
      //  a move that is greater than the maximum allowed move.  We need to check that the
      //  move is within allowable limits and abort if it isn't
      if(isConnected()){
          boolean isValidMove = true;
          if((myTelescopeObject.getRA_OFFSET() > TelescopeObject.MAX_RA_OFFSET)|(myTelescopeObject.getRA_OFFSET() < TelescopeObject.MIN_RA_OFFSET)){
              myTelescopeObject.setStatusMessage("Requested telescope move exceeds maximum or minimum RA offset");
              isValidMove = false;
          }
          if((myTelescopeObject.getDEC_OFFSET() > TelescopeObject.MAX_DEC_OFFSET)|(myTelescopeObject.getDEC_OFFSET() < TelescopeObject.MIN_DEC_OFFSET)){
              myTelescopeObject.setStatusMessage("Requested telescope move exceeds maximum or minimum Dec offset");
              isValidMove = false;
          }
          if(isValidMove){
             MoveTelescopeThread myMoveTelescopeThread = new MoveTelescopeThread();
             myMoveTelescopeThread.start();
          }
      }
      if(!isConnected()){
             myTelescopeObject.setStatusMessage("Move size unknown, connect telescope monitor to continue");
      }
  }
/*=============================================================================================
/     moveTelescope(int direction,double offset)
/=============================================================================================*/
  public void moveTelescope(int direction,double offset){
      MoveTelescopeThread myMoveTelescopeThread = new MoveTelescopeThread(direction,offset);
      myMoveTelescopeThread.start();
  }
/*=============================================================================================
/     moveTelescope(double currentRAoffset, double currentDecoffset)
/=============================================================================================*/
  public void moveTelescope(double currentRAoffset, double currentDecoffset){
      MoveTelescopeThread myMoveTelescopeThread = new MoveTelescopeThread(currentRAoffset,currentDecoffset);
      myMoveTelescopeThread.start();
  }
/*=============================================================================================
/    AbortMove()
/=============================================================================================*/
  public void AbortMove(){
      SendAbortCommandThread mySendAbortCommandThread = new SendAbortCommandThread();
      mySendAbortCommandThread.start();
  }
/*=============================================================================================
/     setTelescopeFocus(double newFocus)
/=============================================================================================*/
//  FOCUSGO (one argument):  change telescope focus to the value given in the
//  argument.  Valid range: 1.00 to 74.00 mm. (P200 only)
//
//  FOCUSINC (one argument):  change telescope focus by the offset given in the
//  argument.  Valid range: -73.00 to 73.00 mm.  Limit checking is done.
//  (P200 only)
  public void setTelescopeFocus(double newFocus){
    AdjustFocusThread myAdjustFocusThread = new AdjustFocusThread(FOCUS_SET,newFocus);
    myAdjustFocusThread.start();
  }
/*=============================================================================================
/     offsetTelescopeFocus(double newFocusOffset)
/=============================================================================================*/
  public void offsetTelescopeFocus(double newFocusOffset){
    AdjustFocusThread myAdjustFocusThread = new AdjustFocusThread(FOCUS_OFFSET,newFocusOffset);
    myAdjustFocusThread.start();
  }
/*=============================================================================================
/    setMoveRates(double newRAMoveRate,double newDecMoveRate)
/=============================================================================================*/
//  MRATES (two arguments):  set rate of RA and dec moves to values given in
//  first and second arguments, respectively.  Valid range: 0 to 50 arcsec/sec.
//  Note:  these rates apply to moves as described in this note and are
//  independent of rates for Night-Assistant-commanded moves.

  public void setMoveRates(double newRAMoveRate,double newDecMoveRate){
      String command = new String();
      command = "MRATES " + Double.toString(newRAMoveRate) + " " + Double.toString(newDecMoveRate);
      SendCommandThread  mySendCommandThread  = new SendCommandThread (command,0.0);
      mySendCommandThread .start();
  }
  public java.lang.String setMoveRates(){
    // This version of the command uses the rates set in the TelescopeObject and directly sends the command to the TCP/IP port        
    String response = new String();
    String command  = new String();
    command = "MRATES " + Double.toString(myTelescopeObject.getRAMoveRate()) + " " + Double.toString(myTelescopeObject.getDecMoveRate());
    response = myControlSocket.sendReceiveCommand(command+TERMINATOR);
    return response;
  }
/*=============================================================================================
/    setTrackRates(double newRATrackRate,double newDecTrackRate)
/=============================================================================================*/
//  RATES (two arguments):  define offsets to sidereal tracking rates for non-
//  sidereal objects.  First argument is RA rate in arcsec/hr, second argument
//  is declination rate in arcsec/hr.  Rates are enabled with the R command -
//  see above.

//  RATESS (two arguments):  define offsets to sidereal tracking rates for non-
//  sidereal objects.  First argument is RA rate in seconds/hr, second argument
//  is declination rate in arcsec/hr.  Rates are enabled with the R command -
//  see above.
  public void setTrackRates(double newRATrackRate,double newDecTrackRate){
      String command = new String();
      command = "RATES " + Double.toString(newRATrackRate) + " " + Double.toString(newDecTrackRate);
      SendCommandThread  mySendCommandThread  = new SendCommandThread (command,0.0);
      mySendCommandThread .start();
  }
  public void enableTracking(){
      String command = new String();
      command = "R";
      SendCommandThread  mySendCommandThread  = new SendCommandThread (command,0.0);
      mySendCommandThread .start();
  }
  public void disableTracking(){
      String command = new String();
      command = "-R";
      SendCommandThread  mySendCommandThread  = new SendCommandThread (command,0.0);
      mySendCommandThread .start();
  }
/*=============================================================================================
/    setTrackRates(double newRATrackRate,double newDecTrackRate)
/=============================================================================================*/
public void query_lamp_status(){
    
} 
public boolean lamp_status(int port){
    java.lang.String commandString = "NPS "+TelescopeObject.LAMP_STATUS+" "+Integer.toString(port)+TERMINATOR;
    java.lang.String response = myControlSocket.sendReceiveCommand(commandString+TERMINATOR,1.0);
    java.lang.String result   = parseResponse(response);
   boolean state = true;
   return state;
} 
/*=============================================================================================
/    all_lamps(boolean state)
/=============================================================================================*/
public void all_lamps(boolean state){
    if(state){
       setLowLamp(true); 
       setHighLamp(true); 
       setArcLamp(true); 
    }
    if(!state){
       java.lang.String commandString = "NPS "+TelescopeObject.LAMP_OFF+" "+"0"+TERMINATOR;       
       java.lang.String response      = myControlSocket.sendReceiveCommand(commandString+TERMINATOR,1.0);
       java.lang.String result        = parseResponse(response);
       myTelescopeObject.setLowLamp(false);
       myTelescopeObject.setHighLamp(false);
       myTelescopeObject.setArcLamp(false);
    }
}
/*=============================================================================================
/    setLowLamp(boolean state)
/=============================================================================================*/
public void setLowLamp(boolean state){ 
    java.lang.String commandString = new java.lang.String();
    if(state){
       commandString = "NPS "+TelescopeObject.LAMP_ON+" "+TelescopeObject.LOW_LAMP_PORT+TERMINATOR;       
    }
    if(!state){
       commandString = "NPS "+TelescopeObject.LAMP_OFF+" "+TelescopeObject.LOW_LAMP_PORT+TERMINATOR;       
    }
    java.lang.String response = myControlSocket.sendReceiveCommand(commandString+TERMINATOR,1.0);
    java.lang.String result   = parseResponse(response);
    myTelescopeObject.setLowLamp(state);
} 
/*=============================================================================================
/   setHighLamp(boolean state)
/=============================================================================================*/
public void setHighLamp(boolean state){
    java.lang.String commandString = new java.lang.String();
    if(state){
       commandString = "NPS "+TelescopeObject.LAMP_ON+" "+TelescopeObject.HIGH_LAMP_PORT+TERMINATOR;       
    }
    if(!state){
       commandString = "NPS "+TelescopeObject.LAMP_OFF+" "+TelescopeObject.HIGH_LAMP_PORT+TERMINATOR;       
    }
    java.lang.String response = myControlSocket.sendReceiveCommand(commandString+TERMINATOR,1.0);
    java.lang.String result   = parseResponse(response);
    myTelescopeObject.setHighLamp(state);
} 
/*=============================================================================================
/   setArcLamp(boolean state)
/=============================================================================================*/
public void setArcLamp(boolean state){
    java.lang.String commandString = new java.lang.String();
    if(state){
       commandString = "NPS "+TelescopeObject.LAMP_ON+" "+TelescopeObject.ARC_LAMP_PORT+TERMINATOR;       
    }
    if(!state){
       commandString = "NPS "+TelescopeObject.LAMP_OFF+" "+TelescopeObject.ARC_LAMP_PORT+TERMINATOR;       
    }
    java.lang.String response = myControlSocket.sendReceiveCommand(commandString+TERMINATOR,1.0);
    java.lang.String result   = parseResponse(response);
    myTelescopeObject.setArcLamp(state);
} 
//  R (no arguments):  turn on offset tracking rates (non-sidereal).  The rates to
//  be used may be entered at the Night Assistant's console or with the RATES or
//  RATESS commands - see below.
//  -R (no arguments):  turn off offset tracking rates.
/*=============================================================================================
/    setTrackRates(double newRATrackRate,double newDecTrackRate)
/=============================================================================================*/
  public void ZTelescopeOffsets(){
//    Z (no arguments):  zero the DRA and DDEC display offsets;  ie, make the
//    current position of the telescope a reference point.
      String command = new String();
      command = "Z";
      SendCommandThread  mySendCommandThread  = new SendCommandThread (command,0.0);
      mySendCommandThread .start();
  }
/*===============================================================================================================
/               Start and Stop and Restart GOTimer Methods
/      public void configureTimer(){
/      public void startTimer()
/      public void stopTimer()
/      public void restartTimer()
/===============================================================================================================*/
 public void registerTimer(){
   GOTimer = new ExposureTimer(TIMER_DELAY,
              new java.awt.event.ActionListener(){
              public void actionPerformed(java.awt.event.ActionEvent e){
                try{
                       // this is really a dummy ActionListener required by the constructor
                       // of the Timer
                 }// end of the try block
                catch(Exception e2){
                      java.lang.String myErrorString = new java.lang.String();
                      myErrorString = "An error occured while trying to create the GOTimer" + e2;
//                      System.out.println(myErrorString);
                    }// end of the catch block
                }// end of the actionPerformed block
              });// end of the GOTimer.addActionListener Method
     configureTimer(); // configure the timer - see INT_TIMER_DELAY and TIMER_DELAY
 }
 /*===============================================================================================================
/               Start and Stop and Restart GOTimer Methods
/      public void configureTimer(){
/      public void startTimer()
/      public void stopTimer()
/      public void restartTimer()
/===============================================================================================================*/
 public void registerFocusTimer(){
   FOCUSTimer = new ExposureTimer(TIMER_DELAY,
              new java.awt.event.ActionListener(){
              public void actionPerformed(java.awt.event.ActionEvent e){
                try{
                       // this is really a dummy ActionListener required by the constructor
                       // of the Timer
                 }// end of the try block
                catch(Exception e2){
                      java.lang.String myErrorString = new java.lang.String();
                      myErrorString = "An error occured while trying to create the GOTimer" + e2;
                      System.out.println(myErrorString);
                    }// end of the catch block
                }// end of the actionPerformed block
              });// end of the GOTimer.addActionListener Method
     configureTimer(); // configure the timer - see INT_TIMER_DELAY and TIMER_DELAY
 }
/*================================================================================================
/      configureTimer()
/=================================================================================================*/
 public void configureTimer(){
      GOTimer.setDelay(TIMER_DELAY);
      GOTimer.setInitialDelay(INT_TIMER_DELAY);
      GOTimer.setLogTimers(false);
      GOTimer.setRepeats(true);
   }
/*================================================================================================
/      startTimer()
/=================================================================================================*/
  public void startTimer(){
      long currentTime = System.currentTimeMillis();
      long myEndTime   = currentTime + (long)(myTelescopeObject.getTotalElapsedTime()*SECOND_TO_MILLI_CONVERSION);
      long duration = myEndTime - currentTime;
      GOTimer.setStartTime(currentTime);
      GOTimer.setEndTime(myEndTime);
      GOTimer.start();
  }
/*================================================================================================
/      stopTimer()
/=================================================================================================*/
  public void stopTimer(){
      GOTimer.stop();
  }
  public void restartTimer(){
      GOTimer.restart();
  }
/*================================================================================================
/      configureTimer()
/=================================================================================================*/
 public void configureFocusTimer(){
      FOCUSTimer.setDelay(TIMER_DELAY);
      FOCUSTimer.setInitialDelay(INT_TIMER_DELAY);
      FOCUSTimer.setLogTimers(false);
      FOCUSTimer.setRepeats(true);
   }
/*================================================================================================
/      startTimer()
/=================================================================================================*/
  public void startFocusTimer(double focus_change_seconds){
      long currentTime = System.currentTimeMillis();
      long myEndTime   = currentTime + (long)(focus_change_seconds*SECOND_TO_MILLI_CONVERSION);
      long duration = myEndTime - currentTime;
      FOCUSTimer.setStartTime(currentTime);
      FOCUSTimer.setEndTime(myEndTime);
      FOCUSTimer.start();
  }
/*================================================================================================
/      stopTimer()
/=================================================================================================*/
  public void stopFocusTimer(){
      FOCUSTimer.stop();
  }
  public void restartFocusTimer(){
      FOCUSTimer.restart();
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
/      add and remove Property Change Listeners
/=================================================================================================*/
  public synchronized void removePropertyChangeListener(PropertyChangeListener l) {
    propertyChangeListeners.removePropertyChangeListener(l);
  }
  public synchronized void addPropertyChangeListener(PropertyChangeListener l) {
    propertyChangeListeners.addPropertyChangeListener(l);
  }
/*=============================================================================================
/                       UpdateTimeLine Inner Class of the P200Compontet
/     This Inner Class changes the Telescope Move Percent Completed variable so that the timer can be updated
/=============================================================================================*/
  public class UpdateTimeLine implements Runnable{
  private Thread myThread;
  private boolean completed;

  public UpdateTimeLine(){
  }
  public synchronized void setCompleted(boolean newCompleted){
     completed = newCompleted;
  }
  public synchronized boolean isCompleted(){
     return completed;
  }
  public void run(){
       startTimer();
       long start     = GOTimer.getStartTime();
       long end       = GOTimer.getEndTime();
       long totalTime = end - start;
       setCompleted(false);

       myTelescopeObject.setStatusMessage("Moving the Telescope");
       // Now loop until the exposure is completed
       while(!isCompleted()){
           // Now get the currentTime
           double   currentTime           = (double)(System.currentTimeMillis());
           double   elapsedTime           = (double)(currentTime - start);
           double   newPercentCompleted   = (elapsedTime/totalTime)*100.0;
           myTelescopeObject.setPercentCompleted(newPercentCompleted);
           try{
              Thread.currentThread().sleep(UPDATE_TIMELINE_DELAY);
             if(elapsedTime >  totalTime){
                 myTelescopeObject.setStatusMessage("Moving Completed");
                 myTelescopeObject.setPercentCompleted(0);
                 //  This is where we can trigger an event to retrieve a FITS file from the MKIR server
                 fireActionPerformed(new java.awt.event.ActionEvent(this,1,"Move Completed"));
                 setCompleted(true);
              }// end of the if elapsedTime > exposureCompletedTime check
              }// end of the try block
              catch(java.lang.InterruptedException ex3){
                   logErrorMessage("An error occurred in the UpdateTimeLine Class: "+ex3);
             }// end of the catch
       }// end of the while notCompleted loop
//   setImageName(" ");
   }// end of the run method
  public void start(){
   myThread = new Thread(this);
   myThread.start();
   }
  }// End of the UpdateTimeLine Inner Class
/*=============================================================================================
/       INNER CLASS that polls the telescope for it's position and status
/=============================================================================================*/
    public class UpdateTelescopeStatusThread  implements Runnable{
    private Thread myThread;
    private int    pollingRate = 5;
    private boolean state      = false;
    private java.lang.String response = new java.lang.String();
/*================================================================================================
/      waitForResponse(int newDelay)
/=================================================================================================*/
public void waitForResponseMilliseconds(int newDelay){
     try{
        Thread.currentThread().sleep(newDelay);} // sleep for awhile to let the messages pile up
      catch (Exception e){
    }
}
public synchronized void setUpdating(boolean newState){
    state = newState;
    if(!state){
        setPolling(false);
    }
}
/*=============================================================================================
/          UpdateTelescopeStatusThread()
/=============================================================================================*/
    private UpdateTelescopeStatusThread(int newPollingRate){
       pollingRate = newPollingRate;
    }
/*=============================================================================================
/           run()
/=============================================================================================*/
    public void run(){
     state = true;
    initializeTelescopeState();    // Update the value of all the parameters before starting to poll the telescope
     while(state){
      setPolling(true);
      for(int i=0; i<10; i++){
        for(int j=0; j<10; j++){
//              for(int k=0; k<2; k++){
                 response = myClientSocket.sendReceiveCommand("REQPOS"+TERMINATOR);
                 parseREQPOS(response);
                 response = myClientSocket.sendReceiveCommand("?MOTION"+TERMINATOR);
                 parseMotion(response);
//                 myTelescopeObject.getJSkyCalcModel().setTelescopePosition(myTelescopeObject.getRightAscension(), myTelescopeObject.getDeclination(), myTelescopeObject.getEquinox());
//                 myTelescopeObject.getJSkyCalcModel().SetToNow();
                 waitForResponseMilliseconds(pollingRate);
 //                System.out.println(response);
                 // Example:
                 // UTC = 216 04:34:38.1, LST = 17:37:45.4
                 // RA = 17:57:57.69, DEC = +04:43:11.8, HA = E00:20:45.8
                 // air mass =  1.143                                                                                                                                                  
                 if(!state){return;}
//              }
              if(!myUpdateEphemerisThread.isUpdating()){
                 myUpdateEphemerisThread.start();
              }
//              response = myClientSocket.sendReceiveCommand("REQSTAT"+TERMINATOR);
//              parseREQSTAT(response);
//              response = myClientSocket.sendReceiveCommand("?NAME"+TERMINATOR);
//              parseName(response);
              waitForResponseMilliseconds(pollingRate);
//              System.out.println(response);
              //  telescope ID = 200, focus = 36.71 mm, tube length = 22.11 mm
              //  offset RA =     137.3 arcsec, DEC =      95.8 arcsec
              //  rate RA =       0.0 arcsec/hr, DEC =       0.0 arcsec/hr
              //  Cass ring angle =  49.35                                        
              if(!state){return;}
          }// inner update loop for the REQSTA
//          response = myClientSocket.sendReceiveCommand("?WEATHER"+TERMINATOR);        // For use with the simulator
          response = myClientSocket.sendReceiveCommand("?WEATHER"+TERMINATOR);     // For use with the real P200
          parseWEATHER(response);
          response = myClientSocket.sendReceiveCommand("?MOTION"+TERMINATOR);
          parseMotion(response);
          waitForResponseMilliseconds(pollingRate);
 //         System.out.println(response);
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
          if(!state){
              setPolling(false);
              return;
          }
       }// outer update loop for "?WEATHER" update
     }
     }
    public void start(){
     myThread = new Thread(this);
     myThread.start();
     }
    }// End of the UpdateTelescopeStatusThread Inner Class
/*=============================================================================================
/                           End of the UpdateTelescopeStatusThread Class
/=============================================================================================*/
/*=============================================================================================
/       INNER CLASS that polls the telescope for it's position and status
/=============================================================================================*/
    public class UpdateEphemerisThread  implements Runnable{
    private Thread myThread;
    private boolean state      = false;
    private java.lang.String response = new java.lang.String();
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
/          setUpdating(boolean newState)
/=============================================================================================*/
public synchronized void setUpdating(boolean newState){
    state = newState;
}
public synchronized boolean isUpdating(){
    return state;
}
/*=============================================================================================
/          UpdateEphemerisThread()
/=============================================================================================*/
    private UpdateEphemerisThread(){
    }
/*=============================================================================================
/           run()
/=============================================================================================*/
    public void run(){
       state = true;
         myTelescopeObject.getJSkyCalcModel().setTelescopePosition(myTelescopeObject.getRightAscension(), myTelescopeObject.getDeclination(), myTelescopeObject.getEquinox());
         if(myTelescopeObject.getJSkyCalcModel().getTimeSource() == JSkyCalcModel.NOW){
             myTelescopeObject.getJSkyCalcModel().SetToNow();           
         }
         if(myTelescopeObject.getJSkyCalcModel().getTimeSource() == JSkyCalcModel.SELECTED_TIME){
            java.lang.String[] selected_date_time = myTelescopeObject.getJSkyCalcModel().getSelectedDateTime();
            myTelescopeObject.getJSkyCalcModel().setToDate(selected_date_time[0], selected_date_time[1]);
         }
       state = false;
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
/*=============================================================================================
/       INNER CLASS that sends a set of coordinates to the telescope
/=============================================================================================*/
    public class SendCoordinatesThread  implements Runnable{
    private Thread myThread;
    private java.lang.String response = new java.lang.String();
    private java.lang.String command = new java.lang.String();
    private AstroObject myAstroObject;
//COORDS (five or six arguments):  supply a position to the TCS.  The arguments
//in order are  1) RA in decimal hours, 2) declination in decimal degrees,
//3) equinox of coordinates (value of zero indicates apparent equinox), 4) RA
//motion, 5) dec motion, 6) motion flag.  (Motion flag absent or 0:  arguments
//4 and 5 are proper motion in RA units of .0001 sec/yr and dec units of .001
//arcsec/yr.  Motion flag = 1:  arguments 4 and 5 are offset tracking rates in
//arcsec/hr.  Motion flag = 2:  arguments 4 and 5 are offset tracking rates
//with arg 4 in seconds/hr and arg 5 in arcsec/hr.)  NOTE: a name string can
//now be provided, by adding an extra parameter enclosed in double quotes;
//this will preferably be at the end of the command line, though it can be
//inserted at any parameter position.
/*=============================================================================================
/          SendCoordinatesThread()
/=============================================================================================*/
    private SendCoordinatesThread(AstroObject newAstroObject){
        myAstroObject = newAstroObject;
    }
/*=============================================================================================
/           run()
/=============================================================================================*/
    public void run(){
       String RAstring = Double.toString(myAstroObject.Alpha.degrees()/15.0);
       String Decstring = Double.toString(myAstroObject.Delta.degrees());
       if(RAstring.length() > 10){
           RAstring = RAstring.substring(0,9);
       }
       if(Decstring.length() > 10){
           Decstring = Decstring.substring(0,9);
       }
       String Equinoxstring   = Double.toString(myAstroObject.Equinox);

       String RAMotionstring  = Double.toString(myAstroObject.r_m);
       String DecMotionstring = Double.toString(myAstroObject.d_m);
       if(RAMotionstring.length() >= 10){
           RAMotionstring = RAMotionstring.substring(0,9);
       }
       if(DecMotionstring.length() > 10){
           DecMotionstring = DecMotionstring.substring(0,9);
       }
       String Mflagstring     = Integer.toString(myTelescopeObject.getTrackingRatesUnits());
       boolean rates = myTelescopeObject.isTracking();
       if(rates){
        command = "COORDS " + RAstring + " " + Decstring + " " + Equinoxstring + " " + RAMotionstring + " " + DecMotionstring + " " + Mflagstring + " "+"'"+ myAstroObject.name.trim() + "'";
       }
       if(!rates){
        command = "COORDS " + RAstring + " " + Decstring + " " + Equinoxstring + " " + RAMotionstring + " " + DecMotionstring + " " +"'"+ myAstroObject.name.trim() + "'";
       }
       //       command = "COORDS " + RAstring.substring(0, 10) + " " + Decstring.substring(0,10) + " " + Equinoxstring + " " + RAMotionstring + " " + DecMotionstring + " " + Mflagstring;
//       System.out.println(command);
       response = myClientSocket.sendReceiveCommand(command+TERMINATOR);
       java.lang.String statusMessage = parseResponse(response);
       setCoordinateStatusMessage(myAstroObject.name.trim() + " Response: " + statusMessage);
//       System.out.println(response);
    }
/*=============================================================================================
/         start()
/=============================================================================================*/
    public void start(){
     myThread = new Thread(this);
     myThread.start();
     }
    }// End of the SendCoordinateThread Inner Class
/*=============================================================================================
/                           End of the SendCoordinatesThread Class
/=============================================================================================*/   
/*=============================================================================================
/       INNER CLASS that moves the telescope to a new position, N,S,E,W or by Offset
/=============================================================================================*/
 public class MoveTelescopeThread  implements Runnable{
    private Thread myThread;
    private int    direction;
    private double offset;
    private double RAoffset;
    private double Decoffset;
    private int    mode;
    public  int    STEP_MODE   =  10;
    public  int    OFFSET_MODE =  20;
    public  int    RETURN_MODE =  30;
    private String commandString = new String();
    private double moveTimeEstimate = 0.0;

    private boolean state      = false;
    private java.lang.String response = new java.lang.String();
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
/          MoveTelescopeThread()
/=============================================================================================*/
    public MoveTelescopeThread(){
      mode = RETURN_MODE;
    }
/*=============================================================================================
/          MoveTelescopeThread()
/=============================================================================================*/
    public MoveTelescopeThread(int direction, double offset){
      this.direction = direction;
      this.offset    = offset;
      mode = STEP_MODE;
    }
/*=============================================================================================
/          MoveTelescopeThread()
/=============================================================================================*/
    public MoveTelescopeThread(double currentRAoffset, double currentDecoffset){
      this.RAoffset  = currentRAoffset;
      this.Decoffset = currentDecoffset;
      mode = OFFSET_MODE;
     }
/*=============================================================================================
/           run()
/=============================================================================================*/
    public void run(){
    setMoving(true);
    double starting_RAMoveRate  = TelescopeObject.MAX_RA_MOVE_RATE;
    double starting_DecMoveRate = TelescopeObject.MAX_RA_MOVE_RATE;
    if(isChangeMoveRates()){
        starting_RAMoveRate  = myTelescopeObject.getRAMoveRate();
        starting_DecMoveRate = myTelescopeObject.getDecMoveRate();
        if(starting_RAMoveRate == 0){
            starting_RAMoveRate = TelescopeObject.MAX_RA_MOVE_RATE;
            myTelescopeObject.setRAMoveRate(TelescopeObject.MAX_RA_MOVE_RATE);
        }
        if(starting_DecMoveRate == 0){
            starting_DecMoveRate = TelescopeObject.MAX_DEC_MOVE_RATE;
            myTelescopeObject.setDecMoveRate(TelescopeObject.MAX_DEC_MOVE_RATE);
        }      
    }
    state = true;
    // Calculate how long the telescope move will take
     if(mode == RETURN_MODE){
       commandString = "RET";
       myTelescopeObject.evaluateMoveDirections(-myTelescopeObject.getRA_OFFSET(),-myTelescopeObject.getDEC_OFFSET());
       moveTimeEstimate = myTelescopeObject.CalculateTelescopeMoveTimeMaxRate(-myTelescopeObject.getRA_OFFSET(),-myTelescopeObject.getDEC_OFFSET());
// RET (no arguments):  move the telescope so the DRA and DDEC offsets on the
// telescope status display go to zero.  (See Z command, below.)  Rate is
// 50 arcsec/sec.  Maximum move: 600 sec in RA, 6000 arcsec in dec.  The
// telescope must be tracking stably.
     }
     //construct the string to send to the telescope
     if(mode == STEP_MODE){
       String OffsetString  = Double.toString(offset);
       switch(direction){
        case 1 : {
          commandString = "N " + OffsetString;
          myTelescopeObject.setTelescopeMotionDirection(TelescopeObject.MOVING_N);
          moveTimeEstimate = myTelescopeObject.CalculateTelescopeMoveTime(0.0,offset);
          break;
        } 
        case 2 : {
          commandString = "E " + OffsetString;               
          myTelescopeObject.setTelescopeMotionDirection(TelescopeObject.MOVING_E);
          moveTimeEstimate = myTelescopeObject.CalculateTelescopeMoveTime(offset,0.0);
          break;
        } 
        case 3 : {
          commandString = "S " + OffsetString;               
          myTelescopeObject.setTelescopeMotionDirection(TelescopeObject.MOVING_S);
          moveTimeEstimate = myTelescopeObject.CalculateTelescopeMoveTime(0.0,offset);
          break;
        } 
        case 4 : {
          commandString = "W " + OffsetString;               
          myTelescopeObject.setTelescopeMotionDirection(TelescopeObject.MOVING_W);
          moveTimeEstimate = myTelescopeObject.CalculateTelescopeMoveTime(offset,0.0);
          break;
        } 
        }
    }
    if(mode == OFFSET_MODE){
       String RAOffsetstring  = Double.toString(RAoffset);
       String DecOffsetstring = Double.toString(Decoffset);
       myTelescopeObject.evaluateMoveDirections(RAoffset,Decoffset);
       moveTimeEstimate = myTelescopeObject.CalculateTelescopeMoveTime(RAoffset,Decoffset);
       //  Set the direction of motion of the Telescope so that the GUI can show which way it's moving
       commandString = "PT " + RAOffsetstring + " " +  DecOffsetstring; 
        //PT (two arguments):  move telescope in RA and dec simultaneously by
        //distance given in first and second arguments, respectively.  Distances are
        //in arcseconds.  Positive values move east and north.  Rates of moves are
        //set by MRATES - see above.  Moves are precessed to the display equinox.
        //Valid range: -6000 to 6000 arcsec in each axis.    
    }
    // Set the move rates in RA and Dec for the upcoming telescope move
    myTelescopeObject.setTotalElapsedTime(moveTimeEstimate);
    myUpdateTimeLine.start();
    // Set the move rates based on the values in the TelescopeObject
    if(isChangeMoveRates()){
       setMoveRates();
    }
    // Send the command to the Telescope
    response = myControlSocket.sendReceiveCommand(commandString+TERMINATOR,moveTimeEstimate);
     parseResponse(response);
    myTelescopeObject.setTelescopeMotionDirection(TelescopeObject.NOT_MOVING);   // Signal to the GUI that the telescope has stopped moving
    myTelescopeObject.setAbortArmed(false);
 //   System.out.println(response);
    if(isChangeMoveRates()){
        myTelescopeObject.setRAMoveRate(starting_RAMoveRate);
        myTelescopeObject.setDecMoveRate(starting_DecMoveRate);
       setMoveRates();
    }
    setMoving(false);
  }//
    public void start(){
     myThread = new Thread(this);
     myThread.start();
     }
    }// End of the UpdateTelescopeStatusThread Inner Class
/*================================================================================================
/       End of the P200Commands() Class Declaration
/=================================================================================================*/
/*=============================================================================================
/       INNER CLASS that sends a command to adjust the focus
/=============================================================================================*/
    public class AdjustFocusThread  implements Runnable{
    private Thread myThread;
    private java.lang.String response    = new java.lang.String();
    private java.lang.String command     = new java.lang.String();
    private double           delay       = 0.0;
    private int              focusMode   = 0;
    private double           focusChange = 0.0;
    private boolean          completed   = false;
/*=============================================================================================
/          SendCoordinatesThread()
/=============================================================================================*/
    private AdjustFocusThread(int focusMode,double focusChange){
      this.focusChange = focusChange;
      this.focusMode   = focusMode;    
    }
/*=============================================================================================
/      setCompleted(boolean newCompleted)
/=============================================================================================*/
public synchronized void setCompleted(boolean newCompleted){
     completed = newCompleted;
  }
  public synchronized boolean isCompleted(){
     return completed;
  }
/*=============================================================================================
/           run()
/=============================================================================================*/
    public void run(){
      setFocusMoving(true);
      String command = new String();
      if(focusMode == FOCUS_SET){
         command = "FOCUSGO " + Double.toString(focusChange);
         myTelescopeObject.evaluateFocusMoveDirection(focusChange);
         focusChange = focusChange - myTelescopeObject.getFOCUS();
      }  
      if(focusMode == FOCUS_OFFSET){
         command = "FOCUSINC " + Double.toString(focusChange);
            if(focusChange > 0.0){
              myTelescopeObject.setFocusMotionDirection(TelescopeObject.FOCUS_MOVING_UP);
            }
            if(focusChange < 0.0){
              myTelescopeObject.setFocusMotionDirection(TelescopeObject.FOCUS_MOVING_DOWN);
            }          
      }
      double focusMoveTime = myTelescopeObject.CalculateFocusMoveTime(focusChange);
 //     System.out.println(command);
//      response = myControlSocket.sendReceiveCommand(command+TERMINATOR,focusMoveTime);
      SendCommandThread  mySendCommandThread  = new SendCommandThread (command,focusMoveTime);
      mySendCommandThread .start();

       startFocusTimer(focusMoveTime);
       long start     = FOCUSTimer.getStartTime();
       long end       = FOCUSTimer.getEndTime();
       long totalTime = end - start;
       setCompleted(false);

       myTelescopeObject.setStatusMessage("Changing Telescope Focus");
       // Now loop until the exposure is completed
       while(!isCompleted()){
           // Now get the currentTime
           double   currentTime           = (double)(System.currentTimeMillis());
           double   elapsedTime           = (double)(currentTime - start);
           double   newPercentCompleted   = (elapsedTime/totalTime)*100.0;
           myTelescopeObject.setPercentCompleted(newPercentCompleted);
           try{
              Thread.currentThread().sleep(UPDATE_TIMELINE_DELAY);
             if(elapsedTime >  totalTime){
                 myTelescopeObject.setStatusMessage("Focus Change Completed");
                 myTelescopeObject.setPercentCompleted(0);
                 //  This is where we can trigger an event to retrieve a FITS file from the MKIR server
                 fireActionPerformed(new java.awt.event.ActionEvent(this,1,"Move Completed"));
                 setCompleted(true);
              }// end of the if elapsedTime > exposureCompletedTime check
              }// end of the try block
              catch(java.lang.InterruptedException ex3){
                   logErrorMessage("An error occurred in the UpdateFocusThread Class: "+ex3);
             }// end of the catch
       }// end of the while notCompleted loop
      myTelescopeObject.setFocusMotionDirection(TelescopeObject.FOCUS_NOT_MOVING);
      response =mySendCommandThread.response;
      parseResponse(response);
      setFocusMoving(false);
//      System.out.println(response);
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
/*=============================================================================================
/       INNER CLASS that sends a set of coordinates to the telescope
/=============================================================================================*/
    public class SendCommandThread  implements Runnable{
    private Thread myThread;
    private java.lang.String response = new java.lang.String();
    private java.lang.String command  = new java.lang.String();
    private double           delay    = 0.0;
/*=============================================================================================
/          SendCoordinatesThread()
/=============================================================================================*/
    private SendCommandThread(String newCommand,double currentDelay){
        command = newCommand;
        delay   = currentDelay;
    }
/*=============================================================================================
/           run()
/=============================================================================================*/
    public void run(){
 //      System.out.println(command);
       response = myControlSocket.sendReceiveCommand(command+TERMINATOR,delay);
       parseResponse(response);
//       System.out.println(response);
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
/*=============================================================================================
/       INNER CLASS that sends an ABORT signal to the telescope
/=============================================================================================*/
    public class SendAbortCommandThread  implements Runnable{
    private Thread myThread;
    private java.lang.String response     = new java.lang.String();
    private java.lang.String command      = new java.lang.String();
    public int               REPEAT_DELAY = 20;
    public int               MAX_RETRY    = 10;
/*=============================================================================================
/          SendAbortCommandThread()
/=============================================================================================*/
    private SendAbortCommandThread(){
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
       System.out.println(command);
       disconnect_control();
       waitForResponseMilliseconds(100);
       connect_abort();
       boolean connect_state = myAbortSocket.isConnected();
       int     retry_count = 0;
       while(!connect_state){
         waitForResponseMilliseconds(REPEAT_DELAY);
         retry_count = retry_count + 1;
         connect_state = myAbortSocket.isConnected();
         if(retry_count >= MAX_RETRY){
             connect_state = true;
//             System.out.println("Error connecting to the Abort Socket");
         }
       }
       if(myAbortSocket.isConnected()){
        command = "MOVESTOP";
        response = myAbortSocket.sendReceiveCommand(command+TERMINATOR);
         parseResponse(response);
         stopTimer();
        disconnect_abort();
        waitForResponseMilliseconds(100);
        connect_control();
        waitForResponseMilliseconds(100);
        myTelescopeObject.setMoving(false);
        myTelescopeObject.setTelescopeMotionDirection(TelescopeObject.NOT_MOVING);
        myTelescopeObject.setStatusMessage("Moving Aborted");
        myTelescopeObject.setPercentCompleted(0);
        myUpdateTimeLine.setCompleted(true);
        myTelescopeObject.setAbortArmed(false);
        }
//       System.out.println(response);
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
/                           End of the SendAbortCommandThread Class
/=============================================================================================*/

/*
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

struct {				/* 100 BYTES TOTAL */ /*
        short 	ra_hours;		/* COORDINATES ALWAYS J2000.0 */
/*	short	ra_minutes;
	short	ra_seconds;
	short	ra_hundredth_seconds;
	short	dummyword_4;
	short	dec_degrees;
	short	dec_minutes;
	short	dec_seconds;
	short	dec_tenth_seconds;
	short	dec_sign;		/* 1 = POS, 0 = NEG */
/*	short	lst_hours;
	short	lst_minutes;
	short	lst_seconds;
	short	lst_tenth_seconds;
	short	dummyword_14;
	short	ha_hours;		/* HA IS APPARENT */
/*	short	ha_minutes;
	short	ha_seconds;
	short	ha_tenth_seconds;
	short	ha_direction;		/* 1 = EAST, 0 = WEST */
/*	short	ut_hours;
	short	ut_minutes;
	short	ut_seconds;
	short	ut_tenth_seconds;
	float	airmass;
	float	display_equinox;	/* J2000.0 */
/*	short	dummyword_28;
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
	short	telescope_id;		/* 200 or 60 */
/*} telescope_data;

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
TCP service (port): 5004
[A sample client program, written in C, is available.]
*/






















}
