package edu.caltech.palomar.telescopes.telemetry;
//=== File Prolog =============================================================
//	This code was developed by California Institute of Technology
//      Caltech Optical Observatories, Palomar Observatory
//
//--- Contents ----------------------------------------------------------------
//	 InstrumentDescriptionObject- This is a class for reading the initialization file
//       associated with an applauncher and storing the parameters for a generic instrument
//
//--- Description -------------------------------------------------------------
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
//
//      October 18, 2010 Jennifer Milburn
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
import com.ibm.inifile.IniFile;
import java.io.File;
/*================================================================================================
/        initializeINIFile() - This method reads the initialization file.
/=================================================================================================*/
public class InstrumentDescriptionObject {
  public        java.lang.String SYSTEM                             = new java.lang.String();
  public        java.lang.String INSTRUMENT_NAME                    = new java.lang.String();
  //  SECTION VARIABLE NAMES
  public        java.lang.String ARCROOT                            = new java.lang.String();
  public        java.lang.String APP_TREE                           = new java.lang.String();
  public        java.lang.String ARCNAME                            = new java.lang.String();
  public        java.lang.String AUTO_DIR                           = new java.lang.String();
  public        java.lang.String CONFIG_FILE                        = new java.lang.String();
  public        java.lang.String FORCE                              = new java.lang.String();
  public        java.lang.String COMSTCP_CMD_PORT                   = new java.lang.String();
  public        java.lang.String COMSTCP_ASYNC_PORT                 = new java.lang.String();
  public        java.lang.String SYNC_PORT                          = new java.lang.String();
  public        java.lang.String SYNC_ASYNC_PORT                    = new java.lang.String();
  public        java.lang.String SYNC_BLOCK_PORT                    = new java.lang.String();
  public        java.lang.String INSTRUMENT_TYPE                    = new java.lang.String();
  public        java.lang.String SERVER_HOST                        = new java.lang.String();
  public        java.lang.String SET_PORTS_SCRIPT                   = new java.lang.String();
  public        java.lang.String INSTRUMENT_DATA_ROOT               = new java.lang.String();
  public        java.lang.String REAL_TIME_DISPLAY                  = new java.lang.String();
  public        java.lang.String FIFO                               = new java.lang.String();
  public        java.lang.String IMAGE_NAME                         = new java.lang.String();
  public        boolean          START_VNCVIEWER;
  public        java.lang.String STARTUP_WAIT_TIME                  = new java.lang.String();
  // Initialization File Bean
  public  IniFile        myIniFile;
/*================================================================================================
/        InstrumentDescriptionObject() - Default Constructor
/=================================================================================================*/
 public InstrumentDescriptionObject(java.lang.String newInstrumentName,IniFile newIniFile){
      this.INSTRUMENT_NAME = newInstrumentName;
      this.myIniFile  = newIniFile;
      initializeParameters();
 }
/*=================================================================================
/          logMessage Method  - 
/=================================================================================*/
public void logMessage(java.lang.String myMessage){
     System.out.println(myMessage);
}
/*================================================================================================
/       initializeParameters()
/=================================================================================================*/
public void initializeParameters(){
  java.lang.String sectionName = new java.lang.String();
  sectionName = INSTRUMENT_NAME;
  try{
         // Instrument Properties
        SYSTEM                      = myIniFile.getProperty("SYSTEM",            INSTRUMENT_NAME);
        INSTRUMENT_TYPE             = myIniFile.getProperty("INSTRUMENT_TYPE",   INSTRUMENT_NAME);
        ARCROOT                     = myIniFile.getProperty("ARCROOT",           INSTRUMENT_NAME);
        APP_TREE                    = myIniFile.getProperty("APP_TREE",          INSTRUMENT_NAME);
        ARCNAME                     = myIniFile.getProperty("ARCNAME",           INSTRUMENT_NAME);
        AUTO_DIR                    = myIniFile.getProperty("AUTO_DIR",          INSTRUMENT_NAME);
        CONFIG_FILE                 = myIniFile.getProperty("CONFIG_FILE",       INSTRUMENT_NAME);
        FORCE                       = myIniFile.getProperty("FORCE",             INSTRUMENT_NAME);
        COMSTCP_CMD_PORT            = myIniFile.getProperty("COMSTCP_CMD_PORT",  INSTRUMENT_NAME);
        COMSTCP_ASYNC_PORT          = myIniFile.getProperty("COMSTCP_ASYNC_PORT",INSTRUMENT_NAME);
        SYNC_PORT                   = myIniFile.getProperty("SYNC_PORT",         INSTRUMENT_NAME);
        SYNC_ASYNC_PORT             = myIniFile.getProperty("SYNC_ASYNC_PORT",   INSTRUMENT_NAME);
        SYNC_BLOCK_PORT             = myIniFile.getProperty("SYNC_BLOCK_PORT",   INSTRUMENT_NAME);
        SERVER_HOST                 = myIniFile.getProperty("SERVER_HOST",       INSTRUMENT_NAME);
        SET_PORTS_SCRIPT            = myIniFile.getProperty("SET_PORTS_SCRIPT",  INSTRUMENT_NAME);
        INSTRUMENT_DATA_ROOT        = myIniFile.getProperty("INSTRUMENT_DATA_ROOT",INSTRUMENT_NAME);
        REAL_TIME_DISPLAY           = myIniFile.getProperty("REAL_TIME_DISPLAY", INSTRUMENT_NAME);
        FIFO                        = myIniFile.getProperty("FIFO",              INSTRUMENT_NAME);
        IMAGE_NAME                  = myIniFile.getProperty("IMAGE_NAME",        INSTRUMENT_NAME);
        START_VNCVIEWER             = Boolean.parseBoolean(myIniFile.getProperty("START_VNCVIEWER",   INSTRUMENT_NAME));
        STARTUP_WAIT_TIME           = myIniFile.getProperty("STARTUP_WAIT_TIME", INSTRUMENT_NAME);
    }catch(Exception e){
    logMessage("An error occurred while reading the parameters from the Initialization File. "+e);
  }
}
}
