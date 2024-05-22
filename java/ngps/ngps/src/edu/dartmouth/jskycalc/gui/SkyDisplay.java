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
import edu.dartmouth.jskycalc.objects.BrightStar;
import edu.dartmouth.jskycalc.coord.Const;
import edu.dartmouth.jskycalc.coord.Celest;
import edu.dartmouth.jskycalc.objects.AstrObj;
import edu.dartmouth.jskycalc.objects.Observation;
import edu.dartmouth.jskycalc.objects.Planets;
import edu.dartmouth.jskycalc.coord.RA;
import edu.dartmouth.jskycalc.coord.dec;
import edu.dartmouth.jskycalc.coord.Spherical;
import java.io.*;
import java.util.*;
import java.util.List.*;
import java.awt.*;
import java.awt.event.*;
import java.awt.geom.*;
import javax.swing.*;
/*================================================================================================
/     class SkyDisplay
/=================================================================================================*/
 public class SkyDisplay extends JComponent implements   MouseMotionListener, KeyListener,  MouseListener{
       int         xpixint, ypixint;       // number of pixels in x and y directions, int
       double      xpix, ypix, aspect;  // number of pixels in x and y directions, double
       double      halfwidthy;          // sets the scale factor.
       double      halfwidthx;
       double      pixperunit;
       double      xmid, ymid;          // x, y coords of the center of the screen.
       double      halfwidthxfull, halfwidthyfull, pixperunitfull;
       double      currentlat;       // keep track of this to avoid recomputing the HAgrid
       Graphics2D  g2;
       Font        smallfont;
       Font        mediumfont;
       Font        largefont;
       FontMetrics smallfontmetrics;
       FontMetrics mediumfontmetrics;
       FontMetrics largefontmetrics;
       BrightStar [] bs;
       GeneralPath [] HAgrid;         // polylines that draw out hour angle paths and equator
       static String [] RASelectors;  // real names, sorted by ra
       MouseEvent mousevent;
       KeyEvent keyevent;
       double zoomedby;
       tinyHelp tinyhelp;
// additions to allow refactoring into packages
   JFrame SkyWin;
   AstroObjSelector ObjSelWin;
   Observation      o;
   Planets          p;
   Color gridcolor;
   Color brightyellow = new Color(255,255,0);  // brighteryellow
   Color objboxcolor = new Color(0,228,152);  // for object box on display
   HashMap<String, AstrObj> presenterKey;
//  The following text fields from the JSkyCalcWindow are directly referenced from this class causing
//  an unfortunate dependency.  The textfield updates can be better handled by modifying a common telescope position
//  object model but this will require some additional design work
//
//              RAfield.setText(markedC.Alpha.RoundedRAString(0," "));
//              RAfield.setText(markedC.Alpha.RoundedRAString(0," "));
//              RAfield.setText(bs[minindex].c.Alpha.RoundedRAString(2," "));
//              decfield.setText(markedC.Delta.RoundedDecString(0," "));
//              decfield.setText(markedC.Delta.RoundedDecString(0," "));
//              decfield.setText(bs[minindex].c.Delta.RoundedDecString(1," "));
//              objnamefield.setText(bs[minindex].name);
//              equinoxfield.setText(String.format(Locale.ENGLISH, "%7.2f",bs[minindex].c.Equinox));
//
//  Major dependencies that are difficult to remove without a redesign of the class communication
//        JSkyCalcWindow  methods
//     advanceTime();
//     advanceTime(false);
//     synchOutput();
//     presenterKey   this is a variable used extensively in astronomical object selection

  public   java.lang.String USERDIR           = System.getProperty("user.dir");
  public   java.lang.String SEP               = System.getProperty("file.separator");
  public   java.lang.String CONFIG            = SEP + "config" + SEP;
  public   java.lang.String RESOURCES         = "resources" + SEP;
  public   java.lang.String PATH              = USERDIR + CONFIG + RESOURCES;
  private transient Vector  actionListeners;
  public   static final int SYNCH_OUTPUT       = 1000;
  public   static final int ADVANCE_TIME       = 1001;
  public   static final int ADVANCE_TIME_TRUE  = 1002;
  public   static final int ADVANCE_TIME_FALSE = 1003;
/*================================================================================================
/     SkyDisplay(int xpixIn, int ypixIn,JFrame frame,AstrObjSelector ObjSelWin,Observation o,Planets p)
/=================================================================================================*/
       public SkyDisplay(int xpixIn, int ypixIn,JFrame frame,AstroObjSelector ObjSelWin,Observation o,Planets p,HashMap<String, AstrObj> presenterKey) {
           this.presenterKey = presenterKey;
           this.p = p;
           this.o = o;
           this.ObjSelWin = ObjSelWin;
           this.SkyWin = frame;
           xpixint = xpixIn;
           ypixint = ypixIn;
           xpix = (double) xpixint;
           ypix = (double) ypixint;
           aspect = xpix / ypix;
           xmid = 0.;  ymid = 0.;
           halfwidthy = 0.88;
           halfwidthx = halfwidthy * aspect;
           pixperunit = ypix / (2. * halfwidthy);

           halfwidthxfull = halfwidthx;  halfwidthyfull = halfwidthy; pixperunitfull = pixperunit;
           zoomedby = 1.;

           Graphics g;

           smallfont  = new Font("Dialog", Font.PLAIN, 11);
           mediumfont = new Font("Dialog",Font.PLAIN, 15);
           largefont  = new Font("Dialog",Font.PLAIN, 18);
           gridcolor  = new Color(153,0,0);  // dark red

           LoadBright();

           currentlat = o.w.where.lat.value;
           makeHAgrid(currentlat);   // computes it, doesn't plot it.

           setFocusable(true);
           addKeyListener(this);
           addMouseMotionListener(this);
           addMouseListener(this);
           tinyhelp = new tinyHelp();
       }
/*================================================================================================
/       MOUSE LISTENER INTERFACE IMPLEMENTATION
/=================================================================================================*/
       public void mousePressed(MouseEvent e) {
            Point parentloc = SkyWin.getLocation();
            if(e.getButton() == MouseEvent.BUTTON3) {
                tinyhelp.show((int) parentloc.getX() + e.getX(),
                              (int) parentloc.getY() + e.getY());
            }
       }
       public void mouseReleased(MouseEvent e) {
            if(e.getButton() == MouseEvent.BUTTON3) {
                tinyhelp.hide(e.getX(),e.getY());
            }
       }
       public void mouseClicked(MouseEvent e) {
            mousevent = e;
            if(mousevent.getButton() == MouseEvent.BUTTON1) {   // select from list
               Celest markedC = pixtocelest(mousevent.getX(),mousevent.getY());
               ObjSelWin.SelObjByPos(markedC);
               synchOutput();
            }
            else if (mousevent.getButton() == MouseEvent.BUTTON2) {  // middle sets to coords
               Celest markedC = pixtocelest(mousevent.getX(),mousevent.getY());
               synchOutput();
            }
       }
       public void mouseExited(MouseEvent e) {
       }
       public void mouseEntered(MouseEvent e) {
           this.requestFocusInWindow();
       }
       public void mouseDragged(MouseEvent e) { }  // not used for anything.
       public void mouseMoved(MouseEvent e) {    // keeps track of cursor location.
           mousevent = e;
           this.requestFocusInWindow();
       }
/*================================================================================================
/       KEY LISTENER INTERFACE
/=================================================================================================*/
       public void keyReleased(KeyEvent k) {
          // we don't care about these but need empty methods for the interface.
       }
       public void keyPressed(KeyEvent k) {
       }
       public void keyTyped(KeyEvent k) {
           keyevent = k;
           if(k.getKeyChar() == 'f') advanceTime();
           else if(k.getKeyChar() == 'b') advanceTime(false);
           else if(k.getKeyChar() == 'c') {
              Celest markedC = pixtocelest(mousevent.getX(),mousevent.getY());
              synchOutput();
           }
           else if(k.getKeyChar() == 's') {
              Celest markedC = pixtocelest(mousevent.getX(),mousevent.getY());
              ObjSelWin.SelObjByPos(markedC);
              synchOutput();
           }
           else if(k.getKeyChar() == 'z') {
              zoom(mousevent.getX(),mousevent.getY(),2.);
           }
           else if(k.getKeyChar() == 'o') {
              zoom(mousevent.getX(),mousevent.getY(),0.5);
           }
           else if(k.getKeyChar() == 'p') {
              pan(mousevent.getX(),mousevent.getY());
           }
           else if(k.getKeyChar() == 'r') {
              restorefull();
           }
           else if(k.getKeyChar() == 'h') {  // HR star ...
              Celest markedC = pixtocelest(mousevent.getX(),mousevent.getY());
              SelBrightByPos(markedC);
              synchOutput();
           }
           else if(k.getKeyChar() == 'q' ||  k.getKeyChar() == 'x') {
              SkyWin.setVisible(false);
           }
  }
/*================================================================================================
/       SelBrightByPos(Celest incel)
/=================================================================================================*/
      void SelBrightByPos(Celest incel) {
       /* get input from the graphical display, or wherever, and get the nearest
          bright star on the list within a tolerance.  Precession is ignored. */
          double tolerance = 0.1;  // radians
          double decband = 6.;     // degrees
          double decin;
          Celest objcel;
          double sep, minsep = 1000000000000.;
          int i, minindex = 0;
          decin = incel.Delta.value;

          for(i = 0; i < bs.length; i++) {
              if(Math.abs(decin - bs[i].c.Delta.value) < decband) {  // guard expensive subtend
                  sep = Spherical.subtend(incel,bs[i].c);
                  if (sep < minsep) {
                      minsep = sep;
                      minindex = i;
                  }
             }
          }

          if(minsep < tolerance) {
// Commented out to eliminate dependency
//              objnamefield.setText(bs[minindex].name);
//              RAfield.setText(bs[minindex].c.Alpha.RoundedRAString(2," "));
//              decfield.setText(bs[minindex].c.Delta.RoundedDecString(1," "));
//              equinoxfield.setText(String.format(Locale.ENGLISH, "%7.2f",bs[minindex].c.Equinox));
          }
      }
/*================================================================================================
/      makeHAgrid(double latit)
/=================================================================================================*/
       void makeHAgrid(double latit) {

           int i;
           double ha, hamiddle, haend;
           double decstart, decmiddle, decend;
           double [] xystart;
           double [] xystartpix = {0.,0.};
           double [] xymiddle;
           double [] xymiddlepix = {0.,0.};
           double [] xyend;
           double [] xyendpix = {0.,0.};

           double coslat = Math.cos(latit / Const.DEG_IN_RADIAN);
           double sinlat = Math.sin(latit / Const.DEG_IN_RADIAN);

           HAgrid = new GeneralPath[8];

           // draw lines of constant HA from -6h to +6h
           for(i = 0; i < 7; i++) {
               HAgrid[i] = new GeneralPath();
               ha = (double) (2 * i - 6);
               decstart = 90.;
               decmiddle = 85.;
               decend = 80.;
               xystart = SkyProject(ha,decstart,coslat,sinlat);
               xystartpix = xytopix(xystart[0],xystart[1]);

               HAgrid[i].moveTo((float) xystartpix[0], (float) xystartpix[1]);
               while(decend > -91.) {
                   xymiddle = SkyProject(ha,decmiddle,coslat,sinlat);
                   xymiddlepix = xytopix(xymiddle[0],xymiddle[1]);
                   xyend = SkyProject(ha,decend,coslat,sinlat);
                   xyendpix = xytopix(xyend[0],xyend[1]);
                   HAgrid[i].quadTo((float) xymiddlepix[0], (float) xymiddlepix[1],
                                     (float) xyendpix[0], (float) xyendpix[1]);
                   decmiddle = decend - 10.;
                   decend = decend - 10.;
               }
           }
           // draw equator -- tuck it in as the last path in HAgrid.
           HAgrid[i] = new GeneralPath();
           ha = -6.;
           hamiddle = -5.5;
           haend = -5.;
           xystart = SkyProject(ha,0.,coslat,sinlat);
           xystartpix = xytopix(xystart[0],xystart[1]);
           HAgrid[i].moveTo((float) xystartpix[0], (float) xystartpix[1]);
           while(haend < 6.1) {
              xymiddle = SkyProject(hamiddle,0.,coslat,sinlat);
              xymiddlepix = xytopix(xymiddle[0],xymiddle[1]);
              xyend = SkyProject(haend,0.,coslat,sinlat);
              xyendpix = xytopix(xyend[0],xyend[1]);
              HAgrid[i].quadTo((float) xymiddlepix[0], (float) xymiddlepix[1],
                                 (float) xyendpix[0], (float) xyendpix[1]);
              hamiddle = hamiddle + 1.0;
              haend = haend + 1.0;
           }
           System.out.printf("\n");
       }
/*================================================================================================
/     paint(Graphics g)
/=================================================================================================*/
       public void paint(Graphics g) {   // this method does the graphics.
           double [] xy1 = {0.,0.};
           double [] xy2 = {0.,0.};
           int i;

           g2 = (Graphics2D) g;
           smallfontmetrics = g2.getFontMetrics(smallfont);
           mediumfontmetrics = g2.getFontMetrics(mediumfont);
           largefontmetrics = g2.getFontMetrics(largefont);
           g2.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
                 RenderingHints.VALUE_ANTIALIAS_ON);
           g2.setBackground(skycolor());
           g2.clearRect(0,0,xpixint,ypixint);

           // there's some attention given here to stacking these in the best order, since the
           // last thing painted scribbles over previous things.

           if(o.w.where.lat.value != currentlat) {
              currentlat = o.w.where.lat.value;
              makeHAgrid(currentlat);
           }

           g2.setPaint(gridcolor);
           g2.setStroke(new BasicStroke(0.7f));
           for (i = 0; i < HAgrid.length; i++) {
               g2.draw(HAgrid[i]);
           }

           drawcircle(0.,0.,1.,"solid",0.7f);   // horizon
           drawcircle(0.,0.,0.57735,"dotted",0.7f); // 2 airmasses
           drawcircle(0.,0.,0.70711,"dotted",0.7f); // 3 airmasses
           drawcircle(0.,0.,0.7746,"dotted",0.7f); // 4 airmasses
           g2.setStroke(new BasicStroke(1.0f));

           labelDirections();

           PlotBright();

           markcoords(1);
           markcoords(2);  // red (telescope) box paints over blue if same.
           plotsun();
           plotmoon();
           plotplanets();

           drawclock(xmid + 0.85 * halfwidthx, ymid + 0.85 * halfwidthy, 0.10 * halfwidthx);

           puttext(xmid + 0.7 * halfwidthx, ymid - 0.90 * halfwidthy, o.w.where.name,
                  mediumfont, mediumfontmetrics);
           puttext(xmid + 0.7 * halfwidthx, ymid - 0.95 * halfwidthy,
                  o.w.when.UTDate.RoundedCalString(7,0) + " UT", mediumfont,
                  mediumfontmetrics);

           plotobjects();

       }
/*================================================================================================
/     skycolor()
/=================================================================================================*/
       Color skycolor() {
           if(o.w.altsun < -18) return Color.BLACK;
           if(o.w.altsun > -0.8) return new Color(61,122,140);  // daytime sky color
           if(o.w.twilight > 5.)  {
                double fac = (o.w.twilight - 5.) / 10.;
                return new Color(51 + (int) (fac * 10.2), 69 + (int) (fac * 54.),
                                112 + (int) (fac * 28.));
           }
           else {
                double fac = (o.w.twilight + 4.) / 9.;
                return new Color((int) (51. * fac), (int) (68.85 * fac), (int) (112.2 * fac));
           }
       }
/*================================================================================================
/        drawclock(double x, double y, double radius)
/=================================================================================================*/
       void drawclock(double x, double y, double radius) {

           int i;
           double angle, tickx1, tickx2, ticky1, ticky2;
           double cosang, sinang;
           double minutes;
           double seconds;
           double timeval;
           boolean isAM;
           double pixrad;

           double [] xy1 = {0.,0.};
           double [] xy2 = {0.,0.};

           g2.setPaint(Color.CYAN);
           xy1 = xytopix(x - radius,y + radius);
           pixrad = radius * pixperunit;
           g2.draw(new Ellipse2D.Double(xy1[0],xy1[1], 2.* pixrad, 2. * pixrad));

           for(i = 0; i < 12; i++) {
              angle = 0.523599 * (double) i;  // constants is 30 degrees expressed in rad
              cosang = Math.cos(angle);
              sinang = Math.sin(angle);
              tickx1 = x + 0.93 * radius * cosang;
              tickx2 = x + radius * cosang;
              ticky1 = y + 0.93 * radius * sinang;
              ticky2 = y + radius * sinang;
              drawline(tickx1,ticky1,tickx2,ticky2);
           }
           timeval = o.w.when.localDate.timeofday.value;
           isAM = true;
           if(timeval >= 12.) isAM = false;
           while(timeval >= 12.) timeval -= 12.;
           // System.out.printf("Timeval = %f\n",timeval);

           // draw hands
           angle = 0.523599 * timeval;
           cosang = Math.cos(angle);
           sinang = Math.sin(angle);
           g2.setStroke(new BasicStroke(2.5f));
           drawline(x,y,x + 0.65*radius*sinang, y + 0.65*radius*cosang);
           g2.setStroke(new BasicStroke(1.5f));

           angle = 6.2831853  * (timeval - (int) timeval);
           cosang = Math.cos(angle);
           sinang = Math.sin(angle);
           drawline(x,y,x + 0.83*radius*sinang, y + 0.83*radius*cosang);

    /* Uncomment to plot a second hand   */
           g2.setPaint(Color.RED);
           minutes = 60. * (timeval - (int) timeval);
           angle = 6.2831853  * (minutes - (int) minutes);
           cosang = Math.cos(angle);
           sinang = Math.sin(angle);
           g2.setStroke(new BasicStroke(1.0f));
           drawline(x + 0.7 * radius*sinang, y + 0.7 * radius * cosang,
                    x + 0.88* radius*sinang, y + 0.88* radius*cosang);
           g2.setStroke(new BasicStroke(1.5f));
           g2.setPaint(Color.CYAN);
    /* */
           String ampm = "AM";
           if(!isAM) ampm = "PM";
           String dststr = "S";
           if (o.w.when.dstInEffect) dststr = "D";

           String outstr = String.format(Locale.ENGLISH, "%s %s%sT",ampm,o.w.where.zone_abbrev,
                  dststr);
           puttext(x, y - 1.4 * radius, outstr, mediumfont,
                    mediumfontmetrics);
           if(o.w.altsun > -0.9) outstr = "Daytime";
           else if (o.w.altsun < -18.) outstr = "Nighttime";
           else outstr = "Twilight";
          // outstr = String.format(Locale.ENGLISH, "%s",o.w.where.name);
           puttext(x, y - 1.7 * radius, outstr, mediumfont,
                    mediumfontmetrics);
//           String dststr = "Standard";
 //          if (o.w.when.dstInEffect) dststr = "Daylight";
  //         outstr = String.format(Locale.ENGLISH, "%s %s Time",o.w.where.timezone_name,dststr);
   //        puttext(x - 0.9 * radius, y - 1.5 * radius, outstr, mediumfont);
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
/      pixtoxy(int xpixel, int ypixel)
/=================================================================================================*/
       double [] pixtoxy(int xpixel, int ypixel) {
           double x, y;  // map coords, zero at zenith, r = 1 at horizon, radius = tan z/2.
           double [] retvals = {0.,0.};

           retvals[0] = xmid + halfwidthx * ((double) (2 * xpixel) / (double) xpix - 1.);
           retvals[1] = ymid + halfwidthy * (1. - (double) (2 * ypixel) / (double) ypix);

           return retvals;
       }
/*================================================================================================
/        pixtocelest(int xpixel, int ypixel)
/=================================================================================================*/
       Celest pixtocelest(int xpixel, int ypixel) {
           double x, y;  // map coords, zero at zenith, r = 1 at horizon, radius = tan z/2.
           double alt, az;
           double [] retvals = {0.,0.};
           double xt, yt, zt;  // topocentric xyz, zt toward zenith, yt south, xt west

           x = xmid + halfwidthx * ((double) (2 * xpixel) / (double) xpix - 1.);
           y = ymid + halfwidthy * (1. - (double) (2 * ypixel) / (double) ypix);
           double mod = Math.sqrt(x*x + y*y);
          // retvals[0] = (Const.PI / 2. - 2. * Math.atan(mod)) * Const.DEG_IN_RADIAN;  // alt

           if(o.w.where.lat.value < 0.) {
               x = x * -1.;  y = y * -1.;    // plot is inverted in s hemishphere ...
           }
           alt = (Const.PI / 2. - 2. * Math.atan(mod)); // alt
           az = Math.atan2(-1.*x, y); //  * Const.DEG_IN_RADIAN;
          // retvals[1] = az;
          // System.out.printf("%d %d -> %f %f -> %f %f\n",xpixel,ypixel,x,y,retvals[0],retvals[1]);
           zt = Math.sin(alt);
           xt = -1. * Math.cos(alt) * Math.sin(az);
           yt = -1. * Math.cos(alt) * Math.cos(az);
           // rotate around x axis, zenith to the pole ...
           double coslat = Math.cos(o.w.where.lat.radians());
           double sinlat = Math.sin(o.w.where.lat.radians());
           double yc = yt * sinlat + zt * coslat;
           double zc = -1. * yt * coslat + zt * sinlat;
           double [] hadecdist = Celest.XYZcel(xt,yc,zc);  // x remains the same
           // hadecdist[0] is the hour angle ...
           RA Alph = new RA(hadecdist[0] - 6. + o.w.sidereal);
           dec Delt = new dec(hadecdist[1]);
           Celest cel = new Celest(Alph,Delt,2000.); // coord eq is sloppy ...
//           System.out.printf("%s %s\n",cel.Alpha.RoundedRAString(0,":"),
 //                 cel.Delta.RoundedDecString(-1,":"));
           return(cel);
       }
/*================================================================================================
/        drawline(double x1, double y1, double x2, double y2)
/=================================================================================================*/
       void drawline(double x1, double y1, double x2, double y2) {  // user coords
           double [] xy1;
           double [] xy2;
           xy1 = xytopix(x1,y1);
           xy2 = xytopix(x2,y2);
           g2.draw(new Line2D.Double(xy1[0],xy1[1],xy2[0],xy2[1]));
       }
/*================================================================================================
/     drawcircle(double x1, double y1, double radius, String style, float thickness)
/=================================================================================================*/
       void drawcircle(double x1, double y1, double radius, String style, float thickness) {
           // centered on x1, y1 user coords, radius is also in user coords,
           // string can be "solid" or "dashed"
           double [] xy = {0.,0.};
           xy = xytopix(x1 - radius, y1 + radius);   // upper left corner
           double diam = 2. * pixperunit * radius;

           if(style.equals("dashed")) {
               float [] dash1 = {10.0f};
               BasicStroke dashed = new BasicStroke(thickness,BasicStroke.CAP_BUTT,
                           BasicStroke.JOIN_MITER,10.0f,dash1,0.0f);
               g2.setStroke(dashed);
           }
           if(style.equals("dotted")) {
               float [] dot1 = {3.0f};
               BasicStroke dotted = new BasicStroke(thickness,BasicStroke.CAP_BUTT,
                           BasicStroke.JOIN_MITER,3.0f,dot1,0.0f);
               g2.setStroke(dotted);
           }

           g2.draw(new Ellipse2D.Double(xy[0],xy[1],diam,diam));
           g2.setStroke(new BasicStroke(thickness));
       }
/*================================================================================================
/      drawbox(double x1, double y1, double edge, String style, float thickness)
/=================================================================================================*/
       void drawbox(double x1, double y1, double edge, String style, float thickness) {
           // centered on x1, y1 user coords, radius is also in user coords,
           // string can be "solid" or "dashed"
           double [] xy = {0.,0.};
           xy = xytopix(x1 - edge/2., y1 + edge/2.);   // upper left corner
           double edgepix =  pixperunit * edge;

           if(style.equals("dashed")) {
               float [] dash1 = {10.0f};
               BasicStroke dashed = new BasicStroke(thickness,BasicStroke.CAP_BUTT,
                           BasicStroke.JOIN_MITER,10.0f,dash1,0.0f);
               g2.setStroke(dashed);
           }
           if(style.equals("dotted")) {
               float [] dot1 = {3.0f};
               BasicStroke dotted = new BasicStroke(thickness,BasicStroke.CAP_BUTT,
                           BasicStroke.JOIN_MITER,3.0f,dot1,0.0f);
               g2.setStroke(dotted);
           }

           g2.draw(new Rectangle2D.Double(xy[0],xy[1],edgepix,edgepix));
           g2.setStroke(new BasicStroke(thickness));
       }
/*================================================================================================
/     markcoords(int option)
/=================================================================================================*/
       void markcoords(int option) {
           // option 1 = program coords (bluish),
           //       other = telescope coords (red).
           double lst;
           double xy[];

           lst = o.w.sidereal;
	   if(option == 1) {
              double coslat = Math.cos(o.w.where.lat.radians());
              double sinlat = Math.sin(o.w.where.lat.radians());
              xy = SkyProject(lst - o.current.Alpha.value, o.current.Delta.value, coslat, sinlat);
              g2.setPaint(objboxcolor);
              drawbox(xy[0],xy[1],0.05,"solid",1.3f);
	   }
           else {
              double coslat = Math.cos(o.w.where.lat.radians());
              double sinlat = Math.sin(o.w.where.lat.radians());
           }
       }
/*================================================================================================
/     plotsun()
/=================================================================================================*/
       void plotsun() {
           double lst;
           double xy[];

           lst = o.w.sidereal;
           double coslat = Math.cos(o.w.where.lat.radians());
           double sinlat = Math.sin(o.w.where.lat.radians());
           xy = SkyProject(lst - o.w.sun.topopos.Alpha.value,
                           o.w.sun.topopos.Delta.value, coslat, sinlat);
           putDot(xy[0],xy[1],0.009,brightyellow);
           g2.setPaint(brightyellow);
           drawcircle(xy[0],xy[1],0.02,"solid",1.5f);
      }
/*================================================================================================
/      puttext(double x, double y, String text, Font font, FontMetrics metrics)
/=================================================================================================*/
      void puttext(double x, double y, String text, Font font, FontMetrics metrics) {
           double [] xy;
           // write text centered left-to-right on user coords x and y.
           xy = xytopix(x,y);

           g2.setFont(font);
           int adv = metrics.stringWidth(text);
           // System.out.printf("adv = %d\n",adv);
           g2.drawString(text,(int) xy[0] - adv / 2,(int) xy[1]);
       }
/*================================================================================================
/       labelDirections()
/=================================================================================================*/
       void labelDirections() {
          if (o.w.where.lat.value > 0.) {
             puttext(-0.97, 0.,   "E", largefont, largefontmetrics);
             puttext( 0.97, 0.,   "W", largefont, largefontmetrics);
             puttext( 0.0,  0.80, "N", largefont, largefontmetrics);
             puttext( 0.0, -0.83, "S", largefont, largefontmetrics);
          }
          else {
             puttext(-0.97, 0.,   "W", largefont, largefontmetrics);
             puttext( 0.97, 0.,   "E", largefont, largefontmetrics);
             puttext( 0.0,  0.80, "S", largefont, largefontmetrics);
             puttext( 0.0, -0.83, "N", largefont, largefontmetrics);
          }
       }
/*================================================================================================
/     putDot(double x, double y, double size, Color color)
/=================================================================================================*/
       void putDot(double x, double y, double size, Color color) {
          double [] xy;
          // dotsize, unfortunately, is in user units.
          xy = xytopix(x - size/2.,y + size/2.);
          g2.setPaint(color);
          g2.fill(new Ellipse2D.Double(xy[0],xy[1],pixperunit * size, pixperunit * size));
       }
/*================================================================================================
/
/=================================================================================================*/
       void LoadBright() {

          int i = 0;
          bs = new BrightStar [904];
          File infile = null;
          FileReader fr = null;
          BufferedReader br = null;
          String st;

          try {
//	      ClassLoader cl = this.getClass().getClassLoader();
//            InputStream is = cl.getResourceAsStream("brightest.dat");
//            br = new BufferedReader(new InputStreamReader(is));
               FileInputStream fis = new FileInputStream(PATH + "brightest.dat");
               br = new BufferedReader(new InputStreamReader(fis));

              // infile = new File("brightest.dat");
            //  fr = new FileReader(infile);
          } catch (Exception e)
            { System.out.printf("Problem opening brightest.dat for input.\n"); }

          try {
             while((st = br.readLine()) != null) {
                 bs[i] = new BrightStar(st);
                 i++;
             }
          } catch (IOException e) { System.out.println(e); }

          //infile.close();

          System.out.printf("%d bright stars read for display.\n",bs.length);

       }
/*================================================================================================
/     PrecessBrightStars(double equinox)
/=================================================================================================*/
       void PrecessBrightStars(double equinox) {
          // precesses the stars in the BrightStar list bs.  This is more efficient
          // with a special method because the matrix only gets computed once.

          // code is largely copied from the precess method of Celest.
          double ti, tf, zeta, z, theta;
          double cosz, coszeta, costheta, sinz, sinzeta, sintheta, cosdelt;
          double [][] p = {{0.,0.,0.,},{0.,0.,0.},{0.,0.,0.}};
          double [] orig = {0.,0.,0.};
          double [] fin = {0.,0.,0.};
          double [] radecdist = {0.,0.,0.};

          int i, j, ist;

          // compute the precession matrix ONCE.
          ti = (bs[0].c.Equinox - 2000.) / 100.;
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

          for(ist = 0; ist < bs.length; ist++) {
             cosdelt = Math.cos(bs[ist].c.Delta.radians());
             orig[0] = cosdelt * Math.cos(bs[ist].c.Alpha.radians());
             orig[1] = cosdelt * Math.sin(bs[ist].c.Alpha.radians());
             orig[2] = Math.sin(bs[ist].c.Delta.radians());
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
             bs[ist].c.Alpha.setRA(radecdist[0]);
             bs[ist].c.Delta.setDec(radecdist[1]);
             bs[ist].c.Equinox = equinox;
          }
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
/     PlotBright()
/=================================================================================================*/
       void PlotBright() {
          // stripped down for speed -- avoids the OO stuff .
          // Precesses only if mismatch is > 1 year.
          double ha, dec, lst;
          double equinoxnow;
          double xy[];
          double magconst1 = 0.002, magslope = 0.002;  //
	  double magzpt = 4.7;
          int i;

          lst = o.w.sidereal;

          // precess the list if the epoch mismatch is > 1 yr
          equinoxnow = o.w.when.JulianEpoch();
          if(Math.abs(equinoxnow - bs[0].c.Equinox) > 1.) {
              // System.out.printf("%s -> ",bs[0].c.checkstring());
              PrecessBrightStars(equinoxnow);
              // System.out.printf("%s \n",bs[0].c.checkstring());
          }

          double coslat = Math.cos(o.w.where.lat.radians());
          double sinlat = Math.sin(o.w.where.lat.radians());
          for(i = 0; i < bs.length; i++) {
             xy = SkyProject(lst - bs[i].c.Alpha.value, bs[i].c.Delta.value, coslat,sinlat);
             //System.out.printf("PB %f %f\n",xy[0],xy[1]);
             putDot(xy[0],xy[1],magconst1 + magslope * (magzpt - bs[i].m),bs[i].col);
          }
       }
/*================================================================================================
/      plotmoon()
/=================================================================================================*/
       void plotmoon() {

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
          cos_sunmoon = Math.cos(o.w.sunmoon / Const.DEG_IN_RADIAN);
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

          double pa = o.w.cusppa + (Const.PI)/2.;   // cusppa already in rad.
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

          double zover2 = (90. - o.w.altmoon) / (Const.DEG_IN_RADIAN * 2.);
          double coszover2 = Math.cos(zover2);
          double moonsize = 3. * coszover2;

          double rafac = 15. * Math.cos(o.w.moon.topopos.Delta.radians());

          double [] ralimb = new double[nseg];
          double [] declimb = new double[nseg];
          double [] raterm = new double[nseg];
          double [] decterm = new double[nseg];

          double racent =  o.w.moon.topopos.Alpha.value;  // saving verbosity
          double deccent = o.w.moon.topopos.Delta.value;

         // double moonsize = 3.;

          for(i = 0; i < nseg; i++) {
              ralimb[i] = o.w.sidereal -   // Hereafter actually HA, not RA
                  (racent + limbxypr[0][i] * moonsize/ rafac);
              declimb[i] = deccent + limbxypr[1][i] * moonsize;
              raterm[i] = o.w.sidereal -
                  (racent + termxypr[0][i] * moonsize / rafac);
              decterm[i] = deccent + termxypr[1][i] * moonsize;
          }

          double coslat = Math.cos(o.w.where.lat.radians());
          double sinlat = Math.sin(o.w.where.lat.radians());

          GeneralPath limbpath = new GeneralPath();
          GeneralPath termpath = new GeneralPath();

          double [] xy = SkyProject(ralimb[0],declimb[0],coslat,sinlat);
          double [] xypix = xytopix(xy[0],xy[1]);
          limbpath.moveTo((float) xypix[0], (float) xypix[1]);

          xy = SkyProject(raterm[0],decterm[0],coslat,sinlat);
          xypix = xytopix(xy[0],xy[1]);
          termpath.moveTo((float) xypix[0], (float) xypix[1]);

        // try rendering with quadratic curves

          double [] xy2 = {0.,0.};
          double [] xypix2 = {0.,0.};

         for(i = 1; i < nseg-1; i = i + 2) {

            xy = SkyProject(ralimb[i],declimb[i],coslat,sinlat);
            xypix = xytopix(xy[0],xy[1]);
            xy2 = SkyProject(ralimb[i+1],declimb[i+1],coslat,sinlat);
            xypix2 = xytopix(xy2[0],xy2[1]);
            limbpath.quadTo((float) xypix[0], (float) xypix[1],
                             (float) xypix2[0], (float) xypix2[1]);

            xy = SkyProject(raterm[i],decterm[i],coslat,sinlat);
            xypix = xytopix(xy[0],xy[1]);
            xy2 = SkyProject(raterm[i+1],decterm[i+1],coslat,sinlat);
            xypix2 = xytopix(xy2[0],xy2[1]);
            termpath.quadTo((float) xypix[0], (float) xypix[1],
                             (float) xypix2[0], (float) xypix2[1]);

         }
          g2.setPaint(brightyellow);
          g2.draw(limbpath);
          g2.draw(termpath);
       }
/*================================================================================================
/     plotplanets() 
/=================================================================================================*/
       void plotplanets() {
          int i;
          double xy[] = {0.,0.};
          double xypix[] = {0.,0.};
          double ha, dec, lst;
          double magconst1 = 0.003, magslope = 0.002;  //

          p.computemags();

          lst = o.w.sidereal;
          double coslat = Math.cos(o.w.where.lat.radians());
          double sinlat = Math.sin(o.w.where.lat.radians());

          for (i = 0; i < 9; i++)  {
             if(i != 2) {
                xy = SkyProject(lst - p.PlanetObs[i].c.Alpha.value,
                          p.PlanetObs[i].c.Delta.value, coslat, sinlat);
                if(p.mags[i] < 4.7)
                  putDot(xy[0],xy[1],magconst1 + magslope * (4.5 - p.mags[i]),Color.YELLOW);
                else
                  putDot(xy[0],xy[1],magconst1,Color.YELLOW);
                puttext(xy[0],xy[1] + 0.01,p.names[i],smallfont,smallfontmetrics);
             }
          }
       }
/*================================================================================================
/     plotobjects()
/=================================================================================================*/
       void plotobjects() {  // plots out the objects on the user list.
           int i;
           double xy[] = {0.,0.};
           double xypix[] = {0.,0.};
           double ha, dec, lst;
           double magconst1 = 0.003, magslope = 0.002;  //
           AstrObj obj;
           Celest cel;
           double currenteq = o.w.when.JulianEpoch();

           double coslat = Math.cos(o.w.where.lat.radians());
           double sinlat = Math.sin(o.w.where.lat.radians());
           lst = o.w.sidereal;
           g2.setPaint(Color.GREEN);

           for(i = 0; i < presenterKey.size(); i++) {
               // System.out.printf("i %d RASelectors[i] = %s, name = \n",i,RASelectors[i]);
               obj = presenterKey.get(RASelectors[i]);
               if( ! obj.name.equals("null")) {
                   cel = obj.c.precessed(currenteq);
                   xy = SkyProject(lst - cel.Alpha.value, cel.Delta.value, coslat, sinlat);
                   puttext(xy[0],xy[1],obj.name,smallfont,smallfontmetrics);
               }
           }
       }
/*================================================================================================
/      zoom(int xpixin, int ypixin, double zoomfac)
/=================================================================================================*/
       void zoom(int xpixin, int ypixin, double zoomfac) {
            // user hands in pixel coordinates around which to zoom.

            double [] xycent = pixtoxy(xpixin, ypixin);
            xmid = xycent[0];
            ymid = xycent[1];
            halfwidthy = halfwidthy / zoomfac;
            halfwidthx = halfwidthx / zoomfac;
            pixperunit = pixperunit * zoomfac;
            zoomedby = zoomedby * zoomfac;
            currentlat = o.w.where.lat.value;
            makeHAgrid(currentlat);
            repaint();
       }
/*================================================================================================
/     pan(int xpixin, int ypixin)
/=================================================================================================*/
       void pan(int xpixin, int ypixin) {
            // user hands in pixel coordinates around which to zoom.

            double [] xycent = pixtoxy(xpixin, ypixin);
            xmid = xycent[0];
            ymid = xycent[1];
            currentlat = o.w.where.lat.value;
            makeHAgrid(currentlat);
            repaint();
       }
/*================================================================================================
/     restorefull()
/=================================================================================================*/
       void restorefull() {
            xmid = 0.;  ymid = 0.;
            halfwidthx = halfwidthxfull; halfwidthy = halfwidthyfull;
            pixperunit = pixperunitfull;
            zoomedby = 1.;
            currentlat = o.w.where.lat.value;
            makeHAgrid(currentlat);
            repaint();
       }
/*================================================================================================
/      tinyHelp extends JWindow
/=================================================================================================*/
       class tinyHelp extends JWindow {

            final JTextArea area;

            tinyHelp() {
               area = new JTextArea();
               area.append("Middle button or 'c'  - sets to coords\n");
               area.append("Left button or 's' - sets to nearest listed obj\n");
               area.append("'f'  - step time Forward \n");
               area.append("'b'  - step time Backward \n");
               area.append("'z'  - Zoom in at mouse position\n");
               area.append("'o'  - zoom Out at mouse position\n");
               area.append("'p'  - Pan, recenter at mouse position \n");
               area.append("'r'  - Restore original scaling \n");
               area.append("'h'  - Set to nearest HR star.\n");
               area.append("'q' or 'x'  - Hide display window.\n");
               area.append("Keys don't respond unless window has focus.\n");
               area.setBackground(brightyellow);
               this.setSize(275,150);
//               this.setSize(300,220);
               this.add(area);
            }
/*================================================================================================
/      show(int x, int y)
/=================================================================================================*/
            void show(int x, int y) {
               this.setLocation(x, y);
               this.setVisible(true);
            }
/*================================================================================================
/     hide(int x, int y)
/=================================================================================================*/
           void hide(int x, int y) {  // giving it arguments avoids an override
                                       // problem.
               this.setVisible(false);
            }
       }
/*================================================================================================
/      public void synchOutput()
/=================================================================================================*/
    public void synchOutput(){
        // This should call the synchOutput() method of the calling class JSkyCalcWindow
        // it is segregated here so that we only need to either make the reference or propagate
        // the synchOutput event from one location.
       fireActionPerformed(new java.awt.event.ActionEvent(this,SYNCH_OUTPUT,"synchOutput()"));
    }

/*================================================================================================
/      public void synchOutput()
/=================================================================================================*/
    public void advanceTime(){
        // This should call the advanceTime() method of the calling class JSkyCalcWindow
        // it is segregated here so that we only need to either make the reference or propagate
        // the advanceTime() event from one location.
       fireActionPerformed(new java.awt.event.ActionEvent(this,ADVANCE_TIME,"advanceTime()"));
   }
 /*================================================================================================
/      public void synchOutput()
/=================================================================================================*/
    public void advanceTime(boolean forward) {
        // This should call the advanceTime() method of the calling class JSkyCalcWindow
        // it is segregated here so that we only need to either make the reference or propagate 
        // the advanceTime() event from one location. 
     int actionFlag = ADVANCE_TIME_TRUE;
        if(forward){
           actionFlag = ADVANCE_TIME_TRUE;
        }
        if(!forward){
           actionFlag = ADVANCE_TIME_FALSE;
        }
      fireActionPerformed(new java.awt.event.ActionEvent(this,actionFlag,"advanceTime(actionFlag)"));
    }
/*=============================================================================================
/   ActionEvent Handling Code - used to informing registered listeners that a FITS
/      file is ready to be retrieved from the server
/  removeActionListener()
/  addActionListener()
/  fireActionPerformed()
/=============================================================================================*/
/*=============================================================================================
/     synchronized void removeActionListener(ActionListener l)
/=============================================================================================*/
  public synchronized void removeActionListener(ActionListener l) {
    if (actionListeners != null && actionListeners.contains(l)) {
      Vector v = (Vector) actionListeners.clone();
      v.removeElement(l);
      actionListeners = v;
    }
  }
/*=============================================================================================
/    synchronized void addActionListener(ActionListener l)
/=============================================================================================*/
  public synchronized void addActionListener(ActionListener l) {
    Vector v = actionListeners == null ? new Vector(2) : (Vector) actionListeners.clone();
    if (!v.contains(l)) {
      v.addElement(l);
      actionListeners = v;
    }
  }
/*=============================================================================================
/     protected void fireActionPerformed(ActionEvent e)
/=============================================================================================*/
  public void fireActionPerformed(ActionEvent e) {
    if (actionListeners != null) {
      Vector listeners = actionListeners;
      int count = listeners.size();
      for (int i = 0; i < count; i++) {
        ((ActionListener) listeners.elementAt(i)).actionPerformed(e);
      }
    }
}
 }