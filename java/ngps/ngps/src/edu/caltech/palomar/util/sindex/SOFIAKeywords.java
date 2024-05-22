package edu.caltech.palomar.util.sindex; 
//=== File Prolog =============================================================
//	This code was developed by UCLA, Department of Physics and Astronomy,
//	for the Stratospheric Observatory for Infrared Astronomy (SOFIA) project.
//
//--- Contents ----------------------------------------------------------------
//	  SOFIAKeyword - The SOFIAKeyword is the
//           class that maintains the specific form of all the SOFIA keywords.
//           The specific syntax of a keyword may change but the strings
//           defined in this class can be dynamically changed.
//--- Description -------------------------------------------------------------
//   This class is used as the base class for all of the FITS KEYWORD to DBMS
//   table relationships.
//
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
//
//      12/05/01        J. Milburn
//        Original Implementation
//      7/4/2002        J. Milburn - renamed and modified for Fitsdriver package
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
//	In no event shall UCLA be liable for any damages, including, but not
//	limited to direct, indirect, special or consequential damages, arising out
//	of, resulting from, or in any way connected with this software, whether or
//	not based upon warranty, contract, tort or otherwise, whether or not
//	injury was sustained by persons or property or otherwise, and whether or
//	not loss was sustained from or arose out of the results of, or use of,
//	their software or services provided hereunder.
//
//=== End File Prolog =========================================================
import java.beans.Beans;
import nom.tam.fits.HeaderCard;
import nom.tam.fits.Header;
import nom.tam.fits.HeaderCardException;
import nom.tam.fits.BadHeaderException;
import nom.tam.fits.FitsUtil;
import java.util.AbstractCollection;
import java.util.AbstractMap;
import java.util.Collection;
import java.util.HashMap;
import java.util.Properties;
//import edu.ucla.astro.dcs.propertysupport.AppProperties;
import java.beans.PropertyChangeSupport;
import java.beans.PropertyChangeListener;
/*================================================================================================
/      SOFIAKeyword() Class Declaration
/=================================================================================================*/
public class SOFIAKeywords {
  protected String _root;
  protected String _base;
  protected java.lang.Class _class;
  private transient PropertyChangeSupport propertyChangeListeners = new PropertyChangeSupport(this);

// FITS Standard Required Keywords
public static java.lang.String SIMPLE    = new java.lang.String("SIMPLE");
public static java.lang.String BITPIX    = new java.lang.String("BITPIX");
public static java.lang.String NAXIS     = new java.lang.String("NAXIS");
public static java.lang.String NAXIS1    = new java.lang.String("NAXIS1");
public static java.lang.String NAXIS2    = new java.lang.String("NAXIS2");
public static java.lang.String NAXISn    = new java.lang.String("NAXISn");

//     Observation Related Keywords
public static java.lang.String DATASOURC = new java.lang.String("DATASOURC");
public static java.lang.String OBSSTAT   = new java.lang.String("OBSSTAT");
public static java.lang.String PIPELINE  = new java.lang.String("PIPELINE");
public static java.lang.String PIPESW    = new java.lang.String("PIPESW");
public static java.lang.String PROCSTAT  = new java.lang.String("PROCSTAT");
public static java.lang.String KWDICT    = new java.lang.String("KWDICT");
public static java.lang.String AOT_ID    = new java.lang.String("AOT_ID");
//     Object Identification Related Keywords
public static java.lang.String OBJECT    = new java.lang.String("OBJECT");
public static java.lang.String OBJNAME   = new java.lang.String("OBJNAME");
public static java.lang.String OBSTYPE   = new java.lang.String("OBSTYPE");
public static java.lang.String OBSID     = new java.lang.String("OBSID");
public static java.lang.String IMAGEID   = new java.lang.String("IMAGEID");

//    Mission Management Related Keywords
public static java.lang.String PROPID    = new java.lang.String("PROPID");
public static java.lang.String DEPLOY    = new java.lang.String("DEPLOY");
public static java.lang.String SCHEDBLK  = new java.lang.String("SCHEDBLK");
public static java.lang.String FLIGHTNO  = new java.lang.String("FLIGHTNO");
public static java.lang.String FLIGHTLG  = new java.lang.String("FLIGHTLG");

//    Origination Related Keywords
public static java.lang.String OBSERVER  = new java.lang.String("OBSERVER");
public static java.lang.String CREATOR   = new java.lang.String("CREATOR");
public static java.lang.String OPERATOR  = new java.lang.String("OPERATOR");
public static java.lang.String FILENAME  = new java.lang.String("FILENAME");
public static java.lang.String OBSERVAT  = new java.lang.String("OBSERVAT");

//     Date and Time Related Keywords
public static java.lang.String DATE_CRE  = new java.lang.String("DATE_CRE");
public static java.lang.String DATE_OBS  = new java.lang.String("DATE_OBS");
public static java.lang.String UTCSTART  = new java.lang.String("UTCSTART");
public static java.lang.String UTCEND    = new java.lang.String("UTCEND");
public static java.lang.String LST_OBS   = new java.lang.String("LST_OBS");
public static java.lang.String MJD_OBS   = new java.lang.String("MJD_OBS");

//     Aircraft Related Keywords
public static java.lang.String AC_LONS   = new java.lang.String("AC_LONS");
public static java.lang.String AC_LATS   = new java.lang.String("AC_LATS");
public static java.lang.String AC_ALTS   = new java.lang.String("AC_ALTS");
public static java.lang.String AC_LONE   = new java.lang.String("AC_LONE");
public static java.lang.String AC_LATE   = new java.lang.String("AC_LATE");
public static java.lang.String AC_ALTE   = new java.lang.String("AC_ALTE");
public static java.lang.String AC_VELO   = new java.lang.String("AC_VELO");
public static java.lang.String AC_HEAD   = new java.lang.String("AC_HEAD");

//     Environmental Keywords
public static java.lang.String WVCOLDEN  = new java.lang.String("WVCOLDEN");
public static java.lang.String HUMID     = new java.lang.String("HUMID");
public static java.lang.String TEMP_OUT  = new java.lang.String("TEMP_OUT");
public static java.lang.String TEMP_PRI  = new java.lang.String("TEMP_PRI");
public static java.lang.String TEMP_SEC  = new java.lang.String("TEMP_SEC");
public static java.lang.String TEMP_DET  = new java.lang.String("TEMP_DET");
public static java.lang.String TEMP_XXX  = new java.lang.String("TEMP_XXX");

//      Telescope Related Keywords
public static java.lang.String TELESCOP  = new java.lang.String("TELESCOP");
public static java.lang.String TELCONF   = new java.lang.String("TELCONF");
public static java.lang.String TELTCS    = new java.lang.String("TELTCS");
public static java.lang.String TELRA     = new java.lang.String("TELRA");
public static java.lang.String TELDEC    = new java.lang.String("TELDEC");
public static java.lang.String OBSRA     = new java.lang.String("OBSRA");
public static java.lang.String OBSDEC    = new java.lang.String("OBSDEC");
public static java.lang.String RADECSYS  = new java.lang.String("RADECSYS");
public static java.lang.String EQUINOX   = new java.lang.String("EQUINOX");
public static java.lang.String ZA        = new java.lang.String("ZA");
public static java.lang.String HA        = new java.lang.String("HA");
public static java.lang.String TELTKRA   = new java.lang.String("TELTKRA");
public static java.lang.String TELTKDEC  = new java.lang.String("TELTKDEC");
public static java.lang.String SUNANGL   = new java.lang.String("SUNANGL");
public static java.lang.String MOONANGL  = new java.lang.String("MOONANGL");
//    User Coordinate System Definition Keywords
public static java.lang.String USRCRDSY  = new java.lang.String("USRCRDSY");
public static java.lang.String USRREFCR  = new java.lang.String("USRREFCR");
public static java.lang.String USRORIGX  = new java.lang.String("USRORIGX");
public static java.lang.String USRORIGY  = new java.lang.String("USRORIGY");
public static java.lang.String USRX      = new java.lang.String("USRX");
public static java.lang.String USRY      = new java.lang.String("USRY");
public static java.lang.String USREQNX   = new java.lang.String("USREQNX");
public static java.lang.String USRCROT   = new java.lang.String("USRCROT");
//   Telescope Focus Keywords
public static java.lang.String TELFOCUS  = new java.lang.String("TELFOCUS");
public static java.lang.String FOCUS_Z   = new java.lang.String("FOCUS_Z");
public static java.lang.String FOCUS_DZ  = new java.lang.String("FOCUS_DZ");
public static java.lang.String ALIGN_X   = new java.lang.String("ALIGN_X");
public static java.lang.String ALIGN_DX  = new java.lang.String("ALIGN_DX");
public static java.lang.String ALIGN_Y   = new java.lang.String("ALIGN_Y");
public static java.lang.String ALIGN_DY  = new java.lang.String("ALIGN_DY");
//
public static java.lang.String ROT_ANGL  = new java.lang.String("ROT_ANGL");
public static java.lang.String HELIO_COR = new java.lang.String("HELIO_COR");
public static java.lang.String LSR_COR   = new java.lang.String("LSR_COR");

//     Data Collection Related Keywords
public static java.lang.String CHOPPING  = new java.lang.String("CHOPPING");
public static java.lang.String NODDING   = new java.lang.String("NODDING");
public static java.lang.String DITHER    = new java.lang.String("DITHER");
public static java.lang.String GUIDE     = new java.lang.String("GUIDE");
public static java.lang.String MAP_SCAN  = new java.lang.String("MAP_SCAN");
public static java.lang.String AIRCRAFT  = new java.lang.String("AIRCRAFT");

//      Commentary and History Related Keywords
public static java.lang.String COMMENT   = new java.lang.String("COMMENT");
public static java.lang.String HISTORY   = new java.lang.String("HISTORY");
public static java.lang.String ERRORnnn  = new java.lang.String("ERRORnnn");

//     Instrument Related Keywords
public static java.lang.String INSTRUME  = new java.lang.String("INSTRUME");
public static java.lang.String INSTCONF  = new java.lang.String("INSTCONF");
public static java.lang.String INSTHWV   = new java.lang.String("INSTHWV");
public static java.lang.String INSTSWV   = new java.lang.String("INSTSWV");
public static java.lang.String EXPTIME   = new java.lang.String("EXPTIME");
public static java.lang.String FILTER    = new java.lang.String("FILTER");
public static java.lang.String WAVECENT  = new java.lang.String("WAVECENT");
public static java.lang.String WAVERANG  = new java.lang.String("WAVERANG");

//     Array Detector Related Keywords
public static java.lang.String DETECTOR  = new java.lang.String("DETECTOR");
public static java.lang.String DETSIZE   = new java.lang.String("DETSIZE");
public static java.lang.String PIXSEP    = new java.lang.String("PIXSEP");
public static java.lang.String SUBARRAY  = new java.lang.String("SUBARRAY");
public static java.lang.String SUBARRNO  = new java.lang.String("SUBARRNO");

public static java.lang.String DATAMAX   = new java.lang.String("DATAMAX");
public static java.lang.String DATAMIN   = new java.lang.String("DATAMIN");
public static java.lang.String SATURATE  = new java.lang.String("SATURATE");
public static java.lang.String DET_ANGL  = new java.lang.String("DET_ANGL");
public static java.lang.String COADDS    = new java.lang.String("COADDS");
//      Subarray Keywords that have multiple values
public static java.lang.String SUBARR  = new java.lang.String("SUBARR");
public static java.lang.String STARTRW  = new java.lang.String("STARTRW");
public static java.lang.String STARTCL  = new java.lang.String("STARTCL");
public static java.lang.String ROWLEN   = new java.lang.String("ROWLEN");
public static java.lang.String COLLEN   = new java.lang.String("COLLEN");

//     Heterodyne Keywords
public static java.lang.String VLSR      = new java.lang.String("VLSR");
public static java.lang.String DELTAV    = new java.lang.String("DELTAV");
public static java.lang.String VELREF    = new java.lang.String("VELREF");
public static java.lang.String ALTRVAL   = new java.lang.String("ALTRVAL");
public static java.lang.String ALTRPIX   = new java.lang.String("ALTRPIX");
public static java.lang.String RESTFREQ  = new java.lang.String("RESTFREQ");
public static java.lang.String LOFREQ    = new java.lang.String("LOFREQ");
public static java.lang.String IFFREQ    = new java.lang.String("IFFREQ");
public static java.lang.String IMAGFREQ  = new java.lang.String("IMAGFREQ");
public static java.lang.String TSYS      = new java.lang.String("TSYS");

//     Data Structure Related Keywords
public static java.lang.String XTENSION  = new java.lang.String("XTENSION");
public static java.lang.String EXTEND    = new java.lang.String("EXTEND");
public static java.lang.String EXTLEVEL  = new java.lang.String("EXTLEVEL");
public static java.lang.String EXTNAME   = new java.lang.String("EXTNAME");
public static java.lang.String EXTVER    = new java.lang.String("EXTVER");


public static java.lang.String BSCALE    = new java.lang.String("BSCALE");
public static java.lang.String BZERO     = new java.lang.String("BZERO");
public static java.lang.String BUNIT     = new java.lang.String("BUNIT");
public static java.lang.String INHERIT   = new java.lang.String("INHERIT");
public static java.lang.String CHECKSUM  = new java.lang.String("CHECKSUM");
public static java.lang.String DATASUM   = new java.lang.String("DATASUM");
public static java.lang.String CECKVER   = new java.lang.String("CECKVER");
//    Data Structure Keywords that have n- suffixes
public static java.lang.String CTYPE    = new java.lang.String("CTYPE");
public static java.lang.String CRPIX    = new java.lang.String("CRPIX");
public static java.lang.String CRVAL    = new java.lang.String("CRVAL");
public static java.lang.String CDELT    = new java.lang.String("CDELT");
public static java.lang.String CROTA    = new java.lang.String("CROTA");

//     CHOPPING RELATED KEYWORDS
public static java.lang.String CHPCRSYS  = new java.lang.String("CHPCRSYS");
public static java.lang.String CHPHWV    = new java.lang.String("CHPHWV");
public static java.lang.String CHPSWV    = new java.lang.String("CHPSWV");
public static java.lang.String CHPFREQ   = new java.lang.String("CHPFREQ");
public static java.lang.String CHPAMP    = new java.lang.String("CHPAMP");
public static java.lang.String CHPANGLE  = new java.lang.String("CHPANGLE");
public static java.lang.String CHPPHAS   = new java.lang.String("CHPPHAS");
public static java.lang.String CHPFUNC   = new java.lang.String("CHPFUNC");
public static java.lang.String CHPSETL   = new java.lang.String("CHPSETL");

//     NODDING RELATED KEYWORDS
public static java.lang.String NODCRSYS  = new java.lang.String("NODCRSYS");
public static java.lang.String NODTIME   = new java.lang.String("NODTIME");
public static java.lang.String NODN      = new java.lang.String("NODN");
public static java.lang.String NODSETL   = new java.lang.String("NODSETL");
public static java.lang.String NODDIST   = new java.lang.String("NODDIST");
public static java.lang.String NODPOSX   = new java.lang.String("NODPOSX");
public static java.lang.String NODPOSY   = new java.lang.String("NODPOSY");
public static java.lang.String NODANGLE  = new java.lang.String("NODANGLE");

//     DITHER RELATED KEYWORDS
public static java.lang.String DTCRDSYS  = new java.lang.String("DTCRDSYS");
public static java.lang.String DTPATERN  = new java.lang.String("DTPATERN");
public static java.lang.String NDITHER   = new java.lang.String("NDITHER");

//     MAPPING/SCANNING RELATED KEYWORDS
public static java.lang.String MAPCRSYS  = new java.lang.String("MAPCRSYS");
public static java.lang.String MAPPATT   = new java.lang.String("MAPPATT");
public static java.lang.String MAPNXPOS  = new java.lang.String("MAPNXPOS");
public static java.lang.String MAPNYPOS  = new java.lang.String("MAPNYPOS");
public static java.lang.String MAPINTX   = new java.lang.String("MAPINTX");
public static java.lang.String MAPINTY   = new java.lang.String("MAPINTY");

//     SCANNING MODE KEYWORDS - Further specification of scanning keywords requires investigation of telescope capability
public static java.lang.String RA_RATE   = new java.lang.String("RA_RATE");
public static java.lang.String DEC_RATE  = new java.lang.String("DEC_RATE");
public static java.lang.String SAMPTIME  = new java.lang.String("SAMPTIME");

//    FITS Extension Related Keywords
public static java.lang.String PCOUNT    = new java.lang.String("PCOUNT");
public static java.lang.String GCOUNT    = new java.lang.String("GCOUNT");
public static java.lang.String TFIELDS   = new java.lang.String("TFIELDS");
//    FITS Extension Related Keywords with n- suffix
public static java.lang.String TBCOL    = new java.lang.String("TBCOL");
public static java.lang.String TFORM    = new java.lang.String("TFORM");
public static java.lang.String TSCAL    = new java.lang.String("TSCAL");
public static java.lang.String TZERO    = new java.lang.String("TZERO");
public static java.lang.String TNULL    = new java.lang.String("TNULL");
public static java.lang.String TTYPE    = new java.lang.String("TTYPE");
public static java.lang.String TUNIT    = new java.lang.String("TUNIT");

public static java.lang.String THEAP     = new java.lang.String("THEAP");

//     Instrument Names
public static java.lang.String FLITECAM  = new java.lang.String("FLITECAM");
public static java.lang.String FORCAST   = new java.lang.String("FORCAST");
public static java.lang.String FIFI      = new java.lang.String("FIFI");
public static java.lang.String AIRES     = new java.lang.String("AIRES");
public static java.lang.String HAWKE     = new java.lang.String("HAWKE");
public static java.lang.String GREAT     = new java.lang.String("GREAT");
public static java.lang.String CASIMIR   = new java.lang.String("CASIMIR");
public static java.lang.String HIPO      = new java.lang.String("HIPO");
public static java.lang.String SAFIRE    = new java.lang.String("SAFIRE");
//     RADECSYS Coordinate Systems
public static java.lang.String GALACTIC  = new java.lang.String("GALACTIC");
public static java.lang.String ECLIPTIC  = new java.lang.String("ECLIPTIC");
public static java.lang.String SGALACTC  = new java.lang.String("SGALACTC");
public static java.lang.String HELIOECL  = new java.lang.String("HELIOECL");
public static java.lang.String ALTAZ     = new java.lang.String("ALTAZ");

//    FLITECAM specific environmental variables
//           Temperature Monitor Sensors
public static java.lang.String TEMPFC1         = new java.lang.String("TEMPFC1");  // FLITECAM
public static java.lang.String TEMPFC2         = new java.lang.String("TEMPFC2");  // LakeShore 218 Temperature Monitor
public static java.lang.String TEMPFC3         = new java.lang.String("TEMPFC3");  // Sensors 1 to 8
public static java.lang.String TEMPFC4         = new java.lang.String("TEMPFC4");
public static java.lang.String TEMPFC5         = new java.lang.String("TEMPFC5");
public static java.lang.String TEMPFC6         = new java.lang.String("TEMPFC6");
public static java.lang.String TEMPFC7         = new java.lang.String("TEMPFC7");
public static java.lang.String TEMPFC8         = new java.lang.String("TEMPFC8");
//           Temperature Controller Sensors and Setpoint
public static java.lang.String TEMPFCA         = new java.lang.String("TEMPFCA");  // FLITECAM  Sample and Control Temperatures
public static java.lang.String TEMPFCB         = new java.lang.String("TEMPFCB");  // LakeShore 330 Temperature Monitor
public static java.lang.String TEMPFCS         = new java.lang.String("TEMPFCS");  // The Setpoint Temperature for the LakeShore 330
//           Helium Level Sensor
public static java.lang.String HELEVEL          = new java.lang.String("HELEVEL");  // Helium Level on the FLITECAM Cryogen Container
//           FLITECAM Vacuum Gauge
public static java.lang.String FCVACUM         = new java.lang.String("FCVACUUM");  // FLITECAM Vacuum Gauge
//           FLITECAM Mechanism Position Keywords
public static java.lang.String FCFILTA         = new java.lang.String("FCFILTA");  // FLITECAM Filterwheel A Filter Name
public static java.lang.String FCFILTB         = new java.lang.String("FCFILTB");  // FLITECAM Filterwheel B Filter Name
public static java.lang.String FCAPERA         = new java.lang.String("FCAPERA");  // FLITECAM Aperature Position OPEN or CLOSED
public static java.lang.String FCPUPIL         = new java.lang.String("FCPUPIL");  // FLITECAM Pupil Lense in or out


/*================================================================================================
/      SOFIAKeyword() main constructor
/=================================================================================================*/
    public SOFIAKeywords(String base, Class c){
	      _root= base;
	      _base= base + ".";
       jbInit();
    }
/*================================================================================================
/      SOFIAKeywordConstants() parameterless constructor
/=================================================================================================*/
    public SOFIAKeywords(){
        jbInit();
    }
/*================================================================================================
/       jbInit() Initialization Method
/=================================================================================================*/
    private void jbInit(){
    }
/*================================================================================================
/      setBase() and getBase()
/=================================================================================================*/
  public void setBase(java.lang.String newBase){
     java.lang.String oldBase = _base;
     _base = newBase;
     propertyChangeListeners.firePropertyChange("base", new java.lang.String(oldBase), new java.lang.String(newBase));
  }
  public java.lang.String getBase(){
    return _base;
  }
/*================================================================================================
/      setRoot() and getRoot()
/=================================================================================================*/
  public void setRoot(java.lang.String newRoot){
     java.lang.String oldRoot = _root;
     _root = newRoot;
     propertyChangeListeners.firePropertyChange("root", new java.lang.String(oldRoot), new java.lang.String(newRoot));
  }
  public java.lang.String getRoot(){
    return _root;
  }
/*================================================================================================
/      setClass() and getClass()
/=================================================================================================*/
  public void setClass(java.lang.Class newClass){
      _class = newClass;
   }
  public java.lang.Class getPropertiesClass(){
    return _class;
  }
/*================================================================================================
/        Property Change Listener Support Methods
/=================================================================================================*/
  public synchronized void removePropertyChangeListener(PropertyChangeListener l) {
     propertyChangeListeners.removePropertyChangeListener(l);
  }
  public synchronized void addPropertyChangeListener(PropertyChangeListener l) {
     propertyChangeListeners.addPropertyChangeListener(l);
  }
/*================================================================================================
/     End of the SOFIAKeyword Class
/=================================================================================================*/
}
