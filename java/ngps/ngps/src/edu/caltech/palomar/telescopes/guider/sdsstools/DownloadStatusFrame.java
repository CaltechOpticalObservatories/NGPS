
package edu.caltech.palomar.telescopes.guider.sdsstools; 
//=== File Prolog =============================================================
//
//      Caltech Optical Observatories, Palomar Observatory
//
//--- Contents ----------------------------------------------------------------
//    DownloadStatusFrame  - Same as the AstroObjectTableModel but with a boolean
//    that indicates the download state of the image for the guider display
//    January 25, 2011 Jennifer Milburn
//
//--- Description -------------------------------------------------------------
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
//
// TERMS OF USE --
// Anyone is free to use this software for any purpose, and to
// modify it for their own purposes, provided that credit is given to the author
// in a prominent place.  For the present program that means that the green
// title and author banner appearing on the main window must not be removed,
// and may not be altered without premission of the author.
//
//--- DISCLAIMER---------------------------------------------------------------
//
//	This software is provided "as is" without any warranty of any kind, either
//	express, implied, or statutory, including, but not limited to, any
//	warranty that the software will conform to specification, any implied
//	warranties of merchantability, fitness for a particular purpose, and
//	freedom from infringement, and any warranty that the documentation will
//	conform to the program, or any warranty that the software will be error
//	free.
//
//	In no event shall Caltech be liable for any damages, including, but not
//	limited to direct, indirect, special or consequential damages, arising out
//	of, resulting from, or in any way connected with this software, whether or
//	not based upon warranty, contract, tort or otherwise, whether or not
//	injury was sustained by persons or property or otherwise, and whether or
//	not loss was sustained from or arose out of the results of, or use of,
//	their software or services provided hereunder.
//
//=== End File Prolog =========================================================
import java.awt.Color;
import java.awt.Component;
import javax.swing.JComponent;
import javax.swing.JLabel;
import javax.swing.JTable;
import javax.swing.table.*;
import javax.swing.ListSelectionModel;
import java.beans.*;
import edu.caltech.palomar.telescopes.guider.sdsstools.DownloadStatusTableModel;
import edu.caltech.palomar.telescopes.guider.sdsstools.SDSSAutoDownloadTool;
import java.awt.event.WindowListener;
import java.awt.event.WindowEvent;
import javax.swing.JOptionPane;
import javax.swing.JFrame;
import javax.swing.event.*;
import javax.swing.table.*;
import edu.caltech.palomar.telescopes.P200.gui.tables.AstroObject;
import com.sharkysoft.printf.Printf;
import com.sharkysoft.printf.PrintfData;
import com.sharkysoft.printf.PrintfTemplate;
import com.sharkysoft.printf.PrintfTemplateException;
//import edu.caltech.palomar.telescopes.guider.gui.GuideStarDisplayFrame;
import java.io.File;
import javax.swing.JFileChooser;
/*=============================================================================================
/      Constructor:DownloadStatusFrame
/=============================================================================================*/
public class DownloadStatusFrame extends javax.swing.JFrame implements WindowListener{
    public DownloadStatusTableModel  myDownloadStatusTableModel;
    public SDSSGuiderImageFinderTool mySDSSGuiderImageFinderTool;
    public SDSSAutoDownloadTool      mySDSSAutoDownloadTool;
    private int                      image_size;
//    public GuideStarDisplayFrame     myGuideStarDisplayFrame;
/*=============================================================================================
/      Constructor:DownloadStatusFrame
/=============================================================================================*/
    /** Creates new form DownloadStatusFrame */
    public DownloadStatusFrame() {
        this.setVisible(false);
        initComponents();
        initialize();
        this.setDefaultCloseOperation(JFrame.HIDE_ON_CLOSE);
        this.addWindowListener(this);
        this.setSize(575, 475);
    } 
/*=============================================================================================
/      getSDSSAutoDownloadTool()
/=============================================================================================*/
 public SDSSAutoDownloadTool getSDSSAutoDownloadTool(){
   return mySDSSAutoDownloadTool;
 }
/*=============================================================================================
/      setGuideStarDisplayFrame(GuideStarDisplayFrame newGuideStarDisplayFrame)
/=============================================================================================*/
 //public void setGuideStarDisplayFrame(GuideStarDisplayFrame newGuideStarDisplayFrame){
//     myGuideStarDisplayFrame = newGuideStarDisplayFrame;
 //}
/*=============================================================================================
/      Constructor: AstroObjectDisplayFrame
/=============================================================================================*/
   private void initialize(){
        mySDSSGuiderImageFinderTool = new SDSSGuiderImageFinderTool(this, false);
        mySDSSAutoDownloadTool      = new SDSSAutoDownloadTool(this);
        mySDSSAutoDownloadTool.setSDSSGuiderImageFinderTool(mySDSSGuiderImageFinderTool);
        mySDSSAutoDownloadTool.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
          public void propertyChange(java.beans.PropertyChangeEvent e) {
             SDSSAutoDownloadTool_propertyChange(e);
          }
        });

        myDownloadStatusTableModel = new DownloadStatusTableModel();
        mySDSSAutoDownloadTool.setTableModel(myDownloadStatusTableModel);
        myDownloadStatusTableModel.setJTable(DownloadTable);
        DownloadTable.setModel(myDownloadStatusTableModel);
        DownloadTable.setAutoResizeMode(DownloadTable.AUTO_RESIZE_OFF);
        DownloadTable.getColumnModel().getColumn(0).setMinWidth(60);
        DownloadTable.getColumnModel().getColumn(1).setMinWidth(120);
        DownloadTable.getColumnModel().getColumn(2).setMinWidth(155);
        DownloadTable.getColumnModel().getColumn(3).setMinWidth(155);
        DownloadTable.setSelectionMode(javax.swing.ListSelectionModel.SINGLE_SELECTION);
        DownloadTable.setRowSelectionAllowed(true);
        DownloadTable.getColumnModel().getColumn(0).setCellRenderer(new StateRenderer());
        DownloadTable.getSelectionModel().addListSelectionListener(new SharedListSelectionHandler());
   }
/*=============================================================================================
/     Property Change Listener for the _propertyChange
/=============================================================================================*/
  private void SDSSAutoDownloadTool_propertyChange(PropertyChangeEvent e)  {
     java.lang.String propertyName = e.getPropertyName();
        if(propertyName == "status_message"){
         java.lang.String statusMessage = (java.lang.String)e.getNewValue();
         StatusMessage.setText(statusMessage);
        }
        if(propertyName == "selected_ra"){
         java.lang.String myString = (java.lang.String)e.getNewValue();
         SelectedTargetRATextField.setText(myString);
        }
        if(propertyName == "selected_dec"){
         java.lang.String myString = (java.lang.String)e.getNewValue();
         SelectedTargetDECTextField.setText(myString);
        }
        if(propertyName == "selected_proper_motion_ra"){
         java.lang.String myString = (java.lang.String)e.getNewValue();
         SelectedProperMotionTargetRATextField.setText(myString);
        }
        if(propertyName == "selected_proper_motion_dec"){
         java.lang.String myString = (java.lang.String)e.getNewValue();
         SelectedProperMotionTargetDECTextField.setText(myString);
        }
        if(propertyName == "selected_target"){
         java.lang.String myString = (java.lang.String)e.getNewValue();
         SelectedTargetLabel.setText(myString);
        }
        if(propertyName == "selected_filename"){
         java.lang.String myString = (java.lang.String)e.getNewValue();
         SelectedFilenameLabel.setText(myString);
        }
  }
/*================================================================================================
/    setImageSize(double newBOX_SIZE)
/=================================================================================================*/
 public void setImageSize(int newIMAGE_SIZE){
     image_size = newIMAGE_SIZE;
 }
 public int getImageSize(){
     return image_size;
 }
/*=============================================================================================
/      StateRenderer extends JLabel implements TableCellRenderer
/=============================================================================================*/
  static class StateRenderer extends JLabel implements TableCellRenderer {
    public StateRenderer() {
        super();
        setOpaque(true); //MUST do this for background to show up.
    }
    public void setValue(Object value) {
        int integer_value = ((Integer)value).intValue();
        switch(integer_value){
            case 0: {setBackground(Color.GRAY); break;}
            case 1: {setBackground(Color.RED);  break;}
            case 2: {setBackground(Color.GREEN);break;}
        }
    }
    public Component getTableCellRendererComponent(JTable table, Object value,boolean isSelected, boolean hasFocus,int row, int column) {
        int integer_value = ((Integer)value).intValue();
        switch(integer_value){
            case 0: {setBackground(Color.GRAY);break;}
            case 1: {setBackground(Color.RED);break;}
            case 2: {setBackground(Color.GREEN);break;}
        }
        return this;
    }
 }

  /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        MainPanel = new javax.swing.JPanel();
        jScrollPane1 = new javax.swing.JScrollPane();
        DownloadTable = new javax.swing.JTable();
        SelectTargetFileButton = new javax.swing.JButton();
        jLabel3 = new javax.swing.JLabel();
        StatusMessage = new javax.swing.JLabel();
        ImageSizeComboBox = new javax.swing.JComboBox();
        DownloadToggleButton = new javax.swing.JToggleButton();
        jLabel2 = new javax.swing.JLabel();
        jLabel34 = new javax.swing.JLabel();
        jLabel33 = new javax.swing.JLabel();
        SelectedTargetRATextField = new javax.swing.JTextField();
        SelectedTargetDECTextField = new javax.swing.JTextField();
        jLabel35 = new javax.swing.JLabel();
        jLabel36 = new javax.swing.JLabel();
        SelectedProperMotionTargetRATextField = new javax.swing.JTextField();
        SelectedProperMotionTargetDECTextField = new javax.swing.JTextField();
        SelectedTargetLabel = new javax.swing.JLabel();
        jLabel5 = new javax.swing.JLabel();
        SelectedFilenameLabel = new javax.swing.JLabel();
        RetrieveImageButton = new javax.swing.JButton();

        setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);

        MainPanel.setBackground(new java.awt.Color(0, 0, 0));
        MainPanel.setBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 0, 0), 1, true));

        jScrollPane1.setHorizontalScrollBarPolicy(javax.swing.ScrollPaneConstants.HORIZONTAL_SCROLLBAR_ALWAYS);
        jScrollPane1.setVerticalScrollBarPolicy(javax.swing.ScrollPaneConstants.VERTICAL_SCROLLBAR_ALWAYS);

        DownloadTable.setModel(new javax.swing.table.DefaultTableModel(
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
        jScrollPane1.setViewportView(DownloadTable);

        SelectTargetFileButton.setFont(new java.awt.Font("DejaVu LGC Sans", 0, 12)); // NOI18N
        SelectTargetFileButton.setText("Select Target List File");
        SelectTargetFileButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                SelectTargetFileButtonActionPerformed(evt);
            }
        });

        jLabel3.setFont(new java.awt.Font("SansSerif", 3, 14)); // NOI18N
        jLabel3.setForeground(new java.awt.Color(102, 255, 0));
        jLabel3.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel3.setText("Automated Image Download Status Display");

        StatusMessage.setFont(new java.awt.Font("SansSerif", 1, 14)); // NOI18N
        StatusMessage.setForeground(new java.awt.Color(0, 255, 51));
        StatusMessage.setText("     ");

        ImageSizeComboBox.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "15x15 arcminutes", "30x30 arcminutes", "60x60 arcminutes" }));
        ImageSizeComboBox.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                ImageSizeComboBoxActionPerformed(evt);
            }
        });

        DownloadToggleButton.setFont(new java.awt.Font("DejaVu LGC Sans", 0, 12)); // NOI18N
        DownloadToggleButton.setText("Download Target Images");
        DownloadToggleButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                DownloadToggleButtonActionPerformed(evt);
            }
        });

        jLabel2.setFont(new java.awt.Font("SansSerif", 1, 14)); // NOI18N
        jLabel2.setForeground(new java.awt.Color(0, 255, 0));
        jLabel2.setText("Selected Target");

        jLabel34.setFont(new java.awt.Font("SansSerif", 1, 14)); // NOI18N
        jLabel34.setForeground(new java.awt.Color(0, 204, 204));
        jLabel34.setText("RA");

        jLabel33.setFont(new java.awt.Font("SansSerif", 1, 14)); // NOI18N
        jLabel33.setForeground(new java.awt.Color(0, 204, 204));
        jLabel33.setText("Dec");

        SelectedTargetRATextField.setEditable(false);
        SelectedTargetRATextField.setBackground(new java.awt.Color(0, 0, 0));
        SelectedTargetRATextField.setFont(new java.awt.Font("Copperplate Gothic Bold", 3, 24)); // NOI18N
        SelectedTargetRATextField.setForeground(new java.awt.Color(0, 204, 51));
        SelectedTargetRATextField.setHorizontalAlignment(javax.swing.JTextField.CENTER);
        SelectedTargetRATextField.setText("00:00:00.0");
        SelectedTargetRATextField.setBorder(null);
        SelectedTargetRATextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                SelectedTargetRATextFieldActionPerformed(evt);
            }
        });

        SelectedTargetDECTextField.setEditable(false);
        SelectedTargetDECTextField.setBackground(new java.awt.Color(0, 0, 0));
        SelectedTargetDECTextField.setFont(new java.awt.Font("Copperplate Gothic Bold", 3, 24)); // NOI18N
        SelectedTargetDECTextField.setForeground(new java.awt.Color(0, 204, 51));
        SelectedTargetDECTextField.setHorizontalAlignment(javax.swing.JTextField.CENTER);
        SelectedTargetDECTextField.setText("00:00:00.0");
        SelectedTargetDECTextField.setBorder(null);
        SelectedTargetDECTextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                SelectedTargetDECTextFieldActionPerformed(evt);
            }
        });

        jLabel35.setFont(new java.awt.Font("SansSerif", 1, 14)); // NOI18N
        jLabel35.setForeground(new java.awt.Color(0, 204, 204));
        jLabel35.setText("Proper Motion RA");

        jLabel36.setFont(new java.awt.Font("SansSerif", 1, 14)); // NOI18N
        jLabel36.setForeground(new java.awt.Color(0, 204, 204));
        jLabel36.setText("Proper Motion DEC");

        SelectedProperMotionTargetRATextField.setEditable(false);
        SelectedProperMotionTargetRATextField.setBackground(new java.awt.Color(0, 0, 0));
        SelectedProperMotionTargetRATextField.setFont(new java.awt.Font("Copperplate Gothic Bold", 3, 18)); // NOI18N
        SelectedProperMotionTargetRATextField.setForeground(new java.awt.Color(0, 204, 51));
        SelectedProperMotionTargetRATextField.setHorizontalAlignment(javax.swing.JTextField.CENTER);
        SelectedProperMotionTargetRATextField.setText("0");
        SelectedProperMotionTargetRATextField.setBorder(null);
        SelectedProperMotionTargetRATextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                SelectedProperMotionTargetRATextFieldActionPerformed(evt);
            }
        });

        SelectedProperMotionTargetDECTextField.setEditable(false);
        SelectedProperMotionTargetDECTextField.setBackground(new java.awt.Color(0, 0, 0));
        SelectedProperMotionTargetDECTextField.setFont(new java.awt.Font("Copperplate Gothic Bold", 3, 18)); // NOI18N
        SelectedProperMotionTargetDECTextField.setForeground(new java.awt.Color(0, 204, 51));
        SelectedProperMotionTargetDECTextField.setHorizontalAlignment(javax.swing.JTextField.CENTER);
        SelectedProperMotionTargetDECTextField.setText("0");
        SelectedProperMotionTargetDECTextField.setBorder(null);
        SelectedProperMotionTargetDECTextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                SelectedProperMotionTargetDECTextFieldActionPerformed(evt);
            }
        });

        SelectedTargetLabel.setFont(new java.awt.Font("SansSerif", 1, 18)); // NOI18N
        SelectedTargetLabel.setForeground(new java.awt.Color(0, 204, 204));
        SelectedTargetLabel.setText(" ");

        jLabel5.setFont(new java.awt.Font("SansSerif", 1, 14)); // NOI18N
        jLabel5.setForeground(new java.awt.Color(0, 255, 0));
        jLabel5.setText("File Name");

        SelectedFilenameLabel.setFont(new java.awt.Font("SansSerif", 1, 14)); // NOI18N
        SelectedFilenameLabel.setForeground(new java.awt.Color(0, 204, 204));
        SelectedFilenameLabel.setText(" ");

        RetrieveImageButton.setFont(new java.awt.Font("DejaVu LGC Sans", 0, 12)); // NOI18N
        RetrieveImageButton.setText("Retrieve Image");
        RetrieveImageButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                RetrieveImageButtonActionPerformed(evt);
            }
        });

        org.jdesktop.layout.GroupLayout MainPanelLayout = new org.jdesktop.layout.GroupLayout(MainPanel);
        MainPanel.setLayout(MainPanelLayout);
        MainPanelLayout.setHorizontalGroup(
            MainPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(MainPanelLayout.createSequentialGroup()
                .add(MainPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(org.jdesktop.layout.GroupLayout.TRAILING, MainPanelLayout.createSequentialGroup()
                        .add(8, 8, 8)
                        .add(MainPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                            .add(MainPanelLayout.createSequentialGroup()
                                .add(jLabel2)
                                .add(2, 2, 2)
                                .add(SelectedTargetLabel, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                            .add(MainPanelLayout.createSequentialGroup()
                                .add(SelectTargetFileButton)
                                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                                .add(DownloadToggleButton, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)))
                        .add(18, 18, 18)
                        .add(MainPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                            .add(ImageSizeComboBox, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 154, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                            .add(RetrieveImageButton, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 129, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                        .add(141, 141, 141))
                    .add(MainPanelLayout.createSequentialGroup()
                        .addContainerGap()
                        .add(MainPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                            .add(StatusMessage, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 472, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                            .add(MainPanelLayout.createSequentialGroup()
                                .add(12, 12, 12)
                                .add(MainPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING, false)
                                    .add(MainPanelLayout.createSequentialGroup()
                                        .add(jLabel35, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 139, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                                        .add(SelectedProperMotionTargetRATextField))
                                    .add(MainPanelLayout.createSequentialGroup()
                                        .add(jLabel34, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 32, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                                        .add(SelectedTargetRATextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 178, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)))
                                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                                .add(MainPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING, false)
                                    .add(MainPanelLayout.createSequentialGroup()
                                        .add(jLabel33, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 32, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                                        .add(SelectedTargetDECTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 180, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                                    .add(MainPanelLayout.createSequentialGroup()
                                        .add(jLabel36, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 139, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                                        .add(SelectedProperMotionTargetDECTextField)))))
                        .add(33, 33, 33))
                    .add(MainPanelLayout.createSequentialGroup()
                        .addContainerGap()
                        .add(MainPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                            .add(jScrollPane1, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 505, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                            .add(MainPanelLayout.createSequentialGroup()
                                .add(jLabel5)
                                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                                .add(SelectedFilenameLabel, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))))
                    .add(jLabel3, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 533, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addContainerGap())
        );
        MainPanelLayout.setVerticalGroup(
            MainPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(org.jdesktop.layout.GroupLayout.TRAILING, MainPanelLayout.createSequentialGroup()
                .add(jLabel3)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .add(MainPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(SelectTargetFileButton)
                    .add(DownloadToggleButton)
                    .add(ImageSizeComboBox, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(7, 7, 7)
                .add(MainPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(SelectedTargetLabel)
                    .add(RetrieveImageButton)
                    .add(jLabel2))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(MainPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                    .add(jLabel5)
                    .add(SelectedFilenameLabel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 17, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(18, 18, 18)
                .add(jScrollPane1, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 206, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(StatusMessage)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(MainPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(SelectedTargetDECTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(jLabel33)
                    .add(SelectedTargetRATextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(jLabel34))
                .add(6, 6, 6)
                .add(MainPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(SelectedProperMotionTargetRATextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(jLabel35)
                    .add(jLabel36)
                    .add(SelectedProperMotionTargetDECTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 27, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(225, 225, 225))
        );

        org.jdesktop.layout.GroupLayout layout = new org.jdesktop.layout.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(MainPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 536, Short.MAX_VALUE)
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .add(MainPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 490, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .add(0, 0, Short.MAX_VALUE))
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents

    private void SelectTargetFileButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_SelectTargetFileButtonActionPerformed
        mySDSSAutoDownloadTool.selectTargetDefinitionFile(false);
        mySDSSAutoDownloadTool.openTargetDefinitionFile();
    }//GEN-LAST:event_SelectTargetFileButtonActionPerformed

    private void DownloadToggleButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_DownloadToggleButtonActionPerformed
        // TODO add your handling code here:
        boolean selected = DownloadToggleButton.isSelected();
        if(selected){
           DownloadToggleButton.setText("Downloading Images");
           mySDSSAutoDownloadTool.execute();
        }
        if(!selected){
           DownloadToggleButton.setText("Download Target Images");
           mySDSSAutoDownloadTool.stop();
        }
    }//GEN-LAST:event_DownloadToggleButtonActionPerformed

    private void ImageSizeComboBoxActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_ImageSizeComboBoxActionPerformed
         int selectedIndex = ImageSizeComboBox.getSelectedIndex();
        if(selectedIndex == 0){
            setImageSize(15);
            mySDSSAutoDownloadTool.setSearchBoxWidth(15.0);
            mySDSSAutoDownloadTool.setSearchBoxHeight(15.0);
        }
        if(selectedIndex == 1){
            setImageSize(30);
            mySDSSAutoDownloadTool.setSearchBoxWidth(30.0);
            mySDSSAutoDownloadTool.setSearchBoxHeight(30.0);
        }
        if(selectedIndex == 2){
            setImageSize(60);
            mySDSSAutoDownloadTool.setSearchBoxWidth(60.0);
            mySDSSAutoDownloadTool.setSearchBoxHeight(60.0);
        }
       myDownloadStatusTableModel.clearDownloadStatus();
    }//GEN-LAST:event_ImageSizeComboBoxActionPerformed

    private void SelectedTargetRATextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_SelectedTargetRATextFieldActionPerformed
        // TODO add your handling code here:
}//GEN-LAST:event_SelectedTargetRATextFieldActionPerformed

    private void SelectedTargetDECTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_SelectedTargetDECTextFieldActionPerformed
        // TODO add your handling code here:
}//GEN-LAST:event_SelectedTargetDECTextFieldActionPerformed

    private void SelectedProperMotionTargetRATextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_SelectedProperMotionTargetRATextFieldActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_SelectedProperMotionTargetRATextFieldActionPerformed

    private void SelectedProperMotionTargetDECTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_SelectedProperMotionTargetDECTextFieldActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_SelectedProperMotionTargetDECTextFieldActionPerformed

    private void RetrieveImageButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_RetrieveImageButtonActionPerformed
        // TODO add your handling code here:
        java.lang.String selectedFile = mySDSSAutoDownloadTool.getSelectedFilename();
//        myGuideStarDisplayFrame.updateFileName(selectedFile);
    }//GEN-LAST:event_RetrieveImageButtonActionPerformed
/*=============================================================================================
/     WindowListener Interface - changing the behavior of the close events
/=============================================================================================*/
    public void windowClosing(WindowEvent e) {
        displayMessage("WindowListener method called: windowClosing.");
//        int selection = JOptionPane.showConfirmDialog(this,"Do you REALLY want to exit?","A plain message",JOptionPane.YES_NO_OPTION);
//        if(selection == 0){
//           System.exit(0);
//        }
    }
    public void windowClosed(WindowEvent e) {
        displayMessage("WindowListener method called: windowClosed.");
    }
    public void windowOpened(WindowEvent e) {
        displayMessage("WindowListener method called: windowOpened.");
    }

    public void windowIconified(WindowEvent e) {
        displayMessage("WindowListener method called: windowIconified.");
    }

    public void windowDeiconified(WindowEvent e) {
        displayMessage("WindowListener method called: windowDeiconified.");
    }

    public void windowActivated(WindowEvent e) {
        displayMessage("WindowListener method called: windowActivated.");
    }

    public void windowDeactivated(WindowEvent e) {
        displayMessage("WindowListener method called: windowDeactivated.");
    }
    void displayMessage(String msg) {
        System.out.println(msg);
    }

/*=============================================================================================
/      Constructor: SharedListSelectionHandler
/=============================================================================================*/
    class SharedListSelectionHandler implements ListSelectionListener {
        public void valueChanged(ListSelectionEvent e) {
            ListSelectionModel lsm = (ListSelectionModel)e.getSource();
           lsm.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
            int     selection   = lsm.getLeadSelectionIndex();
//            int     firstIndex  = e.getFirstIndex();
//            int     lastIndex   = e.getLastIndex();
             boolean isAdjusting = lsm.getValueIsAdjusting();
            if(!isAdjusting){
              if(selection != -1){
                  AstroObject currentObject = myDownloadStatusTableModel.getRecord(selection);
                  mySDSSAutoDownloadTool.setSelectedAstroObject(currentObject);
                  System.out.println(currentObject.name);
                  mySDSSAutoDownloadTool.setSelectedRa(currentObject.Alpha.RoundedRAString(2, ":"));
                  mySDSSAutoDownloadTool.setSelectedDec(currentObject.Delta.RoundedDecString(2, ":"));
                  mySDSSAutoDownloadTool.setSelectedProperMotionRa(Printf.format("%05.2f", new PrintfData().add(currentObject.r_m)));
                  mySDSSAutoDownloadTool.setSelectedProperMotionDec(Printf.format("%05.2f", new PrintfData().add(currentObject.d_m)));
                  mySDSSAutoDownloadTool.setSelectedFilename(currentObject.fileName);
                  mySDSSAutoDownloadTool.setSelectedTarget(currentObject.name);
                }
              } // end of if !isAdjusting
         }
    }    /**
    * @param args the command line arguments
    */
    public static void main(String args[]) {
        java.awt.EventQueue.invokeLater(new Runnable() {
            public void run() {
                new DownloadStatusFrame().setVisible(true);
            }
        });
    }

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JTable DownloadTable;
    private javax.swing.JToggleButton DownloadToggleButton;
    private javax.swing.JComboBox ImageSizeComboBox;
    private javax.swing.JPanel MainPanel;
    private javax.swing.JButton RetrieveImageButton;
    private javax.swing.JButton SelectTargetFileButton;
    private javax.swing.JLabel SelectedFilenameLabel;
    private javax.swing.JTextField SelectedProperMotionTargetDECTextField;
    private javax.swing.JTextField SelectedProperMotionTargetRATextField;
    private javax.swing.JTextField SelectedTargetDECTextField;
    private javax.swing.JLabel SelectedTargetLabel;
    private javax.swing.JTextField SelectedTargetRATextField;
    private javax.swing.JLabel StatusMessage;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JLabel jLabel3;
    private javax.swing.JLabel jLabel33;
    private javax.swing.JLabel jLabel34;
    private javax.swing.JLabel jLabel35;
    private javax.swing.JLabel jLabel36;
    private javax.swing.JLabel jLabel5;
    private javax.swing.JScrollPane jScrollPane1;
    // End of variables declaration//GEN-END:variables

}
