package edu.caltech.palomar.telescopes.P200.gui.tables; 
//=== File Prolog =============================================================
//	This code was developed by John Thorstensen of Dartmouth University
//
//      It has been refactored and modified by Jennifer Milburn for
//      use at the Palomar Observatory.
//
//      Caltech Optical Observatories, Palomar Observatory
//
//--- Contents ----------------------------------------------------------------
//    AstroObjectsModel   -
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
import javax.swing.*;
import java.beans.*;
import java.io.File;
import javax.swing.filechooser.FileFilter;
import java.io.*;
import java.awt.event.*;
import edu.caltech.palomar.telescopes.P200.gui.tables.AstroObject;
import edu.caltech.palomar.telescopes.P200.gui.tables.AstroObjectTableModel;
import edu.dartmouth.jskycalc.coord.RA;
import edu.dartmouth.jskycalc.coord.dec;
import java.util.HashMap;
import edu.dartmouth.jskycalc.gui.AirmassDisplay;
import java.util.Vector;
import edu.caltech.palomar.telescopes.P200.gui.tables.AstroObjectNameListModel;
import edu.caltech.palomar.telescopes.P200.TelescopesIniReader;
import edu.caltech.palomar.util.general.TargetListDocumentModel;
import edu.dartmouth.jskycalc.JSkyCalcModel;
import com.sharkysoft.printf.Printf;
import com.sharkysoft.printf.PrintfData;
import com.sharkysoft.printf.PrintfTemplate;
import com.sharkysoft.printf.PrintfTemplateException;
/*================================================================================================
/          Class Declaration for AstroObjectsModel
/=================================================================================================*/
public class AstroObjectsModel {
  private transient PropertyChangeSupport propertyChangeListeners = new PropertyChangeSupport(this);
  private transient Vector  actionListeners;
  public   java.lang.String USERDIR        = System.getProperty("user.dir");
  public   java.lang.String SEP            = System.getProperty("file.separator");
  public   java.lang.String IMAGE_CACHE    = new java.lang.String(SEP + "images" + SEP);
  public   java.lang.String AORS           = new java.lang.String("aors");
  private  java.lang.String TERMINATOR     = new java.lang.String("\n");

  public static  java.lang.String             COMA       = ",";
  public static  java.lang.String             SEMICOLON  = ";";
  public         java.lang.String             TAB        = "\t";
  public static  java.lang.String             SPACE      = " ";
  public static  java.lang.String             OTHER      = ",";  
  private String selected_right_ascension;
  private String selected_declination;
  private String selected_right_ascension_decimal;
  private String selected_declination_decimal;
  private String selected_equinox_string;
  private String selected_objectname;
  private String selected_right_ascension_track_rate;
  private String selected_declination_track_rate;
  private String rates_units;
  private String delimiter;
  private String other_delimiter;
  private String currentDirectory;
  private AstroObject currentAstroObject;
  private int         selectedAstroObjectIndex;
  private boolean changed;
  private boolean isNew;
  public JFrame mainFrame;
  java.lang.String                    currentFileName    = new java.lang.String();
  java.io.File                        currentFile;
  public static int                   SAVE                     = 100;
  public static int                   SAVE_AS                  = 102;
  public static int                   SELECTION_CHANGED        = 200;
  public static int                   LIST_CHANGED             = 210;
  public AstroObjectTableModel        myAstroObjectTable       = new AstroObjectTableModel();
  public HashMap<String, AstroObject> presenterKey             = new HashMap<String, AstroObject>();
  public HashMap<String, AstroObject> selectedObjects          = new HashMap<String, AstroObject>();
  public AstroObjectNameListModel     myListModel              = new AstroObjectNameListModel();
  public TelescopesIniReader          myTelescopesIniReader    = new TelescopesIniReader();
  public TargetListDocumentModel      myTargetListDocumentModel = new TargetListDocumentModel();
  public JSkyCalcModel                myJSkyCalcModel;
  public static int                   PTIC                     = 500;
  public static int                   CSV                      = 600;
  private       int                   file_type;
//  AirmassDisplay                      myAirmassDisplay;
/*================================================================================================
/        AstroObjectsModel() Constructor
/=================================================================================================*/
  public AstroObjectsModel(JFrame mainFrame){ 
      this.mainFrame = mainFrame;
      initializeProperties();
   }
/*================================================================================================
/       initializeProperties()
/=================================================================================================*/
public void initializeProperties(){
    setSelectedRightAscension("");
    setSelectedDeclination("");
    setSelectedRightAscensionDecimal("");
    setSelectedDeclinationDecimal("");
    setSelectedRightAscensionTrackRate("");
    setSelectedDeclinationTrackRate("");
    setSelectedObjectName("");
    setRatesUnits("");
    setCurrentDirectory(myTelescopesIniReader.DEFAULT_OBSERVER_DIR+SEP);
    setDelimiter(COMA); 
    setOtherDelimiter(COMA);
    setFileType(PTIC);
}
/*=========================================================================================================
/     getTargetListDocumentModel()
/=========================================================================================================*/
public TargetListDocumentModel getTargetListDocumentModel(){
    return myTargetListDocumentModel;
}
/*=========================================================================================================
/     setJSkyCalcModel(JSkyCalcModel newJSkyCalcModel)
/=========================================================================================================*/
public void setJSkyCalcModel(JSkyCalcModel newJSkyCalcModel){
    myJSkyCalcModel = newJSkyCalcModel;
}
public JSkyCalcModel getJSkyCalcModel(){
    return myJSkyCalcModel;
}
/*=============================================================================================
/     updateAirmass()
/=============================================================================================*/
 public void updateAirmass(){
   UpdateAirmassThread  myUpdateAirmassThread = new  UpdateAirmassThread(); 
   myUpdateAirmassThread.start();
 }
/*=========================================================================================================
/     setAirmassDisplay()
/=========================================================================================================*/
// public void setAirmassDisplay(AirmassDisplay newAirmassDisplay){
//   myAirmassDisplay = newAirmassDisplay;
// }
// public void updateAirmassDisplay(){
//     myAirmassDisplay.Update();
 //    myAirmassDisplay.repaint();
 //}
/*=========================================================================================================
/     getSelectedObjects()
/=========================================================================================================*/
 public HashMap<String, AstroObject> getSelectedObjects(){
     return selectedObjects;
 }
/*=========================================================================================================
/     getObjectListModel()
/=========================================================================================================*/
 public AstroObjectNameListModel getObjectListModel(){
     return myListModel;
 }
/*=========================================================================================================
/     New()
/=========================================================================================================*/
 public void New(){
   RunNewTargetDefinitionFile myRunNewTargetDefinitionFile = new RunNewTargetDefinitionFile();
   myRunNewTargetDefinitionFile.start();
 }
/*=========================================================================================================
/     Open()
/=========================================================================================================*/
 public void Open(){
     RunOpenFile myRunOpenFile = new RunOpenFile();
     myRunOpenFile.start();
 }
/*=========================================================================================================
/     Save()
/=========================================================================================================*/
 public void Save(){
     RunSaveTargetDefinitionFile myRunSaveTargetDefinitionFile = new RunSaveTargetDefinitionFile(SAVE);
     myRunSaveTargetDefinitionFile.start();
 }
/*=========================================================================================================
/     SaveAs()
/=========================================================================================================*/
 public void SaveAs(){
     RunSaveTargetDefinitionFile myRunSaveTargetDefinitionFile = new RunSaveTargetDefinitionFile(SAVE_AS);
     myRunSaveTargetDefinitionFile.start();
 }
/*=========================================================================================================
/     Close()
/=========================================================================================================*/
 public void Close(){

 }
/*=========================================================================================================
/     void logMessage(java.lang.String newMessage)
/=========================================================================================================*/
public void logMessage(java.lang.String newMessage){
  System.out.println(newMessage);
}
/*=============================================================================================
/   setChanged(boolean changed)
/=============================================================================================*/
  public boolean isChanged() {
    return changed;
  }
  public void setChanged(boolean changed) {
    boolean  oldChanged = this.changed;
    this.changed = changed;
    propertyChangeListeners.firePropertyChange("changed", Boolean.valueOf(oldChanged), Boolean.valueOf(changed));
  }
/*=============================================================================================
/    setIsNew(boolean isNew)
/=============================================================================================*/
   public boolean isNew() {
     return isNew;
   }
   public void setIsNew(boolean isNew) {
     this.isNew = isNew;
   }
/*================================================================================================
/      setAstroObject(AstroObject newCurrentAstroObject)
/=================================================================================================*/
public void setAstroObject(AstroObject newCurrentAstroObject){
    currentAstroObject = newCurrentAstroObject;
    int selectedRow = myAstroObjectTable.getRecordNumber(currentAstroObject);
    selectedRow = myAstroObjectTable.getJTable().convertRowIndexToView(selectedRow);
    this.myAstroObjectTable.getJTable().getSelectionModel().setLeadSelectionIndex(selectedRow);
}
public AstroObject getAstroObject(){
   return currentAstroObject;
}
/*================================================================================================
/      setSelectedRightAscension
/=================================================================================================*/
  public void setSelectedRightAscension(String right_ascension) {
     String  oldright_ascension = this.selected_right_ascension;
    this.selected_right_ascension = right_ascension;
    propertyChangeListeners.firePropertyChange("selected_right_ascension", oldright_ascension, right_ascension);
  }
  public String getSelectedRightAscension() {
    return selected_right_ascension;
  }
/*================================================================================================
/      setSelectedDeclination
/=================================================================================================*/
  public void setSelectedDeclination(String declination) {
     String  oldDeclination = this.selected_declination;
    this.selected_declination = declination;
    propertyChangeListeners.firePropertyChange("selected_declination", oldDeclination, declination);
  }
  public String getSelectedDeclination() {
    return selected_declination;
  }
/*================================================================================================
/      setSelectedRightAscension
/=================================================================================================*/
  public void setSelectedRightAscensionDecimal(String right_ascension) {
     String  oldright_ascension = this.selected_right_ascension_decimal;
    this.selected_right_ascension_decimal = right_ascension;
    propertyChangeListeners.firePropertyChange("selected_right_ascension_decimal", oldright_ascension, right_ascension);
  }
  public String getSelectedRightAscensionDecimal() {
    return selected_right_ascension;
  }
/*================================================================================================
/      setSelectedDeclination
/=================================================================================================*/
  public void setSelectedDeclinationDecimal(String declination) {
     String  oldDeclination = this.selected_declination_decimal;
    this.selected_declination_decimal = declination;
    propertyChangeListeners.firePropertyChange("selected_declination_decimal", oldDeclination, declination);
  }
  public String getSelectedDeclinationDecimal() {
    return selected_declination_decimal;
  }  
/*================================================================================================
/       setSelectedEquinox(String equinox_string)
/=================================================================================================*/
  public void setSelectedEquinox(String equinox_string) {
      // We are expecting one of the following strings "J2000" or "B1950"
    String  oldequinox_string = this.selected_equinox_string;
    this.selected_equinox_string = equinox_string;
    propertyChangeListeners.firePropertyChange("selected_equinox_string", oldequinox_string, equinox_string);
  }
  public String getSelectedEquinox() {
    return selected_equinox_string;
  }
/*================================================================================================
/      setRightAscensionTrackRate
/=================================================================================================*/
  public void setSelectedRightAscensionTrackRate(String right_ascension_track_rate) {
    String  oldright_ascension_track_rate = this.selected_right_ascension_track_rate;
    this.selected_right_ascension_track_rate = right_ascension_track_rate;
    propertyChangeListeners.firePropertyChange("selected_right_ascension_track_rate", oldright_ascension_track_rate, right_ascension_track_rate);
  }
  public String getSelectedRightAscensionTrackRate() {
    return selected_right_ascension_track_rate;
  }
/*================================================================================================
/      setSelectedDeclinationTrackRate
/=================================================================================================*/
  public void setSelectedDeclinationTrackRate(String declination_track_rate) {
    String  olddeclination_track_rate = this.selected_declination_track_rate;
    this.selected_declination_track_rate = declination_track_rate;
    propertyChangeListeners.firePropertyChange("declination_track_rate", olddeclination_track_rate, declination_track_rate);
  }
  public String getSelectedDeclinationTrackRate() {
    return selected_declination_track_rate;
  }
/*================================================================================================
/       setObjectName(String new_objectname_string)
/=================================================================================================*/
  public void setSelectedObjectName(String new_objectname_string) {
    String  old_objectname_string = this.selected_objectname;
    this.selected_objectname = new_objectname_string;
    propertyChangeListeners.firePropertyChange("selected_objectname", old_objectname_string, new_objectname_string);
  }
  public String getSelectedObjectName() {
    return selected_objectname;
  }
/*================================================================================================
/      setRatesUnits(String new_rates_units)
/=================================================================================================*/
  public void setRatesUnits(String new_rates_units) {
    String  old_rates_units = this.rates_units;
    this.rates_units = new_rates_units;
    propertyChangeListeners.firePropertyChange("rates_units", old_rates_units, new_rates_units);
  }
  public String getRatesUnits() {
    return rates_units;
  }
/*================================================================================================
/      setDelimiter(String new_delimiter)
/=================================================================================================*/
  public void setDelimiter(java.lang.String new_delimiter) {
    String  old_delimiter = this.delimiter;
    this.delimiter = new_delimiter;
    propertyChangeListeners.firePropertyChange("delimiter", old_delimiter, new_delimiter);
  }
  public java.lang.String getDelimiter() {
    return delimiter;
  }
 /*================================================================================================
/      setDelimiter(String new_delimiter)
/=================================================================================================*/
  public void setOtherDelimiter(java.lang.String new_other_delimiter) {
    String  old_other_delimiter = this.other_delimiter;
    this.other_delimiter = new_other_delimiter;
    propertyChangeListeners.firePropertyChange("other_delimiter", old_other_delimiter, new_other_delimiter);
  }
  public java.lang.String getOtherDelimiter() {
    return other_delimiter;
  }
/*================================================================================================
/     setFileType(int new_file_type)
/=================================================================================================*/
  public void setFileType(int new_file_type) {
    int  old_file_type = this.file_type;
    this.file_type = new_file_type;
    propertyChangeListeners.firePropertyChange("file_type", Integer.valueOf(old_file_type), Integer.valueOf(new_file_type));
  }
  public int getFileType() {
    return file_type;
  }  
/*================================================================================================
/      setCurrentDirectory(java.lang.String newCurrentDirectory)
/=================================================================================================*/
 public void setCurrentDirectory(java.lang.String newCurrentDirectory){
   currentDirectory = newCurrentDirectory;
  }
 public java.lang.String getCurrentDirectory(){
   return currentDirectory;
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
/    getAstroObjectTable()
/=============================================================================================*/
 public AstroObjectTableModel getAstroObjectTable(){
    return myAstroObjectTable;
 }
/*=============================================================================================
/   ActionEvent Handling Code - used to informing registered listeners that a FITS
/      file is ready to be retrieved from the server
/  removeActionListener()
/  addActionListener()
/  fireActionPerformed()
/=============================================================================================*/
public void fireSelectionChanged(){
   fireActionPerformed(new java.awt.event.ActionEvent(this,SELECTION_CHANGED,"Airmass Plot Selection Changed"));  
}
public void fireObjectListChanged(){
   fireActionPerformed(new java.awt.event.ActionEvent(this,LIST_CHANGED,"AstroObject List Changed"));
}
 /*=============================================================================================
/     synchronized void removeActionListener(ActionListener l)
/=============================================================================================*/
  public synchronized void removeActionListener(ActionListener l) {
    if (actionListeners != null && actionListeners.contains(l)) {
      Vector v = (Vector) actionListeners.clone();
      v.removeElement(l);
      actionListeners = v;
    }
  }
/*=============================================================================================
/    synchronized void addActionListener(ActionListener l)
/=============================================================================================*/
  public synchronized void addActionListener(ActionListener l) {
    Vector v = actionListeners == null ? new Vector(2) : (Vector) actionListeners.clone();
    if (!v.contains(l)) {
      v.addElement(l);
      actionListeners = v;
    }
  }
/*=============================================================================================
/     protected void fireActionPerformed(ActionEvent e)
/=============================================================================================*/
  public void fireActionPerformed(ActionEvent e) {
    if (actionListeners != null) {
      Vector listeners = actionListeners;
      int count = listeners.size();
      for (int i = 0; i < count; i++) {
        ((ActionListener) listeners.elementAt(i)).actionPerformed(e);
      }
    } 
  }
/*=============================================================================================
/    void saveTargetDefinitionFile()
/=============================================================================================*/
  public void saveTargetDefinitionFile(){
    try{
      BufferedWriter bw = null;
      String st;
      FileOutputStream fis = new FileOutputStream(currentFileName);
                       bw  = new BufferedWriter(new OutputStreamWriter(fis));
      int rowCount = myAstroObjectTable.rowcount;
      AstroObject cao = new AstroObject();
      for(int i = 0; i < rowCount; i++){
         cao = myAstroObjectTable.getRecord(i);
         String recordString = cao.name + "," + (Double.valueOf(cao.Alpha.value)).toString() + "," +
                               (Double.valueOf(cao.Delta.value)).toString() + "," + (Double.valueOf(cao.r_m)).toString() +
                               "," + (Double.valueOf(cao.d_m)).toString() + "," + (Boolean.valueOf(cao.m_flag)).toString() + "\n";
         bw.write(recordString);
      }

    }catch(Exception e){
      logMessage("An error occurred while saving the Target Definition File. " + e.toString());
    }
  }
/*=============================================================================================
/    void openTargetDefinitionFile()
/=============================================================================================*/
  public void openTargetDefinitionFile(){
      presenterKey.clear();
      // Clear the table before we start opening the new file
      myAstroObjectTable.clearTable();
      myListModel.clearTable();
      myTargetListDocumentModel.clearDocument();
      if(getFileType() == CSV){
//       readCSVFile();
       readFile(CSV);
      }
      if(getFileType() == PTIC){
 //      readPTICFile();
       readFile(PTIC);
      }
      updateAirmass();
      fireObjectListChanged();
  }
/*=============================================================================================
/   parseText()
/=============================================================================================*/
 public void parseText(int FILE_TYPE){
    java.lang.String CommentString = new java.lang.String();
    java.lang.String current_line   = new java.lang.String();
    java.lang.String text = myTargetListDocumentModel.getText();    
 
    // Clear the table before we start opening the new file    
    presenterKey.clear();
    myAstroObjectTable.clearTable();
    myListModel.clearTable();
    myTargetListDocumentModel.clearDocument();
    java.util.StringTokenizer tokenString = new java.util.StringTokenizer(text,TERMINATOR);  
    while(tokenString.hasMoreElements()){
        try{
           current_line = tokenString.nextToken(); 
           CommentString = "";
           while(current_line.startsWith("!")|current_line.isEmpty()){
               if(current_line.startsWith("!")){
                        CommentString = CommentString + current_line;
                        myTargetListDocumentModel.insertMessage(TargetListDocumentModel.RESPONSE, current_line);
                }
                current_line = tokenString.nextToken(); 
           }
           if(FILE_TYPE == PTIC){
               currentAstroObject = parsePTICRecord(current_line);
           }
           if(FILE_TYPE == CSV){
               currentAstroObject = parseCSVRecord(current_line);
           }                  
           if(currentAstroObject != null){
               myAstroObjectTable.addRecord(currentAstroObject);
               presenterKey.put(currentAstroObject.name, currentAstroObject);
               myListModel.addRecord(currentAstroObject.name);
               myTargetListDocumentModel.insertMessage(TargetListDocumentModel.COMMAND, current_line);                     
            }
            if(currentAstroObject == null){
                myTargetListDocumentModel.insertMessage(TargetListDocumentModel.ERROR, current_line);                      
            } 
        }catch(Exception e){
            System.out.println("Problem parsing the text of the edited file. "+e.toString());
        }    
    }
 } 
/*=============================================================================================
/   writeFile()
/=============================================================================================*/
 public void writeFile(){
         try {
            java.io.PrintWriter  pw = new java.io.PrintWriter(currentFileName);
            pw.write(myTargetListDocumentModel.getText());
            pw.flush();
            pw.close();
          } catch (Exception e){
              System.out.printf("Problem writing to the Target Definition File.\n");
          }   
 } 
/*=============================================================================================
/   AstroObject parsePTICRecord(java.lang.String new_ptic_record)
/=============================================================================================*/
 public void readFile(int FILE_TYPE){
     java.lang.String          CommentString    = new java.lang.String();
     BufferedReader br = null;
//       String st;
       String current_line = new java.lang.String();
        try {
               FileInputStream fis = new FileInputStream(currentFileName);
               br = new BufferedReader(new InputStreamReader(fis));
          } catch (Exception e){
              System.out.printf("Problem opening selected Target Definition File for input.\n");
          }
          try {
             int index = 0;
             while((current_line = br.readLine()) != null) {
              current_line = current_line;
              try{
                   CommentString = "";
                   while(current_line.startsWith("!")|current_line.isEmpty()){
                     if(current_line.startsWith("!")){
                        CommentString = CommentString + current_line;
                        myTargetListDocumentModel.insertMessage(TargetListDocumentModel.RESPONSE, current_line);
                     }
                     current_line = br.readLine();
                   }
                   if(FILE_TYPE == PTIC){
                     currentAstroObject = parsePTICRecord(current_line);
                   }
                   if(FILE_TYPE == CSV){
                    currentAstroObject = parseCSVRecord(current_line);
                   }                  
                   if(currentAstroObject != null){
                     myAstroObjectTable.addRecord(currentAstroObject);
                     presenterKey.put(currentAstroObject.name, currentAstroObject);
                     myListModel.addRecord(currentAstroObject.name);
                     myTargetListDocumentModel.insertMessage(TargetListDocumentModel.COMMAND, current_line);                     
                   }
                   if(currentAstroObject == null){
                       myTargetListDocumentModel.insertMessage(TargetListDocumentModel.ERROR, current_line);                      
                   }                  
                   index++;
              }catch(Exception e2){
               System.out.println("Error parsing coordinates for a target position.");
               }// end of the catch block
             }// end of the while loop
          } catch (IOException e){
              System.out.printf("Problem parsing selected Target Definition File.\n" + e.toString());
          }        
 }  
/*=============================================================================================
/   AstroObject parsePTICRecord(java.lang.String new_ptic_record)
/=============================================================================================*/
 public AstroObject parsePTICRecord(java.lang.String new_ptic_record){
      java.lang.String          ObjectName       = new java.lang.String();
      java.lang.String          RAString         = new java.lang.String();
      java.lang.String          DecString        = new java.lang.String();
      java.lang.String          RARateString     = new java.lang.String();
      java.lang.String          DecRateString    = new java.lang.String();
      java.lang.String          RateFlagString   = new java.lang.String();
      java.lang.String          CommentString    = new java.lang.String();
      java.lang.String          tempString       = new java.lang.String();
      java.lang.String          token1           = new java.lang.String();
      java.lang.String          token2           = new java.lang.String();
      java.lang.String          token3           = new java.lang.String();
      java.lang.String          EquinoxString    = new java.lang.String();
      java.lang.String          current_line     = new java.lang.String();
      AstroObject               currentAstroObject;
     // Clear the document model before starting to read the file
      RA                        RA;
      dec                       Dec;
      double                    RARate  = 0;
      double                    DecRate = 0;
      boolean                   RateFlag = false;
      double                    equinox = 2000;
      if(new_ptic_record.contains("!")){
           java.util.StringTokenizer  st = new java.util.StringTokenizer(new_ptic_record,"!");
           current_line = st.nextToken();
           CommentString = st.nextToken();          
      }
       if(!new_ptic_record.contains("!")){
           current_line = new_ptic_record;          
       }    
       try{
                   ObjectName = current_line.substring(0,16);
                   new_ptic_record         = current_line.substring(16, current_line.length());

                   java.util.StringTokenizer tokenResponseString = new java.util.StringTokenizer(new_ptic_record," ");
                   tempString               = tokenResponseString.nextToken("pm=");
                   while(tempString.startsWith(" ")){
                       tempString = tempString.substring(1,tempString.length());  // basically strip the blank spaces from the beginning of the string before continuing
                   }
                   java.util.StringTokenizer tokenResponseString2 = new java.util.StringTokenizer(tempString," ");
                   token1                 = tokenResponseString2.nextToken(" ");
                   token2                 = tokenResponseString2.nextToken(" ");
                   token3                 = tokenResponseString2.nextToken(" ");
                   RAString               = token1 + ":" + token2 + ":" + token3;

                   token1                 = tokenResponseString2.nextToken(" ");
                   token2                 = tokenResponseString2.nextToken(" ");
                   token3                 = tokenResponseString2.nextToken(" ");
                   DecString              = token1 + ":" + token2 + ":" + token3;

                   EquinoxString          = tokenResponseString2.nextToken();
                   equinox                = (Double.valueOf(EquinoxString).doubleValue());
                   if(!tokenResponseString.hasMoreElements()){
                      RateFlag               = false;
                   }
                   if(tokenResponseString.hasMoreElements()){
                      RARateString           = tokenResponseString.nextToken(",");
                      RARateString           = RARateString.replaceFirst("pm=", "");
                      RARateString           = RARateString.trim();
//                      RARateString           = RARateString.substring(1, RARateString.length());
                      DecRateString          = tokenResponseString.nextToken();
                      if(RARateString.startsWith("+")){
                         RARateString = RARateString.substring(1, RARateString.length());
                      }
                       if(DecRateString.startsWith("+")){
                         DecRateString = DecRateString.substring(1, DecRateString.length());
                      }
                      RARate                 = (Double.valueOf(RARateString)).doubleValue();
                      DecRate                = (Double.valueOf(DecRateString)).doubleValue();
                      RateFlag               = true;
                   }
                   RA                       = (new RA(RAString));
                   Dec                      = (new dec(DecString));
                   RateFlag                 = (Boolean.valueOf(RateFlagString)).booleanValue();
                   currentAstroObject = new AstroObject(ObjectName,RA, Dec, equinox, RARate,DecRate,RateFlag,CommentString);
               }catch(Exception e2){
               System.out.println("Error parsing coordinates from a PTIC file record");
 //              myTargetListDocumentModel.insertMessage(TargetListDocumentModel.ERROR, current_line);
               currentAstroObject = null;
               }// end of the catch block
    return currentAstroObject;
 } 
/*=============================================================================================
/          AstroObject parseCSVRecord(java.lang.String new_ptic_record)
/=============================================================================================*/
public AstroObject parseCSVRecord(java.lang.String new_ptic_record){
      java.lang.String          ObjectName       = new java.lang.String();
      java.lang.String          RAString         = new java.lang.String();
      java.lang.String          DecString        = new java.lang.String();
      java.lang.String          RARateString     = new java.lang.String();
      java.lang.String          DecRateString    = new java.lang.String();
      java.lang.String          RateFlagString   = new java.lang.String();
      java.lang.String          CommentString    = new java.lang.String();
      java.lang.String          tempString       = new java.lang.String();
      java.lang.String          token1           = new java.lang.String();
      java.lang.String          token2           = new java.lang.String();
      java.lang.String          token3           = new java.lang.String();
      java.lang.String          EquinoxString    = new java.lang.String();
      java.lang.String          current_line     = new java.lang.String();
      AstroObject               currentAstroObject;
      java.util.StringTokenizer tokenResponseString;
     // Clear the document model before starting to read the file
      RA                        RA;
      dec                       Dec;
      double                    RARate  = 0;
      double                    DecRate = 0;
      boolean                   RateFlag = false;
      double                    equinox = 2000;
      current_line = new_ptic_record;
       try{
                   tokenResponseString = new java.util.StringTokenizer(new_ptic_record,delimiter);
                   if(delimiter.regionMatches(0, this.TAB, 0, 1)){
                      tokenResponseString = new java.util.StringTokenizer(new_ptic_record,"\t");                       
                   }
                   ObjectName              = tokenResponseString.nextToken();
                   RAString                = tokenResponseString.nextToken();
                   DecString               = tokenResponseString.nextToken();
                   if(!tokenResponseString.hasMoreElements()){
                      equinox              = 2000.0;
                   }
                   if(tokenResponseString.hasMoreElements()){
                     EquinoxString         = tokenResponseString.nextToken();
                     try{
                      equinox               = (Double.valueOf(EquinoxString).doubleValue());
                     }catch (Exception e_noequinox) {
                           CommentString           = EquinoxString;
                     }
                   }
                   if(tokenResponseString.hasMoreElements()){
                      // If there are more elements then read the proper motions, there MUST be PM for both RA and Dec if this is used
                      try{
                          RARateString           = tokenResponseString.nextToken();
    //                      RARateString           = RARateString.substring(3, RARateString.length());
                          DecRateString          = tokenResponseString.nextToken();
                          // Now parse the RARateString to deal with the + at the beginning of the string
                          if(RARateString.startsWith("+")){
                             RARateString = RARateString.substring(1, RARateString.length());
                          }
                           if(DecRateString.startsWith("+")){
                             DecRateString = DecRateString.substring(1, DecRateString.length());
                          }
                          RARate                 = (Double.valueOf(RARateString)).doubleValue();
                          DecRate                = (Double.valueOf(DecRateString)).doubleValue();
                          RateFlag               = true;
                       }catch(Exception e_notnumber){
                          CommentString           = RARateString; 
                       }
                   }
                   RA                       = (new RA(RAString));
                   Dec                      = (new dec(DecString));
                   RateFlag                 = (Boolean.valueOf(RateFlagString)).booleanValue();
                   if(tokenResponseString.hasMoreElements()){
                       CommentString           = tokenResponseString.nextToken();
                   }
                   currentAstroObject = new AstroObject(ObjectName,RA, Dec, equinox, RARate,DecRate,RateFlag,CommentString);
              }catch(Exception e2){
               System.out.println("Error parsing coordinates from the target list file. ");
               currentAstroObject = null;
              }// end of the catch block
     return currentAstroObject;
} 
/*=============================================================================================
/    setFileName(java.lang.String newFileName)
/=============================================================================================*/
  public void setFileName(java.lang.String new_file_name){
    String  old_file_name = this.currentFileName;
    this.currentFileName = new_file_name;
    propertyChangeListeners.firePropertyChange("current_file_name", old_file_name, new_file_name);

  }
  public java.lang.String getFileName(){
    return currentFileName;
  }
/*=============================================================================================
/    setFile(java.io.File newFile)
/=============================================================================================*/
   public void setFile(java.io.File newFile){
      currentFile = newFile;
   }
   public java.io.File getFile(){
       return currentFile;
   }
/*================================================================================================
/          selectTargetDefinitionFile(boolean newIsSAVE_AS)
/=================================================================================================*/
      public boolean selectTargetDefinitionFile(boolean newIsSAVE_AS){
        boolean   isSAVE_AS = newIsSAVE_AS;
        boolean   saveFile = true;
                      JFileChooser myJFileChooser = new JFileChooser(getCurrentDirectory());
//                      myJFileChooser.addChoosableFileFilter(new CSVFileFilter());
                      java.lang.String filePath;
                      int option = myJFileChooser.showDialog(mainFrame,"Select Target Definition File");
                      if (option == JFileChooser.APPROVE_OPTION & (myJFileChooser.getSelectedFile()!=null)) {
                         try{
                            File newFile = myJFileChooser.getSelectedFile();
                            boolean fileExists = newFile.exists();
                               if(fileExists){
                                 if(isSAVE_AS){
                                   int selection = displayFileOverwriteWarning();
                                    if(selection == JOptionPane.YES_OPTION){
                                      filePath = newFile.getPath();
                                      File newTargetFile = new File(filePath);
                                      setFile(newTargetFile);
                                      setFileName(filePath);
                                      saveFile = true;
                                    }
                                    if(selection == JOptionPane.NO_OPTION){
                                      saveFile = false;
                                    }
                                  }// if isSAVE_AS
                                  if(!isSAVE_AS){
                                       filePath = newFile.getPath();
                                       File newTargetFile = new File(filePath);
                                       setFile(newTargetFile);
                                       setFileName(filePath);
                                       setCurrentDirectory(newTargetFile.getAbsolutePath());
                                       saveFile = true;
                                   }// if isSAVE_AS
                               }// end of if FileExists
                               if(!fileExists){
                                        filePath = newFile.getPath();
                                        File newTargetFile = new File(filePath);
                                        setFile(newTargetFile);
                                        setFileName(filePath);
                                        saveFile = true;
                               }// end of if FileExists
                      }catch(Exception e2){
                          System.out.println("A file I/O error occured while locating the Target Definition File. " + e2);
                      }// end of the catch statement
                   }// end of the If statement
                   if (option == JFileChooser.APPROVE_OPTION & (myJFileChooser.getSelectedFile() == null)) {
                      try{
                         File newFile = myJFileChooser.getSelectedFile();
                         boolean fileExists = newFile.exists();
                            if(!fileExists){
                                     filePath = newFile.getPath();
                                     File newTargetFile = new File(filePath);
                                     setFile(newTargetFile);
                                     setFileName(filePath);
                                     saveFile = true;
                            }// end of if FileExists
                   }catch(Exception e2){
                       System.out.println("A file I/O error occured while locating the Target Definition File. " + e2);
                   }// end of the catch statement
                }// end of the If statement
           return saveFile;
        }  // end of the select the Initialization File
/*================================================================================================
/         displayFileOverwriteWarning()
/=================================================================================================*/
    public int displayFileOverwriteWarning(){
       Object[] options = {"Replace File", "Cancel"};
       int n = JOptionPane.showOptionDialog(mainFrame,
                 "The selected file already exists. Do you wish to over-write??","Save Changes?",
                 JOptionPane.YES_NO_CANCEL_OPTION,JOptionPane.QUESTION_MESSAGE, null,options, options[1]);
       return n;
    }
/*================================================================================================
/         checkChangeStatus()
/=================================================================================================*/
        public boolean checkChangeStatus(){
               boolean continueSave = true;
               if(isChanged()){
                 //Custom button text
                 Object[] options = {"Save Changes", "Don't Save Changes", "Cancel"};
                 int n = JOptionPane.showOptionDialog(mainFrame,
                     "Would you like to Save the changes made to the Target Definition File??","Save Changes?",
                     JOptionPane.YES_NO_CANCEL_OPTION,JOptionPane.QUESTION_MESSAGE, null,options, options[2]);
                 if(n == JOptionPane.YES_OPTION){
                   File newFile = new File(getFileName());
                   boolean fileExists = newFile.exists();
                      if(fileExists){
                        saveTargetDefinitionFile();
                        setChanged(false);
                      }// end of if FileExists
                      if(!fileExists){
                          selectTargetDefinitionFile(false);
                          saveTargetDefinitionFile();
                          setChanged(false);
                      }// end of if FileExists
                   continueSave = true;
                 }
                 if(n == JOptionPane.NO_OPTION){
                   logMessage("File NOT saved by User's Choice");
                   continueSave = true;
                 }
                 if(n == JOptionPane.CANCEL_OPTION){
                   logMessage("File saved canceled by User's Choice");
                   continueSave = false;
                 }
                 if(n == JOptionPane.CLOSED_OPTION){
                   continueSave = false;
                 }
               }
         return continueSave;
      }   
/*=============================================================================================
/                      RunSaveTargetDefinitionFile Inner Class
/=============================================================================================*/
      public class RunSaveTargetDefinitionFile implements Runnable{
      private Thread myThread;
      public int  option;
      java.lang.String filePath;
/*=============================================================================================
/             RunSaveTargetDefinitionFile Constructor
/=============================================================================================*/
      public RunSaveTargetDefinitionFile(int newOption){
        option = newOption;
      }
/*================================================================================================
/=================================================================================================*/
      public void run(){
        if(option == SAVE){
         File newFile = new File(getFileName());
        boolean fileExists = newFile.exists();
           if(fileExists){
             saveTargetDefinitionFile();
             setChanged(false);
           }// end of if FileExists
           if(!fileExists){
               selectTargetDefinitionFile(false);
               saveTargetDefinitionFile();
               setChanged(false);
           }// end of if FileExists
        }
        if(option == SAVE_AS){
          boolean mySaveFile = selectTargetDefinitionFile(true);
          if(mySaveFile){
            saveTargetDefinitionFile();
            setChanged(false);
          }
        }
        setIsNew(false);
      }
/*================================================================================================
/=================================================================================================*/
      public void start(){
       myThread = new Thread(this);
       myThread.start();
       }
 }
/*=============================================================================================
/                      RunNewTargetDefinitionFile Inner Class
/=============================================================================================*/
   public class RunNewTargetDefinitionFile implements Runnable{
       private Thread myThread;
       public RunNewTargetDefinitionFile(){
       }
       public void run(){
         boolean continueOpen = checkChangeStatus();
         if(continueOpen){
           java.lang.String templateFileName = new java.lang.String();
           setFileName(templateFileName);
           openTargetDefinitionFile();
           setChanged(true);
           setIsNew(true);
         }
       }
       public void start(){
         myThread = new Thread(this);
         myThread.start();
       }
    }
/*=============================================================================================
/                      RunOpenFile Inner Class
/=============================================================================================*/
   public class RunOpenFile implements Runnable{
     private Thread myThread;
     public RunOpenFile(){
     }
     public void run(){
       boolean continueOpen = checkChangeStatus();
       if(continueOpen){
         selectTargetDefinitionFile(false);
         openTargetDefinitionFile();
       }// End of the Continue Open block
       setIsNew(false);
     }
     public void start(){
       myThread = new Thread(this);
       myThread.start();
     }
  }//// End of the RunSaveFile Inner Class
/*=============================================================================================
/                      CSVFileFilter Inner Class
/=============================================================================================*/
    public class CSVFileFilter extends FileFilter{
      public final static String xml = "ptic";
      public final static String XML = "PTIC";
      public final static String description = "CSV Document Filter";

        public CSVFileFilter(){
        }
        public boolean accept(File f) {
            if (f.isDirectory()) {
                return true;
            }
            java.lang.String extension = getExtension(f);
            if (extension != null) {
                if (extension.equals(xml) ||
                    extension.equals(XML)) {
                        return true;
                } else {
                    return false;
                }
            }
            return false;
        }
        public String getExtension(File f) {
                String ext = null;
                String s = f.getName();
                int i = s.lastIndexOf('.');

                if (i > 0 &&  i < s.length() - 1) {
                    ext = s.substring(i+1).toLowerCase();
                }
                return ext;
        }
        public java.lang.String getDescription(){
           return description;
        }
    }//////////////////////// End of the AORFileFilter Inner Class
/*=============================================================================================
/                           End of the AORFileFilter Inner Class
/=============================================================================================*/  
/*=============================================================================================
/       INNER CLASS that polls the telescope for it's position and status
/=============================================================================================*/
    public class UpdateAirmassThread  implements Runnable{
    private Thread myThread;
    private boolean state      = false;
    private java.lang.String response = new java.lang.String();
/*================================================================================================
/      waitForResponse(int newDelay)
/=================================================================================================*/
public void waitForResponseMilliseconds(int newDelay){
     try{
        Thread.currentThread().sleep(newDelay);} // sleep for awhile to let the messages pile up
      catch (Exception e){
    }
}
/*=============================================================================================
/          setUpdating(boolean newState)
/=============================================================================================*/
public synchronized void setUpdating(boolean newState){
    state = newState;
}
public synchronized boolean isUpdating(){
    return state;
}
/*=============================================================================================
/          UpdateEphemerisThread()
/=============================================================================================*/
    private UpdateAirmassThread(){
    }
/*=============================================================================================
/           run()
/=============================================================================================*/
    public void run(){
       state = true;
       int rows = getAstroObjectTable().getRowCount();
        for(int i = 0;i<rows;i++){
           AstroObject currentAstroObject = (AstroObject)getAstroObjectTable().getRecord(i);   
           myJSkyCalcModel.setTelescopePosition(currentAstroObject.Alpha.RoundedRAString(2, ":"), currentAstroObject.Delta.RoundedDecString(2, ":"),
                                              Printf.format("%04.0f", new PrintfData().add(currentAstroObject.Equinox)));
                if(myJSkyCalcModel.getTimeSource() == JSkyCalcModel.NOW){
                     myJSkyCalcModel.SetToNow();           
                }
                if(myJSkyCalcModel.getTimeSource() == JSkyCalcModel.SELECTED_TIME){
                    java.lang.String[] selected_date_time = myJSkyCalcModel.getSelectedDateTime();
                    myJSkyCalcModel.setToDate(selected_date_time[0], selected_date_time[1]);           
                 }
 //          myJSkyCalcModel.SetToNow();
           ((AstroObject)getAstroObjectTable().getRecord(i)).airmass = myJSkyCalcModel.getAirmass();
        }
           getAstroObjectTable().fireTableDataChanged();
       state = false;
    }
/*=============================================================================================
/         start()
/=============================================================================================*/
    public void start(){
     myThread = new Thread(this);
     myThread.start();
     }
    }// End of the UpdateEphemerisThread Inner Class
/*=============================================================================================
/                           End of the UpdateEphemerisThread Class
/=============================================================================================*/ 
}
