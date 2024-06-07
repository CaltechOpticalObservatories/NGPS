/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package edu.caltech.palomar.instruments.ngps.util;

import edu.caltech.palomar.instruments.ngps.otm.OTMlauncher;
import edu.caltech.palomar.util.general.CommandLogModel;
import java.io.FileInputStream;
import java.util.Properties;
import org.apache.commons.exec.DefaultExecutor;
import org.apache.commons.exec.ExecuteWatchdog;
import org.apache.commons.exec.PumpStreamHandler;
import org.apache.commons.exec.ShutdownHookProcessDestroyer;
import org.apache.commons.exec.CommandLine;

/**
 *
 * @author developer
 */
public class BrowserDisplay {
   private DefaultExecutor              executor;
   private ShutdownHookProcessDestroyer processDestroyer;
   private ExecuteWatchdog              watchdog;
   public java.lang.String USERDIR           = System.getProperty("user.dir");
   public java.lang.String SEP               = System.getProperty("file.separator");
   
   public java.lang.String URL_FILE = USERDIR+SEP+"config"+SEP+"url.ini";  
   public Properties       URLMAP                 = new Properties();
   
/*================================================================================================
/     BrowserDisplay()
/=================================================================================================*/   
    public BrowserDisplay(){
       //bootstrap();
       System.out.println(URL_FILE);
       try{
       FileInputStream  url_properties_file          = new FileInputStream(URL_FILE);
       URLMAP.load(url_properties_file);
       url_properties_file.close();
       } catch(Exception e){
        System.out.println(e.toString());
        }
       
       System.out.println(URLMAP.getProperty("LINELIST"));

       initialize(); 
       boolean test = false;
       if(test){
//          executeFirefox("https://weather.com/weather/hourbyhour/l/Palomar+Mountain+CA?canonicalCityId=e7b3768d25eeb5f6f9b943def2136fb5b65865ba22e8e1954f006fedbe29d5db"); 
            executeFirefox(HOURLY_WEATHER);
       }
    }
/*================================================================================================
/      initialize()
/=================================================================================================*/
 private void initialize(){
   executor         = new DefaultExecutor();
   processDestroyer = new ShutdownHookProcessDestroyer();
   watchdog         = new ExecuteWatchdog(Integer.MAX_VALUE);
   executor.setWatchdog(watchdog);
   executor.setProcessDestroyer(processDestroyer);
 }  
/*================================================================================================
/       initializeDBMS()
/=================================================================================================*/
public void bootstrap(){
    try{
      java.lang.String BOOTSTRAP_FILE = USERDIR + SEP +"bootstrap.ini";
      FileInputStream  bootstrap_properties_file            = new FileInputStream(BOOTSTRAP_FILE);
      Properties       bootstrap_properties                 = new Properties();
      bootstrap_properties.load(bootstrap_properties_file);
      bootstrap_properties_file.close();
      USERDIR        = bootstrap_properties.getProperty("USERDIR");
      java.lang.String URL_FILE = USERDIR+SEP+"config"+SEP+"url.ini";
      FileInputStream  url_properties_file            = new FileInputStream(URL_FILE);
      Properties       url_properties                 = new Properties();
      URLMAP.load(url_properties_file);
      url_properties_file.close();
    }catch(Exception e){
        System.out.println(e.toString());
    }
} 
/*================================================================================================
/     executeOTM(java.sql.Timestamp start_time,double seeing,int wavelength)
/=================================================================================================*/
 public void executeFirefox(java.lang.String target_url){
   ExecuteProcessThread myExecuteProcessThread = new ExecuteProcessThread(target_url);
   myExecuteProcessThread.start();
 }
/*================================================================================================
/      OTM(java.sql.Timestamp start_time,double seeing,int wavelength)
/=================================================================================================*/
public void firefox(java.lang.String target_url){
    java.lang.String line = new java.lang.String("firefox");
       try{
           CommandLine commandLine = CommandLine.parse(line);
           commandLine.addArgument(target_url);
          int exitValue              = executor.execute(commandLine);              
       }catch(Exception e){                      
       }
} 
 /*=============================================================================================
/     INNER CLASS that executes a process in separate thread so that it doesn't
 *     block execution of the rest of the system. 
/=============================================================================================*/
public class ExecuteProcessThread  implements Runnable{
    private Thread             myThread;
    private java.lang.String   target_url;
    
    public ExecuteProcessThread(java.lang.String target_url){
      this.target_url  = target_url;
    }
    public void run(){
        try{  
           firefox(target_url);
        }catch(Exception e){
           System.out.println(e.toString());
        }
     }
    public void start(){
     myThread = new Thread(this);
     myThread.start();
     }
    }// End of the ExecuteProcessThread Inner Class
/*================================================================================================
/         Main Method - Starts the entire system
/=================================================================================================*/
  //Main method
   public static void main(String[] args) {
     try {
     }
     catch(Exception e) {
        e.printStackTrace();
     }
     new BrowserDisplay();
   }    
}
