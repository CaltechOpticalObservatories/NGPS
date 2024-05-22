/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package edu.caltech.palomar.instruments.ngps.test_run;
import javax.swing.JPanel;
import java.awt.BorderLayout;
import java.awt.FlowLayout;
import javax.swing.BorderFactory;
import javax.swing.JCheckBox;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import javax.swing.JFrame;
import chrriis.dj.nativeswing.swtimpl.components.JWebBrowser;

public class SimpleWebBrowserExample extends JPanel{

public SimpleWebBrowserExample() {
   super(new BorderLayout());
   JPanel webBrowserPanel = new JPanel(new BorderLayout());
   webBrowserPanel.setBorder(BorderFactory.createTitledBorder("Native Web Browser component"));
   final JWebBrowser webBrowser = new JWebBrowser();
   //    webBrowser.setStatusBarVisible(false);
   webBrowser.navigate("http://www.google.com");
   webBrowserPanel.add(webBrowser, BorderLayout.CENTER);
   add(webBrowserPanel, BorderLayout.CENTER);
   // Create an additional bar allowing to show/hide the menu bar of the web browser.
   JPanel buttonPanel = new JPanel(new FlowLayout(FlowLayout.CENTER, 4, 4));
   JCheckBox menuBarCheckBox = new JCheckBox("Menu Bar", webBrowser.isMenuBarVisible());
   menuBarCheckBox.addItemListener(
       new ItemListener() {
         public void itemStateChanged(ItemEvent e) {
           webBrowser.setMenuBarVisible(e.getStateChange() == ItemEvent.SELECTED);
         }
       });
   buttonPanel.add(menuBarCheckBox);
   add(buttonPanel, BorderLayout.SOUTH);
 }
}