package edu.caltech.palomar.telescopes.guider.catalog.server;
//=== File Prolog =============================================================
//	This code was developed by California Institute of Technology
//      Caltech Optical Observatories, Palomar Observatory
//
//--- Contents ----------------------------------------------------------------
//	 CatalogServer
//
//--- Description -------------------------------------------------------------
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
//
//      October 10, 2011 Jennifer Milburn
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
/*================================================================================================
/      CatalogIniReader Class Declaration
/=================================================================================================*/
import java.lang.Object;
import java.io.File;
import com.ibm.inifile.IniFile;
/*=============================================================================================
/      class CatalogIniReader
/=============================================================================================*/
public class CatalogIniReader extends java.lang.Object{
  // Static Strings that represent the sections of the INI file.
  public static java.lang.String CONNECTION_PARAMETERS            = new java.lang.String("CONNECTION_PARAMETERS");
  public static java.lang.String CATALOG_SECTION                  = new java.lang.String("CATALOG_SECTION");
      // Initialization File Bean
  public IniFile myIniFile;
  private String iniFileName;
  private java.io.File iniFile;
  public   java.lang.String USERDIR          = System.getProperty("user.dir");
  public   java.lang.String SEP              = System.getProperty("file.separator");
  public   java.lang.String CONFIG           = new java.lang.String("config");
  public   java.lang.String TELESCOPE        = new java.lang.String("catalog.ini");
  public   java.lang.String INIFILE          = new java.lang.String();
    //   decimal representations of WCS default values
  public   int                    SERVERPORT;
  public   boolean                SIMULATION_STATE_BOOLEAN;
  public  java.lang.String  TYCHO2_INDEX            = new java.lang.String();
  public  java.lang.String  TYCHO2_CATALOG          = new java.lang.String();
  public  java.lang.String  UCAC3PATH               = new java.lang.String();
  public  java.lang.String  DEFAULT_OBSERVER_DIR    = new java.lang.String();
  public  java.lang.String  SDSS_IMAGE_PATH         = new java.lang.String();
  public  java.lang.String  LOG_DIRECTORY           = new java.lang.String();
  public  java.lang.String  SERVERNAME              = new java.lang.String();
/*=================================================================================================
/        CatalogIniReader Default Constructor
/=================================================================================================*/
  public CatalogIniReader() {
    INIFILE = USERDIR + SEP + CONFIG + SEP + TELESCOPE;
    setIniFileName(INIFILE);
    initializeINIFile();
  }
/*================================================================================================
/        TelescopeIniReader Constructor that initializes and reads the INI File
/=================================================================================================*/
  public CatalogIniReader(java.lang.String newIniFileName,boolean newInitialize) {
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
      initializeCatalog();
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
    try{
          // Construct the integer version of the values retrieved from the initialization file
          SERVERPORT        = (Integer.valueOf(myIniFile.getProperty("SERVERPORT",sectionName))).intValue();
          SERVERNAME        = myIniFile.getProperty("SERVERNAME",sectionName);
    }
    catch(Exception e){
      logMessage("An error occurred while reading the SOFIA Simulator Connection Parameters from the Initialization File. "+e);
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
          LOG_DIRECTORY        = myIniFile.getProperty("LOG_DIRECTORY"       ,sectionName);
     }
    catch(Exception e){
      logMessage("An error occurred while reading the UCAC3 catalog location from the Initialization File. "+e);
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

