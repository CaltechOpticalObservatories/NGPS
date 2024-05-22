/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package edu.caltech.palomar.telescopes.P200.gui.allsky;
//=== File Prolog =============================================================
//	This code was developed by UCLA, Department of Physics and Astronomy,
//	for the Stratospheric Observatory for Infrared Astronomy (SOFIA) project.
//
//--- Contents ----------------------------------------------------------------
//	AllSkyImageProcessor  -
//
//--- Description -------------------------------------------------------------
//	Retrieves images from the All Sky Camera from the Website and display them
//
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
//
//      April 22, 2012      Jennifer W Milburn
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
/*=============================================================================================
/     Import packages section
/=============================================================================================*/
import java.awt.Color;
import edu.dartmouth.jskycalc.gui.SkyDisplaySimple;
import nom.tam.fits.Fits;
import nom.tam.fits.ImageData;
import edu.caltech.palomar.telescopes.P200.TelescopesIniReader;
/*=============================================================================================
/    AllSkyImageProcessor Class Definition
/=============================================================================================*/
public class AllSkyImageProcessor {
  public int[][]                       red;
  public int[][]                       green;
  public int[][]                       blue;
  private MonitorAllSkyImageThread     myMonitorAllSkyImageThread;
  public  SkyDisplaySimple             mySkyDisplaySimple;
  public  TelescopesIniReader          myTelescopesIniReader;
  public int minX ;
  public int minY;
  public int height;
  public int width;
  public int bands;
  public int[][] mask;
  public int[][] overlay_mask;
  public int overlay_x;
  public int overlay_y;
  public int start_column;
  public int overlay_center_dy;
  public int overlay_center_dx;
  public int decenter_x;
  public int decenter_y;
  public java.awt.Color[][] overlay;
  private java.awt.Color[][] interpolation_mask;
  public int mask_diameter;
  public double slope;
  public double scale;
  public double theta;
  public int dXJsky    = 0;
  public int dYJsky    = 0;
  public double decenterX;
  public double decenterY;
  public double coef_a_quad;
  public double coef_b_quad;
  public double coef_c_quad;
  public double theta_degrees;
  public  java.lang.String  ALLSKY_HOST                     = new java.lang.String();
  public  java.lang.String  ALLSKY_PATH                     = new java.lang.String();
  public  java.lang.String  ALLSKY_IMAGE_NAME               = new java.lang.String();
  public  int               ALLSKY_PORT;
  public  int               ALLSKY_POLLING_TIME;
/*=============================================================================================
/    AllSkyImageProcessor Constructor
/=============================================================================================*/
   public AllSkyImageProcessor(){
//      startImageMonitor();
//       int dx = 35;
//       int dy = 15;
       height = 480;
       width = 640;
       mask_diameter = 700;
   }
/*=============================================================================================
/    getOverlay()
/=============================================================================================*/
public synchronized Color[][] getOverlay(){
    return overlay;
}
/*=============================================================================================
/    setTelescopeIniReader(TelescopesIniReader newTelescopesIniReader)
/=============================================================================================*/
public void setTelescopeIniReader(TelescopesIniReader newTelescopesIniReader){
  myTelescopesIniReader = newTelescopesIniReader;
  initializeOverlay();
  overlay_mask = constructOverlayMask(myTelescopesIniReader.OVERLAY_MASK_DIAMETER,400,400);
    int dXImage =width/2;
    int dYImage = height/2;
    decenterX = myTelescopesIniReader.DECENTER_X;
    decenterY = myTelescopesIniReader.DECENTER_Y;
    coef_a_quad = myTelescopesIniReader.COEFFICIENT_A;
    coef_b_quad = myTelescopesIniReader.COEFFICIENT_B;
    coef_c_quad = myTelescopesIniReader.COEFFICIENT_C;
    theta_degrees = myTelescopesIniReader.ROTATION;
    theta = degreesToRadians(theta_degrees);
    ALLSKY_HOST = myTelescopesIniReader.ALLSKY_HOST;
    ALLSKY_PATH = myTelescopesIniReader.ALLSKY_PATH;
    ALLSKY_IMAGE_NAME = myTelescopesIniReader.ALLSKY_IMAGE_NAME;
    ALLSKY_PORT = myTelescopesIniReader.ALLSKY_PORT;
    ALLSKY_POLLING_TIME = myTelescopesIniReader.ALLSKY_POLLING_TIME;
    mask = constructMask(myTelescopesIniReader.RAW_MASK_DIAMETER,(int)(dYImage-decenterY),(int)(dXImage-decenterX));
}
/*=============================================================================================
/    setSkyDisplay(SkyDisplaySimple newSkyDisplaySimple)
/=============================================================================================*/
public void setSkyDisplay(SkyDisplaySimple newSkyDisplaySimple){
    mySkyDisplaySimple = newSkyDisplaySimple;
}
public SkyDisplaySimple getSkyDisplay(){
    return mySkyDisplaySimple;
}
/*=============================================================================================
/    startImageMonitor()
/=============================================================================================*/
public void startImageMonitor(){
  myMonitorAllSkyImageThread = new MonitorAllSkyImageThread();
  myMonitorAllSkyImageThread.setPollingTime(ALLSKY_POLLING_TIME);
  myMonitorAllSkyImageThread.start();
}
public void stopImageMonitor(){
    myMonitorAllSkyImageThread.setRunning(false);
}
/*=============================================================================================
/     degreesToRadians(double degrees)
/=============================================================================================*/
 public double degreesToRadians(double degrees){
   double radians = degrees*(Math.PI/180.00);
   return radians;
 }
/*=============================================================================================
/    initializeOverlay()
/=============================================================================================*/
public void initializeOverlay(){
   overlay_x = 800;
   overlay_y = 800;
   overlay = new Color[overlay_y][overlay_x];
   interpolation_mask = new Color[overlay_y][overlay_x];
   for(int i=0;i<overlay_y-1;i++){
       for(int j=0;j<overlay_x-1;j++){
           overlay[i][j] = new Color(0,0,0);
           interpolation_mask[i][j] = new Color(0,0,0);
       }
   }
}
/*=============================================================================================
/    setSlope(double new_slope)
/=============================================================================================*/
public void setSlope(double new_slope){
    slope = new_slope;
}
public void setRotation(double new_rotation){
    double theta_degrees = new_rotation;
    theta = degreesToRadians(theta_degrees);
}
public void setDX(int new_DX){
    dXJsky = new_DX;
}
public void setDY(int new_DY){
    dYJsky = new_DY;
}
/*=============================================================================================
/    constructOverlay()
/=============================================================================================*/
public void constructOverlay(){
    int x_index = 0;
    int y_index = 0;
    double rotated_X,rotated_Y,uncorrected_R,phi,corrected_R,corrected_X,corrected_Y;
    for(int i=0;i<height-1;i++){
       for(int j=0;j<width-1;j++){
           try{
//           x_index = (int)Math.round(((j-start_column+overlay_center_dx-decenter_x)*Math.cos(theta) - (i+overlay_center_dy-decenter_y)*Math.sin(theta))*slope*scale);
//           y_index = (int)Math.round(((j-start_column+overlay_center_dx-decenter_x)*Math.sin(theta) + (i+overlay_center_dy-decenter_y)*Math.cos(theta))*slope*scale);
           rotated_X = ((j-(width/2)+decenterX)*Math.cos(theta))-((i-(height/2)+decenterY)*Math.sin(theta));
           rotated_Y = ((j-(width/2)+decenterX)*Math.sin(theta))+((i-(height/2)+decenterY)*Math.cos(theta));
           uncorrected_R = Math.sqrt((rotated_X*rotated_X) + (rotated_Y*rotated_Y));
           phi           = Math.atan2(rotated_X, rotated_Y);
           corrected_R   = uncorrected_R*coef_a_quad+(uncorrected_R*uncorrected_R)*coef_b_quad+coef_c_quad;
           corrected_X   = corrected_R*Math.sin(phi)+overlay_x/2;
           corrected_Y   = corrected_R*Math.cos(phi)+overlay_y/2;
           x_index = (int)Math.round(corrected_X)+dXJsky;
           y_index = (int)Math.round(corrected_Y)+dYJsky;
//           x_index = (int)(Math.round(((((j-dXImage+decenterX)*Math.cos(theta))-((i-dYImage+decenterY)*Math.sin(theta)))*slope + dXJsky+dXJ)));
//           y_index = (int)(Math.round(((((j-dXImage+decenterX)*Math.sin(theta))+((i-dYImage+decenterY)*Math.cos(theta)))*slope + dYJsky+dYJ)));
            if((x_index >= 0)&(x_index < overlay_x)){
              if((y_index >=0)&(y_index < overlay_y)){
                overlay[y_index][x_index] = new Color(red[i][j],green[i][j],blue[i][j]);
              }
           }
           }catch(Exception e){
               System.out.println("Error at index x_index = "+ x_index + " y_index = "+ y_index);
           }
       }
   }
}
/*=============================================================================================
/    interpolate()
/=============================================================================================*/
public void applyMask(){
   for(int i=0;i<overlay_y-1;i++){
     for(int j=0;j<overlay_x-1;j++){
         if(overlay_mask[i][j] == 0){
             overlay[i][j] = new Color(0,0,0);
         }
      }
   }
}
/*=============================================================================================
/    interpolate()
/=============================================================================================*/
public void interpolate(){
    int currentRed   = 0;
    int currentGreen = 0;
    int currentBlue  = 0;
    int boundary = 10;
    // interpolate the missing pixels due to interpolation to a larger grid size
   try{
    // Make a copy of the array that can be used as the source of values uneffected by the interpolation direction
    for(int i=boundary;i<overlay_y-1-boundary;i++){
        for(int j=boundary;j<overlay_x-1-boundary;j++){
               interpolation_mask[i][j] = overlay[i][j];
        }
    }
    for(int i=boundary;i<overlay_y-1-boundary;i++){
        for(int j=boundary;j<overlay_x-1-boundary;j++){
            if((interpolation_mask[i][j].getBlue() == 0) ){
//            if((interpolation_mask[i][j].getBlue() == 0) | (interpolation_mask[i][j].getGreen() == 0) | (interpolation_mask[i][j].getRed() == 0) ){
               int c1 =  interpolation_mask[i-1][j].getRed();
               int c2 =  interpolation_mask[i+1][j].getRed();
               int c3 =  interpolation_mask[i][j-1].getRed();
               int c4 =  interpolation_mask[i][j+1].getRed();
               int c5 =  interpolation_mask[i-1][j-1].getRed();
               int c6 =  interpolation_mask[i+1][j+1].getRed();
               int sum = 0;
               int count = 0;
               if(c1 != 0){
                  sum = sum + c1;
                  count = count + 1;
               }
               if(c2 != 0){
                  sum = sum + c2;
                  count = count + 1;
               }
               if(c3 != 0){
                  sum = sum + c3;
                  count = count + 1;
               }
               if(c4 != 0){
                  sum = sum + c4;
                  count = count + 1;
               }
               if(c5 != 0){
                  sum = sum + c5;
                  count = count + 1;
               }
               if(c6 != 0){
                  sum = sum + c6;
                  count = count + 1;
               }
               if(count != 0){
                  currentRed = sum/count; 
               }
               if(count == 0){
                   currentRed = 0;
               }
               c1 =  interpolation_mask[i-1][j].getGreen();
               c2 =  interpolation_mask[i+1][j].getGreen();
               c3 =  interpolation_mask[i][j-1].getGreen();
               c4 =  interpolation_mask[i][j+1].getGreen();
               c5 =  interpolation_mask[i-1][j-1].getGreen();
               c6 =  interpolation_mask[i+1][j+1].getGreen();
               sum = 0;
               count = 0;
               if(c1 != 0){
                  sum = sum + c1;
                  count = count + 1;
               }
               if(c2 != 0){
                  sum = sum + c2;
                  count = count + 1;
               }
               if(c3 != 0){
                  sum = sum + c3;
                  count = count + 1;
               }
               if(c4 != 0){
                  sum = sum + c4;
                  count = count + 1;
               }
               if(c5 != 0){
                  sum = sum + c5;
                  count = count + 1;
               }
               if(c6 != 0){
                  sum = sum + c6;
                  count = count + 1;
               }
               if(count != 0){
                  currentGreen= sum/count;
               }
               if(count == 0){
                  currentGreen = 0;
                }
               c1 =  interpolation_mask[i-1][j].getBlue();
               c2 =  interpolation_mask[i+1][j].getBlue();
               c3 =  interpolation_mask[i][j-1].getBlue();
               c4 =  interpolation_mask[i][j+1].getBlue();
               c5 =  interpolation_mask[i-1][j-1].getBlue();
               c6 =  interpolation_mask[i+1][j+1].getBlue();
               sum = 0;
               count = 0;
               if(c1 != 0){
                  sum = sum + c1;
                  count = count + 1;
               }
               if(c2 != 0){
                  sum = sum + c2;
                  count = count + 1;
               }
               if(c3 != 0){
                  sum = sum + c3;
                  count = count + 1;
               }
               if(c4 != 0){
                  sum = sum + c4;
                  count = count + 1;
               }
               if(c5 != 0){
                  sum = sum + c5;
                  count = count + 1;
               }
               if(c6 != 0){
                  sum = sum + c6;
                  count = count + 1;
               }
               if(count != 0){
                  currentBlue= sum/count;
               }
               if(count == 0){
                  currentBlue = 0;
                }
               overlay[i][j] = new Color(currentRed,currentGreen,currentBlue);
            }
        }
    }
    }catch (Exception e) {
      System.out.println("Exception"+e.toString());
    }
 }
/*=============================================================================================
/    startImageMonitor()
/=============================================================================================*/
public int[][] constructMask(int diameter,int center_x,int center_y){
  mask = new int[height][width];
  double r;
  double x;
  double y;
  double RP = (diameter/2.0)+0.5;
  double RM = (diameter/2.0)+0.5;
      for(int i=0;i<=height-1;i++){
          for(int j=0;j<=width-1;j++){
             x = (i-center_x)*(i-center_x);
             y = (j-center_y)*(j-center_y);
             r = Math.sqrt(x+y);
             if(r > RP){
               mask[i][j] = 0;
             }
             if(r< RP){
               mask[i][j] = 1;
             }
          }
      }
  return mask;
}
/*=============================================================================================
/    startImageMonitor()
/=============================================================================================*/
public int[][] constructOverlayMask(int diameter,int center_x,int center_y){
  overlay_mask = new int[overlay_y][overlay_x];
  double r;
  double x;
  double y;
  double RP = (diameter/2.0)+0.5;
  double RM = (diameter/2.0)+0.5;
      for(int i=0;i<=overlay_y-1;i++){
          for(int j=0;j<=overlay_x-1;j++){
             x = (i-center_x)*(i-center_x);
             y = (j-center_y)*(j-center_y);
             r = Math.sqrt(x+y);
             if(r > RP){
               overlay_mask[i][j] = 0;
             }
             if(r< RP){
               overlay_mask[i][j] = 1;
             }
          }
      }
  return overlay_mask;
}
/*=============================================================================================
/   readFits(java.lang.String file_name)
/=============================================================================================*/
public byte[][] readFits(java.lang.String file_name){
    byte[][] myTargetImage = new byte[480][640];
    try{
      nom.tam.fits.Fits myFits = new nom.tam.fits.Fits(file_name);
       ImageData  myTargetImageData      = (ImageData)(myFits.getHDU(0).getData());
       int bitPixTarget       = myFits.getHDU(0).getBitPix();
       int[] targetAxises     = myFits.getHDU(0).getAxes();
           myTargetImage    = new byte[targetAxises[0]][targetAxises[1]];
           myTargetImage            = (byte[][])myTargetImageData.getData();

      System.out.println("");
    }catch(Exception e){
        System.out.println("Error reading fits file. "+file_name);
    }
   return myTargetImage;
}
/*=============================================================================================
/    readImage()
/=============================================================================================*/
public void readImage(){
    java.awt.image.BufferedImage image;

   try{
//      java.net.URL myURL = new java.net.URL("http", "www.palomar.caltech.edu", 8000,"/images/allsky/AllSkyCurrentImage.JPG");
      java.net.URL myURL = new java.net.URL("http", ALLSKY_HOST, ALLSKY_PORT,ALLSKY_PATH+ALLSKY_IMAGE_NAME);
      image = javax.imageio.ImageIO.read(myURL);
///      image = javax.imageio.ImageIO.read(new java.io.File("/home/developer/AllSkyImageCalib.jpg"));
//      javax.media.jai.PlanarImage myPlanarImage = javax.media.jai.PlanarImage.wrapRenderedImage(image);
//      jsky.image.ImageProcessor myImageProcessor = new jsky.image.ImageProcessor(myPlanarImage, null);
      java.awt.image.Raster myRaster =   image.getData();
      minX = myRaster.getMinX();
      minY = myRaster.getMinY();
      height = myRaster.getHeight();
      width = myRaster.getWidth();
      bands = myRaster.getNumBands();
      int[] pixel = new int[bands];
      red = new int[height][width];
      green = new int[height][width];
      blue  = new int[height][width];
//      int [][] mask = constructMask(600,240,320);
      for(int i=minY;i<=height-1;i++){
          for(int j=minX;j<=width-1;j++){
              pixel = myRaster.getPixel(j, i, pixel);
              red[i][j]   = (pixel[0])*mask[i][j];
              green[i][j] = (pixel[1])*mask[i][j];
              blue[i][j]  = (pixel[2])*mask[i][j];
          }
      }
//      boolean calibration_image = false;
//      if(calibration_image){
//      byte[][]  data = readFits("/home/developer/AllSkyThreshold2.fits");
//          for(int i=0;i<=height-1;i++){
//              for(int j=0;j<=width-1;j++){
//                  red[479-i][j]   = (int)data[i][j];
//                  green[479-i][j] = (int)data[i][j];
//                  blue[479-i][j]  = (int)data[i][j];
//              }
//          }
//       }
      constructOverlay();
      interpolate();
      applyMask();
      mySkyDisplaySimple.setOverlay(overlay);
    }catch(Exception e){
      logMessage(e.toString());
    }
}
/*=========================================================================================================
/     imageSave()
/=========================================================================================================*/
public void imageSave(){
   java.awt.image.BufferedImage img = new java.awt.image.BufferedImage(mySkyDisplaySimple.getWidth(),mySkyDisplaySimple.getHeight(),java.awt.image.BufferedImage.TYPE_INT_RGB);
   mySkyDisplaySimple.paint(img.getGraphics());
   try{
     javax.imageio.ImageIO.write(img,"png", new java.io.File("/rdata/allsky.png"));
     System.out.println("Saved image to /rdata/allsky.png");
   }catch(Exception e){
     System.out.println("Error saving allsky image to file."+e.toString());       
   }
}
/*=========================================================================================================
/     void logMessage(java.lang.String newMessage)
/=========================================================================================================*/
 public void logMessage(java.lang.String newMessage){
     System.out.println(newMessage);
 }
 /*=============================================================================================
/       INNER CLASS that sends a set of coordinates to the telescope
/=============================================================================================*/
    public class MonitorAllSkyImageThread  implements Runnable{
    private Thread myThread;
    private boolean running;
    private long    polling_time;
 /*=============================================================================================
/          MonitorAllSkyImageThread()
/=============================================================================================*/
    private MonitorAllSkyImageThread(){
    }
/*================================================================================================
/      waitForResponse(int newDelay)
/=================================================================================================*/
public void retrieveWait(long newDelay){
     try{
        Thread.currentThread().sleep(newDelay);} // sleep for awhile to let the messages pile up
      catch (Exception e){
    }
}
/*=============================================================================================
/        setRunning(boolean new_running)
/=============================================================================================*/
public void setRunning(boolean new_running){
    running = new_running;
}
public boolean isRunning(){
    return running;
}
/*=============================================================================================
/       setPollingTime(long new_polling_time)
/=============================================================================================*/
public void setPollingTime(long new_polling_time){
    polling_time = new_polling_time;
}
/*=============================================================================================
/           run()
/=============================================================================================*/
    public void run(){
       int image_number = 0;
       logMessage("Starting AllSky Image Monitor");
       setRunning(true);
       while(running){
         readImage();
         logMessage("Retrieved Image Number ="+image_number);
         image_number = image_number +1;
         retrieveWait(polling_time);
         imageSave();
       }
     }
/*=============================================================================================
/         start()
/=============================================================================================*/
    public void start(){
     myThread = new Thread(this);
     myThread.start();
     }
    }// End of the MonitorAllSkyImageThread Inner Class
/*=============================================================================================
/      End of the MonitorAllSkyImageThread Class
/=============================================================================================*/

 /*================================================================================================
/         Main Method - Starts the entire system
/=================================================================================================*/
  //Main method
   public static void main(String[] args) {
     try {
//           UIManager.setLookAndFeel("javax.swing.plaf.metal.MetalLookAndFeel");
     }
     catch(Exception e) {
        e.printStackTrace();
     }
     new AllSkyImageProcessor();
   }
/*=============================================================================================
/      AllSkyImageProcessor constructor
/=============================================================================================*/

}
