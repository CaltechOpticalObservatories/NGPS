
/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/GUIForms/JFrame.java to edit this template
 */
package edu.caltech.palomar.instruments.ngps.gui; 

import edu.caltech.palomar.dhe2.ObservationSequencerObject;
import edu.caltech.palomar.instruments.ngps.dbms.NGPSdatabase;
import edu.caltech.palomar.instruments.ngps.dbms.edit_monitor;
import edu.caltech.palomar.instruments.ngps.object.GlobalPreferencesModel;
import edu.caltech.palomar.instruments.ngps.object.Target;
import edu.caltech.palomar.instruments.ngps.os.ObservationSequencerController;
import edu.caltech.palomar.instruments.ngps.otm.OTMlauncher;
import edu.caltech.palomar.instruments.ngps.parser.TargetListParser2;
import edu.caltech.palomar.instruments.ngps.server.SimulationServer;
import edu.caltech.palomar.instruments.ngps.tables.ETCParameterTableModel;
import edu.caltech.palomar.instruments.ngps.tables.EditTableInterface;
import edu.caltech.palomar.instruments.ngps.tables.MasterDBTableModel;
import edu.caltech.palomar.instruments.ngps.tables.MultiLineHeaderRenderer;
import edu.caltech.palomar.instruments.ngps.tables.OTMParameterTableModel;
import edu.caltech.palomar.instruments.ngps.tables.ParametersTableModel;
import edu.caltech.palomar.instruments.ngps.util.BrowserDisplay;
import edu.caltech.palomar.instruments.ngps.util.TableRowTransferHandler;
import edu.caltech.palomar.telescopes.P200.P200Component;
import edu.caltech.palomar.util.general.CommandLogModel;
import edu.dartmouth.jskycalc.coord.InstantInTime;
import edu.dartmouth.jskycalc.coord.Site;
import edu.dartmouth.jskycalc.gui.NightlyWindow;
import edu.dartmouth.jskycalc.objects.NightlyAlmanac;
import edu.dartmouth.jskycalc.objects.WhenWhere;
import java.awt.Color;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Rectangle;
import java.awt.RenderingHints;
import java.awt.Toolkit;
import java.awt.Window;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.beans.PropertyChangeEvent;
import java.io.File;
import java.io.FileInputStream;
import java.sql.Timestamp;
import java.util.Calendar;
import java.util.Properties;
import java.util.Stack;
import javax.swing.Box;
import javax.swing.ButtonGroup;
import javax.swing.DropMode;
import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JFileChooser;
import javax.swing.JMenu;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPopupMenu;
import javax.swing.JTable;
import javax.swing.ListSelectionModel;
import javax.swing.SpinnerNumberModel;
import javax.swing.border.MatteBorder;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;
import javax.swing.filechooser.FileNameExtensionFilter;
import javax.swing.plaf.basic.BasicProgressBarUI;
import javax.swing.table.DefaultTableCellRenderer;
import javax.swing.table.TableCellRenderer;
import org.jdatepicker.impl.JDatePanelImpl;
import org.jdatepicker.impl.JDatePickerImpl;
import org.jdatepicker.impl.UtilDateModel;
//import javax.swing.JFileChooser.FileNameExtensionFilter;
        /**.
 *
 * @author jennifermilburn
 */
public class NGPSFrame extends javax.swing.JFrame {
  public NGPSdatabase              dbms;
  public TargetListParser2         myTargetListParser;
  public ObservationSequencerController myObservationSequencerController;
  public importCSVPanel            myimportCSVPanel;
//  public TargetSetPanel            myTargetSetPanel;
//  public retrieveDatabasePanel     myretrieveDatabasePanel;
  public ParametersTableModel      myParametersTableModel;
  public OTMParameterTableModel    myOTMParametersTableModel;
  public ETCParameterTableModel    myETCParameterTableModel;
  public ObservationSequencerObject myObservationSequencerObject;
  public ConnectionsFrame           myConnectionsFrame;
  public ImportFrame                myImportFrame; 
  private  java.lang.String        initialDirectory        = new java.lang.String();
  private  java.lang.String        target_set_label = new java.lang.String();
  private  int                     selected_table_row;
  private  int[]                   selected_table_rows;
  public   Stack                   copy_stack        = new Stack();
  private  boolean                 paste_state;
  public   JTable                  main_editor_table; 
  public SimulationServer          mySimulationServer;
  public static int                SERVER_PORT = 5690;
  public  JButton                  STARTUP;
  public  JButton                  STOP;
  public  JButton                  GO;
  public  JButton                  ABORT;
  public  JButton                  PAUSE;
  public  JButton                  RESUME;
  private JMenu                    accountMenu;
  private JMenuItem                myaccount_MenuItem;
  private JMenuItem                switch_account_MenuItem;
  private JMenuItem                sign_out_MenuItem;  
  private ButtonGroup              do_one_do_all_ButtonGroup;
  private TargetSetFrame           myTargetSetFrame;
  private signInFrame              my_signInFrame;
  private CreateOwnerFrame         myCreateOwnerFrame;
  public P200Component             myP200Component;
  public DateTimeChooserFrame      myDateTimeChooserFrame;
  public NightlyAlmanac            myNightlyAlmanac;
  public NightlyWindow             myNightlyWindow;
  public JDatePanelImpl            datePanel;
  public JDatePickerImpl           datePicker;
//  private JTextField               timeTextField;
//  private JTextField               seeingTextField;
//  private JTextField               sky_backgroundTextField;
  public java.lang.String          USERDIR = System.getProperty("user.dir");
  public java.lang.String          SEP     = System.getProperty("file.separator");
  public java.lang.String          CONFIG  = new java.lang.String("config");
  public java.lang.String          NGPS_PROPERTIES = new java.lang.String("ngps.ini");
  public java.lang.String          IMAGE_CACHE     = new java.lang.String("images");
  public double                    DEFAULT_SEEING;
  public int                       DEFAULT_WAVELENGTH;
  public double                    DEFAULT_AIRMASS_LIMIT;
  private int                      DEFAULT_FONT = 12;
  private int                      CURRENT_FONT;
  private java.lang.String         CURRENT_FONT_NAME = "Ariel";
  private ImageIcon                ON;
  private ImageIcon                OFF;
  private ImageIcon                UNKNOWN;
  private ImageIcon                ACCEPT_ON;
  private ImageIcon                ACCEPT_OFF;
  private ImageIcon                ACCEPT_SAVE_NEEDED;
  private ImageIcon                CANCEL_ON;
  private ImageIcon                CANCEL_OFF;
  public CommandLogModel           myCommandLogModel = new CommandLogModel();
  public OTMoutputFrame            myOTMoutputFrame;
  public SpinnerNumberModel        hoursSpinnerModel;
  public SpinnerNumberModel        minutesSpinnerModel;
  private int                      hours;
  private int                      minutes;
  public static String [] palomarsite = {"Palomar Mountain",  "7.79089",  "33.35667",  "8.",  "1",  "Pacific",  "P",  "1706.",  "1706."};
  public Site                      palomar = new Site(palomarsite);
  public GlobalPreferencesModel    myGlobalPreferencesModel;
  public EngineeringFrame          myEngineeringFrame ;
  public edit_monitor              myedit_monitor;
  public TargetSearchFrame         myTargetSearchFrame;
  public JMenuItem                 paste_menu_item ;
  public JMenuItem                 insert_copied_menu_item;
  public aboutNGPSFrame            my_aboutNGPSFrame;
  public int                       CONFIGURATION;
  public static int                OBSERVE = 1;
  public static int                PLAN = 2;
  public BrowserDisplay            myBrowserDisplay = new BrowserDisplay();
//  public OScontrolsPanel           myOScontrolsPanel;
  public ChangePasswordFrame       myChangePasswordFrame; 
/*=============================================================================================
/     NGPSFrame()
/=============================================================================================*/
    public NGPSFrame(String args[]) {
         if(args[0].toUpperCase().matches("OBSERVE")){
            CONFIGURATION = OBSERVE;   
         }
         if(args[0].toUpperCase().matches("PLAN")){
            CONFIGURATION = PLAN;   
         } 
        initComponents();
//        initializePanels();
        initializeTargetSetFrame();
        initializeSignInFrame();
        initializeCreateOwnerFrame();
        initializeDatabase();
        initializeParser();   
        initializeMainTable();
        initializeStateMonitor();
//        initializeActionButtons(); 
        mySimulationServer.myObservationSequencerObject.setSTATE("STOPPED");
        initializeCSVFrame(); 
        initializeRightMenu();
        initializeJSkyCalcModel();
        readProperties();
        initializeIcons();
        setFontSize(DEFAULT_FONT);
        main_editor_table.setFont(new Font(CURRENT_FONT_NAME, Font.BOLD,CURRENT_FONT));
        otm_state_Label.setIcon(UNKNOWN);
        dbms_stateLabel.setIcon(UNKNOWN);
        acceptButton.setIcon(ACCEPT_OFF);
        cancelButton.setIcon(CANCEL_OFF);
        initializeOTMoutputFrame();
        initializeChangePasswordFrame();
        initializeSpinners();        
        initializeTimestamp();    
        initializeConnectionsFrame();
        if(CONFIGURATION == OBSERVE){
            initializeObservationSequencerController();
            initializeEngineeringFrame();
            myConnectionsFrame.setDBMS(dbms);
            setTitle("OBSERVING");
            pack();
        }
        if(CONFIGURATION == PLAN){
            left_mainPanel.remove(myOScontrolsPanel);
            myConnectionsFrame.setDBMS(dbms);
            planningPanel.remove(fetchLiveButton);
            planningPanel.remove(auto_start_timeCheckBox);
            planningPanel.remove(auto_fetchCheckBox);            
            setTitle("PLANNING");
            pack();
        }        
        initializeGlobalPreferences();
        initializeQueryTargetsTool();
        initializeAboutFrame();

        centreWindow(this);
    }
/*=============================================================================================
/     initializePanels()
/=============================================================================================*/
  public void initializePanels(){
//      myimportCSVPanel = new importCSVPanel();
//      myTargetSetPanel = new TargetSetPanel();
//      myretrieveDatabasePanel = new retrieveDatabasePanel();
  }   
  public void initializeCSVFrame(){
      myImportFrame = new ImportFrame();
      myImportFrame.setDBMS(dbms);
      myImportFrame.setParser(myTargetListParser);
      myImportFrame.setNGPSFrame(this);
  }
  public void initializeTargetSetFrame(){
      myTargetSetFrame = new TargetSetFrame();    
      myTargetSetFrame.setVisible(false);
  }
  public void initializeSignInFrame(){
      my_signInFrame = new signInFrame();
      my_signInFrame.setVisible(false);
      my_signInFrame.setNGPSFrame(this);
  }
  public void initializeCreateOwnerFrame(){
      myCreateOwnerFrame = new CreateOwnerFrame();
      myCreateOwnerFrame.setVisible(false);
  }
  public void initializeOTMoutputFrame(){
      myOTMoutputFrame = new OTMoutputFrame();
      myOTMoutputFrame.setDBMS(dbms);
  }
  public void initializeChangePasswordFrame(){
      myChangePasswordFrame = new ChangePasswordFrame();
      myChangePasswordFrame.setDBMS(dbms);
  }
  public static void centreWindow(Window frame) {
    Dimension dimension = Toolkit.getDefaultToolkit().getScreenSize();
    int x = (int) ((dimension.getWidth() - frame.getWidth()) / 2);
    int y = (int) ((dimension.getHeight() - frame.getHeight()) / 2);
    frame.setLocation(x, y);
}
  public void initializeAboutFrame(){
      my_aboutNGPSFrame = new aboutNGPSFrame();
  }
  public void initializeConnectionsFrame(){
      myConnectionsFrame = new ConnectionsFrame();
      myConnectionsFrame.setVisible(false);
  }
/*=============================================================================================
/     setFontSize(int new_font_size)
/=============================================================================================*/ 
public void setFontSize(int new_font_size){
   CURRENT_FONT =  new_font_size;
} 
public int getFontSize(){
    return CURRENT_FONT;
}
/*=============================================================================================
/     increase and decrease font size for main table
/=============================================================================================*/ 
public void increaseFontSize(){
    CURRENT_FONT = CURRENT_FONT+1;
    main_editor_table.setFont(new Font(CURRENT_FONT_NAME, Font.BOLD,CURRENT_FONT)); 
}
public void decreaseFontSize(){
    CURRENT_FONT = CURRENT_FONT-1;
    main_editor_table.setFont(new Font(CURRENT_FONT_NAME, Font.BOLD,CURRENT_FONT));    
}
/*=============================================================================================ls
/     initializePanels()
/=============================================================================================*/ 
public JMenu getAccountMenu(){
    return accountMenu;
}  
/*================================================================================================
/     initializeSpinners()
/=================================================================================================*/
 private void initializeSpinners(){
    hoursSpinnerModel      =  new SpinnerNumberModel(3,0,24,1);
    minutesSpinnerModel    =  new SpinnerNumberModel(0,0,60,1);
    HoursSpinner.setModel(     hoursSpinnerModel);
    MinutesSpinner.setModel(   minutesSpinnerModel);
 }
 /*================================================================================================
/     initializeGlobalPreferences()
/=================================================================================================*/
private void initializeGlobalPreferences(){
     myGlobalPreferencesModel = new GlobalPreferencesModel();
     dbms.myOTMlauncher.setGlobalPreferencesModel(myGlobalPreferencesModel);
 }
/*=============================================================================================
/    readProperties()
/=============================================================================================*/ 
private void initializeQueryTargetsTool(){
   myTargetSearchFrame = new TargetSearchFrame();
   myTargetSearchFrame.setNGPSFrame(this);
   myTargetSearchFrame.setNGPSdatabase(dbms);
   myTargetSearchFrame.setVisible(false);
}
/*=============================================================================================
/    readProperties()
/=============================================================================================*/ 
private void readProperties(){
   try{
     java.lang.String NGPS_PROPERTIES_FILE = USERDIR + SEP + CONFIG + SEP + NGPS_PROPERTIES;
      FileInputStream  ngps_properties_file            = new FileInputStream(NGPS_PROPERTIES_FILE);
      Properties       ngps_properties                 = new Properties();
      ngps_properties.load(ngps_properties_file);
      ngps_properties_file.close();
      DEFAULT_SEEING         = Double.parseDouble(ngps_properties.getProperty("DEFAULT_SEEING"));
      DEFAULT_WAVELENGTH     = Integer.parseInt(ngps_properties.getProperty("DEFAULT_WAVELENGTH"));
      DEFAULT_AIRMASS_LIMIT  = Double.parseDouble(ngps_properties.getProperty("DEFAULT_AIRMASS_LIMIT"));
      seeingTextField.setText(Double.toString(DEFAULT_SEEING));
      airmass_limitTextField.setText(Double.toString(DEFAULT_AIRMASS_LIMIT));
      dbms.myOTMlauncher.setSeeing(DEFAULT_SEEING);
      dbms.myOTMlauncher.setWavelength(DEFAULT_WAVELENGTH);
   }catch(Exception e){
       System.out.println(e.toString());
   }  
}
/*================================================================================================
/        initializeIcons()
/=================================================================================================*/
  public void initializeIcons(){
    ON                   = new ImageIcon(USERDIR + SEP + IMAGE_CACHE + SEP + "ON.gif");
    OFF                  = new ImageIcon(USERDIR + SEP + IMAGE_CACHE + SEP + "OFF.gif");
    UNKNOWN              = new ImageIcon(USERDIR + SEP + IMAGE_CACHE + SEP + "UNKNOWN.gif");
//    ACCEPT_OFF           = new ImageIcon(USERDIR + SEP + IMAGE_CACHE + SEP +"database-arrow-down-icon2_greencheck.png");
//    ACCEPT_ON            = new ImageIcon(USERDIR + SEP + IMAGE_CACHE + SEP +"database-arrow-down-icon2_redcheck.png");
    ACCEPT_OFF           = new ImageIcon(USERDIR + SEP + IMAGE_CACHE + SEP +"greencheck_64.png");
    ACCEPT_ON            = new ImageIcon(USERDIR + SEP + IMAGE_CACHE + SEP +"red_check_64.png");
//    ACCEPT_SAVE_NEEDED   = new ImageIcon(USERDIR + SEP + IMAGE_CACHE + SEP +"database-arrow-down-icon2_yellowcheck.png");
    ACCEPT_SAVE_NEEDED   = new ImageIcon(USERDIR + SEP + IMAGE_CACHE + SEP +"orangecheck_64.png");
//    CANCEL_OFF           = new ImageIcon(USERDIR + SEP + IMAGE_CACHE + SEP +"database-arrow-up-icon2_redX.png");
//    CANCEL_ON            = new ImageIcon(USERDIR + SEP + IMAGE_CACHE + SEP +"database-arrow-up-icon2_greenX.png");
    CANCEL_OFF           = new ImageIcon(USERDIR + SEP + IMAGE_CACHE + SEP +"cancelcheck_red_64.png");
    CANCEL_ON            = new ImageIcon(USERDIR + SEP + IMAGE_CACHE + SEP +"cancelcheck_green_64.png");
//    detailedTabbedPane.setForegroundAt(0, Color.BLACK);
//    detailedTabbedPane.setBackgroundAt(0, Color.yellow);
  }
/*=============================================================================================
/     initializeJSkyCalcModel()
/=============================================================================================*/ 
public void initializeTimestamp(){
        java.util.Date mydate = (java.util.Date)(datePanel.getModel().getValue());
       java.time.ZonedDateTime selected_zonedDateTime = java.time.ZonedDateTime.ofInstant(mydate.toInstant(),java.time.ZoneOffset.UTC); 
       java.time.LocalDateTime withoutTimezone = selected_zonedDateTime.toLocalDateTime();               
       Timestamp selected_timestamp = Timestamp.valueOf(withoutTimezone);
       int hours = (Integer)hoursSpinnerModel.getValue();
       int minutes = (Integer)minutesSpinnerModel.getValue();
       selected_timestamp.setHours(hours);
       selected_timestamp.setMinutes(minutes);
       dbms.myOTMlauncher.setStartTimestamp(selected_timestamp);
       java.lang.String[] selected_date_time = constructJSkyCalcDateTime(selected_timestamp);
       java.lang.String timestamp_string = selected_timestamp.toString();
       timestamp_string = timestamp_string.replace("-", " ");
       timestamp_string = timestamp_string.replace(":", " ");
       InstantInTime i = new InstantInTime(timestamp_string,palomar.stdz,palomar.use_dst,true);              
       WhenWhere w = new WhenWhere(i,palomar);               
       NightlyAlmanac current = new NightlyAlmanac(w);
//               myNightlyAlmanac.Update(w);
       myNightlyWindow = new NightlyWindow(current);
       myNightlyWindow.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
              public void propertyChange(java.beans.PropertyChangeEvent e) {
                 nightly_propertyChange(e);
              }
            });
       myP200Component.getTelescopeObject().getJSkyCalcModel().setToDate(selected_date_time[0],selected_date_time[1]);
       myP200Component.getTelescopeObject().getJSkyCalcModel().synchOutput();
       myNightlyAlmanac.Update(myP200Component.getTelescopeObject().getJSkyCalcModel().getWhenWhere());
       myNightlyWindow.UpdateDisplay();   
}  
/*=============================================================================================
/     initializeJSkyCalcModel()
/=============================================================================================*/ 
public void initializeJSkyCalcModel(){
     myP200Component    = new P200Component();
     myP200Component.getTelescopeObject().initialJSkyCalcModel();
     myNightlyAlmanac = new NightlyAlmanac(myP200Component.getTelescopeObject().getJSkyCalcModel().getWhenWhere());
     myNightlyWindow = new NightlyWindow(myNightlyAlmanac);
     myNightlyWindow.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
              public void propertyChange(java.beans.PropertyChangeEvent e) {
                 nightly_propertyChange(e);
              }
            });
     myNightlyWindow.UpdateDisplay();  
     dbms.myOTMlauncher.setNightlyAlmanac(myNightlyAlmanac);   
        UtilDateModel model = new UtilDateModel();
        Properties p = new Properties();
        p.put("text.today", "Today");
        p.put("text.month", "Month");
        p.put("text.year", "Year");
        datePanel = new JDatePanelImpl(model, p);
        // Don't know about the formatter, but there it is...
        datePicker = new JDatePickerImpl(datePanel, new DateLabelFormatter());
        long current_system_datetime = System.currentTimeMillis();
        
        java.time.Instant instance = java.time.Instant.ofEpochMilli(current_system_datetime);
        java.time.LocalDateTime localDateTime = java.time.LocalDateTime.ofInstant(instance, java.time.ZoneId.systemDefault());
        java.time.ZonedDateTime zonedDateTime = java.time.ZonedDateTime.ofInstant(instance,java.time.ZoneOffset.UTC);  
        System.out.println("LOCAL DATE TIME = "+localDateTime.toString()+ " UTC DATE TIME = "+zonedDateTime.toString());
        model.setDate(zonedDateTime.getYear(),zonedDateTime.getMonthValue(),zonedDateTime.getDayOfMonth());
        model.setSelected(true);
        datePanel.getModel().setYear(zonedDateTime.getYear());
        datePanel.getModel().setMonth(zonedDateTime.getMonthValue()-1);
        datePanel.getModel().setDay(zonedDateTime.getDayOfMonth());        
        datePicker.addActionListener(new java.awt.event.ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
               java.util.Date mydate = (java.util.Date)model.getValue();
               java.time.ZonedDateTime selected_zonedDateTime = java.time.ZonedDateTime.ofInstant(mydate.toInstant(),java.time.ZoneOffset.UTC); 
               java.time.LocalDateTime withoutTimezone = selected_zonedDateTime.toLocalDateTime();               
               Timestamp selected_timestamp = Timestamp.valueOf(withoutTimezone);
               int hours = (Integer)hoursSpinnerModel.getValue();
               int minutes = (Integer)minutesSpinnerModel.getValue();
               selected_timestamp.setHours(hours);
               selected_timestamp.setMinutes(minutes);
               dbms.myOTMlauncher.setStartTimestamp(selected_timestamp);
               java.lang.String[] selected_date_time = constructJSkyCalcDateTime(selected_timestamp);
               java.lang.String timestamp_string = selected_timestamp.toString();
               timestamp_string = timestamp_string.replace("-", " ");
               timestamp_string = timestamp_string.replace(":", " ");
               InstantInTime i = new InstantInTime(timestamp_string,palomar.stdz,palomar.use_dst,true);              
               WhenWhere w = new WhenWhere(i,palomar);               
               NightlyAlmanac current = new NightlyAlmanac(w);
//               myNightlyAlmanac.Update(w);
               myNightlyWindow = new NightlyWindow(current);
               myNightlyWindow.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
                public void propertyChange(java.beans.PropertyChangeEvent e) {
                   nightly_propertyChange(e);
                }
               });
               myP200Component.getTelescopeObject().getJSkyCalcModel().setToDate(selected_date_time[0],selected_date_time[1]);
               myP200Component.getTelescopeObject().getJSkyCalcModel().synchOutput();
               myNightlyAlmanac.Update(myP200Component.getTelescopeObject().getJSkyCalcModel().getWhenWhere());
               myNightlyWindow.UpdateDisplay();
               dbms.myOTMlauncher.setNightlyAlmanac(myNightlyAlmanac);
//               timeTextField.setText(myNightlyAlmanac.eveningTwilight.when.localDate.RoundedCalString(2,0));
            }
        });       
        datePicker.setFont(new Font("Ariel Black",Font.BOLD,10));
        datePanel.setFont(new Font("Ariel Black",Font.BOLD,10));
        datePicker.setShowYearButtons(true);
        planning_controlsPanel.add(datePicker);
}

/*=============================================================================================
/      updateStarTime()
/=============================================================================================*/
public void updateStarTime(){
    java.util.Date mydate = (java.util.Date)datePanel.getModel().getValue();
   java.time.ZonedDateTime selected_zonedDateTime = java.time.ZonedDateTime.ofInstant(mydate.toInstant(),java.time.ZoneOffset.UTC); 
   java.time.LocalDateTime withoutTimezone = selected_zonedDateTime.toLocalDateTime();               
   Timestamp selected_timestamp = Timestamp.valueOf(withoutTimezone);
   int hours = (Integer)hoursSpinnerModel.getValue();
   int minutes = (Integer)minutesSpinnerModel.getValue();
   selected_timestamp.setHours(hours);
   selected_timestamp.setMinutes(minutes);
   dbms.myOTMlauncher.setStartTimestamp(selected_timestamp);
   java.lang.String[] selected_date_time = constructJSkyCalcDateTime(selected_timestamp);
   java.lang.String timestamp_string = selected_timestamp.toString();
   timestamp_string = timestamp_string.replace("-", " ");
   timestamp_string = timestamp_string.replace(":", " ");
   InstantInTime i = new InstantInTime(timestamp_string,palomar.stdz,palomar.use_dst,true);              
   WhenWhere w = new WhenWhere(i,palomar);               
   NightlyAlmanac current = new NightlyAlmanac(w);
//               myNightlyAlmanac.Update(w);
   myNightlyWindow = new NightlyWindow(current);
   myNightlyWindow.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
        public void propertyChange(java.beans.PropertyChangeEvent e) {
//           nightly_propertyChange(e);
        }
       });
   myP200Component.getTelescopeObject().getJSkyCalcModel().setToDate(selected_date_time[0],selected_date_time[1]);
   myP200Component.getTelescopeObject().getJSkyCalcModel().synchOutput();
   myNightlyAlmanac.Update(myP200Component.getTelescopeObject().getJSkyCalcModel().getWhenWhere());
   myNightlyWindow.UpdateDisplay();
   dbms.myOTMlauncher.setNightlyAlmanac(myNightlyAlmanac);   
}
/*=============================================================================================
/     Property Change Listener for the Ephemeris_propertyChange
/=============================================================================================*/
  private void nightly_propertyChange(PropertyChangeEvent e)  {
     java.lang.String propertyName = e.getPropertyName();
     System.out.println(propertyName);
/*=============================================================================================
/    PARAMETERS FROM THE JSKYCALCMODEL
/=============================================================================================*/
    if(propertyName.matches("UTC_twilight")){        
        java.lang.String current_value = (java.lang.String)e.getNewValue(); 
        System.out.println(current_value);
        try{
         String[] components = current_value.split(" ");
         int hour   = Integer.valueOf(components[0]); 
         int minute = Integer.valueOf(components[1]); 
         hoursSpinnerModel.setValue(hour);
         minutesSpinnerModel.setValue(minute);
        }catch(Exception e2){
            
        }
    }
}
/*=============================================================================================
/     constructMonthString(int new_month)
/=============================================================================================*/
public java.lang.String constructMonthString(int new_month){
  java.lang.String myMonth = new java.lang.String();
        if(new_month == Calendar.JANUARY){
           myMonth = "Jan";
        }
        if(new_month == Calendar.FEBRUARY){
           myMonth = "Feb";
        }
        if(new_month == Calendar.MARCH){
           myMonth = "Mar";
        }
        if(new_month == Calendar.APRIL){
           myMonth = "April";
        }
        if(new_month == Calendar.MAY){
           myMonth = "May";
        }
        if(new_month == Calendar.JUNE){
           myMonth = "June";
        }
        if(new_month == Calendar.JULY){
           myMonth = "July";
        }
        if(new_month == Calendar.AUGUST){
           myMonth = "Aug";
        }
        if(new_month == Calendar.SEPTEMBER){
           myMonth = "Sept";
        }
        if(new_month == Calendar.OCTOBER){
           myMonth = "Oct";
        }
        if(new_month == Calendar.NOVEMBER){
           myMonth = "Nov";
        }
        if(new_month == Calendar.DECEMBER){
           myMonth = "Dec";
        }
  return myMonth;
}
/*=============================================================================================
/     constructJSkyCalcDateTime()
/=============================================================================================*/
public java.lang.String[] constructJSkyCalcDateTime(java.sql.Timestamp current_timestamp){
    java.lang.String[] results     = new java.lang.String[2];
    java.lang.String hoursString   = new java.lang.String();
    java.lang.String minutesString = new java.lang.String();
    java.lang.String secondsString = new java.lang.String();
    
    java.lang.String timestamp_string = current_timestamp.toString();
    String[] date_time_string = timestamp_string.split(" ");
    String[] date_string      = date_time_string[0].split("-");
    String[] time_string      = date_time_string[1].split(":");
    results[0] = date_string[0]+" "+constructMonthString(Integer.parseInt(date_string[1])-1)+" "+date_string[2];
    if(hours < 10){
        hoursString = "0"+hours;
    }
    if(hours >= 10){
        hoursString = Integer.toString(hours);
    }
    if(minutes < 10){
        minutesString = "0"+minutes;
    }
    if(minutes >= 10){
        minutesString = Integer.toString(minutes);
    }
    secondsString = "00";
//    results[1] = hoursString + " "+ minutesString + " "+ secondsString;
    results[1] = time_string[0] + " "+ time_string[1] + " "+ secondsString;
   return results;
}
/*=============================================================================================
/     initializePanels()
/=============================================================================================*/
 private void initializeRightMenu(){
     mainMenuBar.add(Box.createHorizontalGlue());
     accountMenu = new javax.swing.JMenu();
     accountMenu.setText("Guest");
     myaccount_MenuItem = new javax.swing.JMenuItem();
     myaccount_MenuItem.setText("My Account");
     switch_account_MenuItem = new javax.swing.JMenuItem();
     switch_account_MenuItem.setText("Create Users");
     sign_out_MenuItem = new javax.swing.JMenuItem();
     sign_out_MenuItem.setText("Sign In");
     accountMenu.add(myaccount_MenuItem);
     accountMenu.add(switch_account_MenuItem);
     accountMenu.add(sign_out_MenuItem);
     mainMenuBar.add(accountMenu);
         myaccount_MenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                myaccount_MenuItemActionPerformed(evt);
            }
         });
         switch_account_MenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                switch_account_MenuItemActionPerformed(evt);
            }
        });
        sign_out_MenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                sign_out_MenuItemActionPerformed(evt);
            }
        });
    saveMenuItem.setEnabled(false);
    save_asMenuItem.setEnabled(false);
    exportMenuItem.setEnabled(false);
 }
/*=============================================================================================
/     Action Listeners for the upper right side menu
/=============================================================================================*/
 private void myaccount_MenuItemActionPerformed(java.awt.event.ActionEvent evt) {                                                    
    this.myChangePasswordFrame.setVisible(true);
 } 
 private void switch_account_MenuItemActionPerformed(java.awt.event.ActionEvent evt) {                                                    
    myCreateOwnerFrame.setVisible(true);
 }
 private void sign_out_MenuItemActionPerformed(java.awt.event.ActionEvent evt) {                                                    
     my_signInFrame.setVisible(true);    
 } 
/*=============================================================================================
/     initializeDatabase()`````````
/=============================================================================================*/
   public void initializeDatabase(){
        try{
            dbms = new NGPSdatabase();
            dbms.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
              public void propertyChange(java.beans.PropertyChangeEvent e) {
                 dbms_propertyChange(e);
              }
            });
            my_signInFrame.setDBMS(dbms);
            myCreateOwnerFrame.setDBMS(dbms);
            dbms.initializeOTMlauncher();
            dbms.myOTMlauncher.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
               public void propertyChange(java.beans.PropertyChangeEvent e) {
                otm_propertyChange(e);
              }
            }); 
            dbms.myOTMlauncher.setNGPSFrame(this);           
 //           messagesTextPane.setDocument(myCommandLogModel.getDocument());
         dbms.initializeDBMS();
         myTargetSetFrame.setDBMS(dbms);
         myOScontrolsPanel.setDBMS(dbms);
         dbms.queryOwners();
         dbms.queryObservationSets("");
         dbms.setPublicDocumentModel(myCommandLogModel);
         dbms.myOTMlauncher.setPublicDocumentModel(myCommandLogModel);
         }catch(Exception e){
           System.out.println(e.toString());
        }
    }
/*================================================================================================
/     logPublicMessage(int code,java.lang.String message)
/=================================================================================================*/
public void logPublicMessage(int code,java.lang.String message){
   myCommandLogModel.insertMessage(code, message);
} 
/*================================================================================================
/    initializeObservationSequencerController()
/=================================================================================================*/
public void initializeObservationSequencerController(){
    myObservationSequencerController = new ObservationSequencerController();
    myObservationSequencerController.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
      public void propertyChange(java.beans.PropertyChangeEvent e) {
                 state_propertyChange(e);
              }
      });
    myOScontrolsPanel.initialize(myObservationSequencerController); 
    myObservationSequencerController.connect();
    myObservationSequencerController.setNGPSdatabase(dbms);
    myTargetSetFrame.setObservationSequencerController(myObservationSequencerController);
    myImportFrame.setObservationSequencerController(myObservationSequencerController);
    myConnectionsFrame.setObservationSequencercontroller(myObservationSequencerController);
}
/*=============================================================================================
/     Property Change Listener for the Ephemeris_propertyChange
/=============================================================================================*/
  private void state_propertyChange(PropertyChangeEvent e)  {
     java.lang.String propertyName = e.getPropertyName();
     System.out.println(propertyName);
} 
/*================================================================================================
/   initializeEngineeringFrame()
/=================================================================================================*/
public void initializeEngineeringFrame(){
    myEngineeringFrame = new EngineeringFrame();
    myEngineeringFrame.setObservationSequencerController(myObservationSequencerController);   
}
/*================================================================================================
/       initializeParser()
/=================================================================================================*/
public void initializeStateMonitor(){
   myObservationSequencerObject = new ObservationSequencerObject(); 
//   myObservationSequencerObject.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
//      public void propertyChange(java.beans.PropertyChangeEvent e) {
//                 state_propertyChange(e);
//              }
//      });
   mySimulationServer = new SimulationServer();
   mySimulationServer.setSequencerObject(myObservationSequencerObject);
   mySimulationServer.setServerPort(SERVER_PORT);
   mySimulationServer.startServer();
}  
/*================================================================================================
/       initializeParser
/=================================================================================================*/
public void initializeParser(){
    myTargetListParser      = new TargetListParser2(); 
    myTargetListParser.setNGPSdatabase(dbms);  
//    myimportCSVPanel.setParser(myTargetListParser);
  }  
/*================================================================================================
/      initializeMainTable()
/=================================================================================================*/
public void initializeMainTable(){
  main_editor_table =  constructTable();
  main_editor_table.setModel(dbms.myTargetDBMSTableModel);
  initializeTableModel(main_editor_table,dbms.myTargetDBMSTableModel);
  main_tableScrollPane.setViewportView(main_editor_table);
  myParametersTableModel    = new ParametersTableModel();
  myOTMParametersTableModel = new OTMParameterTableModel();
  myETCParameterTableModel  = new ETCParameterTableModel();
  planTable = constructPlanTable();
  jScrollPane5.setViewportView(planTable);
  observationTable.setModel(myParametersTableModel);
  planTable.setModel(myOTMParametersTableModel);
  etcTable.setModel(myETCParameterTableModel);
  int count = main_editor_table.getColumnCount();
  myedit_monitor = new edit_monitor();
  myedit_monitor.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
      public void propertyChange(java.beans.PropertyChangeEvent e) {
                 table_edited_propertyChange(e);
              }
      });
  dbms.myTargetDBMSTableModel.set_edit_monitor(myedit_monitor);
  myParametersTableModel.set_edit_monitor(myedit_monitor);
  myOTMParametersTableModel.set_edit_monitor(myedit_monitor);
  myETCParameterTableModel.set_edit_monitor(myedit_monitor);
//  Color pale_red   = new Color(161,200,150);
//  Color pale_blue  = new Color(149,196,204);
//  Color pale_green = new Color(149,204,158);

  MultiLineHeaderRenderer my_renderer_default   = new MultiLineHeaderRenderer(javax.swing.UIManager.getColor("TableHeader.background"));  
  MultiLineHeaderRenderer my_renderer_red   = new MultiLineHeaderRenderer(new Color(255,153,153));
  MultiLineHeaderRenderer my_renderer_blue  = new MultiLineHeaderRenderer(new Color(0,255,255));
//  MultiLineHeaderRenderer my_renderer_green = new MultiLineHeaderRenderer(new Color(85,255,0));
  MultiLineHeaderRenderer my_renderer_green = new MultiLineHeaderRenderer(new Color(200,200,200));
  
  main_editor_table.getColumnModel().getColumn(0).setHeaderRenderer(my_renderer_default);
  main_editor_table.getColumnModel().getColumn(1).setHeaderRenderer(my_renderer_default);
  main_editor_table.getColumnModel().getColumn(2).setHeaderRenderer(my_renderer_default);
  main_editor_table.getColumnModel().getColumn(3).setHeaderRenderer(my_renderer_blue);
  main_editor_table.getColumnModel().getColumn(4).setHeaderRenderer(my_renderer_blue);
  main_editor_table.getColumnModel().getColumn(5).setHeaderRenderer(my_renderer_blue);
  main_editor_table.getColumnModel().getColumn(6).setHeaderRenderer(my_renderer_green);
  main_editor_table.getColumnModel().getColumn(7).setHeaderRenderer(my_renderer_blue);
  main_editor_table.getColumnModel().getColumn(8).setHeaderRenderer(my_renderer_green);
  main_editor_table.getColumnModel().getColumn(9).setHeaderRenderer(my_renderer_green);
  main_editor_table.getColumnModel().getColumn(10).setHeaderRenderer(my_renderer_green);
  main_editor_table.getColumnModel().getColumn(11).setHeaderRenderer(my_renderer_blue);
          
  for(int i=0;i<count;i++){
//     main_editor_table.getColumnModel().getColumn(i).setHeaderRenderer(my_renderer_blue);
  }
//  main_editor_table.getColumnModel().getColumn(0).getHeaderRenderer();
  observationTable.getColumnModel().getColumn(0).setHeaderRenderer(my_renderer_blue);
  observationTable.getColumnModel().getColumn(1).setHeaderRenderer(my_renderer_blue);
  etcTable.getColumnModel().getColumn(0).setHeaderRenderer(my_renderer_red);
  etcTable.getColumnModel().getColumn(1).setHeaderRenderer(my_renderer_red);
  planTable.getColumnModel().getColumn(0).setHeaderRenderer(my_renderer_green);
  planTable.getColumnModel().getColumn(1).setHeaderRenderer(my_renderer_green);
  DefaultTableCellRenderer rightRenderer  = new DefaultTableCellRenderer();
  DefaultTableCellRenderer centerRenderer = new DefaultTableCellRenderer();
  DefaultTableCellRenderer leftRenderer   = new DefaultTableCellRenderer();
  rightRenderer.setHorizontalAlignment(DefaultTableCellRenderer.RIGHT);
  centerRenderer.setHorizontalAlignment(DefaultTableCellRenderer.CENTER);
  leftRenderer.setHorizontalAlignment(DefaultTableCellRenderer.LEFT);
//  main_editor_table.getColumnModel().getColumn(0).setCellRenderer(centerRenderer);
  main_editor_table.getColumnModel().getColumn(1).setCellRenderer(centerRenderer);
  main_editor_table.getColumnModel().getColumn(2).setCellRenderer(centerRenderer);
  main_editor_table.getColumnModel().getColumn(3).setCellRenderer(centerRenderer);
  main_editor_table.getColumnModel().getColumn(4).setCellRenderer(centerRenderer);
  main_editor_table.getColumnModel().getColumn(5).setCellRenderer(centerRenderer);
  main_editor_table.getColumnModel().getColumn(6).setCellRenderer(centerRenderer);
  main_editor_table.getColumnModel().getColumn(7).setCellRenderer(centerRenderer);
  main_editor_table.getColumnModel().getColumn(8).setCellRenderer(centerRenderer);
  main_editor_table.getColumnModel().getColumn(9).setCellRenderer(centerRenderer);
  main_editor_table.getColumnModel().getColumn(10).setCellRenderer(centerRenderer);
  
  main_editor_table.getColumnModel().getColumn(0).setMinWidth(20);
  main_editor_table.getColumnModel().getColumn(0).setMaxWidth(20);
  main_editor_table.getColumnModel().getColumn(1).setMinWidth(0);
  main_editor_table.getColumnModel().getColumn(1).setMaxWidth(0);  
  main_editor_table.getColumnModel().getColumn(2).setMinWidth(120);
  main_editor_table.getColumnModel().getColumn(3).setMinWidth(120);
  main_editor_table.getColumnModel().getColumn(4).setMinWidth(120);
  main_editor_table.getColumnModel().getColumn(5).setMinWidth(100);
  main_editor_table.getColumnModel().getColumn(6).setMinWidth(100);
  main_editor_table.getColumnModel().getColumn(7).setMinWidth(100);
  main_editor_table.getColumnModel().getColumn(8).setMinWidth(100);
  main_editor_table.getColumnModel().getColumn(9).setMinWidth(100);
  main_editor_table.getColumnModel().getColumn(10).setMinWidth(100);
  main_editor_table.getColumnModel().getColumn(11).setMinWidth(160);
//  TableColumn tColumn_otm16 = main_editor_table.getColumnModel().getColumn(0);
//              tColumn_otm16.setCellRenderer(new ColumnColorRenderer(java.awt.Color.lightGray, java.awt.Color.BLACK));     
}
/*=============================================================================================
/     Property Change Listener for the Ephemeris_propertyChange
/=============================================================================================*/
  private void table_edited_propertyChange(PropertyChangeEvent e)  {
     java.lang.String propertyName = e.getPropertyName();
     System.out.println(propertyName);
    if(propertyName.matches("edited")){
       java.lang.Boolean current_value = (java.lang.Boolean)e.getNewValue();  
       if(current_value){
           this.acceptButton.setIcon(ACCEPT_SAVE_NEEDED);
       }else if(!current_value){
           this.acceptButton.setIcon(ACCEPT_OFF);
       }
    }
  }
/*================================================================================================
/     JTable getMainTable()
/=================================================================================================*/
public JTable getMainTable(){
    return main_editor_table;
}
public static JTable constructPlanTable(){
        JTable current = new JTable() {
        @Override
        public Component prepareRenderer(TableCellRenderer renderer, int rowIndex,int columnIndex) {
            JComponent component = (JComponent) super.prepareRenderer(renderer, rowIndex, columnIndex);  
                int STATUS_COL = 2;     
                int FLAG_ROW   = 11;
                Component c = super.prepareRenderer(renderer, rowIndex,columnIndex);
                JComponent jc = (JComponent)c;
                Target current_target = ((OTMParameterTableModel)getModel()).getTarget();
                if(rowIndex == FLAG_ROW){
                  if(current_target != null){
                    java.lang.String flags = (current_target.otm.getOTMflag());
                    if (flags.contains("DAY")) {
                        c.setBackground(super.getBackground());
                        c.setForeground(Color.red);
                        jc.setBorder(super.getBorder());                   
                    } else if( !flags.trim().isEmpty() ){
                        c.setBackground(super.getBackground());
                        c.setForeground(Color.BLUE);
                        jc.setBorder(super.getBorder());
                    } else if(flags == null | flags.trim().isEmpty()){
                        c.setBackground(super.getBackground());
                        c.setForeground(super.getForeground()); 
                        jc.setBorder(super.getBorder());     
                    }else{
                        c.setBackground(super.getBackground());
                        c.setForeground(super.getForeground());
                        jc.setBorder(super.getBorder());     
                    }
                  }  
                }
                if(rowIndex != FLAG_ROW){
                    c.setBackground(super.getBackground());
                    c.setForeground(super.getForeground());
                    jc.setBorder(super.getBorder());                        
                }
                return c;
        }
    };
  return current;
 }
/*================================================================================================
/      constructTable()
/=================================================================================================*/
public static JTable constructTable(){
        JTable current = new JTable() {
        @Override
        public Component prepareRenderer(TableCellRenderer renderer, int rowIndex,int columnIndex) {
            JComponent component = (JComponent) super.prepareRenderer(renderer, rowIndex, columnIndex);  
                int STATUS_COL = 2;               
                Component c = super.prepareRenderer(renderer, rowIndex,columnIndex);
                JComponent jc = (JComponent)c;
                Target current = ((MasterDBTableModel)getModel()).getRecord(rowIndex);
                boolean selected_row = current.isSelected();
                String state = current.getSTATE();
                if ("exposing".equals(state)) {
                    c.setBackground(Color.green);
                    c.setForeground(Color.black);
                    jc.setBorder(new MatteBorder(1, 0, 1, 0, Color.BLACK));                   
                } else if("PENDING".matches(state)){
//                    c.setBackground(super.getBackground());
//                    c.setForeground(super.getForeground());
                    c.setBackground(Color.white);
                    c.setForeground(Color.black);
                    jc.setBorder(super.getBorder());
                }
                else if("completed".matches(state)){
                    c.setBackground(Color.gray);
                    c.setForeground(Color.white);
                    jc.setBorder(super.getBorder());
                }
                else if("INACTIVE".matches(state)){
                    c.setBackground(Color.white);
                    c.setForeground(Color.GRAY);
                    jc.setBorder(super.getBorder());                    
                }
                else if("active".matches(state)){
                    c.setBackground(Color.green);
                    c.setForeground(Color.black);
                    jc.setBorder(super.getBorder());                    
                }
                else if("ERROR-OTM".matches(state) | "error-otm".matches(state)){
                    c.setBackground(Color.white);
                    c.setForeground(Color.RED);
                    jc.setBorder(super.getBorder());   
                }
                else if("WARN-OTM".matches(state) | "warn-otm".matches(state)){
                    c.setBackground(Color.white);
                    c.setForeground(Color.BLUE);
                    jc.setBorder(super.getBorder());   
                }
                if((columnIndex > 17)&(columnIndex < 25)){
                    c.setBackground(new Color(100,160,225));
                    c.setForeground(super.getForeground());                    
                }                
                if(columnIndex >= 25){
                    c.setBackground(new Color(115,210,140));
                    c.setForeground(super.getForeground());                    
                }
                if(selected_row){
                   c.setBackground(Color.yellow);
                   jc.setBorder(new MatteBorder(1, 0, 1, 0, Color.black));    
                }
                return c;
        }
    };
  return current;
 }
/*================================================================================================
/       setPaste(boolean state)
/=================================================================================================*/
    public void setPaste(boolean state){
        paste_state = state;
    }
/*================================================================================================
/        selectFITSFiles()
/=================================================================================================*/
    public void initializeTableModel(JTable current_table,EditTableInterface current_table_model){
//        current_table.setModel(current_table_model);
        current_table.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);
        current_table.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        current_table.setRowSelectionAllowed(true);  
        current_table.setDragEnabled(true);
        current_table.setDropMode(DropMode.INSERT_ROWS);
        TableRowTransferHandler trth = new TableRowTransferHandler(current_table);
        current_table.setTransferHandler(trth); 
        JMenuItem insert_menu_item  = new JMenuItem("Insert");
        JMenuItem delete_menu_item  = new JMenuItem("Delete");
        JMenuItem copy_menu_item    = new JMenuItem("Copy");
        paste_menu_item   = new JMenuItem("Paste");
        insert_copied_menu_item   = new JMenuItem("Insert Copied row");
        paste_menu_item.setEnabled(false);
        insert_copied_menu_item.setEnabled(false);
           JPopupMenu popup          = new JPopupMenu("Editing Functions");
                      popup.add(insert_menu_item);
                      popup.add(delete_menu_item);
                      popup.add(copy_menu_item);
                      popup.add(paste_menu_item);
                      popup.add(insert_copied_menu_item);
           insert_menu_item.addActionListener(new ActionListener() {
              public void actionPerformed(ActionEvent ev) {
                 System.out.println("Insert Menu Item pressed");
                 current_table_model.insert(selected_table_row, new Target());
              }
           });
           delete_menu_item.addActionListener(new ActionListener() {
              public void actionPerformed(ActionEvent ev) {
                 System.out.println("Delete Menu Item pressed"); 
                 Target current_target = current_table_model.getRecord(selected_table_row);
                 if(current_target.getObservationID() != 0){
                     dbms.delete_stack.push(current_target);
                 }
                 current_table_model.delete(selected_table_row);
              }
           });
           copy_menu_item.addActionListener(new ActionListener() {
              public void actionPerformed(ActionEvent ev) {
                 System.out.println("Copy Menu Item pressed");
                 Target selected_copy_target = current_table_model.getRecord(selected_table_row);                 
                 copy_stack.push(selected_copy_target.clone());
                 paste_menu_item.setEnabled(true);
                 insert_copied_menu_item.setEnabled(true);
              }
           });
           paste_menu_item.addActionListener(new ActionListener() {
              public void actionPerformed(ActionEvent ev) {
                 System.out.println("Paste Menu Item pressed"); 
                 Target current_target = (Target)copy_stack.pop();
                 current_table_model.delete(selected_table_row);
                 current_table_model.insert(selected_table_row, current_target);
                 current_table_model.reorder_table();
                 boolean state = copy_stack.empty();
                 if(state){
                    setPaste(false); 
                    paste_menu_item.setEnabled(false);
                    insert_copied_menu_item.setEnabled(false);
                 }
              }
           });
           insert_copied_menu_item.addActionListener(new ActionListener() {
              public void actionPerformed(ActionEvent ev) {
                 System.out.println("Paste Menu Item pressed"); 
                 Target current_target = (Target)copy_stack.pop();
//                 current_table_model.delete(selected_table_row);
                 current_table_model.insert(selected_table_row, current_target);
                 current_table_model.reorder_table();
                 boolean state = copy_stack.empty();
                 if(state){
                    setPaste(false); 
                    paste_menu_item.setEnabled(false);
                    insert_copied_menu_item.setEnabled(false);
                 }
              }
           });
        current_table.setComponentPopupMenu(popup);
        current_table.getSelectionModel().addListSelectionListener(new ListSelectionListener(){
            public void valueChanged(ListSelectionEvent event) {
                 if(!event.getValueIsAdjusting() && current_table.getSelectedRow() != -1){
                     try{
                       int count = current_table_model.getRowCount();
                       for(int i=0;i<count;i++){
                           current_table_model.getRecord(i).setSelected(false); 
                       }
 //                      ((javax.swing.table.AbstractTableModel)(current_table.getModel())).fireTableDataChanged();
                     }catch(Exception e){
                         System.out.println(e.toString());
                     }                    
                     selected_table_row  = current_table.getSelectedRow(); 
                     trth.setSelectedRow(selected_table_row);
                     current_table_model.getRecord(selected_table_row).setSelected(true);
                     ((javax.swing.table.AbstractTableModel)(current_table.getModel())).fireTableDataChanged();
                     if(dbms.getCombinedChartTest() != null){
                        dbms.getCombinedChartTest().setSelectedSeries(selected_table_row);
                        dbms.getCombinedChartTest().updateChartColors();                       
                     }
                     dbms.setSelectedTargetName(current_table_model.getRecord(selected_table_row).getName());
                     myParametersTableModel.setTarget(current_table_model.getRecord(selected_table_row));
                     myOTMParametersTableModel.setTarget(current_table_model.getRecord(selected_table_row));
                     myETCParameterTableModel.setTarget(current_table_model.getRecord(selected_table_row));
//                     selected_table_rows = current_table.getSelectedRows();
                  }
            }
            });
        current_table.addMouseListener(new MouseAdapter() {
            @Override
            public void mouseReleased(MouseEvent e) {
                int r = current_table.rowAtPoint(e.getPoint());
                if (r >= 0 && r < current_table.getRowCount()) {
                    current_table.setRowSelectionInterval(r, r);
                } else {
                    current_table.clearSelection();
                }
                int rowindex = current_table.getSelectedRow();
                if (rowindex < 0)
                    return;
                if (e.isPopupTrigger() && e.getComponent() instanceof JTable ) {
                    
                    popup.show(e.getComponent(), e.getX(), e.getY());
                }
            }
        });
//    javax.swing.InputMap  im = current_table.getInputMap(current_table.WHEN_IN_FOCUSED_WINDOW);
//    javax.swing.ActionMap am = current_table.getActionMap();

//    im.put(javax.swing.KeyStroke.getKeyStroke(KeyEvent.VK_RIGHT, 0), "RightArrow");
//    im.put(javax.swing.KeyStroke.getKeyStroke(KeyEvent.VK_LEFT, 0), "LeftArrow");
//    im.put(javax.swing.KeyStroke.getKeyStroke(KeyEvent.VK_UP, 0), "UpArrow");
//    im.put(javax.swing.KeyStroke.getKeyStroke(KeyEvent.VK_DOWN, 0), "DownArrow");

//    am.put("RightArrow", new ArrowAction("RightArrow"));
//    am.put("LeftArrow", new ArrowAction("LeftArrow"));
//    am.put("UpArrow",   new ArrowAction("UpArrow"));
//    am.put("DownArrow", new ArrowAction("DownArrow"));
        current_table.setFocusTraversalKeysEnabled(false);
        current_table.setFocusable(true);
        current_table.addKeyListener(new java.awt.event.KeyListener(){
          int dx = 0;
          int dy = 0;
          public void keyPressed(KeyEvent e) {
            int key = e.getKeyCode();
            if (key == KeyEvent.VK_LEFT) {
                dx = -1;
            }
            if (key == KeyEvent.VK_RIGHT) {
                dx = 1;
            }
            if (key == KeyEvent.VK_UP) {
                dy = -1;
/*                    try{
                       selected_table_row  =selected_table_row + dy;
                       int count = current_table_model.getRowCount();
                       for(int i=0;i<count;i++){
                           current_table_model.getRecord(i).setSelected(false); 
                       }
 //                      ((javax.swing.table.AbstractTableModel)(current_table.getModel())).fireTableDataChanged();
                     }catch(Exception e2){
                         System.out.println(e2.toString());
                     }  
                     current_table.setRowSelectionInterval(selected_table_row,selected_table_row);
//                     selected_table_row  = current_table.getSelectedRow(); 
                     trth.setSelectedRow(selected_table_row);
                     current_table_model.getRecord(selected_table_row).setSelected(true);
                     ((javax.swing.table.AbstractTableModel)(current_table.getModel())).fireTableDataChanged();
                     if(dbms.getCombinedChartTest() != null){
                        dbms.getCombinedChartTest().setSelectedSeries(selected_table_row);
                        dbms.getCombinedChartTest().updateChartColors();                       
                     }
                     dbms.setSelectedTargetName(current_table_model.getRecord(selected_table_row).getName());
                     myParametersTableModel.setTarget(current_table_model.getRecord(selected_table_row));
                     myOTMParametersTableModel.setTarget(current_table_model.getRecord(selected_table_row));
                     myETCParameterTableModel.setTarget(current_table_model.getRecord(selected_table_row));
                
//                selected_table_row = selected_table_row + dy;          
*/                
            }
            if (key == KeyEvent.VK_DOWN) {
               dy = 1;
//               selected_table_row = selected_table_row + dy;
            }
        } 
          public void keyReleased(KeyEvent e) {
            int key = e.getKeyCode();
        }  
          public void keyTyped(KeyEvent e) {
            int key = e.getKeyCode();
        }  
        });
    }    
/*================================================================================================
/        selectFITSFiles()
/=================================================================================================*/
      public void selectFile(){         
          JFileChooser input = new JFileChooser(initialDirectory);
          FileNameExtensionFilter filter = new FileNameExtensionFilter("Comma Separated Variables File","csv");
          input.setFileFilter(filter);
          input.setDialogTitle("Select input *.CSV file to import.");
          input.setFileSelectionMode(JFileChooser.FILES_ONLY);
          int result = input.showOpenDialog(this);
          if(result == JFileChooser.APPROVE_OPTION) {
              try{
                 File selected_file = input.getSelectedFile();
                 java.lang.String file_path = selected_file.getAbsolutePath();
                 java.lang.String file_name = selected_file.getName();
                myTargetListParser.setFileName(file_name);
                myTargetListParser.setFilePath(file_path);
                initialDirectory = selected_file.getPath();
                myTargetListParser.executeParseFile();
                myImportFrame.setVisible(true);
              }catch(Exception e2){
                  System.out.println("A file I/O error occured while locating the AOR File. " + e2);
              }// end of the catch statement
           int model_type = myTargetListParser.getModelType();
              } else if (result == JFileChooser.CANCEL_OPTION) {
               System.out.println("Cancel was selected");
          }
   }  // end 
/*=============================================================================================
/     Property Change Listener for the Ephemeris_propertyChange
/=============================================================================================*/
  private void dbms_propertyChange(PropertyChangeEvent e)  {
     java.lang.String propertyName = e.getPropertyName();
     System.out.println(propertyName);
/*=============================================================================================
/    PARAMETERS FROM THE JSKYCALCMODEL
/=============================================================================================*/
    if(propertyName.matches("connected")){
        java.lang.Boolean current_value = (java.lang.Boolean)e.getNewValue(); 
        if(current_value){
//            database_connect_ToggleButton.setIcon(ON);
//            database_connect_ToggleButton.setText("CONNECTED TO NGPS DATABASE");
//            commit_to_dbms_Button.setEnabled(true);
//            commit_to_dbms_Button.setIcon(ON);
        }else if(!current_value){
//            database_connect_ToggleButton.setIcon(OFF);
//            database_connect_ToggleButton.setText("CONNECT TO NGPS DATABASE");
//            commit_to_dbms_Button.setEnabled(false);
//            commit_to_dbms_Button.setIcon(OFF);
        }
    }
    if(propertyName.matches("selected_set_id")){
        java.lang.Integer current_value = (java.lang.Integer)e.getNewValue(); 
 //       this.selectedSetIDLabel.setText(current_value.toString());
    }
    if(propertyName.matches("selected_set_name")){
        java.lang.String current_value = (java.lang.String)e.getNewValue(); 
        sekected_target_listjLabel.setText(current_value);
    }
    if(propertyName.matches("owner")){
//        java.lang.String current_value = (java.lang.String)e.getNewValue();
//        selectedOwnerTextField.setText(current_value);
    }
    if(propertyName.matches("table_populated")){
      java.lang.Boolean current_value = (java.lang.Boolean)e.getNewValue(); 
      saveMenuItem.setEnabled(current_value);
      save_asMenuItem.setEnabled(current_value);
      exportMenuItem.setEnabled(current_value);
    }
    if(propertyName.matches("ProcessingState")){
       java.lang.Integer current_value = (java.lang.Integer)e.getNewValue();
       if(current_value == NGPSdatabase.RUNNING){
          if (javax.swing.SwingUtilities.isEventDispatchThread()) {              
               dbms_stateLabel.setIcon(ON);
               acceptButton.setEnabled(false);
               acceptButton.setIcon(ACCEPT_ON);
               acceptButton.setDisabledIcon(ACCEPT_ON);              
          } else { 
               javax.swing.SwingUtilities.invokeLater(new Runnable() {
                   public void run() {
                     dbms_stateLabel.setIcon(ON);
                     acceptButton.setEnabled(false);
                     acceptButton.setIcon(ACCEPT_ON);
                     acceptButton.setDisabledIcon(ACCEPT_ON);
                  } 
               });
          }
       }
       if(current_value == NGPSdatabase.IDLE){
          if (javax.swing.SwingUtilities.isEventDispatchThread()) {
               dbms_stateLabel.setIcon(UNKNOWN);
               acceptButton.setEnabled(true);
               acceptButton.setIcon(ACCEPT_OFF);
               acceptButton.setDisabledIcon(ACCEPT_OFF);
               
          } else { 
               javax.swing.SwingUtilities.invokeLater(new Runnable() {
                   public void run() {
                     dbms_stateLabel.setIcon(UNKNOWN);
                     acceptButton.setEnabled(true);
                     acceptButton.setIcon(ACCEPT_OFF);
                     acceptButton.setDisabledIcon(ACCEPT_OFF);
                  } 
               });
          }
       }
    }
    if(propertyName.matches("selected_target_name")){
        java.lang.String current_value = (java.lang.String)e.getNewValue(); 
        selectedTargetNameTextField.setText(current_value);
    }
} 
/*=============================================================================================
/     Property Change Listener for the Ephemeris_propertyChange
/=============================================================================================*/
  private void otm_propertyChange(PropertyChangeEvent e)  {
     java.lang.String propertyName = e.getPropertyName();
     System.out.println(propertyName);
/*=============================================================================================
/    PARAMETERS FROM THE JSKYCALCMODEL
/=============================================================================================*/
    if(propertyName.matches("selected_set_id")){
        
        java.lang.Integer current_value = (java.lang.Integer)e.getNewValue(); 
//        this.selectedSetIDLabel.setText(current_value.toString());
    }
    if(propertyName.matches("ProcessingState")){
       java.lang.Integer current_value = (java.lang.Integer)e.getNewValue();
       if(current_value == OTMlauncher.RUNNING){
           otm_state_Label.setIcon(ON);
       }
       if(current_value == OTMlauncher.IDLE){
           otm_state_Label.setIcon(UNKNOWN);
       }
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

        jMenu3 = new javax.swing.JMenu();
        jCheckBox2 = new javax.swing.JCheckBox();
        jPanel9 = new javax.swing.JPanel();
        dbms_stateLabel = new javax.swing.JLabel();
        otm_state_Label = new javax.swing.JLabel();
        planningPanel = new javax.swing.JPanel();
        jLabel11 = new javax.swing.JLabel();
        twilightButton = new javax.swing.JButton();
        auto_start_timeCheckBox = new javax.swing.JCheckBox();
        fetchLiveButton = new javax.swing.JButton();
        jPanel10 = new javax.swing.JPanel();
        jLabel15 = new javax.swing.JLabel();
        planning_controlsPanel = new javax.swing.JPanel();
        auto_fetchCheckBox = new javax.swing.JCheckBox();
        jLabel8 = new javax.swing.JLabel();
        airmass_limitTextField = new javax.swing.JTextField();
        jLabel12 = new javax.swing.JLabel();
        HoursSpinner = new javax.swing.JSpinner();
        MinutesSpinner = new javax.swing.JSpinner();
        jLabel13 = new javax.swing.JLabel();
        seeingTextField = new javax.swing.JTextField();
        acceptButton = new javax.swing.JButton();
        cancelButton = new javax.swing.JButton();
        jLabel7 = new javax.swing.JLabel();
        jLabel17 = new javax.swing.JLabel();
        sekected_target_listjLabel = new javax.swing.JLabel();
        detailsPanel = new javax.swing.JPanel();
        detailedTabbedPane = new javax.swing.JTabbedPane();
        jPanel6 = new javax.swing.JPanel();
        jScrollPane3 = new javax.swing.JScrollPane();
        observationTable = new javax.swing.JTable();
        jPanel7 = new javax.swing.JPanel();
        jScrollPane4 = new javax.swing.JScrollPane();
        etcTable = new javax.swing.JTable();
        jPanel8 = new javax.swing.JPanel();
        jScrollPane5 = new javax.swing.JScrollPane();
        planTable = new javax.swing.JTable();
        jLabel9 = new javax.swing.JLabel();
        selectedTargetNameTextField = new javax.swing.JTextField();
        jLabel1 = new javax.swing.JLabel();
        jLabel2 = new javax.swing.JLabel();
        jLabel3 = new javax.swing.JLabel();
        jLabel4 = new javax.swing.JLabel();
        jLabel5 = new javax.swing.JLabel();
        jLabel6 = new javax.swing.JLabel();
        left_mainPanel = new javax.swing.JPanel();
        myOScontrolsPanel = new edu.caltech.palomar.instruments.ngps.gui.OScontrolsPanel();
        main_tableScrollPane = new javax.swing.JScrollPane();
        mainTable = new javax.swing.JTable();
        mainMenuBar = new javax.swing.JMenuBar();
        NGPSMenu = new javax.swing.JMenu();
        aboutNGPSMenuItem = new javax.swing.JMenuItem();
        preferencesMenuItem = new javax.swing.JMenuItem();
        shutdownMenuItem = new javax.swing.JMenuItem();
        quitMenuItem = new javax.swing.JMenuItem();
        jMenu1 = new javax.swing.JMenu();
        new_target_listMenuItem = new javax.swing.JMenuItem();
        jSeparator3 = new javax.swing.JPopupMenu.Separator();
        openMenuItem = new javax.swing.JMenuItem();
        jSeparator2 = new javax.swing.JPopupMenu.Separator();
        saveMenuItem = new javax.swing.JMenuItem();
        save_asMenuItem = new javax.swing.JMenuItem();
        jSeparator1 = new javax.swing.JPopupMenu.Separator();
        importMenuItem = new javax.swing.JMenuItem();
        exportMenuItem = new javax.swing.JMenuItem();
        editMenu = new javax.swing.JMenu();
        newTargetCheckBoxMenuItem = new javax.swing.JCheckBoxMenuItem();
        new_target_from_databaseMenuItem = new javax.swing.JMenuItem();
        cutMenuItem = new javax.swing.JMenuItem();
        copyMenuItem = new javax.swing.JMenuItem();
        pasteMenuItem = new javax.swing.JMenuItem();
        deleteMenuItem = new javax.swing.JMenuItem();
        insert_copiedMenuItem = new javax.swing.JMenuItem();
        viewMenu = new javax.swing.JMenu();
        basicRadioButtonMenuItem = new javax.swing.JRadioButtonMenuItem();
        detailedRadioButtonMenuItem = new javax.swing.JRadioButtonMenuItem();
        otm_outputMenuItem = new javax.swing.JMenuItem();
        zoomInMenuItem = new javax.swing.JMenuItem();
        zoomOutMenuItem = new javax.swing.JMenuItem();
        plotMenuItem = new javax.swing.JMenuItem();
        toolsMenu = new javax.swing.JMenu();
        optimizeMenuItem = new javax.swing.JMenuItem();
        hourly_weatherMenuItem = new javax.swing.JMenuItem();
        ten_day_weatherMenuItem = new javax.swing.JMenuItem();
        ephemerisMenuItem = new javax.swing.JMenuItem();
        reportBugMenuItem = new javax.swing.JMenuItem();
        engineeringMenuItem = new javax.swing.JMenuItem();
        connectionsMenuItem = new javax.swing.JMenuItem();
        helpMenu = new javax.swing.JMenu();
        quick_startMenuItem = new javax.swing.JMenuItem();
        etcMenuItem = new javax.swing.JMenuItem();
        data_reductionMenuItem = new javax.swing.JMenuItem();
        calibrationMenuItem = new javax.swing.JMenuItem();
        line_listMenuItem = new javax.swing.JMenuItem();
        example_spectraMenuItem = new javax.swing.JMenuItem();
        ngps_instrument_manualMenuItem = new javax.swing.JMenuItem();
        errors_diagnosticsMenuItem = new javax.swing.JMenuItem();

        jMenu3.setText("jMenu3");

        jCheckBox2.setText("Auto");

        jPanel9.setBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 0, 0), 1, true));

        dbms_stateLabel.setFont(new java.awt.Font("Arial", 1, 12)); // NOI18N
        dbms_stateLabel.setText("DBMS");

        otm_state_Label.setFont(new java.awt.Font("Arial", 1, 12)); // NOI18N
        otm_state_Label.setText("OTM");

        javax.swing.GroupLayout jPanel9Layout = new javax.swing.GroupLayout(jPanel9);
        jPanel9.setLayout(jPanel9Layout);
        jPanel9Layout.setHorizontalGroup(
            jPanel9Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel9Layout.createSequentialGroup()
                .addGap(29, 29, 29)
                .addComponent(otm_state_Label)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addComponent(dbms_stateLabel)
                .addGap(35, 35, 35))
        );
        jPanel9Layout.setVerticalGroup(
            jPanel9Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel9Layout.createSequentialGroup()
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addGroup(jPanel9Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(dbms_stateLabel)
                    .addComponent(otm_state_Label))
                .addContainerGap())
        );

        setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);
        setResizable(false);

        planningPanel.setBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 0, 0), 1, true));

        jLabel11.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        jLabel11.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel11.setText("Planning");

        twilightButton.setFont(new java.awt.Font("Arial", 1, 10)); // NOI18N
        twilightButton.setText("Twilight");
        twilightButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                twilightButtonActionPerformed(evt);
            }
        });

        auto_start_timeCheckBox.setText("Auto");

        fetchLiveButton.setFont(new java.awt.Font("Arial", 1, 10)); // NOI18N
        fetchLiveButton.setText("Fetch Live");

        jPanel10.setBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 0, 0), 1, true));
        jPanel10.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        jLabel15.setFont(new java.awt.Font("Arial", 1, 12)); // NOI18N
        jLabel15.setText("Start Date:");
        jPanel10.add(jLabel15, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 10, -1, -1));

        planning_controlsPanel.setBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 0, 0), 1, true));
        planning_controlsPanel.setLayout(new javax.swing.BoxLayout(planning_controlsPanel, javax.swing.BoxLayout.PAGE_AXIS));

        auto_fetchCheckBox.setText("Auto");

        jLabel8.setFont(new java.awt.Font("Arial", 1, 12)); // NOI18N
        jLabel8.setText("Airmass Limit");

        airmass_limitTextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                airmass_limitTextFieldActionPerformed(evt);
            }
        });

        jLabel12.setFont(new java.awt.Font("Arial", 1, 12)); // NOI18N
        jLabel12.setText("Start Time:");

        HoursSpinner.addChangeListener(new javax.swing.event.ChangeListener() {
            public void stateChanged(javax.swing.event.ChangeEvent evt) {
                HoursSpinnerStateChanged(evt);
            }
        });

        MinutesSpinner.addChangeListener(new javax.swing.event.ChangeListener() {
            public void stateChanged(javax.swing.event.ChangeEvent evt) {
                MinutesSpinnerStateChanged(evt);
            }
        });

        jLabel13.setFont(new java.awt.Font("Arial", 1, 12)); // NOI18N
        jLabel13.setText("Seeing");

        seeingTextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                seeingTextFieldActionPerformed(evt);
            }
        });

        acceptButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                acceptButtonActionPerformed(evt);
            }
        });

        cancelButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                cancelButtonActionPerformed(evt);
            }
        });

        jLabel7.setFont(new java.awt.Font("Arial", 1, 12)); // NOI18N
        jLabel7.setText("   Accept    Cancel");

        jLabel17.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        jLabel17.setText("Target List:");

        sekected_target_listjLabel.setFont(new java.awt.Font("DejaVu Sans", 1, 14)); // NOI18N
        sekected_target_listjLabel.setForeground(new java.awt.Color(5, 180, 7));

        javax.swing.GroupLayout planningPanelLayout = new javax.swing.GroupLayout(planningPanel);
        planningPanel.setLayout(planningPanelLayout);
        planningPanelLayout.setHorizontalGroup(
            planningPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(planningPanelLayout.createSequentialGroup()
                .addComponent(jLabel11, javax.swing.GroupLayout.PREFERRED_SIZE, 413, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, planningPanelLayout.createSequentialGroup()
                .addGap(18, 18, 18)
                .addGroup(planningPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                    .addGroup(planningPanelLayout.createSequentialGroup()
                        .addComponent(jLabel17)
                        .addGap(4, 4, 4)
                        .addComponent(sekected_target_listjLabel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                        .addComponent(jLabel7, javax.swing.GroupLayout.PREFERRED_SIZE, 140, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addGroup(planningPanelLayout.createSequentialGroup()
                        .addGroup(planningPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addGroup(planningPanelLayout.createSequentialGroup()
                                .addGroup(planningPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                    .addComponent(jLabel12)
                                    .addComponent(jPanel10, javax.swing.GroupLayout.PREFERRED_SIZE, 98, javax.swing.GroupLayout.PREFERRED_SIZE))
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addGroup(planningPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                    .addGroup(planningPanelLayout.createSequentialGroup()
                                        .addComponent(HoursSpinner, javax.swing.GroupLayout.PREFERRED_SIZE, 70, javax.swing.GroupLayout.PREFERRED_SIZE)
                                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                        .addComponent(MinutesSpinner, javax.swing.GroupLayout.PREFERRED_SIZE, 60, javax.swing.GroupLayout.PREFERRED_SIZE))
                                    .addComponent(planning_controlsPanel, javax.swing.GroupLayout.PREFERRED_SIZE, 159, javax.swing.GroupLayout.PREFERRED_SIZE))
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                            .addGroup(planningPanelLayout.createSequentialGroup()
                                .addGroup(planningPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                    .addComponent(jLabel8)
                                    .addComponent(jLabel13))
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                                .addGroup(planningPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING, false)
                                    .addComponent(seeingTextField)
                                    .addComponent(airmass_limitTextField))
                                .addGap(0, 0, Short.MAX_VALUE)))
                        .addGroup(planningPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addGroup(planningPanelLayout.createSequentialGroup()
                                .addComponent(twilightButton, javax.swing.GroupLayout.PREFERRED_SIZE, 80, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addGap(6, 6, 6)
                                .addComponent(auto_start_timeCheckBox))
                            .addGroup(planningPanelLayout.createSequentialGroup()
                                .addGap(86, 86, 86)
                                .addComponent(auto_fetchCheckBox))
                            .addComponent(fetchLiveButton)
                            .addGroup(planningPanelLayout.createSequentialGroup()
                                .addGap(12, 12, 12)
                                .addComponent(acceptButton, javax.swing.GroupLayout.PREFERRED_SIZE, 50, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addGap(12, 12, 12)
                                .addComponent(cancelButton, javax.swing.GroupLayout.PREFERRED_SIZE, 50, javax.swing.GroupLayout.PREFERRED_SIZE)))))
                .addGap(29, 29, 29))
        );
        planningPanelLayout.setVerticalGroup(
            planningPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(planningPanelLayout.createSequentialGroup()
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addComponent(jLabel11)
                .addGap(6, 6, 6)
                .addGroup(planningPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(planningPanelLayout.createSequentialGroup()
                        .addGroup(planningPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(jPanel10, javax.swing.GroupLayout.PREFERRED_SIZE, 33, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(planning_controlsPanel, javax.swing.GroupLayout.PREFERRED_SIZE, 33, javax.swing.GroupLayout.PREFERRED_SIZE))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(planningPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(MinutesSpinner, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addGroup(planningPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                                .addComponent(jLabel12)
                                .addComponent(HoursSpinner, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(planningPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(seeingTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(jLabel13))
                        .addGap(3, 3, 3)
                        .addGroup(planningPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(airmass_limitTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 24, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(jLabel8, javax.swing.GroupLayout.PREFERRED_SIZE, 24, javax.swing.GroupLayout.PREFERRED_SIZE))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                    .addGroup(planningPanelLayout.createSequentialGroup()
                        .addGroup(planningPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(twilightButton)
                            .addComponent(auto_start_timeCheckBox))
                        .addGap(6, 6, 6)
                        .addGroup(planningPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(auto_fetchCheckBox)
                            .addComponent(fetchLiveButton))
                        .addGroup(planningPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addGroup(planningPanelLayout.createSequentialGroup()
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                                .addComponent(acceptButton, javax.swing.GroupLayout.PREFERRED_SIZE, 50, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addGap(6, 6, 6))
                            .addGroup(planningPanelLayout.createSequentialGroup()
                                .addGap(18, 18, 18)
                                .addComponent(cancelButton, javax.swing.GroupLayout.PREFERRED_SIZE, 50, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)))))
                .addGroup(planningPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                    .addComponent(jLabel7)
                    .addComponent(jLabel17, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(sekected_target_listjLabel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addGap(30, 30, 30))
        );

        detailsPanel.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));

        jScrollPane3.setHorizontalScrollBarPolicy(javax.swing.ScrollPaneConstants.HORIZONTAL_SCROLLBAR_NEVER);

        observationTable.setModel(new javax.swing.table.DefaultTableModel(
            new Object [][] {
                {null, null},
                {null, null},
                {null, null},
                {null, null}
            },
            new String [] {
                "Parameter", "Value"
            }
        ) {
            Class[] types = new Class [] {
                java.lang.String.class, java.lang.String.class
            };
            boolean[] canEdit = new boolean [] {
                false, true
            };

            public Class getColumnClass(int columnIndex) {
                return types [columnIndex];
            }

            public boolean isCellEditable(int rowIndex, int columnIndex) {
                return canEdit [columnIndex];
            }
        });
        jScrollPane3.setViewportView(observationTable);

        javax.swing.GroupLayout jPanel6Layout = new javax.swing.GroupLayout(jPanel6);
        jPanel6.setLayout(jPanel6Layout);
        jPanel6Layout.setHorizontalGroup(
            jPanel6Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel6Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jScrollPane3)
                .addContainerGap())
        );
        jPanel6Layout.setVerticalGroup(
            jPanel6Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel6Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jScrollPane3, javax.swing.GroupLayout.PREFERRED_SIZE, 289, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        detailedTabbedPane.addTab("Observation", jPanel6);

        jScrollPane4.setHorizontalScrollBarPolicy(javax.swing.ScrollPaneConstants.HORIZONTAL_SCROLLBAR_NEVER);
        jScrollPane4.setVerticalScrollBarPolicy(javax.swing.ScrollPaneConstants.VERTICAL_SCROLLBAR_NEVER);

        etcTable.setModel(new javax.swing.table.DefaultTableModel(
            new Object [][] {
                {null, null},
                {null, null},
                {null, null},
                {null, null}
            },
            new String [] {
                "Parameter", "Value"
            }
        ) {
            Class[] types = new Class [] {
                java.lang.String.class, java.lang.String.class
            };
            boolean[] canEdit = new boolean [] {
                false, true
            };

            public Class getColumnClass(int columnIndex) {
                return types [columnIndex];
            }

            public boolean isCellEditable(int rowIndex, int columnIndex) {
                return canEdit [columnIndex];
            }
        });
        jScrollPane4.setViewportView(etcTable);

        javax.swing.GroupLayout jPanel7Layout = new javax.swing.GroupLayout(jPanel7);
        jPanel7.setLayout(jPanel7Layout);
        jPanel7Layout.setHorizontalGroup(
            jPanel7Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel7Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jScrollPane4)
                .addContainerGap())
        );
        jPanel7Layout.setVerticalGroup(
            jPanel7Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel7Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jScrollPane4)
                .addContainerGap())
        );

        detailedTabbedPane.addTab("ETC", jPanel7);

        jScrollPane5.setHorizontalScrollBarPolicy(javax.swing.ScrollPaneConstants.HORIZONTAL_SCROLLBAR_NEVER);

        planTable.setModel(new javax.swing.table.DefaultTableModel(
            new Object [][] {
                {null, null},
                {null, null},
                {null, null},
                {null, null}
            },
            new String [] {
                "Parameter", "Value"
            }
        ) {
            Class[] types = new Class [] {
                java.lang.String.class, java.lang.String.class
            };
            boolean[] canEdit = new boolean [] {
                false, true
            };

            public Class getColumnClass(int columnIndex) {
                return types [columnIndex];
            }

            public boolean isCellEditable(int rowIndex, int columnIndex) {
                return canEdit [columnIndex];
            }
        });
        jScrollPane5.setViewportView(planTable);

        javax.swing.GroupLayout jPanel8Layout = new javax.swing.GroupLayout(jPanel8);
        jPanel8.setLayout(jPanel8Layout);
        jPanel8Layout.setHorizontalGroup(
            jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel8Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jScrollPane5)
                .addContainerGap())
        );
        jPanel8Layout.setVerticalGroup(
            jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel8Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jScrollPane5)
                .addContainerGap())
        );

        detailedTabbedPane.addTab("Plan", jPanel8);

        jLabel9.setFont(new java.awt.Font("Arial", 1, 13)); // NOI18N
        jLabel9.setText("Target Name:");

        selectedTargetNameTextField.setEditable(false);
        selectedTargetNameTextField.setBackground(java.awt.Color.yellow);
        selectedTargetNameTextField.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N

        jLabel1.setFont(new java.awt.Font("Cantarell", 1, 18)); // NOI18N
        jLabel1.setText("Dispersion Scales (Å/pixel) ");

        jLabel2.setFont(new java.awt.Font("Cantarell", 1, 18)); // NOI18N
        jLabel2.setText("U  0.308         R  0.569");

        jLabel3.setFont(new java.awt.Font("Cantarell", 1, 18)); // NOI18N
        jLabel3.setText("G  0.422         I  0.693");

        jLabel4.setFont(new java.awt.Font("Cantarell", 1, 18)); // NOI18N
        jLabel4.setText("Plate Scale");

        jLabel5.setFont(new java.awt.Font("Cantarell", 3, 14)); // NOI18N

        jLabel6.setFont(new java.awt.Font("Cantarell", 1, 18)); // NOI18N
        jLabel6.setText("0.191\"/pixel");

        javax.swing.GroupLayout detailsPanelLayout = new javax.swing.GroupLayout(detailsPanel);
        detailsPanel.setLayout(detailsPanelLayout);
        detailsPanelLayout.setHorizontalGroup(
            detailsPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(detailsPanelLayout.createSequentialGroup()
                .addGroup(detailsPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(detailsPanelLayout.createSequentialGroup()
                        .addGap(38, 38, 38)
                        .addGroup(detailsPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(jLabel2)
                            .addComponent(jLabel3)))
                    .addGroup(detailsPanelLayout.createSequentialGroup()
                        .addGap(8, 8, 8)
                        .addComponent(jLabel9)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(selectedTargetNameTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 219, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addGroup(detailsPanelLayout.createSequentialGroup()
                        .addContainerGap()
                        .addComponent(detailedTabbedPane, javax.swing.GroupLayout.PREFERRED_SIZE, 438, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addGroup(detailsPanelLayout.createSequentialGroup()
                        .addGap(18, 18, 18)
                        .addComponent(jLabel1)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(jLabel5)
                        .addGap(62, 62, 62)
                        .addGroup(detailsPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(jLabel6)
                            .addComponent(jLabel4))))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );
        detailsPanelLayout.setVerticalGroup(
            detailsPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(detailsPanelLayout.createSequentialGroup()
                .addGap(0, 0, 0)
                .addGroup(detailsPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(selectedTargetNameTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 26, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(jLabel9))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(detailedTabbedPane, javax.swing.GroupLayout.PREFERRED_SIZE, 412, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(detailsPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(jLabel1)
                    .addComponent(jLabel5)
                    .addComponent(jLabel4))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(detailsPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(jLabel2)
                    .addComponent(jLabel6))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jLabel3)
                .addGap(58, 58, 58))
        );

        left_mainPanel.setBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 0, 0), 2, true));

        main_tableScrollPane.setBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 0, 0), 2, true));
        main_tableScrollPane.setVerticalScrollBarPolicy(javax.swing.ScrollPaneConstants.VERTICAL_SCROLLBAR_ALWAYS);
        main_tableScrollPane.setMinimumSize(new java.awt.Dimension(1200, 1200));
        main_tableScrollPane.setPreferredSize(new java.awt.Dimension(1200, 1200));

        mainTable.setModel(new javax.swing.table.DefaultTableModel(
            new Object [][] {
                {null, null, null, null, null, null, null, null, null},
                {null, null, null, null, null, null, null, null, null},
                {null, null, null, null, null, null, null, null, null},
                {null, null, null, null, null, null, null, null, null}
            },
            new String [] {
                "Target Name", "RA", "DEC", "Requested Exptime", "Exptime", "SNR", "Requested Slit", "Slit", "Airmass"
            }
        ));
        mainTable.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyPressed(java.awt.event.KeyEvent evt) {
                mainTableKeyPressed(evt);
            }
        });
        main_tableScrollPane.setViewportView(mainTable);

        javax.swing.GroupLayout left_mainPanelLayout = new javax.swing.GroupLayout(left_mainPanel);
        left_mainPanel.setLayout(left_mainPanelLayout);
        left_mainPanelLayout.setHorizontalGroup(
            left_mainPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(left_mainPanelLayout.createSequentialGroup()
                .addGap(5, 5, 5)
                .addGroup(left_mainPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(main_tableScrollPane, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(myOScontrolsPanel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );
        left_mainPanelLayout.setVerticalGroup(
            left_mainPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(left_mainPanelLayout.createSequentialGroup()
                .addGap(5, 5, 5)
                .addComponent(myOScontrolsPanel, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addComponent(main_tableScrollPane, javax.swing.GroupLayout.PREFERRED_SIZE, 638, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addGap(137, 137, 137))
        );

        NGPSMenu.setText("NGPS");

        aboutNGPSMenuItem.setText("About NGPS planning");
        aboutNGPSMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                aboutNGPSMenuItemActionPerformed(evt);
            }
        });
        NGPSMenu.add(aboutNGPSMenuItem);

        preferencesMenuItem.setText("Preferences");
        preferencesMenuItem.setEnabled(false);
        preferencesMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                preferencesMenuItemActionPerformed(evt);
            }
        });
        NGPSMenu.add(preferencesMenuItem);

        shutdownMenuItem.setText("Shutdown");
        shutdownMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                shutdownMenuItemActionPerformed(evt);
            }
        });
        NGPSMenu.add(shutdownMenuItem);

        quitMenuItem.setText("Quit");
        quitMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                quitMenuItemActionPerformed(evt);
            }
        });
        NGPSMenu.add(quitMenuItem);

        mainMenuBar.add(NGPSMenu);

        jMenu1.setText("File");

        new_target_listMenuItem.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_N, java.awt.event.InputEvent.CTRL_DOWN_MASK));
        new_target_listMenuItem.setText("New Target List");
        new_target_listMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                new_target_listMenuItemActionPerformed(evt);
            }
        });
        jMenu1.add(new_target_listMenuItem);
        jMenu1.add(jSeparator3);

        openMenuItem.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_O, java.awt.event.InputEvent.CTRL_DOWN_MASK));
        openMenuItem.setText("Open");
        openMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                openMenuItemActionPerformed(evt);
            }
        });
        jMenu1.add(openMenuItem);
        jMenu1.add(jSeparator2);

        saveMenuItem.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_S, java.awt.event.InputEvent.CTRL_DOWN_MASK));
        saveMenuItem.setText("Save");
        saveMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                saveMenuItemActionPerformed(evt);
            }
        });
        jMenu1.add(saveMenuItem);

        save_asMenuItem.setText("Save as");
        save_asMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                save_asMenuItemActionPerformed(evt);
            }
        });
        jMenu1.add(save_asMenuItem);
        jMenu1.add(jSeparator1);

        importMenuItem.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_I, java.awt.event.InputEvent.CTRL_DOWN_MASK));
        importMenuItem.setText("Import");
        importMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                importMenuItemActionPerformed(evt);
            }
        });
        jMenu1.add(importMenuItem);

        exportMenuItem.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_E, java.awt.event.InputEvent.CTRL_DOWN_MASK));
        exportMenuItem.setText("Export");
        exportMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                exportMenuItemActionPerformed(evt);
            }
        });
        jMenu1.add(exportMenuItem);

        mainMenuBar.add(jMenu1);

        editMenu.setText("Edit");

        newTargetCheckBoxMenuItem.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_T, java.awt.event.InputEvent.CTRL_DOWN_MASK));
        newTargetCheckBoxMenuItem.setSelected(true);
        newTargetCheckBoxMenuItem.setText("Insert New Target");
        newTargetCheckBoxMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                newTargetCheckBoxMenuItemActionPerformed(evt);
            }
        });
        editMenu.add(newTargetCheckBoxMenuItem);

        new_target_from_databaseMenuItem.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_D, java.awt.event.InputEvent.CTRL_DOWN_MASK));
        new_target_from_databaseMenuItem.setText("Get Target from Database");
        new_target_from_databaseMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                new_target_from_databaseMenuItemActionPerformed(evt);
            }
        });
        editMenu.add(new_target_from_databaseMenuItem);

        cutMenuItem.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_X, java.awt.event.InputEvent.CTRL_DOWN_MASK));
        cutMenuItem.setText("Cut");
        editMenu.add(cutMenuItem);

        copyMenuItem.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_C, java.awt.event.InputEvent.CTRL_DOWN_MASK));
        copyMenuItem.setText("Copy");
        copyMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                copyMenuItemActionPerformed(evt);
            }
        });
        editMenu.add(copyMenuItem);

        pasteMenuItem.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_P, java.awt.event.InputEvent.CTRL_DOWN_MASK));
        pasteMenuItem.setText("Paste");
        editMenu.add(pasteMenuItem);

        deleteMenuItem.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_D, java.awt.event.InputEvent.CTRL_DOWN_MASK));
        deleteMenuItem.setText("Delete");
        editMenu.add(deleteMenuItem);

        insert_copiedMenuItem.setText("Insert Copied Cells");
        insert_copiedMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                insert_copiedMenuItemActionPerformed(evt);
            }
        });
        editMenu.add(insert_copiedMenuItem);

        mainMenuBar.add(editMenu);

        viewMenu.setText("View");

        basicRadioButtonMenuItem.setSelected(true);
        basicRadioButtonMenuItem.setText("Basic");
        viewMenu.add(basicRadioButtonMenuItem);

        detailedRadioButtonMenuItem.setSelected(true);
        detailedRadioButtonMenuItem.setText("Detailed");
        detailedRadioButtonMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                detailedRadioButtonMenuItemActionPerformed(evt);
            }
        });
        viewMenu.add(detailedRadioButtonMenuItem);

        otm_outputMenuItem.setText("OTM Output");
        otm_outputMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                otm_outputMenuItemActionPerformed(evt);
            }
        });
        viewMenu.add(otm_outputMenuItem);

        zoomInMenuItem.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_EQUALS, java.awt.event.InputEvent.CTRL_DOWN_MASK));
        zoomInMenuItem.setText("Zoom In");
        zoomInMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                zoomInMenuItemActionPerformed(evt);
            }
        });
        viewMenu.add(zoomInMenuItem);

        zoomOutMenuItem.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_MINUS, java.awt.event.InputEvent.CTRL_DOWN_MASK));
        zoomOutMenuItem.setText("Zoom Out");
        zoomOutMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                zoomOutMenuItemActionPerformed(evt);
            }
        });
        viewMenu.add(zoomOutMenuItem);

        plotMenuItem.setText("Plot");
        plotMenuItem.setEnabled(false);
        viewMenu.add(plotMenuItem);

        mainMenuBar.add(viewMenu);

        toolsMenu.setText("Tools");

        optimizeMenuItem.setText("Optimize observation order");
        optimizeMenuItem.setEnabled(false);
        toolsMenu.add(optimizeMenuItem);

        hourly_weatherMenuItem.setText("Hourly weather forcast");
        hourly_weatherMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                hourly_weatherMenuItemActionPerformed(evt);
            }
        });
        toolsMenu.add(hourly_weatherMenuItem);

        ten_day_weatherMenuItem.setText("10 day weather forcast");
        ten_day_weatherMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                ten_day_weatherMenuItemActionPerformed(evt);
            }
        });
        toolsMenu.add(ten_day_weatherMenuItem);

        ephemerisMenuItem.setText("Ephemeris");
        toolsMenu.add(ephemerisMenuItem);

        reportBugMenuItem.setText("Report bug");
        reportBugMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                reportBugMenuItemActionPerformed(evt);
            }
        });
        toolsMenu.add(reportBugMenuItem);

        engineeringMenuItem.setText("Engineering");
        engineeringMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                engineeringMenuItemActionPerformed(evt);
            }
        });
        toolsMenu.add(engineeringMenuItem);

        connectionsMenuItem.setText("Connections");
        connectionsMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                connectionsMenuItemActionPerformed(evt);
            }
        });
        toolsMenu.add(connectionsMenuItem);

        mainMenuBar.add(toolsMenu);

        helpMenu.setText("Help");

        quick_startMenuItem.setText("This GUI manual");
        quick_startMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                quick_startMenuItemActionPerformed(evt);
            }
        });
        helpMenu.add(quick_startMenuItem);

        etcMenuItem.setText("OTM and ETC");
        etcMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                etcMenuItemActionPerformed(evt);
            }
        });
        helpMenu.add(etcMenuItem);

        data_reductionMenuItem.setText("Data reduction");
        data_reductionMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                data_reductionMenuItemActionPerformed(evt);
            }
        });
        helpMenu.add(data_reductionMenuItem);

        calibrationMenuItem.setText("Calibration");
        calibrationMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                calibrationMenuItemActionPerformed(evt);
            }
        });
        helpMenu.add(calibrationMenuItem);

        line_listMenuItem.setText("Line list");
        line_listMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                line_listMenuItemActionPerformed(evt);
            }
        });
        helpMenu.add(line_listMenuItem);

        example_spectraMenuItem.setText("Example spectra");
        example_spectraMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                example_spectraMenuItemActionPerformed(evt);
            }
        });
        helpMenu.add(example_spectraMenuItem);

        ngps_instrument_manualMenuItem.setText("NGPS instrument manual");
        ngps_instrument_manualMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                ngps_instrument_manualMenuItemActionPerformed(evt);
            }
        });
        helpMenu.add(ngps_instrument_manualMenuItem);

        errors_diagnosticsMenuItem.setText("OTMflag codes");
        errors_diagnosticsMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                errors_diagnosticsMenuItemActionPerformed(evt);
            }
        });
        helpMenu.add(errors_diagnosticsMenuItem);

        mainMenuBar.add(helpMenu);

        setJMenuBar(mainMenuBar);

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, layout.createSequentialGroup()
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addComponent(left_mainPanel, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(planningPanel, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(detailsPanel, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                    .addComponent(left_mainPanel, javax.swing.GroupLayout.PREFERRED_SIZE, 0, Short.MAX_VALUE)
                    .addGroup(layout.createSequentialGroup()
                        .addComponent(planningPanel, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(detailsPanel, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)))
                .addContainerGap())
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents

    private void preferencesMenuItemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_preferencesMenuItemActionPerformed
        
    }//GEN-LAST:event_preferencesMenuItemActionPerformed

    private void importMenuItemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_importMenuItemActionPerformed
        boolean state = importMenuItem.isSelected();
 //       mainTabbedPane.removeAll();
        selectFile();
//        myimportCSVPanel.setModelType(myTargetListParser.getModelType());
//        mainTabbedPane.addTab("Import CSV file editing",myimportCSVPanel); 
    }//GEN-LAST:event_importMenuItemActionPerformed

    private void copyMenuItemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_copyMenuItemActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_copyMenuItemActionPerformed

    private void detailedRadioButtonMenuItemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_detailedRadioButtonMenuItemActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_detailedRadioButtonMenuItemActionPerformed

    private void zoomOutMenuItemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_zoomOutMenuItemActionPerformed
        decreaseFontSize();
    }//GEN-LAST:event_zoomOutMenuItemActionPerformed

    private void line_listMenuItemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_line_listMenuItemActionPerformed
        myBrowserDisplay.executeFirefox(myBrowserDisplay.URLMAP.getProperty("LINELIST"));
    }//GEN-LAST:event_line_listMenuItemActionPerformed

    private void saveMenuItemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_saveMenuItemActionPerformed
       if(dbms.selectedObservationSet == null){
                java.lang.String current_set_name = JOptionPane.showInputDialog(this,"Enter label for this Target List","",JOptionPane.QUESTION_MESSAGE);
                java.lang.String airmass_limit_string = airmass_limitTextField.getText();
                double airmass_limit = Double.parseDouble(airmass_limit_string);
                dbms.myOTMlauncher.setAirmass_limit(airmass_limit);
                dbms.SaveAs(current_set_name);
       }else{          
               java.lang.String airmass_limit_string = airmass_limitTextField.getText();
               double airmass_limit = Double.parseDouble(airmass_limit_string);
               dbms.myOTMlauncher.setAirmass_limit(airmass_limit);
               dbms.executeUpdateTargetTable(dbms.selectedObservationSet,dbms.myTargetDBMSTableModel);
       } 
    }//GEN-LAST:event_saveMenuItemActionPerformed

    private void openMenuItemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_openMenuItemActionPerformed
        dbms.queryObservationSets(dbms.getOWNER());
        myTargetSetFrame.setVisible(true);  
//         mainTabbedPane.addTab("Selected target set", myretrieveDatabasePanel);
    }//GEN-LAST:event_openMenuItemActionPerformed

    private void acceptButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_acceptButtonActionPerformed
       try{
           if(dbms.selectedObservationSet == null){
               java.lang.String current_set_name = JOptionPane.showInputDialog(this,"Enter label for this Target List","",JOptionPane.QUESTION_MESSAGE);
               java.lang.String airmass_limit_string = airmass_limitTextField.getText();
               double airmass_limit = Double.parseDouble(airmass_limit_string);
               dbms.myOTMlauncher.setAirmass_limit(airmass_limit);
                dbms.SaveAs(current_set_name);
           }else{
               java.lang.String airmass_limit_string = airmass_limitTextField.getText();
               double airmass_limit = Double.parseDouble(airmass_limit_string);
               dbms.myOTMlauncher.setAirmass_limit(airmass_limit);
               dbms.executeUpdateTargetTable(dbms.selectedObservationSet,dbms.myTargetDBMSTableModel);
           }
       }catch(Exception e){
           System.out.println("Error Updating the DBMS table"+e.toString());
       }
    }//GEN-LAST:event_acceptButtonActionPerformed

    private void cancelButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_cancelButtonActionPerformed
        try{
          if(dbms.selectedObservationSet == null){
             dbms.createNewTargetList(); 
          }
          if(dbms.selectedObservationSet != null){
             dbms.queryObservations(dbms.selectedObservationSet.getSET_ID()); 
          }          
        }catch(Exception e){
           System.out.println(e.toString());
        }
    }//GEN-LAST:event_cancelButtonActionPerformed

    private void new_target_listMenuItemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_new_target_listMenuItemActionPerformed
         try{
          dbms.createNewTargetList();
        }catch(Exception e){
           System.out.println(e.toString());
        }       // TODO add your handling code here:
    }//GEN-LAST:event_new_target_listMenuItemActionPerformed

    private void twilightButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_twilightButtonActionPerformed
        myNightlyWindow.UpdateDisplay();
        myNightlyWindow.setVisible(true);
     
    }//GEN-LAST:event_twilightButtonActionPerformed

    private void save_asMenuItemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_save_asMenuItemActionPerformed
        try{
          java.lang.String current_set_name = JOptionPane.showInputDialog(this,"Enter label for this Target List","",JOptionPane.QUESTION_MESSAGE);
          dbms.executeSaveAs(current_set_name);
        }catch(Exception e){
           System.out.println(e.toString());
        }       // TODO add your handling code here:
    }//GEN-LAST:event_save_asMenuItemActionPerformed

    private void exportMenuItemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_exportMenuItemActionPerformed
        java.lang.String user_home = System.getProperty("user.home");
        java.lang.String ngps_directory = user_home+SEP+"ngps";
        java.lang.String set_name = dbms.getSelectedSetName();
        File check_directory = new File(ngps_directory);
        if(!check_directory.exists()){
            check_directory.mkdir();
        } 
        if(set_name.isEmpty()){
            java.lang.String current_set_name = JOptionPane.showInputDialog(this,"Enter file name for exported tabe.","",JOptionPane.QUESTION_MESSAGE);
            dbms.export(ngps_directory+SEP+current_set_name);
        }else{
           dbms.export(ngps_directory+SEP+set_name); 
        }       
    }//GEN-LAST:event_exportMenuItemActionPerformed

    private void zoomInMenuItemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_zoomInMenuItemActionPerformed
       increaseFontSize();
    }//GEN-LAST:event_zoomInMenuItemActionPerformed

    private void insert_copiedMenuItemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_insert_copiedMenuItemActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_insert_copiedMenuItemActionPerformed

    private void shutdownMenuItemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_shutdownMenuItemActionPerformed
       Object[] options = {"Cancel","OK"};
       int n = JOptionPane.showOptionDialog(this,"Do you really want to shutdown the Observation Sequencer?","OS Shutdown",
                                            JOptionPane.OK_CANCEL_OPTION,JOptionPane.QUESTION_MESSAGE,null,options,options[0]);       
        if(n == 1){   
            myObservationSequencerController.shutdown();
        }
    }//GEN-LAST:event_shutdownMenuItemActionPerformed

    private void quitMenuItemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_quitMenuItemActionPerformed
       try{
          dbms.conn.close();
          System.exit(0);
       }catch(Exception e){
           System.out.println(e.toString());
       }
    }//GEN-LAST:event_quitMenuItemActionPerformed

    private void otm_outputMenuItemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_otm_outputMenuItemActionPerformed
        myOTMoutputFrame.setVisible(true);
    }//GEN-LAST:event_otm_outputMenuItemActionPerformed

    private void HoursSpinnerStateChanged(javax.swing.event.ChangeEvent evt) {//GEN-FIRST:event_HoursSpinnerStateChanged
        // TODO add your handling code here:
        hours = ((Integer)HoursSpinner.getValue()).intValue();
        updateStarTime();
//        updateSelectedTimeDate();
    }//GEN-LAST:event_HoursSpinnerStateChanged

    private void MinutesSpinnerStateChanged(javax.swing.event.ChangeEvent evt) {//GEN-FIRST:event_MinutesSpinnerStateChanged
        minutes = ((Integer)MinutesSpinner.getValue()).intValue();
        updateStarTime();
//        updateSelectedTimeDate();
        // TODO add your handling code here:
    }//GEN-LAST:event_MinutesSpinnerStateChanged

    private void seeingTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_seeingTextFieldActionPerformed
        try{
           java.lang.String my_seeing = seeingTextField.getText();
           java.lang.Double current = Double.valueOf(my_seeing);
           dbms.myOTMlauncher.setSeeing(current.intValue());
       }catch(Exception e){
          System.out.println(e.toString());
       }
    }//GEN-LAST:event_seeingTextFieldActionPerformed

    private void engineeringMenuItemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_engineeringMenuItemActionPerformed
       // boolean e_state = engineeringMenuItem.
        myEngineeringFrame.setVisible(true);
    }//GEN-LAST:event_engineeringMenuItemActionPerformed

    private void new_target_from_databaseMenuItemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_new_target_from_databaseMenuItemActionPerformed
        myTargetSearchFrame.setVisible(true);
    }//GEN-LAST:event_new_target_from_databaseMenuItemActionPerformed

    private void newTargetCheckBoxMenuItemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_newTargetCheckBoxMenuItemActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_newTargetCheckBoxMenuItemActionPerformed

    private void aboutNGPSMenuItemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_aboutNGPSMenuItemActionPerformed
        my_aboutNGPSFrame.setVisible(true);
    }//GEN-LAST:event_aboutNGPSMenuItemActionPerformed

    private void hourly_weatherMenuItemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_hourly_weatherMenuItemActionPerformed
        myBrowserDisplay.openURLkey("HOURLY_WEATHER");
    }//GEN-LAST:event_hourly_weatherMenuItemActionPerformed

    private void ten_day_weatherMenuItemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_ten_day_weatherMenuItemActionPerformed
        myBrowserDisplay.openURLkey("DAY10_WEATHER");
    }//GEN-LAST:event_ten_day_weatherMenuItemActionPerformed

    private void reportBugMenuItemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_reportBugMenuItemActionPerformed
        myBrowserDisplay.openURLkey("BUG_REPORT");
    }//GEN-LAST:event_reportBugMenuItemActionPerformed

    private void airmass_limitTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_airmass_limitTextFieldActionPerformed
       try{
          java.lang.String airmass_limit_string = airmass_limitTextField.getText();
          double airmass_limit = Double.parseDouble(airmass_limit_string);
          this.dbms.myOTMlauncher.setAirmass_limit(airmass_limit);
       }catch(Exception e){
          System.out.println(e.toString());
          airmass_limitTextField.setText("");        ;
       }
    }//GEN-LAST:event_airmass_limitTextFieldActionPerformed

    private void quick_startMenuItemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_quick_startMenuItemActionPerformed
       myBrowserDisplay.openURLkey("GUI_MANUAL");
    }//GEN-LAST:event_quick_startMenuItemActionPerformed

    private void etcMenuItemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_etcMenuItemActionPerformed
        myBrowserDisplay.openURLkey("OTM_ETC_MANUAL");
    }//GEN-LAST:event_etcMenuItemActionPerformed

    private void data_reductionMenuItemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_data_reductionMenuItemActionPerformed
        myBrowserDisplay.openURLkey("DATA_REDUCTION");
    }//GEN-LAST:event_data_reductionMenuItemActionPerformed

    private void calibrationMenuItemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_calibrationMenuItemActionPerformed
         myBrowserDisplay.openURLkey("CALIBRATION");
    }//GEN-LAST:event_calibrationMenuItemActionPerformed

    private void example_spectraMenuItemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_example_spectraMenuItemActionPerformed
        myBrowserDisplay.openURLkey("EXAMPLE_SPECTRA");
    }//GEN-LAST:event_example_spectraMenuItemActionPerformed

    private void ngps_instrument_manualMenuItemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_ngps_instrument_manualMenuItemActionPerformed
         myBrowserDisplay.openURLkey("NGPS_INSTRUMENT_MANUAL");
    }//GEN-LAST:event_ngps_instrument_manualMenuItemActionPerformed

    private void errors_diagnosticsMenuItemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_errors_diagnosticsMenuItemActionPerformed
         myBrowserDisplay.openURLkey("OTMFLAGS");
    }//GEN-LAST:event_errors_diagnosticsMenuItemActionPerformed

    private void connectionsMenuItemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_connectionsMenuItemActionPerformed
       myConnectionsFrame.setVisible(true);
    }//GEN-LAST:event_connectionsMenuItemActionPerformed

    private void mainTableKeyPressed(java.awt.event.KeyEvent evt) {//GEN-FIRST:event_mainTableKeyPressed

    }//GEN-LAST:event_mainTableKeyPressed

    private static class MyProgressUI extends BasicProgressBarUI {
        private Rectangle r = new Rectangle();

        @Override
        protected void paintIndeterminate(Graphics g, JComponent c) {
            try{
            Graphics2D g2d = (Graphics2D) g;
            g2d.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
            r = getBox(r);
            g.setColor(Color.green);
            g.fillOval(r.x, r.y, r.width, r.height);
            }catch(Exception e){
                System.out.println(e.toString());
            }
        }
    }
    public class ArrowAction extends javax.swing.AbstractAction {

    private String cmd;

    public ArrowAction(String cmd) {
        this.cmd = cmd;
    }

    @Override
    public void actionPerformed(ActionEvent e) {
        if (cmd.equalsIgnoreCase("LeftArrow")) {
            System.out.println("The left arrow was pressed!");
        } else if (cmd.equalsIgnoreCase("RightArrow")) {
            System.out.println("The right arrow was pressed!");
        } else if (cmd.equalsIgnoreCase("UpArrow")) {
            System.out.println("The up arrow was pressed!");
        } else if (cmd.equalsIgnoreCase("DownArrow")) {
            System.out.println("The down arrow was pressed!");
        }
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
            java.util.logging.Logger.getLogger(NGPSFrame.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (InstantiationException ex) {
            java.util.logging.Logger.getLogger(NGPSFrame.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (IllegalAccessException ex) {
            java.util.logging.Logger.getLogger(NGPSFrame.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (javax.swing.UnsupportedLookAndFeelException ex) {
            java.util.logging.Logger.getLogger(NGPSFrame.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        }
        //</editor-fold>

        /* Create and display the form */
        java.awt.EventQueue.invokeLater(new Runnable() {
            public void run() {
                new NGPSFrame(args).setVisible(true);
            }
        });
    }

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JSpinner HoursSpinner;
    private javax.swing.JSpinner MinutesSpinner;
    private javax.swing.JMenu NGPSMenu;
    private javax.swing.JMenuItem aboutNGPSMenuItem;
    private javax.swing.JButton acceptButton;
    private javax.swing.JTextField airmass_limitTextField;
    private javax.swing.JCheckBox auto_fetchCheckBox;
    private javax.swing.JCheckBox auto_start_timeCheckBox;
    private javax.swing.JRadioButtonMenuItem basicRadioButtonMenuItem;
    private javax.swing.JMenuItem calibrationMenuItem;
    private javax.swing.JButton cancelButton;
    private javax.swing.JMenuItem connectionsMenuItem;
    private javax.swing.JMenuItem copyMenuItem;
    private javax.swing.JMenuItem cutMenuItem;
    private javax.swing.JMenuItem data_reductionMenuItem;
    private javax.swing.JLabel dbms_stateLabel;
    private javax.swing.JMenuItem deleteMenuItem;
    private javax.swing.JRadioButtonMenuItem detailedRadioButtonMenuItem;
    private javax.swing.JTabbedPane detailedTabbedPane;
    private javax.swing.JPanel detailsPanel;
    private javax.swing.JMenu editMenu;
    private javax.swing.JMenuItem engineeringMenuItem;
    private javax.swing.JMenuItem ephemerisMenuItem;
    private javax.swing.JMenuItem errors_diagnosticsMenuItem;
    private javax.swing.JMenuItem etcMenuItem;
    private javax.swing.JTable etcTable;
    private javax.swing.JMenuItem example_spectraMenuItem;
    private javax.swing.JMenuItem exportMenuItem;
    private javax.swing.JButton fetchLiveButton;
    private javax.swing.JMenu helpMenu;
    private javax.swing.JMenuItem hourly_weatherMenuItem;
    private javax.swing.JMenuItem importMenuItem;
    private javax.swing.JMenuItem insert_copiedMenuItem;
    private javax.swing.JCheckBox jCheckBox2;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel11;
    private javax.swing.JLabel jLabel12;
    private javax.swing.JLabel jLabel13;
    private javax.swing.JLabel jLabel15;
    private javax.swing.JLabel jLabel17;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JLabel jLabel3;
    private javax.swing.JLabel jLabel4;
    private javax.swing.JLabel jLabel5;
    private javax.swing.JLabel jLabel6;
    private javax.swing.JLabel jLabel7;
    private javax.swing.JLabel jLabel8;
    private javax.swing.JLabel jLabel9;
    private javax.swing.JMenu jMenu1;
    private javax.swing.JMenu jMenu3;
    private javax.swing.JPanel jPanel10;
    private javax.swing.JPanel jPanel6;
    private javax.swing.JPanel jPanel7;
    private javax.swing.JPanel jPanel8;
    private javax.swing.JPanel jPanel9;
    private javax.swing.JScrollPane jScrollPane3;
    private javax.swing.JScrollPane jScrollPane4;
    private javax.swing.JScrollPane jScrollPane5;
    private javax.swing.JPopupMenu.Separator jSeparator1;
    private javax.swing.JPopupMenu.Separator jSeparator2;
    private javax.swing.JPopupMenu.Separator jSeparator3;
    private javax.swing.JPanel left_mainPanel;
    private javax.swing.JMenuItem line_listMenuItem;
    private javax.swing.JMenuBar mainMenuBar;
    private javax.swing.JTable mainTable;
    private javax.swing.JScrollPane main_tableScrollPane;
    private edu.caltech.palomar.instruments.ngps.gui.OScontrolsPanel myOScontrolsPanel;
    private javax.swing.JCheckBoxMenuItem newTargetCheckBoxMenuItem;
    private javax.swing.JMenuItem new_target_from_databaseMenuItem;
    private javax.swing.JMenuItem new_target_listMenuItem;
    private javax.swing.JMenuItem ngps_instrument_manualMenuItem;
    private javax.swing.JTable observationTable;
    private javax.swing.JMenuItem openMenuItem;
    private javax.swing.JMenuItem optimizeMenuItem;
    private javax.swing.JMenuItem otm_outputMenuItem;
    private javax.swing.JLabel otm_state_Label;
    private javax.swing.JMenuItem pasteMenuItem;
    private javax.swing.JTable planTable;
    private javax.swing.JPanel planningPanel;
    private javax.swing.JPanel planning_controlsPanel;
    private javax.swing.JMenuItem plotMenuItem;
    private javax.swing.JMenuItem preferencesMenuItem;
    private javax.swing.JMenuItem quick_startMenuItem;
    private javax.swing.JMenuItem quitMenuItem;
    private javax.swing.JMenuItem reportBugMenuItem;
    private javax.swing.JMenuItem saveMenuItem;
    private javax.swing.JMenuItem save_asMenuItem;
    private javax.swing.JTextField seeingTextField;
    private javax.swing.JLabel sekected_target_listjLabel;
    private javax.swing.JTextField selectedTargetNameTextField;
    private javax.swing.JMenuItem shutdownMenuItem;
    private javax.swing.JMenuItem ten_day_weatherMenuItem;
    private javax.swing.JMenu toolsMenu;
    private javax.swing.JButton twilightButton;
    private javax.swing.JMenu viewMenu;
    private javax.swing.JMenuItem zoomInMenuItem;
    private javax.swing.JMenuItem zoomOutMenuItem;
    // End of variables declaration//GEN-END:variables
}
