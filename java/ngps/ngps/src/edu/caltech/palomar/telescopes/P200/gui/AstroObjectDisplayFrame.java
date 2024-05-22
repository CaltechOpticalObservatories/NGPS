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
//    AstroObjectDisplayFrame   - Display panel for a list of astronomical objects
//
//--- Description -------------------------------------------------------------
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
//       DERIVED DIRECTLY FROM THE JSKYCALC by JOHN THORSTENSEN, Dartmouth University
//       Original Implementation : John Thorstensen copyright 2007,
//
//       August 14, 2010  Jennifer Milburn, refactor for use at the Palomar Observatory
//
//
//   JSkyCalc24m.java -- 2009 January.  I liked the 1.3m so much I decided
//   to do something similar for the 2.4m.  This has the capability of reading
//   the telescope coords and commenting on them, but it doesn't slew the
//   telescope.  It'll be nice to automatically have the telescope coords there,
//   though. */
//
// JSkyCalc13m.java -- 2009 January.  This version adds an interface to
// * Dick Treffers' 1.3m telescope control server, which communicates via
// * the BAIT protocol.  The target-selection facilities here can then be
// * used. */
//
// JSkyCalc.java -- copyright 2007, John Thorstensen, Dartmouth College. */
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
import edu.caltech.palomar.telescopes.P200.gui.tables.AstroObjectsModel;
import javax.swing.event.*;
import javax.swing.table.*;
import javax.swing.*;
import java.beans.*;
import edu.caltech.palomar.telescopes.P200.gui.tables.AstroObject;
import com.sharkysoft.printf.Printf;
import com.sharkysoft.printf.PrintfData;
import com.sharkysoft.printf.PrintfTemplate;
import com.sharkysoft.printf.PrintfTemplateException;
import edu.dartmouth.jskycalc.JSkyCalcModel;
import edu.caltech.palomar.telescopes.P200.P200Component;
/*=============================================================================================
/      Class Declaration: AstroObjectDisplayFrame
/=============================================================================================*/
public class AstroObjectDisplayFrame extends javax.swing.JFrame {
    public AstroObjectsModel myAstroObjectsModel;
    public JSkyCalcModel     myJSkyCalcModel;
    public EphemerisFrame    myEphemerisFrame;
    public P200Component     myP200Component;
/*=============================================================================================
/      Constructor: AstroObjectDisplayFrame
/=============================================================================================*/
    /** Creates new form AstroObjectDisplayFrame */
    public AstroObjectDisplayFrame() {
        initComponents();
        initializeTable();
        initializeJSkyCalcModel();
        initializeRatesComboBox();  
        initializeEditorPane();
        initializeRadioButtonGroups();
        LoadSelectedButton.setEnabled(false);   // initially disable the load selected button and let the connect to telescope enable it
        this.setDefaultCloseOperation(JFrame.HIDE_ON_CLOSE);
    }
/*=================================================================================================
/        positionFrame()
/=================================================================================================*/
  public void positionFrame(){
     this.setLocation(myP200Component.myTelescopesIniReader.ASTRO_OBJECT_X, myP200Component.myTelescopesIniReader.ASTRO_OBJECT_Y);
     this.setVisible(myP200Component.myTelescopesIniReader.ASTRO_OBJECT_VIS);
     this.pack();
  }      
/*=============================================================================================
/      initializeJSkyCalcModel()
/=============================================================================================*/
  private void initializeJSkyCalcModel(){
      myJSkyCalcModel = new JSkyCalcModel();
      myEphemerisFrame = new EphemerisFrame();
      myEphemerisFrame.myEphemerisPanel.setJSkyCalcModel(myJSkyCalcModel);
      myAstroObjectsModel.setJSkyCalcModel(myJSkyCalcModel);
  }    
/*=============================================================================================
/      initializeJSkyCalcModel()
/=============================================================================================*/
 public AstroObjectsModel getAstroObjectModel(){
     return myAstroObjectsModel;
 }
/*=============================================================================================
/      setP200Component(P200Component newP200Component)
/=============================================================================================*/
 public void setP200Component(P200Component newP200Component){
     myP200Component = newP200Component;
        myP200Component.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
        public void propertyChange(java.beans.PropertyChangeEvent e) {
            P200Component_propertyChange(e);
         }
        });
 }
/*=============================================================================================
/     initializeRadioButtonGroups()
/=============================================================================================*/
private void initializeRadioButtonGroups(){
    // Initialize FileTypeButtonGroup
    FileTypeButtonGroup.add(CSVRadioButton);
    FileTypeButtonGroup.add(PTICRadioButton);
    // Initialize DelimiterButtonGroup
    DelimiterButtonGroup.add(ComaRadioButton);
    DelimiterButtonGroup.add(SemicolonRadioButton);
    DelimiterButtonGroup.add(TabRadioButton);
    DelimiterButtonGroup.add(SpaceRadioButton);
    DelimiterButtonGroup.add(OtherRadioButton);    
    PTICRadioButton.setSelected(true);
    ComaRadioButton.setSelected(true);
    OtherTextField.setText(AstroObjectsModel.COMA);
} 
/*=============================================================================================
/      Constructor: AstroObjectDisplayFrame
/=============================================================================================*/
 private void initializeRatesComboBox(){
 //COORDS (five or six arguments):  supply a position to the TCS.  The arguments
//in order are  1) RA in decimal hours, 2) declination in decimal degrees,
//3) equinox of coordinates (value of zero indicates apparent equinox), 4) RA
//motion, 5) dec motion, 6) motion flag.  
//     Motion flag absent or 0:  arguments 4 and 5 are proper motion in RA units of .0001 sec/yr and dec units of .001 arcsec/yr.
//     Motion flag = 1:  arguments 4 and 5 are offset tracking rates in arcsec/hr. 
//     Motion flag = 2:  arguments 4 and 5 are offset tracking rates with arg 4 in seconds/hr and arg 5 in arcsec/hr.)
//  NOTE: a name string can now be provided, by adding an extra parameter enclosed in double quotes;
//  this will preferably be at the end of the command line, though it can be
//  inserted at any parameter position.
    trackingRatesUnitsComboBox.removeAllItems();
    java.lang.String[]  myTrackingUnits = new java.lang.String[3];
    myTrackingUnits[0] = "Proper Motion (0.0001s/yr,0.001arcsec/yr";
    myTrackingUnits[1] = "Offset Tracking (arcsec/hr,arcsec/hr)";
    myTrackingUnits[2] = "Offset Tracking (s/hr,arcsec/hr)";
    trackingRatesUnitsComboBox.addItem(myTrackingUnits[0]);
    trackingRatesUnitsComboBox.addItem(myTrackingUnits[1]);
    trackingRatesUnitsComboBox.addItem(myTrackingUnits[2]);
    trackingRatesUnitsComboBox.setSelectedIndex(0);
    myAstroObjectsModel.setRatesUnits(myTrackingUnits[0]);
 }
/*=============================================================================================
/      Constructor: AstroObjectDisplayFrame
/=============================================================================================*/
   private void initializeTable(){
        myAstroObjectsModel = new AstroObjectsModel(this);

        myAstroObjectsModel.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
          public void propertyChange(java.beans.PropertyChangeEvent e) {
             AstroObjectsModel_propertyChange(e);
          }
        });
        myAstroObjectsModel.getAstroObjectTable().setJTable(myAstroObjectTable);
        myAstroObjectTable.setModel(myAstroObjectsModel.getAstroObjectTable());
        myAstroObjectTable.getColumnModel().getColumn(0).setMinWidth(200);
        myAstroObjectTable.getColumnModel().getColumn(1).setMinWidth(140);
        myAstroObjectTable.getColumnModel().getColumn(2).setMinWidth(140);
        myAstroObjectTable.getColumnModel().getColumn(3).setMinWidth(100);
        myAstroObjectTable.getColumnModel().getColumn(4).setMinWidth(130);
        myAstroObjectTable.getColumnModel().getColumn(5).setMinWidth(130);
        myAstroObjectTable.getColumnModel().getColumn(6).setMinWidth(130);
        myAstroObjectTable.getColumnModel().getColumn(7).setMinWidth(130);
        myAstroObjectTable.getColumnModel().getColumn(8).setMinWidth(130);
        myAstroObjectTable.getColumnModel().getColumn(9).setMinWidth(500);


        myAstroObjectTable.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);
        myAstroObjectTable.setSelectionMode(javax.swing.ListSelectionModel.SINGLE_SELECTION);
        myAstroObjectTable.setRowSelectionAllowed(true);
        myAstroObjectTable.getSelectionModel().addListSelectionListener(new SharedListSelectionHandler());
        myAstroObjectTable.setUpdateSelectionOnSort(true);
        myAstroObjectTable.setAutoCreateRowSorter(true);
        myAstroObjectTable.getRowSorter().toggleSortOrder(0);
        myAstroObjectsModel.setFileType(AstroObjectsModel.CSV);
   }
/*=============================================================================================
/     initializeEditorPane()
/=============================================================================================*/
   public void initializeEditorPane(){
      TargetTextPane.setDocument(myAstroObjectsModel.getTargetListDocumentModel().getDocument()); 
   }
/*=============================================================================================
/     Property Change Listener for the _propertyChange
/=============================================================================================*/
  private void AstroObjectsModel_propertyChange(PropertyChangeEvent e)  {
     java.lang.String propertyName = e.getPropertyName();
/*=============================================================================================
/
/=============================================================================================*/
     if(propertyName == "selected_right_ascension"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        RAStringTextField.setText(newValue);
    }
    if(propertyName == "selected_declination"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        DecStringTextField.setText(newValue);
    }
    if(propertyName == "selected_right_ascension_decimal"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        RAdecimalTextField.setText(newValue);
    }
    if(propertyName == "selected_declination_decimal"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        DecdecimalTextField.setText(newValue);
    }
    if(propertyName == "selected_equinox_string"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
     }
    if(propertyName == "selected_right_ascension_track_rate"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        RARateTextField.setText(newValue);
    }
    if(propertyName == "declination_track_rate"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        DecRateTextField.setText(newValue);
    }
    if(propertyName == "selected_objectname"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        ObjectNameTextField.setText(newValue);
    }
    if(propertyName == "current_file_name"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        FilePathTextField.setText(newValue);
    }
  }
/*=============================================================================================
/      Constructor: AstroObjectDisplayFrame
/=============================================================================================*/
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        FileTypeButtonGroup = new javax.swing.ButtonGroup();
        DelimiterButtonGroup = new javax.swing.ButtonGroup();
        MainPanel = new javax.swing.JPanel();
        jPanel3 = new javax.swing.JPanel();
        jLabel1 = new javax.swing.JLabel();
        jLabel2 = new javax.swing.JLabel();
        RAStringTextField = new javax.swing.JTextField();
        DecStringTextField = new javax.swing.JTextField();
        RAdecimalTextField = new javax.swing.JTextField();
        DecdecimalTextField = new javax.swing.JTextField();
        RARateTextField = new javax.swing.JTextField();
        DecRateTextField = new javax.swing.JTextField();
        RatesFlagCheckBox = new javax.swing.JCheckBox();
        jLabel4 = new javax.swing.JLabel();
        jLabel5 = new javax.swing.JLabel();
        jLabel6 = new javax.swing.JLabel();
        EphemerisToggleButton = new javax.swing.JToggleButton();
        LoadSelectedButton = new javax.swing.JButton();
        trackingRatesUnitsComboBox = new javax.swing.JComboBox();
        UpdateAirmassButton = new javax.swing.JButton();
        jLabel3 = new javax.swing.JLabel();
        ObjectNameTextField = new javax.swing.JTextField();
        StatusMessage = new javax.swing.JLabel();
        jTabbedPane1 = new javax.swing.JTabbedPane();
        jPanel4 = new javax.swing.JPanel();
        jScrollPane1 = new javax.swing.JScrollPane();
        myAstroObjectTable = new javax.swing.JTable();
        FilePathTextField = new javax.swing.JTextField();
        jPanel5 = new javax.swing.JPanel();
        jScrollPane2 = new javax.swing.JScrollPane();
        TargetTextPane = new javax.swing.JTextPane();
        ParseTextFile = new javax.swing.JButton();
        jPanel1 = new javax.swing.JPanel();
        CSVRadioButton = new javax.swing.JRadioButton();
        PTICRadioButton = new javax.swing.JRadioButton();
        ComaRadioButton = new javax.swing.JRadioButton();
        SemicolonRadioButton = new javax.swing.JRadioButton();
        TabRadioButton = new javax.swing.JRadioButton();
        SpaceRadioButton = new javax.swing.JRadioButton();
        OtherRadioButton = new javax.swing.JRadioButton();
        OtherTextField = new javax.swing.JTextField();
        jLabel7 = new javax.swing.JLabel();
        jLabel8 = new javax.swing.JLabel();
        ReadFileButton = new javax.swing.JButton();
        SaveTargetFileButton = new javax.swing.JButton();
        jMenuBar1 = new javax.swing.JMenuBar();
        FileMenu = new javax.swing.JMenu();
        OpenMenuItem = new javax.swing.JMenuItem();
        ClosMenuItem = new javax.swing.JMenuItem();

        setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);

        MainPanel.setBackground(new java.awt.Color(0, 0, 0));

        jPanel3.setBackground(new java.awt.Color(0, 0, 0));
        jPanel3.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(255, 255, 255), 4));
        jPanel3.setForeground(new java.awt.Color(0, 255, 255));

        jLabel1.setFont(new java.awt.Font("Arial", 1, 18)); // NOI18N
        jLabel1.setForeground(new java.awt.Color(0, 255, 255));
        jLabel1.setText("RA ");

        jLabel2.setFont(new java.awt.Font("Arial", 1, 18)); // NOI18N
        jLabel2.setForeground(new java.awt.Color(0, 255, 255));
        jLabel2.setText("Dec ");

        RAStringTextField.setBackground(new java.awt.Color(0, 0, 0));
        RAStringTextField.setFont(new java.awt.Font("SansSerif", 1, 24)); // NOI18N
        RAStringTextField.setForeground(new java.awt.Color(51, 204, 0));
        RAStringTextField.setText("00:00:00.0");
        RAStringTextField.setBorder(null);
        RAStringTextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                RAStringTextFieldActionPerformed(evt);
            }
        });

        DecStringTextField.setBackground(new java.awt.Color(0, 0, 0));
        DecStringTextField.setFont(new java.awt.Font("SansSerif", 1, 24)); // NOI18N
        DecStringTextField.setForeground(new java.awt.Color(51, 204, 0));
        DecStringTextField.setText("00:00:00.0");
        DecStringTextField.setBorder(null);
        DecStringTextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                DecStringTextFieldActionPerformed(evt);
            }
        });

        RAdecimalTextField.setBackground(new java.awt.Color(0, 0, 0));
        RAdecimalTextField.setFont(new java.awt.Font("SansSerif", 1, 18)); // NOI18N
        RAdecimalTextField.setForeground(new java.awt.Color(51, 204, 0));
        RAdecimalTextField.setHorizontalAlignment(javax.swing.JTextField.CENTER);
        RAdecimalTextField.setText("0.0");
        RAdecimalTextField.setBorder(null);

        DecdecimalTextField.setBackground(new java.awt.Color(0, 0, 0));
        DecdecimalTextField.setFont(new java.awt.Font("SansSerif", 1, 18)); // NOI18N
        DecdecimalTextField.setForeground(new java.awt.Color(51, 204, 0));
        DecdecimalTextField.setHorizontalAlignment(javax.swing.JTextField.CENTER);
        DecdecimalTextField.setText("0.0");
        DecdecimalTextField.setBorder(null);

        RARateTextField.setBackground(new java.awt.Color(0, 0, 0));
        RARateTextField.setFont(new java.awt.Font("SansSerif", 1, 18)); // NOI18N
        RARateTextField.setForeground(new java.awt.Color(51, 204, 0));
        RARateTextField.setHorizontalAlignment(javax.swing.JTextField.CENTER);
        RARateTextField.setText("0.0");
        RARateTextField.setBorder(null);

        DecRateTextField.setBackground(new java.awt.Color(0, 0, 0));
        DecRateTextField.setFont(new java.awt.Font("SansSerif", 1, 18)); // NOI18N
        DecRateTextField.setForeground(new java.awt.Color(51, 204, 0));
        DecRateTextField.setHorizontalAlignment(javax.swing.JTextField.CENTER);
        DecRateTextField.setText("0.0");
        DecRateTextField.setBorder(null);

        RatesFlagCheckBox.setBackground(new java.awt.Color(0, 0, 0));
        RatesFlagCheckBox.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        RatesFlagCheckBox.setForeground(new java.awt.Color(0, 255, 255));
        RatesFlagCheckBox.setText("Apply Rates");
        RatesFlagCheckBox.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                RatesFlagCheckBoxActionPerformed(evt);
            }
        });

        jLabel4.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        jLabel4.setForeground(new java.awt.Color(0, 255, 255));
        jLabel4.setText("sexagesimal");

        jLabel5.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        jLabel5.setForeground(new java.awt.Color(0, 255, 255));
        jLabel5.setText("decimal");

        jLabel6.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        jLabel6.setForeground(new java.awt.Color(0, 255, 255));
        jLabel6.setText("Rates");

        EphemerisToggleButton.setText("Ephemeris");
        EphemerisToggleButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                EphemerisToggleButtonActionPerformed(evt);
            }
        });

        LoadSelectedButton.setText("Load to Telescope");
        LoadSelectedButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                LoadSelectedButtonActionPerformed(evt);
            }
        });

        trackingRatesUnitsComboBox.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "Item 1", "Item 2", "Item 3", "Item 4" }));
        trackingRatesUnitsComboBox.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent evt) {
                trackingRatesUnitsComboBoxItemStateChanged(evt);
            }
        });
        trackingRatesUnitsComboBox.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                trackingRatesUnitsComboBoxActionPerformed(evt);
            }
        });

        UpdateAirmassButton.setText("Update Airmass");
        UpdateAirmassButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                UpdateAirmassButtonActionPerformed(evt);
            }
        });

        org.jdesktop.layout.GroupLayout jPanel3Layout = new org.jdesktop.layout.GroupLayout(jPanel3);
        jPanel3.setLayout(jPanel3Layout);
        jPanel3Layout.setHorizontalGroup(
            jPanel3Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel3Layout.createSequentialGroup()
                .add(jPanel3Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(jPanel3Layout.createSequentialGroup()
                        .add(83, 83, 83)
                        .add(jLabel4))
                    .add(jPanel3Layout.createSequentialGroup()
                        .add(12, 12, 12)
                        .add(jPanel3Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                            .add(jLabel2)
                            .add(jLabel1))
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.UNRELATED)
                        .add(jPanel3Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING, false)
                            .add(RAStringTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 160, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                            .add(DecStringTextField))))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(jPanel3Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(jPanel3Layout.createSequentialGroup()
                        .add(jLabel5)
                        .add(114, 114, 114)
                        .add(jLabel6))
                    .add(jPanel3Layout.createSequentialGroup()
                        .add(15, 15, 15)
                        .add(jPanel3Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING, false)
                            .add(DecdecimalTextField)
                            .add(RAdecimalTextField, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 103, Short.MAX_VALUE))
                        .add(48, 48, 48)
                        .add(jPanel3Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING, false)
                            .add(RARateTextField)
                            .add(DecRateTextField, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 107, Short.MAX_VALUE))))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(jPanel3Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(org.jdesktop.layout.GroupLayout.TRAILING, jPanel3Layout.createSequentialGroup()
                        .add(RatesFlagCheckBox)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.UNRELATED)
                        .add(LoadSelectedButton))
                    .add(org.jdesktop.layout.GroupLayout.TRAILING, jPanel3Layout.createSequentialGroup()
                        .add(UpdateAirmassButton, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.UNRELATED)
                        .add(EphemerisToggleButton, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 128, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                    .add(trackingRatesUnitsComboBox, 0, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .add(61, 61, 61))
        );
        jPanel3Layout.setVerticalGroup(
            jPanel3Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel3Layout.createSequentialGroup()
                .addContainerGap()
                .add(jPanel3Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel4, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 28, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(jLabel5)
                    .add(jLabel6)
                    .add(EphemerisToggleButton)
                    .add(UpdateAirmassButton))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(jPanel3Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel1)
                    .add(RAStringTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(RAdecimalTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(RARateTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(LoadSelectedButton)
                    .add(RatesFlagCheckBox))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(jPanel3Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel2)
                    .add(DecStringTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(DecdecimalTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(DecRateTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(trackingRatesUnitsComboBox, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addContainerGap())
        );

        jLabel3.setFont(new java.awt.Font("Arial", 1, 18)); // NOI18N
        jLabel3.setForeground(new java.awt.Color(0, 255, 255));
        jLabel3.setText("Object Name");

        ObjectNameTextField.setBackground(new java.awt.Color(0, 0, 0));
        ObjectNameTextField.setFont(new java.awt.Font("Arial", 1, 24)); // NOI18N
        ObjectNameTextField.setForeground(new java.awt.Color(0, 255, 51));
        ObjectNameTextField.setBorder(javax.swing.BorderFactory.createEtchedBorder());

        StatusMessage.setFont(new java.awt.Font("SansSerif", 1, 18)); // NOI18N
        StatusMessage.setForeground(new java.awt.Color(0, 200, 0));
        StatusMessage.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(255, 255, 255)));

        jPanel4.setBackground(new java.awt.Color(0, 0, 0));
        jPanel4.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(255, 255, 255), 5));

        jScrollPane1.setHorizontalScrollBarPolicy(javax.swing.ScrollPaneConstants.HORIZONTAL_SCROLLBAR_ALWAYS);
        jScrollPane1.setVerticalScrollBarPolicy(javax.swing.ScrollPaneConstants.VERTICAL_SCROLLBAR_ALWAYS);
        jScrollPane1.setFont(new java.awt.Font("SansSerif", 0, 14)); // NOI18N

        myAstroObjectTable.setFont(new java.awt.Font("Arial", 1, 18)); // NOI18N
        myAstroObjectTable.setModel(new javax.swing.table.DefaultTableModel(
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
        myAstroObjectTable.setSelectionMode(javax.swing.ListSelectionModel.SINGLE_SELECTION);
        jScrollPane1.setViewportView(myAstroObjectTable);

        org.jdesktop.layout.GroupLayout jPanel4Layout = new org.jdesktop.layout.GroupLayout(jPanel4);
        jPanel4.setLayout(jPanel4Layout);
        jPanel4Layout.setHorizontalGroup(
            jPanel4Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel4Layout.createSequentialGroup()
                .addContainerGap()
                .add(jPanel4Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(org.jdesktop.layout.GroupLayout.TRAILING, jScrollPane1, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 788, Short.MAX_VALUE)
                    .add(org.jdesktop.layout.GroupLayout.TRAILING, FilePathTextField, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 788, Short.MAX_VALUE))
                .addContainerGap())
        );
        jPanel4Layout.setVerticalGroup(
            jPanel4Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel4Layout.createSequentialGroup()
                .addContainerGap()
                .add(FilePathTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(jScrollPane1, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 319, Short.MAX_VALUE)
                .addContainerGap())
        );

        jTabbedPane1.addTab("Astronomical Targets Table", jPanel4);

        jPanel5.setBackground(new java.awt.Color(5, 4, 3));
        jPanel5.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        jScrollPane2.setHorizontalScrollBarPolicy(javax.swing.ScrollPaneConstants.HORIZONTAL_SCROLLBAR_ALWAYS);
        jScrollPane2.setVerticalScrollBarPolicy(javax.swing.ScrollPaneConstants.VERTICAL_SCROLLBAR_ALWAYS);
        jScrollPane2.setViewportView(TargetTextPane);

        jPanel5.add(jScrollPane2, new org.netbeans.lib.awtextra.AbsoluteConstraints(12, 118, 744, 247));

        ParseTextFile.setText("parse imported text file");
        ParseTextFile.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                ParseTextFileActionPerformed(evt);
            }
        });
        jPanel5.add(ParseTextFile, new org.netbeans.lib.awtextra.AbsoluteConstraints(491, 47, 254, -1));

        jPanel1.setBackground(new java.awt.Color(5, 4, 3));
        jPanel1.setBorder(new javax.swing.border.LineBorder(new java.awt.Color(8, 190, 2), 2, true));

        CSVRadioButton.setBackground(new java.awt.Color(11, 10, 9));
        CSVRadioButton.setForeground(new java.awt.Color(14, 243, 9));
        CSVRadioButton.setText("CSV File");
        CSVRadioButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                CSVRadioButtonActionPerformed(evt);
            }
        });

        PTICRadioButton.setBackground(new java.awt.Color(11, 10, 9));
        PTICRadioButton.setForeground(new java.awt.Color(14, 243, 9));
        PTICRadioButton.setText("PTIC File");
        PTICRadioButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                PTICRadioButtonActionPerformed(evt);
            }
        });

        ComaRadioButton.setBackground(new java.awt.Color(11, 10, 9));
        ComaRadioButton.setForeground(new java.awt.Color(14, 243, 9));
        ComaRadioButton.setText("Comma");
        ComaRadioButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                ComaRadioButtonActionPerformed(evt);
            }
        });

        SemicolonRadioButton.setBackground(new java.awt.Color(11, 10, 9));
        SemicolonRadioButton.setForeground(new java.awt.Color(14, 243, 9));
        SemicolonRadioButton.setText("Semicolon");
        SemicolonRadioButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                SemicolonRadioButtonActionPerformed(evt);
            }
        });

        TabRadioButton.setBackground(new java.awt.Color(11, 10, 9));
        TabRadioButton.setForeground(new java.awt.Color(14, 243, 9));
        TabRadioButton.setText("Tab");
        TabRadioButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                TabRadioButtonActionPerformed(evt);
            }
        });

        SpaceRadioButton.setBackground(new java.awt.Color(11, 10, 9));
        SpaceRadioButton.setForeground(new java.awt.Color(14, 243, 9));
        SpaceRadioButton.setText("Space");
        SpaceRadioButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                SpaceRadioButtonActionPerformed(evt);
            }
        });

        OtherRadioButton.setBackground(new java.awt.Color(11, 10, 9));
        OtherRadioButton.setForeground(new java.awt.Color(14, 243, 9));
        OtherRadioButton.setText("Other");
        OtherRadioButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                OtherRadioButtonActionPerformed(evt);
            }
        });

        OtherTextField.setForeground(new java.awt.Color(14, 243, 9));
        OtherTextField.setText(" ");
        OtherTextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                OtherTextFieldActionPerformed(evt);
            }
        });

        jLabel7.setFont(new java.awt.Font("DejaVu LGC Sans", 1, 14)); // NOI18N
        jLabel7.setForeground(new java.awt.Color(9, 243, 236));
        jLabel7.setText("Import File Type");

        jLabel8.setFont(new java.awt.Font("DejaVu LGC Sans", 1, 14)); // NOI18N
        jLabel8.setForeground(new java.awt.Color(9, 243, 236));
        jLabel8.setText("Delimiter");

        org.jdesktop.layout.GroupLayout jPanel1Layout = new org.jdesktop.layout.GroupLayout(jPanel1);
        jPanel1.setLayout(jPanel1Layout);
        jPanel1Layout.setHorizontalGroup(
            jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel1Layout.createSequentialGroup()
                .addContainerGap()
                .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(jLabel7)
                    .add(CSVRadioButton)
                    .add(PTICRadioButton))
                .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(jPanel1Layout.createSequentialGroup()
                        .add(27, 27, 27)
                        .add(jLabel8))
                    .add(jPanel1Layout.createSequentialGroup()
                        .add(19, 19, 19)
                        .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                            .add(SemicolonRadioButton)
                            .add(ComaRadioButton))
                        .add(18, 18, 18)
                        .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                            .add(jPanel1Layout.createSequentialGroup()
                                .add(TabRadioButton)
                                .add(18, 18, 18)
                                .add(OtherRadioButton)
                                .add(6, 6, 6)
                                .add(OtherTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 35, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                            .add(SpaceRadioButton))))
                .addContainerGap(24, Short.MAX_VALUE))
        );
        jPanel1Layout.setVerticalGroup(
            jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel1Layout.createSequentialGroup()
                .addContainerGap(org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel7)
                    .add(jLabel8))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(CSVRadioButton)
                    .add(ComaRadioButton)
                    .add(TabRadioButton)
                    .add(OtherRadioButton)
                    .add(OtherTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(PTICRadioButton)
                    .add(SemicolonRadioButton)
                    .add(SpaceRadioButton)))
        );

        jPanel5.add(jPanel1, new org.netbeans.lib.awtextra.AbsoluteConstraints(12, 12, -1, -1));

        ReadFileButton.setText("Import Astronomical Targets File");
        ReadFileButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                ReadFileButtonActionPerformed(evt);
            }
        });
        jPanel5.add(ReadFileButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(491, 12, 254, -1));

        SaveTargetFileButton.setText("Save Target File");
        SaveTargetFileButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                SaveTargetFileButtonActionPerformed(evt);
            }
        });
        jPanel5.add(SaveTargetFileButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(490, 80, 250, -1));

        jTabbedPane1.addTab("Import Text File ", jPanel5);

        org.jdesktop.layout.GroupLayout MainPanelLayout = new org.jdesktop.layout.GroupLayout(MainPanel);
        MainPanel.setLayout(MainPanelLayout);
        MainPanelLayout.setHorizontalGroup(
            MainPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(MainPanelLayout.createSequentialGroup()
                .add(10, 10, 10)
                .add(MainPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(MainPanelLayout.createSequentialGroup()
                        .add(jLabel3)
                        .add(0, 0, 0)
                        .add(ObjectNameTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 690, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                    .add(jPanel3, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(MainPanelLayout.createSequentialGroup()
                        .add(MainPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                            .add(org.jdesktop.layout.GroupLayout.LEADING, jTabbedPane1, 0, 0, Short.MAX_VALUE)
                            .add(org.jdesktop.layout.GroupLayout.LEADING, StatusMessage, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                        .add(10, 10, 10)))
                .add(20, 20, 20))
        );
        MainPanelLayout.setVerticalGroup(
            MainPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(MainPanelLayout.createSequentialGroup()
                .add(12, 12, 12)
                .add(MainPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(jLabel3)
                    .add(ObjectNameTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 30, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(jPanel3, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .add(11, 11, 11)
                .add(StatusMessage, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 30, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .add(10, 10, 10)
                .add(jTabbedPane1)
                .add(25, 25, 25))
        );

        FileMenu.setText("File");

        OpenMenuItem.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_O, java.awt.event.InputEvent.CTRL_MASK));
        OpenMenuItem.setText("Open");
        OpenMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                OpenMenuItemActionPerformed(evt);
            }
        });
        FileMenu.add(OpenMenuItem);

        ClosMenuItem.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_C, 0));
        ClosMenuItem.setText("Close");
        ClosMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                ClosMenuItemActionPerformed(evt);
            }
        });
        FileMenu.add(ClosMenuItem);

        jMenuBar1.add(FileMenu);

        setJMenuBar(jMenuBar1);

        org.jdesktop.layout.GroupLayout layout = new org.jdesktop.layout.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(MainPanel, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(MainPanel, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents

    private void RatesFlagCheckBoxActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_RatesFlagCheckBoxActionPerformed
        // TODO add your handling code here:
        boolean state = RatesFlagCheckBox.isSelected();
        myP200Component.getTelescopeObject().setTracking(state);
    }//GEN-LAST:event_RatesFlagCheckBoxActionPerformed

    private void RAStringTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_RAStringTextFieldActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_RAStringTextFieldActionPerformed

    private void DecStringTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_DecStringTextFieldActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_DecStringTextFieldActionPerformed

    private void OpenMenuItemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_OpenMenuItemActionPerformed
       myAstroObjectsModel.Open();
    }//GEN-LAST:event_OpenMenuItemActionPerformed

    private void ClosMenuItemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_ClosMenuItemActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_ClosMenuItemActionPerformed

    private void EphemerisToggleButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_EphemerisToggleButtonActionPerformed
          EphemerisToggleButton_actionPerformed(evt);
    }//GEN-LAST:event_EphemerisToggleButtonActionPerformed

    private void LoadSelectedButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_LoadSelectedButtonActionPerformed
        // TODO add your handling code here:
      if(myAstroObjectsModel.getAstroObject() != null){
        AstroObject currentObject = myAstroObjectsModel.getAstroObject();
          if(myP200Component.isConnected()){
              myP200Component.sentCoordinates(currentObject);
          }
      }
    }//GEN-LAST:event_LoadSelectedButtonActionPerformed

    private void trackingRatesUnitsComboBoxItemStateChanged(java.awt.event.ItemEvent evt) {//GEN-FIRST:event_trackingRatesUnitsComboBoxItemStateChanged
        // TODO add your handling code here:
    }//GEN-LAST:event_trackingRatesUnitsComboBoxItemStateChanged

    private void trackingRatesUnitsComboBoxActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_trackingRatesUnitsComboBoxActionPerformed
        int selected = trackingRatesUnitsComboBox.getSelectedIndex();
        if(myP200Component != null){
           java.lang.String ratesUnits = (String)(trackingRatesUnitsComboBox.getItemAt(selected));
           myP200Component.getTelescopeObject().setTrackingRatesUnits(selected);
           myAstroObjectsModel.setRatesUnits(ratesUnits);
        }
    }//GEN-LAST:event_trackingRatesUnitsComboBoxActionPerformed

private void ParseTextFileActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_ParseTextFileActionPerformed
// TODO add your handling code here:
    myAstroObjectsModel.parseText(myAstroObjectsModel.getFileType());
    myAstroObjectsModel.updateAirmass();
}//GEN-LAST:event_ParseTextFileActionPerformed

private void CSVRadioButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_CSVRadioButtonActionPerformed
// TODO add your handling code here:
    boolean state = CSVRadioButton.isSelected();
    if(state){
        myAstroObjectsModel.setFileType(AstroObjectsModel.CSV);
    }
}//GEN-LAST:event_CSVRadioButtonActionPerformed

private void PTICRadioButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_PTICRadioButtonActionPerformed
    boolean state = PTICRadioButton.isSelected();
    if(state){
        myAstroObjectsModel.setFileType(AstroObjectsModel.PTIC);
    }
}//GEN-LAST:event_PTICRadioButtonActionPerformed

private void ComaRadioButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_ComaRadioButtonActionPerformed
    boolean state = ComaRadioButton.isSelected();
    if(state){
        myAstroObjectsModel.setDelimiter(AstroObjectsModel.COMA);
    }
}//GEN-LAST:event_ComaRadioButtonActionPerformed

private void SemicolonRadioButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_SemicolonRadioButtonActionPerformed
    boolean state = SemicolonRadioButton.isSelected();
    if(state){
        myAstroObjectsModel.setDelimiter(AstroObjectsModel.SEMICOLON);
    }
}//GEN-LAST:event_SemicolonRadioButtonActionPerformed

private void TabRadioButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_TabRadioButtonActionPerformed
    boolean state = TabRadioButton.isSelected();
    if(state){
        myAstroObjectsModel.setDelimiter(myAstroObjectsModel.TAB);
    }
}//GEN-LAST:event_TabRadioButtonActionPerformed

private void SpaceRadioButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_SpaceRadioButtonActionPerformed
    boolean state = SpaceRadioButton.isSelected();
    if(state){
        myAstroObjectsModel.setDelimiter(AstroObjectsModel.SPACE);
    }
}//GEN-LAST:event_SpaceRadioButtonActionPerformed

private void OtherRadioButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_OtherRadioButtonActionPerformed
    boolean state = OtherRadioButton.isSelected();
    if(state){
        myAstroObjectsModel.setDelimiter(OtherTextField.getText());
    }
}//GEN-LAST:event_OtherRadioButtonActionPerformed

private void OtherTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_OtherTextFieldActionPerformed
    java.lang.String myValue = OtherTextField.getText();
     myAstroObjectsModel.setOtherDelimiter(myValue);
}//GEN-LAST:event_OtherTextFieldActionPerformed

private void ReadFileButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_ReadFileButtonActionPerformed
     myAstroObjectsModel.Open();
}//GEN-LAST:event_ReadFileButtonActionPerformed

private void UpdateAirmassButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_UpdateAirmassButtonActionPerformed
      myAstroObjectsModel.updateAirmass();
}//GEN-LAST:event_UpdateAirmassButtonActionPerformed

    private void SaveTargetFileButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_SaveTargetFileButtonActionPerformed
       myAstroObjectsModel.writeFile();
    }//GEN-LAST:event_SaveTargetFileButtonActionPerformed
/*=============================================================================================
/     Property Change Listener for the _propertyChange
/=============================================================================================*/
  private void P200Component_propertyChange(PropertyChangeEvent e)  {
     java.lang.String propertyName = e.getPropertyName();
       if(propertyName == "connected"){
        boolean state = (java.lang.Boolean)e.getNewValue();
        if(state){
            LoadSelectedButton.setEnabled(true);
        }
        if(!state){
            LoadSelectedButton.setEnabled(false);
        }
      }
       if(propertyName == "coord_message"){
         java.lang.String statusMessage = (java.lang.String)e.getNewValue();
         StatusMessage.setText(statusMessage);
      }
  }
/*=============================================================================================
/      Constructor: AstroObjectDisplayFrame
/=============================================================================================*/
    /**
    * @param args the command line arguments
    */
    public static void main(String args[]) {
        java.awt.EventQueue.invokeLater(new Runnable() {
            public void run() {
                new AstroObjectDisplayFrame().setVisible(true);
            }
        });
    }
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JRadioButton CSVRadioButton;
    private javax.swing.JMenuItem ClosMenuItem;
    private javax.swing.JRadioButton ComaRadioButton;
    private javax.swing.JTextField DecRateTextField;
    private javax.swing.JTextField DecStringTextField;
    private javax.swing.JTextField DecdecimalTextField;
    private javax.swing.ButtonGroup DelimiterButtonGroup;
    private javax.swing.JToggleButton EphemerisToggleButton;
    private javax.swing.JMenu FileMenu;
    private javax.swing.JTextField FilePathTextField;
    private javax.swing.ButtonGroup FileTypeButtonGroup;
    private javax.swing.JButton LoadSelectedButton;
    private javax.swing.JPanel MainPanel;
    private javax.swing.JTextField ObjectNameTextField;
    private javax.swing.JMenuItem OpenMenuItem;
    private javax.swing.JRadioButton OtherRadioButton;
    private javax.swing.JTextField OtherTextField;
    private javax.swing.JRadioButton PTICRadioButton;
    private javax.swing.JButton ParseTextFile;
    private javax.swing.JTextField RARateTextField;
    private javax.swing.JTextField RAStringTextField;
    private javax.swing.JTextField RAdecimalTextField;
    private javax.swing.JCheckBox RatesFlagCheckBox;
    private javax.swing.JButton ReadFileButton;
    private javax.swing.JButton SaveTargetFileButton;
    private javax.swing.JRadioButton SemicolonRadioButton;
    private javax.swing.JRadioButton SpaceRadioButton;
    private javax.swing.JLabel StatusMessage;
    private javax.swing.JRadioButton TabRadioButton;
    private javax.swing.JTextPane TargetTextPane;
    private javax.swing.JButton UpdateAirmassButton;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JLabel jLabel3;
    private javax.swing.JLabel jLabel4;
    private javax.swing.JLabel jLabel5;
    private javax.swing.JLabel jLabel6;
    private javax.swing.JLabel jLabel7;
    private javax.swing.JLabel jLabel8;
    private javax.swing.JMenuBar jMenuBar1;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JPanel jPanel3;
    private javax.swing.JPanel jPanel4;
    private javax.swing.JPanel jPanel5;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JScrollPane jScrollPane2;
    private javax.swing.JTabbedPane jTabbedPane1;
    private javax.swing.JTable myAstroObjectTable;
    private javax.swing.JComboBox trackingRatesUnitsComboBox;
    // End of variables declaration//GEN-END:variables
/*================================================================================================
/         Detector_ActionPerformed - Toolbar Button Action
/=================================================================================================*/
  public void EphemerisToggleButton_actionPerformed(java.awt.event.ActionEvent e){
      boolean isFrameVisible = EphemerisToggleButton.isSelected();
      if(isFrameVisible){
              myEphemerisFrame.setVisible(true);
      }// end of isFrameVisible
      if(!isFrameVisible){
             myEphemerisFrame.setVisible(false);
      }
  }
/*=============================================================================================
/      Constructor: SharedListSelectionHandler
/=============================================================================================*/
    class SharedListSelectionHandler implements ListSelectionListener {
        public void valueChanged(ListSelectionEvent e) {
            ListSelectionModel lsm = (ListSelectionModel)e.getSource();
           lsm.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
//            int     firstIndex  = e.getFirstIndex();
//            int     lastIndex   = e.getLastIndex();
             boolean isAdjusting = lsm.getValueIsAdjusting();
            if(!isAdjusting){
            try{               
                int     selection       = lsm.getLeadSelectionIndex();
                int     selectionModel  = myAstroObjectTable.convertRowIndexToModel(selection);
                AstroObject currentObject = myAstroObjectsModel.getAstroObjectTable().getRecord(selectionModel);
                myAstroObjectsModel.setAstroObject(currentObject);
                System.out.println(currentObject.name);
                myAstroObjectsModel.setSelectedRightAscension(currentObject.Alpha.RoundedRAString(2, ":"));
                myAstroObjectsModel.setSelectedDeclination(currentObject.Delta.RoundedDecString(2, ":"));
                myAstroObjectsModel.setSelectedRightAscensionDecimal(Printf.format("%05.2f", new PrintfData().add(currentObject.Alpha.degrees())));
                myAstroObjectsModel.setSelectedDeclinationDecimal(Printf.format("%05.2f", new PrintfData().add(currentObject.Delta.degrees())));
                myAstroObjectsModel.setSelectedRightAscensionTrackRate(Printf.format("%05.2f", new PrintfData().add(currentObject.r_m)));
                myAstroObjectsModel.setSelectedDeclinationTrackRate(Printf.format("%05.2f", new PrintfData().add(currentObject.d_m)));
                myAstroObjectsModel.setSelectedEquinox(Printf.format("%04.0f", new PrintfData().add(currentObject.Equinox)));
                myAstroObjectsModel.setSelectedObjectName(currentObject.name);
                myJSkyCalcModel.setObjectName(currentObject.name);
                myJSkyCalcModel.setTelescopePosition(currentObject.Alpha.RoundedRAString(2, ":"), currentObject.Delta.RoundedDecString(2, ":"),
                                                     Printf.format("%04.0f", new PrintfData().add(currentObject.Equinox)));
//                myJSkyCalcModel.SetToNow();
                 if(myJSkyCalcModel.getTimeSource() == JSkyCalcModel.NOW){
                     myJSkyCalcModel.SetToNow();
                }
                if(myJSkyCalcModel.getTimeSource() == JSkyCalcModel.SELECTED_TIME){
                    java.lang.String[] selected_date_time = myJSkyCalcModel.getSelectedDateTime();
                    myJSkyCalcModel.setToDate(selected_date_time[0], selected_date_time[1]);
                 }
                myEphemerisFrame.myEphemerisPanel.repaint();
               } catch(Exception error){
                    System.out.println("No Object Selected");
               }
            } // end of if !isAdjusting
         }
    }
}
