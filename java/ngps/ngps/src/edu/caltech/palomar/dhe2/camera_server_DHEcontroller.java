/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package edu.caltech.palomar.dhe2;

//import edu.caltech.palomar.dhe.client.DHEControl;
//import edu.caltech.palomar.dhe.util.DHEStringUtilities;
import edu.caltech.palomar.io.ClientSocket;
import edu.caltech.palomar.util.general.CommandLogModel;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeSupport;
import java.util.Vector;
import org.apache.log4j.BasicConfigurator;
import org.apache.log4j.DailyRollingFileAppender;
import org.apache.log4j.Layout;
import org.apache.log4j.Logger;
import org.apache.log4j.PatternLayout;

/**
 *
 * @author jennifermilburn
 */
public class camera_server_DHEcontroller {
    transient public PropertyChangeSupport propertyChangeListeners = new PropertyChangeSupport(this);
    private transient Vector               actionListeners;
    public  java.lang.String          SEP                = System.getProperty("file.separator");
    private static java.lang.String   TERMINATOR         ="\n";
    public  java.lang.String          USERDIR            = System.getProperty("user.dir");
    static  Logger                    dheLogger          = Logger.getLogger(camera_server_DHEcontroller.class);
    public  DailyRollingFileAppender  fileAppender       = new DailyRollingFileAppender();
    private Layout                    layout             = new PatternLayout("%-5p [%t]: %m%n");
    public  IniReader                 myIniReader;
    public  DHEStringUtilities        stringUtil         = new DHEStringUtilities();
    public ClientSocket               myCommandSocket    = new ClientSocket(); 
    public ClientSocket               myBlockingSocket   = new ClientSocket();
//    public ClientSocket               myAsyncSocket      = new ClientSocket();
    private CommandLogModel           myCommandLogModel  = new CommandLogModel();
    private CommandLogModel           myAsyncLogModel    = new CommandLogModel();
    private  boolean                  command_connected;
    private  boolean                  blocking_connected;
    private  boolean                  async_connected;
    public       int                  initialized;
    public static int                 NOT_INITIALIZED     = 1;
    public static int                 INITIALIZING        = 2;
    public static int                 INITIALIZED         = 3;
    public static int                 COMMAND             = 0;
    public static int                 BLOCKING            = 1;
    public static int                 DEBUG               = 0;
    public static int                 INFO                = 1;
    public static int                 WARN                = 2;
    public static int                 ERROR               = 3;
    public static int                 FATAL               = 4;
    public double                     DEfAULT_DELAY       = 0.1;
    private boolean                   error;
    public static java.lang.String    lowerleft  = "lowerleft";
    public static java.lang.String    upperleft  = "upperleft";
    public static java.lang.String    lowerright = "lowerright";
    public static java.lang.String    upperright = "upperright";
    public static java.lang.String    lowerboth  = "lowerboth";
    public static java.lang.String    upperboth  = "upperboth";
    public static java.lang.String    quad       = "quad";
    public static java.lang.String    CLK        = "CLK";
    public static java.lang.String    VID        = "VID";
    public static java.lang.String    TIME       = "time";
    public static java.lang.String    NUMBER     = "number";           
/*=============================================================================================
/       camera_server_DHEcontroller() constructor
/=============================================================================================*/
   public camera_server_DHEcontroller(){
        initializeLogging();
        initializeSockets();
        setERROR(false);
    }
/*=============================================================================================
/         initializeLogging()
/=============================================================================================*/
 private void initializeLogging(){
     BasicConfigurator.configure();
     fileAppender.setFile(myIniReader.LOG_DIRECTORY + SEP + myIniReader.INSTRUMENT_NAME+SEP+"CAMERA_SERVER.log");
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
        setInitialized(NOT_INITIALIZED);
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
/      abort()
/=================================================================================================*/
public boolean abort(){
    java.lang.String command = new java.lang.String();
    command = "abort";
    java.lang.String response = executeCommand(COMMAND,command,DEfAULT_DELAY);
    if(response.matches("DONE")){
       setERROR(false); 
    }else if(response.matches("ERROR")){
       setERROR(true);         
    }
    return isERROR();
}
/*================================================================================================
/      amplifier(int new_pci,java.lang.String selected_amplifier)
/=================================================================================================*/
public boolean amplifier(int new_pci,java.lang.String selected_amplifier){
    java.lang.String command = new java.lang.String();
    command = "amplifier";
    command = command + Integer.toString(new_pci)+ " " + selected_amplifier;
    java.lang.String response = executeCommand(COMMAND,command,DEfAULT_DELAY);
    if(response.matches("DONE")){
       setERROR(false); 
    }else if(response.matches("ERROR")){
       setERROR(true);         
    }
    return isERROR();
}
/*================================================================================================
/      amplifier(int new_pci,java.lang.String selected_amplifier)
/=================================================================================================*/
public boolean bias(int new_pci,int new_boardid,int new_dac,java.lang.String new_boardtype,int new_adu){
    java.lang.String command = new java.lang.String();
    //Specify PCI device pci={0,1,2,...}, boardid {0:15}, DAC number {0:7}, the string "CLK" or "VID" for the type of bias board (clock or video),
    //and the voltage in A/D units, adu {0:4095} which you must scale to the maximum output voltage of the system.
    //E.G., for a 3.3V system, adu = 4095Ívoltage / 3.3. 
    command = "bias";
    command = command + Integer.toString(new_pci)+ " "+Integer.toString(new_boardid) + " "+Integer.toString(new_dac)+ " "+new_boardtype+" "+new_adu;
    java.lang.String response = executeCommand(COMMAND,command,DEfAULT_DELAY);
    if(response.matches("DONE")){
       setERROR(false); 
    }else if(response.matches("ERROR")){
       setERROR(true);         
    }
    return isERROR();
}
/*================================================================================================
/      buffer(int new_pci,int new_col,int new_row)
/=================================================================================================*/
public boolean buffer(int new_pci,int new_col,int new_row){
    java.lang.String command = new java.lang.String();
//This command will allocate PCI/e buffer space for performing the DMA transfers. If no argument is given then the size of the currently mapped buffer (in bytes) will be returned. 
//If a single value <size> is given, then a buffer of <size> bytes will be allocated and mapped to the PCI/e device. 
//If two values <cols> <rows> are given then a suitably sized buffer will be allocated to contain an image of those dimensions. 
//The PCI device is optional. If specified then the buffer for that device only will be set (or returned). If not specified then all PCI/e buffers will be set (or returned).  
    command = "buffer";
    command = command + Integer.toString(new_pci)+ " "+Integer.toString(new_col) + " "+Integer.toString(new_row);
    java.lang.String response = executeCommand(COMMAND,command,DEfAULT_DELAY);
    if(response.matches("DONE")){
       setERROR(false); 
    }else if(response.matches("ERROR")){
       setERROR(true);         
    }
    return isERROR();
}
/*================================================================================================
/      buffer(int new_pci,int new_col,int new_row)
/=================================================================================================*/
public boolean buffer(int new_pci,int new_size){
    java.lang.String command = new java.lang.String();
//This command will allocate PCI/e buffer space for performing the DMA transfers. If no argument is given then the size of the currently mapped buffer (in bytes) will be returned. 
//If a single value <size> is given, then a buffer of <size> bytes will be allocated and mapped to the PCI/e device. 
//If two values <cols> <rows> are given then a suitably sized buffer will be allocated to contain an image of those dimensions. 
//The PCI device is optional. If specified then the buffer for that device only will be set (or returned). If not specified then all PCI/e buffers will be set (or returned).  
    command = "buffer";
    command = command + Integer.toString(new_pci)+ " "+Integer.toString(new_size);
    java.lang.String response = executeCommand(COMMAND,command,DEfAULT_DELAY);
    if(response.matches("DONE")){
       setERROR(false); 
    }else if(response.matches("ERROR")){
       setERROR(true);         
    }
    return isERROR();
}
/*================================================================================================
/      buffer(int new_pci)
/=================================================================================================*/
public long buffer(int new_pci){
    java.lang.String command = new java.lang.String();
//This command will allocate PCI/e buffer space for performing the DMA transfers. If no argument is given then the size of the currently mapped buffer (in bytes) will be returned. 
//If a single value <size> is given, then a buffer of <size> bytes will be allocated and mapped to the PCI/e device. 
//If two values <cols> <rows> are given then a suitably sized buffer will be allocated to contain an image of those dimensions. 
//The PCI device is optional. If specified then the buffer for that device only will be set (or returned). If not specified then all PCI/e buffers will be set (or returned).  
    long buffer_size = 0;
    command = "buffer";
    command = command + Integer.toString(new_pci);
    java.lang.String response = executeCommand(COMMAND,command,DEfAULT_DELAY);
    try{
      buffer_size = Long.parseLong(response.trim());        
    }catch(Exception e){
       System.out.println(e.toString());
       setERROR(true);
    }
    return buffer_size;
}
/*================================================================================================
/      close()
/=================================================================================================*/
public boolean close(){
    java.lang.String command = new java.lang.String();
    command = "close";
    java.lang.String response = executeCommand(COMMAND,command,DEfAULT_DELAY);
    if(response.matches("DONE")){
       setERROR(false); 
    }else if(response.matches("ERROR")){
       setERROR(true);         
    }
    return isERROR();
}
/*================================================================================================
/      echo()
/=================================================================================================*/
public java.lang.String echo(){
    java.lang.String command = new java.lang.String();
    command = "echo";
    java.lang.String response = executeCommand(COMMAND,command,DEfAULT_DELAY);
    if(response.matches("DONE")){
       setERROR(false); 
    }else if(response.matches("ERROR")){
       setERROR(true);         
    }
    return response;
}
/*================================================================================================
/      exit()
/=================================================================================================*/
public java.lang.String exit(){
    java.lang.String command = new java.lang.String();
    command = "exit";
    java.lang.String response = executeCommand(COMMAND,command,DEfAULT_DELAY);
    myCommandSocket.closeConnection();
    myBlockingSocket.closeConnection();    
    return response;
}
/*================================================================================================
/      expose()
/=================================================================================================*/
public java.lang.String expose(){
    java.lang.String command = new java.lang.String();
    command = "expose";
    java.lang.String response = executeCommand(BLOCKING,command,DEfAULT_DELAY);
    if(response.matches("DONE")){
       setERROR(false); 
    }else if(response.matches("ERROR")){
       setERROR(true);         
    }
    return response;
}
/*================================================================================================
/     expose(int num_images)
/=================================================================================================*/
public boolean expose(int num_images){
    java.lang.String command = new java.lang.String();
    command = "expose";
    command = command + Integer.toString(num_images);
    java.lang.String response = executeCommand(BLOCKING,command,DEfAULT_DELAY);
    if(response.matches("DONE")){
       setERROR(false); 
    }else if(response.matches("ERROR")){
       setERROR(true);         
    }
    return isERROR();
}
/*================================================================================================
/     exptime(long exposure_time)
/=================================================================================================*/
public boolean exptime(long exposure_time){
    java.lang.String command = new java.lang.String();
    command = "exptime";
    command = command + " "+ Long.toString(exposure_time);
    java.lang.String response = executeCommand(BLOCKING,command,DEfAULT_DELAY);
    if(response.matches("DONE")){
       setERROR(false); 
    }else if(response.matches("ERROR")){
       setERROR(true);         
    }
    return isERROR();
}
/*================================================================================================
/     long get_exptime()
/=================================================================================================*/
public long get_exptime(){
    java.lang.String command = new java.lang.String();
    long exposure_time = 0;
    command = "exptime";
    java.lang.String response = executeCommand(BLOCKING,command,DEfAULT_DELAY);
    try{
        response = response.trim();
        exposure_time = Long.parseLong(response);
    }catch(Exception e){
        
    }
    return exposure_time;
}
/*================================================================================================
/      fitsnaming(java.lang.String format)
/=================================================================================================*/
public boolean fitsnaming(java.lang.String format){
//usage: fitsnaming [ type ] 
//returns: type DONE  on success 
//ERROR  on error 
//Set or get the FITS file naming type. When no parameter is specified, the current type is returned. Valid types are "time" and "number".  
//When <type> is number then the FITS files are saved as: 
//imdir/YYYYMMDD/basename_ID_imnum.fits 
//where imnum is an incremental number which starts at 0000 each time the server is started and increments automatically with each successful exposure.  
//When type is time then the FITS files are saved as 
//imdir/YYYYMMDD/basename_ID_YYYYMMDDHHMMSS.fits 
//where YYYYMMDDHHMMSS represents the time of the exposure (to the resolution of the current second). 
//See also the imdir (§7.3.135.3.134.3.13) and basename (§7.3.35.3.34.3.3) commands. 
    java.lang.String command = new java.lang.String();
    command = "fitsnaming";
    command = command + " "+format;
    java.lang.String response = executeCommand(BLOCKING,command,DEfAULT_DELAY);
    if(response.matches("DONE")){
       setERROR(false); 
    }else if(response.matches("ERROR")){
       setERROR(true);         
    } 
  return isERROR();
}
/*================================================================================================
/      fitsnaming(java.lang.String format)
/=================================================================================================*/
public java.lang.String get_fitsnaming(){
//usage: fitsnaming [ type ] 
//returns: type DONE  on success 
//ERROR  on error 
//Set or get the FITS file naming type. When no parameter is specified, the current type is returned. Valid types are "time" and "number".  
//When <type> is number then the FITS files are saved as: 
//imdir/YYYYMMDD/basename_ID_imnum.fits 
//where imnum is an incremental number which starts at 0000 each time the server is started and increments automatically with each successful exposure.  
//When type is time then the FITS files are saved as 
//imdir/YYYYMMDD/basename_ID_YYYYMMDDHHMMSS.fits 
//where YYYYMMDDHHMMSS represents the time of the exposure (to the resolution of the current second). 
//See also the imdir (§7.3.135.3.134.3.13) and basename (§7.3.35.3.34.3.3) commands. 
    java.lang.String command = new java.lang.String();
    command = "fitsnaming";
    java.lang.String response = executeCommand(BLOCKING,command,DEfAULT_DELAY);
    response = response.trim();
  return response;
}
/*================================================================================================
/      geometry(int rows, int cols)
/=================================================================================================*/
public boolean geometry(int rows, int cols){
//usage: geometry [ cols rows ] 
//returns: cols rows DONE  on success 
//ERROR  on error 
//Set or get the detector geometry. 
//When two arguments are specified then set the cols and rows (respectively) on the detector controller to those specified. When no arguments are given then return the cols and rows. 
//This command writes the cols to Y: memory address 0x400001 and the rows to Y: memory address 0x400002. 
    java.lang.String command = new java.lang.String();
    command = "geometry";
    java.lang.String response = executeCommand(BLOCKING,command,DEfAULT_DELAY);
    if(response.matches("DONE")){
       setERROR(false); 
    }else if(response.matches("ERROR")){
       setERROR(true);         
    } 
  return isERROR();   
}
/*================================================================================================
/      geometry(int rows, int cols)
/=================================================================================================*/
public boolean get_geometry(){
//usage: geometry [ cols rows ] 
//returns: cols rows DONE  on success 
//ERROR  on error 
//Set or get the detector geometry. 
//When two arguments are specified then set the cols and rows (respectively) on the detector controller to those specified. When no arguments are given then return the cols and rows. 
//This command writes the cols to Y: memory address 0x400001 and the rows to Y: memory address 0x400002. 
    java.lang.String command = new java.lang.String();
    command = "geometry";
    java.lang.String response = executeCommand(BLOCKING,command,DEfAULT_DELAY);
    try{
        java.util.StringTokenizer st = new java.util.StringTokenizer(response," ");
        response = response.trim();
        java.lang.String columns_string = st.nextToken();
        java.lang.String rows_string    = st.nextToken();
        int columns = Integer.parseInt(columns_string);
        int rows    = Integer.parseInt(rows_string);  
        setERROR(false); 
    }catch(Exception e){
        System.out.println(e.toString());
        setERROR(true); 
    }
  return isERROR();   
}
/*================================================================================================
/     exptime(long exposure_time)
/=================================================================================================*/
public boolean imdir(java.lang.String new_imdir){
    java.lang.String command = new java.lang.String();
    command = "imdir";
    command = command + " "+new_imdir;
    java.lang.String response = executeCommand(BLOCKING,command,DEfAULT_DELAY);
    if(response.matches("DONE")){
       setERROR(false); 
    }else if(response.matches("ERROR")){
       setERROR(true);         
    }
    return isERROR();
}
/*================================================================================================
/     exptime(long exposure_time)
/=================================================================================================*/
public java.lang.String get_imdir(java.lang.String new_imdir){
    java.lang.String command = new java.lang.String();
    command = "imdir";
    java.lang.String response = executeCommand(BLOCKING,command,DEfAULT_DELAY);
    response = response.trim();
    return response;
}
/*================================================================================================
/      executeCommand(int socket,java.lang.String command,double delay)
/=================================================================================================*/
  public java.lang.String executeCommand(int socket,java.lang.String command,double delay){
      java.lang.String response = new java.lang.String();
        logMessage(INFO,command);
       if(socket == COMMAND){
        myCommandLogModel.insertMessage(CommandLogModel.COMMAND,command);
        response = myCommandSocket.sendReceiveCommand(command+TERMINATOR,delay);
        myCommandLogModel.insertMessage(CommandLogModel.RESPONSE,response.trim());
       }
       if(socket == BLOCKING){
        myCommandLogModel.insertMessage(CommandLogModel.COMMAND,command);
        response = myBlockingSocket.sendReceiveCommand(command+TERMINATOR,delay);
        myCommandLogModel.insertMessage(CommandLogModel.RESPONSE,response.trim());
       }
       logMessage(INFO,response);
    return response;
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
/      setInitialized(int new_initialized)
/=================================================================================================*/
  public void setInitialized(int new_initialized) {
    int  old_initialized = this.initialized;
    this.initialized = new_initialized;
    propertyChangeListeners.firePropertyChange("initialized", Integer.valueOf(old_initialized), Integer.valueOf(new_initialized));
  }
  public int isInitialized() {
    return initialized;
  }
/*================================================================================================
/     setDHEIniReader(DHEIniReader newDHEIniReader)
/=================================================================================================*/
public void setIniReader(IniReader newIniReader){
    myIniReader = newIniReader;
}
public IniReader getDHEIniReader(){
    return myIniReader;
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
//     System.out.println(message);
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
/*=============================================================================================
/          SendCoordinatesThread()
/=============================================================================================*/
    private AsynchronousMonitorThread(){
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
/           run()
/===========================================================================crsw==================*/
    public void run(){
//      while(myAsyncSocket.isConnected()){
//          response = myAsyncSocket.getResponse();
//          if(response.length() != 0){
//             myAsyncLogModel.insertMessage(CommandLogModel.RESPONSE,response);
//          }
//          waitForResponseMilliseconds(POLLING_DELAY);
//     }
   }
 /*=============================================================================================
/         start()
/=============================================================================================*/
    public void start(){
     myThread = new Thread(this);
     myThread.start();
     }
    }// End of the SendCommandThread Inner Class

}
