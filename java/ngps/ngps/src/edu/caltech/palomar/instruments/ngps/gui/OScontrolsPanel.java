/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/GUIForms/JPanel.java to edit this template
 */
package edu.caltech.palomar.instruments.ngps.gui;
import javax.swing.JPanel;
import edu.caltech.palomar.instruments.ngps.gui.CustomProgressBar;
import javax.swing.JRadioButton;
import javax.swing.JButton;
import edu.caltech.palomar.instruments.ngps.os.ObservationSequencerController;
import java.awt.Color;
import java.beans.PropertyChangeEvent;
import javax.swing.ButtonGroup;
import javax.swing.JTextPane;
import edu.caltech.palomar.instruments.ngps.dbms.NGPSdatabase;
import javax.swing.ImageIcon;
/**
 *
 * @author developer
 */
public class OScontrolsPanel extends javax.swing.JPanel {
 public ObservationSequencerController myObservationSequencerController;
 public  JButton                  STARTUP;
 public  JButton                  STOP;
 public  JButton                  GO;
 public  JButton                  ABORT;
 public  JButton                  PAUSE;
 public  JButton                  RESUME;
 private ButtonGroup              do_one_do_all_ButtonGroup;
 public NGPSdatabase              myNGPSdatabase;
 private ImageIcon                ON;
 private ImageIcon                OFF;
 private ImageIcon                UNKNOWN;
 public java.lang.String          USERDIR         = System.getProperty("user.dir");
 public java.lang.String          SEP             = System.getProperty("file.separator");
 public java.lang.String          CONFIG          = new java.lang.String("config");
 public java.lang.String          IMAGE_CACHE     = new java.lang.String("images");
/*=============================================================================================
/     OScontrolsPanel(ObservationSequencerController newObservationSequencerController)
/=============================================================================================*/ 
   public OScontrolsPanel(){       
        initComponents();
        initializeActionButtons();
        initializeIcons();
        customexposureProgressBar.setColorstring(Color.black);
        customoverheadProgressBar.setColorstring(Color.black);
        do_one_do_all_ButtonGroup = new ButtonGroup();
        do_one_do_all_ButtonGroup.add(do_oneRadioButton);
        do_one_do_all_ButtonGroup.add(do_allRadioButton);  
        do_oneRadioButton.setSelected(true);
    }
/*=============================================================================================
/   setDBMS(NGPSdatabase current)
/=============================================================================================*/ 
   public void setDBMS(NGPSdatabase current){
     myNGPSdatabase = current;
   }
/*=============================================================================================
/     initialize()
/=============================================================================================*/ 
    public void initialize(ObservationSequencerController newObservationSequencerController) {
    myObservationSequencerController = newObservationSequencerController;
    myObservationSequencerController.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
    public void propertyChange(java.beans.PropertyChangeEvent e) {
             state_propertyChange(e);
          }
     });  
    myObservationSequencerController.setSTATE("OFFLINE");
    messagesTextPane.setDocument(myObservationSequencerController.myCommandLogModel.getDocument());
}
/*================================================================================================
/        initializeIcons()
/=================================================================================================*/
  public void initializeIcons(){
    ON                   = new ImageIcon(USERDIR + SEP + IMAGE_CACHE + SEP + "ON.gif");
    OFF                  = new ImageIcon(USERDIR + SEP + IMAGE_CACHE + SEP + "OFF.gif");
    UNKNOWN              = new ImageIcon(USERDIR + SEP + IMAGE_CACHE + SEP + "UNKNOWN.gif");
  } 
public JPanel get_actionButtonsPanel(){
    return actionButtonsPanel;
}
public CustomProgressBar get_customexposureProgressBar(){
    return  customexposureProgressBar;
}
public CustomProgressBar get_customoverheadProgressBar(){
    return  customoverheadProgressBar;
}
public JRadioButton get_do_allRadioButton(){
   return do_allRadioButton; 
}
public JRadioButton get_do_oneRadioButton(){
   return do_oneRadioButton; 
}
public JButton get_system_stateButton(){
   return system_stateButton; 
}
public JTextPane get_messagesTextPane(){
    return messagesTextPane;
}
/*================================================================================================
/      initializeActionButtons()
/=================================================================================================*/
public void initializeActionButtons(){
  STARTUP = new JButton();
  STARTUP.setText("STARTUP");
  STARTUP.setBackground(new Color(30,90,15));
  STARTUP.setForeground(Color.white);
  STARTUP.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                STARTUPActionPerformed(evt);
            }
        }); 
  STOP    = new JButton();
  STOP.setText("STOP");
  STOP.setBackground(Color.black);
  STOP.setForeground(Color.white);
  STOP.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                STOPActionPerformed(evt);
            }
        });  
  GO      = new JButton();
  GO.setText("GO");
  GO.setBackground(Color.black);
  GO.setForeground(Color.white);
  GO.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                GOActionPerformed(evt);
            }
        });   
  ABORT   = new JButton();
  ABORT.setText("ABORT");
  ABORT.setBackground(Color.black);
  ABORT.setForeground(Color.white);
  ABORT.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                ABORTActionPerformed(evt);
            }
        });   
  PAUSE   = new JButton(); 
  PAUSE.setText("PAUSE");
  PAUSE.setBackground(Color.black);
  PAUSE.setForeground(Color.white);
  PAUSE.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                PAUSEActionPerformed(evt);
            }
        });  
  RESUME   = new JButton(); 
  RESUME.setText("RESUME");
  RESUME.setBackground(Color.GREEN);
  RESUME.setForeground(Color.white);
  RESUME.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                RESUMEActionPerformed(evt);
            }
        });  
}  
 private void STARTUPActionPerformed(java.awt.event.ActionEvent evt){
//    if(myObservationSequencerController.isBlockingConnected()){
        myObservationSequencerController.tcs_isOpen();
        myObservationSequencerController.startup();
//    }    
 } 
 private void STOPActionPerformed(java.awt.event.ActionEvent evt){
 //    if(myObservationSequencerController.isBlockingConnected()){
        myObservationSequencerController.stop();
 //   }       
 } 
 private void GOActionPerformed(java.awt.event.ActionEvent evt){
//     if(myObservationSequencerController.isBlockingConnected()){
         if(do_oneRadioButton.isSelected()){
             myObservationSequencerController.do_one_all(ObservationSequencerController.DO_ONE);
         }
         if(do_allRadioButton.isSelected()){
             myObservationSequencerController.do_one_all(ObservationSequencerController.DO_ALL);
         } 
         java.lang.String set_name = myObservationSequencerController.dbms.getSelectedSetName();
         if(set_name != null){
             myObservationSequencerController.targetset(set_name);
         }         
         myObservationSequencerController.start();
         myObservationSequencerController.state();
//    }       
 } 
 private void ABORTActionPerformed(java.awt.event.ActionEvent evt){
 //     if(myObservationSequencerController.isBlockingConnected()){
        myObservationSequencerController.abort();
//    }       
 } 
 private void PAUSEActionPerformed(java.awt.event.ActionEvent evt){
//      if(myObservationSequencerController.isBlockingConnected()){
        myObservationSequencerController.pause();
 //   }       
 } 
 private void RESUMEActionPerformed(java.awt.event.ActionEvent evt){
//      if(myObservationSequencerController.isBlockingConnected()){
        myObservationSequencerController.resume();
//    }      
 } 
/*=============================================================================================
/     Property Change Listener for the Ephemeris_propertyChange
/=============================================================================================*/
  private void state_propertyChange(PropertyChangeEvent e)  {
     java.lang.String propertyName = e.getPropertyName();
     System.out.println(propertyName);
    if(propertyName.matches("progress")){
       java.lang.Integer current_value = (java.lang.Integer)e.getNewValue();  
       customexposureProgressBar.setValue(current_value);
    }
    if(propertyName.matches("progress_string")){
        java.lang.String current_value = (java.lang.String)e.getNewValue();
        customexposureProgressBar.setString(current_value);
    }
    if(propertyName.matches("overhead_progress")){
        java.lang.Integer current_value = (java.lang.Integer)e.getNewValue();  
        customoverheadProgressBar.setValue(current_value);       
    }
    if(propertyName.matches("overhead_progress_string")){
        java.lang.String current_value = (java.lang.String)e.getNewValue();
        customoverheadProgressBar.setString(current_value);
    }
    if(propertyName.matches("total_time")){
        java.lang.Integer current_value = (java.lang.Integer)e.getNewValue();  
        double current_seconds = current_value/1000;
        totalTimeLabel.setText(""+String.format("%.2f", current_seconds));
    }
    if(propertyName.matches("tcs_connected")){
         boolean state = (java.lang.Boolean)e.getNewValue();
         if(state){
           tcs_Label.setIcon(ON);
         }else{
           tcs_Label.setIcon(OFF);            
         }
     }
     if(propertyName.matches("active_tcs_address")){
         java.lang.String value = (java.lang.String)e.getNewValue();
         this.active_tcs_ip_Label.setText(value);
      }
     if(propertyName.matches("active_tcs_name")){
         java.lang.String value = (java.lang.String)e.getNewValue();
        this.active_tcs_Label1.setText(value);   
     }    
/*=============================================================================================
/    PARAMETERS FROM THE JSKYCALCMODEL
/=============================================================================================*/
    if(propertyName.matches("STATE")){
        java.lang.String current_value = (java.lang.String)e.getNewValue(); 
        System.out.println(current_value);
        if(current_value.matches("OFFLINE")){
          try{
            this.system_stateButton.setText("STOPPED");
            system_stateButton.setBackground(Color.black);
            system_stateButton.setForeground(Color.white);
            int component_count = actionButtonsPanel.getComponentCount();
            int before = actionButtonsPanel.getComponentCount();
            actionButtonsPanel.removeAll();
            actionButtonsPanel.validate();
            actionButtonsPanel.repaint();
            int after = actionButtonsPanel.getComponentCount();   
            System.out.println("Before = "+before+" After = "+after);
            actionButtonsPanel.add(STARTUP);
          }catch(Exception e2){
             System.out.println("STATE = OFFLINE"+e2.toString());
          }            
        }
        if(current_value.matches("READY_NO_TARGETS")){
          try{
            system_stateButton.setBackground(new Color(255,204,0));
            system_stateButton.setForeground(Color.black);
            system_stateButton.setText("No Targets");
            int before = actionButtonsPanel.getComponentCount();
            actionButtonsPanel.removeAll();
            actionButtonsPanel.validate();
            actionButtonsPanel.repaint();
            int after = actionButtonsPanel.getComponentCount();   
            System.out.println("Before = "+before+" After = "+after);
            actionButtonsPanel.add(GO);
            GO.setEnabled(false);
            GO.setBackground(new Color(0,125,0));
            GO.setForeground(new Color(255,255,255));
          }catch(Exception e2){
             System.out.println("STATE = READY_NO_TARGETS"+e2.toString());
          }        }
        if(current_value.matches("READY")){
          try{
            system_stateButton.setBackground(new Color(255,204,0));
            system_stateButton.setForeground(Color.black);
            system_stateButton.setText("IDLE");
            int before = actionButtonsPanel.getComponentCount();
            actionButtonsPanel.removeAll();
            actionButtonsPanel.validate();
            actionButtonsPanel.repaint();
            int after = actionButtonsPanel.getComponentCount();   
            System.out.println("Before = "+before+" After = "+after);  
            if(myNGPSdatabase.myTargetDBMSTableModel.getRowCount() == 0){
                myObservationSequencerController.setSTATE("READY_NO_TARGETS");
            }else{
              actionButtonsPanel.add(GO);
              GO.setEnabled(true);
              GO.setBackground(new Color(0,125,0));
              GO.setForeground(new Color(255,255,255));  
            }
          }catch(Exception e2){
             System.out.println("STATE = READY"+e2.toString());
          }        }
        if(current_value.matches("TCSOP")){
          try{
            system_stateButton.setBackground(new Color(255,204,0));
            system_stateButton.setForeground(Color.black);
            system_stateButton.setText("TCS OPERATOR?");
            int before = actionButtonsPanel.getComponentCount();
            actionButtonsPanel.removeAll();
            actionButtonsPanel.validate();
            actionButtonsPanel.repaint();
            int after = actionButtonsPanel.getComponentCount();   
            System.out.println("Before = "+before+" After = "+after);
            ABORT.setEnabled(true);
            ABORT.setBackground(Color.red);
            ABORT.setForeground(new Color(255,255,255));
            actionButtonsPanel.add(ABORT);
          }catch(Exception e2){
             System.out.println("STATE = TCS_OPERATOR"+e2.toString());
          }        } 
        if(current_value.matches("SLEW")){
         try{
            system_stateButton.setBackground(Color.BLACK);
            system_stateButton.setForeground(Color.WHITE);
            system_stateButton.setText("SLEWING");
            int before = actionButtonsPanel.getComponentCount();
            actionButtonsPanel.removeAll();
            actionButtonsPanel.validate();
            actionButtonsPanel.repaint();
            int after = actionButtonsPanel.getComponentCount();   
            System.out.println("Before = "+before+" After = "+after);
            ABORT.setEnabled(true);
            ABORT.setBackground(Color.red);
            ABORT.setForeground(new Color(255,255,255));
            actionButtonsPanel.add(ABORT);            
          }catch(Exception e2){
             System.out.println("STATE = SLEW"+e2.toString());
          }        }     
        if(current_value.matches("ACQUIRE")){
          try{
            system_stateButton.setBackground(Color.BLACK);
            system_stateButton.setForeground(Color.WHITE);
            system_stateButton.setText("ACQUIRING");
            int before = actionButtonsPanel.getComponentCount();
            actionButtonsPanel.removeAll();
            actionButtonsPanel.validate();
            actionButtonsPanel.repaint();
            int after = actionButtonsPanel.getComponentCount();   
            System.out.println("Before = "+before+" After = "+after);
            ABORT.setEnabled(true);
            ABORT.setBackground(Color.red);
            ABORT.setForeground(new Color(255,255,255));
            actionButtonsPanel.add(ABORT);                        
          }catch(Exception e2){
             System.out.println("STATE = ACQUIRE"+e2.toString());
          }        }     
        if(current_value.matches("AIRMASS")){
          try{
            system_stateButton.setBackground(new Color(255,204,0));
            system_stateButton.setForeground(Color.BLACK);
            system_stateButton.setText("AIRMASS?");
            int before = actionButtonsPanel.getComponentCount();
            actionButtonsPanel.removeAll();
            actionButtonsPanel.validate();
            actionButtonsPanel.repaint();
            int after = actionButtonsPanel.getComponentCount();   
            System.out.println("Before = "+before+" After = "+after);
            ABORT.setEnabled(true);
            ABORT.setBackground(Color.red);
            ABORT.setForeground(Color.BLACK);
            actionButtonsPanel.add(ABORT);                       
          }catch(Exception e2){
             System.out.println("STATE = AIRMASS"+e2.toString());
          }       
        } 
        if(current_value.matches("RUNNING")){
          try{
            system_stateButton.setBackground(new Color(0,128,0));
            system_stateButton.setForeground(Color.white);
            system_stateButton.setText("RUNNING");
            int before = actionButtonsPanel.getComponentCount();
            actionButtonsPanel.removeAll();
            actionButtonsPanel.validate();
            actionButtonsPanel.repaint();
            int after = actionButtonsPanel.getComponentCount();   
            System.out.println("Before = "+before+" After = "+after);
            
            PAUSE.setEnabled(true);
            PAUSE.setBackground(new Color(0,125,0));
            PAUSE.setForeground(new Color(255,255,255));
            
            STOP.setEnabled(true);
            STOP.setBackground(Color.black);
            STOP.setForeground(new Color(255,255,255));
            
            ABORT.setEnabled(true);
            ABORT.setBackground(Color.red);
            ABORT.setForeground(new Color(255,255,255));
            actionButtonsPanel.removeAll();
            actionButtonsPanel.add(PAUSE);
            actionButtonsPanel.add(STOP);
            actionButtonsPanel.add(ABORT);
            actionButtonsPanel.validate();
            actionButtonsPanel.repaint();
         }catch(Exception e2){
             System.out.println("STATE = RUNNING"+e2.toString());
          }
        } 
        if(current_value.matches("EXPOSING")){
         try{
            system_stateButton.setBackground(new Color(0,128,0));
            system_stateButton.setForeground(Color.white);
            system_stateButton.setText("EXPOSING");
            int before = actionButtonsPanel.getComponentCount();
            actionButtonsPanel.removeAll();
            actionButtonsPanel.validate();
            actionButtonsPanel.repaint();
            int after = actionButtonsPanel.getComponentCount();   
            System.out.println("Before = "+before+" After = "+after);
            actionButtonsPanel.removeAll();
            
            PAUSE.setEnabled(true);
            PAUSE.setBackground(new Color(0,125,0));
            PAUSE.setForeground(new Color(255,255,255));
            
            STOP.setEnabled(true);
            STOP.setBackground(Color.black);
            STOP.setForeground(new Color(255,255,255));
            
            ABORT.setEnabled(true);
            ABORT.setBackground(Color.red);
            ABORT.setForeground(new Color(255,255,255));
            actionButtonsPanel.add(PAUSE);
            actionButtonsPanel.add(STOP);
            actionButtonsPanel.add(ABORT);
          }catch(Exception e2){
             System.out.println("STATE = EXPOSING"+e2.toString());
          }        } 
        if(current_value.matches("READOUT")){
          try{
            system_stateButton.setBackground(Color.BLACK);
            system_stateButton.setForeground(Color.WHITE);
            system_stateButton.setText("READOUT");
            int before = actionButtonsPanel.getComponentCount();
            actionButtonsPanel.removeAll();
            actionButtonsPanel.validate();
            actionButtonsPanel.repaint();
            int after = actionButtonsPanel.getComponentCount();   
            System.out.println("Before = "+before+" After = "+after);
            ABORT.setEnabled(true);
            ABORT.setBackground(Color.red);
            ABORT.setForeground(new Color(255,255,255));
            actionButtonsPanel.add(ABORT);                        
          }catch(Exception e2){
             System.out.println("STATE = READOUT"+e2.toString());
          }        }  
        if(current_value.matches("PAUSED")){
          try{
            system_stateButton.setBackground(new Color(255,204,0));
            system_stateButton.setForeground(Color.black);
            system_stateButton.setText("PAUSED");
            int before = actionButtonsPanel.getComponentCount();
            actionButtonsPanel.removeAll();
            actionButtonsPanel.validate();
            actionButtonsPanel.repaint();
            int after = actionButtonsPanel.getComponentCount();   
            System.out.println("Before = "+before+" After = "+after);
            
            RESUME.setEnabled(true);
            RESUME.setText("RESUME");
            RESUME.setBackground(new Color(0,125,0));
            RESUME.setForeground(new Color(255,255,255));
            
            STOP.setEnabled(true);
            STOP.setBackground(Color.black);
            STOP.setForeground(new Color(255,255,255));
            
            ABORT.setEnabled(true);
            ABORT.setBackground(Color.red);
            ABORT.setForeground(new Color(255,255,255));
            actionButtonsPanel.add(RESUME);
            actionButtonsPanel.add(STOP);
            actionButtonsPanel.add(ABORT); 
          }catch(Exception e2){
             System.out.println("STATE = PAUSED"+e2.toString());
          }
        }       
        if(current_value.matches("STOPREQ")){
          try{
            system_stateButton.setBackground(new Color(130,130,130));
            system_stateButton.setForeground(Color.black);
            system_stateButton.setText("STOPPING...");
            int before = actionButtonsPanel.getComponentCount();
            actionButtonsPanel.removeAll();
            actionButtonsPanel.validate();
            actionButtonsPanel.repaint();
            int after = actionButtonsPanel.getComponentCount();   
            System.out.println("Before = "+before+" After = "+after);
          }catch(Exception e2){
             System.out.println("STATE = STOPREQ"+e2.toString());
          }
        }
        if(current_value.matches("ABORTREQ")){
          try{
            system_stateButton.setBackground(new Color(130,130,130));
            system_stateButton.setForeground(Color.black);
            system_stateButton.setText("ABORTING...");
            int before = actionButtonsPanel.getComponentCount();
            actionButtonsPanel.removeAll();
            actionButtonsPanel.validate();
            actionButtonsPanel.repaint();
            int after = actionButtonsPanel.getComponentCount();   
            System.out.println("Before = "+before+" After = "+after);
          }catch(Exception e2){
             System.out.println("STATE = ABORTREQ"+e2.toString());
          }   
        }    
        if(current_value.matches("STARTING")){
          try{
            system_stateButton.setBackground(new Color(130,130,130));
            system_stateButton.setForeground(Color.black);
            system_stateButton.setText("STARTING...");
            int before = actionButtonsPanel.getComponentCount();
            actionButtonsPanel.removeAll();
            actionButtonsPanel.validate();
            actionButtonsPanel.repaint();
            int after = actionButtonsPanel.getComponentCount();   
            System.out.println("Before = "+before+" After = "+after);
          }catch(Exception e2){
             System.out.println("STATE = STARTING"+e2.toString());
          }
        }            
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

        sequencerPanel = new javax.swing.JPanel();
        system_stateButton = new javax.swing.JButton();
        do_oneRadioButton = new javax.swing.JRadioButton();
        do_allRadioButton = new javax.swing.JRadioButton();
        jLabel5 = new javax.swing.JLabel();
        jScrollPane2 = new javax.swing.JScrollPane();
        messagesTextPane = new javax.swing.JTextPane();
        actionButtonsPanel = new javax.swing.JPanel();
        mod_exptimeTextField = new javax.swing.JTextField();
        customexposureProgressBar = new edu.caltech.palomar.instruments.ngps.gui.CustomProgressBar();
        customoverheadProgressBar = new edu.caltech.palomar.instruments.ngps.gui.CustomProgressBar();
        totalTimeLabel = new javax.swing.JLabel();
        tcs_Label = new javax.swing.JLabel();
        active_tcs_ip_Label = new javax.swing.JLabel();
        active_tcs_Label1 = new javax.swing.JLabel();
        jLabel1 = new javax.swing.JLabel();
        jLabel2 = new javax.swing.JLabel();

        sequencerPanel.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));

        system_stateButton.setBackground(new java.awt.Color(255, 204, 0));
        system_stateButton.setText("STOPPED");
        system_stateButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                system_stateButtonActionPerformed(evt);
            }
        });

        do_oneRadioButton.setFont(new java.awt.Font("Arial", 1, 13)); // NOI18N
        do_oneRadioButton.setText("Single Step");
        do_oneRadioButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                do_oneRadioButtonActionPerformed(evt);
            }
        });

        do_allRadioButton.setFont(new java.awt.Font("Arial", 1, 13)); // NOI18N
        do_allRadioButton.setText("Do All");
        do_allRadioButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                do_allRadioButtonActionPerformed(evt);
            }
        });

        jLabel5.setFont(new java.awt.Font("Arial", 1, 12)); // NOI18N
        jLabel5.setText("Messages");

        jScrollPane2.setHorizontalScrollBarPolicy(javax.swing.ScrollPaneConstants.HORIZONTAL_SCROLLBAR_NEVER);
        jScrollPane2.setVerticalScrollBarPolicy(javax.swing.ScrollPaneConstants.VERTICAL_SCROLLBAR_ALWAYS);
        jScrollPane2.setViewportView(messagesTextPane);

        actionButtonsPanel.setBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 0, 0), 1, true));

        mod_exptimeTextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                mod_exptimeTextFieldActionPerformed(evt);
            }
        });

        customexposureProgressBar.setBackground(java.awt.Color.white);
        customexposureProgressBar.setForeground(java.awt.Color.green);

        customoverheadProgressBar.setBackground(java.awt.Color.white);
        customoverheadProgressBar.setForeground(java.awt.Color.green);

        totalTimeLabel.setFont(new java.awt.Font("DejaVu Sans", 1, 14)); // NOI18N
        totalTimeLabel.setForeground(new java.awt.Color(8, 159, 41));

        tcs_Label.setIcon(new javax.swing.ImageIcon("/home/developer/Software/java/ngps/ngps/images/OFF.png")); // NOI18N
        tcs_Label.setText("  ");

        active_tcs_ip_Label.setFont(new java.awt.Font("Cantarell", 1, 15)); // NOI18N
        active_tcs_ip_Label.setForeground(new java.awt.Color(6, 158, 25));
        active_tcs_ip_Label.setText("   ");

        active_tcs_Label1.setFont(new java.awt.Font("Cantarell", 1, 15)); // NOI18N
        active_tcs_Label1.setForeground(new java.awt.Color(6, 158, 25));
        active_tcs_Label1.setText("   ");

        jLabel1.setFont(new java.awt.Font("Cantarell", 3, 15)); // NOI18N
        jLabel1.setText("TCS TYPE");

        jLabel2.setFont(new java.awt.Font("Cantarell", 3, 15)); // NOI18N
        jLabel2.setText("TCS IP address");

        javax.swing.GroupLayout sequencerPanelLayout = new javax.swing.GroupLayout(sequencerPanel);
        sequencerPanel.setLayout(sequencerPanelLayout);
        sequencerPanelLayout.setHorizontalGroup(
            sequencerPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(sequencerPanelLayout.createSequentialGroup()
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addGroup(sequencerPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(sequencerPanelLayout.createSequentialGroup()
                        .addComponent(actionButtonsPanel, javax.swing.GroupLayout.PREFERRED_SIZE, 110, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(sequencerPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(do_oneRadioButton)
                            .addComponent(do_allRadioButton)))
                    .addComponent(system_stateButton, javax.swing.GroupLayout.PREFERRED_SIZE, 174, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addGroup(sequencerPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(sequencerPanelLayout.createSequentialGroup()
                        .addGap(18, 18, 18)
                        .addGroup(sequencerPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING, false)
                            .addGroup(javax.swing.GroupLayout.Alignment.LEADING, sequencerPanelLayout.createSequentialGroup()
                                .addComponent(jLabel5, javax.swing.GroupLayout.PREFERRED_SIZE, 88, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(tcs_Label, javax.swing.GroupLayout.PREFERRED_SIZE, 18, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addGap(8, 8, 8)
                                .addComponent(jLabel1)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(active_tcs_Label1, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                            .addComponent(customoverheadProgressBar, javax.swing.GroupLayout.PREFERRED_SIZE, 274, javax.swing.GroupLayout.PREFERRED_SIZE))
                        .addGap(18, 18, 18)
                        .addGroup(sequencerPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addGroup(sequencerPanelLayout.createSequentialGroup()
                                .addComponent(customexposureProgressBar, javax.swing.GroupLayout.PREFERRED_SIZE, 742, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(mod_exptimeTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 48, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addGap(183, 183, 183)
                                .addComponent(totalTimeLabel, javax.swing.GroupLayout.PREFERRED_SIZE, 55, javax.swing.GroupLayout.PREFERRED_SIZE))
                            .addGroup(sequencerPanelLayout.createSequentialGroup()
                                .addComponent(jLabel2)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(active_tcs_ip_Label, javax.swing.GroupLayout.PREFERRED_SIZE, 258, javax.swing.GroupLayout.PREFERRED_SIZE))))
                    .addGroup(sequencerPanelLayout.createSequentialGroup()
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                        .addComponent(jScrollPane2, javax.swing.GroupLayout.PREFERRED_SIZE, 1095, javax.swing.GroupLayout.PREFERRED_SIZE))))
        );
        sequencerPanelLayout.setVerticalGroup(
            sequencerPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(sequencerPanelLayout.createSequentialGroup()
                .addGap(13, 13, 13)
                .addGroup(sequencerPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(sequencerPanelLayout.createSequentialGroup()
                        .addComponent(totalTimeLabel, javax.swing.GroupLayout.PREFERRED_SIZE, 26, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, 9, Short.MAX_VALUE)
                        .addGroup(sequencerPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(active_tcs_ip_Label)
                            .addGroup(sequencerPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                                .addComponent(jLabel5)
                                .addComponent(tcs_Label)
                                .addComponent(active_tcs_Label1)
                                .addComponent(jLabel1)
                                .addComponent(jLabel2)))
                        .addGap(3, 3, 3)
                        .addComponent(jScrollPane2, javax.swing.GroupLayout.PREFERRED_SIZE, 83, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addGroup(sequencerPanelLayout.createSequentialGroup()
                        .addGroup(sequencerPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(customexposureProgressBar, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(customoverheadProgressBar, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(system_stateButton, javax.swing.GroupLayout.PREFERRED_SIZE, 30, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(mod_exptimeTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 32, javax.swing.GroupLayout.PREFERRED_SIZE))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(sequencerPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(actionButtonsPanel, javax.swing.GroupLayout.PREFERRED_SIZE, 100, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addGroup(sequencerPanelLayout.createSequentialGroup()
                                .addComponent(do_oneRadioButton)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(do_allRadioButton)))))
                .addContainerGap())
        );

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(this);
        this.setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(sequencerPanel, javax.swing.GroupLayout.PREFERRED_SIZE, 1368, Short.MAX_VALUE)
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(sequencerPanel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
        );
    }// </editor-fold>//GEN-END:initComponents

    private void mod_exptimeTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_mod_exptimeTextFieldActionPerformed
        try{
            java.lang.String mod_exptime_string = mod_exptimeTextField.getText();
            mod_exptime_string = mod_exptime_string.trim();
            Double           mod_exptime = Double.parseDouble(mod_exptime_string);
            myObservationSequencerController.modexptime(mod_exptime.doubleValue());
        }catch(Exception e){
            System.out.println(e.toString());
        }// TODO add your handling code here:
    }//GEN-LAST:event_mod_exptimeTextFieldActionPerformed

    private void do_oneRadioButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_do_oneRadioButtonActionPerformed
         if(do_oneRadioButton.isSelected()){
             myObservationSequencerController.do_one_all(ObservationSequencerController.DO_ONE);
         }
    }//GEN-LAST:event_do_oneRadioButtonActionPerformed

    private void do_allRadioButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_do_allRadioButtonActionPerformed
         if(do_allRadioButton.isSelected()){
             myObservationSequencerController.do_one_all(ObservationSequencerController.DO_ALL);
         } 
    }//GEN-LAST:event_do_allRadioButtonActionPerformed

    private void system_stateButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_system_stateButtonActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_system_stateButtonActionPerformed


    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JPanel actionButtonsPanel;
    private javax.swing.JLabel active_tcs_Label1;
    private javax.swing.JLabel active_tcs_ip_Label;
    private edu.caltech.palomar.instruments.ngps.gui.CustomProgressBar customexposureProgressBar;
    private edu.caltech.palomar.instruments.ngps.gui.CustomProgressBar customoverheadProgressBar;
    private javax.swing.JRadioButton do_allRadioButton;
    private javax.swing.JRadioButton do_oneRadioButton;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JLabel jLabel5;
    private javax.swing.JScrollPane jScrollPane2;
    private javax.swing.JTextPane messagesTextPane;
    private javax.swing.JTextField mod_exptimeTextField;
    private javax.swing.JPanel sequencerPanel;
    private javax.swing.JButton system_stateButton;
    private javax.swing.JLabel tcs_Label;
    private javax.swing.JLabel totalTimeLabel;
    // End of variables declaration//GEN-END:variables
}
