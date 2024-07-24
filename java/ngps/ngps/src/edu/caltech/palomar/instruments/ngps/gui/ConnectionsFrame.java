/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/GUIForms/JFrame.java to edit this template
 */
package edu.caltech.palomar.instruments.ngps.gui;

import edu.caltech.palomar.instruments.ngps.dbms.NGPSdatabase;
import edu.caltech.palomar.instruments.ngps.os.ObservationSequencerController;
import edu.caltech.palomar.io.ClientSocket;
import java.awt.Color;
import java.beans.PropertyChangeEvent;
import javax.swing.ImageIcon;
import javax.swing.JFrame;
/**
 *
 * @author developer
 */
public class ConnectionsFrame extends javax.swing.JFrame {
  public  NGPSdatabase                   dbms;
  public  ObservationSequencerController myObservationSequencerController;
  private ImageIcon                      ON;
  private ImageIcon                      OFF;
  private ImageIcon                      UNKNOWN;
  public java.lang.String                USERDIR         = System.getProperty("user.dir");
  public java.lang.String                SEP             = System.getProperty("file.separator");
  public java.lang.String                CONFIG          = new java.lang.String("config");
  public java.lang.String                IMAGE_CACHE     = new java.lang.String("images");
  private java.lang.String               tcs_source      = new java.lang.String();
  /**
     * Creates new form ConnectionsFrame
     */
    public ConnectionsFrame() {
        initComponents();
        initializeIcons();
        setDefaultCloseOperation(JFrame.HIDE_ON_CLOSE);
   }
/*================================================================================================
/        initializeIcons()
/=================================================================================================*/
  public void initializeIcons(){
    ON                   = new ImageIcon(USERDIR + SEP + IMAGE_CACHE + SEP + "ON.gif");
    OFF                  = new ImageIcon(USERDIR + SEP + IMAGE_CACHE + SEP + "OFF.gif");
    UNKNOWN              = new ImageIcon(USERDIR + SEP + IMAGE_CACHE + SEP + "UNKNOWN.gif");
  }  
public void setTCS(java.lang.String new_tcs_source){
    tcs_source = new_tcs_source;
}
/*================================================================================================
/        initializeIcons()
/=================================================================================================*/    
public void setDBMS(NGPSdatabase new_dbms){
   dbms = new_dbms;
   dbms.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
   public void propertyChange(java.beans.PropertyChangeEvent e) {
     dbms_propertyChange(e);
     }
   });
   if(dbms.isConnected()){
       dbmsButton.setIcon(ON);
   }
}
/*=============================================================================================
/     Property Change Listener for the Ephemeris_propertyChange
/=============================================================================================*/
  private void dbms_propertyChange(PropertyChangeEvent e)  {
     java.lang.String propertyName = e.getPropertyName();
     System.out.println("connectionsFrame: "+propertyName);
    if(propertyName.matches("connected")){
        java.lang.Boolean current_value = (java.lang.Boolean)e.getNewValue(); 
        if(current_value){
            this.dbmsButton.setIcon(ON);
        }else if(!current_value){
            this.dbmsButton.setIcon(OFF);
        }
    }
} 
/*=============================================================================================
/     initialize()
/=============================================================================================*/ 
    public void setObservationSequencercontroller(ObservationSequencerController newObservationSequencerController) {
    myObservationSequencerController = newObservationSequencerController;
    myObservationSequencerController.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
    public void propertyChange(java.beans.PropertyChangeEvent e) {
             TCS_propertyChange(e);
          }
     });  
     myObservationSequencerController.myBlockingSocket.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
        public void propertyChange(java.beans.PropertyChangeEvent e) {
            OSCommandSocket_propertyChange(e);
         }
        });
     myObservationSequencerController.myCommandSocket.addPropertyChangeListener((java.beans.PropertyChangeEvent e) -> {
         OSCommandSocket_propertyChange(e);
    });

     if(myObservationSequencerController.myBlockingSocket.isConnected()){ // CHAZ change to state check?
         this.OSButton.setIcon(ON);
     }
     myObservationSequencerController.query_tcs_connection();
     tcs_buttonGroup.add(this.simulatorRadioButton);
     tcs_buttonGroup.add(this.P200RadioButton);
}
/*=============================================================================================
/     Property Change Listener for the Ephemeris_propertyChange
/=============================================================================================*/
  private void TCS_propertyChange(PropertyChangeEvent e)  {
     java.lang.String propertyName = e.getPropertyName();
     System.out.println("TCS_propertyChange"+propertyName);
     if(propertyName.matches("tcs_connected")){
         boolean state = (java.lang.Boolean)e.getNewValue();
         if(state){
           TCSButton.setIcon(ON);
         }else{
           TCSButton.setIcon(OFF); 
         }
     }
     if(propertyName.matches("tcs_connected_in_progress")){
        boolean state = (java.lang.Boolean)e.getNewValue();
          if(state){
           TCSButton.setEnabled(false);
         }else{
           TCSButton.setEnabled(true);
         }       
     }
     if(propertyName.matches("active_tcs_address")){
         java.lang.String value = (java.lang.String)e.getNewValue();
         tcs_ip_Label.setText(value);   
         if(value.contains("sim")){
           simulatorRadioButton.setSelected(true);
         }
         if(value.contains("real")){
           P200RadioButton.setSelected(true);
         }
     }
     if(propertyName.matches("active_tcs_name")){
         java.lang.String value = (java.lang.String)e.getNewValue();
         tcs_name_Label.setText(value);
     }
} 
/*=============================================================================================
/     Property Change Listener for the _propertyChange
/=============================================================================================*/
  private void OSCommandSocket_propertyChange(PropertyChangeEvent e)  {
     java.lang.String propertyName = e.getPropertyName();
       if(propertyName == "connected"){
        boolean state = (java.lang.Boolean)e.getNewValue();
        if(state){   
            OSButton.setIcon(ON);
        }else{
            OSButton.setIcon(OFF);
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

        tcs_buttonGroup = new javax.swing.ButtonGroup();
        jLabel1 = new javax.swing.JLabel();
        jPanel1 = new javax.swing.JPanel();
        dbmsButton = new javax.swing.JButton();
        jLabel2 = new javax.swing.JLabel();
        OSButton = new javax.swing.JButton();
        jLabel3 = new javax.swing.JLabel();
        jPanel2 = new javax.swing.JPanel();
        TCSButton = new javax.swing.JButton();
        jLabel4 = new javax.swing.JLabel();
        jLabel6 = new javax.swing.JLabel();
        jLabel5 = new javax.swing.JLabel();
        tcs_name_Label = new javax.swing.JLabel();
        tcs_ip_Label = new javax.swing.JLabel();
        simulatorRadioButton = new javax.swing.JRadioButton();
        P200RadioButton = new javax.swing.JRadioButton();

        setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);
        getContentPane().setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        jLabel1.setFont(new java.awt.Font("Cantarell", 1, 15)); // NOI18N
        jLabel1.setText("CONNECTIONS");
        getContentPane().add(jLabel1, new org.netbeans.lib.awtextra.AbsoluteConstraints(170, 10, -1, -1));

        jPanel1.setBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 0, 0), 2, true));

        dbmsButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/edu/caltech/palomar/instruments/ngps/gui/OFF.png"))); // NOI18N
        dbmsButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                dbmsButtonActionPerformed(evt);
            }
        });

        jLabel2.setText("mySQL Database");

        OSButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/edu/caltech/palomar/instruments/ngps/gui/OFF.png"))); // NOI18N
        OSButton.addChangeListener(new javax.swing.event.ChangeListener() {
            public void stateChanged(javax.swing.event.ChangeEvent evt) {
                OSButtonStateChanged(evt);
            }
        });
        OSButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                OSButtonActionPerformed(evt);
            }
        });

        jLabel3.setText("Observation Sequencer");

        javax.swing.GroupLayout jPanel1Layout = new javax.swing.GroupLayout(jPanel1);
        jPanel1.setLayout(jPanel1Layout);
        jPanel1Layout.setHorizontalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel1Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel1Layout.createSequentialGroup()
                        .addComponent(dbmsButton)
                        .addGap(12, 12, 12)
                        .addComponent(jLabel2))
                    .addGroup(jPanel1Layout.createSequentialGroup()
                        .addComponent(OSButton)
                        .addGap(8, 8, 8)
                        .addComponent(jLabel3)))
                .addContainerGap(234, Short.MAX_VALUE))
        );
        jPanel1Layout.setVerticalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel1Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(dbmsButton)
                    .addGroup(jPanel1Layout.createSequentialGroup()
                        .addGap(8, 8, 8)
                        .addComponent(jLabel2)))
                .addGap(2, 2, 2)
                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(OSButton)
                    .addGroup(jPanel1Layout.createSequentialGroup()
                        .addGap(10, 10, 10)
                        .addComponent(jLabel3)))
                .addContainerGap(18, Short.MAX_VALUE))
        );

        getContentPane().add(jPanel1, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 30, 450, -1));

        jPanel2.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0), 2));

        TCSButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/edu/caltech/palomar/instruments/ngps/gui/OFF.png"))); // NOI18N
        TCSButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                TCSButtonActionPerformed(evt);
            }
        });

        jLabel4.setText("TCS");

        jLabel6.setText("Connected to:");

        jLabel5.setText("IP Address:");

        tcs_name_Label.setFont(new java.awt.Font("Cantarell", 1, 15)); // NOI18N
        tcs_name_Label.setForeground(new java.awt.Color(13, 157, 23));

        tcs_ip_Label.setFont(new java.awt.Font("Cantarell", 1, 15)); // NOI18N
        tcs_ip_Label.setForeground(new java.awt.Color(13, 157, 23));

        simulatorRadioButton.setText("Simulator");
        simulatorRadioButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                simulatorRadioButtonActionPerformed(evt);
            }
        });

        P200RadioButton.setText("P200");
        P200RadioButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                P200RadioButtonActionPerformed(evt);
            }
        });

        javax.swing.GroupLayout jPanel2Layout = new javax.swing.GroupLayout(jPanel2);
        jPanel2.setLayout(jPanel2Layout);
        jPanel2Layout.setHorizontalGroup(
            jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel2Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(TCSButton)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(jLabel4)
                    .addComponent(simulatorRadioButton)
                    .addComponent(P200RadioButton))
                .addGap(30, 30, 30)
                .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(jLabel6)
                    .addComponent(jLabel5))
                .addGap(10, 10, 10)
                .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(tcs_name_Label, javax.swing.GroupLayout.PREFERRED_SIZE, 70, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(tcs_ip_Label, javax.swing.GroupLayout.PREFERRED_SIZE, 130, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(33, Short.MAX_VALUE))
        );
        jPanel2Layout.setVerticalGroup(
            jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel2Layout.createSequentialGroup()
                .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel2Layout.createSequentialGroup()
                        .addContainerGap()
                        .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(TCSButton)
                            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel2Layout.createSequentialGroup()
                                .addComponent(jLabel4)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)))
                        .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(simulatorRadioButton)
                            .addGroup(jPanel2Layout.createSequentialGroup()
                                .addGap(20, 20, 20)
                                .addComponent(P200RadioButton))))
                    .addGroup(jPanel2Layout.createSequentialGroup()
                        .addGap(22, 22, 22)
                        .addComponent(jLabel6)
                        .addGap(10, 10, 10)
                        .addComponent(jLabel5))
                    .addGroup(jPanel2Layout.createSequentialGroup()
                        .addGap(22, 22, 22)
                        .addComponent(tcs_name_Label, javax.swing.GroupLayout.PREFERRED_SIZE, 20, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addGap(10, 10, 10)
                        .addComponent(tcs_ip_Label, javax.swing.GroupLayout.PREFERRED_SIZE, 20, javax.swing.GroupLayout.PREFERRED_SIZE)))
                .addContainerGap(14, Short.MAX_VALUE))
        );

        getContentPane().add(jPanel2, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 140, 450, -1));

        pack();
    }// </editor-fold>//GEN-END:initComponents

    private void dbmsButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_dbmsButtonActionPerformed
        dbms.initializeDBMS();
    }//GEN-LAST:event_dbmsButtonActionPerformed

    private void OSButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_OSButtonActionPerformed
       myObservationSequencerController.connect();
    }//GEN-LAST:event_OSButtonActionPerformed

    private void TCSButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_TCSButtonActionPerformed
       if(simulatorRadioButton.isSelected()){
           myObservationSequencerController.setTCSConnectedInProgress(true);
           myObservationSequencerController.tcsinit("sim");
       }
       if(P200RadioButton.isSelected()){
           myObservationSequencerController.setTCSConnectedInProgress(true);
           myObservationSequencerController.tcsinit("real");
       }
    }//GEN-LAST:event_TCSButtonActionPerformed

    private void simulatorRadioButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_simulatorRadioButtonActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_simulatorRadioButtonActionPerformed

    private void P200RadioButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_P200RadioButtonActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_P200RadioButtonActionPerformed

    private void OSButtonStateChanged(javax.swing.event.ChangeEvent evt) {//GEN-FIRST:event_OSButtonStateChanged
        // TODO add your handling code here:
    }//GEN-LAST:event_OSButtonStateChanged

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
            java.util.logging.Logger.getLogger(ConnectionsFrame.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (InstantiationException ex) {
            java.util.logging.Logger.getLogger(ConnectionsFrame.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (IllegalAccessException ex) {
            java.util.logging.Logger.getLogger(ConnectionsFrame.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (javax.swing.UnsupportedLookAndFeelException ex) {
            java.util.logging.Logger.getLogger(ConnectionsFrame.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        }
        //</editor-fold>

        /* Create and display the form */
        java.awt.EventQueue.invokeLater(new Runnable() {
            public void run() {
                new ConnectionsFrame().setVisible(true);
            }
        });
    }

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton OSButton;
    private javax.swing.JRadioButton P200RadioButton;
    private javax.swing.JButton TCSButton;
    private javax.swing.JButton dbmsButton;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JLabel jLabel3;
    private javax.swing.JLabel jLabel4;
    private javax.swing.JLabel jLabel5;
    private javax.swing.JLabel jLabel6;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JPanel jPanel2;
    private javax.swing.JRadioButton simulatorRadioButton;
    private javax.swing.ButtonGroup tcs_buttonGroup;
    private javax.swing.JLabel tcs_ip_Label;
    private javax.swing.JLabel tcs_name_Label;
    // End of variables declaration//GEN-END:variables
}
