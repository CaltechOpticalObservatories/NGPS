package edu.caltech.palomar.telescopes;
//=== File Prolog =============================================================
//	This code was developed by California Institute of Technology
//      Caltech Optical Observatories, Palomar Observatory
//
//--- Contents ----------------------------------------------------------------
//	 P200FactSum- This is a main class for the Palomar 200" fact summary
//
//--- Description -------------------------------------------------------------
// Modification History
//
// Modified to include information on the selected object from either
// the AstroObjectDisplayFrame or the SkyDisplayFrame.
// Uses the AstroObjectsModel to update a new panel showing the basic
// information on the selected target (name, RA and dec).  Includes
// a new button to load the results to the telescope.
// November 29,2010  Jennifer Milburn
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
//
//      August 8, 2010 Jennifer Milburn
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
import edu.caltech.palomar.telescopes.P200.TelescopesIniReader;
import edu.caltech.palomar.io.ClientSocket; 
import edu.caltech.palomar.telescopes.P200.P200Component;
import edu.caltech.palomar.telescopes.P200.gui.FactSumForm;
import edu.caltech.palomar.telescopes.P200.gui.EphemerisFrame;
import edu.caltech.palomar.telescopes.P200.gui.SkyDisplayFrame;
import javax.swing.UIManager;
import java.awt.*;
import javax.swing.*;
//import edu.caltech.palomar.util.gui.SplashScreen;
import java.awt.event.*;
import edu.caltech.palomar.telescopes.P200.gui.AstroObjectDisplayFrame;
import edu.dartmouth.jskycalc.objects.NightlyAlmanac;
import edu.dartmouth.jskycalc.gui.NightlyWindow;
import edu.dartmouth.jskycalc.objects.WhenWhere;
import edu.caltech.palomar.telescopes.P200.gui.AirmassFrame;
import edu.caltech.palomar.telescopes.P200.gui.TelescopeControlPanel;
import edu.caltech.palomar.telescopes.guider.catalog.tycho2.TychoCatalogFrame;
import edu.caltech.palomar.telescopes.P200.gui.allsky.DateTimeChooserFrame;
import edu.caltech.palomar.telescopes.P200.gui.allsky.AllSkyCalibrationFrame;
/*=============================================================================================
/         P200FactSum Class Declaration
/=============================================================================================*/
public class P200FactSum {
   boolean                        packFrame               = false;
   ClientSocket                   myClientSocket;
   TelescopesIniReader            myTelescopesIniReader;
   public static int              DEFAULT_SERVERPORT      = 5004;
   P200Component                  myP200Component;
   FactSumForm                    myP200DesignerForm;
   EphemerisFrame                 myEphemerisFrame;
   SkyDisplayFrame                mySkyDisplayFrame;
   AstroObjectDisplayFrame        myAstroObjectDisplayFrame;
   NightlyAlmanac                 Nightly;
   TelescopeControlPanel          myTelescopeControlPanel;
//   WhenWhere                      ww;
   AirmassFrame                   myAirmassFrame;
   public   JFrame                mainFrame;
   public   JDesktopPane          desktop;
   private  java.lang.String      TERMINATOR        = new java.lang.String("\n");
   public   java.lang.String      USERDIR           = System.getProperty("user.dir");
   public   java.lang.String      SEP               = System.getProperty("file.separator");
   public   java.lang.String      IMAGE_CACHE       = new java.lang.String(SEP + "images" + SEP);
   public   java.lang.String      CONFIG            = new java.lang.String(SEP + "config" + SEP);
   private  Dimension             mainframeSize     = new Dimension();
   private  Dimension             screenSize        = new Dimension();
   private  JToolBar              toolBar           = new JToolBar();
   private  JToggleButton         telescopePositionToggleButton      = new JToggleButton();
   private  JToggleButton         ephemerisToggleButton              = new JToggleButton();
   private  JToggleButton         skyDisplayToggleButton             = new JToggleButton();
   private  JToggleButton         astroObjectsToggleButton           = new JToggleButton();
   private  JToggleButton         airmassToggleButton                = new JToggleButton();
   private  JToggleButton         telescopeControlToggleButton       = new JToggleButton();
   private  JToggleButton         spectralTypeToggleButton           = new JToggleButton();
   private  JToggleButton         datetimechooserToggleButton        = new JToggleButton();
   private  JToggleButton         nightlyAlmanacToggleButton         = new JToggleButton();


   private  Dimension             buttonSize                         = new Dimension(24,24);
   private  ImageIcon             telescopePositionImageIcon;
   private  ImageIcon             ephemerisImageIcon;
   private  ImageIcon             skyDisplayImageIcon;
   private  ImageIcon             astroObjectImageIcon;
   private  ImageIcon             airmassImageIcon;
   private  ImageIcon             telescopeControlImageIcon;
   private  ImageIcon             spectralTypeImageIcon;
   private  ImageIcon             datetimechooserImageIcon;
   private  ImageIcon             nightlyAlmanacImageIcon;

   public   JMenuBar              menuBar                            = new JMenuBar();

   private  JToggleButton         planetsToggleButton      = new JToggleButton();
   private  JToggleButton         sunToggleButton          = new JToggleButton();
   private  JToggleButton         moonToggleButton         = new JToggleButton();
   private  JToggleButton         targetsToggleButton      = new JToggleButton();
   private  JToggleButton         airmassGridToggleButton  = new JToggleButton();
   private  JToggleButton         haToggleButton           = new JToggleButton();
   private  JToggleButton         brightStarToggleButton   = new JToggleButton();
   private  JToggleButton         allSkyToggleButton       = new JToggleButton();


   private  ImageIcon             planetsImageIcon;
   private  ImageIcon             sunImageIcon;
   private  ImageIcon             moonImageIcon;
   private  ImageIcon             targetsImageIcon;
   private  ImageIcon             airmassGridImageIcon;
   private  ImageIcon             haImageIcon;
   private  ImageIcon             brightStarImageIcon;
   private  ImageIcon             allSkyImageIcon;

   private  JToolBar              toolBarVis           = new JToolBar();

   private javax.swing.JMenuBar       jMenuBar1;
   private javax.swing.JMenu          FileMenu;
   private javax.swing.JMenuItem      OpenMenuItem;
   private javax.swing.JMenuItem      ClosMenuItem;
   private TychoCatalogFrame          myTychoCatalogFrame;
   private DateTimeChooserFrame       myDateTimeChooserFrame;
//   private AllSkyCalibrationFrame     myAllSkyCalibrationFrame;
   private NightlyWindow              myNightlyWindow;
/*=============================================================================================
/         P200FactSum() constructor
/=============================================================================================*/
      public P200FactSum(){
          System.out.println(USERDIR);
          jbInit();
      }
/*=============================================================================================
/         jbInit()
/=============================================================================================*/
   private void jbInit(){
//     mainFrame          = new JFrame();
//     desktop            = new JDesktopPane();
//     initMainFrame();
     myP200DesignerForm = new FactSumForm(); 
     initTelescopeMonitorFrame();
     initializeToolBar();
     myP200Component    = new P200Component();
     myTychoCatalogFrame = new TychoCatalogFrame();
     myTychoCatalogFrame.setP200Component(myP200Component);
     myEphemerisFrame   = new EphemerisFrame();
     mySkyDisplayFrame  = new SkyDisplayFrame();
     myP200DesignerForm.initializeForm(myP200Component);
     myDateTimeChooserFrame = new DateTimeChooserFrame();
     myDateTimeChooserFrame.setVisible(false);
     myDateTimeChooserFrame.setJSkyCalcModelTelescope(myP200Component.getTelescopeObject().getJSkyCalcModel());
     myEphemerisFrame.myEphemerisPanel.setJSkyCalcModel(myP200Component.getTelescopeObject().getJSkyCalcModel());
     myP200Component.getTelescopeObject().initialJSkyCalcModel();
     mySkyDisplayFrame.setJSkyCalcModel(myP200Component.getTelescopeObject().getJSkyCalcModel());
     myAstroObjectDisplayFrame = new AstroObjectDisplayFrame();
     myAstroObjectDisplayFrame.setP200Component(myP200Component);
     myDateTimeChooserFrame.setJSkyCalcModelObjects(myAstroObjectDisplayFrame.getAstroObjectModel().getJSkyCalcModel());
     mySkyDisplayFrame.getSkyDisplay().setAstroObjectsModel(myAstroObjectDisplayFrame.getAstroObjectModel());
     mySkyDisplayFrame.getSkyDisplay().setJSkyCalcModel(myAstroObjectDisplayFrame.myJSkyCalcModel);
     Nightly = new NightlyAlmanac(myAstroObjectDisplayFrame.myJSkyCalcModel.getWhenWhere());
     myNightlyWindow = new NightlyWindow(Nightly);
//     myNightlyWindow.setVisible(true);
     myNightlyWindow.UpdateDisplay();
     myAirmassFrame = new AirmassFrame();
     myAirmassFrame.setModels( Nightly, myAstroObjectDisplayFrame.getAstroObjectModel(),myAstroObjectDisplayFrame.myJSkyCalcModel);
     myAirmassFrame.setVisible(false);
     myAirmassFrame.executeUpdate();

     myTelescopeControlPanel = new TelescopeControlPanel();
     myTelescopeControlPanel.initializeForm(myP200Component);
     myTelescopeControlPanel.setVisible(false);

     // Add the AstroObjectsModel to the FactSumForm()
     myP200DesignerForm.setAstroObjectsModel(myAstroObjectDisplayFrame.getAstroObjectModel());
     myTychoCatalogFrame.getTycho2Reader().executeReadIndex();
     initializeVisibilityToolBar();
//     myAllSkyCalibrationFrame = new AllSkyCalibrationFrame();
//     myAllSkyCalibrationFrame.setAllSkyImageProcessor(mySkyDisplayFrame.getSkyDisplay().getAllSkyImageProcessor());
//     myAllSkyCalibrationFrame.setVisible(false);
//     myAstroObjectDisplayFrame.getAstroObjectModel().setAirmassDisplay(myAirmassFrame.getAirmassDisplay());
//  Add the internal frames to the rootpane of the desktop
//     desktop.getRootPane().getContentPane().add(myP200DesignerForm);
//     desktop.getRootPane().getContentPane().add(myEphemerisFrame);
//     desktop.getRootPane().getContentPane().add(mySkyDisplayFrame);
//    if (packFrame) {
//      mainFrame.pack();
//    }
//    else {
//      mainFrame.validate();
//    }
//    mainFrame.setSize(1500, 1100);
    //Center the window
//    Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
//    Dimension frameSize = myP200DesignerForm.getSize();
//    if (frameSize.height > screenSize.height) {
//      frameSize.height = screenSize.height;
//    }
//    if (frameSize.width > screenSize.width) {
//      frameSize.width = screenSize.width;
//    }
//    mainFrame.setLocation((screenSize.width - frameSize.width) / 2, (screenSize.height - frameSize.height) / 2);
    myP200DesignerForm.setSize(935, 755);
    myP200DesignerForm.setVisible(true);
    myEphemerisFrame.setVisible(false);
    mySkyDisplayFrame.setVisible(false);
    positionFrames();
 //   mainFrame.setVisible(true);
   }
/*================================================================================================
/         OpenMenuItemActionPerformed(java.awt.event.ActionEvent evt)
/=================================================================================================*/
 public void positionFrames(){
    myP200DesignerForm.positionFrame();
    mySkyDisplayFrame.positionFrame();
    myAirmassFrame.positionFrame();
    myAstroObjectDisplayFrame .positionFrame();
 }
/*=============================================================================================
/      MainFrame Initialization -
/=============================================================================================*/
  public void initMainFrame(){
 //   mainFrame.setContentPane(desktop);
 //   mainFrame.setTitle("Palomar Observatory Hale 200' FACTSUM Display");
 //   mainFrame.setSize(screenSize);
 //   mainFrame.validate();
 //   mainframeSize = mainFrame.getSize();
  }
/*=============================================================================================
/      MainFrame Initialization -
/=============================================================================================*/
  public void initTelescopeMonitorFrame(){
//    mainFrame.setContentPane(desktop);
    myP200DesignerForm.setTitle("Palomar Observatory Hale 200' FACSUM Display");
    myP200DesignerForm.setSize(screenSize);
    myP200DesignerForm.validate();
    mainframeSize = myP200DesignerForm.getSize();
  }
/*=============================================================================================
/      initializeNightlyAlmanac()
/=============================================================================================*/
  public void initializeNightlyAlmanac(){
    nightlyAlmanacToggleButton.setSize(        buttonSize);
    nightlyAlmanacImageIcon               = new ImageIcon(USERDIR + IMAGE_CACHE + "TaskStatusOn.gif");
    nightlyAlmanacToggleButton.setBorderPainted(false);
    nightlyAlmanacToggleButton.setIcon(  nightlyAlmanacImageIcon);
    nightlyAlmanacToggleButton.setSelectedIcon(nightlyAlmanacImageIcon);
    nightlyAlmanacToggleButton.setToolTipText("Nightly Circumstances Control");
     toolBar.add(nightlyAlmanacToggleButton,      null);
      nightlyAlmanacToggleButton.addActionListener(new ActionListener()  {
      public void actionPerformed(ActionEvent e) {
        nightlyAlmanacToggleButton_actionPerformed(e);
      }
      });
    }
/*=============================================================================================
/      MainFrame Initialization -
/=============================================================================================*/
  public void initializeToolBar(){
     constructMenuBar();
     menuBar.add(toolBar);
     myP200DesignerForm.setJMenuBar(menuBar);
     menuBar.setBorderPainted(false);
     menuBar.setSize(200, 30);
     menuBar.setBackground(Color.black);
     menuBar.setForeground(Color.black);
     menuBar.setMargin(null);
     telescopePositionToggleButton.setSize(   buttonSize);
     ephemerisToggleButton.setSize(           buttonSize);
     skyDisplayToggleButton.setSize(          buttonSize);
     astroObjectsToggleButton.setSize(        buttonSize);
     airmassToggleButton.setSize(             buttonSize);
     telescopeControlToggleButton.setSize(    buttonSize);
     spectralTypeToggleButton.setSize(        buttonSize);
     datetimechooserToggleButton.setSize(        buttonSize);

     telescopePositionImageIcon             = new ImageIcon(USERDIR + IMAGE_CACHE + "HaleTelescope2.gif");
     ephemerisImageIcon                     = new ImageIcon(USERDIR + IMAGE_CACHE + "Ephemeris2.gif");
     skyDisplayImageIcon                    = new ImageIcon(USERDIR + IMAGE_CACHE + "SkyDisplay2.jpg");
     astroObjectImageIcon                   = new ImageIcon(USERDIR + IMAGE_CACHE + "ping.gif");
     airmassImageIcon                       = new ImageIcon(USERDIR + IMAGE_CACHE + "lace.gif");
     telescopeControlImageIcon              = new ImageIcon(USERDIR + IMAGE_CACHE + "TelescopeControl.gif");
     spectralTypeImageIcon                  = new ImageIcon(USERDIR + IMAGE_CACHE + "STYPE.gif");
     datetimechooserImageIcon               = new ImageIcon(USERDIR + IMAGE_CACHE + "JCalendarColor32.gif");

     telescopePositionToggleButton.setBorderPainted(false);
     skyDisplayToggleButton.setBorderPainted(false);
     ephemerisToggleButton.setBorderPainted(false);
     astroObjectsToggleButton.setBorderPainted(false);
     airmassToggleButton.setBorderPainted(false);
     telescopeControlToggleButton.setBorderPainted(false);
     spectralTypeToggleButton.setBorderPainted(false);
     datetimechooserToggleButton.setBorderPainted(false);

     telescopePositionToggleButton.setIcon(telescopePositionImageIcon);
     ephemerisToggleButton.setIcon(        ephemerisImageIcon);
     skyDisplayToggleButton.setIcon(       skyDisplayImageIcon);
     astroObjectsToggleButton.setIcon(     astroObjectImageIcon);
     airmassToggleButton.setIcon(          airmassImageIcon);
     telescopeControlToggleButton.setIcon( telescopeControlImageIcon);
     spectralTypeToggleButton.setIcon(     spectralTypeImageIcon);
     datetimechooserToggleButton.setIcon(  datetimechooserImageIcon);

     telescopePositionToggleButton.setToolTipText("Hale 200'' Telescope Position");
     ephemerisToggleButton.setToolTipText(        "Ephemeris Display");
     skyDisplayToggleButton.setToolTipText(       "Sky Display");
     astroObjectsToggleButton.setToolTipText(     "Astronomical Target List");
     airmassToggleButton.setToolTipText(          "Airmass graphs");
     telescopeControlToggleButton.setToolTipText( "Telescope Controls");
     spectralTypeToggleButton.setToolTipText(     "Spectral Type Catalog Search Tool");
     datetimechooserToggleButton.setToolTipText("Ephemeris Calcuation Time Control");
//     toolBar.add(telescopePositionToggleButton,    null);
     toolBar.add(ephemerisToggleButton,            null);
     toolBar.add(skyDisplayToggleButton,           null);
     toolBar.add(astroObjectsToggleButton,         null);
     toolBar.add(airmassToggleButton,              null);
     toolBar.add(telescopeControlToggleButton,     null);
     toolBar.add(spectralTypeToggleButton,         null);
     toolBar.add(datetimechooserToggleButton,      null);

     toolBar.setVisible(true);
// Action Listeners for the toolbar toggle buttons
     telescopePositionToggleButton.addActionListener(new ActionListener()  {
      public void actionPerformed(ActionEvent e) {
        TelescopePosition_actionPerformed(e);
      }
      });
     ephemerisToggleButton.addActionListener(new ActionListener()  {
      public void actionPerformed(ActionEvent e) {
        Ephemeris_actionPerformed(e);
      }
      });
     skyDisplayToggleButton.addActionListener(new ActionListener()  {
      public void actionPerformed(ActionEvent e) {
        SkyDisplay_actionPerformed(e);
      }
      });
      astroObjectsToggleButton.addActionListener(new ActionListener()  {
      public void actionPerformed(ActionEvent e) {
        AstroObjects_actionPerformed(e);
      }
      });
      airmassToggleButton.addActionListener(new ActionListener()  {
      public void actionPerformed(ActionEvent e) {
        Airmass_actionPerformed(e);
      }
      });
      telescopeControlToggleButton.addActionListener(new ActionListener()  {
      public void actionPerformed(ActionEvent e) {
        TelescopeControl_actionPerformed(e);
      }
      });
      spectralTypeToggleButton.addActionListener(new ActionListener()  {
      public void actionPerformed(ActionEvent e) {
        spectralTypeToggleButton_actionPerformed(e);
      }
      });
      datetimechooserToggleButton.addActionListener(new ActionListener()  {
      public void actionPerformed(ActionEvent e) {
        datetimechooserToggleButton_actionPerformed(e);
      }
      });
      initializeNightlyAlmanac();
  }
/*=============================================================================================
/     Visibility Toolbar Initialization -
/=============================================================================================*/
  public void initializeVisibilityToolBar(){
     menuBar.add(toolBarVis);
     planetsToggleButton.setSize(       buttonSize);
     sunToggleButton.setSize(           buttonSize);
     moonToggleButton.setSize(          buttonSize);
     targetsToggleButton.setSize(       buttonSize);
     airmassGridToggleButton.setSize(   buttonSize);
     haToggleButton.setSize(            buttonSize);
     brightStarToggleButton.setSize(            buttonSize);
     allSkyToggleButton.setSize(            buttonSize);
     
     planetsImageIcon             = new ImageIcon(USERDIR + IMAGE_CACHE + "Planets2.gif");
     sunImageIcon                 = new ImageIcon(USERDIR + IMAGE_CACHE + "Sun2.gif");
     moonImageIcon                = new ImageIcon(USERDIR + IMAGE_CACHE + "Moon2.gif");
     targetsImageIcon             = new ImageIcon(USERDIR + IMAGE_CACHE + "DisplayTargets.gif");
     airmassImageIcon             = new ImageIcon(USERDIR + IMAGE_CACHE + "AirmassGrid2.gif");
     haImageIcon                  = new ImageIcon(USERDIR + IMAGE_CACHE + "HAGrid.gif");
     brightStarImageIcon          = new ImageIcon(USERDIR + IMAGE_CACHE + "HAGrid.gif");
     allSkyImageIcon              = new ImageIcon(USERDIR + IMAGE_CACHE + "AllSky.jpg");

     planetsToggleButton.setBorderPainted(false);
     sunToggleButton.setBorderPainted(false);
     moonToggleButton.setBorderPainted(false);
     targetsToggleButton.setBorderPainted(false);
     airmassGridToggleButton.setBorderPainted(false);
     haToggleButton.setBorderPainted(false);
     brightStarToggleButton.setBorderPainted(false);
     allSkyToggleButton.setBorderPainted(false);
     planetsToggleButton.setIcon(    planetsImageIcon);
     sunToggleButton.setIcon(        sunImageIcon);
     moonToggleButton.setIcon(       moonImageIcon);
     targetsToggleButton.setIcon(    targetsImageIcon);
     airmassGridToggleButton.setIcon(airmassImageIcon);
     haToggleButton.setIcon(         haImageIcon);
     brightStarToggleButton.setIcon( brightStarImageIcon );
     allSkyToggleButton.setIcon(     allSkyImageIcon);

     planetsToggleButton.setSelected(true);
     sunToggleButton.setSelected(true);
     moonToggleButton.setSelected(true);
     targetsToggleButton.setSelected(true);
     airmassGridToggleButton.setSelected(true);
     haToggleButton.setSelected(true);
     brightStarToggleButton.setSelected(true);
     allSkyToggleButton.setSelected(true);

     planetsToggleButton.setToolTipText(    "Display Planets");
     sunToggleButton.setToolTipText(        "Display Sun");
     moonToggleButton.setToolTipText(       "Display Moon");
     targetsToggleButton.setToolTipText(    "Display Astronomical Target List");
     airmassGridToggleButton.setToolTipText(    "Display Airmass grid");
     haToggleButton.setToolTipText(         "Display Hour Angle grid");
     brightStarToggleButton.setToolTipText( "Display Bright Star Catalog");
     allSkyToggleButton.setToolTipText(     "Display All Sky Image");
     allSkyToggleButton.setSelected(false);

//     toolBar.add(telescopePositionToggleButton,    null);
     toolBarVis.add(haToggleButton,            null);
     toolBarVis.add(airmassGridToggleButton,   null);
     toolBarVis.add(targetsToggleButton,       null);
     toolBarVis.add(planetsToggleButton,       null);
     toolBarVis.add(sunToggleButton,           null);
     toolBarVis.add(moonToggleButton,          null);
     toolBarVis.add(brightStarToggleButton,    null);
     toolBarVis.add(allSkyToggleButton,        null);

     toolBarVis.setVisible(true);
// Action Listeners for the toolbar toggle buttons
      planetsToggleButton.addActionListener(new ActionListener()  {
      public void actionPerformed(ActionEvent e) {
        Planets_actionPerformed(e);
      }
      });
      sunToggleButton.addActionListener(new ActionListener()  {
      public void actionPerformed(ActionEvent e) {
        Sun_actionPerformed(e);
      }
      });
      moonToggleButton.addActionListener(new ActionListener()  {
      public void actionPerformed(ActionEvent e) {
        Moon_actionPerformed(e);
      }
      });
      targetsToggleButton.addActionListener(new ActionListener()  {
      public void actionPerformed(ActionEvent e) {
        Target_actionPerformed(e);
      }
      });
      airmassGridToggleButton.addActionListener(new ActionListener()  {
      public void actionPerformed(ActionEvent e) {
        AirmassGrid_actionPerformed(e);
      }
      });
      haToggleButton.addActionListener(new ActionListener()  {
      public void actionPerformed(ActionEvent e) {
        HAGrid_actionPerformed(e);
      }
      });
      brightStarToggleButton.addActionListener(new ActionListener()  {
      public void actionPerformed(ActionEvent e) {
        brightStar_actionPerformed(e);
      }
      });
      allSkyToggleButton.addActionListener(new ActionListener()  {
      public void actionPerformed(ActionEvent e) {
        allSky_actionPerformed(e);
      }
      });
  }
/*================================================================================================
/         constructMenuBar()
/=================================================================================================*/
 public void constructMenuBar(){
        FileMenu = new javax.swing.JMenu();
        OpenMenuItem = new javax.swing.JMenuItem();
        ClosMenuItem = new javax.swing.JMenuItem();
        FileMenu.setText("File");
        FileMenu.setForeground(Color.WHITE);
        FileMenu.setPopupMenuVisible(true);
        OpenMenuItem.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_O, java.awt.event.InputEvent.CTRL_MASK));
        OpenMenuItem.setText("Open");
        OpenMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                OpenMenuItemActionPerformed(evt);
            }
        });
        FileMenu.add(OpenMenuItem);

        ClosMenuItem.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_C, 0));
        ClosMenuItem.setText("Close");
        ClosMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                ClosMenuItemActionPerformed(evt);
            }
        });
        FileMenu.add(ClosMenuItem);

        menuBar.add(FileMenu);
  }
/*================================================================================================
/         OpenMenuItemActionPerformed(java.awt.event.ActionEvent evt)
/=================================================================================================*/
   private void OpenMenuItemActionPerformed(java.awt.event.ActionEvent evt) { 
       myAstroObjectDisplayFrame.getAstroObjectModel().Open();
    }
/*================================================================================================
/         ClosMenuItemActionPerformed(java.awt.event.ActionEvent evt)
/=================================================================================================*/
   private void ClosMenuItemActionPerformed(java.awt.event.ActionEvent evt) {
        // TODO add your handling code here:
       myAstroObjectDisplayFrame.getAstroObjectModel().Close();
    }
/*================================================================================================
/         Planets_ActionPerformed - Toolbar Button Action
/=================================================================================================*/
  public void Planets_actionPerformed(ActionEvent e){
      boolean isVisible = planetsToggleButton.isSelected();
      mySkyDisplayFrame.mySkyDisplaySimple.setDisplayPlanets(isVisible);
      mySkyDisplayFrame.mySkyDisplaySimple.repaintSkyDisplay();
   }
/*================================================================================================
/         Sun_ActionPerformed - Toolbar Button Action
/=================================================================================================*/
  public void Sun_actionPerformed(ActionEvent e){
      boolean isVisible = sunToggleButton.isSelected();
      mySkyDisplayFrame.mySkyDisplaySimple.setDisplaySun(isVisible);
      mySkyDisplayFrame.mySkyDisplaySimple.repaintSkyDisplay();
   }
/*================================================================================================
/         Moon_ActionPerformed - Toolbar Button Action
/=================================================================================================*/
  public void Moon_actionPerformed(ActionEvent e){
      boolean isVisible = moonToggleButton.isSelected();
      mySkyDisplayFrame.mySkyDisplaySimple.setDisplayMoon(isVisible);
      mySkyDisplayFrame.mySkyDisplaySimple.repaintSkyDisplay();
   }
/*================================================================================================
/         Target_ActionPerformed - Toolbar Button Action
/=================================================================================================*/
  public void Target_actionPerformed(ActionEvent e){
      boolean isVisible = targetsToggleButton.isSelected();
      mySkyDisplayFrame.mySkyDisplaySimple.setDisplayTargets(isVisible);
      mySkyDisplayFrame.mySkyDisplaySimple.repaintSkyDisplay();
   }
/*================================================================================================
/         HAGrid_ActionPerformed - Toolbar Button Action
/=================================================================================================*/
  public void HAGrid_actionPerformed(ActionEvent e){
      boolean isVisible = haToggleButton.isSelected();
      mySkyDisplayFrame.mySkyDisplaySimple.setDisplayHAGrid(isVisible);
      mySkyDisplayFrame.mySkyDisplaySimple.repaintSkyDisplay();
  }
/*================================================================================================
/         brightStar_ActionPerformed - Toolbar Button Action
/=================================================================================================*/
  public void brightStar_actionPerformed(ActionEvent e){
      boolean isVisible = brightStarToggleButton.isSelected();
      mySkyDisplayFrame.mySkyDisplaySimple.setDisplayBrightStars(isVisible);
      mySkyDisplayFrame.mySkyDisplaySimple.repaintSkyDisplay();
  }
 /*================================================================================================
/         allSky_ActionPerformed - Toolbar Button Action
/=================================================================================================*/
  public void allSky_actionPerformed(ActionEvent e){
      boolean isVisible = allSkyToggleButton.isSelected();
      mySkyDisplayFrame.mySkyDisplaySimple.setDisplayAllSky(isVisible);
      if(isVisible){
        mySkyDisplayFrame.mySkyDisplaySimple.setDisplayBrightStars(false); 
        brightStarToggleButton.setSelected(false);
      }
      mySkyDisplayFrame.mySkyDisplaySimple.repaintSkyDisplay();
  }
/*================================================================================================
/         AirmassGrid_ActionPerformed - Toolbar Button Action
/=================================================================================================*/
  public void AirmassGrid_actionPerformed(ActionEvent e){
      boolean isVisible = airmassGridToggleButton.isSelected();
      mySkyDisplayFrame.mySkyDisplaySimple.setDisplayAirmassGrid(isVisible);
      mySkyDisplayFrame.mySkyDisplaySimple.repaintSkyDisplay();
  }
/*================================================================================================
/         TelescopePosition_ActionPerformed - Toolbar Button Action
/=================================================================================================*/
  public void TelescopePosition_actionPerformed(ActionEvent e){
      boolean isFrameVisible = telescopePositionToggleButton.isSelected();
      if(isFrameVisible){
             myP200DesignerForm.setVisible(true);
      }// end of isFrameVisible
      if(!isFrameVisible){
            myP200DesignerForm.setVisible(false);
      }
  }
 /*================================================================================================
/         EphemerisPosition_ActionPerformed - Toolbar Button Action
/=================================================================================================*/
  public void Ephemeris_actionPerformed(ActionEvent e){
      boolean isFrameVisible = ephemerisToggleButton.isSelected();
      if(isFrameVisible){
             myEphemerisFrame.setVisible(true);
      }// end of isFrameVisible
      if(!isFrameVisible){
            myEphemerisFrame.setVisible(false);
      }
  }
/*================================================================================================
/         SkyDisplayPosition_ActionPerformed - Toolbar Button Action
/=================================================================================================*/
  public void SkyDisplay_actionPerformed(ActionEvent e){
      boolean isFrameVisible = skyDisplayToggleButton.isSelected();
      if(isFrameVisible){
             mySkyDisplayFrame.setVisible(true);
      }// end of isFrameVisible
      if(!isFrameVisible){
             mySkyDisplayFrame.setVisible(false);
      }
  }
/*================================================================================================
/         AstroObjects_ActionPerformed - Toolbar Button Action
/=================================================================================================*/
  public void AstroObjects_actionPerformed(ActionEvent e){
      boolean isFrameVisible = astroObjectsToggleButton.isSelected();
      if(isFrameVisible){
             myAstroObjectDisplayFrame.setVisible(true);
      }// end of isFrameVisible
      if(!isFrameVisible){
             myAstroObjectDisplayFrame.setVisible(false);
      }
  }
/*================================================================================================
/         Airmass_actionPerformed - Toolbar Button Action
/=================================================================================================*/
  public void Airmass_actionPerformed(ActionEvent e){
      boolean isFrameVisible = airmassToggleButton.isSelected();
      if(isFrameVisible){
             myAirmassFrame.setVisible(true);
      }// end of isFrameVisible
      if(!isFrameVisible){
             myAirmassFrame.setVisible(false);
      }
  }
/*================================================================================================
/        TelescopeControl_actionPerformed(ActionEvent e) - Toolbar Button Action
/=================================================================================================*/
  public void TelescopeControl_actionPerformed(ActionEvent e){
      boolean isFrameVisible = telescopeControlToggleButton.isSelected();
      if(isFrameVisible){
             myTelescopeControlPanel.setVisible(true);
      }// end of isFrameVisible
      if(!isFrameVisible){
             myTelescopeControlPanel.setVisible(false);
      }
  }
/*================================================================================================
/        TelescopeControl_actionPerformed(ActionEvent e) - Toolbar Button Action
/=================================================================================================*/
  public void spectralTypeToggleButton_actionPerformed(ActionEvent e){
      boolean isFrameVisible = spectralTypeToggleButton.isSelected();
      if(isFrameVisible){
             myTychoCatalogFrame.setVisible(true);
      }// end of isFrameVisible
      if(!isFrameVisible){
             myTychoCatalogFrame.setVisible(false);
      }
  }
 /*================================================================================================
/        datetimechooserToggleButton_actionPerformed(ActionEvent e) - Toolbar Button Action
/=================================================================================================*/
  public void datetimechooserToggleButton_actionPerformed(ActionEvent e){
      boolean isFrameVisible = datetimechooserToggleButton.isSelected();
      if(isFrameVisible){
             myDateTimeChooserFrame.setVisible(true);
      }// end of isFrameVisible
      if(!isFrameVisible){
             myDateTimeChooserFrame.setVisible(false);
      }
  }
 /*================================================================================================
/        nightlyAlmanacToggleButton_actionPerformed(ActionEvent e) - Toolbar Button Action
/=================================================================================================*/
  public void nightlyAlmanacToggleButton_actionPerformed(ActionEvent e){
      boolean isFrameVisible = nightlyAlmanacToggleButton.isSelected();
      if(isFrameVisible){
             myNightlyWindow.setVisible(true);
      }// end of isFrameVisible
      if(!isFrameVisible){
             myNightlyWindow.setVisible(false);
      }
  }
/*=============================================================================================
/         main method
/=================================================myTychoCatalogFrame============================================*/
  //Main method
  public static void main(String[] args) {
    try {
//      UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
      UIManager.setLookAndFeel(UIManager.getCrossPlatformLookAndFeelClassName());
//      UIManager.put("ProgressBar.selectionBackground",new javax.swing.plaf.ColorUIResource(Color.RED));
//      UIManager.put("ProgressBar.selectionForeground",new javax.swing.plaf.ColorUIResource(Color.RED));
      java.lang.String test = UIManager.getSystemLookAndFeelClassName();
      System.out.println(test);
    }
    catch(Exception e) {
      e.printStackTrace();
    }
    new P200FactSum();
  }
}
