/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/GUIForms/JFrame.java to edit this template
 */
package edu.caltech.palomar.instruments.ngps.gui;

import edu.caltech.palomar.instruments.ngps.dbms.NGPSdatabase;
import edu.caltech.palomar.instruments.ngps.object.Owner;
import edu.caltech.palomar.instruments.ngps.tables.OwnerTableModel;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Toolkit;
import java.awt.Window;
import java.beans.PropertyChangeEvent;
import javax.swing.ImageIcon;
import javax.swing.JFrame;
import javax.swing.JTable;
import javax.swing.ListSelectionModel;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;

/**
 *
 * @author developer
 */
public class signInFrame2 extends javax.swing.JFrame {
    public   java.lang.String      USERDIR           = System.getProperty("user.dir");
    public   java.lang.String      SEP               = System.getProperty("file.separator");
    public   java.lang.String      IMAGE_CACHE       = new java.lang.String(SEP + "images" + SEP);
    public  NGPSdatabase           dbms;
    public  NGPSFrame              frame;
    private boolean                ALLOW_ROW_SELECTION = true;
    public static int              SUCCESS = 1;
    public static int              WRONG_PASSWORD = 3;
    public   JTable                ownersTable; 

    /**
     * Creates new form signInFrame2
     */
    public signInFrame2() {
        initComponents();
         setDefaultCloseOperation(JFrame.HIDE_ON_CLOSE);
        centreWindow(this);
        initialize();
   }
/*=============================================================================================
/     initialize()
/=============================================================================================*/
 public static void centreWindow(Window frame) {
    Dimension dimension = Toolkit.getDefaultToolkit().getScreenSize();
    int x = (int) ((dimension.getWidth() - frame.getWidth()) / 2);
    int y = (int) ((dimension.getHeight() - frame.getHeight()) / 2);
    frame.setLocation(x, y);
}   
/*=============================================================================================
/     initialize()
/=============================================================================================*/
private void initialize(){
    setTitle("Sign In");
    setResizable(false);
} 
/*=============================================================================================
/     setDBMS(NGPS_Database new_dbms)
/=============================================================================================*/
public void setNGPSFrame(NGPSFrame newNGPSFrame){
    frame = newNGPSFrame;
}
/*=============================================================================================
/     setDBMS(NGPS_Database new_dbms)
/=============================================================================================*/
public void sign_in(java.lang.String currentOwner,java.lang.String submitted_password){
    java.lang.String password = new java.lang.String();
    Owner matching_owner = new Owner();
    dbms.setOWNER(currentOwner);
    int rows = dbms.myOwnerTableModel.getRowCount();
    for(int i=0;i<rows;i++){
       Owner current = (Owner)dbms.myOwnerTableModel.getRecord(i);
       java.lang.String current_owner = current.getOwner_ID();
       if(current_owner.matches(currentOwner)){
           matching_owner = current;
       }
    }
    if(matching_owner != null){
       try{
          String encrypted_password_stored = matching_owner.getEncryptedPassword();
          
          // If the submitted and stored passwords match without decrypting, that's good enough
          String encrypted_password = submitted_password.equals(encrypted_password_stored) ? encrypted_password_stored : dbms.encrypt(submitted_password,dbms.originalKey); 
           
          boolean compare = java.security.MessageDigest.isEqual(encrypted_password_stored.getBytes(),encrypted_password.getBytes()) ;
          if(compare){
              dbms.setOWNER_OBJECT(matching_owner);
              dbms.setLoggedIn(true);
              dbms.setLoggedInState(NGPSdatabase.LOGIN_SUCCESSFUL);
              signInButton.setText("Sign Out");
              frame.getAccountMenu().setText("Logged in as: "+matching_owner.getOwner_ID());
          }
          if(!compare){
              dbms.setLoggedIn(false);
              dbms.setLoggedInState(NGPSdatabase.LOGIN_UNSUCCESSFUL);
              frame.getAccountMenu().setText("User Account");
              dbms.setOWNER("Guest");
          }         
       }catch(Exception e){
           dbms.setLoggedIn(false);
           dbms.setLoggedInState(NGPSdatabase.NOT_LOGGED_IN);
           dbms.setOWNER("Guest");
          System.out.println(e.toString());
       }
    }
}

public boolean checkLogin(String currentOwner,String submitted_password){
    java.lang.String password = new java.lang.String();
    Owner matching_owner = new Owner();
    dbms.setOWNER(currentOwner);
    int rows = dbms.myOwnerTableModel.getRowCount();
    for(int i=0;i<rows;i++){
       Owner current = (Owner)dbms.myOwnerTableModel.getRecord(i);
       java.lang.String current_owner = current.getOwner_ID();
       if(current_owner.matches(currentOwner)){
           matching_owner = current;
       }
    }
    
    boolean compare = false;
    if(matching_owner != null){
       try{
          String encrypted_password_stored = matching_owner.getEncryptedPassword();
          // If the submitted and stored passwords match without decrypting, that's good enough
          String encrypted_password = submitted_password.equals(encrypted_password_stored) ? encrypted_password_stored : dbms.encrypt(submitted_password,dbms.originalKey); 
           
          compare = java.security.MessageDigest.isEqual(encrypted_password_stored.getBytes(),encrypted_password.getBytes()) ;
          
        if(compare){
            dbms.setOWNER_OBJECT(matching_owner);
            dbms.setLoggedIn(true);
            dbms.setLoggedInState(NGPSdatabase.LOGIN_SUCCESSFUL);
            frame.getAccountMenu().setText("Logged in as: "+matching_owner.getOwner_ID());
          }

       }catch(Exception e){
          System.out.println(e.toString());
       }
    }
    return compare;
}

/*=============================================================================================
/     setDBMS(NGPS_Database new_dbms)
/=============================================================================================*/
   public void setDBMS(NGPSdatabase new_dbms){      
      dbms = new_dbms;
      dbms.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
          public void propertyChange(java.beans.PropertyChangeEvent e) {
                 dbms_propertyChange(e);
          }
      });

   } 
/*=============================================================================================
/     Property Change Listener for the Ephemeris_propertyChange
/=============================================================================================*/
  private void dbms_propertyChange(PropertyChangeEvent e)  {
     java.lang.String propertyName = e.getPropertyName();
     System.out.println("signinFrame: "+propertyName);
/*=============================================================================================
/    PARAMETERS FROM THE JSKYCALCMODEL
/=============================================================================================*/
    if(propertyName.matches("connected")){
        java.lang.Boolean current_value = (java.lang.Boolean)e.getNewValue(); 
        if(current_value){
         }else if(!current_value){
        }
    }
    if(propertyName.matches("selected_set_id")){
        java.lang.Integer current_value = (java.lang.Integer)e.getNewValue(); 
 //       this.selectedSetIDLabel.setText(current_value.toString());
    }
    if(propertyName.matches("owner")){
        java.lang.String current_value = (java.lang.String)e.getNewValue();
        ownerField.setText(current_value);
        LoggedInAsLabel.setText(current_value);
    }
    if(propertyName.matches("logged_in_state")){
        java.lang.Integer current_value = (java.lang.Integer)e.getNewValue();
        if(current_value == NGPSdatabase.LOGIN_SUCCESSFUL){
            LoginStatusLabel.setText("Login OK");
            LoginStatusLabel.setForeground(new Color(0,150,50));
             signInButton.setText("Sign Out");
       }
        if(current_value == NGPSdatabase.LOGIN_UNSUCCESSFUL){
            LoginStatusLabel.setText("Login Invalid");
            LoginStatusLabel.setForeground(Color.red);
            signInButton.setText("Sign In");
        }
        if(current_value == NGPSdatabase.NOT_LOGGED_IN){
            LoginStatusLabel.setText("Not Logged In");
            LoginStatusLabel.setForeground(Color.red);
            signInButton.setText("Sign In");
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

        jPanel2 = new javax.swing.JPanel();
        ownerField = new javax.swing.JTextField();
        jLabel32 = new javax.swing.JLabel();
        PasswordField = new javax.swing.JPasswordField();
        Owner_ID_label4 = new javax.swing.JLabel();
        signInButton = new javax.swing.JButton();
        Owner_ID_label5 = new javax.swing.JLabel();
        LoginStatusLabel = new javax.swing.JLabel();
        CANCELButton = new javax.swing.JButton();
        jLabel33 = new javax.swing.JLabel();
        LoggedInAsLabel = new javax.swing.JLabel();

        setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);

        ownerField.setFont(new java.awt.Font("Copperplate", 1, 14)); // NOI18N

        jLabel32.setFont(new java.awt.Font("Copperplate", 1, 14)); // NOI18N
        jLabel32.setHorizontalAlignment(javax.swing.SwingConstants.RIGHT);
        jLabel32.setText("Password");

        PasswordField.setMinimumSize(new java.awt.Dimension(11, 400));
        PasswordField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                PasswordFieldActionPerformed(evt);
            }
        });

        Owner_ID_label4.setText("SIGNED IN AS:");

        signInButton.setText("Sign In");
        signInButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                signInButtonActionPerformed(evt);
            }
        });

        Owner_ID_label5.setText("STATUS:");

        LoginStatusLabel.setFont(new java.awt.Font("Copperplate", 1, 18)); // NOI18N
        LoginStatusLabel.setForeground(new java.awt.Color(0, 153, 51));
        LoginStatusLabel.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        LoginStatusLabel.setText("NOT SIGNED IN");

        CANCELButton.setText("CANCEL");
        CANCELButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                CANCELButtonActionPerformed(evt);
            }
        });

        jLabel33.setFont(new java.awt.Font("Copperplate", 1, 14)); // NOI18N
        jLabel33.setHorizontalAlignment(javax.swing.SwingConstants.RIGHT);
        jLabel33.setText("Username");

        LoggedInAsLabel.setFont(new java.awt.Font("Copperplate", 1, 18)); // NOI18N
        LoggedInAsLabel.setForeground(new java.awt.Color(0, 153, 51));
        LoggedInAsLabel.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);

        javax.swing.GroupLayout jPanel2Layout = new javax.swing.GroupLayout(jPanel2);
        jPanel2.setLayout(jPanel2Layout);
        jPanel2Layout.setHorizontalGroup(
            jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel2Layout.createSequentialGroup()
                .addGap(31, 31, 31)
                .addComponent(jLabel33)
                .addGap(18, 18, 18)
                .addComponent(ownerField, javax.swing.GroupLayout.PREFERRED_SIZE, 210, javax.swing.GroupLayout.PREFERRED_SIZE))
            .addGroup(jPanel2Layout.createSequentialGroup()
                .addGap(33, 33, 33)
                .addComponent(jLabel32)
                .addGap(18, 18, 18)
                .addComponent(PasswordField, javax.swing.GroupLayout.PREFERRED_SIZE, 210, javax.swing.GroupLayout.PREFERRED_SIZE))
            .addGroup(jPanel2Layout.createSequentialGroup()
                .addGap(54, 54, 54)
                .addComponent(Owner_ID_label5)
                .addGap(18, 18, 18)
                .addComponent(LoginStatusLabel, javax.swing.GroupLayout.PREFERRED_SIZE, 208, javax.swing.GroupLayout.PREFERRED_SIZE))
            .addGroup(jPanel2Layout.createSequentialGroup()
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addComponent(Owner_ID_label4, javax.swing.GroupLayout.PREFERRED_SIZE, 99, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addGap(18, 18, 18)
                .addComponent(LoggedInAsLabel, javax.swing.GroupLayout.PREFERRED_SIZE, 208, javax.swing.GroupLayout.PREFERRED_SIZE))
            .addGroup(jPanel2Layout.createSequentialGroup()
                .addGap(41, 41, 41)
                .addComponent(signInButton)
                .addGap(147, 147, 147)
                .addComponent(CANCELButton))
        );
        jPanel2Layout.setVerticalGroup(
            jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel2Layout.createSequentialGroup()
                .addGap(10, 10, 10)
                .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel2Layout.createSequentialGroup()
                        .addGap(2, 2, 2)
                        .addComponent(jLabel33, javax.swing.GroupLayout.PREFERRED_SIZE, 20, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addComponent(ownerField, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addGap(15, 15, 15)
                .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel2Layout.createSequentialGroup()
                        .addGap(5, 5, 5)
                        .addComponent(jLabel32))
                    .addComponent(PasswordField, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addGap(15, 15, 15)
                .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel2Layout.createSequentialGroup()
                        .addGap(2, 2, 2)
                        .addComponent(Owner_ID_label5))
                    .addComponent(LoginStatusLabel))
                .addGap(12, 12, 12)
                .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(Owner_ID_label4)
                    .addComponent(LoggedInAsLabel, javax.swing.GroupLayout.PREFERRED_SIZE, 20, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addGap(12, 12, 12)
                .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(signInButton)
                    .addComponent(CANCELButton)))
        );

        ownerField.getAccessibleContext().setAccessibleName("");

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addComponent(jPanel2, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addGap(10, 10, 10)
                .addComponent(jPanel2, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(20, Short.MAX_VALUE))
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents

    private void PasswordFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_PasswordFieldActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_PasswordFieldActionPerformed

    private void signInButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_signInButtonActionPerformed
        String owner_id = ownerField.getText();
        owner_id = owner_id.trim();
        int logged_in_state = dbms.getLoggedInState();
        if(logged_in_state == NGPSdatabase.LOGIN_SUCCESSFUL){
            dbms.setLoggedInState(NGPSdatabase.NOT_LOGGED_IN);
            //frame.getAccountMenu().setText("User Account");
            //dbms.setOWNER("GUEST");
        sign_in("GUEST","guest");
        }
        if(logged_in_state == NGPSdatabase.NOT_LOGGED_IN | logged_in_state == NGPSdatabase.LOGIN_UNSUCCESSFUL){
            java.lang.String submitted_password = new java.lang.String(PasswordField.getPassword());
            sign_in(owner_id,submitted_password);
            PasswordField.setText("");
            if(dbms.isLoggedIn()){
                setVisible(false);
            }
        }
    }//GEN-LAST:event_signInButtonActionPerformed

    private void CANCELButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_CANCELButtonActionPerformed
        setVisible(false);
    }//GEN-LAST:event_CANCELButtonActionPerformed

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
            java.util.logging.Logger.getLogger(signInFrame2.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (InstantiationException ex) {
            java.util.logging.Logger.getLogger(signInFrame2.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (IllegalAccessException ex) {
            java.util.logging.Logger.getLogger(signInFrame2.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (javax.swing.UnsupportedLookAndFeelException ex) {
            java.util.logging.Logger.getLogger(signInFrame2.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        }
        //</editor-fold>

        /* Create and display the form */
        java.awt.EventQueue.invokeLater(new Runnable() {
            public void run() {
                new signInFrame2().setVisible(true);
            }
        });
    }

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton CANCELButton;
    private javax.swing.JLabel LoggedInAsLabel;
    private javax.swing.JLabel LoginStatusLabel;
    private javax.swing.JLabel Owner_ID_label4;
    private javax.swing.JLabel Owner_ID_label5;
    private javax.swing.JPasswordField PasswordField;
    private javax.swing.JLabel jLabel32;
    private javax.swing.JLabel jLabel33;
    private javax.swing.JPanel jPanel2;
    private javax.swing.JTextField ownerField;
    private javax.swing.JButton signInButton;
    // End of variables declaration//GEN-END:variables
}
