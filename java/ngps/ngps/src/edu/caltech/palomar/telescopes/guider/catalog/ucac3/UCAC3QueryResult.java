/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */ 
 
package edu.caltech.palomar.telescopes.guider.catalog.ucac3;
import jsky.catalog.QueryResult;
import jsky.catalog.MemoryCatalog;
import jsky.catalog.FieldDescAdapter;
import java.util.Vector;
import jsky.catalog.TablePlotSymbol;
import jsky.catalog.skycat.SkycatConfigEntry;
import jsky.catalog.skycat.SkycatConfigFile;
import jsky.catalog.Catalog;
import java.util.Properties;
import java.awt.Color;
import jsky.catalog.PlotableCatalog;
import jsky.catalog.RowCoordinates;
import edu.caltech.palomar.telescopes.guider.catalog.ucac3.ucac3Reader;
import edu.caltech.palomar.telescopes.guider.catalog.ucac3.ucac3Star;
import javax.swing.UIManager;
//import jsky.catalog.astrocat.AstroCatTable;
import jsky.catalog.*;
//import jsky.catalog.astrocat.*;
import jsky.coords.CoordinateRadius;
import java.net.URL;
import java.util.Enumeration;
import java.util.Vector;
import java.io.IOException;
import jsky.coords.ImageCoords;
//=== File Prolog =============================================================
//	This code was developed by John Thorstensen of Dartmouth University
//
//      It has been refactored and modified by Jennifer Milburn for
//      use at the Palomar Observatory.
//
//      Caltech Optical Observatories, Palomar Observatory
//
//--- Contents ----------------------------------------------------------------
//    UCAC3QueryResult - a class to hold the query results from a UCAC3 catalog query
//
//--- Description -------------------------------------------------------------
//  A segment is one of the 86400 areas of the sky that is 0.5 degrees in
//  declination and 0.1 hours in size. (30 arcminutes by 90 arcminutes).
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
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
public class UCAC3QueryResult extends MemoryCatalog implements PlotableCatalog{
/*num item   fmt unit        explanation                            remark
------------------------------------------------------------------------
 1  ra     I*4 mas         right ascension at  epoch J2000.0 (ICRS)  (1)
 2  spd    I*4 mas         south pole distance epoch J2000.0 (ICRS)  (1)
 3  im1    I*2 millimag    UCAC fit model magnitude                  (2)
 4  im2    I*2 millimag    UCAC aperture  magnitude                  (2)
 5  sigmag I*2 millimag    UCAC error on magnitude (larger of sc.mod)(3)
 6  objt   I*1             object type                               (4)   
 7  dsf    I*1             double star flag                          (5)   
         16
 8  sigra  I*2 mas         s.e. at central epoch in RA (*cos Dec)      
 9  sigdc  I*2 mas         s.e. at central epoch in Dec                 
10  na1    I*1             total # of CCD images of this star
11  nu1    I*1             # of CCD images used for this star        (6)
12  us1    I*1             # catalogs (epochs) used for proper motions
13  cn1    I*1             total numb. catalogs (epochs) initial match
          8
14  cepra  I*2 0.01 yr     central epoch for mean RA, minus 1900     
15  cepdc  I*2 0.01 yr     central epoch for mean Dec,minus 1900  
16  pmrac  I*4 0.1 mas/yr  proper motion in RA*cos(Dec)           
17  pmdc   I*4 0.1 mas/yr  proper motion in Dec                    
18  sigpmr I*2 0.1 mas/yr  s.e. of pmRA * cos Dec                   
19  sigpmd I*2 0.1 mas/yr  s.e. of pmDec                            
         16
20  id2m   I*4             2MASS pts_key star identifier          
21  jmag   I*2 millimag    2MASS J  magnitude                     
22  hmag   I*2 millimag    2MASS H  magnitude                       
23  kmag   I*2 millimag    2MASS K_s magnitude                     
24  icqflg I*1 * 3         2MASS cc_flg*10 + phot.qual.flag          (7)
25  e2mpho I*1 * 3         2MASS error photom. (1/100 mag)           (8)
         16
26  smB    I*2 millimag    SuperCosmos Bmag
27  smR2   I*2 millimag    SC R2mag                                  (9)
28  smI    I*2 millimag    SC Imag
29  clbl   I*1             SC star/galaxy classif./quality flag     (10)
30  qfB    I*1             SC quality flag Bmag                     (11)
31  qfR2   I*1             SC quality flag R2mag                    (11)
32  qfI    I*1             SC quality flag Imag                     (11)
         10
33  catflg I*1 * 10        mmf flag for 10 major catalogs matched   (12)
34  g1     I*1             Yale SPM object type (g-flag)            (13)
35  c1     I*1             Yale SPM input cat.  (c-flag)            (14)
36  leda   I*1             LEDA galaxy match flag                   (15)
37  x2m    I*1             2MASS extend.source flag                 (16)
38  rn     I*4             MPOS star number; identifies HPM stars   (17)
         18
------------------------------------------------------------------------*/
    private FieldDescAdapter[] currentFields = new FieldDescAdapter[19];
    private Vector             columnNames   = new Vector();
    private java.lang.String[] columns       = new java.lang.String[19];
    /** The catalog configuration entry for this catalog. */
    private SkycatConfigEntry _entry;
    SkycatConfigFile           configFile;
    Catalog                   _catalog;
    TablePlotSymbol[]         _table_plot_symbol = new TablePlotSymbol[1];
   public  ucac3Reader           u3Reader;
   private java.lang.String      NAME                      = "Local UCAC3";
   private java.lang.String      ID                        = "UCAC3";
   private java.lang.String      TITLE                     = "Palomar Observatory local UCAC3 catalog";
   private java.net.URL          readmeURL;
   private static java.lang.String description =
    "UCAC3 is a compiled, all-sky star catalog covering mainly the 8 to"+
    "16 magnitude range in a single bandpass between V and R.  Positional"+
    "errors are about 15 to 20 mas for stars in the 10 to 14 mag range."+
    "It is supplemented by proper motions and SuperCosmos and 2MASS"+
    "photometric data, as well as various flags.";
   private QueryArgs        _queryArgs;
   private CoordinateRadius _region;
/*=============================================================================================
/        UCAC3Catalog Constructor
/=============================================================================================*/
   public UCAC3QueryResult(ucac3Reader index){
      super();  
      initializeCatalogIndex(index);
      initializeFieldDescriptions();
      setFields(currentFields);
      constructColumnNameArray();
      initializeProperties();
   }
/*=============================================================================================
/        UCAC3Catalog Constructor
/=============================================================================================*/
 public UCAC3QueryResult(ucac3Reader index,Vector current_vec){
       initializeCatalogIndex(index);
       initializeFieldDescriptions();
       setFields(currentFields);
       constructColumnNameArray();       
       setData(current_vec);
       initializeProperties();
 } 
/*=============================================================================================
/         main method
/=============================================================================================*/
  private void test(){
    double RA = 202.48219583;
    double DEC = 47.23150889;
    double BOX = 60.0;
    UCAC3QueryResult results;
    u3Reader.setPrint(false);
    ImageCoords     center = new ImageCoords(RA,DEC);
    CoordinateRadius cr = new CoordinateRadius(center, BOX);
    QueryArgs qa =  new BasicQueryArgs(this);
    qa.setRegion(cr);
    try{
      results =   (UCAC3QueryResult)query(qa);
      printObjectList( results.getDataVector()  );
    }catch(Exception e){
      System.out.println("An error occured while querying the UCAC3 catalog");
    }
    //    u3Reader.read(RA,DEC,BOX);
  }
/*=============================================================================================
/      setRegionArgs(QueryArgs queryArgs, CoordinateRadius region)
/=============================================================================================*/
public void initializeCatalogIndex(ucac3Reader index){
       u3Reader = index;
       try{
          readmeURL = new URL(u3Reader.myTelescopesIniReader.UCAC3PATH+"/readme_u3");
       }catch(Exception e){
           System.out.println("Can't locate the readme_u3 file for the UCAC3 catalog description");
       }    
}
/*=============================================================================================
/      setRegionArgs(QueryArgs queryArgs, CoordinateRadius region)
/=============================================================================================*/
    /**
     * Given a description of a region of the sky (center point and radius range),
     * and the current query argument settings, set the values of the corresponding
     * query parameters.
     *
     * @param queryArgs (in/out) describes the query arguments
     * @param region (in) describes the query region (center and radius range)
     */
    public void setRegionArgs(QueryArgs queryArgs, CoordinateRadius region) {
        this._queryArgs = queryArgs;
        this._region    = region;
    }
    /**
     * Return true if this is a local catalog, and false if it requires
     * network access or if a query could hang. A local catalog query is
     * run in the event dispatching thread, while others are done in a
     * separate thread.
     */
/*=============================================================================================
/     query(QueryArgs queryArgs)
/**
* Query the catalog using the given arguments and return the result.
* The result of a query may be any class that implements the QueryResult
* interface. It is up to the calling class to interpret and display the
* result. In the general case where the result is downloaded via HTTP,
* The URLQueryResult class may be used.
*
* @param queryArgs An object describing the query arguments.
* @return An object describing the result of the query.
*/
//=============================================================================================*/
    public QueryResult query(QueryArgs queryArgs) throws IOException {
        double search_ra  = queryArgs.getRegion().getCenterPosition().getX();
        double search_dec = queryArgs.getRegion().getCenterPosition().getY();
        double search_box = queryArgs.getRegion().getMaxRadius();
        Vector results = new Vector();
        results.clear();
        if(u3Reader.isInitialized()){
           results = u3Reader.queryCatalog(search_ra, search_dec, search_box);
        }
        setData(results);
      return this;
    }
/*=============================================================================================
/      constructColumnNameArray()
/=============================================================================================*/
public void setData(Vector current_vec){
    this.setDataVector(current_vec,columnNames);
}
/*=============================================================================================
/      constructColumnNameArray()
/=============================================================================================*/  
public void initializeProperties(){
      configFile = SkycatConfigFile.getConfigFile();
      Properties properties = new Properties();
                 properties.setProperty("serv_type", "catalog");
                 properties.setProperty("long_name", "Local UCAC3"); 
                 properties.setProperty("short_name", "UCAC3");
                 properties.setProperty("url", "");  
                 properties.setProperty("symbol", "mag {circle {} {} {} {}} {{(25-$mag)/3600.} {deg 2000}}");   
                 properties.setProperty("search_cols", "mag {Brightest (min)} {Faintest (max)}");
      _entry = new SkycatConfigEntry(properties);
      TablePlotSymbol tps = new TablePlotSymbol();
      tps.setShape(TablePlotSymbol.CIRCLE);
      tps.setBg(Color.red);
      tps.setFg(Color.GREEN);
      tps.setRaCol(0);
      tps.setDecCol(1);
      tps.setEquinox(2000.0);
      tps.setColNames(columns);
      tps.setSize("(25-$mag1)/2.5");
      _table_plot_symbol[0]=tps;
      _entry.setSymbols(_table_plot_symbol);
      this.setCatalog(this);
      RowCoordinates rc = new RowCoordinates(0,1,2000.0);
      setRowCoordinates(rc);
}
/*=============================================================================================
/      constructColumnNameArray()
/=============================================================================================*/
 private void constructColumnNameArray(){
   columnNames.add("ra");
   columnNames.add("dec");
   columnNames.add("mag1");
   columnNames.add("mag2");
   columnNames.add("sigra");
   columnNames.add("sigdec");
   columnNames.add("pmra");
   columnNames.add("pmdec");
   columnNames.add("epra");
   columnNames.add("epdec");
   columnNames.add("Jmag");
   columnNames.add("Hmag");
   columnNames.add("Kmag");
   columnNames.add("scBmag");
   columnNames.add("scRmag");
   columnNames.add("scImag");
   columnNames.add("objtype");
   columnNames.add("doubleflag");
   columnNames.add("running");
   columns[0]="ra";
   columns[1]="dec";
   columns[2]="mag1";
   columns[3]="mag2";
   columns[4]="sigra";
   columns[5]="sigdec";
   columns[6]="pmra";
   columns[7]="pmdec";
   columns[8]="epra";
   columns[9]="epdec";
   columns[10]="Jmag";
   columns[11]="Hmag";
   columns[12]="Kmag";
   columns[13]="scBmag";
   columns[14]="scRmag";
   columns[15]="scImag";
   columns[16]="objtype";
   columns[17]="doubleflag";
   columns[18]="running";
   this.setColumnIdentifiers(columnNames);
 }
/*=============================================================================================
/        UCAC3Catalog Constructor
/=============================================================================================*/
 public void initializeFieldDescriptions(){
   FieldDescAdapter ra_field          = new FieldDescAdapter("ra"); 
   FieldDescAdapter dec_field         = new FieldDescAdapter("dec"); 
   FieldDescAdapter sigra_field       = new FieldDescAdapter("sigra"); 
   FieldDescAdapter sigdec_field      = new FieldDescAdapter("sigdec"); 
   FieldDescAdapter pmra_field        = new FieldDescAdapter("pmra"); 
   FieldDescAdapter pmdec_field       = new FieldDescAdapter("pmdec"); 
   FieldDescAdapter mag1_field        = new FieldDescAdapter("mag1"); 
   FieldDescAdapter mag2_field        = new FieldDescAdapter("mag2"); 
   FieldDescAdapter epra_field        = new FieldDescAdapter("epra"); 
   FieldDescAdapter epdec_field       = new FieldDescAdapter("epdec"); 
   FieldDescAdapter Jmag_field        = new FieldDescAdapter("Jmag"); 
   FieldDescAdapter Hmag_field        = new FieldDescAdapter("Hmag"); 
   FieldDescAdapter Kmag_field        = new FieldDescAdapter("Kmag"); 
   FieldDescAdapter scBmag_field      = new FieldDescAdapter("scBmag"); 
   FieldDescAdapter scRmag_field      = new FieldDescAdapter("scRmag"); 
   FieldDescAdapter scImag_field      = new FieldDescAdapter("scImag"); 
   FieldDescAdapter objtype_field     = new FieldDescAdapter("objtype"); 
   FieldDescAdapter doubleflag_field  = new FieldDescAdapter("doubleflag"); 
   FieldDescAdapter running_field     = new FieldDescAdapter("running"); 
   // Set the descriptions for each of the fields
   ra_field.setDescription("right ascension at  epoch J2000.0 (ICRS)");
   dec_field.setDescription("south pole distance epoch J2000.0 (ICRS) ");
   sigra_field.setDescription("s.e. at central epoch in RA (*cos Dec)");
   sigdec_field.setDescription("s.e. at central epoch in Dec");
   pmra_field.setDescription("proper motion in RA*cos(Dec) ");
   pmdec_field.setDescription("proper motion in Dec");
   mag1_field.setDescription("UCAC fit model magnitude");
   mag2_field.setDescription("UCAC aperture  magnitude");
   epra_field.setDescription("s.e. of pmRA * cos Dec  ");
   epdec_field.setDescription("s.e. of pmDec");
   Jmag_field.setDescription("2MASS J  magnitude");
   Hmag_field.setDescription("2MASS H  magnitude");
   Kmag_field.setDescription("2MASS K_s magnitude");
   scBmag_field.setDescription("SuperCosmos Bmag");
   scRmag_field.setDescription("SuperCosmos R2mag");
   scImag_field.setDescription("SuperCosmos Imag");
   objtype_field.setDescription("object type");
   doubleflag_field.setDescription("double star flag");
   running_field.setDescription("MPOS star number");
   // set the units for each of the fields
   ra_field.setUnits("mas");
   dec_field.setUnits("mas");
   sigra_field.setUnits("mas");
   sigdec_field.setUnits("mas");
   pmra_field.setUnits("0.1 mas/yr");
   pmdec_field.setUnits("0.1 mas/yr");
   mag1_field.setUnits("millimag");
   mag2_field.setUnits("millimag");
   epra_field.setUnits("0.1 mas/yr");
   epdec_field.setUnits("0.1 mas/yr");
   Jmag_field.setUnits("millimag");
   Hmag_field.setUnits("millimag");
   Kmag_field.setUnits("millimag");
   scBmag_field.setUnits("millimag");
   scRmag_field.setUnits("millimag");
   scImag_field.setUnits("millimag");
   // Specifically set the display names for commonly used parameters
   ra_field.setDisplayName("Right Ascension");
   dec_field.setDisplayName("Declination");
   running_field.setDisplayName("UCAC3 ID");  
   //  Set the fields that will be used as the position identifiers
   ra_field.setIsRA(true);
   dec_field.setIsDec(true);
   running_field.setIsId(true);
   ra_field.setIsRAMain(true);
   dec_field.setIsDecMain(true);
   currentFields[0] = ra_field;
   currentFields[1] = dec_field;
   currentFields[2] = mag1_field;
   currentFields[3] = mag2_field;
   currentFields[4] = sigra_field;
   currentFields[5] = sigdec_field;
   currentFields[6] = pmra_field;
   currentFields[7] = pmdec_field;
   currentFields[8] = epra_field;
   currentFields[9] = epdec_field;
   currentFields[10] = Jmag_field;
   currentFields[11] = Hmag_field;
   currentFields[12] = Kmag_field;
   currentFields[13] = scBmag_field;
   currentFields[14] = scRmag_field;
   currentFields[15] = scImag_field;
   currentFields[16] = objtype_field;
   currentFields[17] = doubleflag_field;
   currentFields[18] = running_field;
 }
/*=============================================================================================
/      getColumnClass(int columnIndex)
/=============================================================================================*/
  public Class getColumnClass(int columnIndex) {
       if(columnIndex == 0 | columnIndex == 1){
           return Double.class;
       }
       if(columnIndex > 1 & columnIndex < 16){
           return Float.class;
       }
       if (columnIndex > 16){
            return Integer.class;
       }
     return Float.class;
  }
/*=============================================================================================
/       Object clone() Implementation of the clone method (makes a shallow copy).  main method
/=============================================================================================*/
    public Object clone() throws CloneNotSupportedException {
        return super.clone();
    }
/*=============================================================================================
/     setName(String name)     Set the name of the catalog
/=============================================================================================*/
     public void setName(String name) {
         this.NAME = name;
    }
/*=============================================================================================
/    getName()     Return the name of the catalog
/=============================================================================================*/
    public String getName() {
        return NAME;
    }
/*=============================================================================================
/    getId()   Return the Id or short name of the catalog
/=============================================================================================*/
    public String getId() {
        return ID;
    }
/*=============================================================================================
/     String getTitle()    Return a string to display as a title for the catalog in a user interface
/=============================================================================================*/
    public String getTitle() {
        return TITLE;
    }
/*=============================================================================================
/    getDescription()   Return a description of the catalog, or null if not available
/=============================================================================================*/
    public String getDescription() {
        return description;
    }
/*=============================================================================================
/     Return a URL pointing to documentation for the catalog, or null if not available
/=============================================================================================*/
    public URL getDocURL() {
        return readmeURL;
    }
/*=============================================================================================
/   If this catalog can be querried, return the number of query parameters that it accepts
/=============================================================================================*/
    public int getNumParams() {
        return 3;
    }
/*=============================================================================================
/     Return a description of the ith query parameter
/=============================================================================================*/
    public FieldDesc getParamDesc(int i) {
        return new FieldDescAdapter("Param" + i);
    }
/*=============================================================================================
/    Return a description of the named query parameter
/=============================================================================================*/
     public FieldDesc getParamDesc(String name) {
        return new FieldDescAdapter(name);
    }

/*=============================================================================================
/      Set a reference to the catalog used to create this table.
/=============================================================================================*/
     public void setCatalog(Catalog cat) {
         _catalog = cat;
    }
/*=============================================================================================
/      Return the catalog used to create this table, or null if not known.
/=============================================================================================*/
    public Catalog getCatalog() {
        return this;
    }
/*=============================================================================================
/      boolean isLocal()
/=============================================================================================*/
   public boolean isLocal() {
        return true;
    }
/*=============================================================================================
/   Set the parent catalog directory
/=============================================================================================*/
    public void setParent(CatalogDirectory catDir) {
      // This catalog does NOT contain any directory structure
    }
/*=============================================================================================
/    Return the catalog type (normally one of the Catalog constants: CATALOG, ARCHIVE, DIRECTORY, LOCAL, IMAGE_SERVER)
/=============================================================================================*/
    public String getType() {
        return Catalog.CATALOG;
    }
/*=============================================================================================
/    Return true if this object represents an image server.
/=============================================================================================*/
     public boolean isImageServer() {
        return false;
    }
/*=============================================================================================
/     Return a reference to the parent catalog directory, or null if not known.
/=============================================================================================*/
    public CatalogDirectory getParent() {
        return null;
    }
/*=============================================================================================
/     * Return an array of Catalog or CatalogDirectory objects representing the
/     * path from the root catalog directory to this catalog.
/=============================================================================================*/
     public Catalog[] getPath() {
        return null;
    }
/*=============================================================================================
/    not applicable here
/=============================================================================================*/
    public Catalog reload() {
        return this; //
    }
/*=============================================================================================
/      Implementation of the PlotableCatalog interface methods
/=============================================================================================*/
/** Return the number of plot symbol definitions associated with this catalog. */
    public int getNumSymbols() {
        return _entry.getNumSymbols();
    }

    /** Return the ith plot symbol description */
    public TablePlotSymbol getSymbolDesc(int i) {
        return _entry.getSymbolDesc(i);
    }

    /** Return the array of symbol descriptions */
    public TablePlotSymbol[] getSymbols() {
        return _entry.getSymbols();
    }

    /** Set the array of catalog table plot symbol definitions for use with this catalog */
    public void setSymbols(TablePlotSymbol[] symbols) {
        _entry.setSymbols(symbols);
    }

    /** Set to true if the user edited the plot symbol definitions (default: false) */
    public void setSymbolsEdited(boolean edited) {
        _entry.setSymbolsEdited(edited);
    }

    /** Return true if the user edited the plot symbol definitions otherwise false */
    public boolean isSymbolsEdited() {
        return _entry.isSymbolsEdited();
    }

    /** Save the catalog symbol information to disk with the user's changes */
    public void saveSymbolConfig() {
        SkycatConfigFile.getConfigFile().save();
    }
/*================================================================================================
/    printObjectList()
/=================================================================================================*/
 public void printObjectList(Vector results_vector){
//   setPrint(true);
     java.lang.String currentStarString = new java.lang.String();
     Enumeration e = results_vector.elements();
      while(e.hasMoreElements()){
      Vector current_star = (Vector)e.nextElement();
      currentStarString = (Double)current_star.elementAt(0)*15.0+","+(Double)current_star.elementAt(1);
      System.out.println(currentStarString);
     }
//   setPrint(false);
  }


}
