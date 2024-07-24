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
 * @author jennifermilburn
 */
public class signInFrame extends javax.swing.JFrame {
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

    /**
     * Creates new form signInFrame
     */
/*=============================================================================================
/    signInFrame() Constructor
/=============================================================================================*/
    public signInFrame() {
        initComponents();
        setDefaultCloseOperation(JFrame.HIDE_ON_CLOSE);
        centreWindow(this);
        initialize();
    }
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
    updateListButton.setToolTipText("Update the list of owners from the database");
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
public void sign_in(java.lang.String submitted_password){
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
          java.lang.String encrypted_password = dbms.encrypt(submitted_password,dbms.originalKey); 
          boolean compare = java.security.MessageDigest.isEqual(encrypted_password_stored.getBytes(),encrypted_password.getBytes()) ;
          if(compare){
              dbms.setOWNER_OBJECT(matching_owner);
              dbms.setLoggedIn(true);
              dbms.setLoggedInState(NGPSdatabase.LOGIN_SUCCESSFUL);
              OwnerList.setEnabled(false);
              signInButton.setText("Sign Out");
              frame.getAccountMenu().setText("Logged in as: "+matching_owner.getOwner_ID());
          }
          if(!compare){
              dbms.setLoggedIn(false);
              dbms.setLoggedInState(NGPSdatabase.LOGIN_UNSUCCESSFUL);
              frame.getAccountMenu().setText("User Account");
          }         
       }catch(Exception e){
           dbms.setLoggedIn(false);
           dbms.setLoggedInState(NGPSdatabase.NOT_LOGGED_IN);
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
    ownersTable.setModel(dbms.myOwnerTableModel);
    ownersTable.getColumnModel().getColumn(0).setMinWidth(160);//NAME
    ownersTable.getColumnModel().getColumn(1).setMinWidth(100);//RA
    ownersTable.getColumnModel().getColumn(2).setMinWidth(500);//DEC
    ownersTable.getColumnModel().getColumn(3).setMinWidth(300);//DEC
    ownersTable.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);
    ownersTable.setSelectionMode(javax.swing.ListSelectionModel.SINGLE_SELECTION);
    ownersTable.setRowSelectionAllowed(true);  
    initializeOwnerList();
    configureUserTableListener();           
   } 
 /*================================================================================================
/     configureUserTableListener()
/=================================================================================================*/
public void configureUserTableListener(){
  ownersTable.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
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
/*================================================================================================
/      initializeOwnerList()
/=================================================================================================*/    
    public void initializeOwnerList(){
        OwnerList.setModel(dbms.ownerslist);
        OwnerList.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
            ListSelectionModel rowSM = OwnerList.getSelectionModel();
            rowSM.addListSelectionListener(new ListSelectionListener() {
                public void valueChanged(ListSelectionEvent e) {
                    //Ignore extra messages.
                    if (e.getValueIsAdjusting()) return;
 
                    ListSelectionModel lsm = (ListSelectionModel)e.getSource();
                    if (lsm.isSelectionEmpty()) {
                        System.out.println("No rows are selected.");
                    } else {
                        int selectedRow = lsm.getMinSelectionIndex();
                        java.lang.String selectedOwner = (java.lang.String)(OwnerList.getModel()).getElementAt(selectedRow);
                        System.out.println("Row " + selectedRow + " is now selected.");
                        System.out.println("OWNER_ID = " + selectedOwner + " is now selected.");
                        dbms.setOWNER(selectedOwner);
                    }
                }
            });  
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
        jLabel33 = new javax.swing.JLabel();
        selectedOwnerTextField = new javax.swing.JTextField();
        jLabel32 = new javax.swing.JLabel();
        jScrollPane10 = new javax.swing.JScrollPane();
        OwnerList = new javax.swing.JList<>();
        Owner_ID_label3 = new javax.swing.JLabel();
        PasswordField = new javax.swing.JPasswordField();
        Owner_ID_label4 = new javax.swing.JLabel();
        LoggedInAsLabel = new javax.swing.JLabel();
        signInButton = new javax.swing.JButton();
        Owner_ID_label5 = new javax.swing.JLabel();
        LoginStatusLabel = new javax.swing.JLabel();
        CANCELButton = new javax.swing.JButton();
        updateListButton = new javax.swing.JButton();
        jPanel2 = new javax.swing.JPanel();
        jScrollPane1 = new javax.swing.JScrollPane();
        ownersTable = new javax.swing.JTable();

        setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);

        jPanel1.setBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 0, 0), 3, true));
        jPanel1.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        jLabel33.setFont(new java.awt.Font("Copperplate", 1, 12)); // NOI18N
        jLabel33.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel33.setText("Database Owners List");
        jPanel1.add(jLabel33, new org.netbeans.lib.awtextra.AbsoluteConstraints(9, 35, 149, -1));

        selectedOwnerTextField.setEditable(false);
        selectedOwnerTextField.setFont(new java.awt.Font("Copperplate", 1, 14)); // NOI18N
        jPanel1.add(selectedOwnerTextField, new org.netbeans.lib.awtextra.AbsoluteConstraints(158, 9, 285, -1));

        jLabel32.setFont(new java.awt.Font("Copperplate", 1, 14)); // NOI18N
        jLabel32.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel32.setText("Selected Owner");
        jPanel1.add(jLabel32, new org.netbeans.lib.awtextra.AbsoluteConstraints(11, 14, 141, -1));

        jScrollPane10.setBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 0, 0), 3, true));
        jScrollPane10.setVerticalScrollBarPolicy(javax.swing.ScrollPaneConstants.VERTICAL_SCROLLBAR_ALWAYS);

        OwnerList.setSelectionMode(javax.swing.ListSelectionModel.SINGLE_SELECTION);
        OwnerList.setAutoscrolls(false);
        jScrollPane10.setViewportView(OwnerList);

        jPanel1.add(jScrollPane10, new org.netbeans.lib.awtextra.AbsoluteConstraints(9, 55, 135, 177));

        Owner_ID_label3.setText("PASSWORD");
        jPanel1.add(Owner_ID_label3, new org.netbeans.lib.awtextra.AbsoluteConstraints(150, 95, -1, -1));

        PasswordField.setText("password");
        PasswordField.setMinimumSize(new java.awt.Dimension(11, 400));
        jPanel1.add(PasswordField, new org.netbeans.lib.awtextra.AbsoluteConstraints(225, 90, 170, -1));

        Owner_ID_label4.setText("SIGNED IN AS:");
        jPanel1.add(Owner_ID_label4, new org.netbeans.lib.awtextra.AbsoluteConstraints(150, 150, -1, -1));

        LoggedInAsLabel.setFont(new java.awt.Font("Copperplate", 1, 18)); // NOI18N
        LoggedInAsLabel.setForeground(new java.awt.Color(0, 153, 51));
        LoggedInAsLabel.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jPanel1.add(LoggedInAsLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(250, 150, 200, 20));

        signInButton.setText("Sign In");
        signInButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                signInButtonActionPerformed(evt);
            }
        });
        jPanel1.add(signInButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(170, 190, -1, -1));

        Owner_ID_label5.setText("STATUS:");
        jPanel1.add(Owner_ID_label5, new org.netbeans.lib.awtextra.AbsoluteConstraints(150, 123, -1, -1));

        LoginStatusLabel.setFont(new java.awt.Font("Copperplate", 1, 18)); // NOI18N
        LoginStatusLabel.setForeground(new java.awt.Color(0, 153, 51));
        LoginStatusLabel.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        LoginStatusLabel.setText("NOT SIGNED IN");
        jPanel1.add(LoginStatusLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(208, 122, 250, -1));

        CANCELButton.setText("CANCEL");
        CANCELButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                CANCELButtonActionPerformed(evt);
            }
        });
        jPanel1.add(CANCELButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(260, 190, -1, -1));

        updateListButton.setText("Update List");
        updateListButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                updateListButtonActionPerformed(evt);
            }
        });
        jPanel1.add(updateListButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(350, 190, -1, -1));

        jTabbedPane1.addTab("Sign In", jPanel1);

        jScrollPane1.setHorizontalScrollBarPolicy(javax.swing.ScrollPaneConstants.HORIZONTAL_SCROLLBAR_ALWAYS);
        jScrollPane1.setVerticalScrollBarPolicy(javax.swing.ScrollPaneConstants.VERTICAL_SCROLLBAR_ALWAYS);

        ownersTable.setModel(new javax.swing.table.DefaultTableModel(
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
        jScrollPane1.setViewportView(ownersTable);

        javax.swing.GroupLayout jPanel2Layout = new javax.swing.GroupLayout(jPanel2);
        jPanel2.setLayout(jPanel2Layout);
        jPanel2Layout.setHorizontalGroup(
            jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel2Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jScrollPane1)
                .addContainerGap())
        );
        jPanel2Layout.setVerticalGroup(
            jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel2Layout.createSequentialGroup()
                .addComponent(jScrollPane1, javax.swing.GroupLayout.DEFAULT_SIZE, 251, Short.MAX_VALUE)
                .addContainerGap())
        );

        jTabbedPane1.addTab("Owners Table", jPanel2);

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jTabbedPane1)
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, layout.createSequentialGroup()
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addComponent(jTabbedPane1, javax.swing.GroupLayout.PREFERRED_SIZE, 303, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap())
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents

    private void signInButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_signInButtonActionPerformed
       int logged_in_state = dbms.getLoggedInState();
       if(logged_in_state == NGPSdatabase.LOGIN_SUCCESSFUL){
           dbms.setLoggedInState(NGPSdatabase.NOT_LOGGED_IN);
           OwnerList.setEnabled(true);
           frame.getAccountMenu().setText("User Account");
       }
       if(logged_in_state == NGPSdatabase.NOT_LOGGED_IN | logged_in_state == NGPSdatabase.LOGIN_UNSUCCESSFUL){
          java.lang.String submitted_password = new java.lang.String(PasswordField.getPassword());
          sign_in(submitted_password);   
          PasswordField.setText("");           
          if(dbms.isLoggedIn()){
              setVisible(false);
          }
       }        
    }//GEN-LAST:event_signInButtonActionPerformed

    private void CANCELButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_CANCELButtonActionPerformed
       setVisible(false);
    }//GEN-LAST:event_CANCELButtonActionPerformed

    private void updateListButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_updateListButtonActionPerformed
        dbms.queryOwners();
    }//GEN-LAST:event_updateListButtonActionPerformed

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
            java.util.logging.Logger.getLogger(signInFrame.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (InstantiationException ex) {
            java.util.logging.Logger.getLogger(signInFrame.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (IllegalAccessException ex) {
            java.util.logging.Logger.getLogger(signInFrame.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (javax.swing.UnsupportedLookAndFeelException ex) {
            java.util.logging.Logger.getLogger(signInFrame.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        }
        //</editor-fold>

        /* Create and display the form */
        java.awt.EventQueue.invokeLater(new Runnable() {
            public void run() {
                new signInFrame().setVisible(true);
            }
        });
    }

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton CANCELButton;
    private javax.swing.JLabel LoggedInAsLabel;
    private javax.swing.JLabel LoginStatusLabel;
    private javax.swing.JList<String> OwnerList;
    private javax.swing.JLabel Owner_ID_label3;
    private javax.swing.JLabel Owner_ID_label4;
    private javax.swing.JLabel Owner_ID_label5;
    private javax.swing.JPasswordField PasswordField;
    private javax.swing.JLabel jLabel32;
    private javax.swing.JLabel jLabel33;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JPanel jPanel2;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JScrollPane jScrollPane10;
    private javax.swing.JTabbedPane jTabbedPane1;
    private javax.swing.JTable ownersTable;
    private javax.swing.JTextField selectedOwnerTextField;
    private javax.swing.JButton signInButton;
    private javax.swing.JButton updateListButton;
    // End of variables declaration//GEN-END:variables
}
