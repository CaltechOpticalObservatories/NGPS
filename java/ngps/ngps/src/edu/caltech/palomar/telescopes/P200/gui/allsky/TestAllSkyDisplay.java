/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package edu.caltech.palomar.telescopes.P200.gui.allsky;

import java.awt.Frame;
import java.awt.Graphics;
import java.awt.Color;
/**
 *
 * @author developer
 */
public class TestAllSkyDisplay extends Frame{
   int[][]                       red;
   int[][]                       green;
   int[][]                       blue;
   int minX ;
   int minY;
   int height;
   int width;
   int bands;

     public TestAllSkyDisplay(){
        readImage();
        init();
     }
     void init(){
         setLayout(null);
         setSize(640,480);
         setVisible(true);
     }
private void readImage(){
    java.awt.image.BufferedImage image;
   try{
      java.net.URL myURL = new java.net.URL("http", "www.palomar.caltech.edu", 8000,"/images/allsky/AllSkyCurrentImage.JPG");
      image = javax.imageio.ImageIO.read(myURL);
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
      for(int i=minY;i<=height-1;i++){
          for(int j=minX;j<=width-1;j++){
              pixel = myRaster.getPixel(j, i, pixel);
              red[i][j]   = pixel[0];
              green[i][j] = pixel[1];
              blue[i][j]  = pixel[2];
//              System.out.println("i ="+i+" j = "+j);
          }
      }
    }catch(Exception e){
      logMessage(e.toString());
    }
}
public void paint(Graphics g){
    for(int i=minY;i<height-1;i++){
        for(int j=minX;j<width-1;j++){
            Color col = new Color(red[i][j],green[i][j],blue[i][j]);
            g.setColor(col);
            g.fillRect(j, i, 1, 1);
        }
    }
}
/*=========================================================================================================
/     void logMessage(java.lang.String newMessage)
/=========================================================================================================*/
 public void logMessage(java.lang.String newMessage){
     System.out.println(newMessage);
 }
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
     new TestAllSkyDisplay();
   }
}
