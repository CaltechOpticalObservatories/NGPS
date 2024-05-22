/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package edu.caltech.palomar.telescopes.telemetry;

import java.io.ByteArrayOutputStream;
import javax.swing.UIManager;
import org.apache.commons.exec.CommandLine;
import org.apache.commons.exec.DefaultExecutor;
import org.apache.commons.exec.PumpStreamHandler;
import org.apache.commons.exec.Watchdog;
import org.apache.commons.exec.ExecuteWatchdog;
import org.apache.commons.exec.ShutdownHookProcessDestroyer;
import org.apache.log4j.BasicConfigurator;
import org.apache.log4j.DailyRollingFileAppender;
import org.apache.log4j.Layout;
import org.apache.log4j.Logger;
import org.apache.log4j.PatternLayout;

/**
 *
 * @author developer
 */
public class ProcessMonitor {
   public java.lang.String       USERDIR = System.getProperty("user.dir");
   public  java.lang.String      SEP     = System.getProperty("file.separator");
   private static java.lang.String  TERMINATOR         ="\n";
   public static int             DEBUG   = 0;
   public static int             INFO    = 1;
   public static int             WARN    = 2;
   public static int             ERROR   = 3;
   public static int             FATAL   = 4;    
   public DailyRollingFileAppender     fileAppender            = new DailyRollingFileAppender();
   static Logger                       appLauncherSocketLogger = Logger.getLogger(ProcessMonitor.class);
   private Layout                      layout                  = new PatternLayout("%-5p [%t]: %m%n");
   public DefaultExecutor              executor;
   public ExecuteWatchdog              watchdog;
   public ShutdownHookProcessDestroyer processDestroyer;
/*=============================================================================================
/       ProcessMonitor() Constructor
/=============================================================================================*/
public ProcessMonitor(){
    initializeLogging();
    isProcessRunning("java8");
    executeStartServer();
//    start_telemetry_server();
}   
/*=============================================================================================
/         initializeLogging()
/=============================================================================================*/
 private void initializeLogging(){
     BasicConfigurator.configure();
     fileAppender.setFile(System.getProperty("user.home") + SEP + "logs" + SEP + "TelemetryServer.log");
     fileAppender.setDatePattern("'.'yyyy-MM-dd");
     fileAppender.setAppend(true);
     fileAppender.setLayout(layout);
     fileAppender.activateOptions();
     appLauncherSocketLogger.addAppender(fileAppender);
 } 
/*================================================================================================
/      executeStartServer()
/=================================================================================================*/
public void executeStartServer(){
    try{
       StartServerThread myStartServerThread = new  StartServerThread();
       myStartServerThread.start();
       Thread.sleep(1000);    
       int pid = isProcessRunning("java-telemetry");
       writePIDtoFile(pid);       
    }catch(Exception e){
        System.out.println(e.toString());
    }
}
/*================================================================================================
/      start_DS9(InstrumentDescriptionObject ido)
/=================================================================================================*/
public void executeMonitor(ExecuteWatchdog new_watchdog,long new_waiting_time){
    ExecuteMonitorThread current = new ExecuteMonitorThread(new_watchdog,new_waiting_time);
    current.start();
}
/*================================================================================================
/      start_DS9(InstrumentDescriptionObject ido)
/=================================================================================================*/
public void start_server(){
    String line = "telemetry_server";
    long SECONDS_TO_MILLISECONDS = 1000;
    long TIMEOUT = 30*SECONDS_TO_MILLISECONDS;
    boolean state = true;
    try{
      executor      = new DefaultExecutor();
      watchdog      = new ExecuteWatchdog(TIMEOUT);
      processDestroyer = new ShutdownHookProcessDestroyer();      
      executor.setProcessDestroyer(processDestroyer);
      executor.setWatchdog(watchdog);      
      executeMonitor(watchdog,TIMEOUT);
      CommandLine commandLine = CommandLine.parse(line);
      int exit_value = executor.execute(commandLine);
    }catch(Exception e){
        System.out.println();
//        start_server();
    }
}  
/*================================================================================================
/      start_DS9(InstrumentDescriptionObject ido)
/=================================================================================================*/
 /**
  *
  * @param ido
  */
 public void start_telemetry_server(){
    String line = "telemetry_server";
        CommandLine commandLine = CommandLine.parse(line);
        logMessage(INFO,"method start_telemetry server: Server = ");
        ExecuteProcessThread commandProcessThread = new ExecuteProcessThread(commandLine);
        commandProcessThread.start();
 }
/*=============================================================================================
/       isProcessRunning(String processName)
/=============================================================================================*/    
   public int isProcessRunning(String processName){
     int                  process_ID     = 0;
     java.util.Stack      myLineStack    = new java.util.Stack();
     java.util.Stack      myProcessStack = new java.util.Stack();

     String line = USERDIR + SEP + "bin" + SEP + "ps_process.sh";
        CommandLine commandLine = CommandLine.parse(line);
        commandLine.addArgument(processName);
        ByteArrayOutputStream stdout            = new ByteArrayOutputStream();
        PumpStreamHandler     pumpStreamHandler = new PumpStreamHandler(stdout);
        DefaultExecutor       executor          = new DefaultExecutor();
                              executor.setStreamHandler( pumpStreamHandler );
        try{
           int exitValue              = executor.execute(commandLine);
           java.lang.String result = stdout.toString();
           if(result.isEmpty()){
            logMessage(INFO,"No active process found by isProcessRunning. "+processName);
            return 0;
           }
           if(!result.isEmpty()){
           // At this point the stdout contains the contents of the command
           java.util.StringTokenizer lineString = new java.util.StringTokenizer(result,TERMINATOR);
            while(lineString.hasMoreElements()){
                String line1 = lineString.nextToken();
                myLineStack.push(line1);
            }
            while(!myLineStack.isEmpty()){
                 java.lang.String          currentLine = (java.lang.String)myLineStack.pop();
                 ProcessDescriptionObject  current_pdo = new ProcessDescriptionObject(currentLine);
//                 if(current_pdo.isLabVIEW()){
//                     if(current_pdo.isProcess(ido)){
                        process_ID = (Integer.valueOf(current_pdo.PID)).intValue();
                        logMessage(INFO,"Process found by isProcessRunning. " + processName + current_pdo.PID);
                        logMessage(INFO,"Process ID = " + current_pdo.PID);
                        logMessage(INFO,"User = " + current_pdo.UID);
                        logMessage(INFO,"Command = " + current_pdo.CMD);
                        logMessage(INFO,"Parameter = " + current_pdo.CMD_PARAM);
                        if((current_pdo.CMD.matches("java-telemetry"))&(current_pdo.CMD_PARAM.matches("-jar"))){
                            process_ID = Integer.parseInt(current_pdo.PID);
                        }
//                    }
                   myProcessStack.push(current_pdo);
//                 }
            }
           }
       }catch(Exception e){
            logMessage(ERROR,"Error in isProcessRunning. "+e.toString());
       }
     return process_ID;
 }
/*================================================================================================
/       killProcess(int processID)
/=================================================================================================*/
  /**
   *
   * @param processID
   * @return
   */
  protected boolean killProcess(int processID){
    boolean success = false;
    if(processID == 0){
        return success;
    }
    String line = "kill";
        CommandLine commandLine   = CommandLine.parse(line);
        String      processString = Integer.toString(processID);
        commandLine.addArgument(processString);
         DefaultExecutor executor = new DefaultExecutor();
        try{
           logMessage(INFO,"Attempting to kill process. ProcessID = " + processString);
           int exitValue              = executor.execute(commandLine);
           success = true;
        }catch(Exception e){
             success = false;
             logMessage(ERROR,"Error while trying to kill process. ProcessID = " + processString + " " + e.toString());
        }

    return success;
 } 
/*=============================================================================================
/        writePIDtoFile(int pid)
/=============================================================================================*/
public void writePIDtoFile(int pid){
    java.lang.String pid_file = new java.lang.String(USERDIR+SEP+"config"+SEP+"pid_file.txt");           
    try{
       java.io.PrintWriter writer = new java.io.PrintWriter(pid_file);
       writer.println(pid);
       writer.close();
    }catch(Exception e){
        System.out.println(e.toString());
    }
}  
/*=============================================================================================
/        readPIDfromFile()
/=============================================================================================*/
public int readPIDfromFile(){
    int current_pid = 0;
    java.lang.String pid_file = new java.lang.String(USERDIR+SEP+"config"+SEP+"pid_file.txt"); 
    try{
        java.io.BufferedReader br = new java.io.BufferedReader(new java.io.FileReader(pid_file));
        java.lang.String pid_string = br.readLine();
        pid_string = pid_string.trim();
        current_pid = Integer.parseInt(pid_string);
    }catch(Exception e){
        System.out.println(e.toString());
    }
   return current_pid;
}
/*=============================================================================================
/        logMessage(java.lang.String message)
/=============================================================================================*/
/**
 *
 * @param code
 * @param message
 */
public void logMessage(int code,java.lang.String message){
     switch(code){
         case 0:  appLauncherSocketLogger.debug(message);  break;
         case 1:  appLauncherSocketLogger.info(message);   break;
         case 2:  appLauncherSocketLogger.warn(message);   break;
         case 3:  appLauncherSocketLogger.error(message);  break;
         case 4:  appLauncherSocketLogger.fatal(message);  break;
     }     
     System.out.println(message);    
 }
/*=============================================================================================
/         Inner Class   ExecuteProcessThread
/=============================================================================================*/
public class StartServerThread  implements Runnable{
    private Thread myThread;
    /**
     *
     * @param commandLine
     */
/*=============================================================================================
/       ExecuteProcessThread constructor
/=============================================================================================*/
    public StartServerThread(){
    }
    public void run(){
        start_server();
    }
    public void start(){
     myThread = new Thread(this);
     myThread.start();
     }
    }// End of the ExecuteProcessThread Inner Class

/*=============================================================================================
/         Inner Class   ExecuteProcessThread
/=============================================================================================*/
public class ExecuteProcessThread  implements Runnable{
    private Thread myThread;
    private CommandLine commandLine;
    /**
     *
     * @param commandLine
     */
/*=============================================================================================
/       ExecuteProcessThread constructor
/=============================================================================================*/
    public ExecuteProcessThread(CommandLine commandLine){
      this.commandLine = commandLine;
    }
    public void run(){
        DefaultExecutor executor = new DefaultExecutor();
        try{
           int exitValue              = executor.execute(commandLine);
        }catch(Exception e){
            logMessage(ERROR,"method ExecuteProcessThread error = " + e.toString());
        }
     }
    public void start(){
     myThread = new Thread(this);
     myThread.start();
     }
    }// End of the ExecuteProcessThread Inner Class
/*=============================================================================================
/         Inner Class   ExecuteProcessThread
/=============================================================================================*/
public class ExecuteMonitorThread  implements Runnable{
    private Thread myThread;
    private ExecuteWatchdog current_watchdog;
    private long waiting_time;
    /**
     *
     * @param commandLine
     */
/*=============================================================================================
/       ExecuteProcessThread constructor
/=============================================================================================*/
    public ExecuteMonitorThread(ExecuteWatchdog new_watchdog,long new_waiting_time){
      this.current_watchdog = new_watchdog;
      this.waiting_time     = new_waiting_time;
    }
    public void run(){
        long start = System.currentTimeMillis();
        try{
          long SECONDS_TO_MILLISECONDS = 1000;
          long TIMEOUT = 23*60*60*SECONDS_TO_MILLISECONDS;
//          TIMEOUT = 30000;
          Thread.sleep(TIMEOUT);         
        }catch(Exception e){
            System.out.println(e.toString());
        }
        long end = System.currentTimeMillis();
        long duration = end - start;
        System.out.println("PROCESS RAN FOR THE FOLLOWING TIME = "+duration);
        int pid = readPIDfromFile();
//        int pid = isProcessRunning("java-telemetry");
        killProcess(pid);
//        current_watchdog.destroyProcess();
//        boolean state =current_watchdog.killedProcess();
        executeStartServer();
     }
    public void start(){
     myThread = new Thread(this);
     myThread.start();
     }
    }// End of the ExecuteProcessThread Inner Class
/*=============================================================================================
/       main
/=============================================================================================*/
  public static void main(String[] args) {

    try {
      UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
     }
    catch(Exception e) {
      e.printStackTrace();
    }
    new ProcessMonitor();
  }
}
