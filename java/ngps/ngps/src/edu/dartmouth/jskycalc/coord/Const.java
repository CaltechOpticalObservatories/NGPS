package edu.dartmouth.jskycalc.coord; 
//=== File Prolog =============================================================
//	This code was developed by John Thorstensen of Dartmouth University
//
//      It has been refactored and modified by Jennifer Milburn for
//      use at the Palomar Observatory.
//
//      Caltech Optical Observatories, Palomar Observatory
//
//--- Contents ----------------------------------------------------------------
//	 Const
//
//--- Description -------------------------------------------------------------
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
//       DERIVED DIRECTLY FROM THE JSKYCALC by JOHN THORSTENSEN, Dartmouth University
//       Original Implementation : John Thorstensen copyright 2007,
//
//       July 24, 2010  Jennifer Milburn, refactor for use at the Palomar Observatory
//
//
//   JSkyCalc24m.java -- 2009 January.  I liked the 1.3m so much I decided
//   to do something similar for the 2.4m.  This has the capability of reading
//   the telescope coords and commenting on them, but it doesn't slew the
//   telescope.  It'll be nice to automatically have the telescope coords there,
//   though. */
//
// JSkyCalc13m.java -- 2009 January.  This version adds an interface to
// * Dick Treffers' 1.3m telescope control server, which communicates via
// * the BAIT protocol.  The target-selection facilities here can then be
// * used. */
//
// JSkyCalc.java -- copyright 2007, John Thorstensen, Dartmouth College. */
//
// TERMS OF USE --
// Anyone is free to use this software for any purpose, and to
// modify it for their own purposes, provided that credit is given to the author
// in a prominent place.  For the present program that means that the green
// title and author banner appearing on the main window must not be removed,
// and may not be altered without premission of the author.
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
import java.util.List.*;
/*================================================================================================
/        Const() Class Declaration
/=================================================================================================*/
public class Const {
  /* some constants */
  public static final double J2000 =  2451545. ; // the julian epoch 2000
  public static final double DEG_IN_RADIAN = 57.2957795130823;
  public static final double HRS_IN_RADIAN = 3.81971863420549;
  public static final double ARCSEC_IN_RADIAN = 206264.806247096;
  public static final double PI_OVER_2 = 1.5707963267949;
  public static final double PI = 3.14159265358979;
  public static final double TWOPI = 6.28318530717959;
  public static final double EARTHRAD_IN_AU = 23454.7910556298; // earth radii in 1 AU
  public static final double EARTHRAD_IN_KM = 6378.1366; // equatorial
  public static final double KMS_AUDAY = 1731.45683633; // 1731 km/sec = 1 AU/d
  public static final double SPEED_OF_LIGHT = 299792.458; // exact, km/s.
  public static final double SS_MASS = 1.00134198; // solar system mass, M_sun
  public static final double ASTRO_UNIT = 1.4959787066e11; // 1 AU in meters
  public static final double LIGHTSEC_IN_AU = 499.0047863852; // 1 AU in SI meters
  public static final double OMEGA_EARTH = 7.292116e-5; // inertial ang vel of earth
  public static final double SID_RATE = 1.0027379093;  // sidereal/solar ratio
}
