/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/GUIForms/JPanel.java to edit this template
 */
package edu.caltech.palomar.instruments.ngps.gui;

import edu.caltech.palomar.instruments.ngps.dbms.NGPSdatabase;
import edu.caltech.palomar.instruments.ngps.object.ObservationSet;
import edu.caltech.palomar.instruments.ngps.tables.ObservationSetTableModel;
import java.beans.PropertyChangeEvent;
import javax.swing.ImageIcon;
import javax.swing.JTable;
import javax.swing.ListSelectionModel;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;

/**
 *
 * @author jennifermilburn
 */
public class TargetSetPanel extends javax.swing.JPanel {
    private boolean          ALLOW_ROW_SELECTION = true;
    public NGPSdatabase     dbms;
    public java.lang.String  USERDIR           = System.getProperty("user.dir");
    public java.lang.String  SEP               = System.getProperty("file.separator");
    public java.lang.String  IMAGE_CACHE       = new java.lang.String(SEP + "images" + SEP);
    public ImageIcon         ON;
    public ImageIcon         OFF;
    public ImageIcon         UNKNOWN;  

    /**
     * Creates new form TargetSetPanel
     */
    public TargetSetPanel() {
        initComponents();
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
   }  
/*=============================================================================================
/     Property Change Listener for the Ephemeris_propertyChange
/=============================================================================================*/
  private void dbms_propertyChange(PropertyChangeEvent e)  {
     java.lang.String propertyName = e.getPropertyName();
     System.out.println(propertyName);
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
                        int selectedRow = lsm.getMinSelectionIndex();
                        ObservationSet selectedSet = ((ObservationSetTableModel)observation_set_Table.getModel()  ).getRecord(selectedRow);
                        dbms.setSelectedSET_ID(selectedSet.getSET_ID());
                        dbms.setSelectedSetName(selectedSet.getSET_NAME());
                        dbms.queryObservations(selectedSet.getSET_ID());
                        System.out.println("Row " + selectedRow + " is now selected.");
                        System.out.println("Observation Set ID = " + selectedSet.getSET_ID() + " is now selected.");
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

        Owner_ID_label3 = new javax.swing.JLabel();
        jScrollPane1 = new javax.swing.JScrollPane();
        observation_set_Table = new javax.swing.JTable();
        jLabel1 = new javax.swing.JLabel();
        jLabel2 = new javax.swing.JLabel();
        selected_set_nameLabel = new javax.swing.JLabel();
        set_id_Label = new javax.swing.JLabel();

        Owner_ID_label3.setFont(new java.awt.Font("Copperplate", 1, 18)); // NOI18N
        Owner_ID_label3.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        Owner_ID_label3.setText("Owner's Target Sets");

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

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(this);
        this.setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                        .addComponent(jScrollPane1, javax.swing.GroupLayout.DEFAULT_SIZE, 1047, Short.MAX_VALUE)
                        .addGroup(javax.swing.GroupLayout.Alignment.LEADING, layout.createSequentialGroup()
                            .addComponent(jLabel1)
                            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                            .addComponent(selected_set_nameLabel, javax.swing.GroupLayout.PREFERRED_SIZE, 146, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                            .addComponent(jLabel2)
                            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                            .addComponent(set_id_Label, javax.swing.GroupLayout.PREFERRED_SIZE, 146, javax.swing.GroupLayout.PREFERRED_SIZE)))
                    .addComponent(Owner_ID_label3, javax.swing.GroupLayout.PREFERRED_SIZE, 1037, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addGap(13, 13, 13))
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addComponent(Owner_ID_label3)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(jLabel1)
                        .addComponent(jLabel2)
                        .addComponent(selected_set_nameLabel, javax.swing.GroupLayout.PREFERRED_SIZE, 16, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addComponent(set_id_Label, javax.swing.GroupLayout.PREFERRED_SIZE, 16, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(jScrollPane1, javax.swing.GroupLayout.PREFERRED_SIZE, 591, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(16, Short.MAX_VALUE))
        );
    }// </editor-fold>//GEN-END:initComponents


    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JLabel Owner_ID_label3;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JTable observation_set_Table;
    private javax.swing.JLabel selected_set_nameLabel;
    private javax.swing.JLabel set_id_Label;
    // End of variables declaration//GEN-END:variables
}
