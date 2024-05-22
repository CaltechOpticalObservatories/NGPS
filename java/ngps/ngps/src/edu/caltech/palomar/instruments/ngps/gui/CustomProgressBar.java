/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package edu.caltech.palomar.instruments.ngps.gui;
import javax.swing.JProgressBar;
import javax.swing.plaf.basic.BasicProgressBarUI;
import java.awt.Graphics;
import java.awt.Insets;
import java.awt.Color;
/**
 *
 * @author developer
 */
public class CustomProgressBar extends JProgressBar{
   private Color color_string = new Color(200,200,200);
    public CustomProgressBar(){
        setStringPainted(true);
        setForeground(new Color(255,255,255));
        setBackground(new Color(69,124,235));
        setUI(new BasicProgressBarUI(){
            @Override
            protected void paintString(Graphics g,int x,int y,int width,int height,int amountFull,Insets b){
                g.setColor(getColorstring());
                super.paintString(g, x, y, width, height, amountFull, b);
            }
        });        
    }
public void setColorstring(Color newColorstring){
  color_string = newColorstring;  
}
public Color getColorstring(){
    return color_string;
}
}
