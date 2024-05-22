/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package edu.caltech.palomar.telescopes.guider.catalog.tycho2;
//=== File Prolog =============================================================
//
//
//      It has been refactored and modified by Jennifer Milburn for
//      use at the Palomar Observatory.
//
//      Caltech Optical Observatories, Palomar Observatory
//
//--- Contents ----------------------------------------------------------------
//       Tycho2star - an object representation of a typcho-2 spectral type star
//
//--- Description -------------------------------------------------------------
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
// TERMS OF USE --
//      June 21,2011   - original development Jennifer Milburn
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
import edu.dartmouth.jskycalc.coord.Celest;
import edu.dartmouth.jskycalc.coord.sexagesimal;
import edu.dartmouth.jskycalc.coord.RA;
import edu.dartmouth.jskycalc.coord.dec;
/*================================================================================================
/        tycho2star
/=================================================================================================*/
/* TYCHO-2  spectral class. */
public class tycho2star implements Cloneable {
/*================================================================================================
/    tycho2star
/=================================================================================================*/
public java.lang.String TYC;
public java.lang.String TYC1;
public java.lang.String TYC2;
public java.lang.String TYC3;
public double           ra;
public double           dec;
public double           Vmag;
public double           Bmag;
public java.lang.String spec_type_source;
public java.lang.String name;
public double           distance;
public double           magnitude;
public java.lang.String magnitude_type;
public java.lang.String temperature_class;
public java.lang.String temperature_subclass;
public int              luminosity;
public double           temperature;
public java.lang.String spectral_type;
public java.lang.String tycho_name;
public Celest           celest;
public java.lang.String ra_string;
public java.lang.String dec_string;
public Vector columnVector;
/*================================================================================================
/   ucac3Star(byte raw[])
/=================================================================================================*/
   // constructor makes instance from a single string from the catalog
public    tycho2star(java.lang.String raw) {
    initialize();
//Byte-by-byte Description of file: catalog.dat
//--------------------------------------------------------------------------------
//   Bytes Format  Units   Label    Explanations
//--------------------------------------------------------------------------------
//   1-  3  A3     ---     ---      [TYC] Tycho-2 label
//   5-  8  I4     ---     TYC1     First part of Tycho-2 identifier
//  10- 14  I5     ---     TYC2     Second part of Tycho-2 identifier
//      16  I1     ---     TYC3     Third part of Tycho-2 identifier
//  18- 29  F12.8  deg     RAdeg    Right Ascension, J2000, decimal deg.
//  31- 42  F12.8  deg     DEdeg    Declination, J2000, decimal deg.
//  44- 49  F6.3   mag     VTmag    ?=99.99 Tycho-2 V_T_ magnitude
//  51- 56  F6.3   mag     BTmag    ?=99.99 Tycho-2 B_T_ magnitude
//  58- 60  A3     ---   r_SpType   Source of spectral type (2)
//  62- 76  A15    ---     Name     Alternate designation for star (3)
//  78- 83  F6.3   arcsec  Dist     Distance between Tycho object and
//                                   spectral type match (4)
//  85- 90  F6.2   mag     Mag      ?=99.99 Magnitude from SpType catalog (5)
//      92  A1     ---   f_Mag      [VPBX*] Flag indicating type of magnitude (6)
//      94  A1     ---     TClass   Temperature class (7)
//      95  I1     ---     SClass   ? Temperature Subclass (7)
//      97  I1     ---     LClass   ? Luminosity class in numeric form (7)
//  99-103  I5     K       Teff     Effective temperature of the star, based on
//                                   spectral type (G1)
// 105-124  A20    ---     SpType   Spectral Type (1)
//--------------------------------------------------------------------------------
 try{
 java.util.StringTokenizer st = new java.util.StringTokenizer(raw);
     TYC                  = st.nextToken(" ");
     TYC1                 = st.nextToken(" ");
     TYC2                 = st.nextToken(" ");
     TYC3                 = st.nextToken(" ");
     tycho_name           = TYC+TYC1+"-"+TYC2+"-"+TYC3;
     ra                   = Double.parseDouble(st.nextToken(" "));
     dec                  = Double.parseDouble(st.nextToken(" "));
     celest               = new Celest(ra/15.0,dec,2000);
     ra_string            = celest.Alpha.RoundedRAString(2, ":");
     dec_string           = celest.Delta.RoundedDecString(2,":");
     Vmag                 = Double.parseDouble(st.nextToken(" "));
     Bmag                 = Double.parseDouble(st.nextToken(" "));
     spec_type_source     = st.nextToken(" ");
     name                 = raw.substring(61, 75);
     java.util.StringTokenizer st2 = new java.util.StringTokenizer(raw.substring(76));
     distance             = Double.parseDouble(st2.nextToken(" "));
     magnitude            = Double.parseDouble(st2.nextToken(" "));
     magnitude_type       = st2.nextToken(" ");
     temperature_class    = st2.nextToken(" ");
     temperature_subclass = temperature_class.substring(1);
     try{
        luminosity        = Integer.parseInt(st2.nextToken(" "));
     }catch(Exception e){

     }
     temperature          = Double.parseDouble((raw.substring(98,104)).trim());
     spectral_type        = raw.substring(104);
     } catch (Exception e2) {
        System.out.println("Error parsing the catalog file: " + raw);
     }
     initializeDataVector();
   }
/*================================================================================================
/    initializeDataVector()
/=================================================================================================*/
private void initializeDataVector(){
    columnVector = new Vector();
    columnVector.add(TYC);
    columnVector.add(TYC1);
    columnVector.add(TYC2);
    columnVector.add(TYC3);
    columnVector.add(ra);
    columnVector.add(dec);
    columnVector.add(Vmag);
    columnVector.add(Bmag);
    columnVector.add(spec_type_source);
    columnVector.add(name);
    columnVector.add(distance);
    columnVector.add(magnitude);
    columnVector.add(magnitude_type);
    columnVector.add(temperature_class);
    columnVector.add(temperature_subclass);
    columnVector.add(luminosity);
    columnVector.add(temperature);
    columnVector.add(spectral_type);
}
/*================================================================================================
/    ucac3Star()
/=================================================================================================*/
   // or, just make a blank one.
 public   tycho2star() {
     initialize();
   }
private void initialize(){
     TYC       = "TYC";
     TYC1      = "0000";
     TYC2      = "00000";
     TYC3      = "1";
     tycho_name = "none";
     ra        = 0.0;
     dec       = 0.0;
     ra_string = " ";
     dec_string = " ";
     Vmag      = 99.99;
     Bmag      = 99.99;
     spec_type_source = "xxx";
     name      = "unknown";
     distance  = 0.0;
     magnitude = 0.0;
     magnitude_type = "X";
     temperature_class = "XX";
     temperature_subclass = "X";
     luminosity    = 0;
     temperature   = 0.0;
     spectral_type = "none";
      initializeDataVector();
}

/*================================================================================================
/     fill (byte raw[]
/=================================================================================================*/
  public  void fill (java.lang.String raw) {
 java.util.StringTokenizer st = new java.util.StringTokenizer(raw);
     TYC       = st.nextToken(" ");
     TYC1      = st.nextToken(" ");
     TYC2      = st.nextToken(" ");
     TYC3      = st.nextToken(" ");
     ra        = Double.parseDouble(st.nextToken(" "));
     dec       = Double.parseDouble(st.nextToken(" "));
     tycho_name = " ";
     ra_string  = " ";
     dec_string =  " ";
     Vmag      = Double.parseDouble(st.nextToken(" "));
     Bmag      = Double.parseDouble(st.nextToken(" "));
     spec_type_source = st.nextToken(" ");
     name      = st.nextToken(" ");
     distance  = Double.parseDouble(st.nextToken(" "));
     magnitude = Double.parseDouble(st.nextToken(" "));
     magnitude_type = st.nextToken(" ");
     temperature_class = st.nextToken(" ");
     temperature_subclass = st.nextToken(" ");
     luminosity    = Integer.parseInt(st.nextToken(" "));
     temperature   = Double.parseDouble(st.nextToken(" "));
     spectral_type = st.nextToken(" ");
      initializeDataVector();
   }
/*================================================================================================
/    clone()
/=================================================================================================*/
   // this makes it possible to copy a ucac3Star
  public tycho2star clone() {
      try {
         tycho2star copy = (tycho2star) super.clone();
         copy.TYC = TYC;
         copy.TYC1 =    TYC1;
         copy.TYC2 =  TYC2;
         copy.TYC3 = TYC3;
         copy.ra =   ra;
         copy.dec =  dec;
         copy.tycho_name = tycho_name;
         copy.ra_string = ra_string;
         copy.dec_string =  dec_string;
         copy.Vmag =   Vmag;
         copy.Bmag =   Bmag;
         copy.spec_type_source =   spec_type_source;
         copy.name =  name;
         copy.distance  =  distance;
         copy.magnitude  =  magnitude;
         copy.magnitude_type  =  magnitude_type;
         copy.temperature_class = temperature_class;
         copy.temperature_subclass = temperature_subclass;
         copy.luminosity = luminosity;
         copy.temperature =  temperature;
         copy.spectral_type = spectral_type;
         copy.columnVector = columnVector;
         return copy;
      } catch (CloneNotSupportedException e) {
         throw new Error("Error cloning tycho2star.\n");
      }
   }
/*
Byte-by-byte Description of file: catalog.dat
--------------------------------------------------------------------------------
   Bytes Format  Units   Label    Explanations
--------------------------------------------------------------------------------
   1-  3  A3     ---     ---      [TYC] Tycho-2 label
   5-  8  I4     ---     TYC1     First part of Tycho-2 identifier
  10- 14  I5     ---     TYC2     Second part of Tycho-2 identifier
      16  I1     ---     TYC3     Third part of Tycho-2 identifier
  18- 29  F12.8  deg     RAdeg    Right Ascension, J2000, decimal deg.
  31- 42  F12.8  deg     DEdeg    Declination, J2000, decimal deg.
  44- 49  F6.3   mag     VTmag    ?=99.99 Tycho-2 V_T_ magnitude
  51- 56  F6.3   mag     BTmag    ?=99.99 Tycho-2 B_T_ magnitude
  58- 60  A3     ---   r_SpType   Source of spectral type (2)
  62- 76  A15    ---     Name     Alternate designation for star (3)
  78- 83  F6.3   arcsec  Dist     Distance between Tycho object and
                                   spectral type match (4)
  85- 90  F6.2   mag     Mag      ?=99.99 Magnitude from SpType catalog (5)
      92  A1     ---   f_Mag      [VPBX*] Flag indicating type of magnitude (6)
      94  A1     ---     TClass   Temperature class (7)
      95  I1     ---     SClass   ? Temperature Subclass (7)
      97  I1     ---     LClass   ? Luminosity class in numeric form (7)
  99-103  I5     K       Teff     Effective temperature of the star, based on
                                   spectral type (G1)
 105-124  A20    ---     SpType   Spectral Type (1)
--------------------------------------------------------------------------------

Note (1): This is the spectral type of the star, exactly as it appears
     in the original spectral type catalog.

Note (2): This column contains a code for the catalog of origin of the
          spectral type:
     mc1 = Michigan Catalog, Vol. 1, <III/31>
     mc2 = Michigan Catalog, Vol. 2, <III/51>
     mc3 = Michigan Catalog, Vol. 3, <III/80>
     mc4 = Michigan Catalog, Vol. 4, <III/133>
     mc5 = Michigan Catalog, Vol. 5, <III/214>
     j64 = Jaschek et al. 1964, <III/18>
     k83 = Kennedy 1983, <III/78>
     fI  = FK5, Part I, <I/149>
     fII = FK5, Part II, <I/175>
     ppN = PPM North, <I/146>
     ppS = PPM South, <I/193>
     sim = SIMBAD Astronomical Database

Note (3): This is an alternate designation for the star, other than
    its Tycho-2 identifier. It is usually the designation for the star
    that appeared in the spectral type catalog.

Note (4): This is the distance in arcsec between the Tycho-2 object
    and the star from the spectral type catalog to which it was matched.

Note (5): This is the magnitude that appears in the spectral type catalog.
    If no magnitude was included, it will have a value of 99.99.

Note (6): This indicates the type of magnitude that appears in the
          spectral type catalog:
    V = visual
    P = photographic
    B = blue
    X = unknown
    * = no magnitude was included

Note (7):
    In the spectral type reformatting, it was necessary to "choose" a
    concrete spectral type for those that were listed ambiguously, and
    the rule adhered to was to take the first listing. For example, if a
    spectral type was originally listed as K2/3 III, it will be K2 3,
    where K is the temperature class, 2 is the subclass, and 3 is the
    luminosity class.

    Other examples: A9/F2 V, B2.5 V, and G8 IV/V in the original catalog
    become A9 5, B2 5, and G8 4 in the reformatted spectral type

 */
 }
