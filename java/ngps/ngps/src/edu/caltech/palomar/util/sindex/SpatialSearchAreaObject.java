package edu.caltech.palomar.util.sindex; 
//=== File Prolog =============================================================
//	This code was developed by UCLA, Department of Physics and Astronomy,
//	for the Stratospheric Observatory for Infrared Astronomy (SOFIA) project.
//
//--- Contents ----------------------------------------------------------------
//	SpatialSearchAreaObject  - This class defines the parameters of a
//          spatial search area.
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
import java.sql.SQLException;
//import com.informix.jdbcx.IfxDataSource;
//import com.informix.jdbcx.IfxConnectionPoolDataSource;
//import com.informix.jdbcx.IfxConnectionPoolDataSourceFactory;
//import com.informix.jdbcx.IfxConnectionPoolManager;
//import com.informix.jdbcx.IfxDataSourceFactory;
//import com.informix.jdbcx.IfxPooledConnection;
//import com.informix.jdbcx.IfxConnectionEventListener;
// com.informix.jdbc.IfmxResultSet;
//import com.informix.jdbc.IfmxPreparedStatement;
//import com.informix.jdbc.IfxConnection;
//import com.informix.jdbc.IfxDate;
//import com.informix.jdbc.IfxDateTime;
//import com.informix.jdbc.IfxPreparedStatement;
//import com.informix.jdbc.IfxResultSet;
import javax.sql.ConnectionPoolDataSource;
import javax.sql.PooledConnection;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.CallableStatement;
import edu.caltech.palomar.util.sindex.HTMTrixelTableModel;
import edu.caltech.palomar.util.sindex.TriangleDataModel;
import edu.jhu.htm.core.Convex;
import edu.jhu.htm.core.HTMindexImp;
import edu.jhu.htm.core.HTMrangeIterator;
import edu.jhu.htm.core.HTMrange;
import edu.jhu.htm.core.Vector3d;
import java.beans.*;
/*================================================================================================
/      class SpatialSearchAreaObject
/=================================================================================================*/
public class SpatialSearchAreaObject {
  private transient PropertyChangeSupport propertyChangeListeners = new PropertyChangeSupport(this);
  public static double DEFAULT_SEARCH_WIDTH  = 900;
  public static double DEFAULT_SEARCH_HEIGHT = 900;
  private  double   searchWidth = DEFAULT_SEARCH_WIDTH;  // default search width of 15 arcminutes
  private  double   searchHeight= DEFAULT_SEARCH_HEIGHT;  // default search height of 15 arcminutes
  public   HTMTrixelTableModel myHTMTrixelTableModel = new HTMTrixelTableModel();
  public static final java.lang.String spatialQueryPreparedStatementString  = "SELECT * FROM telescopetable WHERE htmindex IN (";
  private  double   centralRA  = 0.0;
  private  double   centralDec = 0.0;
  private  Vector3d centralVector;
  private  Vector3d upperLeftVector;
  private  Vector3d upperRightVector;
  private  Vector3d lowerRightVector;
  private  Vector3d lowerLeftVector;
  public   int      indexDepth     = 8;
  public   HTMindexImp indexImplementation;
  private  int         rangeNumber;
  private  int         triangleNumber;
  private  Connection  myConnection;
  private String SQLString;
/*================================================================================================
/      constructor SpatialSearchAreaObject
/=================================================================================================*/
  public SpatialSearchAreaObject() {
      setHTMIndexDepth(indexDepth);
  }
/*================================================================================================
/      constructor SpatialSearchAreaObject
/=================================================================================================*/
  public SpatialSearchAreaObject(int newIndexDepth) {
    indexDepth           = newIndexDepth;
  }
/*================================================================================================
/     constructDomainVectors()
/=================================================================================================*/
  public void constructDomainVectors(){
    double centerRa           = getCentralRA();
    double centerDec          = getCentralDec();
    double searchWidthOffset  = ((getSearchWidth()/2.0)/3600.0);
    double searchHeightOffset = ((getSearchHeight()/2.0)/3600.0);
    double ra1  = centerRa  - searchWidthOffset;
    double dec1 = centerDec + searchHeightOffset;
    double ra2  = centerRa  + searchWidthOffset;
    double dec2 = centerDec + searchHeightOffset;
    double ra3  = centerRa  + searchWidthOffset;
    double dec3 = centerDec - searchHeightOffset;
    double ra4  = centerRa  - searchWidthOffset;
    double dec4 = centerDec - searchHeightOffset;
     upperLeftVector  = new Vector3d(ra1, dec1); // initialize Vector3d from ra,dec
     upperRightVector = new Vector3d(ra2, dec2); // initialize Vector3d from ra,dec
     lowerRightVector = new Vector3d(ra3, dec3); // initialize Vector3d from ra,dec
     lowerLeftVector  = new Vector3d(ra4, dec4); // initialize Vector3d from ra,dec
  }
/*================================================================================================
/     constructHTMList()
/=================================================================================================*/
  public void constructHTMList(){
      constructDomainVectors();
      boolean varlen   = false;
      boolean compress = true;
      int     nranges  = 100;
      boolean expand   = true;
      boolean symb     = true;
      indexImplementation  = new HTMindexImp(getHTMIndexDepth(),5);
      Convex   rectangle = new Convex(upperLeftVector,upperRightVector,lowerRightVector,lowerLeftVector);
               rectangle.setOlevel(getHTMIndexDepth());
      HTMrange htmRange  = new HTMrange();
               rectangle.intersect(indexImplementation,htmRange,false);
               int numberOfRanges = htmRange.nranges();
               setRangeNumber(numberOfRanges);
//     HTMrange  completeRange = new HTMrange();
     try{
        HTMrangeIterator myHTMrangeIterator = new HTMrangeIterator(htmRange,symb);
            if (!varlen) {
              htmRange.defrag();
              if (compress) {
                htmRange.defrag(nranges);
              }
            }
      myHTMTrixelTableModel.clearTable();
//      for (int i = 0; i < numberOfRanges; i++) {
//        completeRange.addRange(htmRange.los[i],htmRange.highs[i]);
//      }
      htmRange.reset();
      while(myHTMrangeIterator.hasNext()){
         java.lang.String  currentIDString          = (String)myHTMrangeIterator.next();
         TriangleDataModel currentTriangleDataModel = new TriangleDataModel(currentIDString);
         myHTMTrixelTableModel.addRecord(currentTriangleDataModel);
      }
      int newTriangleNumber = myHTMTrixelTableModel.getRowCount();
      setTriangleNumber(newTriangleNumber);
      }catch(Exception e){
        System.out.println(e.toString());
      }
    }
/*================================================================================================
/    java.lang.String constructHTMQueryString()-
/=================================================================================================*/
  public java.lang.String constructHTMQueryString(){
//    SELECT  observationkey FROM telescopetable
//    WHERE   htmindex IN (trixel1,trixel2,trixel3,trixelN)
   java.lang.String htmQueryString =  spatialQueryPreparedStatementString;
   try{
     int rowCount = myHTMTrixelTableModel.getRowCount();
     java.lang.String currentTrixelList = new java.lang.String();
     for(int i=0;i < (rowCount);i++){
        TriangleDataModel myTriangleDataModel = (TriangleDataModel)myHTMTrixelTableModel.getRecord(i);
        long              myIndex             =  myTriangleDataModel.getTriangleID();
        currentTrixelList = currentTrixelList + (Long.valueOf(myIndex)).toString() + ",";
     }
     int    stringLength = currentTrixelList.length();
     currentTrixelList = currentTrixelList.substring(0,stringLength-1);
     currentTrixelList = currentTrixelList + ")";
     htmQueryString    = htmQueryString + currentTrixelList;
    }catch(Exception e){
      System.out.println(e.toString());
    }
    setSQLString(htmQueryString);
   return htmQueryString;
  }
/*=========================================================================================================
/     void logMessage(java.lang.String newMessage)
/=========================================================================================================*/
   public void logMessage(java.lang.String newMessage){
        System.out.println(newMessage);
   }
/*================================================================================================
/     int getHTMIndexDepth()
/=================================================================================================*/
    public int getHTMIndexDepth(){
      return indexDepth;
    }
/*================================================================================================
/     int getHTMIndexDepth()
/=================================================================================================*/
  public void setHTMIndexDepth(int newIndexDepth){
     indexDepth = newIndexDepth;
  }
/*================================================================================================
/     HTMTrixelTableModel getTableModel()
/=================================================================================================*/
  public HTMTrixelTableModel getTableModel(){
    return myHTMTrixelTableModel;
  }
/*================================================================================================
/      class SpatialSearchAreaObject
/=================================================================================================*/
  public double getSearchWidth() {
    return searchWidth;
  }
  public void setSearchWidth(double searchWidth) {
    double  oldSearchWidth = this.searchWidth;
    this.searchWidth = searchWidth;
    propertyChangeListeners.firePropertyChange("searchWidth", Double.valueOf(oldSearchWidth),Double.valueOf(searchWidth));
  }
/*================================================================================================
/      class SpatialSearchAreaObject
/=================================================================================================*/
  public double getSearchHeight() {
    return searchHeight;
  }
  public void setSearchHeight(double searchHeight) {
    double  oldSearchHeight = this.searchHeight;
    this.searchHeight = searchHeight;
    propertyChangeListeners.firePropertyChange("searchHeight", Double.valueOf(oldSearchHeight), Double.valueOf(searchHeight));
  }
/*================================================================================================
/      setCentralRA(double centralRA)
/=================================================================================================*/
  public double getCentralRA() {
    return centralRA;
  }
  public void setCentralRA(double centralRA) {
    double  oldCentralRA = this.centralRA;
    this.centralRA = centralRA;
    propertyChangeListeners.firePropertyChange("centralRA", Double.valueOf(oldCentralRA),Double.valueOf(centralRA));
  }
/*================================================================================================
/      setCentralDec(double centralDec)
/=================================================================================================*/
  public double getCentralDec() {
    return centralDec;
  }
  public void setCentralDec(double centralDec) {
    double  oldCentralDec = this.centralDec;
    this.centralDec = centralDec;
    propertyChangeListeners.firePropertyChange("centralDec", Double.valueOf(oldCentralDec),Double.valueOf(centralDec));
  }
/*================================================================================================
/     setRangeNumber(int rangeNumber)
/=================================================================================================*/
    public int getRangeNumber() {
      return rangeNumber;
    }
    public void setRangeNumber(int rangeNumber) {
      int  oldRangeNumber = this.rangeNumber;
      this.rangeNumber = rangeNumber;
      propertyChangeListeners.firePropertyChange("rangeNumber", Integer.valueOf(oldRangeNumber),Integer.valueOf(rangeNumber));
    }
/*================================================================================================
/     setTriangleNumber(int triangleNumber)
/=================================================================================================*/
  public int getTriangleNumber() {
      return triangleNumber;
  }
   public void setTriangleNumber(int triangleNumber) {
        int  oldTriangleNumber = this.triangleNumber;
        this.triangleNumber = triangleNumber;
        propertyChangeListeners.firePropertyChange("triangleNumber", Integer.valueOf(oldTriangleNumber),Integer.valueOf(triangleNumber));
  }
/*================================================================================================
/     setSQLString(String SQLString)
/=================================================================================================*/
  public String getSQLString() {
    return SQLString;
  }
  public void setSQLString(String SQLString) {
    String  oldSQLString = this.SQLString;
    this.SQLString = SQLString;
    propertyChangeListeners.firePropertyChange("SQLString", oldSQLString, SQLString);
  }

/*================================================================================================
/      propertyChangeListener Methods
/=================================================================================================*/
      public synchronized void removePropertyChangeListener(PropertyChangeListener l) {
        propertyChangeListeners.removePropertyChangeListener(l);
      }
      public synchronized void addPropertyChangeListener(PropertyChangeListener l) {
        propertyChangeListeners.addPropertyChangeListener(l);
      }
/*================================================================================================
/    End of the class SpatialSearchAreaObject
/=================================================================================================*/
}