/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/GUIForms/JFrame.java to edit this template
 */
package edu.caltech.palomar.instruments.ngps.gui;

import edu.caltech.palomar.instruments.ngps.dbms.NGPSdatabase;
import javax.swing.ImageIcon;
import edu.caltech.palomar.instruments.ngps.dbms.NGPSdatabase;
import edu.caltech.palomar.instruments.ngps.object.ObservationSet;
import edu.caltech.palomar.instruments.ngps.object.Target;
import edu.caltech.palomar.instruments.ngps.tables.ObservationSetTableModel;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.beans.PropertyChangeEvent;
import javax.swing.ImageIcon;
import javax.swing.JTable;
import javax.swing.ListSelectionModel;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;
import javax.swing.JFrame;
import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;
import edu.caltech.palomar.instruments.ngps.os.ObservationSequencerController;

/**
 *
 * @author jennifermilburn
 */
public class TargetSetFrame extends javax.swing.JFrame {
    private boolean          ALLOW_ROW_SELECTION = true;
    public NGPSdatabase     dbms;
    public java.lang.String  USERDIR           = System.getProperty("user.dir");
    public java.lang.String  SEP               = System.getProperty("file.separator");
    public java.lang.String  IMAGE_CACHE       = new java.lang.String(SEP + "images" + SEP);
    public ImageIcon         ON;
    public ImageIcon         OFF;
    public ImageIcon         UNKNOWN;  
    public ObservationSet selectedSet;
    private int           selected_table_row;
    public ObservationSequencerController myObservationSequencerController;


    /**
     * Creates new form TargetSetFrame
     */
    public TargetSetFrame() {
        initComponents();
        initialize();
        setDefaultCloseOperation(JFrame.HIDE_ON_CLOSE);
    }
/*=============================================================================================
/     setObservationSequencerController(ObservationSequencerController current)
/=============================================================================================*/
public void setObservationSequencerController(ObservationSequencerController current){
    myObservationSequencerController = current;
}
/*=============================================================================================
/     initialize()
/=============================================================================================*/
private void initialize(){
    ON           = new ImageIcon(USERDIR + IMAGE_CACHE + "ON.png");
    OFF          = new ImageIcon(USERDIR + IMAGE_CACHE + "OFF.png");
    UNKNOWN      = new ImageIcon(USERDIR + IMAGE_CACHE + "UNKNOWN.gif");    
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
    observation_set_Table.setModel(dbms.myObservationSetTableModel);
    observation_set_Table.getColumnModel().getColumn(0).setMinWidth(20);//
    observation_set_Table.getColumnModel().getColumn(1).setMinWidth(140);//
    observation_set_Table.getColumnModel().getColumn(2).setMinWidth(100);//
    observation_set_Table.getColumnModel().getColumn(3).setMinWidth(60);//
    observation_set_Table.getColumnModel().getColumn(4).setMinWidth(100);//
    observation_set_Table.getColumnModel().getColumn(5).setMinWidth(200);//
    observation_set_Table.getColumnModel().getColumn(6).setMinWidth(200);//           
    observation_set_Table.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);
    observation_set_Table.setSelectionMode(javax.swing.ListSelectionModel.SINGLE_SELECTION);
    observation_set_Table.setRowSelectionAllowed(true);  
    configureUserTableListener();
    dbms.queryObservationSets(dbms.getOWNER());
           JMenuItem delete_menu_item  = new JMenuItem("Delete");
           JPopupMenu popup          = new JPopupMenu("Editing Functions");
                      popup.add(delete_menu_item);
                      observation_set_Table.setComponentPopupMenu(popup);
           delete_menu_item.addActionListener(new ActionListener() {
              public void actionPerformed(ActionEvent ev) {
                 System.out.println("Delete Menu Item pressed"); 
                 ObservationSet current_set = dbms.myObservationSetTableModel.getRecord(selected_table_row);
                 java.lang.String set_name = current_set.getSET_NAME();
                 int              set_id   = current_set.getSET_ID();
                 int response = displayConfirmationDialog(set_name,set_id);
                 if(response == 0){
                    dbms.deleteObservationSet(set_id);
                    dbms.queryObservationSets(dbms.getOWNER());
                 }
                 if(response == 1){
                     System.out.println("response = 1");
                 }              
              }
           });
           

   } 
private int displayConfirmationDialog(java.lang.String current_set_name,int current_setid){
  int n = javax.swing.JOptionPane.showConfirmDialog(this,
                        "Delete Set Name = "+current_set_name+" Set ID = "+current_setid+"?","Do you really want to delete this Observation Set?",javax.swing.JOptionPane.YES_NO_OPTION);  
  return n;
}
/*=============================================================================================
/     Property Change Listener for the Ephemeris_propertyChange 
/=============================================================================================*/
  private void dbms_propertyChange(PropertyChangeEvent e)  {
     java.lang.String propertyName = e.getPropertyName();
     System.out.println("targetSetFrame "+propertyName);
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
    }
    if(propertyName.matches("owner")){
        java.lang.String current_value = (java.lang.String)e.getNewValue(); 
    
    }
    if(propertyName.matches("selected_set_name")){
        java.lang.String current_value = (java.lang.String)e.getNewValue(); 
        selected_set_nameLabel.setText(current_value);
    }
   if(propertyName.matches("selected_set_id")){
        java.lang.Integer current_value = (java.lang.Integer)e.getNewValue(); 
        set_id_Label.setText(current_value.toString());
   }
} 
/*================================================================================================
/     configureUserTableListener()
/=================================================================================================*/
public void query_target_set(){
    dbms.setSelectedSET_ID(selectedSet.getSET_ID());
    dbms.setSelectedSetName(selectedSet.getSET_NAME());
    dbms.queryObservations(selectedSet.getSET_ID());
    System.out.println("Observation Set ID = " + selectedSet.getSET_ID() + " is now selected.");  
    setVisible(false);
    dbms.setTablePopulated(true);
    if(myObservationSequencerController.getSTATE().matches("READY_NO_TARGETS")){
       if(dbms.myTargetDBMSTableModel.getRowCount() != 0){
           myObservationSequencerController.setSTATE("READY");
       }
    }
}  
/*================================================================================================
/     configureUserTableListener()
/=================================================================================================*/
public void configureUserTableListener(){
        observation_set_Table.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        if (ALLOW_ROW_SELECTION) { // true by default
            ListSelectionModel rowSM = observation_set_Table.getSelectionModel();
            rowSM.addListSelectionListener(new ListSelectionListener() {
                public void valueChanged(ListSelectionEvent e) {
                    //Ignore extra messages.
                    if (e.getValueIsAdjusting()) return;
 
                    ListSelectionModel lsm = (ListSelectionModel)e.getSource();
                    if (lsm.isSelectionEmpty()) {
                        System.out.println("No rows are selected.");
                    } else {
                        selected_table_row = lsm.getMinSelectionIndex();
                        selectedSet = ((ObservationSetTableModel)observation_set_Table.getModel()  ).getRecord(selected_table_row);
                        dbms.setSelectedObservationSet(selectedSet);
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

        jPanel1 = new javax.swing.JPanel();
        jScrollPane1 = new javax.swing.JScrollPane();
        observation_set_Table = new javax.swing.JTable();
        jLabel1 = new javax.swing.JLabel();
        jLabel2 = new javax.swing.JLabel();
        selected_set_nameLabel = new javax.swing.JLabel();
        set_id_Label = new javax.swing.JLabel();
        Owner_ID_label3 = new javax.swing.JLabel();
        jPanel2 = new javax.swing.JPanel();
        CancelButton = new javax.swing.JButton();
        OKButton = new javax.swing.JButton();

        setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);

        jScrollPane1.setHorizontalScrollBarPolicy(javax.swing.ScrollPaneConstants.HORIZONTAL_SCROLLBAR_ALWAYS);
        jScrollPane1.setVerticalScrollBarPolicy(javax.swing.ScrollPaneConstants.VERTICAL_SCROLLBAR_ALWAYS);

        observation_set_Table.setModel(new javax.swing.table.DefaultTableModel(
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
        jScrollPane1.setViewportView(observation_set_Table);

        jLabel1.setText("Selected Target Set");

        jLabel2.setText("Selected Target Set ID");

        selected_set_nameLabel.setForeground(new java.awt.Color(0, 153, 102));

        set_id_Label.setForeground(new java.awt.Color(0, 153, 102));

        javax.swing.GroupLayout jPanel1Layout = new javax.swing.GroupLayout(jPanel1);
        jPanel1.setLayout(jPanel1Layout);
        jPanel1Layout.setHorizontalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel1Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jLabel1)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(selected_set_nameLabel, javax.swing.GroupLayout.PREFERRED_SIZE, 146, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(jLabel2)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(set_id_Label, javax.swing.GroupLayout.PREFERRED_SIZE, 146, javax.swing.GroupLayout.PREFERRED_SIZE))
            .addComponent(jScrollPane1, javax.swing.GroupLayout.PREFERRED_SIZE, 869, javax.swing.GroupLayout.PREFERRED_SIZE)
        );
        jPanel1Layout.setVerticalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel1Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(jLabel1)
                        .addComponent(jLabel2)
                        .addComponent(selected_set_nameLabel, javax.swing.GroupLayout.PREFERRED_SIZE, 16, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addComponent(set_id_Label, javax.swing.GroupLayout.PREFERRED_SIZE, 16, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(jScrollPane1, javax.swing.GroupLayout.DEFAULT_SIZE, 441, Short.MAX_VALUE))
        );

        Owner_ID_label3.setFont(new java.awt.Font("Copperplate", 1, 18)); // NOI18N
        Owner_ID_label3.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        Owner_ID_label3.setText("Owner's Target Sets");

        jPanel2.setBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 0, 0), 1, true));

        CancelButton.setText("Cancel");
        CancelButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                CancelButtonActionPerformed(evt);
            }
        });

        OKButton.setText("OK");
        OKButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                OKButtonActionPerformed(evt);
            }
        });

        javax.swing.GroupLayout jPanel2Layout = new javax.swing.GroupLayout(jPanel2);
        jPanel2.setLayout(jPanel2Layout);
        jPanel2Layout.setHorizontalGroup(
            jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel2Layout.createSequentialGroup()
                .addContainerGap(798, Short.MAX_VALUE)
                .addComponent(CancelButton)
                .addGap(15, 15, 15))
            .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel2Layout.createSequentialGroup()
                    .addContainerGap(696, Short.MAX_VALUE)
                    .addComponent(OKButton)
                    .addGap(91, 91, 91)))
        );
        jPanel2Layout.setVerticalGroup(
            jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel2Layout.createSequentialGroup()
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addComponent(CancelButton)
                .addContainerGap())
            .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel2Layout.createSequentialGroup()
                    .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(OKButton)
                    .addContainerGap()))
        );

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(layout.createSequentialGroup()
                        .addContainerGap()
                        .addComponent(Owner_ID_label3, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                    .addComponent(jPanel2, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap())
            .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(layout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(jPanel1, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addContainerGap(15, Short.MAX_VALUE)))
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(Owner_ID_label3)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, 496, Short.MAX_VALUE)
                .addComponent(jPanel2, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap())
            .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(layout.createSequentialGroup()
                    .addGap(32, 32, 32)
                    .addComponent(jPanel1, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addGap(89, 89, 89)))
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents

    private void OKButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_OKButtonActionPerformed
        query_target_set();
    }//GEN-LAST:event_OKButtonActionPerformed

    private void CancelButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_CancelButtonActionPerformed
        this.setVisible(false);
    }//GEN-LAST:event_CancelButtonActionPerformed

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
            java.util.logging.Logger.getLogger(TargetSetFrame.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (InstantiationException ex) {
            java.util.logging.Logger.getLogger(TargetSetFrame.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (IllegalAccessException ex) {
            java.util.logging.Logger.getLogger(TargetSetFrame.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (javax.swing.UnsupportedLookAndFeelException ex) {
            java.util.logging.Logger.getLogger(TargetSetFrame.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        }
        //</editor-fold>

        /* Create and display the form */
        java.awt.EventQueue.invokeLater(new Runnable() {
            public void run() {
                new TargetSetFrame().setVisible(true);
            }
        });
    }

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton CancelButton;
    private javax.swing.JButton OKButton;
    private javax.swing.JLabel Owner_ID_label3;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JPanel jPanel2;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JTable observation_set_Table;
    private javax.swing.JLabel selected_set_nameLabel;
    private javax.swing.JLabel set_id_Label;
    // End of variables declaration//GEN-END:variables
}
