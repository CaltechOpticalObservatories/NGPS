package edu.caltech.palomar.util.gui;
/**
 * Interface to define the constants and some methods
 * for coordinate system and equinox
 *
 * @author Xiuqin Wu
 */
 

public interface CoordSystemEquinox
{
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

   public String getCoordSystem();
   public String getEquinox();
   public boolean isEquatorial();
}
