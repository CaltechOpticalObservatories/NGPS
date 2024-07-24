/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/GUIForms/JFrame.java to edit this template
 */
package edu.caltech.palomar.instruments.ngps.gui;
import edu.caltech.palomar.instruments.ngps.dbms.NGPSdatabase;
import edu.caltech.palomar.instruments.ngps.object.Target;
import edu.caltech.palomar.instruments.ngps.tables.EditTableInterface;
import edu.caltech.palomar.instruments.ngps.util.TableRowTransferHandler;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.beans.PropertyChangeSupport;
import javax.swing.DropMode;
import javax.swing.JFrame;
import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;
import javax.swing.JTable;
import javax.swing.ListSelectionModel;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;
import edu.caltech.palomar.instruments.ngps.gui.NGPSFrame;
/**
 *
 * @author developer
 */
public class TargetSearchFrame extends javax.swing.JFrame {
  transient protected PropertyChangeSupport propertyChangeListeners = new PropertyChangeSupport(this);
    public NGPSdatabase dbms;
    private int         result_number;
    public NGPSFrame    myNGPSFrame;
    private  int        selected_table_row;
    /**
     * Creates new form TargetSearchFrame
     */
/*================================================================================================
/      constructTable()
/=================================================================================================*/
    public TargetSearchFrame() {
        initComponents();
         
        setDefaultCloseOperation(JFrame.HIDE_ON_CLOSE);
        this.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
               public void propertyChange(java.beans.PropertyChangeEvent e) {
                results_propertyChange(e);
              }
            }); 
    }
/*================================================================================================
/      constructTable()
/=================================================================================================*/
public void setNGPSFrame(NGPSFrame newNGPSFrame){
   myNGPSFrame = newNGPSFrame; 
}    
/*================================================================================================
/      constructTable()
/=================================================================================================*/
public void setNGPSdatabase(NGPSdatabase new_NGPSdatabase){
    dbms = new_NGPSdatabase;
    this.target_searchTable.setModel(dbms.myTargetQueryTableModel);
    this.setTitle("Query Database for Target:  OWNER = "+dbms.getOWNER());
    target_searchTable.setModel(dbms.myTargetQueryTableModel);
    initializeTableModel(target_searchTable,dbms.myTargetQueryTableModel);
}
/*=============================================================================================
/     Property Change Listener for the Ephemeris_propertyChange
/=============================================================================================*/
  private void results_propertyChange(PropertyChangeEvent e)  {
     java.lang.String propertyName = e.getPropertyName();
     System.out.println("TargetSearchFrame "+propertyName);
/*=============================================================================================
/    PARAMETERS FROM THE JSKYCALCMODEL
/=============================================================================================*/
    if(propertyName.matches("result_number")){        
        java.lang.Integer current_value = (java.lang.Integer)e.getNewValue(); 
        this.numberLabel.setText(current_value.toString());
    }
}
  /*================================================================================================
/      setTarget_Number(int new_target_number) 
/=================================================================================================*/
  public void setNumber_of_results(int new_result_number) {
    int  old_result_number = this.result_number;
    this.result_number = new_result_number;
   propertyChangeListeners.firePropertyChange("result_number", Integer.valueOf(old_result_number), Integer.valueOf(new_result_number));
  }
  public int getNumber_of_results() {
    return result_number;
  } 
/*================================================================================================
/        selectFITSFiles()
/=================================================================================================*/
    public void initializeTableModel(JTable current_table,EditTableInterface current_table_model){
//        current_table.setModel(current_table_model);
 //       current_table.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);
        current_table.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        current_table.setRowSelectionAllowed(true);  
        current_table.setDragEnabled(true);
        current_table.setDropMode(DropMode.INSERT_ROWS);
        current_table.setTransferHandler(new TableRowTransferHandler(current_table)); 
        JMenuItem copy_menu_item    = new JMenuItem("Copy");
        JPopupMenu popup            = new JPopupMenu("Editing Functions");
                      popup.add(copy_menu_item);

           copy_menu_item.addActionListener(new ActionListener() {
              public void actionPerformed(ActionEvent ev) {
                 System.out.println("Copy Menu Item pressed");
                 Target selected_copy_target = current_table_model.getRecord(selected_table_row);
                 myNGPSFrame.copy_stack.push(selected_copy_target);
                 myNGPSFrame.paste_menu_item.setEnabled(true);
                 myNGPSFrame.insert_copied_menu_item.setEnabled(true);
//                         myTableModel.delete(selected_table_row);
              }
           });

        current_table.setComponentPopupMenu(popup);
        current_table.getSelectionModel().addListSelectionListener(new ListSelectionListener(){
            public void valueChanged(ListSelectionEvent event) {
                 if(!event.getValueIsAdjusting() && current_table.getSelectedRow() != -1){
                     current_table_model.getRecord(selected_table_row).setSelected(false);
                     selected_table_row  = current_table.getSelectedRow();   
                     current_table_model.getRecord(selected_table_row).setSelected(true);
                  }
            }
            });
        current_table.addMouseListener(new MouseAdapter() {
            @Override
            public void mouseReleased(MouseEvent e) {
                int r = current_table.rowAtPoint(e.getPoint());
                if (r >= 0 && r < current_table.getRowCount()) {
                    current_table.setRowSelectionInterval(r, r);
                } else {
                    current_table.clearSelection();
                }
                int rowindex = current_table.getSelectedRow();
                if (rowindex < 0)
                    return;
                if (e.isPopupTrigger() && e.getComponent() instanceof JTable ) {
                    
                    popup.show(e.getComponent(), e.getX(), e.getY());
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

        jLabel1 = new javax.swing.JLabel();
        jLabel2 = new javax.swing.JLabel();
        target_searchTextField = new javax.swing.JTextField();
        searchButton = new javax.swing.JButton();
        cancelButton = new javax.swing.JButton();
        jScrollPane1 = new javax.swing.JScrollPane();
        target_searchTable = new javax.swing.JTable();
        numberLabel = new javax.swing.JLabel();
        jLabel4 = new javax.swing.JLabel();

        setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);

        jLabel1.setFont(new java.awt.Font("DejaVu Sans", 1, 18)); // NOI18N
        jLabel1.setText("Get Target From Database:");

        jLabel2.setFont(new java.awt.Font("DejaVu Sans", 1, 18)); // NOI18N
        jLabel2.setText("Search for Target Name =");

        searchButton.setFont(new java.awt.Font("DejaVu Sans", 1, 14)); // NOI18N
        searchButton.setText("Search");
        searchButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                searchButtonActionPerformed(evt);
            }
        });

        cancelButton.setFont(new java.awt.Font("DejaVu Sans", 1, 14)); // NOI18N
        cancelButton.setText("Cancel");
        cancelButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                cancelButtonActionPerformed(evt);
            }
        });

        jScrollPane1.setHorizontalScrollBarPolicy(javax.swing.ScrollPaneConstants.HORIZONTAL_SCROLLBAR_ALWAYS);
        jScrollPane1.setVerticalScrollBarPolicy(javax.swing.ScrollPaneConstants.VERTICAL_SCROLLBAR_ALWAYS);

        target_searchTable.setModel(new javax.swing.table.DefaultTableModel(
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
        jScrollPane1.setViewportView(target_searchTable);

        numberLabel.setFont(new java.awt.Font("DejaVu Sans", 1, 14)); // NOI18N
        numberLabel.setForeground(new java.awt.Color(4, 187, 15));
        numberLabel.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);

        jLabel4.setFont(new java.awt.Font("DejaVu Sans", 1, 14)); // NOI18N
        jLabel4.setText("Number of Results = ");

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(layout.createSequentialGroup()
                        .addGap(12, 12, 12)
                        .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                            .addGroup(layout.createSequentialGroup()
                                .addComponent(cancelButton)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                                .addComponent(searchButton)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(jLabel4)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(numberLabel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                            .addGroup(layout.createSequentialGroup()
                                .addComponent(jLabel2)
                                .addGap(3, 3, 3)
                                .addComponent(target_searchTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 237, javax.swing.GroupLayout.PREFERRED_SIZE)))
                        .addContainerGap(510, Short.MAX_VALUE))
                    .addGroup(layout.createSequentialGroup()
                        .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addGroup(layout.createSequentialGroup()
                                .addComponent(jLabel1)
                                .addGap(0, 0, Short.MAX_VALUE))
                            .addComponent(jScrollPane1))
                        .addContainerGap())))
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jLabel1)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(jLabel2)
                    .addComponent(target_searchTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(numberLabel, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.PREFERRED_SIZE, 26, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(cancelButton)
                        .addComponent(searchButton)
                        .addComponent(jLabel4)))
                .addGap(18, 18, 18)
                .addComponent(jScrollPane1, javax.swing.GroupLayout.DEFAULT_SIZE, 231, Short.MAX_VALUE)
                .addContainerGap())
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents

    private void cancelButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_cancelButtonActionPerformed
       setVisible(false);
    }//GEN-LAST:event_cancelButtonActionPerformed

    private void searchButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_searchButtonActionPerformed
        try{
           java.lang.String target_name = target_searchTextField.getText();
           target_name = target_name.trim();
           int result_count = dbms.executeQueryTargetPreparedStatement(target_name);
           selected_table_row = 0;
           this.target_searchTable.setModel(dbms.myTargetQueryTableModel);
           setNumber_of_results(result_count);
        }catch(Exception e){
          System.out.println(e.toString());
        }
    }//GEN-LAST:event_searchButtonActionPerformed
/*================================================================================================
/      add and remove Property Change Listeners
/=================================================================================================*/
  public synchronized void removePropertyChangeListener(PropertyChangeListener l) {
    propertyChangeListeners.removePropertyChangeListener(l);
  }
  public synchronized void addPropertyChangeListener(PropertyChangeListener l) {
    propertyChangeListeners.addPropertyChangeListener(l);
  }  
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
            java.util.logging.Logger.getLogger(TargetSearchFrame.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (InstantiationException ex) {
            java.util.logging.Logger.getLogger(TargetSearchFrame.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (IllegalAccessException ex) {
            java.util.logging.Logger.getLogger(TargetSearchFrame.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (javax.swing.UnsupportedLookAndFeelException ex) {
            java.util.logging.Logger.getLogger(TargetSearchFrame.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        }
        //</editor-fold>

        /* Create and display the form */
        java.awt.EventQueue.invokeLater(new Runnable() {
            public void run() {
                new TargetSearchFrame().setVisible(true);
            }
        });
    }

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton cancelButton;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JLabel jLabel4;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JLabel numberLabel;
    private javax.swing.JButton searchButton;
    private javax.swing.JTable target_searchTable;
    private javax.swing.JTextField target_searchTextField;
    // End of variables declaration//GEN-END:variables
}
