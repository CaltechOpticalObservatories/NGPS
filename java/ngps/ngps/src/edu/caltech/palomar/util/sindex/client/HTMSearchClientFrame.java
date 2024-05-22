package edu.caltech.palomar.util.sindex.client;
 
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import com.borland.jbcl.layout.*;
import edu.caltech.palomar.util.gui.DecTextField;
import edu.caltech.palomar.util.gui.RaTextField;

import edu.caltech.palomar.util.gui.DoubleTextField;
import edu.caltech.palomar.util.gui.IntTextField;
import edu.caltech.palomar.util.gui.CoordSystemEquinox;
import edu.caltech.palomar.util.gui.DoubleTextField;
import edu.caltech.palomar.util.sindex.HTMTrixelTableModel;
import edu.caltech.palomar.util.sindex.TriangleDataModel;
import edu.jhu.htm.core.Convex;
import edu.jhu.htm.core.HTMindexImp;
import edu.jhu.htm.core.HTMrangeIterator;
import edu.jhu.htm.core.HTMrange;
import edu.jhu.htm.core.Vector3d;
import edu.caltech.palomar.util.sindex.SpatialSearchAreaObject;
import jsky.coords.DMS;
import jsky.coords.HMS;
//=== File Prolog =============================================================
//	This code was developed by UCLA, Department of Physics and Astronomy,
//	for the Stratospheric Observatory for Infrared Astronomy (SOFIA) project.
//
//--- Contents ----------------------------------------------------------------
//	HTMSearchClientFrame  - This is a prototype search client application frame
//      for retrieving observation files from SOFIA based on HTM Index coordinates..
//--- Description -------------------------------------------------------------
//
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
//
//      07/07/04        J. Milburn
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
//	In no event shall UCLA be liable for any damages, including, but not
//	limited to direct, indirect, special or consequential damages, arising out
//	of, resulting from, or in any way connected with this software, whether or
//	not based upon warranty, contract, tort or otherwise, whether or not
//	injury was sustained by persons or property or otherwise, and whether or
//	not loss was sustained from or arose out of the results of, or use of,
//	their software or services provided hereunder.
//
//=== End File Prolog =========================================================
/*================================================================================================
/      class HTMSearchClientFrame
/=================================================================================================*/
public class HTMSearchClientFrame extends JFrame {
  JPanel contentPane;
  JPanel RALabelsPanel          = new JPanel();
  JPanel DECLabelsPanel         = new JPanel();
  JPanel RAControlsPanel        = new JPanel();
  JPanel DECControlsPanel       = new JPanel();
  JPanel QueryAreaControlsPanel = new JPanel();
  JPanel TriangleCountPanel     = new JPanel();
  JPanel QueryControlsPanel     = new JPanel();
  JPanel MainPanel              = new JPanel();

  JLabel statusBar               = new JLabel();
  JLabel TitleLabel              = new JLabel();
  JLabel ObjectNameLabel         = new JLabel();
  JLabel GetCoordinatesSourceLabel = new JLabel();
  JLabel RALabel1                = new JLabel();
  JLabel RALabel2                = new JLabel();
  JLabel DECLabel1               = new JLabel();
  JLabel DECLabel2               = new JLabel();
  JLabel SearchHeightLabel       = new JLabel();
  JLabel QueryLimitsLabel        = new JLabel();
  JLabel EquinoxLabel            = new JLabel();
  JLabel SearchWidthLabel        = new JLabel();
  JLabel HTMCoordinatesLabel     = new JLabel();
  JLabel SOFIAObservationsLabel  = new JLabel();
  JLabel TriangleRangeLabel      = new JLabel();
  JLabel TriangleNumberLabel     = new JLabel();
  JLabel MessageLogLabel         = new JLabel();
  JLabel QueryExecutionTimeLabel = new JLabel();

  JTextField         ObjectNameTextField    = new JTextField();
//  RaTextField        RATextField1           = new RaTextField(9,new RADECCoordinateSystem(CoordSystemEquinox.EQUATORIAL,CoordSystemEquinox.J2000,false));
//  DecTextField       DECTextField1          = new DecTextField(9,new RADECCoordinateSystem(CoordSystemEquinox.EQUATORIAL,CoordSystemEquinox.J2000,false));
  DoubleTextField    RATextField2           = new DoubleTextField(9,0.0,360.0,0.0,6,true);
  DoubleTextField    DECTextField2          = new DoubleTextField(9,-90.0,90.0,0.0,7,true);
  DoubleTextField    SearchHeightTextField  = new DoubleTextField(6,60.0,3600.0,1200.0,2,true);
  DoubleTextField    SearchWidthTextField   = new DoubleTextField(6,60.0,3600.0,1200.0,2,true);
  IntTextField       HTMLevelTextField      = new IntTextField(2,1,25,8,false);
  JTextField         RATextField1           = new JTextField();
//  JTextField RATextField2                = new JTextField();
  JTextField         DECTextField1          = new JTextField();
//  JTextField DECTextField2               = new JTextField();
//  JTextField SearchHeightTextField       = new JTextField();
//  JTextField SearchWidthTextField        = new JTextField();
  JTextField TriangleRangesTextField     = new JTextField();
  JTextField TriangleNumberTextField     = new JTextField();
  JTextField QueryExecutionTimeTextField = new JTextField();

  VerticalFlowLayout RALabelsverticalFlowLayout    = new VerticalFlowLayout();
  VerticalFlowLayout DECLabelsverticalFlowLayout   = new VerticalFlowLayout();
  VerticalFlowLayout RAControlsverticalFlowLayout  = new VerticalFlowLayout();
  VerticalFlowLayout DECControlsverticalFlowLayout = new VerticalFlowLayout();

  JRadioButton SIMBADRadioButton = new JRadioButton();
  JRadioButton NEDRadioButton    = new JRadioButton();

  JButton SubmitQueryButton      = new JButton();
  JButton AbortQueryButton       = new JButton();
  JButton GetCoordinatesButton   = new JButton();
  JButton ResetCoordinatesButton = new JButton();

  JScrollPane TrianglesScrollPane      = new JScrollPane();
  JScrollPane ObservationsScrollPane   = new JScrollPane();
  JScrollPane SystemMessagesScrollPane = new JScrollPane();

  XYLayout MainxYLayout              = new XYLayout();
  XYLayout QueryAreaControlsxYLayout = new XYLayout();
  XYLayout TriangleCountxYLayout     = new XYLayout();
  XYLayout QueryControlsxYLayout     = new XYLayout();

  BorderLayout borderLayout1 = new BorderLayout();

  JComboBox EquinoxComboBox = new JComboBox();

  JTable TrianglesTable    = new JTable();
  JTable ObservationsTable = new JTable();

  JTextPane   SystemMessagesTextPane      = new JTextPane();
  ButtonGroup resolutionSourceButtonGroup = new ButtonGroup();
  HTMIndexSearchClient myHTMIndexSearchClient = new HTMIndexSearchClient();
  public static java.lang.String  B1950 ="B1950";
  public static java.lang.String  J2000 ="J2000";
  JButton GetHTMTrianglesButton = new JButton();
  JLabel EquinoxLabel1 = new JLabel();
/*================================================================================================
/      constructor HTMSearchClientFrame
/=================================================================================================*/
  //Construct the frame
  public HTMSearchClientFrame() {
    enableEvents(AWTEvent.WINDOW_EVENT_MASK);
    try {
      jbInit();
    }
    catch(Exception e) {
      e.printStackTrace();
    }
  }
/*================================================================================================
/     jbInit() initialization method
/=================================================================================================*/
  //Component initialization
  private void jbInit() throws Exception  {
    contentPane = (JPanel) this.getContentPane();
    contentPane.setLayout(borderLayout1);
    this.setSize(new Dimension(644, 925));
    this.setTitle(                    "SOFIA Spatial Query Test Application");
    statusBar.setText(                " ");
    TitleLabel.setText(               "Get Object or Image Coordinates");
    ObjectNameTextField.setText(      "");
    ObjectNameLabel.setText(          "Object Name");
    GetCoordinatesSourceLabel.setText("Get Coordinates from:");
    GetCoordinatesButton.setText(   "Get Coordinates");
    ResetCoordinatesButton.setText( "Reset");
    SIMBADRadioButton.setText(      "SIMBAD");
    NEDRadioButton.setText(         "NED");
    RALabel1.setText(               "RA (HH:MM:SS)");
    RALabel2.setText(               "RA (decimal Hours)");
    DECLabel1.setText(              "DEC (DDD:MM:SS)");
    DECLabel2.setText(              "DEC (decimal Degrees)");
    RATextField1.setText(           "");
    RATextField2.setText(           "");
    DECTextField1.setText(          "");
    SearchHeightLabel.setText(      "Height (arcseconds)");
    SearchHeightTextField.setText(  "");
    SearchWidthLabel.setText(       "Width (arcseconds)");
    SearchWidthTextField.setText(   "");
    QueryLimitsLabel.setText(       "Search Limits: minimum = 60 arcseconds, maximum = 3600 arcseconds");
    EquinoxLabel.setText(           "Equinox");
    DECTextField2.setText(          "");
    TriangleRangeLabel.setText(     "Number of Triangles Ranges in Search Area");
    TriangleRangesTextField.setText("");
    TriangleNumberLabel.setText(    "Number of Triangles:");
    TriangleNumberTextField.setText("");
    HTMCoordinatesLabel.setText(    "HTM Triangles and Coordinates within Search Area");
    SOFIAObservationsLabel.setText( "SOFIA Observations within Search Area");
    QueryExecutionTimeLabel.setText("Query Execution Time");
    QueryExecutionTimeTextField.setText("");
    SubmitQueryButton.setText(      "Submit Query");
    AbortQueryButton.setText(       "Abort Query");
    MessageLogLabel.setText(        "Constructed SQL Statement");
    SystemMessagesTextPane.setText( "");
    GetHTMTrianglesButton.setText(  "Get List of HTM Triangles");

    TitleLabel.setFont(               new java.awt.Font("Dialog", 3, 16));
    ObjectNameLabel.setFont(          new java.awt.Font("Dialog", 3, 16));
    GetCoordinatesSourceLabel.setFont(new java.awt.Font("Dialog", 3, 13));
    SIMBADRadioButton.setFont(        new java.awt.Font("Dialog", 1, 13));
    NEDRadioButton.setFont(           new java.awt.Font("Dialog", 1, 13));
    GetCoordinatesButton.setFont(     new java.awt.Font("Dialog", 3, 12));
    ResetCoordinatesButton.setFont(   new java.awt.Font("Dialog", 3, 12));
    RALabel1.setFont(                 new java.awt.Font("Dialog", 1, 11));
    RALabel2.setFont(                 new java.awt.Font("Dialog", 1, 11));
    DECLabel1.setFont(                new java.awt.Font("Dialog", 1, 11));
    DECLabel2.setFont(                new java.awt.Font("Dialog", 1, 11));
    SearchHeightLabel.setFont(        new java.awt.Font("Dialog", 3, 11));
    SearchWidthLabel.setFont(         new java.awt.Font("Dialog", 3, 11));
    QueryLimitsLabel.setFont(         new java.awt.Font("Dialog", 3, 11));
    EquinoxLabel.setFont(             new java.awt.Font("Dialog", 3, 12));
    GetHTMTrianglesButton.setFont(    new java.awt.Font("Dialog", 3, 12));
    TriangleRangeLabel.setFont(       new java.awt.Font("Dialog", 3, 13));
    TriangleNumberLabel.setFont(      new java.awt.Font("Dialog", 3, 13));
    HTMCoordinatesLabel.setFont(      new java.awt.Font("Dialog", 3, 15));
    SOFIAObservationsLabel.setFont(   new java.awt.Font("Dialog", 3, 15));
    QueryExecutionTimeLabel.setFont(  new java.awt.Font("Dialog", 3, 13));
    SubmitQueryButton.setFont(        new java.awt.Font("Dialog", 3, 12));
    AbortQueryButton.setFont(         new java.awt.Font("Dialog", 3, 12));
    MessageLogLabel.setFont(          new java.awt.Font("Dialog", 3, 15));

    MainPanel.setLayout(MainxYLayout);

    TitleLabel.setHorizontalAlignment(             SwingConstants.CENTER);
    ObjectNameLabel.setHorizontalAlignment(        SwingConstants.CENTER);
    SIMBADRadioButton.setHorizontalAlignment(      SwingConstants.CENTER);
    NEDRadioButton.setHorizontalAlignment(         SwingConstants.CENTER);
    HTMCoordinatesLabel.setHorizontalAlignment(    SwingConstants.CENTER);
    SOFIAObservationsLabel.setHorizontalAlignment( SwingConstants.CENTER);
    MessageLogLabel.setHorizontalAlignment(        SwingConstants.CENTER);

    SIMBADRadioButton.setToolTipText("");
    NEDRadioButton.setToolTipText("");

    RALabelsPanel.setBorder(            BorderFactory.createEtchedBorder());
    DECLabelsPanel.setBorder(           BorderFactory.createEtchedBorder());
    RAControlsPanel.setBorder(          BorderFactory.createEtchedBorder());
    DECControlsPanel.setBorder(         BorderFactory.createEtchedBorder());
    QueryAreaControlsPanel.setBorder(   BorderFactory.createEtchedBorder());
    TriangleCountPanel.setBorder(       BorderFactory.createEtchedBorder());
    QueryControlsPanel.setBorder(       BorderFactory.createEtchedBorder());

    DECControlsPanel.setLayout(         DECControlsverticalFlowLayout);
    QueryAreaControlsPanel.setLayout(   QueryAreaControlsxYLayout);
    TriangleCountPanel.setLayout(       TriangleCountxYLayout);
    QueryControlsPanel.setLayout(       QueryControlsxYLayout);

    RALabelsPanel.setLayout(            RALabelsverticalFlowLayout);
    DECLabelsPanel.setLayout(           DECLabelsverticalFlowLayout);
    RAControlsPanel.setLayout(          RAControlsverticalFlowLayout);

    RALabelsverticalFlowLayout.setVgap(7);
    DECLabelsverticalFlowLayout.setVgap(7);

    TrianglesScrollPane.setVerticalScrollBarPolicy(       JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
    ObservationsScrollPane.setVerticalScrollBarPolicy(    JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
    SystemMessagesScrollPane.setVerticalScrollBarPolicy(  JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);

    TrianglesScrollPane.setHorizontalScrollBarPolicy(     JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);
    ObservationsScrollPane.setHorizontalScrollBarPolicy(  JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);
    SystemMessagesScrollPane.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);

    MainPanel.setBackground(Color.lightGray);

    HTMLevelTextField.setText("");
    EquinoxLabel1.setFont(new java.awt.Font("Dialog", 1, 11));
    EquinoxLabel1.setText("HTMLevel");
    resolutionSourceButtonGroup.add(SIMBADRadioButton);
    resolutionSourceButtonGroup.add(NEDRadioButton);

    MainPanel.add(TitleLabel,                             new XYConstraints(  9,   5, 614,  28));
    MainPanel.add(ObjectNameTextField,                    new XYConstraints(129,  41, 243,  25));
    MainPanel.add(ObjectNameLabel,                        new XYConstraints( 16,  40, 113,  28));
    MainPanel.add(TriangleCountPanel,                      new XYConstraints(14, 217, 611, 38));
    TriangleCountPanel.add(TriangleRangeLabel,            new XYConstraints(  5,   8, 277,  -1));
    TriangleCountPanel.add(TriangleRangesTextField,       new XYConstraints(282,   7,  86,  -1));
    TriangleCountPanel.add(TriangleNumberLabel,            new XYConstraints(370, 8, 137, -1));
    TriangleCountPanel.add(TriangleNumberTextField,       new XYConstraints(508,   7,  86,  -1));
    MainPanel.add(TrianglesScrollPane,                     new XYConstraints(19, 286, 607, 224));
    MainPanel.add(HTMCoordinatesLabel,                    new XYConstraints( 15, 255, 603,  28));
    MainPanel.add(SOFIAObservationsLabel,                 new XYConstraints( 16, 512, 603,  28));
    MainPanel.add(ObservationsScrollPane,                 new XYConstraints( 21, 538, 601, 153));
    MainPanel.add(QueryControlsPanel,                     new XYConstraints( 18, 695, 605,  38));
    QueryControlsPanel.add(QueryExecutionTimeTextField,   new XYConstraints(457,   5, 138,  -1));
    QueryControlsPanel.add(SubmitQueryButton,             new XYConstraints(  4,   2, 151,  28));
    QueryControlsPanel.add(AbortQueryButton,              new XYConstraints(158,   3, 151,  28));
    QueryControlsPanel.add(QueryExecutionTimeLabel,       new XYConstraints(311,   7, 146,  -1));
    MainPanel.add(SystemMessagesScrollPane,               new XYConstraints( 19, 758, 604, 104));
    MainPanel.add(MessageLogLabel,                        new XYConstraints( 19, 731, 603,  28));
    MainPanel.add(QueryLimitsLabel,                       new XYConstraints( 15, 198, 396,  -1));
    MainPanel.add(GetCoordinatesButton,                    new XYConstraints(375, 40, 151, 28));
    MainPanel.add(ResetCoordinatesButton,                  new XYConstraints(528, 39, 96, 28));
    MainPanel.add(DECLabelsPanel,                         new XYConstraints(311, 100, 150,  61));
    MainPanel.add(DECControlsPanel,                        new XYConstraints(462, 99, 164, 62));
    QueryAreaControlsPanel.add(SearchHeightLabel,         new XYConstraints(  7,   5, 122,  -1));
    QueryAreaControlsPanel.add(SearchHeightTextField,     new XYConstraints(122,   3,  86,  -1));
    QueryAreaControlsPanel.add(SearchWidthLabel,          new XYConstraints(214,   6, 114,  -1));
    QueryAreaControlsPanel.add(SearchWidthTextField,      new XYConstraints(327,   5,  86,  -1));
    QueryAreaControlsPanel.add(EquinoxLabel,              new XYConstraints(419,   7,  61,  -1));
    QueryAreaControlsPanel.add(EquinoxComboBox,           new XYConstraints(490,   5, 105,  20));
    MainPanel.add(QueryAreaControlsPanel,                  new XYConstraints(13, 163, 613, 34));
    MainPanel.add(RALabelsPanel,                          new XYConstraints( 13, 100, 136,  62));
    MainPanel.add(RAControlsPanel,                        new XYConstraints(151, 100, 158,  62));
    MainPanel.add(GetCoordinatesSourceLabel,                       new XYConstraints(12, 73, 142, -1));
    MainPanel.add(SIMBADRadioButton,             new XYConstraints(154, 70, 98, 25));
    MainPanel.add(NEDRadioButton,           new XYConstraints(256, 69, 77, 26));
    MainPanel.add(GetHTMTrianglesButton,              new XYConstraints(337, 69, 181, 28));

    RAControlsPanel.add(RATextField1,   null);
    RAControlsPanel.add(RATextField2,   null);
    DECLabelsPanel.add(DECLabel1,       null);
    DECLabelsPanel.add(DECLabel2,       null);
    DECControlsPanel.add(DECTextField1, null);
    DECControlsPanel.add(DECTextField2, null);
    RALabelsPanel.add(RALabel1,         null);
    RALabelsPanel.add(RALabel2,         null);

    TrianglesScrollPane.getViewport().add(      TrianglesTable,         null);
    ObservationsScrollPane.getViewport().add(   ObservationsTable,      null);
    SystemMessagesScrollPane.getViewport().add( SystemMessagesTextPane, null);
    contentPane.add(statusBar,                  BorderLayout.SOUTH);
    contentPane.add(MainPanel,                  BorderLayout.CENTER);
    MainPanel.add(HTMLevelTextField,     new XYConstraints(576, 71, 49, -1));
    MainPanel.add(EquinoxLabel1,      new XYConstraints(519, 72, 60, 20));

    HTMSearchClientMediator myHTMSearchClientMediator = new HTMSearchClientMediator();
  }
/*================================================================================================
/    protected void processWindowEvent(WindowEvent e)
/=================================================================================================*/
  //Overridden so we can exit when window is closed
  protected void processWindowEvent(WindowEvent e) {
    super.processWindowEvent(e);
    if (e.getID() == WindowEvent.WINDOW_CLOSING) {
      System.exit(0);
    }
  }
/*=========================================================================================================
/     void logMessage(java.lang.String newMessage)
/=========================================================================================================*/
     public void logMessage(java.lang.String newMessage){
          System.out.println(newMessage);
     }
/*================================================================================================
/    class HTMSearchClientMediator
/=================================================================================================*/
  public class HTMSearchClientMediator  {
/*================================================================================================
/      constructor HTMSearchClientMediator
/=================================================================================================*/
   public HTMSearchClientMediator() {
        try {
          jbInitMediator();
        }
        catch(Exception e) {
          e.printStackTrace();
        }
   }
/*================================================================================================
/      void jbInitMediator()
/=================================================================================================*/
   public void jbInitMediator(){
     HTMLevelTextField.setValue(8);
     EquinoxComboBox.addItem(J2000);
     EquinoxComboBox.addItem(B1950);
     EquinoxComboBox.setSelectedIndex(0);
     myHTMIndexSearchClient.setResolutionSource(HTMIndexSearchClient.SIMBAD);
     SIMBADRadioButton.setSelected(true);
     SearchWidthTextField.setValue(SpatialSearchAreaObject.DEFAULT_SEARCH_WIDTH);
     SearchHeightTextField.setValue(SpatialSearchAreaObject.DEFAULT_SEARCH_HEIGHT);
     TrianglesTable.setModel(myHTMIndexSearchClient.getSpatialSearchAreaObject().getTableModel());
     TrianglesTable.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);
     TrianglesTable.getColumnModel().getColumn(0).setPreferredWidth(90);
     TrianglesTable.getColumnModel().getColumn(1).setPreferredWidth(100);
     TrianglesTable.getColumnModel().getColumn(2).setPreferredWidth(120);
     TrianglesTable.getColumnModel().getColumn(3).setPreferredWidth(120);
     TrianglesTable.getColumnModel().getColumn(4).setPreferredWidth(120);
     TrianglesTable.getColumnModel().getColumn(5).setPreferredWidth(100);
     TrianglesTable.getColumnModel().getColumn(6).setPreferredWidth(70);
     TrianglesTable.getColumnModel().getColumn(7).setPreferredWidth(70);
     TrianglesTable.getColumnModel().getColumn(8).setPreferredWidth(70);
     TrianglesTable.getColumnModel().getColumn(9).setPreferredWidth(110);
     SystemMessagesTextPane.setDocument(myHTMIndexSearchClient.getSQLDisplayModel().getCommentDocument());
/*===============================================================================================================
/    RATextField1 Action Listener
/===============================================================================================================*/
  RATextField1.addActionListener(new java.awt.event.ActionListener(){
  public void actionPerformed(java.awt.event.ActionEvent e){
       java.lang.String myNewValue =  RATextField1.getText();
         try{
           HMS    myHMS    = new HMS(myNewValue);
           double myDouble = myHMS.getVal();
           myHTMIndexSearchClient.setRA(myDouble);
         }
         catch(Exception e2){
           java.lang.String myErrorString = new java.lang.String();
           myErrorString = "A problem occured while setting the Right Ascension.  Attempted Value = ";
           logMessage(myErrorString + myNewValue);
         }
       }
  });
/*===============================================================================================================
/    RATextField2 Action Listener
/===============================================================================================================*/
     RATextField2.addActionListener(new java.awt.event.ActionListener(){
     public void actionPerformed(java.awt.event.ActionEvent e){
              double myNewValue =  RATextField2.getValidatedValue();
              try{
                  myHTMIndexSearchClient.setRA(myNewValue);
                 }
             catch(Exception e2){
                   java.lang.String myErrorString = new java.lang.String();
                   myErrorString = "A problem occured while setting the Right Ascension.  Attempted Value = ";
                  logMessage(myErrorString + myNewValue);
                 }
            }
      });
/*===============================================================================================================
/    DECTextField1 Action Listener
/===============================================================================================================*/
  DECTextField1.addActionListener(new java.awt.event.ActionListener(){
    public void actionPerformed(java.awt.event.ActionEvent e){
    java.lang.String myNewValue =  DECTextField1.getText();
      try{
        DMS    myDMS    = new DMS(myNewValue);
        double myDouble = myDMS.getVal();
        myHTMIndexSearchClient.setDEC(myDouble);
      }
      catch(Exception e2){
        java.lang.String myErrorString = new java.lang.String();
        myErrorString = "A problem occured while setting the Declination.  Attempted Value = ";
        logMessage(myErrorString + myNewValue);
      }
    }
  });
/*===============================================================================================================
/    DECTextField2 Action Listener
/===============================================================================================================*/
    DECTextField2.addActionListener(new java.awt.event.ActionListener(){
      public void actionPerformed(java.awt.event.ActionEvent e){
      double myNewValue =  DECTextField2.getValidatedValue();
        try{
          myHTMIndexSearchClient.setDEC(myNewValue);
        }
        catch(Exception e2){
          java.lang.String myErrorString = new java.lang.String();
          myErrorString = "A problem occured while setting the Declination.  Attempted Value = ";
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
/*===============================================================================================================
/    SearchWidthTextField Action Listener
/===============================================================================================================*/
   SearchWidthTextField.addActionListener(new java.awt.event.ActionListener(){
         public void actionPerformed(java.awt.event.ActionEvent e){
         double myNewValue =  SearchWidthTextField.getValidatedValue();
           try{
             myHTMIndexSearchClient.getSpatialSearchAreaObject().setSearchWidth(myNewValue);
           }
           catch(Exception e2){
             java.lang.String myErrorString = new java.lang.String();
             myErrorString = "A problem occured while setting the Search Area Width. ";
             logMessage(myErrorString + myNewValue);
           }
         }
       });
/*===============================================================================================================
/    SearchHeightTextField Action Listener
/===============================================================================================================*/
  SearchHeightTextField.addActionListener(new java.awt.event.ActionListener(){
     public void actionPerformed(java.awt.event.ActionEvent e){
       double myNewValue =  SearchHeightTextField.getValidatedValue();
         try{
             myHTMIndexSearchClient.getSpatialSearchAreaObject().setSearchHeight(myNewValue);
         }
         catch(Exception e2){
             java.lang.String myErrorString = new java.lang.String();
             myErrorString = "A problem occured while setting the Search Area Height. ";
             logMessage(myErrorString + myNewValue);
         }
     }
   });
/*===============================================================================================================
/    HTMLevelTextField Action Listener
/===============================================================================================================*/
     HTMLevelTextField.addActionListener(new java.awt.event.ActionListener(){
        public void actionPerformed(java.awt.event.ActionEvent e){
          int myNewValue =  HTMLevelTextField.getValidatedValue();
            try{
                myHTMIndexSearchClient.getSpatialSearchAreaObject().setHTMIndexDepth(myNewValue);
            }
            catch(Exception e2){
                java.lang.String myErrorString = new java.lang.String();
                myErrorString = "A problem occured while setting the HTM Index Depth. ";
                logMessage(myErrorString + myNewValue);
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
                myErrorString = "A problem occured while setting the Search Area Height. ";
                logMessage(myErrorString + myNewValue);
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
/     EquinoxComboBox Item Listener
/===============================================================================================================*/
        EquinoxComboBox.addItemListener(new java.awt.event.ItemListener(){
        public void itemStateChanged(java.awt.event.ItemEvent e){
           int selectedIndex   =  EquinoxComboBox.getSelectedIndex();
           java.lang.String equinoxString = ((java.lang.String)(EquinoxComboBox.getSelectedItem()));
           try{
             if(equinoxString.regionMatches(true,0,J2000,0,5)){
               myHTMIndexSearchClient.setEquinox(2000);
             }
             if(equinoxString.regionMatches(true,0,B1950,0,5)){
               myHTMIndexSearchClient.setEquinox(1950);
             }
           }catch(Exception e2){
                java.lang.String myErrorString = new java.lang.String();
                myErrorString = "A problem occured while setting the Equinox.  ";
               logMessage(myErrorString + selectedIndex);
              }
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
         RATextField2.setValue(coordinateArray[0]);
         DECTextField2.setValue(coordinateArray[1]);
         HMS myHMS = new HMS(coordinateArray[0]);
         DMS myDMS = new DMS(coordinateArray[1]);
         java.lang.String newRaString  = myHMS.toString();
         java.lang.String newDecString = myDMS.toString();
         logMessage(newRaString);
         logMessage(newDecString);
         RATextField1.setText(newRaString);
         DECTextField1.setText(newDecString);
       }
       catch(Exception e2){
           java.lang.String myErrorString = new java.lang.String();
           myErrorString = "A problem occured while executing the Get Coordinates method: " + e2;
           logMessage(myErrorString);
       }
     }
   });
/*===============================================================================================================
/    ResetCoordinatesButton  Button ActionListener
/===============================================================================================================*/
     ResetCoordinatesButton.addActionListener(new java.awt.event.ActionListener(){
       public void actionPerformed(java.awt.event.ActionEvent e){
          try{
            ObjectNameTextField.setText("");
            RATextField1.setText("");
            RATextField2.setValue(0.0);
            DECTextField1.setText("");
            DECTextField2.setValue(0.0);
            myHTMIndexSearchClient.setRA(0.0);
            myHTMIndexSearchClient.setDEC(0.0);
            SearchWidthTextField.setValue(SpatialSearchAreaObject.DEFAULT_SEARCH_WIDTH);
            SearchHeightTextField.setValue(SpatialSearchAreaObject.DEFAULT_SEARCH_HEIGHT);
          }
          catch(Exception e2){
              java.lang.String myErrorString = new java.lang.String();
              myErrorString = "A problem occured while executing the Reset Coordinates method: " + e2;
              logMessage(myErrorString);
          }
        }
      });
/*===============================================================================================================
/    SubmitQueryButton  Button ActionListener
/===============================================================================================================*/
     SubmitQueryButton.addActionListener(new java.awt.event.ActionListener(){
       public void actionPerformed(java.awt.event.ActionEvent e){
          try{
          }
          catch(Exception e2){
              java.lang.String myErrorString = new java.lang.String();
              myErrorString = "A problem occured while executing the Submit Query method: " + e2;
              logMessage(myErrorString);
          }
        }
      });
/*===============================================================================================================
/    AbortQueryButton  Button ActionListener
/===============================================================================================================*/
  AbortQueryButton.addActionListener(new java.awt.event.ActionListener(){
     public void actionPerformed(java.awt.event.ActionEvent e){
         try{
         } catch(Exception e2){
           java.lang.String myErrorString = new java.lang.String();
           myErrorString = "A problem occured while executing the Abort Query method: " + e2;
           logMessage(myErrorString);
         }
     }
   });
/*===============================================================================================================
/    GetHTMTrianglesButton  Button ActionListener
/===============================================================================================================*/
     GetHTMTrianglesButton.addActionListener(new java.awt.event.ActionListener(){
        public void actionPerformed(java.awt.event.ActionEvent e){
            try{
               double searchWidth  = SearchWidthTextField.getValue();
               double searchHeight = SearchHeightTextField.getValue();
               myHTMIndexSearchClient.getSpatialSearchAreaObject().setSearchWidth(searchWidth);
               myHTMIndexSearchClient.getSpatialSearchAreaObject().setSearchHeight(searchHeight);
               myHTMIndexSearchClient.getSpatialSearchAreaObject().constructHTMList();
               int rangeNumber    = myHTMIndexSearchClient.getSpatialSearchAreaObject().getRangeNumber();
               int triangleNumber = myHTMIndexSearchClient.getSpatialSearchAreaObject().getTriangleNumber();
               TriangleRangesTextField.setText(""+(Integer.valueOf(rangeNumber)).intValue());
               TriangleNumberTextField.setText(""+(Integer.valueOf(triangleNumber)).intValue());
               java.lang.String queryString = myHTMIndexSearchClient.getSpatialSearchAreaObject().constructHTMQueryString();
               myHTMIndexSearchClient.getSQLDisplayModel().insertCommentMessage(queryString);
            }catch(Exception e2){
              java.lang.String myErrorString = new java.lang.String();
              myErrorString = "A problem occured while executing the Abort Query method: " + e2;
              logMessage(myErrorString);
            }
        }
      });
   }
  }
/*================================================================================================
/     end of the class HTMSearchClientMediator
/=================================================================================================*/
/*================================================================================================
/    End of the class HTMSearchClientFrame
/=================================================================================================*/
}