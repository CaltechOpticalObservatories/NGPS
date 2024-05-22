/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package edu.caltech.palomar.instruments.ngps.os;
import edu.caltech.palomar.instruments.ngps.dbms.NGPSdatabase;
import edu.caltech.palomar.io.ClientSocket;
import edu.caltech.palomar.util.general.CommandLogModel;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.beans.PropertyChangeSupport;
import java.util.Vector;
import org.apache.log4j.BasicConfigurator;
import org.apache.log4j.DailyRollingFileAppender;
import org.apache.log4j.Layout;
import org.apache.log4j.Logger;
import org.apache.log4j.PatternLayout;
import java.net.InetAddress;
import java.net.DatagramSocket;
import java.net.DatagramPacket;
import java.net.SocketTimeoutException;
import java.io.IOException;
import edu.caltech.palomar.instruments.ngps.os.IniReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.net.MulticastSocket;
import java.net.Socket;
import java.util.StringTokenizer;
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
    private  boolean                  command_connected;
    private  boolean                  blocking_connected;
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
    private java.lang.String          runstate            = new java.lang.String();
    private java.lang.String          reqstate            = new java.lang.String();
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
    waitMilliseconds(10);
    state();
}
/*================================================================================================
/      waitForResponse(int newDelay)
/=================================================================================================*/
public void query_tcs_connection(){
    tcs_connect();
    tcs_list();
    boolean isOpen = tcs_isOpen();    
}
/*================================================================================================
/      waitForResponse(int newDelay)
/=================================================================================================*/
private void waitMilliseconds(int newDelay){
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
        setCommandConnected(false);
        setBlockingConnected(false);
        setAsyncConnected(false);
        myCommandSocket = new ClientSocket(myIniReader.SERVERNAME,myIniReader.COMMAND_SERVERPORT);
        myCommandSocket.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
        public void propertyChange(java.beans.PropertyChangeEvent e) {
            myCommandSocket_propertyChange(e);
         }
        });
        myBlockingSocket = new ClientSocket(myIniReader.SERVERNAME,myIniReader.BLOCKING_SERVERPORT);
        myBlockingSocket.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
        public void propertyChange(java.beans.PropertyChangeEvent e) {
            myBlockingSocket_propertyChange(e);
         }
        });
  }
/*=============================================================================================
/     Property Change Listener for the _propertyChange
/=============================================================================================*/
  private void myCommandSocket_propertyChange(PropertyChangeEvent e)  {
     java.lang.String propertyName = e.getPropertyName();
       if(propertyName == "connected"){
        boolean state = (java.lang.Boolean)e.getNewValue();
        setCommandConnected(state);
        if(state){            
        }
      }
  }
/*=============================================================================================
/     Property Change Listener for the _propertyChange
/=============================================================================================*/
  private void myBlockingSocket_propertyChange(PropertyChangeEvent e)  {
     java.lang.String propertyName = e.getPropertyName();
       if(propertyName == "connected"){
        boolean state = (java.lang.Boolean)e.getNewValue();
        setBlockingConnected(state);
        if(state){          
        }
      }
  }  
  /*=============================================================================================
/     start_AsynchronousMonitor()
/=============================================================================================*/
public void start_AsynchronousMonitor(){
   AsynchronousMonitorThread myAsynchronousMonitorThread = new AsynchronousMonitorThread();
   myAsynchronousMonitorThread.start();
}
/*================================================================================================
/       setCommandConnected(boolean new_command_connected)
/=================================================================================================*/
  public void setCommandConnected(boolean new_command_connected) {
    boolean  old_command_connected = this.command_connected;
    this.command_connected = new_command_connected;
    propertyChangeListeners.firePropertyChange("command_connected", Boolean.valueOf(old_command_connected), Boolean.valueOf(new_command_connected));
  }
  public boolean isCommandConnected() {
    return command_connected;
  }
/*================================================================================================
/       setBlockingConnected(boolean new_blocking_connected)
/=================================================================================================*/
  public void setBlockingConnected(boolean new_blocking_connected) {
    boolean  old_blocking_connected = this.blocking_connected;
    this.blocking_connected = new_blocking_connected;
    propertyChangeListeners.firePropertyChange("blocking_connected", Boolean.valueOf(old_blocking_connected), Boolean.valueOf(new_blocking_connected));
  }
  public boolean isBlockingConnected() {
    return blocking_connected;
  }
/*================================================================================================
/       setAsyncConnected(boolean new_async_connected)
/=================================================================================================*/
  public void setAsyncConnected(boolean new_async_connected) {
    boolean  old_async_connected = this.async_connected;
    this.async_connected = new_async_connected;
    propertyChangeListeners.firePropertyChange("async_connected", Boolean.valueOf(old_async_connected), Boolean.valueOf(new_async_connected));
  }
  public boolean isAsyncConnected() {
    return async_connected;
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
  public int getProgress() {
    return progress;
  }  
/*================================================================================================
/     setProgress(int new_progress)
/=================================================================================================*/
  public void setProgressString(java.lang.String new_progress_string) {
    java.lang.String  old_progress_string = this.progress_string;
    this.progress_string = new_progress_string;
    propertyChangeListeners.firePropertyChange("progress_string", old_progress_string, new_progress_string);
  }
  public java.lang.String getProgressString() {
    return progress_string;
  } 
/*================================================================================================
/     setProgress(int new_progress)
/=================================================================================================*/
  public void setOverheadProgressString(java.lang.String new_overhead_progress_string) {
    java.lang.String  old_overhead_progress_string = this.overhead_progress_string;
    this.overhead_progress_string = new_overhead_progress_string;
    propertyChangeListeners.firePropertyChange("overhead_progress_string", old_overhead_progress_string, new_overhead_progress_string);
  }
  public java.lang.String getOverheadProgressString() {
    return overhead_progress_string;
  }
  /*================================================================================================
/     setOverheadProgress(int new_overhead_progress)
/=================================================================================================*/
  public void setOverheadProgress(int new_overhead_progress) {
    int  old_overhead_progress = this.overhead_progress;
    this.overhead_progress = new_overhead_progress;
    propertyChangeListeners.firePropertyChange("overhead_progress", Integer.valueOf(old_overhead_progress),Integer.valueOf(new_overhead_progress));
  }
  public int getOverheadProgress() {
    return overhead_progress;
  }  
/*================================================================================================
/       setAsyncConnected(boolean new_async_connected)
/=================================================================================================*/
  public void setTotalEXPTime(int new_total_time) {
    int  old_total_time = this.total_time;
    this.total_time = new_total_time;
    propertyChangeListeners.firePropertyChange("total_time", Integer.valueOf(old_total_time), Integer.valueOf(new_total_time));
  }
  public int getTotalEXPTime() {
    return total_time;
  }  
/*================================================================================================
/       setAsyncConnected(boolean new_async_connected)
/=================================================================================================*/
  public void setElapsedTime_1(int new_elapsed_time_1) {
    int  old_elapsed_time_1 = this.elapsed_time_1;
    this.elapsed_time_1 = new_elapsed_time_1;
    propertyChangeListeners.firePropertyChange("elapsed_time_1", Integer.valueOf(old_elapsed_time_1), Integer.valueOf(new_elapsed_time_1));
  }
  public int getElapedTime_1() {
    return elapsed_time_1;
  } 
/*================================================================================================
/       setAsyncConnected(boolean new_async_connected)
/=================================================================================================*/
  public void setElapsedTime_2(int new_elapsed_time_2) {
    int  old_elapsed_time_2 = this.elapsed_time_2;
    this.elapsed_time_2 = new_elapsed_time_2;
    propertyChangeListeners.firePropertyChange("elapsed_time_2", Integer.valueOf(old_elapsed_time_2), Integer.valueOf(new_elapsed_time_2));
  }
  public int getElapedTime_2() {
    return elapsed_time_2;
  }  
/*================================================================================================
/       setAsyncConnected(boolean new_async_connected)
/=================================================================================================*/
  public void setElapsedTime_3(int new_elapsed_time_3) {
    int  old_elapsed_time_3 = this.elapsed_time_3;
    this.elapsed_time_3 = new_elapsed_time_3;
    propertyChangeListeners.firePropertyChange("elapsed_time_3", Integer.valueOf(old_elapsed_time_3), Integer.valueOf(new_elapsed_time_3));
  }
  public int getElapedTime_3() {
    return elapsed_time_3;
  }  
/*================================================================================================
/       setAsyncConnected(boolean new_async_connected)
/=================================================================================================*/
  public void setElapsedTime_4(int new_elapsed_time_4) {
    int  old_elapsed_time_4 = this.elapsed_time_4;
    this.elapsed_time_4 = new_elapsed_time_4;
    propertyChangeListeners.firePropertyChange("elapsed_time_4", Integer.valueOf(old_elapsed_time_4), Integer.valueOf(new_elapsed_time_4));
  }
  public int getElapedTime_4() {
    return elapsed_time_4;
  }
 /*================================================================================================
/      setPixelCount_1(int new_pixel_count_1)
/=================================================================================================*/
  public void setImageSize(int new_image_size) {
    int  old_image_size = this.image_size;
    this.image_size = new_image_size;
    propertyChangeListeners.firePropertyChange("image_size", Integer.valueOf(old_image_size),Integer.valueOf(new_image_size));
  }
  public int getImageSize() {
    return image_size;
  }  
/*================================================================================================
/      setPixelCount_1(int new_pixel_count_1)
/=================================================================================================*/
  public void setPixelCount_1(int new_pixel_count_1) {
    int  old_pixel_count_1 = this.pixel_count_1;
    this.pixel_count_1 = new_pixel_count_1;
    propertyChangeListeners.firePropertyChange("pixel_count_1", Integer.valueOf(old_pixel_count_1),Integer.valueOf(new_pixel_count_1));
  }
  public int getPixelCount_1() {
    return pixel_count_1;
  } 
/*================================================================================================
/      setPixelCount_1(int new_pixel_count_1)
/=================================================================================================*/
  public void setPixelCount_2(int new_pixel_count_2) {
    int  old_pixel_count_2 = this.pixel_count_2;
    this.pixel_count_2 = new_pixel_count_2;
    propertyChangeListeners.firePropertyChange("pixel_count_2", Integer.valueOf(old_pixel_count_2), Integer.valueOf(new_pixel_count_2));
  }
  public int getPixelCount_2() {
    return pixel_count_2;
  } 
/*================================================================================================
/      setPixelCount_1(int new_pixel_count_1)
/=================================================================================================*/
  public void setPixelCount_3(int new_pixel_count_3) {
    int  old_pixel_count_3 = this.pixel_count_3;
    this.pixel_count_3 = new_pixel_count_3;
    propertyChangeListeners.firePropertyChange("pixel_count_3", Integer.valueOf(old_pixel_count_3), Integer.valueOf(new_pixel_count_3));
  }
  public int getPixelCount_3() {
    return pixel_count_3;
  } 
/*================================================================================================
/      setPixelCount_1(int new_pixel_count_1)
/=================================================================================================*/
  public void setPixelCount_4(int new_pixel_count_4) {
    int  old_pixel_count_4 = this.pixel_count_4;
    this.pixel_count_4 = new_pixel_count_4;
    propertyChangeListeners.firePropertyChange("pixel_count_4", Integer.valueOf(old_pixel_count_4), Integer.valueOf(new_pixel_count_4));
  }
  public int getPixelCount_4() {
    return pixel_count_4;
  }   
/*================================================================================================
/      executeCommand(int socket,java.lang.String command,double delay)
/=================================================================================================*/
  public java.lang.String executeCommand(int socket,java.lang.String command,double delay){
      java.lang.String response = new java.lang.String();
        logMessage(INFO,command);
       if(socket == COMMAND){
//         myCommandSocket.startConnection(ClientSocket.USE_HOSTNAME);
           if(myCommandSocket.isConnected()){
              myCommandLogModel.insertMessage(CommandLogModel.COMMAND,command);
              response = myCommandSocket.sendReceiveCommandARCHON(command+TERMINATOR);
              myCommandLogModel.insertMessage(CommandLogModel.RESPONSE,response.trim());               
           }
//        myCommandSocket.closeConnection();
       }
       if(socket == BLOCKING){
        myCommandLogModel.insertMessage(CommandLogModel.COMMAND,command);
        response = myBlockingSocket.sendReceiveCommand(command+TERMINATOR,delay);
        myCommandLogModel.insertMessage(CommandLogModel.RESPONSE,response.trim());
       }
       logMessage(INFO,response);
    return response;
  } 
/*================================================================================================
/      executeCommand(int socket,java.lang.String command,double delay)
/=================================================================================================*/
  public java.lang.String executeCommand(java.lang.String command){
      java.lang.String response = new java.lang.String();
        logMessage(INFO,command);
           if(myCommandSocket.isConnected()){
              myCommandLogModel.insertMessage(CommandLogModel.COMMAND,command);
              response = myCommandSocket.sendReceiveCommandARCHON(command+TERMINATOR);
              myCommandLogModel.insertMessage(CommandLogModel.RESPONSE,response.trim());               
           }
       logMessage(INFO,response);
    return response;
  }   
 /*================================================================================================
/      executeCommand(int socket,java.lang.String command,double delay)
/=================================================================================================*/
  public java.lang.String executeCommandB(java.lang.String command){
      java.lang.String response = new java.lang.String();
        logMessage(INFO,command);
           if(myBlockingSocket.isConnected()){
              myCommandLogModel.insertMessage(CommandLogModel.COMMAND,command);
              response = myBlockingSocket.sendReceiveCommandARCHON(command+TERMINATOR);
              myCommandLogModel.insertMessage(CommandLogModel.RESPONSE,response.trim());               
           }
       logMessage(INFO,response);
    return response;
  }  
/*================================================================================================
/      executeCommand(int socket,java.lang.String command,double delay)
/=================================================================================================*/
  public java.lang.String executeCommand_local(java.lang.String command){
      java.lang.String response = new java.lang.String();      
       logMessage(INFO,command);
           OutputStream       outputStream;
           OutputStreamWriter outputStreamWriter;
           InputStream        inputStream;
           InputStreamReader  inputStreamReader;
           String host    = myIniReader.SERVERNAME;
           int port       = myIniReader.COMMAND_SERVERPORT;
           char term      = '\n';

            command.trim();    // strip the last space
            command += term;   // add the terminating character
            try {
              // connect to the host socket
              //
              Socket socket = new Socket( host, port );

              outputStream       = socket.getOutputStream();
              outputStreamWriter = new OutputStreamWriter( outputStream );

              outputStreamWriter.write(command);  // send command
              outputStreamWriter.flush();

              inputStream         = socket.getInputStream();
              inputStreamReader   = new InputStreamReader( inputStream );
              StringBuilder reply = new StringBuilder();

              // read reply until terminating character received from host
              //
              int chin;
              while ( ( chin = inputStreamReader.read() ) != term ) {
                reply.append( (char)chin );
              }
              response = reply.toString();

              System.out.println( reply );   // print the reply
              socket.close();                // close the connection
            }
            catch ( Exception exception ) {
              System.out.println( exception );
            } 
       logMessage(INFO,response);
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
    java.lang.String command = new java.lang.String();
    command = "abort";
    java.lang.String response = executeCommand(command);
    if(response.matches("DONE")){
       setERROR(false); 
    }else if(response.matches("ERROR")){
       setERROR(true);         
    }
    return isERROR();
} 
/*================================================================================================
/      abort()
/=================================================================================================*/
public boolean airmass(double airmass_limit){
    java.lang.String command = new java.lang.String();
    command = "airmass "+Double.toString(airmass_limit);
//    java.lang.String response = executeCommand(COMMAND,command,DEfAULT_DELAY);
    java.lang.String response = executeCommand(command);
    if(response.matches("DONE")){
       setERROR(false); 
    }else if(response.matches("ERROR")){
       setERROR(true);         
    }
    return isERROR();
}
/*================================================================================================
/      abort()
/=================================================================================================*/
public boolean do_one_all(int code){
    java.lang.String command = new java.lang.String();
    if(code == DO_ONE){
        command = "do one";
    }
    if(code == DO_ALL){
        command = "do all";
    }
//    java.lang.String response = executeCommand(COMMAND,command,DEfAULT_DELAY);
    java.lang.String response = executeCommand(command);
    if(response.matches("DONE")){
       setERROR(false); 
    }else if(response.matches("ERROR")){
       setERROR(true);         
    }
    return isERROR();
}
/*================================================================================================
/      abort()
/=================================================================================================*/
public boolean modexptime(double mod_exptime){
    java.lang.String command = new java.lang.String();
    command = "modexptime "+Double.toString(mod_exptime);
//    java.lang.String response = executeCommand(COMMAND,command,DEfAULT_DELAY);
    java.lang.String response = executeCommand(command);
    if(response.matches("DONE")){
       setERROR(false); 
    }else if(response.matches("ERROR")){
       setERROR(true);         
    }
    return isERROR();
}
/*================================================================================================
/      pause()
/=================================================================================================*/
public boolean pause(){
    java.lang.String command = new java.lang.String();
    command = "pause";
//    java.lang.String response = executeCommand(COMMAND,command,DEfAULT_DELAY);
     java.lang.String response = executeCommand(command);
   if(response.matches("DONE")){
       setERROR(false); 
    }else if(response.matches("ERROR")){
       setERROR(true);         
    }
    return isERROR();
} 
/*================================================================================================
/     resume()
/=================================================================================================*/
public boolean resume(){
    java.lang.String command = new java.lang.String();
    command = "resume";
//    java.lang.String response = executeCommand(COMMAND,command,DEfAULT_DELAY);
    java.lang.String response = executeCommand(command);
    if(response.matches("DONE")){
       setERROR(false); 
    }else if(response.matches("ERROR")){
       setERROR(true);         
    }
    return isERROR();
} 
/*================================================================================================
/     shutdown()
/=================================================================================================*/
public boolean shutdown(){
    java.lang.String command = new java.lang.String();
    command = "shutdown";
//    java.lang.String response = executeCommand(COMMAND,command,DEfAULT_DELAY);
    java.lang.String response = executeCommand(command);
    if(response.matches("DONE")){
       setERROR(false); 
    }else if(response.matches("ERROR")){
       setERROR(true);         
    }
    return isERROR();
} 
/*================================================================================================
/     start()
/=================================================================================================*/
public boolean start(){
    java.lang.String command = new java.lang.String();
    command = "start";
//    java.lang.String response = executeCommand(COMMAND,command,DEfAULT_DELAY);
    java.lang.String response = executeCommand(command);
    if(response.matches("DONE")){
       setERROR(false); 
    }else if(response.matches("ERROR")){
       setERROR(true);         
    }
    return isERROR();
} 
/*================================================================================================
/     startup()
/=================================================================================================*/
public boolean startup(){
    java.lang.String command = new java.lang.String();
    command = "startup";
 //   java.lang.String response = executeCommand(COMMAND,command,DEfAULT_DELAY);
    java.lang.String response = executeCommand(command);
    if(response.matches("DONE")){
       setERROR(false); 
    }else if(response.matches("ERROR")){
       setERROR(true);         
    }
    return isERROR();
} 
/*================================================================================================
/     stop()
/=================================================================================================*/
public boolean stop(){
    java.lang.String command = new java.lang.String();
    command = "stop";
//    java.lang.String response = executeCommand(COMMAND,command,DEfAULT_DELAY);
   java.lang.String response = executeCommand(command);
   if(response.matches("DONE")){
       setERROR(false); 
    }else if(response.matches("ERROR")){
       setERROR(true);         
    }
    return isERROR();
} 
/*================================================================================================
/    state()
/=================================================================================================*/
public boolean state(){
    java.lang.String command = new java.lang.String();
    command = "state";
    java.lang.String response = executeCommandB(command); 
//   java.lang.String response = executeCommand_local(command);
   System.out.println(response);
   try{
   java.util.ArrayList runstate_arraylist = new java.util.ArrayList();
    //    RUNSTATE: OFFLINE | REQSTATE: OFFLINE | THREADS: async_listener
        java.lang.String runstate = response;
        runstate = runstate.replace("RUNSTATE:", "");
        StringTokenizer st = new StringTokenizer(runstate," ");
        int count = st.countTokens();
        if(st.nextToken() != null ){
            java.lang.String current_runstate = (java.lang.String )st.nextToken();
            count = st.countTokens();
            for(int i=1;i<count;i++){
                java.lang.String current_secondary_state = st.nextToken();
                runstate_arraylist.add(current_secondary_state);
            }       
            runstate = runstate.trim();
            System.out.println("RUNSTATE = "+runstate);        
            evaluateState(current_runstate,runstate_arraylist);
        }
   }catch(Exception e){
       System.out.println(e.toString());
   }   
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
    java.lang.String command = new java.lang.String();
    command = "tcs llist";
   java.lang.String response = executeCommandB(command);
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
  public java.lang.String getActiveTCSAddress() {
    return active_tcs_address;
  } 
/*================================================================================================
/     setSimulatorAddress(java.lang.String new_simulator_address)
/=================================================================================================*/
  public void setActiveTCSname(java.lang.String new_active_tcs_name) {
    java.lang.String  old_active_tcs_name = this.active_tcs_name;
    this.active_tcs_name = new_active_tcs_name;
    propertyChangeListeners.firePropertyChange("active_tcs_name", old_active_tcs_name, new_active_tcs_name);
  }
  public java.lang.String getActiveTCSname() {
    return active_tcs_name;
  }   
/*================================================================================================
/       setCommandConnected(boolean new_command_connected)
/=================================================================================================*/
  public void setTCSConnected(boolean new_tcs_connected) {
    boolean  old_tcs_connected = this.tcs_connected;
    this.tcs_connected = new_tcs_connected;
    propertyChangeListeners.firePropertyChange("tcs_connected", Boolean.valueOf(old_tcs_connected), Boolean.valueOf(new_tcs_connected));
  }
  public boolean isTCSConnected() {
    return tcs_connected;
  }
  /*================================================================================================
/       setCommandConnected(boolean new_command_connected)
/=================================================================================================*/
  public void setTCSConnectedInProgress(boolean new_tcs_connected_in_progress) {
    boolean  old_tcs_connected_in_progress = this.tcs_connected_in_progress;
    this.tcs_connected_in_progress = new_tcs_connected_in_progress;
    propertyChangeListeners.firePropertyChange("tcs_connected_in_progress", Boolean.valueOf(old_tcs_connected_in_progress), Boolean.valueOf(new_tcs_connected_in_progress));
  }
  public boolean isTCSConnected_in_progress() {
    return tcs_connected_in_progress;
  }
/*================================================================================================
/   tcs_connect(
/=================================================================================================*/
public boolean tcs_connect(){
    java.lang.String command = new java.lang.String();
    command = "tcs connect";
//    java.lang.String response = executeCommand(COMMAND,command,DEfAULT_DELAY);
   java.lang.String response = executeCommandB(command);
   if(response.matches("CDONE")){
       setERROR(false); 
    }else if(response.matches("ERROR")){
       setERROR(true);         
    }
    return isERROR();
} 
/*================================================================================================
/  tcs_open_sim()
/=================================================================================================*/
public boolean tcs_open_sim(){
    java.lang.String command = new java.lang.String();
    command = "tcs open sim";
   java.lang.String response = executeCommandB(command);
   if(response.matches("DONE")){
       setERROR(false); 
    }else if(response.matches("ERROR")){
       setERROR(true);         
    }
    return isERROR();
} 
/*================================================================================================
/  tcs_open_sim()
/=================================================================================================*/
public boolean tcs_open_real(){
    java.lang.String command = new java.lang.String();
    command = "tcs open real";
   java.lang.String response = executeCommandB(command);
   if(response.matches("DONE")){
       setERROR(false); 
    }else if(response.matches("ERROR")){
       setERROR(true);         
    }
    return isERROR();
} 
/*================================================================================================
/   tcs_close()
/=================================================================================================*/
public boolean tcs_close(){
    java.lang.String command = new java.lang.String();
    command = "tcs close";
   java.lang.String response = executeCommandB(command);
   if(response.matches("DONE")){
       setERROR(false); 
    }else if(response.matches("ERROR")){
       setERROR(true);         
    }
    return isERROR();
} 
/*================================================================================================
/   tcs_close()
/=================================================================================================*/
public boolean tcs_isOpen(){
   boolean isopen = false;
   java.lang.String command = new java.lang.String();
   command = "tcs isopen";
   java.lang.String response = executeCommandB(command);   
   response = response.replace(" DONE","");
   response = response.trim();
   if(response.matches("false")){
      isopen = false; 
      setActiveTCSAddress("NOT CONNECTED");
      setActiveTCSname(response);
   }else if(response.matches("sim")){
      isopen = true; 
      setActiveTCSAddress(getSimulatorAddress());
      setActiveTCSname(response);
   }else if(response.matches("real")){
      isopen = true;
      setActiveTCSAddress(getTCSAddress());
      setActiveTCSname(response);
   }   
   this.setTCSConnected(isopen);
   return isopen;
} 
/*================================================================================================
/   tcs_close()
/=================================================================================================*/
public boolean tcs_isConnected(){
   boolean isconnected = false;
   java.lang.String command = new java.lang.String();
   command = "tcs isconnected";
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
/   connect_real_tcs()
/=================================================================================================*/
public void connect_real_tcs2(){
    java.lang.String command = new java.lang.String();
    command = "tcsinit real";
   java.lang.String response = executeCommandB(command);
   if(response.matches("DONE")){
       setERROR(false); 
    }else if(response.matches("ERROR")){
       setERROR(true);         
    }
//    tcs_isOpen();
}
/*================================================================================================
/   connect_real_tcs()
/=================================================================================================*/
public void connect_sim_tcs2(){
    java.lang.String command = new java.lang.String();
    command = "tcsinit sim";
   java.lang.String response = executeCommandB(command);
   if(response.matches("DONE")){
       setERROR(false); 
    }else if(response.matches("ERROR")){
       setERROR(true);         
    }
//    tcs_isOpen();
}
/*================================================================================================
/   connect_real_tcs()
/=================================================================================================*/
public void connect_real_tcs(){
     boolean connected = tcs_isConnected();
     if(!connected){
        tcs_connect(); 
     }     
     tcs_close();
     tcs_open_real();
     tcs_isOpen();
}
/*================================================================================================
/   connect_real_tcs()
/=================================================================================================*/
public void connect_sim_tcs(){
     boolean connected = tcs_isConnected();
     if(!connected){
        tcs_connect(); 
     }  
     tcs_close();
     tcs_open_sim();
     tcs_isOpen();
}
/*================================================================================================
/    targetset(int id)
/=================================================================================================*/
public synchronized void evaluateState(java.lang.String current_runstate,java.util.ArrayList current_modifiers){
    System.out.println("RUNSTATE = "+current_runstate+" "+" MODIFIER COUNT = "+current_modifiers.size());
    if(current_runstate.matches("OFFLINE")){
       setSTATE("OFFLINE");
    }
    if(current_runstate.matches("STARTING")){
       setSTATE("STARTING");
    }
     if(current_runstate.matches("SHUTTING")){
       setSTATE("SHUTTING");   // THIS STATE IS NOT ACTUALLY CAPTURED BY THE LISTENER
    }    
    if(current_runstate.matches("READY")){
       setSTATE("READY");
       setProgress(0);
       setProgressString("");
       setOverheadProgress(0);
       setOverheadProgressString("");
//       if(isUpdateArmed()){
//           disarmUpdate();
//           dbms.executeQueryState(dbms.selectedObservationSet,dbms.myTargetDBMSTableModel);           
//       }
    }  
    if(current_runstate.matches("RUNNING")){
        int count = current_modifiers.size();     
        System.out.println("Number of Modifiers = "+count);
          for(int i=0;i<count;i++){
            java.lang.String current_modifier = (java.lang.String)current_modifiers.get(i);
            System.out.println(current_modifier);
            if(current_modifier.matches("TCSOP")){
                setSTATE("TCSOP");
            }else if(current_modifier.matches("SLEW")){
                setSTATE("SLEW");
            }else if(current_modifier.matches("ACQUIRE")){
                setSTATE("ACQUIRE");
            }else if(current_modifier.matches("EXPOSING")){
                setSTATE("EXPOSING");
           }else if(current_modifier.matches("READOUT")){ 
                setSTATE("READOUT");
            }else {
                setSTATE("RUNNING");
            }
          }
    } 
     if(current_runstate.matches("PAUSED")){
       setSTATE("PAUSED");
    } 
    if(current_runstate.matches("STOPREQ")){
       setSTATE("STOPREQ");
    }  
    if(current_runstate.matches("ABORTREQ")){
       setSTATE("ABORTREQ");
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
     if(message.contains("ERROR")){
//        message = message.replace("ERROR", "");
        if(message.contains("ERROR")){
            myCommandLogModel.insertMessage(CommandLogModel.ERROR, message);
        }else{
            myCommandLogModel.insertMessage(CommandLogModel.COMMAND, message);
        }       
     }
     if(message.contains("TCSD")){
        String[] messages = message.split(":"); 
           boolean isopen = false;
        if(messages[1].matches("isopen")){
           if(messages[2].matches("sim")){
              isopen = true;
              setActiveTCSAddress(getSimulatorAddress());
              setActiveTCSname(messages[2]);
           }else if(messages[2].matches("real")){
              isopen = true;
              setActiveTCSAddress(getTCSAddress());
              setActiveTCSname(messages[2]);
           }
        setTCSConnectedInProgress(false);
        setTCSConnected(isopen);
        }else if(messages[1].matches("close")){
            if(messages[2].matches("DONE")){                
                setTCSConnected(false);
                setActiveTCSAddress("NOT CONNECTED");
                setActiveTCSname("NOT CONNECTED");
            }            
        }
     }
     if(message.contains("TARGETSTATE")){
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
     if(message.contains("RUNSTATE")){
        java.lang.String runstate = message;
        runstate = runstate.replace("RUNSTATE:", "");
        StringTokenizer st = new StringTokenizer(runstate," ");
        java.lang.String current_runstate = (java.lang.String )st.nextToken();
        int count = st.countTokens();
        for(int i=0;i<count;i++){
            java.lang.String current_secondary_state = st.nextToken();
            runstate_arraylist.add(current_secondary_state);
        }       
        runstate = runstate.trim();
        System.out.println("RUNSTATE = "+runstate);        
        evaluateState(current_runstate,runstate_arraylist);
     }
     if(message.contains("ELAPSEDTIME")){
         if(message.contains("_0")){
             message = message.replace("ELAPSEDTIME_0:","");
             message = message.trim();
             String[] messages = message.split(" ");
             int elapse_time_milliseconds = Integer.parseInt(messages[0]);
             setElapsedTime_1(elapse_time_milliseconds);
             messages[1] = messages[1].replace("EXPTIME:", "");
             int total_time_milliseconds = Integer.parseInt(messages[1]);
             setTotalEXPTime(total_time_milliseconds);  
             double current_progress = ((double)elapse_time_milliseconds/(double)total_time_milliseconds)*100.0;
             setProgress((int)current_progress);
             double time_in_seconds = (double)elapse_time_milliseconds/1000.0;
             setProgressString("ELAPSED TIME = "+time_in_seconds);
             System.out.println("PROGRESS = "+current_progress);
         }
         if(message.contains("_1")){
             message = message.replace("ELAPSEDTIME_1:","");
             message = message.trim();
             String[] messages = message.split(" ");
             int elapse_time_milliseconds = Integer.parseInt(messages[0]);
             setElapsedTime_2(elapse_time_milliseconds);
             messages[1] = messages[1].replace("EXPTIME:", "");
             int total_time_milliseconds = Integer.parseInt(messages[1]);
             setTotalEXPTime(total_time_milliseconds);  
             double current_progress = ((double)elapse_time_milliseconds/(double)total_time_milliseconds)*100.0;
             setProgress((int)current_progress);
             double time_in_seconds = (double)elapse_time_milliseconds/1000.0;
             setProgressString("ELAPSED TIME = "+time_in_seconds);
             System.out.println("PROGRESS = "+current_progress);
         }
         if(message.contains("_2")){
             message = message.replace("ELAPSEDTIME_2:","");
             message = message.trim();
             String[] messages = message.split(" ");
             int elapse_time_milliseconds = Integer.parseInt(messages[0]);
             setElapsedTime_3(elapse_time_milliseconds);
             messages[1] = messages[1].replace("EXPTIME:", "");
             int total_time_milliseconds = Integer.parseInt(messages[1]);
             setTotalEXPTime(total_time_milliseconds);  
             double current_progress = ((double)elapse_time_milliseconds/(double)total_time_milliseconds)*100.0;
             setProgress((int)current_progress);
             double time_in_seconds = (double)elapse_time_milliseconds/1000.0;
             setProgressString("ELAPSED TIME = "+time_in_seconds);
             System.out.println("PROGRESS = "+current_progress);
         }
         if(message.contains("_3")){
             message = message.replace("ELAPSEDTIME_3:","");
             message = message.trim();
             String[] messages = message.split(" ");
             int elapse_time_milliseconds = Integer.parseInt(messages[0]);
             setElapsedTime_4(elapse_time_milliseconds);
             messages[1] = messages[1].replace("EXPTIME:", "");
             int total_time_milliseconds = Integer.parseInt(messages[1]);
             setTotalEXPTime(total_time_milliseconds);  
             double current_progress = ((double)elapse_time_milliseconds/(double)total_time_milliseconds)*100.0;
             setProgress((int)current_progress);
             double time_in_seconds = (double)elapse_time_milliseconds/1000.0;
             setProgressString("ELAPSED TIME = "+time_in_seconds);
             System.out.println("PROGRESS = "+current_progress);
         }
     }
     if(message.contains("PIXELCOUNT")){
         if(message.contains("_0")){
             //  EXAMPLE     PIXELCOUNT_3:1048576 IMAGESIZE: 1048576
             String[] messages = message.split(" ");            
             java.lang.String pixcount = messages[0].replace("PIXELCOUNT_0:","");
             java.lang.String totalpix = messages[2];
             pixcount = pixcount.trim();
             totalpix = totalpix.trim();
             try{
             int pixel_count = Integer.parseInt(pixcount);
             int total_pixels = Integer.parseInt(totalpix);
             setPixelCount_1(pixel_count);
             setImageSize(total_pixels);
             if(pixel_count != 0){
               double current_progress = ((double)pixel_count/(double)total_pixels)*100.0;
               setOverheadProgress((int)current_progress);
               setOverheadProgressString("READOUT % ="+df.format(current_progress));
             }
             }catch(Exception e){
                System.out.println(e.toString());
             }
         }
         if(message.contains("_1")){
             String[] messages = message.split(" ");            
             java.lang.String pixcount = messages[0].replace("PIXELCOUNT_1:","");
             java.lang.String totalpix = messages[2];
             pixcount = pixcount.trim();
             totalpix = totalpix.trim();
             try{
             int pixel_count = Integer.parseInt(pixcount);
             int total_pixels = Integer.parseInt(totalpix);
             setPixelCount_2(pixel_count);
             setImageSize(total_pixels);
             if(pixel_count != 0){
               double current_progress = ((double)pixel_count/(double)total_pixels)*100.0;
               setOverheadProgress((int)current_progress);  
               setOverheadProgressString("READOUT % ="+df.format(current_progress));
             }
             }catch(Exception e){
                System.out.println(e.toString());
             }             
         }
         if(message.contains("_2")){
             String[] messages = message.split(" ");            
             java.lang.String pixcount = messages[0].replace("PIXELCOUNT_2:","");
             java.lang.String totalpix = messages[2];
             pixcount = pixcount.trim();
             totalpix = totalpix.trim();
             try{
             int pixel_count = Integer.parseInt(pixcount);
             int total_pixels = Integer.parseInt(totalpix);
             setPixelCount_3(pixel_count);
             setImageSize(total_pixels);
             if(pixel_count != 0){
               double current_progress = ((double)pixel_count/(double)total_pixels)*100.0;
               setOverheadProgress((int)current_progress);   
               setOverheadProgressString("READOUT % ="+df.format(current_progress));
             }
             }catch(Exception e){
                System.out.println(e.toString());
             }
         }
         if(message.contains("_3")){
             String[] messages = message.split(" ");            
             java.lang.String pixcount = messages[0].replace("PIXELCOUNT_3:","");
             java.lang.String totalpix = messages[2];
             pixcount = pixcount.trim();
             totalpix = totalpix.trim();
             try{
             int pixel_count = Integer.parseInt(pixcount);
             int total_pixels = Integer.parseInt(totalpix);
             setPixelCount_4(pixel_count);
             setImageSize(total_pixels);
             if(pixel_count != 0){
                  double current_progress = ((double)pixel_count/(double)total_pixels)*100.0;
                  setOverheadProgress((int)current_progress);
                  setOverheadProgressString("READOUT % ="+df.format(current_progress));
             }
             }catch(Exception e){
                System.out.println(e.toString());
             }         
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
    java.lang.String command = new java.lang.String();
    command = "targetset "+Integer.toString(id);
//    java.lang.String response = executeCommand(COMMAND,command,DEfAULT_DELAY);
   java.lang.String response = executeCommand(command);
   if(response.matches("DONE")){
       setERROR(false); 
    }else if(response.matches("ERROR")){
       setERROR(true);         
    }
    return isERROR();
} 
/*================================================================================================
/    targetset(java.lang.String target_set_name)
/=================================================================================================*/
public boolean targetset(java.lang.String target_set_name){
    java.lang.String command = new java.lang.String();
    command = "targetset "+target_set_name;
//    java.lang.String response = executeCommand(COMMAND,command,DEfAULT_DELAY);
     java.lang.String response = executeCommand(command);
   if(response.matches("DONE")){
       setERROR(false); 
    }else if(response.matches("ERROR")){
       setERROR(true);         
    }
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
/      setReadmode(java.lang.String new_Readmode)
/=================================================================================================*/
  public void setRUNSTATE(java.lang.String new_runstate) {
    java.lang.String  old_runstate = this.runstate;
    this.runstate = new_runstate;
   propertyChangeListeners.firePropertyChange("RUNSTATE", (old_runstate), (new_runstate));
  }
  public java.lang.String getRUNSTATE() {
    return runstate;
  }  
 /*================================================================================================
/      setReadmode(java.lang.String new_Readmode)
/=================================================================================================*/
  public void setREQSTATE(java.lang.String new_reqstate) {
    java.lang.String  old_reqstate = this.reqstate;
    this.reqstate = new_reqstate;
   propertyChangeListeners.firePropertyChange("REQSTATE", (old_reqstate), (new_reqstate));
  }
  public java.lang.String getREQSTATE() {
    return reqstate;
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
/*================================================================================================
/      waitForResponse(int newDelay)
/=================================================================================================*/
private void waitForResponseMilliseconds(int newDelay){
     try{
        Thread.currentThread().sleep(newDelay);} // sleep for awhile to let the messages pile up
      catch (Exception e){
    }
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
