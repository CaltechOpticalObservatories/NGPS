package edu.caltech.palomar.util.nameresolver;  
//=== File Prolog =============================================================
//	This code was developed by California Institute of Technology,
//      Calech Optical Observatories for the Palomar Observatory.
//
//--- Contents ----------------------------------------------------------------
//	NameResolver  -
//
//--- Description -------------------------------------------------------------
//	A utility class for using the CADC name resolution service that uses
//      both the SIMBAD and NED name resolution service
//
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
//
//      1/22/2011        Jennifer Milburn
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
//	In no event shall California Institute of Technology be liable for any damages, including, but not
//	limited to direct, indirect, special or consequential damages, arising out
//	of, resulting from, or in any way connected with this software, whether or
//	not based upon warranty, contract, tort or otherwise, whether or not
//	injury was sustained by persons or property or otherwise, and whether or
//	not loss was sustained from or arose out of the results of, or use of,
//	their software or services provided hereunder.
import java.io.*;
import java.net.URL;
import java.net.URLEncoder;
import java.net.URLConnection;
import java.net.UnknownHostException;
import java.net.MalformedURLException;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.Vector;
import java.util.StringTokenizer;
import java.util.NoSuchElementException;
import edu.caltech.palomar.util.simbad.CgiUtil;
import edu.dartmouth.jskycalc.coord.Celest;
/*=========================================================================================================
/   Class Declaration NameResolver
/=========================================================================================================*/
public class NameResolver {
	/**
	 * Name of the CADC host, could be passed in when instantiating the
	 * object
	**/
	private String			fServerName = "www1.cadc-ccda.hia-iha.nrc-cnrc.gc.ca";
	private String			fScriptLocation = "/NameResolver/find";
 // Server Name       =       http://www1.cadc-ccda.hia-iha.nrc-cnrc.gc.ca/
 // Script Location   =       NameResolver/find
 // Target parameter  =       ?target=object
 // Service parameter =       &service=[ned|simbad|vizier|all]
 // Format  parameter =       &format=[ascii|xml]
 // Cached  parameter =       &cached=[yes|no]
        public static String  NED    = "ned";
        public static String  SIMBAD = "simbad";
        public static String  VIZIER = "vizier";
        public static String  ASCII  = "ascii";
        public static String  XML    = "xml";
        public static String  YES    = "yes";
        public static String  NO     = "no";
        private       String  target;
        private       String  service;
        private       String  format;
        private       String  cached;
   // Parameters returned by the query
//  target=m31
//  service=NED(nedwww.ipac.caltech.edu)
//  coordsys=ICRS
//  ra=10.68469
//  dec=41.26904
//  time(ms)=524
     private String target_result;
     private String service_result;
     private String coordsys_result;
     private String ra_result;
     private String dec_result;
     private String time_result;
     public  Celest myObject = new Celest();
 /*=========================================================================================================
 /   Constructor for the NameResolver class
 /=========================================================================================================*/
 public NameResolver(){
     setService(SIMBAD);
     setFormat(ASCII);
     setCached(NO);
     Test();
 }
/*=========================================================================================================
/   Test()
/=========================================================================================================*/
 private void Test(){
    String objectName = "M51";
    searchByName(objectName);
 }
/*=========================================================================================================
/        getCelest()
/=========================================================================================================*/
 public Celest getCelest(){
     return myObject;
 }
 /*=========================================================================================================
/   Constructor for the NameResolver class
/=========================================================================================================*/
protected String createSearchByNameUrl(String objectName) {// created by Jennifer Milburn January 22, 2011 developed for
String url = "http://" + fServerName + fScriptLocation + "?target="+URLEncoder.encode(objectName)
	      + "&service="+service + "&format="+format + "&cached="+cached;
  return url;
}
/*=========================================================================================================
/   setTarget(java.lang.String newTarget)
/=========================================================================================================*/
 public void setTarget(java.lang.String newTarget){
     target = newTarget;
 }
 public java.lang.String getTarget(){
     return target;
 }
/*=========================================================================================================
/   setService(java.lang.String newService)
/=========================================================================================================*/
 public void setService(java.lang.String newService){
     service = newService;
 }
 public java.lang.String getService(){
     return service;
 }
/*=========================================================================================================
/   setFormat(java.lang.String newFormat)
/=========================================================================================================*/
 public void setFormat(java.lang.String newFormat){
     format = newFormat;
 }
 public java.lang.String getFormat(){
     return format;
 }
/*=========================================================================================================
/   setCached(java.lang.String newCached)
/=========================================================================================================*/
 public void setCached(java.lang.String newCached){
     cached = newCached;
 }
 public java.lang.String getCached(){
     return cached;
 }
/*=========================================================================================================
/    searchByName(String objectName)
/=========================================================================================================*/
  public Celest searchByName(String objectName){
     System.out.println("Searching for \"" + objectName + "\"...");
     String urlString = createSearchByNameUrl(objectName);
     System.out.println("URL String = "+urlString);
     try{
     // Retrieve contents as string buffer
     StringBuffer contents = CgiUtil.fetchUrl(urlString);
     System.out.println(contents);
     parseContents(contents.toString());

      }catch(Exception e){
          System.out.println("IOException occurred retrieving object name. " + e.toString());
      }
     return myObject;
   }
/*=========================================================================================================
/    searchByName(String objectName)
/=========================================================================================================*/
  public void parseContents(String result){
     char[] array = result.toCharArray();
     java.lang.String     TERMINATOR                = new java.lang.String("\n");
     StringTokenizer st = new StringTokenizer(result,TERMINATOR);
     target_result   = st.nextToken();
     service_result  = st.nextToken();
     coordsys_result = st.nextToken();
     ra_result       = st.nextToken();
     dec_result      = st.nextToken();
     time_result     = st.nextToken();
//  target=m31
//  service=NED(nedwww.ipac.caltech.edu)
//  coordsys=ICRS
//  ra=10.68469
//  dec=41.26904
//  time(ms)=524
   target_result   =  target_result.replace("target=", "");
   service_result  = service_result.replace("service=", "");
   coordsys_result = coordsys_result.replace("coordsys=", "");
   ra_result       = ra_result.replace("ra=", "");
   dec_result      = dec_result.replace("dec=", "");
   time_result     = time_result.replace("time(ms)=", "");
   System.out.println("RA = "+ra_result+ " DEC = "+dec_result);
   int    ndigits   = 3;
   String delimiter = ":";
   double ra = (Double.valueOf(ra_result)).doubleValue();
   double dec = (Double.valueOf(dec_result)).doubleValue();

   myObject.Alpha.setRA(ra/15);
   myObject.Delta.setDec(dec);
   ra_result  = myObject.Alpha.RoundedRAString(ndigits,  delimiter);
   dec_result = myObject.Delta.RoundedDecString(ndigits, delimiter);
   System.out.println("RA = "+ra_result+ " DEC = "+dec_result);
  }
//	protected double[] parseCoordinates(String result_string){
//
//	}
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
     new NameResolver();
   }
/*=============================================================================================
/      SDSSImageFinderTool() constructor
/=============================================================================================*/}
