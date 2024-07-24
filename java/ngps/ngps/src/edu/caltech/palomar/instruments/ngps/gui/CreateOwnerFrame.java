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
    ownersTable.setModel(dbms.myOwnerTableModel);
 //   ownersTable.setModel(dbms.myOwnerTableModel);
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
/    getOwnersTable()
/=================================================================================================*/
public JTable getOwnersTable(){
   return ownersTable;
}  
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
        selectedOwnerTextField.setText(current_value);
    }
}  
/*================================================================================================
/   createUser()
/=================================================================================================*/
public void createOwner(){
        Owner current = new Owner();
        current.setOwner_ID(Owner_IDTextField.getText());
        current.setProposal_ID(ProposalIDTextField.getText());
        current.setProposal_Title(ProposalTitleTextField.getText());
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
                        dbms.queryObservationSets(selectedOwner.getOwner_ID());
                        System.out.println("Row " + selectedRow + " is now selected.");
                        System.out.println("OWNER_ID = " + selectedOwner.getOwner_ID() + " is now selected.");
                    }
                }
            });
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

        jPanel6 = new javax.swing.JPanel();
        Owner_ID_label2 = new javax.swing.JLabel();
        Owner_ID_label = new javax.swing.JLabel();
        ProposalTitleTextField = new javax.swing.JTextField();
        Owner_IDTextField = new javax.swing.JTextField();
        Owner_ID_label1 = new javax.swing.JLabel();
        ProposalIDTextField = new javax.swing.JTextField();
        CreateUserButton = new javax.swing.JButton();
        Owner_ID_label3 = new javax.swing.JLabel();
        Owner_ID_label5 = new javax.swing.JLabel();
        PasswordField = new javax.swing.JPasswordField();
        confirmPasswordField = new javax.swing.JPasswordField();
        Owner_ID_label4 = new javax.swing.JLabel();
        jScrollPane1 = new javax.swing.JScrollPane();
        ownersTable = new javax.swing.JTable();
        jPanel1 = new javax.swing.JPanel();
        jLabel33 = new javax.swing.JLabel();
        selectedOwnerTextField = new javax.swing.JTextField();
        jLabel32 = new javax.swing.JLabel();
        jScrollPane10 = new javax.swing.JScrollPane();
        OwnerList = new javax.swing.JList<>();

        setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);

        jPanel6.setBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 0, 0), 3, true));

        Owner_ID_label2.setText("PROPOSAL_TITLE");

        Owner_ID_label.setText("OWNER_ID");

        Owner_ID_label1.setText("PROPOSAL_ID");

        CreateUserButton.setText("Create OWNER");
        CreateUserButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                CreateUserButtonActionPerformed(evt);
            }
        });

        Owner_ID_label3.setText("PASSWORD");

        Owner_ID_label5.setText("CONFIRM PASSWORD");

        PasswordField.setText("jPasswordField1");

        confirmPasswordField.setText("jPasswordField1");

        javax.swing.GroupLayout jPanel6Layout = new javax.swing.GroupLayout(jPanel6);
        jPanel6.setLayout(jPanel6Layout);
        jPanel6Layout.setHorizontalGroup(
            jPanel6Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel6Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel6Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                    .addGroup(jPanel6Layout.createSequentialGroup()
                        .addComponent(Owner_ID_label5)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                        .addComponent(confirmPasswordField, javax.swing.GroupLayout.DEFAULT_SIZE, 237, Short.MAX_VALUE))
                    .addGroup(jPanel6Layout.createSequentialGroup()
                        .addGroup(jPanel6Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addGroup(jPanel6Layout.createSequentialGroup()
                                .addGap(1, 1, 1)
                                .addComponent(Owner_ID_label2))
                            .addComponent(Owner_ID_label1)
                            .addComponent(Owner_ID_label)
                            .addComponent(Owner_ID_label3))
                        .addGap(31, 31, 31)
                        .addGroup(jPanel6Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                            .addComponent(ProposalTitleTextField)
                            .addComponent(Owner_IDTextField)
                            .addComponent(ProposalIDTextField)
                            .addComponent(PasswordField, javax.swing.GroupLayout.DEFAULT_SIZE, 239, Short.MAX_VALUE))))
                .addContainerGap(20, Short.MAX_VALUE))
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel6Layout.createSequentialGroup()
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addComponent(CreateUserButton)
                .addGap(87, 87, 87))
        );
        jPanel6Layout.setVerticalGroup(
            jPanel6Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel6Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel6Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(Owner_ID_label2, javax.swing.GroupLayout.PREFERRED_SIZE, 19, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(ProposalTitleTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel6Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(Owner_ID_label)
                    .addComponent(Owner_IDTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel6Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(Owner_ID_label1)
                    .addComponent(ProposalIDTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel6Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(Owner_ID_label3)
                    .addComponent(PasswordField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel6Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(Owner_ID_label5)
                    .addComponent(confirmPasswordField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(CreateUserButton)
                .addContainerGap(13, Short.MAX_VALUE))
        );

        Owner_ID_label4.setFont(new java.awt.Font("Copperplate", 1, 18)); // NOI18N
        Owner_ID_label4.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        Owner_ID_label4.setText("Account OWNERS");

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

        jPanel1.setBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 0, 0), 3, true));

        jLabel33.setFont(new java.awt.Font("Copperplate", 1, 12)); // NOI18N
        jLabel33.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel33.setText("Database Owners List");

        selectedOwnerTextField.setEditable(false);
        selectedOwnerTextField.setFont(new java.awt.Font("Copperplate", 1, 14)); // NOI18N

        jLabel32.setFont(new java.awt.Font("Copperplate", 1, 14)); // NOI18N
        jLabel32.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel32.setText("Selected Owner");

        jScrollPane10.setVerticalScrollBarPolicy(javax.swing.ScrollPaneConstants.VERTICAL_SCROLLBAR_ALWAYS);

        OwnerList.setSelectionMode(javax.swing.ListSelectionModel.SINGLE_SELECTION);
        OwnerList.setAutoscrolls(false);
        jScrollPane10.setViewportView(OwnerList);

        javax.swing.GroupLayout jPanel1Layout = new javax.swing.GroupLayout(jPanel1);
        jPanel1.setLayout(jPanel1Layout);
        jPanel1Layout.setHorizontalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel1Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jScrollPane10, javax.swing.GroupLayout.PREFERRED_SIZE, 135, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel1Layout.createSequentialGroup()
                        .addGap(2, 2, 2)
                        .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(selectedOwnerTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 135, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(jLabel33, javax.swing.GroupLayout.PREFERRED_SIZE, 149, javax.swing.GroupLayout.PREFERRED_SIZE)))
                    .addGroup(jPanel1Layout.createSequentialGroup()
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(jLabel32, javax.swing.GroupLayout.PREFERRED_SIZE, 162, javax.swing.GroupLayout.PREFERRED_SIZE)))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );
        jPanel1Layout.setVerticalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel1Layout.createSequentialGroup()
                .addGap(19, 19, 19)
                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(jScrollPane10, javax.swing.GroupLayout.PREFERRED_SIZE, 175, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addGroup(jPanel1Layout.createSequentialGroup()
                        .addComponent(selectedOwnerTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(jLabel33)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(jLabel32)))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(jScrollPane1, javax.swing.GroupLayout.PREFERRED_SIZE, 993, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addGroup(layout.createSequentialGroup()
                        .addContainerGap()
                        .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addGroup(layout.createSequentialGroup()
                                .addComponent(jPanel6, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(jPanel1, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                            .addGroup(layout.createSequentialGroup()
                                .addComponent(Owner_ID_label4, javax.swing.GroupLayout.PREFERRED_SIZE, 985, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addGap(0, 0, Short.MAX_VALUE)))))
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                    .addComponent(jPanel6, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(jPanel1, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(Owner_ID_label4)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jScrollPane1, javax.swing.GroupLayout.DEFAULT_SIZE, 171, Short.MAX_VALUE)
                .addContainerGap())
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
    private javax.swing.JList<String> OwnerList;
    private javax.swing.JTextField Owner_IDTextField;
    private javax.swing.JLabel Owner_ID_label;
    private javax.swing.JLabel Owner_ID_label1;
    private javax.swing.JLabel Owner_ID_label2;
    private javax.swing.JLabel Owner_ID_label3;
    private javax.swing.JLabel Owner_ID_label4;
    private javax.swing.JLabel Owner_ID_label5;
    private javax.swing.JPasswordField PasswordField;
    private javax.swing.JTextField ProposalIDTextField;
    private javax.swing.JTextField ProposalTitleTextField;
    private javax.swing.JPasswordField confirmPasswordField;
    private javax.swing.JLabel jLabel32;
    private javax.swing.JLabel jLabel33;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JPanel jPanel6;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JScrollPane jScrollPane10;
    private javax.swing.JTable ownersTable;
    private javax.swing.JTextField selectedOwnerTextField;
    // End of variables declaration//GEN-END:variables
}
