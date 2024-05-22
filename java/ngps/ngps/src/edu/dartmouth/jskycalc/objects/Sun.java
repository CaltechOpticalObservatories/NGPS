package edu.dartmouth.jskycalc.objects;     
//=== File Prolog =============================================================
//	This code was developed by John Thorstensen of Dartmouth University
//
//      It has been refactored and modified by Jennifer Milburn for
//      use at the Palomar Observatory.
//
//      Caltech Optical Observatories, Palomar Observatory
//
//--- Contents ----------------------------------------------------------------
//
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
import edu.dartmouth.jskycalc.coord.Site;
import edu.dartmouth.jskycalc.coord.InstantInTime;
import edu.dartmouth.jskycalc.coord.Topo;
import edu.dartmouth.jskycalc.coord.Ecliptic;
import edu.dartmouth.jskycalc.coord.Const;
import edu.dartmouth.jskycalc.coord.Celest;
import edu.dartmouth.jskycalc.coord.deltaT;
import java.util.List.*;
/*================================================================================================
/
/=================================================================================================*/
public class Sun implements Cloneable {
   public Celest geopos;
   public Celest topopos;
   double [] xyz = {0.,0.,0.};     /* for use in barycentric correction */
   double [] xyzvel = {0.,0.,0.};  /* ditto. */

   public Sun(WhenWhere w) {
       update(w.when,w.where,w.sidereal);
   }
/*================================================================================================
/
/=================================================================================================*/
   public Sun(InstantInTime inst) {  // no site, so no topo
       double retvals [];
       retvals = computesun(inst.jd);
       xyz[0] = retvals[3]; xyz[1] = retvals[4]; xyz[2] = retvals[5];
       geopos = new Celest(retvals[0],retvals[1],inst.JulianEpoch());
       geopos.distance = retvals[2];
       topopos = new Celest(0.,0.,inst.JulianEpoch());  // not set
       topopos.distance = 0.;
   }
/*================================================================================================
/
/=================================================================================================*/
   public Sun(double jdin) {        // no site, so no topo possible
       double retvals [];
       double eq;

       retvals = computesun(jdin);
       xyz[0] = retvals[3]; xyz[1] = retvals[4]; xyz[2] = retvals[5];
       eq = 2000. + (jdin - Const.J2000) / 365.25;
       geopos = new Celest(retvals[0],retvals[1],eq);
       geopos.distance = retvals[2];
       topopos = new Celest(0.,0.,eq); // not set, no geogr. info
       topopos.distance = 0.;
   }
/*================================================================================================
/
/=================================================================================================*/
   public Sun clone() {
      try {
          Sun copy = (Sun) super.clone();
          copy.geopos = (Celest) geopos.clone();
          copy.topopos = (Celest) topopos.clone();
          copy.xyz = xyz;
          copy.xyzvel = xyzvel;
          return copy;
      } catch (CloneNotSupportedException e) {
          throw new Error("This should never happen!");
      }
   }
/*================================================================================================
/
/=================================================================================================*/
    public void update(InstantInTime when, Site where, double sidereal) {
       // need to avoid handing in a whenwhere or get circularity ...
        double [] retvals;

       // System.out.printf("updating sun jd %f ... ",w.when.jd);
       retvals = computesun(when.jd);
       xyz = new double [3];
       xyz[0] = retvals[3]; xyz[1] = retvals[4]; xyz[2] = retvals[5];

       //System.out.printf("Sun constructor - jd %f xyz = %f %f %f\n",
       //     w.when.jd,xyz[0],xyz[1],xyz[2]);

       // ignoring topocentric part of helio time correction.

       geopos = new Celest(retvals[0],retvals[1],when.JulianEpoch());
       geopos.distance = retvals[2];
       topopos = Topo.topocorr(geopos,when,where,sidereal);
       // System.out.printf("topo radec %s %s\n",topopos.Alpha.RoundedRAString(2,":"),
         //        topopos.Delta.RoundedDecString(1,":"));
   }
/*================================================================================================
/
/=================================================================================================*/
/* Implements Jean Meeus' solar ephemeris, from Astronomical
   Formulae for Calculators, pp. 79 ff.  Position is wrt *mean* equinox of
   date. */

  public static double [] computesun(double jd) {
       double xecl, yecl, zecl;
       double [] equatorial = {0.,0.,0.};
       double e, L, T, Tsq, Tcb;
       double M, Cent, nu, sunlong;
       double Lrad, Mrad, nurad, R;
       double A, B, C, D, E, H;
       double [] retvals = {0.,0.,0.,0.,0.,0.};
            // will be geora, geodec, geodist, x, y, z (geo)

       // correct jd to ephemeris time once we have that done ...

       jd = jd + deltaT.etcorr(jd) / 86400.;
       T = (jd - 2415020.) / 36525.;  // Julian centuries since 1900
       Tsq = T*T;   Tcb = T*Tsq;

       L = 279.69668 + 36000.76892*T + 0.0003025*Tsq;
       M = 358.47583 + 35999.04975*T - 0.000150*Tsq - 0.0000033*Tcb;
       e = 0.01675104 - 0.0000418*T - 0.000000126*Tsq;

       A = (153.23 + 22518.7541 * T) / Const.DEG_IN_RADIAN;  /* A, B due to Venus */
       B = (216.57 + 45037.5082 * T) / Const.DEG_IN_RADIAN;
       C = (312.69 + 32964.3577 * T) / Const.DEG_IN_RADIAN;  /* C due to Jupiter */
                /* D -- rough correction from earth-moon
                        barycenter to center of earth. */
       D = (350.74 + 445267.1142*T - 0.00144*Tsq) / Const.DEG_IN_RADIAN;
       E = (231.19 + 20.20*T) / Const.DEG_IN_RADIAN;
                       /* "inequality of long period .. */
       H = (353.40 + 65928.7155*T) / Const.DEG_IN_RADIAN;  /* Jupiter. */

       L = L + 0.00134 * Math.cos(A)
             + 0.00154 * Math.cos(B)
	     + 0.00200 * Math.cos(C)
	     + 0.00179 * Math.sin(D)
	     + 0.00178 * Math.sin(E);

       Lrad = L/Const.DEG_IN_RADIAN;
       Mrad = M/Const.DEG_IN_RADIAN;

       Cent = (1.919460 - 0.004789*T -0.000014*Tsq)*Math.sin(Mrad)
	     + (0.020094 - 0.000100*T) * Math.sin(2.0*Mrad)
	     + 0.000293 * Math.sin(3.0*Mrad);
       sunlong = L + Cent;


       nu = M + Cent;
       nurad = nu / Const.DEG_IN_RADIAN;

       R = (1.0000002 * (1 - e*e)) / (1. + e * Math.cos(nurad));
       R = R + 0.00000543 * Math.sin(A)
	      + 0.00001575 * Math.sin(B)
	      + 0.00001627 * Math.sin(C)
	      + 0.00003076 * Math.cos(D)
	      + 0.00000927 * Math.sin(H);
/*      printf("solar longitude: %10.5f  Radius vector %10.7f\n",sunlong,R);
	printf("eccentricity %10.7f  eqn of center %10.5f\n",e,Cent);   */

       sunlong = sunlong/Const.DEG_IN_RADIAN;

       retvals[2] = R; // distance
       xecl = Math.cos(sunlong);  /* geocentric */
       yecl = Math.sin(sunlong);
       zecl = 0.;
       equatorial = Ecliptic.eclrot(jd, xecl, yecl, zecl);

       retvals[0] = Math.atan2(equatorial[1],equatorial[0]) * Const.HRS_IN_RADIAN;
       while(retvals[0] < 0.) retvals[0] = retvals[0] + 24.;
       retvals[1] = Math.asin(equatorial[2]) * Const.DEG_IN_RADIAN;

       retvals[3] = equatorial[0] * R;  // xyz
       retvals[4] = equatorial[1] * R;
       retvals[5] = equatorial[2] * R;
//       System.out.printf("computesun XYZ %f %f %f  %f\n",
//          retvals[3],retvals[4],retvals[5],jd);

       return(retvals);
    }
/*================================================================================================
/
/=================================================================================================*/
   public  void sunvel(double jd) {
       /* numerically differentiates sun xyz to get velocity. */
       double dt = 0.05; // days ... gives about 8 digits ...
       int i;

       double [] pos1 = computesun(jd - dt/2.);
       double [] pos2 = computesun(jd + dt/2.);
       for(i = 0; i < 3; i++) {
           xyzvel[i] = (pos2[i+3] - pos1[i+3]) / dt;  // AU/d, eq. of date.
       }
    }
}