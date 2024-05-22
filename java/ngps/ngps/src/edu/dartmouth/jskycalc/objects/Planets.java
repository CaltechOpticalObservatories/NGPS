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
import edu.dartmouth.jskycalc.coord.Celest;
import edu.dartmouth.jskycalc.coord.Const;
import edu.dartmouth.jskycalc.coord.Ecliptic;
import java.util.List.*;
/*================================================================================================
/
/=================================================================================================*/
public class Planets  {
/** This includes everything for the planets. */
/* The planetary ephemeris used is based on Meeus' Astronomical
   Forumulae for Calculators.  This gives fairly good (< 1 arcmin)
   positions for the inner planets but somewhat worse for the outer
   planets.  These should NOT be used in applications where precise
   positions are needed.  This is intended as a very lightweight
   application for purposes such as  (a) telling you which planet is
   which in the sky; (b) plotting the planet recognizably among the
   constellations; (c) warning you if your target is fairly close to
   a bright planet; (d) generating a reasonably accurate value for the
   offset of the solar system barycenter from the center of the sun. */

   double jd_el;

   /* Arrays of planetary elements */
   public static String [] names = {"Mercury","Venus","Earth","Mars","Jupiter","Saturn","Uranus","Neptune","Pluto"}; // what the heck, let's keep Pluto
   double [] incl = {0.,0.,0.,0.,0.,0.,0.,0.,0.};  // inclination
   double [] Omega = {0.,0.,0.,0.,0.,0.,0.,0.,0.}; // longit of asc node
   double [] omega = {0.,0.,0.,0.,0.,0.,0.,0.,0.}; // longit of perihelion
   double [] a = {0.,0.,0.,0.,0.,0.,0.,0.,0.};     // semimajor axis
   double [] daily = {0.,0.,0.,0.,0.,0.,0.,0.,0.}; // mean daily motion, degr.
   double [] ecc = {0.,0.,0.,0.,0.,0.,0.,0.,0.};   // eccentricity
   double [] L_0 = {0.,0.,0.,0.,0.,0.,0.,0.,0.};   // starting longitude (?)
   static double [] mass = {1.660137e-7, 2.447840e-6, 3.040433e-6,
          3.227149e-7,9.547907e-4,2.858776e-4,4.355401e-5,5.177591e-5,
          7.69e-9};  // IAU 1976 masses of planets in terms of solar mass
                     // used to weight barycenter calculation only.
// Need to include instance variables of x,y,z and xdot,ydot,zdot
   public double [][] xyz;
   public double [][] xyzvel;
   public double [] barycor;  // 0,1,2 = xyz in AU; 3,4,5 = xyzvel in km/s.
   public double [] mags = {0.,0.,0.,0.,0.,0.,0.,0.,0.};   // apparent mags
   static final double [] V0 = {-0.42,-4.40,-3.86,-1.52,-9.40,-9.22,-7.19,
                                      -6.87,-1.0};

/*  From Astronomical Almanac, 2003, p. E88.  V mag of planet when
    full face on at unit distance from both sun and earth.  Saturn
    has been goosed up a bit b/c Almanac quantity didn't have rings
    in it ...   */

   public Observation [] PlanetObs;
/*================================================================================================
/
/=================================================================================================*/
   public Planets(WhenWhere wIn) {
      comp_el(wIn.when.jd);
      PlanetObs = pposns(wIn);
   }
/*================================================================================================
/
/=================================================================================================*/
   public void Update(WhenWhere wIn) {
      comp_el(wIn.when.jd);
      PlanetObs = pposns(wIn);
   }
/*================================================================================================
/
/=================================================================================================*/
   public void comp_el(double jd_in) {
      /* Compute and load mean elements for the planets. */
      double T, Tsq, Tcb, d;
      double ups, P, Q, S, V, W, G, H, zeta, psi;  // Meeus p. 110 ff.
      double sinQ, sinZeta, cosQ, cosZeta, sinV, cosV, sin2Zeta, cos2Zeta;
      double sin2Q, cos2Q, sinH, sin2H, cosH, cos2H;

      jd_el = jd_in; d = jd_el - 2415020.;    // 1900
      T = d / 36525.;
      Tsq = T * T;
      Tcb = Tsq * T;

      // Mercury, Venus, and Mars from Explanatory Suppl. p. 113

      // Mercury = 0
      incl[0] = 7.002880 + 1.8608e-3 * T - 1.83e-5 * Tsq;
      Omega[0] = 47.14594 + 1.185208 * T + 1.74e-4 * Tsq;
      omega[0] = 75.899697 + 1.55549 * T + 2.95e-4 * Tsq;
      a[0] = 0.3870986;
      daily[0] = 4.0923388;
      ecc[0] = 0.20561421 + 0.00002046 * T;
      L_0[0] = 178.179078 + 4.0923770233 * d  +
	 0.0000226 * Math.pow((3.6525 * T),2.);

      // Venus = 1
      incl[1] = 3.39363 + 1.00583e-03 * T - 9.722e-7 * Tsq;
      Omega[1] = 75.7796472 + 0.89985 * T + 4.1e-4 * Tsq;
      omega[1] = 130.16383 + 1.4080 * T + 9.764e-4 * Tsq;
      a[1] = 0.723325;
      daily[1] = 1.60213049;
      ecc[1] = 0.00682069 - 0.00004774 * T;
      L_0[1] = 342.767053 + 1.6021687039 * 36525 * T +
	 0.000023212 * Math.pow((3.6525 * T),2.);

      // Earth = 2  ... elements from old Nautical Almanac
      ecc[2] = 0.01675104 - 0.00004180*T + 0.000000126*Tsq;
      incl[2] = 0.0;
      Omega[2] = 0.0;
      omega[2] = 101.22083 + 0.0000470684*d + 0.000453*Tsq + 0.000003*Tcb;
      a[2] = 1.0000007;;
      daily[2] = 0.985599;
      L_0[2] = 358.47583 + 0.9856002670*d - 0.000150*Tsq - 0.000003*Tcb +
	    omega[2];

      // Mars = 3
      incl[3] = 1.85033 - 6.75e-04 * T - 1.833e-5 * Tsq;
      Omega[3] = 48.786442 + .770992 * T + 1.39e-6 * Tsq;
      omega[3] = 334.218203 + 1.840758 * T + 1.299e-4 * Tsq;
      a[3] = 1.5236915;
      daily[3] = 0.5240329502 + 1.285e-9 * T;
      ecc[3] = 0.09331290 - 0.000092064 * T - 0.000000077 * Tsq;
      L_0[3] = 293.747628 + 0.5240711638 * d  +
	 0.000023287 * Math.pow((3.6525 * T),2.);

      // Outer planets from Jean Meeus, Astronomical Formulae for
      // Calculators, 3rd Edn, Willman-Bell; p. 100
      // Mutual interactions get pretty big; I'm including some of the
      // larger perturbation terms from Meeus' book.

      // Jupiter = 4

     incl[4] = 1.308736 - 0.0056961 * T + 0.0000039 * Tsq;
     Omega[4] = 99.443414 + 1.0105300 * T + 0.0003522 * Tsq
  		- 0.00000851 * Tcb;
     omega[4] = 12.720972 + 1.6099617 * T + 1.05627e-3 * Tsq
  	- 3.43e-6 * Tcb;
     a[4] = 5.202561;
     daily[4] = 0.08312941782;
     ecc[4] = .04833475  + 1.64180e-4 * T - 4.676e-7*Tsq -
  	1.7e-9 * Tcb;
     L_0[4] = 238.049257 + 3036.301986 * T + 0.0003347 * Tsq -
  	1.65e-6 * Tcb;

     ups = 0.2*T + 0.1;
     P = (237.47555 + 3034.9061 * T) / Const.DEG_IN_RADIAN;
     Q = (265.91650 + 1222.1139 * T) / Const.DEG_IN_RADIAN;
     S = (243.51721 + 428.4677 * T) / Const.DEG_IN_RADIAN;
     V = 5*Q - 2*P;
     W = 2*P - 6*Q + 3*S;
     zeta = Q - P;
     psi = S - Q;
     sinQ = Math.sin(Q);  // compute some of the more popular ones ...
     cosQ = Math.cos(Q);
     sin2Q = Math.sin(2.*Q);
     cos2Q = Math.cos(2.*Q);
     sinV = Math.sin(V);
     cosV = Math.cos(V);
     sinZeta = Math.sin(zeta);
     cosZeta = Math.cos(zeta);
     sin2Zeta = Math.sin(2*zeta);
     cos2Zeta = Math.cos(2*zeta);

     L_0[4] = L_0[4]
	+ (0.331364 - 0.010281*ups - 0.004692*ups*ups)*sinV
	+ (0.003228 - 0.064436*ups + 0.002075*ups*ups)*cosV
	- (0.003083 + 0.000275*ups - 0.000489*ups*ups)*Math.sin(2*V)
	+ 0.002472 * Math.sin(W) + 0.013619 * sinZeta + 0.018472 * sin2Zeta
	+ 0.006717 * Math.sin(3*zeta)
	+ (0.007275  - 0.001253*ups) * sinZeta * sinQ
	+ 0.006417 * sin2Zeta * sinQ
	- (0.033839 + 0.001253 * ups) * cosZeta * sinQ
	- (0.035681 + 0.001208 * ups) * sinZeta * sinQ;
	/* only part of the terms, the ones first on the list and
	   selected larger-amplitude terms from farther down. */

     ecc[4] = ecc[4] + 1e-7 * (
	  (3606 + 130 * ups - 43 * ups*ups) * sinV
	+ (1289 - 580 * ups) * cosV - 6764 * sinZeta * sinQ
	- 1110 * sin2Zeta * sinQ
	+ (1284 + 116 * ups) * cosZeta * sinQ
	+ (1460 + 130 * ups) * sinZeta * cosQ
	+ 6074 * cosZeta * cosQ);

     omega[4] = omega[4]
	+ (0.007192 - 0.003147 * ups) * sinV
	+ ( 0.000197*ups*ups - 0.00675*ups - 0.020428) * cosV
	+ 0.034036 * cosZeta * sinQ + 0.037761 * sinZeta * cosQ;

     a[4] = a[4] + 1.0e-6 * (
	205 * cosZeta - 263 * cosV + 693 * cos2Zeta + 312 * Math.sin(3*zeta)
	+ 147 * Math.cos(4*zeta) + 299 * sinZeta * sinQ
	+ 181 * cos2Zeta * sinQ + 181 * cos2Zeta * sinQ
	+ 204 * sin2Zeta * cosQ + 111 * Math.sin(3*zeta) * cosQ
	- 337 * cosZeta * cosQ - 111 * cos2Zeta * cosQ
	);

     // Saturn = 5
      incl[5] = 2.492519 - 0.00034550*T - 7.28e-7*Tsq;
      Omega[5] = 112.790414 + 0.8731951*T - 0.00015218*Tsq - 5.31e-6*Tcb ;
      omega[5] = 91.098214 + 1.9584158*T + 8.2636e-4*Tsq;
      a[5] = 9.554747;
      daily[5] = 0.0334978749897;
      ecc[5] = 0.05589232 - 3.4550e-4 * T - 7.28e-7*Tsq;
      L_0[5] = 266.564377 + 1223.509884*T + 0.0003245*Tsq - 5.8e-6*Tcb
	+ (0.018150*ups - 0.814181 + 0.016714 * ups*ups) * sinV
	+ (0.160906*ups - 0.010497 - 0.004100 * ups*ups) * cosV
	+ 0.007581 * Math.sin(2*V) - 0.007986 * Math.sin(W)
	- 0.148811 * sinZeta - 0.040786*sin2Zeta
	- 0.015208 * Math.sin(3*zeta) - 0.006339 * Math.sin(4*zeta)
	- 0.006244 * sinQ
	+ (0.008931 + 0.002728 * ups) * sinZeta * sinQ
	- 0.016500 * sin2Zeta * sinQ
	- 0.005775 * Math.sin(3*zeta) * sinQ
	+ (0.081344 + 0.003206 * ups) * cosZeta * sinQ
	+ 0.015019 * cos2Zeta * sinQ
	+ (0.085581 + 0.002494 * ups) * sinZeta * cosQ
	+ (0.025328 - 0.003117 * ups) * cosZeta * cosQ
	+ 0.014394 * cos2Zeta * cosQ;   /* truncated here -- no
		      terms larger than 0.01 degrees, but errors may
		      accumulate beyond this.... */
      ecc[5] = ecc[5] + 1.0e-7 * (
	  (2458. * ups - 7927.) * sinV + (13381. + 1226. * ups) * cosV
	+ 12415. * sinQ + 26599. * cosZeta * sinQ
	- 4687. * cos2Zeta * sinQ - 12696. * sinZeta * cosQ
	- 4200. * sin2Zeta * cosQ +(2211. - 286*ups) * sinZeta*sin2Q
	- 2208. * sin2Zeta * sin2Q
	- 2780. * cosZeta * sin2Q + 2022. * cos2Zeta*sin2Q
	- 2842. * sinZeta * cos2Q - 1594. * cosZeta * cos2Q
	+ 2162. * cos2Zeta*cos2Q );  /* terms with amplitudes
	    > 2000e-7;  some secular variation ignored. */
      omega[5] = omega[5]
	+ (0.077108 + 0.007186 * ups - 0.001533 * ups*ups) * sinV
	+ (0.045803 - 0.014766 * ups - 0.000536 * ups*ups) * cosV
	- 0.075825 * sinZeta * sinQ - 0.024839 * sin2Zeta*sinQ
	- 0.072582 * cosQ - 0.150383 * cosZeta * cosQ +
	0.026897 * cos2Zeta * cosQ;  /* all terms with amplitudes
	    greater than 0.02 degrees -- lots of others! */
      a[5] = a[5] + 1.0e-6 * (
	2933. * cosV + 33629. * cosZeta - 3081. * cos2Zeta
	- 1423. * Math.cos(3*zeta) + 1098. * sinQ - 2812. * sinZeta * sinQ
	+ 2138. * cosZeta * sinQ  + 2206. * sinZeta * cosQ
	- 1590. * sin2Zeta*cosQ + 2885. * cosZeta * cosQ
	+ 2172. * cos2Zeta * cosQ);  /* terms with amplitudes greater
	   than 1000 x 1e-6 */

      // Uranus = 6
      incl[6] = 0.772464 + 0.0006253*T + 0.0000395*Tsq;
      Omega[6] = 73.477111 + 0.4986678*T + 0.0013117*Tsq;
      omega[6] = 171.548692 + 1.4844328*T + 2.37e-4*Tsq - 6.1e-7*Tcb;
      a[6] = 19.21814;
      daily[6] = 1.1769022484e-2;
      ecc[6] = 0.0463444 - 2.658e-5 * T;
      L_0[6] = 244.197470 + 429.863546*T + 0.000316*Tsq - 6e-7*Tcb;
      /* stick in a little bit of perturbation -- this one really gets
         yanked around.... after Meeus p. 116*/
      G = (83.76922 + 218.4901 * T)/Const.DEG_IN_RADIAN;
      H = 2*G - S;

      sinH = Math.sin(H); sin2H = Math.sin(2.*H);
      cosH = Math.cos(H); cos2H = Math.cos(2.*H);

      L_0[6] = L_0[6] + (0.864319 - 0.001583 * ups) * sinH
   	+ (0.082222 - 0.006833 * ups) * cosH
   	+ 0.036017 * sin2H;
      omega[6] = omega[6] + 0.120303 * sinH
   	+ (0.019472 - 0.000947 * ups) * cosH
   	+ 0.006197 * sin2H;
      ecc[6] = ecc[6] + 1.0e-7 * (
   	20981. * cosH - 3349. * sinH + 1311. * cos2H);
      a[6] = a[6] - 0.003825 * cosH;

      /* other corrections to "true longitude" are ignored. */

      // Neptune = 7
      incl[7] = 1.779242 - 9.5436e-3 * T - 9.1e-6*Tsq;
      Omega[7] = 130.681389 + 1.0989350 * T + 2.4987e-4*Tsq - 4.718e-6*Tcb;
      omega[7] = 46.727364 + 1.4245744*T + 3.9082e-3*Tsq - 6.05e-7*Tcb;
      a[7] = 30.10957;
      daily[7] = 6.020148227e-3;
      ecc[7] = 0.00899704 + 6.33e-6 * T;
      L_0[7] = 84.457994 + 219.885914*T + 0.0003205*Tsq - 6e-7*Tcb;
      L_0[7] = L_0[7]
	- (0.589833 - 0.001089 * ups) * sinH
	- (0.056094 - 0.004658 * ups) * cosH
	- 0.024286 * sin2H;
      omega[7] = omega[7] + 0.024039 * sinH
	- 0.025303 * cosH;
      ecc[7] = ecc[7] + 1.0e-7 * (
	4389. * sinH + 1129. * sin2H
	+ 4262. * cosH + 1089. * cos2H);
      a[7] = a[7] + 8.189e-3 * cosH;

      // Pluto = 8; very approx elements, osculating for Sep 15 1992.
      d = jd_el - 2448880.5;  /* 1992 Sep 15 */
      T = d / 36525.;
      incl[8] = 17.1426;
      Omega[8] = 110.180;
      omega[8] = 223.782;
      a[8] = 39.7465;
      daily[8] = 0.00393329;
      ecc[8] = 0.253834;
      L_0[8] = 228.1027 + 0.00393329 * d;

  }
/*================================================================================================
/
/=================================================================================================*/
  public double [] planetxyz(int p, double jd) {
  /** produces ecliptic X,Y,Z coords for planet number 'p' at date jd. */

     double M, omnotil, nu, r;
     double e, LL, Om, om, nuu, ii;
     double [] retvals = {0.,0.,0.};

  // 1992 Astronomical Almanac p. E4 has these formulae.

     ii = incl[p] / Const.DEG_IN_RADIAN;
     e = ecc[p];

     LL = (daily[p] * (jd - jd_el) + L_0[p]) / Const.DEG_IN_RADIAN;
     Om = Omega[p] / Const.DEG_IN_RADIAN;
     om = omega[p] / Const.DEG_IN_RADIAN;

     M = LL - om;
     omnotil = om - Om;
     // approximate formula for Kepler equation solution ...
     nu = M + (2.*e - 0.25 * Math.pow(e,3.)) * Math.sin(M) +
	     1.25 * e * e * Math.sin(2 * M) +
	     1.08333333 * Math.pow(e,3.) * Math.sin(3 * M);
     r = a[p] * (1. - e*e) / (1 + e * Math.cos(nu));

     retvals[0] = r *
         (Math.cos(nu + omnotil) * Math.cos(Om) - Math.sin(nu +  omnotil) *
               Math.cos(ii) * Math.sin(Om));
     retvals[1] = r *
         (Math.cos(nu +  omnotil) * Math.sin(Om) + Math.sin(nu +  omnotil) *
		Math.cos(ii) * Math.cos(Om));
     retvals[2] = r * Math.sin(nu +  omnotil) * Math.sin(ii);

     return retvals;

  }
/*================================================================================================
/
/=================================================================================================*/
  public double [] planetvel(int p, double jd)  {

  /* numerically evaluates planet velocity by brute-force
  numerical differentiation. Very unsophisticated algorithm. */

	double dt; /* timestep */
	double x1,y1,z1,x2,y2,z2,r1,d1,r2,d2,ep1;
        double [] pos1 = {0.,0.,0.};
        double [] pos2 = {0.,0.,0.};
        double [] retval = {0.,0.,0.};
        int i;

	dt = 0.1 / daily[p]; /* time for mean motion of 0.1 degree */
	pos1 = planetxyz(p, (jd - dt));
	pos2 = planetxyz(p, (jd + dt));
        for (i = 0; i < 3; i ++) retval[i] = 0.5 * (pos2[i] - pos1[i]) / dt;
        return retval;
	/* answer should be in ecliptic coordinates, in AU per day.*/
   }
/*================================================================================================
/
/=================================================================================================*/
  public  double modulus (double [] a) {
      int i;
      double result = 0.;

      for(i = 0; i < a.length; i++)  result += a[i] * a[i];
      return Math.sqrt(result);

   }
/*================================================================================================
/
/=================================================================================================*/
  public  double dotprod (double [] a, double [] b) {
      int i;
      double result = 0.;
      for(i = 0; i < a.length; i++) result += a[i] * b[i];
      return result;
   }
/*================================================================================================
/
/=================================================================================================*/
   public void computemags() {
        /* assumes xyz[][] has been updated.  All the calculations are relative,
           so it doesn't matter what the system is. */
         int i, j;
         double sun2planet, earth2planet, phasefac;
         double [] dxyz = {0., 0., 0.};

         for (i = 0; i < 9; i++) {
             if(i != 2) {  // skip earth
                 sun2planet = modulus(xyz[i]);
                 for(j = 0; j < 3; j++)  {
                    dxyz[j] = xyz[i][j] - xyz[2][j];
                 }
                 earth2planet = modulus(dxyz);
                 phasefac = 0.5 * (dotprod(xyz[i] , dxyz) / (sun2planet * earth2planet) + 1.);
                 // this should be the illuminated fraction.

                 mags[i] = V0[i] +
                   2.5 * Math.log10(phasefac) + 5. * Math.log10(sun2planet * earth2planet);
              }
              else {
                 mags[i] = -99.;   // earth
              }
         }

        // for(i = 0; i < 9; i++) System.out.printf("%s  %f\n",names[i],mags[i]);
   }
/*================================================================================================
/
/=================================================================================================*/
    public  Celest earthview(double [] earthxyz, double [] planxyz, double jd) {
        double [] dxyz = {0.,0.,0.};
        double [] retvals = {0.,0.,0.};
        double eq;
        int i;

        eq = 2000. + (jd - Const.J2000) / 365.25;
        for(i = 0; i < 3; i++) {
           dxyz[i] = planxyz[i] - earthxyz[i];
        }
        retvals = Celest.XYZcel(dxyz[0],dxyz[1],dxyz[2]);
        Celest ViewFromEarth = new Celest(retvals[0],retvals[1],eq,retvals[2]);
        return ViewFromEarth;
   }
/*================================================================================================
/
/=================================================================================================*/
   public Observation [] pposns(WhenWhere w) {
        /* returns Observations for all the planets. */

        int i;
        double [] earthxyz = {0.,0.,0.};
        double [] eclipt = {0.,0.,0.};
        double [] equat  = {0.,0.,0.};
        double [] ecliptvel   = {0.,0.,0.};
        double [] equatvel   = {0.,0.,0.};
        Celest planetcel;

        Observation [] planetpos = new Observation [9];

        xyz = new double [9][3];
        xyzvel = new double [9][3];

        // compute_el(w.when.jd);   refresh the planetary elements

        eclipt = planetxyz(2,w.when.jd);   // earth  ... do separately
        earthxyz = Ecliptic.eclrot(w.when.jd,eclipt[0],eclipt[1],eclipt[2]);
             xyz[2][0] = earthxyz[0];  xyz[2][1] = earthxyz[1];
                                     xyz[2][2] = earthxyz[2];
        ecliptvel = planetvel(2,w.when.jd);
        equatvel = Ecliptic.eclrot(w.when.jd,ecliptvel[0],ecliptvel[1],
                                             ecliptvel[2]);
        xyzvel[2][0] = equatvel[0]; xyzvel[2][1] = equatvel[1];
                                     xyzvel[2][2] = equatvel[2];

        for(i = 0; i < 9; i++) {
          if(i != 2)  {  // skip earth
             eclipt = planetxyz(i,w.when.jd);
             equat = Ecliptic.eclrot(w.when.jd,eclipt[0],eclipt[1],eclipt[2]);
             // save xyz position of planet for barycentric correction.
             xyz[i][0] = equat[0];  xyz[i][1] = equat[1];
                                     xyz[i][2] = equat[2];
             // and the velocities, too
             ecliptvel = planetvel(i,w.when.jd);
             equatvel = Ecliptic.eclrot(w.when.jd,ecliptvel[0],ecliptvel[1],
                                                  ecliptvel[2]);
             xyzvel[i][0] = equatvel[0]; xyzvel[i][1] = equatvel[1];
                                          xyzvel[i][2] = equatvel[2];
             planetcel = earthview(earthxyz,equat,w.when.jd);
             planetpos[i] = new Observation(w,planetcel);
           //  System.out.printf("i %d %s\n",i,planetpos[i].ha.RoundedHAString(0,":"));
          }
          else {         // earth
             planetpos[i] = new Observation(w,new Celest(0.,0.,2000.,0.));
          }
        }
        return planetpos;
   }
 /*================================================================================================
/
/=================================================================================================*/
   public void printxyz() {  // for diagn.
      int i, j;
      for(i = 0; i < 9; i++) {
         System.out.printf("%d ",i);
         for(j = 0; j < 3; j++)
            System.out.printf("%f ",xyz[i][j]);
         System.out.printf("  ");
         for(j = 0; j < 3; j++)
            System.out.printf("%f ",xyzvel[i][j]);
         System.out.printf("\n");
      }
   }
/*================================================================================================
/
/=================================================================================================*/
   public void ComputeBaryCor () {
     // Using PREVIOUSLY COMPUTED xyz and xyzvel, computes the offset to the
     // barycenter.
       int i, j;

       barycor = new double [6];
       barycor[0] = 0.; barycor[1] = 0.; barycor[2] = 0.;
       barycor[3] = 0.; barycor[4] = 0.; barycor[5] = 0.;

       for(i = 0; i < 9; i++) {
          for(j = 0; j < 3; j++) {
             barycor[j] = barycor[j] + xyz[i][j] * mass[i];
             barycor[j+3] = barycor[j+3] + xyzvel[i][j] * mass[i];
          }
       }
       for(j = 0; j < 3; j++) {
          barycor[j] = barycor[j] / Const.SS_MASS;
          barycor[j+3] = barycor[j+3] / Const.SS_MASS;
       }
   }
}


