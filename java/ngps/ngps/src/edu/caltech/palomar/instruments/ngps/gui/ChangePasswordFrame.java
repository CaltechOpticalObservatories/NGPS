/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/GUIForms/JFrame.java to edit this template
 */
package edu.caltech.palomar.instruments.ngps.gui;


import edu.caltech.palomar.instruments.ngps.dbms.NGPSdatabase;
import edu.caltech.palomar.instruments.ngps.object.Owner;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.beans.PropertyChangeSupport;
import javax.swing.JFrame;
/**
 *
 * @author developer
 */
public class ChangePasswordFrame extends javax.swing.JFrame {
  transient protected PropertyChangeSupport propertyChangeListeners = new PropertyChangeSupport(this);
  public NGPSdatabase dbms;
  private java.lang.String message;
    /**
     * Creates new form ChangePasswordFrame
     */
    public ChangePasswordFrame() {
        initComponents();
        this.setVisible(false);
        setDefaultCloseOperation(JFrame.HIDE_ON_CLOSE);
    }
/*=============================================================================================
/     setDBMS(NGPSdatabase new_dbms)
/=============================================================================================*/
    public void setDBMS(NGPSdatabase new_dbms){
       this.dbms = new_dbms; 
       dbms.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
              public void propertyChange(java.beans.PropertyChangeEvent e) {
                 dbms_propertyChange(e);
              }
            });
       this.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
              public void propertyChange(java.beans.PropertyChangeEvent e) {
                 message_propertyChange(e);
              }
            });
    }
 /*=============================================================================================
/     Property Change Listener for the Ephemeris_propertyChange
/=============================================================================================*/
  private void message_propertyChange(PropertyChangeEvent e)  {
     java.lang.String propertyName = e.getPropertyName();
     System.out.println("message_propertyChange "+propertyName);
    if(propertyName.matches("message")){
        java.lang.String current_value = (java.lang.String)e.getNewValue();
        messageLabel.setText(current_value);
    }
}      
/*=============================================================================================
/     Property Change Listener for the Ephemeris_propertyChange
/=============================================================================================*/
  private void dbms_propertyChange(PropertyChangeEvent e)  {
     java.lang.String propertyName = e.getPropertyName();
     System.out.println("passwordFrame: "+propertyName);
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
    if(propertyName.matches("owner")){
        java.lang.String current_value = (java.lang.String)e.getNewValue();
        Owner_ID_label.setText(current_value);
    }
}    
/*================================================================================================
/      validatePasswords()
/=================================================================================================*/
     private void validatePasswords(){
       int logged_in_state = dbms.getLoggedInState();
       if(logged_in_state == NGPSdatabase.LOGIN_SUCCESSFUL){
           dbms.setLoggedInState(NGPSdatabase.NOT_LOGGED_IN);
//           OwnerList.setEnabled(true);
//           frame.getAccountMenu().setText("Not Logged In");
       }
       if(logged_in_state == NGPSdatabase.NOT_LOGGED_IN | logged_in_state == NGPSdatabase.LOGIN_UNSUCCESSFUL){
          java.lang.String submitted_password = new java.lang.String(PasswordField.getPassword());
          sign_in(submitted_password);          
          PasswordField.setText("");           
       }  
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
 /*================================================================================================
/      setOWNER(java.lang.String new_owner)
/=================================================================================================*/
  public void setMessage(java.lang.String new_message) {
    java.lang.String  old_message = this.message;
    this.message = new_message;
   propertyChangeListeners.firePropertyChange("message", (old_message), (new_message));
  }
  public java.lang.String getMessage() {
    return message;
  }      
/*=============================================================================================
/     setDBMS(NGPS_Database new_dbms)
/=============================================================================================*/
public boolean sign_in(java.lang.String submitted_password){
    boolean matched = false;
    java.lang.String password = new java.lang.String();
    Owner matching_owner = new Owner();
    java.lang.String selected_owner = dbms.getOWNER();
    int rows = dbms.myOwnerTableModel.getRowCount();
    for(int i=0;i<rows;i++){
       Owner current = (Owner)dbms.myOwnerTableModel.getRecord(i);
       java.lang.String current_owner = current.getOwner_ID();
       if(current_owner.matches(selected_owner)){
           matching_owner = current;
       }
    }
    if(matching_owner != null){
       try{
          java.lang.String encrypted_password_stored = matching_owner.getEncryptedPassword();
          java.lang.String encrypted_password        = dbms.encrypt(submitted_password,dbms.originalKey); 
          boolean          compare                   = java.security.MessageDigest.isEqual(encrypted_password_stored.getBytes(),encrypted_password.getBytes()) ;
          if(compare){
              matched = true;
              dbms.setOWNER(matching_owner.getOwner_ID());
              dbms.setOWNER_OBJECT(matching_owner);
              dbms.setLoggedIn(true);
              dbms.setLoggedInState(NGPSdatabase.LOGIN_SUCCESSFUL);
          }
          if(!compare){
              dbms.setLoggedIn(false);
              dbms.setLoggedInState(NGPSdatabase.LOGIN_UNSUCCESSFUL);
              matched = false;
          }         
       }catch(Exception e){
           dbms.setLoggedIn(false);
           dbms.setLoggedInState(NGPSdatabase.NOT_LOGGED_IN);
           System.out.println(e.toString());
           matched = false;
       }
    }
    return matched;
}
    /**
     * This method is called from within the constructor to initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is always
     * regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        PasswordField = new javax.swing.JPasswordField();
        Owner_ID_label3 = new javax.swing.JLabel();
        Owner_ID_label4 = new javax.swing.JLabel();
        Owner_ID_label5 = new javax.swing.JLabel();
        newPasswordField = new javax.swing.JPasswordField();
        new_confirmPasswordField = new javax.swing.JPasswordField();
        Owner_ID_label6 = new javax.swing.JLabel();
        Owner_ID_label = new javax.swing.JLabel();
        OKButton = new javax.swing.JButton();
        messageLabel = new javax.swing.JLabel();
        ChangePasswordButton = new javax.swing.JButton();

        setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);

        PasswordField.setText("password");
        PasswordField.setMinimumSize(new java.awt.Dimension(11, 400));

        Owner_ID_label3.setText("CONFIRM NEW PASSWORD");

        Owner_ID_label4.setText("PASSWORD");

        Owner_ID_label5.setText("NEW PASSWORD");

        newPasswordField.setText("password");
        newPasswordField.setMinimumSize(new java.awt.Dimension(11, 400));

        new_confirmPasswordField.setText("password");
        new_confirmPasswordField.setMinimumSize(new java.awt.Dimension(11, 400));

        Owner_ID_label6.setText("LOGGED IN AS:  ");

        Owner_ID_label.setForeground(new java.awt.Color(10, 171, 19));

        OKButton.setText("OK");
        OKButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                OKButtonActionPerformed(evt);
            }
        });

        ChangePasswordButton.setText("CHANGE PASSWORD");
        ChangePasswordButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                ChangePasswordButtonActionPerformed(evt);
            }
        });

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                    .addComponent(messageLabel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                        .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                            .addGroup(layout.createSequentialGroup()
                                .addComponent(Owner_ID_label3)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(new_confirmPasswordField, javax.swing.GroupLayout.PREFERRED_SIZE, 170, javax.swing.GroupLayout.PREFERRED_SIZE))
                            .addGroup(layout.createSequentialGroup()
                                .addComponent(Owner_ID_label5)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                                .addComponent(newPasswordField, javax.swing.GroupLayout.PREFERRED_SIZE, 170, javax.swing.GroupLayout.PREFERRED_SIZE)))
                        .addGroup(layout.createSequentialGroup()
                            .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, layout.createSequentialGroup()
                                    .addComponent(Owner_ID_label4)
                                    .addGap(117, 117, 117))
                                .addGroup(layout.createSequentialGroup()
                                    .addComponent(Owner_ID_label6)
                                    .addGap(87, 87, 87)))
                            .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                                .addComponent(PasswordField, javax.swing.GroupLayout.DEFAULT_SIZE, 170, Short.MAX_VALUE)
                                .addComponent(Owner_ID_label, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))))
                    .addGroup(layout.createSequentialGroup()
                        .addComponent(ChangePasswordButton, javax.swing.GroupLayout.PREFERRED_SIZE, 202, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(OKButton)))
                .addGap(0, 24, Short.MAX_VALUE))
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addGap(24, 24, 24)
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                    .addComponent(Owner_ID_label6, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(Owner_ID_label, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(Owner_ID_label4)
                    .addComponent(PasswordField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(Owner_ID_label5)
                    .addComponent(newPasswordField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(Owner_ID_label3)
                    .addComponent(new_confirmPasswordField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(messageLabel, javax.swing.GroupLayout.DEFAULT_SIZE, 20, Short.MAX_VALUE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(OKButton)
                    .addComponent(ChangePasswordButton))
                .addContainerGap())
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents

    private void OKButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_OKButtonActionPerformed
       setVisible(false);
    }//GEN-LAST:event_OKButtonActionPerformed

    private void ChangePasswordButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_ChangePasswordButtonActionPerformed
       char[] new_password_array         = this.newPasswordField.getPassword();
       char[] confirm_password_array     = this.new_confirmPasswordField.getPassword();
       java.lang.String new_password     = String.valueOf(new_password_array);
       java.lang.String confirm_password = String.valueOf(confirm_password_array);
       boolean isSame = new_password.matches(confirm_password);
       System.out.println(isSame);
       char[] current_password_array     = this.PasswordField.getPassword();
       java.lang.String current_password = String.valueOf(current_password_array);
       boolean state = sign_in(current_password);
       if(!isSame){
           setMessage("New passwords do not match");
       }
       if(!state){
           setMessage("Current password is incorrect");
       }
       if(isSame){
           if(state){
               try{
                   setMessage("Changing password");
                   java.lang.String encrypted_password = dbms.encrypt(new_password,dbms.originalKey); 
                   dbms.current_owner.setPassword(new_password);
                   dbms.current_owner.setEncryptedPassword(encrypted_password);
                   dbms.executeUpdateOwnerPreparedStatement(dbms.current_owner);
                   setMessage("Password Changed");
               }catch(Exception e){
                  System.out.println(e.toString());
               }              
           }
       }
    }//GEN-LAST:event_ChangePasswordButtonActionPerformed

    /**
     * @param args the command line arguments= 
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
            java.util.logging.Logger.getLogger(ChangePasswordFrame.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (InstantiationException ex) {
            java.util.logging.Logger.getLogger(ChangePasswordFrame.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (IllegalAccessException ex) {
            java.util.logging.Logger.getLogger(ChangePasswordFrame.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (javax.swing.UnsupportedLookAndFeelException ex) {
            java.util.logging.Logger.getLogger(ChangePasswordFrame.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        }
        //</editor-fold>

        /* Create and display the form */
        java.awt.EventQueue.invokeLater(new Runnable() {
            public void run() {
                new ChangePasswordFrame().setVisible(true);
            }
        });
    }

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton ChangePasswordButton;
    private javax.swing.JButton OKButton;
    private javax.swing.JLabel Owner_ID_label;
    private javax.swing.JLabel Owner_ID_label3;
    private javax.swing.JLabel Owner_ID_label4;
    private javax.swing.JLabel Owner_ID_label5;
    private javax.swing.JLabel Owner_ID_label6;
    private javax.swing.JPasswordField PasswordField;
    private javax.swing.JLabel messageLabel;
    private javax.swing.JPasswordField newPasswordField;
    private javax.swing.JPasswordField new_confirmPasswordField;
    // End of variables declaration//GEN-END:variables
}
