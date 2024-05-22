package edu.caltech.palomar.telescopes.guider.sdsstools; 
//=== File Prolog =============================================================
//	This code was developed by California Institute of Technology
//      Caltech Optical Observatories, Palomar Observatory
//
//--- Contents ----------------------------------------------------------------
//	 SDSSAutoDownloadTool- This is a tool for automatically downloading
//       a set of images from the MAST catalog of FITS images.
//
//--- Description -------------------------------------------------------------
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
//
//      January 24, 2011 Jennifer Milburn
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
import javax.swing.JFrame;
import org.apache.http.protocol.HttpContext;
import org.apache.http.protocol.BasicHttpContext;
import org.apache.http.protocol.ExecutionContext;
import org.apache.http.protocol.HttpProcessor;
import org.apache.http.protocol.HttpRequestExecutor;
import org.apache.http.protocol.ImmutableHttpProcessor;
import org.apache.http.protocol.RequestConnControl;
import org.apache.http.protocol.RequestContent;
import org.apache.http.protocol.RequestExpectContinue;
import org.apache.http.protocol.RequestTargetHost;
import org.apache.http.protocol.RequestUserAgent;
import org.apache.http.util.EntityUtils;
import java.net.Socket;
import org.apache.http.Header;
import org.apache.http.HttpEntity;
import org.apache.http.client.HttpClient;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.conn.ClientConnectionManager;
import org.apache.http.conn.scheme.PlainSocketFactory;
import org.apache.http.conn.scheme.Scheme;
import org.apache.http.conn.scheme.SchemeRegistry;
import org.apache.http.impl.client.DefaultHttpClient;
import org.apache.http.impl.conn.tsccm.ThreadSafeClientConnManager;
import org.apache.http.params.BasicHttpParams;
import org.apache.http.params.HttpParams;
import org.apache.http.params.HttpProtocolParams;
import org.apache.http.util.EntityUtils;
import org.apache.http.ConnectionReuseStrategy;
import org.apache.http.HttpHost;
import org.apache.http.HttpRequestInterceptor;
import org.apache.http.HttpResponse;
import org.apache.http.HttpVersion;
import org.apache.http.impl.DefaultConnectionReuseStrategy;
import org.apache.http.impl.DefaultHttpClientConnection;
import org.apache.http.message.BasicHttpRequest;
import org.apache.http.params.HttpParams;
import org.apache.http.params.HttpProtocolParams;
import org.apache.http.params.SyncBasicHttpParams;
import org.apache.http.protocol.HttpContext;
import org.apache.http.protocol.BasicHttpContext;
import org.apache.http.protocol.ExecutionContext;
import org.apache.http.protocol.HttpProcessor;
import org.apache.http.protocol.HttpRequestExecutor;
import org.apache.http.protocol.ImmutableHttpProcessor;
import org.apache.http.protocol.RequestConnControl;
import org.apache.http.protocol.RequestContent;
import org.apache.http.protocol.RequestExpectContinue;
import org.apache.http.protocol.RequestTargetHost;
import org.apache.http.protocol.RequestUserAgent;
import org.apache.http.util.EntityUtils;
import java.beans.*;
import java.util.Vector;
import edu.dartmouth.jskycalc.coord.HA;
import edu.dartmouth.jskycalc.coord.dec;
import edu.dartmouth.jskycalc.coord.RA;
import edu.caltech.palomar.util.general.TimeUtilities;
import java.io.File;
import java.io.FileOutputStream;
import jsky.util.FileUtil;
import java.io.OutputStream;
import java.io.IOException;
import java.io.InputStream;
import edu.caltech.palomar.telescopes.P200.gui.tables.AstroObjectsModel;
import edu.caltech.palomar.telescopes.P200.gui.tables.AstroObject;
import edu.caltech.palomar.telescopes.P200.gui.tables.AstroObjectTableModel;
import java.io.File;
import javax.swing.filechooser.FileFilter;
import java.io.*;
import javax.swing.*;
import edu.caltech.palomar.telescopes.P200.TelescopesIniReader;
import org.apache.log4j.Level;
/*================================================================================================
/        SDSSAutoDownloadTool() Constructor
/=================================================================================================*/
public class SDSSAutoDownloadTool {
  private transient PropertyChangeSupport propertyChangeListeners = new PropertyChangeSupport(this);
  private transient Vector       actionListeners;
  private JFrame                 mainFrame;
  private double                 search_box_width;
  private double                 search_box_height;
  private java.lang.String       urlString;
  private double                 ra;
  private double                 dec;
  private AstroObject            selected_astroObject;
  private java.lang.String       selected_ra                = new java.lang.String();
  private java.lang.String       selected_dec               = new java.lang.String();
  private java.lang.String       selected_proper_motion_ra  = new java.lang.String();
  private java.lang.String       selected_proper_motion_dec = new java.lang.String();
  private java.lang.String       selected_target            = new java.lang.String();
  private java.lang.String       selected_filename          = new java.lang.String();
  // HTTPClient implementation structures and objects
   HttpParams                  params;
   HttpProcessor               httpproc;
   HttpRequestExecutor         httpexecutor;
   HttpContext                 context;
   HttpHost                    host;
   DefaultHttpClientConnection conn;
   ConnectionReuseStrategy     connStrategy;
   // HTTPClient implementation 2
   SchemeRegistry              supportedSchemes;
   ClientConnectionManager     connMgr;
   DefaultHttpClient           httpclient;
  public        java.lang.String USERDIR        = System.getProperty("user.dir");
  public        java.lang.String SEP            = System.getProperty("file.separator");
  public static java.lang.String tempDir        = "temp";
  public static java.lang.String sdssDir        = "sdss";
  public static java.lang.String filePrefix     = "Finder";
  public static java.lang.String fileSuffix     = ".jpeg";
  public static java.lang.String jpeg           = ".jpeg";
  public static java.lang.String fits           = ".fits";
  private       java.lang.String downloadFileName;
  public static java.lang.String raString       = "?ra=";
  public static java.lang.String decString      = "&dec=";
  public static java.lang.String scaleString    = "&scale=";
  public static java.lang.String widthString    = "&width=";
  public static java.lang.String heightString   = "&height=";
  public static java.lang.String optString      = "&opt=I";
  public static java.lang.String queryString    = "&query=";
  public static java.lang.String GRID           = "G";
  public static java.lang.String LABEL          = "L";
  public static java.lang.String PHOTOOBJ       = "P";
  public static java.lang.String SPECOBJ        = "S";
  public static java.lang.String TARGET         = "T";
  public static java.lang.String OUTLINE        = "O";
  public static java.lang.String BOUNDING       = "B";
  public static java.lang.String FIELDS         = "F";
  public static java.lang.String MASKS          = "M";
  public static java.lang.String PLATES         = "Q";
  public static java.lang.String INVERT         = "I";
  private java.lang.String dateDirectoryString;
  public AstroObjectsModel myAstroObjectsModel;
  DownloadStatusTableModel            myAstroObjectTable;
  java.lang.String                    currentFileName          = new java.lang.String();
  java.io.File                        currentFile;
  private        java.lang.String status_message   = new java.lang.String();
  private        java.lang.String currentDirectory = new java.lang.String();
  private        RunRetrieveImages myRunRetrieveImages;
  private        long              TIMEOUT = 60*1000;
  public         SDSSGuiderImageFinderTool mySDSSGuiderImageFinderTool;
  public         TelescopesIniReader       myTelescopesIniReader    = new TelescopesIniReader();
  public static int                        PTIC                     = 500;
  public static int                        CSV                      = 600;
  private       int                        file_type;
/*================================================================================================
/        SDSSAutoDownloadTool() Constructor
/=================================================================================================*/
  public SDSSAutoDownloadTool(JFrame mainFrame){
    this.mainFrame = mainFrame;
    myAstroObjectsModel = new AstroObjectsModel(mainFrame);
    fileSuffix = fits;
    setSearchBoxWidth(15.0);
    setSearchBoxHeight(15.0);   
    setCurrentDirectory(myTelescopesIniReader.DEFAULT_OBSERVER_DIR+SEP);
    configureLogging();
  }
/*================================================================================================
/        SDSSAutoDownloadTool() Constructor
/=================================================================================================*/
  public SDSSAutoDownloadTool(){
   mainFrame = new JFrame();
   fileSuffix = fits;
   mySDSSGuiderImageFinderTool = new SDSSGuiderImageFinderTool(false);
   configureLogging();
//   Test();
  }
/*=============================================================================================
/     execute()
/=============================================================================================*/
 public void Test(){
     execute();
 }
/*=============================================================================================
/    configureLogging()
/=============================================================================================*/
 public void configureLogging(){
   org.apache.log4j.Logger.getLogger("org.apache.http.wire").setLevel(org.apache.log4j.Level.ERROR);
 }
/*=============================================================================================
/    setSDSSGuiderImageFinderTool(SDSSGuiderImageFinderTool newSDSSGuiderImageFinderTool)
/=============================================================================================*/
 public void setSDSSGuiderImageFinderTool(SDSSGuiderImageFinderTool newSDSSGuiderImageFinderTool){
    mySDSSGuiderImageFinderTool = newSDSSGuiderImageFinderTool; 
 }
 /*=============================================================================================
/    setTableModel(DownloadStatusTableModel   newAstroObjectTable)
/=============================================================================================*/
 public void setTableModel(DownloadStatusTableModel   newAstroObjectTable){
     myAstroObjectTable = newAstroObjectTable;
 }
/*=============================================================================================
/     execute()
/=============================================================================================*/
 public void execute(){
   myRunRetrieveImages = new RunRetrieveImages();
   myRunRetrieveImages.start();
 }
/*=============================================================================================
/      stop()
/=============================================================================================*/
 public void stop(){
   myRunRetrieveImages.stop();
 }
/*=============================================================================================
/     java.lang.String constructFITSImageQueryString_HTTP_GET()
/=============================================================================================*/
  public java.lang.String constructFITSImageQueryString_HTTP_GET(){
    java.lang.String currentImageQueryString = new java.lang.String();
    currentImageQueryString = "/cgi-bin/dss_search" + "?r=" + getRa() + "&d=" + getDec() + "&h=" + getSearchBoxHeight() + "&w=" + getSearchBoxWidth() +
                              "&f=FITS"+ "&v=3"+"&s=ON";
    currentImageQueryString = currentImageQueryString + optString + GRID + LABEL;
    setUrlString(currentImageQueryString);
    return currentImageQueryString;
  }
 /*=========================================================================================================
 /     setDateDirectoryString(java.lang.String newDateDirectoryString)
 /=========================================================================================================*/
public void setDateDirectoryString(java.lang.String newDateDirectoryString){
     dateDirectoryString = newDateDirectoryString;
 }
 /*=========================================================================================================
 /     getDateDirectoryString()
 /=========================================================================================================*/
  public java.lang.String getDateDirectoryString(){
     return dateDirectoryString;
  }
  /*=========================================================================================================
 /     getDateDirectoryString()
 /=========================================================================================================*/
  public void constructDateDirectory(){
      java.lang.String myDateString        = new java.lang.String();
      myDateString = mySDSSGuiderImageFinderTool.constructDateDirectory();
      setDateDirectoryString(myDateString);

      /*    java.lang.String myString        = new java.lang.String();
      TimeUtilities    myTimeUtilities = new TimeUtilities();
      java.lang.String myDateString = myTimeUtilities.constructDateTag(TimeUtilities.DATE_OBS_YEAR_STAMP);
      myString = USERDIR + SEP + tempDir + SEP + sdssDir + SEP + myDateString;
      try{
      java.io.File myDirectory = new java.io.File(myString);
      if(!myDirectory.exists()){
          myDirectory.mkdir();
      }
      if(myDirectory.exists()){
          setDateDirectoryString(myDateString);
      }
      }catch(Exception e){
          System.out.println(e.toString());
      }
*/
 }
 /*=========================================================================================================
 /     constructTemporaryFileNameRADEC()
 /=========================================================================================================*/
 public java.lang.String constructTemporaryFileNameRADEC(){
     java.lang.String tempFileName      = new java.lang.String();
     tempFileName = mySDSSGuiderImageFinderTool.constructTemporaryFileNameRADEC();
     setDownloadFileName(tempFileName);
    return tempFileName;

     /*    java.lang.String tempFileName      = new java.lang.String();
    java.lang.String coordinatesString = new java.lang.String();
    java.lang.String sizeString        = new java.lang.String();

    java.lang.String RAString          = new java.lang.String();
    java.lang.String DECString         = new java.lang.String();
    dec              currentDec        = new dec(dec);
    RA               currentRA         = new RA(ra/15.0);
                     DECString        =  currentDec.RoundedDecString(2, "_");
                     RAString          = currentRA.RoundedRAString(2, "_");
    coordinatesString = RAString + "-" + DECString;
    coordinatesString = coordinatesString.replace(".","-");
    sizeString        = (new Integer((int)getSearchBoxHeight())).toString() + "x"+(new Integer((int)getSearchBoxWidth())).toString();
    java.lang.String tempFile     = filePrefix + coordinatesString+"-"+sizeString + fits;
    tempFileName = USERDIR + SEP + tempDir + SEP + sdssDir + SEP + dateDirectoryString + SEP + tempFile;
    setDownloadFileName(tempFileName);
    return tempFileName;
*/
 }
/*=============================================================================================
/    void initializeHTTP_GET2()
/=============================================================================================*/
 public void initializeHTTP_GET(){
    host = new HttpHost("archive.stsci.edu", 80);
    // general setup
    supportedSchemes = new SchemeRegistry();
    // Register the "http" protocol scheme, it is required by the default operator to look up socket factories.
    supportedSchemes.register(new Scheme("http", PlainSocketFactory.getSocketFactory(), 80));
    // prepare parameters
    params = new BasicHttpParams();
    HttpProtocolParams.setVersion(params, HttpVersion.HTTP_1_1);
    HttpProtocolParams.setContentCharset(params, "UTF-8");
    HttpProtocolParams.setUseExpectContinue(params, true);
    connMgr = new ThreadSafeClientConnManager(params, supportedSchemes);
    httpclient = new DefaultHttpClient(connMgr, params);
 }
/*=============================================================================================
/    void executeHTTP_GET()
/=============================================================================================*/
 public void setTIMEOUT(long newTIMEOUT){
     this.TIMEOUT = newTIMEOUT;
 }
/*=============================================================================================
/    void configureTIMEOUT()
/=============================================================================================*/
 public void configureTIMEOUT(){
    double image_size = getSearchBoxWidth();
        if(image_size == 15.0){
            setTIMEOUT(30*1000);
        }
        if(image_size == 30.0){
            setTIMEOUT(60*1000);
        }
        if(image_size == 60.0){
            setTIMEOUT(100*1000);
        }   
 }
/*=============================================================================================
/    void executeHTTP_GET()
/=============================================================================================*/
 public boolean executeHTTP_GET(){
   boolean status = false;
   constructFITSImageQueryString_HTTP_GET();
   constructTemporaryFileNameRADEC();
   java.lang.String currentQueryString       = getUrlString();
//   logMessage("Current Query String = " + currentQueryString);
   HttpGet req = new HttpGet(currentQueryString);
//   System.out.println("executing request to " + host);
    try{
        HttpResponse rsp = httpclient.execute(host, req);
        HttpEntity entity = rsp.getEntity();

        if (entity != null) {
            int length = (int)entity.getContentLength();
            
            File file = new File(getDownloadFileName());
//            byte[] myData = EntityUtils.toByteArray(entity);
//            out.write(myData);
            configureTIMEOUT();
            long start = System.currentTimeMillis();
            DownloadMonitor myDownloadMonitor = new DownloadMonitor(entity,file);
            myDownloadMonitor.start();
            while(!myDownloadMonitor.finished){
               ThreadSleep(1000);
               long current = System.currentTimeMillis();
               long delta   = current - start;
               if(delta > TIMEOUT){
                   checkFileSize();
                   return status;
               }
            }
//            entity.writeTo(out);
          status = true;
        }
    } catch (Exception e) {
        status = false;
        checkFileSize();
        System.out.println("IOException occurred trying to retrieve the FITS image"+ e.toString());
    }
   return status;
 }
/*=============================================================================================
/    void ThreadSleep()
/=============================================================================================*/
  public void ThreadSleep(int SLEEP_TIME){
    try{
          Thread.currentThread().sleep(SLEEP_TIME);
      }
      catch (Exception e) {
            logMessage("An error occured while waiting for the image to be transfered. " +e.toString());
     }
  }
 /*=============================================================================================
/    void initializeHTTP_GET2()
/=============================================================================================*/
 public void finalizeHTTP_GET2(){
    httpclient.getConnectionManager().shutdown();
// When HttpClient instance is no longer needed, shut down the connection manager to ensure immediate deallocation of all system resources
 }
/*=============================================================================================
/     setRa(double ra)
/=============================================================================================*/
    public double getRa() {
      return ra;
    }
    public void setRa(double ra) {
      double  oldRa = this.ra;
      this.ra = ra;
      mySDSSGuiderImageFinderTool.setRa(ra);
      propertyChangeListeners.firePropertyChange("ra", Double.valueOf(oldRa),Double.valueOf(ra));
    }
/*=============================================================================================
/     setDec(double dec)
/=============================================================================================*/
  public double getDec() {
    return dec;
  }
  public void setDec(double dec) {
    double  oldDec = this.dec;
    this.dec = dec;
    mySDSSGuiderImageFinderTool.setDec(dec);
    propertyChangeListeners.firePropertyChange("dec", Double.valueOf(oldDec), Double.valueOf(dec));
  }
/*=============================================================================================
/     setSelectedAstroObject(AstroObject newAstroObject)
/=============================================================================================*/
public void setSelectedAstroObject(AstroObject newAstroObject){
    AstroObject old_selected_astroObject = this.selected_astroObject;
    this.selected_astroObject = newAstroObject; 
    propertyChangeListeners.firePropertyChange("astro_object", old_selected_astroObject, newAstroObject);
}
 public AstroObject getSelectedAstroObject(){
     return selected_astroObject;
 }
/*=============================================================================================
/     setSelecteRa(double ra)
/=============================================================================================*/
    public java.lang.String getSelectedRa() {
      return selected_ra;
    }
    public void setSelectedRa(java.lang.String selected_ra) {
      java.lang.String  old_selected_Ra = this.selected_ra;
      this.selected_ra = selected_ra;
      propertyChangeListeners.firePropertyChange("selected_ra", old_selected_Ra, selected_ra);
    }
/*=============================================================================================
/     setSelecteDec(java.lang.String dec)
/=============================================================================================*/
  public java.lang.String getSelecteDec() {
    return selected_dec;
  }
  public void setSelectedDec(java.lang.String new_dec) {
    java.lang.String  old_selected_dec = this.selected_dec;
    this.selected_dec = new_dec;
   propertyChangeListeners.firePropertyChange("selected_dec", old_selected_dec, selected_dec);
  }
 /*=============================================================================================
/     setSelecteRa(double ra)
/=============================================================================================*/
    public java.lang.String getSelecteProperMotiondRa() {
      return selected_proper_motion_ra;
    }
    public void setSelectedProperMotionRa(java.lang.String selected_proper_motion_ra) {
      java.lang.String  old_selected_proper_motion_Ra = this.selected_proper_motion_ra;
      this.selected_proper_motion_ra = selected_proper_motion_ra;
      propertyChangeListeners.firePropertyChange("selected_proper_motion_ra", old_selected_proper_motion_Ra, selected_proper_motion_ra);
    }
/*=============================================================================================
/     setSelecteDec(java.lang.String dec)
/=============================================================================================*/
  public java.lang.String getSelectedProperMotionDec() {
    return selected_proper_motion_dec;
  }
  public void setSelectedProperMotionDec(java.lang.String new_dec) {
    java.lang.String  old_selected_proper_motion_dec = this.selected_proper_motion_dec;
    this.selected_proper_motion_dec = new_dec;
   propertyChangeListeners.firePropertyChange("selected_proper_motion_dec", old_selected_proper_motion_dec, selected_proper_motion_dec);
  }
/*=============================================================================================
/     setSelectedTarget(java.lang.String new_selected_target)
/=============================================================================================*/
  public java.lang.String getSelectedTarget() {
    return selected_target;
  }
  public void setSelectedTarget(java.lang.String new_selected_target) {
    java.lang.String  old_selected_target = this.selected_target;
    this.selected_target = new_selected_target;
   propertyChangeListeners.firePropertyChange("selected_target", old_selected_target, new_selected_target);
  }
/*=============================================================================================
/     setSelectedFilename(java.lang.String new_selected_filename)
/=============================================================================================*/
  public java.lang.String getSelectedFilename() {
    return selected_filename;
  }
  public void setSelectedFilename(java.lang.String new_selected_filename) {
    java.lang.String  old_selected_filename = this.selected_filename;
    this.selected_filename = new_selected_filename;
   propertyChangeListeners.firePropertyChange("selected_filename", old_selected_filename, new_selected_filename);
  }      
/*=============================================================================================
/     setSearchBoxWidth(int width)
/=============================================================================================*/
  public double getSearchBoxWidth() {
    return search_box_width;
  }
  public void setSearchBoxWidth(double search_box_width) {
    double  oldsearch_box_width = this.search_box_width;
    this.search_box_width = search_box_width;
    if(mySDSSGuiderImageFinderTool != null){
     mySDSSGuiderImageFinderTool.setSearchBoxWidth(search_box_width);
    }
    propertyChangeListeners.firePropertyChange("search_box_width", Double.valueOf(oldsearch_box_width),Double.valueOf(search_box_width));
  }
/*=============================================================================================
/     setSearchBoxHeight(int search_box_height)
/=============================================================================================*/
  public double getSearchBoxHeight() {
    return search_box_height;
  }
  public void setSearchBoxHeight(double search_box_height) {
    double  oldsearch_box_height = this.search_box_height;
    this.search_box_height = search_box_height;
    if(mySDSSGuiderImageFinderTool != null){
      mySDSSGuiderImageFinderTool.setSearchBoxHeight(search_box_height);
    }
    propertyChangeListeners.firePropertyChange("search_box_height", Double.valueOf(oldsearch_box_height), Double.valueOf(search_box_height));
  }
/*=============================================================================================
/     setUrlString(String urlString)
/=============================================================================================*/
  public String getUrlString() {
    return urlString;
  }
  public void setUrlString(String urlString) {
    String  oldUrlString = this.urlString;
    this.urlString = urlString;
    propertyChangeListeners.firePropertyChange("urlString", oldUrlString, urlString);
  }
/*=============================================================================================
/     setDownloadFileName(String downloadFileName)
/=============================================================================================*/
  public String getDownloadFileName() {
    return downloadFileName;
  }
  public void setDownloadFileName(String downloadFileName) {
    String  oldDownloadFileName = this.downloadFileName;
    this.downloadFileName = downloadFileName;
    propertyChangeListeners.firePropertyChange("downloadFileName", oldDownloadFileName, downloadFileName);
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
/*=============================================================================================
/    setStatusMessage(java.lang.String new_status_message)
/=============================================================================================*/
  public void setStatusMessage(java.lang.String new_status_message){
    String  old_status_message = this.status_message;
    this.status_message = new_status_message;
    propertyChangeListeners.firePropertyChange("status_message", old_status_message, new_status_message);

  }
  public java.lang.String getStatusMessage(){
    return status_message;
  }
/*================================================================================================
/     setFileType(int new_file_type)
/=================================================================================================*/
  public void setFileType(int new_file_type) {
    int  old_file_type = this.file_type;
    this.file_type = new_file_type;
    propertyChangeListeners.firePropertyChange("file_type", Integer.valueOf(old_file_type), Integer.valueOf(new_file_type));
  }
  public int getFileType() {
    return file_type;
  }
/*=========================================================================================================
/     void logMessage(java.lang.String newMessage)
/=========================================================================================================*/
 public void logMessage(java.lang.String newMessage){
     System.out.println(newMessage);
     setStatusMessage(newMessage);
 }
/*=============================================================================================
/    void openTargetDefinitionFile()
/=============================================================================================*/
  public void openTargetDefinitionFile(){
      // Clear the table before we start opening the new file
      myAstroObjectTable.clearTable();
      if(currentFileName.contains(".csv")){
       readCSVFile();
      }
      if(currentFileName.contains(".ptic")){
       readPTICFile();
      }
      if(!currentFileName.contains(".ptic")&!currentFileName.contains(".csv")){
       readCSVFile();
      }
  } 
/*=============================================================================================
/    void openTargetDefinitionFile()
/=============================================================================================*/
  public void openTargetDefinitionFile(int fileType){
      // Clear the table before we start opening the new file
      myAstroObjectTable.clearTable();
      if(fileType == AstroObjectsModel.CSV){
        readCSVFile();
      }
      if(fileType == AstroObjectsModel.PTIC){
        readPTICFile();
      }
   }
 /*================================================================================================
/      setCurrentDirectory(java.lang.String newCurrentDirectory)
/=================================================================================================*/
 public void setCurrentDirectory(java.lang.String newCurrentDirectory){
   currentDirectory = newCurrentDirectory;
  }
 public java.lang.String getCurrentDirectory(){
   return currentDirectory;
 }
/*=============================================================================================
/    readPTICFile()
/=============================================================================================*/
 public void readPTICFile(){
      java.lang.String          ObjectName       = new java.lang.String();
      java.lang.String          RAString         = new java.lang.String();
      java.lang.String          DecString        = new java.lang.String();
      java.lang.String          RARateString     = new java.lang.String();
      java.lang.String          DecRateString    = new java.lang.String();
      java.lang.String          RateFlagString   = new java.lang.String();
      java.lang.String          CommentString    = new java.lang.String();
      java.lang.String          tempString       = new java.lang.String();
      java.lang.String          token1           = new java.lang.String();
      java.lang.String          token2           = new java.lang.String();
      java.lang.String          token3           = new java.lang.String();
      java.lang.String          EquinoxString    = new java.lang.String();

      RA                        RA;
      dec                       Dec;
      double                    RARate  = 0;
      double                    DecRate = 0;
      boolean                   RateFlag = false;
      double                    equinox = 2000;
      BufferedReader br = null;
       String st;
        try {
               FileInputStream fis = new FileInputStream(currentFileName);
               br = new BufferedReader(new InputStreamReader(fis));
          } catch (Exception e){
              System.out.printf("Problem opening selected Target Definition File for input.\n");
          }
          try {
             int index = 0;
             while((st = br.readLine()) != null) {
              try{
                   CommentString = "";
                   while(st.startsWith("!")|st.isEmpty()){
                     if(st.startsWith("!")){
                        CommentString = CommentString + st;
                     }
                     st = br.readLine();
                   }
                   ObjectName = st.substring(0,16);
                   st         = st.substring(16, st.length());

                   java.util.StringTokenizer tokenResponseString = new java.util.StringTokenizer(st," ");
                   tempString               = tokenResponseString.nextToken("pm=");
                   while(tempString.startsWith(" ")){
                       tempString = tempString.substring(1,tempString.length());  // basically strip the blank spaces from the beginning of the string before continuing
                   }
                   java.util.StringTokenizer tokenResponseString2 = new java.util.StringTokenizer(tempString," ");
                   token1                 = tokenResponseString2.nextToken(" ");
                   token2                 = tokenResponseString2.nextToken(" ");
                   token3                 = tokenResponseString2.nextToken(" ");
                   RAString               = token1 + ":" + token2 + ":" + token3;

                   token1                 = tokenResponseString2.nextToken(" ");
                   token2                 = tokenResponseString2.nextToken(" ");
                   token3                 = tokenResponseString2.nextToken(" ");
                   DecString              = token1 + ":" + token2 + ":" + token3;

                   EquinoxString          = tokenResponseString2.nextToken();
                   equinox                = (Double.valueOf(EquinoxString).doubleValue());
                   if(!tokenResponseString.hasMoreElements()){
                      RateFlag               = false;
                   }
                   if(tokenResponseString.hasMoreElements()){
                      RARateString           = tokenResponseString.nextToken(",");
                      RARateString           = RARateString.substring(3, RARateString.length());
                      DecRateString          = tokenResponseString.nextToken();
                      if(RARateString.startsWith("+")){
                         RARateString = RARateString.substring(1, RARateString.length());
                      }
                       if(DecRateString.startsWith("+")){
                         DecRateString = DecRateString.substring(1, DecRateString.length());
                      }
                      RARate                 = (Double.valueOf(RARateString)).doubleValue();
                      DecRate                = (Double.valueOf(DecRateString)).doubleValue();
                      RateFlag               = true;
                   }
                   RA                       = (new RA(RAString));
                   Dec                      = (new dec(DecString));
                   RateFlag                 = (Boolean.valueOf(RateFlagString)).booleanValue();
                   AstroObject currentAstroObject = new AstroObject(ObjectName,RA, Dec, equinox, RARate,DecRate,RateFlag,CommentString);
                   myAstroObjectTable.addRecord(currentAstroObject);
                    index++;
              }catch(Exception e2){
               System.out.println("Error parsing coordinates for a position. parseREQPOS");
              }// end of the catch block
             }// end of the while loop
          } catch (IOException e){
              System.out.printf("Problem parsing selected Target Definition File.\n" + e.toString());
          }
 }
/*=============================================================================================
/    readPTICFile()
/=============================================================================================*/
 public void readCSVFile(){
        java.lang.String          ObjectName       = new java.lang.String();
      java.lang.String          RAString         = new java.lang.String();
      java.lang.String          DecString        = new java.lang.String();
      java.lang.String          RARateString     = new java.lang.String();
      java.lang.String          DecRateString    = new java.lang.String();
      java.lang.String          RateFlagString   = new java.lang.String();
      java.lang.String          CommentString    = new java.lang.String();
      java.lang.String          tempString       = new java.lang.String();
      java.lang.String          token1           = new java.lang.String();
      java.lang.String          token2           = new java.lang.String();
      java.lang.String          token3           = new java.lang.String();
      java.lang.String          EquinoxString    = new java.lang.String();
      RA                        RA;
      dec                       Dec;
      double                    RARate  = 0;
      double                    DecRate = 0;
      boolean                   RateFlag = false;
      double                    equinox = 2000;
      BufferedReader br = null;
       String st;
        try {
               FileInputStream fis = new FileInputStream(currentFileName);
               br = new BufferedReader(new InputStreamReader(fis));
          } catch (Exception e){
              System.out.printf("Problem opening selected Target Definition File for input.\n");
          }
          try {
             int index = 0;
             while((st = br.readLine()) != null) {
              try{
                   CommentString = "";
                   while(st.startsWith("!")|st.isEmpty()){
                     if(st.startsWith("!")){
                        CommentString = CommentString + st;
                     }
                     st = br.readLine();
                   }
                   java.util.StringTokenizer tokenResponseString = new java.util.StringTokenizer(st,",");
                   ObjectName              = tokenResponseString.nextToken(",");
                   RAString                = tokenResponseString.nextToken(",");
                   DecString               = tokenResponseString.nextToken(",");
                   if(!tokenResponseString.hasMoreElements()){
                      equinox              = 2000.0;
                   }
                   if(tokenResponseString.hasMoreElements()){
                     EquinoxString         = tokenResponseString.nextToken(",");
                     equinox               = (Double.valueOf(EquinoxString).doubleValue());
                   }
                   if(tokenResponseString.hasMoreElements()){
                      // If there are more elements then read the proper motions, there MUST be PM for both RA and Dec if this is used
                      RARateString           = tokenResponseString.nextToken(",");
                      RARateString           = RARateString.substring(3, RARateString.length());
                      DecRateString          = tokenResponseString.nextToken();
                      // Now parse the RARateString to deal with the + at the beginning of the string
                      if(RARateString.startsWith("+")){
                         RARateString = RARateString.substring(1, RARateString.length());
                      }
                       if(DecRateString.startsWith("+")){
                         DecRateString = DecRateString.substring(1, DecRateString.length());
                      }
                      RARate                 = (Double.valueOf(RARateString)).doubleValue();
                      DecRate                = (Double.valueOf(DecRateString)).doubleValue();
                      RateFlag               = true;
                   }
                    RA                       = (new RA(RAString));
                   Dec                      = (new dec(DecString));
                   RateFlag                 = (Boolean.valueOf(RateFlagString)).booleanValue();
                   AstroObject currentAstroObject = new AstroObject(ObjectName,RA, Dec, equinox, RARate,DecRate,RateFlag,CommentString);
                   myAstroObjectTable.addRecord(currentAstroObject);
                   index++;
              }catch(Exception e2){
               System.out.println("Error parsing coordinates for a position. parseREQPOS");
              }// end of the catch block
             }// end of the while loop
          } catch (IOException e){
              System.out.printf("Problem parsing selected Target Definition File.\n" + e.toString());
          }
 } 
/*================================================================================================
/          selectTargetDefinitionFile(boolean newIsSAVE_AS)
/=================================================================================================*/
      public boolean selectTargetDefinitionFile(boolean newIsSAVE_AS){
        boolean   isSAVE_AS = newIsSAVE_AS;
        boolean   saveFile = true;
//                   java.lang.String initialDirectory = USERDIR;
                      JFileChooser myJFileChooser = new JFileChooser(getCurrentDirectory());
//                      myJFileChooser.addChoosableFileFilter(new PTICFileFilter());
                      java.lang.String filePath;
                      int option = myJFileChooser.showDialog(mainFrame,"Select Target Definition File");
                      if (option == JFileChooser.APPROVE_OPTION & (myJFileChooser.getSelectedFile()!=null)) {
                         try{
                            File newFile = myJFileChooser.getSelectedFile();
                            boolean fileExists = newFile.exists();
                               if(fileExists){
                                 if(isSAVE_AS){
                                   int selection = displayFileOverwriteWarning();
                                    if(selection == JOptionPane.YES_OPTION){
                                      filePath = newFile.getPath();
                                      File newTargetFile = new File(filePath);
                                      setFile(newTargetFile);
                                      setFileName(filePath);
                                      saveFile = true;
                                    }
                                    if(selection == JOptionPane.NO_OPTION){
                                      saveFile = false;
                                    }
                                  }// if isSAVE_AS
                                  if(!isSAVE_AS){
                                       filePath = newFile.getPath();
                                       File newTargetFile = new File(filePath);
                                       setFile(newTargetFile);
                                       setFileName(filePath);
                                       setCurrentDirectory(newTargetFile.getAbsolutePath());
                                       saveFile = true;
                                   }// if isSAVE_AS
                               }// end of if FileExists
                               if(!fileExists){
                                        filePath = newFile.getPath();
                                        File newTargetFile = new File(filePath);
                                        setFile(newTargetFile);
                                        setFileName(filePath);
                                        saveFile = true;
                               }// end of if FileExists
                      }catch(Exception e2){
                          System.out.println("A file I/O error occured while locating the Target Definition File. " + e2);
                      }// end of the catch statement
                   }// end of the If statement
                   if (option == JFileChooser.APPROVE_OPTION & (myJFileChooser.getSelectedFile() == null)) {
                      try{
                         File newFile = myJFileChooser.getSelectedFile();
                         boolean fileExists = newFile.exists();
                            if(!fileExists){
                                     filePath = newFile.getPath();
                                     File newTargetFile = new File(filePath);
                                     setFile(newTargetFile);
                                     setFileName(filePath);
                                     saveFile = true;
                            }// end of if FileExists
                   }catch(Exception e2){
                       System.out.println("A file I/O error occured while locating the Target Definition File. " + e2);
                   }// end of the catch statement
                }// end of the If statement
           return saveFile;
        }  // end of the select the Initialization File
/*================================================================================================
/         displayFileOverwriteWarning()
/=================================================================================================*/
    public int displayFileOverwriteWarning(){
       Object[] options = {"Replace File", "Cancel"};
       int n = JOptionPane.showOptionDialog(mainFrame,
                 "The selected file already exists. Do you wish to over-write??","Save Changes?",
                 JOptionPane.YES_NO_CANCEL_OPTION,JOptionPane.QUESTION_MESSAGE, null,options, options[1]);
       return n;
    }
/*=============================================================================================
/     doesFileAlreadyExist()
/=============================================================================================*/
 public boolean doesFileAlreadyExist(){
    boolean exists = false;
    java.lang.String currentTemporaryFileName = getDownloadFileName();
    try{
        File testFile = new File(currentTemporaryFileName);
        exists = testFile.exists();
    }catch(Exception ipwe){
      logMessage("An error occured while trying to retrieve the FITS Image from MAST.  " + ipwe.toString());
      ipwe.printStackTrace();
   }
  return exists;
 }
/*=============================================================================================
/     doesFileAlreadyExist()
/=============================================================================================*/
public boolean checkFileSize(){
    boolean complete = false;
    java.lang.String currentTemporaryFileName = getDownloadFileName();
    try{
        File testFile = new File(currentTemporaryFileName);
        long file_size = testFile.length();
        double image_size = getSearchBoxWidth();
        if(image_size == 15.0){
           if(file_size >= 1600000){
              complete = true;
              logMessage("Image Complete 15x15");
           }
        }
        if(image_size == 30.0){
           if(file_size >= 6400000){
              complete = true;
              logMessage("Image Complete 30x30");
           }
        }
        if(image_size == 60.0){
           if(file_size >= 25000000){
              complete = true;
              logMessage("Image Complete 60x60");
          }
        }
        if(!complete){
          boolean successful_delete = testFile.delete();
          logMessage("File deletion result = "+successful_delete);
        }
        System.out.println("File Size = "+ file_size);
    }catch(Exception ipwe){
      logMessage("An error occured while trying to retrieve the FITS Image from MAST.  " + ipwe.toString());
      ipwe.printStackTrace();
   }
  return complete;
}
/*=============================================================================================
/     addPropertyChangeListener(PropertyChangeListener l)
/=============================================================================================*/
  public synchronized void removePropertyChangeListener(PropertyChangeListener l) {
    propertyChangeListeners.removePropertyChangeListener(l);
  }
  public synchronized void addPropertyChangeListener(PropertyChangeListener l) {
    propertyChangeListeners.addPropertyChangeListener(l);
  }
/*=============================================================================================
/                      PTICFileFilter Inner Class
/=============================================================================================*/
    public class PTICFileFilter extends FileFilter{
      public final static String ptic = "ptic";
      public final static String PTIC = "PTIC";
      public final static String description = "PTIC Document Filter";

        public PTICFileFilter(){
        }
        public boolean accept(File f) {
            if (f.isDirectory()) {
                return true;
            }
            java.lang.String extension = getExtension(f);
            if (extension != null) {
                if (extension.equals(ptic) ||
                    extension.equals(PTIC)) {
                        return true;
                } else {
                    return false;
                }
            }
            return false;
        }
        public String getExtension(File f) {
                String ext = null;
                String s = f.getName();
                int i = s.lastIndexOf('.');

                if (i > 0 &&  i < s.length() - 1) {
                    ext = s.substring(i+1).toLowerCase();
                }
                return ext;
        }
        public java.lang.String getDescription(){
           return description;
        }
    }//////////////////////// End of the PTICFileFilter Inner Class
/*=============================================================================================
/                           End of the PTICFileFilter Inner Class
/=============================================================================================*/
/*=============================================================================================
/                      RunRetrieveImage Inner Class
/=============================================================================================*/
    public class RunRetrieveImages implements Runnable{
    private Thread myThread;
    private boolean stop;
 
    public RunRetrieveImages(){
      stop = false;
    }
    public void run(){
        constructDateDirectory();
        // Initialize the HTTPClient
        initializeHTTP_GET();
        // Now loop through the file and retrieve an image for each of the entries
        int rows = myAstroObjectTable.getRowCount();
        int  incomplete_downloads = 1;
        while(incomplete_downloads != 0){
              incomplete_downloads = 0;
        for(int i = 0; i < (rows); i++){
          long start = System.currentTimeMillis();
          AstroObject currentAstroObject = myAstroObjectTable.getRecord(i);
          currentAstroObject.download_status = DownloadStatusTableModel.NOT_DOWNLOADED;
          setRa(currentAstroObject.Alpha.degrees());
          setDec(currentAstroObject.Delta.degrees());
          if(!stop){
                constructTemporaryFileNameRADEC();
    //            logMessage("Retrieving image for "+currentAstroObject.name);
                boolean exists = doesFileAlreadyExist();
                if(exists){
                    currentAstroObject.fileName = getDownloadFileName();
                }
                currentAstroObject.download_status = DownloadStatusTableModel.DOWNLOADED;
                myAstroObjectTable.fireTableDataChanged();
                if(!exists){
                  currentAstroObject.download_status = DownloadStatusTableModel.DOWNLOADING;
                  myAstroObjectTable.fireTableDataChanged();
                  boolean success = executeHTTP_GET();
                  if(!success){
                     currentAstroObject.download_status = DownloadStatusTableModel.NOT_DOWNLOADED;
                     incomplete_downloads = incomplete_downloads + 1;
                     logMessage("Retrieval Stalled and Unsuccessful - continuing to the next image");
                     finalizeHTTP_GET2();
                     initializeHTTP_GET();
                  }
                  if(success){
                    currentAstroObject.download_status = DownloadStatusTableModel.DOWNLOADED;
                    currentAstroObject.fileName = getDownloadFileName();
                  }
                  myAstroObjectTable.fireTableDataChanged();
                 }
            }
            long end = System.currentTimeMillis();
            long delta = end - start;
            logMessage("Retrieved image for = "+currentAstroObject.name + "Transfer Time = "+delta);
        }
        logMessage("Number of incomplete downloads: " + incomplete_downloads);
        }// end of the while loop,  continues until ALL of the files are completely downloaded
        // clean up and close the http stream resources
        finalizeHTTP_GET2();
        logMessage("Image Retrieval Completed");
    }
    public void stop(){
      stop = true;
      finalizeHTTP_GET2();
      logMessage("Image Retrieval Canceled");
    }
    public void start(){
     myThread = new Thread(this);
     myThread.start();
     }
    }//// End of the RunRetrieveImage Inner Class
/*=============================================================================================
/                           End of the RunRetrieveImage Inner Class
/=============================================================================================*/
/*=============================================================================================
/                      DownloadMonitor Inner Class
/=============================================================================================*/
 // NOTE this attempt at monitoring the download progress DOES NOT WORK YET!
    public class DownloadMonitor implements Runnable{
    private Thread        myThread;
    int     POLL_RATE     = 1000;
    int     MAX_RETRYS    = 3;
    int     RETRYS        = 0;
    private HttpEntity       entity;
    private File             file;
    public  boolean          finished;

    public DownloadMonitor(HttpEntity entity,File file){
       this.entity = entity;
       this.file    = file;
    }
    public void run(){
      finished = false;
      try{
        FileOutputStream out  = new FileOutputStream(file);

        entity.writeTo(out);
        }catch(IOException ioe){
           if(file != null){
             file.delete();
           }
           logMessage("IOException writing the FITS image file"+ ioe.toString());
        }
      finished = true;
    }
    public void start(){
     myThread = new Thread(this);
     myThread.start();
     }
    }//// End of the DownloadMonitor Inner Class
/*=============================================================================================
/                           End of the DownloadMonitor Inner Class
/=============================================================================================*/
/*================================================================================================
/         Main Method - Starts the entire system
/=================================================================================================*/
  //Main method
   public static void main(String[] args) {
     try {
//           UIManager.setLookAndFeel("javax.swing.plaf.metal.MetalLookAndFeel");
     }
     catch(Exception e) {
        e.printStackTrace();
     }
     new SDSSAutoDownloadTool();
   }
/*=============================================================================================
/      SDSSImageFinderTool() constructor
/=============================================================================================*/





}
