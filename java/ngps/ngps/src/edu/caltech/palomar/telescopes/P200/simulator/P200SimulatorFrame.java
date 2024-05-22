package edu.caltech.palomar.telescopes.P200.simulator;
//=== File Prolog =============================================================
//	This code was developed by California Institute of Technology
//      Caltech Optical Observatories, Palomar Observatory
//
//--- Contents ----------------------------------------------------------------
//	 P200SimulatorFrame- This is the main frame for the P200 Simulator
//
//--- Description -------------------------------------------------------------
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
//
//      July 14, 2010 Jennifer Milburn
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
import edu.caltech.palomar.util.gui.DoubleTextField;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import com.borland.jbcl.layout.*;
import java.io.File;
import java.awt.event.*;
import edu.caltech.palomar.util.gui.*;
import java.beans.*;
import edu.caltech.palomar.util.sindex.client.HTMIndexSearchClient;
import jsky.science.Coordinates;
import jsky.science.CoordinatesOffset;
import edu.caltech.palomar.telescopes.P200.simulator.P200ServerSocket;
import edu.caltech.palomar.telescopes.P200.TelescopeObject;
import edu.caltech.palomar.telescopes.P200.TelescopesIniReader;
/*================================================================================================
/       P200SimulatorFrame extends JFrame
/=================================================================================================*/
public class P200SimulatorFrame extends JFrame {
 transient private PropertyChangeSupport propertyChangeListeners = new PropertyChangeSupport(this);
  public     P200ServerSocket             myServerSocket;
  int        myServerPort = 2345;
  private    boolean isDVConnected;
  public int                              READ_DELAY              = 20;
  private double azimuth;
  private double hourAngle;
  private double zenithAngle;
  private double airMass;
  private double rightAscension;
  private double declination;

  JPanel contentPane;
  JPanel jPanel1               = new JPanel();
  JPanel RALabelsPanel         = new JPanel();
  JPanel DECLabelsPanel        = new JPanel();
  JPanel jPanel4               = new JPanel();
  JPanel DECControlsPanel      = new JPanel();
  JPanel jPanel3               = new JPanel();
  JPanel jPanel2               = new JPanel();
  JPanel ResolutionSourcePanel = new JPanel();
  JPanel RAControlsPanel       = new JPanel();

  JMenuBar  menuBar1      = new JMenuBar();
  JMenu     menuFile      = new JMenu();
  JMenu     menuHelp      = new JMenu();
  JMenuItem menuFileExit  = new JMenuItem();
  JMenuItem menuHelpAbout = new JMenuItem();

//  JToolBar toolBar = new JToolBar();

  JButton GetCoordinatesButton   = new JButton();
  JButton ResetCoordinatesButton = new JButton();

  ImageIcon image1;
  ImageIcon image2;
  ImageIcon image3;
  ImageIcon DVonImage;
  ImageIcon DVoffImage;

  VerticalFlowLayout verticalFlowLayout2           = new VerticalFlowLayout();
  VerticalFlowLayout DECControlsverticalFlowLayout = new VerticalFlowLayout();
  VerticalFlowLayout verticalFlowLayout3           = new VerticalFlowLayout();
  VerticalFlowLayout RAControlsverticalFlowLayout  = new VerticalFlowLayout();
  VerticalFlowLayout DECLabelsverticalFlowLayout   = new VerticalFlowLayout();
  VerticalFlowLayout verticalFlowLayout1           = new VerticalFlowLayout();

  JLabel statusBar       = new JLabel();
  JLabel RALabel1        = new JLabel();
  JLabel DECLabel2       = new JLabel();
  JLabel DECLabel1       = new JLabel();
  JLabel jLabel4         = new JLabel();
  JLabel jLabel5         = new JLabel();
  JLabel jLabel6         = new JLabel();
  JLabel jLabel7         = new JLabel();
  JLabel jLabel1         = new JLabel();
  JLabel jLabel2         = new JLabel();
  JLabel jLabel3         = new JLabel();
  JLabel ObjectNameLabel = new JLabel();
  JLabel RALabel2        = new JLabel();
  JLabel TitleLabel         = new JLabel();

  DoubleTextField ZenithAngleTextField = new DoubleTextField(9,-90.0,90.0,0.0,7,true);
  DoubleTextField HourAngleTextField   = new DoubleTextField(9,0.0,360.0,0.0,6,true);
  DoubleTextField AirMassTextField     = new DoubleTextField(9,-90.0,90.0,0.0,7,true);
  DoubleTextField RATextField2         = new DoubleTextField(9,0.0,360.0,0.0,6,true);
  DoubleTextField DECTextField2        = new DoubleTextField(9,-90.0,90.0,0.0,7,true);

  JTextField RATextField1        = new JTextField();
  JTextField DECTextField1       = new JTextField();
  JTextField AzimuthTextField    = new JTextField();
  JTextField ObjectNameTextField = new JTextField();

  BorderLayout borderLayout1 = new BorderLayout();

  FlowLayout ResolutionSourceflowLayout = new FlowLayout();

  XYLayout xYLayout1 = new XYLayout();
  XYLayout xYLayout2 = new XYLayout();

  JScrollPane jScrollPane1 = new JScrollPane();
  JScrollPane jScrollPane2 = new JScrollPane();
  JScrollPane jScrollPane3 = new JScrollPane();

  public   java.lang.String USERDIR              = System.getProperty("user.dir");
  public   java.lang.String SEP                  = System.getProperty("file.separator");
  public   java.lang.String IMAGE_CACHE          = SEP + "images" + SEP;

  public JTextArea errorTextArea   = new JTextArea();
  public JTextArea receiveTextArea = new JTextArea();
  public JTextArea sendTextArea    = new JTextArea();

  JRadioButton                 SIMBADRadioButton              = new JRadioButton();
  JRadioButton                 NEDRadioButton                 = new JRadioButton();
  HTMIndexSearchClient         myHTMIndexSearchClient         = new HTMIndexSearchClient();
  ServerMediator               myServerMediator;
  private ButtonGroup          GSResolutionSourceButtonGroup  = new ButtonGroup();
  public TelescopeObject       myTelescopeObject;
  //Construct the frame
   public static int              DEFAULT_SERVERPORT = 49200;
    TelescopesIniReader myTelescopesIniReader = new TelescopesIniReader();
/*================================================================================================
/        P200SimulatorFrame Class Declaration
/=================================================================================================*/
  public P200SimulatorFrame() {
    enableEvents(AWTEvent.WINDOW_EVENT_MASK);
    myTelescopeObject = new TelescopeObject();
    myServerSocket = new P200ServerSocket(myTelescopesIniReader.SERVERPORT_INTEGER,myTelescopeObject);
    try {
      jbInit();
    }
    catch(Exception e) {
      e.printStackTrace();
    }
  }
/*================================================================================================
/        void jbInit() throws Exception
/=================================================================================================*/
  //Component initialization
  private void jbInit() throws Exception  {
    image1     = new ImageIcon(USERDIR + IMAGE_CACHE + "openFile.gif");
    image2     = new ImageIcon(USERDIR + IMAGE_CACHE + "closeFile.gif");
    image3     = new ImageIcon(USERDIR + IMAGE_CACHE + "help.gif");
    DVonImage  = new ImageIcon(USERDIR + IMAGE_CACHE + "DVon.gif");
    DVoffImage = new ImageIcon(USERDIR + IMAGE_CACHE + "DVoff.gif");

    contentPane = (JPanel) this.getContentPane();
    contentPane.setLayout(borderLayout1);
    this.setSize(new Dimension(735, 663));
    this.setTitle("Palomar Observatory: Hale 200 \" Telescope Simulator");
    statusBar.setText(" ");
    menuFile.setText("File");
    menuFileExit.setText("Exit");
    menuFileExit.addActionListener(new ActionListener()  {

      public void actionPerformed(ActionEvent e) {
        fileExit_actionPerformed(e);
      }
    });
    menuHelp.setText("Help");
    jLabel1.setText("Commands Received ");
    jLabel2.setText("Responses Sent");
    jLabel3.setText("Error Messages");
    ObjectNameLabel.setText("Object Name");
    SIMBADRadioButton.setText("SIMBAD");
    NEDRadioButton.setText("NED");
    GetCoordinatesButton.setText("Get Coordinates");
    ResetCoordinatesButton.setText("Reset");
    RALabel1.setText("RA (HH:MM:SS)");
    RALabel2.setText("RA (decimal Hours)");
    DECLabel2.setText("DEC (decimal Degrees)");
    DECLabel1.setText("DEC (DDD:MM:SS)");
    jLabel4.setText("Azimuth");
    jLabel5.setText("Zenith Angle");
    jLabel6.setText("Hour Angle");
    AzimuthTextField.setText("");
    ZenithAngleTextField.setText("");
    HourAngleTextField.setText("");
    ObjectNameTextField.setText("");
    RATextField2.setText("");
    RATextField1.setText("");
    DECTextField1.setText("");
    jLabel7.setText("Airmass");
    AirMassTextField.setText("");
    DECTextField2.setText("");
    TitleLabel.setText("Palomar Observatory: Hale 200 \" Telescope Simulator");

    SIMBADRadioButton.setToolTipText("");
    NEDRadioButton.setToolTipText("");

    jLabel1.setFont(new java.awt.Font("Dialog", 1, 14));
    jLabel2.setFont(new java.awt.Font("Dialog", 1, 14));
    jLabel3.setFont(new java.awt.Font("Dialog", 1, 14));
    ObjectNameLabel.setFont(new java.awt.Font("Dialog", 3, 16));
    SIMBADRadioButton.setFont(new java.awt.Font("Dialog", 1, 13));
    GetCoordinatesButton.setFont(new java.awt.Font("Dialog", 3, 12));
    ResetCoordinatesButton.setFont(new java.awt.Font("Dialog", 3, 12));
    NEDRadioButton.setFont(new java.awt.Font("Dialog", 1, 13));
    RALabel1.setFont(new java.awt.Font("Dialog", 1, 11));
    RALabel2.setFont(new java.awt.Font("Dialog", 1, 11));
    DECLabel2.setFont(new java.awt.Font("Dialog", 1, 11));
    DECLabel1.setFont(new java.awt.Font("Dialog", 1, 11));
    jLabel4.setFont(new java.awt.Font("Dialog", 1, 11));
    jLabel5.setFont(new java.awt.Font("Dialog", 1, 11));
    jLabel6.setFont(new java.awt.Font("Dialog", 1, 11));
    jLabel7.setFont(new java.awt.Font("Dialog", 1, 11));
    TitleLabel.setFont(new java.awt.Font("Dialog", 1, 14));

    jScrollPane1.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
    jScrollPane2.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
    jScrollPane3.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);

    jScrollPane1.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);
    jScrollPane2.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);
    jScrollPane3.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);

    jLabel1.setForeground(Color.black);
    jLabel2.setForeground(Color.black);
    jLabel3.setForeground(Color.black);
    ObjectNameLabel.setForeground(Color.white);
    TitleLabel.setForeground(Color.black);

    jPanel2.setBackground(Color.darkGray);

    RALabelsPanel.setBorder(BorderFactory.createLineBorder(Color.black));
    jPanel2.setBorder(BorderFactory.createRaisedBevelBorder());
    ResolutionSourcePanel.setBorder(BorderFactory.createRaisedBevelBorder());
    RAControlsPanel.setBorder(BorderFactory.createLineBorder(Color.black));
    DECLabelsPanel.setBorder(BorderFactory.createEtchedBorder());
    DECLabelsPanel.setBorder(BorderFactory.createLineBorder(Color.black));
    DECControlsPanel.setBorder(BorderFactory.createEtchedBorder());
    DECControlsPanel.setBorder(BorderFactory.createLineBorder(Color.black));
    jPanel3.setBorder(BorderFactory.createRaisedBevelBorder());
    jPanel4.setBorder(BorderFactory.createRaisedBevelBorder());

    jPanel1.setLayout(xYLayout1);
    jLabel1.setHorizontalAlignment(SwingConstants.CENTER);
    jLabel2.setHorizontalAlignment(SwingConstants.CENTER);
    jLabel3.setHorizontalAlignment(SwingConstants.CENTER);
    ObjectNameLabel.setHorizontalAlignment(SwingConstants.CENTER);
    NEDRadioButton.setHorizontalAlignment(SwingConstants.CENTER);
    SIMBADRadioButton.setHorizontalAlignment(SwingConstants.CENTER);
    TitleLabel.setHorizontalAlignment(SwingConstants.CENTER);

    DECLabelsPanel.setLayout(DECLabelsverticalFlowLayout);
    jPanel2.setLayout(xYLayout2);
    ResolutionSourcePanel.setLayout(ResolutionSourceflowLayout);
    RAControlsPanel.setLayout(RAControlsverticalFlowLayout);
    DECControlsPanel.setLayout(DECControlsverticalFlowLayout);
    jPanel3.setLayout(verticalFlowLayout2);
    jPanel4.setLayout(verticalFlowLayout3);
    RALabelsPanel.setLayout(verticalFlowLayout1);

    SIMBADRadioButton.setSelected(true);
    ResetCoordinatesButton.setAlignmentX((float) 0.0);

    ResolutionSourceflowLayout.setVgap(1);
    verticalFlowLayout1.setVgap(8);
    DECLabelsverticalFlowLayout.setVgap(8);
    verticalFlowLayout2.setVgap(10);

    menuFile.add(menuFileExit);
    menuHelp.add(menuHelpAbout);
    menuBar1.add(menuFile);
    menuBar1.add(menuHelp);

    RAControlsPanel.add(RATextField1,   null);
    RAControlsPanel.add(RATextField2,   null);
    DECControlsPanel.add(DECTextField1, null);
    DECControlsPanel.add(DECTextField2, null);
    DECLabelsPanel.add(DECLabel1,       null);
    DECLabelsPanel.add(DECLabel2,       null);
    jPanel3.add(jLabel4,                null);
    jPanel3.add(jLabel5,                null);
    jPanel3.add(jLabel6,                null);
    jPanel3.add(jLabel7,                null);
    jPanel4.add(AzimuthTextField,       null);
    jPanel4.add(ZenithAngleTextField,   null);
    jPanel4.add(HourAngleTextField,     null);

    jPanel1.add(jLabel1,                 new XYConstraints(14, 457, 709, -1));
    jPanel1.add(jScrollPane2,            new XYConstraints(14, 329, 707, 128));
    jPanel1.add(jLabel2,                 new XYConstraints(15, 306, 704, -1));
    jPanel1.add(jPanel2,                 new XYConstraints(11, 26, 702, 192));
    jPanel2.add(ObjectNameLabel,         new XYConstraints(6, 3, 106, 28));
    jPanel2.add(ObjectNameTextField,     new XYConstraints(113, 2, 200, 33));
    jPanel2.add(ResolutionSourcePanel,   new XYConstraints(315, 2, 158, 34));
    jPanel2.add(jPanel4,                 new XYConstraints(407, 43, 274, 123));
    jPanel2.add(DECLabelsPanel,          new XYConstraints(10, 114, 141, 70));
    jPanel2.add(DECControlsPanel,        new XYConstraints(151, 114, 164, 70));
    jPanel2.add(RAControlsPanel,         new XYConstraints(151, 41, 163, 70));
    jPanel2.add(jPanel3,                 new XYConstraints(316, 42, 90, 124));
    jPanel2.add(GetCoordinatesButton,    new XYConstraints(475, 3, 129, 33));
    jPanel2.add(ResetCoordinatesButton,  new XYConstraints(606, 4, 76, 32));
    jPanel1.add(jScrollPane1,            new XYConstraints(13, 229, 704, 74));
    jPanel1.add(jLabel3,                 new XYConstraints(10, 209, 705, -1));
    jPanel1.add(jScrollPane3,            new XYConstraints(13, 486, 710, 132));
    jPanel1.add(TitleLabel,                 new XYConstraints(11, 6, 705, -1));
    jPanel2.add(RALabelsPanel,           new XYConstraints(10, 40, 140, 70));

    ResolutionSourcePanel.add(SIMBADRadioButton,    null);
    ResolutionSourcePanel.add(NEDRadioButton,       null);
    RALabelsPanel.add(RALabel2,                     null);
    RALabelsPanel.add(RALabel1,                     null);
    ResolutionSourcePanel.add(SIMBADRadioButton,    null);
    ResolutionSourcePanel.add(NEDRadioButton,       null);
    RALabelsPanel.add(RALabel2,                     null);
    RALabelsPanel.add(RALabel1,                     null);
    RAControlsPanel.add(RATextField1,               null);
    RAControlsPanel.add(RATextField2,               null);
    DECLabelsPanel.add(DECLabel1,                   null);
    DECLabelsPanel.add(DECLabel2,                   null);
    DECControlsPanel.add(DECTextField1,             null);
    DECControlsPanel.add(DECTextField2,             null);
    jPanel4.add(AirMassTextField,                   null);

    jScrollPane2.getViewport().add(receiveTextArea, null);
    jScrollPane1.getViewport().add(errorTextArea,   null);
    jScrollPane3.getViewport().add(sendTextArea,    null);

    contentPane.add(statusBar, BorderLayout.SOUTH);
    contentPane.add(jPanel1,   BorderLayout.CENTER);

    this.setJMenuBar(menuBar1);
    GSResolutionSourceButtonGroup.add(SIMBADRadioButton);
    GSResolutionSourceButtonGroup.add(NEDRadioButton);
    setConnected(false);
    myServerMediator = new ServerMediator();
    SIMBADRadioButton.setSelected(true);
  }
/*================================================================================================
/         startServer() methods
/=================================================================================================*/
  public void startServer(){
     myServerSocket.setErrorLoggingTextArea(errorTextArea);
     myServerSocket.setSendLoggingTextArea(sendTextArea);
     myServerSocket.setReceiveLoggingTextArea(receiveTextArea);
     myServerSocket.startServer();
   }
/*================================================================================================
/         P200ServerSocket getP200ServerSocket() methods
/=================================================================================================*/
  public P200ServerSocket getServerSocket(){
    return myServerSocket;
  }
/*=============================================================================================
/         componentsTesting() constructor
/=============================================================================================*/
  public void componentsTesting(){
    myServerSocket.prepareResponse("WEATHER");
  }
/*================================================================================================
/          isConnected and setDVConnected methods
/=================================================================================================*/
  public void setConnected(boolean newDVConnected){
      isDVConnected = newDVConnected;
  }
  public boolean isConnected(){
     return isDVConnected;
  }
/*================================================================================================
/          fileExit_actionPerformed(ActionEvent e) Method
/=================================================================================================*/
  //File | Exit action performed
  public void fileExit_actionPerformed(ActionEvent e) {
    System.exit(0);
  }
/*================================================================================================
/      processWindowEvent(WindowEvent e)
/=================================================================================================*/
  //Overridden so we can exit when window is closed
  protected void processWindowEvent(WindowEvent e) {
    super.processWindowEvent(e);
    if (e.getID() == WindowEvent.WINDOW_CLOSING) {
      fileExit_actionPerformed(null);
    }
  }
/*================================================================================================
/      setRightAscension
/=================================================================================================*/
    public void setRightAscension(double rightAscension) {
      double  oldRightAscension = this.rightAscension;
      this.rightAscension = rightAscension;
      propertyChangeListeners.firePropertyChange("rightAscension", Double.valueOf(oldRightAscension),Double.valueOf(rightAscension));
    }
    public double getRightAscension() {
      return rightAscension;
    }
/*================================================================================================
/      setDeclination
/=================================================================================================*/
    public void setDeclination(double declination) {
      double  oldDeclination = this.declination;
      this.declination = declination;
      propertyChangeListeners.firePropertyChange("declination", Double.valueOf(oldDeclination), Double.valueOf(declination));
    }
    public double getDeclination() {
      return declination;
    }
/*=============================================================================================
/                      LickServerMediator Inner Class
/=============================================================================================*/
  public class ServerMediator {
   public ServerMediator(){
     myHTMIndexSearchClient.setResolutionSource(HTMIndexSearchClient.SIMBAD);
/*=================================================================================
/     Property Change Listener Initialization for the LickServerSocket
/=================================================================================*/
    myTelescopeObject.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
       public void propertyChange(java.beans.PropertyChangeEvent e) {
          myTelescopeObject_propertyChange(e);
       }
    });
/*===============================================================================================================
/    GetCoordinatesButton  Button ActionListener
/===============================================================================================================*/
     GetCoordinatesButton.addActionListener(new java.awt.event.ActionListener(){
         public void actionPerformed(java.awt.event.ActionEvent e){
                   try{
                     java.lang.String currentObjectName = ObjectNameTextField.getText();
                     myHTMIndexSearchClient.setObjectName(currentObjectName);
                     double[]  coordinateArray = myHTMIndexSearchClient.resolveObject(currentObjectName);
                     myHTMIndexSearchClient.setRA(coordinateArray[0]);
                     myHTMIndexSearchClient.setDEC(coordinateArray[1]);
                     setRightAscension(coordinateArray[0]);
                     setDeclination(coordinateArray[1]);
                     RATextField2.setValue(coordinateArray[0]);
                     DECTextField2.setValue(coordinateArray[1]);
                     Coordinates      configuredCoordinates = new Coordinates();
                                      configuredCoordinates.setSeparatorStyle(Coordinates.COLON_SEPARATOR_STYLE);
                                      configuredCoordinates.setRa(coordinateArray[0]);
                                      configuredCoordinates.setDec(coordinateArray[1]);
                     java.lang.String newRaString           = configuredCoordinates.raToString();
                     java.lang.String newDecString          = configuredCoordinates.decToString();
                     myTelescopeObject.setRightAscension(newRaString);
                     myTelescopeObject.setDeclination(newDecString);
                     myTelescopeObject.setRA(coordinateArray[0]);
                     myTelescopeObject.setDEC(coordinateArray[1]);
                     logMessage(newRaString);
                     logMessage(newDecString);
                     logMessage("RA String = " + newRaString + " " + "RA Double = " + coordinateArray[0]);
                     logMessage("Dec String = " + newDecString + " " + "Dec Double = " + coordinateArray[1]);
                   }
                   catch(Exception e2){
                       java.lang.String myErrorString = new java.lang.String();
                       myErrorString = "A problem occured while executing the Get Coordinates method: " + e2;
                       logMessage(myErrorString);
                         ObjectNameTextField.setText("");
                         RATextField1.setText("");
                         RATextField2.setValue(0.0);
                         DECTextField1.setText("");
                         DECTextField2.setValue(0.0);
                         myHTMIndexSearchClient.setRA(0.0);
                         myHTMIndexSearchClient.setDEC(0.0);
                   }
                 }
        });
/*===============================================================================================================
/    SIMBADRadioButton Action Listener
/===============================================================================================================*/
    SIMBADRadioButton.addActionListener(new java.awt.event.ActionListener(){
      public void actionPerformed(java.awt.event.ActionEvent e){
                   boolean myNewValue =  SIMBADRadioButton.isSelected();
                     try{
                         if(myNewValue){
                           myHTMIndexSearchClient.setResolutionSource(HTMIndexSearchClient.SIMBAD);
                         }
                     }
                     catch(Exception e2){
                         java.lang.String myErrorString = new java.lang.String();
                         myErrorString = "A problem occured while setting the Name Resolution Source. ";
                         logMessage(myErrorString + myNewValue);
                     }
                   }
      });
/*===============================================================================================================
/    NEDRadioButton Action Listener
/===============================================================================================================*/
    NEDRadioButton.addActionListener(new java.awt.event.ActionListener(){
       public void actionPerformed(java.awt.event.ActionEvent e){
                     boolean myNewValue =  NEDRadioButton.isSelected();
                       try{
                           if(myNewValue){
                             myHTMIndexSearchClient.setResolutionSource(HTMIndexSearchClient.NED);
                           }
                       }
                       catch(Exception e2){
                         java.lang.String myErrorString = new java.lang.String();
                         myErrorString = "A problem occured while setting the Name Resolution Source. ";
                         logMessage(myErrorString + myNewValue);
                       }
                   }
        });
/*===============================================================================================================
/     ObjectNameTextField ActionListener   -  Object Text Field
/===============================================================================================================*/
   ObjectNameTextField.addActionListener(new java.awt.event.ActionListener(){
      public void actionPerformed(java.awt.event.ActionEvent e){
        java.lang.String myNewString = new java.lang.String();
        myNewString =  ObjectNameTextField.getText();
          try{
            myHTMIndexSearchClient.setObjectName(myNewString);
          }
          catch(Exception e2){
            java.lang.String myErrorString = new java.lang.String();
            myErrorString = "A problem occured while setting the Object Name.  Attempted Value = ";
            logMessage(myErrorString + myNewString);
          }
        }
      });
  }
/*=================================================================================
/        Property Change Listener for the LickServerSocket Object
/=================================================================================*/
   public void myTelescopeObject_propertyChange(PropertyChangeEvent e)  {
      java.lang.String propertyName = e.getPropertyName();
          if(propertyName ==  "right_ascension"){
             java.lang.String myStringValue  = (java.lang.String)e.getNewValue();
             RATextField1.setText(myStringValue);
          }
          if(propertyName ==  "declination"){
             java.lang.String myStringValue  = (java.lang.String)e.getNewValue();
             DECTextField1.setText(myStringValue);
          }
          if(propertyName ==  "RA"){
             java.lang.Double myDoubleValue  = (java.lang.Double)e.getNewValue();
             RATextField2.setValue(myDoubleValue.doubleValue());
          }
          if(propertyName ==  "DEC"){
            java.lang.Double myDoubleValue  = (java.lang.Double)e.getNewValue();
            DECTextField2.setValue(myDoubleValue.doubleValue());
          }
   }// end of propertyChange Listener
/*================================================================================================
/         Summary of Property Change Events propagated by the TelescopeObject Class
/=================================================================================================*/
/*  propertyChangeListeners.firePropertyChange("right_ascension", oldright_ascension, right_ascension);
    propertyChangeListeners.firePropertyChange("declination", oldDeclination, declination);
    propertyChangeListeners.firePropertyChange("equinox_string", oldequinox_string, equinox_string);
    propertyChangeListeners.firePropertyChange("hourangle_string", oldhourangle_string, hourangle_string);
    propertyChangeListeners.firePropertyChange("airmass_string", oldairmass_string, airmass_string);
    propertyChangeListeners.firePropertyChange("UTCSTART", oldUTCSTART, newUTCSTART);
    propertyChangeListeners.firePropertyChange("UTCEND", oldUTCEND, newUTCEND);
    propertyChangeListeners.firePropertyChange("DATE_OBS", oldDATE_OBS, newDATE_OBS);
    propertyChangeListeners.firePropertyChange("DATE", oldDATE, newDATE);
    propertyChangeListeners.firePropertyChange("right_ascension_offset", oldright_ascension_offset, right_ascension_offset);
    propertyChangeListeners.firePropertyChange("declination_offset", olddeclination_offset, declination_offset);
    propertyChangeListeners.firePropertyChange("right_ascension_track_rate", oldright_ascension_track_rate, right_ascension_track_rate);
    propertyChangeListeners.firePropertyChange("declination_track_rate", olddeclination_track_rate, declination_track_rate);
    propertyChangeListeners.firePropertyChange("telescope_focus", oldtelescope_focus, telescope_focus);
    propertyChangeListeners.firePropertyChange("telescope_focus", oldtelescope_tube_length, telescope_tube_length);
    propertyChangeListeners.firePropertyChange("cass_ring_angle", oldcass_ring_angle, cass_ring_angle);
    propertyChangeListeners.firePropertyChange("telescope_ID", oldtelescope_ID, telescope_ID);
    propertyChangeListeners.firePropertyChange("RA", new Double(oldRA), new Double(newRA));
    propertyChangeListeners.firePropertyChange("DEC", new Double(oldDEC), new Double(newDEC));
    propertyChangeListeners.firePropertyChange("EQUINOX", new Double(oldEQUINOX), new Double(newEQUINOX));
    propertyChangeListeners.firePropertyChange("HA", new Double(oldHA), new Double(newHA));
    propertyChangeListeners.firePropertyChange("AIRMASS", new Double(oldAIRMASS), new Double(newAIRMASS));
    propertyChangeListeners.firePropertyChange("RA_OFFSET", new Integer(oldRA_OFFSET), new Integer(newRA_OFFSET));
    propertyChangeListeners.firePropertyChange("DEC_OFFSET", new Integer(oldDEC_OFFSET), new Integer(newDEC_OFFSET));
    propertyChangeListeners.firePropertyChange("RA_TRACK_RATE", new Integer(oldRA_TRACK_RATE), new Integer(newRA_TRACK_RATE));
    propertyChangeListeners.firePropertyChange("DEC_TRACK_RATE", new Integer(oldDEC_TRACK_RATE), new Integer(newDEC_TRACK_RATE));
    propertyChangeListeners.firePropertyChange("RA_GUIDE_RATE", new Integer(oldRA_GUIDE_RATE), new Integer(newRA_GUIDE_RATE));
    propertyChangeListeners.firePropertyChange("DEC_GUIDE_RATE", new Integer(oldDEC_GUIDE_RATE), new Integer(newDEC_GUIDE_RATE));
    propertyChangeListeners.firePropertyChange("RA_SET_RATE", new Integer(oldRA_SET_RATE), new Integer(newRA_SET_RATE));
    propertyChangeListeners.firePropertyChange("DEC_SET_RATE", new Integer(oldDEC_SET_RATE), new Integer(newDEC_SET_RATE));
    propertyChangeListeners.firePropertyChange("FOCUS", new Double(oldFOCUS), new Double(newFOCUS));
    propertyChangeListeners.firePropertyChange("TUBE_LENGTH", new Double(oldTUBE_LENGTH), new Double(newTUBE_LENGTH));
    propertyChangeListeners.firePropertyChange("CASS_RING_ANGLE", new Double(oldCASS_RING_ANGLE), new Double(newCASS_RING_ANGLE));
    propertyChangeListeners.firePropertyChange("LST", new Double(oldLST), new Double(LST));
    propertyChangeListeners.firePropertyChange("azimuth", new Double(oldAZIMUTH), new Double(AZIMUTH));
    propertyChangeListeners.firePropertyChange("zenithAngle", new Double(oldZENITH_ANGLE), new Double(ZENITH_ANGLE));
    propertyChangeListeners.firePropertyChange("DOME_AZIMUTH", new Double(oldDOME_AZIMUTH), new Double(DOME_AZIMUTH));
    propertyChangeListeners.firePropertyChange("WINDSCREEN_POSITION", new Double(oldWINDSCREEN_POSITION), new Double(WINDSCREEN_POSITION));
    propertyChangeListeners.firePropertyChange("DOME_SHUTTERS", new Integer(oldDOME_SHUTTERS), new Integer(DOME_SHUTTERS));
    propertyChangeListeners.firePropertyChange("INSTRUMENT_POSITION", new Integer(oldINSTRUMENT_POSITION), new Integer(INSTRUMENT_POSITION));
    propertyChangeListeners.firePropertyChange("INSTRUMENT", oldINSTRUMENT, INSTRUMENT);
    propertyChangeListeners.firePropertyChange("PUMPS", oldPUMPS, PUMPS);
    propertyChangeListeners.firePropertyChange("connected", new Boolean(oldConnected), new Boolean(connected));
    propertyChangeListeners.firePropertyChange("moving", new Boolean(oldMoving), new Boolean(newMoving));
    propertyChangeListeners.firePropertyChange("updating", new Boolean(oldUpdating), new Boolean(newUpdating));
    propertyChangeListeners.firePropertyChange("lock", new Boolean(oldLock), new Boolean(newLock));
    propertyChangeListeners.firePropertyChange("tracking", new Boolean(oldTracking), new Boolean(newTracking));
    propertyChangeListeners.firePropertyChange("progress", new Integer(oldProgress), new Integer(progress));
    propertyChangeListeners.firePropertyChange("caption", oldCaption, newCaption);
*/
/*================================================================================================
/        java.lang.String doubleToRAString(double newDoubleRA)
/=================================================================================================*/
         public java.lang.String doubleToRAString(double newDoubleRA){
           java.lang.String myRAString = new java.lang.String();
           Coordinates      configuredCoordinates = new Coordinates();
                            configuredCoordinates.setSeparatorStyle(Coordinates.COLON_SEPARATOR_STYLE);
                            configuredCoordinates.setRa(newDoubleRA);
                  myRAString = configuredCoordinates.raToString();
           return myRAString;
         }
/*================================================================================================
/        java.lang.String doubleToDECString(double newDoubleDEC)
/=================================================================================================*/
           public java.lang.String doubleToDECString(double newDoubleDEC){
             java.lang.String myDECString = new java.lang.String();
             Coordinates      configuredCoordinates = new Coordinates();
                              configuredCoordinates.setSeparatorStyle(Coordinates.COLON_SEPARATOR_STYLE);
                              configuredCoordinates.setDec(newDoubleDEC);
                   myDECString = configuredCoordinates.decToString();
             return myDECString;
           }
/*=========================================================================================================
/     void logMessage(java.lang.String newMessage)
/=========================================================================================================*/
   public void logMessage(java.lang.String newMessage){
     System.out.println(newMessage);
   }
}//// End of the ServerMediator Inner Class
}
