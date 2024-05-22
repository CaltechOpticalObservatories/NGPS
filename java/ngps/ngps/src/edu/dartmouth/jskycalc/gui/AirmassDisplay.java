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
import edu.dartmouth.jskycalc.coord.Celest;
import edu.dartmouth.jskycalc.coord.Const;
import edu.dartmouth.jskycalc.objects.Observation;
import edu.dartmouth.jskycalc.objects.WhenWhere;
import edu.dartmouth.jskycalc.objects.NightlyAlmanac;
import edu.dartmouth.jskycalc.objects.AstrObj;
import java.util.List.*;
import java.awt.*;
import java.awt.event.*;
import java.awt.geom.*;
import javax.swing.*;
import java.util.*;
import edu.caltech.palomar.telescopes.P200.gui.tables.AstroObjectsModel;
import edu.caltech.palomar.telescopes.P200.gui.tables.AstroObject;
import edu.dartmouth.jskycalc.JSkyCalcModel;
import edu.dartmouth.jskycalc.objects.WhenWhere;
import edu.dartmouth.jskycalc.coord.Celest;
import edu.dartmouth.jskycalc.objects.Observation;
import edu.dartmouth.jskycalc.coord.InstantInTime;
import edu.dartmouth.jskycalc.coord.Site;
/*================================================================================================
/     Class Declaration : AirmassDisplay
/=================================================================================================*/
   public class AirmassDisplay extends JComponent implements MouseListener{
       int xpix, ypix;             // window size
       double xwidth, yheight;      // same, but double
       double xlobord, xhibord, ylobord, yhibord;   // user coords of extreme edge
       double xlo, xhi, ylo, yhi;      // user coordinates of frame
       double endfade, startfade;      // beginning and end of "twilight" for fade
       double xvplo, xvphi, yvplo, yvphi;  // borders of frame as fraction of viewport
       Graphics2D g2;
       double jdstart, jdend;
       Font smallfont;
       FontMetrics smallfontmetrics;
       Font mediumfont;
       FontMetrics mediumfontmetrics;
       Font largefont;
       FontMetrics largefontmetrics;
       MouseEvent mousevent;
       Color [] objcolors = {Color.RED,Color.GREEN,Color.CYAN,Color.MAGENTA};
       Color brightyellow = new Color(255,255,0);  // brighteryellow
       public Site        s;
       public InstantInTime i;
       public WhenWhere   w;
       public Observation o;
       NightlyAlmanac Nightly;
//       public Object [] airmassPlotSelections = {null};
       HashMap<String, AstroObject> presenterKey;
       public   AstroObjectsModel myAstroObjectsModel;
       public   JSkyCalcModel     myJSkyCalcModel;
       public static String [] palomarsite = {"Palomar Mountain",  "7.79089",  "33.35667",  "8.",  "1",  "Pacific",  "P",  "1706.",  "1706."};
       public HashMap<String, AstroObject> selectedObjects    = new HashMap<String, AstroObject>();

//  Dependencies that are difficult to remove
//     presenterKey   this is a variable used extensively in astronomical object selection
/*================================================================================================
/     AirmassDisplay()
/=================================================================================================*/
      public AirmassDisplay() {  // constructed after Nightly has been updated ...
//          this.presenterKey = presenterKey;
//          this.o = o;
//          this.Nightly = Nightly;
// NOTE: the proper construction sequence for this component is as follows
//     AirmassDisplay myAirmassDisplay = new AirmassDisplay();
//     myAirmassDisplay.setComponentSize(x,y);
//     myAirmassDisplay.initializeAirmassDisplay(o,Nightly,newAstroObjectsModel);
          s = new Site(palomarsite);
          i = new InstantInTime(s.stdz,s.use_dst);
          w = new WhenWhere(i,s);
          o = new Observation(w,w.zenith2000());
          setComponentSize(800, 500);
          setFocusable(true);
        //  addMouseMotionListener(this);
          addMouseListener(this);

       }
/*================================================================================================
/     setComponentSize(int xpixin, int ypixin)
/=================================================================================================*/
   public void setComponentSize(int xpixin, int ypixin){
          xpix = xpixin;
          ypix = ypixin;
          yheight = (double) ypixin;
          xwidth = (double) xpixin;
          ylo = 3.2;  yhi = 0.9;
          xvplo = 0.08; xvphi = 0.88;
          yvplo = 0.15; yvphi = 0.90;

          smallfont = new Font("Dialog", Font.PLAIN, 11);
          mediumfont = new Font("Dialog",Font.PLAIN, 15);
   }
/*================================================================================================
/     setPresenterKey(HashMap<String, AstroObject> presenterKey)
/=================================================================================================*/
   public void initializeAirmassDisplay(NightlyAlmanac Nightly,AstroObjectsModel newAstroObjectsModel, JSkyCalcModel newJSkyCalcModel){
      setObservation(newJSkyCalcModel.getObsevation());
      setAstroObjectsModel(newAstroObjectsModel);
      setNightly(Nightly);
      setJSkyCalcModel(newJSkyCalcModel);
      setPresenterKey(myAstroObjectsModel.presenterKey);
      setSelectedObjects(myAstroObjectsModel.getSelectedObjects());
      myAstroObjectsModel.addActionListener( new ActionListener()  {
        public void actionPerformed(ActionEvent e) {
            Update();
            repaint();
        }
      });
      Update();
   }
/*================================================================================================
/     setSelectedObjects(HashMap<String, AstroObject> selectedObjects)
/=================================================================================================*/
  public void setSelectedObjects(HashMap<String, AstroObject> selectedObjects){
     this.selectedObjects = selectedObjects;
  }
/*================================================================================================
/     setPresenterKey(HashMap<String, AstroObject> presenterKey)
/=================================================================================================*/
   public void setPresenterKey(HashMap<String, AstroObject> presenterKey){
      this.presenterKey = presenterKey;
   }
/*================================================================================================
/     setAstroObjectsModel(AstroObjectsModel newAstroObjectsModel)
/=================================================================================================*/
   public void setAstroObjectsModel(AstroObjectsModel newAstroObjectsModel){
       this.myAstroObjectsModel = newAstroObjectsModel;
       setPresenterKey(myAstroObjectsModel.presenterKey);
   }
/*================================================================================================
/     setJSkyCalcModel(JSkyCalcModel newJSkyCalcModel)
/=================================================================================================*/
   public void setJSkyCalcModel(JSkyCalcModel newJSkyCalcModel){
      this.myJSkyCalcModel = newJSkyCalcModel;
      this.s = myJSkyCalcModel.getSite();
      this.i = myJSkyCalcModel.i;
      this.w = myJSkyCalcModel.getWhenWhere();
      this.o = myJSkyCalcModel.getObsevation();
   }
/*================================================================================================
/    setNightly(NightlyAlmanac Nightly)
/=================================================================================================*/
 public void setNightly(NightlyAlmanac Nightly){
       this.Nightly = Nightly;  
 }
/*================================================================================================
/   setObservation(Observation o)
/=================================================================================================*/
 public void setObservation(Observation o){
       this.o = o;
 }
/*================================================================================================
/
/=================================================================================================*/
      public void Update() {
          double timespan;
          o.w.SetToNow();
          w = (WhenWhere) Nightly.sunset.clone();
         
          jdstart = w.when.jd;
          xlo = w.when.UTDate.timeofday.value;

          jdend = Nightly.sunrise.when.jd;
          timespan = (jdend - jdstart) * 24.;
          // System.out.printf("timespan = %f hrs\n",timespan);
          xhi = xlo + timespan;
          endfade = xlo + (Nightly.eveningTwilight.when.jd - jdstart) * 24.;
          //System.out.printf("xlo %f dt %f Endfade = %f\n",xlo,
           //      (Nightly.eveningTwilight.when.jd - jdstart) * 24.,endfade);
          startfade = xlo + (Nightly.morningTwilight.when.jd - jdstart) * 24.;

          double span = xhi - xlo;
          double fracx = xvphi - xvplo;
          xlobord = xlo - xvplo * span / fracx;
          xhibord = xhi + (1 - xvphi) * span/fracx;

          span = yhi - ylo;
          double fracy = yvphi - yvplo;
          ylobord = ylo - yvplo * span / fracy;
          yhibord = yhi + (1 - yvphi) * span / fracy;

          // System.out.printf("start %f end %f\n",jdstart, jdend);
          repaint();
       }
/*================================================================================================
/   Mouse Listener Interface
/=================================================================================================*/
       public void mouseClicked(MouseEvent e) {
            // New feature -- set time to the time clicked in the AirmassDisplay window
            mousevent = e;
            double mousetimeofday = xlobord + ((float) mousevent.getX() / (float) xpix) * (xhibord - xlobord);
            double since_xlo = mousetimeofday - xlo;
            double jdmouse = jdstart + since_xlo / 24.;
 // COMMENTED OUT SO THAT A SUB-SET WILL COMPILE
 //           JDfield.setText(String.format(Locale.ENGLISH, "%15.6f",jdmouse));
 //           setToJD();
            // System.out.printf("x %d y %d mousetimeofday %f since_xlo %f jdmouse %f\n",
              //    mousevent.getX(),mousevent.getY(),mousetimeofday,since_xlo,jdmouse);

       }
       // need to define all the rest of the MouseEvent interface of course ...
       public void mousePressed(MouseEvent e) {
       }
       public void mouseReleased(MouseEvent e) {
       }
       public void mouseEntered(MouseEvent e) {
       }
       public void mouseExited(MouseEvent e) {
       }
/*================================================================================================
/      paint(Graphics g)
/=================================================================================================*/
       public void paint(Graphics g) {

          int i, j;
          double xtick;
          double [] xy1 = {0.,0.};
          double [] xy2 = {0.,0.};
          double [] xy3 = {0.,0.};
          double [] xy4 = {0.,0.};
          double jd, ut, local;

          Observation omoon = (Observation) o.clone();

          g2 = (Graphics2D) g;

          Stroke defStroke = g2.getStroke();   // store the default stroke.
          Color gridcolor = new Color(153,0,0);  // dark red

          float [] dot1 = {1.0f,4.0f};
          BasicStroke dotted = new BasicStroke(0.3f,BasicStroke.CAP_BUTT,
                           BasicStroke.JOIN_MITER,1.0f,dot1,0.0f);

          smallfontmetrics = g2.getFontMetrics(smallfont);
          mediumfontmetrics = g2.getFontMetrics(mediumfont);

          g2.setBackground(Color.BLACK);
          g2.clearRect(0,0,xpix,ypix);

          //Color dayColor = new Color(70,130,180);
          // Color dayColor = new Color(60,80,180);
          Color dayColor = new Color(60,105,190);
          Color deepTwilight = new Color(10,20,30);   // faint bluish-grey
          // Color deepTwilight = new Color(40,40,40);   // Brighter grey for testing.

          g2.setPaint(dayColor);                     // left bdy = day color
          xy1 = xytopix(xlo,yhi);
          // g2.fill(new Rectangle2D.Double(0.,0.,xy1[0],yheight));

          xy2 = xytopix(endfade,yhi);
          // System.out.printf("xy1 %f %f  xy2 %f %f\n",xy1[0],xy1[1],xy2[0],xy2[1]);

          GradientPaint daytonight = new GradientPaint((int)xy1[0],0,dayColor,
                                                       (int)xy2[0],0,deepTwilight);
          g2.setPaint(daytonight);
          xy2 = xytopix(endfade,ylo);
          //g2.fill(new Rectangle2D.Double(xy1[0],0.,
           //                              xy2[0],yheight));
          g2.fill(new Rectangle2D.Double(xy1[0],xy1[1],
                                        xy2[0],xy2[1]));

          xy1 = xytopix(startfade,yhi);   // beginning of twilight

          g2.setPaint(Color.BLACK);       // have to overpaint the black ...
          g2.fill(new Rectangle2D.Double(xy2[0],0.,xy1[0],yheight));

          xy2 = xytopix(xhi,yhi);
          GradientPaint nighttoday = new GradientPaint((int)xy1[0],0,deepTwilight,(int)xy2[0],0,dayColor);
          g2.setPaint(nighttoday);
          xy2 = xytopix(xhi,ylo);
          g2.fill(new Rectangle2D.Double(xy1[0],xy1[1],xy2[0],xy2[1]));

          g2.setPaint(Color.BLACK);   // for some reason have to overpaint this ...
          g2.fill(new Rectangle2D.Double(xy2[0],0.,xwidth,yheight));

          // overpaint the bottom, too ...
          xy1 = xytopix(xlo,ylo);
          g2.fill(new Rectangle2D.Double(0,(int)xy1[1],xpix,ypix));

          g2.setPaint(Color.WHITE);

          g2.setStroke(new BasicStroke(1.0f));
          // draw the box
          drawline(xlo,yhi,xhi,yhi);
          drawline(xlo,yhi,xlo,ylo);
          drawline(xlo,ylo,xhi,ylo);
          drawline(xhi,ylo,xhi,yhi);

          double span = xhi - xlo;

          xy1 = xytopix(xlo - 0.07 * span, (ylo + yhi) / 2.);
          g2.rotate(-1. * Const.PI/2.,xy1[0],xy1[1]);
          puttext(xlo - 0.07 * span ,(ylo + yhi)/2.,"Airmass",mediumfont,mediumfontmetrics);
          // g2.transform(saveXform);
          g2.rotate(Const.PI/2,xy1[0],xy1[1]);

          span = yhi - ylo;
          double ytoplabel = yhi + 0.02 * span;
          double ybottomlabel = ylo - 0.05 * span;
          double ybanner = yhi + 0.07 * span;

          span = xhi - xlo;
          double ticklen = (yhi - ylo)/80.;
          double minordiv = 0.25; // hard code divisions, sorry ...

          puttext(xlo - span * 0.04,ytoplabel,"UT:",mediumfont,mediumfontmetrics);
          puttext(xlo - span * 0.05,ybottomlabel,"Local:",mediumfont,mediumfontmetrics);

          w.ChangeWhen(jdstart);

          String banner = String.format("%s - Evening date %s",
               w.where.name,w.when.localDate.RoundedCalString(6,0));
          puttext((xlo + xhi) / 2., ybanner, banner, mediumfont, mediumfontmetrics);


          xtick = (double)((int) xlo + 1);
          double xminortick;

          for(j = 1; j < 4; j++) {  // fill in start ..
             xminortick = xtick - j * minordiv;
             if(xminortick > xlo) {
                drawline(xminortick, yhi, xminortick, yhi - ticklen/2.);
                drawline(xminortick, ylo, xminortick, ylo + ticklen/2.);
             }
          }

          while( xtick < xhi ) {
             w.ChangeWhen(jdstart + (xtick - xlo) / 24.);
             ut = w.when.UTDate.timeofday.value;
             local = w.when.localDate.timeofday.value;
             puttext(xtick,ytoplabel,String.format("%02.0f",ut),
                     mediumfont,mediumfontmetrics);
             puttext(xtick,ybottomlabel,String.format("%02.0f",local),
                     mediumfont,mediumfontmetrics);

             drawline(xtick, yhi, xtick, yhi - ticklen);
             drawline(xtick, ylo, xtick, ylo + ticklen);
             g2.setStroke(dotted);
             g2.setPaint(gridcolor);
             drawline(xtick,yhi,xtick,ylo);
             g2.setStroke(defStroke);
             g2.setPaint(Color.WHITE);
             for(j = 1; j < 4; j ++) {
                 xminortick = xtick + j * minordiv;
                 if(xminortick < xhi) {
                     drawline(xminortick, yhi, xminortick, yhi - ticklen/2.);
                     drawline(xminortick, ylo, xminortick, ylo + ticklen/2.);
                 }
             }

             omoon.w.ChangeWhen(jdstart + (xtick - xlo) / 24.);
             omoon.ComputeSky();
             omoon.ComputeSunMoon();
             if(omoon.w.altmoon > 0.) plotmoon();

             xtick += 1.;
          }

          // Draw a vertical line at current time ...

          // System.out.printf("o.w.when.jd = %f, jdstart = %f, ",o.w.when.jd,jdstart);
	  double hrs_since = xlo + (o.w.when.jd - jdstart) * 24.;
          // System.out.printf("hrs_since = %f\n",hrs_since);
//          System.out.println("Airmass graph: xlo ="+xlo+" xhi ="+xhi+" hrs_since ="+hrs_since);

          if(hrs_since > xlo && hrs_since < xhi) {
                Color nowColor = new Color(128,128,128); // grey
                // g2.setStroke(dotted);
                g2.setPaint(nowColor);
          	drawline(hrs_since,yhi,hrs_since,ylo);
                // g2.setStroke(defStroke);
                g2.setPaint(Color.RED);
//                System.out.println("Airmass graph: Hours since ="+hrs_since);
	  }


          double yticklen = (xhi - xlo) / 120.;
          double yminortick;
          double ytick = 1.0;
          double labelx = xlo - (xhi - xlo) / 30.;
          double ylabeloffset = (yhi - ylo) / 60.;
          while (ytick < ylo) {
             drawline(xlo,ytick,xlo+yticklen,ytick);
             drawline(xhi,ytick,xhi-yticklen,ytick);
             puttext(labelx,ytick-ylabeloffset,String.format("%3.1f",ytick),
                    mediumfont,mediumfontmetrics);
             g2.setStroke(dotted);
             g2.setPaint(gridcolor);
             drawline(xlo,ytick,xhi,ytick);
             g2.setStroke(defStroke);
             g2.setPaint(Color.WHITE);
             for(j = 1; j < 5; j++) {
                yminortick = ytick + j * 0.1;
                if(yminortick < ylo) {
                   drawline(xlo,yminortick,xlo+yticklen/2.,yminortick);
                   drawline(xhi,yminortick,xhi-yticklen/2.,yminortick);
                }
             }
             ytick = ytick + 0.5;
          }

          // draw a separate vertical axis for moon altitude, off to the side.
          // scale is not the same as for the airmass axes.

          g2.setPaint(Color.YELLOW);
          span = xhi - xlo;
          double xmoonaxis = xhi + span * 0.04;
          drawline(xmoonaxis,ylo,xmoonaxis,yhi);
          double tickstep = (ylo - yhi) / 9.;  // 10 degrees
          for(i = 0; i < 10; i++) {
             ytick = yhi + i * tickstep;
             drawline(xmoonaxis-yticklen,ytick,xmoonaxis,ytick);
             if(i % 3 == 0)
                 puttext(xmoonaxis + span/40.,ytick + 0.05,String.format("%d",(9 - i) *10),
                    mediumfont,mediumfontmetrics);
          }

          double xmoonlabel = xmoonaxis + span * 0.05;
          xy1 = xytopix(xmoonlabel,(ylo + yhi) / 2.);
          g2.rotate(Const.PI/2, xy1[0],xy1[1]);
          puttext(xmoonlabel,(ylo + yhi) / 2.,"Moon Altitude [deg]", mediumfont,
              mediumfontmetrics);
          g2.rotate(-1. * Const.PI/2, xy1[0],xy1[1]);

          g2.setPaint(Color.WHITE);

          plotAirmass(o.w,o.c, " ", Color.WHITE);  // white = main window object.
          if (selectedObjects.size() > 0 && selectedObjects != null) {
             String sel;
             String airmname;
             int cindex = 0;
             Iterator iter = selectedObjects.keySet().iterator();
             cindex = 0;
             String currentKey;
             while(iter.hasNext()){
                 currentKey = (String)iter.next();
                 AstroObject currentSelection = (AstroObject)presenterKey.get(currentKey);
                 plotAirmass(o.w, currentSelection, currentSelection.name, objcolors[cindex]);
                 cindex++;
                if(cindex == objcolors.length) cindex = 0;  // cycle colors
             }
          }
       }
/*================================================================================================
/      drawline(double x1, double y1, double x2, double y2)
/=================================================================================================*/
       void drawline(double x1, double y1, double x2, double y2) {  // user coords
               double [] xy1;
               double [] xy2;
               xy1 = xytopix(x1,y1);
               xy2 = xytopix(x2,y2);
          //     System.out.printf("x2 y2 %f %f xy2 %f %f\n",x2,y2,xy2[0],xy2[1]);
               g2.draw(new Line2D.Double(xy1[0],xy1[1],xy2[0],xy2[1]));
       }
/*================================================================================================
/     xytopix(double x, double y)
/=================================================================================================*/
       double [] xytopix(double x, double y) {
            double [] retvals = {0.,0.};
            retvals[0] = xpix * (x - xlobord) / (xhibord - xlobord);
            retvals[1] = ypix * (1. - (y - ylobord) / (yhibord - ylobord));
            return(retvals);
       }

//       double [] pixtoxy(double pixx, double pixy) {
//            double [] retvals = {0.,0.};
//            retvals[0] = xpix * (x - xlobord) / (xhibord - xlobord);
//            retvals[1] = ypix * (1. - (y - ylobord) / (yhibord - ylobord));
//            return(retvals);
//       }
/*================================================================================================
/     puttext(double x, double y, String text, Font font, FontMetrics metrics)
/=================================================================================================*/
       void puttext(double x, double y, String text, Font font, FontMetrics metrics) {
           double [] xy;
           // write text centered left-to-right on user coords x and y.
           xy = xytopix(x,y);

           g2.setFont(font);
           int adv = metrics.stringWidth(text);
           g2.drawString(text,(int) xy[0] - adv / 2,(int) xy[1]);
       }
/*================================================================================================
/     plotAirmass(WhenWhere wIn, Celest cIn, String objectname, Color objcolor)
/=================================================================================================*/
       void plotAirmass(WhenWhere wIn, Celest cIn, String objectname, Color objcolor) {

          g2.setPaint(objcolor);

          Observation oairm = new Observation((WhenWhere) wIn.clone(),
                                   (Celest) cIn.clone());
          // System.out.printf("%s\n",oairm.c.Alpha.RoundedRAString(2,":"));
          double jd = jdstart;
          double dt = 0.005;
          double [] xy1 = {0.,0.};

          float [] dot1 = {2.0f};
          BasicStroke dotted = new BasicStroke(1.0f,BasicStroke.CAP_BUTT,
                           BasicStroke.JOIN_MITER,3.0f,dot1,0.0f);

          GeneralPath airpath = new GeneralPath();
          GeneralPath airpath2 = new GeneralPath();
//          GeneralPath airpath3 = new GeneralPath();
//          GeneralPath airpath4 = new GeneralPath();

          Stroke defStroke = g2.getStroke();

          boolean firstptfound = false;
          boolean uptwice = false;
          double xpl = 0.;

          while(!firstptfound && jd < jdend) {
             oairm.w.ChangeWhen(jd);
             oairm.ComputeSky();
             // System.out.printf("%s %f\n",oairm.w.when.UTDate.RoundedCalString(0,0),oairm.airmass);
             if(oairm.airmass < ylo && oairm.airmass > 0.99
                   && Math.abs(oairm.ha.value) < 6.) {
                // System.out.printf("firstptfound, airmass %f\n",oairm.airmass);
                firstptfound = true;
                xpl = xlo + (jd - jdstart) * 24.;
                xy1 = xytopix(xpl,oairm.airmass);
                airpath.moveTo((float) xy1[0], (float) xy1[1]);
             }
             jd += dt;
          }
          // System.out.printf("Out, airmass %f, jd %f\n",oairm.airmass,jd);
          while(oairm.airmass < ylo && oairm.airmass > 0.99 && jd < jdend
                   && Math.abs(oairm.ha.value) < 6.) {
             // System.out.printf("plt - %s %f\n",oairm.w.when.UTDate.RoundedCalString(0,0),oairm.airmass);
             oairm.w.ChangeWhen(jd);
             oairm.ComputeSky();
             xpl = xlo + (jd - jdstart) * 24.;
             xy1 = xytopix(xpl,oairm.airmass);
             if (oairm.airmass > 0.99 && oairm.airmass < ylo
                   && Math.abs(oairm.ha.value) < 6.) {   // don't follow it past max airm
               //if(Math.abs(oairm.ha.value) > 6.) g2.setStroke(dotted);
               //else g2.setStroke(defStroke);
               airpath.lineTo((float) xy1[0], (float) xy1[1]);
             }
             jd += dt;
          }


          if(firstptfound) {  // label tracks near their ends ....
             if(jd > jdend) puttext(xpl - 0.5, oairm.airmass, objectname,
                        smallfont, smallfontmetrics);
             else if (oairm.airmass > ylo) puttext(xpl + 0.2, ylo-0.05, objectname,
                        smallfont, smallfontmetrics);
             else puttext(xpl + 0.2, oairm.airmass, objectname, smallfont,
                        smallfontmetrics);
          }

          // objects can come up a second time ...

          firstptfound = false;
          while(!firstptfound && jd < jdend) {
             oairm.w.ChangeWhen(jd);
             oairm.ComputeSky();
             if(oairm.airmass < ylo && oairm.airmass > 0.99
                   && Math.abs(oairm.ha.value) < 6.) {
                firstptfound = true;
                uptwice = true;
                xpl = xlo + (jd - jdstart) * 24.;
                xy1 = xytopix(xpl,oairm.airmass);
                airpath2.moveTo((float) xy1[0], (float) xy1[1]);
             }
             jd += dt;
          }
          while(oairm.airmass < ylo && oairm.airmass > 0.99 && jd < jdend
                   && Math.abs(oairm.ha.value) < 6.) {
             oairm.w.ChangeWhen(jd);
             oairm.ComputeSky();
             xpl = xlo + (jd - jdstart) * 24.;
             xy1 = xytopix(xpl,oairm.airmass);
             if (oairm.airmass > 0.99 && oairm.airmass < ylo) {   // don't follow it past 3.5
               //if(Math.abs(oairm.ha.value) > 6.) g2.setStroke(dotted);
               //else g2.setStroke(defStroke);
               airpath2.lineTo((float) xy1[0], (float) xy1[1]);
             }
             jd += dt;
          }
          // now we're sure to have it all.
          g2.draw(airpath);
          if(uptwice) g2.draw(airpath2);  // second part, if there is one.

          g2.setPaint(Color.WHITE);
       }
/*================================================================================================
/     plotmoon()
/=================================================================================================*/
       void plotmoon() {

          Observation omoon = new Observation(w,w.moon.topopos);
          omoon.ComputeSky();
          omoon.ComputeSunMoon();

          int nseg = 21;

          double [][] limbxy = new double [2][nseg];    // in system where cusp axis = y
          double [][] termxy = new double [2][nseg];

          double [][] limbxypr = new double [2][nseg];  // in system where North = y
          double [][] termxypr = new double [2][nseg];

          double theta;
          double cos_sunmoon;
          int i, j, k;

          // set up points describing terminator and limb -- then rotate into
          // position.
          cos_sunmoon = Math.cos(omoon.w.sunmoon / Const.DEG_IN_RADIAN);
          // System.out.printf("sunmoon %f cos_sunmoon %f\n",o.w.sunmoon,cos_sunmoon);
          double dtheta = Const.PI / (double)(nseg - 1);
          for(j = 0; j < nseg; j++) {
             theta = dtheta * (double) j;
             limbxy[0][j] = Math.cos(theta);
             limbxy[1][j] = Math.sin(theta);
             termxy[0][j] = limbxy[0][j];   // need a second copy later
             termxy[1][j] = limbxy[1][j] * cos_sunmoon;
          }

          // rotate to appropriate position angle

//          double pa = w.cusppa + (Const.PI)/2. +
 //                    omoon.parallactic / Const.DEG_IN_RADIAN;
//          double pa = w.cusppa -
 //                   omoon.parallactic / Const.DEG_IN_RADIAN;
          double pa = omoon.w.cusppa - (Const.PI)/2.;
                                // cusppa already in rad.
          if(w.where.lat.value < 0.) pa += Const.PI;  // flip in S.
          double[][] turnmoon = {{Math.cos(pa),Math.sin(pa)},
                                   {-1. * Math.sin(pa),Math.cos(pa)}};
          for(j = 0; j < nseg; j++) {
             for(i = 0; i < 2; i++) {
                limbxypr[i][j] = 0.;
                termxypr[i][j] = 0.;
                for (k = 0; k < 2; k++) {
                   limbxypr[i][j] += turnmoon[i][k] * limbxy[k][j];
                   termxypr[i][j] += turnmoon[i][k] * termxy[k][j];
                }
             }
          }

          double ycent = ylo + (omoon.w.altmoon / 90.) * (yhi - ylo);  // scale ...
          double xcent = xlo + (omoon.w.when.jd - jdstart) * 24.;
          double [] xypix = xytopix(xcent, ycent);
//          System.out.printf("jd %f xcent ycent %f %f altmoon %f \n",
//                          o.w.when.jd,xcent,ycent,omoon.w.altmoon);
//          System.out.printf(" -> pix %f %f\n",xypix[0],xypix[1]);
          double moonsizeinpix = 8.;  // radius
        // double moonsize = 3.;

          GeneralPath limbpath = new GeneralPath();
          GeneralPath termpath = new GeneralPath();

          limbpath.moveTo((float) (xypix[0] + limbxypr[0][0] * moonsizeinpix),
                          (float) (xypix[1] + limbxypr[1][0] * moonsizeinpix));

          termpath.moveTo((float) (xypix[0] + termxypr[0][0] * moonsizeinpix),
                          (float) (xypix[1] + termxypr[1][0] * moonsizeinpix));

          double x1, y1, x2, y2;

          for(i = 1; i < nseg-1; i = i + 2) {
            x1 =  xypix[0] + termxypr[0][i] * moonsizeinpix;
            y1 =  xypix[1] + termxypr[1][i] * moonsizeinpix;
            x2 =  xypix[0] + termxypr[0][i+1] * moonsizeinpix;
            y2 =  xypix[1] + termxypr[1][i+1] * moonsizeinpix;
            termpath.quadTo((float) x1, (float) y1, (float) x2, (float) y2);

            x1 =  xypix[0] + limbxypr[0][i] * moonsizeinpix;
            y1 =  xypix[1] + limbxypr[1][i] * moonsizeinpix;
            x2 =  xypix[0] + limbxypr[0][i+1] * moonsizeinpix;
            y2 =  xypix[1] + limbxypr[1][i+1] * moonsizeinpix;
            limbpath.quadTo((float) x1, (float) y1, (float) x2, (float) y2);

         }

          g2.setPaint(brightyellow);
          g2.draw(limbpath);
          g2.draw(termpath);
          g2.setPaint(Color.WHITE);
       }
    }

