package edu.caltech.palomar.util.sindex.client;
import edu.caltech.palomar.util.gui.CoordSystemEquinox; 
/*================================================================================================
/      class  RADECCoordinateSystem()
/=================================================================================================*/
public class RADECCoordinateSystem  implements CoordSystemEquinox{
  public static final String  EQUATORIAL = "Equatorial";
  public static final String  GALACTIC = "Galactic";
  public static final String  ECLIPTIC = "Ecliptic";
  public static final String  B1950 = "B1950";
  public static final String  J2000 = "J2000";

  public static final String  EQUJ2000 = EQUATORIAL + " " + J2000;
  public static final String  EQUB1950 = EQUATORIAL + " " + B1950;
  public static final String  ECLJ2000 = ECLIPTIC + " " + J2000;
  public static final String  ECLB1950 = ECLIPTIC + " " + B1950;

  public static final String[]  CSYS_LIST = {EQUJ2000, EQUB1950, GALACTIC,
                                             ECLJ2000, ECLB1950};
  public java.lang.String currentCoordSystem = new java.lang.String();
  public java.lang.String currentEquinox     = new java.lang.String();
  public boolean          currentIsEquatorial = true;
/*================================================================================================
/      default constructor  RADECCoordinateSystem()
/=================================================================================================*/
  public RADECCoordinateSystem() {
  }
/*================================================================================================
/      prefered constructor  RADECCoordinateSystem()
/=================================================================================================*/
  public RADECCoordinateSystem(java.lang.String newCoordSystem,java.lang.String newEquinox,boolean newEquatorial) {
    currentCoordSystem = newCoordSystem;
    currentEquinox = newEquinox;
    currentIsEquatorial = newEquatorial;
  }
/*================================================================================================
/     Required methods for implementing the CooordSystemEquinox Interface
/=================================================================================================*/
  public void   setCoordSystem(java.lang.String newCoordSystem){
    currentCoordSystem = newCoordSystem;
  }
  public void   setEquinox(java.lang.String newEquinox){
    currentEquinox = newEquinox;
  }
  public void   setEquinox(boolean newEquatorial){
    currentIsEquatorial = newEquatorial;
  }
  public String getCoordSystem(){
     return currentCoordSystem;
  }
  public String getEquinox(){
     return currentEquinox;
  }
  public boolean isEquatorial(){
     return currentIsEquatorial;
  }
/*================================================================================================
/      class  RACoordinateSystem()
/=================================================================================================*/
}