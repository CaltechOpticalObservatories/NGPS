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
//    FactSumForm   - Display panel for the principle telescope pointing
//                         and status display parameters.
//
//--- Description -------------------------------------------------------------

// Modification History
//
// Modified to include information on the selected object from either
// the AstroObjectDisplayFrame or the SkyDisplayFrame.
// Uses the AstroObjectsModel to update a new panel showing the basic
// information on the selected target (name, RA and dec).  Includes
// a new button to load the results to the telescope.
// November 29,2010  Jennifer Milburn
//
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
//       DERIVED DIRECTLY FROM THE JSKYCALC by JOHN THORSTENSEN, Dartmouth University
//       Original Implementation : John Thorstensen copyright 2007,
//
//       July 24, 2010  Jennifer Milburn, refactor for use at the Palomar Observatory
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
import edu.caltech.palomar.telescopes.P200.TelescopeObject;
import edu.caltech.palomar.telescopes.P200.P200Component;
import java.beans.*;
import edu.caltech.palomar.telescopes.P200.gui.tables.AstroObjectsModel;
import edu.caltech.palomar.telescopes.P200.gui.tables.AstroObject;
import java.awt.Color;
import java.awt.event.WindowListener;
import java.awt.event.WindowEvent;
import javax.swing.JOptionPane;
import javax.swing.JFrame;
/*=============================================================================================
/
/=============================================================================================*/
public class FactSumForm extends javax.swing.JFrame implements WindowListener{
     public AstroObjectsModel myAstroObjectsModel;
     public TelescopeObject   myTelescopeObject;
     public P200Component     myP200Component;
    /** Creates new form FactSumForm */
/*================================================================================================
/         constructor public FactSumForm()
/=================================================================================================*/
   public FactSumForm() { 
        initComponents();
        this.setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);
        this.addWindowListener(this);
    }
/*=================================================================================================
/        positionFrame()
/=================================================================================================*/
  public void positionFrame(){
     this.setLocation(myP200Component.myTelescopesIniReader.FACSUMFORM_X, myP200Component.myTelescopesIniReader.FACSUMFORM_Y);
//     this.setVisible(myP200Component.myTelescopesIniReader.FACSUMFORM_VIS);
//     this.pack();
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
     myTelescopeObject.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
      public void propertyChange(java.beans.PropertyChangeEvent e) {
          TelescopeObject_propertyChange(e);
       }
     });
    }
 /*================================================================================================
/     setTelescopeObject(TelescopeObject newTelescopeObject)
/=================================================================================================*/
public void setAstroObjectsModel(AstroObjectsModel newAstroObjectsModel){
    this.myAstroObjectsModel = newAstroObjectsModel;
    myAstroObjectsModel.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
          public void propertyChange(java.beans.PropertyChangeEvent e) {
             AstroObjectsModel_propertyChange(e);
          }
        });
    RateUnitsLabel.setText(myAstroObjectsModel.getRatesUnits());
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
        SelectedObjectRALabel.setText(newValue);
    }
    if(propertyName == "selected_declination"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        SelectedObjectDecLabel.setText(newValue);
    }
//   if(propertyName == "selected_right_ascension_decimal"){
//      java.lang.String newValue = (java.lang.String)e.getNewValue();
//    }
//    if(propertyName == "selected_declination_decimal"){
//       java.lang.String newValue = (java.lang.String)e.getNewValue();
//    }
//    if(propertyName == "selected_equinox_string"){
//        java.lang.String newValue = (java.lang.String)e.getNewValue();
//     }
//    if(propertyName == "selected_right_ascension_track_rate"){
//        java.lang.String newValue = (java.lang.String)e.getNewValue();
//    }
//    if(propertyName == "declination_track_rate"){
//        java.lang.String newValue = (java.lang.String)e.getNewValue();
//    }
    if(propertyName == "selected_objectname"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        SelectedObjectLabel.setText(newValue);
    }
    if(propertyName.matches("rates_units")){
         java.lang.String newValue = (java.lang.String)e.getNewValue();
         RateUnitsLabel.setText(newValue);
    }
//    if(propertyName == "current_file_name"){
//        java.lang.String newValue = (java.lang.String)e.getNewValue();
//   }
  }
 public void initializeStates(){
     UpdateToggleButton.setEnabled(false);
     ConnectToTCSToggleButton.setEnabled(true);
 }
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

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
        jLabel1 = new javax.swing.JLabel();
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
        jPanel8 = new javax.swing.JPanel();
        jLabel8 = new javax.swing.JLabel();
        SelectedObjectLabel = new javax.swing.JLabel();
        jLabel10 = new javax.swing.JLabel();
        jLabel11 = new javax.swing.JLabel();
        SelectedObjectRALabel = new javax.swing.JLabel();
        SelectedObjectDecLabel = new javax.swing.JLabel();
        SendToTelescopeButton = new javax.swing.JButton();
        jLabel9 = new javax.swing.JLabel();
        TCSResponseLabel = new javax.swing.JLabel();
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

        setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);
        setBackground(new java.awt.Color(255, 255, 255));

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

        org.jdesktop.layout.GroupLayout jPanel2Layout = new org.jdesktop.layout.GroupLayout(jPanel2);
        jPanel2.setLayout(jPanel2Layout);
        jPanel2Layout.setHorizontalGroup(
            jPanel2Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel2Layout.createSequentialGroup()
                .add(jPanel2Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(jPanel2Layout.createSequentialGroup()
                        .add(46, 46, 46)
                        .add(EquinoxTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 158, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                    .add(jPanel2Layout.createSequentialGroup()
                        .add(32, 32, 32)
                        .add(jPanel2Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                            .add(HATextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 178, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                            .add(AirmassTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 178, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                            .add(FocusTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 188, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))))
                .addContainerGap(org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );
        jPanel2Layout.setVerticalGroup(
            jPanel2Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel2Layout.createSequentialGroup()
                .add(10, 10, 10)
                .add(AirmassTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(FocusTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(HATextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(EquinoxTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
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

        org.jdesktop.layout.GroupLayout jPanel3Layout = new org.jdesktop.layout.GroupLayout(jPanel3);
        jPanel3.setLayout(jPanel3Layout);
        jPanel3Layout.setHorizontalGroup(
            jPanel3Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel3Layout.createSequentialGroup()
                .addContainerGap()
                .add(jPanel3Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(RALabel1)
                    .add(RALabel4)
                    .add(RALabel5)
                    .add(RALabel8))
                .addContainerGap(52, Short.MAX_VALUE))
        );
        jPanel3Layout.setVerticalGroup(
            jPanel3Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel3Layout.createSequentialGroup()
                .add(20, 20, 20)
                .add(RALabel5)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(RALabel1, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 22, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(RALabel4)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.UNRELATED)
                .add(RALabel8)
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

        org.jdesktop.layout.GroupLayout jPanel6Layout = new org.jdesktop.layout.GroupLayout(jPanel6);
        jPanel6.setLayout(jPanel6Layout);
        jPanel6Layout.setHorizontalGroup(
            jPanel6Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel6Layout.createSequentialGroup()
                .add(8, 8, 8)
                .add(RALabel)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(LSTTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 319, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .add(18, 18, 18)
                .add(RALabel2)
                .add(18, 18, 18)
                .add(UTCTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 407, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(35, Short.MAX_VALUE))
        );
        jPanel6Layout.setVerticalGroup(
            jPanel6Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel6Layout.createSequentialGroup()
                .addContainerGap()
                .add(jPanel6Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(RALabel)
                    .add(LSTTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(RALabel2)
                    .add(UTCTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addContainerGap())
        );

        jPanel1.add(jPanel6, new org.netbeans.lib.awtextra.AbsoluteConstraints(30, 120, 890, 60));

        jLabel1.setIcon(new javax.swing.ImageIcon(getClass().getResource("/edu/caltech/palomar/telescopes/P200/simulator/resources/PalomarHaleLogo.jpg"))); // NOI18N
        jPanel1.add(jLabel1, new org.netbeans.lib.awtextra.AbsoluteConstraints(70, 10, -1, 70));

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

        org.jdesktop.layout.GroupLayout jPanel7Layout = new org.jdesktop.layout.GroupLayout(jPanel7);
        jPanel7.setLayout(jPanel7Layout);
        jPanel7Layout.setHorizontalGroup(
            jPanel7Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel7Layout.createSequentialGroup()
                .add(20, 20, 20)
                .add(jPanel7Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(jLabel2)
                    .add(jLabel3)
                    .add(jLabel4)
                    .add(jLabel6)
                    .add(jLabel7)
                    .add(jLabel5))
                .add(jPanel7Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING, false)
                    .add(AzimuthTextField)
                    .add(ZenithAngleTextField)
                    .add(CassRingAngleTextField)
                    .add(DomeAzimuthTextField)
                    .add(DomeShuttersTextField)
                    .add(WindScreensTextField, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 111, Short.MAX_VALUE))
                .add(424, 424, 424))
        );
        jPanel7Layout.setVerticalGroup(
            jPanel7Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel7Layout.createSequentialGroup()
                .add(0, 0, 0)
                .add(jPanel7Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel2)
                    .add(AzimuthTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(0, 0, 0)
                .add(jPanel7Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel3)
                    .add(ZenithAngleTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(0, 0, 0)
                .add(jPanel7Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel4)
                    .add(CassRingAngleTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(0, 0, 0)
                .add(jPanel7Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel6)
                    .add(DomeAzimuthTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(0, 0, 0)
                .add(jPanel7Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel7)
                    .add(DomeShuttersTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(0, 0, 0)
                .add(jPanel7Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel5)
                    .add(WindScreensTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 22, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
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

        org.jdesktop.layout.GroupLayout jPanel9Layout = new org.jdesktop.layout.GroupLayout(jPanel9);
        jPanel9.setLayout(jPanel9Layout);
        jPanel9Layout.setHorizontalGroup(
            jPanel9Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel9Layout.createSequentialGroup()
                .add(6, 6, 6)
                .add(ConnectToTCSToggleButton)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(UpdateToggleButton, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 88, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(38, Short.MAX_VALUE))
        );
        jPanel9Layout.setVerticalGroup(
            jPanel9Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel9Layout.createSequentialGroup()
                .addContainerGap()
                .add(jPanel9Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(ConnectToTCSToggleButton)
                    .add(UpdateToggleButton, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 29, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        jPanel1.add(jPanel9, new org.netbeans.lib.awtextra.AbsoluteConstraints(620, 580, 300, 50));

        jPanel10.setBackground(new java.awt.Color(0, 0, 0));

        RALabel6.setBackground(new java.awt.Color(0, 0, 0));
        RALabel6.setFont(new java.awt.Font("SansSerif", 1, 18)); // NOI18N
        RALabel6.setForeground(new java.awt.Color(0, 204, 204));
        RALabel6.setText("Current");

        RALabel7.setBackground(new java.awt.Color(0, 0, 0));
        RALabel7.setFont(new java.awt.Font("SansSerif", 1, 18)); // NOI18N
        RALabel7.setForeground(new java.awt.Color(0, 204, 204));
        RALabel7.setText("Last GO");

        org.jdesktop.layout.GroupLayout jPanel10Layout = new org.jdesktop.layout.GroupLayout(jPanel10);
        jPanel10.setLayout(jPanel10Layout);
        jPanel10Layout.setHorizontalGroup(
            jPanel10Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel10Layout.createSequentialGroup()
                .addContainerGap(31, Short.MAX_VALUE)
                .add(jPanel10Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(org.jdesktop.layout.GroupLayout.TRAILING, jPanel10Layout.createSequentialGroup()
                        .add(RALabel7)
                        .add(20, 20, 20))
                    .add(org.jdesktop.layout.GroupLayout.TRAILING, jPanel10Layout.createSequentialGroup()
                        .add(RALabel6)
                        .addContainerGap())))
        );
        jPanel10Layout.setVerticalGroup(
            jPanel10Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(org.jdesktop.layout.GroupLayout.TRAILING, jPanel10Layout.createSequentialGroup()
                .addContainerGap()
                .add(RALabel6)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED, 36, Short.MAX_VALUE)
                .add(RALabel7)
                .add(28, 28, 28))
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

        org.jdesktop.layout.GroupLayout jPanel5Layout = new org.jdesktop.layout.GroupLayout(jPanel5);
        jPanel5.setLayout(jPanel5Layout);
        jPanel5Layout.setHorizontalGroup(
            jPanel5Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel5Layout.createSequentialGroup()
                .add(86, 86, 86)
                .add(RAOffsetTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 162, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED, 258, Short.MAX_VALUE)
                .add(DecOffsetTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 162, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .add(132, 132, 132))
        );
        jPanel5Layout.setVerticalGroup(
            jPanel5Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel5Layout.createSequentialGroup()
                .add(jPanel5Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(RAOffsetTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(DecOffsetTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        jPanel1.add(jPanel5, new org.netbeans.lib.awtextra.AbsoluteConstraints(120, 310, 800, 33));

        jPanel8.setBackground(new java.awt.Color(0, 0, 0));
        jPanel8.setBorder(javax.swing.BorderFactory.createEtchedBorder(new java.awt.Color(102, 153, 0), null));

        jLabel8.setBackground(new java.awt.Color(0, 0, 0));
        jLabel8.setFont(new java.awt.Font("SansSerif", 1, 14)); // NOI18N
        jLabel8.setForeground(new java.awt.Color(0, 204, 204));
        jLabel8.setText("Selected Object: ");

        SelectedObjectLabel.setBackground(new java.awt.Color(0, 0, 0));
        SelectedObjectLabel.setFont(new java.awt.Font("SansSerif", 1, 18)); // NOI18N
        SelectedObjectLabel.setForeground(new java.awt.Color(0, 255, 0));

        jLabel10.setBackground(new java.awt.Color(0, 0, 0));
        jLabel10.setFont(new java.awt.Font("SansSerif", 1, 14)); // NOI18N
        jLabel10.setForeground(new java.awt.Color(0, 204, 204));
        jLabel10.setText("RA");

        jLabel11.setBackground(new java.awt.Color(0, 0, 0));
        jLabel11.setFont(new java.awt.Font("SansSerif", 1, 14)); // NOI18N
        jLabel11.setForeground(new java.awt.Color(0, 204, 204));
        jLabel11.setText("Dec");

        SelectedObjectRALabel.setBackground(new java.awt.Color(0, 0, 0));
        SelectedObjectRALabel.setFont(new java.awt.Font("SansSerif", 1, 18)); // NOI18N
        SelectedObjectRALabel.setForeground(new java.awt.Color(0, 255, 0));
        SelectedObjectRALabel.setText("                      ");

        SelectedObjectDecLabel.setBackground(new java.awt.Color(0, 0, 0));
        SelectedObjectDecLabel.setFont(new java.awt.Font("SansSerif", 1, 18)); // NOI18N
        SelectedObjectDecLabel.setForeground(new java.awt.Color(0, 255, 0));
        SelectedObjectDecLabel.setText("                       ");

        SendToTelescopeButton.setText("Send to Telescope");
        SendToTelescopeButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                SendToTelescopeButtonActionPerformed(evt);
            }
        });

        jLabel9.setBackground(new java.awt.Color(0, 0, 0));
        jLabel9.setFont(new java.awt.Font("SansSerif", 1, 14)); // NOI18N
        jLabel9.setForeground(new java.awt.Color(0, 204, 204));
        jLabel9.setText("TCS Response");

        TCSResponseLabel.setBackground(new java.awt.Color(0, 0, 0));
        TCSResponseLabel.setFont(new java.awt.Font("SansSerif", 1, 18)); // NOI18N
        TCSResponseLabel.setForeground(new java.awt.Color(0, 255, 0));
        TCSResponseLabel.setText("none");

        org.jdesktop.layout.GroupLayout jPanel8Layout = new org.jdesktop.layout.GroupLayout(jPanel8);
        jPanel8.setLayout(jPanel8Layout);
        jPanel8Layout.setHorizontalGroup(
            jPanel8Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel8Layout.createSequentialGroup()
                .addContainerGap()
                .add(jPanel8Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                    .add(jPanel8Layout.createSequentialGroup()
                        .add(jLabel8)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED))
                    .add(jPanel8Layout.createSequentialGroup()
                        .add(jLabel9, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 102, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                        .add(25, 25, 25)))
                .add(jPanel8Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING, false)
                    .add(jPanel8Layout.createSequentialGroup()
                        .add(SelectedObjectLabel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 209, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(jLabel10)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(SelectedObjectRALabel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 147, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(jLabel11)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(SelectedObjectDecLabel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 147, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                    .add(TCSResponseLabel, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(SendToTelescopeButton)
                .addContainerGap(org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );
        jPanel8Layout.setVerticalGroup(
            jPanel8Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel8Layout.createSequentialGroup()
                .addContainerGap(13, Short.MAX_VALUE)
                .add(jPanel8Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(jPanel8Layout.createSequentialGroup()
                        .add(jLabel8)
                        .add(7, 7, 7)
                        .add(jLabel9, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 27, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                    .add(SendToTelescopeButton)
                    .add(jPanel8Layout.createSequentialGroup()
                        .add(jPanel8Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                            .add(jLabel10)
                            .add(SelectedObjectRALabel)
                            .add(jLabel11)
                            .add(SelectedObjectDecLabel)
                            .add(SelectedObjectLabel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 19, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(TCSResponseLabel)))
                .addContainerGap())
        );

        jPanel1.add(jPanel8, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 490, 900, 80));

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

        org.jdesktop.layout.GroupLayout jPanel11Layout = new org.jdesktop.layout.GroupLayout(jPanel11);
        jPanel11.setLayout(jPanel11Layout);
        jPanel11Layout.setHorizontalGroup(
            jPanel11Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel11Layout.createSequentialGroup()
                .add(21, 21, 21)
                .add(RALabel9)
                .add(47, 47, 47)
                .add(TrackRateRATextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 67, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED, 42, Short.MAX_VALUE)
                .add(RALabel10)
                .add(31, 31, 31)
                .add(TrackRateDecTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 74, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .add(45, 45, 45))
            .add(org.jdesktop.layout.GroupLayout.TRAILING, jPanel11Layout.createSequentialGroup()
                .add(274, 274, 274)
                .add(RateUnitsLabel, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 302, Short.MAX_VALUE)
                .addContainerGap())
        );
        jPanel11Layout.setVerticalGroup(
            jPanel11Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel11Layout.createSequentialGroup()
                .add(12, 12, 12)
                .add(jPanel11Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(TrackRateDecTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 22, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(RALabel10)
                    .add(TrackRateRATextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 22, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(RALabel9))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .add(RateUnitsLabel)
                .addContainerGap())
        );

        jPanel1.add(jPanel11, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 580, 590, 60));

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

        org.jdesktop.layout.GroupLayout jPanel12Layout = new org.jdesktop.layout.GroupLayout(jPanel12);
        jPanel12.setLayout(jPanel12Layout);
        jPanel12Layout.setHorizontalGroup(
            jPanel12Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel12Layout.createSequentialGroup()
                .addContainerGap()
                .add(RALabel11)
                .add(18, 18, 18)
                .add(ObjectNameTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 253, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(37, Short.MAX_VALUE))
        );
        jPanel12Layout.setVerticalGroup(
            jPanel12Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel12Layout.createSequentialGroup()
                .add(jPanel12Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(RALabel11)
                    .add(ObjectNameTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 31, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
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

        org.jdesktop.layout.GroupLayout layout = new org.jdesktop.layout.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel1, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 935, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel1, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 655, Short.MAX_VALUE)
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents

    private void LSTTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_LSTTextFieldActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_LSTTextFieldActionPerformed

    private void CurrentDecTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_CurrentDecTextFieldActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_CurrentDecTextFieldActionPerformed

    private void UTCTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_UTCTextFieldActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_UTCTextFieldActionPerformed

    private void CurrentRATextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_CurrentRATextFieldActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_CurrentRATextFieldActionPerformed

    private void OffsetRATextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_OffsetRATextFieldActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_OffsetRATextFieldActionPerformed

    private void OffsetDecTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_OffsetDecTextFieldActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_OffsetDecTextFieldActionPerformed

    private void DecOffsetTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_DecOffsetTextFieldActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_DecOffsetTextFieldActionPerformed

    private void HATextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_HATextFieldActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_HATextFieldActionPerformed

    private void FocusTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_FocusTextFieldActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_FocusTextFieldActionPerformed

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

    private void EquinoxTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_EquinoxTextFieldActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_EquinoxTextFieldActionPerformed

    private void ConnectToTCSToggleButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_ConnectToTCSToggleButtonActionPerformed
        // TODO add your handling code here:
        boolean selected = ConnectToTCSToggleButton.isSelected();
        if(selected){
          myP200Component.connect();
          UpdateToggleButton.setEnabled(true);
        }
        if(!selected){
          myP200Component.disconnect();
          UpdateToggleButton.setEnabled(false);
        }
    }//GEN-LAST:event_ConnectToTCSToggleButtonActionPerformed

    private void UpdateToggleButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_UpdateToggleButtonActionPerformed
        // TODO add your handling code here:
        boolean selected = UpdateToggleButton.isSelected();
        if(myP200Component.isConnected()){
           if(selected){
             myP200Component.startPolling();
             UpdateToggleButton.setText("Updating");
           }
           if(!selected){
             myP200Component.stopPolling();
             UpdateToggleButton.setText("Update");
           }
        }
    }//GEN-LAST:event_UpdateToggleButtonActionPerformed

    private void AirmassTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_AirmassTextFieldActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_AirmassTextFieldActionPerformed

    private void RAOffsetTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_RAOffsetTextFieldActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_RAOffsetTextFieldActionPerformed

    private void SendToTelescopeButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_SendToTelescopeButtonActionPerformed
     if(myAstroObjectsModel.getAstroObject() != null){
        AstroObject currentObject = myAstroObjectsModel.getAstroObject();
          if(myP200Component.isConnected()){
              myP200Component.sentCoordinates(currentObject);
          }
      }
    }//GEN-LAST:event_SendToTelescopeButtonActionPerformed

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
/*=============================================================================================
/     Property Change Listener for the _propertyChange
/=============================================================================================*/
  private void P200Component_propertyChange(PropertyChangeEvent e)  {
     java.lang.String propertyName = e.getPropertyName();
       if(propertyName == "connected"){
        boolean state = (java.lang.Boolean)e.getNewValue();
        if(state){
          ConnectToTCSToggleButton.setText("Connected to Telescope");
          UpdateToggleButton.setEnabled(true);
          UpdateToggleButton.setText("Update");
        }
        if(!state){
          ConnectToTCSToggleButton.setText("Connect to Telescope");
          UpdateToggleButton.setEnabled(false);
          UpdateToggleButton.setText("");
        }
      }
      if(propertyName == "coord_message"){
         java.lang.String statusMessage = (java.lang.String)e.getNewValue();
         TCSResponseLabel.setText(statusMessage);
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
/     WindowListener Interface - changing the behavior of the close events
/=============================================================================================*/
    public void windowClosing(WindowEvent e) {
        displayMessage("WindowListener method called: windowClosing.");
        int selection = JOptionPane.showConfirmDialog(this,"Do you REALLY want to exit?","A plain message",JOptionPane.YES_NO_OPTION);
        if(selection == 0){
           System.exit(0);
        }
    }
    public void windowClosed(WindowEvent e) {
        displayMessage("WindowListener method called: windowClosed.");
    }
    public void windowOpened(WindowEvent e) {
        displayMessage("WindowListener method called: windowOpened.");
    }

    public void windowIconified(WindowEvent e) {
        displayMessage("WindowListener method called: windowIconified.");
    }

    public void windowDeiconified(WindowEvent e) {
        displayMessage("WindowListener method called: windowDeiconified.");
    }

    public void windowActivated(WindowEvent e) {
        displayMessage("WindowListener method called: windowActivated.");
    }

    public void windowDeactivated(WindowEvent e) {
        displayMessage("WindowListener method called: windowDeactivated.");
    }
    void displayMessage(String msg) {
        System.out.println(msg);
    }
  /**
    * @param args the command line arguments
    */
    public static void main(String args[]) {
        java.awt.EventQueue.invokeLater(new Runnable() {
            public void run() {
                new FactSumForm().setVisible(true);
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
    private javax.swing.JTextField LSTTextField;
    private javax.swing.JTextField ObjectNameTextField;
    private javax.swing.JTextField OffsetDecTextField;
    private javax.swing.JTextField OffsetRATextField;
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
    private javax.swing.JLabel RateUnitsLabel;
    private javax.swing.JLabel SelectedObjectDecLabel;
    private javax.swing.JLabel SelectedObjectLabel;
    private javax.swing.JLabel SelectedObjectRALabel;
    private javax.swing.JButton SendToTelescopeButton;
    private javax.swing.JLabel TCSResponseLabel;
    private javax.swing.JTextField TrackRateDecTextField;
    private javax.swing.JTextField TrackRateRATextField;
    private javax.swing.JTextField UTCTextField;
    private javax.swing.JToggleButton UpdateToggleButton;
    private javax.swing.JTextField WindScreensTextField;
    private javax.swing.JTextField ZenithAngleTextField;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel10;
    private javax.swing.JLabel jLabel11;
    private javax.swing.JLabel jLabel2;
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
    private javax.swing.JPanel jPanel2;
    private javax.swing.JPanel jPanel3;
    private javax.swing.JPanel jPanel4;
    private javax.swing.JPanel jPanel5;
    private javax.swing.JPanel jPanel6;
    private javax.swing.JPanel jPanel7;
    private javax.swing.JPanel jPanel8;
    private javax.swing.JPanel jPanel9;
    private javax.swing.JTextField motionStateTextField;
    // End of variables declaration//GEN-END:variables

}
