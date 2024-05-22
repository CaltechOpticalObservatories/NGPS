package edu.caltech.palomar.telescopes.P200.gui;
//=== File Prolog =============================================================
//	This code was developed by John Thorstensen of Dartmouth University
//
//      It has been refactored and modified by Jennifer Milburn for
//      use at the Palomar Observatory.
//
//      Caltech Optical Observatories, Palomar Observatory
//
//--- Contents ----------------------------------------------------------------
//    TelescopeControlPanel - JPanel containing all the controls for controlling
//                            the Hale 200" telescope
//
//--- Description -------------------------------------------------------------
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
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
import edu.caltech.palomar.telescopes.P200.TelescopeObject;
import edu.caltech.palomar.telescopes.P200.P200Component;
import java.beans.*;
import java.awt.*;
import javax.swing.*;
/*=============================================================================================
/     TelescopeControlPanel Class Declaration
/=============================================================================================*/
public class TelescopeControlPanel extends javax.swing.JFrame {
    /** Creates new form TelescopeControlPanel */
     TelescopeObject myTelescopeObject;
     P200Component   myP200Component;
     ImageIcon       LeftRed;
     ImageIcon       RightRed;
     ImageIcon       UpRed;
     ImageIcon       DownRed;
     ImageIcon       LeftGrey;
     ImageIcon       RightGrey;
     ImageIcon       UpGrey;
     ImageIcon       DownGrey;
     ImageIcon       LeftGreen;
     ImageIcon       RightGreen;
     ImageIcon       UpGreen;
     ImageIcon       DownGreen;
     ImageIcon       FOCUSLeftRed;
     ImageIcon       FOCUSRightRed;
     ImageIcon       FOCUSLeftGreen;
     ImageIcon       FOCUSRightGreen;
     ImageIcon       FOCUSLeftGrey;
     ImageIcon       FOCUSRightGrey;
     ImageIcon       ArmAbort;
     ImageIcon       DisarmAbort;
     double offset;
     double RAoffset;
     double Decoffset;
     double RAMoveRate;
     double DecMoveRate;
     double RATrackRate;
     double DecTrackRate;
     double FocusOffset;
     double Focus;
     public double MIN_OFFSET         = 0.0;
     public double MAX_OFFSET         = 6000.0;
     public double MIN_RA_OFFSET      = -6000.0;
     public double MAX_RA_OFFSET      = 6000.0;
     public double MIN_DEC_OFFSET     = -6000.0;
     public double MAX_DEC_OFFSET     = 6000.0;
     public double MIN_RA_MOVE_RATE   = 0;
     public double MAX_RA_MOVE_RATE   = 50.0;
     public double MIN_DEC_MOVE_RATE  = 0.0;
     public double MAX_DEC_MOVE_RATE  = 50.0;

     public double MIN_DEC_TRACK_RATE  = -100000.0;
     public double MAX_DEC_TRACK_RATE  = 100000.0;
     public double MIN_RA_TRACK_RATE   = -200000.0;
     public double MAX_RA_TRACK_RATE   = 200000.0;

     public double MIN_FOCUS           = 1.00;
     public double MAX_FOCUS           = 74.00;
     public double MIN_FOCUS_OFFSET    = 0.0;
     public double MAX_FOCUS_OFFSET    = 36.5;

     private  java.lang.String      TERMINATOR        = new java.lang.String("\n");
     public   java.lang.String      USERDIR           = System.getProperty("user.dir");
     public   java.lang.String      SEP               = System.getProperty("file.separator");
     public   java.lang.String      IMAGE_CACHE       = new java.lang.String(SEP + "images" + SEP);
     public   java.lang.String      CONFIG            = new java.lang.String(SEP + "config" + SEP);
     private  double currentDouble = 0.0;
     private  String currentString = new String();
     private  BoundedRangeModel boundedRangemodel;
/*=============================================================================================
/      TelescopeControlPanel()   constructor
/=============================================================================================*/
    public TelescopeControlPanel() {
        initComponents();
        initializeIcons();
        initializeButtons();
        disabledState();
        configureStates(false);
        configureComponents();
        this.setSize(500, 870);
        this.setDefaultCloseOperation(JFrame.HIDE_ON_CLOSE);
    }
 /*================================================================================================
 /       setInitialValues()
 /=================================================================================================*/
  private void initializeButtons(){
            MoveTelescopeUpButton.setDisabledIcon(UpGrey);
            MoveTelescopeDownButton.setDisabledIcon(DownGrey);
            MoveTelescopeLeftButton.setDisabledIcon(LeftGrey);
            MoveTelescopeRightButton.setDisabledIcon(RightGrey);
            FocusUpButton.setDisabledIcon(FOCUSRightGrey);
            FocusDownButton.setDisabledIcon(FOCUSLeftGrey);
            MoveTelescopeUpButton.setIcon(UpGrey);
            MoveTelescopeDownButton.setIcon(DownGrey);
            MoveTelescopeLeftButton.setIcon(LeftGrey);
            MoveTelescopeRightButton.setIcon(RightGrey);
            FocusUpButton.setIcon(FOCUSRightGrey);
            FocusDownButton.setIcon(FOCUSLeftGrey);
            AbortButton.setIcon(DisarmAbort);
  }
/*================================================================================================
/       setInitialValues()
/=================================================================================================*/
  public void setInitialValues(){
      String param1 = myP200Component.myTelescopesIniReader.DEFAULT_OFFSET;
      String param2 = myP200Component.myTelescopesIniReader.DEFAULT_RA_OFFSET;
      String param3 = myP200Component.myTelescopesIniReader.DEFAULT__DEC_OFFSET;
      String param4 = myP200Component.myTelescopesIniReader.DEFAULT__RA_MOVE_RATE;
      String param5 = myP200Component.myTelescopesIniReader.DEFAULT__DEC_MOVE_RATE;
      String param6 = myP200Component.myTelescopesIniReader.DEFAULT__FOCUS_OFFSET;
      OffsetTextField.setText(param1);
      RAOffsetTextField.setText(param2);
      DecOffsetTextField.setText(param3);
      RAMoveRateTextField.setText(param4);
      DecMoveRateTextField.setText(param5);
      OffsetFocusTextField.setText(param6);
      setOffset((Double.valueOf(myP200Component.myTelescopesIniReader.DEFAULT_OFFSET)).doubleValue());
      setRAOffset((Double.valueOf(myP200Component.myTelescopesIniReader.DEFAULT_RA_OFFSET)).doubleValue());
      setDecOffset((Double.valueOf(myP200Component.myTelescopesIniReader.DEFAULT__DEC_OFFSET)).doubleValue());
      setRAMoveRate((Double.valueOf(myP200Component.myTelescopesIniReader.DEFAULT__RA_MOVE_RATE)).doubleValue());
      setDecMoveRate((Double.valueOf(myP200Component.myTelescopesIniReader.DEFAULT__DEC_MOVE_RATE)).doubleValue());
      setFocusOffset((Double.valueOf(myP200Component.myTelescopesIniReader.DEFAULT__FOCUS_OFFSET)).doubleValue());
      myTelescopeObject.TELESCOPE_SETTLE_TIME = (Double.valueOf(myP200Component.myTelescopesIniReader.TELESCOPE_SETTLE_TIME)).doubleValue();
      myTelescopeObject.FOCUS_MOVE_RATE       = (Double.valueOf(myP200Component.myTelescopesIniReader.FOCUS_MOVE_RATE)).doubleValue();        
  }
/*================================================================================================
/        initializeIcons()
/=================================================================================================*/
  public void initializeIcons(){
 //   LeftRed              = new ImageIcon(USERDIR + IMAGE_CACHE + "ControlArrows128"+SEP+"RedArrows_128_RIGHT.gif");
 //   RightRed             = new ImageIcon(USERDIR + IMAGE_CACHE + "ControlArrows128"+SEP+"RedArrows_128_LEFT.gif");
 //   UpRed                = new ImageIcon(USERDIR + IMAGE_CACHE + "ControlArrows128"+SEP+"RedArrows_128_UP.gif");
 //   DownRed              = new ImageIcon(USERDIR + IMAGE_CACHE + "ControlArrows128"+SEP+"RedArrows_128_DOWN.gif");
 //   LeftGrey             = new ImageIcon(USERDIR + IMAGE_CACHE + "ControlArrows128"+SEP+"GreyArrows_128_RIGHT.gif");
 //   RightGrey            = new ImageIcon(USERDIR + IMAGE_CACHE + "ControlArrows128"+SEP+"GreyArrows_128_LEFT.gif");
 //   UpGrey               = new ImageIcon(USERDIR + IMAGE_CACHE + "ControlArrows128"+SEP+"GreyArrows_128_UP.gif");
 //   DownGrey             = new ImageIcon(USERDIR + IMAGE_CACHE + "ControlArrows128"+SEP+"GreyArrows_128_DOWN.gif");
 //   RightGreen           = new ImageIcon(USERDIR + IMAGE_CACHE + "ControlArrows128"+SEP+"GreenArrows_128_LEFT.gif");
 //   LeftGreen            = new ImageIcon(USERDIR + IMAGE_CACHE + "ControlArrows128"+SEP+"GreenArrows_128_RIGHT.gif");
 //   UpGreen              = new ImageIcon(USERDIR + IMAGE_CACHE + "ControlArrows128"+SEP+"GreenArrows_128_UP.gif");
 //   DownGreen            = new ImageIcon(USERDIR + IMAGE_CACHE + "ControlArrows128"+SEP+"GreenArrows_128_DOWN.gif");

    LeftRed              = new ImageIcon(USERDIR + IMAGE_CACHE + "RedArrows_LEFT.gif");
    RightRed             = new ImageIcon(USERDIR + IMAGE_CACHE + "RedArrows_RIGHT.gif");
    UpRed                = new ImageIcon(USERDIR + IMAGE_CACHE + "RedArrows_UP.gif");
    DownRed              = new ImageIcon(USERDIR + IMAGE_CACHE + "RedArrows_DOWN.gif");
    LeftGrey             = new ImageIcon(USERDIR + IMAGE_CACHE + "GreyArrows_LEFT.gif");
    RightGrey            = new ImageIcon(USERDIR + IMAGE_CACHE + "GreyArrows_RIGHT.gif");
    UpGrey               = new ImageIcon(USERDIR + IMAGE_CACHE + "GreyArrows_UP.gif");
    DownGrey             = new ImageIcon(USERDIR + IMAGE_CACHE + "GreyArrows_DOWN.gif");
    LeftGreen            = new ImageIcon(USERDIR + IMAGE_CACHE + "GreenArrows_LEFT.gif");
    RightGreen           = new ImageIcon(USERDIR + IMAGE_CACHE + "GreenArrows_RIGHT.gif");
    UpGreen              = new ImageIcon(USERDIR + IMAGE_CACHE + "GreenArrows_UP.gif");
    DownGreen            = new ImageIcon(USERDIR + IMAGE_CACHE + "GreenArrows_DOWN.gif");
    FOCUSLeftGrey        = new ImageIcon(USERDIR + IMAGE_CACHE + "FocusArrows_LeftGrey.gif");
    FOCUSRightGrey       = new ImageIcon(USERDIR + IMAGE_CACHE + "FocusArrows_RightGrey.gif");
    FOCUSLeftGreen       = new ImageIcon(USERDIR + IMAGE_CACHE + "FocusArrows_LeftGreen.gif");
    FOCUSRightGreen      = new ImageIcon(USERDIR + IMAGE_CACHE + "FocusArrows_RightGreen.gif");
    FOCUSLeftRed         = new ImageIcon(USERDIR + IMAGE_CACHE + "FocusArrows_LeftRed.gif");
    FOCUSRightRed        = new ImageIcon(USERDIR + IMAGE_CACHE + "FocusArrows_RightRed.gif");
    ArmAbort             = new ImageIcon(USERDIR + IMAGE_CACHE + "AbortAutofocus.gif");
    DisarmAbort          = new ImageIcon(USERDIR + IMAGE_CACHE + "AbortAutofocusDisabled.gif");
  }
/*================================================================================================
/         initializeForm(P200Component newP200Component)
/=================================================================================================*/
  private void configureComponents(){
    FocusDownButton.setFocusPainted(false);
    FocusUpButton.setFocusPainted(false);
    MoveTelescopeDownButton.setFocusPainted(false);
    MoveTelescopeLeftButton.setFocusPainted(false);
    MoveTelescopeRightButton.setFocusPainted(false);
    MoveTelescopeUpButton.setFocusPainted(false);
  }
/*================================================================================================
/         initializeForm(P200Component newP200Component)
/=================================================================================================*/
   public void initializeForm(P200Component newP200Component){
        this.myP200Component =  newP200Component;
        setTelescopeObject(myP200Component.getTelescopeObject());
        myP200Component.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
        public void propertyChange(java.beans.PropertyChangeEvent e) {
            P200Component_propertyChange(e);
         }
        });
   }
/*================================================================================================
/     setTelescopeObject(TelescopeObject newTelescopeObject)
/=================================================================================================*/
 public void setTelescopeObject(TelescopeObject newTelescopeObject){
     myTelescopeObject = newTelescopeObject;
     setInitialValues();
     myTelescopeObject.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
      public void propertyChange(java.beans.PropertyChangeEvent e) {
          TelescopeObject_propertyChange(e);
       }
     });
    }
 public void initializeStates(){
      ConnectTelescopeToggleButton.setEnabled(true);
 }
/*=============================================================================================
/     Property Change Listener for the _propertyChange
/=============================================================================================*/
  private void P200Component_propertyChange(PropertyChangeEvent e)  {
     java.lang.String propertyName = e.getPropertyName();
       if(propertyName == "control_connected"){
        boolean state = (java.lang.Boolean)e.getNewValue();
        configureStates(state);
        if(state){
          ConnectTelescopeToggleButton.setText("Connected to Telescope");
          ConnectTelescopeToggleButton.setSelected(true);
          myTelescopeObject.setTelescopeMotionDirection(TelescopeObject.NOT_MOVING);
          FocusUpButton.setIcon(FOCUSRightGreen);
          FocusDownButton.setIcon(FOCUSLeftGreen);
          FocusTextField.setText(myTelescopeObject.getTelescopeFocus());
          setFocus((Double.valueOf(myTelescopeObject.getTelescopeFocus())).doubleValue());
        }
        if(!state){
          ConnectTelescopeToggleButton.setText("Connect to Telescope");
          ConnectTelescopeToggleButton.setSelected(false);
          myTelescopeObject.setTelescopeMotionDirection(TelescopeObject.NOT_MOVING);
            MoveTelescopeUpButton.setDisabledIcon(UpGrey);
            MoveTelescopeDownButton.setDisabledIcon(DownGrey);
            MoveTelescopeLeftButton.setDisabledIcon(LeftGrey);
            MoveTelescopeRightButton.setDisabledIcon(RightGrey);
            FocusUpButton.setDisabledIcon(FOCUSRightGrey);
            FocusDownButton.setDisabledIcon(FOCUSLeftGrey);
        }
      }
  }
/*=============================================================================================
/     armAbort()
/=============================================================================================*/
public void armAbort(){
   AbortButton.setSelected(false);
   AbortButton.setEnabled(true);
   AbortButton.setIcon(ArmAbort);
}
/*=============================================================================================
/     disarmAbort()
/=============================================================================================*/
public void disarmAbort(){
   AbortButton.setSelected(false);
   AbortButton.setEnabled(false);
   AbortButton.setIcon(DisarmAbort);
}
/*=============================================================================================
/     Property Change Listener for the _propertyChange
/=============================================================================================*/
  private void TelescopeObject_propertyChange(PropertyChangeEvent e)  {
     java.lang.String propertyName = e.getPropertyName();

     if(propertyName == "abortArmed"){
        java.lang.Boolean newValue = (java.lang.Boolean)e.getNewValue();
        if(newValue.booleanValue()){
           AbortButton.setSelected(false);
           AbortButton.setEnabled(true);
           AbortButton.setIcon(ArmAbort);
        }
        if(!newValue.booleanValue()){
           AbortButton.setSelected(false);
           AbortButton.setEnabled(false);
           AbortButton.setIcon(DisarmAbort);
        }
    }
     if(propertyName == "right_ascension_track_rate"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        RATrackRateDisplayTextField.setText(newValue);
    }
    if(propertyName == "declination_track_rate"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        RATrackRateDisplayTextField.setText(newValue);
    }
    if(propertyName == "telescope_focus"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        FocusDisplayTextField.setText(newValue);
    }
    if(propertyName == "cass_ring_angle"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "percentCompleted"){
        java.lang.Double newValue = (java.lang.Double)e.getNewValue();
        TelescopeMoveProgressBar.setValue(newValue.intValue());
    }
    if(propertyName == "statusMessage"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        StatusMessageLabel.setText(newValue);
    }
    if(propertyName == "RAMoveRate" ){
        java.lang.Double newValue = (java.lang.Double)e.getNewValue();
        RAMoveRateDisplayTextField.setText(newValue.toString());
    }
    if(propertyName == "DecMoveRate" ){
        java.lang.Double newValue = (java.lang.Double)e.getNewValue();
        DecMoveRateDisplayTextField.setText(newValue.toString());
    }
     if(propertyName == "focusMotion"){
        java.lang.Integer newValue = (java.lang.Integer)e.getNewValue();
        int motionCode = newValue.intValue();     
        if(motionCode == TelescopeObject.FOCUS_NOT_MOVING){
             if(myP200Component.isControlConnected()){
                configureStates(true);
                FocusUpButton.setIcon(FOCUSRightGreen);
                FocusDownButton.setIcon(FOCUSLeftGreen);            
             }
             if(!myP200Component.isControlConnected()){
                configureStates(false);
                FocusUpButton.setDisabledIcon(FOCUSRightGrey);
                FocusDownButton.setDisabledIcon(FOCUSLeftGrey);            
                FocusUpButton.setIcon(FOCUSRightGrey);
                FocusDownButton.setIcon(FOCUSLeftGrey);
             }
        }
        if(motionCode == TelescopeObject.FOCUS_MOVING_UP){
            FocusUpButton.setDisabledIcon(FOCUSRightRed);
            FocusDownButton.setDisabledIcon(FOCUSLeftGrey);            
            FocusUpButton.setIcon(FOCUSRightRed);
            FocusDownButton.setIcon(FOCUSLeftGrey);
        }
        if(motionCode == TelescopeObject.FOCUS_MOVING_DOWN){
            FocusUpButton.setDisabledIcon(FOCUSRightGrey);
            FocusDownButton.setDisabledIcon(FOCUSLeftRed);           
            FocusUpButton.setIcon(FOCUSRightGrey);
            FocusDownButton.setIcon(FOCUSLeftRed);
        }
     }
     if(propertyName == "telescopeMotion"){
        java.lang.Integer newValue = (java.lang.Integer)e.getNewValue();
        int motionCode = newValue.intValue();
// Change the icons on the telescope offset to reflect the direction of motion
//    public static int NOT_MOVING = 0;
//    public static int MOVING_N = 1;
//    public static int MOVING_E = 2;
//    public static int MOVING_S = 3;
//    public static int MOVING_W = 4;
//    public static int MOVING_NE = 10;
//    public static int MOVING_SE = 11;
//    public static int MOVING_SW = 12;
//    public static int MOVING_NW = 13;
        switch(motionCode){
          case 0 : {
             if(myP200Component.isControlConnected()){
                configureStates(true);
                myP200Component.getTelescopeObject().setAbortArmed(false);
                MoveTelescopeUpButton.setIcon(UpGreen);
                MoveTelescopeDownButton.setIcon(DownGreen);
                MoveTelescopeLeftButton.setIcon(LeftGreen);
                MoveTelescopeRightButton.setIcon(RightGreen);
             }
             if(!myP200Component.isControlConnected()){
                configureStates(false);
                disabledState();
                disarmAbort();
           }
          break;}
          case 1 : {     // Telescope Moving North
             if(myP200Component.isControlConnected()){
                configureStates(false);
                myP200Component.getTelescopeObject().setAbortArmed(true);
                MoveTelescopeUpButton.setDisabledIcon(UpRed);
                MoveTelescopeDownButton.setDisabledIcon(DownGrey);
                MoveTelescopeLeftButton.setDisabledIcon(LeftGrey);
                MoveTelescopeRightButton.setDisabledIcon(RightGrey);
                FocusUpButton.setDisabledIcon(FOCUSRightGrey);
                FocusDownButton.setDisabledIcon(FOCUSLeftGrey);
             }
             if(!myP200Component.isControlConnected()){
                disabledState();
                disarmAbort();
             }
          break;}
          case 2 : {     // Telescope Moving East
             if(myP200Component.isControlConnected()){
                configureStates(false);
                myP200Component.getTelescopeObject().setAbortArmed(true);
                MoveTelescopeUpButton.setDisabledIcon(UpGrey);
                MoveTelescopeDownButton.setDisabledIcon(DownGrey);
                MoveTelescopeLeftButton.setDisabledIcon(LeftGrey);
                MoveTelescopeRightButton.setDisabledIcon(RightRed);
                FocusUpButton.setDisabledIcon(FOCUSRightGrey);
                FocusDownButton.setDisabledIcon(FOCUSLeftGrey);
             }
             if(!myP200Component.isControlConnected()){
                disabledState();
                disarmAbort();
             }
          break;}
          case 3 : {     // Telescope Moving South
             if(myP200Component.isControlConnected()){
                configureStates(false);
                myP200Component.getTelescopeObject().setAbortArmed(true);
                MoveTelescopeUpButton.setDisabledIcon(UpGrey);
                MoveTelescopeDownButton.setDisabledIcon(DownRed);
                MoveTelescopeLeftButton.setDisabledIcon(LeftGrey);
                MoveTelescopeRightButton.setDisabledIcon(RightGrey);
                FocusUpButton.setDisabledIcon(FOCUSRightGrey);
                FocusDownButton.setDisabledIcon(FOCUSLeftGrey);
             }
             if(!myP200Component.isControlConnected()){
                disabledState();
                disarmAbort();
              }
          break;}
          case 4 : {     // Telescope Moving West
             if(myP200Component.isControlConnected()){
                configureStates(false);
                myP200Component.getTelescopeObject().setAbortArmed(true);
                MoveTelescopeUpButton.setDisabledIcon(UpGrey);
                MoveTelescopeDownButton.setDisabledIcon(DownGrey);
                MoveTelescopeLeftButton.setDisabledIcon(LeftRed);
                MoveTelescopeRightButton.setDisabledIcon(RightGrey);
                FocusUpButton.setDisabledIcon(FOCUSRightGrey);
                FocusDownButton.setDisabledIcon(FOCUSLeftGrey);
             }
             if(!myP200Component.isControlConnected()){
                disabledState();
                disarmAbort();
              }
          break;}
          case 10 : {     // Telescope Moving West
             if(myP200Component.isControlConnected()){
                configureStates(false);
                myP200Component.getTelescopeObject().setAbortArmed(true);
                MoveTelescopeUpButton.setDisabledIcon(UpRed);
                MoveTelescopeDownButton.setDisabledIcon(DownGrey);
                MoveTelescopeLeftButton.setDisabledIcon(LeftGrey);
                MoveTelescopeRightButton.setDisabledIcon(RightRed);
                 FocusUpButton.setDisabledIcon(FOCUSRightGrey);
                FocusDownButton.setDisabledIcon(FOCUSLeftGrey);
            }
             if(!myP200Component.isControlConnected()){
                disabledState();
                disarmAbort();
              }
          break;}
          case 11 : {     // Telescope Moving West
             if(myP200Component.isControlConnected()){
                configureStates(false);
                myP200Component.getTelescopeObject().setAbortArmed(true);
                MoveTelescopeUpButton.setDisabledIcon(UpGrey);
                MoveTelescopeDownButton.setDisabledIcon(DownRed);
                MoveTelescopeLeftButton.setDisabledIcon(LeftGrey);
                MoveTelescopeRightButton.setDisabledIcon(RightRed);
                FocusUpButton.setDisabledIcon(FOCUSRightGrey);
                FocusDownButton.setDisabledIcon(FOCUSLeftGrey);
             }
             if(!myP200Component.isControlConnected()){
                disabledState();
                disarmAbort();
               }
          break;}
          case 12 : {     // Telescope Moving West
             if(myP200Component.isControlConnected()){
                configureStates(false);
                myP200Component.getTelescopeObject().setAbortArmed(true);
                MoveTelescopeUpButton.setDisabledIcon(UpGrey);
                MoveTelescopeDownButton.setDisabledIcon(DownRed);
                MoveTelescopeLeftButton.setDisabledIcon(LeftRed);
                MoveTelescopeRightButton.setDisabledIcon(RightGrey);
                FocusUpButton.setDisabledIcon(FOCUSRightGrey);
                FocusDownButton.setDisabledIcon(FOCUSLeftGrey);
             }
             if(!myP200Component.isControlConnected()){
                disabledState();
                disarmAbort();
               }
          break;}
          case 13 : {     // Telescope Moving West
             if(myP200Component.isControlConnected()){
                configureStates(false);
                myP200Component.getTelescopeObject().setAbortArmed(true);
                MoveTelescopeUpButton.setDisabledIcon(UpRed);
                MoveTelescopeDownButton.setDisabledIcon(DownGrey);
                MoveTelescopeLeftButton.setDisabledIcon(LeftRed);
                MoveTelescopeRightButton.setDisabledIcon(RightGrey);
                FocusUpButton.setDisabledIcon(FOCUSRightGrey);
                FocusDownButton.setDisabledIcon(FOCUSLeftGrey);           
             }
             if(!myP200Component.isControlConnected()){
                disabledState();
                disarmAbort();
             }
          break;}
        }
    }
  }
private void disabledState(){
  MoveTelescopeUpButton.setDisabledIcon(UpGrey);
  MoveTelescopeDownButton.setDisabledIcon(DownGrey);
  MoveTelescopeLeftButton.setDisabledIcon(LeftGrey);
  MoveTelescopeRightButton.setDisabledIcon(RightGrey);
  FocusUpButton.setDisabledIcon(FOCUSRightGrey);
  FocusDownButton.setDisabledIcon(FOCUSLeftGrey);  
}

/*=============================================================================================
/       configureStates(boolean state)
/=============================================================================================*/
public void configureStates(boolean state){
   AbortButton.setFocusCycleRoot(state);
   MoveTelescopeUpButton.setEnabled(state);
   MoveTelescopeDownButton.setEnabled(state);
   MoveTelescopeLeftButton.setEnabled(state);
   MoveTelescopeRightButton.setEnabled(state);
   OffsetTelescopeButton.setEnabled(state);
   SetMoveRateButton.setEnabled(state);
   SetTrackRateButton.setEnabled(state);                                    
   SetFocusButton.setEnabled(state);                                                 
   FocusDownButton.setEnabled(state);                                             
   FocusUpButton.setEnabled(state);
   ReturnToGOButton.setEnabled(state);
   ZButton.setEnabled(state);
}
/*=============================================================================================
/       setFocusOffset(double newOffset)
/=============================================================================================*/
 public void setFocusOffset(double newFocusOffset){
     FocusOffset = newFocusOffset;
 }
/*=============================================================================================
/       getFocusOffset()
/=============================================================================================*/
 public double getFocusOffset(){
     return FocusOffset;
 } 
/*=============================================================================================
/       setFocusOffset(double newOffset)
/=============================================================================================*/
 public void setFocus(double newFocus){
     Focus = newFocus;
 }
/*=============================================================================================
/       getFocusOffset()
/=============================================================================================*/
 public double getFocus(){
     return Focus;
 }
/*=============================================================================================
/       setOffset(double newOffset)
/=============================================================================================*/
 public void setOffset(double newOffset){
     offset = newOffset;
 }
/*=============================================================================================
/       getOffset()
/=============================================================================================*/
 public double getOffset(){
     return offset;
 } 
/*=============================================================================================
/       setRAOffset(double newOffset)
/=============================================================================================*/
 public void setRAOffset(double newRAOffset){
     RAoffset = newRAOffset;
 } 
/*=============================================================================================
/       getOffset()
/=============================================================================================*/
 public double getRAOffset(){
     return RAoffset;
 }  
/*=============================================================================================
/       setDecOffset(double newOffset)
/=============================================================================================*/
 public void setDecOffset(double newDecOffset){
     Decoffset = newDecOffset;
 }  
/*=============================================================================================
/       setDecOffset(double newOffset)
/=============================================================================================*/
 public double getDecOffset(){
     return Decoffset;
 }  
/*=============================================================================================
/       setRAOffset(double newOffset)
/=============================================================================================*/
 public void setRAMoveRate(double newRAMoveRate){
     RAMoveRate = newRAMoveRate;
     myTelescopeObject.setRAMoveRateManual(RAMoveRate);
 } 
/*=============================================================================================
/       getRAMoveRate()
/=============================================================================================*/
 public double getRAMoveRate(){
     return RAMoveRate;
 }  
/*=============================================================================================
/       setDecMoveRate(double newDecMoveRate)
/=============================================================================================*/
 public void setDecMoveRate(double newDecMoveRate){
     DecMoveRate = newDecMoveRate;
     myTelescopeObject.setDecMoveRateManual(DecMoveRate);
 } 
/*=============================================================================================
/       getDecMoveRate()
/=============================================================================================*/
 public double getDecMoveRate(){
     return DecMoveRate;
 } 
/*=============================================================================================
/       setRATrackRate(double newRATrackRate)
/=============================================================================================*/
 public void setRATrackRate(double newRATrackRate){
     RATrackRate = newRATrackRate;
 } 
/*=============================================================================================
/       getRATrackRate()
/=============================================================================================*/
 public double getRATrackRate(){
     return RATrackRate;
 } 
/*=============================================================================================
/       setDecTrackRate(double newOffset)
/=============================================================================================*/
 public void setDecTrackRate(double newDecTrackRate){
     DecTrackRate = newDecTrackRate;
 } 
/*=============================================================================================
/       getDecTrackRate()
/=============================================================================================*/
 public double getDecTrackRate(){
     return DecTrackRate;
 }
  /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
 
/*=============================================================================================
/       OffsetTextFieldAction(java.awt.event.ActionEvent evt)
/=============================================================================================*/
 public void OffsetTextFieldAction(){
     try{
           currentString = OffsetTextField.getText();
           currentDouble = (Double.valueOf(currentString)).doubleValue();
           if((currentDouble < MIN_OFFSET)| (currentDouble > MAX_OFFSET)){
              currentString = "0.0";
              currentDouble = 0.0;
           }
        } catch (Exception e) {
           currentString = "0.0";
           currentDouble = 0.0;
        }
        setOffset(currentDouble);
    }

/*=============================================================================================
/       OffsetTextFieldActionPerformedMethod(java.awt.event.ActionEvent evt)
/=============================================================================================*/
public void OffsetTextFieldActionPerformedMethod(java.awt.event.ActionEvent evt){
     OffsetTextFieldAction();
}
/*=============================================================================================
/       RAOffsetTextFieldActionPerformedMethod(java.awt.event.ActionEvent evt)
/=============================================================================================*/
public void RAOffsetTextFieldActionPerformedMethod(java.awt.event.ActionEvent evt){
        try{
           currentString = RAOffsetTextField.getText();
           currentDouble = (Double.valueOf(currentString)).doubleValue();
           if((currentDouble < MIN_RA_OFFSET)| (currentDouble > MAX_RA_OFFSET)){
              currentString = "0.0";
              currentDouble = 0.0;          
           }
        } catch (Exception e) {
           currentString = "0.0";
           currentDouble = 0.0;
        }
        setRAOffset(currentDouble);    
}
/*=============================================================================================
/       RAOffsetTextFieldActionPerformedMethod(java.awt.event.ActionEvent evt)
/=============================================================================================*/
public void DecOffsetTextFieldActionPerformedMethod(java.awt.event.ActionEvent evt){
        try{
           currentString = DecOffsetTextField.getText();
           currentDouble = (Double.valueOf(currentString)).doubleValue();
           if((currentDouble < MIN_DEC_OFFSET)| (currentDouble > MAX_DEC_OFFSET)){
              currentString = "0.0";
              currentDouble = 0.0;
           }
        } catch (Exception e) {
           currentString = "0.0";
           currentDouble = 0.0;
        }
        setDecOffset(currentDouble);
}

/*=============================================================================================
/       RAMoveRateTextFieldActionPerformedMethod(java.awt.event.ActionEvent evt)
/=============================================================================================*/
public void RAMoveRateTextFieldActionPerformedMethod(java.awt.event.ActionEvent evt){
        try{
           currentString = RAMoveRateTextField.getText();
           currentDouble = (Double.valueOf(currentString)).doubleValue();
           if((currentDouble < MIN_RA_MOVE_RATE)| (currentDouble > MAX_RA_MOVE_RATE)){
              currentString = "0.0";
              currentDouble = 0.0;
           }
        } catch (Exception e) {
           currentString = "0.0";
           currentDouble = 0.0;
        }
        setRAMoveRate(currentDouble);
}
/*=============================================================================================
/       DecMoveRateTextFieldActionPerformedMethod(java.awt.event.ActionEvent evt)
/=============================================================================================*/
public void DecMoveRateTextFieldActionPerformedMethod(java.awt.event.ActionEvent evt){
        try{
           currentString = DecMoveRateTextField.getText();
           currentDouble = (Double.valueOf(currentString)).doubleValue();
           if((currentDouble < MIN_DEC_MOVE_RATE)| (currentDouble > MAX_DEC_MOVE_RATE)){
              currentString = "0.0";
              currentDouble = 0.0;
           }
        } catch (Exception e) {
           currentString = "0.0";
           currentDouble = 0.0;
        }
        setDecMoveRate(currentDouble);
}
/*=============================================================================================
/       RATrackRateTextFieldActionPerformedMethod(java.awt.event.ActionEvent evt)
/=============================================================================================*/
public void RATrackRateTextFieldActionPerformedMethod(java.awt.event.ActionEvent evt){
        try{
           currentString = RATrackRateTextField.getText();
           currentDouble = (Double.valueOf(currentString)).doubleValue();
           if((currentDouble < MIN_RA_TRACK_RATE)| (currentDouble > MAX_RA_TRACK_RATE)){
              currentString = "0.0";
              currentDouble = 0.0;
           }
        } catch (Exception e) {
           currentString = "0.0";
           currentDouble = 0.0;
        }
        setRATrackRate(currentDouble);
}
/*=============================================================================================
/       DecTrackRateTextFieldActionPerformedMethod(java.awt.event.ActionEvent evt)
/=============================================================================================*/
public void DecTrackRateTextFieldActionPerformedMethod(java.awt.event.ActionEvent evt){
        try{
           currentString = DecTrackRateTextField.getText();
           currentDouble = (Double.valueOf(currentString)).doubleValue();
           if((currentDouble < MIN_DEC_TRACK_RATE)| (currentDouble > MAX_DEC_TRACK_RATE)){
              currentString = "0.0";
              currentDouble = 0.0;
           }
        } catch (Exception e) {
           currentString = "0.0";
           currentDouble = 0.0;
        }
        setDecTrackRate(currentDouble);
}
/*=============================================================================================
/       calculateFocusOffsetLimits()
/=============================================================================================*/
 public void calculateFocusOffsetLimits(){
     double currentFocus = myTelescopeObject.getFOCUS();
     MIN_FOCUS_OFFSET = currentFocus - MIN_FOCUS;
     MAX_FOCUS_OFFSET = MAX_FOCUS - currentFocus;
 }

/*=============================================================================================
/       FocusOffsetTextFieldActionPerformedMethod(java.awt.event.ActionEvent evt)
/=============================================================================================*/
public void FocusOffsetTextFieldActionPerformedMethod(java.awt.event.ActionEvent evt){
       try{
           calculateFocusOffsetLimits();
           currentString = OffsetFocusTextField.getText();
           currentDouble = (Double.valueOf(currentString)).doubleValue();
           if((currentDouble > MIN_FOCUS_OFFSET)| (currentDouble > MAX_FOCUS_OFFSET)){
              currentString = "0.0";
              currentDouble = 0.0;
           }
        } catch (Exception e) {
           currentString = "0.0";
           currentDouble = 0.0;
        }
        setFocusOffset(currentDouble);
}
/*=============================================================================================
/       FocusTextFieldActionPerformedMethod(java.awt.event.ActionEvent evt)
/=============================================================================================*/
public void FocusTextFieldActionPerformedMethod(java.awt.event.ActionEvent evt){
       try{
           currentString = FocusTextField.getText();
           currentDouble = (Double.valueOf(currentString)).doubleValue();
           if((currentDouble < MIN_FOCUS)| (currentDouble > MAX_FOCUS)){
              currentString = "0.0";
              currentDouble = 0.0;
           }
        } catch (Exception e) {
           currentString = "0.0";
           currentDouble = 0.0;
        }
        setFocus(currentDouble);
}
/*=============================================================================================
/      MoveTelescopeDownButtonActionPerformedMethod(java.awt.event.ActionEvent evt)
/=============================================================================================*/
public void MoveTelescopeDownButtonActionPerformedMethod(java.awt.event.ActionEvent evt){
    OffsetTextFieldAction();
    myP200Component.moveTelescope(myP200Component.MOVING_S, offset);
}
/*=============================================================================================
/      MoveTelescopeDownButtonActionPerformedMethod(java.awt.event.ActionEvent evt)
/=============================================================================================*/
public void MoveTelescopeUpButtonActionPerformedMethod(java.awt.event.ActionEvent evt){
    OffsetTextFieldAction();
    myP200Component.moveTelescope(myP200Component.MOVING_N, offset);
}
/*=============================================================================================
/      MoveTelescopeLeftButtonActionPerformedMethod(java.awt.event.ActionEvent evt)
/=============================================================================================*/
public void MoveTelescopeLeftButtonActionPerformedMethod(java.awt.event.ActionEvent evt){
    OffsetTextFieldAction();
    myP200Component.moveTelescope(myP200Component.MOVING_W, offset);
}
/*=============================================================================================
/      MoveTelescopeRightButtonActionPerformedMethod(java.awt.event.ActionEvent evt)
/=============================================================================================*/
public void MoveTelescopeRightButtonActionPerformedMethod(java.awt.event.ActionEvent evt){
    OffsetTextFieldAction();
    myP200Component.moveTelescope(myP200Component.MOVING_E, offset);
}
/*=============================================================================================
/      OffsetTelescopeButtonActionPerformedMethod(java.awt.event.ActionEvent evt)
/=============================================================================================*/
public void OffsetTelescopeButtonActionPerformedMethod(java.awt.event.ActionEvent evt){
       double currentDouble = 0.0;
       double currentDouble2 = 0.0;
       try{
           currentString = RAOffsetTextField.getText();
           currentDouble = (Double.valueOf(currentString)).doubleValue();
           if((currentDouble < MIN_RA_OFFSET)| (currentDouble > MAX_RA_OFFSET)){
              currentString = "0.0";
              currentDouble = 0.0;
           }
        } catch (Exception e) {
           currentString = "0.0";
           currentDouble = 0.0;
        }
        setRAOffset(currentDouble);
        RAoffset= currentDouble;
         try{
           currentString = DecOffsetTextField.getText();
           currentDouble2 = (Double.valueOf(currentString)).doubleValue();
           if((currentDouble2 < MIN_DEC_OFFSET)| (currentDouble2 > MAX_DEC_OFFSET)){
              currentString = "0.0";
              currentDouble2 = 0.0;
           }
        } catch (Exception e) {
           currentString = "0.0";
           currentDouble2 = 0.0;
        }
        setDecOffset(currentDouble);
        Decoffset=currentDouble2;
    myP200Component.moveTelescope(RAoffset, Decoffset);
}
/*=============================================================================================
/      ReturnToGOButtonActionPerformedMethod(java.awt.event.ActionEvent evt)
/=============================================================================================*/
public void ReturnToGOButtonActionPerformedMethod(java.awt.event.ActionEvent evt){
    myP200Component.returnToGO();
}
/*=============================================================================================
/      OffsetTelescopeButtonActionPerformedMethod(java.awt.event.ActionEvent evt)
/=============================================================================================*/
public void SetFocusButtonActionPerformedMethod(java.awt.event.ActionEvent evt){
    myP200Component.setTelescopeFocus(Focus);
}
/*=============================================================================================
/      FocusUpButtonActionPerformedMethod(java.awt.event.ActionEvent evt)
/=============================================================================================*/
public void FocusUpButtonActionPerformedMethod(java.awt.event.ActionEvent evt){
    myP200Component.offsetTelescopeFocus(FocusOffset);
}
/*=============================================================================================
/      FocusUpButtonActionPerformedMethod(java.awt.event.ActionEvent evt)
/=============================================================================================*/
public void FocusDownButtonActionPerformedMethod(java.awt.event.ActionEvent evt){
    myP200Component.offsetTelescopeFocus(-FocusOffset);
}
/*=============================================================================================
/      SetMoveRateButtonActionPerformedMethod(java.awt.event.ActionEvent evt)
/=============================================================================================*/
public void SetMoveRateButtonActionPerformedMethod(java.awt.event.ActionEvent evt){
    myP200Component.setMoveRates(RAMoveRate, DecMoveRate);
}
/*=============================================================================================
/      SetTrackRateButtonActionPerformedMethod(java.awt.event.ActionEvent evt)
/=============================================================================================*/
public void SetTrackRateButtonActionPerformedMethod(java.awt.event.ActionEvent evt){
        try{
           currentString = RATrackRateTextField.getText();
           RATrackRate = (Double.valueOf(currentString)).doubleValue();
           if((RATrackRate < MIN_RA_TRACK_RATE)| (RATrackRate > MAX_RA_TRACK_RATE)){
              currentString = "0.0";
              RATrackRate = 0.0;
           }
        } catch (Exception e) {
           currentString = "0.0";
           RATrackRate = 0.0;
        }
        setRATrackRate(RATrackRate);
         try{
           currentString = DecTrackRateTextField.getText();
           DecTrackRate = (Double.valueOf(currentString)).doubleValue();
           if((DecTrackRate < MIN_DEC_TRACK_RATE)| (DecTrackRate > MAX_DEC_TRACK_RATE)){
              currentString = "0.0";
              DecTrackRate = 0.0;
           }
        } catch (Exception e) {
           currentString = "0.0";
           currentDouble = 0.0;
        }
        setDecTrackRate(DecTrackRate);
    myP200Component.setTrackRates(RATrackRate, DecTrackRate);
}
/*=============================================================================================
/      ManualMoveRateCheckBoxActionPerformedMethod(java.awt.event.ActionEvent evt)
/=============================================================================================*/
public void ManualMoveRateCheckBoxActionPerformedMethod(java.awt.event.ActionEvent evt){
    myTelescopeObject.setManualMoveRateState(ManualMoveRateCheckBox.isSelected());
}
/*=============================================================================================
/      ZButtonActionPerformedMethod(java.awt.event.ActionEvent evt)
/=============================================================================================*/
public void ZButtonActionPerformedActionPerformedMethod(java.awt.event.ActionEvent evt){
    myP200Component.ZTelescopeOffsets();
}
/*=============================================================================================
/      TrackRatesCheckBoxActionPerformedMethod(java.awt.event.ActionEvent evt)
/=============================================================================================*/
public void TrackRatesCheckBoxActionPerformedMethod(java.awt.event.ActionEvent evt){
    boolean state = TrackRatesCheckBox.isSelected();
    if(state){
       myP200Component.enableTracking();
       System.out.println("Enabling Tracking");
    }
    if(!state){
       myP200Component.disableTracking();
       System.out.println("Disabling Tracking");
    }
}
/*=============================================================================================
/     ConnectTelescopeToggleButtonActionPerformedMethod(java.awt.event.ActionEvent evt)
/=============================================================================================*/
public void ConnectTelescopeToggleButtonActionPerformedMethod(java.awt.event.ActionEvent evt){
        boolean selected = ConnectTelescopeToggleButton.isSelected();
        if(selected){
          myP200Component.connect_control();
        }
        if(!selected){
          myP200Component.disconnect_control();
        }
}
/*=============================================================================================
/     AbortToggleButtonActionPerformedMethod(java.awt.event.ActionEvent evt)
/=============================================================================================*/
public void AbortToggleButtonActionPerformedMethod(java.awt.event.ActionEvent evt) {
    myP200Component.AbortMove();
//    disarmAbort();
}
 @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        jPanel3 = new javax.swing.JPanel();
        BannerLabel = new javax.swing.JLabel();
        jPanel2 = new javax.swing.JPanel();
        TelescopeMoveProgressBar = new javax.swing.JProgressBar();
        RAOffsetTextField = new javax.swing.JTextField();
        jLabel1 = new javax.swing.JLabel();
        jLabel2 = new javax.swing.JLabel();
        DecOffsetTextField = new javax.swing.JTextField();
        OffsetTelescopeButton = new javax.swing.JButton();
        jLabel3 = new javax.swing.JLabel();
        DecTrackRateTextField = new javax.swing.JTextField();
        jLabel4 = new javax.swing.JLabel();
        FocusTextField = new javax.swing.JTextField();
        SetFocusButton = new javax.swing.JButton();
        jLabel5 = new javax.swing.JLabel();
        RAMoveRateTextField = new javax.swing.JTextField();
        jLabel6 = new javax.swing.JLabel();
        DecMoveRateTextField = new javax.swing.JTextField();
        SetMoveRateButton = new javax.swing.JButton();
        jLabel7 = new javax.swing.JLabel();
        RATrackRateTextField = new javax.swing.JTextField();
        SetTrackRateButton = new javax.swing.JButton();
        FocusUpButton = new javax.swing.JButton();
        FocusDownButton = new javax.swing.JButton();
        OffsetFocusTextField = new javax.swing.JTextField();
        jLabel8 = new javax.swing.JLabel();
        ManualMoveRateCheckBox = new javax.swing.JCheckBox();
        ConnectTelescopeToggleButton = new javax.swing.JToggleButton();
        TrackRatesCheckBox = new javax.swing.JCheckBox();
        jPanel4 = new javax.swing.JPanel();
        FocusDisplayTextField = new javax.swing.JTextField();
        RATrackRateDisplayTextField = new javax.swing.JTextField();
        jLabel12 = new javax.swing.JLabel();
        jLabel13 = new javax.swing.JLabel();
        RAMoveRateDisplayTextField = new javax.swing.JTextField();
        jLabel10 = new javax.swing.JLabel();
        DecMoveRateDisplayTextField = new javax.swing.JTextField();
        jLabel14 = new javax.swing.JLabel();
        DECTrackRateDisplayTextField1 = new javax.swing.JTextField();
        jLabel15 = new javax.swing.JLabel();
        jLabel16 = new javax.swing.JLabel();
        jPanel1 = new javax.swing.JPanel();
        MoveTelescopeUpButton = new javax.swing.JButton();
        MoveTelescopeDownButton = new javax.swing.JButton();
        MoveTelescopeLeftButton = new javax.swing.JButton();
        MoveTelescopeRightButton = new javax.swing.JButton();
        OffsetTextField = new javax.swing.JTextField();
        ReturnToGOButton = new javax.swing.JButton();
        jLabel17 = new javax.swing.JLabel();
        StatusMessageLabel = new javax.swing.JLabel();
        AbortButton = new javax.swing.JButton();
        ZButton = new javax.swing.JButton();

        setBackground(new java.awt.Color(0, 0, 0));
        getContentPane().setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        jPanel3.setBackground(new java.awt.Color(0, 0, 0));
        jPanel3.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        BannerLabel.setIcon(new javax.swing.ImageIcon(getClass().getResource("/edu/caltech/palomar/telescopes/P200/gui/PalomarObservatoryMilkyWayPanoramaLogoSmall.jpg"))); // NOI18N
        jPanel3.add(BannerLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(50, 0, -1, -1));

        jPanel2.setBackground(new java.awt.Color(6, 4, 4));
        jPanel2.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());
        jPanel2.add(TelescopeMoveProgressBar, new org.netbeans.lib.awtextra.AbsoluteConstraints(18, 6, 450, 25));

        RAOffsetTextField.setHorizontalAlignment(javax.swing.JTextField.CENTER);
        RAOffsetTextField.setText("0.0");
        RAOffsetTextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                RAOffsetTextFieldActionPerformed(evt);
            }
        });
        jPanel2.add(RAOffsetTextField, new org.netbeans.lib.awtextra.AbsoluteConstraints(108, 38, 117, -1));

        jLabel1.setBackground(new java.awt.Color(0, 0, 0));
        jLabel1.setFont(new java.awt.Font("Arial", 1, 14));
        jLabel1.setForeground(new java.awt.Color(51, 204, 0));
        jLabel1.setText("RA offset");
        jPanel2.add(jLabel1, new org.netbeans.lib.awtextra.AbsoluteConstraints(30, 44, -1, -1));

        jLabel2.setBackground(new java.awt.Color(0, 0, 0));
        jLabel2.setFont(new java.awt.Font("Arial", 1, 14));
        jLabel2.setForeground(new java.awt.Color(51, 204, 0));
        jLabel2.setText("Dec offset");
        jPanel2.add(jLabel2, new org.netbeans.lib.awtextra.AbsoluteConstraints(235, 44, -1, -1));

        DecOffsetTextField.setHorizontalAlignment(javax.swing.JTextField.CENTER);
        DecOffsetTextField.setText("0.0");
        DecOffsetTextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                DecOffsetTextFieldActionPerformed(evt);
            }
        });
        jPanel2.add(DecOffsetTextField, new org.netbeans.lib.awtextra.AbsoluteConstraints(319, 38, 150, -1));

        OffsetTelescopeButton.setText("Offset Telescope");
        OffsetTelescopeButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                OffsetTelescopeButtonActionPerformed(evt);
            }
        });
        jPanel2.add(OffsetTelescopeButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(190, 70, 196, -1));

        jLabel3.setBackground(new java.awt.Color(0, 0, 0));
        jLabel3.setFont(new java.awt.Font("Arial", 1, 12));
        jLabel3.setForeground(new java.awt.Color(51, 204, 0));
        jLabel3.setText("Dec move rate");
        jPanel2.add(jLabel3, new org.netbeans.lib.awtextra.AbsoluteConstraints(170, 118, -1, -1));

        DecTrackRateTextField.setHorizontalAlignment(javax.swing.JTextField.CENTER);
        DecTrackRateTextField.setText("0.0");
        DecTrackRateTextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                DecTrackRateTextFieldActionPerformed(evt);
            }
        });
        jPanel2.add(DecTrackRateTextField, new org.netbeans.lib.awtextra.AbsoluteConstraints(260, 140, 70, -1));

        jLabel4.setBackground(new java.awt.Color(0, 0, 0));
        jLabel4.setFont(new java.awt.Font("Arial", 1, 12));
        jLabel4.setForeground(new java.awt.Color(51, 204, 0));
        jLabel4.setText("Dec track rate");
        jPanel2.add(jLabel4, new org.netbeans.lib.awtextra.AbsoluteConstraints(170, 150, -1, -1));

        FocusTextField.setHorizontalAlignment(javax.swing.JTextField.CENTER);
        FocusTextField.setText("0.0");
        FocusTextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                FocusTextFieldActionPerformed(evt);
            }
        });
        jPanel2.add(FocusTextField, new org.netbeans.lib.awtextra.AbsoluteConstraints(100, 172, 70, -1));

        SetFocusButton.setText("Set Focus");
        SetFocusButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                SetFocusButtonActionPerformed(evt);
            }
        });
        jPanel2.add(SetFocusButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(0, 172, -1, -1));

        jLabel5.setBackground(new java.awt.Color(0, 0, 0));
        jLabel5.setFont(new java.awt.Font("Arial", 1, 12));
        jLabel5.setForeground(new java.awt.Color(51, 204, 0));
        jLabel5.setText("RA move rate");
        jPanel2.add(jLabel5, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 118, -1, -1));

        RAMoveRateTextField.setHorizontalAlignment(javax.swing.JTextField.CENTER);
        RAMoveRateTextField.setText("0.0");
        RAMoveRateTextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                RAMoveRateTextFieldActionPerformed(evt);
            }
        });
        jPanel2.add(RAMoveRateTextField, new org.netbeans.lib.awtextra.AbsoluteConstraints(100, 108, 70, -1));

        jLabel6.setBackground(new java.awt.Color(0, 0, 0));
        jLabel6.setFont(new java.awt.Font("Arial", 1, 12));
        jLabel6.setForeground(new java.awt.Color(51, 204, 0));
        jLabel6.setText("RA track rate");
        jPanel2.add(jLabel6, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 150, -1, -1));

        DecMoveRateTextField.setHorizontalAlignment(javax.swing.JTextField.CENTER);
        DecMoveRateTextField.setText("0.0");
        DecMoveRateTextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                DecMoveRateTextFieldActionPerformed(evt);
            }
        });
        jPanel2.add(DecMoveRateTextField, new org.netbeans.lib.awtextra.AbsoluteConstraints(260, 108, 70, -1));

        SetMoveRateButton.setText("Set Move Rates");
        SetMoveRateButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                SetMoveRateButtonActionPerformed(evt);
            }
        });
        jPanel2.add(SetMoveRateButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(339, 109, -1, -1));

        jLabel7.setBackground(new java.awt.Color(0, 0, 0));
        jLabel7.setFont(new java.awt.Font("Arial", 1, 12));
        jLabel7.setForeground(new java.awt.Color(51, 204, 0));
        jLabel7.setText("Adjust Focus");
        jPanel2.add(jLabel7, new org.netbeans.lib.awtextra.AbsoluteConstraints(220, 200, -1, -1));

        RATrackRateTextField.setHorizontalAlignment(javax.swing.JTextField.CENTER);
        RATrackRateTextField.setText("0.0");
        RATrackRateTextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                RATrackRateTextFieldActionPerformed(evt);
            }
        });
        jPanel2.add(RATrackRateTextField, new org.netbeans.lib.awtextra.AbsoluteConstraints(100, 140, 70, -1));

        SetTrackRateButton.setText("Set Track Rates");
        SetTrackRateButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                SetTrackRateButtonActionPerformed(evt);
            }
        });
        jPanel2.add(SetTrackRateButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(339, 141, -1, -1));

        FocusUpButton.setBackground(new java.awt.Color(0, 0, 0));
        FocusUpButton.setBorderPainted(false);
        FocusUpButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                FocusUpButtonActionPerformed(evt);
            }
        });
        jPanel2.add(FocusUpButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(280, 170, -1, 30));

        FocusDownButton.setBackground(new java.awt.Color(0, 0, 0));
        FocusDownButton.setBorderPainted(false);
        FocusDownButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                FocusDownButtonActionPerformed(evt);
            }
        });
        jPanel2.add(FocusDownButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(170, 170, 50, 30));

        OffsetFocusTextField.setHorizontalAlignment(javax.swing.JTextField.CENTER);
        OffsetFocusTextField.setText("0.0");
        OffsetFocusTextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                OffsetFocusTextFieldActionPerformed(evt);
            }
        });
        jPanel2.add(OffsetFocusTextField, new org.netbeans.lib.awtextra.AbsoluteConstraints(230, 170, 54, -1));

        jLabel8.setBackground(new java.awt.Color(0, 0, 0));
        jLabel8.setFont(new java.awt.Font("Arial", 1, 12));
        jLabel8.setForeground(new java.awt.Color(51, 204, 0));
        jLabel8.setText("Focus");
        jPanel2.add(jLabel8, new org.netbeans.lib.awtextra.AbsoluteConstraints(110, 209, -1, -1));

        ManualMoveRateCheckBox.setBackground(new java.awt.Color(0, 0, 0));
        ManualMoveRateCheckBox.setForeground(new java.awt.Color(0, 204, 0));
        ManualMoveRateCheckBox.setText("Manual Move Rate");
        ManualMoveRateCheckBox.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                ManualMoveRateCheckBoxActionPerformed(evt);
            }
        });
        jPanel2.add(ManualMoveRateCheckBox, new org.netbeans.lib.awtextra.AbsoluteConstraints(30, 70, -1, -1));

        ConnectTelescopeToggleButton.setText("Connect To Telescope");
        ConnectTelescopeToggleButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                ConnectTelescopeToggleButtonActionPerformed(evt);
            }
        });
        jPanel2.add(ConnectTelescopeToggleButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(180, 220, -1, -1));

        TrackRatesCheckBox.setBackground(new java.awt.Color(0, 0, 0));
        TrackRatesCheckBox.setForeground(new java.awt.Color(0, 204, 0));
        TrackRatesCheckBox.setText("Track Rates");
        TrackRatesCheckBox.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                TrackRatesCheckBoxActionPerformed(evt);
            }
        });
        jPanel2.add(TrackRatesCheckBox, new org.netbeans.lib.awtextra.AbsoluteConstraints(360, 170, -1, -1));

        jPanel3.add(jPanel2, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 500, 480, 250));

        jPanel4.setBackground(new java.awt.Color(0, 0, 0));
        jPanel4.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));
        jPanel4.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        FocusDisplayTextField.setBackground(new java.awt.Color(0, 0, 0));
        FocusDisplayTextField.setFont(new java.awt.Font("Arial", 1, 12));
        FocusDisplayTextField.setForeground(new java.awt.Color(0, 204, 51));
        FocusDisplayTextField.setText(" ");
        FocusDisplayTextField.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(204, 204, 204)));
        jPanel4.add(FocusDisplayTextField, new org.netbeans.lib.awtextra.AbsoluteConstraints(350, 10, 120, -1));

        RATrackRateDisplayTextField.setBackground(new java.awt.Color(0, 0, 0));
        RATrackRateDisplayTextField.setFont(new java.awt.Font("Arial", 1, 12));
        RATrackRateDisplayTextField.setForeground(new java.awt.Color(0, 204, 0));
        RATrackRateDisplayTextField.setText(" ");
        RATrackRateDisplayTextField.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(204, 204, 204)));
        jPanel4.add(RATrackRateDisplayTextField, new org.netbeans.lib.awtextra.AbsoluteConstraints(350, 70, 120, -1));

        jLabel12.setBackground(new java.awt.Color(0, 51, 51));
        jLabel12.setFont(new java.awt.Font("Arial", 1, 14));
        jLabel12.setForeground(new java.awt.Color(0, 204, 0));
        jLabel12.setText("Dec Track Rate");
        jPanel4.add(jLabel12, new org.netbeans.lib.awtextra.AbsoluteConstraints(240, 70, -1, -1));

        jLabel13.setBackground(new java.awt.Color(0, 51, 51));
        jLabel13.setFont(new java.awt.Font("Arial", 1, 18));
        jLabel13.setForeground(new java.awt.Color(0, 204, 0));
        jLabel13.setText("Display Parameters:");
        jPanel4.add(jLabel13, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 10, 190, -1));

        RAMoveRateDisplayTextField.setBackground(new java.awt.Color(0, 0, 0));
        RAMoveRateDisplayTextField.setFont(new java.awt.Font("Arial", 1, 12));
        RAMoveRateDisplayTextField.setForeground(new java.awt.Color(0, 204, 0));
        RAMoveRateDisplayTextField.setText(" ");
        RAMoveRateDisplayTextField.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(204, 204, 204)));
        jPanel4.add(RAMoveRateDisplayTextField, new org.netbeans.lib.awtextra.AbsoluteConstraints(120, 40, 110, -1));

        jLabel10.setBackground(new java.awt.Color(0, 51, 51));
        jLabel10.setFont(new java.awt.Font("Arial", 1, 14));
        jLabel10.setForeground(new java.awt.Color(0, 204, 0));
        jLabel10.setText("Dec Move Rate");
        jPanel4.add(jLabel10, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 70, -1, -1));

        DecMoveRateDisplayTextField.setBackground(new java.awt.Color(0, 0, 0));
        DecMoveRateDisplayTextField.setFont(new java.awt.Font("Arial", 1, 12));
        DecMoveRateDisplayTextField.setForeground(new java.awt.Color(0, 204, 0));
        DecMoveRateDisplayTextField.setText(" ");
        DecMoveRateDisplayTextField.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(204, 204, 204)));
        jPanel4.add(DecMoveRateDisplayTextField, new org.netbeans.lib.awtextra.AbsoluteConstraints(120, 70, 110, 20));

        jLabel14.setBackground(new java.awt.Color(0, 51, 51));
        jLabel14.setFont(new java.awt.Font("Arial", 1, 14));
        jLabel14.setForeground(new java.awt.Color(0, 204, 0));
        jLabel14.setText("RA Track Rate");
        jPanel4.add(jLabel14, new org.netbeans.lib.awtextra.AbsoluteConstraints(240, 40, -1, -1));

        DECTrackRateDisplayTextField1.setBackground(new java.awt.Color(0, 0, 0));
        DECTrackRateDisplayTextField1.setFont(new java.awt.Font("Arial", 1, 12));
        DECTrackRateDisplayTextField1.setForeground(new java.awt.Color(0, 204, 0));
        DECTrackRateDisplayTextField1.setText(" ");
        DECTrackRateDisplayTextField1.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(204, 204, 204)));
        jPanel4.add(DECTrackRateDisplayTextField1, new org.netbeans.lib.awtextra.AbsoluteConstraints(350, 40, 120, -1));

        jLabel15.setBackground(new java.awt.Color(0, 51, 51));
        jLabel15.setFont(new java.awt.Font("Arial", 1, 14));
        jLabel15.setForeground(new java.awt.Color(0, 204, 0));
        jLabel15.setText("Focus");
        jPanel4.add(jLabel15, new org.netbeans.lib.awtextra.AbsoluteConstraints(240, 10, -1, -1));

        jLabel16.setBackground(new java.awt.Color(0, 51, 51));
        jLabel16.setFont(new java.awt.Font("Arial", 1, 14));
        jLabel16.setForeground(new java.awt.Color(0, 204, 0));
        jLabel16.setText("RA Move Rate");
        jPanel4.add(jLabel16, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 40, -1, -1));

        jPanel3.add(jPanel4, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 750, 480, 100));

        jPanel1.setBackground(new java.awt.Color(0, 0, 0));
        jPanel1.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        MoveTelescopeUpButton.setBackground(new java.awt.Color(0, 0, 0));
        MoveTelescopeUpButton.setBorderPainted(false);
        MoveTelescopeUpButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                MoveTelescopeUpButtonActionPerformed(evt);
            }
        });
        jPanel1.add(MoveTelescopeUpButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(190, 0, -1, -1));

        MoveTelescopeDownButton.setBackground(new java.awt.Color(0, 0, 0));
        MoveTelescopeDownButton.setBorderPainted(false);
        MoveTelescopeDownButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                MoveTelescopeDownButtonActionPerformed(evt);
            }
        });
        jPanel1.add(MoveTelescopeDownButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(190, 190, -1, -1));

        MoveTelescopeLeftButton.setBackground(new java.awt.Color(0, 0, 0));
        MoveTelescopeLeftButton.setBorderPainted(false);
        MoveTelescopeLeftButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                MoveTelescopeLeftButtonActionPerformed(evt);
            }
        });
        jPanel1.add(MoveTelescopeLeftButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(30, 120, -1, -1));

        MoveTelescopeRightButton.setBackground(new java.awt.Color(0, 0, 0));
        MoveTelescopeRightButton.setBorderPainted(false);
        MoveTelescopeRightButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                MoveTelescopeRightButtonActionPerformed(evt);
            }
        });
        jPanel1.add(MoveTelescopeRightButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(300, 120, -1, -1));

        OffsetTextField.setHorizontalAlignment(javax.swing.JTextField.CENTER);
        OffsetTextField.setText("0.0");
        OffsetTextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                OffsetTextFieldActionPerformed(evt);
            }
        });
        jPanel1.add(OffsetTextField, new org.netbeans.lib.awtextra.AbsoluteConstraints(190, 160, 110, -1));

        ReturnToGOButton.setText("Return to Last GO");
        ReturnToGOButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                ReturnToGOButtonActionPerformed(evt);
            }
        });
        jPanel1.add(ReturnToGOButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 290, -1, -1));

        jLabel17.setBackground(new java.awt.Color(0, 51, 51));
        jLabel17.setFont(new java.awt.Font("Arial", 1, 18));
        jLabel17.setForeground(new java.awt.Color(0, 204, 0));
        jLabel17.setText("Status:");
        jPanel1.add(jLabel17, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 340, 70, -1));

        StatusMessageLabel.setBackground(new java.awt.Color(0, 0, 0));
        StatusMessageLabel.setFont(new java.awt.Font("Arial", 1, 14));
        StatusMessageLabel.setForeground(new java.awt.Color(51, 204, 0));
        StatusMessageLabel.setText("                                         ");
        jPanel1.add(StatusMessageLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(80, 340, 370, 20));

        AbortButton.setBackground(new java.awt.Color(0, 0, 0));
        AbortButton.setBorderPainted(false);
        AbortButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                AbortButtonActionPerformed(evt);
            }
        });
        jPanel1.add(AbortButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(370, 280, -1, -1));

        ZButton.setText("Z current position");
        ZButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                ZButtonActionPerformed(evt);
            }
        });
        jPanel1.add(ZButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 40, -1, -1));

        jPanel3.add(jPanel1, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 140, 470, 360));

        getContentPane().add(jPanel3, new org.netbeans.lib.awtextra.AbsoluteConstraints(0, 0, 510, 850));
    }// </editor-fold>//GEN-END:initComponents

    private void MoveTelescopeDownButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_MoveTelescopeDownButtonActionPerformed
       MoveTelescopeDownButtonActionPerformedMethod(evt);
    }//GEN-LAST:event_MoveTelescopeDownButtonActionPerformed

    private void MoveTelescopeUpButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_MoveTelescopeUpButtonActionPerformed
       MoveTelescopeUpButtonActionPerformedMethod(evt);
    }//GEN-LAST:event_MoveTelescopeUpButtonActionPerformed

    private void MoveTelescopeLeftButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_MoveTelescopeLeftButtonActionPerformed
       MoveTelescopeLeftButtonActionPerformedMethod(evt);
    }//GEN-LAST:event_MoveTelescopeLeftButtonActionPerformed

    private void MoveTelescopeRightButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_MoveTelescopeRightButtonActionPerformed
       MoveTelescopeRightButtonActionPerformedMethod(evt);
    }//GEN-LAST:event_MoveTelescopeRightButtonActionPerformed

    private void OffsetTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_OffsetTextFieldActionPerformed
       OffsetTextFieldActionPerformedMethod(evt);
    }//GEN-LAST:event_OffsetTextFieldActionPerformed

    private void RAOffsetTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_RAOffsetTextFieldActionPerformed
       RAOffsetTextFieldActionPerformedMethod(evt);
    }//GEN-LAST:event_RAOffsetTextFieldActionPerformed

    private void DecTrackRateTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_DecTrackRateTextFieldActionPerformed
       DecTrackRateTextFieldActionPerformedMethod(evt);
    }//GEN-LAST:event_DecTrackRateTextFieldActionPerformed

    private void FocusTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_FocusTextFieldActionPerformed
       FocusTextFieldActionPerformedMethod(evt);
    }//GEN-LAST:event_FocusTextFieldActionPerformed

    private void RAMoveRateTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_RAMoveRateTextFieldActionPerformed
       RAMoveRateTextFieldActionPerformedMethod(evt);
    }//GEN-LAST:event_RAMoveRateTextFieldActionPerformed

    private void DecMoveRateTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_DecMoveRateTextFieldActionPerformed
       DecMoveRateTextFieldActionPerformedMethod(evt);
    }//GEN-LAST:event_DecMoveRateTextFieldActionPerformed

    private void RATrackRateTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_RATrackRateTextFieldActionPerformed
       RATrackRateTextFieldActionPerformedMethod(evt);
    }//GEN-LAST:event_RATrackRateTextFieldActionPerformed

    private void SetFocusButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_SetFocusButtonActionPerformed
        SetFocusButtonActionPerformedMethod(evt);
    }//GEN-LAST:event_SetFocusButtonActionPerformed

    private void FocusDownButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_FocusDownButtonActionPerformed
        FocusDownButtonActionPerformedMethod(evt);
    }//GEN-LAST:event_FocusDownButtonActionPerformed

    private void FocusUpButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_FocusUpButtonActionPerformed
        FocusUpButtonActionPerformedMethod(evt);
    }//GEN-LAST:event_FocusUpButtonActionPerformed

    private void OffsetFocusTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_OffsetFocusTextFieldActionPerformed
        FocusOffsetTextFieldActionPerformedMethod(evt);
    }//GEN-LAST:event_OffsetFocusTextFieldActionPerformed

    private void DecOffsetTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_DecOffsetTextFieldActionPerformed
        DecOffsetTextFieldActionPerformedMethod(evt);
    }//GEN-LAST:event_DecOffsetTextFieldActionPerformed

    private void OffsetTelescopeButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_OffsetTelescopeButtonActionPerformed
        OffsetTelescopeButtonActionPerformedMethod(evt);
    }//GEN-LAST:event_OffsetTelescopeButtonActionPerformed

    private void SetMoveRateButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_SetMoveRateButtonActionPerformed
       SetMoveRateButtonActionPerformedMethod(evt);
    }//GEN-LAST:event_SetMoveRateButtonActionPerformed

    private void SetTrackRateButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_SetTrackRateButtonActionPerformed
       SetTrackRateButtonActionPerformedMethod(evt);
    }//GEN-LAST:event_SetTrackRateButtonActionPerformed

    private void ConnectTelescopeToggleButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_ConnectTelescopeToggleButtonActionPerformed
        ConnectTelescopeToggleButtonActionPerformedMethod(evt);
    }//GEN-LAST:event_ConnectTelescopeToggleButtonActionPerformed

    private void ManualMoveRateCheckBoxActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_ManualMoveRateCheckBoxActionPerformed
        ManualMoveRateCheckBoxActionPerformedMethod(evt);
    }//GEN-LAST:event_ManualMoveRateCheckBoxActionPerformed

    private void ReturnToGOButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_ReturnToGOButtonActionPerformed
       ReturnToGOButtonActionPerformedMethod(evt);
    }//GEN-LAST:event_ReturnToGOButtonActionPerformed

    private void AbortButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_AbortButtonActionPerformed
        AbortToggleButtonActionPerformedMethod(evt);
    }//GEN-LAST:event_AbortButtonActionPerformed

    private void TrackRatesCheckBoxActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_TrackRatesCheckBoxActionPerformed
       TrackRatesCheckBoxActionPerformedMethod(evt);
    }//GEN-LAST:event_TrackRatesCheckBoxActionPerformed

    private void ZButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_ZButtonActionPerformed
       ZButtonActionPerformedActionPerformedMethod(evt);
    }//GEN-LAST:event_ZButtonActionPerformed


    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton AbortButton;
    private javax.swing.JLabel BannerLabel;
    private javax.swing.JToggleButton ConnectTelescopeToggleButton;
    private javax.swing.JTextField DECTrackRateDisplayTextField1;
    private javax.swing.JTextField DecMoveRateDisplayTextField;
    private javax.swing.JTextField DecMoveRateTextField;
    private javax.swing.JTextField DecOffsetTextField;
    private javax.swing.JTextField DecTrackRateTextField;
    private javax.swing.JTextField FocusDisplayTextField;
    private javax.swing.JButton FocusDownButton;
    private javax.swing.JTextField FocusTextField;
    private javax.swing.JButton FocusUpButton;
    private javax.swing.JCheckBox ManualMoveRateCheckBox;
    private javax.swing.JButton MoveTelescopeDownButton;
    private javax.swing.JButton MoveTelescopeLeftButton;
    private javax.swing.JButton MoveTelescopeRightButton;
    private javax.swing.JButton MoveTelescopeUpButton;
    private javax.swing.JTextField OffsetFocusTextField;
    private javax.swing.JButton OffsetTelescopeButton;
    private javax.swing.JTextField OffsetTextField;
    private javax.swing.JTextField RAMoveRateDisplayTextField;
    private javax.swing.JTextField RAMoveRateTextField;
    private javax.swing.JTextField RAOffsetTextField;
    private javax.swing.JTextField RATrackRateDisplayTextField;
    private javax.swing.JTextField RATrackRateTextField;
    private javax.swing.JButton ReturnToGOButton;
    private javax.swing.JButton SetFocusButton;
    private javax.swing.JButton SetMoveRateButton;
    private javax.swing.JButton SetTrackRateButton;
    private javax.swing.JLabel StatusMessageLabel;
    private javax.swing.JProgressBar TelescopeMoveProgressBar;
    private javax.swing.JCheckBox TrackRatesCheckBox;
    private javax.swing.JButton ZButton;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel10;
    private javax.swing.JLabel jLabel12;
    private javax.swing.JLabel jLabel13;
    private javax.swing.JLabel jLabel14;
    private javax.swing.JLabel jLabel15;
    private javax.swing.JLabel jLabel16;
    private javax.swing.JLabel jLabel17;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JLabel jLabel3;
    private javax.swing.JLabel jLabel4;
    private javax.swing.JLabel jLabel5;
    private javax.swing.JLabel jLabel6;
    private javax.swing.JLabel jLabel7;
    private javax.swing.JLabel jLabel8;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JPanel jPanel2;
    private javax.swing.JPanel jPanel3;
    private javax.swing.JPanel jPanel4;
    // End of variables declaration//GEN-END:variables
/*================================================================================================
/       End of the P200Commands() Class Declaration
/=================================================================================================*/
/*
REMOTE COMMANDS FOR THE PALOMAR TELESCOPE CONTROL SYSTEMS (TCS)    10/07/08


OVERVIEW

Remote commands are those commands that can be issued by another computer via
serial link or stream socket to the Telescope Control System (TCS).  The
commands and their functions are listed below.

The command format is an ASCII string starting with the command name, followed
by the correct number of arguments and terminated with a '\r' (carriage-return
character, 015 octal).  Spaces are used to delimit fields.  Numeric arguments
may be integer or floating-point.  Commands may be entered in upper or lower
case.



REMOTE COMMANDS

(null command, '\r' only):  Get-header command.  (See also REQPOS and REQSTAT
commands below.)  Assembles current telescope data as native DEC (Digital
Equipment Corp) binary numbers and returns to the caller 100 8-bit bytes.
These are NOT ASCII characters, and must be interpreted as PDP11-format
numbers ("shorts" are two byte short integer, LSB first;  and "floats" are
modified IEEE format: MSB-1, MSB, LSB, LSB+1.  In other words, each 16-bit
word is byte-swapped).  A "C" declaration for the data structure that is
returned is provided below:

struct {				/* 100 BYTES TOTAL */ /*
        short 	ra_hours;		/* COORDINATES ALWAYS J2000.0 */
/*	short	ra_minutes;
	short	ra_seconds;
	short	ra_hundredth_seconds;
	short	dummyword_4;
	short	dec_degrees;
	short	dec_minutes;
	short	dec_seconds;
	short	dec_tenth_seconds;
	short	dec_sign;		/* 1 = POS, 0 = NEG */
/*	short	lst_hours;
	short	lst_minutes;
	short	lst_seconds;
	short	lst_tenth_seconds;
	short	dummyword_14;
	short	ha_hours;		/* HA IS APPARENT */
/*	short	ha_minutes;
	short	ha_seconds;
	short	ha_tenth_seconds;
	short	ha_direction;		/* 1 = EAST, 0 = WEST */
/*	short	ut_hours;
	short	ut_minutes;
	short	ut_seconds;
	short	ut_tenth_seconds;
	float	airmass;
	float	display_equinox;	/* J2000.0 */
/*	short	dummyword_28;
	short	dummyword_29;
	short	dummyword_30;
	short	dummyword_31;
	short	dummyword_32;
	short	dummyword_33;
	float	cass_ring_angle;
	float	telescope_focus_mm;
	float	telescope_tubelength_mm;
	float	ra_offset_arcsec;
	float	dec_offset_arcsec;
	float	ra_track_rate_arcsec;
	float	dec_track_rate_arcsec;
	short	dummyword_48;
	short	telescope_id;		/* 200 or 60 */
/*} telescope_data;

?NAME (no arguments): return the name of the currently observed position
(the last position for which a GO command has been issued).

?PARALLACTIC (no arguments):  return the current telescope parallactic
angle in ASCII format, as follows: "PARALLACTIC = %.2f\n".  This is intended
for the Double Spectrograph instrument.

?WEATHER (no arguments): does not provide weather information!  Was created
to provide telescope information to the P200 weather system, acting as a
client.  The format of the returned string is:
----------------------------------------------------------------------------
RA=%05.2f\nDec=%+.2f\nHA=%+.2f\nLST=%05.2f\nAir mass=%06.3f\nAzimuth=%06.2f\n\
Zenith angle=%05.2f\nFocus Point=%05.2f\nDome Azimuth=%05.1f\n\
Dome shutters=%1d\nWindscreen position=%05.2f\nInstPos=%1d\nInstrument=%.12s\n\
Pumps=%1d\n
----------------------------------------------------------------------------
Apart from low precision telescope position information, the command indicates
whether the dome shutters are closed (0) or open (1), dome and windscreen
positions, and instrument name.

COORDS (five or six arguments):  supply a position to the TCS.  The arguments
in order are  1) RA in decimal hours, 2) declination in decimal degrees,
3) equinox of coordinates (value of zero indicates apparent equinox), 4) RA
motion, 5) dec motion, 6) motion flag.  (Motion flag absent or 0:  arguments
4 and 5 are proper motion in RA units of .0001 sec/yr and dec units of .001
arcsec/yr.  Motion flag = 1:  arguments 4 and 5 are offset tracking rates in
arcsec/hr.  Motion flag = 2:  arguments 4 and 5 are offset tracking rates
with arg 4 in seconds/hr and arg 5 in arcsec/hr.)  NOTE: a name string can
now be provided, by adding an extra parameter enclosed in double quotes;
this will preferably be at the end of the command line, though it can be
inserted at any parameter position.

E (one argument):  move telescope east by number of arcseconds given in
argument.  Rate of move is set by first argument of MRATES - see below.
Move is precessed to display equinox.  Valid range is 0 to 6000 arcsec.

ES (one argument):  move telescope east by seconds of time given in
argument.  Rate of move is set by first argument of MRATES - see below.
Move is precessed to display equinox.  Valid range: 0 to 600 seconds.

F (no arguments):  move the telescope to the position established by the last
GO command (GO can only be issued from the Night Assistant's console).  This
move is done at 50 arcsec/sec.  Maximum move is 600 sec in RA and 6000 arcsec
in dec.  The telescope must be tracking stably.

FOCUSGO (one argument):  change telescope focus to the value given in the
argument.  Valid range: 1.00 to 74.00 mm. (P200 only)

FOCUSINC (one argument):  change telescope focus by the offset given in the
argument.  Valid range: -73.00 to 73.00 mm.  Limit checking is done.
(P200 only)

GHEAD (one argument):  for instruments with internal mechanisms that
affect the orientation of North on the video guider display, this command
provides a way for the instrument computer to notify the TCS of changes
in orientation.  Valid range: 0 to 360 degrees.

GUIDE (two arguments):  set the RA and dec rates for the handpaddle Guide
buttons.  Valid range: 0 to 50 arcsec/sec.

MRATES (two arguments):  set rate of RA and dec moves to values given in
first and second arguments, respectively.  Valid range: 0 to 50 arcsec/sec.
Note:  these rates apply to moves as described in this note and are
independent of rates for Night-Assistant-commanded moves.

N (one argument):  move telescope north by number of arcseconds given in
argument.  Rate of move is set by second argument of MRATES - see above.
Move is precessed to display equinox.  Valid range: 0 to 6000 arcsec.

NPS (two arguments): control power to lamps and LWIR camera in prime focus
cage.  First argument: 0 = turn off port given in second argument; 1 =
turn on port; 2 = get status of port ("OFF" or "ON ").  Special case:
NPS 0 0 turns off all lamps (ports 1, 2, 3).  Port allocations: 1 = low lamp;
2 = high lamp; 3 = arclamp; 7 = LWIR camera; 8 = LWIR camera shutter.

PT (two arguments):  move telescope in RA and dec simultaneously by
distance given in first and second arguments, respectively.  Distances are
in arcseconds.  Positive values move east and north.  Rates of moves are
set by MRATES - see above.  Moves are precessed to the display equinox.
Valid range: -6000 to 6000 arcsec in each axis.

PTS (two arguments):  move telescope in RA and dec simultaneously by
distance given in first and second arguments, respectively.  RA distance is
in seconds of time (range: -600 to 600 sec), dec is in arcseconds (range:
-6000 to 6000 arcsec).  Positive values move east and north.  Rates of moves
are set by MRATES - see above.  Moves are precessed to the display equinox.

R (no arguments):  turn on offset tracking rates (non-sidereal).  The rates to
be used may be entered at the Night Assistant's console or with the RATES or
RATESS commands - see below.

-R (no arguments):  turn off offset tracking rates.

RATES (two arguments):  define offsets to sidereal tracking rates for non-
sidereal objects.  First argument is RA rate in arcsec/hr, second argument
is declination rate in arcsec/hr.  Rates are enabled with the R command -
see above.

RATESS (two arguments):  define offsets to sidereal tracking rates for non-
sidereal objects.  First argument is RA rate in seconds/hr, second argument
is declination rate in arcsec/hr.  Rates are enabled with the R command -
see above.

RAWDEC (no arguments):  returns the encoder value of declination in ASCII
format, as follows: "RAW_DEC = %.4f\n".  This is intended for LGS operations.

RAWPOS (no arguments):  returns the encoder values of hour angle (in hours)
and declination (in degrees) in ASCII format, as follows:
"HA= %.6f\nDEC= %.5f\n".  This is intended for LGS operations.

RCLEAR (no arguments):  disable offset tracking rates and set offset rates
to zero.

REQPOS (no arguments):  obtain telescope position information in ASCII
format.  Format of returned string (dashed lines not included):
----------------------------------------------------------------------------
UTC = ddd hh:mm:ss.s, LST = hh:mm:ss.s\n
RA = hh:mm:ss.ss, DEC = [+/-]dd:mm:ss.s, HA = [W/E]hh:mm:ss.s\n
air mass = aa.aaa
----------------------------------------------------------------------------
In UTC, ddd is UT day of year.  Fields are fixed width.  RA and DEC are
J2000.  HA is apparent.  There is no terminating newline.  (See also Get-
header command above.)

REQSTAT (no arguments): obtain telescope status information in ASCII
format.  Format of returned string (dashed lines not included):
----------------------------------------------------------------------------
UTC = ddd hh:mm:ss.s\n
telescope ID = ttt, focus = ***** mm, tube length = tt.tt mm\n
offset RA = rrrrrrr.r arcsec, DEC = ddddddd.d arcsec\n
rate RA = rrrrrrr.r arcsec/hr, DEC = ddddddd.d arcsec/hr\n
Cass ring angle = ccc.cc
----------------------------------------------------------------------------
In UTC, ddd is UT day of year.  ID is 200 or 60.  At P200, focus format
is ff.ff mm;  at P60 the focus reading is fffff and is in arbitrary units
(not mm, although "mm" is displayed).  Tube length and Cass ring angle are
meaningless at P60.  Fields are fixed width.  There is no terminating
newline.  (See also Get-header command above.)

RET (no arguments):  move the telescope so the DRA and DDEC offsets on the
telescope status display go to zero.  (See Z command, below.)  Rate is
50 arcsec/sec.  Maximum move: 600 sec in RA, 6000 arcsec in dec.  The
telescope must be tracking stably.

RINGGO (one argument):  rotate the Cass ring to the position specified in
the argument, in degrees.  If the position is available in more than one
turn of the ring, the shortest move will be taken.  Valid range: 0 to
359.99... degrees.

RINGMOVE (one argument):  rotate the Cass ring by the displacement specified
in the argument, in degrees.  Valid range: depends on current position with
respect to limits.  If out of range, "-3" (unable to execute) is returned.

S (one argument):  move telescope south by number of arcseconds given in
argument.  Rate of move is set by second argument of MRATES - see above.
Move is precessed to display equinox.  Valid range: 0 to 6000 arcsec.

SET (two arguments):  set the RA and dec rates for the handpaddle Set
buttons.  Valid range: 0 to 50 arcsec/sec.

TX (no arguments):  define the current position of the telescope to be the
position established with the last GO command (operator command only).
Similar to X -- see below, but pointing offsets are not saved, and the
operator can restore the pointing offsets associated with the last X.
DISABLED-- not available to clients; telescope operator only.

W (one argument):  move telescope west by number of arcseconds given in
argument.  Rate of move is set by first argument of MRATES - see above.
Move is precessed to display equinox.  Valid range: 0 to 6000 arcsec.

WS (one argument):  move telescope west by seconds of time given in
argument.  Rate of move is set by first argument of MRATES - see above.
Move is precessed to display equinox.  Valid range: 0 to 600 seconds.

X (no arguments):  define the current position of the telescope to be the
position established with the last GO command (operator command only).
Normally done after GOing to a star with known good coordinates and centering
the star in the field, to optimize local pointing.  X zeroes the displayed
DRA and DDEC offsets.  Valid range:  change in pointing offsets less than
10 sec in RA and 300 arcsec in dec.  NOTE: this command must be used with
care -- if used at the wrong position, subsequent telescope pointing will be
bad.  DISABLED-- not available to clients; telescope operator only.

Z (no arguments):  zero the DRA and DDEC display offsets;  ie, make the
current position of the telescope a reference point.



RETURNS

The get-header command and the ?NAME, ?PARALLACTIC, ?WEATHER, RAWDEC, RAWPOS,
REQPOS, and REQSTAT commands return the requested information.  All other
commands return an ASCII string to show completion status.  (Motion commands
do not return until motion is complete.)  All ASCII strings have a null
character appended.  The meaning of return strings is as follows:

	"0":  Successful completion.
	"-1": Unrecognized command.
	"-2": Invalid parameter(s).
	"-3": Unable to execute at this time.
	"-4": TCS Sparc host unavailable.



COMMUNICATIONS PROTOCOLS

SERIAL PROTOCOL:  9600 bps, 1 start bit, 8 data bits, 1 stop bit, no parity.
No handshaking.

STREAM SOCKET PARAMETERS:
TCS IP address (P200): 198.202.125.194 (proxy address)
TCP service (port): 5004
[A sample client program, written in C, is available.]
*/
}
