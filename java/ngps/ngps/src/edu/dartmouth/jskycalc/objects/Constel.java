package edu.dartmouth.jskycalc.objects;  
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
import java.util.List.*;
/*================================================================================================
/
/=================================================================================================*/
public class Constel {
  /** Just a static routine to deliver a constellation. */
  /* This is adapted from an implementation of an algorithm from
     \bibitem[Roman(1987)]{1987PASP...99..695R} Roman, N.~G.\ 1987,
     \pasp, 99, 695.  I adpated the skycalc routine, which was in turn
     written by Francois Ochsenbein of the CDS, Strasbourg.  There are
     big arrays that define the corners of the constellations (in 1875
     coords).
  */
/*================================================================================================
/
/=================================================================================================*/
  public static String getconstel(Celest c) {
      double ra1875, dec1875;
      int i;
      double [] ra1 =
      {0,  0.0000,  120.0000,  315.0000,  270.0000,  0.0000,  137.5000,  0.0000,
      160.0000,  262.5000,  302.5000,  0.0000,  172.5000,  248.0000,  302.5000,  119.5000,
      137.5000,  195.0000,  46.5000,  306.2500,  170.0000,  0.0000,  210.0000,  353.7500,
      180.0000,  202.5000,  347.5000,  91.5000,  300.0000,  308.0500,  105.0000,  119.5000,
      296.5000,  300.0000,  343.0000,  0.0000,  291.2500,  25.5000,  36.5000,  46.5000,
      334.7500,  75.0000,  210.5000,  216.2500,  47.5000,  332.0000,  309.0000,  0.0000,
      91.5000,  181.2500,  228.7500,  329.5000,  50.0000,  343.0000,  236.2500,  30.6250,
      255.0000,  0.0000,  20.5000,  97.5000,  350.0000,  202.5000,  0.0000,  353.7500,
      272.6250,  273.5000,  286.2500,  25.0000,  126.2500,  2.5000,  180.0000,  102.0000,
      328.6250,  328.1250,  287.5000,  137.5000,  152.5000,  231.5000,  236.2500,  138.750,
      0.0000,  37.7500,  290.3750,  67.5000,  326.0000,  328.1250,  98.0000,  110.5000,
      0.0000,  330.0000,  342.2500,  343.0000,  38.5000,  161.7500,  180.0000,  116.2500,
      138.7500,  10.7500,  227.7500,  352.5000,  185.0000,  356.2500,  209.3750,  36.2500,
      40.7500,  67.5000,  272.6250,  165.0000,  295.0000,  71.2500,  148.2500,  198.7500,
      0.0000,  21.1250,  88.2500,  118.2500,  313.7500,  288.8750,  28.7500,  242.5000,
      226.2500,  227.7500,  275.5000,  161.2500,  283.0000,  25.0000,  10.7500,  157.5000,
      318.7500,  85.5000,  1.0000,  238.7500,  88.2500,  297.5000,  283.0000,  2.1250,
      303.7500,  117.1250,  308.5000,  288.7500,  49.2500,  283.0000,  85.5000,  93.2500,
      285.0000,  74.5000,  238.7500,  297.5000,  69.2500,  80.0000,  192.5000,  258.7500,
      178.0000,  112.5000,  251.2500,  0.0000,  84.0000,  105.0000,  316.7500,  94.6250,
      273.7500,  313.1250,  315.7500,  172.7500,  93.6250,  104.0000,  117.1250,  357.5000,
      25.0000,  302.1250,  202.5000,  341.2500,  118.8750,  138.7500,  273.7500,  279.9333,
      312.5000,  105.0000,  273.7500,  241.2500,  273.7500,  322.0000,  0.0000,  278.7500,
      304.5000,  312.5000,  320.0000,  330.0000,  325.0000,  105.2500,  53.7500,  69.2500,
      108.0000,  220.0000,  267.5000,  39.7500,  49.2500,  226.2500,  70.0000,  87.5000,
      267.5000,  273.7500,  278.7500,  341.2500,  161.2500,  172.7500,  0.0000,  357.5000,
      213.7500,  238.7500,  300.0000,  320.0000,  257.5000,  87.5000,  73.7500,  76.2500,
      121.2500,  143.7500,  177.5000,  263.7500,  283.0000,  72.5000,  308.0000,  257.5000,
      273.7500,  125.5000,  244.0000,  128.7500,  161.2500,  244.0000,  235.0000,  188.750,
      192.5000,  136.2500,  25.0000,  39.7500,  162.5000,  177.5000,  213.7500,  244.0000,
      0.0000,  320.0000,  328.0000,  357.5000,  146.2500,  70.5000,  72.5000,  300.0000,
      153.7500,  188.7500,  223.7500,  235.0000,  68.7500,  251.2500,  264.0000,  158.7500,
      91.7500,  183.7500,  162.5000,  52.5000,  125.5000,  64.0000,  267.5000,  320.0000,
      345.0000,  45.0000,  140.5000,  0.0000,  25.0000,  58.0000,  350.0000,  212.5000,
      235.0000,  240.0000,  72.5000,  75.0000,  120.0000,  51.2500,  246.3125,  267.5000,
      287.5000,  305.0000,  45.0000,  67.5000,  230.0000,  0.0000,  40.0000,  61.2500,
      64.0000,  320.0000,  90.0000,  120.0000,  36.2500,  57.5000,  0.0000,  90.0000,
      122.5000,  52.5000,  57.5000,  0.0000,  32.5000,  67.5000,  225.7500,  126.7500,
      92.5000,  177.5000,  212.5000,  225.7500,  60.0000,  132.5000,  165.0000,  262.5000,
      270.0000,  330.0000,  48.0000,  75.0000,  97.5000,  0.0000,  20.0000,  350.0000,
      65.0000,  230.0000,  305.0000,  82.5000,  227.5000,  246.3125,  223.7500,  248.7500,
      90.0000,  102.5000,  168.7500,  177.5000,  192.5000,  202.5000,  251.2500,  32.5000,
      48.0000,  221.2500,  252.5000,  262.5000,  330.0000,  68.7500,  205.0000,  221.2500,
      0.0000,  52.5000,  98.7500,  135.5000,  168.7500,  270.0000,  320.0000,  350.0000,
      11.2500,  0.0000,  115.0000,  205.0000,  52.5000,  0.0000};
/*================================================================================================
/
/=================================================================================================*/
      double [] ra2 =
      {360,  360.0000,  217.5000,  345.0000,  315.0000,  120.0000,  160.0000,   75.0000,
      217.5000,  270.0000,  315.0000,   52.6250,  203.7500,  262.5000,  310.0000,  137.500,
      170.0000,  248.0000,   51.2500,  310.0000,  180.0000,    5.0000,  235.0000,  360.000,
      202.5000,  216.2500,  353.7500,  105.0000,  306.2500,  309.0000,  119.5000,  126.250,
      300.0000,  308.0500,  347.5000,   36.5000,  296.5000,   28.6250,   46.5000,   47.500,
      343.0000,   91.5000,  216.2500,  291.2500,   50.0000,  334.7500,  329.5000,   25.500,
       97.5000,  202.5000,  236.2500,  332.0000,   75.0000,  350.0000,  255.0000,   37.750,
      273.5000,   20.5000,   25.0000,  102.0000,  360.0000,  210.5000,   16.7500,  360.000,
      273.5000,  286.2500,  287.5000,   30.6250,  137.5000,   13.0000,  181.2500,  110.500,
      329.5000,  328.6250,  291.0000,  152.5000,  161.7500,  236.2500,  245.0000,  143.750,
       37.7500,   38.5000,  291.0000,   70.3750,  328.1250,  330.0000,  110.5000,  116.250,
       30.0000,  342.2500,  343.0000,  352.5000,   40.7500,  165.0000,  185.0000,  138.750,
      148.2500,   21.1250,  231.5000,  356.2500,  198.7500,  360.0000,  210.5000,   40.750,
       67.5000,   71.2500,  290.3750,  180.0000,  313.7500,   88.2500,  157.5000,  209.375,
        1.0000,   25.0000,   98.0000,  120.0000,  326.0000,  295.0000,   36.2500,  245.000,
      227.7500,  242.5000,  283.0000,  165.0000,  288.8750,   28.7500,   12.7500,  161.250,
      321.2500,   88.2500,    2.1250,  240.5000,   93.2500,  303.7500,  288.7500,   12.750,
      308.5000,  118.2500,  318.7500,  297.5000,   50.5000,  285.0000,   86.5000,   94.625,
      297.5000,   80.0000,  241.2500,  303.7500,   74.5000,   84.0000,  202.5000,  273.750,
      192.5000,  117.1250,  258.7500,    2.1250,   86.5000,  112.5000,  320.0000,  104.000,
      283.0000,  315.7500,  316.7500,  178.0000,   94.6250,  105.0000,  118.8750,  360.000,
       49.2500,  304.5000,  226.2500,  357.5000,  138.7500,  161.2500,  279.9333,  283.000,
      313.1250,  105.2500,  276.3750,  251.2500,  276.3750,  325.0000,   30.0000,  283.000,
      312.5000,  320.0000,  322.0000,  341.2500,  330.0000,  108.0000,   69.2500,   70.000,
      121.2500,  226.2500,  273.7500,   49.2500,   53.7500,  244.0000,   76.2500,   93.625,
      269.5000,  278.7500,  283.0000,  357.5000,  172.7500,  177.5000,    5.0000,  360.000,
      220.0000,  244.0000,  308.0000,  328.0000,  269.5000,  121.2500,   76.2500,   87.500,
      125.5000,  161.2500,  192.5000,  265.0000,  300.0000,   73.7500,  320.0000,  273.750,
      283.0000,  128.7500,  245.6250,  136.2500,  162.5000,  245.6250,  238.7500,  192.500,
      213.7500,  146.2500,   39.7500,   56.2500,  177.5000,  188.7500,  223.7500,  251.250,
       25.0000,  328.0000,  357.5000,  360.0000,  153.7500,   72.5000,   91.7500,  320.000,
      158.7500,  223.7500,  235.0000,  240.0000,   70.5000,  264.0000,  267.5000,  162.500,
      110.5000,  188.7500,  183.7500,   56.2500,  140.5000,   68.7500,  287.5000,  345.000,
      350.0000,   52.5000,  165.0000,   25.0000,   45.0000,   64.0000,  360.0000,  223.750,
      240.0000,  246.3125,   75.0000,   98.7500,  125.5000,   58.0000,  267.5000,  287.500,
      305.0000,  320.0000,   51.2500,   72.5000,  235.0000,   35.0000,   45.0000,   64.000,
       67.5000,  330.0000,  120.0000,  122.5000,   40.0000,   61.2500,   27.5000,   92.500,
      126.7500,   57.5000,   60.0000,   23.7500,   36.2500,   75.0000,  230.0000,  132.500,
       97.5000,  192.5000,  225.7500,  230.0000,   65.0000,  165.0000,  168.7500,  270.000,
      305.0000,  350.0000,   52.5000,   82.5000,  102.5000,   20.0000,   32.5000,  360.000,
       68.7500,  246.3125,  320.0000,   90.0000,  230.0000,  248.7500,  227.5000,  251.250,
      102.5000,  135.5000,  177.5000,  192.5000,  218.0000,  205.0000,  252.5000,   48.000,
       68.7500,  223.7500,  262.5000,  270.0000,  350.0000,   98.7500,  221.2500,  255.000,
       20.0000,   68.7500,  135.5000,  168.7500,  205.0000,  320.0000,  350.0000,  360.000,
       20.0000,   52.5000,  205.0000,  270.0000,  115.0000,  360.0000};
/*================================================================================================
/
/=================================================================================================*/
      double [] decs =
       {90,   88.0000,   86.5000,   86.1667,   86.0000,   85.0000,   82.0000,   80.0000,
       80.0000,   80.0000,   80.0000,   77.0000,   77.0000,   75.0000,   75.0000,   73.500,
       73.5000,   70.0000,   68.0000,   67.0000,   66.5000,   66.0000,   66.0000,   66.000,
       64.0000,   63.0000,   63.0000,   62.0000,   61.5000,   60.9167,   60.0000,   60.000,
       59.5000,   59.5000,   59.0833,   58.5000,   58.0000,   57.5000,   57.0000,   57.000,
       56.2500,   56.0000,   55.5000,   55.5000,   55.0000,   55.0000,   54.8333,   54.000,
       54.0000,   53.0000,   53.0000,   52.7500,   52.5000,   52.5000,   51.5000,   50.500,
       50.5000,   50.0000,   50.0000,   50.0000,   50.0000,   48.5000,   48.0000,   48.000,
       47.5000,   47.5000,   47.5000,   47.0000,   47.0000,   46.0000,   45.0000,   44.500,
       44.0000,   43.7500,   43.5000,   42.0000,   40.0000,   40.0000,   40.0000,   39.750,
       36.7500,   36.7500,   36.5000,   36.0000,   36.0000,   36.0000,   35.5000,   35.500,
       35.0000,   35.0000,   34.5000,   34.5000,   34.0000,   34.0000,   34.0000,   33.500,
       33.5000,   33.0000,   33.0000,   32.0833,   32.0000,   31.3333,   30.7500,   30.666,
       30.6667,   30.0000,   30.0000,   29.0000,   29.0000,   28.5000,   28.5000,   28.500,
       28.0000,   28.0000,   28.0000,   28.0000,   28.0000,   27.5000,   27.2500,   27.000,
       26.0000,   26.0000,   26.0000,   25.5000,   25.5000,   25.0000,   23.7500,   23.500,
       23.5000,   22.8333,   22.0000,   22.0000,   21.5000,   21.2500,   21.0833,   21.000,
       20.5000,   20.0000,   19.5000,   19.1667,   19.0000,   18.5000,   18.0000,   17.500,
       16.1667,   16.0000,   16.0000,   15.7500,   15.5000,   15.5000,   15.0000,   14.333,
       14.0000,   13.5000,   12.8333,   12.5000,   12.5000,   12.5000,   12.5000,   12.000,
       12.0000,   11.8333,   11.8333,   11.0000,   10.0000,   10.0000,   10.0000,   10.000,
        9.9167,    8.5000,    8.0000,    7.5000,    7.0000,    7.0000,    6.2500,    6.250,
        6.0000,    5.5000,    4.5000,    4.0000,    3.0000,    2.7500,    2.0000,    2.000,
        2.0000,    2.0000,    2.0000,    2.0000,    1.7500,    1.5000,    0.0000,    0.000,
        0.0000,    0.0000,    0.0000,   -1.7500,   -1.7500,   -3.2500,   -4.0000,   -4.000,
       -4.0000,   -4.0000,   -4.0000,   -4.0000,   -6.0000,   -6.0000,   -7.0000,   -7.000,
       -8.0000,   -8.0000,   -9.0000,   -9.0000,  -10.0000,  -11.0000,  -11.0000,  -11.000,
      -11.0000,  -11.0000,  -11.0000,  -11.6667,  -12.0333,  -14.5000,  -15.0000,  -16.000,
      -16.0000,  -17.0000,  -18.2500,  -19.0000,  -19.0000,  -19.2500,  -20.0000,  -22.000,
      -22.0000,  -24.0000,  -24.3833,  -24.3833,  -24.5000,  -24.5000,  -24.5000,  -24.583,
      -25.5000,  -25.5000,  -25.5000,  -25.5000,  -26.5000,  -27.2500,  -27.2500,  -28.000,
      -29.1667,  -29.5000,  -29.5000,  -29.5000,  -30.0000,  -30.0000,  -30.0000,  -31.166,
      -33.0000,  -33.0000,  -35.0000,  -36.0000,  -36.7500,  -37.0000,  -37.0000,  -37.000,
      -37.0000,  -39.5833,  -39.7500,  -40.0000,  -40.0000,  -40.0000,  -40.0000,  -42.000,
      -42.0000,  -42.0000,  -43.0000,  -43.0000,  -43.0000,  -44.0000,  -45.5000,  -45.500,
      -45.5000,  -45.5000,  -46.0000,  -46.5000,  -48.0000,  -48.1667,  -49.0000,  -49.000,
      -49.0000,  -50.0000,  -50.7500,  -50.7500,  -51.0000,  -51.0000,  -51.5000,  -52.500,
      -53.0000,  -53.1667,  -53.1667,  -53.5000,  -54.0000,  -54.0000,  -54.0000,  -54.500,
      -55.0000,  -55.0000,  -55.0000,  -55.0000,  -56.5000,  -56.5000,  -56.5000,  -57.000,
      -57.0000,  -57.0000,  -57.5000,  -57.5000,  -58.0000,  -58.5000,  -58.5000,  -58.500,
      -59.0000,  -60.0000,  -60.0000,  -61.0000,  -61.0000,  -61.0000,  -63.5833,  -63.583,
      -64.0000,  -64.0000,  -64.0000,  -64.0000,  -64.0000,  -65.0000,  -65.0000,  -67.500,
      -67.5000,  -67.5000,  -67.5000,  -67.5000,  -67.5000,  -70.0000,  -70.0000,  -70.000,
      -75.0000,  -75.0000,  -75.0000,  -75.0000,  -75.0000,  -75.0000,  -75.0000,  -75.000,
      -76.0000,  -82.5000,  -82.5000,  -82.5000,  -85.0000,  -90.0000};
/*================================================================================================
/
/=================================================================================================*/
      String [] abbrevs =
      { " " , "UMi" , "UMi" , "UMi" , "UMi" , "Cep" , "Cam" , "Cep" ,
      "Cam" , "UMi" , "Dra" , "Cep" , "Cam" , "UMi" , "Cep" , "Cam" ,
      "Dra" , "UMi" , "Cas" , "Dra" , "Dra" , "Cep" , "UMi" , "Cep" ,
      "Dra" , "Dra" , "Cep" , "Cam" , "Dra" , "Cep" , "Cam" , "UMa" ,
      "Dra" , "Cep" , "Cep" , "Cas" , "Dra" , "Cas" , "Cas" , "Cam" ,
      "Cep" , "Cam" , "UMa" , "Dra" , "Cam" , "Cep" , "Cep" , "Cas" ,
      "Lyn" , "UMa" , "Dra" , "Cep" , "Cam" , "Cas" , "Dra" , "Per" ,
      "Dra" , "Cas" , "Per" , "Lyn" , "Cas" , "UMa" , "Cas" , "Cas" ,
      "Her" , "Dra" , "Cyg" , "Per" , "UMa" , "Cas" , "UMa" , "Lyn" ,
      "Cyg" , "Cyg" , "Cyg" , "UMa" , "UMa" , "Boo" , "Her" , "Lyn" ,
      "And" , "Per" , "Lyr" , "Per" , "Cyg" , "Lac" , "Aur" , "Lyn" ,
      "And" , "Lac" , "Lac" , "And" , "Per" , "UMa" , "CVn" , "Lyn" ,
      "LMi" , "And" , "Boo" , "And" , "CVn" , "And" , "CVn" , "Tri" ,
      "Per" , "Aur" , "Lyr" , "UMa" , "Cyg" , "Aur" , "LMi" , "CVn" ,
      "And" , "Tri" , "Aur" , "Gem" , "Cyg" , "Cyg" , "Tri" , "CrB" ,
      "Boo" , "CrB" , "Lyr" , "LMi" , "Lyr" , "Tri" , "Psc" , "LMi" ,
      "Vul" , "Tau" , "And" , "Ser" , "Gem" , "Vul" , "Vul" , "And" ,
      "Vul" , "Gem" , "Vul" , "Vul" , "Ari" , "Sge" , "Ori" , "Gem" ,
      "Sge" , "Tau" , "Her" , "Sge" , "Tau" , "Tau" , "Com" , "Her" ,
      "Com" , "Gem" , "Her" , "Peg" , "Tau" , "Gem" , "Peg" , "Gem" ,
      "Her" , "Del" , "Peg" , "Leo" , "Ori" , "Gem" , "Cnc" , "Peg" ,
      "Ari" , "Del" , "Boo" , "Peg" , "Cnc" , "Leo" , "Oph" , "Aql" ,
      "Del" , "CMi" , "Ser" , "Her" , "Oph" , "Peg" , "Psc" , "Ser" ,
      "Del" , "Equ" , "Peg" , "Peg" , "Peg" , "CMi" , "Tau" , "Ori" ,
      "CMi" , "Vir" , "Oph" , "Cet" , "Tau" , "Ser" , "Ori" , "Ori" ,
      "Ser" , "Ser" , "Aql" , "Psc" , "Leo" , "Vir" , "Psc" , "Psc" ,
      "Vir" , "Oph" , "Aql" , "Aqr" , "Oph" , "Mon" , "Eri" , "Ori" ,
      "Hya" , "Sex" , "Vir" , "Oph" , "Aql" , "Eri" , "Aqr" , "Ser" ,
      "Sct" , "Hya" , "Oph" , "Hya" , "Crt" , "Sco" , "Lib" , "Crv" ,
      "Vir" , "Hya" , "Cet" , "Eri" , "Crt" , "Crv" , "Lib" , "Oph" ,
      "Cet" , "Cap" , "Aqr" , "Cet" , "Hya" , "Eri" , "Lep" , "Cap" ,
      "Hya" , "Hya" , "Lib" , "Sco" , "Eri" , "Oph" , "Sgr" , "Hya" ,
      "CMa" , "Hya" , "Hya" , "For" , "Pyx" , "Eri" , "Sgr" , "PsA" ,
      "Scl" , "For" , "Ant" , "Scl" , "For" , "Eri" , "Scl" , "Cen" ,
      "Lup" , "Sco" , "Cae" , "Col" , "Pup" , "Eri" , "Sco" , "CrA" ,
      "Sgr" , "Mic" , "Eri" , "Cae" , "Lup" , "Phe" , "Eri" , "Hor" ,
      "Cae" , "Gru" , "Pup" , "Vel" , "Eri" , "Hor" , "Phe" , "Car" ,
      "Vel" , "Hor" , "Dor" , "Phe" , "Eri" , "Pic" , "Lup" , "Vel" ,
      "Car" , "Cen" , "Lup" , "Nor" , "Dor" , "Vel" , "Cen" , "Ara" ,
      "Tel" , "Gru" , "Hor" , "Pic" , "Car" , "Phe" , "Eri" , "Phe" ,
      "Dor" , "Nor" , "Ind" , "Pic" , "Cir" , "Ara" , "Cir" , "Ara" ,
      "Pic" , "Car" , "Cen" , "Cru" , "Cen" , "Cir" , "Ara" , "Hor" ,
      "Ret" , "Cir" , "Ara" , "Pav" , "Tuc" , "Dor" , "Cir" , "TrA" ,
      "Tuc" , "Hyi" , "Vol" , "Car" , "Mus" , "Pav" , "Ind" , "Tuc" ,
      "Tuc" , "Hyi" , "Cha" , "Aps" , "Men" , "Oct" };

       Celest c1875 = c.precessed(1875.);
       ra1875 = c1875.Alpha.degrees();
       dec1875 = c1875.Delta.degrees();
       i = 0;
       while(i < ra1.length) {
           if((ra1875 >= ra1[i]) && (ra1875 < ra2[i]) && (dec1875 >= decs[i])) break;
           i++;
       }
       if (i < ra1.length) return (abbrevs[i]);
       else return "???";
   }
}