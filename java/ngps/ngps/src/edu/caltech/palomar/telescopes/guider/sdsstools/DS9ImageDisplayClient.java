/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
 
package edu.caltech.palomar.telescopes.guider.sdsstools;
//=== File Prolog =============================================================
//	This code was developed by John Thorstensen of Dartmouth University
//
//      It has been refactored and modified by Jennifer Milburn for
//      use at the Palomar Observatory.
//
//      Caltech Optical Observatories, Palomar Observatory
//
//--- Contents ----------------------------------------------------------------
//    DS9ImageDisplayClient   Jennifer Milburn September 30,2011
//
//--- Description -------------------------------------------------------------
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
//
// TERMS OF USE --
// Anyone is free to use this software for any purpose, and to
// modify it for their own purposes, provided that credit is given to the author
// in a prominent place.  For the present program that means that the green
// title and author banner appearing on the main window must not be removed,
// and may not be altered without premission of the author.
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
import edu.caltech.palomar.telescopes.P200.TelescopesIniReader;
import java.beans.PropertyChangeEvent;
import java.awt.event.ActionEvent;
import javax.swing.JFrame;
import edu.caltech.palomar.instruments.gui.ds9.DS9Accessor;
import org.apache.commons.exec.*;
/*===============================================================================================================
/    DS9ImageDisplayClient Class Definition
/===============================================================================================================*/
public class DS9ImageDisplayClient {
  public  SDSSGuiderImageFinderTool mySDSSImageFinderTool;
  public  TelescopesIniReader       myTelescopesIniReader = new TelescopesIniReader();
  public  DS9Accessor               myDS9Accessor = new DS9Accessor();
  public  JFrame                    parentFrame;
  private boolean                   display_DS9;
  private boolean                   display_catalog;
  private java.lang.String          my_Region_File_Name;
/*===============================================================================================================
/    DS9ImageDisplayClient Constructor()
/===============================================================================================================*/
  public DS9ImageDisplayClient(JFrame parentFrame){
    this.parentFrame           = parentFrame;
    initializeImageFinder();
    setDisplayDS9(true);
  }
/*===============================================================================================================
/    DS9ImageDisplayClient Constructor()
/===============================================================================================================*/
  public DS9ImageDisplayClient(){
    initializeImageFinder();
  }
/*===============================================================================================================
/    setDisplayDS9(boolean new_display_DS9)
/===============================================================================================================*/
public void setDisplayDS9(boolean new_display_DS9){
     display_DS9 = new_display_DS9;
 }
 public boolean isDisplayDS9(){
     return display_DS9;
 }
/*===============================================================================================================
/    setDisplayCatalog(boolean new_display_catalog)
/===============================================================================================================*/
public void setDisplayCatalog(boolean new_display_catalog){
     display_catalog = new_display_catalog;
 }
 public boolean isDisplayCatalog(){
     return display_catalog;
 }
/*=============================================================================================
/   setTelescopeIniReader(TelescopesIniReader newTelescopesIniReader)
/=============================================================================================*/
public void setRegionFileName(java.lang.String new_Region_File_Name){
    my_Region_File_Name = new_Region_File_Name;
 }
 public java.lang.String getRegionFileName(){
     return my_Region_File_Name;
 }
/*=============================================================================================
/   setTelescopeIniReader(TelescopesIniReader newTelescopesIniReader)
/=============================================================================================*/
 public void setTelescopeIniReader(TelescopesIniReader newTelescopesIniReader){
     myTelescopesIniReader = newTelescopesIniReader;
 }
/*================================================================================================
/     retrieveImage()
/=================================================================================================*/
   public void retrieveImage(double ra, double dec,double new_width ,double new_height){
      mySDSSImageFinderTool.setRa(ra);
      mySDSSImageFinderTool.setDec(dec);
      mySDSSImageFinderTool.setSearchBoxWidth(new_width);
      mySDSSImageFinderTool.setSearchBoxHeight(new_height);
          try{
            mySDSSImageFinderTool.runImageQuery(SDSSGuiderImageFinderTool.FITS);
          }catch(Exception e){
              System.out.println("MalformedURLException "+e.toString());
       }
    }
/*================================================================================================
/      initializeImageFinder()
/=================================================================================================*/
private void initializeImageFinder(){
/*================================================================================================
/=================================================================================================*/
    if(parentFrame != null){
      mySDSSImageFinderTool          = new SDSSGuiderImageFinderTool(parentFrame,false);
    }
    if(parentFrame == null){
      mySDSSImageFinderTool          = new SDSSGuiderImageFinderTool(false);
    }
//     mySDSSImageFinderTool.setNavigatorImageDisplayControl(myImageDisplayControl);
     mySDSSImageFinderTool.setTelescopeIniReader(myTelescopesIniReader);
     mySDSSImageFinderTool.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
        public void propertyChange(java.beans.PropertyChangeEvent e) {
            mySDSSImageFinderTool_propertyChange(e);
          }
        });
     mySDSSImageFinderTool.addActionListener(new java.awt.event.ActionListener(){
      public void actionPerformed(java.awt.event.ActionEvent e){
         mySDSSImageFinderTool_actionPerformed(e);
      }
     });
}
/*=============================================================================================
/      updateFileName()
/=============================================================================================*/
    public void updateImage(java.lang.String newFileName){
        RunUpdateImage myRunUpdateImage = new RunUpdateImage(newFileName);
        myRunUpdateImage.start();
    }
/*=============================================================================================
/      updateImage(java.lang.String newFileName,java.lang.String new_region_FileName)
/=============================================================================================*/
    public void updateImage(java.lang.String newFileName,java.lang.String new_region_FileName){
        RunUpdateImage myRunUpdateImage = new RunUpdateImage(newFileName,new_region_FileName);
        myRunUpdateImage.start();
    }
/*=============================================================================================
/     Property Change Listener for the myFFISDSSImageFinderTool
/=============================================================================================*/
    private void mySDSSImageFinderTool_propertyChange(PropertyChangeEvent e)  {
           java.lang.String propertyName = e.getPropertyName();
/*=============================================================================================
/                 myFFISDSSImageFinderTool_propertyChange
/=============================================================================================*/
           if(propertyName == "transferState"){
              java.lang.Integer newIntegerValue = (java.lang.Integer)e.getNewValue();
              int               transState      = newIntegerValue.intValue();
              if(transState == SDSSGuiderImageFinderTool.TRANSFER_COMPLETED){
                java.lang.String newFileName = mySDSSImageFinderTool.getDownloadFileName();
                if(isDisplayDS9()){
                    if(isDisplayCatalog()){
                       updateImage(newFileName,my_Region_File_Name);
                    }
                    if(!isDisplayCatalog()){
                       updateImage(newFileName);
                    }
                }
               }
              if(transState == SDSSGuiderImageFinderTool.TRANSFER_STARTED){
              }
           }
           if(propertyName == "progress"){
               java.lang.Integer newIntegerValue = (java.lang.Integer)e.getNewValue();
               int progress      = newIntegerValue.intValue();
           }
  }
/*================================================================================================
/         mySDSSImageFinderTool_actionPerformed - Toolbar Button Action
/=================================================================================================*/
 public void mySDSSImageFinderTool_actionPerformed(ActionEvent e){
   int myAction = e.ACTION_PERFORMED;
   if(myAction == mySDSSImageFinderTool.FILE_RETRIEVED){
       java.lang.String fitsFile     = new java.lang.String();
       fitsFile = e.getActionCommand();
   }
 }
//================================================================================================
//     Action Listeners that are autogenerated by the Netbeans graphical editor
//=================================================================================================
 public void start_DS9(){
     String TERMINAL   = "xterm";
     String XGTERMINAL = "xgterm";
     String EXEC       = "-exec";
        CommandLine commandLine = CommandLine.parse(TERMINAL);
        commandLine.addArgument(EXEC);
        commandLine.addArgument("ds9");
 //       commandLine.addArgument(" &");
        logMessage("method start_DS9:");
        ExecuteProcessThread commandProcessThread = new ExecuteProcessThread(commandLine);
        commandProcessThread.start();
 }
 //================================================================================================
//    logMessage(java.lang.String message)
//=================================================================================================
public void logMessage(java.lang.String message){
     System.out.println(message);
 }
/*=============================================================================================
/    void ThreadSleep()
/=============================================================================================*/
  public void ThreadSleep(int SLEEP_TIME){
    try{
          Thread.currentThread().sleep(SLEEP_TIME);
      }
      catch (Exception e) {
            logMessage("An error occured while waiting for the image to be transfered. " +e.toString());
     }
  }
 //================================================================================================
//     Action Listeners that are autogenerated by the Netbeans graphical editor
//=================================================================================================
  public void displayDS9(java.lang.String DS9_filename){
    try{
      if(myDS9Accessor.isDS9Ready(true)){
         myDS9Accessor.showFits(DS9_filename);
     }
     if(!myDS9Accessor.isDS9Ready(false)){
         start_DS9();
         ThreadSleep(1000);
         myDS9Accessor.showFits(DS9_filename);
    }
     } catch (Exception error) {
        System.out.println("DS9 access throws and error");
    }
  }
//================================================================================================
//     Action Listeners that are autogenerated by the Netbeans graphical editor
//=================================================================================================
  public void displayDS9(java.lang.String DS9_filename,java.lang.String new_region_FileName){
    try{
      if(myDS9Accessor.isDS9Ready(true)){
         java.lang.String display_region_command = "xpaset -p ds9 regions load "+new_region_FileName+".reg";
          try{
              myDS9Accessor.showFits(DS9_filename);
              myDS9Accessor.doCmd(display_region_command);
           }catch(Exception ea){
             System.out.println(ea.toString());
           }
     }
     } catch (Exception error) {
        System.out.println("DS9 access throws and error");
    }
  }
/*=============================================================================================
/                      RunUpdateImage Inner Class
/=============================================================================================*/
   public class RunUpdateImage implements Runnable{
        private Thread myThread;
        private java.net.URL myURL;
        private java.lang.String myFileName         = new java.lang.String();
        private java.lang.String my_region_FileName;
        public RunUpdateImage(java.lang.String newFileName){
         myFileName = newFileName;
        }
        public RunUpdateImage(java.lang.String newFileName,java.lang.String new_region_FileName){
         myFileName = newFileName;
         my_region_FileName = new_region_FileName;
        }
        public void run(){
          try{
           if(my_region_FileName != null){
             displayDS9(myFileName,my_region_FileName);
           }
           if(my_region_FileName == null){
             displayDS9(myFileName);
           }
          }catch(Exception e){
            System.out.println("MalformedURLException "+e.toString());
          }
        }
        public void start(){
         myThread = new Thread(this);
         myThread.start();
         }
        }//// End of the RunUpdateFFIImage Inner Class
/*=============================================================================================
/     INNER CLASS that executes a process in separate thread so that it doesn't
 *     block execution of the rest of the system.
/=============================================================================================*/
public class ExecuteProcessThread  implements Runnable{
    private Thread myThread;
    private CommandLine commandLine;
    /**
     *
     * @param commandLine
     */
    public ExecuteProcessThread(CommandLine commandLine){
      this.commandLine = commandLine;
    }
    public void run(){
        DefaultExecutor executor = new DefaultExecutor();
        try{
           int exitValue              = executor.execute(commandLine);
        }catch(Exception e){
            logMessage("method ExecuteProcessThread error = " + e.toString());
        }
     }
    /**
     *
     */
    public void start(){
     myThread = new Thread(this);
     myThread.start();
     }
    }// End of the ExecuteProcessThread Inner Class
 /*================================================================================================
/       End of the AppLauncherSocket Class
/=================================================================================================*/
/*=============================================================================================
/                           End of the RunUpdateImage Inner Class
/=============================================================================================*/
}
