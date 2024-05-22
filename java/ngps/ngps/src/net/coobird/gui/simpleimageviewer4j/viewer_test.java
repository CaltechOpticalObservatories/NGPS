/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package net.coobird.gui.simpleimageviewer4j;
import java.awt.*;
import java.awt.event.*;
import java.awt.image.*;
import java.io.*;
import javax.imageio.*;
import javax.swing.*;
import net.coobird.gui.simpleimageviewer4j.Viewer;
 
/**
 *
 * @author jennifermilburn
 */
public class viewer_test {
    public Viewer myViewer; 
    public viewer_test(){
     display_timeline_image();   
    }
    public void display_timeline_image(){
       java.lang.String OTM_INSTALL_DIR    = "/Users/jennifermilburn/Desktop/NGPS/python/git/OTM/";
       java.lang.String IMAGE_NAME = "timeline.png";
       BufferedImage img = null;
      try {
          img = javax.imageio.ImageIO.read(new File(OTM_INSTALL_DIR+IMAGE_NAME));
          new Viewer(img).show();
      }catch (IOException e) {
      }       
    }
/*================================================================================================
/      add and remove Property Change Listeners
/=================================================================================================*/
      public static void main(String args[]) {
        /* Set the Nimbus look and feel */
             try {
                 UIManager.setLookAndFeel(UIManager.getCrossPlatformLookAndFeelClassName());
                 java.lang.String test = UIManager.getSystemLookAndFeelClassName();
                 System.out.println(test);
               } catch(Exception e) {
                   e.printStackTrace();
               }
               new viewer_test();
    }    
}