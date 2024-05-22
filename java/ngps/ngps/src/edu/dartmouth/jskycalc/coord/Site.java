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
/*================================================================================================
/
/=================================================================================================*/
public class Site implements Cloneable {

   // Complicated class holding much data about sites, which at this point
   // are all `canned'.

   public String name;          // name of site
   public latitude lat;         // geographic latitude
   public longitude longit;     // longitude
   public double stdz;          // time zone offset from UT, decimal hrs
   public int use_dst;          // code for DST use, 0 = none, 1 = US, etc.
   public String timezone_name; // name of timezone e.g. "Mountain"
   public String zone_abbrev;   // one-letter abbreviation of timezone
   public double elevsea;       // elevation above sea level, meters
   public double elevhoriz;     // elevation above the typical local horizon, m.

//   Site(HashMap<String, Site> sitelist, String sitename) {
//      Reload(sitelist, sitename);
//   }

//   void Reload(Site [] sitelist, string sitename) {
//        Site tempsite = sitelist.get(sitename);
//        name = tempsite.name;
//        longit = new longitude(tempsite.longit.value);
//        lat = new latitude(tempsite.lat.value);
//        stdz = tempsize.stdz;
//        use_dst = tempsite.use_dst;
//        timezone_name = tempsite.timezone_name;
//        zone_abbrev = tempsite.zone_abbrev;
//        elevsea = tempsite.elevsea;
//        elevhoriz = tempsite.elev_horiz;
//   }
/*================================================================================================
/
/=================================================================================================*/
  public void Reload(Site s) {
        name = s.name;
        longit = (longitude) s.longit.clone();
        lat = (latitude) s.lat.clone();
        stdz = s.stdz;
        use_dst = s.use_dst;
        timezone_name = s.timezone_name;
        zone_abbrev = s.zone_abbrev;
        elevsea = s.elevsea;
        elevhoriz = s.elevhoriz;
   }
/*================================================================================================
/
/=================================================================================================*/
  public Site(String [] sitepars) {
        // for(int j = 0; j < 9; j++) System.out.printf("%d %s\n",j,sitepars[j]);
        name = sitepars[0];
        // System.out.printf("%s %s",sitepars[0],sitepars[1]);
        // System.out.printf("-> %f\n",Double.parseDouble(sitepars[1].trim()));
        longit = new longitude(Double.parseDouble(sitepars[1].trim()));
        lat = new latitude(Double.parseDouble(sitepars[2].trim()));
        stdz = Double.parseDouble(sitepars[3].trim());
        use_dst = Integer.parseInt(sitepars[4].trim());
        timezone_name = sitepars[5];
        zone_abbrev = sitepars[6];
        elevsea = Double.parseDouble(sitepars[7].trim());
        elevhoriz = Double.parseDouble(sitepars[8].trim());
  }
/*================================================================================================
/
/=================================================================================================*/
  public void dumpsite() {  // for diagnostics
    System.out.printf("%s\n",name);
    System.out.printf("longitude %s\n",longit.RoundedLongitString(1,":",true));
    System.out.printf("latitude  %s\n",lat.RoundedDecString(0,":"));
    System.out.printf("Zone offset from UT %6.3f hours\n",stdz);
  }
/*================================================================================================
/
/=================================================================================================*/
  public boolean equals(Object arg) {
    if((arg != null) && (arg instanceof Site)) {
       Site ss = (Site) arg;
       if(!(ss.name.equals(name))) return false;
       if(ss.lat.value != lat.value) return false;
       if(ss.longit.value != longit.value) return false;
       if(ss.stdz != stdz) return false;
       // close enough ...
    }
    return true;
  }
/*================================================================================================
/
/=================================================================================================*/
  public Site clone() {
     try {
        Site copy = (Site) super.clone();
         copy.name = name;
         copy.lat = (latitude) lat.clone();
         copy.longit = (longitude) longit.clone();
         copy.stdz = stdz;
         copy.use_dst = use_dst;
         copy.timezone_name = timezone_name;
         copy.zone_abbrev = zone_abbrev;
         copy.elevsea = elevsea;
         copy.elevhoriz = elevhoriz;
        return copy;
     } catch (CloneNotSupportedException e) {
        throw new Error("This should never happen!");
     }
  }
}