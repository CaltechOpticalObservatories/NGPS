/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package edu.caltech.palomar.telescopes.telemetry;
//=== File Prolog =============================================================
//	This code was developed by California Institute of Technology
//      Caltech Optical Observatories, Palomar Observatory
//
//--- Contents ----------------------------------------------------------------
//	 ProcessDescriptionObject- This is a class for holding
//       a complete process description resulting from the ps command
//
//--- Description -------------------------------------------------------------
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
//
//      October 29, 2010 Jennifer Milburn
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
/        ProcessDescriptionObject() - This method reads the initialization file.
/=================================================================================================*/
public class ProcessDescriptionObject {
  public        java.lang.String F                    = new java.lang.String();
  public        java.lang.String S                    = new java.lang.String();
  public        java.lang.String UID                  = new java.lang.String();
  public        java.lang.String PID                  = new java.lang.String();
  public        java.lang.String PPID                 = new java.lang.String();
  public        java.lang.String C                    = new java.lang.String();
  public        java.lang.String PRI                  = new java.lang.String();
  public        java.lang.String NI                   = new java.lang.String();
  public        java.lang.String ADDR                 = new java.lang.String();
  public        java.lang.String SZ                   = new java.lang.String();
  public        java.lang.String WCHAN                = new java.lang.String();
  public        java.lang.String STIME                = new java.lang.String();
  public        java.lang.String TTY                  = new java.lang.String();
  public        java.lang.String TIME                 = new java.lang.String();
  public        java.lang.String CMD                  = new java.lang.String();
  public        java.lang.String CMD_PARAM            = new java.lang.String();
  java.util.StringTokenizer      trs;
  public        java.lang.String currentLine          = new java.lang.String();
/*================================================================================================
/        ProcessDescriptionObject() - Default Constructor
/=================================================================================================*/
 public ProcessDescriptionObject(java.lang.String currentLine){
    this.currentLine = currentLine;
    process(currentLine);
 }
/*================================================================================================
/        process(java.lang.String currentLine)
/=================================================================================================*/
 public void process(java.lang.String currentLine){
     trs       = new java.util.StringTokenizer(currentLine," ");
     F         = trs.nextToken();
     S         = trs.nextToken();
     UID       = trs.nextToken();
     PID       = trs.nextToken();
     PPID      = trs.nextToken();
     C         = trs.nextToken();
     PRI       = trs.nextToken();
     NI        = trs.nextToken();
     ADDR      = trs.nextToken();
     SZ        = trs.nextToken();
     WCHAN     = trs.nextToken();
     STIME     = trs.nextToken();
     TTY       = trs.nextToken();
     TIME      = trs.nextToken();
     CMD       = trs.nextToken();
     CMD_PARAM = trs.nextToken();
    }
/*================================================================================================
/      isLabVIEW()
/=================================================================================================*/
public boolean isLabVIEW(){
   boolean state = false;
   if(CMD.contains("labview")){
       state = true;
   }
   return state;
}
/*================================================================================================
/      isLabVIEW()
/=================================================================================================*/
public boolean isAppLauncher(){
   boolean state = false;
   if(CMD.contains("AppLauncher")){
       state = true;
   }
   return state;
}
/*================================================================================================
/        boolean isGUI()
/=================================================================================================*/
public boolean isProcess(InstrumentDescriptionObject ido){
   boolean state = false;
   if(CMD_PARAM.contains(ido.APP_TREE) & CMD_PARAM.contains(ido.ARCROOT)){
       state = true;
   }
   return state;
}
}
