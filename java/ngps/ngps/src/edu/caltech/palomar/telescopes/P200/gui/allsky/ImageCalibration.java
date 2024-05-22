/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package edu.caltech.palomar.telescopes.P200.gui.allsky;
//=== File Prolog =============================================================
//	This code was developed by California Institute of Technology
//      Caltech Optical Observatories, Palomar Observatory
//
//--- Contents ----------------------------------------------------------------
//	 ImageCalibration - a tool used for calibration of the all sky camera
//
//--- Description -------------------------------------------------------------
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
//
//      April 12, 2012 Jennifer Milburn
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
//	In no event shall Caltech be liable for any damages, including, but not
//	limited to direct, indirect, special or consequential damages, arising out
//	of, resulting from, or in any way connected with this software, whether or
//	not based upon warranty, contract, tort or otherwise, whether or not
//	injury was sustained by persons or property or otherwise, and whether or
//	not loss was sustained from or arose out of the results of, or use of,
//	their software or services provided hereunder.
//
//=== End File Prolog =========================================================
import edu.dartmouth.jskycalc.JSkyCalcModel;
import edu.dartmouth.jskycalc.objects.BrightStar;
import edu.dartmouth.jskycalc.coord.Celest;
import edu.dartmouth.jskycalc.coord.Const;
import com.sharkysoft.printf.Printf;
import com.sharkysoft.printf.PrintfData;
import com.sharkysoft.printf.PrintfTemplate;
import com.sharkysoft.printf.PrintfTemplateException;
import java.io.File;
import java.io.*;
import java.util.ArrayList;
/*================================================================================================
/     ImageCalibration Class Declaration
/=================================================================================================*/
public class ImageCalibration {
    JSkyCalcModel myJSkyCalcModel;
    BrightStar[]  bright_star_array;
    BrightStar[]  target_star_array;
    public ArrayList     target_star_array_list = new ArrayList();
    int           bright_star_number;
    public java.lang.String dateString = new java.lang.String();
    public java.lang.String timeString = new java.lang.String();
    double ti, tf, zeta, z, theta;
    double cosz, coszeta, costheta, sinz, sinzeta, sintheta, cosdelt;
    double [][] p = {{0.,0.,0.,},{0.,0.,0.},{0.,0.,0.}};
    double [] orig = {0.,0.,0.};
    double [] fin = {0.,0.,0.};
    double [] radecdist = {0.,0.,0.};
    int         xpixint, ypixint;       // number of pixels in x and y directions, int
    double      xpix, ypix, aspect;  // number of pixels in x and y directions, double
    double      xmid, ymid;          // x, y coords of the center of the screen.
    double      halfwidthy;          // sets the scale factor.
    double      halfwidthx;
    double      pixperunit;
    double      halfwidthxfull, halfwidthyfull, pixperunitfull;
/*================================================================================================
/     ImageCalibration Constructor()
/=================================================================================================*/
   public ImageCalibration(){
      initializeJSkyCalcModel();
      test();
   }
   public void test(){
      initializeScreenParameters(800,700);
      double center[] = xytopix(0, 0);
      System.out.println(myJSkyCalcModel.s.lat.degrees());
      System.out.println("center X = "+ center[0]+" center Y = "+ center[0]);
      readFile("/home/developer/CalibrationStars2.csv");
      calculatePositions("2010 Sep 20", "05 16 23");
//      constructTargetArray();
//      bright_star_array = target_star_array;
//      myJSkyCalcModel.setBrightStars(bright_star_array);
//      calculatePositions("2010 Sep 20", "05 16 13");
  }
/*================================================================================================
/     constructTargetArray()
/=================================================================================================*/
public void constructTargetArray(){
    int num_target_stars = 24;
    target_star_array = new BrightStar[num_target_stars];
    target_star_array[0] = constructBrightStar("Arneb,05 32 43.816 ,-17 49 20.24,2.817");
    target_star_array[1] = constructBrightStar("Saiph,05 47 45.389,-09 40 10.58,1.937");
    target_star_array[2] = constructBrightStar("Rigel,05 14 32.272,-08 12 05.90,0.09");
    target_star_array[3] = constructBrightStar("Bellatrix,05 25 07.863,+06 20 58.93,1.42");
    target_star_array[4] = constructBrightStar("Betelgeuse,05 55 10.305,+07 24 25.43,2.27");
    target_star_array[5] = constructBrightStar("Procyon,07 39 18.119,+05 13 29.96,0.34");
    target_star_array[6] = constructBrightStar("Alhena,06 37 42.711,+16 23 57.41,1.93");
    target_star_array[7] = constructBrightStar("Pollux,07 45 18.950,+28 01 34.32,2.15");
    target_star_array[8] = constructBrightStar("ThetaAurigae,05 59 43.270,+37 12 45.30,2.54");
    target_star_array[9] = constructBrightStar("Algol,03 08 10.132,+40 57 20.33,2.12");
    target_star_array[10] = constructBrightStar("Menkalinan,05 59 31.723,+44 56 50.76,1.969");
    target_star_array[11] = constructBrightStar("Capella,05 16 41.359,+45 59 52.77,0.88");
    target_star_array[12] = constructBrightStar("Mirach,01 09 43.924,+35 37 14.01,3.64");
    target_star_array[13] = constructBrightStar("Alpheratz,00 08 23.260,+29 05 25.55,2.012");
    target_star_array[14] = constructBrightStar("Schedar,00 40 30.441,+56 32 14.39,3.434");
    target_star_array[15] = constructBrightStar("Caph,00 09 10.685,+59 08 59.21,2.61");
    target_star_array[16] = constructBrightStar("Merak,11 01 50.477,+56 22 56.73,2.346");
    target_star_array[17] = constructBrightStar("Dubhe,11 03 43.672,+61 45 03.72,1.79");
    target_star_array[18] = constructBrightStar("Phecda,11 53 49.847,+53 41 41.14,2.45");
    target_star_array[19] = constructBrightStar("Polaris,02 31 49.095,+89 15 50.79,2.591");
    target_star_array[20] = constructBrightStar("Kochab,14 50 42.326,+74 09 19.81,3.589");
    target_star_array[21] = constructBrightStar("Castor,07 34 35.873,+31 53 17.82,1.63");
    target_star_array[22] = constructBrightStar("NorthPole,00 00 0.0,+90 00 00.0,1.63");
    target_star_array[23] = constructBrightStar("Zenith,05 16 13.0,+33 21 24.0,1.63");
}
/*=============================================================================================
/   readFile(java.lang.String currentFileName)
/=============================================================================================*/
 public void readFile(java.lang.String currentFileName){
     java.lang.String          CommentString    = new java.lang.String();
     BufferedReader br = null;
//       String st;
       String current_line;
        try {
               FileInputStream fis = new FileInputStream(currentFileName);
               br = new BufferedReader(new InputStreamReader(fis));
          } catch (Exception e){
              System.out.printf("Problem opening selected Bright Star Calibration File for input.\n");
          }
          try {
             int index = 0;
             while((current_line = (br.readLine()).trim()) != null & current_line.length() != 0) {
              current_line = current_line;
              try{
                BrightStar current_bright_star = constructBrightStar(current_line);
                target_star_array_list.add(current_bright_star);
//                System.out.println(current_line);
                index++;
              }catch(Exception e2){
               System.out.println("Error Bright Star Calibration File. Row = "+index);

               }// end of the catch block
             }// end of the while loop
          } catch (IOException e){
              System.out.printf("Problem parsing Bright Star Calibration File.\n" + e.toString());
          }
        loadBrightStarArray(target_star_array_list);
 }
/*================================================================================================
/     constructBrightStar(java.lang.String description)
/=================================================================================================*/
public void loadBrightStarArray(ArrayList     star_array_list){
    int length = star_array_list.size();
    for(int i=0;i<length-1;i++){
       bright_star_array[i] = (BrightStar)star_array_list.get(i);
    }
    myJSkyCalcModel.setBrightStars(bright_star_array);
}
/*================================================================================================
/     constructBrightStar(java.lang.String description)
/=================================================================================================*/
public BrightStar constructBrightStar(java.lang.String description){
   java.util.StringTokenizer st = new java.util.StringTokenizer(description,",");
   java.lang.String name = st.nextToken();
   java.lang.String RA   = st.nextToken();
   java.lang.String Dec  = st.nextToken();
   java.lang.String mag  = st.nextToken();
   Celest myCelest = new Celest(RA,Dec,"2000");
   BrightStar myStar = new BrightStar();
   myStar.c = myCelest;
   myStar.name = name;
   myStar.m    = Double.parseDouble(mag);
   return myStar;
 }
/*================================================================================================
/     initializeJSkyCalcModel()
/=================================================================================================*/
public void initializeScreenParameters(int xpixIn, int ypixIn){
   xpixint     = xpixIn;
   ypixint     = ypixIn;
   xpix        = (double) xpixint;
   ypix        = (double) ypixint;
   aspect      = xpix / ypix;
   xmid        = 0.;
   ymid        = 0.;
   halfwidthy  = 0.88;
   halfwidthx  = halfwidthy * aspect;
   pixperunit  = ypix / (2. * halfwidthy);
   halfwidthxfull = halfwidthx;
   halfwidthyfull = halfwidthy;
   pixperunitfull = pixperunit;
}
/*================================================================================================
/     initializeJSkyCalcModel()
/=================================================================================================*/
  public void initializeJSkyCalcModel(){
      myJSkyCalcModel = new JSkyCalcModel();
      bright_star_array = myJSkyCalcModel.getBrightStars();
      bright_star_number = bright_star_array.length;
      myJSkyCalcModel.setTelescopePosition(bright_star_array[0].c.Alpha.RoundedRAString(2, ":"), bright_star_array[0].c.Delta.RoundedDecString(2, ":"),
                                            Printf.format("%04.0f", new PrintfData().add(bright_star_array[0].c.Equinox)));
      myJSkyCalcModel.SetToNow();
   }
/*================================================================================================
/     SkyProject(double hain, double decin, double coslat, double sinlat)
/=================================================================================================*/
       double [] SkyProject(double hain, double decin, double coslat, double sinlat) {

          double [] retvals  = {0.,0.};

          // This will be called many times so code it for speed!

          double ha = hain / Const.HRS_IN_RADIAN;
          double dec = decin / Const.DEG_IN_RADIAN;
          double x,y,z;

          double cosdec = Math.cos(dec);

          x = cosdec * Math.sin(ha);
          y = cosdec * Math.cos(ha);
          z = Math.sin(dec);

          double ypr = sinlat * y - coslat * z;   // rotating backward, by CO-latitude, so sin
          double zpr = coslat * y + sinlat * z;   // and cos switch around.

          double zdist = Math.acos(zpr);
          double r = Math.tan(zdist / 2.);
          double inground = Math.sqrt(x * x + ypr * ypr);
          retvals[0] =  r * (x / inground);
          retvals[1] = -1. * r * (ypr / inground);
          if(sinlat < 0.)  {  // invert for south
              retvals[0] = -1. * retvals[0];
              retvals[1] = -1. * retvals[1];
          }

          return retvals;
       }
/*================================================================================================
/       xytopix(double x, double y)
/=================================================================================================*/
   double [] xytopix(double x, double y) {
       double [] retvals = {0.,0.,};
       retvals[0] = 0.5 * xpix * (1. + (x - xmid) / halfwidthx);
       retvals[1] = 0.5 * ypix * (1. - (y - ymid) / halfwidthy);
       return retvals;
   }
 /*================================================================================================
/     PrecessBrightStars(double equinox)
/=================================================================================================*/
 public void initializePrecessionMatrix(double equinox){
          // precesses the stars in the BrightStar list bs.  This is more efficient
          // with a special method because the matrix only gets computed once.
          // code is largely copied from the precess method of Celest.
          // compute the precession matrix ONCE.
          ti = (bright_star_array[0].c.Equinox - 2000.) / 100.;
          tf = (equinox - 2000. - 100. * ti) / 100.;

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
 
 }
/*================================================================================================
/     PrecessBrightStars(double equinox)
/=================================================================================================*/
   void PrecessBrightStars(double equinox,BrightStar currentBrightStar) {
          int i, j, ist;
          initializePrecessionMatrix(equinox);
//          for(ist = 0; ist < bs.length; ist++) {
         cosdelt = Math.cos(currentBrightStar.c.Delta.radians());
         orig[0] = cosdelt * Math.cos(currentBrightStar.c.Alpha.radians());
         orig[1] = cosdelt * Math.sin(currentBrightStar.c.Alpha.radians());
         orig[2] = Math.sin(currentBrightStar.c.Delta.radians());
         for(i = 0; i < 3; i++) {  // matrix multiplication
            fin[i] = 0.;
            //System.out.printf("orig[%d] = %f\n",i,orig[i]);
            for(j = 0; j < 3; j++) {
                //System.out.printf("%d%d: %f  ",i,j,p[i][j]);
                fin[i] += p[i][j] * orig[j];
            }
            //System.out.printf("\nfin[%d] = %f\n\n",i,fin[i]);
         }
         radecdist = Celest.XYZcel(fin[0],fin[1],fin[2]);
         currentBrightStar.c.Alpha.setRA(radecdist[0]);
         currentBrightStar.c.Delta.setDec(radecdist[1]);
         currentBrightStar.c.Equinox = equinox;
//          }
       }

/*================================================================================================
/     PlotBright()
/=================================================================================================*/
       double[] PlotBright(BrightStar currentBrightStar) {
          // stripped down for speed -- avoids the OO stuff .
          // Precesses only if mismatch is > 1 year.
          double ha, dec, lst;
          double equinoxnow;
          double xy[];
          double magconst1 = 0.002, magslope = 0.002;  //
	  double magzpt = 4.7;
          int i;

          lst = myJSkyCalcModel.getObsevation().w.sidereal;

          // precess the list if the epoch mismatch is > 1 yr
          equinoxnow = myJSkyCalcModel.getObsevation().w.when.JulianEpoch();
          if(Math.abs(equinoxnow - currentBrightStar.c.Equinox) > 1.) {
              // System.out.printf("%s -> ",bs[0].c.checkstring());
              PrecessBrightStars(equinoxnow,currentBrightStar);
              // System.out.printf("%s \n",bs[0].c.checkstring());
          }
          double coslat = Math.cos( myJSkyCalcModel.getObsevation().w.where.lat.radians());
          double sinlat = Math.sin( myJSkyCalcModel.getObsevation().w.where.lat.radians());
//          for(i = 0; i < bs.length; i++) {
             xy = SkyProject(lst - currentBrightStar.c.Alpha.value, currentBrightStar.c.Delta.value, coslat,sinlat);
             //System.out.printf("PB %f %f\n",xy[0],xy[1]);
 //            putDot(xy[0],xy[1],magconst1 + magslope * (magzpt - bs[i].m),bs[i].col);
 //         }
         return xy;
   }
/*================================================================================================
/     main(String args[])
/=================================================================================================*/
public void calculatePositions(java.lang.String newDateString, java.lang.String newTimeString){
   dateString = newDateString;
   timeString = newTimeString;
   double[] xy;
   double[] pix;
   bright_star_number = bright_star_array.length;
   myJSkyCalcModel.setToDate(newDateString, newTimeString);
   myJSkyCalcModel.synchSite();
//   myJSkyCalcModel.o.ComputeSky();
   java.lang.String currentRecord = new java.lang.String();
   for(int i=0;i<bright_star_number;i++){
       myJSkyCalcModel.setObjectName(bright_star_array[i].name);
       myJSkyCalcModel.setTelescopePosition(bright_star_array[i].c.Alpha.RoundedRAString(2, ":"), bright_star_array[i].c.Delta.RoundedDecString(2, ":"),
                                            Printf.format("%04.0f", new PrintfData().add(bright_star_array[i].c.Equinox)));
       myJSkyCalcModel.synchOutput();
       xy =  PlotBright(bright_star_array[i]);
         // dotsize, unfortunately, is in user units.
       pix = xytopix(xy[0],xy[1]);
       currentRecord = bright_star_array[i].name + ","+bright_star_array[i].m+","
                       +myJSkyCalcModel.getAzimuthString() + ","+Printf.format("%6.2f", new PrintfData().add(myJSkyCalcModel.getAltitude()))+","+
                       Printf.format("%6.2f", new PrintfData().add(xy[0]))+","+Printf.format("%6.2f", new PrintfData().add(xy[1]))+","+
                       Printf.format("%6.2f", new PrintfData().add(pix[0]))+","+Printf.format("%6.2f", new PrintfData().add(pix[1]));
       System.out.println(currentRecord);
    }
  }
/*================================================================================================
/     main(String args[])
/=================================================================================================*/
   public static void main(String args[]) {
        java.awt.EventQueue.invokeLater(new Runnable() {
            public void run() {
                new ImageCalibration();
            }
        });
    }
}
