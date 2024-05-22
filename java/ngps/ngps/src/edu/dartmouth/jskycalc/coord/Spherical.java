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
import java.util.List.*;
/*================================================================================================
/
/=================================================================================================*/
public class Spherical {
  /** container for several static spherical trig methods. */

 public static double subtend(Celest a, Celest b) {
   /** angle in radians between two positions. */
      double [] aCart = {0.,0.,0.};
      double [] bCart = {0.,0.,0.};
      Celest bprime;
      double dotproduct, theta;
      double dr,dd;

      if(Math.abs(a.Equinox - b.Equinox) > 0.001)
         bprime = b.precessed(a.Equinox);
      else bprime = (Celest) b.clone();

      aCart = a.cel_unitXYZ();
      bCart = bprime.cel_unitXYZ();

      dotproduct = aCart[0] * bCart[0] + aCart[1] * bCart[1] +
                   aCart[2] * bCart[2];

      theta = Math.acos(dotproduct);

      // if the angle is tiny, use a flat sky approximation away from
      // the poles to get a more accurate answer.
      if(theta < 1.0e-5) {
          if(Math.abs(a.Delta.radians()) < (Const.PI_OVER_2 - 0.001) &&
             Math.abs(bprime.Delta.radians()) < (Const.PI_OVER_2 - 0.001)) {
		dr = (bprime.Alpha.radians() - a.Alpha.radians()) *
                       Math.cos((a.Delta.radians() + bprime.Delta.radians())/2.);
                dd = bprime.Delta.radians() - a.Delta.radians();
                theta = Math.sqrt(dr*dr + dd*dd);

          }
      }
      return theta;
  }
/*================================================================================================
/
/=================================================================================================*/
  public static double [] CuspPA(Celest s, Celest m) {
  /** Given the positions of the sun and the moon, returns the position angle
      of the line connecting the moon's two cusps.  Ported from python. */
      double codecsun = (90. - s.Delta.value) / Const.DEG_IN_RADIAN;
      double codecmoon = (90. - m.Delta.value) / Const.DEG_IN_RADIAN;
      double dra = s.Alpha.value - m.Alpha.value;
      while(dra < -12.) dra += 24.;
      while(dra >= 12.) dra -= 24.;
      dra = dra / Const.HRS_IN_RADIAN;

     // Spherical law of cosines gives moon-sun separation
     double moonsun = Math.acos(Math.cos(codecsun) * Math.cos(codecmoon) +
       Math.sin(codecsun) * Math.sin(codecmoon) * Math.cos(dra));
     // spherical law of sines + law of cosines needed to get
     // sine and cosine of pa separately; this gets quadrant etc.!
     double pasin = Math.sin(dra) * Math.sin(codecsun) / Math.sin(moonsun);
     double pacos = (Math.cos(codecsun) -
         Math.cos(codecmoon) * Math.cos(moonsun)) /
        (Math.sin(codecmoon) * Math.sin(moonsun));
     double pa = Math.atan2(pasin,pacos);

//   print "pa of arc from moon to sun is %5.2f deg." % (pa * _skysub.DEG_IN_RADIAN)

     double cusppa = pa - Const.PI/2.;   // line of cusps ...
//     System.out.printf("cusppa = %f\n",cusppa);
     double [] retvals = {cusppa, moonsun};
     return retvals ;
  }
/*================================================================================================
/
/=================================================================================================*/
  public static double [] min_max_alt (double lat, double dec) {
      /** returns the minimum and maximum altitudes (degrees) of an object at
          declination dec as viewed from lat.  */
      /* translated straight from skycalc. */

      double x, latrad, decrad;
      double [] retvals = {0.,0.};

      latrad = lat / Const.DEG_IN_RADIAN;
      decrad = dec / Const.DEG_IN_RADIAN;
      x = Math.cos(decrad) * Math.cos(latrad) + Math.sin(decrad) * Math.sin(latrad);
      if(Math.abs(x) <= 1.) retvals[1] = Math.asin(x) * Const.DEG_IN_RADIAN;
      else System.out.printf("min_max_alt ... asin(>1)\n");

      x = Math.sin(decrad) * Math.sin(latrad) - Math.cos(decrad) * Math.cos(latrad);
      if(Math.abs(x) <= 1.) retvals[0] = Math.asin(x) * Const.DEG_IN_RADIAN;
      else System.out.printf("min_max_alt ... asin(>1)\n");

      return retvals;
  }
/*================================================================================================
/
/=================================================================================================*/
  public static double ha_alt(double dec, double lat, double alt)  {
      /** Finds the hour angle at which an object at declination dec is at altitude
          alt, as viewed from latitude lat;  returns 1000 if always higher,
          -1000 if always lower. */

      double x, coalt, codec, colat;
      double [] minmax;

      minmax = min_max_alt(lat,dec);
      if(alt < minmax[0]) return(1000.);   // always higher than asked
      if(alt > minmax[1]) return(-1000.);  // always lower than asked

      // System.out.printf("dec %f lat %f alt %f ... \n",dec,lat,alt);
      codec = Const.PI_OVER_2 - dec / Const.DEG_IN_RADIAN;
      colat = Const.PI_OVER_2 - lat / Const.DEG_IN_RADIAN;
      coalt = Const.PI_OVER_2 - alt / Const.DEG_IN_RADIAN;
      x = (Math.cos(coalt) - Math.cos(codec) * Math.cos(colat)) /
           (Math.sin(codec) * Math.sin(colat));
      if(Math.abs(x) <= 1.) return(Math.acos(x) * Const.HRS_IN_RADIAN);
      else {
          System.out.printf("Bad inverse trig in ha_alt ... acos(%f)\n",x);
          return(1000.);
      }
   }
/*================================================================================================
/
/=================================================================================================*/
  public  static double true_airmass(double alt) {
	/* returns the true airmass for a given altitude (degrees).  Ported from C. */
	/* The expression used is based on a tabulation of the mean KPNO
           atmosphere given by C. M. Snell & A. M. Heiser, 1968,
	   PASP, 80, 336.  They tabulated the airmass at 5 degr
           intervals from z = 60 to 85 degrees; I fit the data with
           a fourth order poly for (secz - airmass) as a function of
           (secz - 1) using the IRAF curfit routine, then adjusted the
           zeroth order term to force (secz - airmass) to zero at
           z = 0.  The poly fit is very close to the tabulated points
	   (largest difference is 3.2e-4) and appears smooth.
           This 85-degree point is at secz = 11.47, so for secz > 12
           I just return secz - 1.5 ... about the largest offset
           properly determined. */
        double secz, seczmin1;
        int i, ord = 3;
        double [] coef = {2.879465e-3, 3.033104e-3, 1.351167e-3, -4.716679e-5};
        double result = 0.;

        if(alt <= 0.) return (-1.);   /* out of range. */
        secz = 1. / Math.sin(alt / Const.DEG_IN_RADIAN);
        seczmin1 = secz - 1.;
        if(secz > 12.) return (secz - 1.5);   // approx at extreme airmass
        for(i = ord; i >= 0; i--) result = (result + coef[i]) * seczmin1;
        result = secz - result;
        return result;
   }
/*================================================================================================
/
/=================================================================================================*/
  public  static Celest Gal2Cel(double galacticlongit, double galacticlatit) {
      /** computes instance variables galong and galat.  Algorithm is
          rigorous. */

      double [] xyz = {0., 0., 0.};
      double [] xyzgal = {0.,0.,0.};
      double [] temp;

      double  p11= -0.066988739415,
	p12= -0.872755765853,
	p13= -0.483538914631,
	p21=  0.492728466047,
	p22= -0.450346958025,
	p23=  0.744584633299,
	p31= -0.867600811168,
	p32= -0.188374601707,
	p33=  0.460199784759;

      double galongitrad = galacticlongit / Const.DEG_IN_RADIAN;
      double galatitrad = galacticlatit / Const.DEG_IN_RADIAN;

      xyzgal[0] = Math.cos(galongitrad) * Math.cos(galatitrad);
      xyzgal[1] = Math.sin(galongitrad) * Math.cos(galatitrad);
      xyzgal[2] = Math.sin(galatitrad);
      // System.out.printf("Galactic xyz %f %f %f\n",xyzgal[0],xyzgal[1],xyzgal[2]);

      // for rotation matrices, inverse is the transpose, so ...
      xyz[0] = xyzgal[0] * p11 + xyzgal[1] * p21 + xyzgal[2] * p31;
      xyz[1] = xyzgal[0] * p12 + xyzgal[1] * p22 + xyzgal[2] * p32;
      xyz[2] = xyzgal[0] * p13 + xyzgal[1] * p23 + xyzgal[2] * p33;
      // System.out.printf("Equatorial xyz %f %f %f\n",xyz[0],xyz[1],xyz[2]);

      double [] retvals = Celest.XYZcel(xyz[0],xyz[1],xyz[2]);
      Celest cel = new Celest(retvals[0],retvals[1],1950.);  // galactic are defined for 1950

      return cel;   // and precess elsehwere to whatever.

   }
/*================================================================================================
/
/=================================================================================================*/
  public  static double [] tostd(Celest posn, Celest centerpos) {
      /** computes the tangent-plane coordinates for posn relative to centerpos.*/

      double [] xieta = {0.,0.};

      // These coords should be in the same equinox.  Check here
      // that they are.
      if(Math.abs(posn.Equinox - centerpos.Equinox) > 0.003) {
         System.out.printf("Equinox mismatch in tostd.  Fails.\n");
         return(xieta);  // fail.  It would be possible to work
              // around, but at the expense of speed and memory.
             // there can be 50000+ of these ...
      }

      double radiff = posn.Alpha.value - centerpos.Alpha.value;
      if(radiff < -12.) radiff += 24.;
      if(radiff > 12.) radiff -= 24.;
      radiff /= Const.HRS_IN_RADIAN;
      double deccent = centerpos.Delta.radians();
      double decposn = posn.Delta.radians();

      double denom = Math.sin(deccent) * Math.tan(decposn) +
                     Math.cos(deccent) * Math.cos(radiff);

      if(denom != 0.) xieta[0] = Math.sin(radiff) / denom;  // [0] -> X, or RA
      else {
          System.out.printf("Error in tostd ... zero denom.\n");
          return xieta;
      }
      xieta[1] = (Math.cos(deccent) * Math.tan(decposn) -
           Math.sin(deccent) * Math.cos(radiff)) / denom;  // [1] -> Y, or dec.
      xieta[0] = xieta[0] * Const.ARCSEC_IN_RADIAN;   // return in arcsec
      xieta[1] = xieta[1] * Const.ARCSEC_IN_RADIAN;   // return in arcsec
      return (xieta);
   }
}
