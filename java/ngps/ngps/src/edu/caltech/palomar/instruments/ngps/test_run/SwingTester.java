/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package edu.caltech.palomar.instruments.ngps.test_run;

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.LayoutManager;
import java.io.IOException;
import java.net.URL;
import java.io.File;

import javax.swing.JEditorPane;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JScrollPane;

import edu.caltech.palomar.instruments.ngps.test_run.SimpleWebBrowserExample;
import chrriis.dj.nativeswing.swtimpl.NativeInterface;
public class SwingTester {
   public java.lang.String USERDIR           = System.getProperty("user.dir");
   public java.lang.String SEP               = System.getProperty("file.separator");
   public SimpleWebBrowserExample            mySimpleWebBrowserExample;
   public SwingTester(){
       createWindow();
   }
   public static void main(String[] args) {
      NativeInterface.initialize();
      new SwingTester();
   }

   private void createWindow() {    
      JFrame frame = new JFrame("Swing Tester");
      frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
      createUI(frame);
      frame.setSize(560, 450);      
      frame.setLocationRelativeTo(null);  
      frame.setVisible(true);
   }

   private void createUI(final JFrame frame){  
      JPanel panel = new JPanel();
      LayoutManager layout = new FlowLayout();  
      panel.setLayout(layout);       

      mySimpleWebBrowserExample = new SimpleWebBrowserExample();
      
      
//      JEditorPane jEditorPane = new JEditorPane();
//      jEditorPane.setEditable(false);   
      java.lang.String PLOT_OUTPUT_FILE    = USERDIR+SEP+"config"+SEP+"otm"+SEP+"PLOT.html";
//      URL url= SwingTester.class.getResource(PLOT_OUTPUT_FILE);
//
      try {   
         URL url = new File(PLOT_OUTPUT_FILE).toURI().toURL();
//         jEditorPane.setPage(url);
      } catch (IOException e) { 
//         jEditorPane.setContentType("text/html");
//         jEditorPane.setText("<html>Page not found.</html>");
      }

 //     JScrollPane jScrollPane = new JScrollPane(jEditorPane);
 //     jScrollPane.setPreferredSize(new Dimension(540,400));      

      panel.add(mySimpleWebBrowserExample);
      frame.getContentPane().add(panel, BorderLayout.CENTER);    
   }  
} 
