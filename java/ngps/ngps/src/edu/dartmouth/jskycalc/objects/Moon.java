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
import edu.dartmouth.jskycalc.coord.Celest;
import edu.dartmouth.jskycalc.coord.Const;
import edu.dartmouth.jskycalc.coord.Ecliptic;
import edu.dartmouth.jskycalc.coord.InstantInTime;
import edu.dartmouth.jskycalc.coord.Topo;
import edu.dartmouth.jskycalc.coord.deltaT;
import java.util.*;
import java.util.List.*;
/*================================================================================================
/
/=================================================================================================*/
public class Moon implements Cloneable {
   public Celest geopos;
   public Celest topopos;
/*================================================================================================
/
/=================================================================================================*/
   public Moon(double jd) {
      double [] retvals;
      double eq;
      retvals = computemoon(jd);
      eq = 2000. + (jd - Const.J2000) / 365.25;
      geopos = new Celest(retvals[0],retvals[1],eq,retvals[2]);
      topopos = new Celest(0.,0.,eq);  // not set, no geogr info
   }
/*================================================================================================
/
/=================================================================================================*/
   public Moon(WhenWhere w) {
//      double [] retvals;
//      retvals = computemoon(w.when.jd);
//      geopos = new Celest(retvals[0],retvals[1],w.when.JulianEpoch(),retvals[2]);
//      topopos = Topo.topocorr(geopos, w);
        update(w.when,w.where,w.sidereal);
   }
/*================================================================================================
/
/=================================================================================================*/
   public void update(InstantInTime when, Site where, double sidereal) {
      double [] retvals;

      retvals = computemoon(when.jd);
      geopos = new Celest(retvals[0],retvals[1],when.JulianEpoch(),retvals[2]);
      topopos = Topo.topocorr(geopos, when, where, sidereal);
   }
/*================================================================================================
/
/=================================================================================================*/
       public Moon clone() {
      try {
          Moon copy = (Moon) super.clone();
          copy.geopos = (Celest) geopos.clone();
          copy.topopos = (Celest) topopos.clone();
          return copy;
      } catch (CloneNotSupportedException e) {
          throw new Error("This should never happen!");
      }
   }
/*================================================================================================
/
/=================================================================================================*/
   public static double [] computemoon(double jd) {
 /* Rather accurate lunar
   ephemeris, from Jean Meeus' *Astronomical Formulae For Calculators*,
   pub. Willman-Bell.  Includes all the terms given there. */
      double Lpr,M,Mpr,D,F,Om,T,Tsq,Tcb;
      double e,lambda,B,beta,om1,om2,dist;
      double sinx, x, y, z, l, m, n, pie;
      double [] retvals = {0.,0.,0.,0.,0.,0.};  //ra,dec,distance,x,y,z.
      double [] equatorial = {0.,0.,0.};

      jd = jd + deltaT.etcorr(jd)/86400.;
          /* approximate correction to ephemeris time */
      T = (jd - 2415020.) / 36525.;   /* this based around 1900 ... */
      Tsq = T * T;
      Tcb = Tsq * T;

      Lpr = 270.434164 + 481267.8831 * T - 0.001133 * Tsq
      		+ 0.0000019 * Tcb;
      M = 358.475833 + 35999.0498*T - 0.000150*Tsq
      		- 0.0000033*Tcb;
      Mpr = 296.104608 + 477198.8491*T + 0.009192*Tsq
      		+ 0.0000144*Tcb;
      D = 350.737486 + 445267.1142*T - 0.001436 * Tsq
      		+ 0.0000019*Tcb;
      F = 11.250889 + 483202.0251*T -0.003211 * Tsq
      		- 0.0000003*Tcb;
      Om = 259.183275 - 1934.1420*T + 0.002078*Tsq
      		+ 0.0000022*Tcb;

      Lpr = Lpr % 360.;  M = M % 360.;  Mpr = Mpr % 360.;
      D = D % 360.;  F = F % 360.;  Om = Om % 360.;

      sinx =  Math.sin((51.2 + 20.2 * T)/Const.DEG_IN_RADIAN);
      Lpr = Lpr + 0.000233 * sinx;
      M = M - 0.001778 * sinx;
      Mpr = Mpr + 0.000817 * sinx;
      D = D + 0.002011 * sinx;

      sinx = 0.003964 * Math.sin((346.560+132.870*T -0.0091731*Tsq)/Const.DEG_IN_RADIAN);

      Lpr = Lpr + sinx;
      Mpr = Mpr + sinx;
      D = D + sinx;
      F = F + sinx;


      sinx = Math.sin(Om/Const.DEG_IN_RADIAN);
      Lpr = Lpr + 0.001964 * sinx;
      Mpr = Mpr + 0.002541 * sinx;
      D = D + 0.001964 * sinx;
      F = F - 0.024691 * sinx;
      F = F - 0.004328 * Math.sin((Om + 275.05 -2.30*T)/Const.DEG_IN_RADIAN);

      e = 1 - 0.002495 * T - 0.00000752 * Tsq;

      M = M / Const.DEG_IN_RADIAN;   /* these will all be arguments ... */
      Mpr = Mpr / Const.DEG_IN_RADIAN;
      D = D / Const.DEG_IN_RADIAN;
      F = F / Const.DEG_IN_RADIAN;

      lambda = Lpr + 6.288750 * Math.sin(Mpr)
      	+ 1.274018 * Math.sin(2*D - Mpr)
      	+ 0.658309 * Math.sin(2*D)
      	+ 0.213616 * Math.sin(2*Mpr)
      	- e * 0.185596 * Math.sin(M)
      	- 0.114336 * Math.sin(2*F)
      	+ 0.058793 * Math.sin(2*D - 2*Mpr)
      	+ e * 0.057212 * Math.sin(2*D - M - Mpr)
      	+ 0.053320 * Math.sin(2*D + Mpr)
      	+ e * 0.045874 * Math.sin(2*D - M)
      	+ e * 0.041024 * Math.sin(Mpr - M)
      	- 0.034718 * Math.sin(D)
      	- e * 0.030465 * Math.sin(M+Mpr)
      	+ 0.015326 * Math.sin(2*D - 2*F)
      	- 0.012528 * Math.sin(2*F + Mpr)
      	- 0.010980 * Math.sin(2*F - Mpr)
      	+ 0.010674 * Math.sin(4*D - Mpr)
      	+ 0.010034 * Math.sin(3*Mpr)
      	+ 0.008548 * Math.sin(4*D - 2*Mpr)
      	- e * 0.007910 * Math.sin(M - Mpr + 2*D)
      	- e * 0.006783 * Math.sin(2*D + M)
      	+ 0.005162 * Math.sin(Mpr - D);

      	/* And furthermore.....*/

      lambda = lambda + e * 0.005000 * Math.sin(M + D)
      	+ e * 0.004049 * Math.sin(Mpr - M + 2*D)
      	+ 0.003996 * Math.sin(2*Mpr + 2*D)
      	+ 0.003862 * Math.sin(4*D)
      	+ 0.003665 * Math.sin(2*D - 3*Mpr)
      	+ e * 0.002695 * Math.sin(2*Mpr - M)
      	+ 0.002602 * Math.sin(Mpr - 2*F - 2*D)
      	+ e * 0.002396 * Math.sin(2*D - M - 2*Mpr)
      	- 0.002349 * Math.sin(Mpr + D)
      	+ e * e * 0.002249 * Math.sin(2*D - 2*M)
      	- e * 0.002125 * Math.sin(2*Mpr + M)
      	- e * e * 0.002079 * Math.sin(2*M)
      	+ e * e * 0.002059 * Math.sin(2*D - Mpr - 2*M)
      	- 0.001773 * Math.sin(Mpr + 2*D - 2*F)
      	- 0.001595 * Math.sin(2*F + 2*D)
      	+ e * 0.001220 * Math.sin(4*D - M - Mpr)
      	- 0.001110 * Math.sin(2*Mpr + 2*F)
      	+ 0.000892 * Math.sin(Mpr - 3*D)
      	- e * 0.000811 * Math.sin(M + Mpr + 2*D)
      	+ e * 0.000761 * Math.sin(4*D - M - 2*Mpr)
      	+ e * e * 0.000717 * Math.sin(Mpr - 2*M)
      	+ e * e * 0.000704 * Math.sin(Mpr - 2 * M - 2*D)
      	+ e * 0.000693 * Math.sin(M - 2*Mpr + 2*D)
      	+ e * 0.000598 * Math.sin(2*D - M - 2*F)
      	+ 0.000550 * Math.sin(Mpr + 4*D)
      	+ 0.000538 * Math.sin(4*Mpr)
      	+ e * 0.000521 * Math.sin(4*D - M)
      	+ 0.000486 * Math.sin(2*Mpr - D);

   /*              *eclongit = lambda;  */

      B = 5.128189 * Math.sin(F)
      	+ 0.280606 * Math.sin(Mpr + F)
      	+ 0.277693 * Math.sin(Mpr - F)
      	+ 0.173238 * Math.sin(2*D - F)
      	+ 0.055413 * Math.sin(2*D + F - Mpr)
      	+ 0.046272 * Math.sin(2*D - F - Mpr)
      	+ 0.032573 * Math.sin(2*D + F)
      	+ 0.017198 * Math.sin(2*Mpr + F)
      	+ 0.009267 * Math.sin(2*D + Mpr - F)
      	+ 0.008823 * Math.sin(2*Mpr - F)
      	+ e * 0.008247 * Math.sin(2*D - M - F)
      	+ 0.004323 * Math.sin(2*D - F - 2*Mpr)
      	+ 0.004200 * Math.sin(2*D + F + Mpr)
      	+ e * 0.003372 * Math.sin(F - M - 2*D)
      	+ 0.002472 * Math.sin(2*D + F - M - Mpr)
      	+ e * 0.002222 * Math.sin(2*D + F - M)
      	+ e * 0.002072 * Math.sin(2*D - F - M - Mpr)
      	+ e * 0.001877 * Math.sin(F - M + Mpr)
      	+ 0.001828 * Math.sin(4*D - F - Mpr)
      	- e * 0.001803 * Math.sin(F + M)
      	- 0.001750 * Math.sin(3*F)
      	+ e * 0.001570 * Math.sin(Mpr - M - F)
      	- 0.001487 * Math.sin(F + D)
      	- e * 0.001481 * Math.sin(F + M + Mpr)
      	+ e * 0.001417 * Math.sin(F - M - Mpr)
      	+ e * 0.001350 * Math.sin(F - M)
      	+ 0.001330 * Math.sin(F - D)
      	+ 0.001106 * Math.sin(F + 3*Mpr)
      	+ 0.001020 * Math.sin(4*D - F)
      	+ 0.000833 * Math.sin(F + 4*D - Mpr);
        /* not only that, but */
      B = B + 0.000781 * Math.sin(Mpr - 3*F)
      	+ 0.000670 * Math.sin(F + 4*D - 2*Mpr)
      	+ 0.000606 * Math.sin(2*D - 3*F)
      	+ 0.000597 * Math.sin(2*D + 2*Mpr - F)
      	+ e * 0.000492 * Math.sin(2*D + Mpr - M - F)
      	+ 0.000450 * Math.sin(2*Mpr - F - 2*D)
      	+ 0.000439 * Math.sin(3*Mpr - F)
      	+ 0.000423 * Math.sin(F + 2*D + 2*Mpr)
      	+ 0.000422 * Math.sin(2*D - F - 3*Mpr)
      	- e * 0.000367 * Math.sin(M + F + 2*D - Mpr)
      	- e * 0.000353 * Math.sin(M + F + 2*D)
      	+ 0.000331 * Math.sin(F + 4*D)
      	+ e * 0.000317 * Math.sin(2*D + F - M + Mpr)
      	+ e * e * 0.000306 * Math.sin(2*D - 2*M - F)
      	- 0.000283 * Math.sin(Mpr + 3*F);


      om1 = 0.0004664 * Math.cos(Om/Const.DEG_IN_RADIAN);
      om2 = 0.0000754 * Math.cos((Om + 275.05 - 2.30*T)/Const.DEG_IN_RADIAN);

      beta = B * (1. - om1 - om2);
   /*      *eclatit = beta; */

      pie = 0.950724
      	+ 0.051818 * Math.cos(Mpr)
      	+ 0.009531 * Math.cos(2*D - Mpr)
      	+ 0.007843 * Math.cos(2*D)
      	+ 0.002824 * Math.cos(2*Mpr)
      	+ 0.000857 * Math.cos(2*D + Mpr)
      	+ e * 0.000533 * Math.cos(2*D - M)
      	+ e * 0.000401 * Math.cos(2*D - M - Mpr)
      	+ e * 0.000320 * Math.cos(Mpr - M)
      	- 0.000271 * Math.cos(D)
      	- e * 0.000264 * Math.cos(M + Mpr)
      	- 0.000198 * Math.cos(2*F - Mpr)
      	+ 0.000173 * Math.cos(3*Mpr)
      	+ 0.000167 * Math.cos(4*D - Mpr)
      	- e * 0.000111 * Math.cos(M)
      	+ 0.000103 * Math.cos(4*D - 2*Mpr)
      	- 0.000084 * Math.cos(2*Mpr - 2*D)
      	- e * 0.000083 * Math.cos(2*D + M)
      	+ 0.000079 * Math.cos(2*D + 2*Mpr)
      	+ 0.000072 * Math.cos(4*D)
      	+ e * 0.000064 * Math.cos(2*D - M + Mpr)
      	- e * 0.000063 * Math.cos(2*D + M - Mpr)
      	+ e * 0.000041 * Math.cos(M + D)
      	+ e * 0.000035 * Math.cos(2*Mpr - M)
      	- 0.000033 * Math.cos(3*Mpr - 2*D)
      	- 0.000030 * Math.cos(Mpr + D)
      	- 0.000029 * Math.cos(2*F - 2*D)
      	- e * 0.000029 * Math.cos(2*Mpr + M)
      	+ e * e * 0.000026 * Math.cos(2*D - 2*M)
      	- 0.000023 * Math.cos(2*F - 2*D + Mpr)
      	+ e * 0.000019 * Math.cos(4*D - M - Mpr);

      // System.out.printf("beta lambda %f %f",beta,lambda);
      beta = beta/Const.DEG_IN_RADIAN;
      lambda = lambda/Const.DEG_IN_RADIAN;
      dist = 1./Math.sin((pie)/Const.DEG_IN_RADIAN);
//      System.out.printf("dist %f\n",dist);

      retvals[2] = dist / Const.EARTHRAD_IN_AU;

      l = Math.cos(lambda) * Math.cos(beta);
      m = Math.sin(lambda) * Math.cos(beta);
      n = Math.sin(beta);

      equatorial = Ecliptic.eclrot(jd,l,m,n);
      retvals[3] = equatorial[0] * dist;
      retvals[4] = equatorial[1] * dist;
      retvals[5] = equatorial[2] * dist;

      retvals[0] = Math.atan2(equatorial[1],equatorial[0])
                 * Const.HRS_IN_RADIAN;
      retvals[1] = Math.asin(equatorial[2]) * Const.DEG_IN_RADIAN;

      return retvals;

   }
/*================================================================================================
/
/=================================================================================================*/
   public static double flmoon(int n, int nph) {
    /* Gives JD (+- 2 min) of phase nph during a given lunation n.
       Implements formulae in Meeus' Astronomical Formulae for Calculators,
       2nd Edn, publ. by Willman-Bell. */
    /* nph = 0 for new, 1 first qtr, 2 full, 3 last quarter. */

       double jd, cor;
	double M, Mpr, F;
	double T;
	double lun;

	lun = (double) n + (double) nph / 4.;
	T = lun / 1236.85;
	jd = 2415020.75933 + 29.53058868 * lun
		+ 0.0001178 * T * T
		- 0.000000155 * T * T * T
		+ 0.00033 * Math.sin((166.56 + 132.87 * T - 0.009173 * T * T)/Const.DEG_IN_RADIAN);
	M = 359.2242 + 29.10535608 * lun - 0.0000333 * T * T - 0.00000347 * T * T * T;
	M = M / Const.DEG_IN_RADIAN;
	Mpr = 306.0253 + 385.81691806 * lun + 0.0107306 * T * T + 0.00001236 * T * T * T;
	Mpr = Mpr / Const.DEG_IN_RADIAN;
	F = 21.2964 + 390.67050646 * lun - 0.0016528 * T * T - 0.00000239 * T * T * T;
	F = F / Const.DEG_IN_RADIAN;
	if((nph == 0) || (nph == 2)) {/* new or full */
		cor =   (0.1734 - 0.000393*T) * Math.sin(M)
			+ 0.0021 * Math.sin(2*M)
			- 0.4068 * Math.sin(Mpr)
			+ 0.0161 * Math.sin(2*Mpr)
			- 0.0004 * Math.sin(3*Mpr)
			+ 0.0104 * Math.sin(2*F)
			- 0.0051 * Math.sin(M + Mpr)
			- 0.0074 * Math.sin(M - Mpr)
			+ 0.0004 * Math.sin(2*F+M)
			- 0.0004 * Math.sin(2*F-M)
			- 0.0006 * Math.sin(2*F+Mpr)
			+ 0.0010 * Math.sin(2*F-Mpr)
			+ 0.0005 * Math.sin(M+2*Mpr);
		jd = jd + cor;
	}
	else {
		cor = (0.1721 - 0.0004*T) * Math.sin(M)
			+ 0.0021 * Math.sin(2 * M)
			- 0.6280 * Math.sin(Mpr)
			+ 0.0089 * Math.sin(2 * Mpr)
			- 0.0004 * Math.sin(3 * Mpr)
			+ 0.0079 * Math.sin(2*F)
			- 0.0119 * Math.sin(M + Mpr)
			- 0.0047 * Math.sin(M - Mpr)
			+ 0.0003 * Math.sin(2 * F + M)
			- 0.0004 * Math.sin(2 * F - M)
			- 0.0006 * Math.sin(2 * F + Mpr)
			+ 0.0021 * Math.sin(2 * F - Mpr)
			+ 0.0003 * Math.sin(M + 2 * Mpr)
			+ 0.0004 * Math.sin(M - 2 * Mpr)
			- 0.0003 * Math.sin(2*M + Mpr);
		if(nph == 1) cor = cor + 0.0028 -
				0.0004 * Math.cos(M) + 0.0003 * Math.cos(Mpr);
		if(nph == 3) cor = cor - 0.0028 +
				0.0004 * Math.cos(M) - 0.0003 * Math.cos(Mpr);
		jd = jd + cor;

	}
	return jd;
   }
/*================================================================================================
/
/=================================================================================================*/
   public static int lunation(double jd) {
       int n; // approximate lunation ...
       int nlast;
       double newjd, lastnewjd;
       int kount = 0;
       double x;

       nlast = (int) ((jd - 2415020.5) / 29.5307) - 1;

       lastnewjd = flmoon(nlast,0);
       nlast++;
       newjd = flmoon(nlast,0);
       // increment lunations until you're sure the last and next new
       // moons bracket your input value.
       while((newjd < jd) && (kount < 40)) {
           lastnewjd = newjd;
           nlast++; kount++;
           newjd = flmoon(nlast,0);
       }
       if(kount > 35) System.out.printf("didn't find lunation!\n");
       return (nlast - 1);
   }
/*================================================================================================
/
/=================================================================================================*/
   public static String MoonPhaseDescr(double jd) {

       int n;
       int nlast, noctiles;
       double newjd, lastnewjd;
       double fqjd, fljd, lqjd;
       int kount = 0;
       double x;

       nlast = lunation(jd);
       lastnewjd = flmoon(nlast,0);
       x = jd - lastnewjd;
       noctiles = (int) (x / 3.69134);  // 1/8 month, truncated.
       if(noctiles == 0) return
          String.format(Locale.ENGLISH, "%3.1f days since new moon.",x);
       else if (noctiles <= 2) {  // nearest first quarter ...
          fqjd = flmoon(nlast,1);
          x = jd - fqjd;
          if(x < 0.)
              return String.format(Locale.ENGLISH, "%3.1f days before first quarter.",(-1.*x));
          else
              return String.format(Locale.ENGLISH, "%3.1f days after first quarter.",x);
       }
       else if (noctiles <= 4) {  // nearest full ...
          fljd = flmoon(nlast,2);
          x = jd - fljd;
          if(x < 0.)
               return String.format(Locale.ENGLISH, "%3.1f days before full moon.",(-1.*x));
          else
              return String.format(Locale.ENGLISH, "%3.1f days after full moon.",x);
       }
       else if (noctiles <= 6) {  // nearest last quarter ...
          lqjd = flmoon(nlast,3);
          x = jd - lqjd;
          if(x < 0.)
               return String.format(Locale.ENGLISH, "%3.1f days before last quarter.",(-1.*x));
          else
              return String.format(Locale.ENGLISH, "%3.1f days after last quarter.",x);
       }
       else {
          newjd = flmoon(nlast + 1,0);
          x = jd - newjd;
          return String.format(Locale.ENGLISH, "%3.1f days before new moon.",(-1.*x));
       }
   }
}
