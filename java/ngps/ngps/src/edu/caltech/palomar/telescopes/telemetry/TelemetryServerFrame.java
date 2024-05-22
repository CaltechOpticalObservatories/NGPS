/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package edu.caltech.palomar.telescopes.telemetry;

import edu.caltech.palomar.telescopes.P200.P200Component;
import java.awt.Color;
import java.beans.PropertyChangeEvent;
import edu.caltech.palomar.telescopes.telemetry.TelemetryController;
import edu.caltech.palomar.telescopes.telemetry.WeatherController;
import edu.caltech.palomar.telescopes.telemetry.TelescopeObject;
import edu.caltech.palomar.util.general.CommandLogModel;
import edu.dartmouth.jskycalc.JSkyCalcModel;
import edu.dartmouth.jskycalc.coord.RA;
import java.awt.geom.Point2D;
import javax.swing.ImageIcon;
import javax.swing.ListSelectionModel;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;
import javax.swing.JFileChooser;
import java.io.File;
import java.io.PrintWriter;
import javax.swing.table.DefaultTableCellRenderer;
 
/** 
 *
 * @author developer
 */
public class TelemetryServerFrame extends javax.swing.JFrame {
   public TelemetryController myTelemetryController;
   public TelescopeObject     myTelescopeObject;
   public JSkyCalcModel       myJSkyCalcModel;
   public  CommandLogModel    myCommandLogModel;
   public  CommandLogModel    myConnectionLogModel;
   public  CommandLogModel    myHeaderLogModel = new CommandLogModel();
   private int                startYear;
   private int                startMonth;
   private int                startDay;
   private int                startHour;
   private int                startMinute;
   private int                startSecond;
   private int                endYear;
   private int                endMonth;
   private int                endDay;
   private int                endHour;
   private int                endMinute;
   private int                endSecond;
   public  ImageIcon          ON;
   public  ImageIcon          OFF;
   public  ImageIcon          UNKNOWN;   
   public   java.lang.String      USERHOME          = System.getProperty("user.home");
   public   java.lang.String      USERDIR           = System.getProperty("user.dir");
   public   java.lang.String      SEP               = System.getProperty("file.separator");
   public   java.lang.String      IMAGE_CACHE       = new java.lang.String(SEP + "images" + SEP);
   public WeatherController   myWeatherController;
    /**
     * Creates new form TelemetryServerFrame
     */
    public TelemetryServerFrame() {
        initComponents();
        initializeImageIcons();
    }
/*================================================================================================
/         initializeForm(P200Component newP200Component)
/=================================================================================================*/
  public void intializeTimestamps(){
     java.time.ZonedDateTime now = java.time.ZonedDateTime.now(java.time.ZoneOffset.UTC);
     startYear   = now.getYear();
     startMonth  = now.getMonthValue();
     startDay    = now.getDayOfMonth();
     startHour   = now.getHour();
     startMinute = now.getMinute();
     startSecond = now.getSecond();
     endYear     = now.getYear();
     endMonth    = now.getMonthValue();
     endDay      = now.getDayOfMonth();
     endHour     = now.getHour();
     endMinute   = now.getMinute();
     endSecond   = now.getSecond();
     startYearSpinner.setValue(startYear);
     startMonthSpinner.setValue(startMonth);
     startDaySpinner.setValue(startDay);
     startHourSpinner.setValue(startHour);
     startMinuteSpinner.setValue(startMinute);
     startSecondSpinner.setValue(startSecond);
     endYearSpinner.setValue(endYear);
     endMonthSpinner.setValue(endMonth);
     endDaySpinner.setValue(endDay);
     endHourSpinner.setValue(endHour);
     endMinuteSpinner.setValue(endMinute);
     endSecondSpinner.setValue(endSecond);
     constructStartTimestamp();
     constructEndTimestamp();      
  } 
/*================================================================================================
/     initializeImageIcons()
/=================================================================================================*/
  private void initializeImageIcons(){
     ON           = new ImageIcon(USERDIR + IMAGE_CACHE + "ON.png");
     OFF          = new ImageIcon(USERDIR + IMAGE_CACHE + "OFF.png");
     UNKNOWN      = new ImageIcon(USERDIR + IMAGE_CACHE + "UNKNOWN.gif");
     outputDirectoryTextField.setText(USERHOME);
  }
/*================================================================================================
/     browseForOutputDirectory()
/=================================================================================================*/
  private void browseForOutputDirectory(){
    JFileChooser chooser = new JFileChooser(); 
    chooser.setCurrentDirectory(new java.io.File(USERHOME));
    chooser.setDialogTitle("Browse for Output Directory");
    chooser.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
    chooser.setAcceptAllFileFilterUsed(false);  
    if (chooser.showOpenDialog(this) == JFileChooser.APPROVE_OPTION) { 
      System.out.println("getCurrentDirectory(): " +  chooser.getCurrentDirectory());
      myTelemetryController.setOutputDirectory(chooser.getSelectedFile().getAbsolutePath());
//      System.out.println("getSelectedFile() : " +  chooser.getSelectedFile());
    }
    else {
      System.out.println("No Selection ");
    }
  }
/*================================================================================================
/         initializeForm(P200Component newP200Component)
/=================================================================================================*/
   public void initialize(TelemetryController newTelemetryController){
        myTelemetryController =  newTelemetryController;
        setTelescopeObject(myTelemetryController.getTelescopeObject());
        setJSkyCalcModel(myTelemetryController.myTelescopeObject.getJSkyCalcModel());
        myTelemetryController.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
        public void propertyChange(java.beans.PropertyChangeEvent e) {
            TelemetryController_propertyChange(e);
         }
        });
        intializeTimestamps();
        queryResultsTable.setModel(myTelemetryController.getTelemetryTableModel());
        queryResultsTable.setAutoResizeMode(queryResultsTable.AUTO_RESIZE_OFF);
        queryResultsTable.getColumnModel().getColumn(0).setMinWidth(80);
        queryResultsTable.getColumnModel().getColumn(1).setMinWidth(120);
        queryResultsTable.getColumnModel().getColumn(2).setMinWidth(120);
        queryResultsTable.getColumnModel().getColumn(3).setMinWidth(120);
        queryResultsTable.getColumnModel().getColumn(4).setMinWidth(120);
        queryResultsTable.getColumnModel().getColumn(5).setMinWidth(120);
        queryResultsTable.getColumnModel().getColumn(6).setMinWidth(120);
        queryResultsTable.getColumnModel().getColumn(7).setMinWidth(120);
        queryResultsTable.getColumnModel().getColumn(8).setMinWidth(120);
        queryResultsTable.getColumnModel().getColumn(9).setMinWidth(120);
        queryResultsTable.getColumnModel().getColumn(10).setMinWidth(120);
        queryResultsTable.getColumnModel().getColumn(11).setMinWidth(120);
        queryResultsTable.getColumnModel().getColumn(12).setMinWidth(120);
        queryResultsTable.getColumnModel().getColumn(13).setMinWidth(120);
        queryResultsTable.getColumnModel().getColumn(14).setMinWidth(120);
        queryResultsTable.getColumnModel().getColumn(15).setMinWidth(120);
        queryResultsTable.getColumnModel().getColumn(16).setMinWidth(120);
        queryResultsTable.getColumnModel().getColumn(17).setMinWidth(120);
        queryResultsTable.getColumnModel().getColumn(18).setMinWidth(120);
        queryResultsTable.getColumnModel().getColumn(19).setMinWidth(120);
        queryResultsTable.getColumnModel().getColumn(20).setMinWidth(120);
        queryResultsTable.getColumnModel().getColumn(21).setMinWidth(120);
        queryResultsTable.getColumnModel().getColumn(22).setMinWidth(120);
        queryResultsTable.getColumnModel().getColumn(23).setMinWidth(120);
        queryResultsTable.getColumnModel().getColumn(24).setMinWidth(120);
        queryResultsTable.getColumnModel().getColumn(25).setMinWidth(120);
        queryResultsTable.getColumnModel().getColumn(26).setMinWidth(120);
        queryResultsTable.getColumnModel().getColumn(27).setMinWidth(120);
        queryResultsTable.getColumnModel().getColumn(28).setMinWidth(120);
        queryResultsTable.getColumnModel().getColumn(29).setMinWidth(120);
        queryResultsTable.getColumnModel().getColumn(30).setMinWidth(120);
        queryResultsTable.getColumnModel().getColumn(31).setMinWidth(120);
        queryResultsTable.getColumnModel().getColumn(32).setMinWidth(120);
        queryResultsTable.getColumnModel().getColumn(33).setMinWidth(120);
        queryResultsTable.getColumnModel().getColumn(34).setMinWidth(120);
        queryResultsTable.getColumnModel().getColumn(35).setMinWidth(120);
        queryResultsTable.getColumnModel().getColumn(36).setMinWidth(120);
        queryResultsTable.getColumnModel().getColumn(37).setMinWidth(120);
        queryResultsTable.getColumnModel().getColumn(38).setMinWidth(120);
        queryResultsTable.getColumnModel().getColumn(39).setMinWidth(120);
        queryResultsTable.getColumnModel().getColumn(40).setMinWidth(120);
        queryResultsTable.getColumnModel().getColumn(41).setMinWidth(120);
        queryResultsTable.getColumnModel().getColumn(42).setMinWidth(120);
        queryResultsTable.getColumnModel().getColumn(43).setMinWidth(120);
        queryResultsTable.getColumnModel().getColumn(44).setMinWidth(120);
        queryResultsTable.getColumnModel().getColumn(45).setMinWidth(120);
        queryResultsTable.getColumnModel().getColumn(46).setMinWidth(120);
        queryResultsTable.getColumnModel().getColumn(47).setMinWidth(120);
        queryResultsTable.getColumnModel().getColumn(48).setMinWidth(120);
        queryResultsTable.getColumnModel().getColumn(49).setMinWidth(120);
        queryResultsTable.getColumnModel().getColumn(1).setCellRenderer(new TimestampRenderer());
      SelectedFITSHeaderEditorPane.setDocument(myHeaderLogModel.getDocument());
    queryResultsTable.setAutoCreateRowSorter(true);
    queryResultsTable.getRowSorter().toggleSortOrder(2);
    queryResultsTable.getSelectionModel().addListSelectionListener(new SharedListSelectionHandler());
    queryResultsTable.setUpdateSelectionOnSort(true);
    errorEditorPane.setDocument(myTelemetryController.getCommandLogModel().getDocument());
    myTelemetryController.setOutputDirectory(USERHOME);
    filenameTextField.setText("output.csv");
   }
/*================================================================================================
/     setTelescopeObject(TelescopeObject newTelescopeObject)
/=================================================================================================*/
 public void setTelescopeObject(TelescopeObject newTelescopeObject){
     myTelescopeObject = newTelescopeObject;
     myTelescopeObject.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
      public void propertyChange(java.beans.PropertyChangeEvent e) {
          TelescopeObject_propertyChange(e);
       }
     });
    }  
/*================================================================================================
/     setJSkyCalcModel(JSkyCalcModel newJSkyCalcModel)
/=================================================================================================*/
 public void setJSkyCalcModel(JSkyCalcModel newJSkyCalcModel){
     myJSkyCalcModel = newJSkyCalcModel;
     myJSkyCalcModel.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
      public void propertyChange(java.beans.PropertyChangeEvent e) {
          Ephemeris_propertyChange(e);
       }
     });
    } 
/*================================================================================================
/    setWeatherController(WeatherController newWeatherController)
/=================================================================================================*/
 public void setWeatherController(WeatherController newWeatherController){
     myWeatherController = newWeatherController; 
     myWeatherController.myP200WeatherObject.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
      public void propertyChange(java.beans.PropertyChangeEvent e) {
          p200weather_propertyChange(e);
       }
     });
     myWeatherController.myP60WeatherObject.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
      public void propertyChange(java.beans.PropertyChangeEvent e) {
          p60weather_propertyChange(e);
       }
     });
     myWeatherController.myP48WeatherObject.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
      public void propertyChange(java.beans.PropertyChangeEvent e) {
          p48weather_propertyChange(e);
       }
     });
 }
/*================================================================================================
/    setComsLog(CommandLogModel   newCommandLogModel)
/=================================================================================================*/
 public void setComsLog(CommandLogModel   newCommandLogModel){
   myCommandLogModel = newCommandLogModel; 
   this.comslogEditorPane.setDocument(myCommandLogModel.getDocument());
 }
 public void setConnectionLog(CommandLogModel newCommandLogModel){
    myConnectionLogModel =  newCommandLogModel;
    connectionEditorPane.setDocument(myConnectionLogModel.getDocument());
 }
 /*=============================================================================================
/     Property Change Listener for the _propertyChange
/=============================================================================================*/
  private void TelemetryController_propertyChange(PropertyChangeEvent e)  {
     java.lang.String propertyName = e.getPropertyName();
       if(propertyName == "connected"){
        boolean state = (java.lang.Boolean)e.getNewValue();
        if(state){
          ConnectToTCSToggleButton.setText("Connected to Telescope");
          UpdateToggleButton.setEnabled(true);
          UpdateToggleButton.setText("Update");
          ConnectToTCSToggleButton.setIcon(ON);
        }
        if(!state){
          ConnectToTCSToggleButton.setText("Connect to Telescope");
          UpdateToggleButton.setEnabled(false);
          UpdateToggleButton.setText("");
          ConnectToTCSToggleButton.setIcon(OFF);
        }
      }
       if(propertyName.matches("start_timestamp")){
           java.lang.String start_timestamp = (java.lang.String)e.getNewValue();
           startDatetimeTextField.setText(start_timestamp);
       }
       if(propertyName.matches("end_timestamp")){
           java.lang.String end_timestamp = (java.lang.String)e.getNewValue();
           endDatetimeTextField.setText(end_timestamp);
       }
       if(propertyName.matches("output_directory")){
           java.lang.String new_output_directory = (java.lang.String)e.getNewValue();
           outputDirectoryTextField.setText(new_output_directory);
       }
       if(propertyName.matches("polling")){
        boolean state = (java.lang.Boolean)e.getNewValue();
        if(state){
           this.UpdateToggleButton.setIcon(ON);
           UpdateToggleButton.setText("Updating");
           UpdateToggleButton.setSelected(true);
        }   
        if(!state){
           this.UpdateToggleButton.setIcon(OFF); 
           UpdateToggleButton.setText("Update");
           UpdateToggleButton.setSelected(false);
        }
       }
  }
/*=============================================================================================
/     Property Change Listener for the _propertyChange
/=============================================================================================*/
  private void TelescopeObject_propertyChange(PropertyChangeEvent e)  {
     java.lang.String propertyName = e.getPropertyName();
/*=============================================================================================
/
/=============================================================================================*/
     if(propertyName == "right_ascension"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        CurrentRATextField.setText(newValue);
    }
    if(propertyName == "declination"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        CurrentDecTextField.setText(newValue);
    }
    if(propertyName == "right_ascension_last_GO"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        OffsetRATextField.setText(newValue);
    }
    if(propertyName == "declination_last_GO"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        OffsetDecTextField.setText(newValue);
    }
    if(propertyName == "equinox_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        EquinoxTextField.setText(newValue);
    }
    if(propertyName == "hourangle_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        HATextField.setText(newValue);
    }
    if(propertyName == "airmass_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        AirmassTextField.setText("");
        AirmassTextField.setText(newValue);
    }
    if(propertyName == "UTCSTART"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        UTCTextField.setText(newValue);
    }
    if(propertyName == "universal_time"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        UTCTextField.setText(newValue);
    }
    if(propertyName == "sidereal"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        LSTTextField.setText(newValue);
    }
    if(propertyName == "right_ascension_offset"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        RAOffsetTextField.setText(newValue);
    }
    if(propertyName == "declination_offset"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        DecOffsetTextField.setText(newValue);
    }
    if(propertyName == "right_ascension_track_rate"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "declination_track_rate"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "telescope_focus"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        FocusTextField.setText(newValue);
    }
    if(propertyName == "telescope_tube_length"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "cass_ring_angle"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        CassRingAngleTextField.setText(newValue);
    }
    if(propertyName == "telescope_ID"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "azimuth"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        AzimuthTextField.setText(newValue);
    }
    if(propertyName == "zenith_angle"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        ZenithAngleTextField.setText(newValue);
    }
    if(propertyName == "dome_azimuth"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        DomeAzimuthTextField.setText(newValue);
    }
    if(propertyName == "windscreen_position"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        WindScreensTextField.setText(newValue);
    }
    if(propertyName == "dome_shutters"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        if(newValue.matches("0")){
           DomeShuttersTextField.setText("Closed");
        }
        if(newValue.matches("1")){
           DomeShuttersTextField.setText("Open");
        }
     }
     if(propertyName.matches("right_ascension_track_rate")){
         java.lang.String newValue = (java.lang.String)e.getNewValue();
         TrackRateRATextField.setText(newValue);
     }
     if(propertyName.matches("declination_track_rate")){
         java.lang.String newValue = (java.lang.String)e.getNewValue();
         TrackRateDecTextField.setText(newValue);
     }
     if(propertyName.matches("objectname_string")){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        ObjectNameTextField.setText(newValue);  
     }
     if(propertyName.matches("motion_status_string")){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        if(newValue.matches("SLEWING")){
           motionStateTextField.setForeground(Color.red);
        }
        if(newValue.matches("OFFSETTING")){
           motionStateTextField.setForeground(Color.yellow);
        }
        if(newValue.matches("STOPPED")){
           motionStateTextField.setForeground(Color.gray);
        }
        if(newValue.matches("SETTLING")){
           motionStateTextField.setForeground(Color.orange);            
        }
        if(newValue.matches("TRACKING STABLY")){
           motionStateTextField.setForeground(Color.green);
        }       
        motionStateTextField.setText(newValue);
     }
  }
/*=============================================================================================
/     Property Change Listener for the Ephemeris_propertyChange
/=============================================================================================*/
  private void Ephemeris_propertyChange(PropertyChangeEvent e)  {
     java.lang.String propertyName = e.getPropertyName();
/*=============================================================================================
/    PARAMETERS FROM THE JSKYCALCMODEL
/=============================================================================================*/
    if(propertyName == "jd_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        jdfield.setText(newValue);
    }
    if(propertyName == "moonobj_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        moonobjanglefield.setText(newValue);
    }
    if(propertyName == "baryjd_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        baryjdfield.setText(newValue);
    }
    if(propertyName == "baryvcor_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        baryvcorfield.setText(newValue);
    }
    if(propertyName == "barytcor_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        baryvcorfield.setText(newValue);
    }
    if(propertyName == "constellation"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        constellationfield.setText(newValue);
    }
    if(propertyName == "planetwarning"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        planetproximfield.setText(newValue);
    }
    if(propertyName == "moonra_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        moonrafield.setText(newValue);
    }
    if(propertyName == "moondec_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        moondecfield.setText(newValue);
    }
    if(propertyName == "altmoon_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        moonaltfield.setText(newValue);
    }
    if(propertyName == "azmoon_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        moonazfield.setText(newValue);
    }
    if(propertyName == "sunra_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        sunrafield.setText(newValue);
    }
    if(propertyName == "sundec_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        sundecfield.setText(newValue);
    }
    if(propertyName == "altsun_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        sunaltfield.setText(newValue);
    }
    if(propertyName == "azsun_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        sunazfield.setText(newValue);
    }
    if(propertyName == "ztwilight_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        ztwilightfield.setText(newValue);
    }
    if(propertyName == "moonphasedescription"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        moonphasefield.setText(newValue);
    }
    if(propertyName == "siderialobj_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        siderialfield.setText(newValue);
    }
    if(propertyName == "ha_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        hafield.setText(newValue);
    }
    if(propertyName == "airmass_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        airmassfield.setText(newValue);
    }
    if(propertyName == "parallactic_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        parallacticfield.setText(newValue);
    }
    if(propertyName == "altparallactic_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        antiparallacticfield.setText(newValue);
    }
    if(propertyName == "moonlight_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "moonillum_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        moonillumfield.setText(newValue);
    }
    if(propertyName == "ra_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        RAfield.setText(newValue);
    }
    if(propertyName == "dec_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        decfield.setText(newValue);
    }
    if(propertyName == "equinox_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        equinoxfield.setText(newValue);
    }
    if(propertyName == "azimuth_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        azimuthfield.setText(newValue);
    }
    if(propertyName == "altitude_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        altitudefield.setText(newValue);
    }
    if(propertyName == "UTDateDate"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        utdatefield.setText(newValue);
    }
    if(propertyName == "UTDateTime"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        uttimefield.setText(newValue);
    }
    if(propertyName == "localDateDate"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        localdatefield.setText(newValue);
    }
    if(propertyName == "localDateTime"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        localtimefield.setText(newValue);
    }
    if(propertyName == "obsname"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        observatoryfield.setText(newValue);
    }
    if(propertyName == "longitude_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        longitudefield.setText(newValue);
    }
    if(propertyName == "latitude_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        latitudefield.setText(newValue);
    }
    if(propertyName == "elevsea_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        elevseafield.setText(newValue);
    }
    if(propertyName == "elevhorizon_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        elevhorizonfield.setText(newValue);
    }
    if(propertyName == "stdz_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "use_dst_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "zonename_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "nearestBrightStar"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
    }
    if(propertyName == "objectname_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        objectnamefield.setText(newValue);
    }
  }
/*=============================================================================================
/     constructStartTimestamp()
/=============================================================================================*/
  public java.lang.String constructStartTimestamp(){
      java.lang.String monthString = Integer.toString(startMonth);
      if(monthString.length() == 1){
        monthString = "0"+monthString;  
      }
      java.lang.String dayString = Integer.toString(startDay);
      if(dayString.length() == 1){
        dayString = "0"+dayString;  
      }
      java.lang.String hourString = Integer.toString(startHour);
      if(hourString.length() == 1){
        hourString = "0"+hourString;  
      }
      java.lang.String minuteString = Integer.toString(startMinute);
      if(minuteString.length() == 1){
        minuteString = "0"+minuteString;  
      }
      java.lang.String secondString = Integer.toString(startSecond);
      if(secondString.length() == 1){
        secondString = "0"+secondString;  
      }      
      java.lang.String start_timestamp = Integer.toString(startYear)+"-"+monthString+"-"+dayString+" "+
                                         hourString+":"+minuteString+":"+secondString;
      myTelemetryController.setStartTimestamp(start_timestamp);
     return start_timestamp;
  }
/*=============================================================================================
/     constructStartTimestamp()
/=============================================================================================*/
  public java.lang.String constructEndTimestamp(){
      java.lang.String monthString = Integer.toString(endMonth);
      if(monthString.length() == 1){
        monthString = "0"+monthString;  
      }
      java.lang.String dayString = Integer.toString(endDay);
      if(dayString.length() == 1){
        dayString = "0"+dayString;  
      }
      java.lang.String hourString = Integer.toString(endHour);
      if(hourString.length() == 1){
        hourString = "0"+hourString;  
      }
      java.lang.String minuteString = Integer.toString(endMinute);
      if(minuteString.length() == 1){
        minuteString = "0"+minuteString;  
      }
      java.lang.String secondString = Integer.toString(endSecond);
      if(secondString.length() == 1){
        secondString = "0"+secondString;  
      }      
      java.lang.String end_timestamp = Integer.toString(endYear)+"-"+monthString+"-"+dayString+" "+
                                         hourString+":"+minuteString+":"+secondString;
      myTelemetryController.setEndTimestamp(end_timestamp);
     return end_timestamp;
  } 
/*=============================================================================================
/     constructStartTimestamp()
/=============================================================================================*/
public void writeCSV(){
    try{
       java.lang.String output_filename = this.filenameTextField.getText();
       java.lang.String complete_filename = myTelemetryController.getOutputDirectory()+SEP+output_filename;
       File output_file = new File(complete_filename);
       PrintWriter pw = new PrintWriter(output_file);
       int count = myTelemetryController.getTelemetryTableModel().getRowCount();
       pw.println(myTelemetryController.getTelemetryTableModel().getRecord(1).headerString());
       for(int i=0;i<count;i++){
           TelemetryObject current =  myTelemetryController.getTelemetryTableModel().getRecord(i);
           pw.println(current.toString());
       }
       pw.flush();
       pw.close();
    }catch(Exception e){
       myTelemetryController.myCommandLogModel.insertMessage(CommandLogModel.ERROR,"Error writing CSV file. "+e.toString());
    }
}
/*=============================================================================================
/     Property Change Listener for the Ephemeris_propertyChange
/=============================================================================================*/
  private void p200weather_propertyChange(PropertyChangeEvent e)  {
     java.lang.String propertyName = e.getPropertyName();
/*=============================================================================================
/    PARAMETERS FROM THE JSKYCALCMODEL
/=============================================================================================*/
    if(propertyName.matches("P200_dewpoint")){
        java.lang.Double current_value = (java.lang.Double)e.getNewValue();   
        p200_dewpoint.setText(String.format("%.2f", current_value.doubleValue()));
    }
    if(propertyName.matches("P200_outside_air_temperature")){
        java.lang.Double current_value = (java.lang.Double)e.getNewValue();        
        this.p200_outside_temperature.setText(String.format("%.2f", current_value.doubleValue()));
    }    
    if(propertyName.matches("P200_dome_air_ttemp")){
        java.lang.Double current_value = (java.lang.Double)e.getNewValue();        
        this.p200_inside_temperature.setText(String.format("%.2f", current_value.doubleValue()));
    }    
    if(propertyName.matches("P200_primary_mirror_temp")){
        java.lang.Double current_value = (java.lang.Double)e.getNewValue();   
        this.p200_primary_mirror_temperature.setText(String.format("%.2f", current_value.doubleValue()));
    }    
    if(propertyName.matches("P200_prime_focus_temp")){
        java.lang.Double current_value = (java.lang.Double)e.getNewValue(); 
        this.p200_secondary_mirror_temperature.setText(String.format("%.2f", current_value.doubleValue()));
    }    
    if(propertyName.matches("P200_floor_temp")){
        java.lang.Double current_value = (java.lang.Double)e.getNewValue();    
        this.p200_floor_temperature.setText(String.format("%.2f", current_value.doubleValue()));
    }
   if(propertyName.matches("P200_primary_cell_temp")){
        java.lang.Double current_value = (java.lang.Double)e.getNewValue();   
        this.p200_primary_cell_temperature.setText(String.format("%.2f", current_value.doubleValue()));
    }
   if(propertyName.matches("P200_windspeed_median")){
        java.lang.Double current_value = (java.lang.Double)e.getNewValue(); 
        this.p200_average_windspeed.setText(String.format("%.2f", current_value.doubleValue()));
    }   
   if(propertyName.matches("P200_windspeed_peak")){
        java.lang.Double current_value = (java.lang.Double)e.getNewValue();   
        this.p200_peak_windspeed.setText(String.format("%.2f", current_value.doubleValue()));
    }
   if(propertyName.matches("P200_wind_direction")){
        java.lang.Double current_value = (java.lang.Double)e.getNewValue();  
        this.p200_wind_direction.setText(String.format("%.2f", current_value.doubleValue()));
    }
   if(propertyName.matches("P200_windspeed_stddev")){
        java.lang.Double current_value = (java.lang.Double)e.getNewValue();  
        this.p200_gust_windspeed.setText(String.format("%.2f", current_value.doubleValue()));      
   }
   if(propertyName.matches("raining")){
        java.lang.Boolean current_value = (java.lang.Boolean)e.getNewValue(); 
        if(current_value.booleanValue()){
            this.p200_raining.setText("RAINING");
        }
        if(!current_value.booleanValue()){
            this.p200_raining.setText("NONE");            
        }
    }
   if(propertyName.matches("P200_temp_stairs0")){
        java.lang.Double current_value = (java.lang.Double)e.getNewValue();
        this.p200_stairs20.setText(String.format("%.2f", current_value.doubleValue()));
    }
   if(propertyName.matches("P200_temp_stairs20")){
        java.lang.Double current_value = (java.lang.Double)e.getNewValue();        
        this.p200_stairs40.setText(String.format("%.2f", current_value.doubleValue()));
    }
   if(propertyName.matches("P200_temp_stairs40")){
        java.lang.Double current_value = (java.lang.Double)e.getNewValue();        
        this.p200_stairs60.setText(String.format("%.2f", current_value.doubleValue()));
    }
   if(propertyName.matches("P200_temp_stairs60")){
        java.lang.Double current_value = (java.lang.Double)e.getNewValue();        
         this.p200_stairs75.setText(String.format("%.2f", current_value.doubleValue()));
   }
   if(propertyName.matches("P200_temp_stairs75")){
        java.lang.Double current_value = (java.lang.Double)e.getNewValue();        
        this.p200_stairs0.setText(String.format("%.2f", current_value.doubleValue()));
    }
   if(propertyName.matches("P200_temp_stairs90")){
        java.lang.Double current_value = (java.lang.Double)e.getNewValue();        
        this.p200_stairs90.setText(String.format("%.2f", current_value.doubleValue()));
    }
   if(propertyName.matches("P200_temp_gantryUR")){
        java.lang.Double current_value = (java.lang.Double)e.getNewValue();  
        this.p200_gantry_ur.setText(String.format("%.2f", current_value.doubleValue()));
    }
  if(propertyName.matches("P200_temp_gantryUL")){
        java.lang.Double current_value = (java.lang.Double)e.getNewValue();        
        this.p200_gantryul.setText(String.format("%.2f", current_value.doubleValue()));
    }
  if(propertyName.matches("P200_temp_gantryLR")){
        java.lang.Double current_value = (java.lang.Double)e.getNewValue();        
        this.p200_gantrylr.setText(String.format("%.2f", current_value.doubleValue()));
    }
  if(propertyName.matches("P200_temp_gantryLL")){
        java.lang.Double current_value = (java.lang.Double)e.getNewValue();        
        this.p200_gantryll.setText(String.format("%.2f", current_value.doubleValue()));
    }
  if(propertyName.matches("P200_temp_gantry_farR")){
        java.lang.Double current_value = (java.lang.Double)e.getNewValue(); 
        this.p200_grantry_far_r.setText(String.format("%.2f", current_value.doubleValue()));
    }
  if(propertyName.matches("P200_temp_gantry_farL")){
        java.lang.Double current_value = (java.lang.Double)e.getNewValue();   
        this.p200_grantry_far_l.setText(String.format("%.2f", current_value.doubleValue()));
    }
  if(propertyName.matches("P200_temp_crane")){
        java.lang.Double current_value = (java.lang.Double)e.getNewValue(); 
        this.p200_crane.setText(String.format("%.2f", current_value.doubleValue()));
    }
  if(propertyName.matches("P200_temp_exhaust")){
        java.lang.Double current_value = (java.lang.Double)e.getNewValue(); 
        this.p200_exhaust.setText(String.format("%.2f", current_value.doubleValue()));
    }
  if(propertyName.matches("P200_temp_shutter")){
        java.lang.Double current_value = (java.lang.Double)e.getNewValue();    
        this.p200_shutter.setText(String.format("%.1f", current_value.doubleValue()));
    }
  if(propertyName.matches("P200_temp_fanR")){
        java.lang.Double current_value = (java.lang.Double)e.getNewValue();    
        this.p200_fanr.setText(String.format("%.2f", current_value.doubleValue()));
    }
  if(propertyName.matches("P200_temp_fanL")){
        java.lang.Double current_value = (java.lang.Double)e.getNewValue(); 
        this.p200_fanl.setText(String.format("%.2f", current_value.doubleValue()));
    }
  if(propertyName.matches("P200_temp_catwalk")){
        java.lang.Double current_value = (java.lang.Double)e.getNewValue();   
        this.p200_catwalk.setText(String.format("%.2f", current_value.doubleValue()));
    }
  if(propertyName.matches("P200_temp_ArchR")){
        java.lang.Double current_value = (java.lang.Double)e.getNewValue();  
        this.p200_arch_r.setText(String.format("%.2f", current_value.doubleValue()));
    }
  if(propertyName.matches("P200_temp_ArchL")){
        java.lang.Double current_value = (java.lang.Double)e.getNewValue(); 
        this.p200_arch_l.setText(String.format("%.2f", current_value.doubleValue()));
    }
  if(propertyName.matches("p200_utc_timestamp")){
        java.lang.String current_value = (java.lang.String)e.getNewValue(); 
        p200_utc_timestamp.setText(current_value);
  }
} 
/*=============================================================================================
/     Property Change Listener for the Ephemeris_propertyChange
/=============================================================================================*/
  private void p60weather_propertyChange(PropertyChangeEvent e)  {
     java.lang.String propertyName = e.getPropertyName();
/*=============================================================================================
/    PARAMETERS FROM THE JSKYCALCMODEL
/=============================================================================================*/
    if(propertyName.matches("P60_outside_air_temp")){
        java.lang.Double current_value = (java.lang.Double)e.getNewValue();  
        this.P60_outside_temperature.setText(String.format("%.3f", current_value.doubleValue()));
    }
    if(propertyName.matches("P60_dome_air_ttemp")){
        java.lang.Double current_value = (java.lang.Double)e.getNewValue();   
        this.P60_inside_temperature.setText(String.format("%.3f", current_value.doubleValue()));
    }    
    if(propertyName.matches("P60_floor_temp")){
        java.lang.Double current_value = (java.lang.Double)e.getNewValue(); 
        this.P60_floor_temperature.setText(String.format("%.3f", current_value.doubleValue()));
    }    
    if(propertyName.matches("P60_inside_dewpoint")){
        java.lang.Double current_value = (java.lang.Double)e.getNewValue(); 
        this.P60_inside_dewpoint.setText(String.format("%.3f", current_value.doubleValue()));
    }    
    if(propertyName.matches("P60_outside_dewpoint")){
        java.lang.Double current_value = (java.lang.Double)e.getNewValue();  
        this.P60_outside_dewpoint.setText(String.format("%.3f", current_value.doubleValue()));
    }    
    if(propertyName.matches("P60_tube_air_temperature")){
        java.lang.Double current_value = (java.lang.Double)e.getNewValue(); 
        this.P60_tube_air_temperature.setText(String.format("%.3f", current_value.doubleValue()));
    }
   if(propertyName.matches("P60_tube_air_temperature_top")){
        java.lang.Double current_value = (java.lang.Double)e.getNewValue();  
        this.p60_tube_top_temperature.setText(String.format("%.3f", current_value.doubleValue()));
    }
   if(propertyName.matches("P60_tube_air_temperature_middle")){
        java.lang.Double current_value = (java.lang.Double)e.getNewValue(); 
        this.P60_tube_middle_temperature.setText(String.format("%.3f", current_value.doubleValue()));
    }   
   if(propertyName.matches("P60_tube_air_temperature_bottom")){
        java.lang.Double current_value = (java.lang.Double)e.getNewValue(); 
        this.P60_tube_bottom_temperature.setText(String.format("%.3f", current_value.doubleValue()));
    }
   if(propertyName.matches("P60_primary_mirror_temp")){
        java.lang.Double current_value = (java.lang.Double)e.getNewValue(); 
        this.P60_mirror_temperature.setText(String.format("%.3f", current_value.doubleValue()));
    }
   if(propertyName.matches("P60_primary_cell_temp")){
        java.lang.Double current_value = (java.lang.Double)e.getNewValue();  
        this.P60_primary_cell_temperature.setText(String.format("%.3f", current_value.doubleValue()));
    }
   if(propertyName.matches("P60_secondary_cell_temp")){
        java.lang.Double current_value = (java.lang.Double)e.getNewValue();     
        this.P60_secondary_cell_temperature.setText(String.format("%.3f", current_value.doubleValue()));
    }
   if(propertyName.matches("P60_outside_relative_humidity")){
        java.lang.Double current_value = (java.lang.Double)e.getNewValue();   
        this.P60_outside_relative_humidity.setText(String.format("%.3f", current_value.doubleValue()));
    }
   if(propertyName.matches("P60_inside_relative_humidity")){
        java.lang.Double current_value = (java.lang.Double)e.getNewValue();   
        this.P60_inside_relative_humidity.setText(String.format("%.3f", current_value.doubleValue()));
    }
   if(propertyName.matches("P60_barometric_pressure")){
        java.lang.Double current_value = (java.lang.Double)e.getNewValue();
        this.P60_barometric_pressure.setText(String.format("%.3f", current_value.doubleValue()));
    }
   if(propertyName.matches("raining")){
        java.lang.Boolean current_value = (java.lang.Boolean)e.getNewValue(); 
        if(current_value.booleanValue()){
            this.P60_raining.setText("RAINING");
        }
        if(!current_value.booleanValue()){
            this.P60_raining.setText("NONE");            
        }
    }
   if(propertyName.matches("P60_wind_direction")){
        java.lang.Double current_value = (java.lang.Double)e.getNewValue();  
        this.P60_wind_direction.setText(String.format("%.3f", current_value.doubleValue()));
    }
   if(propertyName.matches("P60_windspeed_median")){
        java.lang.Double current_value = (java.lang.Double)e.getNewValue();  
        this.P60_wind_median.setText(String.format("%.3f", current_value.doubleValue()));
    }
  if(propertyName.matches("P60_windspeed_peak")){
        java.lang.Double current_value = (java.lang.Double)e.getNewValue();
        this.P60_wind_gusts.setText(String.format("%.3f", current_value.doubleValue()));
    }
  if(propertyName.matches("p60_utc_timestamp")){
        java.lang.String current_value = (java.lang.String)e.getNewValue(); 
        p60_utc_timestamp.setText(current_value);      
  }
} 
/*=============================================================================================
/     Property Change Listener for the Ephemeris_propertyChange
/=============================================================================================*/
  private void p48weather_propertyChange(PropertyChangeEvent e)  {
     java.lang.String propertyName = e.getPropertyName();
     System.out.println(propertyName);
/*=============================================================================================
/    PARAMETERS FROM THE JSKYCALCMODEL
/=============================================================================================*/
    if(propertyName.matches("P48_Windspeed_Avg_Threshold")){
        java.lang.Float current_value = (java.lang.Float)e.getNewValue();   
        this.P48_average_windspeed_threshold.setText(String.format("%.3f", current_value.doubleValue()));
    }
    if(propertyName.matches("P48_Gust_Speed_Threshold")){
        java.lang.Float current_value = (java.lang.Float)e.getNewValue(); 
        this.P48_peak_windspeed_threshold.setText(String.format("%.3f", current_value.doubleValue()));
    }    
    if(propertyName.matches("P48_Alarm_Hold_Time")){
        java.lang.Float current_value = (java.lang.Float)e.getNewValue();  
        this.p48_alarm_holdtime.setText(String.format("%.3f", current_value.doubleValue()));
    }    
    if(propertyName.matches("P48_Remaining_Hold_Time")){
        java.lang.Float current_value = (java.lang.Float)e.getNewValue();  
        this.P48_remainging_holdtime.setText(String.format("%.3f", current_value.doubleValue()));
    }    
    if(propertyName.matches("P48_Outside_DewPt_Threshold")){
        java.lang.Float current_value = (java.lang.Float)e.getNewValue();  
        this.P48_outside_dewpoint_threshold.setText(String.format("%.3f", current_value.doubleValue()));
    }    
    if(propertyName.matches("P48_Inside_DewPt_Threshold")){
        java.lang.Float current_value = (java.lang.Float)e.getNewValue();  
        this.P48_inside_dewpoint_threshold.setText(String.format("%.3f", current_value.doubleValue()));
    }
   if(propertyName.matches("P48_Wind_Dir_Current")){
        java.lang.Float current_value = (java.lang.Float)e.getNewValue();  
        this.P48_current_wind_direction.setText(String.format("%.3f", current_value.doubleValue()));
    }
   if(propertyName.matches("P48_Windspeed_Current")){
        java.lang.Float current_value = (java.lang.Float)e.getNewValue();  
        this.P48_current_windspeed.setText(String.format("%.3f", current_value.doubleValue()));
    }   
   if(propertyName.matches("P48_Windspeed_Average")){
        java.lang.Float current_value = (java.lang.Float)e.getNewValue();  
        this.P48_windspeed_average.setText(String.format("%.3f", current_value.doubleValue()));
    }
   if(propertyName.matches("P48_Outside_Air_Temp")){
        java.lang.Float current_value = (java.lang.Float)e.getNewValue();  
        this.P48_outside_air_temperature.setText(String.format("%.3f", current_value.doubleValue()));
    }
   if(propertyName.matches("P48_Outside_Rel_Hum")){
        java.lang.Float current_value = (java.lang.Float)e.getNewValue();  
        this.P48_outside_relative_humidity.setText(String.format("%.3f", current_value.doubleValue()));
    }
   if(propertyName.matches("P48_Outside_DewPt")){
        java.lang.Float current_value = (java.lang.Float)e.getNewValue();  
        this.P48_outside_dewpoint.setText(String.format("%.3f", current_value.doubleValue()));
    }
   if(propertyName.matches("P48_Inside_Air_Temp")){
        java.lang.Float current_value = (java.lang.Float)e.getNewValue();  
        this.P48_inside_air_temperature.setText(String.format("%.3f", current_value.doubleValue()));
    }
   if(propertyName.matches("P48_Inside_Rel_Hum")){
        java.lang.Float current_value = (java.lang.Float)e.getNewValue();  
        this.P48_inside_relative_humidity.setText(String.format("%.3f", current_value.doubleValue()));
    }
   if(propertyName.matches("P48_Inside_DewPt")){
        java.lang.Float current_value = (java.lang.Float)e.getNewValue();  
        this.P48_inside_dewpoint.setText(String.format("%.3f", current_value.doubleValue()));
    }
   if(propertyName.matches("P48_Wetness")){
       java.lang.String current_value = (java.lang.String)e.getNewValue();  
        this.P48_wetness.setText(current_value);
    }
   if(propertyName.matches("P48_Weather_Status")){
        java.lang.String current_value = (java.lang.String)e.getNewValue();  
        this.P48_status.setText(current_value);
    }
   if(propertyName.matches("P48_UTC")){
        java.lang.String current_value = (java.lang.String)e.getNewValue();  
         p48_utc_timestamp.setText(current_value);            
   }
} 
     /**
     * This method is called from within the constructor to initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is always
     * regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        jTabbedPane1 = new javax.swing.JTabbedPane();
        jPanel1 = new javax.swing.JPanel();
        jPanel2 = new javax.swing.JPanel();
        FocusTextField = new javax.swing.JTextField();
        HATextField = new javax.swing.JTextField();
        EquinoxTextField = new javax.swing.JTextField();
        AirmassTextField = new javax.swing.JTextField();
        jPanel3 = new javax.swing.JPanel();
        RALabel5 = new javax.swing.JLabel();
        RALabel1 = new javax.swing.JLabel();
        RALabel4 = new javax.swing.JLabel();
        RALabel8 = new javax.swing.JLabel();
        jPanel4 = new javax.swing.JPanel();
        CurrentRATextField = new javax.swing.JTextField();
        OffsetRATextField = new javax.swing.JTextField();
        OffsetDecTextField = new javax.swing.JTextField();
        CurrentDecTextField = new javax.swing.JTextField();
        jPanel6 = new javax.swing.JPanel();
        RALabel2 = new javax.swing.JLabel();
        LSTTextField = new javax.swing.JTextField();
        RALabel = new javax.swing.JLabel();
        UTCTextField = new javax.swing.JTextField();
        titleImageLabel = new javax.swing.JLabel();
        jPanel7 = new javax.swing.JPanel();
        jLabel2 = new javax.swing.JLabel();
        jLabel3 = new javax.swing.JLabel();
        jLabel4 = new javax.swing.JLabel();
        jLabel5 = new javax.swing.JLabel();
        jLabel6 = new javax.swing.JLabel();
        jLabel7 = new javax.swing.JLabel();
        AzimuthTextField = new javax.swing.JTextField();
        ZenithAngleTextField = new javax.swing.JTextField();
        CassRingAngleTextField = new javax.swing.JTextField();
        DomeAzimuthTextField = new javax.swing.JTextField();
        DomeShuttersTextField = new javax.swing.JTextField();
        WindScreensTextField = new javax.swing.JTextField();
        jPanel9 = new javax.swing.JPanel();
        ConnectToTCSToggleButton = new javax.swing.JToggleButton();
        UpdateToggleButton = new javax.swing.JToggleButton();
        jPanel10 = new javax.swing.JPanel();
        RALabel6 = new javax.swing.JLabel();
        RALabel7 = new javax.swing.JLabel();
        RALabel3 = new javax.swing.JLabel();
        jPanel5 = new javax.swing.JPanel();
        DecOffsetTextField = new javax.swing.JTextField();
        RAOffsetTextField = new javax.swing.JTextField();
        jPanel11 = new javax.swing.JPanel();
        RALabel9 = new javax.swing.JLabel();
        RALabel10 = new javax.swing.JLabel();
        TrackRateRATextField = new javax.swing.JTextField();
        TrackRateDecTextField = new javax.swing.JTextField();
        RateUnitsLabel = new javax.swing.JLabel();
        jPanel12 = new javax.swing.JPanel();
        RALabel11 = new javax.swing.JLabel();
        ObjectNameTextField = new javax.swing.JTextField();
        motionStateTextField = new javax.swing.JTextField();
        jPanel8 = new javax.swing.JPanel();
        observatoryparamlabel = new javax.swing.JLabel();
        equinoxlabel = new javax.swing.JLabel();
        airmasslabel = new javax.swing.JLabel();
        azimuthlabel = new javax.swing.JLabel();
        antiparallacticlabel = new javax.swing.JLabel();
        sundeclabel = new javax.swing.JLabel();
        sunazimuthlabel = new javax.swing.JLabel();
        ztwilightlabel = new javax.swing.JLabel();
        lunarskybrightlabel = new javax.swing.JLabel();
        moondeclabel = new javax.swing.JLabel();
        moonazimuthlabel = new javax.swing.JLabel();
        moonillumfraclabel = new javax.swing.JLabel();
        moonphaselabel = new javax.swing.JLabel();
        moonobjanglabel = new javax.swing.JLabel();
        barjdlabel = new javax.swing.JLabel();
        baryvcorlabel = new javax.swing.JLabel();
        constellationlabel = new javax.swing.JLabel();
        planetproximlabel = new javax.swing.JLabel();
        siderialfield = new javax.swing.JTextField();
        airmassfield = new javax.swing.JTextField();
        sunrafield = new javax.swing.JTextField();
        jdfield = new javax.swing.JTextField();
        moonrafield = new javax.swing.JTextField();
        sunaltfield = new javax.swing.JTextField();
        lunarskybrightfield = new javax.swing.JTextField();
        sunazfield = new javax.swing.JTextField();
        moonaltfield = new javax.swing.JTextField();
        moonazfield = new javax.swing.JTextField();
        constellationfield = new javax.swing.JTextField();
        parallacticfield = new javax.swing.JTextField();
        antiparallacticfield = new javax.swing.JTextField();
        moonobjanglefield = new javax.swing.JTextField();
        moonillumfield = new javax.swing.JTextField();
        ztwilightfield = new javax.swing.JTextField();
        moonphasefield = new javax.swing.JTextField();
        planetproximfield = new javax.swing.JTextField();
        jLabel8 = new javax.swing.JLabel();
        altitudefield = new javax.swing.JTextField();
        sundecfield = new javax.swing.JTextField();
        moondecfield = new javax.swing.JTextField();
        baryjdfield = new javax.swing.JTextField();
        baryvcorfield = new javax.swing.JTextField();
        azimuthfield = new javax.swing.JTextField();
        sideriallabel1 = new javax.swing.JLabel();
        declabel = new javax.swing.JLabel();
        jdlabel = new javax.swing.JLabel();
        equinoxfield = new javax.swing.JTextField();
        RAfield = new javax.swing.JTextField();
        altitudelabel = new javax.swing.JLabel();
        sunralabel = new javax.swing.JLabel();
        moonralabel = new javax.swing.JLabel();
        sunaltlabel = new javax.swing.JLabel();
        moonaltlabel = new javax.swing.JLabel();
        parallacticlabel1 = new javax.swing.JLabel();
        ralabel = new javax.swing.JLabel();
        observatorylabel = new javax.swing.JLabel();
        latitudelabel = new javax.swing.JLabel();
        longitudelabel = new javax.swing.JLabel();
        elevsealevellabel = new javax.swing.JLabel();
        sideriallabel9 = new javax.swing.JLabel();
        decfield = new javax.swing.JTextField();
        observatoryfield = new javax.swing.JTextField();
        latitudefield = new javax.swing.JTextField();
        longitudefield = new javax.swing.JTextField();
        elevseafield = new javax.swing.JTextField();
        elevhorizonlabel = new javax.swing.JLabel();
        localdatelabel = new javax.swing.JLabel();
        utdatelabel = new javax.swing.JLabel();
        uttimelabel = new javax.swing.JLabel();
        timelabel = new javax.swing.JLabel();
        elevhorizonfield = new javax.swing.JTextField();
        utdatefield = new javax.swing.JTextField();
        uttimefield = new javax.swing.JTextField();
        localdatefield = new javax.swing.JTextField();
        localtimelabel = new javax.swing.JLabel();
        localtimefield = new javax.swing.JTextField();
        HAlabel1 = new javax.swing.JLabel();
        hafield = new javax.swing.JTextField();
        objectnamelabel = new javax.swing.JLabel();
        objectnamefield = new javax.swing.JTextField();
        timelabel1 = new javax.swing.JLabel();
        jLabel9 = new javax.swing.JLabel();
        jdlabel1 = new javax.swing.JLabel();
        jPanel16 = new javax.swing.JPanel();
        jTabbedPane2 = new javax.swing.JTabbedPane();
        jPanel24 = new javax.swing.JPanel();
        jPanel21 = new javax.swing.JPanel();
        p200_average_windspeed_label31 = new javax.swing.JLabel();
        p200_average_windspeed_label36 = new javax.swing.JLabel();
        p200_utc_timestamp = new javax.swing.JLabel();
        jPanel18 = new javax.swing.JPanel();
        p200_average_windspeed_label = new javax.swing.JLabel();
        p200_average_windspeed_label1 = new javax.swing.JLabel();
        p200_average_windspeed_label2 = new javax.swing.JLabel();
        p200_average_windspeed_label3 = new javax.swing.JLabel();
        p200_average_windspeed_label4 = new javax.swing.JLabel();
        p200_average_windspeed_label5 = new javax.swing.JLabel();
        p200_average_windspeed_label7 = new javax.swing.JLabel();
        p200_average_windspeed_label6 = new javax.swing.JLabel();
        p200_average_windspeed_label8 = new javax.swing.JLabel();
        p200_average_windspeed_label10 = new javax.swing.JLabel();
        p200_average_windspeed_label9 = new javax.swing.JLabel();
        p200_average_windspeed_label27 = new javax.swing.JLabel();
        p200_average_windspeed_label28 = new javax.swing.JLabel();
        p200_average_windspeed_label29 = new javax.swing.JLabel();
        p200_average_windspeed_label30 = new javax.swing.JLabel();
        p200_average_windspeed = new javax.swing.JLabel();
        p200_peak_windspeed = new javax.swing.JLabel();
        p200_gust_windspeed = new javax.swing.JLabel();
        p200_wind_direction = new javax.swing.JLabel();
        p200_dewpoint = new javax.swing.JLabel();
        p200_outside_temperature = new javax.swing.JLabel();
        p200_inside_temperature = new javax.swing.JLabel();
        p200_primary_mirror_temperature = new javax.swing.JLabel();
        p200_secondary_mirror_temperature = new javax.swing.JLabel();
        p200_arch_r = new javax.swing.JLabel();
        p200_floor_temperature = new javax.swing.JLabel();
        p200_raining = new javax.swing.JLabel();
        p200_arch_l = new javax.swing.JLabel();
        p200_grantry_far_r = new javax.swing.JLabel();
        p200_grantry_far_l = new javax.swing.JLabel();
        p200_average_windspeed_label32 = new javax.swing.JLabel();
        p200_primary_cell_temperature = new javax.swing.JLabel();
        jPanel19 = new javax.swing.JPanel();
        p200_average_windspeed_label11 = new javax.swing.JLabel();
        p200_average_windspeed_label12 = new javax.swing.JLabel();
        p200_average_windspeed_label17 = new javax.swing.JLabel();
        p200_average_windspeed_label19 = new javax.swing.JLabel();
        p200_average_windspeed_label18 = new javax.swing.JLabel();
        p200_average_windspeed_label16 = new javax.swing.JLabel();
        p200_average_windspeed_label13 = new javax.swing.JLabel();
        p200_average_windspeed_label20 = new javax.swing.JLabel();
        p200_average_windspeed_label21 = new javax.swing.JLabel();
        p200_average_windspeed_label22 = new javax.swing.JLabel();
        p200_average_windspeed_label14 = new javax.swing.JLabel();
        p200_average_windspeed_label15 = new javax.swing.JLabel();
        p200_average_windspeed_label23 = new javax.swing.JLabel();
        p200_average_windspeed_label24 = new javax.swing.JLabel();
        p200_average_windspeed_label25 = new javax.swing.JLabel();
        p200_average_windspeed_label26 = new javax.swing.JLabel();
        p200_stairs0 = new javax.swing.JLabel();
        p200_stairs20 = new javax.swing.JLabel();
        p200_stairs40 = new javax.swing.JLabel();
        p200_stairs60 = new javax.swing.JLabel();
        p200_stairs75 = new javax.swing.JLabel();
        p200_stairs90 = new javax.swing.JLabel();
        p200_gantry_ur = new javax.swing.JLabel();
        p200_gantrylr = new javax.swing.JLabel();
        p200_gantryul = new javax.swing.JLabel();
        p200_gantryll = new javax.swing.JLabel();
        p200_crane = new javax.swing.JLabel();
        p200_exhaust = new javax.swing.JLabel();
        p200_shutter = new javax.swing.JLabel();
        p200_fanr = new javax.swing.JLabel();
        p200_fanl = new javax.swing.JLabel();
        p200_catwalk = new javax.swing.JLabel();
        jPanel25 = new javax.swing.JPanel();
        jPanel17 = new javax.swing.JPanel();
        p200_average_windspeed_label100 = new javax.swing.JLabel();
        p200_average_windspeed_label101 = new javax.swing.JLabel();
        p60_utc_timestamp = new javax.swing.JLabel();
        jPanel20 = new javax.swing.JPanel();
        p200_average_windspeed_label39 = new javax.swing.JLabel();
        p200_average_windspeed_label40 = new javax.swing.JLabel();
        p200_average_windspeed_label43 = new javax.swing.JLabel();
        p200_average_windspeed_label44 = new javax.swing.JLabel();
        p200_average_windspeed_label47 = new javax.swing.JLabel();
        p200_average_windspeed_label50 = new javax.swing.JLabel();
        P60_tube_top_temperature_label = new javax.swing.JLabel();
        p200_average_windspeed_label71 = new javax.swing.JLabel();
        p200_average_windspeed_label72 = new javax.swing.JLabel();
        P60_outside_temperature = new javax.swing.JLabel();
        P60_inside_temperature = new javax.swing.JLabel();
        P60_floor_temperature = new javax.swing.JLabel();
        P60_outside_dewpoint = new javax.swing.JLabel();
        P60_inside_dewpoint = new javax.swing.JLabel();
        P60_tube_air_temperature = new javax.swing.JLabel();
        p60_tube_top_temperature = new javax.swing.JLabel();
        P60_tube_middle_temperature = new javax.swing.JLabel();
        P60_tube_bottom_temperature = new javax.swing.JLabel();
        p200_average_windspeed_label79 = new javax.swing.JLabel();
        P60_barometric_pressure = new javax.swing.JLabel();
        jPanel27 = new javax.swing.JPanel();
        p200_average_windspeed_label73 = new javax.swing.JLabel();
        p200_average_windspeed_label74 = new javax.swing.JLabel();
        P60_mirror_temperature = new javax.swing.JLabel();
        P60_primary_cell_temperature = new javax.swing.JLabel();
        p200_average_windspeed_label75 = new javax.swing.JLabel();
        P60_secondary_cell_temperature = new javax.swing.JLabel();
        p200_average_windspeed_label76 = new javax.swing.JLabel();
        P60_outside_relative_humidity = new javax.swing.JLabel();
        p200_average_windspeed_label77 = new javax.swing.JLabel();
        P60_inside_relative_humidity = new javax.swing.JLabel();
        p200_average_windspeed_label78 = new javax.swing.JLabel();
        P60_raining = new javax.swing.JLabel();
        p200_average_windspeed_label94 = new javax.swing.JLabel();
        P60_wind_direction = new javax.swing.JLabel();
        p200_average_windspeed_label95 = new javax.swing.JLabel();
        P60_wind_gusts = new javax.swing.JLabel();
        p200_average_windspeed_label96 = new javax.swing.JLabel();
        P60_wind_median = new javax.swing.JLabel();
        jPanel26 = new javax.swing.JPanel();
        jPanel22 = new javax.swing.JPanel();
        p200_average_windspeed_label102 = new javax.swing.JLabel();
        p200_average_windspeed_label103 = new javax.swing.JLabel();
        p48_utc_timestamp = new javax.swing.JLabel();
        jPanel23 = new javax.swing.JPanel();
        p200_average_windspeed_label104 = new javax.swing.JLabel();
        p200_average_windspeed_label105 = new javax.swing.JLabel();
        p200_average_windspeed_label106 = new javax.swing.JLabel();
        p200_average_windspeed_label107 = new javax.swing.JLabel();
        p200_average_windspeed_label108 = new javax.swing.JLabel();
        p200_average_windspeed_label109 = new javax.swing.JLabel();
        p200_average_windspeed_label110 = new javax.swing.JLabel();
        p200_average_windspeed_label111 = new javax.swing.JLabel();
        p200_average_windspeed_label112 = new javax.swing.JLabel();
        p200_average_windspeed_label113 = new javax.swing.JLabel();
        p200_average_windspeed_label114 = new javax.swing.JLabel();
        p200_average_windspeed_label115 = new javax.swing.JLabel();
        p200_average_windspeed_label116 = new javax.swing.JLabel();
        p200_average_windspeed_label117 = new javax.swing.JLabel();
        p200_average_windspeed_label118 = new javax.swing.JLabel();
        P48_average_windspeed_threshold = new javax.swing.JLabel();
        P48_peak_windspeed_threshold = new javax.swing.JLabel();
        p48_alarm_holdtime = new javax.swing.JLabel();
        P48_remainging_holdtime = new javax.swing.JLabel();
        P48_outside_dewpoint_threshold = new javax.swing.JLabel();
        P48_inside_dewpoint_threshold = new javax.swing.JLabel();
        P48_current_wind_direction = new javax.swing.JLabel();
        P48_current_windspeed = new javax.swing.JLabel();
        P48_windspeed_average = new javax.swing.JLabel();
        P48_outside_dewpoint = new javax.swing.JLabel();
        P48_outside_air_temperature = new javax.swing.JLabel();
        P48_outside_relative_humidity = new javax.swing.JLabel();
        P48_inside_air_temperature = new javax.swing.JLabel();
        P48_inside_relative_humidity = new javax.swing.JLabel();
        P48_inside_dewpoint = new javax.swing.JLabel();
        p200_average_windspeed_label134 = new javax.swing.JLabel();
        p200_average_windspeed_label135 = new javax.swing.JLabel();
        P48_wetness = new javax.swing.JLabel();
        P48_status = new javax.swing.JLabel();
        jPanel13 = new javax.swing.JPanel();
        jScrollPane1 = new javax.swing.JScrollPane();
        connectionEditorPane = new javax.swing.JEditorPane();
        jScrollPane2 = new javax.swing.JScrollPane();
        comslogEditorPane = new javax.swing.JEditorPane();
        jLabel10 = new javax.swing.JLabel();
        jLabel11 = new javax.swing.JLabel();
        jScrollPane3 = new javax.swing.JScrollPane();
        errorEditorPane = new javax.swing.JEditorPane();
        jLabel12 = new javax.swing.JLabel();
        jPanel14 = new javax.swing.JPanel();
        jPanel15 = new javax.swing.JPanel();
        startYearSpinner = new javax.swing.JSpinner();
        jLabel13 = new javax.swing.JLabel();
        startDaySpinner = new javax.swing.JSpinner();
        startMonthSpinner = new javax.swing.JSpinner();
        startHourSpinner = new javax.swing.JSpinner();
        startMinuteSpinner = new javax.swing.JSpinner();
        startSecondSpinner = new javax.swing.JSpinner();
        jLabel14 = new javax.swing.JLabel();
        jLabel15 = new javax.swing.JLabel();
        jLabel16 = new javax.swing.JLabel();
        jLabel17 = new javax.swing.JLabel();
        jLabel18 = new javax.swing.JLabel();
        jLabel19 = new javax.swing.JLabel();
        jLabel20 = new javax.swing.JLabel();
        endYearSpinner = new javax.swing.JSpinner();
        endMonthSpinner = new javax.swing.JSpinner();
        endDaySpinner = new javax.swing.JSpinner();
        endHourSpinner = new javax.swing.JSpinner();
        endMinuteSpinner = new javax.swing.JSpinner();
        endSecondSpinner = new javax.swing.JSpinner();
        startDatetimeTextField = new javax.swing.JTextField();
        jLabel21 = new javax.swing.JLabel();
        endDatetimeTextField = new javax.swing.JTextField();
        queryTimestampButton = new javax.swing.JButton();
        queryRangeButton = new javax.swing.JButton();
        browseOutputDirctoryButton = new javax.swing.JButton();
        outputDirectoryTextField = new javax.swing.JTextField();
        jLabel22 = new javax.swing.JLabel();
        filenameTextField = new javax.swing.JTextField();
        exportAsCSVButton = new javax.swing.JButton();
        jLabel23 = new javax.swing.JLabel();
        jScrollPane4 = new javax.swing.JScrollPane();
        queryResultsTable = new javax.swing.JTable();
        jLabel24 = new javax.swing.JLabel();
        jScrollPane5 = new javax.swing.JScrollPane();
        SelectedFITSHeaderEditorPane = new javax.swing.JEditorPane();
        jLabel25 = new javax.swing.JLabel();

        setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);

        jPanel1.setBackground(new java.awt.Color(0, 0, 0));
        jPanel1.setBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 0, 0), 3, true));
        jPanel1.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        jPanel2.setBackground(new java.awt.Color(0, 0, 0));

        FocusTextField.setEditable(false);
        FocusTextField.setBackground(new java.awt.Color(0, 0, 0));
        FocusTextField.setFont(new java.awt.Font("Copperplate Gothic Bold", 3, 18)); // NOI18N
        FocusTextField.setForeground(new java.awt.Color(255, 255, 255));
        FocusTextField.setHorizontalAlignment(javax.swing.JTextField.CENTER);
        FocusTextField.setText("0.0");
        FocusTextField.setBorder(null);
        FocusTextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                FocusTextFieldActionPerformed(evt);
            }
        });

        HATextField.setEditable(false);
        HATextField.setBackground(new java.awt.Color(0, 0, 0));
        HATextField.setFont(new java.awt.Font("Copperplate Gothic Bold", 3, 24)); // NOI18N
        HATextField.setForeground(new java.awt.Color(255, 255, 255));
        HATextField.setHorizontalAlignment(javax.swing.JTextField.CENTER);
        HATextField.setText("0.0");
        HATextField.setBorder(null);
        HATextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                HATextFieldActionPerformed(evt);
            }
        });

        EquinoxTextField.setEditable(false);
        EquinoxTextField.setBackground(new java.awt.Color(0, 0, 0));
        EquinoxTextField.setFont(new java.awt.Font("Copperplate Gothic Bold", 3, 24)); // NOI18N
        EquinoxTextField.setForeground(new java.awt.Color(255, 255, 255));
        EquinoxTextField.setHorizontalAlignment(javax.swing.JTextField.CENTER);
        EquinoxTextField.setText("J2000");
        EquinoxTextField.setBorder(null);
        EquinoxTextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                EquinoxTextFieldActionPerformed(evt);
            }
        });

        AirmassTextField.setEditable(false);
        AirmassTextField.setBackground(new java.awt.Color(0, 0, 0));
        AirmassTextField.setFont(new java.awt.Font("Copperplate Gothic Bold", 3, 24)); // NOI18N
        AirmassTextField.setForeground(new java.awt.Color(255, 255, 255));
        AirmassTextField.setHorizontalAlignment(javax.swing.JTextField.CENTER);
        AirmassTextField.setText("0.0");
        AirmassTextField.setBorder(null);
        AirmassTextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                AirmassTextFieldActionPerformed(evt);
            }
        });

        javax.swing.GroupLayout jPanel2Layout = new javax.swing.GroupLayout(jPanel2);
        jPanel2.setLayout(jPanel2Layout);
        jPanel2Layout.setHorizontalGroup(
            jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel2Layout.createSequentialGroup()
                .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel2Layout.createSequentialGroup()
                        .addGap(46, 46, 46)
                        .addComponent(EquinoxTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 158, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addGroup(jPanel2Layout.createSequentialGroup()
                        .addGap(32, 32, 32)
                        .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(HATextField, javax.swing.GroupLayout.PREFERRED_SIZE, 178, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(AirmassTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 178, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(FocusTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 188, javax.swing.GroupLayout.PREFERRED_SIZE))))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );
        jPanel2Layout.setVerticalGroup(
            jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel2Layout.createSequentialGroup()
                .addGap(10, 10, 10)
                .addComponent(AirmassTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(FocusTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(HATextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(EquinoxTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        jPanel1.add(jPanel2, new org.netbeans.lib.awtextra.AbsoluteConstraints(170, 340, -1, 150));

        jPanel3.setBackground(new java.awt.Color(0, 0, 0));

        RALabel5.setFont(new java.awt.Font("SansSerif", 1, 18)); // NOI18N
        RALabel5.setForeground(new java.awt.Color(0, 204, 204));
        RALabel5.setText("Airmass");

        RALabel1.setFont(new java.awt.Font("SansSerif", 1, 18)); // NOI18N
        RALabel1.setForeground(new java.awt.Color(0, 204, 204));
        RALabel1.setText("Focus");

        RALabel4.setFont(new java.awt.Font("SansSerif", 1, 18)); // NOI18N
        RALabel4.setForeground(new java.awt.Color(0, 204, 204));
        RALabel4.setText("HA");

        RALabel8.setFont(new java.awt.Font("SansSerif", 1, 18)); // NOI18N
        RALabel8.setForeground(new java.awt.Color(0, 204, 204));
        RALabel8.setText("Equinox");

        javax.swing.GroupLayout jPanel3Layout = new javax.swing.GroupLayout(jPanel3);
        jPanel3.setLayout(jPanel3Layout);
        jPanel3Layout.setHorizontalGroup(
            jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel3Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(RALabel1)
                    .addComponent(RALabel4)
                    .addComponent(RALabel5)
                    .addComponent(RALabel8))
                .addContainerGap(52, Short.MAX_VALUE))
        );
        jPanel3Layout.setVerticalGroup(
            jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel3Layout.createSequentialGroup()
                .addGap(20, 20, 20)
                .addComponent(RALabel5)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(RALabel1, javax.swing.GroupLayout.PREFERRED_SIZE, 22, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(RALabel4)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(RALabel8)
                .addContainerGap(18, Short.MAX_VALUE))
        );

        jPanel1.add(jPanel3, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 340, -1, 150));

        jPanel4.setBackground(new java.awt.Color(0, 0, 0));
        jPanel4.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        CurrentRATextField.setBackground(new java.awt.Color(0, 0, 0));
        CurrentRATextField.setEditable(false);
        CurrentRATextField.setFont(new java.awt.Font("Copperplate Gothic Bold", 3, 48)); // NOI18N
        CurrentRATextField.setForeground(new java.awt.Color(0, 255, 0));
        CurrentRATextField.setHorizontalAlignment(javax.swing.JTextField.CENTER);
        CurrentRATextField.setText("00:00:00.0");
        CurrentRATextField.setBorder(null);
        CurrentRATextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                CurrentRATextFieldActionPerformed(evt);
            }
        });
        jPanel4.add(CurrentRATextField, new org.netbeans.lib.awtextra.AbsoluteConstraints(0, 0, 348, -1));

        OffsetRATextField.setBackground(new java.awt.Color(0, 0, 0));
        OffsetRATextField.setEditable(false);
        OffsetRATextField.setFont(new java.awt.Font("Copperplate Gothic Bold", 3, 36)); // NOI18N
        OffsetRATextField.setForeground(new java.awt.Color(0, 255, 0));
        OffsetRATextField.setHorizontalAlignment(javax.swing.JTextField.CENTER);
        OffsetRATextField.setText("00:00:00.0");
        OffsetRATextField.setBorder(null);
        OffsetRATextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                OffsetRATextFieldActionPerformed(evt);
            }
        });
        jPanel4.add(OffsetRATextField, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 70, 298, -1));

        OffsetDecTextField.setBackground(new java.awt.Color(0, 0, 0));
        OffsetDecTextField.setEditable(false);
        OffsetDecTextField.setFont(new java.awt.Font("Copperplate Gothic Bold", 3, 36)); // NOI18N
        OffsetDecTextField.setForeground(new java.awt.Color(0, 255, 0));
        OffsetDecTextField.setHorizontalAlignment(javax.swing.JTextField.CENTER);
        OffsetDecTextField.setText("-00:00:00");
        OffsetDecTextField.setBorder(null);
        OffsetDecTextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                OffsetDecTextFieldActionPerformed(evt);
            }
        });
        jPanel4.add(OffsetDecTextField, new org.netbeans.lib.awtextra.AbsoluteConstraints(360, 70, 400, -1));

        CurrentDecTextField.setBackground(new java.awt.Color(0, 0, 0));
        CurrentDecTextField.setEditable(false);
        CurrentDecTextField.setFont(new java.awt.Font("Copperplate Gothic Bold", 3, 48)); // NOI18N
        CurrentDecTextField.setForeground(new java.awt.Color(0, 255, 0));
        CurrentDecTextField.setHorizontalAlignment(javax.swing.JTextField.CENTER);
        CurrentDecTextField.setText("-00:00:00");
        CurrentDecTextField.setBorder(null);
        CurrentDecTextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                CurrentDecTextFieldActionPerformed(evt);
            }
        });
        jPanel4.add(CurrentDecTextField, new org.netbeans.lib.awtextra.AbsoluteConstraints(360, 0, 392, -1));

        jPanel1.add(jPanel4, new org.netbeans.lib.awtextra.AbsoluteConstraints(120, 190, 800, 120));

        jPanel6.setBackground(new java.awt.Color(0, 0, 0));

        RALabel2.setFont(new java.awt.Font("Engravers MT", 0, 24)); // NOI18N
        RALabel2.setForeground(new java.awt.Color(0, 204, 204));
        RALabel2.setText("UT");

        LSTTextField.setEditable(false);
        LSTTextField.setBackground(new java.awt.Color(0, 0, 0));
        LSTTextField.setFont(new java.awt.Font("Copperplate Gothic Bold", 3, 36)); // NOI18N
        LSTTextField.setForeground(new java.awt.Color(0, 255, 0));
        LSTTextField.setText("00:00:00");
        LSTTextField.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(102, 102, 102)));
        LSTTextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                LSTTextFieldActionPerformed(evt);
            }
        });

        RALabel.setBackground(new java.awt.Color(0, 0, 0));
        RALabel.setFont(new java.awt.Font("Engravers MT", 0, 24)); // NOI18N
        RALabel.setForeground(new java.awt.Color(0, 153, 153));
        RALabel.setText("LST");

        UTCTextField.setEditable(false);
        UTCTextField.setBackground(new java.awt.Color(0, 0, 0));
        UTCTextField.setFont(new java.awt.Font("Copperplate Gothic Bold", 3, 36)); // NOI18N
        UTCTextField.setForeground(new java.awt.Color(0, 255, 0));
        UTCTextField.setText("January  1, 2000");
        UTCTextField.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(102, 102, 102)));
        UTCTextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                UTCTextFieldActionPerformed(evt);
            }
        });

        javax.swing.GroupLayout jPanel6Layout = new javax.swing.GroupLayout(jPanel6);
        jPanel6.setLayout(jPanel6Layout);
        jPanel6Layout.setHorizontalGroup(
            jPanel6Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel6Layout.createSequentialGroup()
                .addGap(8, 8, 8)
                .addComponent(RALabel)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(LSTTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 319, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addGap(18, 18, 18)
                .addComponent(RALabel2)
                .addGap(18, 18, 18)
                .addComponent(UTCTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 407, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(35, Short.MAX_VALUE))
        );
        jPanel6Layout.setVerticalGroup(
            jPanel6Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel6Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel6Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(RALabel)
                    .addComponent(LSTTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(RALabel2)
                    .addComponent(UTCTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap())
        );

        jPanel1.add(jPanel6, new org.netbeans.lib.awtextra.AbsoluteConstraints(30, 120, 890, 60));

        titleImageLabel.setIcon(new javax.swing.ImageIcon(getClass().getResource("/edu/caltech/palomar/telescopes/P200/simulator/resources/PalomarHaleLogo2.jpg"))); // NOI18N
        jPanel1.add(titleImageLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(70, 10, -1, 70));

        jPanel7.setBackground(new java.awt.Color(0, 0, 0));

        jLabel2.setFont(new java.awt.Font("SansSerif", 1, 18)); // NOI18N
        jLabel2.setForeground(new java.awt.Color(0, 204, 204));
        jLabel2.setText("Azimuth");

        jLabel3.setFont(new java.awt.Font("SansSerif", 1, 18)); // NOI18N
        jLabel3.setForeground(new java.awt.Color(0, 204, 204));
        jLabel3.setText("Zenith Angle");

        jLabel4.setFont(new java.awt.Font("SansSerif", 1, 18)); // NOI18N
        jLabel4.setForeground(new java.awt.Color(0, 204, 204));
        jLabel4.setText("Cass Ring Angle");

        jLabel5.setFont(new java.awt.Font("SansSerif", 1, 18)); // NOI18N
        jLabel5.setForeground(new java.awt.Color(0, 204, 204));
        jLabel5.setText("Wind Screen Position");

        jLabel6.setFont(new java.awt.Font("SansSerif", 1, 18)); // NOI18N
        jLabel6.setForeground(new java.awt.Color(0, 204, 204));
        jLabel6.setText("Dome Azimuth");

        jLabel7.setFont(new java.awt.Font("SansSerif", 1, 18)); // NOI18N
        jLabel7.setForeground(new java.awt.Color(0, 204, 204));
        jLabel7.setText("Dome Shutters");

        AzimuthTextField.setBackground(new java.awt.Color(0, 0, 0));
        AzimuthTextField.setFont(new java.awt.Font("Lucida Grande", 1, 18)); // NOI18N
        AzimuthTextField.setForeground(new java.awt.Color(255, 255, 255));
        AzimuthTextField.setHorizontalAlignment(javax.swing.JTextField.CENTER);
        AzimuthTextField.setText("0.0");
        AzimuthTextField.setBorder(null);
        AzimuthTextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                AzimuthTextFieldActionPerformed(evt);
            }
        });

        ZenithAngleTextField.setBackground(new java.awt.Color(0, 0, 0));
        ZenithAngleTextField.setFont(new java.awt.Font("Lucida Grande", 1, 18)); // NOI18N
        ZenithAngleTextField.setForeground(new java.awt.Color(255, 255, 255));
        ZenithAngleTextField.setHorizontalAlignment(javax.swing.JTextField.CENTER);
        ZenithAngleTextField.setText("0.0");
        ZenithAngleTextField.setBorder(null);
        ZenithAngleTextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                ZenithAngleTextFieldActionPerformed(evt);
            }
        });

        CassRingAngleTextField.setBackground(new java.awt.Color(0, 0, 0));
        CassRingAngleTextField.setFont(new java.awt.Font("Lucida Grande", 1, 18)); // NOI18N
        CassRingAngleTextField.setForeground(new java.awt.Color(255, 255, 255));
        CassRingAngleTextField.setHorizontalAlignment(javax.swing.JTextField.CENTER);
        CassRingAngleTextField.setText("0.0");
        CassRingAngleTextField.setBorder(null);
        CassRingAngleTextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                CassRingAngleTextFieldActionPerformed(evt);
            }
        });

        DomeAzimuthTextField.setBackground(new java.awt.Color(0, 0, 0));
        DomeAzimuthTextField.setFont(new java.awt.Font("Lucida Grande", 1, 18)); // NOI18N
        DomeAzimuthTextField.setForeground(new java.awt.Color(255, 255, 255));
        DomeAzimuthTextField.setHorizontalAlignment(javax.swing.JTextField.CENTER);
        DomeAzimuthTextField.setText("0.0");
        DomeAzimuthTextField.setBorder(null);
        DomeAzimuthTextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                DomeAzimuthTextFieldActionPerformed(evt);
            }
        });

        DomeShuttersTextField.setBackground(new java.awt.Color(0, 0, 0));
        DomeShuttersTextField.setFont(new java.awt.Font("Lucida Grande", 1, 18)); // NOI18N
        DomeShuttersTextField.setForeground(new java.awt.Color(255, 255, 255));
        DomeShuttersTextField.setHorizontalAlignment(javax.swing.JTextField.CENTER);
        DomeShuttersTextField.setText("0.0");
        DomeShuttersTextField.setBorder(null);
        DomeShuttersTextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                DomeShuttersTextFieldActionPerformed(evt);
            }
        });

        WindScreensTextField.setBackground(new java.awt.Color(0, 0, 0));
        WindScreensTextField.setFont(new java.awt.Font("Lucida Grande", 1, 18)); // NOI18N
        WindScreensTextField.setForeground(new java.awt.Color(255, 255, 255));
        WindScreensTextField.setHorizontalAlignment(javax.swing.JTextField.CENTER);
        WindScreensTextField.setText("0.0");
        WindScreensTextField.setBorder(null);
        WindScreensTextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                WindScreensTextFieldActionPerformed(evt);
            }
        });

        javax.swing.GroupLayout jPanel7Layout = new javax.swing.GroupLayout(jPanel7);
        jPanel7.setLayout(jPanel7Layout);
        jPanel7Layout.setHorizontalGroup(
            jPanel7Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel7Layout.createSequentialGroup()
                .addGap(20, 20, 20)
                .addGroup(jPanel7Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(jLabel2)
                    .addComponent(jLabel3)
                    .addComponent(jLabel4)
                    .addComponent(jLabel6)
                    .addComponent(jLabel7)
                    .addComponent(jLabel5))
                .addGroup(jPanel7Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                    .addComponent(AzimuthTextField)
                    .addComponent(ZenithAngleTextField)
                    .addComponent(CassRingAngleTextField)
                    .addComponent(DomeAzimuthTextField)
                    .addComponent(DomeShuttersTextField)
                    .addComponent(WindScreensTextField, javax.swing.GroupLayout.DEFAULT_SIZE, 111, Short.MAX_VALUE))
                .addGap(424, 424, 424))
        );
        jPanel7Layout.setVerticalGroup(
            jPanel7Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel7Layout.createSequentialGroup()
                .addGap(0, 0, 0)
                .addGroup(jPanel7Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(jLabel2)
                    .addComponent(AzimuthTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addGap(0, 0, 0)
                .addGroup(jPanel7Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(jLabel3)
                    .addComponent(ZenithAngleTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addGap(0, 0, 0)
                .addGroup(jPanel7Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(jLabel4)
                    .addComponent(CassRingAngleTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addGap(0, 0, 0)
                .addGroup(jPanel7Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(jLabel6)
                    .addComponent(DomeAzimuthTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addGap(0, 0, 0)
                .addGroup(jPanel7Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(jLabel7)
                    .addComponent(DomeShuttersTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addGap(0, 0, 0)
                .addGroup(jPanel7Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(jLabel5)
                    .addComponent(WindScreensTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 22, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        jPanel1.add(jPanel7, new org.netbeans.lib.awtextra.AbsoluteConstraints(440, 340, 382, 150));

        jPanel9.setBackground(new java.awt.Color(0, 0, 0));
        jPanel9.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 144, 0)));

        ConnectToTCSToggleButton.setFont(new java.awt.Font("Lucida Grande", 1, 13)); // NOI18N
        ConnectToTCSToggleButton.setText("Connect To Telescope");
        ConnectToTCSToggleButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                ConnectToTCSToggleButtonActionPerformed(evt);
            }
        });

        UpdateToggleButton.setBackground(java.awt.SystemColor.control);
        UpdateToggleButton.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        UpdateToggleButton.setText("Update");
        UpdateToggleButton.setBorder(javax.swing.BorderFactory.createEtchedBorder());
        UpdateToggleButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                UpdateToggleButtonActionPerformed(evt);
            }
        });

        javax.swing.GroupLayout jPanel9Layout = new javax.swing.GroupLayout(jPanel9);
        jPanel9.setLayout(jPanel9Layout);
        jPanel9Layout.setHorizontalGroup(
            jPanel9Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel9Layout.createSequentialGroup()
                .addGap(6, 6, 6)
                .addComponent(ConnectToTCSToggleButton)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, 61, Short.MAX_VALUE)
                .addComponent(UpdateToggleButton, javax.swing.GroupLayout.PREFERRED_SIZE, 101, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap())
        );
        jPanel9Layout.setVerticalGroup(
            jPanel9Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel9Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel9Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(ConnectToTCSToggleButton)
                    .addComponent(UpdateToggleButton, javax.swing.GroupLayout.PREFERRED_SIZE, 29, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        jPanel1.add(jPanel9, new org.netbeans.lib.awtextra.AbsoluteConstraints(616, 490, 342, 58));

        jPanel10.setBackground(new java.awt.Color(0, 0, 0));

        RALabel6.setBackground(new java.awt.Color(0, 0, 0));
        RALabel6.setFont(new java.awt.Font("SansSerif", 1, 18)); // NOI18N
        RALabel6.setForeground(new java.awt.Color(0, 204, 204));
        RALabel6.setText("Current");

        RALabel7.setBackground(new java.awt.Color(0, 0, 0));
        RALabel7.setFont(new java.awt.Font("SansSerif", 1, 18)); // NOI18N
        RALabel7.setForeground(new java.awt.Color(0, 204, 204));
        RALabel7.setText("Last GO");

        javax.swing.GroupLayout jPanel10Layout = new javax.swing.GroupLayout(jPanel10);
        jPanel10.setLayout(jPanel10Layout);
        jPanel10Layout.setHorizontalGroup(
            jPanel10Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel10Layout.createSequentialGroup()
                .addContainerGap(31, Short.MAX_VALUE)
                .addGroup(jPanel10Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel10Layout.createSequentialGroup()
                        .addComponent(RALabel7)
                        .addGap(20, 20, 20))
                    .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel10Layout.createSequentialGroup()
                        .addComponent(RALabel6)
                        .addContainerGap())))
        );
        jPanel10Layout.setVerticalGroup(
            jPanel10Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel10Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(RALabel6)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, 36, Short.MAX_VALUE)
                .addComponent(RALabel7)
                .addGap(28, 28, 28))
        );

        jPanel1.add(jPanel10, new org.netbeans.lib.awtextra.AbsoluteConstraints(0, 190, -1, 120));

        RALabel3.setFont(new java.awt.Font("SansSerif", 1, 18)); // NOI18N
        RALabel3.setForeground(new java.awt.Color(0, 204, 204));
        RALabel3.setText("Offsets");
        jPanel1.add(RALabel3, new org.netbeans.lib.awtextra.AbsoluteConstraints(30, 310, -1, -1));

        jPanel5.setBackground(new java.awt.Color(0, 0, 0));

        DecOffsetTextField.setEditable(false);
        DecOffsetTextField.setBackground(new java.awt.Color(0, 0, 0));
        DecOffsetTextField.setFont(new java.awt.Font("Copperplate Gothic Bold", 3, 24)); // NOI18N
        DecOffsetTextField.setForeground(new java.awt.Color(0, 255, 0));
        DecOffsetTextField.setHorizontalAlignment(javax.swing.JTextField.CENTER);
        DecOffsetTextField.setText("0.0");
        DecOffsetTextField.setBorder(null);
        DecOffsetTextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                DecOffsetTextFieldActionPerformed(evt);
            }
        });

        RAOffsetTextField.setEditable(false);
        RAOffsetTextField.setBackground(new java.awt.Color(0, 0, 0));
        RAOffsetTextField.setFont(new java.awt.Font("Copperplate Gothic Bold", 3, 24)); // NOI18N
        RAOffsetTextField.setForeground(new java.awt.Color(0, 255, 0));
        RAOffsetTextField.setHorizontalAlignment(javax.swing.JTextField.CENTER);
        RAOffsetTextField.setText("0.0");
        RAOffsetTextField.setBorder(null);
        RAOffsetTextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                RAOffsetTextFieldActionPerformed(evt);
            }
        });

        javax.swing.GroupLayout jPanel5Layout = new javax.swing.GroupLayout(jPanel5);
        jPanel5.setLayout(jPanel5Layout);
        jPanel5Layout.setHorizontalGroup(
            jPanel5Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel5Layout.createSequentialGroup()
                .addGap(86, 86, 86)
                .addComponent(RAOffsetTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 162, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, 258, Short.MAX_VALUE)
                .addComponent(DecOffsetTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 162, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addGap(132, 132, 132))
        );
        jPanel5Layout.setVerticalGroup(
            jPanel5Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel5Layout.createSequentialGroup()
                .addGroup(jPanel5Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(RAOffsetTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(DecOffsetTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        jPanel1.add(jPanel5, new org.netbeans.lib.awtextra.AbsoluteConstraints(120, 310, 800, 33));

        jPanel11.setBackground(new java.awt.Color(16, 13, 9));
        jPanel11.setBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 144, 0), 1, true));

        RALabel9.setFont(new java.awt.Font("SansSerif", 1, 18)); // NOI18N
        RALabel9.setForeground(new java.awt.Color(0, 204, 204));
        RALabel9.setText("Tracking Rates:   RA =");

        RALabel10.setFont(new java.awt.Font("SansSerif", 1, 18)); // NOI18N
        RALabel10.setForeground(new java.awt.Color(0, 204, 204));
        RALabel10.setText("Dec =");

        TrackRateRATextField.setBackground(new java.awt.Color(0, 0, 0));
        TrackRateRATextField.setFont(new java.awt.Font("Lucida Grande", 1, 18)); // NOI18N
        TrackRateRATextField.setForeground(new java.awt.Color(255, 255, 255));
        TrackRateRATextField.setHorizontalAlignment(javax.swing.JTextField.CENTER);
        TrackRateRATextField.setText("0.0");
        TrackRateRATextField.setBorder(null);
        TrackRateRATextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                TrackRateRATextFieldActionPerformed(evt);
            }
        });

        TrackRateDecTextField.setBackground(new java.awt.Color(0, 0, 0));
        TrackRateDecTextField.setFont(new java.awt.Font("Lucida Grande", 1, 18)); // NOI18N
        TrackRateDecTextField.setForeground(new java.awt.Color(255, 255, 255));
        TrackRateDecTextField.setHorizontalAlignment(javax.swing.JTextField.CENTER);
        TrackRateDecTextField.setText("0.0");
        TrackRateDecTextField.setBorder(null);
        TrackRateDecTextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                TrackRateDecTextFieldActionPerformed(evt);
            }
        });

        RateUnitsLabel.setFont(new java.awt.Font("DejaVu LGC Sans", 1, 10)); // NOI18N
        RateUnitsLabel.setForeground(new java.awt.Color(5, 221, 59));
        RateUnitsLabel.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        RateUnitsLabel.setText(" ");

        javax.swing.GroupLayout jPanel11Layout = new javax.swing.GroupLayout(jPanel11);
        jPanel11.setLayout(jPanel11Layout);
        jPanel11Layout.setHorizontalGroup(
            jPanel11Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel11Layout.createSequentialGroup()
                .addGap(21, 21, 21)
                .addComponent(RALabel9)
                .addGap(47, 47, 47)
                .addComponent(TrackRateRATextField, javax.swing.GroupLayout.PREFERRED_SIZE, 67, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, 42, Short.MAX_VALUE)
                .addComponent(RALabel10)
                .addGap(31, 31, 31)
                .addComponent(TrackRateDecTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 74, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addGap(45, 45, 45))
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel11Layout.createSequentialGroup()
                .addGap(274, 274, 274)
                .addComponent(RateUnitsLabel, javax.swing.GroupLayout.DEFAULT_SIZE, 302, Short.MAX_VALUE)
                .addContainerGap())
        );
        jPanel11Layout.setVerticalGroup(
            jPanel11Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel11Layout.createSequentialGroup()
                .addGap(12, 12, 12)
                .addGroup(jPanel11Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(TrackRateDecTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 22, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(RALabel10)
                    .addComponent(TrackRateRATextField, javax.swing.GroupLayout.PREFERRED_SIZE, 22, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(RALabel9))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addComponent(RateUnitsLabel)
                .addContainerGap())
        );

        jPanel1.add(jPanel11, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 490, 590, 60));

        jPanel12.setBackground(new java.awt.Color(9, 7, 4));

        RALabel11.setFont(new java.awt.Font("Engravers MT", 0, 24)); // NOI18N
        RALabel11.setForeground(new java.awt.Color(0, 204, 204));
        RALabel11.setText("Current Object:");

        ObjectNameTextField.setEditable(false);
        ObjectNameTextField.setBackground(new java.awt.Color(0, 0, 0));
        ObjectNameTextField.setFont(new java.awt.Font("Copperplate Gothic Bold", 3, 36)); // NOI18N
        ObjectNameTextField.setForeground(new java.awt.Color(0, 255, 0));
        ObjectNameTextField.setHorizontalAlignment(javax.swing.JTextField.CENTER);
        ObjectNameTextField.setText(" ");
        ObjectNameTextField.setBorder(null);
        ObjectNameTextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                ObjectNameTextFieldActionPerformed(evt);
            }
        });

        javax.swing.GroupLayout jPanel12Layout = new javax.swing.GroupLayout(jPanel12);
        jPanel12.setLayout(jPanel12Layout);
        jPanel12Layout.setHorizontalGroup(
            jPanel12Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel12Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(RALabel11)
                .addGap(18, 18, 18)
                .addComponent(ObjectNameTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 253, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(37, Short.MAX_VALUE))
        );
        jPanel12Layout.setVerticalGroup(
            jPanel12Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel12Layout.createSequentialGroup()
                .addGroup(jPanel12Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(RALabel11)
                    .addComponent(ObjectNameTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 31, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        jPanel1.add(jPanel12, new org.netbeans.lib.awtextra.AbsoluteConstraints(180, 90, 500, 30));

        motionStateTextField.setEditable(false);
        motionStateTextField.setBackground(new java.awt.Color(0, 0, 0));
        motionStateTextField.setFont(new java.awt.Font("Copperplate Gothic Bold", 3, 18)); // NOI18N
        motionStateTextField.setForeground(new java.awt.Color(0, 255, 0));
        motionStateTextField.setHorizontalAlignment(javax.swing.JTextField.CENTER);
        motionStateTextField.setText(" ");
        motionStateTextField.setBorder(null);
        motionStateTextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                motionStateTextFieldActionPerformed(evt);
            }
        });
        jPanel1.add(motionStateTextField, new org.netbeans.lib.awtextra.AbsoluteConstraints(678, 87, 200, 34));

        jTabbedPane1.addTab("P200 Telescope ", jPanel1);

        jPanel8.setBackground(new java.awt.Color(0, 0, 0));
        jPanel8.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(51, 153, 0), 5));
        jPanel8.setForeground(new java.awt.Color(51, 255, 0));

        observatoryparamlabel.setFont(new java.awt.Font("Arial", 3, 18)); // NOI18N
        observatoryparamlabel.setForeground(new java.awt.Color(0, 255, 255));
        observatoryparamlabel.setText("Observatory Parameters");

        equinoxlabel.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        equinoxlabel.setForeground(new java.awt.Color(0, 255, 255));
        equinoxlabel.setText("Equinox");

        airmasslabel.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        airmasslabel.setForeground(new java.awt.Color(0, 255, 255));
        airmasslabel.setText("Airmass");

        azimuthlabel.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        azimuthlabel.setForeground(new java.awt.Color(0, 255, 255));
        azimuthlabel.setText("Azimuth");

        antiparallacticlabel.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        antiparallacticlabel.setForeground(new java.awt.Color(0, 255, 255));
        antiparallacticlabel.setText("antiparallactic");

        sundeclabel.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        sundeclabel.setForeground(new java.awt.Color(0, 255, 255));
        sundeclabel.setText("Sun dec");

        sunazimuthlabel.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        sunazimuthlabel.setForeground(new java.awt.Color(0, 255, 255));
        sunazimuthlabel.setText("Sun Azimuth");

        ztwilightlabel.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        ztwilightlabel.setForeground(new java.awt.Color(0, 255, 255));
        ztwilightlabel.setText("ZTwiight");

        lunarskybrightlabel.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        lunarskybrightlabel.setForeground(new java.awt.Color(0, 255, 255));
        lunarskybrightlabel.setText("Lunar Sky Bright");

        moondeclabel.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        moondeclabel.setForeground(new java.awt.Color(0, 255, 255));
        moondeclabel.setText("Moon dec");

        moonazimuthlabel.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        moonazimuthlabel.setForeground(new java.awt.Color(0, 255, 255));
        moonazimuthlabel.setText("Moon Azimuth");

        moonillumfraclabel.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        moonillumfraclabel.setForeground(new java.awt.Color(0, 255, 255));
        moonillumfraclabel.setText("Moon Illum Frac");

        moonphaselabel.setFont(new java.awt.Font("Arial", 1, 18)); // NOI18N
        moonphaselabel.setForeground(new java.awt.Color(0, 255, 255));
        moonphaselabel.setText("Moon Phase");

        moonobjanglabel.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        moonobjanglabel.setForeground(new java.awt.Color(0, 255, 255));
        moonobjanglabel.setText("Moon-Object angle");

        barjdlabel.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        barjdlabel.setForeground(new java.awt.Color(0, 255, 255));
        barjdlabel.setText("Bary. JD");

        baryvcorlabel.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        baryvcorlabel.setForeground(new java.awt.Color(0, 255, 255));
        baryvcorlabel.setText("Bary. Vcorrn");

        constellationlabel.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        constellationlabel.setForeground(new java.awt.Color(0, 255, 255));
        constellationlabel.setText("Constellation");

        planetproximlabel.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        planetproximlabel.setForeground(new java.awt.Color(0, 255, 255));
        planetproximlabel.setText("Planet Warning");

        siderialfield.setEditable(false);
        siderialfield.setBackground(new java.awt.Color(0, 0, 0));
        siderialfield.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        siderialfield.setForeground(new java.awt.Color(51, 255, 0));
        siderialfield.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));

        airmassfield.setEditable(false);
        airmassfield.setBackground(new java.awt.Color(0, 0, 0));
        airmassfield.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        airmassfield.setForeground(new java.awt.Color(51, 255, 0));
        airmassfield.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));

        sunrafield.setEditable(false);
        sunrafield.setBackground(new java.awt.Color(0, 0, 0));
        sunrafield.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        sunrafield.setForeground(new java.awt.Color(51, 255, 0));
        sunrafield.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));

        jdfield.setEditable(false);
        jdfield.setBackground(new java.awt.Color(0, 0, 0));
        jdfield.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        jdfield.setForeground(new java.awt.Color(51, 255, 0));
        jdfield.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));

        moonrafield.setEditable(false);
        moonrafield.setBackground(new java.awt.Color(0, 0, 0));
        moonrafield.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        moonrafield.setForeground(new java.awt.Color(51, 255, 0));
        moonrafield.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));

        sunaltfield.setEditable(false);
        sunaltfield.setBackground(new java.awt.Color(0, 0, 0));
        sunaltfield.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        sunaltfield.setForeground(new java.awt.Color(51, 255, 0));
        sunaltfield.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));

        lunarskybrightfield.setEditable(false);
        lunarskybrightfield.setBackground(new java.awt.Color(0, 0, 0));
        lunarskybrightfield.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        lunarskybrightfield.setForeground(new java.awt.Color(51, 255, 0));
        lunarskybrightfield.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));

        sunazfield.setEditable(false);
        sunazfield.setBackground(new java.awt.Color(0, 0, 0));
        sunazfield.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        sunazfield.setForeground(new java.awt.Color(51, 255, 0));
        sunazfield.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));

        moonaltfield.setEditable(false);
        moonaltfield.setBackground(new java.awt.Color(0, 0, 0));
        moonaltfield.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        moonaltfield.setForeground(new java.awt.Color(51, 255, 0));
        moonaltfield.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));

        moonazfield.setEditable(false);
        moonazfield.setBackground(new java.awt.Color(0, 0, 0));
        moonazfield.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        moonazfield.setForeground(new java.awt.Color(51, 255, 0));
        moonazfield.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));

        constellationfield.setEditable(false);
        constellationfield.setBackground(new java.awt.Color(0, 0, 0));
        constellationfield.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        constellationfield.setForeground(new java.awt.Color(51, 255, 0));
        constellationfield.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));

        parallacticfield.setEditable(false);
        parallacticfield.setBackground(new java.awt.Color(0, 0, 0));
        parallacticfield.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        parallacticfield.setForeground(new java.awt.Color(51, 255, 0));
        parallacticfield.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));

        antiparallacticfield.setEditable(false);
        antiparallacticfield.setBackground(new java.awt.Color(0, 0, 0));
        antiparallacticfield.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        antiparallacticfield.setForeground(new java.awt.Color(51, 255, 0));
        antiparallacticfield.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));

        moonobjanglefield.setEditable(false);
        moonobjanglefield.setBackground(new java.awt.Color(0, 0, 0));
        moonobjanglefield.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        moonobjanglefield.setForeground(new java.awt.Color(51, 255, 0));
        moonobjanglefield.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));

        moonillumfield.setEditable(false);
        moonillumfield.setBackground(new java.awt.Color(0, 0, 0));
        moonillumfield.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        moonillumfield.setForeground(new java.awt.Color(51, 255, 0));
        moonillumfield.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));

        ztwilightfield.setEditable(false);
        ztwilightfield.setBackground(new java.awt.Color(0, 0, 0));
        ztwilightfield.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        ztwilightfield.setForeground(new java.awt.Color(51, 255, 0));
        ztwilightfield.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));

        moonphasefield.setEditable(false);
        moonphasefield.setBackground(new java.awt.Color(0, 0, 0));
        moonphasefield.setFont(new java.awt.Font("Arial", 1, 18)); // NOI18N
        moonphasefield.setForeground(new java.awt.Color(51, 255, 0));
        moonphasefield.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));

        planetproximfield.setEditable(false);
        planetproximfield.setBackground(new java.awt.Color(0, 0, 0));
        planetproximfield.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        planetproximfield.setForeground(new java.awt.Color(51, 255, 0));
        planetproximfield.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));

        jLabel8.setFont(new java.awt.Font("Arial", 1, 18)); // NOI18N
        jLabel8.setIcon(new javax.swing.ImageIcon(getClass().getResource("/edu/caltech/palomar/telescopes/P200/simulator/resources/JThorstenson4.jpg"))); // NOI18N

        altitudefield.setEditable(false);
        altitudefield.setBackground(new java.awt.Color(0, 0, 0));
        altitudefield.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        altitudefield.setForeground(new java.awt.Color(51, 255, 0));
        altitudefield.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));

        sundecfield.setEditable(false);
        sundecfield.setBackground(new java.awt.Color(0, 0, 0));
        sundecfield.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        sundecfield.setForeground(new java.awt.Color(51, 255, 0));
        sundecfield.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));

        moondecfield.setEditable(false);
        moondecfield.setBackground(new java.awt.Color(0, 0, 0));
        moondecfield.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        moondecfield.setForeground(new java.awt.Color(51, 255, 0));
        moondecfield.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));

        baryjdfield.setEditable(false);
        baryjdfield.setBackground(new java.awt.Color(0, 0, 0));
        baryjdfield.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        baryjdfield.setForeground(new java.awt.Color(51, 255, 0));
        baryjdfield.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));

        baryvcorfield.setEditable(false);
        baryvcorfield.setBackground(new java.awt.Color(0, 0, 0));
        baryvcorfield.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        baryvcorfield.setForeground(new java.awt.Color(51, 255, 0));
        baryvcorfield.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));

        azimuthfield.setEditable(false);
        azimuthfield.setBackground(new java.awt.Color(0, 0, 0));
        azimuthfield.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        azimuthfield.setForeground(new java.awt.Color(51, 255, 0));
        azimuthfield.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));

        sideriallabel1.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        sideriallabel1.setForeground(new java.awt.Color(0, 255, 255));
        sideriallabel1.setText("Siderial Time");

        declabel.setFont(new java.awt.Font("Arial", 1, 18)); // NOI18N
        declabel.setForeground(new java.awt.Color(0, 255, 255));
        declabel.setText("Declination");

        jdlabel.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        jdlabel.setForeground(new java.awt.Color(0, 255, 255));
        jdlabel.setText("Julian Date");

        equinoxfield.setEditable(false);
        equinoxfield.setBackground(new java.awt.Color(0, 0, 0));
        equinoxfield.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        equinoxfield.setForeground(new java.awt.Color(51, 255, 0));
        equinoxfield.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));

        RAfield.setEditable(false);
        RAfield.setBackground(new java.awt.Color(0, 0, 0));
        RAfield.setFont(new java.awt.Font("Arial", 1, 18)); // NOI18N
        RAfield.setForeground(new java.awt.Color(0, 255, 0));
        RAfield.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));

        altitudelabel.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        altitudelabel.setForeground(new java.awt.Color(0, 255, 255));
        altitudelabel.setText("Altitude");

        sunralabel.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        sunralabel.setForeground(new java.awt.Color(0, 255, 255));
        sunralabel.setText("Sun RA ");

        moonralabel.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        moonralabel.setForeground(new java.awt.Color(0, 255, 255));
        moonralabel.setText("Moon RA");

        sunaltlabel.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        sunaltlabel.setForeground(new java.awt.Color(0, 255, 255));
        sunaltlabel.setText("Sun Altitude");

        moonaltlabel.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        moonaltlabel.setForeground(new java.awt.Color(0, 255, 255));
        moonaltlabel.setText("Moon Altitude");

        parallacticlabel1.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        parallacticlabel1.setForeground(new java.awt.Color(0, 255, 255));
        parallacticlabel1.setText("parallactic ");

        ralabel.setFont(new java.awt.Font("Arial", 1, 18)); // NOI18N
        ralabel.setForeground(new java.awt.Color(0, 255, 255));
        ralabel.setText("Right Ascension");

        observatorylabel.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        observatorylabel.setForeground(new java.awt.Color(0, 255, 255));
        observatorylabel.setText("Observatory Site");

        latitudelabel.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        latitudelabel.setForeground(new java.awt.Color(0, 255, 255));
        latitudelabel.setText("Latitude");

        longitudelabel.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        longitudelabel.setForeground(new java.awt.Color(0, 255, 255));
        longitudelabel.setText("Longitude");

        elevsealevellabel.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        elevsealevellabel.setForeground(new java.awt.Color(0, 255, 255));
        elevsealevellabel.setText("Elevation: Sea Level");

        sideriallabel9.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        sideriallabel9.setForeground(new java.awt.Color(0, 255, 255));
        sideriallabel9.setText("Elevation: Horizon");

        decfield.setEditable(false);
        decfield.setBackground(new java.awt.Color(0, 0, 0));
        decfield.setFont(new java.awt.Font("Arial", 1, 18)); // NOI18N
        decfield.setForeground(new java.awt.Color(51, 255, 0));
        decfield.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));

        observatoryfield.setEditable(false);
        observatoryfield.setBackground(new java.awt.Color(0, 0, 0));
        observatoryfield.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        observatoryfield.setForeground(new java.awt.Color(51, 255, 0));
        observatoryfield.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));

        latitudefield.setEditable(false);
        latitudefield.setBackground(new java.awt.Color(0, 0, 0));
        latitudefield.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        latitudefield.setForeground(new java.awt.Color(51, 255, 0));
        latitudefield.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));

        longitudefield.setEditable(false);
        longitudefield.setBackground(new java.awt.Color(0, 0, 0));
        longitudefield.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        longitudefield.setForeground(new java.awt.Color(51, 255, 0));
        longitudefield.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));

        elevseafield.setEditable(false);
        elevseafield.setBackground(new java.awt.Color(0, 0, 0));
        elevseafield.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        elevseafield.setForeground(new java.awt.Color(51, 255, 0));
        elevseafield.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));

        elevhorizonlabel.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        elevhorizonlabel.setForeground(new java.awt.Color(0, 255, 255));
        elevhorizonlabel.setText("Elevation: Horizon");

        localdatelabel.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        localdatelabel.setForeground(new java.awt.Color(0, 255, 255));
        localdatelabel.setText("Local Date");

        utdatelabel.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        utdatelabel.setForeground(new java.awt.Color(0, 255, 255));
        utdatelabel.setText("UT Date");

        uttimelabel.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        uttimelabel.setForeground(new java.awt.Color(0, 255, 255));
        uttimelabel.setText("UT Time");

        timelabel.setFont(new java.awt.Font("Arial", 3, 18)); // NOI18N
        timelabel.setForeground(new java.awt.Color(0, 255, 255));
        timelabel.setText("UT and Local Time");

        elevhorizonfield.setEditable(false);
        elevhorizonfield.setBackground(new java.awt.Color(0, 0, 0));
        elevhorizonfield.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        elevhorizonfield.setForeground(new java.awt.Color(51, 255, 0));
        elevhorizonfield.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));

        utdatefield.setEditable(false);
        utdatefield.setBackground(new java.awt.Color(0, 0, 0));
        utdatefield.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        utdatefield.setForeground(new java.awt.Color(51, 255, 0));
        utdatefield.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));

        uttimefield.setEditable(false);
        uttimefield.setBackground(new java.awt.Color(0, 0, 0));
        uttimefield.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        uttimefield.setForeground(new java.awt.Color(51, 255, 0));
        uttimefield.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));

        localdatefield.setEditable(false);
        localdatefield.setBackground(new java.awt.Color(0, 0, 0));
        localdatefield.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        localdatefield.setForeground(new java.awt.Color(51, 255, 0));
        localdatefield.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));

        localtimelabel.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        localtimelabel.setForeground(new java.awt.Color(0, 255, 255));
        localtimelabel.setText("Local Time");

        localtimefield.setEditable(false);
        localtimefield.setBackground(new java.awt.Color(0, 0, 0));
        localtimefield.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        localtimefield.setForeground(new java.awt.Color(51, 255, 0));
        localtimefield.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));

        HAlabel1.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        HAlabel1.setForeground(new java.awt.Color(0, 255, 255));
        HAlabel1.setText("Hour Angle");

        hafield.setEditable(false);
        hafield.setBackground(new java.awt.Color(0, 0, 0));
        hafield.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        hafield.setForeground(new java.awt.Color(51, 255, 0));
        hafield.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));

        objectnamelabel.setFont(new java.awt.Font("Arial", 1, 18)); // NOI18N
        objectnamelabel.setForeground(new java.awt.Color(0, 255, 255));
        objectnamelabel.setText("Object Name");

        objectnamefield.setEditable(false);
        objectnamefield.setBackground(new java.awt.Color(0, 0, 0));
        objectnamefield.setFont(new java.awt.Font("Arial", 1, 24)); // NOI18N
        objectnamefield.setForeground(new java.awt.Color(0, 255, 0));
        objectnamefield.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));

        timelabel1.setFont(new java.awt.Font("Arial", 3, 18)); // NOI18N
        timelabel1.setForeground(new java.awt.Color(0, 255, 255));
        timelabel1.setIcon(new javax.swing.ImageIcon(getClass().getResource("/edu/caltech/palomar/telescopes/P200/simulator/resources/Dartmouth2.jpg"))); // NOI18N

        jLabel9.setIcon(new javax.swing.ImageIcon(getClass().getResource("/edu/caltech/palomar/telescopes/P200/simulator/resources/CaltechAstronomy2.jpg"))); // NOI18N
        jLabel9.setText("jLabel2");

        jdlabel1.setBackground(new java.awt.Color(0, 0, 0));
        jdlabel1.setFont(new java.awt.Font("Arial", 1, 12)); // NOI18N
        jdlabel1.setForeground(new java.awt.Color(255, 255, 255));
        jdlabel1.setText("* values may differ from those reported by the telescope");

        javax.swing.GroupLayout jPanel8Layout = new javax.swing.GroupLayout(jPanel8);
        jPanel8.setLayout(jPanel8Layout);
        jPanel8Layout.setHorizontalGroup(
            jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel8Layout.createSequentialGroup()
                .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel8Layout.createSequentialGroup()
                        .addGap(17, 17, 17)
                        .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(sunralabel)
                            .addComponent(altitudelabel)
                            .addComponent(sideriallabel1)
                            .addComponent(airmasslabel)
                            .addComponent(moonaltlabel)
                            .addComponent(moonillumfraclabel)
                            .addComponent(sunaltlabel)
                            .addComponent(barjdlabel)
                            .addComponent(parallacticlabel1)
                            .addComponent(lunarskybrightlabel)
                            .addComponent(moonralabel)
                            .addComponent(moonphaselabel)
                            .addComponent(constellationlabel)))
                    .addGroup(jPanel8Layout.createSequentialGroup()
                        .addContainerGap()
                        .addComponent(ralabel))
                    .addGroup(jPanel8Layout.createSequentialGroup()
                        .addContainerGap()
                        .addComponent(declabel))
                    .addGroup(jPanel8Layout.createSequentialGroup()
                        .addContainerGap()
                        .addComponent(objectnamelabel)))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                    .addGroup(jPanel8Layout.createSequentialGroup()
                        .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                            .addComponent(siderialfield, javax.swing.GroupLayout.PREFERRED_SIZE, 130, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(airmassfield, javax.swing.GroupLayout.PREFERRED_SIZE, 130, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(altitudefield, javax.swing.GroupLayout.PREFERRED_SIZE, 130, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(sunrafield, javax.swing.GroupLayout.PREFERRED_SIZE, 128, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(moonrafield, javax.swing.GroupLayout.PREFERRED_SIZE, 128, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(sunaltfield, javax.swing.GroupLayout.PREFERRED_SIZE, 128, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(moonaltfield, javax.swing.GroupLayout.PREFERRED_SIZE, 128, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(parallacticfield, javax.swing.GroupLayout.PREFERRED_SIZE, 128, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(baryjdfield, javax.swing.GroupLayout.PREFERRED_SIZE, 128, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(moonillumfield, javax.swing.GroupLayout.PREFERRED_SIZE, 128, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(lunarskybrightfield, javax.swing.GroupLayout.PREFERRED_SIZE, 130, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(constellationfield, javax.swing.GroupLayout.PREFERRED_SIZE, 130, javax.swing.GroupLayout.PREFERRED_SIZE))
                        .addGap(10, 10, 10)
                        .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(equinoxlabel)
                            .addComponent(HAlabel1)
                            .addComponent(sunazimuthlabel)
                            .addComponent(moondeclabel)
                            .addComponent(moonobjanglabel)
                            .addComponent(ztwilightlabel)
                            .addComponent(sundeclabel)
                            .addComponent(moonazimuthlabel)
                            .addComponent(azimuthlabel)
                            .addComponent(antiparallacticlabel)
                            .addComponent(baryvcorlabel)
                            .addComponent(planetproximlabel))
                        .addGap(3, 3, 3)
                        .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(hafield, javax.swing.GroupLayout.PREFERRED_SIZE, 130, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(azimuthfield, javax.swing.GroupLayout.PREFERRED_SIZE, 128, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(sundecfield, javax.swing.GroupLayout.PREFERRED_SIZE, 128, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(moondecfield, javax.swing.GroupLayout.PREFERRED_SIZE, 128, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(sunazfield, javax.swing.GroupLayout.PREFERRED_SIZE, 128, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(moonazfield, javax.swing.GroupLayout.PREFERRED_SIZE, 128, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(antiparallacticfield, javax.swing.GroupLayout.PREFERRED_SIZE, 128, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(baryvcorfield, javax.swing.GroupLayout.PREFERRED_SIZE, 128, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(moonobjanglefield, javax.swing.GroupLayout.PREFERRED_SIZE, 128, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(ztwilightfield, javax.swing.GroupLayout.PREFERRED_SIZE, 130, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(planetproximfield, javax.swing.GroupLayout.PREFERRED_SIZE, 130, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(equinoxfield, javax.swing.GroupLayout.PREFERRED_SIZE, 130, javax.swing.GroupLayout.PREFERRED_SIZE))
                        .addGap(29, 29, 29)
                        .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addGroup(jPanel8Layout.createSequentialGroup()
                                .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                    .addComponent(latitudelabel)
                                    .addComponent(observatorylabel)
                                    .addComponent(sideriallabel9)
                                    .addComponent(elevsealevellabel)
                                    .addComponent(elevhorizonlabel)
                                    .addComponent(longitudelabel))
                                .addGap(11, 11, 11)
                                .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                    .addComponent(observatoryfield, javax.swing.GroupLayout.PREFERRED_SIZE, 140, javax.swing.GroupLayout.PREFERRED_SIZE)
                                    .addComponent(latitudefield, javax.swing.GroupLayout.PREFERRED_SIZE, 140, javax.swing.GroupLayout.PREFERRED_SIZE)
                                    .addComponent(longitudefield, javax.swing.GroupLayout.PREFERRED_SIZE, 140, javax.swing.GroupLayout.PREFERRED_SIZE)
                                    .addComponent(elevseafield, javax.swing.GroupLayout.PREFERRED_SIZE, 140, javax.swing.GroupLayout.PREFERRED_SIZE)
                                    .addComponent(elevhorizonfield, javax.swing.GroupLayout.PREFERRED_SIZE, 140, javax.swing.GroupLayout.PREFERRED_SIZE)
                                    .addComponent(timelabel1, javax.swing.GroupLayout.Alignment.TRAILING)))
                            .addGroup(jPanel8Layout.createSequentialGroup()
                                .addGap(50, 50, 50)
                                .addComponent(timelabel))
                            .addGroup(jPanel8Layout.createSequentialGroup()
                                .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                    .addComponent(utdatelabel)
                                    .addComponent(uttimelabel))
                                .addGap(24, 24, 24)
                                .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                    .addComponent(utdatefield, javax.swing.GroupLayout.PREFERRED_SIZE, 210, javax.swing.GroupLayout.PREFERRED_SIZE)
                                    .addComponent(uttimefield, javax.swing.GroupLayout.PREFERRED_SIZE, 210, javax.swing.GroupLayout.PREFERRED_SIZE)))
                            .addGroup(jPanel8Layout.createSequentialGroup()
                                .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                    .addComponent(localtimelabel)
                                    .addComponent(localdatelabel))
                                .addGap(5, 5, 5)
                                .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                    .addComponent(localdatefield, javax.swing.GroupLayout.PREFERRED_SIZE, 210, javax.swing.GroupLayout.PREFERRED_SIZE)
                                    .addComponent(localtimefield, javax.swing.GroupLayout.PREFERRED_SIZE, 210, javax.swing.GroupLayout.PREFERRED_SIZE)))
                            .addGroup(jPanel8Layout.createSequentialGroup()
                                .addComponent(jdlabel)
                                .addGap(3, 3, 3)
                                .addComponent(jdfield, javax.swing.GroupLayout.PREFERRED_SIZE, 210, javax.swing.GroupLayout.PREFERRED_SIZE))))
                    .addGroup(jPanel8Layout.createSequentialGroup()
                        .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                            .addComponent(objectnamefield)
                            .addComponent(RAfield)
                            .addComponent(decfield, javax.swing.GroupLayout.DEFAULT_SIZE, 328, Short.MAX_VALUE))
                        .addGap(18, 18, 18)
                        .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                            .addGroup(jPanel8Layout.createSequentialGroup()
                                .addComponent(jLabel9, javax.swing.GroupLayout.PREFERRED_SIZE, 88, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(jLabel8))
                            .addComponent(observatoryparamlabel)))
                    .addGroup(jPanel8Layout.createSequentialGroup()
                        .addComponent(moonphasefield, javax.swing.GroupLayout.PREFERRED_SIZE, 311, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(jdlabel1)))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );
        jPanel8Layout.setVerticalGroup(
            jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel8Layout.createSequentialGroup()
                .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel8Layout.createSequentialGroup()
                        .addGap(34, 34, 34)
                        .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                            .addGroup(jPanel8Layout.createSequentialGroup()
                                .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                                    .addComponent(objectnamelabel, javax.swing.GroupLayout.PREFERRED_SIZE, 21, javax.swing.GroupLayout.PREFERRED_SIZE)
                                    .addComponent(objectnamefield, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                                    .addComponent(ralabel, javax.swing.GroupLayout.PREFERRED_SIZE, 21, javax.swing.GroupLayout.PREFERRED_SIZE)
                                    .addComponent(RAfield, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)))
                            .addComponent(jLabel8)
                            .addComponent(timelabel1))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(declabel, javax.swing.GroupLayout.PREFERRED_SIZE, 21, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(decfield, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(observatoryparamlabel, javax.swing.GroupLayout.PREFERRED_SIZE, 21, javax.swing.GroupLayout.PREFERRED_SIZE)))
                    .addGroup(jPanel8Layout.createSequentialGroup()
                        .addGap(28, 28, 28)
                        .addComponent(jLabel9)))
                .addGap(18, 18, 18)
                .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel8Layout.createSequentialGroup()
                        .addGap(21, 21, 21)
                        .addComponent(siderialfield, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addGap(2, 2, 2)
                        .addComponent(airmassfield, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addGap(2, 2, 2)
                        .addComponent(altitudefield, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addGap(2, 2, 2)
                        .addComponent(sunrafield, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addGap(2, 2, 2)
                        .addComponent(moonrafield, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addGap(2, 2, 2)
                        .addComponent(sunaltfield, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addGap(2, 2, 2)
                        .addComponent(moonaltfield, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addGap(2, 2, 2)
                        .addComponent(parallacticfield, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addGap(2, 2, 2)
                        .addComponent(baryjdfield, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addGap(2, 2, 2)
                        .addComponent(moonillumfield, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addGap(2, 2, 2)
                        .addComponent(lunarskybrightfield, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addGap(2, 2, 2)
                        .addComponent(constellationfield, javax.swing.GroupLayout.PREFERRED_SIZE, 20, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addGroup(jPanel8Layout.createSequentialGroup()
                        .addGap(20, 20, 20)
                        .addComponent(equinoxlabel)
                        .addGap(4, 4, 4)
                        .addComponent(HAlabel1)
                        .addGap(4, 4, 4)
                        .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addGroup(jPanel8Layout.createSequentialGroup()
                                .addGap(60, 60, 60)
                                .addComponent(sunazimuthlabel, javax.swing.GroupLayout.PREFERRED_SIZE, 21, javax.swing.GroupLayout.PREFERRED_SIZE))
                            .addGroup(jPanel8Layout.createSequentialGroup()
                                .addGap(40, 40, 40)
                                .addComponent(moondeclabel, javax.swing.GroupLayout.PREFERRED_SIZE, 21, javax.swing.GroupLayout.PREFERRED_SIZE))
                            .addGroup(jPanel8Layout.createSequentialGroup()
                                .addGap(140, 140, 140)
                                .addComponent(moonobjanglabel, javax.swing.GroupLayout.PREFERRED_SIZE, 21, javax.swing.GroupLayout.PREFERRED_SIZE))
                            .addGroup(jPanel8Layout.createSequentialGroup()
                                .addGap(160, 160, 160)
                                .addComponent(ztwilightlabel, javax.swing.GroupLayout.PREFERRED_SIZE, 21, javax.swing.GroupLayout.PREFERRED_SIZE))
                            .addGroup(jPanel8Layout.createSequentialGroup()
                                .addGap(20, 20, 20)
                                .addComponent(sundeclabel, javax.swing.GroupLayout.PREFERRED_SIZE, 21, javax.swing.GroupLayout.PREFERRED_SIZE))
                            .addGroup(jPanel8Layout.createSequentialGroup()
                                .addGap(80, 80, 80)
                                .addComponent(moonazimuthlabel, javax.swing.GroupLayout.PREFERRED_SIZE, 21, javax.swing.GroupLayout.PREFERRED_SIZE))
                            .addComponent(azimuthlabel, javax.swing.GroupLayout.PREFERRED_SIZE, 21, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addGroup(jPanel8Layout.createSequentialGroup()
                                .addGap(100, 100, 100)
                                .addComponent(antiparallacticlabel, javax.swing.GroupLayout.PREFERRED_SIZE, 21, javax.swing.GroupLayout.PREFERRED_SIZE))
                            .addGroup(jPanel8Layout.createSequentialGroup()
                                .addGap(120, 120, 120)
                                .addComponent(baryvcorlabel, javax.swing.GroupLayout.PREFERRED_SIZE, 21, javax.swing.GroupLayout.PREFERRED_SIZE))
                            .addGroup(jPanel8Layout.createSequentialGroup()
                                .addGap(180, 180, 180)
                                .addComponent(planetproximlabel, javax.swing.GroupLayout.PREFERRED_SIZE, 21, javax.swing.GroupLayout.PREFERRED_SIZE))))
                    .addGroup(jPanel8Layout.createSequentialGroup()
                        .addGap(42, 42, 42)
                        .addComponent(hafield, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addGap(2, 2, 2)
                        .addComponent(azimuthfield, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addGap(2, 2, 2)
                        .addComponent(sundecfield, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addGap(2, 2, 2)
                        .addComponent(moondecfield, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addGap(2, 2, 2)
                        .addComponent(sunazfield, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addGap(2, 2, 2)
                        .addComponent(moonazfield, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addGap(2, 2, 2)
                        .addComponent(antiparallacticfield, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addGap(2, 2, 2)
                        .addComponent(baryvcorfield, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addGap(2, 2, 2)
                        .addComponent(moonobjanglefield, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addGap(2, 2, 2)
                        .addComponent(ztwilightfield, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addGap(2, 2, 2)
                        .addComponent(planetproximfield, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addGroup(jPanel8Layout.createSequentialGroup()
                        .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addGroup(jPanel8Layout.createSequentialGroup()
                                .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                    .addGroup(jPanel8Layout.createSequentialGroup()
                                        .addGap(20, 20, 20)
                                        .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                                            .addComponent(latitudelabel, javax.swing.GroupLayout.PREFERRED_SIZE, 21, javax.swing.GroupLayout.PREFERRED_SIZE)
                                            .addComponent(equinoxfield, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)))
                                    .addComponent(observatorylabel, javax.swing.GroupLayout.PREFERRED_SIZE, 21, javax.swing.GroupLayout.PREFERRED_SIZE))
                                .addGap(19, 19, 19)
                                .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                    .addComponent(elevsealevellabel, javax.swing.GroupLayout.PREFERRED_SIZE, 21, javax.swing.GroupLayout.PREFERRED_SIZE)
                                    .addGroup(jPanel8Layout.createSequentialGroup()
                                        .addGap(20, 20, 20)
                                        .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                            .addComponent(sideriallabel9, javax.swing.GroupLayout.PREFERRED_SIZE, 21, javax.swing.GroupLayout.PREFERRED_SIZE)
                                            .addComponent(elevhorizonlabel, javax.swing.GroupLayout.PREFERRED_SIZE, 21, javax.swing.GroupLayout.PREFERRED_SIZE)))))
                            .addGroup(jPanel8Layout.createSequentialGroup()
                                .addGap(40, 40, 40)
                                .addComponent(longitudelabel, javax.swing.GroupLayout.PREFERRED_SIZE, 21, javax.swing.GroupLayout.PREFERRED_SIZE))
                            .addGroup(jPanel8Layout.createSequentialGroup()
                                .addComponent(observatoryfield, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addGap(2, 2, 2)
                                .addComponent(latitudefield, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addGap(2, 2, 2)
                                .addComponent(longitudefield, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addGap(2, 2, 2)
                                .addComponent(elevseafield, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addGap(2, 2, 2)
                                .addComponent(elevhorizonfield, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)))
                        .addGap(9, 9, 9)
                        .addComponent(timelabel, javax.swing.GroupLayout.PREFERRED_SIZE, 21, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addGap(9, 9, 9)
                        .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(utdatelabel, javax.swing.GroupLayout.PREFERRED_SIZE, 21, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addGroup(jPanel8Layout.createSequentialGroup()
                                .addGap(20, 20, 20)
                                .addComponent(uttimelabel, javax.swing.GroupLayout.PREFERRED_SIZE, 21, javax.swing.GroupLayout.PREFERRED_SIZE))
                            .addGroup(jPanel8Layout.createSequentialGroup()
                                .addComponent(utdatefield, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addGap(2, 2, 2)
                                .addComponent(uttimefield, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)))
                        .addGap(9, 9, 9)
                        .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addGroup(jPanel8Layout.createSequentialGroup()
                                .addGap(20, 20, 20)
                                .addComponent(localtimelabel, javax.swing.GroupLayout.PREFERRED_SIZE, 20, javax.swing.GroupLayout.PREFERRED_SIZE))
                            .addComponent(localdatelabel, javax.swing.GroupLayout.PREFERRED_SIZE, 21, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addGroup(jPanel8Layout.createSequentialGroup()
                                .addComponent(localdatefield, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addGap(2, 2, 2)
                                .addComponent(localtimefield, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)))
                        .addGap(10, 10, 10)
                        .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(jdlabel, javax.swing.GroupLayout.PREFERRED_SIZE, 21, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(jdfield, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)))
                    .addGroup(jPanel8Layout.createSequentialGroup()
                        .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addGroup(jPanel8Layout.createSequentialGroup()
                                .addGap(20, 20, 20)
                                .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                    .addGroup(jPanel8Layout.createSequentialGroup()
                                        .addGap(60, 60, 60)
                                        .addComponent(sunralabel, javax.swing.GroupLayout.PREFERRED_SIZE, 21, javax.swing.GroupLayout.PREFERRED_SIZE))
                                    .addGroup(jPanel8Layout.createSequentialGroup()
                                        .addGap(40, 40, 40)
                                        .addComponent(altitudelabel, javax.swing.GroupLayout.PREFERRED_SIZE, 21, javax.swing.GroupLayout.PREFERRED_SIZE))
                                    .addComponent(sideriallabel1, javax.swing.GroupLayout.PREFERRED_SIZE, 21, javax.swing.GroupLayout.PREFERRED_SIZE)
                                    .addGroup(jPanel8Layout.createSequentialGroup()
                                        .addGap(20, 20, 20)
                                        .addComponent(airmasslabel, javax.swing.GroupLayout.PREFERRED_SIZE, 21, javax.swing.GroupLayout.PREFERRED_SIZE)))
                                .addGap(19, 19, 19)
                                .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                    .addGroup(jPanel8Layout.createSequentialGroup()
                                        .addGap(20, 20, 20)
                                        .addComponent(moonaltlabel, javax.swing.GroupLayout.PREFERRED_SIZE, 21, javax.swing.GroupLayout.PREFERRED_SIZE))
                                    .addGroup(jPanel8Layout.createSequentialGroup()
                                        .addGap(80, 80, 80)
                                        .addComponent(moonillumfraclabel, javax.swing.GroupLayout.PREFERRED_SIZE, 21, javax.swing.GroupLayout.PREFERRED_SIZE))
                                    .addComponent(sunaltlabel, javax.swing.GroupLayout.PREFERRED_SIZE, 21, javax.swing.GroupLayout.PREFERRED_SIZE)
                                    .addGroup(jPanel8Layout.createSequentialGroup()
                                        .addGap(60, 60, 60)
                                        .addComponent(barjdlabel, javax.swing.GroupLayout.PREFERRED_SIZE, 21, javax.swing.GroupLayout.PREFERRED_SIZE))
                                    .addGroup(jPanel8Layout.createSequentialGroup()
                                        .addGap(40, 40, 40)
                                        .addComponent(parallacticlabel1, javax.swing.GroupLayout.PREFERRED_SIZE, 21, javax.swing.GroupLayout.PREFERRED_SIZE))
                                    .addGroup(jPanel8Layout.createSequentialGroup()
                                        .addGap(100, 100, 100)
                                        .addComponent(lunarskybrightlabel, javax.swing.GroupLayout.PREFERRED_SIZE, 21, javax.swing.GroupLayout.PREFERRED_SIZE))))
                            .addGroup(jPanel8Layout.createSequentialGroup()
                                .addGap(100, 100, 100)
                                .addComponent(moonralabel, javax.swing.GroupLayout.PREFERRED_SIZE, 21, javax.swing.GroupLayout.PREFERRED_SIZE)))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(constellationlabel, javax.swing.GroupLayout.PREFERRED_SIZE, 21, javax.swing.GroupLayout.PREFERRED_SIZE)))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(moonphaselabel, javax.swing.GroupLayout.PREFERRED_SIZE, 21, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(moonphasefield, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addComponent(jdlabel1, javax.swing.GroupLayout.PREFERRED_SIZE, 21, javax.swing.GroupLayout.PREFERRED_SIZE)))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        jTabbedPane1.addTab("Ephemeris", jPanel8);

        jPanel16.setBackground(java.awt.Color.black);

        jPanel24.setBackground(java.awt.Color.black);

        jPanel21.setBackground(java.awt.Color.black);
        jPanel21.setBorder(javax.swing.BorderFactory.createLineBorder(java.awt.Color.lightGray));

        p200_average_windspeed_label31.setFont(new java.awt.Font("Century Schoolbook L", 1, 18)); // NOI18N
        p200_average_windspeed_label31.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label31.setText("P200 WEATHER");

        p200_average_windspeed_label36.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label36.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label36.setText("Timestamp");

        p200_utc_timestamp.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_utc_timestamp.setForeground(new java.awt.Color(0, 255, 0));
        p200_utc_timestamp.setText(" ");
        p200_utc_timestamp.setBorder(javax.swing.BorderFactory.createLineBorder(java.awt.Color.white));

        javax.swing.GroupLayout jPanel21Layout = new javax.swing.GroupLayout(jPanel21);
        jPanel21.setLayout(jPanel21Layout);
        jPanel21Layout.setHorizontalGroup(
            jPanel21Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel21Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel21Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(p200_average_windspeed_label31)
                    .addGroup(jPanel21Layout.createSequentialGroup()
                        .addComponent(p200_average_windspeed_label36)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(p200_utc_timestamp, javax.swing.GroupLayout.PREFERRED_SIZE, 327, javax.swing.GroupLayout.PREFERRED_SIZE)))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );
        jPanel21Layout.setVerticalGroup(
            jPanel21Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel21Layout.createSequentialGroup()
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addComponent(p200_average_windspeed_label31)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel21Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(p200_average_windspeed_label36)
                    .addComponent(p200_utc_timestamp))
                .addGap(25, 25, 25))
        );

        jPanel18.setBackground(java.awt.Color.black);
        jPanel18.setBorder(javax.swing.BorderFactory.createLineBorder(java.awt.Color.lightGray));

        p200_average_windspeed_label.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label.setText("Average Windspeed");

        p200_average_windspeed_label1.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label1.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label1.setText("Peak Windspeed");

        p200_average_windspeed_label2.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label2.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label2.setText("Gust Windspeed");

        p200_average_windspeed_label3.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label3.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label3.setText("Wind Direction");

        p200_average_windspeed_label4.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label4.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label4.setText("Dewpoint");

        p200_average_windspeed_label5.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label5.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label5.setText("Outside Temperature");

        p200_average_windspeed_label7.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label7.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label7.setText("Inside Temperature");

        p200_average_windspeed_label6.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label6.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label6.setText("Primary Miror Temperature");

        p200_average_windspeed_label8.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label8.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label8.setText("Second Mirror Temperature");

        p200_average_windspeed_label10.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label10.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label10.setText("Floor Temperature");

        p200_average_windspeed_label9.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label9.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label9.setText("Raining?");

        p200_average_windspeed_label27.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label27.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label27.setText("Arch R");

        p200_average_windspeed_label28.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label28.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label28.setText("Arch L");

        p200_average_windspeed_label29.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label29.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label29.setText("Gantry Far R");

        p200_average_windspeed_label30.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label30.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label30.setText("Gantry Far L");

        p200_average_windspeed.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed.setForeground(new java.awt.Color(0, 255, 0));
        p200_average_windspeed.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        p200_average_windspeed.setText(" ");

        p200_peak_windspeed.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_peak_windspeed.setForeground(new java.awt.Color(0, 255, 0));
        p200_peak_windspeed.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        p200_peak_windspeed.setText(" ");

        p200_gust_windspeed.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_gust_windspeed.setForeground(new java.awt.Color(0, 255, 0));
        p200_gust_windspeed.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        p200_gust_windspeed.setText(" ");

        p200_wind_direction.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_wind_direction.setForeground(new java.awt.Color(0, 255, 0));
        p200_wind_direction.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        p200_wind_direction.setText(" ");

        p200_dewpoint.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_dewpoint.setForeground(new java.awt.Color(0, 255, 0));
        p200_dewpoint.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        p200_dewpoint.setText(" ");

        p200_outside_temperature.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_outside_temperature.setForeground(new java.awt.Color(0, 255, 0));
        p200_outside_temperature.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        p200_outside_temperature.setText(" ");

        p200_inside_temperature.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_inside_temperature.setForeground(new java.awt.Color(0, 255, 0));
        p200_inside_temperature.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        p200_inside_temperature.setText(" ");

        p200_primary_mirror_temperature.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_primary_mirror_temperature.setForeground(new java.awt.Color(0, 255, 0));
        p200_primary_mirror_temperature.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        p200_primary_mirror_temperature.setText("  ");

        p200_secondary_mirror_temperature.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_secondary_mirror_temperature.setForeground(new java.awt.Color(0, 255, 0));
        p200_secondary_mirror_temperature.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        p200_secondary_mirror_temperature.setText(" ");

        p200_arch_r.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_arch_r.setForeground(new java.awt.Color(0, 255, 0));
        p200_arch_r.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        p200_arch_r.setText(" ");

        p200_floor_temperature.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_floor_temperature.setForeground(new java.awt.Color(0, 255, 0));
        p200_floor_temperature.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        p200_floor_temperature.setText(" ");

        p200_raining.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_raining.setForeground(new java.awt.Color(0, 255, 0));
        p200_raining.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        p200_raining.setText(" ");

        p200_arch_l.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_arch_l.setForeground(new java.awt.Color(0, 255, 0));
        p200_arch_l.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        p200_arch_l.setText(" ");

        p200_grantry_far_r.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_grantry_far_r.setForeground(new java.awt.Color(0, 255, 0));
        p200_grantry_far_r.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        p200_grantry_far_r.setText(" ");

        p200_grantry_far_l.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_grantry_far_l.setForeground(new java.awt.Color(0, 255, 0));
        p200_grantry_far_l.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        p200_grantry_far_l.setText(" ");

        p200_average_windspeed_label32.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label32.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label32.setText("Primary Cell Temperature");

        p200_primary_cell_temperature.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_primary_cell_temperature.setForeground(new java.awt.Color(0, 255, 0));
        p200_primary_cell_temperature.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        p200_primary_cell_temperature.setText(" ");

        javax.swing.GroupLayout jPanel18Layout = new javax.swing.GroupLayout(jPanel18);
        jPanel18.setLayout(jPanel18Layout);
        jPanel18Layout.setHorizontalGroup(
            jPanel18Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel18Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel18Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel18Layout.createSequentialGroup()
                        .addComponent(p200_average_windspeed_label32, javax.swing.GroupLayout.PREFERRED_SIZE, 208, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                        .addComponent(p200_primary_cell_temperature, javax.swing.GroupLayout.DEFAULT_SIZE, 85, Short.MAX_VALUE))
                    .addGroup(jPanel18Layout.createSequentialGroup()
                        .addGroup(jPanel18Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(p200_average_windspeed_label)
                            .addComponent(p200_average_windspeed_label1)
                            .addComponent(p200_average_windspeed_label30)
                            .addComponent(p200_average_windspeed_label29)
                            .addComponent(p200_average_windspeed_label28)
                            .addComponent(p200_average_windspeed_label27)
                            .addComponent(p200_average_windspeed_label9)
                            .addComponent(p200_average_windspeed_label10)
                            .addComponent(p200_average_windspeed_label8)
                            .addComponent(p200_average_windspeed_label6)
                            .addComponent(p200_average_windspeed_label7)
                            .addComponent(p200_average_windspeed_label5)
                            .addComponent(p200_average_windspeed_label4)
                            .addComponent(p200_average_windspeed_label3)
                            .addComponent(p200_average_windspeed_label2))
                        .addGap(25, 25, 25)
                        .addGroup(jPanel18Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING, false)
                            .addComponent(p200_grantry_far_r, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, 83, Short.MAX_VALUE)
                            .addComponent(p200_arch_l, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .addComponent(p200_arch_r, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .addComponent(p200_raining, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .addComponent(p200_floor_temperature, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .addComponent(p200_secondary_mirror_temperature, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .addComponent(p200_primary_mirror_temperature, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .addComponent(p200_inside_temperature, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .addComponent(p200_outside_temperature, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .addComponent(p200_dewpoint, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .addComponent(p200_wind_direction, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .addComponent(p200_gust_windspeed, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .addComponent(p200_peak_windspeed, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .addComponent(p200_average_windspeed, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .addComponent(p200_grantry_far_l, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))))
                .addContainerGap(22, Short.MAX_VALUE))
        );
        jPanel18Layout.setVerticalGroup(
            jPanel18Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel18Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel18Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(p200_average_windspeed_label)
                    .addComponent(p200_average_windspeed))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel18Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(p200_average_windspeed_label1)
                    .addComponent(p200_peak_windspeed))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel18Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(p200_average_windspeed_label2)
                    .addComponent(p200_gust_windspeed))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel18Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(p200_average_windspeed_label3)
                    .addComponent(p200_wind_direction, javax.swing.GroupLayout.PREFERRED_SIZE, 17, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel18Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(p200_average_windspeed_label4)
                    .addComponent(p200_dewpoint))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel18Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(p200_average_windspeed_label5)
                    .addComponent(p200_outside_temperature))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel18Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(p200_average_windspeed_label7)
                    .addComponent(p200_inside_temperature))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel18Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(p200_average_windspeed_label6)
                    .addComponent(p200_primary_mirror_temperature))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel18Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(p200_average_windspeed_label8)
                    .addComponent(p200_secondary_mirror_temperature))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel18Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(p200_average_windspeed_label10)
                    .addComponent(p200_floor_temperature))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel18Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(p200_average_windspeed_label9)
                    .addComponent(p200_raining))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel18Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(p200_average_windspeed_label27)
                    .addComponent(p200_arch_r))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel18Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(p200_average_windspeed_label28)
                    .addComponent(p200_arch_l))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel18Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(p200_average_windspeed_label29)
                    .addComponent(p200_grantry_far_r))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel18Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(p200_average_windspeed_label30)
                    .addComponent(p200_grantry_far_l))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addGroup(jPanel18Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(p200_average_windspeed_label32)
                    .addComponent(p200_primary_cell_temperature))
                .addContainerGap())
        );

        jPanel19.setBackground(java.awt.Color.black);
        jPanel19.setBorder(new javax.swing.border.LineBorder(java.awt.Color.white, 1, true));

        p200_average_windspeed_label11.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label11.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label11.setText("Stairs 0");

        p200_average_windspeed_label12.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label12.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label12.setText("Stairs 20");

        p200_average_windspeed_label17.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label17.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label17.setText("Stairs 40");

        p200_average_windspeed_label19.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label19.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label19.setText("Stairs 60");

        p200_average_windspeed_label18.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label18.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label18.setText("Stairs 75");

        p200_average_windspeed_label16.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label16.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label16.setText("Stairs 90");

        p200_average_windspeed_label13.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label13.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label13.setText("Gantry UR");

        p200_average_windspeed_label20.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label20.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label20.setText("Gantry LR");

        p200_average_windspeed_label21.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label21.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label21.setText("Gantry UL");

        p200_average_windspeed_label22.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label22.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label22.setText("Gantry LL");

        p200_average_windspeed_label14.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label14.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label14.setText("Crane");

        p200_average_windspeed_label15.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label15.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label15.setText("Exhaust");

        p200_average_windspeed_label23.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label23.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label23.setText("Shutter");

        p200_average_windspeed_label24.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label24.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label24.setText("Fan R");

        p200_average_windspeed_label25.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label25.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label25.setText("Fan L");

        p200_average_windspeed_label26.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label26.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label26.setText("Catwalk");

        p200_stairs0.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_stairs0.setForeground(new java.awt.Color(0, 255, 0));
        p200_stairs0.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        p200_stairs0.setText(" ");

        p200_stairs20.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_stairs20.setForeground(new java.awt.Color(0, 255, 0));
        p200_stairs20.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        p200_stairs20.setText(" ");

        p200_stairs40.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_stairs40.setForeground(new java.awt.Color(0, 255, 0));
        p200_stairs40.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        p200_stairs40.setText(" ");

        p200_stairs60.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_stairs60.setForeground(new java.awt.Color(0, 255, 0));
        p200_stairs60.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        p200_stairs60.setText("  ");

        p200_stairs75.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_stairs75.setForeground(new java.awt.Color(0, 255, 0));
        p200_stairs75.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        p200_stairs75.setText(" ");

        p200_stairs90.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_stairs90.setForeground(new java.awt.Color(0, 255, 0));
        p200_stairs90.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        p200_stairs90.setText(" ");

        p200_gantry_ur.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_gantry_ur.setForeground(new java.awt.Color(0, 255, 0));
        p200_gantry_ur.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        p200_gantry_ur.setText(" ");

        p200_gantrylr.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_gantrylr.setForeground(new java.awt.Color(0, 255, 0));
        p200_gantrylr.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        p200_gantrylr.setText(" ");

        p200_gantryul.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_gantryul.setForeground(new java.awt.Color(0, 255, 0));
        p200_gantryul.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        p200_gantryul.setText(" ");

        p200_gantryll.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_gantryll.setForeground(new java.awt.Color(0, 255, 0));
        p200_gantryll.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        p200_gantryll.setText(" ");

        p200_crane.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_crane.setForeground(new java.awt.Color(0, 255, 0));
        p200_crane.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        p200_crane.setText(" ");

        p200_exhaust.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_exhaust.setForeground(new java.awt.Color(0, 255, 0));
        p200_exhaust.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        p200_exhaust.setText(" ");

        p200_shutter.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_shutter.setForeground(new java.awt.Color(0, 255, 0));
        p200_shutter.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        p200_shutter.setText(" ");

        p200_fanr.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_fanr.setForeground(new java.awt.Color(0, 255, 0));
        p200_fanr.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        p200_fanr.setText(" ");

        p200_fanl.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_fanl.setForeground(new java.awt.Color(0, 255, 0));
        p200_fanl.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        p200_fanl.setText(" ");

        p200_catwalk.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_catwalk.setForeground(new java.awt.Color(0, 255, 0));
        p200_catwalk.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        p200_catwalk.setText(" ");

        javax.swing.GroupLayout jPanel19Layout = new javax.swing.GroupLayout(jPanel19);
        jPanel19.setLayout(jPanel19Layout);
        jPanel19Layout.setHorizontalGroup(
            jPanel19Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel19Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel19Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(p200_average_windspeed_label11)
                    .addComponent(p200_average_windspeed_label12)
                    .addComponent(p200_average_windspeed_label17)
                    .addComponent(p200_average_windspeed_label19)
                    .addComponent(p200_average_windspeed_label18)
                    .addComponent(p200_average_windspeed_label16)
                    .addComponent(p200_average_windspeed_label13)
                    .addComponent(p200_average_windspeed_label20)
                    .addComponent(p200_average_windspeed_label21)
                    .addComponent(p200_average_windspeed_label22)
                    .addComponent(p200_average_windspeed_label14)
                    .addComponent(p200_average_windspeed_label15)
                    .addComponent(p200_average_windspeed_label23)
                    .addComponent(p200_average_windspeed_label24)
                    .addComponent(p200_average_windspeed_label25)
                    .addComponent(p200_average_windspeed_label26))
                .addGap(18, 18, 18)
                .addGroup(jPanel19Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                    .addComponent(p200_fanr, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(p200_shutter, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(p200_exhaust, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(p200_crane, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(p200_gantryll, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(p200_gantryul, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(p200_gantrylr, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(p200_gantry_ur, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(p200_stairs90, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(p200_stairs75, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(p200_stairs60, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(p200_stairs40, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(p200_stairs20, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(p200_stairs0, javax.swing.GroupLayout.DEFAULT_SIZE, 83, Short.MAX_VALUE)
                    .addComponent(p200_fanl, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(p200_catwalk, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap(134, Short.MAX_VALUE))
        );
        jPanel19Layout.setVerticalGroup(
            jPanel19Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel19Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel19Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel19Layout.createSequentialGroup()
                        .addComponent(p200_average_windspeed_label11)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(p200_average_windspeed_label12)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(p200_average_windspeed_label17)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(p200_average_windspeed_label19)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(p200_average_windspeed_label18)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(p200_average_windspeed_label16)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(p200_average_windspeed_label13)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(p200_average_windspeed_label20)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(p200_average_windspeed_label21)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(p200_average_windspeed_label22)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(p200_average_windspeed_label14)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(p200_average_windspeed_label15)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(p200_average_windspeed_label23)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(p200_average_windspeed_label24))
                    .addGroup(jPanel19Layout.createSequentialGroup()
                        .addComponent(p200_stairs0)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(p200_stairs20)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(p200_stairs40)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(p200_stairs60, javax.swing.GroupLayout.PREFERRED_SIZE, 17, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(p200_stairs75)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(p200_stairs90)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(p200_gantry_ur)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(p200_gantrylr)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(p200_gantryul)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(p200_gantryll)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(p200_crane)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(p200_exhaust)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(p200_shutter)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(p200_fanr)))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel19Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                    .addGroup(jPanel19Layout.createSequentialGroup()
                        .addComponent(p200_average_windspeed_label25)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(p200_average_windspeed_label26))
                    .addGroup(jPanel19Layout.createSequentialGroup()
                        .addComponent(p200_fanl)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(p200_catwalk)))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        javax.swing.GroupLayout jPanel24Layout = new javax.swing.GroupLayout(jPanel24);
        jPanel24.setLayout(jPanel24Layout);
        jPanel24Layout.setHorizontalGroup(
            jPanel24Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel24Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel24Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(jPanel21, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addGroup(jPanel24Layout.createSequentialGroup()
                        .addComponent(jPanel18, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                        .addComponent(jPanel19, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addGap(0, 247, Short.MAX_VALUE)))
                .addContainerGap())
        );
        jPanel24Layout.setVerticalGroup(
            jPanel24Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel24Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jPanel21, javax.swing.GroupLayout.PREFERRED_SIZE, 63, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel24Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                    .addComponent(jPanel19, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(jPanel18, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        jTabbedPane2.addTab("P200 Weather", jPanel24);

        jPanel25.setBackground(java.awt.Color.black);

        jPanel17.setBackground(java.awt.Color.black);
        jPanel17.setBorder(new javax.swing.border.LineBorder(java.awt.Color.white, 1, true));

        p200_average_windspeed_label100.setFont(new java.awt.Font("Century Schoolbook L", 1, 18)); // NOI18N
        p200_average_windspeed_label100.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label100.setText("P60 WEATHER");

        p200_average_windspeed_label101.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label101.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label101.setText("Timestamp");

        p60_utc_timestamp.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p60_utc_timestamp.setForeground(new java.awt.Color(0, 255, 0));
        p60_utc_timestamp.setText(" ");
        p60_utc_timestamp.setBorder(javax.swing.BorderFactory.createLineBorder(java.awt.Color.white));

        javax.swing.GroupLayout jPanel17Layout = new javax.swing.GroupLayout(jPanel17);
        jPanel17.setLayout(jPanel17Layout);
        jPanel17Layout.setHorizontalGroup(
            jPanel17Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel17Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel17Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(p200_average_windspeed_label100)
                    .addGroup(jPanel17Layout.createSequentialGroup()
                        .addComponent(p200_average_windspeed_label101)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(p60_utc_timestamp, javax.swing.GroupLayout.PREFERRED_SIZE, 327, javax.swing.GroupLayout.PREFERRED_SIZE)))
                .addContainerGap(492, Short.MAX_VALUE))
        );
        jPanel17Layout.setVerticalGroup(
            jPanel17Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel17Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(p200_average_windspeed_label100)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addGroup(jPanel17Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(p200_average_windspeed_label101)
                    .addComponent(p60_utc_timestamp))
                .addContainerGap())
        );

        jPanel20.setBackground(java.awt.Color.black);
        jPanel20.setBorder(javax.swing.BorderFactory.createLineBorder(java.awt.Color.lightGray));

        p200_average_windspeed_label39.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label39.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label39.setText("Outside Temperature");

        p200_average_windspeed_label40.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label40.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label40.setText("Inside Temperature");

        p200_average_windspeed_label43.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label43.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label43.setText("Floor Temperature");

        p200_average_windspeed_label44.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label44.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label44.setText("Outside Dewpoint");

        p200_average_windspeed_label47.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label47.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label47.setText("Inside Dewpoint");

        p200_average_windspeed_label50.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label50.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label50.setText("Tube Air Temperature");

        P60_tube_top_temperature_label.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        P60_tube_top_temperature_label.setForeground(new java.awt.Color(0, 255, 255));
        P60_tube_top_temperature_label.setText("Tube Top Temperature");

        p200_average_windspeed_label71.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label71.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label71.setText("Tube Middle Temperature");

        p200_average_windspeed_label72.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label72.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label72.setText("Tube Bottom Temperature");

        P60_outside_temperature.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        P60_outside_temperature.setForeground(new java.awt.Color(0, 255, 0));
        P60_outside_temperature.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        P60_outside_temperature.setText(" ");

        P60_inside_temperature.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        P60_inside_temperature.setForeground(new java.awt.Color(0, 255, 0));
        P60_inside_temperature.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        P60_inside_temperature.setText(" ");

        P60_floor_temperature.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        P60_floor_temperature.setForeground(new java.awt.Color(0, 255, 0));
        P60_floor_temperature.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        P60_floor_temperature.setText(" ");

        P60_outside_dewpoint.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        P60_outside_dewpoint.setForeground(new java.awt.Color(0, 255, 0));
        P60_outside_dewpoint.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        P60_outside_dewpoint.setText(" ");

        P60_inside_dewpoint.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        P60_inside_dewpoint.setForeground(new java.awt.Color(0, 255, 0));
        P60_inside_dewpoint.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        P60_inside_dewpoint.setText(" ");

        P60_tube_air_temperature.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        P60_tube_air_temperature.setForeground(new java.awt.Color(0, 255, 0));
        P60_tube_air_temperature.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        P60_tube_air_temperature.setText(" ");

        p60_tube_top_temperature.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p60_tube_top_temperature.setForeground(new java.awt.Color(0, 255, 0));
        p60_tube_top_temperature.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        p60_tube_top_temperature.setText(" ");

        P60_tube_middle_temperature.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        P60_tube_middle_temperature.setForeground(new java.awt.Color(0, 255, 0));
        P60_tube_middle_temperature.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        P60_tube_middle_temperature.setText(" ");

        P60_tube_bottom_temperature.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        P60_tube_bottom_temperature.setForeground(new java.awt.Color(0, 255, 0));
        P60_tube_bottom_temperature.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        P60_tube_bottom_temperature.setText("  ");

        p200_average_windspeed_label79.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label79.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label79.setText("Barometric Pressure");

        P60_barometric_pressure.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        P60_barometric_pressure.setForeground(new java.awt.Color(0, 255, 0));
        P60_barometric_pressure.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        P60_barometric_pressure.setText(" ");

        javax.swing.GroupLayout jPanel20Layout = new javax.swing.GroupLayout(jPanel20);
        jPanel20.setLayout(jPanel20Layout);
        jPanel20Layout.setHorizontalGroup(
            jPanel20Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel20Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel20Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(p200_average_windspeed_label39)
                    .addComponent(p200_average_windspeed_label40)
                    .addComponent(p200_average_windspeed_label72)
                    .addComponent(p200_average_windspeed_label71)
                    .addComponent(P60_tube_top_temperature_label)
                    .addComponent(p200_average_windspeed_label50)
                    .addComponent(p200_average_windspeed_label47)
                    .addComponent(p200_average_windspeed_label44)
                    .addComponent(p200_average_windspeed_label43)
                    .addComponent(p200_average_windspeed_label79))
                .addGap(37, 37, 37)
                .addGroup(jPanel20Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                    .addComponent(P60_barometric_pressure, javax.swing.GroupLayout.DEFAULT_SIZE, 83, Short.MAX_VALUE)
                    .addComponent(P60_tube_bottom_temperature, javax.swing.GroupLayout.DEFAULT_SIZE, 83, Short.MAX_VALUE)
                    .addComponent(P60_tube_middle_temperature, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(p60_tube_top_temperature, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(P60_tube_air_temperature, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(P60_inside_dewpoint, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(P60_outside_dewpoint, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(P60_floor_temperature, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(P60_inside_temperature, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(P60_outside_temperature, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );
        jPanel20Layout.setVerticalGroup(
            jPanel20Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel20Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel20Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(p200_average_windspeed_label39)
                    .addComponent(P60_outside_temperature))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel20Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(p200_average_windspeed_label40)
                    .addComponent(P60_inside_temperature))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel20Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(p200_average_windspeed_label43)
                    .addComponent(P60_floor_temperature))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel20Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(p200_average_windspeed_label44)
                    .addComponent(P60_outside_dewpoint, javax.swing.GroupLayout.PREFERRED_SIZE, 17, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel20Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(p200_average_windspeed_label47)
                    .addComponent(P60_inside_dewpoint))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel20Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(p200_average_windspeed_label50)
                    .addComponent(P60_tube_air_temperature))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel20Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(P60_tube_top_temperature_label)
                    .addComponent(p60_tube_top_temperature))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel20Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(p200_average_windspeed_label71)
                    .addComponent(P60_tube_middle_temperature))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel20Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(p200_average_windspeed_label72)
                    .addComponent(P60_tube_bottom_temperature))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel20Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(p200_average_windspeed_label79)
                    .addComponent(P60_barometric_pressure))
                .addContainerGap(148, Short.MAX_VALUE))
        );

        jPanel27.setBackground(java.awt.Color.black);
        jPanel27.setBorder(new javax.swing.border.LineBorder(java.awt.Color.white, 1, true));

        p200_average_windspeed_label73.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label73.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label73.setText("Primary Mirror Temperature");

        p200_average_windspeed_label74.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label74.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label74.setText("Primary Cell Temperature");

        P60_mirror_temperature.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        P60_mirror_temperature.setForeground(new java.awt.Color(0, 255, 0));
        P60_mirror_temperature.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        P60_mirror_temperature.setText(" ");

        P60_primary_cell_temperature.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        P60_primary_cell_temperature.setForeground(new java.awt.Color(0, 255, 0));
        P60_primary_cell_temperature.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        P60_primary_cell_temperature.setText(" ");

        p200_average_windspeed_label75.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label75.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label75.setText("Secondary Cell Temperature");

        P60_secondary_cell_temperature.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        P60_secondary_cell_temperature.setForeground(new java.awt.Color(0, 255, 0));
        P60_secondary_cell_temperature.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        P60_secondary_cell_temperature.setText(" ");

        p200_average_windspeed_label76.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label76.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label76.setText("Ouside Relative Humidity");

        P60_outside_relative_humidity.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        P60_outside_relative_humidity.setForeground(new java.awt.Color(0, 255, 0));
        P60_outside_relative_humidity.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        P60_outside_relative_humidity.setText("  ");

        p200_average_windspeed_label77.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label77.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label77.setText("Inside Relative Humidity");

        P60_inside_relative_humidity.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        P60_inside_relative_humidity.setForeground(new java.awt.Color(0, 255, 0));
        P60_inside_relative_humidity.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        P60_inside_relative_humidity.setText(" ");

        p200_average_windspeed_label78.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label78.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label78.setText("Raining?");

        P60_raining.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        P60_raining.setForeground(new java.awt.Color(0, 255, 0));
        P60_raining.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        P60_raining.setText(" ");

        p200_average_windspeed_label94.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label94.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label94.setText("Wind Direction");

        P60_wind_direction.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        P60_wind_direction.setForeground(new java.awt.Color(0, 255, 0));
        P60_wind_direction.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        P60_wind_direction.setText(" ");

        p200_average_windspeed_label95.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label95.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label95.setText("Wind Gusts");

        P60_wind_gusts.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        P60_wind_gusts.setForeground(new java.awt.Color(0, 255, 0));
        P60_wind_gusts.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        P60_wind_gusts.setText(" ");

        p200_average_windspeed_label96.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label96.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label96.setText("Wind Median");

        P60_wind_median.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        P60_wind_median.setForeground(new java.awt.Color(0, 255, 0));
        P60_wind_median.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        P60_wind_median.setText(" ");

        javax.swing.GroupLayout jPanel27Layout = new javax.swing.GroupLayout(jPanel27);
        jPanel27.setLayout(jPanel27Layout);
        jPanel27Layout.setHorizontalGroup(
            jPanel27Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel27Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel27Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel27Layout.createSequentialGroup()
                        .addComponent(p200_average_windspeed_label96)
                        .addGap(135, 135, 135)
                        .addComponent(P60_wind_median, javax.swing.GroupLayout.PREFERRED_SIZE, 83, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addGroup(jPanel27Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                        .addGroup(jPanel27Layout.createSequentialGroup()
                            .addComponent(p200_average_windspeed_label95)
                            .addGap(146, 146, 146)
                            .addComponent(P60_wind_gusts, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                        .addGroup(jPanel27Layout.createSequentialGroup()
                            .addGroup(jPanel27Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                .addComponent(p200_average_windspeed_label78)
                                .addComponent(p200_average_windspeed_label77)
                                .addComponent(p200_average_windspeed_label76)
                                .addComponent(p200_average_windspeed_label75)
                                .addComponent(p200_average_windspeed_label74)
                                .addComponent(p200_average_windspeed_label73))
                            .addGap(25, 25, 25)
                            .addGroup(jPanel27Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING, false)
                                .addComponent(P60_inside_relative_humidity, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, 83, Short.MAX_VALUE)
                                .addComponent(P60_outside_relative_humidity, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                                .addComponent(P60_secondary_cell_temperature, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                                .addComponent(P60_primary_cell_temperature, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                                .addComponent(P60_mirror_temperature, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                                .addComponent(P60_raining, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)))
                        .addGroup(jPanel27Layout.createSequentialGroup()
                            .addComponent(p200_average_windspeed_label94)
                            .addGap(121, 121, 121)
                            .addComponent(P60_wind_direction, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))))
                .addGap(6, 253, Short.MAX_VALUE))
        );
        jPanel27Layout.setVerticalGroup(
            jPanel27Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel27Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel27Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(p200_average_windspeed_label73)
                    .addComponent(P60_mirror_temperature))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel27Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(p200_average_windspeed_label74)
                    .addComponent(P60_primary_cell_temperature))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel27Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(p200_average_windspeed_label75)
                    .addComponent(P60_secondary_cell_temperature))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel27Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(p200_average_windspeed_label76)
                    .addComponent(P60_outside_relative_humidity))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel27Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(p200_average_windspeed_label77)
                    .addComponent(P60_inside_relative_humidity))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel27Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(p200_average_windspeed_label78)
                    .addComponent(P60_raining))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel27Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(p200_average_windspeed_label94)
                    .addComponent(P60_wind_direction))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel27Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(p200_average_windspeed_label95)
                    .addComponent(P60_wind_gusts))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel27Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(p200_average_windspeed_label96)
                    .addComponent(P60_wind_median))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        javax.swing.GroupLayout jPanel25Layout = new javax.swing.GroupLayout(jPanel25);
        jPanel25.setLayout(jPanel25Layout);
        jPanel25Layout.setHorizontalGroup(
            jPanel25Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel25Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel25Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel25Layout.createSequentialGroup()
                        .addComponent(jPanel20, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(jPanel27, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                    .addComponent(jPanel17, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );
        jPanel25Layout.setVerticalGroup(
            jPanel25Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel25Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jPanel17, javax.swing.GroupLayout.PREFERRED_SIZE, 65, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel25Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(jPanel20, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(jPanel27, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );

        jTabbedPane2.addTab("P60 Weather", jPanel25);

        jPanel26.setBackground(java.awt.Color.black);

        jPanel22.setBackground(java.awt.Color.black);
        jPanel22.setBorder(new javax.swing.border.LineBorder(java.awt.Color.white, 1, true));

        p200_average_windspeed_label102.setFont(new java.awt.Font("Century Schoolbook L", 1, 18)); // NOI18N
        p200_average_windspeed_label102.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label102.setText("P48 WEATHER");

        p200_average_windspeed_label103.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label103.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label103.setText("Timestamp");

        p48_utc_timestamp.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p48_utc_timestamp.setForeground(new java.awt.Color(0, 255, 0));
        p48_utc_timestamp.setText(" ");
        p48_utc_timestamp.setBorder(javax.swing.BorderFactory.createLineBorder(java.awt.Color.white));

        javax.swing.GroupLayout jPanel22Layout = new javax.swing.GroupLayout(jPanel22);
        jPanel22.setLayout(jPanel22Layout);
        jPanel22Layout.setHorizontalGroup(
            jPanel22Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel22Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel22Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(p200_average_windspeed_label102)
                    .addGroup(jPanel22Layout.createSequentialGroup()
                        .addComponent(p200_average_windspeed_label103)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                        .addComponent(p48_utc_timestamp, javax.swing.GroupLayout.PREFERRED_SIZE, 327, javax.swing.GroupLayout.PREFERRED_SIZE)))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );
        jPanel22Layout.setVerticalGroup(
            jPanel22Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel22Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(p200_average_windspeed_label102)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addGroup(jPanel22Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(p200_average_windspeed_label103)
                    .addComponent(p48_utc_timestamp))
                .addContainerGap())
        );

        jPanel23.setBackground(java.awt.Color.black);
        jPanel23.setBorder(javax.swing.BorderFactory.createLineBorder(java.awt.Color.lightGray));

        p200_average_windspeed_label104.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label104.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label104.setText("Average Windspeed Threshold");

        p200_average_windspeed_label105.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label105.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label105.setText("Peak Windspeed Thresold");

        p200_average_windspeed_label106.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label106.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label106.setText("Alarm Holdtime");

        p200_average_windspeed_label107.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label107.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label107.setText("Remaining Holdtime");

        p200_average_windspeed_label108.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label108.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label108.setText("Outside Dewpoint Threshold");

        p200_average_windspeed_label109.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label109.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label109.setText("Inside Dewpoint Threshold");

        p200_average_windspeed_label110.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label110.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label110.setText("Current Wind Direction");

        p200_average_windspeed_label111.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label111.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label111.setText("Current Windspeed");

        p200_average_windspeed_label112.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label112.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label112.setText("Windspeed Average");

        p200_average_windspeed_label113.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label113.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label113.setText("Outside Air Temperature");

        p200_average_windspeed_label114.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label114.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label114.setText("Outside Relative Humidity");

        p200_average_windspeed_label115.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label115.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label115.setText("Outside Dewpoint");

        p200_average_windspeed_label116.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label116.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label116.setText("Inside Air Temperature");

        p200_average_windspeed_label117.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label117.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label117.setText("Inside Relative Humidity");

        p200_average_windspeed_label118.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label118.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label118.setText("Inside Dewpoint");

        P48_average_windspeed_threshold.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        P48_average_windspeed_threshold.setForeground(new java.awt.Color(0, 255, 0));
        P48_average_windspeed_threshold.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        P48_average_windspeed_threshold.setText("  ");

        P48_peak_windspeed_threshold.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        P48_peak_windspeed_threshold.setForeground(new java.awt.Color(0, 255, 0));
        P48_peak_windspeed_threshold.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        P48_peak_windspeed_threshold.setText(" ");

        p48_alarm_holdtime.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p48_alarm_holdtime.setForeground(new java.awt.Color(0, 255, 0));
        p48_alarm_holdtime.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        p48_alarm_holdtime.setText(" ");

        P48_remainging_holdtime.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        P48_remainging_holdtime.setForeground(new java.awt.Color(0, 255, 0));
        P48_remainging_holdtime.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        P48_remainging_holdtime.setText(" ");

        P48_outside_dewpoint_threshold.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        P48_outside_dewpoint_threshold.setForeground(new java.awt.Color(0, 255, 0));
        P48_outside_dewpoint_threshold.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        P48_outside_dewpoint_threshold.setText(" ");

        P48_inside_dewpoint_threshold.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        P48_inside_dewpoint_threshold.setForeground(new java.awt.Color(0, 255, 0));
        P48_inside_dewpoint_threshold.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        P48_inside_dewpoint_threshold.setText(" ");

        P48_current_wind_direction.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        P48_current_wind_direction.setForeground(new java.awt.Color(0, 255, 0));
        P48_current_wind_direction.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        P48_current_wind_direction.setText(" ");

        P48_current_windspeed.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        P48_current_windspeed.setForeground(new java.awt.Color(0, 255, 0));
        P48_current_windspeed.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        P48_current_windspeed.setText(" ");

        P48_windspeed_average.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        P48_windspeed_average.setForeground(new java.awt.Color(0, 255, 0));
        P48_windspeed_average.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        P48_windspeed_average.setText(" ");

        P48_outside_dewpoint.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        P48_outside_dewpoint.setForeground(new java.awt.Color(0, 255, 0));
        P48_outside_dewpoint.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        P48_outside_dewpoint.setText(" ");

        P48_outside_air_temperature.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        P48_outside_air_temperature.setForeground(new java.awt.Color(0, 255, 0));
        P48_outside_air_temperature.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        P48_outside_air_temperature.setText(" ");

        P48_outside_relative_humidity.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        P48_outside_relative_humidity.setForeground(new java.awt.Color(0, 255, 0));
        P48_outside_relative_humidity.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        P48_outside_relative_humidity.setText(" ");

        P48_inside_air_temperature.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        P48_inside_air_temperature.setForeground(new java.awt.Color(0, 255, 0));
        P48_inside_air_temperature.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        P48_inside_air_temperature.setText(" ");

        P48_inside_relative_humidity.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        P48_inside_relative_humidity.setForeground(new java.awt.Color(0, 255, 0));
        P48_inside_relative_humidity.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        P48_inside_relative_humidity.setText(" ");

        P48_inside_dewpoint.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        P48_inside_dewpoint.setForeground(new java.awt.Color(0, 255, 0));
        P48_inside_dewpoint.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        P48_inside_dewpoint.setText(" ");

        p200_average_windspeed_label134.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label134.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label134.setText("Wetness?");

        p200_average_windspeed_label135.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        p200_average_windspeed_label135.setForeground(new java.awt.Color(0, 255, 255));
        p200_average_windspeed_label135.setText("Status");

        P48_wetness.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        P48_wetness.setForeground(new java.awt.Color(0, 255, 0));
        P48_wetness.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        P48_wetness.setText(" ");

        P48_status.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        P48_status.setForeground(new java.awt.Color(0, 255, 0));
        P48_status.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        P48_status.setText(" ");

        javax.swing.GroupLayout jPanel23Layout = new javax.swing.GroupLayout(jPanel23);
        jPanel23.setLayout(jPanel23Layout);
        jPanel23Layout.setHorizontalGroup(
            jPanel23Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel23Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel23Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(p200_average_windspeed_label104)
                    .addComponent(p200_average_windspeed_label105)
                    .addComponent(p200_average_windspeed_label112)
                    .addComponent(p200_average_windspeed_label111)
                    .addComponent(p200_average_windspeed_label110)
                    .addComponent(p200_average_windspeed_label109)
                    .addComponent(p200_average_windspeed_label108)
                    .addComponent(p200_average_windspeed_label107)
                    .addComponent(p200_average_windspeed_label106))
                .addGap(25, 25, 25)
                .addGroup(jPanel23Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING, false)
                    .addComponent(P48_windspeed_average, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, 83, Short.MAX_VALUE)
                    .addComponent(P48_current_windspeed, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(P48_current_wind_direction, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(P48_inside_dewpoint_threshold, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(P48_outside_dewpoint_threshold, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(P48_remainging_holdtime, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(p48_alarm_holdtime, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(P48_peak_windspeed_threshold, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(P48_average_windspeed_threshold, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel23Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(p200_average_windspeed_label117)
                    .addComponent(p200_average_windspeed_label116)
                    .addComponent(p200_average_windspeed_label115)
                    .addComponent(p200_average_windspeed_label114)
                    .addComponent(p200_average_windspeed_label113)
                    .addComponent(p200_average_windspeed_label118)
                    .addComponent(p200_average_windspeed_label134)
                    .addComponent(p200_average_windspeed_label135))
                .addGap(25, 25, 25)
                .addGroup(jPanel23Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(P48_status, javax.swing.GroupLayout.PREFERRED_SIZE, 83, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(P48_wetness, javax.swing.GroupLayout.PREFERRED_SIZE, 83, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(P48_inside_dewpoint, javax.swing.GroupLayout.PREFERRED_SIZE, 83, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addGroup(jPanel23Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING, false)
                        .addComponent(P48_inside_relative_humidity, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, 83, Short.MAX_VALUE)
                        .addComponent(P48_inside_air_temperature, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addComponent(P48_outside_dewpoint, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addComponent(P48_outside_relative_humidity, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addComponent(P48_outside_air_temperature, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)))
                .addContainerGap(20, Short.MAX_VALUE))
        );
        jPanel23Layout.setVerticalGroup(
            jPanel23Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel23Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel23Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel23Layout.createSequentialGroup()
                        .addGroup(jPanel23Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(p200_average_windspeed_label104)
                            .addComponent(P48_average_windspeed_threshold))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(jPanel23Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(p200_average_windspeed_label105)
                            .addComponent(P48_peak_windspeed_threshold))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(jPanel23Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(p200_average_windspeed_label106)
                            .addComponent(p48_alarm_holdtime))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(jPanel23Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(p200_average_windspeed_label107)
                            .addComponent(P48_remainging_holdtime, javax.swing.GroupLayout.PREFERRED_SIZE, 17, javax.swing.GroupLayout.PREFERRED_SIZE))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(jPanel23Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(p200_average_windspeed_label108)
                            .addComponent(P48_outside_dewpoint_threshold)))
                    .addGroup(jPanel23Layout.createSequentialGroup()
                        .addGroup(jPanel23Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(p200_average_windspeed_label113)
                            .addComponent(P48_outside_air_temperature))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(jPanel23Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(p200_average_windspeed_label114)
                            .addComponent(P48_outside_relative_humidity))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(jPanel23Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(p200_average_windspeed_label115)
                            .addComponent(P48_outside_dewpoint))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(jPanel23Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(p200_average_windspeed_label116)
                            .addComponent(P48_inside_air_temperature))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(jPanel23Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(p200_average_windspeed_label117)
                            .addComponent(P48_inside_relative_humidity))))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel23Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel23Layout.createSequentialGroup()
                        .addGroup(jPanel23Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(p200_average_windspeed_label109)
                            .addComponent(P48_inside_dewpoint_threshold)
                            .addComponent(p200_average_windspeed_label118))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(jPanel23Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(p200_average_windspeed_label110)
                            .addComponent(P48_current_wind_direction)
                            .addComponent(p200_average_windspeed_label134))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(jPanel23Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(p200_average_windspeed_label111)
                            .addComponent(P48_current_windspeed)
                            .addComponent(p200_average_windspeed_label135))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(jPanel23Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(p200_average_windspeed_label112)
                            .addComponent(P48_windspeed_average)))
                    .addGroup(jPanel23Layout.createSequentialGroup()
                        .addComponent(P48_inside_dewpoint)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(P48_wetness)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(P48_status)))
                .addContainerGap(162, Short.MAX_VALUE))
        );

        javax.swing.GroupLayout jPanel26Layout = new javax.swing.GroupLayout(jPanel26);
        jPanel26.setLayout(jPanel26Layout);
        jPanel26Layout.setHorizontalGroup(
            jPanel26Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel26Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel26Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel26Layout.createSequentialGroup()
                        .addComponent(jPanel23, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addGap(0, 255, Short.MAX_VALUE))
                    .addComponent(jPanel22, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );
        jPanel26Layout.setVerticalGroup(
            jPanel26Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel26Layout.createSequentialGroup()
                .addGap(21, 21, 21)
                .addComponent(jPanel22, javax.swing.GroupLayout.PREFERRED_SIZE, 65, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jPanel23, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addContainerGap())
        );

        jTabbedPane2.addTab("P48 Weather", jPanel26);

        javax.swing.GroupLayout jPanel16Layout = new javax.swing.GroupLayout(jPanel16);
        jPanel16.setLayout(jPanel16Layout);
        jPanel16Layout.setHorizontalGroup(
            jPanel16Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel16Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jTabbedPane2)
                .addContainerGap())
        );
        jPanel16Layout.setVerticalGroup(
            jPanel16Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel16Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jTabbedPane2, javax.swing.GroupLayout.PREFERRED_SIZE, 524, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(25, Short.MAX_VALUE))
        );

        jTabbedPane1.addTab("Weather", jPanel16);

        jScrollPane1.setHorizontalScrollBarPolicy(javax.swing.ScrollPaneConstants.HORIZONTAL_SCROLLBAR_ALWAYS);
        jScrollPane1.setVerticalScrollBarPolicy(javax.swing.ScrollPaneConstants.VERTICAL_SCROLLBAR_ALWAYS);
        jScrollPane1.setViewportView(connectionEditorPane);

        jScrollPane2.setHorizontalScrollBarPolicy(javax.swing.ScrollPaneConstants.HORIZONTAL_SCROLLBAR_ALWAYS);
        jScrollPane2.setVerticalScrollBarPolicy(javax.swing.ScrollPaneConstants.VERTICAL_SCROLLBAR_ALWAYS);
        jScrollPane2.setViewportView(comslogEditorPane);

        jLabel10.setFont(new java.awt.Font("Century Schoolbook L", 1, 18)); // NOI18N
        jLabel10.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel10.setText("Error Log");

        jLabel11.setFont(new java.awt.Font("Century Schoolbook L", 1, 18)); // NOI18N
        jLabel11.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel11.setText("Connection Log");

        jScrollPane3.setHorizontalScrollBarPolicy(javax.swing.ScrollPaneConstants.HORIZONTAL_SCROLLBAR_ALWAYS);
        jScrollPane3.setVerticalScrollBarPolicy(javax.swing.ScrollPaneConstants.VERTICAL_SCROLLBAR_ALWAYS);
        jScrollPane3.setViewportView(errorEditorPane);

        jLabel12.setFont(new java.awt.Font("Century Schoolbook L", 1, 18)); // NOI18N
        jLabel12.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel12.setText("Communication Log");

        javax.swing.GroupLayout jPanel13Layout = new javax.swing.GroupLayout(jPanel13);
        jPanel13.setLayout(jPanel13Layout);
        jPanel13Layout.setHorizontalGroup(
            jPanel13Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel13Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel13Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel13Layout.createSequentialGroup()
                        .addGroup(jPanel13Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING, false)
                            .addComponent(jScrollPane3)
                            .addComponent(jScrollPane2, javax.swing.GroupLayout.DEFAULT_SIZE, 918, Short.MAX_VALUE)
                            .addComponent(jScrollPane1)
                            .addComponent(jLabel10, javax.swing.GroupLayout.DEFAULT_SIZE, 918, Short.MAX_VALUE))
                        .addGap(0, 0, Short.MAX_VALUE))
                    .addComponent(jLabel12, javax.swing.GroupLayout.DEFAULT_SIZE, 957, Short.MAX_VALUE))
                .addContainerGap())
            .addGroup(jPanel13Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(jPanel13Layout.createSequentialGroup()
                    .addGap(22, 22, 22)
                    .addComponent(jLabel11, javax.swing.GroupLayout.DEFAULT_SIZE, 946, Short.MAX_VALUE)
                    .addGap(13, 13, 13)))
        );
        jPanel13Layout.setVerticalGroup(
            jPanel13Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel13Layout.createSequentialGroup()
                .addGap(42, 42, 42)
                .addComponent(jScrollPane1, javax.swing.GroupLayout.PREFERRED_SIZE, 84, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jLabel12)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jScrollPane2, javax.swing.GroupLayout.PREFERRED_SIZE, 191, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(jLabel10)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jScrollPane3, javax.swing.GroupLayout.DEFAULT_SIZE, 164, Short.MAX_VALUE)
                .addContainerGap())
            .addGroup(jPanel13Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(jPanel13Layout.createSequentialGroup()
                    .addGap(14, 14, 14)
                    .addComponent(jLabel11)
                    .addContainerGap(528, Short.MAX_VALUE)))
        );

        jTabbedPane1.addTab("Communications", jPanel13);

        jPanel15.setBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 0, 0), 1, true));

        startYearSpinner.setFont(new java.awt.Font("Century Schoolbook L", 1, 12)); // NOI18N
        startYearSpinner.setModel(new javax.swing.SpinnerNumberModel(2019, 2019, 2100, 1));
        startYearSpinner.setBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 0, 0), 1, true));
        startYearSpinner.addChangeListener(new javax.swing.event.ChangeListener() {
            public void stateChanged(javax.swing.event.ChangeEvent evt) {
                startYearSpinnerStateChanged(evt);
            }
        });

        jLabel13.setFont(new java.awt.Font("Century Schoolbook L", 1, 12)); // NOI18N
        jLabel13.setText("Start Date");

        startDaySpinner.setFont(new java.awt.Font("Century Schoolbook L", 1, 12)); // NOI18N
        startDaySpinner.setModel(new javax.swing.SpinnerNumberModel(1, 1, 31, 1));
        startDaySpinner.setBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 0, 0), 1, true));
        startDaySpinner.addChangeListener(new javax.swing.event.ChangeListener() {
            public void stateChanged(javax.swing.event.ChangeEvent evt) {
                startDaySpinnerStateChanged(evt);
            }
        });

        startMonthSpinner.setFont(new java.awt.Font("Century Schoolbook L", 1, 12)); // NOI18N
        startMonthSpinner.setModel(new javax.swing.SpinnerNumberModel(1, 1, 12, 1));
        startMonthSpinner.setBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 0, 0), 1, true));
        startMonthSpinner.addChangeListener(new javax.swing.event.ChangeListener() {
            public void stateChanged(javax.swing.event.ChangeEvent evt) {
                startMonthSpinnerStateChanged(evt);
            }
        });

        startHourSpinner.setFont(new java.awt.Font("Century Schoolbook L", 1, 12)); // NOI18N
        startHourSpinner.setModel(new javax.swing.SpinnerNumberModel(1, 0, 23, 1));
        startHourSpinner.setBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 0, 0), 1, true));
        startHourSpinner.addChangeListener(new javax.swing.event.ChangeListener() {
            public void stateChanged(javax.swing.event.ChangeEvent evt) {
                startHourSpinnerStateChanged(evt);
            }
        });

        startMinuteSpinner.setFont(new java.awt.Font("Century Schoolbook L", 1, 12)); // NOI18N
        startMinuteSpinner.setModel(new javax.swing.SpinnerNumberModel(1, 0, 59, 1));
        startMinuteSpinner.setBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 0, 0), 1, true));
        startMinuteSpinner.addChangeListener(new javax.swing.event.ChangeListener() {
            public void stateChanged(javax.swing.event.ChangeEvent evt) {
                startMinuteSpinnerStateChanged(evt);
            }
        });

        startSecondSpinner.setFont(new java.awt.Font("Century Schoolbook L", 1, 12)); // NOI18N
        startSecondSpinner.setModel(new javax.swing.SpinnerNumberModel(1, 0, 59, 1));
        startSecondSpinner.setBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 0, 0), 1, true));
        startSecondSpinner.addChangeListener(new javax.swing.event.ChangeListener() {
            public void stateChanged(javax.swing.event.ChangeEvent evt) {
                startSecondSpinnerStateChanged(evt);
            }
        });

        jLabel14.setFont(new java.awt.Font("Century Schoolbook L", 1, 12)); // NOI18N
        jLabel14.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel14.setText("Year");

        jLabel15.setFont(new java.awt.Font("Century Schoolbook L", 1, 12)); // NOI18N
        jLabel15.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel15.setText("Month");

        jLabel16.setFont(new java.awt.Font("Century Schoolbook L", 1, 12)); // NOI18N
        jLabel16.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel16.setText("Day");

        jLabel17.setFont(new java.awt.Font("Century Schoolbook L", 1, 12)); // NOI18N
        jLabel17.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel17.setText("Hour");

        jLabel18.setFont(new java.awt.Font("Century Schoolbook L", 1, 12)); // NOI18N
        jLabel18.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel18.setText("Minute");

        jLabel19.setFont(new java.awt.Font("Century Schoolbook L", 1, 12)); // NOI18N
        jLabel19.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel19.setText("Second");

        jLabel20.setFont(new java.awt.Font("Century Schoolbook L", 1, 12)); // NOI18N
        jLabel20.setText("End Date");

        endYearSpinner.setFont(new java.awt.Font("Century Schoolbook L", 1, 12)); // NOI18N
        endYearSpinner.setModel(new javax.swing.SpinnerNumberModel(2019, 2019, 2100, 1));
        endYearSpinner.setBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 0, 0), 1, true));
        endYearSpinner.addChangeListener(new javax.swing.event.ChangeListener() {
            public void stateChanged(javax.swing.event.ChangeEvent evt) {
                endYearSpinnerStateChanged(evt);
            }
        });

        endMonthSpinner.setFont(new java.awt.Font("Century Schoolbook L", 1, 12)); // NOI18N
        endMonthSpinner.setModel(new javax.swing.SpinnerNumberModel(1, 1, 12, 1));
        endMonthSpinner.setBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 0, 0), 1, true));
        endMonthSpinner.addChangeListener(new javax.swing.event.ChangeListener() {
            public void stateChanged(javax.swing.event.ChangeEvent evt) {
                endMonthSpinnerStateChanged(evt);
            }
        });

        endDaySpinner.setFont(new java.awt.Font("Century Schoolbook L", 1, 12)); // NOI18N
        endDaySpinner.setModel(new javax.swing.SpinnerNumberModel(1, 1, 31, 1));
        endDaySpinner.setBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 0, 0), 1, true));
        endDaySpinner.addChangeListener(new javax.swing.event.ChangeListener() {
            public void stateChanged(javax.swing.event.ChangeEvent evt) {
                endDaySpinnerStateChanged(evt);
            }
        });

        endHourSpinner.setFont(new java.awt.Font("Century Schoolbook L", 1, 12)); // NOI18N
        endHourSpinner.setModel(new javax.swing.SpinnerNumberModel(1, 0, 23, 1));
        endHourSpinner.setBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 0, 0), 1, true));
        endHourSpinner.addChangeListener(new javax.swing.event.ChangeListener() {
            public void stateChanged(javax.swing.event.ChangeEvent evt) {
                endHourSpinnerStateChanged(evt);
            }
        });

        endMinuteSpinner.setFont(new java.awt.Font("Century Schoolbook L", 1, 12)); // NOI18N
        endMinuteSpinner.setModel(new javax.swing.SpinnerNumberModel(1, 0, 59, 1));
        endMinuteSpinner.setBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 0, 0), 1, true));
        endMinuteSpinner.addChangeListener(new javax.swing.event.ChangeListener() {
            public void stateChanged(javax.swing.event.ChangeEvent evt) {
                endMinuteSpinnerStateChanged(evt);
            }
        });

        endSecondSpinner.setFont(new java.awt.Font("Century Schoolbook L", 1, 12)); // NOI18N
        endSecondSpinner.setModel(new javax.swing.SpinnerNumberModel(1, 0, 59, 1));
        endSecondSpinner.setBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 0, 0), 1, true));
        endSecondSpinner.addChangeListener(new javax.swing.event.ChangeListener() {
            public void stateChanged(javax.swing.event.ChangeEvent evt) {
                endSecondSpinnerStateChanged(evt);
            }
        });

        jLabel21.setFont(new java.awt.Font("Century Schoolbook L", 1, 12)); // NOI18N
        jLabel21.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel21.setText("Formated Timestamp");

        queryTimestampButton.setFont(new java.awt.Font("Century Schoolbook L", 1, 12)); // NOI18N
        queryTimestampButton.setText("Query");
        queryTimestampButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                queryTimestampButtonActionPerformed(evt);
            }
        });

        queryRangeButton.setFont(new java.awt.Font("Century Schoolbook L", 1, 12)); // NOI18N
        queryRangeButton.setText("Query Range");
        queryRangeButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                queryRangeButtonActionPerformed(evt);
            }
        });

        browseOutputDirctoryButton.setFont(new java.awt.Font("Century Schoolbook L", 1, 12)); // NOI18N
        browseOutputDirctoryButton.setText("Browse for output directory");
        browseOutputDirctoryButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                browseOutputDirctoryButtonActionPerformed(evt);
            }
        });

        jLabel22.setFont(new java.awt.Font("Century Schoolbook L", 1, 12)); // NOI18N
        jLabel22.setText("File Name");

        filenameTextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                filenameTextFieldActionPerformed(evt);
            }
        });

        exportAsCSVButton.setFont(new java.awt.Font("Century Schoolbook L", 1, 12)); // NOI18N
        exportAsCSVButton.setText("Export as CSV");
        exportAsCSVButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                exportAsCSVButtonActionPerformed(evt);
            }
        });

        javax.swing.GroupLayout jPanel15Layout = new javax.swing.GroupLayout(jPanel15);
        jPanel15.setLayout(jPanel15Layout);
        jPanel15Layout.setHorizontalGroup(
            jPanel15Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel15Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel15Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel15Layout.createSequentialGroup()
                        .addGroup(jPanel15Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                            .addGroup(jPanel15Layout.createSequentialGroup()
                                .addComponent(jLabel13)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addGroup(jPanel15Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                                    .addComponent(jLabel14, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                                    .addComponent(startYearSpinner, javax.swing.GroupLayout.DEFAULT_SIZE, 92, Short.MAX_VALUE)))
                            .addGroup(jPanel15Layout.createSequentialGroup()
                                .addComponent(jLabel20)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(endYearSpinner)))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(jPanel15Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addGroup(jPanel15Layout.createSequentialGroup()
                                .addGroup(jPanel15Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                    .addGroup(jPanel15Layout.createSequentialGroup()
                                        .addComponent(startMonthSpinner, javax.swing.GroupLayout.PREFERRED_SIZE, 62, javax.swing.GroupLayout.PREFERRED_SIZE)
                                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                        .addComponent(startDaySpinner, javax.swing.GroupLayout.PREFERRED_SIZE, 60, javax.swing.GroupLayout.PREFERRED_SIZE))
                                    .addGroup(jPanel15Layout.createSequentialGroup()
                                        .addComponent(jLabel15, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                                        .addGap(29, 29, 29)
                                        .addComponent(jLabel16, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)))
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addGroup(jPanel15Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                                    .addComponent(jLabel17, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                                    .addComponent(startHourSpinner, javax.swing.GroupLayout.DEFAULT_SIZE, 60, Short.MAX_VALUE)))
                            .addGroup(jPanel15Layout.createSequentialGroup()
                                .addComponent(endMonthSpinner, javax.swing.GroupLayout.PREFERRED_SIZE, 62, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(endDaySpinner, javax.swing.GroupLayout.PREFERRED_SIZE, 60, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                                .addComponent(endHourSpinner)))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(jPanel15Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                            .addComponent(endMinuteSpinner, javax.swing.GroupLayout.DEFAULT_SIZE, 62, Short.MAX_VALUE)
                            .addComponent(jLabel18, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .addComponent(startMinuteSpinner))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(jPanel15Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addGroup(jPanel15Layout.createSequentialGroup()
                                .addComponent(jLabel19, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                                .addGap(18, 18, 18)
                                .addComponent(jLabel21, javax.swing.GroupLayout.PREFERRED_SIZE, 183, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addGap(409, 409, 409))
                            .addGroup(jPanel15Layout.createSequentialGroup()
                                .addGroup(jPanel15Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                                    .addGroup(jPanel15Layout.createSequentialGroup()
                                        .addComponent(endSecondSpinner, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                                        .addComponent(endDatetimeTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 191, javax.swing.GroupLayout.PREFERRED_SIZE)
                                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                        .addComponent(queryRangeButton))
                                    .addGroup(jPanel15Layout.createSequentialGroup()
                                        .addComponent(startSecondSpinner, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                                        .addComponent(startDatetimeTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 191, javax.swing.GroupLayout.PREFERRED_SIZE)
                                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                        .addComponent(queryTimestampButton, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)))
                                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))))
                    .addGroup(jPanel15Layout.createSequentialGroup()
                        .addComponent(browseOutputDirctoryButton)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(outputDirectoryTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 326, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(jLabel22)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(filenameTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 116, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(exportAsCSVButton)
                        .addGap(0, 0, Short.MAX_VALUE))))
        );
        jPanel15Layout.setVerticalGroup(
            jPanel15Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel15Layout.createSequentialGroup()
                .addGap(7, 7, 7)
                .addGroup(jPanel15Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(jLabel14)
                    .addComponent(jLabel15)
                    .addComponent(jLabel16)
                    .addComponent(jLabel17)
                    .addComponent(jLabel18)
                    .addComponent(jLabel19)
                    .addComponent(jLabel21))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel15Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(startYearSpinner, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(jLabel13)
                    .addComponent(startDaySpinner, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(startMonthSpinner, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(startHourSpinner, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(startMinuteSpinner, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(startSecondSpinner, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(startDatetimeTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(queryTimestampButton))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel15Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(endYearSpinner, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(jLabel20)
                    .addComponent(endDaySpinner, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(endMonthSpinner, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(endHourSpinner, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(endMinuteSpinner, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(endSecondSpinner, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(endDatetimeTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(queryRangeButton))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addGroup(jPanel15Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(browseOutputDirctoryButton)
                    .addComponent(outputDirectoryTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(jLabel22)
                    .addComponent(filenameTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(exportAsCSVButton))
                .addContainerGap(21, Short.MAX_VALUE))
        );

        jLabel23.setFont(new java.awt.Font("Century Schoolbook L", 1, 18)); // NOI18N
        jLabel23.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel23.setText("Configure Query");

        jScrollPane4.setHorizontalScrollBarPolicy(javax.swing.ScrollPaneConstants.HORIZONTAL_SCROLLBAR_ALWAYS);
        jScrollPane4.setVerticalScrollBarPolicy(javax.swing.ScrollPaneConstants.VERTICAL_SCROLLBAR_ALWAYS);

        queryResultsTable.setModel(new javax.swing.table.DefaultTableModel(
            new Object [][] {
                {null, null, null, null},
                {null, null, null, null},
                {null, null, null, null},
                {null, null, null, null}
            },
            new String [] {
                "Title 1", "Title 2", "Title 3", "Title 4"
            }
        ));
        jScrollPane4.setViewportView(queryResultsTable);

        jLabel24.setFont(new java.awt.Font("Century Schoolbook L", 1, 18)); // NOI18N
        jLabel24.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel24.setText("Selected FITS Header");

        jScrollPane5.setHorizontalScrollBarPolicy(javax.swing.ScrollPaneConstants.HORIZONTAL_SCROLLBAR_ALWAYS);
        jScrollPane5.setVerticalScrollBarPolicy(javax.swing.ScrollPaneConstants.VERTICAL_SCROLLBAR_ALWAYS);
        jScrollPane5.setViewportView(SelectedFITSHeaderEditorPane);

        jLabel25.setFont(new java.awt.Font("Century Schoolbook L", 1, 18)); // NOI18N
        jLabel25.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel25.setText("Query Results");

        javax.swing.GroupLayout jPanel14Layout = new javax.swing.GroupLayout(jPanel14);
        jPanel14.setLayout(jPanel14Layout);
        jPanel14Layout.setHorizontalGroup(
            jPanel14Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel14Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel14Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(jLabel24, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addGroup(jPanel14Layout.createSequentialGroup()
                        .addGroup(jPanel14Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addGroup(jPanel14Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING, false)
                                .addComponent(jLabel23, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                                .addComponent(jPanel15, javax.swing.GroupLayout.PREFERRED_SIZE, 900, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addComponent(jScrollPane4))
                            .addComponent(jScrollPane5, javax.swing.GroupLayout.PREFERRED_SIZE, 902, javax.swing.GroupLayout.PREFERRED_SIZE))
                        .addGap(0, 55, Short.MAX_VALUE)))
                .addContainerGap())
            .addGroup(jPanel14Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(jPanel14Layout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(jLabel25, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addContainerGap()))
        );
        jPanel14Layout.setVerticalGroup(
            jPanel14Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel14Layout.createSequentialGroup()
                .addGap(6, 6, 6)
                .addComponent(jLabel23)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jPanel15, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addGap(30, 30, 30)
                .addComponent(jScrollPane4, javax.swing.GroupLayout.PREFERRED_SIZE, 163, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jLabel24)
                .addGap(12, 12, 12)
                .addComponent(jScrollPane5, javax.swing.GroupLayout.DEFAULT_SIZE, 138, Short.MAX_VALUE)
                .addContainerGap())
            .addGroup(jPanel14Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(jPanel14Layout.createSequentialGroup()
                    .addGap(196, 196, 196)
                    .addComponent(jLabel25)
                    .addContainerGap(346, Short.MAX_VALUE)))
        );

        jTabbedPane1.addTab("Query", jPanel14);

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addComponent(jTabbedPane1, javax.swing.GroupLayout.PREFERRED_SIZE, 993, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addGap(0, 0, Short.MAX_VALUE))
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(jTabbedPane1, javax.swing.GroupLayout.PREFERRED_SIZE, 604, javax.swing.GroupLayout.PREFERRED_SIZE)
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents

    private void FocusTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_FocusTextFieldActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_FocusTextFieldActionPerformed

    private void HATextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_HATextFieldActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_HATextFieldActionPerformed

    private void EquinoxTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_EquinoxTextFieldActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_EquinoxTextFieldActionPerformed

    private void AirmassTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_AirmassTextFieldActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_AirmassTextFieldActionPerformed

    private void CurrentRATextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_CurrentRATextFieldActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_CurrentRATextFieldActionPerformed

    private void OffsetRATextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_OffsetRATextFieldActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_OffsetRATextFieldActionPerformed

    private void OffsetDecTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_OffsetDecTextFieldActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_OffsetDecTextFieldActionPerformed

    private void CurrentDecTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_CurrentDecTextFieldActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_CurrentDecTextFieldActionPerformed

    private void LSTTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_LSTTextFieldActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_LSTTextFieldActionPerformed

    private void UTCTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_UTCTextFieldActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_UTCTextFieldActionPerformed

    private void AzimuthTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_AzimuthTextFieldActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_AzimuthTextFieldActionPerformed

    private void ZenithAngleTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_ZenithAngleTextFieldActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_ZenithAngleTextFieldActionPerformed

    private void CassRingAngleTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_CassRingAngleTextFieldActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_CassRingAngleTextFieldActionPerformed

    private void DomeAzimuthTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_DomeAzimuthTextFieldActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_DomeAzimuthTextFieldActionPerformed

    private void DomeShuttersTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_DomeShuttersTextFieldActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_DomeShuttersTextFieldActionPerformed

    private void WindScreensTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_WindScreensTextFieldActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_WindScreensTextFieldActionPerformed

    private void ConnectToTCSToggleButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_ConnectToTCSToggleButtonActionPerformed
        // TODO add your handling code here:
        boolean selected = ConnectToTCSToggleButton.isSelected();
        if(selected){
            myTelemetryController.connect();
            UpdateToggleButton.setEnabled(true);
        }
        if(!selected){
            myTelemetryController.disconnect();
            UpdateToggleButton.setEnabled(false);
        }
    }//GEN-LAST:event_ConnectToTCSToggleButtonActionPerformed

    private void UpdateToggleButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_UpdateToggleButtonActionPerformed
        // TODO add your handling code here:
        boolean selected = UpdateToggleButton.isSelected();
        if(myTelemetryController.isConnected()){
            if(selected){
                myTelemetryController.startPolling();
                UpdateToggleButton.setText("Updating");
            }
            if(!selected){
                myTelemetryController.stopPolling();
                UpdateToggleButton.setText("Update");
            }
        }
    }//GEN-LAST:event_UpdateToggleButtonActionPerformed

    private void DecOffsetTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_DecOffsetTextFieldActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_DecOffsetTextFieldActionPerformed

    private void RAOffsetTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_RAOffsetTextFieldActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_RAOffsetTextFieldActionPerformed

    private void TrackRateRATextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_TrackRateRATextFieldActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_TrackRateRATextFieldActionPerformed

    private void TrackRateDecTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_TrackRateDecTextFieldActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_TrackRateDecTextFieldActionPerformed

    private void ObjectNameTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_ObjectNameTextFieldActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_ObjectNameTextFieldActionPerformed

    private void motionStateTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_motionStateTextFieldActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_motionStateTextFieldActionPerformed

    private void startYearSpinnerStateChanged(javax.swing.event.ChangeEvent evt) {//GEN-FIRST:event_startYearSpinnerStateChanged
        startYear = ((Integer)startYearSpinner.getValue()).intValue();
        constructStartTimestamp();
    }//GEN-LAST:event_startYearSpinnerStateChanged

    private void startMonthSpinnerStateChanged(javax.swing.event.ChangeEvent evt) {//GEN-FIRST:event_startMonthSpinnerStateChanged
        startMonth = ((Integer)startMonthSpinner.getValue()).intValue();
        constructStartTimestamp();
    }//GEN-LAST:event_startMonthSpinnerStateChanged

    private void startDaySpinnerStateChanged(javax.swing.event.ChangeEvent evt) {//GEN-FIRST:event_startDaySpinnerStateChanged
        startDay = ((Integer)startDaySpinner.getValue()).intValue();
        constructStartTimestamp();
    }//GEN-LAST:event_startDaySpinnerStateChanged

    private void startHourSpinnerStateChanged(javax.swing.event.ChangeEvent evt) {//GEN-FIRST:event_startHourSpinnerStateChanged
        startHour = ((Integer)startHourSpinner.getValue()).intValue();  
        constructStartTimestamp();
    }//GEN-LAST:event_startHourSpinnerStateChanged

    private void startMinuteSpinnerStateChanged(javax.swing.event.ChangeEvent evt) {//GEN-FIRST:event_startMinuteSpinnerStateChanged
        startMinute = ((Integer)startMinuteSpinner.getValue()).intValue(); 
        constructStartTimestamp();
    }//GEN-LAST:event_startMinuteSpinnerStateChanged

    private void startSecondSpinnerStateChanged(javax.swing.event.ChangeEvent evt) {//GEN-FIRST:event_startSecondSpinnerStateChanged
        startSecond = ((Integer)startSecondSpinner.getValue()).intValue(); 
        constructStartTimestamp();
    }//GEN-LAST:event_startSecondSpinnerStateChanged

    private void endYearSpinnerStateChanged(javax.swing.event.ChangeEvent evt) {//GEN-FIRST:event_endYearSpinnerStateChanged
        endYear = ((Integer)endYearSpinner.getValue()).intValue();
        constructEndTimestamp();
    }//GEN-LAST:event_endYearSpinnerStateChanged

    private void endMonthSpinnerStateChanged(javax.swing.event.ChangeEvent evt) {//GEN-FIRST:event_endMonthSpinnerStateChanged
       endMonth = ((Integer)endMonthSpinner.getValue()).intValue();
       constructEndTimestamp();
    }//GEN-LAST:event_endMonthSpinnerStateChanged

    private void endDaySpinnerStateChanged(javax.swing.event.ChangeEvent evt) {//GEN-FIRST:event_endDaySpinnerStateChanged
       endDay = ((Integer)endDaySpinner.getValue()).intValue();
       constructEndTimestamp();
    }//GEN-LAST:event_endDaySpinnerStateChanged

    private void endHourSpinnerStateChanged(javax.swing.event.ChangeEvent evt) {//GEN-FIRST:event_endHourSpinnerStateChanged
       endHour = ((Integer)endHourSpinner.getValue()).intValue();
       constructEndTimestamp();
    }//GEN-LAST:event_endHourSpinnerStateChanged

    private void endMinuteSpinnerStateChanged(javax.swing.event.ChangeEvent evt) {//GEN-FIRST:event_endMinuteSpinnerStateChanged
       endMinute = ((Integer)endMinuteSpinner.getValue()).intValue(); 
       constructEndTimestamp();
    }//GEN-LAST:event_endMinuteSpinnerStateChanged

    private void endSecondSpinnerStateChanged(javax.swing.event.ChangeEvent evt) {//GEN-FIRST:event_endSecondSpinnerStateChanged
       endSecond = ((Integer)endSecondSpinner.getValue()).intValue(); 
       constructEndTimestamp();
    }//GEN-LAST:event_endSecondSpinnerStateChanged

    private void queryTimestampButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_queryTimestampButtonActionPerformed
        myTelemetryController.executeTimestampQuery(myTelemetryController.getStartTimestamp());
    }//GEN-LAST:event_queryTimestampButtonActionPerformed

    private void queryRangeButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_queryRangeButtonActionPerformed
        myTelemetryController.executeRangeQuery(myTelemetryController.getStartTimestamp(),myTelemetryController.getEndTimestamp());
    }//GEN-LAST:event_queryRangeButtonActionPerformed

    private void browseOutputDirctoryButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_browseOutputDirctoryButtonActionPerformed
       browseForOutputDirectory();
    }//GEN-LAST:event_browseOutputDirctoryButtonActionPerformed

    private void filenameTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_filenameTextFieldActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_filenameTextFieldActionPerformed

    private void exportAsCSVButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_exportAsCSVButtonActionPerformed
       writeCSV();
    }//GEN-LAST:event_exportAsCSVButtonActionPerformed
/*=============================================================================================
/      Constructor: SharedListSelectionHandler
/=============================================================================================*/
    class SharedListSelectionHandler implements ListSelectionListener {
        public void valueChanged(ListSelectionEvent e) {
          Point2D.Double p;
          ListSelectionModel lsm = (ListSelectionModel)e.getSource();
           lsm.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
//            int     firstIndex  = e.getFirstIndex();
//            int     lastIndex   = e.getLastIndex();
             boolean isAdjusting = lsm.getValueIsAdjusting();
            if(!isAdjusting){
            try{
             int     selectionView   = lsm.getLeadSelectionIndex();
             int     selectionModel  = queryResultsTable.convertRowIndexToModel(selectionView);
             int count = myTelemetryController.getTelemetryTableModel().getRowCount();
             TelemetryObject current = myTelemetryController.getTelemetryTableModel().getRecord(selectionModel);
             java.lang.String header_string = myTelemetryController.constructHeader(current);
             myHeaderLogModel.clearDocument();
             myHeaderLogModel.insertMessage(header_string);
             }catch(Exception ex){
   //            System.out.println(ex.toString());
               lsm.setLeadSelectionIndex(0);
             }
        } // end of if !isAdjusting
     }        
    }
/*=============================================================================================
/      HARenderer extends DefaultTableCellRenderer
/=============================================================================================*/
  static class TimestampRenderer extends DefaultTableCellRenderer {
    public TimestampRenderer() { super(); }
    public void setValue(Object value) {
        java.text.SimpleDateFormat sdf = new java.text.SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        setText((value == null) ? "" : sdf.format(value));
    }
}
    /**
     * @param args the command line arguments
     */
    public static void main(String args[]) {
        /* Set the Nimbus look and feel */
        //<editor-fold defaultstate="collapsed" desc=" Look and feel setting code (optional) ">
        /* If Nimbus (introduced in Java SE 6) is not available, stay with the default look and feel.
         * For details see http://download.oracle.com/javase/tutorial/uiswing/lookandfeel/plaf.html 
         */
        try {
            for (javax.swing.UIManager.LookAndFeelInfo info : javax.swing.UIManager.getInstalledLookAndFeels()) {
                if ("Nimbus".equals(info.getName())) {
                    javax.swing.UIManager.setLookAndFeel(info.getClassName());
                    break;
                }
            }
        } catch (ClassNotFoundException ex) {
            java.util.logging.Logger.getLogger(TelemetryServerFrame.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (InstantiationException ex) {
            java.util.logging.Logger.getLogger(TelemetryServerFrame.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (IllegalAccessException ex) {
            java.util.logging.Logger.getLogger(TelemetryServerFrame.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (javax.swing.UnsupportedLookAndFeelException ex) {
            java.util.logging.Logger.getLogger(TelemetryServerFrame.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        }
        //</editor-fold>

        /* Create and display the form */
        java.awt.EventQueue.invokeLater(new Runnable() {
            public void run() {
                new TelemetryServerFrame().setVisible(true);
            }
        });
    }

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JTextField AirmassTextField;
    private javax.swing.JTextField AzimuthTextField;
    private javax.swing.JTextField CassRingAngleTextField;
    private javax.swing.JToggleButton ConnectToTCSToggleButton;
    private javax.swing.JTextField CurrentDecTextField;
    private javax.swing.JTextField CurrentRATextField;
    private javax.swing.JTextField DecOffsetTextField;
    private javax.swing.JTextField DomeAzimuthTextField;
    private javax.swing.JTextField DomeShuttersTextField;
    private javax.swing.JTextField EquinoxTextField;
    private javax.swing.JTextField FocusTextField;
    private javax.swing.JTextField HATextField;
    private javax.swing.JLabel HAlabel1;
    private javax.swing.JTextField LSTTextField;
    private javax.swing.JTextField ObjectNameTextField;
    private javax.swing.JTextField OffsetDecTextField;
    private javax.swing.JTextField OffsetRATextField;
    private javax.swing.JLabel P48_average_windspeed_threshold;
    private javax.swing.JLabel P48_current_wind_direction;
    private javax.swing.JLabel P48_current_windspeed;
    private javax.swing.JLabel P48_inside_air_temperature;
    private javax.swing.JLabel P48_inside_dewpoint;
    private javax.swing.JLabel P48_inside_dewpoint_threshold;
    private javax.swing.JLabel P48_inside_relative_humidity;
    private javax.swing.JLabel P48_outside_air_temperature;
    private javax.swing.JLabel P48_outside_dewpoint;
    private javax.swing.JLabel P48_outside_dewpoint_threshold;
    private javax.swing.JLabel P48_outside_relative_humidity;
    private javax.swing.JLabel P48_peak_windspeed_threshold;
    private javax.swing.JLabel P48_remainging_holdtime;
    private javax.swing.JLabel P48_status;
    private javax.swing.JLabel P48_wetness;
    private javax.swing.JLabel P48_windspeed_average;
    private javax.swing.JLabel P60_barometric_pressure;
    private javax.swing.JLabel P60_floor_temperature;
    private javax.swing.JLabel P60_inside_dewpoint;
    private javax.swing.JLabel P60_inside_relative_humidity;
    private javax.swing.JLabel P60_inside_temperature;
    private javax.swing.JLabel P60_mirror_temperature;
    private javax.swing.JLabel P60_outside_dewpoint;
    private javax.swing.JLabel P60_outside_relative_humidity;
    private javax.swing.JLabel P60_outside_temperature;
    private javax.swing.JLabel P60_primary_cell_temperature;
    private javax.swing.JLabel P60_raining;
    private javax.swing.JLabel P60_secondary_cell_temperature;
    private javax.swing.JLabel P60_tube_air_temperature;
    private javax.swing.JLabel P60_tube_bottom_temperature;
    private javax.swing.JLabel P60_tube_middle_temperature;
    private javax.swing.JLabel P60_tube_top_temperature_label;
    private javax.swing.JLabel P60_wind_direction;
    private javax.swing.JLabel P60_wind_gusts;
    private javax.swing.JLabel P60_wind_median;
    private javax.swing.JLabel RALabel;
    private javax.swing.JLabel RALabel1;
    private javax.swing.JLabel RALabel10;
    private javax.swing.JLabel RALabel11;
    private javax.swing.JLabel RALabel2;
    private javax.swing.JLabel RALabel3;
    private javax.swing.JLabel RALabel4;
    private javax.swing.JLabel RALabel5;
    private javax.swing.JLabel RALabel6;
    private javax.swing.JLabel RALabel7;
    private javax.swing.JLabel RALabel8;
    private javax.swing.JLabel RALabel9;
    private javax.swing.JTextField RAOffsetTextField;
    private javax.swing.JTextField RAfield;
    private javax.swing.JLabel RateUnitsLabel;
    private javax.swing.JEditorPane SelectedFITSHeaderEditorPane;
    private javax.swing.JTextField TrackRateDecTextField;
    private javax.swing.JTextField TrackRateRATextField;
    private javax.swing.JTextField UTCTextField;
    private javax.swing.JToggleButton UpdateToggleButton;
    private javax.swing.JTextField WindScreensTextField;
    private javax.swing.JTextField ZenithAngleTextField;
    private javax.swing.JTextField airmassfield;
    private javax.swing.JLabel airmasslabel;
    private javax.swing.JTextField altitudefield;
    private javax.swing.JLabel altitudelabel;
    private javax.swing.JTextField antiparallacticfield;
    private javax.swing.JLabel antiparallacticlabel;
    private javax.swing.JTextField azimuthfield;
    private javax.swing.JLabel azimuthlabel;
    private javax.swing.JLabel barjdlabel;
    private javax.swing.JTextField baryjdfield;
    private javax.swing.JTextField baryvcorfield;
    private javax.swing.JLabel baryvcorlabel;
    private javax.swing.JButton browseOutputDirctoryButton;
    private javax.swing.JEditorPane comslogEditorPane;
    private javax.swing.JEditorPane connectionEditorPane;
    private javax.swing.JTextField constellationfield;
    private javax.swing.JLabel constellationlabel;
    private javax.swing.JTextField decfield;
    private javax.swing.JLabel declabel;
    private javax.swing.JTextField elevhorizonfield;
    private javax.swing.JLabel elevhorizonlabel;
    private javax.swing.JTextField elevseafield;
    private javax.swing.JLabel elevsealevellabel;
    private javax.swing.JTextField endDatetimeTextField;
    private javax.swing.JSpinner endDaySpinner;
    private javax.swing.JSpinner endHourSpinner;
    private javax.swing.JSpinner endMinuteSpinner;
    private javax.swing.JSpinner endMonthSpinner;
    private javax.swing.JSpinner endSecondSpinner;
    private javax.swing.JSpinner endYearSpinner;
    private javax.swing.JTextField equinoxfield;
    private javax.swing.JLabel equinoxlabel;
    private javax.swing.JEditorPane errorEditorPane;
    private javax.swing.JButton exportAsCSVButton;
    private javax.swing.JTextField filenameTextField;
    private javax.swing.JTextField hafield;
    private javax.swing.JLabel jLabel10;
    private javax.swing.JLabel jLabel11;
    private javax.swing.JLabel jLabel12;
    private javax.swing.JLabel jLabel13;
    private javax.swing.JLabel jLabel14;
    private javax.swing.JLabel jLabel15;
    private javax.swing.JLabel jLabel16;
    private javax.swing.JLabel jLabel17;
    private javax.swing.JLabel jLabel18;
    private javax.swing.JLabel jLabel19;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JLabel jLabel20;
    private javax.swing.JLabel jLabel21;
    private javax.swing.JLabel jLabel22;
    private javax.swing.JLabel jLabel23;
    private javax.swing.JLabel jLabel24;
    private javax.swing.JLabel jLabel25;
    private javax.swing.JLabel jLabel3;
    private javax.swing.JLabel jLabel4;
    private javax.swing.JLabel jLabel5;
    private javax.swing.JLabel jLabel6;
    private javax.swing.JLabel jLabel7;
    private javax.swing.JLabel jLabel8;
    private javax.swing.JLabel jLabel9;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JPanel jPanel10;
    private javax.swing.JPanel jPanel11;
    private javax.swing.JPanel jPanel12;
    private javax.swing.JPanel jPanel13;
    private javax.swing.JPanel jPanel14;
    private javax.swing.JPanel jPanel15;
    private javax.swing.JPanel jPanel16;
    private javax.swing.JPanel jPanel17;
    private javax.swing.JPanel jPanel18;
    private javax.swing.JPanel jPanel19;
    private javax.swing.JPanel jPanel2;
    private javax.swing.JPanel jPanel20;
    private javax.swing.JPanel jPanel21;
    private javax.swing.JPanel jPanel22;
    private javax.swing.JPanel jPanel23;
    private javax.swing.JPanel jPanel24;
    private javax.swing.JPanel jPanel25;
    private javax.swing.JPanel jPanel26;
    private javax.swing.JPanel jPanel27;
    private javax.swing.JPanel jPanel3;
    private javax.swing.JPanel jPanel4;
    private javax.swing.JPanel jPanel5;
    private javax.swing.JPanel jPanel6;
    private javax.swing.JPanel jPanel7;
    private javax.swing.JPanel jPanel8;
    private javax.swing.JPanel jPanel9;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JScrollPane jScrollPane2;
    private javax.swing.JScrollPane jScrollPane3;
    private javax.swing.JScrollPane jScrollPane4;
    private javax.swing.JScrollPane jScrollPane5;
    private javax.swing.JTabbedPane jTabbedPane1;
    private javax.swing.JTabbedPane jTabbedPane2;
    private javax.swing.JTextField jdfield;
    private javax.swing.JLabel jdlabel;
    private javax.swing.JLabel jdlabel1;
    private javax.swing.JTextField latitudefield;
    private javax.swing.JLabel latitudelabel;
    private javax.swing.JTextField localdatefield;
    private javax.swing.JLabel localdatelabel;
    private javax.swing.JTextField localtimefield;
    private javax.swing.JLabel localtimelabel;
    private javax.swing.JTextField longitudefield;
    private javax.swing.JLabel longitudelabel;
    private javax.swing.JTextField lunarskybrightfield;
    private javax.swing.JLabel lunarskybrightlabel;
    private javax.swing.JTextField moonaltfield;
    private javax.swing.JLabel moonaltlabel;
    private javax.swing.JTextField moonazfield;
    private javax.swing.JLabel moonazimuthlabel;
    private javax.swing.JTextField moondecfield;
    private javax.swing.JLabel moondeclabel;
    private javax.swing.JTextField moonillumfield;
    private javax.swing.JLabel moonillumfraclabel;
    private javax.swing.JLabel moonobjanglabel;
    private javax.swing.JTextField moonobjanglefield;
    private javax.swing.JTextField moonphasefield;
    private javax.swing.JLabel moonphaselabel;
    private javax.swing.JTextField moonrafield;
    private javax.swing.JLabel moonralabel;
    private javax.swing.JTextField motionStateTextField;
    private javax.swing.JTextField objectnamefield;
    private javax.swing.JLabel objectnamelabel;
    private javax.swing.JTextField observatoryfield;
    private javax.swing.JLabel observatorylabel;
    private javax.swing.JLabel observatoryparamlabel;
    private javax.swing.JTextField outputDirectoryTextField;
    private javax.swing.JLabel p200_arch_l;
    private javax.swing.JLabel p200_arch_r;
    private javax.swing.JLabel p200_average_windspeed;
    private javax.swing.JLabel p200_average_windspeed_label;
    private javax.swing.JLabel p200_average_windspeed_label1;
    private javax.swing.JLabel p200_average_windspeed_label10;
    private javax.swing.JLabel p200_average_windspeed_label100;
    private javax.swing.JLabel p200_average_windspeed_label101;
    private javax.swing.JLabel p200_average_windspeed_label102;
    private javax.swing.JLabel p200_average_windspeed_label103;
    private javax.swing.JLabel p200_average_windspeed_label104;
    private javax.swing.JLabel p200_average_windspeed_label105;
    private javax.swing.JLabel p200_average_windspeed_label106;
    private javax.swing.JLabel p200_average_windspeed_label107;
    private javax.swing.JLabel p200_average_windspeed_label108;
    private javax.swing.JLabel p200_average_windspeed_label109;
    private javax.swing.JLabel p200_average_windspeed_label11;
    private javax.swing.JLabel p200_average_windspeed_label110;
    private javax.swing.JLabel p200_average_windspeed_label111;
    private javax.swing.JLabel p200_average_windspeed_label112;
    private javax.swing.JLabel p200_average_windspeed_label113;
    private javax.swing.JLabel p200_average_windspeed_label114;
    private javax.swing.JLabel p200_average_windspeed_label115;
    private javax.swing.JLabel p200_average_windspeed_label116;
    private javax.swing.JLabel p200_average_windspeed_label117;
    private javax.swing.JLabel p200_average_windspeed_label118;
    private javax.swing.JLabel p200_average_windspeed_label12;
    private javax.swing.JLabel p200_average_windspeed_label13;
    private javax.swing.JLabel p200_average_windspeed_label134;
    private javax.swing.JLabel p200_average_windspeed_label135;
    private javax.swing.JLabel p200_average_windspeed_label14;
    private javax.swing.JLabel p200_average_windspeed_label15;
    private javax.swing.JLabel p200_average_windspeed_label16;
    private javax.swing.JLabel p200_average_windspeed_label17;
    private javax.swing.JLabel p200_average_windspeed_label18;
    private javax.swing.JLabel p200_average_windspeed_label19;
    private javax.swing.JLabel p200_average_windspeed_label2;
    private javax.swing.JLabel p200_average_windspeed_label20;
    private javax.swing.JLabel p200_average_windspeed_label21;
    private javax.swing.JLabel p200_average_windspeed_label22;
    private javax.swing.JLabel p200_average_windspeed_label23;
    private javax.swing.JLabel p200_average_windspeed_label24;
    private javax.swing.JLabel p200_average_windspeed_label25;
    private javax.swing.JLabel p200_average_windspeed_label26;
    private javax.swing.JLabel p200_average_windspeed_label27;
    private javax.swing.JLabel p200_average_windspeed_label28;
    private javax.swing.JLabel p200_average_windspeed_label29;
    private javax.swing.JLabel p200_average_windspeed_label3;
    private javax.swing.JLabel p200_average_windspeed_label30;
    private javax.swing.JLabel p200_average_windspeed_label31;
    private javax.swing.JLabel p200_average_windspeed_label32;
    private javax.swing.JLabel p200_average_windspeed_label36;
    private javax.swing.JLabel p200_average_windspeed_label39;
    private javax.swing.JLabel p200_average_windspeed_label4;
    private javax.swing.JLabel p200_average_windspeed_label40;
    private javax.swing.JLabel p200_average_windspeed_label43;
    private javax.swing.JLabel p200_average_windspeed_label44;
    private javax.swing.JLabel p200_average_windspeed_label47;
    private javax.swing.JLabel p200_average_windspeed_label5;
    private javax.swing.JLabel p200_average_windspeed_label50;
    private javax.swing.JLabel p200_average_windspeed_label6;
    private javax.swing.JLabel p200_average_windspeed_label7;
    private javax.swing.JLabel p200_average_windspeed_label71;
    private javax.swing.JLabel p200_average_windspeed_label72;
    private javax.swing.JLabel p200_average_windspeed_label73;
    private javax.swing.JLabel p200_average_windspeed_label74;
    private javax.swing.JLabel p200_average_windspeed_label75;
    private javax.swing.JLabel p200_average_windspeed_label76;
    private javax.swing.JLabel p200_average_windspeed_label77;
    private javax.swing.JLabel p200_average_windspeed_label78;
    private javax.swing.JLabel p200_average_windspeed_label79;
    private javax.swing.JLabel p200_average_windspeed_label8;
    private javax.swing.JLabel p200_average_windspeed_label9;
    private javax.swing.JLabel p200_average_windspeed_label94;
    private javax.swing.JLabel p200_average_windspeed_label95;
    private javax.swing.JLabel p200_average_windspeed_label96;
    private javax.swing.JLabel p200_catwalk;
    private javax.swing.JLabel p200_crane;
    private javax.swing.JLabel p200_dewpoint;
    private javax.swing.JLabel p200_exhaust;
    private javax.swing.JLabel p200_fanl;
    private javax.swing.JLabel p200_fanr;
    private javax.swing.JLabel p200_floor_temperature;
    private javax.swing.JLabel p200_gantry_ur;
    private javax.swing.JLabel p200_gantryll;
    private javax.swing.JLabel p200_gantrylr;
    private javax.swing.JLabel p200_gantryul;
    private javax.swing.JLabel p200_grantry_far_l;
    private javax.swing.JLabel p200_grantry_far_r;
    private javax.swing.JLabel p200_gust_windspeed;
    private javax.swing.JLabel p200_inside_temperature;
    private javax.swing.JLabel p200_outside_temperature;
    private javax.swing.JLabel p200_peak_windspeed;
    private javax.swing.JLabel p200_primary_cell_temperature;
    private javax.swing.JLabel p200_primary_mirror_temperature;
    private javax.swing.JLabel p200_raining;
    private javax.swing.JLabel p200_secondary_mirror_temperature;
    private javax.swing.JLabel p200_shutter;
    private javax.swing.JLabel p200_stairs0;
    private javax.swing.JLabel p200_stairs20;
    private javax.swing.JLabel p200_stairs40;
    private javax.swing.JLabel p200_stairs60;
    private javax.swing.JLabel p200_stairs75;
    private javax.swing.JLabel p200_stairs90;
    private javax.swing.JLabel p200_utc_timestamp;
    private javax.swing.JLabel p200_wind_direction;
    private javax.swing.JLabel p48_alarm_holdtime;
    private javax.swing.JLabel p48_utc_timestamp;
    private javax.swing.JLabel p60_tube_top_temperature;
    private javax.swing.JLabel p60_utc_timestamp;
    private javax.swing.JTextField parallacticfield;
    private javax.swing.JLabel parallacticlabel1;
    private javax.swing.JTextField planetproximfield;
    private javax.swing.JLabel planetproximlabel;
    private javax.swing.JButton queryRangeButton;
    private javax.swing.JTable queryResultsTable;
    private javax.swing.JButton queryTimestampButton;
    private javax.swing.JLabel ralabel;
    private javax.swing.JTextField siderialfield;
    private javax.swing.JLabel sideriallabel1;
    private javax.swing.JLabel sideriallabel9;
    private javax.swing.JTextField startDatetimeTextField;
    private javax.swing.JSpinner startDaySpinner;
    private javax.swing.JSpinner startHourSpinner;
    private javax.swing.JSpinner startMinuteSpinner;
    private javax.swing.JSpinner startMonthSpinner;
    private javax.swing.JSpinner startSecondSpinner;
    private javax.swing.JSpinner startYearSpinner;
    private javax.swing.JTextField sunaltfield;
    private javax.swing.JLabel sunaltlabel;
    private javax.swing.JTextField sunazfield;
    private javax.swing.JLabel sunazimuthlabel;
    private javax.swing.JTextField sundecfield;
    private javax.swing.JLabel sundeclabel;
    private javax.swing.JTextField sunrafield;
    private javax.swing.JLabel sunralabel;
    private javax.swing.JLabel timelabel;
    private javax.swing.JLabel timelabel1;
    private javax.swing.JLabel titleImageLabel;
    private javax.swing.JTextField utdatefield;
    private javax.swing.JLabel utdatelabel;
    private javax.swing.JTextField uttimefield;
    private javax.swing.JLabel uttimelabel;
    private javax.swing.JTextField ztwilightfield;
    private javax.swing.JLabel ztwilightlabel;
    // End of variables declaration//GEN-END:variables
}
