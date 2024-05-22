package edu.caltech.palomar.util.sindex;
import jsky.science.Coordinates;
import jsky.science.CoordinatesOffset;
import edu.jhu.htm.core.*;
import jsky.coords.WCSTransform;
import java.io.*;
import java.util.*; 
//=== File Prolog =============================================================
//	This code was developed by UCLA, Department of Physics and Astronomy,
//	for the Stratospheric Observatory for Infrared Astronomy (SOFIA) project.
//
//--- Contents ----------------------------------------------------------------
//	HTMSpatialIndexing -
//--- Description -------------------------------------------------------------
//
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
//
//      10/31/03        J. Milburn
//        Original Implementation
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
/*================================================================================================
/      class HTMSpatialIndexing
/=================================================================================================*/
public class HTMSpatialIndexing {
//  Variables associated with creating a WCSTransform
  public double   cra          = 0.0;
  public double   cdec         = 0.0;
  public double   plate_scale  = 0.0;
  public int      nxpix        = 1024;
  public int      nypix        = 1024;
  public double   equinox      = 2000;
  public double   xrpix        = 0.0;
  public double   yrpix        = 0.0;
  public double   plate_ra     = 0.0;
  public double   plate_dec    = 0.0;
//  Assumed to be the same as the plate_scale
  public double   x_pixel_size = 0.0;
  public double   y_pixel_size = 0.0;
//  Assumed to be the RA/DEC of the center pixel of the Chip (ie. plate_ra,plate_dec) units are in RA and DEC
  public double   xref         = 0.0;
  public double   yref         = 0.0;
//  Assumed to be the center pixel of the Chip (ie. xrpix,yrpix) units are x,y row values
  public int      xrefpix      = 512;
  public int      yrefpix      = 512;
//  Assumed to be the same as the plate_scale as long as x_pixel_size and y_pixel_size are the same
  public double   xinc         = 0.0;
  public double   yinc         = 0.0;
//  Assumed to be the same as xrpix and yrpix - center of rotation is the same as the center of the chip
  public double   xmpix        = 0.0;
  public double   ympix        = 0.0;
//  Assumed to Zero unless specifically stated
  public double   mrot         = 0.0;
//  Assumed to Zero unless specifically stated
  public double   rot          = 0.0;
//  Rotation Matrix may be calculated from previously defined values
//  The rotation matrix should yeild NO rotation unless either mrot or rot are not equal to zero
//  This should be a three dimensional matrix [3,3]  cd11,cd12,cd21,cd22;
//  Assumed to be the same as the equinox unless specifcally stated
  public static java.lang.String  KF4 = "FK4";
  public static java.lang.String  KF5 = "FK5";
  public double   epoch           = 2000;
//  Assumed to be zero unless specifically stated
  public double   x_pixel_offset  = 0.0;
  public double   y_pixel_offset  = 0.0;
/*================================================================================================
/      constructor for the HTMSpatialIndexing class
/=================================================================================================*/
  public HTMSpatialIndexing() {
    double myRA    =   37.83660978;
    double myDEC   =   89.38311833;
    int    myWidth =   1800;
//    HTMTest(7,myRA,myDEC,myWidth);
//    HTMTest(8,myRA,myDEC,myWidth);
//    HTMTest(9,myRA,myDEC,myWidth);
//    HTMTest(10,myRA,myDEC,myWidth);
//    HTMTest(11,myRA,myDEC,myWidth);
    DomainTest(5,myRA,myDEC,myWidth);
    DomainTest(6,myRA,myDEC,myWidth);
    DomainTest(7,myRA,myDEC,myWidth);
    DomainTest(8,myRA,myDEC,myWidth);
    DomainTest(9,myRA,myDEC,myWidth);
    DomainTest(10,myRA,myDEC,myWidth);
    DomainTest(11,myRA,myDEC,myWidth);
//    DomainTest(12,myRA,myDEC,myWidth);
//    DomainTest(13,myRA,myDEC,myWidth);
//    DomainTest(14,myRA,myDEC,myWidth);
//    HTMTest(11,82.96366871,0.23576133);
//    WCSTransform myWCSTransform = constructWCSTransform( 8.299033538e+01,2.624279970e-01,0.48,0.48,512,512,1024,1024,0,2000,2000,"LINEAR");
//    System.out.println("This is a test");
  }
/*================================================================================================
/      WCSTransform constructWCSTransform()
/=================================================================================================*/
 public WCSTransform constructWCSTransform(double cra,double cdec,double xsecpix,double ysecpix,
                                           double xrpix,double yrpix,int nxpix,int nypix,double rotate,
                                           int equinox,double epoch,String proj){
     WCSTransform myWCSTransform = new WCSTransform(cra,cdec,xsecpix,ysecpix,xrpix,yrpix,nxpix,nypix,rotate,equinox,epoch,proj);
   return myWCSTransform;
 }
/*================================================================================================
/       getRADEC()
/=================================================================================================*/
public void Test(){
 double myRA   = 0.0;
 double myDEC  = 0.0;
 double myRA2  = 1.0;
 double myDEC2 = 1.0;
 Coordinates lastCoordinates    = new Coordinates(myRA,myDEC);
 Coordinates currentCoordinates = new Coordinates(myRA2,myDEC2);
             evaluateMove(lastCoordinates, currentCoordinates);
}
/*================================================================================================
/       evaluateMoveAccuracy()
/=================================================================================================*/
public void evaluateMove(Coordinates startCoordinates,Coordinates endCoordinates){
    if((startCoordinates != null)&(endCoordinates != null)){
        CoordinatesOffset myCoordinatesOffset = endCoordinates.subtract(startCoordinates);
          double myDeltaRA  = myCoordinatesOffset.getRa(Coordinates.ARCSEC);
          double myDeltaDec = myCoordinatesOffset.getDec(Coordinates.ARCSEC);
        System.out.print("Change in RA measured = " + myDeltaRA + "\n");
        System.out.print("Change in Dec measured = " + myDeltaDec + "\n");
    }
}// end of the method
/*================================================================================================
/       void HTMTest(int newDepth,double newRA,double newDEC)
/=================================================================================================*/
 public void HTMTest(int newDepth,double newRA,double newDEC){
    int depth = newDepth;
    double ra = newRA;
    double dec = newDEC;
    try{
          // read point and initialize
          Vector3d vec = new Vector3d(ra, dec); // initialize Vector3d from ra,dec
          HTMindexImp si = new HTMindexImp(depth, 5);
          // Lookup
          long id = 0;
          id = HTMfunc.lookupId(vec.x(), vec.y(), vec.z(), depth); // lookup id by Vector3d
          long sid = si.lookupId(vec);
        // Print result
        logMessage("(x,y,z)  = " + vec.toString());
        logMessage("(ra,dec) = " + vec.ra() + " " + vec.dec());
        logMessage("ID/Name cc  = " + id + " " + HTMfunc.idToName(id));
        logMessage("ID/Name spatialIndex  = " + sid + " " + si.idToName(id));
  }catch(edu.jhu.htm.core.HTMException htme){
      logMessage("Error constructing HTM Index : " + htme.toString());
  }
 }
/*================================================================================================
/       void HTMTest(int newDepth,double newRA,double newDEC)
/=================================================================================================*/
  public void HTMTest(int newDepth,double newRA,double newDEC,int detectorWidth){
     int depth = newDepth;
     double ra = newRA;
     double dec = newDEC;
     double offset = (double)(detectorWidth/2);
     double delta  = (offset/3600.0);
     System.out.println("Level = " + newDepth);

     double ra1  = ra-delta;
     double dec1 = dec+delta;
     double ra2  = ra+delta;
     double dec2 = dec+delta;
     double ra3  = ra+delta;
     double dec3 = dec-delta;
     double ra4  = ra-delta;
     double dec4 = dec-delta;

     try{
           // read point and initialize
           Vector3d vec = new Vector3d(ra, dec); // initialize Vector3d from ra,dec
           Vector3d vec1 = new Vector3d(ra1, dec1); // initialize Vector3d from ra,dec
           Vector3d vec2 = new Vector3d(ra2, dec2); // initialize Vector3d from ra,dec
           Vector3d vec3 = new Vector3d(ra3, dec3); // initialize Vector3d from ra,dec
           Vector3d vec4 = new Vector3d(ra4, dec4); // initialize Vector3d from ra,dec
           HTMindexImp si  = new HTMindexImp(depth, 5);
           HTMindexImp si1 = new HTMindexImp(depth, 5);
           HTMindexImp si2 = new HTMindexImp(depth, 5);
           HTMindexImp si3 = new HTMindexImp(depth, 5);
           HTMindexImp si4 = new HTMindexImp(depth, 5);
           // Lookup
           long id = 0;
           long id1 = 0;
           long id2 = 0;
           long id3 = 0;
           long id4 = 0;
           id  = HTMfunc.lookupId(vec.x(), vec.y(), vec.z(), depth); // lookup id by Vector3d
           id1 = HTMfunc.lookupId(vec1.x(), vec1.y(), vec1.z(), depth); // lookup id by Vector3d
           id2 = HTMfunc.lookupId(vec2.x(), vec2.y(), vec2.z(), depth); // lookup id by Vector3d
           id3 = HTMfunc.lookupId(vec3.x(), vec3.y(), vec3.z(), depth); // lookup id by Vector3d
           id4 = HTMfunc.lookupId(vec4.x(), vec4.y(), vec4.z(), depth); // lookup id by Vector3d
           long sid  = si.lookupId(vec);
           long sid1 = si1.lookupId(vec1);
           long sid2 = si2.lookupId(vec2);
           long sid3 = si3.lookupId(vec3);
           long sid4 = si4.lookupId(vec4);

         // Print result
         logMessage(vec.ra()  + "," + vec.dec() +"," + sid + " " + si.idToName(id));
         logMessage(vec1.ra() + "," + vec1.dec()+"," + sid1 + " " + si1.idToName(id1));
         logMessage(vec2.ra() + "," + vec2.dec()+"," + sid2 + " " + si2.idToName(id2));
         logMessage(vec3.ra() + "," + vec3.dec()+"," + sid3 + " " + si3.idToName(id3));
         logMessage(vec4.ra() + "," + vec4.dec()+"," + sid4 + " " + si4.idToName(id4));
         // Now construct the HTM Triangle for the given center point and display it.
         double v0[] = new double[3];
         double v1[] = new double[3];
         double v2[] = new double[3];
         java.lang.String centralHTMName = HTMfunc.idToName(sid);
         logMessage("Central Vector HTM Name = " + centralHTMName);
         Object[] vectorArray =  HTMfunc.nameToTriangle(centralHTMName);
         double[] arrayVector1 =  (double[])(vectorArray[0]);
         double[] arrayVector2 =  (double[])(vectorArray[1]);
         double[] arrayVector3 =  (double[])(vectorArray[2]);
         Vector3d   vector1     = new Vector3d(arrayVector1[0],arrayVector1[1],arrayVector1[2]);
         Vector3d   vector2     = new Vector3d(arrayVector2[0],arrayVector2[1],arrayVector2[2]);
         Vector3d   vector3     = new Vector3d(arrayVector3[0],arrayVector3[1],arrayVector3[2]);
//         vector1.set(arrayVector1[0],arrayVector1[1],arrayVector1[2]);
//         vector2.set(arrayVector2[0],arrayVector2[1],arrayVector2[2]);
//         vector3.set(arrayVector3[0],arrayVector2[1],arrayVector3[2]);

         HTMfunc.idToPoint(centralHTMName);
         double     vertex1RA    = vector1.ra();
         double     vertex1DEC   = vector1.dec();
         double     vertex2RA    = vector2.ra();
         double     vertex2DEC   = vector2.dec();
         double     vertex3RA    = vector3.ra();
         double     vertex3DEC   = vector3.dec();
         logMessage("Vertices for the central Vector :");
         logMessage(vertex1RA + "," + vertex1DEC);
         logMessage(vertex2RA + "," + vertex2DEC);
         logMessage(vertex3RA + "," + vertex3DEC);

//         int[] verticesArray = si.nodeVertexIds((int)sid);

         logMessage("HTM Id = " + sid4 + "Id Name = " + si4.idToName(id4) + "HTM Name to ID = " + si4.nameToId(si4.idToName(sid4)));

   }catch(edu.jhu.htm.core.HTMException htme){
       logMessage("Error constructing HTM Index : " + htme.toString());
   }
  }
/*================================================================================================
/       void HTMTest(int newDepth,double newX,double newY,double newZ)
/=================================================================================================*/
  public void DomainTest(int newDepth,double newRA,double newDEC,int detectorWidth){
    int depth = newDepth;
    double ra = newRA;
    double dec = newDEC;
    double offset = (double)(detectorWidth/2);
    double delta  = (offset/3600.0);
    System.out.println("Level = " + newDepth);
    boolean varlen = false;
    boolean compress = false;
    int nranges = 100;
    boolean expand = true;
    boolean symb = true;

    double ra1  = ra-delta;
    double dec1 = dec+delta;
    double ra2  = ra+delta;
    double dec2 = dec+delta;
    double ra3  = ra+delta;
    double dec3 = dec-delta;
    double ra4  = ra-delta;
    double dec4 = dec-delta;

    Coordinates Coordinates1    = new Coordinates(ra1,dec1);
    Coordinates Coordinates2    = new Coordinates(ra2,dec2);
    Coordinates Coordinates3    = new Coordinates(ra3,dec3);
    Coordinates Coordinates4    = new Coordinates(ra4,dec4);
                evaluateMove(Coordinates1, Coordinates2);
                evaluateMove(Coordinates2, Coordinates3);
                evaluateMove(Coordinates3, Coordinates4);
                evaluateMove(Coordinates4, Coordinates1);


    Vector3d vec  = new Vector3d(ra, dec); // initialize Vector3d from ra,dec
    Vector3d vec1 = new Vector3d(ra1, dec1); // initialize Vector3d from ra,dec
    Vector3d vec2 = new Vector3d(ra2, dec2); // initialize Vector3d from ra,dec
    Vector3d vec3 = new Vector3d(ra3, dec3); // initialize Vector3d from ra,dec
    Vector3d vec4 = new Vector3d(ra4, dec4); // initialize Vector3d from ra,dec


    HTMindexImp si = new HTMindexImp(depth,5);
    Domain   domain    = new Domain();
    Convex   rectangle = new Convex(vec1,vec2,vec3,vec4);
             rectangle.setOlevel(depth);
    HTMrange htmRange  = new HTMrange();
             rectangle.intersect(si,htmRange,false);
             int numberOfRanges = htmRange.nranges();
             logMessage("Number of Ranges = " + numberOfRanges);
//             domain.add(rectangle);
//             domain.intersect(si,htmRange,false);

    try{
      HTMrangeIterator myHTMrangeIterator = new HTMrangeIterator(htmRange,true);
//      while(myHTMrangeIterator.hasNext()){
//         System.out.println("HTM Range Iteration = " + myHTMrangeIterator.nextAsString());
//      }
      if (!varlen) {
        htmRange.defrag();
        if (compress) {
          htmRange.defrag(nranges);
        }
        if (expand) {
          expandList(System.out, htmRange, symb);
        }
        else {
          System.out.println(htmRange.toString(symb));
        }
      }
      else {
        System.out.println(htmRange.toString(HTMrange.LOWS, symb));
      }
    }catch(Exception e){
      System.out.println(e.toString());
    }
  }
/*================================================================================================
/      public void expandList(PrintStream out, HTMrange range , boolean symbolic)
/=================================================================================================*/
  static public void expandList(PrintStream out, HTMrange range , boolean symbolic) throws Exception{
          HTMrangeIterator iter = new HTMrangeIterator(range,symbolic);
          while (iter.hasNext()) {
                  out.println(iter.next());
          }
  }
/*================================================================================================
/       void HTMTest(int newDepth,double newX,double newY,double newZ)
/=================================================================================================*/
  public void HTMTest(int newDepth,double newX,double newY,double newZ){
    int    depth = newDepth;
    double x = newX;
    double y = newY;
    double z = newZ;
    try{
          // read point and initialize
          Vector3d vec = new Vector3d(x,y,z); // initialize from x,y,z
                   vec.normalize();
          HTMindexImp si = new HTMindexImp(depth,5);
          // Lookup
          long id =0;
          id = HTMfunc.lookupId(vec.x(),vec.y(),vec.z(),depth);// lookup id by Vector3d
          long sid =si.lookupId(vec);
        // Print result
        logMessage("(x,y,z)  = "             + vec.toString());
        logMessage("(ra,dec) = "             + vec.ra() + " " + vec.dec());
        logMessage("ID/Name cc  = "          + id + " "  +HTMfunc.idToName(id));
        logMessage("ID/Name spatialIndex  = "+ sid + " " +si.idToName(id));
  }catch(edu.jhu.htm.core.HTMException htme){
      logMessage("Error constructing HTM Index : " + htme.toString());
  }
  }
/*=================================================================================
/          logMessage Method  - not yet implemented
/=================================================================================*/
    public void logMessage(String myMessage){
       System.out.println(myMessage);
    }
/*================================================================================================
/       main (String[] argv)
/=================================================================================================*/
   public static void main (String[] argv) throws Exception{
      new HTMSpatialIndexing();
   }
/*================================================================================================
/      end of the class HTMSpatialIndexing
/=================================================================================================*/
}