/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package edu.caltech.palomar.instruments.ngps.parser;

//import edu.caltech.palomar.telescopes.P200.gui.tables.AstroObjectTableModel;
import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.StringTokenizer;
import edu.caltech.palomar.util.general.TargetListDocumentModel;
import java.util.HashMap;
import java.beans.PropertyChangeListener;
import java.beans.PropertyChangeSupport;
import javax.swing.UIManager;
import java.util.ArrayList;
import javax.swing.JTable;
import javax.swing.JEditorPane;
import java.sql.Timestamp;
import java.util.Iterator;
import java.util.Map;
import edu.caltech.palomar.instruments.ngps.object.*;
import edu.caltech.palomar.instruments.ngps.dbms.NGPSdatabase;
import edu.caltech.palomar.instruments.ngps.tables.DefaultTargetTableModel;
import edu.caltech.palomar.instruments.ngps.tables.ExtendedTargetTableModel;
import javax.swing.tree.DefaultTreeModel;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.JTree;
/**
 *
 * @author jennifermilburn
 */
public class TargetListParser2 {
  private transient PropertyChangeSupport propertyChangeListeners = new PropertyChangeSupport(this);
  public DefaultTargetTableModel      myTargetSimpleTableModel    = new DefaultTargetTableModel();
  public ExtendedTargetTableModel     myTargetExtendedTableModel  = new ExtendedTargetTableModel();
//  public DefaultTableModel2           myDefaultTableModel2        = new DefaultTableModel2();
  public TargetListDocumentModel      myTargetListDocumentModel   = new TargetListDocumentModel();
  public DefaultTreeModel             myDefaultTreeModel;
  private java.lang.String            currentFileName             = new java.lang.String();
  private java.io.File                currentFile;
  private  java.lang.String TERMINATOR     = new java.lang.String("\n");
  public   java.lang.String USERDIR        = System.getProperty("user.dir");
  public   java.lang.String SEP            = System.getProperty("file.separator");
  public static java.lang.String NAME      = "NAME";
  public static java.lang.String RA        = "RA";
  public static java.lang.String DEC       = "DEC";
  public static java.lang.String EPOCH     = "EPOCH";  
  public java.lang.String        file_path = new java.lang.String();
  public JEditorPane             currentJEditorPane;
  public JTable                  currentJTable;
  public ObservationSet          currentObservationSet;
  public NGPSdatabase            dbms;
  public int                     model_type;
  public static int              DEFAULT_MODEL  = 1;
  public static int              ETC_MODEL      = 2;
  public static int              EXTENDED_MODEL = 3;
  public JTree                   myJTree;
/*=============================================================================================
/        TargetListParser()
/=============================================================================================*/
 public TargetListParser2(){
     boolean test = false;
     setModelType(DEFAULT_MODEL);
     if(test){
        java.lang.String file_name = new java.lang.String("/Users/jennifermilburn/Desktop/NGPS/targets/ngps_targets_example.txt");
        readFile(file_name);         
     }
 } 
/*=============================================================================================
/   setJEditorPane(JEditorPane newEditorPane)
/=============================================================================================*/
public NGPSdatabase getNGPSdatabase(){
   return dbms;
}
public void setNGPSdatabase(NGPSdatabase current_dbms){
    dbms = current_dbms;
}
/*=============================================================================================
/   setJEditorPane(JEditorPane newEditorPane)
/=============================================================================================*/
public void setJEditorPane(JEditorPane newEditorPane){
   currentJEditorPane = newEditorPane; 
}
public void setJTable(JTable newJTable){
   currentJTable = newJTable; 
}
public void setJTree(JTree newJTree){
    myJTree = newJTree;
}
/*=============================================================================================
/   commitFileToDBMS(ObservationSet current)
/=============================================================================================*/
public void commitFileToDBMS(ObservationSet current,java.lang.String set_name){
    dbms.setProcessingState(NGPSdatabase.RUNNING);
    try{
       HashMap<String, Target> map = current.getObservationHashMap();
       int num_observations = map.size();
       current.setSET_NAME(set_name);
       current.setNUM_OBSERVATIONS(num_observations);
       Timestamp current_timestamp = current.constructTimestamp();
       current.setCreation_Timestamp(current_timestamp);
       current.setLastUpdate_Timestamp(current_timestamp);
       current.setOWNER(dbms.getOWNER());              
       int set_id = dbms.executeObservationSetInsertStatement(current);
       dbms.setSelectedObservationSet(current);
       Iterator hmIterator = map.entrySet().iterator();
       int target_number = 1;
       while (hmIterator.hasNext()) {
          Map.Entry mapElement = (Map.Entry)hmIterator.next();
            Target current_observation = ((Target)mapElement.getValue());
            current_observation.setSET_ID(set_id);
            current_observation.setOWNER(dbms.getOWNER());
            current_observation.setTarget_Number(target_number);
            int nexp = current_observation.instrument.getNEXP();
            for(int k=1;k<(nexp+1);k++){
              java.lang.String current_key    = ((java.lang.String)mapElement.getKey());
              System.out.println("Processing Key = "+current_key);
              current_observation.setSequence_Number(k);
              dbms.executeTargetsInsertStatement(current_observation);               
            }
            target_number = target_number+1;
        }   
       dbms.queryObservationSets(dbms.getOWNER());
    }catch(Exception e){
        System.out.println(e.toString());
    }
    dbms.setProcessingState(NGPSdatabase.IDLE);
}
public int commitFileToDBMS_2(ObservationSet current,java.lang.String set_name){
    int set_id = 0;
    dbms.setProcessingState(NGPSdatabase.RUNNING);
    try{
       int num_observations = myTargetExtendedTableModel.getRowCount();
//       int num_observations = map.size();
       current.setNUM_OBSERVATIONS(num_observations);
       Timestamp current_timestamp = current.constructTimestamp();
       current.setSET_NAME(set_name);
       current.setCreation_Timestamp(current_timestamp);
       current.setLastUpdate_Timestamp(current_timestamp);
       current.setOWNER(dbms.getOWNER());              
       set_id = dbms.executeObservationSetInsertStatement(current);
       dbms.setSelectedObservationSet(current);
//       Iterator hmIterator = map.entrySet().iterator();
       int target_number = 1;
       for(int i=0;i<num_observations;i++){
            Target current_observation = myTargetExtendedTableModel.getRecord(i);
            current_observation.setSET_ID(set_id);
            current_observation.setOWNER(dbms.getOWNER());
            current_observation.setTarget_Number(target_number);
            int nexp = current_observation.instrument.getNEXP();
            for(int k=1;k<(nexp+1);k++){
              current_observation.setSequence_Number(k);
              current_observation.setSTATE("PENDING");
              dbms.executeTargetsInsertStatement(current_observation);               
            }
            target_number = target_number+1;           
       }
       dbms.queryObservationSets(dbms.getOWNER());
    }catch(Exception e){
        System.out.println(e.toString());
    } 
    dbms.setProcessingState(NGPSdatabase.IDLE);
  return set_id;
}
/*=============================================================================================
/   executeParseFile()
/=============================================================================================*/
public void executeParseFile(){
    executeParseFile myexecuteParseFile = new executeParseFile();
    myexecuteParseFile.start();
}
/*=============================================================================================
/   parseText()
/=============================================================================================*/
public ArrayList parseHeaderLine(java.lang.String header_line){
    java.util.StringTokenizer st = new java.util.StringTokenizer(header_line,",");
    setModelType(DEFAULT_MODEL);
    int token_count = st.countTokens();
    ArrayList header_list = new ArrayList();
    for(int i=0;i<token_count;i++){
        java.lang.String token = st.nextToken();
        header_list.add(token.toUpperCase());
        System.out.println(token.toUpperCase());
        token = token.toUpperCase();
        if(token.matches("CHANNEL")){
           setModelType(ETC_MODEL); 
        }
        if(token.matches("WRANGE")){
           setModelType(ETC_MODEL); 
        }  
        if(token.matches("MAG")){
           setModelType(ETC_MODEL); 
        }
        if(token.matches("MAGSYSTEM")){
           setModelType(ETC_MODEL); 
        }
         if(token.matches("MAGFILTER")){
           setModelType(ETC_MODEL); 
        }
       if(token.matches("SRCMODEL")){
           setModelType(ETC_MODEL); 
        }
    }
  return header_list;
} 
public void setModelType(int new_model_type){
    model_type = new_model_type;
}
public int getModelType(){
    return model_type;
}
/*=============================================================================================
/   readFile(int FILE_TYPE)
/=============================================================================================*/
 public void readFile(java.lang.String current_file){
     Target               currentObservation;    
     myTargetSimpleTableModel.clearTable();
     myTargetExtendedTableModel.clearTable();
//     myDefaultTableModel2.clearTable();
 //    myDefaultTableModel2.initializeTableColumns();
     currentObservationSet = new ObservationSet();
     currentObservationSet.getObservationHashMap().clear();
     currentObservationSet.setSET_NAME(getFileName());
     currentObservationSet.setSTATE(ObservationSet.PENDING);
     myDefaultTreeModel = currentObservationSet.getTreeModel();
     DefaultMutableTreeNode root_node = (DefaultMutableTreeNode)myDefaultTreeModel.getRoot();
     java.lang.String          CommentString    = new java.lang.String();
     BufferedReader br           = null;
     String         current_line = new java.lang.String();
        try {
             FileInputStream fis = new FileInputStream(current_file);
             br = new BufferedReader(new InputStreamReader(fis));
          } catch (Exception e){
              System.out.printf("Problem opening selected Target Definition File for input.\n");
          }
          try {
             java.lang.String header_line = br.readLine();
             myTargetListDocumentModel.insertMessage(TargetListDocumentModel.RESPONSE, header_line);
             ArrayList        header_list = parseHeaderLine(header_line);
             int index = 1;
             while((current_line = br.readLine()) != null) {
              current_line = current_line;
              try{
                   CommentString = "";
                   while(current_line.startsWith("!")|current_line.isEmpty()){
                     if(current_line.startsWith("!")){
                        CommentString = CommentString + current_line;
                        myTargetListDocumentModel.insertMessage(TargetListDocumentModel.RESPONSE, current_line);
                     }
                     current_line = br.readLine();
                   }
                   currentObservation = parseObservationRecord(header_list,current_line);         
                   if(currentObservation != null){
                     currentObservation.setOrder(index);
                     currentObservationSet.getObservationHashMap().put(currentObservation.name, currentObservation);
                     myTargetSimpleTableModel.addTarget(currentObservation);  
                     myTargetExtendedTableModel.addTarget(currentObservation);  
//                     myDefaultTableModel2.addTarget(currentObservation);
                     myDefaultTreeModel.insertNodeInto(currentObservation.constructTreeNode(), root_node,root_node.getChildCount());
                     myTargetListDocumentModel.insertMessage(TargetListDocumentModel.COMMAND, current_line);                     
                   }
                   if(currentObservation== null){
                       myTargetListDocumentModel.insertMessage(TargetListDocumentModel.ERROR, current_line);                      
                   }                  
                   index++;
              }catch(Exception e2){
               System.out.println("Error parsing coordinates for a target position.");
               }// end of the catch block
             }// end of the while loop
          } catch (IOException e){
              System.out.printf("Problem parsing selected Target Definition File.\n" + e.toString());
          } 
//       myJTree.setModel(myDefaultTreeModel);
//    commitFileToDBMS(currentObservationSet);      
 }     
/*=============================================================================================
/   parseText()
/=============================================================================================*/
 public void parseText(){
    Target      currentObservation;
    java.lang.String CommentString = new java.lang.String();
    java.lang.String current_line   = new java.lang.String();
    java.lang.String text = myTargetListDocumentModel.getText();    
 
    // Clear the table before we start opening the new file    
//    presenterKey.clear();
    myTargetSimpleTableModel.clearTable();
    myTargetExtendedTableModel.clearTable();
//    myListModel.clearTable();
    myTargetListDocumentModel.clearDocument();
    myDefaultTreeModel = currentObservationSet.getTreeModel();
    DefaultMutableTreeNode root_node = (DefaultMutableTreeNode)myDefaultTreeModel.getRoot();
    java.util.StringTokenizer tokenString = new java.util.StringTokenizer(text,TERMINATOR);  
    java.lang.String header_line = tokenString.nextToken();
    ArrayList        header_list = parseHeaderLine(header_line);
    myTargetListDocumentModel.insertMessage(TargetListDocumentModel.RESPONSE, header_line);
    int index = 1;
    while(tokenString.hasMoreElements()){
        try{
           current_line = tokenString.nextToken(); 
           CommentString = "";
           while(current_line.startsWith("!")|current_line.isEmpty()){
               if(current_line.startsWith("!")){
                        CommentString = CommentString + current_line;
                        myTargetListDocumentModel.insertMessage(TargetListDocumentModel.RESPONSE, current_line);
                }
                current_line = tokenString.nextToken(); 
           }   

           currentObservation = parseObservationRecord(header_list,current_line);         
           if(currentObservation != null){
             currentObservation.setOrder(index);
             index = index+1;
//                     myAstroObjectTable.addRecord(currentObservation);
             currentObservationSet.getObservationHashMap().put(currentObservation.name, currentObservation);
             myTargetSimpleTableModel.addTarget(currentObservation);  
             myTargetExtendedTableModel.addTarget(currentObservation);  
             myDefaultTreeModel.insertNodeInto(currentObservation.constructTreeNode(), root_node,root_node.getChildCount());
//                     myListModel.addRecord(currentAstroObject.name);0
             myTargetListDocumentModel.insertMessage(TargetListDocumentModel.COMMAND, current_line);                     
           }               
//            }
            if(currentObservation == null){
                myTargetListDocumentModel.insertMessage(TargetListDocumentModel.ERROR, current_line);                      
            } 
        }catch(Exception e){
            System.out.println("Problem parsing the text of the edited file. "+e.toString());
        }  
//        myJTree.setModel(myDefaultTreeModel);
    }
 } 
/*=============================================================================================
/   parseObservationRecord(java.util.ArrayList current_list,java.lang.String current)
/=============================================================================================*/
 public Target parseObservationRecord(java.util.ArrayList current_list,java.lang.String current){
     Target current_observation = new Target();
     StringTokenizer st = new StringTokenizer(current,",");
     java.lang.String[] token_array = current.split(",");
     int count_header = current_list.size();
     int count_current = st.countTokens();
     System.out.println("Counted Tokens Header = "+count_header + " Counted Tokens String = "+count_current);
     for(int i=0;i<count_current;i++){
        java.lang.String field = ((java.lang.String)current_list.get(i)).trim();
        java.lang.String current_value = token_array[i];
        current_value = current_value.trim();
        if(current_value == null){
            System.out.println("Null value");
        }
        System.out.println(field + " "+ current_value);
        if(field.matches("NAME")){
            current_observation.name = current_value;
        }        
        if(field.matches("RA")){
           current_observation.sky.setRightAscension(current_value);
        }
        if(field.matches("DECL")){
            current_observation.sky.setDeclination(current_value);
        }
        if(field.matches("OFFSET_RA")){
            try{
               java.lang.Double current_offset_RA = Double.parseDouble(current_value);
               current_observation.sky.setOFFSET_RA(current_offset_RA.doubleValue()); 
            }catch(Exception e){
               System.out.println(e.toString()); 
            }        
        }
        if(field.matches("OFFSET_DEC")){
            try{
               java.lang.Double current_offset_DEC = Double.parseDouble(current_value);
               current_observation.sky.setOFFSET_DEC(current_offset_DEC.doubleValue()); 
            }catch(Exception e){
               System.out.println(e.toString()); 
            }        
        }
//        if(field.matches("EPOCH")){
//            try{                
//                current_observation.sky.setEPOCH(current_value);
//            }catch(Exception e){
//                System.out.println(e.toString());
//            }            
//        }
        if(field.matches("EXPTIME")){
            try{
                current_observation.instrument.setExposuretime(current_value.trim());
                java.util.StringTokenizer st2 = new java.util.StringTokenizer(current_value," ");
                java.lang.String          code_string   = st2.nextToken();
                java.lang.String          number_string = st2.nextToken();                
                java.lang.Double exptime = Double.valueOf(number_string);
                current_observation.instrument.setEXPTIME_CODE(code_string);
                current_observation.instrument.setEXPTIME(exptime.doubleValue());
            }catch(Exception e){
                System.out.println(e.toString());
            }            
        }
        if(field.matches("NEXP")){
            try{  
                java.lang.Integer nexp = Integer.valueOf(current_value);
                current_observation.instrument.setNEXP(nexp);
            }catch(Exception e){
                System.out.println(e.toString());
            }            
        }  
        if(field.matches("SLITWIDTH")){
            try{
                current_observation.instrument.setSlitwidth_string(current_value.trim()); 
//                java.util.StringTokenizer st3 = new java.util.StringTokenizer(current_value," ");
//                java.lang.String code_token1 = st3.nextToken();
//                java.lang.String value_token2 = st3.nextToken();
//                current_observation.instrument.setSlitwidth_code(code_token1);
//                java.lang.Double slitw = Double.valueOf(value_token2);
//                   if(slitw >= 0.0 & slitw <= 10.00){
//                      current_observation.instrument.setSlitwidth(slitw); 
//                   }                            
            }catch(Exception e){
                System.out.println(e.toString());
            }            
        }
      if(field.matches("SLITOFFSET")){
            try{
                java.lang.Double slitoffset = Double.valueOf(current_value);
                current_observation.instrument.setSlitOffset(slitoffset);
            }catch(Exception e){
                System.out.println(e.toString());
            }            
       }  
      if(field.matches("CCDMODE")){
            try{            
                current_observation.instrument.setOBSMODE(current_value);
            }catch(Exception e){
                System.out.println(e.toString());
            }            
       }       
       if(field.matches("BINSPAT")){
            try{  
                java.lang.Integer binspat = Integer.valueOf(current_value);
                current_observation.instrument.setBIN_SPACE(binspat); 
            }catch(Exception e){
                System.out.println(e.toString());
            }            
       }  
       if(field.matches("BINSPECT")){
            try{  
                java.lang.Integer binspect = Integer.valueOf(current_value);
                current_observation.instrument.setBIN_SPEC(binspect); 
            }catch(Exception e){
                System.out.println(e.toString());
            }            
       } 
       if(field.matches("SLITANGLE")){
            try{
              current_observation.instrument.setRequestedSlitAngle(current_value);
            }catch(Exception e){
                System.out.println(e.toString());
            }            
       }  
       if(field.matches("WRANGE")){
            try{
              java.util.StringTokenizer st4 = new java.util.StringTokenizer(current_value," ");
              java.lang.String    start_w = st4.nextToken();
              java.lang.String    end_w   = st4.nextToken();              
              current_observation.etc.setWRANGE_LOW((Integer.valueOf(start_w)).intValue());
              current_observation.etc.setWRANGE_HIGH((Integer.valueOf(end_w)).intValue());
            }catch(Exception e){
                System.out.println(e.toString());
            }            
       }   
       if(field.matches("CHANNEL")){
            try{
              current_observation.etc.setChannel(current_value.trim());
            }catch(Exception e){
                System.out.println(e.toString());
            }            
       }   
       if(field.matches("MAG")){
            try{
              double magnitude = (Double.valueOf(current_value)).doubleValue();
              current_observation.etc.setMagnitude(magnitude);
            }catch(Exception e){
                System.out.println(e.toString());
            }            
       }  
       if(field.matches("MAGREF")){
            try{
              current_observation.etc.setMagref(current_value.trim());
              java.util.StringTokenizer st4 = new java.util.StringTokenizer(current_value," ");
              java.lang.String    magref_system  = st4.nextToken();
              java.lang.String    mageref_filter = st4.nextToken();              
              current_observation.etc.setMagref_system(magref_system);
              current_observation.etc.setMagref_filter(mageref_filter);
            }catch(Exception e){
                System.out.println(e.toString());
            }            
       } 
       if(field.matches("MAGSYSTEM")){
            try{
              current_observation.etc.setMagref_system(current_value.trim());
//              current_observation.etc.setMagref_filter(current_value.trim());
            }catch(Exception e){
                System.out.println(e.toString());
            }            
       } 
       if(field.matches("MAGFILTER")){
            try{
//              current_observation.etc.setMagref_system(current_value.trim());
              current_observation.etc.setMagref_filter(current_value.trim());
            }catch(Exception e){
                System.out.println(e.toString());
            }            
       }        
       if(field.matches("SRCMODEL")){
            try{
              current_observation.etc.setSrcmodel(current_value.trim());
            }catch(Exception e){
                System.out.println(e.toString());
            }            
       }          
      }
     return current_observation;
 }
/*=============================================================================================
/   writeFile()
/=============================================================================================*/
 public void writeFile(){
         try {
            java.io.PrintWriter  pw = new java.io.PrintWriter(currentFileName);
            pw.write(myTargetListDocumentModel.getText());
            pw.flush();
            pw.close();
          } catch (Exception e){
              System.out.printf("Problem writing to the Target Definition File.\n");
          }   
 } 
/*=============================================================================================
/    setFileName(java.lang.String newFileName)
/=============================================================================================*/
  public void setFileName(java.lang.String new_file_name){
    String  old_file_name = this.currentFileName;
    this.currentFileName = new_file_name;
    propertyChangeListeners.firePropertyChange("current_file_name", old_file_name, new_file_name);
  }
  public java.lang.String getFileName(){
    return currentFileName;
  }
/*=============================================================================================
/    setFile(java.io.File newFile)
/=============================================================================================*/
   public void setFile(java.io.File newFile){
      currentFile = newFile;
   }
   public java.io.File getFile(){
       return currentFile;
   }  
/*================================================================================================
/     setDarkImage(java.lang.String new_dark_image)
/=================================================================================================*/
  public void setFilePath(java.lang.String new_file_path) {
    java.lang.String old_file_path = file_path;
    file_path = new_file_path;
    propertyChangeListeners.firePropertyChange("file_path", old_file_path, new_file_path);
  }
  public java.lang.String getFilePath() {
    return file_path;
  } 
/*================================================================================================
/      add and remove Property Change Listeners
/=================================================================================================*/
  public synchronized void removePropertyChangeListener(PropertyChangeListener l) {
    propertyChangeListeners.removePropertyChangeListener(l);
  }
  public synchronized void addPropertyChangeListener(PropertyChangeListener l) {
    propertyChangeListeners.addPropertyChangeListener(l);
  }
/*=============================================================================================
/       INNER CLASS executeFlatFieldThread
/=============================================================================================*/
    public class executeParseFile  implements Runnable{
    private Thread myThread; 
/*=============================================================================================
/         executeParseFile()
/=============================================================================================*/
    private executeParseFile(){
    }
/*=============================================================================================
/           run()
/=============================================================================================*/
    public void run(){
        java.lang.String current = file_path;
        myTargetListDocumentModel.clearDocument();
        readFile(current);
        currentJEditorPane.setDocument(myTargetListDocumentModel.getDocument());
        int model_type = getModelType();
        if(model_type == DEFAULT_MODEL){
            currentJTable.setModel(myTargetExtendedTableModel);
        }
        if(model_type == ETC_MODEL){
            currentJTable.setModel(myTargetExtendedTableModel);
        }    
        if(model_type == EXTENDED_MODEL){
            currentJTable.setModel(myTargetExtendedTableModel);
        }
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
/*================================================================================================
/      add and remove Property Change Listeners
/=================================================================================================*/
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
            java.util.logging.Logger.getLogger(TargetListParser2.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (InstantiationException ex) {
            java.util.logging.Logger.getLogger(TargetListParser2.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (IllegalAccessException ex) {
            java.util.logging.Logger.getLogger(TargetListParser2.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (javax.swing.UnsupportedLookAndFeelException ex) {
            java.util.logging.Logger.getLogger(TargetListParser2.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        }
        //</editor-fold>
             try {
                 UIManager.setLookAndFeel(UIManager.getCrossPlatformLookAndFeelClassName());
                 java.lang.String test = UIManager.getSystemLookAndFeelClassName();
                 System.out.println(test);
               } catch(Exception e) {
                   e.printStackTrace();
               }
               new TargetListParser2();
    }
}
