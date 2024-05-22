package edu.caltech.palomar.util.sindex.client;
 
import java.beans.*;
import edu.caltech.palomar.util.simbad.SimbadClient;
import edu.caltech.palomar.util.simbad.SimbadObject;
//import edu.caltech.palomar.util.ned.NedObject;
import edu.caltech.palomar.util.ned.NedReader;
//import edu.caltech.palomar.util.ned.NedResultSet;
import edu.caltech.palomar.util.ned.AstroDatabaseException;
import edu.caltech.palomar.util.ned.NedException;
import edu.caltech.palomar.util.ned.NedCrossId;
import java.util.Enumeration;
import edu.caltech.palomar.util.sindex.SpatialSearchAreaObject;
import edu.caltech.palomar.util.sindex.SQLDisplayModel;
import edu.caltech.palomar.util.nameresolver.NameResolver;
import edu.dartmouth.jskycalc.coord.Celest;

//import edu.caltech.ipac.astro.ned.NedObject;
//import edu.caltech.ipac.astro.ned.NedResultSet;
//import edu.caltech.ipac.targetgui.net.NedNameResolver;
//=== File Prolog =============================================================
//	This code was developed by UCLA, Department of Physics and Astronomy,
//	for the Stratospheric Observatory for Infrared Astronomy (SOFIA) project.
//
//--- Contents ----------------------------------------------------------------
//	HTMIndexSearchClient  - This is a prototype search client for retrieving
//      observation files from SOFIA based on HTM Index coordinates..
//--- Description -------------------------------------------------------------
//
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
//
//      07/07/04        J. Milburn
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
//
//=== End File Prolog =========================================================
/*================================================================================================
/      class HTMIndexSearchClient
/=================================================================================================*/
public class HTMIndexSearchClient { 
  private String objectName;
  private transient PropertyChangeSupport propertyChangeListeners = new PropertyChangeSupport(this);
  private int resolutionSource;
  public static int    SIMBAD = 100;
  public static int    NED    = 200;
  private double RA;
  private double DEC;
  SpatialSearchAreaObject mySpatialSearchAreaObject = new SpatialSearchAreaObject();
  private double equinox;
  SQLDisplayModel         mySQLDisplayModel         = new SQLDisplayModel();
  private int returnCode;
  private java.lang.String DecString                = new java.lang.String();
  private java.lang.String RAString                 = new java.lang.String();
/*================================================================================================
/      class HTMIndexSearchClient
/=================================================================================================*/
  public HTMIndexSearchClient() {
    setEquinox(2000);
   }
/*================================================================================================
/    SimbadObject resolveUsingSIMBAD(java.lang.String newObjectName)
/=================================================================================================*/
   public void Test(){
     java.lang.String mytestobject = new java.lang.String();
     mytestobject = "NGC 6751";
     Celest myCelest = resolveUsingSIMBAD(mytestobject);
//     NedResultSet objects        = resolveUsingNED(mytestobject);
   }
/*================================================================================================
/     double[] resolveObject(java.lang.String newObjectName)
/=================================================================================================*/
     public double[] resolveObject(java.lang.String newObjectName){
       setObjectName(newObjectName);
       double[] coordinateArray = new double[2];
       if(getResolutionSource() == SIMBAD){
         Celest myCelest = resolveUsingSIMBAD(newObjectName);
         double myRA  = myCelest.Alpha.degrees();
         double myDec = myCelest.Delta.degrees();
         setRA(myRA);
         setDEC(myDec);
         coordinateArray[0] = myRA;
         coordinateArray[1] = myDec;
       }
       if(getResolutionSource() == NED){
//         NedResultSet   objects        = resolveUsingNED(newObjectName);
//         for (Enumeration enum1 = objects.elements(); enum1.hasMoreElements();){
//                 NedObject myNedObject = ((NedObject)enum1.nextElement());
//                 double myRA   = myNedObject.getRA();
//                 double myDec  = myNedObject.getDec();
//                 setRA(myRA);
//                 setDEC(myDec);
//                 coordinateArray[0] = myRA;
//                 coordinateArray[1] = myDec;
//         }
       }
      return coordinateArray;
     }
/*================================================================================================
/    SimbadObject resolveUsingSIMBAD(java.lang.String newObjectName)
/=================================================================================================*/
  public Celest resolveUsingSIMBAD(java.lang.String newObjectName){
    setObjectName(newObjectName);
    NameResolver myNameResolver = new NameResolver();
    Celest       myCelest = new Celest();
    try{
      myCelest =  myNameResolver.searchByName(newObjectName);
    }catch(Exception ioe){
      logMessage("Problem connecting to SIMBAD server and resolving Object Name : " + ioe.toString());
    }
    return myCelest;
  }
/*================================================================================================
/    NedResultSet resolveUsingNED(java.lang.String newObjectName)
/=================================================================================================*/
//    public NedResultSet resolveUsingNED(java.lang.String newObjectName){
//      setObjectName(newObjectName);
//      NedResultSet objects     = null;
//      NedReader    myNedReader = new NedReader();
//      try{
//          NedNameResolver nedname = new NedNameResolver(newObjectName,null);
//          objects = nedname.getNedResult(newObjectName);
//                  myNedReader.connect();
//        objects = myNedReader.searchByName(newObjectName);
//                  myNedReader.disconnect();
//      }catch(NedException nedex){
//        logMessage("A NedException Problem connecting to NED server and resolving Object Name : " + nedex.toString());
//      }catch(java.io.IOException ioe){
//        logMessage("An IOException Problem connecting to NED server and resolving Object Name : " + ioe.toString());
//      }catch(Exception e){
//        logMessage("An IOException Problem connecting to NED server and resolving Object Name : " + e.toString());
//      }
//      return objects;
//    }
/*================================================================================================
/     logMessage(java.lang.String newMessage)
/=================================================================================================*/
  public SQLDisplayModel getSQLDisplayModel(){
    return mySQLDisplayModel;
  }
/*================================================================================================
/     logMessage(java.lang.String newMessage)
/=================================================================================================*/
 public void logMessage(java.lang.String newMessage){
   System.out.println(newMessage);
 }
/*================================================================================================
/      Object Name Methods
/=================================================================================================*/
  public String getObjectName() {
    return objectName;
  }
  public void setObjectName(String objectName) {
    String  oldObjectName = this.objectName;
    this.objectName = objectName;
    propertyChangeListeners.firePropertyChange("objectName", oldObjectName, objectName);
  }
/*================================================================================================
/      Resolution Source Methods - SIMBAD or NED
/=================================================================================================*/
  public int getResolutionSource() {
    return resolutionSource;
  }
  public void setResolutionSource(int resolutionSource) {
    int  oldResolutionSource = this.resolutionSource;
    this.resolutionSource = resolutionSource;
    propertyChangeListeners.firePropertyChange("resolutionSource", Integer.valueOf(oldResolutionSource), Integer.valueOf(resolutionSource));
  }
/*================================================================================================
/     setting the Right Ascension
/=================================================================================================*/
    public double getRA() {
      return RA;
    }
    public void setRA(double RA) {
      double  oldRA = this.RA;
      this.RA = RA;
      mySpatialSearchAreaObject.setCentralRA(RA);
      propertyChangeListeners.firePropertyChange("RA", Double.valueOf(oldRA), Double.valueOf(RA));
    }
/*================================================================================================
/     setting the Right Ascension String
/=================================================================================================*/
  public java.lang.String getRAString() {
     return RAString;
  }
  public void setRAString(java.lang.String newRAString) {
     java.lang.String  oldRAString = this.RAString;
     this.RAString = newRAString;
     propertyChangeListeners.firePropertyChange("RAString", oldRAString, RAString);
  }
/*================================================================================================
/     setting the Declination String
/=================================================================================================*/
    public java.lang.String getDecString() {
       return DecString;
    }
    public void setDecString(java.lang.String newDecString) {
       java.lang.String  oldDecString = this.DecString;
       this.DecString = newDecString;
       propertyChangeListeners.firePropertyChange("DecString", oldDecString, DecString);
    }
/*================================================================================================
/     setting the Declination
/=================================================================================================*/
    public double getDEC() {
      return DEC;
    }
    public void setDEC(double DEC) {
      double  oldDEC = this.DEC;
      this.DEC = DEC;
      mySpatialSearchAreaObject.setCentralDec(DEC);
      propertyChangeListeners.firePropertyChange("DEC", Double.valueOf(oldDEC),Double.valueOf(DEC));
    }
/*================================================================================================
/     setEquinox(double equinox)
/=================================================================================================*/
    public double getEquinox() {
      return equinox;
    }
    public void setEquinox(double equinox) {
      double  oldEquinox = this.equinox;
      this.equinox = equinox;
      propertyChangeListeners.firePropertyChange("equinox", Double.valueOf(oldEquinox), Double.valueOf(equinox));
    }
/*================================================================================================
/     setReturnCode(int returnCode)
/=================================================================================================*/
    public int getReturnCode() {
      return returnCode;
    }
    public void setReturnCode(int returnCode) {
      int  oldReturnCode = this.returnCode;
      this.returnCode = returnCode;
      propertyChangeListeners.firePropertyChange("returnCode", Integer.valueOf(oldReturnCode),Integer.valueOf(returnCode));
    }
/*================================================================================================
/    SpatialSearchAreaObject getSpatialSearchAreaObject()
/=================================================================================================*/
  public SpatialSearchAreaObject getSpatialSearchAreaObject(){
    return mySpatialSearchAreaObject;
  }
/*================================================================================================
/     Property Change Listerners
/=================================================================================================*/
    public synchronized void removePropertyChangeListener(PropertyChangeListener l) {
      propertyChangeListeners.removePropertyChangeListener(l);
    }
    public synchronized void addPropertyChangeListener(PropertyChangeListener l) {
      propertyChangeListeners.addPropertyChangeListener(l);
    }
/*================================================================================================
/      Main Method - HTMIndexSearchClient()
/=================================================================================================*/
      //Main method
      public static void main(String[] args) {
        try {
        }
        catch(Exception e) {
          e.printStackTrace();
        }
        new HTMIndexSearchClient();
      }
/*================================================================================================
/      class HTMIndexSearchClient
/=================================================================================================*/
}