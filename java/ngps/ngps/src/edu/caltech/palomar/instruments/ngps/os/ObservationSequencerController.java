/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package edu.caltech.palomar.instruments.ngps.os;
import edu.caltech.palomar.instruments.ngps.dbms.NGPSdatabase;
import edu.caltech.palomar.io.ClientSocket;
import edu.caltech.palomar.util.general.CommandLogModel;
import java.beans.PropertyChangeListener;
import java.beans.PropertyChangeSupport;
import java.io.IOException;
import java.lang.reflect.Field;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.MulticastSocket;
import java.net.SocketTimeoutException;
import java.util.Vector;
import org.apache.log4j.BasicConfigurator;
import org.apache.log4j.DailyRollingFileAppender;
import org.apache.log4j.Layout;
import org.apache.log4j.Logger;
import org.apache.log4j.PatternLayout;
        
//=== File Prolog =============================================================
//	This code was developed by California Institute of Technology
//      Caltech Optical Observatories, Palomar Observatory
//
//--- Contents ----------------------------------------------------------------    
//       ObservationSequencerController
//
//--- Description -------------------------------------------------------------
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
//
//      December 14, 2022 Jennifer Milburn
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
public class ObservationSequencerController {
    transient public PropertyChangeSupport propertyChangeListeners = new PropertyChangeSupport(this);
    private transient Vector               actionListeners;
    public  java.lang.String          SEP                = System.getProperty("file.separator");
    private static java.lang.String   TERMINATOR         ="\n";
    public  java.lang.String          USERDIR            = System.getProperty("user.dir");
    static  Logger                    dheLogger          = Logger.getLogger(ObservationSequencerController.class);
    public  DailyRollingFileAppender  fileAppender       = new DailyRollingFileAppender();
    private Layout                    layout             = new PatternLayout("%-5p [%t]: %m%n");
    public  IniReader                 myIniReader;
    public  DHEStringUtilities        stringUtil         = new DHEStringUtilities();
    public ClientSocket               myCommandSocket    = new ClientSocket(); 
    public ClientSocket               myBlockingSocket   = new ClientSocket();
//    public ClientSocket               myAsyncSocket      = new ClientSocket();
    public CommandLogModel            myCommandLogModel  = new CommandLogModel();
    public CommandLogModel            myAsyncLogModel    = new CommandLogModel();
    private  boolean                  async_connected;
    private boolean                   error;   
    public static int                 COMMAND             = 0;
    public static int                 BLOCKING            = 1;
    public double                     DEfAULT_DELAY       = 0.1;
    public static int                 DEBUG               = 0;
    public static int                 INFO                = 1;
    public static int                 WARN                = 2;
    public static int                 ERROR               = 3;
    public static int                 FATAL               = 4;
    public static int                 DO_ONE              = 1;
    public static int                 DO_ALL              = 2;
    private java.lang.String          state               = new java.lang.String();
    private int                       elapsed_time_1;
    private int                       elapsed_time_2;
    private int                       elapsed_time_3;
    private int                       elapsed_time_4;         
    private int                       pixel_count_1;
    private int                       pixel_count_2;
    private int                       pixel_count_3;
    private int                       pixel_count_4;
    private boolean                   update_armed = false;
    public NGPSdatabase               dbms;
    private int                       total_time;
    private int                       progress;
    private int                       overhead_progress;
    private int                       image_size;
    private java.lang.String          overhead_progress_string;
    private java.lang.String          progress_string;
    private java.lang.String          simulator_address  = new java.lang.String();
    private java.lang.String          tcs_address        = new java.lang.String();
    private java.lang.String          active_tcs_address = new java.lang.String();
    private boolean                   tcs_connected;
    private boolean                   tcs_connected_in_progress;
    private java.lang.String          active_tcs_name    = new java.lang.String();
/*=============================================================================================
/         initializeLogging()
/=============================================================================================*/
public  ObservationSequencerController(){
    myIniReader = new IniReader();
    initializeLogging();
    initializeSockets();
    boolean test = false;
    if(test){
       connect();
       while(test){
          state();
       }
    }
} 
/*=============================================================================================
/    setNGPSdatabase(NGPSdatabase new_dbms)
/=============================================================================================*/
public void setNGPSdatabase(NGPSdatabase new_dbms){
   dbms = new_dbms;   
}
/*=============================================================================================
/        connect()
/=============================================================================================*/
public void connect(){
    myCommandSocket.startConnection(ClientSocket.USE_HOSTNAME);
    myBlockingSocket.startConnection(ClientSocket.USE_HOSTNAME);
    start_AsynchronousMonitor();
    waitMilliseconds(2000);
    state();
}
/*================================================================================================
/      waitForResponse(int newDelay)
/=================================================================================================*/
public void query_tcs_connection(){
    tcs_connect();
    tcs_list();
    tcs_isOpen();    
}
/*================================================================================================
/      waitForResponse(int newDelay)
/=================================================================================================*/
public void waitMilliseconds(int newDelay){
     try{
        Thread.currentThread().sleep(newDelay);} // sleep for awhile to let the messages pile up
      catch (Exception e){
    }
}
/*=============================================================================================
/         initializeLogging()
/=============================================================================================*/
 private void initializeLogging(){
     BasicConfigurator.configure();
     fileAppender.setFile(myIniReader.LOG_DIRECTORY + SEP + myIniReader.INSTRUMENT_NAME+SEP+"OS_SERVER.log");
     fileAppender.setDatePattern("'.'yyyy-MM-dd");
     fileAppender.setAppend(true);
     fileAppender.setLayout(layout);
     fileAppender.activateOptions();
     dheLogger .addAppender(fileAppender);
 }
/*================================================================================================
/        jbInit() initialization method
/=================================================================================================*/
  public void initializeSockets(){
        initializeLogging(); 
        stringUtil.setInstrumentName(myIniReader.INSTRUMENT_NAME);
        setAsyncConnected(false);
        myCommandSocket = new ClientSocket(myIniReader.SERVERNAME,myIniReader.COMMAND_SERVERPORT);        
        myBlockingSocket = new ClientSocket(myIniReader.SERVERNAME,myIniReader.BLOCKING_SERVERPORT);
  }
  
/*=============================================================================================
/     start_AsynchronousMonitor()
/=============================================================================================*/
public void start_AsynchronousMonitor(){
   AsynchronousMonitorThread myAsynchronousMonitorThread = new AsynchronousMonitorThread();
   myAsynchronousMonitorThread.start();
}
/*================================================================================================
/   setAttribute()
/      Set the value of any String attribute of this class
/=================================================================================================*/
public void setAttributeString(String fieldToSet, String newValue) {
    Class<?>c = this.getClass();
    try{
        Field f = c.getDeclaredField(fieldToSet);
        String oldValue = (String) f.get(this);
        f.set(this, newValue);
        propertyChangeListeners.firePropertyChange(fieldToSet, oldValue, newValue);
    } catch(Exception e){
      System.out.println(e.toString());
    }  
 }
/*================================================================================================
/   setAttribute()
/      Set the value of any String attribute of this class
/=================================================================================================*/
public void setAttributeBool(String fieldToSet, Boolean newValue) {
    Class<?>c = this.getClass();
    try{
        Field f = c.getDeclaredField(fieldToSet);
        Boolean oldValue = (Boolean) f.get(this) ;
        f.set(this, newValue);
        propertyChangeListeners.firePropertyChange(fieldToSet, oldValue, newValue);
    } catch(Exception e){
      System.out.println(e.toString());
    }  
 }

/*================================================================================================
/       setAsyncConnected(boolean new_async_connected)
/=================================================================================================*/
  public void setAsyncConnected(boolean new_async_connected) {
    boolean  old_async_connected = this.async_connected;
    this.async_connected = new_async_connected;
    propertyChangeListeners.firePropertyChange("async_connected", Boolean.valueOf(old_async_connected), Boolean.valueOf(new_async_connected));
  }
/*================================================================================================
/      setERROR(boolean new_error)
/=================================================================================================*/
  public void setERROR(boolean new_error) {
    boolean  old_error = this.error;
    this.error = new_error;
    propertyChangeListeners.firePropertyChange("error", Boolean.valueOf(old_error), Boolean.valueOf(new_error));
  }
  public boolean isERROR() {
    return error;
  }
 /*================================================================================================
/     setProgress(int new_progress)
/=================================================================================================*/
  public void setProgress(int new_progress) {
    int  old_progress = this.progress;
    this.progress = new_progress;
    propertyChangeListeners.firePropertyChange("progress", Integer.valueOf(old_progress), Integer.valueOf(new_progress));
  }
/*================================================================================================
/     setProgress(int new_progress)
/=================================================================================================*/
  public void setProgressString(java.lang.String new_progress_string) {
    java.lang.String  old_progress_string = this.progress_string;
    this.progress_string = new_progress_string;
    propertyChangeListeners.firePropertyChange("progress_string", old_progress_string, new_progress_string);
  }
/*================================================================================================
/     setProgress(int new_progress)
/=================================================================================================*/
  public void setOverheadProgressString(java.lang.String new_overhead_progress_string) {
    java.lang.String  old_overhead_progress_string = this.overhead_progress_string;
    this.overhead_progress_string = new_overhead_progress_string;
    propertyChangeListeners.firePropertyChange("overhead_progress_string", old_overhead_progress_string, new_overhead_progress_string);
  }
  /*================================================================================================
/     setOverheadProgress(int new_overhead_progress)
/=================================================================================================*/
  public void setOverheadProgress(int new_overhead_progress) {
    int  old_overhead_progress = this.overhead_progress;
    this.overhead_progress = new_overhead_progress;
    propertyChangeListeners.firePropertyChange("overhead_progress", Integer.valueOf(old_overhead_progress),Integer.valueOf(new_overhead_progress));
  }
/*================================================================================================
/       setAsyncConnected(boolean new_async_connected)
/=================================================================================================*/
  public void setTotalEXPTime(int new_total_time) {
    int  old_total_time = this.total_time;
    this.total_time = new_total_time;
    propertyChangeListeners.firePropertyChange("total_time", Integer.valueOf(old_total_time), Integer.valueOf(new_total_time));
  }
/*================================================================================================
/       setAsyncConnected(boolean new_async_connected)
/=================================================================================================*/
  public void setElapsedTime_1(int new_elapsed_time_1) {
    int  old_elapsed_time_1 = this.elapsed_time_1;
    this.elapsed_time_1 = new_elapsed_time_1;
    propertyChangeListeners.firePropertyChange("elapsed_time_1", Integer.valueOf(old_elapsed_time_1), Integer.valueOf(new_elapsed_time_1));
  }
/*================================================================================================
/       setAsyncConnected(boolean new_async_connected)
/=================================================================================================*/
  public void setElapsedTime_2(int new_elapsed_time_2) {
    int  old_elapsed_time_2 = this.elapsed_time_2;
    this.elapsed_time_2 = new_elapsed_time_2;
    propertyChangeListeners.firePropertyChange("elapsed_time_2", Integer.valueOf(old_elapsed_time_2), Integer.valueOf(new_elapsed_time_2));
  }
/*================================================================================================
/       setAsyncConnected(boolean new_async_connected)
/=================================================================================================*/
  public void setElapsedTime_3(int new_elapsed_time_3) {
    int  old_elapsed_time_3 = this.elapsed_time_3;
    this.elapsed_time_3 = new_elapsed_time_3;
    propertyChangeListeners.firePropertyChange("elapsed_time_3", Integer.valueOf(old_elapsed_time_3), Integer.valueOf(new_elapsed_time_3));
  }
/*================================================================================================
/       setAsyncConnected(boolean new_async_connected)
/=================================================================================================*/
  public void setElapsedTime_4(int new_elapsed_time_4) {
    int  old_elapsed_time_4 = this.elapsed_time_4;
    this.elapsed_time_4 = new_elapsed_time_4;
    propertyChangeListeners.firePropertyChange("elapsed_time_4", Integer.valueOf(old_elapsed_time_4), Integer.valueOf(new_elapsed_time_4));
  }
 /*================================================================================================
/      setPixelCount_1(int new_pixel_count_1)
/=================================================================================================*/
  public void setImageSize(int new_image_size) {
    int  old_image_size = this.image_size;
    this.image_size = new_image_size;
    propertyChangeListeners.firePropertyChange("image_size", Integer.valueOf(old_image_size),Integer.valueOf(new_image_size));
  }
/*================================================================================================
/      setPixelCount_1(int new_pixel_count_1)
/=================================================================================================*/
  public void setPixelCount_1(int new_pixel_count_1) {
    int  old_pixel_count_1 = this.pixel_count_1;
    this.pixel_count_1 = new_pixel_count_1;
    propertyChangeListeners.firePropertyChange("pixel_count_1", Integer.valueOf(old_pixel_count_1),Integer.valueOf(new_pixel_count_1));
  }
/*================================================================================================
/      setPixelCount_1(int new_pixel_count_1)
/=================================================================================================*/
  public void setPixelCount_2(int new_pixel_count_2) {
    int  old_pixel_count_2 = this.pixel_count_2;
    this.pixel_count_2 = new_pixel_count_2;
    propertyChangeListeners.firePropertyChange("pixel_count_2", Integer.valueOf(old_pixel_count_2), Integer.valueOf(new_pixel_count_2));
  }
/*================================================================================================
/      setPixelCount_1(int new_pixel_count_1)
/=================================================================================================*/
  public void setPixelCount_3(int new_pixel_count_3) {
    int  old_pixel_count_3 = this.pixel_count_3;
    this.pixel_count_3 = new_pixel_count_3;
    propertyChangeListeners.firePropertyChange("pixel_count_3", Integer.valueOf(old_pixel_count_3), Integer.valueOf(new_pixel_count_3));
  }
/*================================================================================================
/      setPixelCount_1(int new_pixel_count_1)
/=================================================================================================*/
  public void setPixelCount_4(int new_pixel_count_4) {
    int  old_pixel_count_4 = this.pixel_count_4;
    this.pixel_count_4 = new_pixel_count_4;
    propertyChangeListeners.firePropertyChange("pixel_count_4", Integer.valueOf(old_pixel_count_4), Integer.valueOf(new_pixel_count_4));
  }
/*================================================================================================
/      executeCommand(int socket,java.lang.String command,double delay)
/=================================================================================================*/
  public String executeCommand(String command){
      // RETURNS ACK or ERR
      logMessage(INFO,command);
      System.out.println("COMMAND: "+command);

      String response = myCommandSocket.sendReceiveCommandSequencer(command+TERMINATOR);

      // Handle null response; converts to "null"
      response = String.valueOf(response);
      System.out.println("RESPONSE: "+response);      
      logMessage(INFO,response);
       
        if(response.contains("ACK")){ // Sequencer always finishes line with DONE if successful
            setERROR(false);
        } 
        else if(response.isEmpty() || response=="null"){
            setERROR(true);
            myCommandLogModel.insertMessage(CommandLogModel.ERROR, command+"--> No sequencer response (maybe nothing was sent)");
            }
        else {
            setERROR(true);
            myCommandLogModel.insertMessage(CommandLogModel.ERROR, command+"--> "+response);
            }
        
    return response;
  }  
 /*================================================================================================
/      executeCommand(int socket,java.lang.String command,double delay)
/=================================================================================================*/
  public String executeCommandB(String command){
        logMessage(INFO,command);
        System.out.println("BLOCKING COMMAND: "+command);

      //  System.out.println("CONNECTION 1 == "+String.valueOf(myBlockingSocket.isConnected())); 
      String response = myBlockingSocket.sendReceiveCommandSequencer(command+TERMINATOR);

      // Handle null response; converts to "null"
      response = String.valueOf(response);
      System.out.println("BLOCKING RESPONSE: "+response);      
      logMessage(INFO,response);
       
        if(response.contains("DONE")){ // Sequencer always finishes line with DONE if successful
            setERROR(false);
        } 
        else if(response.isEmpty() || response=="null"){
            setERROR(true);
            myCommandLogModel.insertMessage(CommandLogModel.ERROR, command+"--> No sequencer response (maybe nothing was sent)");
            }
        else {
            setERROR(true);
            myCommandLogModel.insertMessage(CommandLogModel.ERROR, command+"--> "+response);
            }
        
    return response;
  }  
/*=============================================================================================
/        logMessage(int code,java.lang.String message)
/=============================================================================================*/
public void logMessage(int code,java.lang.String message){

     switch(code){
         case 0:  dheLogger.debug(message);  break;
         case 1:  dheLogger.info(message);   break;
         case 2:  dheLogger.warn(message);   break;
         case 3:  dheLogger.error(message);  break;
         case 4:  dheLogger.fatal(message);  break;         
     }
     if((code == ERROR)|(code ==FATAL)){
//         myDHEObject.setERROR(true);
     }
 }
/*================================================================================================
/      abort()
/=================================================================================================*/
public boolean abort(){
    String response = executeCommand("abort");
    return isERROR();
} 
/*================================================================================================
 /      airmass(double airmass_limit)
 /=================================================================================================*/
public boolean airmass(double airmass_limit){
    String command = "airmass "+Double.toString(airmass_limit);
    java.lang.String response = executeCommandB(command);

    return isERROR();
}
/*================================================================================================
/      do_one_all(int code){
/=================================================================================================*/
public boolean do_one_all(int code){
    String command = new String();
    if(code == DO_ONE){
        command = "do one";
    }
    if(code == DO_ALL){
        command = "do all";
    }

    java.lang.String response = executeCommandB(command);

    return isERROR();
}
/*================================================================================================
/      modexptime()
/=================================================================================================*/
public boolean modexptime(double mod_exptime){
    String command = "modexptime "+Double.toString(mod_exptime);
    java.lang.String response = executeCommandB(command);

    return isERROR();
}
/*================================================================================================
/      pause()
/=================================================================================================*/
public boolean pause(){
    String response = executeCommand("pause");

     return isERROR();
} 
/*================================================================================================
/     resume()
/=================================================================================================*/
public boolean resume(){
    String response = executeCommand("resume");
    return isERROR();
} 
/*================================================================================================
/     shutdown()
/=================================================================================================*/
public boolean shutdown(){
    String response = executeCommand("shutdown");
    return isERROR();
} 
/*================================================================================================
/     start()
/=================================================================================================*/
public boolean start(){
    String response = executeCommand("start");
    return isERROR();
} 
/*================================================================================================
/     startup()
/=================================================================================================*/
public boolean startup(){
    String response = executeCommand("startup");
    return isERROR();
} 
/*================================================================================================
/     stop()
/=================================================================================================*/
public boolean stop(){
    String response = executeCommand("stop");
   return isERROR();
} 
/*================================================================================================
/    state()
/=================================================================================================*/
public boolean state(){
    String response = executeCommand("state"); 
    return isERROR();
}
/*================================================================================================
/    targetset(int id)
/=================================================================================================*/
public void armUpdate(){
    update_armed = true;
}
public void disarmUpdate(){
    update_armed = false;
}
public boolean isUpdateArmed(){
    return update_armed;
}
/*================================================================================================
/     stop()
/=================================================================================================*/
public void tcs_list(){
   String response = executeCommandB("tcs llist");
   // returns something like "real 10.200.99.2:49200 false,sim localhost:49200 false DONE"
   try{
      java.lang.String[] records = response.split(",");
      java.lang.String[] record1 = records[0].split(" ");
      java.lang.String[] record2 = records[1].split(" ");  
      setSimulatorAddress(record2[1]);
      setTCSAddress(record1[1]);
      System.out.println(records);
   }catch(Exception e){
       System.out.println("Problem parsing the tcs llist output.");
   }
} 
/*================================================================================================
/     setSimulatorAddress(java.lang.String new_simulator_address)
/=================================================================================================*/
  public void setSimulatorAddress(java.lang.String new_simulator_address) {
    java.lang.String  old_simulator_address = this.simulator_address;
    this.simulator_address = new_simulator_address;
    propertyChangeListeners.firePropertyChange("simulator_address", old_simulator_address, new_simulator_address);
  }
  public java.lang.String getSimulatorAddress() {
    return simulator_address;
  }  
 /*================================================================================================
/     setSimulatorAddress(java.lang.String new_simulator_address)
/=================================================================================================*/
  public void setTCSAddress(java.lang.String new_tcs_address) {
    java.lang.String  old_tcs_address = this.tcs_address;
    this.tcs_address = new_tcs_address;
    propertyChangeListeners.firePropertyChange("tcs_address", old_tcs_address, new_tcs_address);
  }
  public java.lang.String getTCSAddress() {
    return tcs_address;
  }  
/*================================================================================================
/     setSimulatorAddress(java.lang.String new_simulator_address)
/=================================================================================================*/
  public void setActiveTCSAddress(java.lang.String new_active_tcs_address) {
    java.lang.String  old_active_tcs_address = this.active_tcs_address;
    this.active_tcs_address = new_active_tcs_address;
    propertyChangeListeners.firePropertyChange("active_tcs_address", old_active_tcs_address, new_active_tcs_address);
  }
/*================================================================================================
/     setSimulatorAddress(java.lang.String new_simulator_address)
/=================================================================================================*/
  public void setActiveTCSname(java.lang.String new_active_tcs_name) {
    java.lang.String  old_active_tcs_name = this.active_tcs_name;
    this.active_tcs_name = new_active_tcs_name;
    propertyChangeListeners.firePropertyChange("active_tcs_name", old_active_tcs_name, new_active_tcs_name);
  }
/*================================================================================================
/       setCommandConnected(boolean new_command_connected)
/=================================================================================================*/
  public void setTCSConnected(boolean new_tcs_connected) {
    boolean  old_tcs_connected = this.tcs_connected;
    this.tcs_connected = new_tcs_connected;
    propertyChangeListeners.firePropertyChange("tcs_connected", Boolean.valueOf(old_tcs_connected), Boolean.valueOf(new_tcs_connected));
  }
  /*================================================================================================
/       setCommandConnected(boolean new_command_connected)
/=================================================================================================*/
  public void setTCSConnectedInProgress(boolean new_tcs_connected_in_progress) {
    boolean  old_tcs_connected_in_progress = this.tcs_connected_in_progress;
    this.tcs_connected_in_progress = new_tcs_connected_in_progress;
    propertyChangeListeners.firePropertyChange("tcs_connected_in_progress", Boolean.valueOf(old_tcs_connected_in_progress), Boolean.valueOf(new_tcs_connected_in_progress));
  }
/*================================================================================================
/   tcs_connect( 
/=================================================================================================*/
public boolean tcs_connect(){
    String command = "tcs connect";
    java.lang.String response = executeCommandB(command);
    return isERROR();
} 
/*================================================================================================
/   tcs_isOpen()
/     Ask if sequencer is connected to tcsd (TCS daemon)
/=================================================================================================*/
//public boolean tcs_getName(){ 
//    // tcs getname --> sim or real
//}


public boolean tcs_isOpen(){
   String response = executeCommandB("tcs isopen");  
   return isERROR();
} 
/*================================================================================================
/   tcs_isConnected()
      Ask if TCS daemon is connected to TCS
/=================================================================================================*/
public boolean tcs_isConnected(){
   boolean isconnected = false;
   String command = "tcs isconnected";
   java.lang.String response = executeCommandB(command);   
   response = response.replace(" DONE","");
   response = response.trim();
   if(response.matches("false")){
      isconnected = false; 
   }else if(response.matches("true")){
     isconnected = true; 
   } 
   return isconnected; 
} 
/*================================================================================================
/   tcsinit(String realOrSim)
/     Tell TCS daemon to connect to TCS
/=================================================================================================*/
public void tcsinit(String realOrSim){
    if( !(realOrSim.equalsIgnoreCase("sim") || realOrSim.equalsIgnoreCase("real")  ) ){
        throw new IllegalArgumentException("tcsinit() argument must be 'real' or 'sim' ");
    }
    String command = "tcsinit " + realOrSim;
    String response = executeCommandB(command);
}
/*================================================================================================
/    evaluateState(String stateString)  // CHAZ ADDED
/=================================================================================================*/
public synchronized void evaluateState(String stateString){
    
    System.out.println("statestring = "+stateString);
    
    if(!stateString.contains("RUNSTATE:")){ return; }
    stateString = stateString.replace("RUNSTATE:", "");
    
    if(stateString.contains("READY")){
       setSTATE("READY");
       setProgress(0);
       setProgressString("");
       setOverheadProgress(0);
       setOverheadProgressString("");
    }  
    
    if(stateString.contains("RUNNING")){
        
//        List<String> runOpts = List.of("TCSOP", "SLEW", "ACQUIRE", "EXPOSING", "READOUT");
        String[] runOpts = {"TCSOP", "SLEW", "ACQUIRE", "EXPOSING", "READOUT"};
        
        for(String s : runOpts){
            if (stateString.contains(s)) {
                setSTATE(s);
            } else {setSTATE("RUNNING");
            }
        }
    } // end of if RUNNING

//    List<String> stateOpts = List.of("OFFLINE", "STARTING", "SHUTTING", "PAUSED", "STOPREQ","ABORTREQ");
    String[] stateOpts = {"OFFLINE", "STARTING", "SHUTTING", "PAUSED", "STOPREQ","ABORTREQ"};
    
    for(String s : stateOpts){
        if (stateString.contains(s)) {setSTATE(s); } 
    }

    this.notifyAll();
}
/*================================================================================================
/      add and remove Property Change Listeners
/=================================================================================================*/
public void parseAsyncMessage(java.lang.String message){
 java.text.DecimalFormat df                 = new java.text.DecimalFormat("0.00");
 java.util.ArrayList     runstate_arraylist = new java.util.ArrayList();
   myAsyncLogModel.insertMessage(message);
   try{
     if(message.trim().startsWith("ERROR")){
        myCommandLogModel.insertMessage(CommandLogModel.ERROR, message);
     }
     if(message.trim().startsWith("NOTICE")){
        myCommandLogModel.insertMessage(CommandLogModel.COMMAND, message);
     }
     if(message.trim().startsWith("GUICLEAR")){
        myCommandLogModel.clearDocument();
     }

     // Refresh connection to sequencerd
     if(message.startsWith("SEQUENCERD:started")){
        connect();  
     }
          
     if(message.startsWith("TCSD:open")){
        String[] messages = message.split(":"); // e.g. TCSD:open:true
        if(messages[2].contains("true")){   // if Boolean.valueOf(messages[2])...
            setTCSConnectedInProgress(false);
            setTCSConnected(true);
        }else if(messages[2].contains("false")){
            setTCSConnected(false);
            setActiveTCSAddress("NOT CONNECTED");
            setActiveTCSname("NOT CONNECTED");
        }
     }

     if(message.startsWith("TCSD:name")){
        String[] messages = message.split(":"); // e.g. TCSD:name:real or sim
        setActiveTCSname(messages[2]);
        if(messages[2].contains("sim")){
            setActiveTCSAddress(simulator_address);
        }else if(messages[2].contains("real")){
            setActiveTCSAddress(tcs_address);
        }else{
            setActiveTCSAddress("UNKNOWN");
        }
         
     }     

     if(message.startsWith("TARGETSTATE")){
        // Example TARGETSTATE:active TARGET:ZTF20ackgfep OBSID:187
        String[] messages = message.split(" ");
        java.lang.String current_state = messages[0].replace("TARGETSTATE:","");
        current_state = current_state.trim();
        java.lang.String current_target = messages[1].replace("TARGET:","");
        current_target = current_target.trim();
        java.lang.String current_obsid = messages[2].replace("OBSIDT:","");
        current_obsid = current_obsid.trim();
        //dbms.executeQueryState(dbms.selectedObservationSet,dbms.myTargetDBMSTableModel); 
        dbms.queryState(dbms.selectedObservationSet,dbms.myTargetDBMSTableModel);
     }
     if(message.startsWith("RUNSTATE")){
        evaluateState(message);
     }
     if(message.startsWith("ELAPSEDTIME")){
         // message looks like "ELAPSEDTIME_n:xxx EXPTIME:yyy" where xxx and yyy are ints
         
         String[] msgList = message.split(" ");
         int elapse_time_milliseconds = Integer.parseInt( msgList[0].split(":")[1] );
         int total_time_milliseconds = Integer.parseInt( msgList[1].split(":")[1] );
         
         // setter _n are off from message _n by 1
         if(message.contains("_0")){ setElapsedTime_1(elapse_time_milliseconds); }
         if(message.contains("_1")){ setElapsedTime_2(elapse_time_milliseconds); }
         if(message.contains("_2")){ setElapsedTime_3(elapse_time_milliseconds); }
         if(message.contains("_3")){ setElapsedTime_4(elapse_time_milliseconds); }

         setTotalEXPTime(total_time_milliseconds);  
         double current_progress = ((double)elapse_time_milliseconds/(double)total_time_milliseconds)*100.0;
         setProgress((int)current_progress);
         double time_in_seconds = (double)elapse_time_milliseconds/1000.0;
         setProgressString("ELAPSED TIME = "+time_in_seconds);
         System.out.println("PROGRESS = "+current_progress);

     }
     if(message.startsWith("PIXELCOUNT")){
         // message looks like "PIXELCOUNT_n:xxx IMAGESIZE:yyy" where xxx and yyy are ints
         String[] msgList = message.split(" ");
         int pixel_count = Integer.parseInt( msgList[0].split(":")[1] );
         int total_pixels = Integer.parseInt( msgList[1].split(":")[1] );

         // setter _n are off from message _n by 1
         if(message.contains("_0")){ setPixelCount_1(pixel_count); }
         if(message.contains("_1")){ setPixelCount_2(pixel_count); }
         if(message.contains("_2")){ setPixelCount_3(pixel_count); }
         if(message.contains("_3")){ setPixelCount_4(pixel_count); }

         setImageSize(total_pixels);
         if(pixel_count != 0){
           double current_progress = ((double)pixel_count/(double)total_pixels)*100.0;
           setOverheadProgress((int)current_progress);
           setOverheadProgressString("READOUT % ="+df.format(current_progress));
           System.out.println("READOUT % = "+current_progress);
         }
         
     }
   }catch(Exception e){
      System.out.println("ERROR PARSING ASYNC MESSAGE"+e.toString());
   }
}
/*================================================================================================
/    targetset(int id)
/=================================================================================================*/
public boolean targetset(int id){
    String command = "targetset "+Integer.toString(id);
   java.lang.String response = executeCommandB(command);

   return isERROR();
} 
/*================================================================================================
/    targetset(java.lang.String target_set_name)
/=================================================================================================*/
public boolean targetset(java.lang.String target_set_name){
    String command = "targetset "+target_set_name;
     java.lang.String response = executeCommandB(command);

    return isERROR();
} 
/*================================================================================================
/      setReadmode(java.lang.String new_Readmode)
/=================================================================================================*/
  public synchronized void setSTATE(java.lang.String new_state) {
    java.lang.String  old_state = this.state;
    this.state = new_state;
    propertyChangeListeners.firePropertyChange("STATE", (old_state), (new_state));
  }
  public java.lang.String getSTATE() {
    return state;
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
/       INNER CLASS that sends a set of coordinates to the telescope
/=============================================================================================*/
    public class AsynchronousMonitorThread  implements Runnable{
    private Thread myThread;
    private java.lang.String response = new java.lang.String();
    private java.lang.String command  = new java.lang.String();
    private double           delay    = 0.0;
    private int              socket   = 0;
    private int              POLLING_DELAY = 500;
    private InetAddress      address;
    private DatagramSocket   datagram_socket;
    private MulticastSocket  multi_socket;
    private int              async_port;
    private java.lang.String async_host;
/*=============================================================================================
/          SendCoordinatesThread()
/=============================================================================================*/
    private AsynchronousMonitorThread(){      
        initialize();
//        async_port = 1300;
//        async_host = "239.1.1.234";
   }
/*=============================================================================================
/       )initialize()
/===========================================================================crsw==================*/
private void initialize(){
   try{
     async_port      = myIniReader.ASYNC_SERVERPORT;
     async_host      = myIniReader.ASYNC_HOST;
     address         = InetAddress.getByName(async_host); 
     multi_socket    = new MulticastSocket(async_port);  
     multi_socket.joinGroup(address);
   }catch(Exception e){
      System.out.println(e.toString());
   }
}
/*=============================================================================================
/           run()
/===========================================================================crsw==================*/
    public void run(){  
        try {
            while (true) {
//                DatagramPacket request = new DatagramPacket(new byte[1], 1, address, async_port);
//                multi_socket.send(request);
 
                byte[] buffer = new byte[1024];
                DatagramPacket response = new DatagramPacket(buffer, buffer.length);
                multi_socket.receive(response);
                
                String async_message = new String(buffer, 0, response.getLength());
                parseAsyncMessage(async_message);
 
                System.out.println(async_message);
//                waitForResponseMilliseconds(1);
            }
 
        } catch (SocketTimeoutException ex) {
            System.out.println("Timeout error: " + ex.getMessage());
            ex.printStackTrace();
        } catch (IOException ex) {
            System.out.println("Client error: " + ex.getMessage());
            ex.printStackTrace();
        } catch(Exception e){
            System.out.println("Unspecified Error: "+e.toString());
        }
   }
 /*=============================================================================================
/         start()
/=============================================================================================*/
    public void start(){
     myThread = new Thread(this);
     myThread.start();
     }
    }// End of the SendCommandThread Inner Class
    public static void main(String args[]) {
        /* Set the Nimbus look and feel */
        //<editor-fold defaultstate="collapsed" desc=" Look and feel setting code (optional) ">
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
            java.util.logging.Logger.getLogger(ObservationSequencerController.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (InstantiationException ex) {
            java.util.logging.Logger.getLogger(ObservationSequencerController.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (IllegalAccessException ex) {
            java.util.logging.Logger.getLogger(ObservationSequencerController.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (javax.swing.UnsupportedLookAndFeelException ex) {
            java.util.logging.Logger.getLogger(ObservationSequencerController.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        }
        //</editor-fold>

        /* Create and display the form */
        java.awt.EventQueue.invokeLater(new Runnable() {
            public void run() {
                new ObservationSequencerController();
            }
        });
    }
}
