/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package edu.caltech.palomar.telescopes.P200.simulator;

import edu.caltech.palomar.telescopes.P200.TelescopeObject;
import edu.caltech.palomar.telescopes.P200.TelescopesIniReader;
import edu.caltech.palomar.util.sindex.client.HTMIndexSearchClient;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeSupport;
import jsky.science.Coordinates;
 import com.sharkysoft.printf.Printf;
 import com.sharkysoft.printf.PrintfData;
 import com.sharkysoft.printf.PrintfTemplate;
 import com.sharkysoft.printf.PrintfTemplateException;
import javax.swing.ButtonGroup;

/**
 * 
 * @author developer
 */
public class P200SimulatorFrame2 extends javax.swing.JFrame {
  transient private PropertyChangeSupport propertyChangeListeners = new PropertyChangeSupport(this);
  public     P200ServerSocket             myServerSocket;
  public     HTMIndexSearchClient         myHTMIndexSearchClient         = new HTMIndexSearchClient();
  public     int                          myServerPort                   = 2345;
  public     TelescopeObject              myTelescopeObject;
  public     TelescopesIniReader          myTelescopesIniReader          = new TelescopesIniReader();
  private    double rightAscension;
  private    double declination;
  private    ButtonGroup          GSResolutionSourceButtonGroup  = new ButtonGroup();

    /**
     * Creates new form P200SimulatorFrame2
     */
    public P200SimulatorFrame2() {
        initComponents();
        initialize();
    }
/*================================================================================================
/      initialize()
/=================================================================================================*/
private void initialize(){
    myTelescopeObject = new TelescopeObject();
    myServerSocket = new P200ServerSocket(myTelescopesIniReader.SERVERPORT_INTEGER,myTelescopeObject);
    myHTMIndexSearchClient.setResolutionSource(HTMIndexSearchClient.SIMBAD);
/*=================================================================================
/     Property Change Listener Initialization for the LickServerSocket
/=================================================================================*/
    myTelescopeObject.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
       public void propertyChange(java.beans.PropertyChangeEvent e) {
          myTelescopeObject_propertyChange(e);
       }
    });
    GSResolutionSourceButtonGroup.add(SIMBADRadioButton);
    GSResolutionSourceButtonGroup.add(NEDRadioButton);
    SIMBADRadioButton.setSelected(true);
}
/*================================================================================================
/         startServer() methods
/=================================================================================================*/
  public void startServer(){
     myServerSocket.setErrorLoggingTextArea(this.errorsTextArea);
     myServerSocket.setSendLoggingTextArea(this.commandsTextArea);
     myServerSocket.setReceiveLoggingTextArea(this.responseTextArea);
     myServerSocket.startServer();
   }
/*================================================================================================
/         P200ServerSocket getP200ServerSocket() methods
/=================================================================================================*/
  public P200ServerSocket getServerSocket(){
    return myServerSocket;
  }
/*=================================================================================
/        Property Change Listener for the LickServerSocket Object
/=================================================================================*/
   public void myTelescopeObject_propertyChange(PropertyChangeEvent e)  {
      java.lang.String propertyName = e.getPropertyName();
          if(propertyName ==  "right_ascension"){
             java.lang.String myStringValue  = (java.lang.String)e.getNewValue();
             TargetRATextField.setText(myStringValue);
          }
          if(propertyName ==  "declination"){
             java.lang.String myStringValue  = (java.lang.String)e.getNewValue();
             TargetDECTextField.setText(myStringValue);
          }
          if(propertyName ==  "RA"){
             java.lang.Double myDoubleValue  = (java.lang.Double)e.getNewValue();
             RADecimalTextField.setText(Printf.format("%08.5f", new PrintfData().add(myDoubleValue.doubleValue())));
          }
          if(propertyName ==  "DEC"){
            java.lang.Double myDoubleValue  = (java.lang.Double)e.getNewValue();
            DECDecimalTextField.setText(Printf.format("%08.5f", new PrintfData().add(myDoubleValue.doubleValue())));
          }
   }// end of propertyChange Listener
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
    
    /**
     * This method is called from within the constructor to initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is always
     * regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        mainPanel = new javax.swing.JPanel();
        jPanel1 = new javax.swing.JPanel();
        jLabel1 = new javax.swing.JLabel();
        ObjectNameTextField = new javax.swing.JTextField();
        jPanel2 = new javax.swing.JPanel();
        SIMBADRadioButton = new javax.swing.JRadioButton();
        NEDRadioButton = new javax.swing.JRadioButton();
        GetCoordinatesButton = new javax.swing.JButton();
        jLabel2 = new javax.swing.JLabel();
        jPanel3 = new javax.swing.JPanel();
        jLabel3 = new javax.swing.JLabel();
        jLabel4 = new javax.swing.JLabel();
        TargetRATextField = new javax.swing.JTextField();
        TargetDECTextField = new javax.swing.JTextField();
        RADecimalTextField = new javax.swing.JTextField();
        DECDecimalTextField = new javax.swing.JTextField();
        jLabel5 = new javax.swing.JLabel();
        CassAngleTextField = new javax.swing.JTextField();
        jPanel4 = new javax.swing.JPanel();
        jScrollPane1 = new javax.swing.JScrollPane();
        commandsTextArea = new javax.swing.JTextArea();
        jScrollPane2 = new javax.swing.JScrollPane();
        errorsTextArea = new javax.swing.JTextArea();
        jScrollPane3 = new javax.swing.JScrollPane();
        responseTextArea = new javax.swing.JTextArea();
        jLabel6 = new javax.swing.JLabel();
        jLabel7 = new javax.swing.JLabel();
        jLabel8 = new javax.swing.JLabel();

        setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);

        jPanel1.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));

        jLabel1.setFont(new java.awt.Font("Century Schoolbook L", 1, 14)); // NOI18N
        jLabel1.setText("Object Name");

        ObjectNameTextField.setFont(new java.awt.Font("Century Schoolbook L", 0, 14)); // NOI18N
        ObjectNameTextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                ObjectNameTextFieldActionPerformed(evt);
            }
        });

        jPanel2.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));

        SIMBADRadioButton.setFont(new java.awt.Font("Century Schoolbook L", 1, 12)); // NOI18N
        SIMBADRadioButton.setText("SIMBAD");
        SIMBADRadioButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                SIMBADRadioButtonActionPerformed(evt);
            }
        });

        NEDRadioButton.setFont(new java.awt.Font("Century Schoolbook L", 1, 12)); // NOI18N
        NEDRadioButton.setText("NED");
        NEDRadioButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                NEDRadioButtonActionPerformed(evt);
            }
        });

        javax.swing.GroupLayout jPanel2Layout = new javax.swing.GroupLayout(jPanel2);
        jPanel2.setLayout(jPanel2Layout);
        jPanel2Layout.setHorizontalGroup(
            jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel2Layout.createSequentialGroup()
                .addComponent(SIMBADRadioButton)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(NEDRadioButton)
                .addGap(0, 13, Short.MAX_VALUE))
        );
        jPanel2Layout.setVerticalGroup(
            jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                .addComponent(SIMBADRadioButton)
                .addComponent(NEDRadioButton))
        );

        GetCoordinatesButton.setFont(new java.awt.Font("Century Schoolbook L", 1, 12)); // NOI18N
        GetCoordinatesButton.setText("Get Coordinates");
        GetCoordinatesButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                GetCoordinatesButtonActionPerformed(evt);
            }
        });

        javax.swing.GroupLayout jPanel1Layout = new javax.swing.GroupLayout(jPanel1);
        jPanel1.setLayout(jPanel1Layout);
        jPanel1Layout.setHorizontalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel1Layout.createSequentialGroup()
                .addGap(6, 6, 6)
                .addComponent(jLabel1)
                .addGap(6, 6, 6)
                .addComponent(ObjectNameTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 203, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jPanel2, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(GetCoordinatesButton)
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );
        jPanel1Layout.setVerticalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel1Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(GetCoordinatesButton)
                    .addComponent(jPanel2, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(jLabel1)
                        .addComponent(ObjectNameTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)))
                .addGap(0, 15, Short.MAX_VALUE))
        );

        jLabel2.setFont(new java.awt.Font("Century Schoolbook L", 1, 18)); // NOI18N
        jLabel2.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel2.setText("Palomar Observatory Hale 200\" Telescope Simulator");

        jPanel3.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));

        jLabel3.setText("RA");

        jLabel4.setText("DEC");

        TargetRATextField.setEditable(false);
        TargetRATextField.setBackground(new java.awt.Color(0, 0, 0));
        TargetRATextField.setFont(new java.awt.Font("Copperplate Gothic Bold", 3, 24)); // NOI18N
        TargetRATextField.setForeground(new java.awt.Color(0, 204, 51));
        TargetRATextField.setHorizontalAlignment(javax.swing.JTextField.CENTER);
        TargetRATextField.setText("00:00:00.0");
        TargetRATextField.setBorder(null);
        TargetRATextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                TargetRATextFieldActionPerformed(evt);
            }
        });

        TargetDECTextField.setEditable(false);
        TargetDECTextField.setBackground(new java.awt.Color(0, 0, 0));
        TargetDECTextField.setFont(new java.awt.Font("Copperplate Gothic Bold", 3, 24)); // NOI18N
        TargetDECTextField.setForeground(new java.awt.Color(0, 204, 51));
        TargetDECTextField.setHorizontalAlignment(javax.swing.JTextField.CENTER);
        TargetDECTextField.setText("00:00:00.0");
        TargetDECTextField.setBorder(null);
        TargetDECTextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                TargetDECTextFieldActionPerformed(evt);
            }
        });

        RADecimalTextField.setEditable(false);
        RADecimalTextField.setBackground(new java.awt.Color(0, 0, 0));
        RADecimalTextField.setFont(new java.awt.Font("Copperplate Gothic Bold", 3, 24)); // NOI18N
        RADecimalTextField.setForeground(new java.awt.Color(0, 204, 51));
        RADecimalTextField.setHorizontalAlignment(javax.swing.JTextField.CENTER);
        RADecimalTextField.setText("0");
        RADecimalTextField.setBorder(null);
        RADecimalTextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                RADecimalTextFieldActionPerformed(evt);
            }
        });

        DECDecimalTextField.setEditable(false);
        DECDecimalTextField.setBackground(new java.awt.Color(0, 0, 0));
        DECDecimalTextField.setFont(new java.awt.Font("Copperplate Gothic Bold", 3, 24)); // NOI18N
        DECDecimalTextField.setForeground(new java.awt.Color(0, 204, 51));
        DECDecimalTextField.setHorizontalAlignment(javax.swing.JTextField.CENTER);
        DECDecimalTextField.setText("0");
        DECDecimalTextField.setBorder(null);
        DECDecimalTextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                DECDecimalTextFieldActionPerformed(evt);
            }
        });

        jLabel5.setText("Cass Angle");

        CassAngleTextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                CassAngleTextFieldActionPerformed(evt);
            }
        });

        javax.swing.GroupLayout jPanel3Layout = new javax.swing.GroupLayout(jPanel3);
        jPanel3.setLayout(jPanel3Layout);
        jPanel3Layout.setHorizontalGroup(
            jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel3Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(jLabel3)
                    .addComponent(jLabel4))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(TargetRATextField, javax.swing.GroupLayout.PREFERRED_SIZE, 178, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(TargetDECTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 180, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel3Layout.createSequentialGroup()
                        .addComponent(RADecimalTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 178, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addGap(18, 18, 18)
                        .addComponent(jLabel5))
                    .addComponent(DECDecimalTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 180, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addComponent(CassAngleTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 93, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap())
        );
        jPanel3Layout.setVerticalGroup(
            jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel3Layout.createSequentialGroup()
                .addGap(17, 17, 17)
                .addGroup(jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                    .addGroup(jPanel3Layout.createSequentialGroup()
                        .addGroup(jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(TargetRATextField, javax.swing.GroupLayout.PREFERRED_SIZE, 30, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(RADecimalTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 30, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(jLabel3)
                            .addComponent(jLabel5)
                            .addComponent(CassAngleTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                        .addGap(27, 27, 27))
                    .addGroup(jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(jLabel4)
                        .addComponent(TargetDECTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 30, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addComponent(DECDecimalTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 30, javax.swing.GroupLayout.PREFERRED_SIZE)))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        jPanel4.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));

        jScrollPane1.setVerticalScrollBarPolicy(javax.swing.ScrollPaneConstants.VERTICAL_SCROLLBAR_ALWAYS);

        commandsTextArea.setColumns(20);
        commandsTextArea.setRows(5);
        jScrollPane1.setViewportView(commandsTextArea);

        jScrollPane2.setVerticalScrollBarPolicy(javax.swing.ScrollPaneConstants.VERTICAL_SCROLLBAR_ALWAYS);

        errorsTextArea.setColumns(20);
        errorsTextArea.setRows(5);
        jScrollPane2.setViewportView(errorsTextArea);

        jScrollPane3.setVerticalScrollBarPolicy(javax.swing.ScrollPaneConstants.VERTICAL_SCROLLBAR_ALWAYS);

        responseTextArea.setColumns(20);
        responseTextArea.setRows(5);
        jScrollPane3.setViewportView(responseTextArea);

        jLabel6.setFont(new java.awt.Font("Century Schoolbook L", 1, 14)); // NOI18N
        jLabel6.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel6.setText("COMMANDS RECEIVED");

        jLabel7.setFont(new java.awt.Font("Century Schoolbook L", 1, 14)); // NOI18N
        jLabel7.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel7.setText("RESPONSES SEND");

        jLabel8.setFont(new java.awt.Font("Century Schoolbook L", 1, 14)); // NOI18N
        jLabel8.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel8.setText("ERRORS");

        javax.swing.GroupLayout jPanel4Layout = new javax.swing.GroupLayout(jPanel4);
        jPanel4.setLayout(jPanel4Layout);
        jPanel4Layout.setHorizontalGroup(
            jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel4Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(jLabel7, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(jLabel8, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addGroup(jPanel4Layout.createSequentialGroup()
                        .addGroup(jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(jLabel6, javax.swing.GroupLayout.PREFERRED_SIZE, 641, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(jScrollPane1, javax.swing.GroupLayout.PREFERRED_SIZE, 609, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(jScrollPane3, javax.swing.GroupLayout.PREFERRED_SIZE, 609, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(jScrollPane2, javax.swing.GroupLayout.PREFERRED_SIZE, 613, javax.swing.GroupLayout.PREFERRED_SIZE))
                        .addGap(0, 0, Short.MAX_VALUE)))
                .addContainerGap())
        );
        jPanel4Layout.setVerticalGroup(
            jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel4Layout.createSequentialGroup()
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addComponent(jLabel6)
                .addGap(9, 9, 9)
                .addComponent(jScrollPane1, javax.swing.GroupLayout.PREFERRED_SIZE, 119, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jLabel7)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jScrollPane3, javax.swing.GroupLayout.PREFERRED_SIZE, 129, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jLabel8)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jScrollPane2, javax.swing.GroupLayout.PREFERRED_SIZE, 119, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addGap(30, 30, 30))
        );

        javax.swing.GroupLayout mainPanelLayout = new javax.swing.GroupLayout(mainPanel);
        mainPanel.setLayout(mainPanelLayout);
        mainPanelLayout.setHorizontalGroup(
            mainPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(mainPanelLayout.createSequentialGroup()
                .addGroup(mainPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING, false)
                    .addGroup(javax.swing.GroupLayout.Alignment.LEADING, mainPanelLayout.createSequentialGroup()
                        .addContainerGap()
                        .addComponent(jLabel2, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                    .addComponent(jPanel4, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.PREFERRED_SIZE, 0, Short.MAX_VALUE)
                    .addComponent(jPanel3, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(jPanel1, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addGap(0, 9, Short.MAX_VALUE))
        );
        mainPanelLayout.setVerticalGroup(
            mainPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(mainPanelLayout.createSequentialGroup()
                .addGap(17, 17, 17)
                .addComponent(jLabel2)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jPanel1, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jPanel3, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jPanel4, javax.swing.GroupLayout.PREFERRED_SIZE, 470, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(mainPanel, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(mainPanel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents

    private void TargetRATextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_TargetRATextFieldActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_TargetRATextFieldActionPerformed

    private void TargetDECTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_TargetDECTextFieldActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_TargetDECTextFieldActionPerformed

    private void RADecimalTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_RADecimalTextFieldActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_RADecimalTextFieldActionPerformed

    private void DECDecimalTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_DECDecimalTextFieldActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_DECDecimalTextFieldActionPerformed

    private void ObjectNameTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_ObjectNameTextFieldActionPerformed
       java.lang.String text = ObjectNameTextField.getText();        
       myTelescopeObject.setObjectName(text.trim());
    }//GEN-LAST:event_ObjectNameTextFieldActionPerformed

    private void SIMBADRadioButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_SIMBADRadioButtonActionPerformed
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
    }//GEN-LAST:event_SIMBADRadioButtonActionPerformed

    private void NEDRadioButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_NEDRadioButtonActionPerformed
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
           }        // TODO add your handling code here:
    }//GEN-LAST:event_NEDRadioButtonActionPerformed

    private void GetCoordinatesButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_GetCoordinatesButtonActionPerformed
        try{
         java.lang.String currentObjectName = ObjectNameTextField.getText();
         myHTMIndexSearchClient.setObjectName(currentObjectName);
         double[]  coordinateArray = myHTMIndexSearchClient.resolveObject(currentObjectName);
         myHTMIndexSearchClient.setRA(coordinateArray[0]);
         myHTMIndexSearchClient.setDEC(coordinateArray[1]);
         setRightAscension(coordinateArray[0]);
         setDeclination(coordinateArray[1]);
         RADecimalTextField.setText(Printf.format("%08.5f", new PrintfData().add(coordinateArray[0])));
         DECDecimalTextField.setText(Printf.format("%08.5f", new PrintfData().add(coordinateArray[1])));
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
       }catch(Exception e2){
           java.lang.String myErrorString = new java.lang.String();
           myErrorString = "A problem occured while executing the Get Coordinates method: " + e2;
           logMessage(myErrorString);
           ObjectNameTextField.setText("");
           TargetRATextField.setText("");
           RADecimalTextField.setText("0.0");
           TargetDECTextField.setText("");
           DECDecimalTextField.setText("0.0");
           myHTMIndexSearchClient.setRA(0.0);
           myHTMIndexSearchClient.setDEC(0.0);
       }
       // TODO add your handling code here:
    }//GEN-LAST:event_GetCoordinatesButtonActionPerformed

    private void CassAngleTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_CassAngleTextFieldActionPerformed
        try{
           double value = Double.parseDouble(CassAngleTextField.getText());
           myTelescopeObject.setCASS_RING_ANGLE(value);
           myTelescopeObject.setCassRingAngle(Printf.format("%08.5f", new PrintfData().add(value)));
        }catch(Exception e){
            logMessage(e.toString());
        }
    }//GEN-LAST:event_CassAngleTextFieldActionPerformed
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
            java.util.logging.Logger.getLogger(P200SimulatorFrame2.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (InstantiationException ex) {
            java.util.logging.Logger.getLogger(P200SimulatorFrame2.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (IllegalAccessException ex) {
            java.util.logging.Logger.getLogger(P200SimulatorFrame2.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (javax.swing.UnsupportedLookAndFeelException ex) {
            java.util.logging.Logger.getLogger(P200SimulatorFrame2.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        }
        //</editor-fold>

        /* Create and display the form */
        java.awt.EventQueue.invokeLater(new Runnable() {
            public void run() {
                new P200SimulatorFrame2().setVisible(true);
            }
        });
    }

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JTextField CassAngleTextField;
    private javax.swing.JTextField DECDecimalTextField;
    private javax.swing.JButton GetCoordinatesButton;
    private javax.swing.JRadioButton NEDRadioButton;
    private javax.swing.JTextField ObjectNameTextField;
    private javax.swing.JTextField RADecimalTextField;
    private javax.swing.JRadioButton SIMBADRadioButton;
    private javax.swing.JTextField TargetDECTextField;
    private javax.swing.JTextField TargetRATextField;
    private javax.swing.JTextArea commandsTextArea;
    private javax.swing.JTextArea errorsTextArea;
    private javax.swing.JLabel jLabel1;
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
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JScrollPane jScrollPane2;
    private javax.swing.JScrollPane jScrollPane3;
    private javax.swing.JPanel mainPanel;
    private javax.swing.JTextArea responseTextArea;
    // End of variables declaration//GEN-END:variables
}
