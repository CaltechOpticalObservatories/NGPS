package edu.dartmouth.jskycalc.gui;
//=== File Prolog =============================================================
//	This code was developed by John Thorstensen of Dartmouth University
//
//      It has been refactored and modified by Jennifer Milburn for
//      use at the Palomar Observatory.
//
//      Caltech Optical Observatories, Palomar Observatory
//
//--- Contents ----------------------------------------------------------------
//
//
//--- Description -------------------------------------------------------------
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
//       DERIVED DIRECTLY FROM THE JSKYCALC by JOHN THORSTENSEN, Dartmouth University
//       Original Implementation : John Thorstensen copyright 2007,
//
//       July 24, 2010  Jennifer Milburn, refactor for use at the Palomar Observatory
//
//
//   JSkyCalc24m.java -- 2009 January.  I liked the 1.3m so much I decided
//   to do something similar for the 2.4m.  This has the capability of reading
//   the telescope coords and commenting on them, but it doesn't slew the
//   telescope.  It'll be nice to automatically have the telescope coords there,
//   though. */
//
// JSkyCalc13m.java -- 2009 January.  This version adds an interface to
// * Dick Treffers' 1.3m telescope control server, which communicates via
// * the BAIT protocol.  The target-selection facilities here can then be
// * used. */
//
// JSkyCalc.java -- copyright 2007, John Thorstensen, Dartmouth College. */
//
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
import edu.dartmouth.jskycalc.objects.NightlyAlmanac;
import edu.dartmouth.jskycalc.coord.Spherical;
import edu.dartmouth.jskycalc.coord.Site;
import edu.caltech.palomar.telescopes.guider.catalog.ucac3.ucac3Reader;
import edu.dartmouth.jskycalc.objects.WhenWhere;
import edu.dartmouth.jskycalc.coord.Celest;
import edu.dartmouth.jskycalc.objects.Planets;
import edu.dartmouth.jskycalc.objects.Observation;
import edu.dartmouth.jskycalc.objects.Constel;
import edu.dartmouth.jskycalc.objects.AstrObj;
import edu.dartmouth.jskycalc.objects.AllBrightStar;
import edu.dartmouth.jskycalc.coord.InstantInTime;
import edu.dartmouth.jskycalc.coord.Const;
import java.io.*;
import java.util.*;
import java.util.List.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.border.BevelBorder;
import com.borland.jbcl.layout.*;
import edu.caltech.palomar.telescopes.P200.gui.tables.AstroObjectsModel;
import edu.dartmouth.jskycalc.JSkyCalcModel;
/*================================================================================================
/
/=================================================================================================*/
public class JSkyCalcWindow extends JComponent {
/** This ENORMOUS class includes the entire user interface -- subwindows are
    implemented as subclasses. */
/*
   final UIManager.LookAndFeelInfo [] landfs  = UIManager.getInstalledLookAndFeels();
   final String className;
   className = landfs[2].getClassName();
   try  {
     UIManager.setLookAndFeel(className);
   } catch catch (Exception e) { System.out.println(e); }
*/

/* I also tried to add a MouseListener to this, but it was ignored because the
   SkyDisplay subclass pre-empts it.  Mouse events are always dispatched to the
   "deepest" component in the hierarchy.  */

   public WhenWhere w;
   public InstantInTime i;
   public Site s;
   public Observation o; // oooh!
   public Observation telcoo;
   public Planets p;
   public ucac3Reader u3Reader;

   AllBrightStar [] allbrights;

   SkyDisplay mySkyDisp;
   boolean skydisplayvisible = true;
   final JFrame SkyWin;

   SiteWindow siteframe;
   boolean siteframevisible = false;  // for 1.3m controller, make this invisible

   AirmassDisplay AirDisp;
   boolean airmasswindowvisible = false;
   final JFrame AirWin;

   NightlyAlmanac Nightly;
   boolean nightlyframevisible = false;
   NightlyWindow NgWin;

   Container GPan;  // outer container for window
   boolean GSDvisible = false;

   int sleepinterval = 15000;
   boolean autoupdaterunning = false;
   AutoUpdate au;
   boolean autosteprunning = false;
   AutoStep as;
   Thread thr;
   wait_for_mis wfm;
   Thread wfmthr;
   boolean wfmrunning = false;


//   static AstrObj obj;
   AstroObjSelector ObjSelWin;

//   boolean objselwinvisible = true;  // for this version ...
   static String st = null;
//   public HashMap<String, AstrObj> byname = new HashMap<String, AstrObj>();
//   public HashMap<Double, AstrObj> byra = new HashMap<Double, AstrObj>();
//   static Double rakey;
//   static String [] NameSelectors;  // real names, sorted by dec.
   static HashMap<String, AstrObj> presenterKey = new HashMap<String, AstrObj>();
//   static HashMap<String, Site> siteDict = new HashMap<String, Site>();
   static int max_lists = 20;
   static int num_lists = 0;
   static String [] objListNames; // names (only) of object lists.
//   static HashMap<String, String> objListFilesDict = new HashMap<String,String>();

   AstroObjSelector AirmSelWin;

   // littleHints hints;

   JTextField datefield;
   JTextField timefield;
   JTextField JDfield;

   JTextField objnamefield;
   JTextField RAfield;
   JTextField decfield;
   JTextField equinoxfield;

   JTextField timestepfield;
   JTextField sleepfield;

   JTextField obsnamefield;
   JTextField longitudefield;
   JTextField latitudefield;
   JTextField stdzfield;
   JTextField use_dstfield;
   JTextField zonenamefield;
   JTextField elevseafield;
   JTextField elevhorizfield;

   JTextField siderealfield;
   JTextField HAfield;
   JTextField airmassfield;
   JTextField altazfield;
   JTextField parallacticfield;
   JTextField sunradecfield;
   JTextField sunaltazfield;
   JTextField ztwilightfield;
   JTextField moonphasefield;
   JTextField moonradecfield;
   JTextField moonaltazfield;
   JTextField illumfracfield;
   JTextField lunskyfield;
   JTextField moonobjangfield;
   JTextField baryjdfield;
   JTextField baryvcorfield;
   JTextField constelfield;
   JTextField planetproximfield;

   ButtonGroup UTbuttons;
   JRadioButton UTradiobutton;  // save references for swapper (later)
   JRadioButton Localradiobutton;

   Color outputcolor    = new Color(230,230,230);  // light grey
   Color inputcolor     = Color.WHITE;  // for input fields
   Color lightblue      = new Color(150,220,255);
   Color lightpurple    = new Color(221,136,255);
   Color sitecolor      = new Color(255,215,215); // a little pink
   Color panelcolor     = new Color(185,190,190); // a little darker
   Color brightyellow   = new Color(255,255,0);  // brighteryellow
   Color runningcolor   = new Color(154,241,162);  // pale green, for autostep highlighting
   Color objboxcolor    = new Color(0,228,152);  // for object box on display
   Color dartmouthgreen = new Color(0,100,0);  // dark green, "official" rgb values.
  public   java.lang.String USERDIR           = System.getProperty("user.dir");
  public   java.lang.String SEP               = System.getProperty("file.separator");
  public   java.lang.String CONFIG            = SEP + "config" + SEP;
  public   java.lang.String RESOURCES         = "resources" + SEP;
  public   java.lang.String PATH              = USERDIR + CONFIG + RESOURCES;
  public   AstroObjectsModel myAstroObjectsModel;
  public   JSkyCalcModel     myJSkyCalcModel;
/*================================================================================================
/      JSkyCalcWindow()
/=================================================================================================*/
  public JSkyCalcWindow() {

      final JFrame frame = new JFrame("JSkyCalc24m");

      /* Put up site panel and grab the first site ... */

      siteframe = new SiteWindow();
      siteframevisible = false;   // for 1.3m controller, always MDM.
      siteframe.setVisible(siteframevisible);

//      String [] initialsite = {"Kitt Peak [MDM Obs.]",  "7.44111",  "31.9533",
//           "7.",  "0",  "Mountain",  "M",  "1925",  "700."};

      LoadAllBright();

      s = siteframe.firstSite();
      i = new InstantInTime(s.stdz,s.use_dst);
      w = new WhenWhere(i,s);
      o = new Observation(w,w.zenith2000());
//      telcoo = new Observation(w,w.zenith2000());
//      u3Reader = new ucac3Reader();
      o.ComputeSky();
      o.w.MakeLocalSun();
      o.w.MakeLocalMoon();
      o.ComputeSunMoon();
      p = new Planets(w);
      o.computebary(p);

      int iy = 0;
/*================================================================================================
/    CONSTRUCT SEPARATE CONTROLS AND FRAMES
/=================================================================================================*/
      Nightly = new NightlyAlmanac(o.w);
     // int skywinxpix = 850;
      int skywinxpix = 800;
      int skywinypix = 700;
      // First construct the frames that will ho
      SkyWin = new JFrame("Sky Display");
      SkyWin.setSize(skywinxpix+15, skywinypix+35);
      SkyWin.setLocation(215,590);
      SkyWin.setVisible(true);

      
      AirWin = new JFrame("Airmass Display");
      AirWin.setSize(800,530);
      AirWin.setLocation(400,200);
      AirWin.setVisible(airmasswindowvisible);
/*================================================================================================
/    CONSTRUCT SEPARATE CONTROLS AND FRAMES
/=================================================================================================*/
      myAstroObjectsModel = new AstroObjectsModel(AirWin);
      myJSkyCalcModel     = new JSkyCalcModel();
      mySkyDisp    = new SkyDisplay(      skywinxpix, skywinypix,SkyWin,ObjSelWin,o,p,presenterKey);
      mySkyDisp.repaint();
      AirDisp    = new AirmassDisplay();
      AirDisp.setComponentSize(800,500);
      AirDisp.initializeAirmassDisplay(Nightly,myAstroObjectsModel,myJSkyCalcModel);
      ObjSelWin  = new AstroObjSelector(true,mySkyDisp,AirDisp,presenterKey);  // argument is "is_single"
      AirmSelWin = new AstroObjSelector(false,mySkyDisp,AirDisp,presenterKey);

      JPanel testPanel = new JPanel(new BorderLayout());
      testPanel.setBorder(BorderFactory.createLoweredBevelBorder());
      testPanel.add(mySkyDisp,BorderLayout.CENTER);
      XYLayout MainxYLayout                         = new XYLayout();
      SkyWin.setLayout(MainxYLayout);
      SkyWin.add(testPanel,new XYConstraints(  50,   50, 800,  700));
      AirWin.add(AirDisp);
      // after extensive experimentation, BoxLayout does the trick.
 
      ObjSelWin.setVisible(true);
      ObjSelWin.setLocation(0,390);

      AirmSelWin.setVisible(airmasswindowvisible);

      /* make up the separate subpanels */

/* ****************** DO NOT REMOVE THIS BANNER *********************************
   ******************************************************************************

   I am distributing this COPYRIGHTED code freely with the condition that this
         credit banner will not be removed.  */
      JPanel bannerpanel = new JPanel();
      JLabel bannerlabel = new JLabel("JSkyCalc24m: John Thorstensen, Dartmouth College");
      bannerpanel.setBackground(dartmouthgreen);
      bannerlabel.setBackground(dartmouthgreen);
      bannerlabel.setForeground(Color.WHITE);
      bannerpanel.add(bannerlabel);

/* ******************************************************************************
   ****************************************************************************** */
      // make a JPanel into which to stuff the picture and the controls
      JPanel GP               = new JPanel();
      JPanel controlbuttonpan = new JPanel();
      JPanel buttonpan        = new JPanel();
      JPanel textpanel        = new JPanel();
             UTbuttons        = new ButtonGroup();
      Container outer = frame.getContentPane();
                outer.setLayout(           new FlowLayout());
                GP.setLayout(              new BoxLayout(GP,BoxLayout.PAGE_AXIS));
                controlbuttonpan.setLayout(new FlowLayout());  // it's the default anyway


      buttonpan.add(Localradiobutton = new JRadioButton("Local",true));
      buttonpan.add(UTradiobutton    = new JRadioButton("UT",false));
      UTbuttons.add(Localradiobutton);
      UTbuttons.add(UTradiobutton);


      UTradiobutton.setActionCommand("UT");
      Localradiobutton.setActionCommand("Local");

      textpanel.setLayout(new GridBagLayout());
      GridBagConstraints constraints = new GridBagConstraints();
//  Declaration of JButtons
      JButton       synchbutton      = new JButton("Refresh output");
      JButton       nowbutton        = new JButton("Set to Now");
      JButton       forwardbutton    = new JButton("Step Forward");
      JButton       backbutton       = new JButton("Step Back");
      final JButton updatebutton     = new JButton("Auto Update");
      final JButton stepbutton       = new JButton("Auto Step");
      JButton       sitebutton       = new JButton("Site\n Menu");
      JButton       nightlybutton    = new JButton("Nightly Almanac");
      JButton       objselshow       = new JButton("Object Lists ...");
      JButton       skydisplayshow   = new JButton("Sky Display");
      JButton       airmassshow      = new JButton("Airmass Graphs");
      JButton       brightstarbutton = new JButton("Nearest Bright Star");
      JButton       stopper          = new JButton("Quit");
// Declaration of JLabels
      JLabel buttonlabel       = new JLabel("Time is:");
      JLabel objnamelabel      = new JLabel("Object: ",SwingConstants.RIGHT);
      JLabel RAlabel           = new JLabel("RA: ",SwingConstants.RIGHT);
      JLabel declabel          = new JLabel("dec: ",SwingConstants.RIGHT);
      JLabel equinoxlabel      = new JLabel("equinox: ",SwingConstants.RIGHT);
      JLabel datelabel         = new JLabel("Date: ",SwingConstants.RIGHT);
      JLabel timelabel         = new JLabel("Time: ",SwingConstants.RIGHT);
      JLabel JDlabel           = new JLabel("JD: ",SwingConstants.RIGHT);
      JLabel timesteplabel     = new JLabel("timestep: ",SwingConstants.RIGHT);
      JLabel sleeplabel        = new JLabel("sleep for (s): ",SwingConstants.RIGHT);
      JLabel obsnamelabel      = new JLabel("Site name: ",SwingConstants.RIGHT);
      JLabel longitudelabel    = new JLabel("Longitude: ",SwingConstants.RIGHT);
      JLabel latitudelabel     = new JLabel("Latitude: ",SwingConstants.RIGHT);
      JLabel stdzlabel         = new JLabel("Time zone: ",SwingConstants.RIGHT);
      JLabel use_dstlabel      = new JLabel("DST code: ",SwingConstants.RIGHT);
      JLabel zonenamelabel     = new JLabel("Zone name: ",SwingConstants.RIGHT);
      JLabel elevsealabel      = new JLabel("Elevation: ",SwingConstants.RIGHT);
      JLabel elevhorizlabel    = new JLabel("Terrain elev: ",SwingConstants.RIGHT);
      JLabel sidereallabel     = new JLabel(" Sidereal ",SwingConstants.RIGHT);
      JLabel HAlabel           = new JLabel(" HA ",SwingConstants.RIGHT);
      JLabel airmasslabel      = new JLabel(" Airmass ",SwingConstants.RIGHT);
      JLabel altazlabel        = new JLabel(" AltAz ",SwingConstants.RIGHT);
      JLabel parallacticlabel  = new JLabel(" parallactic ",SwingConstants.RIGHT);
      JLabel sunradeclabel     = new JLabel(" SunRAdec ",SwingConstants.RIGHT);
      JLabel sunaltazlabel     = new JLabel(" SunAltAz ",SwingConstants.RIGHT);
      JLabel ztwilightlabel    = new JLabel(" ZTwilight",SwingConstants.RIGHT);
      JLabel moonphaselabel    = new JLabel(" MoonPhase ",SwingConstants.RIGHT);
      JLabel moonradeclabel    = new JLabel(" MoonRAdec ",SwingConstants.RIGHT);
      JLabel moonaltazlabel    = new JLabel(" MoonAltAz ",SwingConstants.RIGHT);
      JLabel illumfraclabel    = new JLabel(" MoonIllumFrac ",SwingConstants.RIGHT);
      JLabel lunskylabel       = new JLabel(" LunSkyBrght ",SwingConstants.RIGHT);
      JLabel moonobjanglabel   = new JLabel(" Moon-Obj ang. ",SwingConstants.RIGHT);
      JLabel baryjdlabel       = new JLabel(" Bary. JD ",SwingConstants.RIGHT);
      JLabel baryvcorlabel     = new JLabel(" Bary. Vcorrn. ",SwingConstants.RIGHT);
      JLabel constellabel      = new JLabel(" Constellation ",SwingConstants.RIGHT);
      JLabel planetproximlabel = new JLabel("Planet Warning? ",SwingConstants.RIGHT);
// Declaration of JTextFields
      RAfield           = new JTextField(13);
      decfield          = new JTextField(13);
      equinoxfield      = new JTextField(13);
      datefield         = new JTextField(13);
      timefield         = new JTextField(13);
      JDfield           = new JTextField(13);
      timestepfield     = new JTextField(13);
      sleepfield        = new JTextField(13);
      obsnamefield      = new JTextField(13);
      longitudefield    = new JTextField(13);
      latitudefield     = new JTextField(13);
      stdzfield         = new JTextField(13);
      use_dstfield      = new JTextField(13);
      zonenamefield     = new JTextField(13);
      elevseafield      = new JTextField(13);
      elevhorizfield    = new JTextField(13);
      siderealfield     = new JTextField(16);
      HAfield           = new JTextField(16);
      airmassfield      = new JTextField(16);
      altazfield        = new JTextField(16);
      parallacticfield  = new JTextField(16);
      sunradecfield     = new JTextField(16);
      sunaltazfield     = new JTextField(16);
      ztwilightfield    = new JTextField(16);
      moonphasefield    = new JTextField(16);
      moonradecfield    = new JTextField(16);
      moonaltazfield    = new JTextField(16);
      illumfracfield    = new JTextField(16);
      lunskyfield       = new JTextField(16);
      moonobjangfield   = new JTextField(16);
      baryjdfield       = new JTextField(16);
      baryvcorfield     = new JTextField(16);
      constelfield      = new JTextField(16);
      planetproximfield = new JTextField(16);
      objnamefield      = new JTextField(13);

      objnamefield.setText("null");
      timestepfield.setText("1 h");
      sleepfield.setText("15");   // 15 second sleep time for auto-update.

      RAfield.setToolTipText("White fields accept input; Enter key synchs output.");
      JDfield.setToolTipText("Hitting Enter in JD field forces time to JD.");
      timestepfield.setToolTipText("Units can be h, m, s, d, t (sid. day), l (lunation)");
      sleepfield.setToolTipText("Used in Auto Update and Auto Step");
      obsnamefield.setToolTipText("You must select 'Allow User Input' for sites not on menu");
      obsnamefield.setToolTipText("If these fields are pink, they're not accepting input.");
      objnamefield.setToolTipText("If object list is loaded, you can select by name.");

      obsnamefield.setBackground(sitecolor);
      longitudefield.setBackground(sitecolor);
      latitudefield.setBackground(sitecolor); 
      stdzfield.setBackground(sitecolor);
      use_dstfield.setBackground(sitecolor);
      zonenamefield.setBackground(sitecolor);
      elevseafield.setBackground(sitecolor); 
      elevhorizfield.setBackground(sitecolor);
      siderealfield.setBackground(outputcolor);
      HAfield.setBackground(outputcolor);
      airmassfield.setBackground(outputcolor);
      altazfield.setBackground(outputcolor);
      parallacticfield.setBackground(outputcolor);
      sunradecfield.setBackground(outputcolor);
      sunaltazfield.setBackground(outputcolor);
      ztwilightfield.setBackground(outputcolor);
      moonphasefield.setBackground(outputcolor);
      moonradecfield.setBackground(outputcolor);
      moonaltazfield.setBackground(outputcolor);
      illumfracfield.setBackground(outputcolor);
      lunskyfield.setBackground(outputcolor);
      moonobjangfield.setBackground(outputcolor);
      baryjdfield.setBackground(outputcolor);
      baryvcorfield.setBackground(outputcolor);
      constelfield.setBackground(outputcolor);
      planetproximfield.setBackground(outputcolor);
//   Construction of GridBagConstraints and addition of components to the textpanel
      iy = 0; constraints.gridx = 0;  constraints.gridy = iy;
      textpanel.add(objnamelabel, constraints);
      constraints.gridx = 1;  constraints.gridy = iy;
      textpanel.add(objnamefield, constraints);
 
      iy++; constraints.gridx = 0;  constraints.gridy = iy;
      textpanel.add(RAlabel, constraints);
      constraints.gridx = 1;  constraints.gridy = iy;
      textpanel.add(RAfield, constraints);

      iy++; constraints.gridx = 0;  constraints.gridy = iy;
      textpanel.add(declabel, constraints);
      constraints.gridx = 1;  constraints.gridy = iy;
      textpanel.add(decfield, constraints);

      iy++; constraints.gridx = 0;  constraints.gridy = iy;
      textpanel.add(equinoxlabel, constraints);
      constraints.gridx = 1;  constraints.gridy = iy;
      textpanel.add(equinoxfield, constraints);

      iy++; constraints.gridx = 0;  constraints.gridy = iy;
      textpanel.add(datelabel, constraints);
      constraints.gridx = 1;  constraints.gridy = iy;
      textpanel.add(datefield, constraints);

      iy++; constraints.gridx = 0;  constraints.gridy = iy;
      textpanel.add(timelabel, constraints);
      constraints.gridx = 1;  constraints.gridy = iy;
      textpanel.add(timefield, constraints);

      iy++; constraints.gridx = 0;  constraints.gridy = iy;
      textpanel.add(buttonlabel,constraints);
      constraints.gridx = 1;  constraints.gridy = iy;
      textpanel.add(buttonpan,constraints);

      iy++; constraints.gridx = 0;  constraints.gridy = iy;
      textpanel.add(timesteplabel, constraints);
      constraints.gridx = 1;  constraints.gridy = iy;
      textpanel.add(timestepfield, constraints);

      iy++; constraints.gridx = 0;  constraints.gridy = iy;
      textpanel.add(sleeplabel, constraints);
      constraints.gridx = 1;  constraints.gridy = iy;
      textpanel.add(sleepfield, constraints);

      iy++; constraints.gridx = 0;  constraints.gridy = iy;
      textpanel.add(JDlabel, constraints);
      constraints.gridx = 1;  constraints.gridy = iy;
      textpanel.add(JDfield, constraints);

      iy++; constraints.gridx = 0;  constraints.gridy = iy;
      textpanel.add(obsnamelabel, constraints);
      constraints.gridx = 1;  constraints.gridy = iy;
      textpanel.add(obsnamefield, constraints);

      iy++; constraints.gridx = 0;  constraints.gridy = iy;
      textpanel.add(longitudelabel, constraints);
      constraints.gridx = 1;  constraints.gridy = iy;
      textpanel.add(longitudefield, constraints);

      iy++; constraints.gridx = 0;  constraints.gridy = iy;
      textpanel.add(latitudelabel, constraints);
      constraints.gridx = 1;  constraints.gridy = iy;
      textpanel.add(latitudefield, constraints);

      iy++; constraints.gridx = 0;  constraints.gridy = iy;
      textpanel.add(stdzlabel, constraints);
      constraints.gridx = 1;  constraints.gridy = iy;
      textpanel.add(stdzfield, constraints);

      iy++; constraints.gridx = 0;  constraints.gridy = iy;
      textpanel.add(use_dstlabel, constraints);
      constraints.gridx = 1;  constraints.gridy = iy;
      textpanel.add(use_dstfield, constraints);

      iy++; constraints.gridx = 0;  constraints.gridy = iy;
      textpanel.add(zonenamelabel, constraints);
      constraints.gridx = 1;  constraints.gridy = iy;
      textpanel.add(zonenamefield, constraints);

      iy++; constraints.gridx = 0;  constraints.gridy = iy;
      textpanel.add(elevsealabel, constraints);
      constraints.gridx = 1;  constraints.gridy = iy;
      textpanel.add(elevseafield, constraints);

      iy++; constraints.gridx = 0;  constraints.gridy = iy;
      textpanel.add(elevhorizlabel, constraints);
      constraints.gridx = 1;  constraints.gridy = iy;
      textpanel.add(elevhorizfield, constraints);

// Right-hand column of output ...

      iy = 0; constraints.gridx = 2;  constraints.gridy = iy;
      textpanel.add(sidereallabel, constraints);
      constraints.gridx = 3;  constraints.gridy = iy;
      textpanel.add(siderealfield, constraints);

      iy++; constraints.gridx = 2;  constraints.gridy = iy;
      textpanel.add(HAlabel, constraints);
      constraints.gridx = 3;  constraints.gridy = iy;
      textpanel.add(HAfield, constraints);

      iy++; constraints.gridx = 2;  constraints.gridy = iy;
      textpanel.add(airmasslabel, constraints);
      constraints.gridx = 3;  constraints.gridy = iy;
      textpanel.add(airmassfield, constraints);

      iy++; constraints.gridx = 2;  constraints.gridy = iy;
      textpanel.add(altazlabel, constraints);
      constraints.gridx = 3;  constraints.gridy = iy;
      textpanel.add(altazfield, constraints);

      iy++; constraints.gridx = 2;  constraints.gridy = iy;
      textpanel.add(parallacticlabel, constraints);
      constraints.gridx = 3;  constraints.gridy = iy;
      textpanel.add(parallacticfield, constraints);

      iy++; constraints.gridx = 2;  constraints.gridy = iy;
      textpanel.add(sunradeclabel, constraints);
      constraints.gridx = 3;  constraints.gridy = iy;
      textpanel.add(sunradecfield, constraints);

      iy++; constraints.gridx = 2;  constraints.gridy = iy;
      textpanel.add(sunaltazlabel, constraints);
      constraints.gridx = 3;  constraints.gridy = iy;
      textpanel.add(sunaltazfield, constraints);

      iy++; constraints.gridx = 2;  constraints.gridy = iy;
      textpanel.add(ztwilightlabel, constraints);
      constraints.gridx = 3;  constraints.gridy = iy;
      textpanel.add(ztwilightfield, constraints);

      iy++; constraints.gridx = 2;  constraints.gridy = iy;
      textpanel.add(moonphaselabel, constraints);
      constraints.gridx = 3;  constraints.gridy = iy;
      textpanel.add(moonphasefield, constraints);

      iy++; constraints.gridx = 2;  constraints.gridy = iy;
      textpanel.add(moonradeclabel, constraints);
      constraints.gridx = 3;  constraints.gridy = iy;
      textpanel.add(moonradecfield, constraints);

      iy++; constraints.gridx = 2;  constraints.gridy = iy;
      textpanel.add(moonaltazlabel, constraints);
      constraints.gridx = 3;  constraints.gridy = iy;
      textpanel.add(moonaltazfield, constraints);

      iy++; constraints.gridx = 2;  constraints.gridy = iy;
      textpanel.add(illumfraclabel, constraints);
      constraints.gridx = 3;  constraints.gridy = iy;
      textpanel.add(illumfracfield, constraints);

      iy++; constraints.gridx = 2;  constraints.gridy = iy;
      textpanel.add(lunskylabel, constraints);
      constraints.gridx = 3;  constraints.gridy = iy;
      textpanel.add(lunskyfield, constraints);

      iy++; constraints.gridx = 2;  constraints.gridy = iy;
      textpanel.add(moonobjanglabel, constraints);
      constraints.gridx = 3;  constraints.gridy = iy;
      textpanel.add(moonobjangfield, constraints);

      iy++; constraints.gridx = 2;  constraints.gridy = iy;
      textpanel.add(baryjdlabel, constraints);
      constraints.gridx = 3;  constraints.gridy = iy;
      textpanel.add(baryjdfield, constraints);

      iy++; constraints.gridx = 2;  constraints.gridy = iy;
      textpanel.add(baryvcorlabel, constraints);
      constraints.gridx = 3;  constraints.gridy = iy;
      textpanel.add(baryvcorfield, constraints);

      iy++; constraints.gridx = 2;  constraints.gridy = iy;
      textpanel.add(constellabel, constraints);
      constraints.gridx = 3;  constraints.gridy = iy;
      textpanel.add(constelfield, constraints);

      iy++; constraints.gridx = 2;  constraints.gridy = iy;
      textpanel.add(planetproximlabel, constraints);
      constraints.gridx = 3;  constraints.gridy = iy;
      textpanel.add(planetproximfield, constraints);


    // control buttons go in a separate panel at the bottom ...

      Dimension controlbuttonpansize = new Dimension(540,170);  // ugh
      controlbuttonpan.setPreferredSize(controlbuttonpansize);

      controlbuttonpan.add(synchbutton);
      controlbuttonpan.add(nowbutton);
      controlbuttonpan.add(forwardbutton);
      controlbuttonpan.add(backbutton);
      controlbuttonpan.add(updatebutton);
      controlbuttonpan.add(stepbutton);
      controlbuttonpan.add(sitebutton);
      controlbuttonpan.add(nightlybutton);
      controlbuttonpan.add(objselshow);
      controlbuttonpan.add(skydisplayshow);
      controlbuttonpan.add(airmassshow);
      controlbuttonpan.add(brightstarbutton);
      controlbuttonpan.add(stopper);

      outer.add(bannerpanel);
      outer.add(textpanel);
      outer.add(controlbuttonpan);

      stopper.setBackground(sitecolor);
      controlbuttonpan.setBackground(panelcolor);
      sleepfield.setBackground(runningcolor);  // draw attention
      outer.setBackground(panelcolor);
      textpanel.setBackground(panelcolor);
                /* Panel for text I/O fields and their labels ... */
      buttonpan.setBackground(panelcolor);

      frame.setDefaultCloseOperation( JFrame.EXIT_ON_CLOSE );
      frame.setSize(540,590);
//      frame.setSize(560,620);
      frame.setLocation(345,20);
      frame.setContentPane(outer);
      frame.setVisible(true);

            // Need to fill in values or synchOutput chokes later ...
      RAfield.setText(o.c.Alpha.RoundedRAString(2,":"));
      decfield.setText(o.c.Delta.RoundedDecString(1,":"));
      equinoxfield.setText(String.format(Locale.ENGLISH, "%7.2f",o.c.Equinox));
/*================================================================================================
/      action listeners for the text fields ...
/=================================================================================================*/
      Localradiobutton.addActionListener(new ActionListener() {    // so it toggles time ...
         public void actionPerformed(ActionEvent e) {
            setToJD();
         }
      });
      synchbutton.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent e) {
            setToDate();
         }
      });
      UTradiobutton.addActionListener(new ActionListener() {    // so it toggles time ...
         public void actionPerformed(ActionEvent e) {
            setToJD();
         }
      });
      nowbutton.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent e) {
            SetToNow();
         }
      });
      forwardbutton.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent e) {
            advanceTime();
         }
      });
      backbutton.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent e) {
            advanceTime(false);
         }
      });
      updatebutton.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent e) {
            if (!autoupdaterunning) {
               sleepfield.setBackground(runningcolor);  // draw attention
               thr = new Thread(au);
               autoupdaterunning = true;
               thr.start();
               updatebutton.setText("Stop Update");
               updatebutton.setBackground(Color.ORANGE);
               // TelWin.setVisible(true);   // only expose tel control when update is on.
               // TelWinvisible = true;
            }
            else {
               autoupdaterunning = false;
               thr.interrupt();
               updatebutton.setText("Resume Update");
               sleepfield.setBackground(Color.WHITE);
               updatebutton.setBackground(Color.WHITE);  // not quite right
               synchOutput();  // to display warning colors in TelWin.
               // TelWin.setVisible(false);   // only expose tel control when update is on.
               // TelWinvisible = false;   // only expose tel control when update is on.
            }
         }
      });
      stepbutton.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent e) {
            if (!autosteprunning) {
               sleepfield.setBackground(runningcolor);  // draw attention
               timestepfield.setBackground(runningcolor);  // draw attention
               thr = new Thread(as);
               autosteprunning = true;
               thr.start();
               stepbutton.setText("Stop Stepping");
               stepbutton.setBackground(Color.ORANGE);
            }
            else {
               autosteprunning = false;
               thr.interrupt();
               stepbutton.setText("Resume Stepping");
               sleepfield.setBackground(Color.WHITE);
               timestepfield.setBackground(Color.WHITE);
               stepbutton.setBackground(Color.WHITE);  // not quite right
           }
         }
      });
        sitebutton.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent e) {
            if(siteframevisible)  {  // there's got to be a better way ..
               siteframe.setVisible(false);
               siteframevisible = false;
            }
            else {  // site frame is invisible
               siteframe.setVisible(true);
               siteframevisible = true;
            }
         }
      });
        nightlybutton.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent e) {
            if(nightlyframevisible)  {  // there's got to be a better way ..
               NgWin.setVisible(false);
               nightlyframevisible = false;
            }
            else {  // nightly frame is invisible
               Nightly.Update(o.w);
               NgWin.UpdateDisplay();
               NgWin.setVisible(true);
               nightlyframevisible = true;
            }
         }
      });
       objselshow.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent e) {
             ObjSelWin.objselwinvisible = true;
             ObjSelWin.setVisible(true);
         }
      });
        skydisplayshow.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent e) {
             if(skydisplayvisible) {
                skydisplayvisible = false;
                SkyWin.setVisible(false);
             }
             else {
                skydisplayvisible = true;
                mySkyDisp.repaint();
                SkyWin.setVisible(true);
             }
         }
      });
       airmassshow.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent e) {
            if(!airmasswindowvisible) {
               airmasswindowvisible = true;
               AirWin.setVisible(true);
               AirmSelWin.setVisible(true);
               synchOutput();
            }
            else {
               airmasswindowvisible = false;
               AirWin.setVisible(false);
               AirmSelWin.setVisible(false);
            }
         }
      });
       brightstarbutton.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent e) {
               // System.out.printf("Bright star feature not yet implemented.\n");
//               NearestBright(telcoo.c);
               synchOutput();
         }
      });
       stopper.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent e) {
            /* If you hate the confirm-exit, start killing lines here.... */
            int result = JOptionPane.showConfirmDialog(frame,
                "Really quit JSkyCalc24m?");
            switch(result) {
              case JOptionPane.YES_OPTION:
                  System.exit(1);      // protected exit call ...
                  break;
              case JOptionPane.NO_OPTION:
              case JOptionPane.CANCEL_OPTION:
              case JOptionPane.CLOSED_OPTION:
                  break;
            }
            /* ... and stop killing them here, then uncomment the line below. */
            // System.exit(1);    // ... naked exit call.
         }
      });
       objnamefield.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent e) {
            try {
               String sel = objnamefield.getText();
               RAfield.setText(
                    presenterKey.get(sel).c.Alpha.RoundedRAString(3," "));
               decfield.setText(
                    presenterKey.get(sel).c.Delta.RoundedDecString(2," "));
               equinoxfield.setText(String.format(Locale.ENGLISH, "%7.2f",
                    presenterKey.get(sel).c.Equinox));
               synchOutput();
            } catch (Exception exc) {
               objnamefield.setText("Not Found.");
            }
         }
      });
       RAfield.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent e) {
            setToDate();
        }
      });

      decfield.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent e) {
            setToDate();
        }
      });

      equinoxfield.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent e) {
            setToDate();
        }
      });

      datefield.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent e) {
            setToDate();
        }
      });

      timefield.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent e) {
            setToDate();
        }
      });

      JDfield.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent e) {
            setToJD();
        }
      });

      timestepfield.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent e) {
            advanceTime();
        }
      });

      // Actions on observatory parameters lead to a setToJD()
      // to parallel behavior of the site menu.

      obsnamefield.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent e) {
            setToJD();
        }
      });

      longitudefield.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent e) {
            setToJD();
        }
      });

      latitudefield.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent e) {
            setToJD();
        }
      });

      stdzfield.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent e) {
            setToJD();
        }
      });

      use_dstfield.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent e) {
            setToJD();
        }
      });

      zonenamefield.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent e) {
            setToJD();
        }
      });

      elevseafield.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent e) {
            setToJD();
        }
      });

      elevhorizfield.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent e) {
            setToJD();
        }
      });
      siteframe.addActionListener(new java.awt.event.ActionListener(){
      public void actionPerformed(ActionEvent e) {
           java.lang.String fileName        = e.getActionCommand();
            int             actionID        = e.getID();
            switch(actionID){
                case SiteWindow.SETTOJD : setToJD(); break;
                case SiteWindow.COLOR_USER_INPUT_TRUE : ColorUserInput(true); break;
                case SiteWindow.COLOR_USER_INPUT_FALSE: ColorUserInput(false); break;
            }
      }
    });
      mySkyDisp.addActionListener(new java.awt.event.ActionListener(){
      public void actionPerformed(ActionEvent e) {
           java.lang.String fileName        = e.getActionCommand();
            int             actionID        = e.getID();
            switch(actionID){
                case SkyDisplay.SYNCH_OUTPUT       : synchOutput();
                     mySkyDisp.repaint(); break;
                case SkyDisplay.ADVANCE_TIME       : advanceTime(); 
                     mySkyDisp.repaint(); break;
                case SkyDisplay.ADVANCE_TIME_TRUE  : advanceTime(true);
                     mySkyDisp.repaint(); break;
                case SkyDisplay.ADVANCE_TIME_FALSE : advanceTime(false); 
                     mySkyDisp.repaint(); break;
            }
      }
    });
/*================================================================================================
/
/=================================================================================================*/
      synchSite();
      synchOutput();

      au = new AutoUpdate("x");   // instantiate for later start if desired
      as = new AutoStep("s");

      thr = new Thread(au);
      autoupdaterunning = true;
      thr.start();
      updatebutton.setText("Stop Update");
      updatebutton.setBackground(Color.ORANGE);
 }
/*================================================================================================
/       synchOutput()
/=================================================================================================*/
   void synchOutput() {
      boolean is_ut;
      Color mooncolor;  // used for several fields
      double parallactic, altparallactic;

      o.c.UpdateFromStrings(RAfield.getText(),decfield.getText(),
          equinoxfield.getText());

      // special hook for equinox of date ...
      if(o.c.Equinox < 0.)  {
         o.c.Equinox = o.w.when.JulianEpoch();
      }

      // and repeat them back ...

      RAfield.setText(       o.c.Alpha.RoundedRAString(2," "));
      decfield.setText(      o.c.Delta.RoundedDecString(1," "));
      equinoxfield.setText(  String.format(Locale.ENGLISH, "%7.2f",o.c.Equinox));

      o.ComputeSky();
      o.ComputeSunMoon();
      o.computebary(p);
      checkPlanets();


      HAfield.setText(o.ha.RoundedHAString(0," "));
      HAfield.setBackground(HAWarningColor(o.ha.value));

      if(o.altitude < 0.) airmassfield.setText("(down.)");
      else if (o.airmass > 10.) airmassfield.setText("> 10.");
      else airmassfield.setText(String.format(Locale.ENGLISH, "%6.3f",o.airmass));
      airmassfield.setBackground(AirMassWarningColor(o.altitude,o.airmass));

      altazfield.setText(String.format(Locale.ENGLISH, "%5.1f  az = %6.1f",o.altitude,o.azimuth));

      parallactic = o.parallactic;
      while(parallactic < -180.) parallactic += 360.;
      while(parallactic >= 180.) parallactic -= 360.;
      altparallactic = parallactic + 180.;
      while(altparallactic < -180.) altparallactic += 360.;
      while(altparallactic >= 180.) altparallactic -= 360.;

      parallacticfield.setText(String.format(Locale.ENGLISH, "%5.1f  [%5.1f] degr.",parallactic,
        altparallactic));

      if(o.w.altsun > 0.)
         ztwilightfield.setText("(Daytime.)");
      else if(o.w.altsun < -18.)
         ztwilightfield.setText("No twilight.");
      else {
         ztwilightfield.setText(String.format(Locale.ENGLISH, "%5.1f mag (blue)",o.w.twilight));
      }
      ztwilightfield.setBackground(TwilightWarningColor(o.w.altsun,o.w.twilight));

      mooncolor = MoonWarningColor(o.w.altmoon,o.altitude,o.w.altsun,o.moonobj,o.moonlight);
      if (o.w.altmoon < -2.) lunskyfield.setText("Moon is down.");
      else if (o.altitude < 0.) lunskyfield.setText("Target is down.");
      else if(o.w.altsun < -12.) {
         lunskyfield.setText(String.format(Locale.ENGLISH, "%5.1f V mag/sq arcsec",o.moonlight));
      }
      else if (o.w.altsun < 0.) lunskyfield.setText("(Bright twilight.)");
      else lunskyfield.setText("(Daytime.)");
      lunskyfield.setBackground(mooncolor);

      baryjdfield.setText(    String.format(Locale.ENGLISH, "%13.5f  [%6.1f s]",o.baryjd,o.barytcor));
      baryvcorfield.setText(  String.format(Locale.ENGLISH, "%6.2f km/s",o.baryvcor));
      constelfield.setText(   Constel.getconstel(o.c));

      String UTstring = UTbuttons.getSelection().getActionCommand();
      if (UTstring.equals("UT")) is_ut = true;
      else is_ut = false;      // easy binary choice

      if(is_ut) {
         datefield.setText(      o.w.when.UTDate.RoundedCalString(6,0));
         timefield.setText(      o.w.when.UTDate.RoundedCalString(11,1));
      }
      else {
         datefield.setText(      o.w.when.localDate.RoundedCalString(6,0));
         timefield.setText(      o.w.when.localDate.RoundedCalString(11,1));
      }
      JDfield.setText(String.format(Locale.ENGLISH, "%15.6f",o.w.when.jd));
      // w.siderealobj.setRA(w.sidereal);  // update
      siderealfield.setText(     o.w.siderealobj.RoundedRAString(0," "));
      sunradecfield.setText(     o.w.sun.topopos.shortstring());
      moonradecfield.setText(    o.w.moon.topopos.shortstring());
      sunaltazfield.setText(     String.format(Locale.ENGLISH, "%5.1f   az = %6.1f",o.w.altsun,o.w.azsun));
      moonaltazfield.setText(    String.format(Locale.ENGLISH, "%5.1f   az = %6.1f",o.w.altmoon,o.w.azmoon));
      moonphasefield.setText(    String.format(Locale.ENGLISH, "%s",o.w.moon.MoonPhaseDescr(o.w.when.jd)));
      illumfracfield.setText(    String.format(Locale.ENGLISH, "%5.3f",o.w.moonillum));
      moonobjangfield.setText(   String.format(Locale.ENGLISH, "%5.1f deg",o.moonobj));
      moonobjangfield.setBackground(mooncolor);
  // This line is here for debugging only
     mySkyDisp.repaint();


   }
/*================================================================================================
/        synchSite()
/=================================================================================================*/
   void synchSite() {

      String SiteString = siteframe.SiteButtons.getSelection().getActionCommand();
      if( SiteString.equals("x") ) {   // code for load from fields ...
          o.w.where.name          = obsnamefield.getText();
          o.w.where.lat.setDec(latitudefield.getText());
          o.w.where.longit.setFromString(longitudefield.getText());
          o.w.where.stdz          = Double.parseDouble(stdzfield.getText());
          o.w.where.use_dst       = Integer.parseInt(use_dstfield.getText());
          o.w.where.timezone_name = zonenamefield.getText();
          String [] fields        = elevseafield.getText().trim().split("\\s+");  // chuck unit
          o.w.where.elevsea       = Double.parseDouble(fields[0]);
          fields = elevhorizfield.getText().trim().split("\\s+");  // chuck unit
          // System.out.printf("About to try to parse %s ... \n",fields[0]);
          double elevhoriz_in = Double.parseDouble(fields[0]);
          if (elevhoriz_in >= 0.)  o.w.where.elevhoriz = Double.parseDouble(fields[0]);
          else {
              System.out.printf("Negative elev_horiz causes a square root error later.  Set to zero.\n");
              o.w.where.elevhoriz = 0.;
          }
      }
      else {
          o.w.ChangeSite(siteframe.siteDict, SiteString);
//          telcoo.w.ChangeSite(siteDict, SiteString);  // don't forget these ...
      }
      // and spit them all back out ...
      obsnamefield.setText(     o.w.where.name);
      longitudefield.setText(   o.w.where.longit.RoundedLongitString(1," ",false));
      latitudefield.setText(    o.w.where.lat.RoundedDecString(0," "));
      stdzfield.setText(        String.format(Locale.ENGLISH, "%5.2f",o.w.where.stdz));
      use_dstfield.setText(     String.format(Locale.ENGLISH, "%d",o.w.where.use_dst));
      zonenamefield.setText(    o.w.where.timezone_name);
      elevseafield.setText(     String.format(Locale.ENGLISH, "%4.0f m",o.w.where.elevsea));
      elevhorizfield.setText(   String.format(Locale.ENGLISH, "%4.0f m",o.w.where.elevhoriz));
   }
/*================================================================================================
/        MoonWarningColor
/=================================================================================================*/
   Color MoonWarningColor(double altmoon,
           double altitude, double altsun, double moonobj, double moonlight) {
      if(altmoon < 0. | altitude < 0.) return outputcolor;
      if(altsun > -12.) return outputcolor;  // twilight dominates
      if(moonobj > 25.) {   // not proximity
         if(moonlight > 21.5) return outputcolor;
         else if (moonlight > 19.5) {
            // System.out.printf("lightblue moon \n");
            return lightblue;
         }
         else {
              // System.out.printf("lightpurple moon\n");
              return lightpurple;
         }
      }
      if(moonobj < 10.) return Color.RED;  // always flag < 10 deg
      if(moonlight > 21.5) return outputcolor;
      if(moonlight > 19.5) {
         // System.out.printf("brightyellow moon \n");
         return brightyellow;
      }
      if(moonlight > 18.) return Color.ORANGE;
      return Color.RED;
   }
/*================================================================================================
/      AirMassWarningColor(double altitude,double airmass)
/=================================================================================================*/
   Color AirMassWarningColor(double altitude,double airmass) {
      if(altitude < 0.) return Color.RED;
      if(airmass < 2.) return outputcolor;
      if(airmass > 4.) return Color.RED;
      if(airmass > 3.) return Color.ORANGE;
      return brightyellow;
   }
/*================================================================================================
/      TwilightWarningColor(double altsun, double twilight)
/=================================================================================================*/
   Color TwilightWarningColor(double altsun, double twilight) {
      if(altsun < -18.) return outputcolor;
      if(altsun > 0.) return lightblue;
      if(twilight < 3.5) return brightyellow;
      if(twilight < 8.) return Color.ORANGE;
      return Color.RED;
   }
/*================================================================================================
/       HAWarningColor(Double haval)
/=================================================================================================*/
   Color HAWarningColor(Double haval) {
      if(Math.abs(haval) > 6.) return Color.ORANGE;
      else return outputcolor;
   }
/*================================================================================================
/     ColorUserInput(boolean allowed)
/=================================================================================================*/
      public void ColorUserInput(boolean allowed) {
        /* sets the background color in all the site param boxes according to whether
           user input is allowed or not. */
         if(allowed) {
            obsnamefield.setBackground(inputcolor);
            longitudefield.setBackground(inputcolor);
            latitudefield.setBackground(inputcolor);
            stdzfield.setBackground(inputcolor);
            use_dstfield.setBackground(inputcolor);
            zonenamefield.setBackground(inputcolor);
            elevseafield.setBackground(inputcolor);
            elevhorizfield.setBackground(inputcolor);
         }
         else {
            obsnamefield.setBackground(sitecolor);
            longitudefield.setBackground(sitecolor);
            latitudefield.setBackground(sitecolor);
            stdzfield.setBackground(sitecolor);
            use_dstfield.setBackground(sitecolor);
            zonenamefield.setBackground(sitecolor);
            elevseafield.setBackground(sitecolor);
            elevhorizfield.setBackground(sitecolor);
         }
      }
/*================================================================================================
/      SetToNow()
/=================================================================================================*/
   void SetToNow() {

      // Get site and UT options
      synchSite();

      o.w.SetToNow();
      o.w.ComputeSunMoon();
      synchOutput();
   }
/*================================================================================================
/     checkPlanets()
/=================================================================================================*/
      void checkPlanets() {
      int i;
      double sepn;

      String warningtext = "";
      int warninglevel = 0;  // save worst warning ... 0 none, 1 orange, 2 red.

      for(i = 0; i < 9; i++) {
         if (i != 2) {  // skip earth ....
            sepn = Spherical.subtend(o.c, p.PlanetObs[i].c) * Const.DEG_IN_RADIAN;
            if(sepn < 3.) {
               if(i > 0 & i < 6) {  // Venus through Saturn
                  warningtext = warningtext + String.format(Locale.ENGLISH, "%s - %4.2f deg ",p.names[i],sepn);
                  if(sepn < 1. ) {
                     warninglevel = 2;
                  }
                  else if (warninglevel < 2) warninglevel = 1;
               }
               else {   // the rest of the planets
                  if(sepn < 1.) {
                     warningtext = warningtext + String.format(Locale.ENGLISH, "%s - %4.2f deg ",p.names[i],sepn);
                     if (warninglevel < 1) warninglevel = 1;
                  }
               }
            }
         }
      }
      if (warningtext.equals("")) {
         planetproximfield.setText(" --- ");
         planetproximfield.setBackground(outputcolor);
      }
      else {
         planetproximfield.setText(warningtext);
         if (warninglevel == 1) planetproximfield.setBackground(Color.ORANGE);
         else planetproximfield.setBackground(Color.RED);
      }
   }
/*================================================================================================
/     advanceTime()
/=================================================================================================*/
   void advanceTime() {

      synchSite();

      String advanceString = timestepfield.getText();
      o.w.AdvanceWhen(advanceString);
      o.w.ComputeSunMoon();

      synchOutput();

   }
/*================================================================================================
/     advanceTime(boolean forward)
/=================================================================================================*/
   void advanceTime(boolean forward) {

      synchSite();

      String advanceString = timestepfield.getText();
      o.w.AdvanceWhen(advanceString, forward);
      o.w.ComputeSunMoon();

      synchOutput();
   }
/*================================================================================================
/     setToDate()
/=================================================================================================*/
   void setToDate() {

      String dateTimeString;
      boolean is_ut;

      // Get site and UT options
      //String SiteString = SiteButtons.getSelection().getActionCommand();
      //o.w.ChangeSite(SiteString);
      synchSite();

      String UTstring = UTbuttons.getSelection().getActionCommand();
      if (UTstring.equals("UT")) is_ut = true;
      else is_ut = false;      // easy binary choice

      // grab date and time from input fields
      String dateString = datefield.getText();
      String [] datepieces = dateString.split("\\s+");
      // take first three fields in date -- to ignore day of week
      dateTimeString = String.format(Locale.ENGLISH, "%s %s %s %s",datepieces[0],datepieces[1],
             datepieces[2],timefield.getText());

      // System.out.printf("%s %s %s\n",dateTimeString,UTstring,is_ut);
      // set the actual time ...
      o.w.ChangeWhen(dateTimeString, is_ut);
      // and update the display with the new values.
      o.w.ComputeSunMoon();
      synchOutput();
  }
/*================================================================================================
/       public void setToJD
/=================================================================================================*/
  public void setToJD() {

      double jd;
      boolean is_ut;

      // Get site and UT options, and set them ...
      //String SiteString = SiteButtons.getSelection().getActionCommand();
      //o.w.ChangeSite(SiteString);
      synchSite();

      String UTstring = UTbuttons.getSelection().getActionCommand();
      if (UTstring.equals("UT")) is_ut = true;
      else is_ut = false;      // easy binary choice

      // grab the jd from the display
      jd = Double.parseDouble(JDfield.getText());

      // System.out.printf("jd = %f\n",jd);
      // change the time using current options
      o.w.ChangeWhen(jd);
      o.w.ComputeSunMoon();
      synchOutput();
  }
/*================================================================================================
/     LoadAllBright()
/=================================================================================================*/
  public void LoadAllBright() {

       int i = 0;
       allbrights = new AllBrightStar [9096];
       File infile = null;
       FileReader fr = null;
       BufferedReader br = null;
       String st;

       try {
//           ClassLoader cl = this.getClass().getClassLoader();
//    	   InputStream is = cl.getResourceAsStream("bright_pmupdated.dat");
//           br = new BufferedReader(new InputStreamReader(is));
           FileInputStream fis = new FileInputStream(PATH + "bright_pmupdated.dat");
           br = new BufferedReader(new InputStreamReader(fis));
       } catch (Exception e)
         { System.out.printf("Problem opening brightest.dat for input.\n"); }

       try {
          while((st = br.readLine()) != null) {
              allbrights[i] = new AllBrightStar(st);
              i++;
          }
       } catch (IOException e) { System.out.println(e); }

       //infile.close();

       System.out.printf("%d bright stars read (entire catlog).\n",allbrights.length);
  }
/*================================================================================================
/     NearestBright(Celest incel)
/=================================================================================================*/
  void NearestBright(Celest incel) {

       /* Find the bright star nearest to the given coordinates,
       from the _full_ Bright Star List and load it into the display. */

       double decband = 10.;     // degrees
       double decin;
       Celest objcel;
       double sep, minsep = 1000000000000.;
       int i, minindex = 0;
       decin = incel.Delta.value;

       System.out.printf("Looking up bright star nearest to %s ... \n",incel.checkstring());

       for(i = 0; i < allbrights.length; i++) {
           if(Math.abs(decin - allbrights[i].c.Delta.value) < decband) {  // guard expensive subtend
               sep = Spherical.subtend(incel,allbrights[i].c);
               if (sep < minsep) {
                   minsep = sep;
                   minindex = i;
               }
          }
       }

       String outname = String.format("%s_V=%4.1f",allbrights[minindex].name,
          allbrights[minindex].mag);

       objnamefield.setText(outname);
       RAfield.setText(allbrights[minindex].c.Alpha.RoundedRAString(2," "));
       decfield.setText(allbrights[minindex].c.Delta.RoundedDecString(1," "));
       equinoxfield.setText(String.format(Locale.ENGLISH, "%7.2f",allbrights[minindex].c.Equinox));
   }
/*================================================================================================
/      Inner Class used for suspending thread execution for a short period
/=================================================================================================*/
    class wait_for_mis implements Runnable {
        int msec;
        public wait_for_mis(int msecin) {
           msec = msecin;
        }
        public void run() {
           while(wfmrunning) {
               try {
                  Thread.sleep(msec);
               }
               catch (InterruptedException e) {
                  System.out.println("wait_for_mis interrupted.\n");
               }
            }
        }
     }
/*================================================================================================
/     class AutoStep
/=================================================================================================*/
     class AutoStep implements Runnable {
      String mystr;
      AutoStep(String s) {
         this.mystr = s;   // constructor does basically nothing ...
      }
      public void run() {
         while(autosteprunning) {
            advanceTime();
            sleepinterval = 1000 * Integer.parseInt(sleepfield.getText());
            try {
              Thread.sleep(sleepinterval);
            } catch( InterruptedException e ) {
               System.out.println("Auto-step interrupted.");
            }
         }
      }
   }
/*================================================================================================
/     class AutoUpdate
/=================================================================================================*/
   class AutoUpdate implements Runnable {
      String mystr;
      AutoUpdate(String s) {
         this.mystr = s;   // constructor does basically nothing ...
      }
      public void run() {
         while(autoupdaterunning) {
            // Telescope query only when window is up.
            SetToNow();
            sleepinterval = 1000 * Integer.parseInt(sleepfield.getText());
            try {
              Thread.sleep(sleepinterval);
            } catch( InterruptedException e ) {
               System.out.println("Auto-update interrupted.");
            }
         }
      }
   }
/*================================================================================================
/     main(String [] args)
/=================================================================================================*/
  public static void main(String [] args) {
   //   Locale locale = Locale.getDefault();  ... fixes the problems with German locale,
   //   Locale.setDefault(Locale.ENGLISH);    ... but, breaks the webstart!!

      try {
	    // Set cross-platform Java L&F (also called "Metal")
        UIManager.setLookAndFeel(
            UIManager.getCrossPlatformLookAndFeelClassName());
      }
      catch (UnsupportedLookAndFeelException e) {
         System.out.printf("UnsupportedLookAndFeelException ... \n");
      }
      catch (ClassNotFoundException e) {
         System.out.printf("Class not found while trying to set look-and-feel ...\n");
      }
      catch (InstantiationException e) {
         System.out.printf("Instantiation exception while trying to set look-and-feel ...\n");
      }
      catch (IllegalAccessException e) {
         System.out.printf("Illegal access exception while trying to set look-and-feel ...\n");
      }

      JSkyCalcWindow MainWin = new JSkyCalcWindow() ;

   }




}
