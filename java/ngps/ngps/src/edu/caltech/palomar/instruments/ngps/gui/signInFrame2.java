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
    public  ImageIcon              ON;
    public  ImageIcon              OFF;
    public  ImageIcon              UNKNOWN;  
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
    ON           = new ImageIcon(USERDIR + IMAGE_CACHE + "ON.png");
    OFF          = new ImageIcon(USERDIR + IMAGE_CACHE + "OFF.png");
    UNKNOWN      = new ImageIcon(USERDIR + IMAGE_CACHE + "UNKNOWN.gif");   
    CANCELButton.setToolTipText("Exit this dialog without signing in.");
    signInButton.setToolTipText("Sign into the seleced account using the provided password");
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
          java.lang.String encrypted_password_stored = matching_owner.getEncryptedPassword();
          java.lang.String encrypted_password = dbms.encrypt(submitted_password,dbms.originalKey); 
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
//    ownersTable.setModel(dbms.myOwnerTableModel);
//    ownersTable.getColumnModel().getColumn(0).setMinWidth(160);//NAME
//    ownersTable.getColumnModel().getColumn(1).setMinWidth(100);//RA
//    ownersTable.getColumnModel().getColumn(2).setMinWidth(500);//DEC
//    ownersTable.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);
//    ownersTable.setSelectionMode(javax.swing.ListSelectionModel.SINGLE_SELECTION);
//    ownersTable.setRowSelectionAllowed(true);  
//     configureUserTableListener();           
   } 
 /*================================================================================================
/     configureUserTableListener()
/=================================================================================================*/
public void configureUserTableListener(){
//  ownersTable.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        if (ALLOW_ROW_SELECTION) { // true by default
            ListSelectionModel rowSM = ownersTable.getSelectionModel();
            rowSM.addListSelectionListener(new ListSelectionListener() {
                public void valueChanged(ListSelectionEvent e) {
                    //Ignore extra messages.
                    if (e.getValueIsAdjusting()) return;
 
                    ListSelectionModel lsm = (ListSelectionModel)e.getSource();
                    if (lsm.isSelectionEmpty()) {
                        System.out.println("No rows are selected.");
                    } else {
                        int selectedRow = lsm.getMinSelectionIndex();
                        Owner selectedOwner = ((OwnerTableModel)ownersTable.getModel()).getRecord(selectedRow);
                        dbms.setOWNER(selectedOwner.getOwner_ID());
                        dbms.queryObservationSets(selectedOwner.getOwner_ID());
                        System.out.println("Row " + selectedRow + " is now selected.");
                        System.out.println("OWNER_ID = " + selectedOwner.getOwner_ID() + " is now selected.");
                    }
                }
            });
        }
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
        selectedOwnerTextField.setText(current_value);
        LoggedInAsLabel.setText(current_value);
    }
    if(propertyName.matches("logged_in_state")){
        java.lang.Integer current_value = (java.lang.Integer)e.getNewValue();
        if(current_value == NGPSdatabase.LOGIN_SUCCESSFUL){
            LoginStatusLabel.setText("Login Successful");
            LoginStatusLabel.setForeground(new Color(0,150,50));
             signInButton.setText("Sign Out");
       }
        if(current_value == NGPSdatabase.LOGIN_UNSUCCESSFUL){
            LoginStatusLabel.setText("Login Not Successful");
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

        jTabbedPane1 = new javax.swing.JTabbedPane();
        jPanel1 = new javax.swing.JPanel();
        selectedOwnerTextField = new javax.swing.JTextField();
        jLabel32 = new javax.swing.JLabel();
        PasswordField = new javax.swing.JPasswordField();
        Owner_ID_label4 = new javax.swing.JLabel();
        LoggedInAsLabel = new javax.swing.JLabel();
        signInButton = new javax.swing.JButton();
        Owner_ID_label5 = new javax.swing.JLabel();
        LoginStatusLabel = new javax.swing.JLabel();
        CANCELButton = new javax.swing.JButton();
        jLabel33 = new javax.swing.JLabel();

        setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);
        getContentPane().setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        jPanel1.setBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 0, 0), 3, true));
        jPanel1.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        selectedOwnerTextField.setFont(new java.awt.Font("Copperplate", 1, 14)); // NOI18N
        jPanel1.add(selectedOwnerTextField, new org.netbeans.lib.awtextra.AbsoluteConstraints(158, 9, 285, -1));

        jLabel32.setFont(new java.awt.Font("Copperplate", 1, 14)); // NOI18N
        jLabel32.setHorizontalAlignment(javax.swing.SwingConstants.LEFT);
        jLabel32.setText("Password");
        jPanel1.add(jLabel32, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 40, 141, -1));

        PasswordField.setText("password");
        PasswordField.setMinimumSize(new java.awt.Dimension(11, 400));
        PasswordField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                PasswordFieldActionPerformed(evt);
            }
        });
        jPanel1.add(PasswordField, new org.netbeans.lib.awtextra.AbsoluteConstraints(160, 40, 280, -1));

        Owner_ID_label4.setText("SIGNED IN AS:");
        jPanel1.add(Owner_ID_label4, new org.netbeans.lib.awtextra.AbsoluteConstraints(50, 100, -1, -1));

        LoggedInAsLabel.setFont(new java.awt.Font("Copperplate", 1, 18)); // NOI18N
        LoggedInAsLabel.setForeground(new java.awt.Color(0, 153, 51));
        LoggedInAsLabel.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jPanel1.add(LoggedInAsLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(160, 100, 280, 20));

        signInButton.setText("Sign In");
        signInButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                signInButtonActionPerformed(evt);
            }
        });
        jPanel1.add(signInButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(50, 130, -1, -1));

        Owner_ID_label5.setText("STATUS:");
        jPanel1.add(Owner_ID_label5, new org.netbeans.lib.awtextra.AbsoluteConstraints(50, 70, -1, -1));

        LoginStatusLabel.setFont(new java.awt.Font("Copperplate", 1, 18)); // NOI18N
        LoginStatusLabel.setForeground(new java.awt.Color(0, 153, 51));
        LoginStatusLabel.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        LoginStatusLabel.setText("NOT SIGNED IN");
        jPanel1.add(LoginStatusLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(110, 70, 250, -1));

        CANCELButton.setText("CANCEL");
        CANCELButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                CANCELButtonActionPerformed(evt);
            }
        });
        jPanel1.add(CANCELButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(130, 130, -1, -1));

        jLabel33.setFont(new java.awt.Font("Copperplate", 1, 14)); // NOI18N
        jLabel33.setHorizontalAlignment(javax.swing.SwingConstants.LEFT);
        jLabel33.setText("Owner");
        jPanel1.add(jLabel33, new org.netbeans.lib.awtextra.AbsoluteConstraints(11, 14, 141, -1));

        jTabbedPane1.addTab("Sign In", jPanel1);

        getContentPane().add(jTabbedPane1, new org.netbeans.lib.awtextra.AbsoluteConstraints(12, 12, 470, 230));

        pack();
    }// </editor-fold>//GEN-END:initComponents

    private void PasswordFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_PasswordFieldActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_PasswordFieldActionPerformed

    private void signInButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_signInButtonActionPerformed
        java.lang.String current_ownner = selectedOwnerTextField.getText();
        current_ownner = current_ownner.trim();
        int logged_in_state = dbms.getLoggedInState();
        if(logged_in_state == NGPSdatabase.LOGIN_SUCCESSFUL){
            dbms.setLoggedInState(NGPSdatabase.NOT_LOGGED_IN);
            frame.getAccountMenu().setText("User Account");
        }
        if(logged_in_state == NGPSdatabase.NOT_LOGGED_IN | logged_in_state == NGPSdatabase.LOGIN_UNSUCCESSFUL){
            java.lang.String submitted_password = new java.lang.String(PasswordField.getPassword());
            sign_in(current_ownner,submitted_password);
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
    private javax.swing.JPanel jPanel1;
    private javax.swing.JTabbedPane jTabbedPane1;
    private javax.swing.JTextField selectedOwnerTextField;
    private javax.swing.JButton signInButton;
    // End of variables declaration//GEN-END:variables
}
