/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/GUIForms/JFrame.java to edit this template
 */
package edu.caltech.palomar.instruments.ngps.gui;
import edu.caltech.palomar.instruments.ngps.dbms.NGPSdatabase;
import edu.caltech.palomar.instruments.ngps.os.ObservationSequencerController;
import edu.caltech.palomar.instruments.ngps.parser.TargetListParser2;
import static edu.caltech.palomar.instruments.ngps.parser.TargetListParser2.DEFAULT_MODEL;
import static edu.caltech.palomar.instruments.ngps.parser.TargetListParser2.ETC_MODEL;
import edu.caltech.palomar.instruments.ngps.tables.EditTableInterface;
import edu.caltech.palomar.instruments.ngps.util.TableRowTransferHandler;
import java.beans.PropertyChangeEvent;
import java.util.Stack;
import javax.swing.ButtonGroup;
import javax.swing.JTable;
import javax.swing.ListSelectionModel;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;
import javax.swing.JOptionPane;
/**
 *
 * @author jennifermilburn
 */
public class ImportFrame extends javax.swing.JFrame {
    public   TargetListParser2 myTargetListParser;
    private  JTable            extendedTargetTable;
    public   NGPSdatabase      dbms;
    public   java.lang.String  USERDIR           = System.getProperty("user.dir");
    public   java.lang.String  SEP               = System.getProperty("file.separator");
    public   ButtonGroup       simple_extended_buttongroup = new ButtonGroup();
    public   NGPSFrame         myNGPSFrame;
    public ObservationSequencerController myObservationSequencerController;
    /**
     * Creates new form ImportFrame
     */
    public ImportFrame() {
        initComponents();
        setTitle("Import CSV File");
        initialize();   
        //setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);     
    }
/*=============================================================================================
/     setNGPSFrame(NGPSFrame newNGPSFrame)
/=============================================================================================*/
public void setNGPSFrame(NGPSFrame newNGPSFrame){
    myNGPSFrame = newNGPSFrame;
} 
/*=============================================================================================
/     setObservationSequencerController(ObservationSequencerController current)
/=============================================================================================*/
public void setObservationSequencerController(ObservationSequencerController current){
    myObservationSequencerController = current;
}
/*=============================================================================================
/     execute_save_to_database()
/=============================================================================================*/
 public void execute_save_to_database(){
     ExecuteSaveToDMBSThread myExecuteSaveToDMBSThread = new ExecuteSaveToDMBSThread();
     myExecuteSaveToDMBSThread.start();
 }
/*=============================================================================================
/     save_to_database()
/=============================================================================================*/
public void save_to_database(){
    
    String current_set_name = dbms.promptForSetName();
    setVisible(false); //Hide the label input dialog either way

    if(current_set_name != null){
        int set_id = myTargetListParser.commitFileToDBMS_2(myTargetListParser.currentObservationSet,current_set_name); 
        myTargetListParser.getNGPSdatabase().queryObservations(set_id);
      //  myNGPSFrame.getMainTable().setModel(myTargetListParser.getNGPSdatabase().myTargetDBMSTableModel);
    //     java.lang.String start_time_string = OTMstartTimeTextField.getText();
        double seeing = dbms.myOTMlauncher.getSeeing();
        int wavelength = dbms.myOTMlauncher.getWavelength();
        myNGPSFrame.updateStarTime();
           java.sql.Timestamp current_timestamp_otm = dbms.myOTMlauncher.getStartTimestamp();       
        dbms.myOTMlauncher.OTM(current_timestamp_otm,seeing,wavelength);
        dbms.setTablePopulated(true);
        if(myObservationSequencerController.getSTATE().matches("READY_NO_TARGETS")){
            if(dbms.myTargetDBMSTableModel.getRowCount() != 0){
               myObservationSequencerController.setSTATE("READY");
            }
        }
    }    
 
}       
/*=============================================================================================
/     initializeDatabase()
/=============================================================================================*/
private void initialize(){
    simple_extended_buttongroup.add(simpleRadioButton);
    simple_extended_buttongroup.add(extendedRadioButton);
    simpleRadioButton.setSelected(true);
} 
/*=============================================================================================
/    setModelType(int model_type)
/=============================================================================================*/
public void setModelType(int model_type){
    if(model_type == DEFAULT_MODEL){
        change_to_simpleTable();
    }
    if(model_type == ETC_MODEL){
        change_to_extendedTable();
    }    
}
/*=============================================================================================
/     initializeDatabase()`````````
/=============================================================================================*/
 public void initializeTables(){
     extendedTargetTable = new JTable();
     simpleTargetTable.setModel(myTargetListParser.myTargetSimpleTableModel);
     extendedTargetTable.setModel(myTargetListParser.myTargetExtendedTableModel);
     initializeTableModel(simpleTargetTable,myTargetListParser.myTargetSimpleTableModel);  
     initializeTableModel(extendedTargetTable,myTargetListParser.myTargetExtendedTableModel); 
 } 
/*=============================================================================================
/     initializeDatabase()`````````
/=============================================================================================*/
 public void change_to_extendedTable(){
  if(simpleTargetTable != null){
    targetScrollPane.remove(simpleTargetTable);
    targetScrollPane.setViewportView(extendedTargetTable);      
  }
 }
 public void change_to_simpleTable(){
   if(extendedTargetTable != null){
      targetScrollPane.remove(extendedTargetTable);
      targetScrollPane.setViewportView(simpleTargetTable);      
   }
 } 
/*=============================================================================================
/     setParser(TargetListParser2 newTargetListParser)`
/=============================================================================================*/
    public void setParser(TargetListParser2 newTargetListParser){
        myTargetListParser = newTargetListParser;
        myTargetListParser.setJEditorPane(observationTextEditorPane);
        myTargetListParser.setJTable(simpleTargetTable);
        observationTextEditorPane.setDocument(myTargetListParser.myTargetListDocumentModel.getDocument());
        initializeTables();
        myTargetListParser.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
          public void propertyChange(java.beans.PropertyChangeEvent e) {
                 parser_propertyChange(e);
          }
      });
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
/     initializeDatabase()`````````
/=============================================================================================*/
    public void initializeTableModel(JTable current_table,EditTableInterface current_table_model){
        current_table.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);
        current_table.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        current_table.setRowSelectionAllowed(false);  
        current_table.setDragEnabled(false);
        current_table.setTransferHandler(new TableRowTransferHandler(current_table)); 
    } 
/*=============================================================================================
/     Property Change Listener for the Ephemeris_propertyChange
/=============================================================================================*/
  private void dbms_propertyChange(PropertyChangeEvent e)  {
     java.lang.String propertyName = e.getPropertyName();
     System.out.println("importFrame: "+propertyName);

    if(propertyName.matches("selected_set_id")){
        java.lang.Integer current_value = (java.lang.Integer)e.getNewValue(); 
    }
}
/*=============================================================================================
/     Property Change Listener for the parser_propertyChange(PropertyChangeEvent e)
/=============================================================================================*/
private void parser_propertyChange(PropertyChangeEvent e)  {
     java.lang.String propertyName = e.getPropertyName();
         if(propertyName.matches("current_file_name")){
             java.lang.String current_value = (java.lang.String)e.getNewValue(); 
             selectedFileLabel.setText(current_value);
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
        jTabbedPane2 = new javax.swing.JTabbedPane();
        jPanel8 = new javax.swing.JPanel();
        targetScrollPane = new javax.swing.JScrollPane();
        simpleTargetTable = new javax.swing.JTable();
        jPanel7 = new javax.swing.JPanel();
        jScrollPane6 = new javax.swing.JScrollPane();
        observationTextEditorPane = new javax.swing.JTextPane();
        jPanel2 = new javax.swing.JPanel();
        extendedRadioButton = new javax.swing.JRadioButton();
        simpleRadioButton = new javax.swing.JRadioButton();
        jLabel1 = new javax.swing.JLabel();
        selectedFileLabel = new javax.swing.JLabel();
        CanelButton = new javax.swing.JButton();
        OKButton = new javax.swing.JButton();

        jTabbedPane2.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0), 3));

        targetScrollPane.setHorizontalScrollBarPolicy(javax.swing.ScrollPaneConstants.HORIZONTAL_SCROLLBAR_ALWAYS);
        targetScrollPane.setVerticalScrollBarPolicy(javax.swing.ScrollPaneConstants.VERTICAL_SCROLLBAR_ALWAYS);

        simpleTargetTable.setModel(new javax.swing.table.DefaultTableModel(
            new Object [][] {

            },
            new String [] {
                "Name", "RA", "DEC", "EPOCH", "EXPTIME", "NIMAGES"
            }
        ) {
            Class[] types = new Class [] {
                java.lang.String.class, java.lang.String.class, java.lang.String.class, java.lang.Double.class, java.lang.Double.class, java.lang.Integer.class
            };

            public Class getColumnClass(int columnIndex) {
                return types [columnIndex];
            }
        });
        targetScrollPane.setViewportView(simpleTargetTable);

        javax.swing.GroupLayout jPanel8Layout = new javax.swing.GroupLayout(jPanel8);
        jPanel8.setLayout(jPanel8Layout);
        jPanel8Layout.setHorizontalGroup(
            jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel8Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(targetScrollPane, javax.swing.GroupLayout.DEFAULT_SIZE, 1026, Short.MAX_VALUE)
                .addContainerGap())
        );
        jPanel8Layout.setVerticalGroup(
            jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel8Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(targetScrollPane, javax.swing.GroupLayout.DEFAULT_SIZE, 386, Short.MAX_VALUE)
                .addGap(4, 4, 4))
        );

        jTabbedPane2.addTab("Table View", jPanel8);

        jScrollPane6.setHorizontalScrollBarPolicy(javax.swing.ScrollPaneConstants.HORIZONTAL_SCROLLBAR_ALWAYS);
        jScrollPane6.setVerticalScrollBarPolicy(javax.swing.ScrollPaneConstants.VERTICAL_SCROLLBAR_ALWAYS);
        jScrollPane6.setViewportView(observationTextEditorPane);

        javax.swing.GroupLayout jPanel7Layout = new javax.swing.GroupLayout(jPanel7);
        jPanel7.setLayout(jPanel7Layout);
        jPanel7Layout.setHorizontalGroup(
            jPanel7Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel7Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jScrollPane6, javax.swing.GroupLayout.DEFAULT_SIZE, 1026, Short.MAX_VALUE)
                .addContainerGap())
        );
        jPanel7Layout.setVerticalGroup(
            jPanel7Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel7Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jScrollPane6, javax.swing.GroupLayout.DEFAULT_SIZE, 386, Short.MAX_VALUE)
                .addGap(4, 4, 4))
        );

        jTabbedPane2.addTab("Text View", jPanel7);

        jPanel2.setBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 0, 0), 3, true));

        extendedRadioButton.setText("Extended view");
        extendedRadioButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                extendedRadioButtonActionPerformed(evt);
            }
        });

        simpleRadioButton.setText("Simple view");
        simpleRadioButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                simpleRadioButtonActionPerformed(evt);
            }
        });

        jLabel1.setText("Selected File:");

        javax.swing.GroupLayout jPanel2Layout = new javax.swing.GroupLayout(jPanel2);
        jPanel2.setLayout(jPanel2Layout);
        jPanel2Layout.setHorizontalGroup(
            jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel2Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(simpleRadioButton, javax.swing.GroupLayout.PREFERRED_SIZE, 118, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(extendedRadioButton)
                .addGap(36, 36, 36)
                .addComponent(jLabel1)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(selectedFileLabel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addGap(170, 170, 170))
        );
        jPanel2Layout.setVerticalGroup(
            jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel2Layout.createSequentialGroup()
                .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING, false)
                    .addComponent(selectedFileLabel, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(simpleRadioButton)
                        .addComponent(extendedRadioButton)
                        .addComponent(jLabel1)))
                .addGap(0, 9, Short.MAX_VALUE))
        );

        CanelButton.setText("Cancel");
        CanelButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                CanelButtonActionPerformed(evt);
            }
        });

        OKButton.setText("OK");
        OKButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                OKButtonActionPerformed(evt);
            }
        });

        javax.swing.GroupLayout jPanel1Layout = new javax.swing.GroupLayout(jPanel1);
        jPanel1.setLayout(jPanel1Layout);
        jPanel1Layout.setHorizontalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(jPanel2, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
            .addComponent(jTabbedPane2, javax.swing.GroupLayout.DEFAULT_SIZE, 1058, Short.MAX_VALUE)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel1Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(OKButton)
                .addGap(18, 18, 18)
                .addComponent(CanelButton)
                .addGap(23, 23, 23))
        );
        jPanel1Layout.setVerticalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel1Layout.createSequentialGroup()
                .addComponent(jPanel2, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jTabbedPane2, javax.swing.GroupLayout.DEFAULT_SIZE, 440, Short.MAX_VALUE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(OKButton)
                    .addComponent(CanelButton))
                .addGap(4, 4, 4))
        );

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 706, Short.MAX_VALUE)
            .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(layout.createSequentialGroup()
                    .addGap(6, 6, 6)
                    .addComponent(jPanel1, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addGap(6, 6, 6)))
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 541, Short.MAX_VALUE)
            .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(layout.createSequentialGroup()
                    .addGap(6, 6, 6)
                    .addComponent(jPanel1, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addGap(6, 6, 6)))
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents

    private void OKButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_OKButtonActionPerformed
       execute_save_to_database();
    }//GEN-LAST:event_OKButtonActionPerformed

    private void CanelButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_CanelButtonActionPerformed
       setVisible(false);       
    }//GEN-LAST:event_CanelButtonActionPerformed

    private void extendedRadioButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_extendedRadioButtonActionPerformed
        change_to_extendedTable();
    }//GEN-LAST:event_extendedRadioButtonActionPerformed

    private void simpleRadioButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_simpleRadioButtonActionPerformed
        change_to_simpleTable();
    }//GEN-LAST:event_simpleRadioButtonActionPerformed
/*=============================================================================================
/     INNER CLASS that executes a process in separate thread so that it doesn't
 *     block execution of the rest of the system. 
/=============================================================================================*/
public class ExecuteSaveToDMBSThread  implements Runnable{
    private Thread             myThread;
    
    public ExecuteSaveToDMBSThread(){
    }
    public void run(){
        try{  
            save_to_database();
        }catch(Exception e){
           System.out.println(e.toString());
        }
     }
    public void start(){
     myThread = new Thread(this);
     myThread.start();
     }
    }// End of the ExecuteProcessThread Inner Class
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
            java.util.logging.Logger.getLogger(ImportFrame.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (InstantiationException ex) {
            java.util.logging.Logger.getLogger(ImportFrame.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (IllegalAccessException ex) {
            java.util.logging.Logger.getLogger(ImportFrame.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (javax.swing.UnsupportedLookAndFeelException ex) {
            java.util.logging.Logger.getLogger(ImportFrame.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        }
        //</editor-fold>

        /* Create and display the form */
        java.awt.EventQueue.invokeLater(new Runnable() {
            public void run() {
                new ImportFrame().setVisible(true);
            }
        });
    }

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton CanelButton;
    private javax.swing.JButton OKButton;
    private javax.swing.JRadioButton extendedRadioButton;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JPanel jPanel2;
    private javax.swing.JPanel jPanel7;
    private javax.swing.JPanel jPanel8;
    private javax.swing.JScrollPane jScrollPane6;
    private javax.swing.JTabbedPane jTabbedPane2;
    private javax.swing.JTextPane observationTextEditorPane;
    private javax.swing.JLabel selectedFileLabel;
    private javax.swing.JRadioButton simpleRadioButton;
    private javax.swing.JTable simpleTargetTable;
    private javax.swing.JScrollPane targetScrollPane;
    // End of variables declaration//GEN-END:variables
}
