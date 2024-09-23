/* TODOS:
de-hardcode the mysql database/table names

*/
package edu.caltech.palomar.instruments.ngps.gui;

import edu.caltech.palomar.instruments.ngps.dbms.NGPSdatabase;
import javax.swing.JOptionPane;
import java.sql.*;
import java.util.HashMap;

public class CreateOwnerFrameDEV extends javax.swing.JFrame {
   public NGPSdatabase dbms;
     /**
     * Creates new form CreateOwnerFrame
     */
    public CreateOwnerFrameDEV() {
        initComponents();       
    }
    
public void setDBMS(NGPSdatabase current_dbms){
    dbms = current_dbms;
}

/*================================================================================================
/   createOwner() -- assigned to Create User button
/=================================================================================================*/
public void createOwner(){
    
    // Collect entries from frame
    String owner_id = Owner_IDTextField.getText();
    String newPassword = new String(PasswordField.getPassword());
    
    // Do validity checks
    if(!newPassword.equals( new String(confirmPasswordField.getPassword()) )){
        JOptionPane.showMessageDialog(null, "Passwords don't match");
        return;           
    }

    if(EmailTextField.getText().isBlank()){
        JOptionPane.showMessageDialog(null, "Missing email address");
        return;           
    }
    
    // encrypt the password
    try{
        newPassword = dbms.encrypt(newPassword,dbms.originalKey);
    } catch(Exception e){ System.out.println(e.toString()); }

    //handy has table  //keys match table columns
    HashMap ht = new HashMap<String, String>(); 
    ht.put("owner_id", owner_id);
    ht.put("password", newPassword);
    ht.put("email", EmailTextField.getText());

    // Ready mysql 
    Statement statement;
    ResultSet rs;
    
    String allKeys = String.join(", ", ht.keySet());
    String allVals = String.join("', '" ,ht.values()); // looks like 'val1', 'val2', ...
    allVals = "'" + allVals + "'"; // add 1st and last quotes to list

    // Check for existing username
    try{
        statement = dbms.conn.createStatement();
        
        rs = statement.executeQuery("SELECT COUNT(*) from ngps.owner WHERE owner_id = "+"'"+owner_id+"'"); //HARDCODE
        rs.next();
        int userExists = rs.getInt(1); // value on row 1 
        rs.close();
        if(userExists>0){
            JOptionPane.showMessageDialog(null, "Username already exists: "+owner_id);
            return;
        }        
        
    } catch(Exception e){ System.out.println(e.toString()); }

    try{
        statement = dbms.conn.createStatement();
        String cmd = "INSERT INTO ngps.owner (%1$s) VALUES (%2$s) ;"; //HARDCODE
        cmd = String.format(cmd, allKeys, allVals);
        int success = statement.executeUpdate(cmd); // number of rows added to owner
        
        if(success==0){
            JOptionPane.showMessageDialog(null, "Problem adding user: "+owner_id);
            return;            
        } else{
            JOptionPane.showMessageDialog(null, "User added: "+owner_id);
        }
        
    }catch(Exception e){
       System.out.println(e.toString());
    }
    
    //cleanup
    Owner_IDTextField.setText("");
    EmailTextField.setText("");
    PasswordField.setText("");
    confirmPasswordField.setText("");
    dispose(); // kill the popup
    
    dbms.queryOwners(); // update owner table in the GUI
        
}
public void executeCreateOwnerDEV(){
    executeCreateOwnerDEV myexecuteCreateOwner = new executeCreateOwnerDEV();
    myexecuteCreateOwner.start();
}

@SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        jPanel1 = new javax.swing.JPanel();
        Owner_ID_label = new javax.swing.JLabel();
        Owner_IDTextField = new javax.swing.JTextField();
        PasswordField = new javax.swing.JPasswordField();
        Owner_ID_label3 = new javax.swing.JLabel();
        Owner_ID_label5 = new javax.swing.JLabel();
        confirmPasswordField = new javax.swing.JPasswordField();
        EmailTextField = new javax.swing.JTextField();
        CreateUserButton = new javax.swing.JButton();
        Owner_ID_label4 = new javax.swing.JLabel();

        setTitle("CREATE USER");
        setResizable(false);

        Owner_ID_label.setText("USERNAME");

        Owner_ID_label3.setText("PASSWORD");

        Owner_ID_label5.setText("EMAIL");

        confirmPasswordField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                confirmPasswordFieldActionPerformed(evt);
            }
        });

        CreateUserButton.setText("Create User");
        CreateUserButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                CreateUserButtonActionPerformed(evt);
            }
        });

        Owner_ID_label4.setText("CONFIRM PASSWORD");

        javax.swing.GroupLayout jPanel1Layout = new javax.swing.GroupLayout(jPanel1);
        jPanel1.setLayout(jPanel1Layout);
        jPanel1Layout.setHorizontalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel1Layout.createSequentialGroup()
                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                        .addGroup(jPanel1Layout.createSequentialGroup()
                            .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                                    .addGroup(jPanel1Layout.createSequentialGroup()
                                        .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                                        .addComponent(Owner_ID_label3))
                                    .addGroup(jPanel1Layout.createSequentialGroup()
                                        .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                                        .addComponent(Owner_ID_label)))
                                .addGroup(jPanel1Layout.createSequentialGroup()
                                    .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                                    .addComponent(Owner_ID_label4)))
                            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                            .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                                .addComponent(PasswordField)
                                .addComponent(confirmPasswordField)
                                .addComponent(Owner_IDTextField, javax.swing.GroupLayout.DEFAULT_SIZE, 177, Short.MAX_VALUE)))
                        .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel1Layout.createSequentialGroup()
                            .addContainerGap()
                            .addComponent(CreateUserButton)))
                    .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel1Layout.createSequentialGroup()
                        .addContainerGap(119, Short.MAX_VALUE)
                        .addComponent(Owner_ID_label5)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(EmailTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 177, javax.swing.GroupLayout.PREFERRED_SIZE)))
                .addContainerGap())
        );
        jPanel1Layout.setVerticalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel1Layout.createSequentialGroup()
                .addGap(0, 0, Short.MAX_VALUE)
                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(Owner_IDTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(Owner_ID_label))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(Owner_ID_label3)
                    .addComponent(PasswordField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(confirmPasswordField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(Owner_ID_label4))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(EmailTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(Owner_ID_label5))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(CreateUserButton))
        );

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jPanel1, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jPanel1, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addContainerGap())
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents

    private void CreateUserButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_CreateUserButtonActionPerformed
        executeCreateOwnerDEV();
    }//GEN-LAST:event_CreateUserButtonActionPerformed

    private void confirmPasswordFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_confirmPasswordFieldActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_confirmPasswordFieldActionPerformed

    public class executeCreateOwnerDEV  implements Runnable{
    private Thread myThread; 
/*=============================================================================================
/         executeCreateOwner()
/=============================================================================================*/
    private executeCreateOwnerDEV(){
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
    }
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton CreateUserButton;
    private javax.swing.JTextField EmailTextField;
    private javax.swing.JTextField Owner_IDTextField;
    private javax.swing.JLabel Owner_ID_label;
    private javax.swing.JLabel Owner_ID_label3;
    private javax.swing.JLabel Owner_ID_label4;
    private javax.swing.JLabel Owner_ID_label5;
    private javax.swing.JPasswordField PasswordField;
    private javax.swing.JPasswordField confirmPasswordField;
    private javax.swing.JPanel jPanel1;
    // End of variables declaration//GEN-END:variables
}
