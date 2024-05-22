/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */ 
 
package edu.caltech.palomar.telescopes.guider.catalog.ucac3;  
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
import edu.dartmouth.jskycalc.util.Conv;
import java.util.List.*;
import java.util.Vector;
/*================================================================================================
/
/=================================================================================================*/

/* UCAC3 guide star section. */

public class ucac3Star implements Cloneable {
   public double ra, dec;
   public float sigra, sigdec;
   public float pmra, pmdec;
   public float sigpmra, sigpmdec;
   public float epra, epdec;  // mean epoch of pos determ; coords are all J2000.
   public float mag1, mag2;   // ucac3 mags
   public float Jmag, Hmag, Kmag;  // from 2MASS
   public float scBmag, scRmag, scImag;  // supercosmos b, r, i
   public int objtype, doubleflag;  // many other flags are ignored.
   public int running;  // in whole UCAC3.
   public Vector columnVector;
/*================================================================================================
/   ucac3Star(byte raw[])
/=================================================================================================*/
   // constructor makes instance from 84-byte raw record.
public    ucac3Star(byte raw[]) {
      ra =     Conv.getint32(raw,0) / 54000000.;
      dec =    Conv.getint32(raw,4) / 3600000. - 90.;
      sigra =  Conv.getint16(raw,36) * 0.1f;
      sigdec = Conv.getint16(raw,38) * 0.1f;
      pmra =   Conv.getint32(raw,28) * 0.1f;
      pmdec =  Conv.getint32(raw,32) * 0.1f;
      mag1 =   Conv.getint16(raw,8) / 1000.f;
      mag2 =   Conv.getint16(raw,10) / 1000.f;
      epra =   Conv.getint16(raw,24) * 0.01f + 1900.f;
      epdec =  Conv.getint16(raw,26) * 0.01f + 1900.f;
      Jmag  =  Conv.getint16(raw,44) / 1000.f;
      Hmag  =  Conv.getint16(raw,46) / 1000.f;
      Kmag  =  Conv.getint16(raw,48) / 1000.f;
      scBmag = Conv.getint16(raw,56) / 1000.f;
      scRmag = Conv.getint16(raw,58) / 1000.f;
      scImag = Conv.getint16(raw,60) / 1000.f;
      objtype =    Conv.getint08(raw,14);
      doubleflag = Conv.getint08(raw,15);
      running = Conv.getint32(raw,80);
      convertToDegrees();
      initializeDataVector();
   }
/*================================================================================================
/    convertToDegrees()
/=================================================================================================*/
private void convertToDegrees(){
    ra = ra*15.0;
}
/*================================================================================================
/    initializeDataVector()
/=================================================================================================*/
private void initializeDataVector(){
    columnVector = new Vector();
    columnVector.add(ra);
    columnVector.add(dec);
    columnVector.add(mag1);
    columnVector.add(mag2);
    columnVector.add(sigra);
    columnVector.add(sigdec);
    columnVector.add(pmra);
    columnVector.add(pmdec);
    columnVector.add(epra);
    columnVector.add(epdec);
    columnVector.add(Jmag);
    columnVector.add(Hmag);
    columnVector.add(Kmag);    
    columnVector.add(scBmag);
    columnVector.add(scRmag);
    columnVector.add(scImag);
    columnVector.add(objtype);
    columnVector.add(doubleflag);
    columnVector.add(running);    
}
/*================================================================================================
/    ucac3Star(byte raw[], int flag)
/=================================================================================================*/
   // or, just fill in minimal info, for fast screening.
public    ucac3Star(byte raw[], int flag) {  // any flag will do, for now.
      ra =     Conv.getint32(raw,0) / 54000000.;
      dec =    Conv.getint32(raw,4) / 3600000. - 90.;
      mag1 =   Conv.getint16(raw,8) / 1000.f;
      objtype =    Conv.getint08(raw,14);
      doubleflag = Conv.getint08(raw,15);
      convertToDegrees();
   }
/*================================================================================================
/    ucac3Star()
/=================================================================================================*/
   // or, just make a blank one.
 public   ucac3Star() {
      ra =     0.;
      dec =    0.;
      sigra =  0.f;
      sigdec = 0.f;
      pmra =   0.f;
      pmdec =  0.f;
      mag1 =   0.f;
      mag2 =   0.f;
      epra =   0.f;
      epdec =  0.f;
      Jmag  =  -99.f;
      Hmag  =  -99.f;
      Kmag  =  -99.f;
      scBmag = -99.f;
      scRmag = -99.f;
      scImag = -99.f;
      objtype =  0;
      doubleflag = 0;
      running = -1;
      convertToDegrees();
      initializeDataVector();
   }
/*================================================================================================
/     fill (byte raw[]
/=================================================================================================*/
  public  void fill (byte raw[]) {  // fills an existing ucac3Star from data.
      ra =     Conv.getint32(raw,0) / 54000000.;
      dec =    Conv.getint32(raw,4) / 3600000. - 90.;
      sigra =  Conv.getint16(raw,36) * 0.1f;
      sigdec = Conv.getint16(raw,38) * 0.1f;
      pmra =   Conv.getint32(raw,28) * 0.1f;
      pmdec =  Conv.getint32(raw,32) * 0.1f;
      mag1 =   Conv.getint16(raw,8) / 1000.f;
      mag2 =   Conv.getint16(raw,10) / 1000.f;
      epra =   Conv.getint16(raw,24) * 0.01f + 1900.f;
      epdec =  Conv.getint16(raw,26) * 0.01f + 1900.f;
      Jmag  =  Conv.getint16(raw,44) / 1000.f;
      Hmag  =  Conv.getint16(raw,46) / 1000.f;
      Kmag  =  Conv.getint16(raw,48) / 1000.f;
      scBmag = Conv.getint16(raw,56) / 1000.f;
      scRmag = Conv.getint16(raw,58) / 1000.f;
      scImag = Conv.getint16(raw,60) / 1000.f;
      objtype =    Conv.getint08(raw,14);
      doubleflag = Conv.getint08(raw,15);
      running = Conv.getint32(raw,80);
      convertToDegrees();
      initializeDataVector();
   }
/*================================================================================================
/    clone()
/=================================================================================================*/
   // this makes it possible to copy a ucac3Star
  public ucac3Star clone() {
      try {
         ucac3Star copy = (ucac3Star) super.clone();
         copy.ra = ra;
         copy.dec =    dec;
         copy.sigra =  sigra;
         copy.sigdec = sigdec;
         copy.pmra =   pmra;
         copy.pmdec =  pmdec;
         copy.mag1 =   mag1;
         copy.mag2 =   mag2;
         copy.epra =   epra;
         copy.epdec =  epdec;
         copy.Jmag  =  Jmag;
         copy.Hmag  =  Hmag;
         copy.Kmag  =  Kmag;
         copy.scBmag = scBmag;
         copy.scRmag = scRmag;
         copy.scImag = scImag;
         copy.objtype =  objtype;
         copy.doubleflag = doubleflag;
         copy.running = running;
         copy.columnVector = columnVector;
         return copy;
      } catch (CloneNotSupportedException e) {
         throw new Error("Error cloning ucac3Star.\n");
      }
   }
}



