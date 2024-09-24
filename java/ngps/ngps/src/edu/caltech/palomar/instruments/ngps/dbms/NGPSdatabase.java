/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package edu.caltech.palomar.instruments.ngps.dbms;

//import static edu.caltech.palomar.instruments.ngps.dbms.NGPS_Database.ERROR;
import edu.caltech.palomar.instruments.ngps.object.ObservationSet;
import edu.caltech.palomar.instruments.ngps.object.Owner;
import edu.caltech.palomar.instruments.ngps.object.Target;
import edu.caltech.palomar.instruments.ngps.otm.OTMlauncher;
//import edu.caltech.palomar.instruments.ngps.DefaultTableModel2;
//import edu.caltech.palomar.instruments.ngps.tables.DBMSTargetTableModel;
import edu.caltech.palomar.instruments.ngps.tables.ObservationSetTableModel;
import edu.caltech.palomar.instruments.ngps.tables.OwnerTableModel;
import edu.caltech.palomar.util.general.CommandLogModel;
import java.beans.PropertyChangeListener;
import java.beans.PropertyChangeSupport;
import java.io.FileInputStream;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.Statement;
import java.util.Base64;
import java.util.Properties;
import javax.crypto.Cipher;
import javax.crypto.SecretKey;
import javax.crypto.spec.SecretKeySpec;
import javax.swing.DefaultListModel;
import javax.swing.JFrame;
import org.apache.log4j.DailyRollingFileAppender;
import org.apache.log4j.Layout;
import org.apache.log4j.Logger;
import org.apache.log4j.PatternLayout;
import edu.caltech.palomar.instruments.ngps.tables.MasterDBTableModel;
import java.sql.Timestamp;
import java.util.ArrayList;
import java.util.Stack;
import java.io.FileWriter;
import java.io.PrintWriter;
import edu.caltech.palomar.instruments.ngps.charts.CombinedChartTest;
import javax.swing.JOptionPane;
import javax.swing.UIManager;

/**
 *
 * @author jennifermilburn
 */
public class NGPSdatabase {
    transient protected PropertyChangeSupport propertyChangeListeners = new PropertyChangeSupport(this);
    public PreparedStatement         INSERT_OBSERVATION_SET_PREP_STATEMENT;
    public PreparedStatement         INSERT_OBSERVATION_PREP_STATEMENT;
    public PreparedStatement         UPDATE_OBSERVATION_PREP_STATEMENT;
    public PreparedStatement         INSERT_OWNER_PREP_STATEMENT;
    public PreparedStatement         UPDATE_OWNER_PREP_STATEMENT;
    public PreparedStatement         UPDATE_OBSERVATION_SET_PREP_STATEMENT;
    public PreparedStatement         DELETE_TARGET_PREP_STATEMENT;
    public PreparedStatement         QUERY_STATE_PREP_STATEMENT;
    public PreparedStatement         QUERY_TARGET_PREP_STATEMENT;
    public static int                DEBUG              = 0;
    public static int                INFO               = 1;
    public static int                WARN               = 2;
    public static int                ERROR              = 3;
    public static int                FATAL              = 4;    
    static Logger                    ngps_dbms_Logger    = Logger.getLogger(NGPSdatabase.class);
    public DailyRollingFileAppender  fileAppender       = new DailyRollingFileAppender();
    private Layout                   layout             = new PatternLayout("%-5p [%t]: %m%n");
    public CommandLogModel           myCommandLogModel  = new CommandLogModel();
    private java.lang.String         SEP             = System.getProperty("file.separator");
    public java.lang.String          USERDIR         = System.getProperty("user.dir");
    private java.lang.String         CONFIG          = new java.lang.String("config");
    private java.lang.String         OTM             = new java.lang.String("otm");    
    public  java.lang.String         DBMS_CONNECTION_PROPERTIES = "ngps_dbms.ini";
    public java.lang.String          LOG_DIRECTORY   = new java.lang.String();
    public String                       MYSQLDB;
    public String                       TARGET_TABLE;
    public String                       TARGETSET_TABLE;
    public String                       ACCOUNT_TABLE;
    public Connection                conn = null;
    private boolean                  connected;
    private boolean                  table_populated;
    private static  Cipher           cipher;    
    public SecretKey                 originalKey; 
    public OTMlauncher               myOTMlauncher;
    private JFrame                   frame;
    public MasterDBTableModel        myTargetDBMSTableModel        = new MasterDBTableModel(MasterDBTableModel.MINIMAL_TABLE);
    public MasterDBTableModel        myTargetQueryTableModel       = new MasterDBTableModel(MasterDBTableModel.MINIMAL_TABLE);
    public OwnerTableModel           myOwnerTableModel             = new OwnerTableModel();
    public DefaultListModel          ownerslist                    = new DefaultListModel();
    public ObservationSetTableModel  myObservationSetTableModel    = new ObservationSetTableModel();
    public int                       selected_set_id;
    private java.lang.String         owner                      = new java.lang.String();
    private boolean                  logged_in;
    public  Owner                    current_owner;
    private int                      logged_in_state;
    public static int                NOT_LOGGED_IN = 1;
    public static int                LOGIN_SUCCESSFUL = 2;
    public static int                LOGIN_UNSUCCESSFUL =3;
    public ObservationSet            selectedObservationSet;
    public Stack                     delete_stack      = new Stack();
    private int                      ProcessingState;
    public static int                IDLE    = 1;
    public static int                RUNNING = 2; 
    public java.lang.String          PYTHON_INSTALL_DIR;
    private java.lang.String         selected_set_name    = new java.lang.String();
    private java.lang.String         selected_target_name = new java.lang.String();
    private CommandLogModel          publicCommandLogModel;
    public  CombinedChartTest        currentCombinedChartTest;
/*================================================================================================
/      NGPSdatabase()    
/=================================================================================================*/
public NGPSdatabase(){
  //bootstrap();
  setTablePopulated(false);  
  setProcessingState(IDLE);
  boolean test = false;
  if(test){
    java.lang.String test_owner = "jennifer";
    java.lang.String test_target = "ZTF22aaheakp";
    this.setOWNER(test_owner);
    initializeDBMS();
    executeQueryTargetPreparedStatement(test_target);
  }
}
/*================================================================================================
/       setCombinedChartTest(CombinedChartTest newCombinedChartTest)
/=================================================================================================*/
public void setCombinedChartTest(CombinedChartTest newCombinedChartTest){
    currentCombinedChartTest = newCombinedChartTest;
}
public CombinedChartTest getCombinedChartTest(){
    return currentCombinedChartTest;
}
public void a_test(){
    
}
/*================================================================================================
/       initializeDBMS()
/=================================================================================================*/
public void bootstrap(){
    try{
      java.lang.String BOOTSTRAP_FILE = USERDIR + SEP +"bootstrap.ini";
      FileInputStream  bootstrap_properties_file            = new FileInputStream(BOOTSTRAP_FILE);
      Properties       bootstrap_properties                 = new Properties();
      bootstrap_properties.load(bootstrap_properties_file);
      bootstrap_properties_file.close();
      USERDIR        = bootstrap_properties.getProperty("USERDIR");
    }catch(Exception e){
        System.out.println(e.toString());
    }
}
/*================================================================================================
/       initializeDBMS()
/=================================================================================================*/
public void initializeDBMS(){
   try{
      java.lang.String DBMS_CONNECTION_PROPERTIES_FILE = USERDIR + SEP + CONFIG + SEP + DBMS_CONNECTION_PROPERTIES;
      FileInputStream  dbms_properties_file            = new FileInputStream(DBMS_CONNECTION_PROPERTIES_FILE);
      Properties       dbms_properties                 = new Properties();
      dbms_properties.load(dbms_properties_file);
      dbms_properties_file.close();
      
      MYSQLDB = dbms_properties.getProperty("DBMS");
      TARGET_TABLE = MYSQLDB+"."+dbms_properties.getProperty("TARGET_TABLE");
      TARGETSET_TABLE = MYSQLDB+"."+dbms_properties.getProperty("TARGETSET_TABLE");
      ACCOUNT_TABLE = MYSQLDB+"."+dbms_properties.getProperty("ACCOUNT_TABLE");

      String SYSTEM             = dbms_properties.getProperty("SYSTEM");
      String USERNAME           = dbms_properties.getProperty("USERNAME");
      String PASSWORD           = dbms_properties.getProperty("PASSWORD");
             PYTHON_INSTALL_DIR = dbms_properties.getProperty("PYTHON_INSTALL_DIR");
      LOG_DIRECTORY = dbms_properties.getProperty("LOG_DIRECTORY");      
      Class.forName("com.mysql.jdbc.Driver").newInstance(); 
      conn = DriverManager.getConnection("jdbc:mysql://"+SYSTEM+SEP+MYSQLDB+"?"+"user="+USERNAME+"&password="+PASSWORD);
      setConnected(true);
      constructTargetsPreparedStatement();
      constructUpdateTargetsPreparedStatement();
      constructObservationSetPreparedStatement();
      constructOwnerPreparedStatement(); 
      constructUpdateOwnerPreparedStatement();
      constructUpdateObservationSetPreparedStatement();
      constructDeleteTargetPreparedStatement();
      constructQueryState();
      constructQueryTargetPreparedStatement();
      read_encrpytion_key();
      setLoggedInState(NGPSdatabase.NOT_LOGGED_IN);
   }catch(Exception e){
       setConnected(false);
       logMessage(ERROR,"Error loading the JDBC driver. "+e.toString());
   }     
}
public void setFrame(JFrame new_frame){
  frame = new_frame;  
}
/*================================================================================================
/       initializeOTMlauncher()
/=================================================================================================*/
public void setPublicDocumentModel(CommandLogModel new_CommandLogModel){
   publicCommandLogModel = new_CommandLogModel; 
}
/*================================================================================================
/       initializeOTMlauncher()
/=================================================================================================*/
public void initializeOTMlauncher(){
   myOTMlauncher  = new OTMlauncher();
   myOTMlauncher.setNGPSDatabase(this);
   myOTMlauncher.setFrame(frame);
}
/*================================================================================================
/     setOrder(int new_order)
/=================================================================================================*/
  public void setConnected(boolean new_connected) {
    boolean  old_connected = this.connected;
    connected = new_connected;
   propertyChangeListeners.firePropertyChange("connected", Boolean.valueOf(old_connected), Boolean.valueOf(new_connected));
  }
  public boolean isConnected() {
    return connected;
  } 
 /*================================================================================================
/     setOrder(int new_order)
/=================================================================================================*/
  public void setTablePopulated(boolean new_table_populated) {
    boolean  old_table_populated = this.table_populated;
    table_populated = new_table_populated;
   propertyChangeListeners.firePropertyChange("table_populated", Boolean.valueOf(old_table_populated), Boolean.valueOf(new_table_populated));
  }
  public boolean isTablePopulated() {
    return table_populated;
  } 
/*================================================================================================
/      setNUM_OBSERVATIONS(int new_num_observations)
/=================================================================================================*/
  public void setSelectedSET_ID(int new_selected_set_id) {
    int  old_selected_set_id = this.selected_set_id;
    this.selected_set_id = new_selected_set_id;
   propertyChangeListeners.firePropertyChange("selected_set_id", Integer.valueOf(old_selected_set_id), Integer.valueOf(new_selected_set_id));
  }
  public int getSelectedSET_ID() {
    return selected_set_id;
  } 
/*================================================================================================
/      setOWNER(java.lang.String new_owner)
/=================================================================================================*/
  public void setOWNER(java.lang.String new_owner) {
    java.lang.String  old_owner = this.owner;
    this.owner = new_owner;
   propertyChangeListeners.firePropertyChange("owner", (old_owner), (new_owner));
  }
  public java.lang.String getOWNER() {
    return owner;
  } 
/*================================================================================================
/      setOWNER_OBJECT(Owner new_Owner)
/=================================================================================================*/
 public void setOWNER_OBJECT(Owner new_Owner){
     Owner old_owner = current_owner;
     this.current_owner = new_Owner;
     propertyChangeListeners.firePropertyChange("current_owner", (old_owner), (new_Owner));
 }    
/*================================================================================================
/      setSelectedSetName(java.lang.String new_selected_set_name)
/=================================================================================================*/
  public void setSelectedSetName(java.lang.String new_selected_set_name) {
    java.lang.String  old_selected_set_name = this.selected_set_name;
    this.selected_set_name = new_selected_set_name;
   propertyChangeListeners.firePropertyChange("selected_set_name", (old_selected_set_name), (new_selected_set_name));
  }   

  public java.lang.String getSelectedSetName() {
    return selected_set_name;
  } 
/*================================================================================================
/      setSelectedSetName(java.lang.String new_selected_set_name)
/=================================================================================================*/
  public void setSelectedTargetName(java.lang.String new_selected_target_name) {
    java.lang.String  old_selected_target_name = this.selected_target_name;
    this.selected_target_name = new_selected_target_name;
   propertyChangeListeners.firePropertyChange("selected_target_name", (old_selected_target_name), (new_selected_target_name));
  }   

  public java.lang.String getSelectedTargetName() {
    return selected_target_name;
  }   
/*================================================================================================
/     setLoggedIn(boolean new_connected) 
/=================================================================================================*/
  public void setLoggedIn(boolean new_logged_in) {
    boolean  old_logged_in = this.logged_in;
    logged_in = new_logged_in;
   propertyChangeListeners.firePropertyChange("logged_in", Boolean.valueOf(old_logged_in), Boolean.valueOf(new_logged_in));
  }
  public boolean isLoggedIn() {
    return logged_in;
  }  
/*================================================================================================
/     setLoggedIn(boolean new_connected) 
/=================================================================================================*/
  public void setLoggedInState(int new_logged_in_state) {
    int  old_logged_in_state = this.logged_in_state;
    logged_in_state = new_logged_in_state;
   propertyChangeListeners.firePropertyChange("logged_in_state", Integer.valueOf(old_logged_in_state), Integer.valueOf(new_logged_in_state));
  }
  public int getLoggedInState() {
    return logged_in_state;
  }  
/*================================================================================================
/      setSpectrumState(int newSpectrumState)
/=================================================================================================*/
  public void setProcessingState(int newProcessingState) {
    int  oldProcessingState = this.ProcessingState;
    this.ProcessingState = newProcessingState;
    propertyChangeListeners.firePropertyChange("ProcessingState", Integer.valueOf(oldProcessingState), Integer.valueOf(newProcessingState));
  }
  public int getProcessingState() {
    return ProcessingState;
  }   
/*=============================================================================================
/       encrypt(String plainText, SecretKey secretKey)
/=============================================================================================*/
    public String encrypt(String plainText, SecretKey secretKey)
            throws Exception {
        byte[] plainTextByte = plainText.getBytes();
        cipher.init(Cipher.ENCRYPT_MODE, secretKey);
        byte[] encryptedByte = cipher.doFinal(plainTextByte);
        Base64.Encoder encoder = Base64.getEncoder();
        String encryptedText = encoder.encodeToString(encryptedByte);
        return encryptedText;
    }
/*=============================================================================================
/       decrypt(String encryptedText, SecretKey secretKey)
/=============================================================================================*/
    public String decrypt(String encryptedText, SecretKey secretKey)
            throws Exception {
        Base64.Decoder decoder = Base64.getDecoder();
        byte[] encryptedTextByte = decoder.decode(encryptedText);
        cipher.init(Cipher.DECRYPT_MODE, secretKey);
        byte[] decryptedByte = cipher.doFinal(encryptedTextByte);
        String decryptedText = new String(decryptedByte);
        return decryptedText;
    }    
/*=============================================================================================
/       read_encrpytion_key()
/=============================================================================================*/
public void read_encrpytion_key(){
    try{
      java.lang.String path = USERDIR+SEP+CONFIG+SEP+"ngps_keystore_file";
      java.io.FileReader fr = new java.io.FileReader(path);
      java.io.BufferedReader br = new java.io.BufferedReader(fr);
      java.lang.String encodedKey = br.readLine();
      byte[] decodedKey = Base64.getDecoder().decode(encodedKey);
      originalKey = new SecretKeySpec(decodedKey, 0, decodedKey.length, "AES"); 
      cipher = Cipher.getInstance("AES"); //SunJCE provider AES algorithm, mode(optional) and padding schema(optional)        
    }catch(Exception e){
        System.out.println(e.toString());
    }
}
/*================================================================================================
/      constructDeleteTargetPreparedStatement()
/=================================================================================================*/
public void createObservationSet(java.lang.String set_name){
   int num_observations = myTargetDBMSTableModel.getRowCount();
       ObservationSet     current = new ObservationSet();
       current.setNUM_OBSERVATIONS(num_observations);
       Timestamp current_timestamp = current.constructTimestamp();
       current.setSET_NAME(set_name);
       current.setCreation_Timestamp(current_timestamp);
       current.setLastUpdate_Timestamp(current_timestamp);
       current.setOWNER(getOWNER());              
       int set_id = executeObservationSetInsertStatement(current);
       setSelectedObservationSet(current);
       setSelectedSET_ID(set_id);
       setSelectedSetName(set_name);
}
/*================================================================================================
/      constructDeleteTargetPreparedStatement()
/=================================================================================================*/
public void executeSaveAs(java.lang.String set_name){
    executeSaveAs myexecuteSaveAs = new executeSaveAs(set_name);
    myexecuteSaveAs.start();
}
/*================================================================================================
/      constructDeleteTargetPreparedStatement()
/=================================================================================================*/
public int SaveAs(java.lang.String set_name){
  setProcessingState(RUNNING);
  logMessage(CommandLogModel.COMMAND, "Saving target set to the dmbs: "+set_name);
    int set_id = 0;
    try{
       ObservationSet     current = new ObservationSet();
       current.getObservationHashMap().clear();
       current.setSTATE(ObservationSet.PENDING);
       double seeing     = myOTMlauncher.getSeeing();
       int    wavelength = myOTMlauncher.getWavelength();
//       java.lang.String start_time_string = "2022-01-01T03:00:00.000";
//       java.sql.Timestamp current_timestamp_otm = myOTMlauncher.string_to_timestamp(start_time_string);
       java.sql.Timestamp current_timestamp_otm = myOTMlauncher.getStartTimestamp();       
       myOTMlauncher.OTM(current_timestamp_otm,seeing,wavelength);
       int num_observations = myTargetDBMSTableModel.getRowCount();
//       int num_observations = map.size();
       current.setNUM_OBSERVATIONS(num_observations);
       Timestamp current_timestamp = current.constructTimestamp();
       current.setSET_NAME(set_name);
       current.setCreation_Timestamp(current_timestamp);
       current.setLastUpdate_Timestamp(current_timestamp);
       current.setOWNER(getOWNER());              
       set_id = executeObservationSetInsertStatement(current);
       setSelectedObservationSet(current);
       setSelectedSET_ID(set_id);
       current.setSET_ID(set_id);
       setSelectedSetName(set_name);
//       Iterator hmIterator = map.entrySet().iterator();
       int target_number = 1;
       for(int i=0;i<num_observations;i++){
            Target current_observation = myTargetDBMSTableModel.getRecord(i);
            current.getObservationHashMap().put(current_observation.name, current_observation);
            current_observation.setSET_ID(set_id);
            executeTargetsInsertStatement(current_observation);                           
       }
       queryObservationSets(getOWNER());
       queryObservations(set_id);
    }catch(Exception e){
        System.out.println(e.toString());
    } 
    myTargetDBMSTableModel.setEdited(false);
    setProcessingState(IDLE);
  logMessage(CommandLogModel.COMMAND, "Saving target set to the dmbs complete.");
  return set_id;
}
/*================================================================================================
/      constructDeleteTargetPreparedStatement()
/=================================================================================================*/
public void export(java.lang.String new_export_file){
    try{
        logMessage(CommandLogModel.COMMAND, "Exporting table: "+new_export_file);
        boolean autoFlush = true;
        FileWriter  file   = new FileWriter(new_export_file);
        PrintWriter output = new PrintWriter(file, autoFlush);
        int num_observations = myTargetDBMSTableModel.getRowCount();  
        java.lang.String header_row = "OBSERVATION_ID"+","+"SET_ID"+","+"STATE"+","+"OBS_ORDER"+","+
        "TARGET_NUMBER"+","+"SEQUENCE_NUMBER"+","+"NAME"+","+"RA"+","+"DECL"+","+"OFFSET_RA"+","+"OFFSET_DEC"+","+"EXPTIME"+","+       
        "SLIT_WIDTH"+","+"SLIT_OFFSET"+","+"CCDMODE"+","+"BIN_SPEC"+","+"BIN_SPAC"+","+"AIRMASS_MAX"+","+"SLIT_ANGLE"+","+"WRANGE_HIGH"+","+"WRANGE_LOW"+","+"CHANNEL"+","+       
        "MAGNITUDE"+","+"MAGREF_SYSTEM"+","+"MAGREF_FILTER"+","+"SRCMODEL"+","+"OTMexpt"+","+"OTMslitwidth"+","+"OTMcass"+","+"AIRMASS_START"+","+"AIRMASS_END"+","+       
        "OTMskymag"+","+"OTMdead"+","+"OTMslewgo"+","+"OTMstart"+","+"OTMend"+","+"OTMpa"+","+"OTMwait"+","+"OTMflag"+","+"OTMlast"+","+       
        "OTMslew"+","+"OTMmoon"+","+"OTMSNR"+","+"OTMres"+","+"OTMseeing"+","+"OTMslitangle"+","+"NOTE"+","+"COMMENT"+","+"POINTMODE";         
         output.println(header_row);                
         for(int i=0;i<num_observations;i++){
            Target current = myTargetDBMSTableModel.getRecord(i);
            java.lang.String current_record = 
                current.getObservationID()+","+
                current.getSET_ID()+","+
                current.getSTATE()+","+
                current.getOrder()+","+
                current.getTarget_Number()+","+
                current.getSequence_Number()+","+        
                current.getName()+","+
                current.sky.getRightAscension()+","+
                current.sky.getDeclination()+","+
                current.sky.getOFFSET_RA()+","+
                current.sky.getOFFSET_DEC()+","+  
                current.instrument.getExposuretime()+","+
                current.instrument.getSlitwidth_string()+","+
                current.instrument.getSlitOffset()+","+
                current.instrument.getOBSMODE()+","+
                current.instrument.getBIN_SPEC()+","+
                current.instrument.getBIN_SPACE()+","+
                current.instrument.getRequestedSlitAngle() +","+   
                current.sky.getAIRMASS_MAX()+","+                 
                current.etc.getWRANGE_LOW() +","+    
                current.etc.getWRANGE_HIGH() +","+     
                current.etc.getChannel()+","+       
                current.etc.getMagnitude() +","+      
                current.etc.getMagref_system()+","+       
                current.etc.getMagref_filter()+","+       
                current.etc.getSrcmodel() +","+       
                current.otm.getOTMexpt()+","+  
                current.otm.getOTMslitwidth()+","+  
                current.otm.getOTMcass()+","+  
                current.otm.getOTMAirmass_start()+","+ 
                current.otm.getOTMAirmass_end() +","+ 
                current.otm.getSkymag()+","+ 
                current.otm.getOTMdead()+","+ 
                current.otm.getOTMslewgo()+","+  
                current.otm.getOTMstart()+","+  
                current.otm.getOTMend()+","+  
                current.otm.getOTMpa()+","+  
                current.otm.getOTMwait()+","+  
                current.otm.getOTMflag()+","+  
                current.otm.getOTMlast()+","+  
                current.otm.getOTMslew()+","+          
                current.otm.getOTMmoon()+","+
                current.otm.getOTMSNR()+","+
                current.otm.getOTMres()+","+
                current.otm.getOTMseeing()+","+                   
                current.otm.getOTMslitangle()+","+
                current.getNOTE()+","+
                current.getCOMMENT()+","+    
                //current.getOWNER()+","+ 
                //current.otm.getOTMAirmass_start()+","+ 
                current.otm.getOTMpointmode();  // Should not be from OTM
            output.println(current_record);
         }  
       output.flush();
       output.close();
    }catch(Exception e){
        System.out.println(e.toString());
    }
    logMessage(CommandLogModel.COMMAND, "Exporting table complete. ");
}
/*================================================================================================
/      constructQueryTargetPreparedStatement()
/=================================================================================================*/
public void constructQueryTargetPreparedStatement(){
   String query = String.format("SELECT * FROM %1$s WHERE %1$s.NAME = ? AND %1$s.OWNER = ?",TARGET_TABLE);
   try{
       QUERY_TARGET_PREP_STATEMENT = conn.prepareStatement(query);
    }catch(Exception e){
        logMessage(ERROR,"Creating query targets prepared statement. "+e.toString());
    }
}
/*================================================================================================
/      executeDeleteTargetsStatement(Target current)
/=================================================================================================*/
public int executeQueryTargetPreparedStatement(java.lang.String target_name){
   java.sql.ResultSet rs    = null; 
   myTargetQueryTableModel.clearTable();
   int result_count = 0;
   setProcessingState(RUNNING);
   try{
        QUERY_TARGET_PREP_STATEMENT.clearParameters();        
        QUERY_TARGET_PREP_STATEMENT.setString(1,target_name);
        QUERY_TARGET_PREP_STATEMENT.setString(2,getOWNER());
        System.out.println(QUERY_TARGET_PREP_STATEMENT.toString());
        // now that we have all the parameters set execute the update on the DBMS
        rs =  QUERY_TARGET_PREP_STATEMENT.executeQuery();
        while(rs.next()){
           Target current = transformResultSetToObservation(rs);
           myTargetQueryTableModel.addRow(current);
           result_count = result_count+1;
           System.out.println(current.name);
        }     
        myTargetQueryTableModel.fireTableDataChanged();
    }catch(Exception e){
       logMessage(ERROR,"Error in method QUERY_TARGET_PREP_STATEMENT "+e.toString());
    } 
   setProcessingState(IDLE);
   return result_count;
} 
/*================================================================================================
/      constructDeleteTargetPreparedStatement()
/=================================================================================================*/
public void constructDeleteTargetPreparedStatement(){
    String query = String.format("DELETE FROM %1$s WHERE %1$s.OBSERVATION_ID = ?", TARGET_TABLE);
    try{
       DELETE_TARGET_PREP_STATEMENT = conn.prepareStatement(query);
    }catch(Exception e){
        logMessage(ERROR,"Creating delete prepared statement. "+e.toString());
    }
}
/*================================================================================================
/      executeDeleteTargetsStatement(Target current)
/=================================================================================================*/
public void executeDeleteTargetsStatement(Target current){
    try{
        DELETE_TARGET_PREP_STATEMENT.clearParameters();        
        DELETE_TARGET_PREP_STATEMENT.setInt(1,current.getObservationID());
        System.out.println(DELETE_TARGET_PREP_STATEMENT.toString());
        // now that we have all the parameters set execute the update on the DBMS
        DELETE_TARGET_PREP_STATEMENT.executeUpdate();
    }catch(Exception e){
       logMessage(ERROR,"Error in method DELETE_TARGET_PREP_STATEMENT "+e.toString());
    }  
} 
/*================================================================================================
/       constructObservationSetPreparedStatement()
/=================================================================================================*/
public void constructTargetsPreparedStatement(){   
    String query = String.format("INSERT INTO %1$s (", TARGET_TABLE)
    + " SET_ID,"
    + " STATE,"
    + " OBS_ORDER,"
    + " TARGET_NUMBER,"
    + " SEQUENCE_NUMBER,"            
    + " NAME,"
    + " RA,"
    + " DECL," 
    + " OFFSET_RA,"        
    + " OFFSET_DEC,"        
    + " EXPTIME,"                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   
    + " SLITWIDTH,"
    + " SLITOFFSET,"
    + " OBSMODE,"
    + " BINSPECT,"
    + " BINSPAT,"
    + " SLITANGLE,"            
    + " AIRMASS_MAX,"
    + " WRANGE_LOW,"            
    + " WRANGE_HIGH,"            
    + " CHANNEL,"  
    + " MAGNITUDE,"            
    + " MAGSYSTEM,"            
    + " MAGFILTER,"            
    + " SRCMODEL,"
    + " OTMexpt,"
    + " OTMslitwidth,"
    + " OTMcass,"
    + " OTMairmass_start,"
    + " OTMairmass_end,"
    + " OTMsky,"
    + " OTMdead,"
    + " OTMslewgo,"
    + " OTMexp_start,"
    + " OTMexp_end,"
    + " OTMpa,"
    + " OTMwait,"
    + " OTMflag,"
    + " OTMlast,"
    + " OTMslew,"           
    + " OTMmoon,"   
    +  "OTMSNR,"       
    +  "OTMres,"       
    +  "OTMseeing,"       
    + " OTMslitangle,"   
    + " NOTE,"
    + " COMMENT,"
    + " OWNER,"
    + " NOTBEFORE,"
    + " POINTMODE"
    + " ) VALUES ("
    +  " ?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)";
    try{
       INSERT_OBSERVATION_PREP_STATEMENT = conn.prepareStatement(query);
    }catch(Exception e){
        logMessage(ERROR,"Creating prepared statement. "+e.toString());
    }
} 
/*================================================================================================
/       executeObservationSetInsertStatement(ObservationSet current)
/=================================================================================================*/
public void executeTargetsInsertStatement(Target current){
    try{
        INSERT_OBSERVATION_PREP_STATEMENT.clearParameters();        
        INSERT_OBSERVATION_PREP_STATEMENT.setInt(1,current.getSET_ID());
        INSERT_OBSERVATION_PREP_STATEMENT.setString(2,current.getSTATE());
        INSERT_OBSERVATION_PREP_STATEMENT.setInt(3,current.getOrder());
        INSERT_OBSERVATION_PREP_STATEMENT.setInt(4,current.getTarget_Number());
        INSERT_OBSERVATION_PREP_STATEMENT.setInt(5,current.getSequence_Number());        
        INSERT_OBSERVATION_PREP_STATEMENT.setString(6,current.getName());
        INSERT_OBSERVATION_PREP_STATEMENT.setString(7,current.sky.getRightAscension());
        INSERT_OBSERVATION_PREP_STATEMENT.setString(8,current.sky.getDeclination());
        INSERT_OBSERVATION_PREP_STATEMENT.setDouble(9,current.sky.getOFFSET_RA());
        INSERT_OBSERVATION_PREP_STATEMENT.setDouble(10,current.sky.getOFFSET_DEC());        
        INSERT_OBSERVATION_PREP_STATEMENT.setString(11,current.instrument.getExposuretime());
        INSERT_OBSERVATION_PREP_STATEMENT.setString(12,current.instrument.getSlitwidth_string());
        INSERT_OBSERVATION_PREP_STATEMENT.setDouble(13,current.instrument.getSlitOffset());
        INSERT_OBSERVATION_PREP_STATEMENT.setString(14,current.instrument.getOBSMODE());
        INSERT_OBSERVATION_PREP_STATEMENT.setInt(15,current.instrument.getBIN_SPEC());
        INSERT_OBSERVATION_PREP_STATEMENT.setInt(16,current.instrument.getBIN_SPACE());
        INSERT_OBSERVATION_PREP_STATEMENT.setString(17,current.instrument.getRequestedSlitAngle());    
        INSERT_OBSERVATION_PREP_STATEMENT.setString(18,current.sky.getAIRMASS_MAX());       
        INSERT_OBSERVATION_PREP_STATEMENT.setInt(19,current.etc.getWRANGE_LOW());       
        INSERT_OBSERVATION_PREP_STATEMENT.setInt(20,current.etc.getWRANGE_HIGH());       
        INSERT_OBSERVATION_PREP_STATEMENT.setString(21,current.etc.getChannel());       
        INSERT_OBSERVATION_PREP_STATEMENT.setDouble(22,current.etc.getMagnitude());       
        INSERT_OBSERVATION_PREP_STATEMENT.setString(23,current.etc.getMagref_system());       
        INSERT_OBSERVATION_PREP_STATEMENT.setString(24,current.etc.getMagref_filter());       
        INSERT_OBSERVATION_PREP_STATEMENT.setString(25,current.etc.getSrcmodel());         
        INSERT_OBSERVATION_PREP_STATEMENT.setDouble(26,current.otm.getOTMexpt());  
        INSERT_OBSERVATION_PREP_STATEMENT.setDouble(27,current.otm.getOTMslitwidth());  
        INSERT_OBSERVATION_PREP_STATEMENT.setDouble(28,current.otm.getOTMcass());  
        INSERT_OBSERVATION_PREP_STATEMENT.setDouble(29,current.otm.getOTMAirmass_start());  
        INSERT_OBSERVATION_PREP_STATEMENT.setDouble(30,current.otm.getOTMAirmass_end());  
        INSERT_OBSERVATION_PREP_STATEMENT.setDouble(31,current.otm.getSkymag());  
        INSERT_OBSERVATION_PREP_STATEMENT.setDouble(32,current.otm.getOTMdead());  
        INSERT_OBSERVATION_PREP_STATEMENT.setTimestamp(33,current.otm.getOTMslewgo());  
        INSERT_OBSERVATION_PREP_STATEMENT.setTimestamp(34,current.otm.getOTMstart());  
        INSERT_OBSERVATION_PREP_STATEMENT.setTimestamp(35,current.otm.getOTMend());  
        INSERT_OBSERVATION_PREP_STATEMENT.setDouble(36,current.otm.getOTMpa());  
        INSERT_OBSERVATION_PREP_STATEMENT.setDouble(37,current.otm.getOTMwait());  
        INSERT_OBSERVATION_PREP_STATEMENT.setString(38,current.otm.getOTMflag());  
        INSERT_OBSERVATION_PREP_STATEMENT.setString(39,current.otm.getOTMlast());  
        INSERT_OBSERVATION_PREP_STATEMENT.setDouble(40,current.otm.getOTMslew());          
        INSERT_OBSERVATION_PREP_STATEMENT.setString(41,current.otm.getOTMmoon());  
        INSERT_OBSERVATION_PREP_STATEMENT.setString(42,current.otm.getOTMSNR());  
        INSERT_OBSERVATION_PREP_STATEMENT.setDouble(43,current.otm.getOTMres());   
        INSERT_OBSERVATION_PREP_STATEMENT.setDouble(44,current.otm.getOTMseeing());   
        INSERT_OBSERVATION_PREP_STATEMENT.setDouble(45,current.otm.getOTMslitangle());                          
        INSERT_OBSERVATION_PREP_STATEMENT.setString(46,current.getNOTE());  
        INSERT_OBSERVATION_PREP_STATEMENT.setString(47,current.getCOMMENT());  
        INSERT_OBSERVATION_PREP_STATEMENT.setString(48,current.getOWNER());  
        INSERT_OBSERVATION_PREP_STATEMENT.setTimestamp(49,current.otm.getOTMnotbefore()); 
        INSERT_OBSERVATION_PREP_STATEMENT.setString(50,current.otm.getOTMpointmode());  
        System.out.println(INSERT_OBSERVATION_PREP_STATEMENT.toString());
        // now that we have all the parameters set execute the update on the DBMS
        INSERT_OBSERVATION_PREP_STATEMENT.executeUpdate();
    }catch(Exception e){
       logMessage(ERROR,"Error in method executeInsertStatement "+e.toString());
    }  
} 
/*================================================================================================
/       constructObservationSetPreparedStatement()
/=================================================================================================*/
public void constructUpdateTargetsPreparedStatement(){   
    String query = "UPDATE "+TARGET_TABLE
    + " SET "
    + " SET_ID = ?,"
    + " STATE = ?,"
    + " OBS_ORDER = ?,"
    + " TARGET_NUMBER = ?,"
    + " SEQUENCE_NUMBER = ?,"            
    + " NAME = ?,"
    + " RA = ?,"
    + " DECL = ?," 
    + " OFFSET_RA = ?,"        
    + " OFFSET_DEC = ?,"        
    + " EXPTIME = ?,"                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   
    + " SLITWIDTH = ?,"
    + " SLITOFFSET = ?,"
    + " OBSMODE = ?,"
    + " BINSPECT = ?,"
    + " BINSPAT = ?,"
    + " SLITANGLE = ?,"  
    + " AIRMASS_MAX = ?,"              
    + " WRANGE_LOW = ?,"            
    + " WRANGE_HIGH = ?,"            
    + " CHANNEL = ?,"            
    + " MAGNITUDE = ?,"            
    + " MAGSYSTEM = ?,"            
    + " MAGFILTER = ?,"            
    + " SRCMODEL = ?,"
    + " OTMexpt = ?,"
    + " OTMslitwidth = ?,"
    + " OTMcass = ?,"
    + " OTMairmass_start = ?,"
    + " OTMairmass_end = ?,"
    + " OTMsky = ?,"
    + " OTMdead = ?,"  
    + " OTMslewgo = ?,"
    + " OTMexp_start = ?,"
    + " OTMexp_end = ?,"
    + " OTMpa = ?,"
    + " OTMwait = ?,"
    + " OTMflag = ?,"
    + " OTMlast = ?,"
    + " OTMslew = ?,"           
    + " OTMmoon = ?,"  
    + " OTMSNR = ?,"  
    + " OTMres = ?,"  
    + " OTMseeing = ?,"            
    + " OTMslitangle = ?,"    
    + " NOTE = ?,"    
    + " COMMENT = ?,"               
    + " OWNER = ?,"  
    + " NOTBEFORE = ?,"        
    + " POINTMODE = ?"        
    + " WHERE OBSERVATION_ID = ? ";
    try{
       UPDATE_OBSERVATION_PREP_STATEMENT = conn.prepareStatement(query);
    }catch(Exception e){
        logMessage(ERROR,"Creating prepared statement. "+e.toString());
    }
} 
/*================================================================================================
/      constructQueryState()
/=================================================================================================*/
public void constructQueryState(){
    String query = String.format("SELECT %1$s.OBSERVATION_ID, %1$s.STATE FROM %1$s WHERE %1$s.SET_ID = ?; ",TARGET_TABLE);
    try{
       QUERY_STATE_PREP_STATEMENT = conn.prepareStatement(query);
    }catch(Exception e){
        logMessage(ERROR,"Creating prepared statement. "+e.toString());
    }    
}
/*================================================================================================
/      queryState(ObservationSet current_set,MasterDBTableModel current_table)
/=================================================================================================*/
public void executeQueryState(ObservationSet current_set,MasterDBTableModel current_table){
   executeQueryStateThread myexecuteQueryStateThread = new executeQueryStateThread(current_set,current_table);
   myexecuteQueryStateThread.start();
}
/*================================================================================================
/      queryState(ObservationSet current_set,MasterDBTableModel current_table)
/=================================================================================================*/
public void queryState(ObservationSet current_set,MasterDBTableModel current_table){
   java.sql.ResultSet rs    = null; 
   setProcessingState(RUNNING);
   int obs_set_ID =  current_set.getSET_ID();
    try{
       QUERY_STATE_PREP_STATEMENT.clearParameters();        
       QUERY_STATE_PREP_STATEMENT.setInt(1,obs_set_ID);
       System.out.println(QUERY_STATE_PREP_STATEMENT.toString());         
       rs  = QUERY_STATE_PREP_STATEMENT.executeQuery();  
       int count = current_table.getRowCount();
       while(rs.next()){
         for(int i=0;i< count;i++){
           Target current = current_table.getRecord(i);
           int obs_ID = rs.getInt("OBSERVATION_ID");
           java.lang.String current_state = rs.getString("STATE");
           if(current.getObservationID() == obs_ID){
               current.setSTATE(current_state);
           }
         }
       }
       current_table.fireTableDataChanged();
       if(currentCombinedChartTest != null){
             currentCombinedChartTest.updateChartColors();
        }
     }catch(Exception e){
       logMessage(ERROR,"Error in method executeTargetUpdteStatement "+e.toString());
    }  
//   this.notifyAll();
   setProcessingState(IDLE);
}
/*================================================================================================
/       executeObservationSetInsertStatement(ObservationSet current)
/=================================================================================================*/
public void setSelectedObservationSet(ObservationSet newObservationSet){
   selectedObservationSet = newObservationSet;
   setSelectedSetName(selectedObservationSet.getSET_NAME());
}
/*================================================================================================
/       executeObservationSetInsertStatement(ObservationSet current)
/=================================================================================================*/
public void createNewTargetList(){
   Target current = new Target();
   myTargetDBMSTableModel.clearTable();
   myTargetDBMSTableModel.addTarget(current);
   myTargetDBMSTableModel.fireTableDataChanged();
   selectedObservationSet = null;
   setTablePopulated(true);
}
/*================================================================================================
/       executeUpdateTargetTable(ObservationSet current_set,MasterDBTableModel current_table)
/=================================================================================================*/
public void executeUpdateTargetTable(ObservationSet current_set,MasterDBTableModel current_table){
    executeSave myexecuteSave = new executeSave(current_set,current_table);
    myexecuteSave.start();    
}
/*================================================================================================
/       executeObservationSetInsertStatement(ObservationSet current)
/=================================================================================================*/
public void updateTargetTable(ObservationSet current_set,MasterDBTableModel current_table){
//    ArrayList new_targets = new ArrayList(); 
    setProcessingState(RUNNING);
//    double seeing     = 1.25;
//    int    wavelength = 500;
//    java.lang.String start_time_string = "2022-01-01T03:00:00.000";
    double seeing     = myOTMlauncher.getSeeing();
    int    wavelength = myOTMlauncher.getWavelength();
//       java.lang.String start_time_string = "2022-01-01T03:00:00.000";
//       java.sql.Timestamp current_timestamp_otm = myOTMlauncher.string_to_timestamp(start_time_string);
       java.sql.Timestamp current_timestamp_otm = myOTMlauncher.getStartTimestamp();           
//    java.sql.Timestamp current_timestamp_otm = myOTMlauncher.string_to_timestamp(start_time_string);
    myOTMlauncher.OTM(current_timestamp_otm,seeing,wavelength);
    int set_ID =0;
    int count = current_table.getRowCount();
    for(int i=0;i<count;i++){
      Target current = current_table.getRecord(i);
      if(current.getSET_ID() != 0){
         set_ID =  current.getSET_ID();
      }
      int observation_id = current.getObservationID();
      if(observation_id == Target.MISSING_ID){
          current.setSET_ID(set_ID);
          executeTargetsInsertStatement(current);
      }     
      if(observation_id != Target.MISSING_ID){
          executeTargetsUpdateStatement(current);
      }  
    } 
    while(!delete_stack.empty()){
        Target current_target = (Target)delete_stack.pop();
        executeDeleteTargetsStatement(current_target);
    }
    Timestamp current_timestamp = current_set.constructTimestamp();
    current_set.setSET_ID(set_ID);
    current_set.setNUM_OBSERVATIONS(count);
    current_set.setLastUpdate_Timestamp(current_timestamp);
    executeUpdateObservationSet(current_set);
    queryObservations(current_set.getSET_ID());
    myTargetDBMSTableModel.setEdited(false);
    setProcessingState(IDLE);
}
/*================================================================================================
/       executeObservationSetInsertStatement(ObservationSet current)
/=================================================================================================*/
public void executeTargetsUpdateStatement(Target current){
    try{
        UPDATE_OBSERVATION_PREP_STATEMENT.clearParameters();        
        UPDATE_OBSERVATION_PREP_STATEMENT.setInt(1,current.getSET_ID());
        UPDATE_OBSERVATION_PREP_STATEMENT.setString(2,current.getSTATE());
        UPDATE_OBSERVATION_PREP_STATEMENT.setInt(3,current.getOrder());
        UPDATE_OBSERVATION_PREP_STATEMENT.setInt(4,current.getTarget_Number());
        UPDATE_OBSERVATION_PREP_STATEMENT.setInt(5,current.getSequence_Number());        
        UPDATE_OBSERVATION_PREP_STATEMENT.setString(6,current.getName());
        UPDATE_OBSERVATION_PREP_STATEMENT.setString(7,current.sky.getRightAscension());
        UPDATE_OBSERVATION_PREP_STATEMENT.setString(8,current.sky.getDeclination());
        UPDATE_OBSERVATION_PREP_STATEMENT.setDouble(9,current.sky.getOFFSET_RA());
        UPDATE_OBSERVATION_PREP_STATEMENT.setDouble(10,current.sky.getOFFSET_DEC());        
        UPDATE_OBSERVATION_PREP_STATEMENT.setString(11,current.instrument.getExposuretime());
        UPDATE_OBSERVATION_PREP_STATEMENT.setString(12,current.instrument.getSlitwidth_string());
        UPDATE_OBSERVATION_PREP_STATEMENT.setDouble(13,current.instrument.getSlitOffset());
        UPDATE_OBSERVATION_PREP_STATEMENT.setString(14,current.instrument.getOBSMODE());
        UPDATE_OBSERVATION_PREP_STATEMENT.setInt(15,current.instrument.getBIN_SPEC());
        UPDATE_OBSERVATION_PREP_STATEMENT.setInt(16,current.instrument.getBIN_SPACE());
        UPDATE_OBSERVATION_PREP_STATEMENT.setString(17,current.instrument.getRequestedSlitAngle()); 
        UPDATE_OBSERVATION_PREP_STATEMENT.setString(18,current.sky.getAIRMASS_MAX());         
        UPDATE_OBSERVATION_PREP_STATEMENT.setInt(19,current.etc.getWRANGE_LOW());       
        UPDATE_OBSERVATION_PREP_STATEMENT.setInt(20,current.etc.getWRANGE_HIGH());       
        UPDATE_OBSERVATION_PREP_STATEMENT.setString(21,current.etc.getChannel());       
        UPDATE_OBSERVATION_PREP_STATEMENT.setDouble(22,current.etc.getMagnitude());       
        UPDATE_OBSERVATION_PREP_STATEMENT.setString(23,current.etc.getMagref_system());       
        UPDATE_OBSERVATION_PREP_STATEMENT.setString(24,current.etc.getMagref_filter());       
        UPDATE_OBSERVATION_PREP_STATEMENT.setString(25,current.etc.getSrcmodel());         
        UPDATE_OBSERVATION_PREP_STATEMENT.setDouble(26,current.otm.getOTMexpt());  
        UPDATE_OBSERVATION_PREP_STATEMENT.setDouble(27,current.otm.getOTMslitwidth());  
        UPDATE_OBSERVATION_PREP_STATEMENT.setDouble(28,current.otm.getOTMcass());  
        UPDATE_OBSERVATION_PREP_STATEMENT.setDouble(29,current.otm.getOTMAirmass_start());  
        UPDATE_OBSERVATION_PREP_STATEMENT.setDouble(30,current.otm.getOTMAirmass_end());  
        UPDATE_OBSERVATION_PREP_STATEMENT.setDouble(31,current.otm.getSkymag());  
        UPDATE_OBSERVATION_PREP_STATEMENT.setDouble(32,current.otm.getOTMdead());  
        UPDATE_OBSERVATION_PREP_STATEMENT.setTimestamp(33,current.otm.getOTMslewgo());  
        UPDATE_OBSERVATION_PREP_STATEMENT.setTimestamp(34,current.otm.getOTMstart());  
        UPDATE_OBSERVATION_PREP_STATEMENT.setTimestamp(35,current.otm.getOTMend());  
        UPDATE_OBSERVATION_PREP_STATEMENT.setDouble(36,current.otm.getOTMpa());  
        UPDATE_OBSERVATION_PREP_STATEMENT.setDouble(37,current.otm.getOTMwait());  
        UPDATE_OBSERVATION_PREP_STATEMENT.setString(38,current.otm.getOTMflag());  
        UPDATE_OBSERVATION_PREP_STATEMENT.setString(39,current.otm.getOTMlast());  
        UPDATE_OBSERVATION_PREP_STATEMENT.setDouble(40,current.otm.getOTMslew());          
        UPDATE_OBSERVATION_PREP_STATEMENT.setString(41,current.otm.getOTMmoon());          
        UPDATE_OBSERVATION_PREP_STATEMENT.setString(42,current.otm.getOTMSNR());  
        UPDATE_OBSERVATION_PREP_STATEMENT.setDouble(43,current.otm.getOTMres());  
        UPDATE_OBSERVATION_PREP_STATEMENT.setDouble(44,current.otm.getOTMseeing()); 
        UPDATE_OBSERVATION_PREP_STATEMENT.setDouble(45,current.otm.getOTMslitangle());          
        UPDATE_OBSERVATION_PREP_STATEMENT.setString(46,current.getNOTE());  
        UPDATE_OBSERVATION_PREP_STATEMENT.setString(47,current.getCOMMENT());  
        UPDATE_OBSERVATION_PREP_STATEMENT.setString(48,current.getOWNER());  
        UPDATE_OBSERVATION_PREP_STATEMENT.setTimestamp(49,current.otm.getOTMnotbefore());  
        UPDATE_OBSERVATION_PREP_STATEMENT.setString(50,current.otm.getOTMpointmode());         
        UPDATE_OBSERVATION_PREP_STATEMENT.setInt(51,current.getObservationID());
        System.out.println(UPDATE_OBSERVATION_PREP_STATEMENT.toString());
        // now that we have all the parameters set execute the update on the DBMS
        UPDATE_OBSERVATION_PREP_STATEMENT.executeUpdate();
    }catch(Exception e){
       logMessage(ERROR,"Error in method executeTargetUpdteStatement "+e.toString());
    }  
} 
/*================================================================================================
/      int queryLastInsertID()
/=================================================================================================*/
public int queryLastInsertID(){
    int autoIncKeyFromFunc = -1;
    try{
       Statement st = conn.createStatement();  
       boolean state = st.execute("SET @@SESSION.information_schema_stats_expiry=0");  
       ResultSet rs = st.executeQuery("SELECT Auto_increment FROM information_schema.TABLES WHERE TABLE_SCHEMA = 'ngps' AND TABLE_NAME = 'target_sets'");
       if (rs.next()) {
        autoIncKeyFromFunc = rs.getInt(1);
       } else {
        // throw an exception from here
       }
       autoIncKeyFromFunc = autoIncKeyFromFunc-1;
       System.out.println("Last Autoincrement = "+autoIncKeyFromFunc);
    }catch(Exception e){
        System.out.println(e.toString());
    }
  return autoIncKeyFromFunc;
} 
/*================================================================================================
/       constructObservationSetPreparedStatement()
/=================================================================================================*/
public void constructObservationSetPreparedStatement(){
    String query = String.format("INSERT INTO %1$s (",TARGETSET_TABLE)
    + " SET_NAME,"
    + " OWNER,"
    + " STATE,"
    + " NUM_OBSERVATIONS,"        
    + " SET_CREATION_TIMESTAMP,"        
    + " LAST_UPDATE_TIMESTAMP"
    + " ) VALUES ("
    +  " ?,?,?,?,?,?)";
    try{
       INSERT_OBSERVATION_SET_PREP_STATEMENT = conn.prepareStatement(query);
    }catch(Exception e){
        logMessage(ERROR,"Creating prepared statement. "+e.toString());
    }
}
/*================================================================================================
/       constructObservationSetPreparedStatement()
/=================================================================================================*/
public void constructUpdateObservationSetPreparedStatement(){   
    String query = String.format("UPDATE %1$s ",TARGETSET_TABLE)
    + "SET "
    + " NUM_OBSERVATIONS = ?, "
    + " LAST_UPDATE_TIMESTAMP = ?"
    + " WHERE SET_ID = ? ";
    try{
       UPDATE_OBSERVATION_SET_PREP_STATEMENT = conn.prepareStatement(query);
    }catch(Exception e){
        logMessage(ERROR,"Creating prepared statement. "+e.toString());
    }    
} 
/*================================================================================================
/       executeObservationSetInsertStatement(ObservationSet current)
/=================================================================================================*/
public void executeUpdateObservationSet(ObservationSet current){
    try{
        UPDATE_OBSERVATION_SET_PREP_STATEMENT.clearParameters();
        UPDATE_OBSERVATION_SET_PREP_STATEMENT.setInt(1,current.getNUM_OBSERVATIONS());
        UPDATE_OBSERVATION_SET_PREP_STATEMENT.setTimestamp(2,current.getLastUpdate_Timestamp());
        UPDATE_OBSERVATION_SET_PREP_STATEMENT.setInt(3,current.getSET_ID());
        UPDATE_OBSERVATION_SET_PREP_STATEMENT.executeUpdate();
    }catch(Exception e){
        logMessage(ERROR,"Execution the ObservationSet Update Statement" + e.toString());
    }
}
/*================================================================================================
/       executeObservationSetInsertStatement(ObservationSet current)
/=================================================================================================*/
public int executeObservationSetInsertStatement(ObservationSet current){
    int set_id = -1;
    try{
        INSERT_OBSERVATION_SET_PREP_STATEMENT.clearParameters();        
        INSERT_OBSERVATION_SET_PREP_STATEMENT.setString(1,current.getSET_NAME());
        INSERT_OBSERVATION_SET_PREP_STATEMENT.setString(2,current.getOWNER());
        INSERT_OBSERVATION_SET_PREP_STATEMENT.setString(3,current.getSTATE());
        INSERT_OBSERVATION_SET_PREP_STATEMENT.setInt(4,current.getNUM_OBSERVATIONS());
        INSERT_OBSERVATION_SET_PREP_STATEMENT.setTimestamp(5,current.getCreation_Timestamp());
        INSERT_OBSERVATION_SET_PREP_STATEMENT.setTimestamp(6,current.getLastUpdate_Timestamp());        
        System.out.println(INSERT_OBSERVATION_SET_PREP_STATEMENT.toString());
        // now that we have all the parameters set execute the update on the DBMS
        INSERT_OBSERVATION_SET_PREP_STATEMENT.executeUpdate();
        set_id = queryLastInsertID();
    }catch(Exception e){
       logMessage(ERROR,"Error in method executeInsertStatement "+e.toString());
    } 
  return set_id;
}
/*================================================================================================
/       constructObservationSetPreparedStatement()
/=================================================================================================*/
public void constructOwnerPreparedStatement(){
    String query = String.format("INSERT INTO %1$s (",ACCOUNT_TABLE)
    + " OWNER_ID,"
    + " PASSWORD,"
    + " EMAIL"
    + " ) VALUES ("
    +  " ?,?,?)";
    try{
       INSERT_OWNER_PREP_STATEMENT = conn.prepareStatement(query);
    }catch(Exception e){
        logMessage(ERROR,"Creating prepared statement. "+e.toString());
    }
}
/*================================================================================================
/      constructUpdateOwnerPreparedStatement()
/=================================================================================================*/
public void constructUpdateOwnerPreparedStatement(){
     String query = String.format("UPDATE %1$s SET %1$s.PASSWORD = ? WHERE %1$s.OWNER_ID = ? ",ACCOUNT_TABLE);
    try{
       UPDATE_OWNER_PREP_STATEMENT = conn.prepareStatement(query);
    }catch(Exception e){
        logMessage(ERROR,"Creating update owner prepared statement. "+e.toString());
    }    
}
/*================================================================================================
/     executeUpdateOwnerPreparedStatement(Owner current)
/=================================================================================================*/
public void executeUpdateOwnerPreparedStatement(String username, String encrypted_password){
    try{
        UPDATE_OWNER_PREP_STATEMENT.clearParameters();
        UPDATE_OWNER_PREP_STATEMENT.setString(1,encrypted_password);
        UPDATE_OWNER_PREP_STATEMENT.setString(2,username);
        System.out.println(UPDATE_OWNER_PREP_STATEMENT.toString());
        UPDATE_OWNER_PREP_STATEMENT.executeUpdate();
   }catch(Exception e){
       logMessage(ERROR,"Error in method executeUpdateOwnerPreparedStatement "+e.toString());        
    }
}
/*================================================================================================
/       executeObservationSetInsertStatement(ObservationSet current)
/=================================================================================================*/
public int executeOwnerInsertStatement(Owner current){
    int set_id = -1;
    try{
        INSERT_OWNER_PREP_STATEMENT.clearParameters();        
        INSERT_OWNER_PREP_STATEMENT.setString(1,current.getOwner_ID());
        INSERT_OWNER_PREP_STATEMENT.setString(2,current.getEncryptedPassword());
        INSERT_OWNER_PREP_STATEMENT.setString(3,current.getEmail());
        System.out.println(INSERT_OWNER_PREP_STATEMENT.toString());
        // now that we have all the parameters set execute the update on the DBMS
        INSERT_OWNER_PREP_STATEMENT.executeUpdate();
    }catch(Exception e){
       logMessage(ERROR,"Error in method executeObservationSetInsertStatement "+e.toString());
    } 
  return set_id; 
}
/*================================================================================================
/       constructPreparedStatement()
/=================================================================================================*/
public Owner transformResultSetToOwner(java.sql.ResultSet results){
     Owner current = new Owner();
      try{  
          current.setOwner_ID(results.getString("OWNER_ID"));
          current.setEncryptedPassword(results.getString("PASSWORD"));
          current.setEmail(results.getString("EMAIL"));
       }catch(Exception e){
         logMessage(ERROR,"Error in method transformResultSetToOwner "+e.toString());
      }
  return current;
}
/*================================================================================================
/       constructPreparedStatement()
/=================================================================================================*/
public OwnerTableModel queryOwners(){
    java.sql.ResultSet rs    = null; 
    myOwnerTableModel.clearTable();
    ownerslist.clear();
    String query = String.format("SELECT * FROM %1$s;",ACCOUNT_TABLE); 
     try{
        java.sql.PreparedStatement st       = conn.prepareStatement(query);    
                                  rs       = st.executeQuery();  
       java.sql.ResultSetMetaData metaData = rs.getMetaData();
       while(rs.next()){
           Owner current = transformResultSetToOwner(rs);
           myOwnerTableModel.addRecord(current);
           ownerslist.addElement(current.getOwner_ID());
       }       
     }catch(Exception e){
       logMessage(ERROR,"Error in method queryTimestamp "+e.toString());
     }
  return myOwnerTableModel;
}

public boolean userExists(String username){
    for(int i=0;i<myOwnerTableModel.getRowCount();i++){
       Owner current = (Owner)myOwnerTableModel.getRecord(i);
       if(username.matches(current.getOwner_ID())){ return true; }
    }
    return false;
}

// DIRECT MYSQL WAY for userExists():
//        statement = dbms.conn.createStatement();
//        
//        rs = statement.executeQuery("SELECT COUNT(*) from ngps.owner WHERE owner_id = "+"'"+owner_id+"'"); //HARDCODE
//        rs.next();
//        int userExists = rs.getInt(1); // value on row 1 
//        rs.close();
//        if(userExists>0){
//            JOptionPane.showMessageDialog(null, "Username already exists: "+owner_id);
//            return;
//        }        

public boolean verifyLogin(String username, String password){
    
    String encrypted_password_stored = null;
    boolean matchFound = false;

    for(int i=0;i<myOwnerTableModel.getRowCount();i++){
       Owner current = (Owner)myOwnerTableModel.getRecord(i);
       if(username.matches(current.getOwner_ID())){ 
           matchFound = true;
           encrypted_password_stored = current.getEncryptedPassword();
           break;  
       }
    }
    if(!matchFound){
        JOptionPane.showMessageDialog(null, "Username not found: "+username,"ERROR", JOptionPane.ERROR_MESSAGE);
        return false;
    }
    
    if(encrypted_password_stored == null){
        return false;
    }
    // If the submitted and stored passwords match without decrypting, that's good enough
    if(password.matches(encrypted_password_stored)){
        return true;
    }
    try{          
        // If the submitted and stored passwords match without decrypting, that's good enough
        String encrypted_password = encrypt(password,originalKey);
        boolean compare = java.security.MessageDigest.isEqual(encrypted_password_stored.getBytes(),encrypted_password.getBytes()) ;
        if(!compare){
            JOptionPane.showMessageDialog(null, "Invalid password for "+username,"ERROR", JOptionPane.ERROR_MESSAGE);
            return false;
        }
        return compare;
    }
    catch(Exception e){
           System.out.println(e.toString());
           return false;
       }
    }

/*================================================================================================
/      transformResultSetToObservationSet(java.sql.ResultSet results)
/=================================================================================================*/
public Target transformResultSetToObservation(java.sql.ResultSet results){
     Target current = new Target();
      try{ 
          current.setObservationID(results.getInt("OBSERVATION_ID"));
          current.setSET_ID(results.getInt("SET_ID"));
          current.setSTATE(results.getString("STATE"));
          current.setOrder(results.getInt("OBS_ORDER"));
          current.setTarget_Number(results.getInt("TARGET_NUMBER"));
          current.setSequence_Number(results.getInt("SEQUENCE_NUMBER"));
          current.setName(results.getString("NAME"));
          current.sky.setRightAscension(results.getString("RA"));
          current.sky.setDeclination(results.getString("DECL"));
          current.instrument.setExposuretime(results.getString("EXPTIME"));
          current.instrument.setSlitwidth_string(results.getString("SLITWIDTH"));
          current.instrument.setSlitOffset(results.getDouble("SLITOFFSET"));
          current.instrument.setOBSMODE(results.getString("OBSMODE"));
          current.instrument.setBIN_SPEC(results.getInt("BINSPECT"));
          current.instrument.setBIN_SPACE(results.getInt("BINSPAT"));
          current.instrument.setRequestedSlitAngle(results.getString("SLITANGLE"));
          current.sky.setAIRMASS_MAX(results.getString("AIRMASS_MAX"));
          current.etc.setWRANGE_LOW(results.getInt("WRANGE_LOW"));
          current.etc.setWRANGE_HIGH(results.getInt("WRANGE_HIGH"));
          current.etc.setChannel(results.getString("CHANNEL"));
          current.etc.setMagnitude(results.getDouble("MAGNITUDE"));
          current.etc.setMagref_system(results.getString("MAGSYSTEM"));
          current.etc.setMagref_filter(results.getString("MAGFILTER"));
          current.etc.setSrcmodel(results.getString("SRCMODEL"));
          current.otm.setOTMexpt(results.getDouble("OTMexpt"));
          current.otm.setOTMslitwidth(results.getDouble("OTMslitwidth"));
          current.otm.setOTMcass(results.getDouble("OTMcass"));
          current.otm.setOTMAirmass_start(results.getDouble("OTMairmass_start"));
          current.otm.setOTMAirmass_end(results.getDouble("OTMairmass_end"));
          current.otm.setSkymag(results.getDouble("OTMsky"));
          current.otm.setOTMdead(results.getDouble("OTMdead"));
          current.otm.setOTMslewgo(results.getTimestamp("OTMslewgo"));      
          current.otm.setOTMstart(results.getTimestamp("OTMexp_start"));
          current.otm.setOTMend(results.getTimestamp("OTMexp_end"));
          current.otm.setOTMpa(results.getDouble("OTMpa"));
          current.otm.setOTMwait(results.getDouble("OTMwait"));
          current.otm.setOTMflag(results.getString("OTMflag"));
          current.otm.setOTMlast(results.getString("OTMlast"));
          current.otm.setOTMslew(results.getDouble("OTMslew"));
          current.otm.setOTMmoon(results.getString("OTMmoon"));
          current.otm.setOTMSNR(results.getString("OTMSNR"));
          current.otm.setOTMres(results.getDouble("OTMres"));       
          current.otm.setOTMseeing(results.getDouble("OTMseeing"));                
          current.otm.setOTMslitangle(results.getDouble("OTMslitangle"));
          current.setNOTE(results.getString("NOTE"));
          current.setCOMMENT(results.getString("COMMENT"));
          current.setOWNER(results.getString("OWNER"));
          current.otm.setOTMnotbefore(results.getTimestamp("NOTBEFORE"));
          current.otm.setOTMpointmode(results.getString("POINTMODE"));
       }catch(Exception e){
         logMessage(ERROR,"Error in method transformResultSetToObservation( "+e.toString());
      }
  return current;
}
/*================================================================================================
/       queryObservationSets(java.lang.String current_owner)
/=================================================================================================*/
public MasterDBTableModel queryObservations(int current_set_id){
    java.sql.ResultSet rs    = null; 
    java.util.HashMap myHashmap = new java.util.HashMap();
    myTargetDBMSTableModel.clearTable();
//    myTargetDBMSTableModel.fireTableDataChanged();
    java.lang.String   query = String.format("SELECT * FROM %1$s WHERE %1$s.SET_ID = ?;",TARGET_TABLE); 
     try{
        java.sql.PreparedStatement st  = conn.prepareStatement(query); 
                                   st.setInt(1,current_set_id);
                                   rs  = st.executeQuery();  
       java.sql.ResultSetMetaData metaData = rs.getMetaData();
       while(rs.next()){
           Target current = transformResultSetToObservation(rs);
           myHashmap.put(current.getOrder(),current);
       } 
       int num_targets = myHashmap.size();
       for(int i=0;i<num_targets;i++){
           if(myHashmap.containsKey(i)){
             Target current_target = (Target)myHashmap.get(i);
             current_target.setSelected(false);
             System.out.println("OBS_ORDER = "+current_target.getOrder()+" Name ="+current_target.getName());
             myTargetDBMSTableModel.addRow(current_target);                
           }
       }
       myTargetDBMSTableModel.fireTableDataChanged();
     }catch(Exception e){
       logMessage(ERROR,"Error in method queryTimestamp "+e.toString());
     }
  return myTargetDBMSTableModel;
}
/*================================================================================================
/      deleteObservationSet(int current_set_id)
/=================================================================================================*/
public void deleteObservationSet(int current_set_id){
   java.sql.ResultSet rs     = null; 
   java.sql.ResultSet rs2    = null; 
   java.lang.String   query  = String.format("DELETE FROM %1$s WHERE %1$s.SET_ID = ?",TARGET_TABLE); 
   java.lang.String   query2 = String.format("DELETE FROM %1$s WHERE %1$s.SET_ID = ?",TARGETSET_TABLE); 
     try{
        java.sql.PreparedStatement st  = conn.prepareStatement(query); 
                                   st.setInt(1,current_set_id);
                                   boolean state  = st.execute();
        java.sql.PreparedStatement st2  = conn.prepareStatement(query2); 
                                   st2.setInt(1,current_set_id);
                                   boolean state2  = st2.execute();                                   
     }catch(Exception e){
        System.out.println(e.toString());
     }     
}
/*================================================================================================
/      transformResultSetToObservationSet(java.sql.ResultSet results)
/=================================================================================================*/
public ObservationSet transformResultSetToObservationSet(java.sql.ResultSet results){
     ObservationSet current = new ObservationSet();
      try{  
          current.setSET_ID(results.getInt("SET_ID"));
          current.setSET_NAME(results.getString("SET_NAME"));
          current.setOWNER(results.getString("OWNER"));
          current.setSTATE(results.getString("STATE"));
          current.setNUM_OBSERVATIONS(results.getInt("NUM_OBSERVATIONS"));
          current.setCreation_Timestamp(results.getTimestamp("SET_CREATION_TIMESTAMP"));
          current.setLastUpdate_Timestamp(results.getTimestamp("LAST_UPDATE_TIMESTAMP"));
       }catch(Exception e){
         logMessage(ERROR,"Error in method transformResultSetToObservationSet "+e.toString());
      }
  return current;
}
/*================================================================================================
/       queryObservationSets(java.lang.String current_owner)
/=================================================================================================*/
public ObservationSetTableModel queryObservationSets(java.lang.String current_owner){
    java.sql.ResultSet rs    = null; 
    myObservationSetTableModel.clearTable();
    java.lang.String query = String.format("SELECT * FROM %1$s WHERE %1$s.owner = ?;",TARGETSET_TABLE); 
     try{
        java.sql.PreparedStatement st  = conn.prepareStatement(query); 
                                   st.setString(1,current_owner);
                                   rs  = st.executeQuery();  
       java.sql.ResultSetMetaData metaData = rs.getMetaData();
       while(rs.next()){
           ObservationSet current = transformResultSetToObservationSet(rs);
           myObservationSetTableModel.addRecord(current);
       }       
     }catch(Exception e){
       logMessage(ERROR,"Error in method queryTimestamp "+e.toString());
     }
  return myObservationSetTableModel;
}
/*=============================================================================================
/        logMessage(int code,java.lang.String message)
/=============================================================================================*/
public void logMessage(int code,java.lang.String message){
     publicCommandLogModel.insertMessage(CommandLogModel.COMMAND, message);
     switch(code){
         case 0:  ngps_dbms_Logger.debug(message);  break;
         case 1:  ngps_dbms_Logger.info(message);   break;
         case 2:  ngps_dbms_Logger.warn(message);   break;
         case 3:  ngps_dbms_Logger.error(message);  break;
         case 4:  ngps_dbms_Logger.fatal(message);  break;
     }
     if((code ==WARN)|(code == ERROR)|(code ==FATAL)){
         myCommandLogModel.insertMessage(CommandLogModel.ERROR, message);
     }
     System.out.println(message);
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
/       INNER CLASS eexecuteSaveAs 
/=============================================================================================*/
    public class executeSaveAs  implements Runnable{
    private Thread myThread; 
    private java.lang.String my_set_name;
/*=============================================================================================
/         executeParseFile()
/=============================================================================================*/
    private executeSaveAs(java.lang.String new_set_name){
       my_set_name = new_set_name;
    }
/*=============================================================================================
/           run()
/=============================================================================================*/
    public void run(){
       logMessage(CommandLogModel.COMMAND, "Updating database with edits. "+my_set_name);
       SaveAs(my_set_name);
       logMessage(CommandLogModel.COMMAND, "Updating database complete. ");
     return;
    }
/*=============================================================================================
/         start()
/=============================================================================================*/
    public void start(){
     myThread = new Thread(this);
     myThread.start();
     }
    }// End of theexecuteSaveAs  Inner Class
/*=============================================================================================
/                           End of the executeSaveAs 
/=============================================================================================*/ 
/*=============================================================================================
/       INNER CLASS eexecuteSaveAs 
/=============================================================================================*/
    public class executeSave  implements Runnable{
    private Thread myThread; 
    private ObservationSet current_set;
    private MasterDBTableModel current_table;
/*=============================================================================================
/         executeParseFile()
/=============================================================================================*/
    private executeSave(ObservationSet new_current_set,MasterDBTableModel new_current_table){
        current_set = new_current_set;
        current_table = new_current_table;
    }
/*=============================================================================================
/           run()
/=============================================================================================*/
    public void run(){
        logMessage(CommandLogModel.COMMAND, "Updating database with edits. "+current_set.getSET_NAME());
        updateTargetTable(current_set,current_table);
        myTargetDBMSTableModel.setEdited(false);
        logMessage(CommandLogModel.COMMAND, "Database has been updated. ");
     return;
    }
/*=============================================================================================
/         start()
/=============================================================================================*/
    public void start(){
     myThread = new Thread(this);
     myThread.start();
     }
    }// End of theexecuteSaveAs  Inner Class
/*=============================================================================================
/                           End of the executeSaveAs 
/=============================================================================================*/    
 /*=============================================================================================
/       INNER CLASS eexecuteSaveAs 
/=============================================================================================*/
    public class executeQueryStateThread  implements Runnable{
    private Thread myThread; 
    private ObservationSet current_set;
    private MasterDBTableModel current_table;
/*=============================================================================================
/         executeParseFile()
/=============================================================================================*/
    private executeQueryStateThread(ObservationSet new_current_set,MasterDBTableModel new_current_table){
        current_set = new_current_set;
        current_table = new_current_table;
    }
/*=============================================================================================
/           run()
/=============================================================================================*/
    public void run(){
        logMessage(CommandLogModel.COMMAND, "Querying the state "+current_set.getSET_NAME());
        queryState(current_set,current_table);
        logMessage(CommandLogModel.COMMAND, "Database has been queried. ");
     return;
    }
/*=============================================================================================
/         start()
/=============================================================================================*/
    public void start(){
     myThread = new Thread(this);
     myThread.start();
     }
    }// End of theexecuteSaveAs  Inner Class
/*=============================================================================================
/                           End of the executeSaveAs 
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
            java.util.logging.Logger.getLogger(NGPSdatabase.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (InstantiationException ex) {
            java.util.logging.Logger.getLogger(NGPSdatabase.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (IllegalAccessException ex) {
            java.util.logging.Logger.getLogger(NGPSdatabase.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (javax.swing.UnsupportedLookAndFeelException ex) {
            java.util.logging.Logger.getLogger(NGPSdatabase.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        }
        //</editor-fold>
             try {
                 UIManager.setLookAndFeel(UIManager.getCrossPlatformLookAndFeelClassName());
                 java.lang.String test = UIManager.getSystemLookAndFeelClassName();
                 System.out.println(test);
               } catch(Exception e) {
                   e.printStackTrace();
               }
               new NGPSdatabase();
    }  
}
