/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/GUIForms/JPanel.java to edit this template
 */
package edu.caltech.palomar.instruments.ngps.gui;

import edu.caltech.palomar.instruments.ngps.util.ColumnColorRenderer;
import edu.caltech.palomar.instruments.ngps.dbms.NGPSdatabase;
import edu.caltech.palomar.instruments.ngps.object.Target;
import edu.caltech.palomar.instruments.ngps.tables.EditTableInterface;
import edu.caltech.palomar.instruments.ngps.util.TableRowTransferHandler;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.util.Stack;
import javax.swing.DropMode;
import javax.swing.ImageIcon;
import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;
import javax.swing.JTable;
import javax.swing.ListSelectionModel;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;
import javax.swing.table.TableColumn;

/**
 *
 * @author jennifermilburn
 */
public class retrieveDatabasePanel extends javax.swing.JPanel {
    public java.lang.String  USERDIR           = System.getProperty("user.dir");
    public java.lang.String  SEP               = System.getProperty("file.separator");
    public java.lang.String  IMAGE_CACHE       = new java.lang.String(SEP + "images" + SEP);
    public ImageIcon         ON;
    public ImageIcon         OFF;
    public ImageIcon         UNKNOWN;  
    public NGPSdatabase     dbms;
    private  boolean         paste_state;
    private  int             selected_table_row;
    public Stack             copy_stack          = new Stack();
    /**
     * Creates new form retrieveDatabasePanel
     */
    public retrieveDatabasePanel() {
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
/     initialize()
/=============================================================================================*/
public void setDBMS(NGPSdatabase new_dbms){
        dbms = new_dbms;
        targetsDBMSTable.setModel(dbms.myTargetDBMSTableModel);
        targetsDBMSTable.setRowSelectionAllowed(true);    
        targetsDBMSTable.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);          
        targetsDBMSTable.getColumnModel().getColumn(0).setMinWidth(25);//
        targetsDBMSTable.getColumnModel().getColumn(1).setMinWidth(25);//
        targetsDBMSTable.getColumnModel().getColumn(2).setMinWidth(25);//
        targetsDBMSTable.getColumnModel().getColumn(3).setMinWidth(25);//
        targetsDBMSTable.getColumnModel().getColumn(4).setMinWidth(25);//
        targetsDBMSTable.getColumnModel().getColumn(5).setMinWidth(100);//
        targetsDBMSTable.getColumnModel().getColumn(6).setMinWidth(100);//
        targetsDBMSTable.getColumnModel().getColumn(7).setMinWidth(100);//
        targetsDBMSTable.getColumnModel().getColumn(8).setMinWidth(100);//
        targetsDBMSTable.getColumnModel().getColumn(9).setMinWidth(100);//
        targetsDBMSTable.getColumnModel().getColumn(10).setMinWidth(100);//
        targetsDBMSTable.getColumnModel().getColumn(11).setMinWidth(100);//
//        targetsDBMSTable.getColumnModel().getColumn(12).setMinWidth(100);//
//        targetsDBMSTable.getColumnModel().getColumn(13).setMinWidth(100);//
/*        targetsDBMSTable.getColumnModel().getColumn(14).setMinWidth(100);//
        targetsDBMSTable.getColumnModel().getColumn(15).setMinWidth(100);//
        targetsDBMSTable.getColumnModel().getColumn(16).setMinWidth(100);//
        targetsDBMSTable.getColumnModel().getColumn(17).setMinWidth(100);//
        targetsDBMSTable.getColumnModel().getColumn(18).setMinWidth(100);//
          TableColumn tColumn_otm16 = targetsDBMSTable.getColumnModel().getColumn(15);
          tColumn_otm16.setCellRenderer(new ColumnColorRenderer(java.awt.Color.lightGray, java.awt.Color.BLACK));            
          TableColumn tColumn_otm17 = targetsDBMSTable.getColumnModel().getColumn(16);
          tColumn_otm17.setCellRenderer(new ColumnColorRenderer(java.awt.Color.lightGray, java.awt.Color.BLACK));                       
          TableColumn tColumn_otm18 = targetsDBMSTable.getColumnModel().getColumn(17);
          tColumn_otm18.setCellRenderer(new ColumnColorRenderer(java.awt.Color.lightGray, java.awt.Color.BLACK));            
          TableColumn tColumn_otm19 = targetsDBMSTable.getColumnModel().getColumn(18);
          tColumn_otm19.setCellRenderer(new ColumnColorRenderer(java.awt.Color.lightGray, java.awt.Color.BLACK));            
          TableColumn tColumn_otm20 = targetsDBMSTable.getColumnModel().getColumn(19);
          tColumn_otm20.setCellRenderer(new ColumnColorRenderer(java.awt.Color.lightGray, java.awt.Color.BLACK));            
          TableColumn tColumn_otm21 = targetsDBMSTable.getColumnModel().getColumn(20);
          tColumn_otm21.setCellRenderer(new ColumnColorRenderer(java.awt.Color.lightGray, java.awt.Color.BLACK));            
          TableColumn tColumn_otm22 = targetsDBMSTable.getColumnModel().getColumn(21);
          tColumn_otm22.setCellRenderer(new ColumnColorRenderer(java.awt.Color.lightGray, java.awt.Color.BLACK));  
*/
        targetsDBMSTable.getSelectionModel().addListSelectionListener(new ListSelectionListener(){
        public void valueChanged(ListSelectionEvent event) {
             if(!event.getValueIsAdjusting() && targetsDBMSTable.getSelectedRow() != -1){
                 selected_table_row = targetsDBMSTable.getSelectedRow();
              }
        }
        });          
    initializeTableModel(targetsDBMSTable,dbms.myTargetDBMSTableModel);
}  
/*=============================================================================================
/     setPaste(boolean state)
/=============================================================================================*/
    public void setPaste(boolean state){
        paste_state = state;
    }
/*=============================================================================================
/     initializeDatabase()`````````
/=============================================================================================*/
    public void initializeTableModel(JTable current_table,EditTableInterface current_table_model){
        current_table.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);
        current_table.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        current_table.setRowSelectionAllowed(true);  
        current_table.setDragEnabled(true);
        current_table.setDropMode(DropMode.INSERT_ROWS);
        current_table.setTransferHandler(new TableRowTransferHandler(current_table)); 
        JMenuItem insert_menu_item  = new JMenuItem("Insert");
        JMenuItem delete_menu_item  = new JMenuItem("Delete");
        JMenuItem copy_menu_item    = new JMenuItem("Copy");
        JMenuItem paste_menu_item   = new JMenuItem("Paste");
        paste_menu_item.setEnabled(false);
           JPopupMenu popup          = new JPopupMenu("Editing Functions");
                      popup.add(insert_menu_item);
                      popup.add(delete_menu_item);
                      popup.add(copy_menu_item);
                      popup.add(paste_menu_item);
           insert_menu_item.addActionListener(new ActionListener() {
              public void actionPerformed(ActionEvent ev) {
                 System.out.println("Insert Menu Item pressed");
                 current_table_model.insert(selected_table_row, new Target());
              }
           });
           delete_menu_item.addActionListener(new ActionListener() {
              public void actionPerformed(ActionEvent ev) {
                 System.out.println("Delete Menu Item pressed"); 
                 current_table_model.delete(selected_table_row);
              }
           });
           copy_menu_item.addActionListener(new ActionListener() {
              public void actionPerformed(ActionEvent ev) {
                 System.out.println("Copy Menu Item pressed");
                 Target selected_copy_target = current_table_model.getRecord(selected_table_row);
                 copy_stack.push(selected_copy_target);
                 paste_menu_item.setEnabled(true);
//                         myTableModel.delete(selected_table_row);
              }
           });
           paste_menu_item.addActionListener(new ActionListener() {
              public void actionPerformed(ActionEvent ev) {
                 System.out.println("Paste Menu Item pressed"); 
                 Target current_target = (Target)copy_stack.pop();
                 current_table_model.delete(selected_table_row);
                 current_table_model.insert(selected_table_row, current_target);
                 current_table_model.reorder_table();
                 boolean state = copy_stack.empty();
                 if(state){
                    setPaste(false); 
                    paste_menu_item.setEnabled(false);
                 }
              }
           });
        current_table.setComponentPopupMenu(popup);
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

        jScrollPane2 = new javax.swing.JScrollPane();
        targetsDBMSTable = new javax.swing.JTable();

        jScrollPane2.setHorizontalScrollBarPolicy(javax.swing.ScrollPaneConstants.HORIZONTAL_SCROLLBAR_ALWAYS);
        jScrollPane2.setVerticalScrollBarPolicy(javax.swing.ScrollPaneConstants.VERTICAL_SCROLLBAR_ALWAYS);

        targetsDBMSTable.setModel(new javax.swing.table.DefaultTableModel(
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
        jScrollPane2.setViewportView(targetsDBMSTable);

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(this);
        this.setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 400, Short.MAX_VALUE)
            .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(layout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(jScrollPane2, javax.swing.GroupLayout.DEFAULT_SIZE, 388, Short.MAX_VALUE)
                    .addContainerGap()))
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 300, Short.MAX_VALUE)
            .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(layout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(jScrollPane2, javax.swing.GroupLayout.DEFAULT_SIZE, 294, Short.MAX_VALUE)))
        );
    }// </editor-fold>//GEN-END:initComponents


    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JScrollPane jScrollPane2;
    private javax.swing.JTable targetsDBMSTable;
    // End of variables declaration//GEN-END:variables
}
