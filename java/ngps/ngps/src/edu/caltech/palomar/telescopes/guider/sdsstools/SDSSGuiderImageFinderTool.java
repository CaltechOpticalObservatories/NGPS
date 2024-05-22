/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */ 

package edu.caltech.palomar.telescopes.guider.sdsstools;
//=== File Prolog =============================================================
//	This code was developed by UCLA, Department of Physics and Astronomy,
//	for the Stratospheric Observatory for Infrared Astronomy (SOFIA) project.
//
//--- Contents ----------------------------------------------------------------
//	SDSSImageFinderTool  -
//
//--- Description -------------------------------------------------------------
//	A utility class for obtaining an "finder guide" image from SDSS
//
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
//
//      10/05/04        J. Milburn
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
//	In no event shall UCLA be liable for any damages, including, but not
//	limited to direct, indirect, special or consequential damages, arising out
//	of, resulting from, or in any way connected with this software, whether or
//	not based upon warranty, contract, tort or otherwise, whether or not
//	injury was sustained by persons or property or otherwise, and whether or
//	not loss was sustained from or arose out of the results of, or use of,
//	their software or services provided hereunder.
//  <IMG SRC="http://casjobs.sdss.org/ImgCutout/getjpeg.aspx?ra=18.87837&dec=-0.86083&scale=0.396127&width=400&height=400&opt=GST&query=SR(10,20)">
//Parameters:
//
//      ra     center point right ascension in J2000 decimal degrees or hh:mm:ss.s
//      dec    center point declination in J2000 decimal degrees or dd:mm:ss.s
//      scale  arcsec/pixel (the natural scale of SDSS is 0.396127)
//      height image height in pixels, limited to [64..2048]
//      width  image width in pixels, limited to [64..2048]
//      opt    options string, a set of upper-case characters, like 'GPST'.
//
/*Drawing options:
      The characters present will select the corresponding option
      from the list below. Characters not in the list are ignored.

      G Grid Draw a N-S E-W grid through the center
      L Label Draw the name, scale, ra, and dec on image
      P PhotoObj Draw a small cicle around each primary photoObj
      S SpecObj Draw a small square around each specObj
      T Target Draw a small square around each Target
      O Outline Draw the outline of each photoObj
      B Bounding Box Draw the bounding box of each photoObj
      F Fields Draw the outline of each field
      M Masks Draw the outline of each mask considered to be important
      Q Plates Draw the outline of each plate
      I Invert Invert the image (B on W)
*/
/*Use query to mark objects:

      This option will draw a triangle on top of objects selected by a marking string.
      Objects must be inside the field of view of the image to be displayed.
      The format of the string can be from the following choices:

         1. List of objects. A header with RA and DEC columns must be included.
            Columns must be separated by tabs, spaces, commas or semicolons.
            The list may contain as many columns as wished.

            ObjId RA DEC RMag
            587731511532060697 18.87684 -0.860973 13.193
            587731511532060708 18.879646 -0.858836 21.078

         2. SQL SELECT query. RA and DEC columns must be included.

            SELECT top 10 p.objID, p.ra, p.dec, p.r
            FROM fGetObjFromRect(18.865,18.890,-0.875,-0.845) n, PhotoPrimary p
            WHERE n.objID=p.objID

         3. String following the pattern: ObjType Band (low_mag, high_mag)

            ObjType: S | G | P
            marks Stars, Galaxies or PhotoPrimary objects.
            Band: U | G | R | I | Z | A
            restricts marks to objects with Band BETWEEN low_mag AND high_mag
            Band 'A' will mark all objects within the specified magnitude range in any band (ORs composition).
            Examples: S
            S R (0.0, 23.5)
            G A (20, 30)
 * test
 * "http://archive.stsci.edu/cgi-bin/dss_search?ra=202.482194&dec=47.2315089&scale=1.7578125&width=60&height=60&opt=GSTI&query=SR(10,20)">
*/
//=== End File Prolog =========================================================
//import ipworks.Http;
import java.beans.*;
//import ipworks.IPWorksException;
//import ipworks.HttpEventListener;
//import ipworks.HttpConnectedEvent;
//import ipworks.HttpDisconnectedEvent;
//import ipworks.HttpEndTransferEvent;
//import ipworks.HttpErrorEvent;
//import ipworks.HttpHeaderEvent;
//import ipworks.HttpSetCookieEvent;
//import ipworks.HttpStartTransferEvent;
//import ipworks.HttpTransferEvent;
//import ipworks.HttpStatusEvent;
//import ipworks.HttpRedirectEvent;
//import ipworks.HttpConnectionStatusEvent;
import java.util.TooManyListenersException;
import java.util.Vector;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import jsky.util.gui.SwingUtil;
import jsky.util.SwingWorker;
//import jsky.image.gui.ImageHistoryList;
import java.net.URL;
import jsky.util.gui.ProgressPanel;
import jsky.util.gui.ProgressBarFilterInputStream;
import java.io.InputStream;
import com.sun.media.jai.codec.SeekableStream;
import javax.media.jai.JAI;
//import jsky.navigator.NavigatorImageDisplayControl;
import java.io.File;
import java.io.FileOutputStream;
import jsky.util.FileUtil;
import jsky.util.gui.DialogUtil;
import jsky.util.gui.ProgressException;
import javax.swing.JFrame;
import edu.dartmouth.jskycalc.coord.HA;
import edu.dartmouth.jskycalc.coord.dec;
import edu.dartmouth.jskycalc.coord.RA;
import edu.caltech.palomar.util.general.TimeUtilities;
import java.io.OutputStream;
import java.io.IOException;
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
import edu.caltech.palomar.telescopes.P200.TelescopesIniReader;
/*=============================================================================================
/     SDSSImageFinderTool() constructor
/=============================================================================================*/
public class SDSSGuiderImageFinderTool {
  private transient PropertyChangeSupport propertyChangeListeners = new PropertyChangeSupport(this);
  private transient Vector actionListeners;
  public static java.lang.String SDSSUrlString  = "http://casjobs.sdss.org/ImgCutoutDR5/getjpeg.aspx";
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
  public static double           GUIDER_SCALE_2048 = 1.7578125;
  public static double           GUIDER_SCALE_1024 = 3.515625;
  public static double           GUIDER_SCALE_512  = 7.03125;

  public static double           FFI_SCALE_1024 = 3.92578125;
  public static double           FFI_SCALE_512  = 7.8515625;
  public static double           FPI_SCALE_1024 = 0.46875;
  public static double           FPI_SCALE_512  = 0.9375;
  public static double           FLITECAM_FOV   = 480.0;
  public static int              IMAGE_2048     = 2048;
  public static int              IMAGE_512      = 512;
  public static int              IMAGE_1024     = 1024;
  public static int              FFI_1024       = 100;
  public static int              FFI_512        = 102;
  public static int              FPI_1024       = 200;
  public static int              FPI_512        = 202;
  public static int              GUIDER_2048    = 300;
  public static int              GUIDER_1024    = 301;
  public static int              GUIDER_512     = 302;

  public static int              TRANSFER_STARTED    = 302;
  public static int              TRANSFER_COMPLETED  = 304;
  public static int              TRANSFER_ERROR      = 306;
  public static int              TRANSFER_IDLE       = 308;
  public static int              CONNECTED           = 402;
  public static int              DISCONNECTED        = 404;
  public static long             SLEEP_TIME          = 100;

  private double                 ra;
  private double                 dec;
  private double                 image_center_RA;
  private double                 image_center_Dec;
  private double                 scale;
  private int                    width;
  private int                    height;
  private long                   estimated_bytes;
  private int                    progress;
  private double                 search_box_width;
  private double                 search_box_height;
  private java.lang.String       urlString;
//  public Http                    myHttp         = new Http();
  private double                 testRA         = 202.582194;
  private double                 testDEC        = 47.2315089;
  private int                    transferState;
//  private SDSSImageFinderToolMediator mySDSSImageFinderToolMediator;
  private int bytesTransfered;
  private int connectionState;
  private long start;
  private long stop;
  private long delta;
  public  static int             FITS = 100;
  public  static int             JPEG = 200;
  public  static int             FILE_RETRIEVED = 300;
//  public  NavigatorImageDisplayControl myImageDisplayControl;
  public JFrame parent;
  // JSky dependent objects
   private SwingWorker _worker;
  //Manages the list of images previously viewed
//  private ImageHistoryList _historyList;
 //The URL for the image, if one was specified (after downloading, if possible)
  private URL _url;
  // Panel used to display download progress information
  private ProgressPanel _progressPanel;
  private java.lang.String dateDirectoryString;
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
   public int                  bytesReadTotal;
   TelescopesIniReader         myTelescopesIniReader = new TelescopesIniReader();
   public java.lang.String     ARCHIVE_LOCATION = new java.lang.String();
   public java.lang.String     SDSS_IMAGE_PATH  = new java.lang.String();
/*=============================================================================================
/      SDSSGuiderImageFinderTool() constructor
/=============================================================================================*/
  public SDSSGuiderImageFinderTool(JFrame parent,boolean test) {
    this.parent = parent;
    double x = 0.0;
//    mySDSSImageFinderToolMediator = new SDSSImageFinderToolMediator();
    setSearchBoxHeight(15.0);
    setSearchBoxWidth(15.0); 
    setCurrentImageCenterRa(0.0);
    setCurrentImageCenterDec(0.0);
    ARCHIVE_LOCATION = myTelescopesIniReader.SDSS_SERVER_ADDRESS;
    SDSS_IMAGE_PATH  = myTelescopesIniReader.SDSS_IMAGE_PATH;
    constructDateDirectory();
    initializeHTTP_GET2();
    configureLogging();
    if(test){
      testFixture();
    }
  }
/*=============================================================================================
/      SDSSGuiderImageFinderTool() constructor
/=============================================================================================*/
  public SDSSGuiderImageFinderTool(boolean test) {
//    this.parent = parent;
//    mySDSSImageFinderToolMediator = new SDSSImageFinderToolMediator();
    setSearchBoxHeight(15.0);
    setSearchBoxWidth(15.0);
    setCurrentImageCenterRa(0.0);
    setCurrentImageCenterDec(0.0);
     ARCHIVE_LOCATION = myTelescopesIniReader.SDSS_SERVER_ADDRESS;
     SDSS_IMAGE_PATH  = myTelescopesIniReader.SDSS_IMAGE_PATH;
    constructDateDirectory();
    initializeHTTP_GET2();
    configureLogging();
    if(test){
      testFixture();
    }
  }
/*=============================================================================================
/   setTelescopeIniReader(TelescopesIniReader newTelescopesIniReader)
/=============================================================================================*/
 public void setTelescopeIniReader(TelescopesIniReader newTelescopesIniReader){
     myTelescopesIniReader = newTelescopesIniReader;
     ARCHIVE_LOCATION = myTelescopesIniReader.SDSS_SERVER_ADDRESS;
     SDSS_IMAGE_PATH  = myTelescopesIniReader.SDSS_IMAGE_PATH;
    constructDateDirectory();
 }
/*=============================================================================================
/    configureLogging()
/=============================================================================================*/
 public void configureLogging(){
   org.apache.log4j.Logger.getLogger("org.apache.http.wire").setLevel(org.apache.log4j.Level.FATAL);
 }
/*=============================================================================================
/   setNavigatorImageDisplayControl(NavigatorImageDisplayControl newNavigatorImageDisplayControl)
/=============================================================================================*/
// public void setNavigatorImageDisplayControl(NavigatorImageDisplayControl newNavigatorImageDisplayControl){
//    this.myImageDisplayControl = newNavigatorImageDisplayControl;
//    _historyList = new ImageHistoryList(myImageDisplayControl.getImageDisplay());
// }
/*=============================================================================================
/    void testFixture()
/=============================================================================================*/
 public void testFixture(){
   configureImageQuery(GUIDER_2048);
   setRa(testRA);
   setDec(testDEC);
//   runImageQuery(JPEG);
//   runImageQuery(FITS);
   executeFITSImageQuery_HTTP_GET();
   try{
         Thread.currentThread().sleep(3000);
     }
     catch (Exception e) {
           logMessage("An error occured while waiting for the image to be transfered. " +e.toString());
    }
 }
/*=============================================================================================
/    void executeHTTP_GET()
/=============================================================================================*/
 public void initializeHTTP_GET(){
        params = new SyncBasicHttpParams();
        HttpProtocolParams.setVersion(params, HttpVersion.HTTP_1_1);
        HttpProtocolParams.setContentCharset(params, "UTF-8");
        HttpProtocolParams.setUserAgent(params, "HttpComponents/1.1");
        HttpProtocolParams.setUseExpectContinue(params, true);

        httpproc = new ImmutableHttpProcessor(new HttpRequestInterceptor[] {
                // Required protocol interceptors
                   new RequestContent(),
                   new RequestTargetHost(),
                // Recommended protocol interceptors
                   new RequestConnControl(),
                   new RequestUserAgent(),
                   new RequestExpectContinue()
        });

        httpexecutor = new HttpRequestExecutor();

        context = new BasicHttpContext(null);
        host = new HttpHost(ARCHIVE_LOCATION, 80);

        conn = new DefaultHttpClientConnection();
        connStrategy = new DefaultConnectionReuseStrategy();

        context.setAttribute(ExecutionContext.HTTP_CONNECTION, conn);
        context.setAttribute(ExecutionContext.HTTP_TARGET_HOST, host);   
 }
/*=============================================================================================
/    void initializeHTTP_GET2()
/=============================================================================================*/
 public void initializeHTTP_GET2(){
    host = new HttpHost(ARCHIVE_LOCATION, 80);
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
 public void executeHTTP_GET2(){
   long start = System.currentTimeMillis();
   constructFITSImageQueryString_HTTP_GET();
   fileSuffix = fits;
   java.lang.String currentQueryString       = getUrlString();
   logMessage("Current Query String = " + currentQueryString);
   HttpGet req = new HttpGet(currentQueryString);
   System.out.println("executing request to " + host);
    try{
        HttpResponse rsp = httpclient.execute(host, req);
        HttpEntity entity = rsp.getEntity();

        if (entity != null) {
            int length = (int)entity.getContentLength();
            constructTemporaryFileNameRADEC();
            File             file = new File(getDownloadFileName());
            FileOutputStream out  = new FileOutputStream(file);
            InputStream      in   = entity.getContent();
            initProgressPanel();
            ProgressBarFilterInputStream in2 = _progressPanel.getLoggedInputStream(in,length);
            copy(in2, out);
            out.close();
            in2.close();
            checkFileSize();
        }

    } catch (Exception e) {
           System.out.println("IOException occurred trying to retrieve the FITS image"+ e.toString());
    }
    httpclient.getConnectionManager().shutdown();
    long end = System.currentTimeMillis();
    long delta = (end - start)/1000;
    System.out.println("Retrieve Time = "+delta+ " seconds");
// When HttpClient instance is no longer needed, shut down the connection manager to ensure immediate deallocation of all system resources
 }
/*=============================================================================================
/    void executeHTTP_GET()
/=============================================================================================*/
 public void executeHTTP_GET(){
    constructFITSImageQueryString_HTTP_GET();
    fileSuffix = fits;
    java.lang.String currentQueryString       = getUrlString();
    logMessage("Current Query String = " + currentQueryString);
    java.lang.String currentTemporaryFileName = getDownloadFileName();
    logMessage("Temporary File = " + currentTemporaryFileName);
    try {
//          String  target = new String("/cgi-bin/dss_search?r=15.0&d=45.0&h=60.0&w=60.0&f=FITS&v=3&s=ON&opt=IGL");
             try{
                    if (!conn.isOpen()) {
                        Socket socket = new Socket(host.getHostName(), host.getPort());
                        conn.bind(socket, params);
                    }
                        BasicHttpRequest request = new BasicHttpRequest("GET", currentQueryString);
                        System.out.println(">> Request URI: " + request.getRequestLine().getUri());

                        request.setParams(params);
                        httpexecutor.preProcess(request, httpproc, context);
                        HttpResponse response = httpexecutor.execute(request, conn, context);
                                     response.setParams(params);
                        httpexecutor.postProcess(response, httpproc, context);
                        int length = (int)response.getEntity().getContentLength();
 
                    constructTemporaryFileNameRADEC();
                    File             file = new File(getDownloadFileName());
                    FileOutputStream out  = new FileOutputStream(file);
                    InputStream      in   = response.getEntity().getContent();
                    initProgressPanel();
                    ProgressBarFilterInputStream in2 = _progressPanel.getLoggedInputStream(in,length);
                    copy(in2, out);
 //                   response.getEntity().writeTo(out);
                    out.close();
                    in2.close();
        } finally {
            conn.close();
        }
        } catch (Exception e) {
           System.out.println("IOException occurred trying to retrieve the FITS image"+ e.toString());
        }
 }
/*=============================================================================================
/    runImageQuery()
/=============================================================================================*/
public void estimateTransferBytes(){
  double search_box_size = search_box_width*search_box_height;
  double current_estimated_bytes = (7070.47619047619*search_box_size)+60480.0;
  estimated_bytes = (int)Math.round(current_estimated_bytes);
  setEstimatedBytes(estimated_bytes);
}
 /*=============================================================================================
/    runImageQuery()
/=============================================================================================*/
 public void runImageQuery(int type){
   RunRetrieveImage  myRunRetrieveImage = new RunRetrieveImage(type);
   myRunRetrieveImage.start();
 }
/*=============================================================================================
/     executeImageQuery()
/=============================================================================================*/
 public void executeImageQuery(){
   constructImageQueryString();
   fileSuffix = jpeg;
   constructTemporaryFileNameRADEC();
   java.lang.String currentQueryString       = getUrlString();
   logMessage("Current Query String = " + currentQueryString);
   java.lang.String currentTemporaryFileName = getDownloadFileName();
   logMessage("Temporary File = " + currentTemporaryFileName);
   try{
        URL url = new URL(currentQueryString);
        downloadImageToTempFile(url);
//     myHttp.setLocalFile(currentTemporaryFileName);
//     myHttp.get(currentQueryString);
   }catch(Exception ipwe){
      logMessage("An error occured while trying to retrieve the JPEG Image from SDSS.  " + ipwe.toString());
      ipwe.printStackTrace();
   }
 }
/*=============================================================================================
/     executeImageQuery()
/=============================================================================================*/
 public void executeFITSImageQuery(){
   constructFITSImageQueryString();
   fileSuffix = fits;
   constructTemporaryFileNameRADEC();
   java.lang.String currentQueryString       = getUrlString();
   logMessage("Current Query String = " + currentQueryString);
   java.lang.String currentTemporaryFileName = getDownloadFileName();
   logMessage("Temporary File = " + currentTemporaryFileName);
   try{
        if(!doesFileAlreadyExist()){
           URL url = new URL(currentQueryString);
           downloadImageToTempFile(url);
        }
        if(doesFileAlreadyExist()){
           transferComplete();
        }
   //  myHttp.setLocalFile(currentTemporaryFileName);
   //  myHttp.get(currentQueryString);
   }catch(Exception ipwe){
      logMessage("An error occured while trying to retrieve the FITS Image from MAST.  " + ipwe.toString());
      ipwe.printStackTrace();
   }
 }
/*=============================================================================================
/     executeImageQuery()
/=============================================================================================*/
 public void executeFITSImageQuery_HTTP_GET(){
    try{
        constructTemporaryFileNameRADEC();
        if(!doesFileAlreadyExist()){
           downloadImageToTempFile_HTTP_GET();
        }
        if(doesFileAlreadyExist()){
           transferComplete();
        }
   //  myHttp.setLocalFile(currentTemporaryFileName);
   //  myHttp.get(currentQueryString);
   }catch(Exception ipwe){
      logMessage("An error occured while trying to retrieve the FITS Image from MAST.  " + ipwe.toString());
      ipwe.printStackTrace();
   }
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
//     setDownloadState(boolean downloading)
 /  Set the state of the menubar/toolbar actions
 /  @param downloading set to true if an image is being downloaded
/=============================================================================================*/
    protected void setDownloadState(boolean downloading) {
//       if(_historyList != null){
//          _historyList.setDownloadState(downloading);
//       }
    }    /**
 /
/*=============================================================================================
/      Copy the given input stream to the given output stream.
/=============================================================================================*/
// This WAS static and changing it MAY cause problems
    public void copy(InputStream in, OutputStream out) throws IOException {
 // changed to read only 4*1024 bytes instead of 8*1024 bytes and this seems to
 // eliminate the stalling.   Jennifer W. Milburn January 6, 2011
 //     bytesReadTotal = 0;
      try{
        byte[] buffer = new byte[8 * 1024];
        while (true) {
            int bytesRead = in.read(buffer);
 //           bytesReadTotal = bytesReadTotal + bytesRead;
            if (bytesRead == -1) {
                break;
            }
            out.write(buffer, 0, bytesRead);
        }
         }catch(Exception e){
             System.out.println("Error reading from the input stream");
         }
    }
 /*=============================================================================================
/     getBytesRead()
/=============================================================================================*/
 public int getBytesRead(){
    return bytesReadTotal; 
 }
 /*=========================================================================================================
 /     downloadImageToTempFile(final URL url)
 /  Download the given URL to a temporary file in a separate thread and then
 /  display the image file when done.
 /  @param url points to the image file
 /=========================================================================================================*/
   protected void downloadImageToTempFile(final URL url) {
        //System.out.println("XXX downloadImageToTempFile: " + url);
        initProgressPanel();
        _worker = new SwingWorker() {

            String filename;

            public Object construct() {
                setDownloadState(true);
                transferStarted();
                try {
                    constructTemporaryFileNameRADEC();
                    File file = new File(getDownloadFileName());
                    ProgressBarFilterInputStream in = _progressPanel.getLoggedInputStream(url);
                    FileOutputStream out = new FileOutputStream(file);
  //                  FileUtil.copy(in, out);
                    copy(in,out);
                    in.close();
                    out.close();
                    filename = file.toString();
                } catch (Exception e) {
                    return e;
                }
                return null;
            }

            public void finished() {
                _progressPanel.stop();
                setDownloadState(false);
                _worker = null;
                Object o = getValue();
                if ((o instanceof Exception) && !(o instanceof ProgressException)) {
                    transferError();
                    DialogUtil.error((Exception) o);
                    return;
                }
                if (!_progressPanel.isInterrupted()) {
                    transferComplete();
                }
            }

        };
        _worker.start();
    }
 /*=========================================================================================================
 /   Version of the downloadImageToTempFile that uses the HTTPClient infrastructure
 /=========================================================================================================*/
   protected void downloadImageToTempFile_HTTP_GET() {
        //System.out.println("XXX downloadImageToTempFile: " + url);
       initializeHTTP_GET2();
       initProgressPanel();
        _worker = new SwingWorker() {

            String filename;

            public Object construct() {
                setDownloadState(true);
                transferStarted();
                try {
                   executeHTTP_GET2();
                } catch (Exception e) {
                    return e;
                }
                return null;
            }

            public void finished() {
                _progressPanel.stop();
                setDownloadState(false);
                _worker = null;
                Object o = getValue();
                if ((o instanceof Exception) && !(o instanceof ProgressException)) {
                    transferError();
                    DialogUtil.error((Exception) o);
                    return;
                }
                if (!_progressPanel.isInterrupted()) {
                    transferComplete();
                }
            }

        };
        _worker.start();
    }
 /*=========================================================================================================
 /   transferError()
 /=========================================================================================================*/
  public void transferError(){
    setTransferState(TRANSFER_ERROR);
    logMessage("Transfer ERROR occurred");
  }
 /*=========================================================================================================
 /    transferComplete()
 /=========================================================================================================*/
    public void transferComplete(){
    setTransferState(TRANSFER_COMPLETED);
    stop = System.currentTimeMillis();
    delta = stop - start;
    setProgress(0);
    setCurrentImageCenterRa(ra);
    setCurrentImageCenterDec(dec);
    logMessage("Transfer Completed transfer time = " + delta);
  }
 /*=========================================================================================================
 /   transferStarted()
 /=========================================================================================================*/
 public void transferStarted(){
    setTransferState(TRANSFER_STARTED);
    estimateTransferBytes();
    start = System.currentTimeMillis();
    logMessage("Transfer Started");     
 }   
 /*=========================================================================================================
 /    Make the download progress panel if needed, and display it
 /=========================================================================================================*/
    protected void initProgressPanel() {
        if (_progressPanel == null) {
            _progressPanel = ProgressPanel.makeProgressPanel("Downloading image data ...", parent);
            _progressPanel.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    if (_worker != null) {
                        _worker.interrupt();
                        _worker = null;
                        if(doesFileAlreadyExist()){
                            java.io.File currentFile = new java.io.File(getDownloadFileName());
                            currentFile.delete();
                        }
                    }
                }
            });
        }
        _progressPanel.start();
    }
 /*=========================================================================================================
 /     constructTemporaryFileName()
 /=========================================================================================================*/
 public void constructTemporaryFileName(){
    java.lang.String tempFileName = new java.lang.String();
    java.lang.String randomString = new java.lang.String();
    java.security.SecureRandom  myRandom = new java.security.SecureRandom();
    int    randomNumber    = myRandom.nextInt();
    Integer myRandomInteger = Integer.valueOf(Math.abs(randomNumber));
    randomString    = myRandomInteger.toString();

    java.lang.String tempFile     = filePrefix + randomString + fileSuffix;
    //tempFileName = USERDIR + SEP + tempDir + SEP + sdssDir + SEP + tempFile;
    tempFileName = SDSS_IMAGE_PATH + SEP + dateDirectoryString + SEP + tempFile;
    setDownloadFileName(tempFileName);
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
  public java.lang.String constructDateDirectory(){
      java.lang.String myString        = new java.lang.String();
      TimeUtilities    myTimeUtilities = new TimeUtilities();
      java.lang.String myDateString = myTimeUtilities.constructDateTag(TimeUtilities.DATE_OBS_YEAR_STAMP);
//      myString = USERDIR + SEP + tempDir + SEP + sdssDir + SEP + myDateString;
      myString = SDSS_IMAGE_PATH + SEP +  myDateString;
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
    return myDateString;
  }
 /*=========================================================================================================
 /     constructTemporaryFileNameRADEC()
 /=========================================================================================================*/
 public java.lang.String constructTemporaryFileNameRADEC(){
    java.lang.String tempFileName      = new java.lang.String();
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
    sizeString        = (Integer.valueOf((int)getSearchBoxHeight())).toString() + "x"+(Integer.valueOf((int)getSearchBoxWidth())).toString();
    java.lang.String tempFile     = filePrefix + coordinatesString+"-"+sizeString + fits;
//    tempFileName = USERDIR + SEP + tempDir + SEP + sdssDir + SEP + dateDirectoryString + SEP + tempFile;
    tempFileName = SDSS_IMAGE_PATH + SEP + dateDirectoryString + SEP + tempFile;

    setDownloadFileName(tempFileName);
    return tempFileName;
 }
/*=========================================================================================================
/     void logMessage(java.lang.String newMessage)
/=========================================================================================================*/
 public void logMessage(java.lang.String newMessage){
     System.out.println(newMessage);
 }
/*=============================================================================================
/     configureImageQuery(int imageType)
/=============================================================================================*/
 public void configureImageQuery(int imageType){
    if(imageType == FFI_1024){
       setScale(FFI_SCALE_1024);
       setWidth(IMAGE_1024);
       setHeight(IMAGE_1024);
    }
    if(imageType == FFI_512){
       setScale(FFI_SCALE_512);
       setWidth(IMAGE_512);
       setHeight(IMAGE_512);
    }
    if(imageType == FPI_1024){
       setScale(FPI_SCALE_1024);
       setWidth(IMAGE_1024);
       setHeight(IMAGE_1024);
    }
    if(imageType == FPI_512){
       setScale(FPI_SCALE_512);
       setWidth(IMAGE_512);
       setHeight(IMAGE_512);
    }
    if(imageType == GUIDER_2048){
       setScale(GUIDER_SCALE_2048);
       setWidth(IMAGE_2048);
       setHeight(IMAGE_2048);
    }
    if(imageType == GUIDER_1024){
       setScale(GUIDER_SCALE_1024);
       setWidth(IMAGE_1024);
       setHeight(IMAGE_1024);
    }
    if(imageType == GUIDER_512){
       setScale(GUIDER_SCALE_512);
       setWidth(IMAGE_512);
       setHeight(IMAGE_512);
    }
 }
/*=============================================================================================
/     java.lang.String constructImageQueryString()
/=============================================================================================*/
  public java.lang.String constructImageQueryString(){
    java.lang.String currentImageQueryString = new java.lang.String();
    currentImageQueryString = SDSSUrlString + raString + getRa() + decString + getDec() + scaleString + getScale() + widthString + getWidth() + heightString + getHeight();
    currentImageQueryString = currentImageQueryString + optString + GRID + LABEL;
    setUrlString(currentImageQueryString);
    return currentImageQueryString;
  }
/*=============================================================================================
/     java.lang.String constructImageQueryString()
/=============================================================================================*/
  public java.lang.String constructFITSImageQueryString(){
    java.lang.String currentImageQueryString = new java.lang.String();
    currentImageQueryString = "http://ARCHIVE_LOCATION/cgi-bin/dss_search" + "?r=" + getRa() + "&d=" + getDec() + "&h=" + getSearchBoxHeight() + "&w=" + getSearchBoxWidth() +
                              "&f=FITS"+ "&v=3"+"&s=ON";
    currentImageQueryString = currentImageQueryString + optString + GRID + LABEL;
    setUrlString(currentImageQueryString);
    return currentImageQueryString;
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
/     setCurrentImageCenterRa(double new_image_center_RA)
/=============================================================================================*/
// These methods store the center RA and Dec of the currently downloaded image file
    public double getCurrentImageCenterRa() {
      return image_center_RA;
    }
    public void setCurrentImageCenterRa(double new_image_center_RA) {
      double  old_image_center_RA = this.image_center_RA;
      this.image_center_RA = new_image_center_RA;
      propertyChangeListeners.firePropertyChange("image_center_RA", Double.valueOf(old_image_center_RA), Double.valueOf(new_image_center_RA));
    }
/*=============================================================================================
/     setCurrentImageCenterRa(double new_image_center_RA)
/=============================================================================================*/
// These methods store the center RA and Dec of the currently downloaded image file
    public double getCurrentImageCenterDec() {
      return image_center_Dec;
    }
    public void setCurrentImageCenterDec(double new_image_center_Dec) {
      double  old_image_center_Dec = this.image_center_Dec;
      this.image_center_Dec = new_image_center_Dec;
      propertyChangeListeners.firePropertyChange("image_center_Dec", Double.valueOf(old_image_center_Dec), Double.valueOf(new_image_center_Dec));
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
      propertyChangeListeners.firePropertyChange("ra", Double.valueOf(oldRa), Double.valueOf(ra));
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
    propertyChangeListeners.firePropertyChange("dec", Double.valueOf(oldDec), Double.valueOf(dec));
  }
/*=============================================================================================
/     setWidth(int width)
/=============================================================================================*/
  public int getWidth() {
    return width;
  }
  public void setWidth(int width) {
    int  oldWidth = this.width;
    this.width = width;
    propertyChangeListeners.firePropertyChange("width", Integer.valueOf(oldWidth),Integer.valueOf(width));
  }
/*=============================================================================================
/     setHeight(int height)
/=============================================================================================*/
  public int getHeight() {
    return height;
  }
  public void setHeight(int height) {
    int  oldHeight = this.height;
    this.height = height;
    propertyChangeListeners.firePropertyChange("height",Integer.valueOf(oldHeight),Integer.valueOf(height));
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
    propertyChangeListeners.firePropertyChange("search_box_width", Double.valueOf(oldsearch_box_width), Double.valueOf(search_box_width));
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
    propertyChangeListeners.firePropertyChange("search_box_height", Double.valueOf(oldsearch_box_height),Double.valueOf(search_box_height));
  }

/*=============================================================================================
/     setScale(double scale)
/=============================================================================================*/
  public double getScale() {
    return scale;
  }
  public void setScale(double scale) {
    double  oldScale = this.scale;
    this.scale = scale;
    propertyChangeListeners.firePropertyChange("scale", Double.valueOf(oldScale),Double.valueOf(scale));
  }
/*=============================================================================================
/     setTransferState(int transferState)
/=============================================================================================*/
  public int getTransferState() {
    return transferState;
  }
  public synchronized void setTransferState(int transferState) {
    int  oldTransferState = this.transferState;
    this.transferState = transferState;
    propertyChangeListeners.firePropertyChange("transferState", Integer.valueOf(oldTransferState),Integer.valueOf(transferState));
  }
/*=============================================================================================
/     setBytesTransfered(int bytesTransfered)
/=============================================================================================*/
  public int getBytesTransfered() {
    return bytesTransfered;
  }
  public void setBytesTransfered(int bytesTransfered) {
    int  oldBytesTransfered = this.bytesTransfered;
    this.bytesTransfered = bytesTransfered;
    propertyChangeListeners.firePropertyChange("bytesTransfered", Integer.valueOf(oldBytesTransfered),Integer.valueOf(bytesTransfered));
  }
/*=============================================================================================
/     setEstimatedBytes(int newEsitmatedBytes)
/=============================================================================================*/
  public long getEstimateBytes(){
      return estimated_bytes;
  }
  public void setEstimatedBytes(long newEsitmatedBytes){
      long oldEstimatedBytes = this.estimated_bytes;
      this.estimated_bytes  = newEsitmatedBytes;
    propertyChangeListeners.firePropertyChange("estimatedBytes", Long.valueOf(oldEstimatedBytes),Long.valueOf(estimated_bytes));
    }
/*=============================================================================================
/     setProgress(int newProgress)
/=============================================================================================*/
  public int getProgress(){
      return progress;
  }
  public void setProgress(int newProgress){
      int oldProgress = this.progress;
      this.progress  = newProgress;
    propertyChangeListeners.firePropertyChange("progress", Integer.valueOf(oldProgress),Integer.valueOf(progress));
  }
/*=============================================================================================
/     setConnectionState(int connectionState)
/=============================================================================================*/
  public int getConnectionState() {
    return connectionState;
  }
  public void setConnectionState(int connectionState) {
    int  oldConnectionState = this.connectionState;
    this.connectionState = connectionState;
    propertyChangeListeners.firePropertyChange("connectionState", Integer.valueOf(oldConnectionState), Integer.valueOf(connectionState));
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
/   ActionEvent Handling Code - used to informing registered listeners that a FITS
/      file is ready to be retrieved from the server
/  removeActionListener()
/  addActionListener()
/  fireActionPerformed()
/=============================================================================================*/
/*=============================================================================================
/     synchronized void removeActionListener(ActionListener l)
/=============================================================================================*/
  public synchronized void removeActionListener(ActionListener l) {
    if (actionListeners != null && actionListeners.contains(l)) {
      Vector v = (Vector) actionListeners.clone();
      v.removeElement(l);
      actionListeners = v;
    }
  }
/*=============================================================================================
/    synchronized void addActionListener(ActionListener l)
/=============================================================================================*/
  public synchronized void addActionListener(ActionListener l) {
    Vector v = actionListeners == null ? new Vector(2) : (Vector) actionListeners.clone();
    if (!v.contains(l)) {
      v.addElement(l);
      actionListeners = v;
    }
  }
/*=============================================================================================
/     protected void fireActionPerformed(ActionEvent e)
/=============================================================================================*/
  public void fireActionPerformed(ActionEvent e) {
    if (actionListeners != null) {
      Vector listeners = actionListeners;
      int count = listeners.size();
      for (int i = 0; i < count; i++) {
        ((ActionListener) listeners.elementAt(i)).actionPerformed(e);
      }
    }
  }
/*=============================================================================================
/    void ThreadSleep()
/=============================================================================================*/
  public void ThreadSleep(){
    try{
          Thread.currentThread().sleep(SLEEP_TIME);
      }
      catch (Exception e) {
            logMessage("An error occured while waiting for the image to be transfered. " +e.toString());
     }
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
//           if(file_size >= 6400000){
           if(file_size >= 6000000){
              complete = true;
              logMessage("Image Complete 30x30");
           }
        }
        if(image_size >= 60.0){
           if(file_size >= 24000000){
              complete = true;
              logMessage("Image Complete 60x60");
          }
        }
        if(image_size == 15.0 | image_size == 30.0 | image_size == 60.0){
         if(!complete){
          testFile.delete();
         }
        }
        System.out.println("File Size = "+ file_size);
    }catch(Exception ipwe){
      logMessage("An error occured while trying to retrieve the FITS Image from MAST.  " + ipwe.toString());
      ipwe.printStackTrace();
   }
  return complete;
}
/*=============================================================================================
/                      RunRetrieveImage Inner Class
/=============================================================================================*/
    public class RunRetrieveImage implements Runnable{
    private Thread myThread;
    private int    TYPE;
 
    public RunRetrieveImage(int type){
        this.TYPE = type;
    }
    public void run(){
       setTransferState(TRANSFER_IDLE);
       setConnectionState(DISCONNECTED);
       if(TYPE == JPEG){
          executeImageQuery();
       }
       if(TYPE == FITS){
          executeFITSImageQuery_HTTP_GET();
       }
       ThreadSleep();
       if(getConnectionState() == CONNECTED){
         while ( (getTransferState() != TRANSFER_COMPLETED) | (getTransferState() != TRANSFER_ERROR)) {
           ThreadSleep();
         }
       }
       if(getTransferState() == TRANSFER_COMPLETED){
         setTransferState(TRANSFER_IDLE);
       }
    }
    public void start(){
     myThread = new Thread(this);
     myThread.start();
     }
    }//// End of the RunRetrieveImage Inner Class
/*=============================================================================================
/                           End of the RunRetrieveImage Inner Class
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
     new SDSSGuiderImageFinderTool(true);
   }
/*=============================================================================================
/      SDSSImageFinderTool() constructor
/=============================================================================================*/
}
