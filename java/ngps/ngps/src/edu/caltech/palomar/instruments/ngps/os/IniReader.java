/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package edu.caltech.palomar.instruments.ngps.os;

import com.ibm.inifile.IniFile;
import java.io.FileInputStream;
import java.util.Properties;

/**
 *
 * @author jennifermilburn
 */
public class IniReader {
  public static java.lang.String CONNECTION_PARAMETERS            = new java.lang.String("CONNECTION_PARAMETERS");
  public   IniFile          myIniFile;
  private  String           iniFileName;
  private  java.io.File     iniFile;
  public   java.lang.String  USERDIR          = System.getProperty("user.dir");
  public   java.lang.String  SEP              = System.getProperty("file.separator");
  public   java.lang.String  CONFIG           = new java.lang.String("config");
  public   java.lang.String  DHE              = new java.lang.String("obs_seq_connect.ini");
  public   java.lang.String  INIFILE          = new java.lang.String();
  public   java.lang.String  SERVERNAME           = new java.lang.String();
  public   java.lang.String  INSTRUMENT_NAME      = new java.lang.String();
  public java.lang.String    BASENAME             =  new java.lang.String();
  public java.lang.String    ASYNC_GROUP          =  new java.lang.String();
  public   int               COMMAND_SERVERPORT;
  public   int               BLOCKING_SERVERPORT;
  public   int               ASYNC_SERVERPORT;
  public java.lang.String    ASYNC_HOST;
  public java.lang.String    LOG_DIRECTORY          = new java.lang.String();
/*================================================================================================
/        IniReader() constructor
/=================================================================================================*/
    public IniReader(){
       //bootstrap();
       INIFILE = USERDIR + SEP + CONFIG + SEP + DHE;
       setIniFileName(INIFILE);
       initializeINIFile();
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
    }catch(Exception e){
        System.out.println(e.toString());
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
          SERVERNAME                = myIniFile.getProperty("SERVERNAME",sectionName);
          INSTRUMENT_NAME           = myIniFile.getProperty("INSTRUMENT_NAME", sectionName);
          COMMAND_SERVERPORT        = Integer.parseInt(myIniFile.getProperty("COMMAND_SERVERPORT",sectionName));
          BLOCKING_SERVERPORT       = Integer.parseInt(myIniFile.getProperty("BLOCKING_SERVERPORT",sectionName));
          ASYNC_SERVERPORT          = Integer.parseInt(myIniFile.getProperty("ASYNC_SERVERPORT",sectionName));
          ASYNC_HOST                = myIniFile.getProperty("ASYNC_HOST", sectionName);
          BASENAME                  = myIniFile.getProperty("BASENAME",sectionName);
          LOG_DIRECTORY             = myIniFile.getProperty("LOG_DIRECTORY"       ,sectionName);
    }
    catch(Exception e){
      logMessage("An error occurred while reading the Connection Parameters from the Initialization File. "+e);
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
    public static void main(String args[]) {
 
        /* Create and display the form */
        java.awt.EventQueue.invokeLater(new Runnable() {
            public void run() {
               new IniReader();
            }
        });
    }
}
