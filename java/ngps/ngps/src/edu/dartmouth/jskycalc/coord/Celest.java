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
import java.util.*;
/*================================================================================================
/     class Celest implements Cloneable
/=================================================================================================*/
public class Celest implements Cloneable {

   // A celestial coordinate, which knows its equinox.

   public RA Alpha;
   public dec Delta;
   public double Equinox;
   public double distance;  // not always used but sometimes useful
   public double galat, galong;  //  galactic coords, in degrees.
/*================================================================================================
/     Celest(RA a, dec d, double e)
/=================================================================================================*/
   // To do:  Add an elaborate input parsing mechanism and overload
   // constructors like crazy.
   public Celest() {
      Alpha = new RA(0.0);;
      Delta = new dec(0.0);
      Equinox = 2000.0;
      distance = 0.;
   }
/*================================================================================================
/     Celest(RA a, dec d, double e)
/=================================================================================================*/
   // To do:  Add an elaborate input parsing mechanism and overload
   // constructors like crazy.
   public Celest(RA a, dec d, double e) {
      Alpha = a;
      Delta = d;
      Equinox = e;
      distance = 0.;
   }
/*================================================================================================
/     Celest(double r, double d, double e)
/=================================================================================================*/
   public Celest(double r, double d, double e) {  // ra, dec, and equinox
                                         // decimal hours, degr.
      Alpha = new RA(r);
      Delta = new dec(d);
      Equinox = e;
      distance = 0.;
   }
/*================================================================================================
/     Celest(double r, double d, double e, double dist)
/=================================================================================================*/
   public Celest(double r, double d, double e, double dist) {
                   // ra, dec, and equinox decimal hours, degr; dist in
                   // undefined units.
      Alpha = new RA(r);
      Delta = new dec(d);
      Equinox = e;
      distance = dist;
   }
/*================================================================================================
/     Celest(String [] s)
/=================================================================================================*/
   public Celest(String [] s) {  // ra, dec, and equinox as separate strings
      Alpha = new RA(s[0]);
      Delta = new dec(s[1]);
      Equinox = Double.parseDouble(s[2]);
      distance = 0.;
   }
/*================================================================================================
/      Celest(String ras, String decs, String eqs)
/=================================================================================================*/
   public Celest(String ras, String decs, String eqs) {
             // ra, dec, and equinox as separate strings not in an array
      Alpha = new RA(ras);
      Delta = new dec(decs);
      Equinox = Double.parseDouble(eqs);
      distance = 0.;
   }
/*================================================================================================
/     Celest(String s)
/=================================================================================================*/
   public Celest(String s) {  // colon-separated ra, dec, equinox (3 string)
      String [] fields;

      fields = s.split("\\s+");  // whitespace
      Alpha = new RA(fields[0]);
      Delta = new dec(fields[1]);
      Equinox = Double.parseDouble(fields[2]);
      distance = 0.;
   }
/*================================================================================================
/     equals( Object arg )
/=================================================================================================*/
   public boolean equals( Object arg ) {
       // override (I hope) the Object equals method to use in at least one test later.
       if(( arg != null) && (arg instanceof Celest)) {
          Celest c = (Celest) arg;
          if(c.Alpha.value == Alpha.value && c.Delta.value == Delta.value &&
              c.Equinox == Equinox) return true;
          else return false;
       }
       else return false;
   }
/*================================================================================================
/     UpdateFromStrings(String rastr, String decstr, String eqstr)
/=================================================================================================*/
  public void UpdateFromStrings(String rastr, String decstr, String eqstr) {
      // updates a previously instantiated celest from three strings.
      Alpha.setRA(rastr);
      Delta.setDec(decstr);
      // This can crash if the user has erased the field during an upate.
      try {
         Equinox = Double.parseDouble(eqstr);
      } catch (NumberFormatException e) {
         Equinox = 2000.;  // unsatisfactory, but the most likely case
      }
   }
/*================================================================================================
/     Celest clone()
/=================================================================================================*/
   public Celest clone() {
      try {
          Celest copy = (Celest) super.clone();
          copy.Alpha = (RA) Alpha.clone();
          copy.Delta = (dec) Delta.clone();
          copy.Equinox = Equinox;
          copy.distance = distance;
          copy.galat = galat;
          copy.galong = galong;
          return copy;
      } catch (CloneNotSupportedException e) {
          throw new Error("This should never happen!");
      }
   }
/*================================================================================================
/     checkstring()
/=================================================================================================*/
  public String checkstring() {
      String outstr = "";
      outstr = outstr + Alpha.RoundedRAString(2,":") + "  ";
      outstr = outstr + Delta.RoundedDecString(1,":") + "  ";
      outstr = outstr + String.format(Locale.ENGLISH, "%6.1f",Equinox);
      return outstr;
   }
/*================================================================================================
/       shortstring()
/=================================================================================================*/
  public String shortstring() {
      String outstr = "";
      outstr = outstr + Alpha.RoundedRAString(1,":") + "  ";
      outstr = outstr + Delta.RoundedDecString(0,":") + "  ";
      //outstr = outstr + String.format(Locale.ENGLISH, "%6.1f",Equinox);
      return outstr;
   }
/*================================================================================================
/     XYZcel(double x, double y, double z)
/=================================================================================================*/
  public static double [] XYZcel(double x, double y, double z) {
   /** Given x, y, and z, returns {ra, dec, distance}.  */

      double mod;
      double xy;
      double raout, decout;
      double [] radecReturn = {0.,0.,0.};

      mod = Math.sqrt(x*x + y*y + z*z);
      if(mod > 0.) {
         x = x / mod; y = y / mod; z = z / mod;
      }
      else {
         System.out.println("Zero modulus in XYZcel.");
         radecReturn[0] = 0.; radecReturn[1] = 0.;
         radecReturn[2] = 0.;
         return radecReturn;
      }

      // System.out.printf("XYZcel: %f %f %f ->\n",x,y,z);
      xy = Math.sqrt(x * x + y * y);
      if(xy < 1.0e-11) {  // on the pole
         raout = 0.;      // ra is degenerate
         decout = Const.PI_OVER_2;
         if(z < 0.) decout *= -1.;
      }
      else {
         raout = Math.atan2(y,x) * Const.HRS_IN_RADIAN;
         decout = Math.asin(z) * Const.DEG_IN_RADIAN;
      }
      // System.out.printf("%f %f\n",raout,decout);
      radecReturn[0] = raout;
      radecReturn[1] = decout;
      radecReturn[2] = mod;  // it will sometimees be useful to have the dist.

      return radecReturn;
   }
/*================================================================================================
/      cel_unitXYZ()
/=================================================================================================*/
  public double [] cel_unitXYZ() {
      /** Given instance variables, returns UNIT VECTOR {x,y,z}. */
      double [] retvals = {0.,0.,0.};
      double cosdec;

      cosdec = Math.cos(Delta.radians());
      retvals[0] = Math.cos(Alpha.radians()) * cosdec;
      retvals[1] = Math.sin(Alpha.radians()) * cosdec;
      retvals[2] = Math.sin(Delta.radians());

      return retvals;
   }
/*================================================================================================
/     precess(double NewEquinox)
/=================================================================================================*/
  public double [] precess(double NewEquinox) {
      /** generates a unit vector XYZ in equinox NewEquinox from the current
          Alpha, Delta, and Equinox.  I believe these are IUA 1976 precession
          constants, which are not fully up-to-date but which are close
          enough for most puropses. */
      double ti, tf, zeta, z, theta;
      double cosz, coszeta, costheta, sinz, sinzeta, sintheta;
      double [][] p = {{0.,0.,0.,},{0.,0.,0.},{0.,0.,0.}};
      double [] orig = {0.,0.,0.};
      double [] fin = {0.,0.,0.};

      int i, j;


      //System.out.printf("equinoxes %f %f\n",Equinox,NewEquinox);
      ti = (Equinox - 2000.) / 100.;
      tf = (NewEquinox - 2000. - 100. * ti) / 100.;

      zeta = (2306.2181 + 1.39656 * ti + 0.000139 * ti * ti) * tf +
         (0.30188 - 0.000344 * ti) * tf * tf + 0.017998 * tf * tf * tf;
      z = zeta + (0.79280 + 0.000410 * ti) * tf * tf + 0.000205 * tf * tf * tf;
      theta = (2004.3109 - 0.8533 * ti - 0.000217 * ti * ti) * tf
         - (0.42665 + 0.000217 * ti) * tf * tf - 0.041833 * tf * tf * tf;

      cosz = Math.cos(z / Const.ARCSEC_IN_RADIAN);
      coszeta = Math.cos(zeta / Const.ARCSEC_IN_RADIAN);
      costheta = Math.cos(theta / Const.ARCSEC_IN_RADIAN);
      sinz = Math.sin(z / Const.ARCSEC_IN_RADIAN);
      sinzeta = Math.sin(zeta / Const.ARCSEC_IN_RADIAN);
      sintheta = Math.sin(theta / Const.ARCSEC_IN_RADIAN);

      p[0][0] = coszeta * cosz * costheta - sinzeta * sinz;
      p[0][1] = -1. * sinzeta * cosz * costheta - coszeta * sinz;
      p[0][2] = -1. * cosz * sintheta;

      p[1][0] = coszeta * sinz * costheta + sinzeta * cosz;
      p[1][1] = -1. * sinzeta * sinz * costheta + coszeta * cosz;
      p[1][2] = -1. * sinz * sintheta;

      p[2][0] = coszeta * sintheta;
      p[2][1] = -1. * sinzeta * sintheta;
      p[2][2] = costheta;

      orig[0] = Math.cos(Delta.radians()) * Math.cos(Alpha.radians());
      orig[1] = Math.cos(Delta.radians()) * Math.sin(Alpha.radians());
      orig[2] = Math.sin(Delta.radians());

      for(i = 0; i < 3; i++) {  // matrix multiplication
         fin[i] = 0.;
         //System.out.printf("orig[%d] = %f\n",i,orig[i]);
         for(j = 0; j < 3; j++) {
             //System.out.printf("%d%d: %f  ",i,j,p[i][j]);
             fin[i] += p[i][j] * orig[j];
         }
         //System.out.printf("\nfin[%d] = %f\n\n",i,fin[i]);
      }
      return XYZcel(fin[0],fin[1],fin[2]);

   }
/*================================================================================================
/     selfprecess(double newEquinox)
/=================================================================================================*/
   public void selfprecess(double newEquinox) {
      /** precesses a Celest in place. */
      double [] radecOut;
      radecOut = precess(newEquinox);
      Alpha.setRA(radecOut[0]);
      Delta.setDec(radecOut[1]);
      Equinox = newEquinox;
   }
/*================================================================================================
/     precessed(double newEquinox)
/=================================================================================================*/
   public Celest precessed(double newEquinox) {
      /** returns a new Celest precessed from this one. */
      double [] radecOut;
      radecOut = precess(newEquinox);
      // System.out.printf("radecOut %f %f\n",radecOut[0],radecOut[1]);
      Celest outputCel = new Celest(radecOut[0],radecOut[1],newEquinox);
      return outputCel;
   }
/*================================================================================================
/       galactic()
/=================================================================================================*/
  public void galactic() {
      /** computes instance variables galong and galat.  Algorithm is
          rigorous. */

      Celest cel1950 = precessed(1950);
      double [] xyz;
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

	xyz = cel1950.cel_unitXYZ();

        xyzgal[0] = xyz[0] * p11 + xyz[1] * p12 + xyz[2] * p13;
        xyzgal[1] = xyz[0] * p21 + xyz[1] * p22 + xyz[2] * p23;
        xyzgal[2] = xyz[0] * p31 + xyz[1] * p32 + xyz[2] * p33;
        temp = XYZcel(xyzgal[0],xyzgal[1],xyzgal[2]);
        galong = temp[0] * 15.;
        while (galong < 0.) galong += 360.;
        galat = temp[1];
   }
}

