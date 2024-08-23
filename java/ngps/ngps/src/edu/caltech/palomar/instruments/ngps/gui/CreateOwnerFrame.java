/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/GUIForms/JFrame.java to edit this template
 */
package edu.caltech.palomar.instruments.ngps.gui;

import edu.caltech.palomar.instruments.ngps.dbms.NGPSdatabase;
import edu.caltech.palomar.instruments.ngps.object.Owner;
import edu.caltech.palomar.instruments.ngps.tables.OwnerTableModel;
import java.beans.PropertyChangeEvent;
import javax.swing.JFrame;
import javax.swing.JTable;
import javax.swing.ListSelectionModel;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;
import javax.swing.JOptionPane;

/**
 *
 * @author jennifermilburn
 */
public class CreateOwnerFrame extends javax.swing.JFrame {
   public NGPSdatabase dbms;
   private boolean     ALLOW_ROW_SELECTION = true;
     /**
     * Creates new form CreateOwnerFrame
     */
    public CreateOwnerFrame() {
        initComponents();       
        setDefaultCloseOperation(JFrame.HIDE_ON_CLOSE);
    }
/*================================================================================================
/      initializeOwnerList()
/=================================================================================================*/    
/*    public void initializeOwnerList(){
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
    }   */
/*=============================================================================================
/   setJEditorPane(JEditorPane newEditorPane)
/=============================================================================================*/
public NGPSdatabase getNGPS_Database(){
   return dbms;
}
public void setDBMS(NGPSdatabase current_dbms){
    dbms = current_dbms;
    dbms.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
              public void propertyChange(java.beans.PropertyChangeEvent e) {
                 dbms_propertyChange(e);
              }
            });
//    ownersTable.setModel(dbms.myOwnerTableModel);
 //   ownersTable.setModel(dbms.myOwnerTableModel);
//    ownersTable.getColumnModel().getColumn(0).setMinWidth(160);//NAME
//    ownersTable.getColumnModel().getColumn(1).setMinWidth(100);//RA
//    ownersTable.getColumnModel().getColumn(2).setMinWidth(500);//DEC
//    ownersTable.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);
//    ownersTable.setSelectionMode(javax.swing.ListSelectionModel.SINGLE_SELECTION);
//    ownersTable.setRowSelectionAllowed(true);  
//    initializeOwnerList();
//    configureUserTableListener();
}
/*================================================================================================
/    getOwnersTable()
/=================================================================================================*/
//public JTable getOwnersTable(){
//   return ownersTable;
//}  
/*=============================================================================================
/     Property Change Listener for the Ephemeris_propertyChange
/=============================================================================================*/
  private void dbms_propertyChange(PropertyChangeEvent e)  {
     java.lang.String propertyName = e.getPropertyName();
     //System.out.println(propertyName);
/*=============================================================================================
/    PARAMETERS FROM THE JSKYCALCMODEL
/=============================================================================================*/
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
    if(propertyName.matches("selected_set_id")){
//        java.lang.Integer current_value = (java.lang.Integer)e.getNewValue(); 
//        this.selectedSetIDLabel.setText(current_value.toString());
    }
    if(propertyName.matches("owner")){
        java.lang.String current_value = (java.lang.String)e.getNewValue();
//        selectedOwnerTextField.setText(current_value);
    }
}  
/*================================================================================================
/   createUser()
/=================================================================================================*/
public void createOwner(){
        Owner current = new Owner();
        current.setOwner_ID(Owner_IDTextField.getText());
        current.setEmail(EmailTextField.getText());
        char[] password         = PasswordField.getPassword();
        char[] confirm_password = confirmPasswordField.getPassword();
        java.lang.String myPasswordString = new java.lang.String(password);
        java.lang.String myconfirmPasswordString = new java.lang.String(confirm_password);
        if(myPasswordString.matches(myconfirmPasswordString)){
           try{
               current.setPassword(myPasswordString);
               java.lang.String encrypted_password = dbms.encrypt(myPasswordString,dbms.originalKey);
               current.setEncryptedPassword(encrypted_password);
           }catch(Exception e){
              System.out.println(e.toString());
           }           
        }
        if(!myPasswordString.matches(myconfirmPasswordString)){
            JOptionPane.showMessageDialog(this,"","",JOptionPane.WARNING_MESSAGE);           
        }        
        dbms.executeOwnerInsertStatement(current);
        dbms.queryOwners();   
}
public void executeCreateOwner(){
    executeCreateOwner myexecuteCreateOwner = new executeCreateOwner();
    myexecuteCreateOwner.start();
}
/*================================================================================================
/     configureUserTableListener()
/=================================================================================================*/
/*public void configureUserTableListener(){
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
                        dbms.queryObservationSets(selectedOwner.getOwner_ID());
                        System.out.println("Row " + selectedRow + " is now selected.");
                        System.out.println("OWNER_ID = " + selectedOwner.getOwner_ID() + " is now selected.");
                    }
                }
            });
        }
}*/
    /**
     * This method is called from within the constructor to initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is always
     * regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        jTabbedPane1 = new javax.swing.JTabbedPane();
        jPanel6 = new javax.swing.JPanel();
        Owner_ID_label = new javax.swing.JLabel();
        Owner_IDTextField = new javax.swing.JTextField();
        CreateUserButton = new javax.swing.JButton();
        Owner_ID_label3 = new javax.swing.JLabel();
        Owner_ID_label5 = new javax.swing.JLabel();
        PasswordField = new javax.swing.JPasswordField();
        confirmPasswordField = new javax.swing.JPasswordField();
        Owner_ID_label6 = new javax.swing.JLabel();
        EmailTextField = new javax.swing.JTextField();

        setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);

        jTabbedPane1.setBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 0, 0), 2, true));

        jPanel6.setBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 0, 0), 3, true));

        Owner_ID_label.setText("OWNER_ID");

        CreateUserButton.setText("Create OWNER");
        CreateUserButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                CreateUserButtonActionPerformed(evt);
            }
        });

        Owner_ID_label3.setText("PASSWORD");

        Owner_ID_label5.setText("EMAIL");

        PasswordField.setText("jPasswordField1");

        confirmPasswordField.setText("jPasswordField1");

        Owner_ID_label6.setText("CONFIRM PASSWORD");

        javax.swing.GroupLayout jPanel6Layout = new javax.swing.GroupLayout(jPanel6);
        jPanel6.setLayout(jPanel6Layout);
        jPanel6Layout.setHorizontalGroup(
            jPanel6Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel6Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel6Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel6Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                        .addGroup(jPanel6Layout.createSequentialGroup()
                            .addComponent(Owner_ID_label6)
                            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                            .addComponent(confirmPasswordField, javax.swing.GroupLayout.DEFAULT_SIZE, 237, Short.MAX_VALUE))
                        .addGroup(jPanel6Layout.createSequentialGroup()
                            .addGroup(jPanel6Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                .addComponent(Owner_ID_label)
                                .addComponent(Owner_ID_label3))
                            .addGap(80, 80, 80)
                            .addGroup(jPanel6Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                .addComponent(PasswordField)
                                .addComponent(Owner_IDTextField))))
                    .addGroup(jPanel6Layout.createSequentialGroup()
                        .addComponent(Owner_ID_label5)
                        .addGap(119, 119, 119)
                        .addComponent(EmailTextField, javax.swing.GroupLayout.DEFAULT_SIZE, 288, Short.MAX_VALUE)))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
            .addGroup(jPanel6Layout.createSequentialGroup()
                .addGap(134, 134, 134)
                .addComponent(CreateUserButton)
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );
        jPanel6Layout.setVerticalGroup(
            jPanel6Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel6Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel6Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(Owner_ID_label)
                    .addComponent(Owner_IDTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel6Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(Owner_ID_label3)
                    .addComponent(PasswordField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel6Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(confirmPasswordField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(Owner_ID_label6))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel6Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(Owner_ID_label5)
                    .addComponent(EmailTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addGap(18, 18, 18)
                .addComponent(CreateUserButton)
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        jTabbedPane1.addTab("Create Owner", jPanel6);

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(jTabbedPane1, javax.swing.GroupLayout.PREFERRED_SIZE, 484, javax.swing.GroupLayout.PREFERRED_SIZE)
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(jTabbedPane1, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents

    private void CreateUserButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_CreateUserButtonActionPerformed
        executeCreateOwner();
    }//GEN-LAST:event_CreateUserButtonActionPerformed
/*=============================================================================================
/       INNER CLASS executeFlatFieldThread
/=============================================================================================*/
    public class executeCreateOwner  implements Runnable{
    private Thread myThread; 
/*=============================================================================================
/         executeCreateOwner()
/=============================================================================================*/
    private executeCreateOwner(){
    }
/*=============================================================================================
/           run()
/=============================================================================================*/
    public void run(){
       createOwner();
     return;
    }
/*=============================================================================================
/         start()
/=============================================================================================*/
    public void start(){
     myThread = new Thread(this);
     myThread.start();
     }
    }// End of the SendCommandThread Inner Class
/*=============================================================================================
/                           End of the executeBiasThread
/=============================================================================================*/  

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
            java.util.logging.Logger.getLogger(CreateOwnerFrame.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (InstantiationException ex) {
            java.util.logging.Logger.getLogger(CreateOwnerFrame.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (IllegalAccessException ex) {
            java.util.logging.Logger.getLogger(CreateOwnerFrame.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (javax.swing.UnsupportedLookAndFeelException ex) {
            java.util.logging.Logger.getLogger(CreateOwnerFrame.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        }
        //</editor-fold>

        /* Create and display the form */
        java.awt.EventQueue.invokeLater(new Runnable() {
            public void run() {
                new CreateOwnerFrame().setVisible(true);
            }
        });
    }

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton CreateUserButton;
    private javax.swing.JTextField EmailTextField;
    private javax.swing.JTextField Owner_IDTextField;
    private javax.swing.JLabel Owner_ID_label;
    private javax.swing.JLabel Owner_ID_label3;
    private javax.swing.JLabel Owner_ID_label5;
    private javax.swing.JLabel Owner_ID_label6;
    private javax.swing.JPasswordField PasswordField;
    private javax.swing.JPasswordField confirmPasswordField;
    private javax.swing.JPanel jPanel6;
    private javax.swing.JTabbedPane jTabbedPane1;
    // End of variables declaration//GEN-END:variables
}
