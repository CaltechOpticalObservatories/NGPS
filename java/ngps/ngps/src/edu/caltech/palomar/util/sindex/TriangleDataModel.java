package edu.caltech.palomar.util.sindex;
import jsky.science.Coordinates;
import jsky.science.CoordinatesOffset;
import edu.jhu.htm.core.*;
import jsky.coords.WCSTransform;
import java.io.*;
import java.util.*;
import jsky.coords.DMS;
import jsky.coords.HMS; 
//=== File Prolog =============================================================
//	This code was developed by UCLA, Department of Physics and Astronomy,
//	for the Stratospheric Observatory for Infrared Astronomy (SOFIA) project.
//
//--- Contents ----------------------------------------------------------------
//	TriangleDataModel  -  A comprehensive description of an HTM Triangle
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
/     TriangleDataModel Inner Class
/=================================================================================================*/
    public class TriangleDataModel  extends java.lang.Object {
    int                 columns      = 10;
    public static int   DEFAULT_INDEX_LEVEL = 8;
    java.lang.Long      triangleIDObject;                       // Column 1
    java.lang.String    nodeName     = new java.lang.String();  // Column 2
    java.lang.Double    raObject;                               // Column 3
    java.lang.Double    decObject;                              // Column 4
    java.lang.String    raString     = new java.lang.String();  // Column 5
    java.lang.String    decString    = new java.lang.String();  // Column 6
    java.lang.Double    xObject;                                // Column 7
    java.lang.Double    yObject;                                // Column 8
    java.lang.Double    zObject;                                // Column 9
    java.lang.Double    triangleAreaObject;                     // Column 10
    double              ra;
    double              dec;
    double              x;
    double              y;
    double              z;
    long                triangleID;
    int                 indexLevel  = DEFAULT_INDEX_LEVEL;
    double              triangleArea;
    Vector3d            centralVector;
//    HTMindexImp         myHTMindexImp;
    double              triangleAreaFactor = 1000000;
/*=========================================================================================================
/     TriangleDataModel() Public constructor
/=========================================================================================================*/
   public TriangleDataModel(){
   }
/*=========================================================================================================
/     TriangleDataModel(java.lang.String newNodeName) Public constructor
/=========================================================================================================*/
   public TriangleDataModel(java.lang.String newNodeName){
       setNodeName(newNodeName);
       try{
         long     newID         = HTMfunc.nameToId(newNodeName);
         setTriangleID(newID);
         setTriangleIDObject(Long.valueOf(newID));
         double[] point         = HTMfunc.idToPoint(newID);
                  centralVector = new Vector3d(point[0],point[1],point[2]);
         double   newRa         = centralVector.ra();
         double   newDec        = centralVector.dec();
         double   newX          = centralVector.x();
         double   newY          = centralVector.y();
         double   newZ          = centralVector.z();
         setRA(newRa);
         setDec(newDec);
         setX(newX);
         setY(newY);
         setZ(newZ);
         setRAObject(Double.valueOf(newRa));
         setDecObject(Double.valueOf(newDec));
         setXObject(Double.valueOf(newX));
         setYObject(Double.valueOf(newY));
         setZObject(Double.valueOf(newZ));
         DMS myDMS = new DMS(newDec);
         HMS myHMS = new HMS(newRa);
         java.lang.String newDecString = myDMS.toString();
         java.lang.String newRaString  = myHMS.toString();
         setDecString(newDecString);
         setRAString(newRaString);
         HTMindexImp myHTMindexImp = new HTMindexImp(indexLevel);
         double newArea = myHTMindexImp.area(newNodeName);
         double newArea2 = newArea * triangleAreaFactor;
         setTriangleArea(newArea2);
         setTriangleAreaObject(Double.valueOf(newArea2));
         printDescription();
       }catch(HTMException htme){
          logMessage("An HTM Index Exception occured in the constructor 1 :  " + htme.toString());
       }
   }
/*=========================================================================================================
/     TriangleDataModel(long newNodeID) Public constructor
/=========================================================================================================*/
   public TriangleDataModel(long newNodeID){
     setTriangleID(newNodeID);
     setTriangleIDObject(Long.valueOf(newNodeID));
     try{
       java.lang.String newNodeName  = HTMfunc.idToName(newNodeID);
       setNodeName(newNodeName);
       double[] point         = HTMfunc.idToPoint(newNodeID);
                centralVector = new Vector3d(point[0],point[1],point[2]);
       double   newRa         = centralVector.ra();
       double   newDec        = centralVector.dec();
       double   newX          = centralVector.x();
       double   newY          = centralVector.y();
       double   newZ          = centralVector.z();
       setRA(newRa);
       setDec(newDec);
       setX(newX);
       setY(newY);
       setZ(newZ);
       setRAObject(Double.valueOf(newRa));
       setDecObject(Double.valueOf(newDec));
       setXObject(Double.valueOf(newX));
       setYObject(Double.valueOf(newY));
       setZObject(Double.valueOf(newZ));
       DMS myDMS = new DMS(newDec);
       HMS myHMS = new HMS(newRa);
       java.lang.String newDecString = myDMS.toString();
       java.lang.String newRaString  = myHMS.toString();
       setDecString(newDecString);
       setRAString(newRaString);
       HTMindexImp myHTMindexImp = new HTMindexImp(indexLevel);
       double newArea = myHTMindexImp.area(newNodeName);
       double newArea2 = newArea * triangleAreaFactor;
       setTriangleArea(newArea2);
       setTriangleAreaObject(Double.valueOf(newArea2));
       printDescription();
     }catch(HTMException htme){
        logMessage("An HTM Index Exception occured in the constructor 2 :  " + htme.toString());
     }
   }
/*=========================================================================================================
/     TriangleDataModel(double newRA,double newDec) Public constructor
/=========================================================================================================*/
   public TriangleDataModel(double newRa,double newDec,int newIndexLevel){
     setRA(newRa);
     setDec(newDec);
     setRAObject(Double.valueOf(newRa));
     setDecObject(Double.valueOf(newDec));
     DMS myDMS = new DMS(newDec);
     HMS myHMS = new HMS(newRa);
     java.lang.String newDecString = myDMS.toString();
     java.lang.String newRaString  = myHMS.toString();
     setDecString(newDecString);
     setRAString(newRaString);
     try{
       HTMindexImp myHTMindexImp = new HTMindexImp(newIndexLevel);
       long             newNodeID    = myHTMindexImp.lookupId(newRa,newDec);
       java.lang.String newNodeName  = myHTMindexImp.lookup(newRa,newDec);
       setNodeName(newNodeName);
       setTriangleID(newNodeID);
       setTriangleIDObject(Long.valueOf(newNodeID));
       double[] point         = HTMfunc.idToPoint(newNodeID);
                centralVector = new Vector3d(point[0],point[1],point[2]);
       double   newX          = centralVector.x();
       double   newY          = centralVector.y();
       double   newZ          = centralVector.z();
       setX(newX);
       setY(newY);
       setZ(newZ);
       setXObject(Double.valueOf(newX));
       setYObject(Double.valueOf(newY));
       setZObject(Double.valueOf(newZ));
       double newArea = myHTMindexImp.area(newNodeName);
       double newArea2 = newArea * triangleAreaFactor;
       setTriangleArea(newArea2);
       setTriangleAreaObject(Double.valueOf(newArea2));
       printDescription();
     }catch(HTMException htme){
        logMessage("An HTM Index Exception occured in the constructor 3 :  " + htme.toString());
     }
   }
/*=========================================================================================================
/     void logMessage(java.lang.String newMessage)
/=========================================================================================================*/
  public void logMessage(java.lang.String newMessage){
    System.out.println(newMessage);
  }
/*=========================================================================================================
/   setTriangleIDObject(java.lang.Integer newTriangleIDObject) and getTriangleIDObject()
/=========================================================================================================*/
   public void setTriangleIDObject(java.lang.Long newTriangleIDObject){
      triangleIDObject = newTriangleIDObject;
   }
   public java.lang.Long getTriangleIDObject(){
     return triangleIDObject;
   }
/*=========================================================================================================
/   setTriangleIDObject(int newTriangleID) and getTriangleID()
/=========================================================================================================*/
  public void setTriangleID(long newTriangleID){
         triangleID = newTriangleID;
  }
  public long getTriangleID(){
        return triangleID;
  }
/*=========================================================================================================
/   setNodeName(java.lang.String newNodeName) and getNodeName()
/=========================================================================================================*/
     public void setNodeName(java.lang.String newNodeName){
        nodeName = newNodeName;
     }
     public java.lang.String getNodeName(){
       return nodeName;
}
/*=========================================================================================================
/   setRA(double newRA) and getRA()
/=========================================================================================================*/
    public void setRA(double newRA){
      ra = newRA;
    }
    public double getRA(){
      return ra;
    }
/*=========================================================================================================
/   setRAObject(java.lang.Double newRAObject) and getRAObject()
/=========================================================================================================*/
    public void setRAObject(java.lang.Double newRAObject){
        raObject = newRAObject;
    }
    public java.lang.Double getRAObject(){
       return raObject;
    }
/*=========================================================================================================
/   setRAString(java.lang.String newRAString) and getRAString()
/=========================================================================================================*/
    public void setRAString(java.lang.String newRAString){
      raString = newRAString;
    }
    public java.lang.String getRAString(){
      return raString;
    }
/*=========================================================================================================
/   setDec(double newDec) and getDec()
/=========================================================================================================*/
    public void setDec(double newDec){
      dec = newDec;
    }
    public double getDec(){
      return dec;
    }
/*=========================================================================================================
/   setDecObject(java.lang.Double newDecObject) and getDecObject()
/=========================================================================================================*/
    public void setDecObject(java.lang.Double newDecObject){
      decObject = newDecObject;
    }
    public java.lang.Double getDecObject(){
      return decObject;
    }
/*=========================================================================================================
/   setDecString(java.lang.String newDecString) and getDecString()
/=========================================================================================================*/
    public void setDecString(java.lang.String newDecString){
      decString = newDecString;
    }
    public java.lang.String getDecString(){
       return decString;
    }
/*=========================================================================================================
/   setX(double newX) and getX()
/=========================================================================================================*/
  public void setX(double newX){
   x = newX;
  }
  public double getX(){
   return x;
  }
/*=========================================================================================================
/   setXObject(java.lang.Double newXObject) and getXObject()
/=========================================================================================================*/
  public void setXObject(java.lang.Double newXObject){
   xObject = newXObject;
  }
  public java.lang.Double getXObject(){
    return xObject;
  }
/*=========================================================================================================
/   setY(double newY) and getY()
/=========================================================================================================*/
  public void setY(double newY){
    y = newY;
  }
  public double getY(){
    return y;
  }
/*=========================================================================================================
/   setYObject(java.lang.Double newYObject) and getYObject()
/=========================================================================================================*/
  public void setYObject(java.lang.Double newYObject){
   yObject = newYObject;
  }
  public java.lang.Double getYObject(){
    return yObject;
  }
/*=========================================================================================================
/   setZ(double newZ) and getZ()
/=========================================================================================================*/
  public void setZ(double newZ){
    z = newZ;
  }
  public double getZ(){
    return z;
  }
/*=========================================================================================================
/   setZObject(java.lang.Double newZObject) and getZObject()
/=========================================================================================================*/
  public void setZObject(java.lang.Double newZObject){
    zObject = newZObject;
  }
  public java.lang.Double getZObject(){
    return zObject;
  }
/*=========================================================================================================
/   setTriangleArea(double newTriangleArea) and getTriangleArea()
/=========================================================================================================*/
    public void setTriangleArea(double newTriangleArea){
      triangleArea = newTriangleArea;
    }
    public double getTriangleArea(){
      return triangleArea;
    }
/*=========================================================================================================
/   setTriangleAreaObject(java.lang.Double newTriangleAreaObject) and getTriangleAreaObject()
/=========================================================================================================*/
    public void setTriangleAreaObject(java.lang.Double newTriangleAreaObject){
      triangleAreaObject = newTriangleAreaObject;
    }
    public java.lang.Double getTriangleAreaObject(){
      return triangleAreaObject;
    }
/*=========================================================================================================
/   getCentralVector()
/=========================================================================================================*/
  public Vector3d getCentralVector(){
    return centralVector;
  }
/*=========================================================================================================
/   printDescription()
/=========================================================================================================*/
  public void printDescription(){
    java.lang.String record = new java.lang.String();
    record = getTriangleID() + "," + getNodeName() + "," + getRA() + "," + getDec() + "," + getX() + "," + getY() + "," + getZ() + "," + getTriangleArea();
    System.out.println(record);
  }
/*=========================================================================================================
/    setValue Methods
/=========================================================================================================*/
      public void setValue(int column,java.lang.Object value){
        if(column == 0){
           triangleIDObject = ((java.lang.Long)(value));
           setTriangleID(triangleIDObject.longValue());
        }
        if(column == 1){
           nodeName = ((java.lang.String)(value));
        }
        if(column == 2){
           raObject = ((java.lang.Double)(value));
           setRA(raObject.doubleValue());
           HMS myHMS = new HMS(raObject.doubleValue());
           java.lang.String newRaString  = myHMS.toString();
           setRAString(newRaString);
        }
        if(column == 3){
           decObject = ((java.lang.Double)(value));
           setDec(decObject.doubleValue());
           DMS myDMS = new DMS(decObject.doubleValue());
           java.lang.String newDecString  = myDMS.toString();
           setDecString(newDecString);
        }
        if(column == 4){
           raString = ((java.lang.String)(value));
           HMS myHMS = new HMS(raString);
           double myNewRa = myHMS.getVal();
           setRA(myNewRa);
           setRAObject(Double.valueOf(myNewRa));
        }
        if(column == 5){
           decString = ((java.lang.String)(value));
           DMS myDMS = new DMS(decString);
           double myNewDec = myDMS.getVal();
           setDec(myNewDec);
           setDecObject(Double.valueOf(myNewDec));
        }
        if(column == 6){
           xObject = ((java.lang.Double)(value));
           setX(xObject.doubleValue());
           setXObject(xObject);
        }
        if(column == 7){
           yObject = ((java.lang.Double)(value));
           setY(yObject.doubleValue());
           setYObject(yObject);
        }
        if(column == 8){
           zObject = ((java.lang.Double)(value));
           setZ(zObject.doubleValue());
           setZObject(zObject);
        }
        if(column == 9){
           triangleAreaObject = ((java.lang.Double)(value));
           setTriangleArea(triangleAreaObject.doubleValue());
        }
      }
/*=========================================================================================================
/    getValue Methods
/=========================================================================================================*/
      public java.lang.Object getValue(int column){
        java.lang.Object returnObject = null;
        if(column == 0){
            returnObject = triangleIDObject;
        }
        if(column == 1){
            returnObject = nodeName;
        }
        if(column == 2){
           returnObject = raObject;
        }
        if(column == 3){
           returnObject = decObject;
        }
        if(column == 4){
           returnObject = raString;
        }
        if(column == 5){
          returnObject = decString;
        }
        if(column == 6){
          returnObject = xObject;
        }
        if(column == 7){
           returnObject = yObject;
        }
        if(column == 8){
          returnObject = zObject;
        }
        if(column == 9){
          returnObject = triangleAreaObject;
        }
      return returnObject;
    }
/*=========================================================================================================
/       End of the TriangleDataModel Class
/=========================================================================================================*/
   }
