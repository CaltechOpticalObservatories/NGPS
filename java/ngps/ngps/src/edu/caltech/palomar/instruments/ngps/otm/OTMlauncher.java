/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package edu.caltech.palomar.instruments.ngps.otm;

 
import edu.caltech.palomar.instruments.ngps.object.Target;
import org.apache.commons.exec.CommandLine;
import org.apache.commons.exec.DefaultExecutor;
import org.apache.commons.exec.ExecuteWatchdog;
import org.apache.commons.exec.ShutdownHookProcessDestroyer;
import org.apache.commons.exec.*;
import edu.caltech.palomar.util.general.CommandLogModel;
import java.beans.PropertyChangeListener;
import java.beans.PropertyChangeSupport;
import javax.swing.JOptionPane;
import javax.swing.JFrame;
import java.io.FileWriter;
import java.io.PrintWriter;
import edu.caltech.palomar.instruments.ngps.dbms.NGPSdatabase;
import edu.caltech.palomar.instruments.ngps.tables.OTMTableModel;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.StringTokenizer;
import net.coobird.gui.simpleimageviewer4j.Viewer;
import edu.caltech.palomar.instruments.ngps.object.GlobalPreferencesModel;
import org.jfree.data.gantt.Task;
import org.jfree.data.gantt.TaskSeries;
import org.jfree.data.gantt.TaskSeriesCollection;
import org.jfree.data.gantt.XYTaskDataset;
import org.jfree.data.time.SimpleTimePeriod;
import edu.caltech.palomar.instruments.ngps.charts.OTM_Gantt_Chart;
import org.jfree.data.time.TimeSeries;
import org.jfree.data.time.TimeSeriesCollection;
import org.jfree.data.time.Millisecond;
import java.util.Date;
import edu.caltech.palomar.instruments.ngps.charts.CombinedAirmassGanttChart;
import org.jfree.data.time.Day;
import org.jfree.data.time.Hour;
import org.jfree.data.xy.DefaultIntervalXYDataset;
import edu.caltech.palomar.instruments.ngps.charts.XYIntervalChart;
import java.beans.PropertyChangeEvent;
import edu.caltech.palomar.instruments.ngps.gui.NGPSFrame;
import org.jfree.data.xy.XYSeries;
import org.jfree.data.xy.XYSeriesCollection;
import edu.caltech.palomar.instruments.ngps.charts.CombinedChartTest;
import edu.dartmouth.jskycalc.coord.InstantInTime;
import edu.dartmouth.jskycalc.coord.Site;
import edu.dartmouth.jskycalc.gui.NightlyWindow;
import edu.dartmouth.jskycalc.objects.NightlyAlmanac;
import edu.dartmouth.jskycalc.objects.WhenWhere;
import java.io.FileInputStream;
import java.util.Properties;
import edu.caltech.palomar.instruments.ngps.otm.OTMErrorsLog;
//=== File Prolog =============================================================
//	This code was developed by California Institute of Technology
//      Caltech Optical Observatories, Palomar Observatory
//
//--- Contents ----------------------------------------------------------------
//	 AstrometryPipelineLauncher
//
//--- Description -------------------------------------------------------------
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
//
//      March 8, 2015 Jennifer Milburn
//        Original Implementation
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
/*=============================================================================================
/         AstrometryPipelineLauncher Class Declaration
/=============================================================================================*/
public class OTMlauncher {
   transient private PropertyChangeSupport propertyChangeListeners = new PropertyChangeSupport(this);
   public static int                    DEBUG              = 0;
   public static int                    INFO               = 1;
   public static int                    WARN               = 2;
   public static int                    ERROR              = 3;
   public static int                    FATAL              = 4;
   private String                       right_ascension;
   private String                       declination;
   
   private DefaultExecutor              executor;
   private ShutdownHookProcessDestroyer processDestroyer;
   private ExecuteWatchdog              watchdog;
   
   private DefaultExecutor              plot_executor;
   private ShutdownHookProcessDestroyer plot_processDestroyer;
   private ExecuteWatchdog              plot_watchdog;
  
   public java.lang.String USERDIR           = System.getProperty("user.dir");
   public java.lang.String USERDIR_PYTHON    = System.getProperty("user.dir");
   public java.lang.String SEP               = System.getProperty("file.separator");
   public java.lang.String TEMPDIR           = SEP+"tmp";
   public java.lang.String OTM_EXEC          = USERDIR+SEP+"python"+SEP+"otm"+SEP+"OTM"+SEP+"OTM.py";
   public java.lang.String ETC_PATH          = USERDIR_PYTHON+SEP+"python"+SEP+"etc";
   public java.lang.String OTM_INFILE        = TEMPDIR+SEP+"OTM_JAVA_INPUT.csv";
   public java.lang.String OTM_OUTFILE        = TEMPDIR+SEP+"OTM_JAVA_OUTPUT.csv";
   public java.lang.String OTM_SCRIPT        = TEMPDIR+SEP+"OTM_JAVA_SCRIPT.txt";
   

   public java.lang.String DIR               = new java.lang.String();
   private PumpStreamHandler psh;
   private PumpStreamHandler plot_psh;
   public CommandLogModel myCommandLogModel = new CommandLogModel();
   private       int ProcessingState;
   private       int BatchProcessingState;
   private       int ProfileState;
   public static int IDLE    = 1;
   public static int RUNNING = 2;
   private JFrame                 frame;
   private java.lang.String           targetImageFile               = new java.lang.String();
   private java.lang.String           StatusMessage                 = new java.lang.String();
   private double                     ra;
   private double                     dec;
   public static int                    DISPLAY_OBJS = 1;
   public static int                    DISPLAY_NGC  = 2;
   public static int                    DISPLAY_INDX = 3;
   public static int                    DISPLAY_NEW  = 4; 
   private       int                    IMAGE_TYPE   = DISPLAY_INDX;
   private       int                    IdentifiedSourceCount    = 0;
   private       int                    AllSourceCount           = 0;
   private       int                    total_progress           = 0;
   public NGPSdatabase                  dbms;
   private java.lang.String             CONFIG          = new java.lang.String("config");
   private java.lang.String             OTM             = new java.lang.String("otm"); 
   public OTMTableModel                 myOTMTableModel = new OTMTableModel();
   private  TimelineChart               myTimelineChart;   
   private java.lang.String             PLOT_OUTPUT_FILE   = TEMPDIR+SEP+"PLOT.html";
   public  JFrame                       timeline_graph_frame;
   private double                       seeing;
   private int                          wavelength;
   private java.sql.Timestamp           my_start_time;
   private CommandLogModel              publicCommandLogModel;
   private GlobalPreferencesModel       myGlobalPreferencesModel;
   public TaskSeries                    myTaskSeries;
   public TaskSeriesCollection          myTaskSeriesCollection;
 //  public TimeSeries                    myTimeSeries;
   public TimeSeriesCollection          myOTMTimeSeriesCollection;
   public OTM_Gantt_Chart               myOTM_Gantt_Chart;
   public CombinedAirmassGanttChart     myCombinedAirmassGanttChart;
   public DefaultIntervalXYDataset      myDefaultIntervalXYDataset;
   public XYSeriesCollection            myXYSeriesCollection;
   private long                         start_first_exposure;
   public XYIntervalChart               myXYIntervalChart;
   public CombinedChartTest             myCombinedChartTest;
   public NGPSFrame                     myNGPSFrame;
   public NightlyAlmanac                myNightlyAlmanac;
   public static String [] palomarsite = {"Palomar Mountain",  "7.79089",  "33.35667",  "8.",  "1",  "Pacific",  "P",  "1706.",  "1706."};
   public Site                          palomar = new Site(palomarsite);
   private double                       airmass_limit;
   public static double                 DEFAULT_AIRMASS_LIMIT = 4.0;
   public OTMErrorsLog                  myOTMErrorsLog;
/*=============================================================================================
/         AstrometryPipelineLauncherr Class Declaration
/=============================================================================================*/
    public OTMlauncher(){ 
        bootstrap();
        initialize();
        initializeTimelineFrame();
        setAirmass_limit(DEFAULT_AIRMASS_LIMIT);
    }
/*================================================================================================
/      bootstrap()
/=================================================================================================*/
public void bootstrap(){
    try{
      java.lang.String BOOTSTRAP_FILE = USERDIR + SEP +"bootstrap.ini";
      FileInputStream  bootstrap_properties_file            = new FileInputStream(BOOTSTRAP_FILE);
      Properties       bootstrap_properties                 = new Properties();
      bootstrap_properties.load(bootstrap_properties_file);
      bootstrap_properties_file.close();
      USERDIR        = bootstrap_properties.getProperty("USERDIR");
      PLOT_OUTPUT_FILE   = TEMPDIR+SEP+"PLOT.html";
    }catch(Exception e){
        System.out.println(e.toString());
    }
}
 /*================================================================================================
/       setGlobalPreferencesModel(GlobalPreferencesModel newGlobalPreferencesModel)
/=================================================================================================*/
public void setNightlyAlmanac(NightlyAlmanac newNightlyAlmanac){
    myNightlyAlmanac = newNightlyAlmanac;
}    
/*================================================================================================
/       setGlobalPreferencesModel(GlobalPreferencesModel newGlobalPreferencesModel)
/=================================================================================================*/
public void setGlobalPreferencesModel(GlobalPreferencesModel newGlobalPreferencesModel){
     myGlobalPreferencesModel = newGlobalPreferencesModel;
 }
/*================================================================================================
/       initializeOTMlauncher()
/=================================================================================================*/
public void setPublicDocumentModel(CommandLogModel new_CommandLogModel){
   publicCommandLogModel = new_CommandLogModel; 
}   
/*================================================================================================
/      setNGPS_Database(NGPS_Database new_dbms)
/=================================================================================================*/
  public void setNGPSDatabase(NGPSdatabase new_dbms){
    dbms = new_dbms;  
  }
/*================================================================================================
/      setFrame(JFrame newJFrame)
/=================================================================================================*/
  public void setFrame(JFrame newJFrame){
      frame = newJFrame;
  } 
  public void setNGPSFrame(NGPSFrame ngps_frame){
     myNGPSFrame = ngps_frame;
  }
/*================================================================================================
/    getAstrometryBinaryTableReader()
/=================================================================================================*/
  public CommandLogModel getCommandLogModel(){
      return myCommandLogModel;
  }
  public void initializeTimelineFrame(){
      myTimelineChart = new TimelineChart(false);
  } 
/*================================================================================================
/      initialize()
/=================================================================================================*/
 private void initialize(){
   executor         = new DefaultExecutor();
   processDestroyer = new ShutdownHookProcessDestroyer();
   watchdog         = new ExecuteWatchdog(Integer.MAX_VALUE);
   executor.setWatchdog(watchdog);
   executor.setProcessDestroyer(processDestroyer);
   psh = new PumpStreamHandler(new OutputHandler());
   executor.setStreamHandler(psh);
   
   plot_executor         = new DefaultExecutor();
   plot_processDestroyer = new ShutdownHookProcessDestroyer();
   plot_watchdog         = new ExecuteWatchdog(Integer.MAX_VALUE);
   plot_executor.setWatchdog(plot_watchdog);
   plot_executor.setProcessDestroyer(plot_processDestroyer);
   plot_psh = new PumpStreamHandler(new OutputHandler());
   plot_executor.setStreamHandler(plot_psh);
   myOTMErrorsLog = new OTMErrorsLog();
; }
/*================================================================================================
/     executeOTM(java.sql.Timestamp start_time,double seeing,int wavelength)
/=================================================================================================*/
 public void executeOTM(java.sql.Timestamp start_time,double seeing,int wavelength){
   ExecuteProcessThread myExecuteProcessThread = new ExecuteProcessThread(start_time,seeing,wavelength);
   myExecuteProcessThread.start();
 }
/*================================================================================================
/      OTM(java.sql.Timestamp start_time,double seeing,int wavelength)
/=================================================================================================*/
public void OTM(java.sql.Timestamp start_time,double seeing,int wavelength){
   myOTMTableModel.clearTable();
   myCommandLogModel.insertMessage(CommandLogModel.COMMAND, "Constructing execution script."); 
   constructScriptFile(start_time,seeing,wavelength); 
   myCommandLogModel.insertMessage(CommandLogModel.COMMAND, "Creating OTM input file."); 
   createOTMInputFile();
   
       String line = OTM_SCRIPT;
       setProcessingState(RUNNING);
       myOTMErrorsLog.clearList();
       myOTMErrorsLog.setVisible(false);
       logMessage(INFO,"Starting OTM.py Program ");
       try{
          myCommandLogModel.insertMessage(CommandLogModel.COMMAND, "Execution of the OTM started."); 
          CommandLine commandLine = CommandLine.parse(line);
          int exitValue              = executor.execute(commandLine);
//          readOTMoutput();
          if(exitValue == 0){
             myCommandLogModel.insertMessage(CommandLogModel.COMMAND, "Execution of the OTM complete, reading output."); 
             readOTMoutput();
          }
           if(exitValue != 0){
               displayScreenMessage("OTM existed with an error. Please see execution log.");
           }                     
       }catch(Exception e){                      
         setProcessingState(IDLE);
         logMessage(INFO,"EXCEPTION IN OTM EXECUTION  "+e.toString());
       }
       setProcessingState(IDLE);
       logMessage(INFO,"OTM program complete ");
       try{
           Runtime.getRuntime().exec("chmod a+w "+OTM_OUTFILE);  //make writable by all
       }catch(Exception e){                      
         logMessage(INFO, "COULD NOT CHMOD OUTPUT:  "+e.toString());
       }

//       java.lang.String OTM_TIMELINE_IMAGE    = "/Users/jennifermilburn/Desktop/NGPS/python/git/OTM/timeline.png";       
//       myTimelineChart.loadImage(OTM_TIMELINE_IMAGE);
//       myTimelineChart.setVisible(true);
//       display_timeline_image(OTM_TIMELINE_IMAGE);
} 
/*================================================================================================
/    executePLOT()
/=================================================================================================*/
 public void executePLOT(java.sql.Timestamp start_time,double seeing,int wavelength){
   ExecutePlotThread myExecutePlotThread = new ExecutePlotThread(start_time,seeing,wavelength);
   myExecutePlotThread.start();
 }
/*================================================================================================
/      OTM(java.sql.Timestamp start_time,double seeing,int wavelength)
/=================================================================================================*/
public void PLOT(){
   myOTMTableModel.clearTable();
   myCommandLogModel.insertMessage(CommandLogModel.COMMAND, "Constructing plot execution script."); 
   constructPlotScriptFile(); 
   myCommandLogModel.insertMessage(CommandLogModel.COMMAND, "Creating OTM input file."); 
       String line = TEMPDIR+SEP+"PLOT_SCRIPT.txt";
       setProcessingState(RUNNING);
       logMessage(INFO,"Starting OTM_plot.py Program ");
       try{
          myCommandLogModel.insertMessage(CommandLogModel.COMMAND, "Execution of the OTM_plot started."); 
          CommandLine commandLine = CommandLine.parse(line);
          int exitValue              = executor.execute(commandLine);
//          readOTMoutput();
          if(exitValue == 0){
             myCommandLogModel.insertMessage(CommandLogModel.COMMAND, "Execution of the OTM_plot complete, reading output."); 
//             readOTMoutput();
          }
           if(exitValue != 0){
               displayScreenMessage("OTM_plot existed with an error. Please see execution log.");
           }                     
       }catch(Exception e){                      
         setProcessingState(IDLE);
       }
       setProcessingState(IDLE);
       logMessage(INFO,"OTM_plot program complete ");
       java.lang.String OTM_TIMELINE_IMAGE    = USERDIR+SEP+"config"+SEP+"otm"+SEP+"timeline.png";
       display_timeline_image(OTM_TIMELINE_IMAGE);

//       myTimelineChart.loadImage(OTM_TIMELINE_IMAGE);
//       myTimelineChart.setVisible(true);
//       timeline_graph_frame = BrowserFactory.createFrame("Timeline Model","file:"+PLOT_OUTPUT_FILE, 0);
}  
/*================================================================================================
/      constructScriptFile(java.sql.Timestamp start_time,double seeing,int wavelength)
/=================================================================================================*/
   public void display_timeline_image(java.lang.String image_name){
//       java.lang.String IMAGE_NAME = "timeline.png";
       BufferedImage img = null;
      try {
          img = javax.imageio.ImageIO.read(new File(image_name));
          new Viewer(img).show();
      }catch (IOException e) {
      }       
    }
/*================================================================================================
/      constructScriptFile(java.sql.Timestamp start_time,double seeing,int wavelength)
/=================================================================================================*/
public void constructScriptFile(java.sql.Timestamp start_time,double seeing,int wavelength){

 java.lang.String PYTHON_INSTALL_DIR = dbms.PYTHON_INSTALL_DIR;
 //java.lang.String OTM_INPUT_FILE     = TEMPDIR+SEP+"OTM_JAVA_INPUT.csv";
 //java.lang.String OTM_OUTPUT_FILE    = TEMPDIR+SEP+"OTM_JAVA_OUTPUT.csv";
 //java.lang.String SCRIPT_FILENAME    = TEMPDIR+SEP+"OTM_SCRIPT.txt";
 double           ALT_TWILIGHT       = -12.0;
 try{
    java.lang.String start_time_string = timestamp_to_string(start_time);
 //   start_time_string = start_time_string.trim();
 //   start_time_string = start_time_string.replaceAll(" ","T");
    java.io.File current = new java.io.File(OTM_SCRIPT);
    java.io.PrintWriter pw = new java.io.PrintWriter(current);
    pw.println("export PYTHONPATH="+ETC_PATH);
    pw.println(PYTHON_INSTALL_DIR+" "+OTM_EXEC+" "+OTM_INFILE+" "+start_time_string+" -seeing "+seeing+" "+wavelength+" -out "+OTM_OUTFILE+" -alt_twilight "+ALT_TWILIGHT+" -forceSNR"+" -airmass_max "+airmass_limit);
//    pw.println("./OTM.py"+" "+OTM_INPUT_FILE+" "+start_time_string+" -seeing "+seeing+" "+wavelength+" -out "+OTM_OUTPUT_FILE+" -alt_twilight "+ALT_TWILIGHT);
//    pw.println(PYTHON_INSTALL_DIR+" "+"./OTM.py"+" "+OTM_INPUT_FILE+" "+start_time_string+" -seeing "+seeing+" "+wavelength+" -out "+OTM_OUTPUT_FILE+" -timeline timeline.png");
    pw.flush();
    pw.close();
    //current.setExecutable(true);
    Runtime.getRuntime().exec("chmod 777 "+OTM_SCRIPT);  //make executable and writable by all
 }catch(Exception e){
    System.out.println(""+e.toString());
 }
} 
/*================================================================================================
/      constructScriptFile(java.sql.Timestamp start_time,double seeing,int wavelength)
/=================================================================================================*/
public void constructPlotScriptFile(){
    //  EXAMPLE SCRIPT FILE 
//cd /Users/jennifermilburn/Desktop/NGPS/python/git/OTM
//export PYTHONPATH=/Users/jennifermilburn/Desktop/NGPS/python/git
///Users/jennifermilburn/opt/anaconda3/envs/astro/bin/python ./OTM.py /Users/jennifermilburn/Desktop/NGPS/telemtry/telemetry_server/config/otm/OTM_JAVA_INPUT.csv 2022-01-01T03:00:00.000 -seeing 1.25 500 -out /Users/jennifermilburn/Desktop/NGPS/telemtry/telemetry_server/config/otm/OTM_JAVA_OUTPUT.csv
// java.lang.String PYTHON_INSTALL_DIR = "/Users/jennifermilburn/opt/anaconda3/envs/astro/bin/python";
//
 java.lang.String PYTHON_INSTALL_DIR = dbms.PYTHON_INSTALL_DIR;
 java.lang.String OTM_INPUT_FILE     = TEMPDIR+SEP+"OTM_JAVA_INPUT.csv";
 java.lang.String OTM_OUTPUT_FILE    = TEMPDIR+SEP+"OTM_JAVA_OUTPUT.csv";
 java.lang.String PLOT_OUTPUT_FILE   = TEMPDIR+SEP+"PLOT.html";
 java.lang.String SCRIPT_FILENAME    = TEMPDIR+SEP+"PLOT_SCRIPT.txt";
 try{
    java.io.File current = new java.io.File(SCRIPT_FILENAME);
    java.io.PrintWriter pw = new java.io.PrintWriter(current);
    pw.println("export PYTHONPATH="+ETC_PATH);
    pw.println(PYTHON_INSTALL_DIR+" "+OTM_EXEC+" "+OTM_OUTPUT_FILE+" "+PLOT_OUTPUT_FILE);
//    pw.println(PYTHON_INSTALL_DIR+" "+"./OTM.py"+" "+OTM_INPUT_FILE+" "+start_time_string+" -seeing "+seeing+" "+wavelength+" -out "+OTM_OUTPUT_FILE+" -timeline timeline.png");
    pw.flush();
    pw.close();
 }catch(Exception e){
    System.out.println(""+e.toString());
 }
} 
/*=============================================================================================
/   executeParseFile()
/=============================================================================================*/
public void createOTMInputFile(){
   try{
      FileWriter   output_file = new FileWriter(OTM_INFILE); 
      PrintWriter  pw          = new PrintWriter(output_file);
      pw.println("name,	RA,DECL,binspect,binspat,ccdmode,slitangle,slitwidth,exptime,wrange,channel,mag,magsystem,magfilter,airmass_max,notbefore,pointmode,srcmodel");
//      pw.println("name,RA,DECL,epoch,ccdmode,casangle,slitwidth,exptime,wrange,channel,mag,magref,srcmodel");
      int num_targets = dbms.myTargetDBMSTableModel.getRowCount(); 
      for(int i=0;i<num_targets;i++){
         Target current = dbms.myTargetDBMSTableModel.getRecord(i);
         java.lang.String current_line = new java.lang.String();
         if(current.instrument.getRequestedSlitAngle() == null){
             current.instrument.setRequestedSlitAngle("0");
         }
         java.lang.String current_airmass_max = new java.lang.String();
//         if(current.sky.getAIRMASS_MAX() == null){             
//           current_airmass_max =(Double.toString(myGlobalPreferencesModel.getGLOBAL_AIRMASS_MAX()));
//        }else if(current.sky.getAIRMASS_MAX() != null){
         if(current.sky.getAIRMASS_MAX() != null){
             current_airmass_max = current.sky.getAIRMASS_MAX();
         }
         java.lang.String not_before = new java.lang.String();
         if(current.otm.getOTMnotbefore() == null){
             not_before = " ";
         }else{
             not_before = current.otm.getOTMnotbefore().toString();
         }
         if(current.sky.getRightAscension() == null){
            current.sky.setRightAscension("");
         }
         if(current.sky.getDeclination() == null){
            current.sky.setDeclination("");
         }
         current_line = current.getName()+","+
                        current.sky.getRightAscension()+","+
                        current.sky.getDeclination()+","+                
//                        current.sky.getEPOCH()+","+
                        current.instrument.getBIN_SPEC()+","+
                        current.instrument.getBIN_SPACE()+","+
                        current.instrument.getOBSMODE()+","+
                        current.instrument.getRequestedSlitAngle()+","+
                        current.instrument.getSlitwidth_string()+","+
                        current.instrument.getExposuretime()+","+
                        current.etc.getWRANGE_LOW()+" "+
                        current.etc.getWRANGE_HIGH()+","+
                        current.etc.getChannel()+","+
                        current.etc.getMagnitude()+","+
                        current.etc.getMagref_system()+","+
                        current.etc.getMagref_filter()+","+
                        current_airmass_max+","+                       
                        not_before+","+
                        current.otm.getOTMpointmode()+","+
                        current.etc.getSrcmodel();
         pw.println(current_line);
      } 
      pw.flush();
      pw.close();
      Runtime.getRuntime().exec("chmod a+w "+OTM_INFILE);  //make writable by all
   }catch(Exception e){
      System.out.println(e.toString());
   }
}
public void setSlewgoStart_FirstExposure(long new_start_first_exposure){
    start_first_exposure = new_start_first_exposure;
}
/*================================================================================================
/    readOTMoutput()
/=================================================================================================*/
public void readOTMoutput(){
    try{
//       myTaskSeriesCollection    = new TaskSeriesCollection();
//       myOTMTimeSeriesCollection = new TimeSeriesCollection();  
       myDefaultIntervalXYDataset = new DefaultIntervalXYDataset();  
       myXYSeriesCollection       = new XYSeriesCollection();
      java.lang.String reference_time = new java.lang.String();       
       
       java.lang.String OTM_OUTPUT_FILE_STRING    = TEMPDIR+SEP+"OTM_JAVA_OUTPUT.csv";
       java.io.File OTM_OUTPUT_FILE = new java.io.File(OTM_OUTPUT_FILE_STRING);
       java.io.FileReader fr = new java.io.FileReader(OTM_OUTPUT_FILE);
       java.io.BufferedReader br = new java.io.BufferedReader(fr);
       int num_targets = dbms.myTargetDBMSTableModel.getRowCount(); 
       java.lang.String header_line = br.readLine();
       ArrayList        header_list = parseHeaderLine(header_line);
       boolean first_image = true;
       myOTMTableModel.clearTable();
         for(int i=0;i<num_targets;i++){
             Target current = dbms.myTargetDBMSTableModel.getRecord(i);
             java.lang.String current_line = br.readLine();
             current =  parseOTMRecord(current,header_list,current_line);
             myOTMTableModel.addRecord(current);
             java.lang.String flags = current.otm.getOTMflag();
//             System.out.println(flags);
             if(!flags.contains("-1")){
                 if(current.getSTATE().matches("ERROR-OTM"))
                 current.setSTATE("PENDING");
                 dbms.myTargetDBMSTableModel.fireTableDataChanged();
             }             
             if(flags.contains("-1")){
                 current.setSTATE("ERROR-OTM");
                 dbms.myTargetDBMSTableModel.fireTableDataChanged();
             }
             else if(!flags.contains("-1")){
                 if(first_image){
                     first_image = false;
                     long first_image_start = current.otm.getOTMstart().getTime();
                     reference_time = current.otm.getOTMstart().toString();
                     setSlewgoStart_FirstExposure(first_image_start);
                     constructChartData(i,current);
                 }else
                 if(current.otm.getOTMstart().getTime() <= current.otm.getOTMend().getTime()){
                     constructChartData(i,current);
                    // addXIntervalXYDataset(current);
                    // constructXYSeries(i,current);
                 }                    
             }        
         } 
   java.lang.String cal_reference_time = reference_time.replace("-", " ");
   InstantInTime i = new InstantInTime(cal_reference_time,palomar.stdz,palomar.use_dst,true);              
   WhenWhere     w = new WhenWhere(i,palomar);               
   myNightlyAlmanac = new NightlyAlmanac(w);
   myNightlyAlmanac.Update(w);
   java.lang.String sunset         = myNightlyAlmanac.sunset.when.UTDate.RoundedCalString(13, 2);
   java.lang.String sunrise        = myNightlyAlmanac.sunrise.when.UTDate.RoundedCalString(13, 2);
   java.lang.String evening_twilight =  myNightlyAlmanac.eveningTwilight.when.UTDate.RoundedCalString(13, 2);
   java.lang.String morning_twilight =  myNightlyAlmanac.morningTwilight.when.UTDate.RoundedCalString(13, 2);
     
    dbms.myTargetDBMSTableModel.fireTableDataChanged();
    
    myCombinedChartTest = new CombinedChartTest("Observation Timline: "+dbms.getSelectedSetName(),
                                                myDefaultIntervalXYDataset,myXYSeriesCollection,
                                                sunset,sunrise,reference_time,evening_twilight,morning_twilight,start_first_exposure);
    dbms.setCombinedChartTest(myCombinedChartTest);
    myCombinedChartTest.setDBMS(dbms);
    myCombinedChartTest.setVisible(true);
    myCombinedChartTest.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
               public void propertyChange(java.beans.PropertyChangeEvent e) {
                chart_selection_propertyChange(e);
              }
            }); 
    }catch(Exception e){
       System.out.println("ERROR parsing the OTM output."+e.toString()); 
    }

//    myOTM_Gantt_Chart = new OTM_Gantt_Chart(dbms.getSelectedSetName(),myTaskSeriesCollection);
//    myOTM_Gantt_Chart.setVisible(true);   
//    XYTaskDataset dataset_gantt = new XYTaskDataset(myTaskSeriesCollection);
//    myCombinedAirmassGanttChart = new CombinedAirmassGanttChart("TEST",myOTMTimeSeriesCollection,dataset_gantt);
//    myCombinedAirmassGanttChart.setVisible(true);
}
/*=============================================================================================
/     Property Change Listener for the Ephemeris_propertyChange
/=============================================================================================*/
  private void chart_selection_propertyChange(PropertyChangeEvent e)  {
     java.lang.String propertyName = e.getPropertyName();
     System.out.println(propertyName);
/*=============================================================================================
/    PARAMETERS FROM THE JSKYCALCMODEL
/=============================================================================================*/
    if(propertyName.matches("selected_series")){        
        java.lang.Integer current_value = (java.lang.Integer)e.getNewValue(); 
        myNGPSFrame.getMainTable().setRowSelectionInterval(current_value,current_value);
    }
}
/*=============================================================================================
/   constructTask(Target current)
/=============================================================================================*/
public void addXIntervalXYDataset(Target current){
    long start_time = (current.otm.getOTMstart().getTime() - start_first_exposure)/(1000*60);
    long end_time   = (current.otm.getOTMend().getTime() - start_first_exposure)/(1000*60);
    long center_of_observation = (end_time - start_time)/2;
    double airmass_start = current.otm.getOTMAirmass_start();
    double airmass_end   = current.otm.getOTMAirmass_end();
    double mean_airmass  = (airmass_start + airmass_end)/2;
    double[] x = {center_of_observation};
    double[] startx = {(double)start_time};
    double[] endx = {(double)end_time};
    double[] y = {mean_airmass};
    double[] starty = {0.00};
    double[] endy = {5.0};
    double[][] data = new double[][] {x, startx, endx, y, starty, endy};
    if(startx[0] > 0){
     myDefaultIntervalXYDataset.addSeries(current.getName(), data);   
    }
    System.out.println(current.getName()+ " "+Double.toString(startx[0])+" "+Double.toString(endx[0])+" "+Double.toString(starty[0])+" "+Double.toString(endy[0]));
}
public void constructXYSeries(int sequence_number,Target current){
      XYSeries series = new XYSeries(current.getName()+"-"+sequence_number);
      long start_time = (current.otm.getOTMstart().getTime() - start_first_exposure)/(1000*60);
      long end_time   = (current.otm.getOTMend().getTime() - start_first_exposure)/(1000*60);      
      series.add(start_time,current.otm.getOTMAirmass_start());
      series.add(end_time,current.otm.getOTMAirmass_end());
      myXYSeriesCollection.addSeries(series);
}
public void constructChartData(int sequence_number,Target current){
    long start_time = (current.otm.getOTMstart().getTime() - start_first_exposure)/(1000*60);
    long end_time   = (current.otm.getOTMend().getTime() - start_first_exposure)/(1000*60);
    long center_of_observation = (end_time - start_time)/2;
    double airmass_start = current.otm.getOTMAirmass_start();
    double airmass_end   = current.otm.getOTMAirmass_end();
    double mean_airmass  = (airmass_start + airmass_end)/2;
    double[] x = {center_of_observation};
    double[] startx = {(double)start_time};
    double[] endx = {(double)end_time};
    double[] y = {mean_airmass};
    double[] starty = {1.00};
    double[] endy = {3.0};
    double[][] data = new double[][] {x, startx, endx, y, starty, endy};
    if(startx[0] >= 0){
          XYSeries series = new XYSeries(current.getName()+"-"+sequence_number);
          myDefaultIntervalXYDataset.addSeries(current.getName()+"-"+sequence_number, data);   
          series.add(start_time,current.otm.getOTMAirmass_start());
          series.add(end_time,current.otm.getOTMAirmass_end());
          myXYSeriesCollection.addSeries(series);
    }
    System.out.println(current.getName()+ " "+Double.toString(startx[0])+" "+Double.toString(endx[0])+" "+Double.toString(starty[0])+" "+Double.toString(endy[0]));
    System.out.println(current.getName()+ " "+Double.toString(start_time)+" "+Double.toString(current.otm.getOTMAirmass_start())+" "+Double.toString(end_time)+" "+Double.toString(current.otm.getOTMAirmass_end()));   
}
/*=============================================================================================
/   constructTask(Target current)
/=============================================================================================*/
public TaskSeries constructTask(Target current){
    Task       current_task = new Task(current.getName(),new SimpleTimePeriod(current.otm.getOTMstart(),current.otm.getOTMend()));   
    TaskSeries current_taskseries = new TaskSeries(current.getName());
               current_taskseries.add(current_task);
 return current_taskseries;                       
}
public void addTaskSeries(TaskSeries current){   
    myTaskSeriesCollection.add(current);
}
public TimeSeries constructTimeSeries(Target current){
    TimeSeries current_time_series = new TimeSeries(current.getName());
    Millisecond start = new Millisecond(new Date(current.otm.getOTMstart().getTime()));
    Millisecond end   = new Millisecond(new Date(current.otm.getOTMend().getTime()));
    current_time_series.add(start,current.otm.getOTMAirmass_start(), true);
    current_time_series.add(end,current.otm.getOTMAirmass_end(), true);
    return current_time_series;
}
public void addTimeSeries(TimeSeries current_series){
    myOTMTimeSeriesCollection.addSeries(current_series);
}
/*=============================================================================================
/   parseText()
/=============================================================================================*/
public ArrayList parseHeaderLine(java.lang.String header_line){
    java.util.StringTokenizer st = new java.util.StringTokenizer(header_line,",");
    int token_count = st.countTokens();
    ArrayList header_list = new ArrayList();
    for(int i=0;i<token_count;i++){
        java.lang.String token = st.nextToken();
        header_list.add(token);
//        System.out.println(token.toUpperCase());
//        token = token.toUpperCase();
    }
  return header_list;
} 
/*================================================================================================
/     string_to_timestamp(java.lang.String current_datetime)
/=================================================================================================*/
public java.sql.Timestamp string_to_timestamp(java.lang.String current_datetime){
java.sql.Timestamp timestamp =new java.sql.Timestamp(System.currentTimeMillis());
 try {
    java.text.SimpleDateFormat dateFormat = new java.text.SimpleDateFormat("yyyy-MM-dd'T'hh:mm:ss.SSS");
    java.util.Date parsedDate = dateFormat.parse(current_datetime);
    timestamp = new java.sql.Timestamp(parsedDate.getTime());
} catch(Exception e) { //this generic but you can control another types of exception
    // look the origin of excption 
}
return timestamp;
}
public java.lang.String timestamp_to_string(java.sql.Timestamp current){
//    java.sql.Timestamp timestamp = java.sql.Timestamp.valueOf("2018-12-12 01:02:03.123456789");
    java.time.format.DateTimeFormatter formatter = java.time.format.DateTimeFormatter.ISO_LOCAL_DATE_TIME;
    String timestampAsString = formatter.format(current.toLocalDateTime());
//    assertEquals("2018-12-12T01:02:03.123456789", timestampAsString);
return timestampAsString;
}
/*=============================================================================================
/   parseObservationRecord(java.util.ArrayList current_list,java.lang.String current)
/=============================================================================================*/
 public Target parseOTMRecord(Target current_target,java.util.ArrayList current_list,java.lang.String current){
     StringTokenizer st = new StringTokenizer(current,",");
     java.lang.String[] token_array = current.split(",");
     int count_header = current_list.size();
     int count_current = st.countTokens();
//     System.out.println("Counted Tokens Header = "+count_header + " Counted Tokens String = "+count_current);
     for(int i=0;i<count_header;i++){
        java.lang.String field = ((java.lang.String)current_list.get(i)).trim();
        java.lang.String current_value = token_array[i];
        current_value = current_value.trim();
        if(current_value == null){
//            System.out.println("Null value");
        }
//        System.out.println(field + " "+ current_value);
        if(field.matches("name")){
            current_target.otm.setOTMname(current_value);
        }        
        if(field.matches("OTMpa")){
           try{
              java.lang.Double value = Double.valueOf(current_value);
              current_target.otm.setOTMpa(value);
           }catch(Exception e){
              System.out.println("Error parsing the OTMpa value from the OTM output"+e.toString());
           }
        }
        if(field.matches("OTMslitangle")){
           try{
              java.lang.Double value = Double.valueOf(current_value);
              current_target.otm.setOTMslitangle(value);
           }catch(Exception e){
              System.out.println("Error parsing the OTMcass value from the OTM output"+e.toString());
           }        
        }
        if(field.matches("OTMcass")){
           try{
              java.lang.Double value = Double.valueOf(current_value);
              current_target.otm.setOTMcass(value);
           }catch(Exception e){
              System.out.println("Error parsing the OTMcass value from the OTM output"+e.toString());
           }        
        }
        if(field.matches("OTMwait")){
            try{                
              java.lang.Double value = Double.valueOf(current_value);
              current_target.otm.setOTMwait(value);
            }catch(Exception e){
                System.out.println("Error parsing the OTMwait value from the OTM output"+e.toString());
            }            
        }
        if(field.matches("OTMflag")){
            try{
                current_target.otm.setOTMflag(current_value);
             }catch(Exception e){
                System.out.println("Error parsing the OTMflag value from the OTM output"+e.toString());
            }            
        }
        if(field.matches("OTMslewgo")){
            try{  
                java.sql.Timestamp current_timestamp = string_to_timestamp(current_value);
                current_target.otm.setOTMslewgo(current_timestamp);
            }catch(Exception e){
                System.out.println("Error parsing the OTMslewgo value from the OTM output"+e.toString());
            }            
        }  
        if(field.matches("OTMlast")){
            try{
               current_target.otm.setOTMlast(current_value);
             }catch(Exception e){
                System.out.println("Error parsing the OTMlast value from the OTM output"+e.toString());
            }            
        }
      if(field.matches("OTMslew")){
            try{
                java.lang.Double value = Double.valueOf(current_value);
                current_target.otm.setOTMslew(value);
            }catch(Exception e){
                System.out.println("Error parsing the OTMslew value from the OTM output"+ e.toString());
            }             
       }  
      if(field.matches("OTMdead")){
            try{            
                java.lang.Double value = Double.valueOf(current_value);
                current_target.otm.setOTMdead(value);
            }catch(Exception e){
                System.out.println(e.toString());
            }            
       }       
       if(field.matches("OTMstart")){
            try{  
                java.sql.Timestamp current_timestamp = string_to_timestamp(current_value);
                current_target.otm.setOTMstart(current_timestamp);
            }catch(Exception e){
                System.out.println(e.toString());
            }            
       }  
       if(field.matches("OTMend")){
            try{  
                java.sql.Timestamp current_timestamp = string_to_timestamp(current_value);
                current_target.otm.setOTMend(current_timestamp);
            }catch(Exception e){
                System.out.println(e.toString());
            }            
       } 
       if(field.matches("OTMSNR")){
           try{
               java.lang.String value = new java.lang.String(current_value);
               current_target.otm.setOTMSNR(value);
           }catch(Exception e){
              System.out.println(e.toString()); 
           }
       }
       if(field.matches("OTMres")){
           try{
               java.lang.Double value = Double.valueOf(current_value);
               current_target.otm.setOTMres(value.doubleValue());
           }catch(Exception e){
              System.out.println(e.toString()); 
           }
       }
       if(field.matches("OTMairmass_start")){
            try{
                java.lang.Double value = Double.valueOf(current_value);
                current_target.otm.setOTMAirmass_start(value);
            }catch(Exception e){
                System.out.println(e.toString());
            }            
       }  
       if(field.matches("OTMairmass_end")){
            try{
                java.lang.Double value = Double.valueOf(current_value);
                current_target.otm.setOTMAirmass_end(value);
            }catch(Exception e){
                System.out.println(e.toString());
            }            
       }        
       if(field.matches("OTMsky")){
            try{
                java.lang.Double value = Double.valueOf(current_value);
                current_target.otm.setSkymag(value);
            }catch(Exception e){
                System.out.println(e.toString());
            }            
       }   
       if(field.matches("OTMexptime")){
            try{
                java.lang.Double value = Double.valueOf(current_value);
                current_target.otm.setOTMexpt(value);
            }catch(Exception e){
                System.out.println(e.toString());
            }            
       }          
       if(field.matches("OTMslitwidth")){
            try{
                java.lang.Double value = Double.valueOf(current_value);
                current_target.otm.setOTMslitwidth(value);
            }catch(Exception e){
                System.out.println(e.toString());
            }            
       }    
        if(field.matches("OTMmoon")){
            try{
                java.lang.String value = new java.lang.String(current_value);
                current_target.otm.setOTMmoon(value);
            }catch(Exception e){
                System.out.println(e.toString());
            }            
       }   
       if(field.matches("OTMseeing")){
            try{
                java.lang.Double value = Double.valueOf(current_value);
                current_target.otm.setOTMseeing(value);
            }catch(Exception e){
                System.out.println(e.toString());
            }            
       } 
       if(field.matches("notbefore")){
            try{  
                java.sql.Timestamp current_timestamp = string_to_timestamp(current_value);
                current_target.otm.setOTMnotbefore(current_timestamp);
            }catch(Exception e){
                System.out.println(e.toString());
            }            
       } 
       if(field.matches("pointmode")){
           try{
               java.lang.String value = new java.lang.String(current_value);
               current_target.otm.setOTMpointmode(value);
           }catch(Exception e){
              System.out.println(e.toString()); 
           }
       }
      }
     return current_target;
 }
/*================================================================================================
/      test()
/=================================================================================================*/
 private void test(){
     try{   
           java.sql.Timestamp start_time_initial = new java.sql.Timestamp(System.currentTimeMillis());
           double seeing = 1.25;
           int    wavelength = 500;
           constructScriptFile(start_time_initial,seeing,wavelength);
           long start_time = System.currentTimeMillis();
           CommandLine commandLine = start_OTM();
           int exitValue              = executor.execute(commandLine);
           if(exitValue != 0){
               displayScreenMessage("OTM exited with an error. Please see execution log.");
           }           
           setProcessingState(IDLE);
    }catch(Exception e){
            logMessage(ERROR,"method ExecuteProcessThread error = " + e.toString());
            setProcessingState(IDLE);
    }  
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
/*================================================================================================
/     setStartTimestamp(java.sql.Timestamp new_start_time)
/=================================================================================================*/
public void setStartTimestamp(java.sql.Timestamp new_start_time){
    my_start_time = new_start_time;
}
public java.sql.Timestamp getStartTimestamp(){
    return my_start_time;
}
/*================================================================================================
/     setAirmass_limit(double new_airmass_limit)
/=================================================================================================*/
  public void setAirmass_limit(double new_airmass_limit) {
    double  old_airmass_limit = this.airmass_limit;
    this.airmass_limit = new_airmass_limit;
    propertyChangeListeners.firePropertyChange("airmass_limit", Double.valueOf(old_airmass_limit), Double.valueOf(new_airmass_limit));
  }
  public double getAirmass_limit() {
    return airmass_limit;
  } 
/*================================================================================================
/      setSpectrumState(int newSpectrumState)
/=================================================================================================*/
  public void setSeeing(double new_seeing) {
    double  oldseeing = this.seeing;
    this.seeing = new_seeing;
    propertyChangeListeners.firePropertyChange("seeing", Double.valueOf(oldseeing), Double.valueOf(new_seeing));
  }
  public double getSeeing() {
    return seeing;
  } 
/*================================================================================================
/      setSpectrumState(int newSpectrumState)
/=================================================================================================*/
  public void setWavelength(int new_wavelength) {
    int  oldwavelength = this.wavelength;
    this.wavelength = new_wavelength;
    propertyChangeListeners.firePropertyChange("wavelength", Integer.valueOf(oldwavelength), Integer.valueOf(new_wavelength));
  }
  public int getWavelength() {
    return wavelength;
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
/*================================================================================================
/        setTargetImageFile(String newTargetImageFile)
/=================================================================================================*/
   public void setTargetImageFile(String newTargetImageFile) {
     java.lang.String oldTargetImageFile = this.targetImageFile;
     this.targetImageFile                = newTargetImageFile;
    propertyChangeListeners.firePropertyChange("targetImageFile", oldTargetImageFile, newTargetImageFile);
   }
   public String getTargetImageFile() {
     return targetImageFile;
   }
/*================================================================================================
/        setStatusMessage(String newStatusMessage) 
/=================================================================================================*/
   public void setStatusMessage(String newStatusMessage) {
     java.lang.String oldStatusMessage = this.StatusMessage;
     this.StatusMessage                = newStatusMessage;
     propertyChangeListeners.firePropertyChange("StatusMessage", oldStatusMessage, newStatusMessage);
   }
   public String getStatusMessage() {
     return StatusMessage;
   }   
/*================================================================================================
/      setDIR(java.lang.String newDIR)
/=================================================================================================*/
  public void setDIR(java.lang.String newDIR){
      java.lang.String oldDIR = this.DIR;
      this.DIR = newDIR;
      propertyChangeListeners.firePropertyChange("DIR", oldDIR, newDIR);
  }
  public java.lang.String getDIR(){
      return DIR;
  }
 /*================================================================================================
/      setTotalProgress(int new_exposure_progress)
/=================================================================================================*/
  public void setTotalProgress(int new_total_progress) {
    int  old_total_progress = this.total_progress;
    this.total_progress = new_total_progress;
   propertyChangeListeners.firePropertyChange("total_progress", Integer.valueOf(old_total_progress), Integer.valueOf(new_total_progress));
  }
  public int getTotalProgress() {
    return total_progress;
  } 
/*================================================================================================
/     start_OTM(java.lang.String fits_file_name,double seeing)
/=================================================================================================*/
 public CommandLine start_OTM(){
//    String line = "/Users/jennifermilburn/Desktop/NGPS/python/git/OTM/start_OTM.txt";
//    String line = "/Users/jennifermilburn/Desktop/NGPS/telemtry/telemetry_server/config/otm/start_OTM.txt";
//    String line = "/Users/jennifermilburn/Desktop/NGPS/telemtry/telemetry_server/config/otm/OTM_SCRIPT.txt";
    String line = "/tmp/OTM_SCRIPT.txt";
      setProcessingState(RUNNING);
       CommandLine commandLine = CommandLine.parse(line);
 //       commandLine.addArgument("/Users/jennifermilburn/Desktop/NGPS/python/git/OTM/OTM.py");
//        commandLine.addArgument("-seeing");
//        commandLine.addArgument("1.25");
//        commandLine.addArgument(Printf.format("%5.4f", new PrintfData().add(scale_low)));
//        commandLine.addArgument("-out");
//        commandLine.addArgument(Printf.format("%5.4f", new PrintfData().add(scale_high))); 
//        commandLine.addArgument("/Users/jennifermilburn/Desktop/NGPS/python/git/OUTPUT_OTM.csv");
        logMessage(INFO,"Starting OTM.py Program ");
   return commandLine;     
 }  
/*================================================================================================
/      kill_Python()
/=================================================================================================*/
 public boolean kill_Solve_Field(){
    watchdog.destroyProcess();
    boolean state = watchdog.killedProcess();
    return state;
 }
/*=============================================================================================
/        logMessage(java.lang.String message)
/=============================================================================================*/
public void logMessage(int code,java.lang.String message){
     switch(code){
         case 0:  logMessage(message);  break;
         case 1:  logMessage(message);  break;
         case 2:  logMessage(message);  break;
         case 3:  logMessage(message);  break;
         case 4:  logMessage(message);  break;
     }  
     publicCommandLogModel.insertMessage(CommandLogModel.COMMAND, message);
     System.out.println(message);    
 }
/*=============================================================================================
/        logMessage(java.lang.String message)
/=============================================================================================*/
public void logMessage(java.lang.String message){
      System.out.println(message);    
 }
/*=============================================================================================
/        displayScreenMessage(java.lang.String message)
/=============================================================================================*/
public void displayScreenMessage(java.lang.String message){
    javax.swing.SwingUtilities.invokeLater(new Runnable() {
            public void run() {
                if(!myOTMErrorsLog.isVisible()){
                   myOTMErrorsLog.setVisible(true);
                }
                myOTMErrorsLog.addError(message);
//                JOptionPane.showMessageDialog(frame, message);
            }
        });   
}
/*=============================================================================================
/     INNER CLASS that executes a process in separate thread so that it doesn't
 *     block execution of the rest of the system. 
/=============================================================================================*/
public class ExecuteProcessThread  implements Runnable{
    private Thread             myThread;
    private java.sql.Timestamp  start_time;
    private double              seeing;
    private int                 wavelength;
    
    public ExecuteProcessThread(java.sql.Timestamp start_time,double seeing,int wavelength){
      this.start_time  = start_time;
      this.seeing      = seeing;
      this.wavelength  = wavelength;
    }
    public void run(){
        try{  
           myCommandLogModel.insertMessage(CommandLogModel.COMMAND, "OTM python process starting"); 
           OTM(start_time,seeing,wavelength);
           myCommandLogModel.insertMessage(CommandLogModel.COMMAND, "OTM python process complete"); 
        }catch(Exception e){
           System.out.println(e.toString());
        }
     }
    public void start(){
     myThread = new Thread(this);
     myThread.start();
     }
    }// End of the ExecuteProcessThread Inner Class
/*=============================================================================================
/     INNER CLASS that executes a process in separate thread so that it doesn't
 *     block execution of the rest of the system. 
/=============================================================================================*/
public class ExecutePlotThread  implements Runnable{
    private Thread             myThread;
    private java.sql.Timestamp  start_time;
    private double              seeing;
    private int                 wavelength;
    
    public ExecutePlotThread(java.sql.Timestamp new_start_time,double new_seeing,int new_wavelength){
        start_time = new_start_time;
        seeing     = new_seeing;
        wavelength = new_wavelength;
    }
    public void run(){
        try{  
           myCommandLogModel.insertMessage(CommandLogModel.COMMAND, "OTM_plot python process starting"); 
           OTM(start_time,seeing,wavelength);
           PLOT();
           myCommandLogModel.insertMessage(CommandLogModel.COMMAND, "OTM_plot python process complete"); 
        }catch(Exception e){
           System.out.println(e.toString());
        }
     }
    public void start(){
     myThread = new Thread(this);
     myThread.start();
     }
    }// End of the ExecuteProcessThread Inner Class
/*=============================================================================================
/     INNER CLASS that executes a process in separate thread so that it doesn't
 *     block execution of the rest of the system. 
/=============================================================================================*/
public class OutputHandler  extends LogOutputStream{
    
    public OutputHandler(){
    }
    protected void processLine(String output_line,int level){
        level = CommandLogModel.RESPONSE;        
        if(output_line.contains("ERROR")){
            level = CommandLogModel.ERROR;
            displayScreenMessage("ERROR in OTM execution. "+output_line);
        }
        myCommandLogModel.insertMessage(level, output_line);  
    }
    }// End of the ExecuteProcessThread Inner Class

/*================================================================================================
/         Main Method - Starts the entire system
/=================================================================================================*/
  //Main method
   public static void main(String[] args) {
     try {
     }
     catch(Exception e) {
        e.printStackTrace();
     }
     new OTMlauncher();
   }
}

