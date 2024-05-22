//=== File Prolog =============================================================
//	This code was developed by NASA, Goddard Space Flight Center, Code 588
//	for the Scientist's Expert Assistant (SEA) project.
//
//--- Contents ----------------------------------------------------------------
//	class WCSTransform
//
//--- Description -------------------------------------------------------------
//	A port of pieces of the WCSTools library version 2.6, originally written in C.
//	WCSTranform is the main class. It can construct a proper transform
//	using a nom.tam.fits.Header object. The methods pix2wcs and wcs2pix
//	do the actual conversions between image pixel values and WCS coordinates.
//
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
//
//	07/16/98	J. Jones / 588
//
//		Original implementation.
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
//	In no event shall NASA be liable for any damages, including, but not
//	limited to direct, indirect, special or consequential damages, arising out
//	of, resulting from, or in any way connected with this software, whether or
//	not based upon warranty, contract, tort or otherwise, whether or not
//	injury was sustained by persons or property or otherwise, and whether or
//	not loss was sustained from or arose out of the results of, or use of,
//	their software or services provided hereunder.
//
//=== End File Prolog =========================================================

package jsky.coords;

import java.awt.geom.Point2D;

import nom.tam.fits.Header;
import nom.tam.fits.HeaderCardException;


/**
 * A port of pieces of the WCSTools library version 2.6, originally written in C.
 * WCSTranform is the main class. It can construct a proper transform
 * using a WCSKeywordProvider object. The methods pix2wcs and wcs2pix
 * do the actual conversions between image pixel values and WCS coordinates.
 * <p/>
 * <P>In porting the code to Java, I attempted to change as little as possible,
 * hoping that this would make it easier to keep up-to-date with the C version.
 * <p/>
 * <P>See the <A HREF="http://tdc-www.harvard.edu/software/wcstools/">WCSTools page</A>
 * for more information.
 * <p/>
 * <P>Original file:
 * <BR>libwcs/wcs.c
 * <BR>February 6, 1998
 * <BR>By Doug Mink, Harvard-Smithsonian Center for Astrophysics
 * <p/>
 * <P>This code was developed by NASA, Goddard Space Flight Center, Code 588
 * for the Scientist's Expert Assistant (SEA) project.
 *
 * @author J. Jones / 588
 * @author A. Brighton (changes for JSky)
 * @version $Revision: 1.3 $
 */
public class WCSTransform implements WorldCoordinateConverter {

    double xref;        // X reference coordinate value (deg)
    double yref;        // Y reference coordinate value (deg)
    double xrefpix;    // X reference pixel
    double yrefpix;    // Y reference pixel
    double xinc;        // X coordinate increment (deg)
    double yinc;        // Y coordinate increment (deg)
    double rot;        // rotation around opt. axis (deg) (N through E)
    double crot, srot;    // Cosine and sine of rotation angle
    double cd11, cd12, cd21, cd22; // rotation matrix
    double dc11, dc12, dc21, dc22; // inverse rotation matrix
    double mrot;        // Chip rotation angle (deg) (N through E)
    double cmrot, smrot;    // Cosine and sine of chip rotation angle
    double xmpix, ympix;    // X and Y center for chip rotation
    double equinox;    // Equinox of coordinates default to 1950.0
    double epoch;        // Epoch of coordinates default to equinox
    double nxpix;        // Number of pixels in X-dimension of image
    double nypix;        // Number of pixels in Y-dimension of image
    double plate_ra;    // Right ascension of plate center
    double plate_dec;    // Declination of plate center
    double plate_scale;    // Plate scale in arcsec/mm
    double x_pixel_offset;    // X pixel offset of image lower right
    double y_pixel_offset;    // Y pixel offset of image lower right
    double x_pixel_size;    // X pixel_size
    double y_pixel_size;    // Y pixel_size
    double ppo_coeff[] = new double[6];
    double amd_x_coeff[] = new double[20]; // X coefficients for plate model
    double amd_y_coeff[] = new double[20]; // Y coefficients for plate model
    double xpix;        // X (RA) coordinate (pixels)
    double ypix;        // Y (dec) coordinate (pixels)
    double xpos;        // X (RA) coordinate (deg)
    double ypos;        // Y (dec) coordinate (deg)
    int pcode;        // projection code (-1-8)
    int changesys;    // 1 for FK4->FK5, 2 for FK5->FK4
    // 3 for FK4->galactic, 4 for FK5->galactic
    int printsys;    // 1 to print coordinate system, else 0
    int ndec;        // Number of decimal places in PIX2WCST
    int degout;        // 1 to always print degrees in PIX2WCST
    int tabsys;        // 1 to put tab between RA & Dec, else 0
    int rotmat;        // 0 if CDELT, CROTA; 1 if CD
    int coorflip;    // 0 if x=RA, y=Dec; 1 if x=Dec, y=RA
    int offscl;        // 0 if OK, 1 if offscale
    int plate_fit;    // 1 if plate fit, else 0
    int wcson;        // 1 if WCS is set, else 0
    int detector;    // Instrument detector number
    String instrument = "";// Instrument name
    String c1type = "";    //  1st coordinate type code: RA--, GLON, ELON
    String c2type = "";    //  2nd coordinate type code: DEC-, GLAT, ELAT
    String[] ctypes = {"-SIN", "-TAN", "-ARC", "-NCP", "-GLS", "-MER", "-AIT", "-STG"};
    String ptype = "";    //  projection type code (on of the ctype values)
    String radecsys = "";    // Reference frame: FK4, FK4-NO-E, FK5, GAPPT
    String sysout = "";    // Reference frame for output: FK4, FK5
    String center = "";    // Center coordinates (with frame)

    // These fields added for convenience
    double fCenterRa;
    double fCenterDec;
    double fHalfWidthRa;
    double fHalfWidthDec;
    double fWidthDeg;
    double fHeightDeg;

    // allan: added for quick access to degrees per pixel
    Point2D.Double degPerPixel;

    /**
     * Message if header does not contain a valid World Coordinate System
     */
    public static final String NO_WCS_IN_HEADER_MESSAGE // 00/23/06 Added for OPR 41378, jDoggett
            = "The header does not contain a valid world coordinate system.".intern();

    /**
     * Conversions among hours of RA, degrees and radians.
     */
    public static double degrad(double x) {
        return ((x) * Math.PI / 180.0);
    }

    public static double raddeg(double x) {
        return ((x) * 180.0 / Math.PI);
    }

    public static double hrdeg(double x) {
        return (x * 15.0);
    }

    public static double deghr(double x) {
        return (x / 15.0);
    }

    public static double hrrad(double x) {
        return (degrad(hrdeg(x)));
    }

    public static double radhr(double x) {
        return (deghr(raddeg(x)));
    }

    /**
     * Constructs a new WCSTransform using data from the specified FITS Header.
     *
     * @param head provides the required WCS keywords and associated values
     * @throws IllegalArgumentException if the header does not contain a valid World Coordinate System.
     */
    public WCSTransform(WCSKeywordProvider head) {
        super();

        // 00/23/06 IllegalArgumentExceptions now have text (NO_WCS_IN_HEADER_MESSAGE)
        // for OPR 41378, jDoggett

        // set up a WCS structure from a FITS image header

        String wcstemp;
        char decsign;
        double rah, ram, ras, dsign, decd, decm, decs;
        double dec_deg, ra_hours, secpix, cddet;
        int ieq;

        // Plate solution coefficients
        this.plate_fit = 0;
        this.nxpix = head.getDoubleValue("NAXIS1");
        this.nypix = head.getDoubleValue("NAXIS2");
        this.xmpix = 0.5 * this.nxpix;
        this.ympix = 0.5 * this.nypix;
        this.mrot = 0.0;

        this.equinox = head.getDoubleValue("EQUINOX");

        if (head.findKey("PLTRAH")) {
            this.plate_fit = 1;

            rah = head.getDoubleValue("PLTRAH");
            ram = head.getDoubleValue("PLTRAM");
            ras = head.getDoubleValue("PLTRAS");

            ra_hours = rah + (ram / 60.0) + (ras / 3600.0);
            this.plate_ra = hrrad(ra_hours);

            decsign = '+';
            String signString = head.getStringValue("PLTDECSN");
            if (signString != null) {
                decsign = signString.charAt(0);
            }
            if (decsign == '-') {
                dsign = -1.0;
            } else {
                dsign = 1.0;
            }

            decd = head.getDoubleValue("PLTDECD");
            decm = head.getDoubleValue("PLTDECM");
            decs = head.getDoubleValue("PLTDECS");

            dec_deg = dsign * (decd + (decm / 60.0) + (decs / 3600.0));
            this.plate_dec = degrad(dec_deg);

            ieq = (int) this.equinox;
            if (ieq == 1950) {
                this.radecsys = "FK4";
            } else {
                this.radecsys = "FK5";
            }

            this.epoch = this.equinox;
            if (head.findKey("EPOCH")) {
                this.epoch = head.getDoubleValue("EPOCH");
            }

            this.plate_scale = head.getDoubleValue("PLTSCALE");
            this.x_pixel_size = head.getDoubleValue("XPIXELSZ");
            this.y_pixel_size = head.getDoubleValue("YPIXELSZ");
            this.x_pixel_offset = head.getDoubleValue("CNPIX1");
            this.y_pixel_offset = head.getDoubleValue("CNPIX2");
            this.ppo_coeff[0] = head.getDoubleValue("PPO1");
            this.ppo_coeff[1] = head.getDoubleValue("PPO2");
            this.ppo_coeff[2] = head.getDoubleValue("PPO3");
            this.ppo_coeff[3] = head.getDoubleValue("PPO4");
            this.ppo_coeff[4] = head.getDoubleValue("PPO5");
            this.ppo_coeff[5] = head.getDoubleValue("PPO6");
            this.amd_x_coeff[0] = head.getDoubleValue("AMDX1");
            this.amd_x_coeff[1] = head.getDoubleValue("AMDX2");
            this.amd_x_coeff[2] = head.getDoubleValue("AMDX3");
            this.amd_x_coeff[3] = head.getDoubleValue("AMDX4");
            this.amd_x_coeff[4] = head.getDoubleValue("AMDX5");
            this.amd_x_coeff[5] = head.getDoubleValue("AMDX6");
            this.amd_x_coeff[6] = head.getDoubleValue("AMDX7");
            this.amd_x_coeff[7] = head.getDoubleValue("AMDX8");
            this.amd_x_coeff[8] = head.getDoubleValue("AMDX9");
            this.amd_x_coeff[9] = head.getDoubleValue("AMDX10");
            this.amd_x_coeff[10] = head.getDoubleValue("AMDX11");
            this.amd_x_coeff[11] = head.getDoubleValue("AMDX12");
            this.amd_x_coeff[12] = head.getDoubleValue("AMDX13");
            this.amd_x_coeff[13] = head.getDoubleValue("AMDX14");
            this.amd_x_coeff[14] = head.getDoubleValue("AMDX15");
            this.amd_x_coeff[15] = head.getDoubleValue("AMDX16");
            this.amd_x_coeff[16] = head.getDoubleValue("AMDX17");
            this.amd_x_coeff[17] = head.getDoubleValue("AMDX18");
            this.amd_x_coeff[18] = head.getDoubleValue("AMDX19");
            this.amd_x_coeff[19] = head.getDoubleValue("AMDX20");
            this.amd_y_coeff[0] = head.getDoubleValue("AMDY1");
            this.amd_y_coeff[1] = head.getDoubleValue("AMDY2");
            this.amd_y_coeff[2] = head.getDoubleValue("AMDY3");
            this.amd_y_coeff[3] = head.getDoubleValue("AMDY4");
            this.amd_y_coeff[4] = head.getDoubleValue("AMDY5");
            this.amd_y_coeff[5] = head.getDoubleValue("AMDY6");
            this.amd_y_coeff[6] = head.getDoubleValue("AMDY7");
            this.amd_y_coeff[7] = head.getDoubleValue("AMDY8");
            this.amd_y_coeff[8] = head.getDoubleValue("AMDY9");
            this.amd_y_coeff[9] = head.getDoubleValue("AMDY10");
            this.amd_y_coeff[10] = head.getDoubleValue("AMDY11");
            this.amd_y_coeff[11] = head.getDoubleValue("AMDY12");
            this.amd_y_coeff[12] = head.getDoubleValue("AMDY13");
            this.amd_y_coeff[13] = head.getDoubleValue("AMDY14");
            this.amd_y_coeff[14] = head.getDoubleValue("AMDY15");
            this.amd_y_coeff[15] = head.getDoubleValue("AMDY16");
            this.amd_y_coeff[16] = head.getDoubleValue("AMDY17");
            this.amd_y_coeff[17] = head.getDoubleValue("AMDY18");
            this.amd_y_coeff[18] = head.getDoubleValue("AMDY19");
            this.amd_y_coeff[19] = head.getDoubleValue("AMDY20");

            this.wcson = 1;
            this.c1type = "RA-";
            this.c2type = "DEC";
            this.ptype = "PLATE";
            this.degout = 0;
            this.ndec = 3;
        } else if ((wcstemp = head.getStringValue("CTYPE1")) != null) {
            // World coordinate system reference coordinate information
            int wcstempLength = wcstemp.length();

            // Deal appropriately with linear coordinates
            if (wcstemp.startsWith("LINEAR")) {
                this.pcode = 0;
                this.c1type = wcstemp;
                this.ptype = wcstemp;
            }

            // Deal appropriately with pixel coordinates
            else if (wcstemp.startsWith("PIXEL")) {
                this.pcode = -1;
                this.c1type = wcstemp;
                this.ptype = wcstemp;
            }

            // Set up right ascension, declination, latitude, or longitude
            else if (wcstempLength > 1 &&
                    (wcstemp.charAt(0) == 'R' ||
                            wcstemp.charAt(0) == 'D' ||
                            wcstemp.charAt(0) == 'A' ||
                            wcstemp.charAt(1) == 'L')) {
                this.c1type = wcstemp.substring(0, 2);
                if (wcstempLength > 2 && wcstemp.charAt(2) != '-') {
                    this.c1type += wcstemp.charAt(2);
                }
                if (wcstempLength > 3 && wcstemp.charAt(3) != '-') {
                    this.c1type += wcstemp.charAt(3);
                }
                if (wcstempLength > 4) {
                    // Chris S. changed this -- is it a zero- or one-based issue?
                    //this.ptype = wcstemp.substring(4, 8);
                    this.ptype = wcstemp.substring(3, 7);
                }
                //  Find projection type
                this.pcode = 0;  // default type is linear
                for (int i = 0; i < 8; i++) {
                    if (this.ptype.startsWith(ctypes[i])) {
                        this.pcode = i + 1;
                        break;
                    }
                }
            }

            // If not linear or sky coordinates, drop out with error message
            else {
                System.err.println("WCSINIT: CTYPE1 not sky coordinates or LINEAR -> no WCS");

                throw new IllegalArgumentException();
            }

            // Second coordinate type
            if ((wcstemp = head.getStringValue("CTYPE2")) == null) {
                System.err.println("WCSINIT: No CTYPE2 -> no WCS");

                throw new IllegalArgumentException(NO_WCS_IN_HEADER_MESSAGE);
            }

            wcstempLength = wcstemp.length();

            // Deal appropriately with linear coordinates
            if (wcstemp.startsWith("LINEAR")) {
                this.pcode = 0;
                this.c2type = wcstemp;
            }

            // Deal appropriately with pixel coordinates
            else if (wcstemp.startsWith("PIXEL")) {
                this.pcode = -1;
                this.c2type = wcstemp;
            }

            // Set up right ascension, declination, latitude, or longitude
            else if (wcstempLength > 1 &&
                    (wcstemp.charAt(0) == 'R' ||
                            wcstemp.charAt(0) == 'D' ||
                            wcstemp.charAt(0) == 'A' ||
                            wcstemp.charAt(1) == 'L')) {
                this.c2type = wcstemp.substring(0, 2);
                if (wcstempLength > 2 && wcstemp.charAt(2) != '-') {
                    this.c2type += wcstemp.charAt(2);
                }
                if (wcstempLength > 3 && wcstemp.charAt(3) != '-') {
                    this.c2type += wcstemp.charAt(3);
                }

                if (this.c1type.startsWith("DEC") ||
                        this.c1type.startsWith("GLAT")) {
                    this.coorflip = 1;
                } else {
                    this.coorflip = 0;
                }

                if (wcstemp.charAt(1) == 'L' || wcstemp.charAt(0) == 'A') {
                    this.degout = 1;
                    this.ndec = 5;
                } else {
                    this.degout = 0;
                    this.ndec = 3;
                }
            }

            // If not linear or sky coordinates, drop out with error message
            else {
                System.err.println("WCSINIT: CTYPE2 not sky coordinates or LINEAR -> no WCS");

                throw new IllegalArgumentException(NO_WCS_IN_HEADER_MESSAGE);
            }

            // Reference pixel coordinates and WCS value
            this.xrefpix = head.getDoubleValue("CRPIX1", 1.0);
            this.yrefpix = head.getDoubleValue("CRPIX2", 1.0);
            this.xref = head.getDoubleValue("CRVAL1", 1.0);
            this.yref = head.getDoubleValue("CRVAL2", 1.0);
            if ((this.xinc = head.getDoubleValue("CDELT1")) != 0) {
                this.yinc = head.getDoubleValue("CDELT2", this.xinc);
                this.rot = head.getDoubleValue("CROTA1", 0.0);
                if (this.rot == 0.0) {
                    this.rot = head.getDoubleValue("CROTA2");
                }
                this.cd11 = 0.0;
                this.cd21 = 0.0;
                this.cd12 = 0.0;
                this.cd22 = 0.0;
                this.rotmat = 0;
            } else if ((this.cd11 = head.getDoubleValue("CD1_1")) != 0) {
                this.rotmat = 1;
                this.cd12 = head.getDoubleValue("CD1_2", 0.0);
                this.cd21 = head.getDoubleValue("CD2_1", 0.0);
                this.cd22 = head.getDoubleValue("CD2_2", this.cd11);
                cddet = (this.cd11 * this.cd22) - (this.cd12 * this.cd21);
                if (cddet != 0.0) {
                    this.dc11 = this.cd22 / cddet;
                    this.dc12 = -this.cd12 / cddet;
                    this.dc21 = -this.cd21 / cddet;
                    this.dc22 = this.cd11 / cddet;
                }
                this.xinc = Math.sqrt(this.cd11 * this.cd11 + this.cd21 * this.cd21);
                this.yinc = Math.sqrt(this.cd12 * this.cd12 + this.cd22 * this.cd22);
                if ((this.cd11 * this.cd11 - this.cd12 * this.cd12) < 0) {
                    if ((this.c1type.startsWith("RA")) || (this.c1type.startsWith("GLON"))) {
                        this.xinc = -this.xinc;
                    }

                    if ((this.c2type.startsWith("RA")) || (this.c2type.startsWith("GLON"))) {
                        this.yinc = -this.yinc;
                    }

                    this.rot = raddeg(Math.atan2(-this.cd12, this.cd22));
                } else {
                    this.rot = raddeg(Math.atan2(this.cd12, this.cd22));
                }
            } else {
                this.xinc = 1.0;
                this.yinc = 1.0;

                System.err.println("WCSINIT: setting CDELT to 1");
            }

            // Chip rotation
            this.xmpix = head.getDoubleValue("CCPIX1");
            this.ympix = head.getDoubleValue("CCPIX2");
            this.mrot = head.getDoubleValue("CCROT1");

            // Coordinate reference frame, equinox, and epoch
            if (this.ptype.startsWith("LINEAR") && this.ptype.startsWith("PIXEL")) {
                this.degout = -1;
            } else {
                wcseq(head);
            }

            this.wcson = 1;
        }

        // Approximate world coordinate system if plate scale is known
        else if (head.findKey("SECPIX") || head.findKey("PIXSCAL1") || head.findKey("SECPIX1")) {
            secpix = head.getDoubleValue("SECPIX", 0.0);
            if (secpix == 0.0) {
                secpix = head.getDoubleValue("SECPIX1", 0.0);
                if (secpix != 0.0) {
                    this.xinc = -secpix / 3600.0;
                    secpix = head.getDoubleValue("SECPIX2", 0.0);
                    this.yinc = secpix / 3600.0;
                } else {
                    secpix = head.getDoubleValue("PIXSCAL1", 0.0);
                    this.xinc = -secpix / 3600.0;
                    secpix = head.getDoubleValue("PIXSCAL2", 0.0);
                    this.yinc = secpix / 3600.0;
                }
            } else {
                this.yinc = secpix / 3600.0;
                this.xinc = -this.yinc;
            }

            this.xrefpix = head.getDoubleValue("CRPIX1", this.nxpix * 0.5);
            this.yrefpix = head.getDoubleValue("CRPIX2", this.nypix * 0.5);

            this.xref = 0.0;

            if ((this.xref = head.getDoubleValue("RA", 0.0)) == 0.0) {
                // TBD: should be version of "hgetra"

                System.err.println("WCSINIT: No RA with SECPIX, no WCS");

                throw new IllegalArgumentException(NO_WCS_IN_HEADER_MESSAGE);
            }

            this.yref = 0.0;
            if ((this.yref = head.getDoubleValue("DEC", 0.0)) == 0.0) {
                // TBD: should be version of "hgetdec"

                System.err.println("WCSINIT: No DEC with SECPIX, no WCS");

                throw new IllegalArgumentException(NO_WCS_IN_HEADER_MESSAGE);
            }

            this.c1type = "RA--TAN";
            this.c2type = "DEC-TAN";
            this.ptype = "-TAN";
            this.pcode = 1;
            this.coorflip = 0;
            this.degout = 0;
            this.ndec = 3;
            this.rot = head.getDoubleValue("CROTA1", 0.0);
            if (this.rot == 0.0) {
                this.rot = head.getDoubleValue("CROTA2", 0.0);
            }
            this.cd11 = 0.0;
            this.cd21 = 0.0;
            this.cd12 = 0.0;
            this.cd22 = 0.0;
            this.dc11 = 0.0;
            this.dc21 = 0.0;
            this.dc12 = 0.0;
            this.dc22 = 0.0;
            this.rotmat = 0;

            // Chip rotation
            this.xmpix = head.getDoubleValue("CCPIX1");
            this.ympix = head.getDoubleValue("CCPIX2");
            this.mrot = head.getDoubleValue("CCROT1");

            // Coordinate reference frame and equinox
            wcseq(head);

            // Epoch of image (from observation date, if possible)
            if ((this.epoch = head.getDoubleValue("DATE-OBS", 0.0)) == 0.0) {
                // TBD: Should be version of "hgetdate"

                if ((this.epoch = head.getDoubleValue("EPOCH", 0.0)) == 0.0) {
                    this.epoch = this.equinox;
                }
            }

            this.wcson = 1;
        } else {
            throw new IllegalArgumentException(NO_WCS_IN_HEADER_MESSAGE);
        }

        this.sysout = this.radecsys;
        this.changesys = 0;
        this.printsys = 1;
        this.tabsys = 0;
        wcsfull();                // allan: added this line 4/24/00
    }

    /**
     * Constructs a new WCSTransform.
     *
     * @param cra     Center right ascension in degrees
     * @param cdec    Center declination in degrees
     * @param xsecpix Number of arcseconds per pixel along x-axis
     * @param ysecpix Number of arcseconds per pixel along y-axis
     * @param xrpix   Reference pixel X coordinate
     * @param yrpix   Reference pixel Y coordinate
     * @param nxpix   Number of pixels along x-axis
     * @param nypix   Number of pixels along y-axis
     * @param rotate  Rotation angle (clockwise positive) in degrees
     * @param equinox Equinox of coordinates, 1950 and 2000 supported
     * @param epoch   Epoch of coordinates, used for FK4/FK5 conversion no effect if 0
     * @param proj    Projection
     */
    public WCSTransform(double cra,
                        double cdec,
                        double xsecpix,
                        double ysecpix,
                        double xrpix,
                        double yrpix,
                        int nxpix,
                        int nypix,
                        double rotate,
                        int equinox,
                        double epoch,
                        String proj) {
        super();

        // Plate solution coefficients
        this.plate_fit = 0;
        this.nxpix = nxpix;
        this.nypix = nypix;

        // Approximate world coordinate system from a known plate scale
        this.xinc = xsecpix / 3600.0;
        this.yinc = ysecpix / 3600.0;
        this.xrefpix = xrpix;
        this.yrefpix = yrpix;

        this.xref = cra;
        this.yref = cdec;
        this.c1type = "RA-";
        this.c2type = "DEC";
        this.ptype = proj;
        this.pcode = 1;
        this.coorflip = 0;
        this.rot = rotate;
        this.rotmat = 0;
        this.cd11 = 0.0;
        this.cd21 = 0.0;
        this.cd12 = 0.0;
        this.cd22 = 0.0;
        this.dc11 = 0.0;
        this.dc21 = 0.0;
        this.dc12 = 0.0;
        this.dc22 = 0.0;

        // Coordinate reference frame and equinox
        this.equinox = (double) equinox;
        if (equinox > 1980) {
            this.radecsys = "FK5";
        } else {
            this.radecsys = "FK4";
        }
        if (epoch > 0) {
            this.epoch = epoch;
        } else {
            this.epoch = 0.0;
        }
        this.wcson = 1;

        this.sysout = this.radecsys;
        this.changesys = 0;
        this.printsys = 1;
        this.tabsys = 0;

        // wcsfull();				// allan: added this line 4/24/00; Chris S. moved this line to after the patch

        // Patch. PACS Simulation Working Group; 25 Aug 2002
        // pcode is set to 1 above, but never updated to the value
        // corresponding to the projection type.

        //  Find projection type
        this.pcode = 0;  // default type is linear
        for (int i = 0; i < 8; i++) {
            if (this.ptype.startsWith(ctypes[i])) {
                this.pcode = i + 1;
            }
        }
        // end of patch

        wcsfull(); // Chris S. moved this line from before the patch
    }


    // --- allan: added the methods below: ---

    /**
     * Return the equinox used for coordinates (usually the equionx of the image)
     */
    public double getEquinox() {
        return equinox;
    }

    /**
     * Return the center RA,DEC coordinates in deg.
     */
    public Point2D.Double getWCSCenter() {
        if (isValid()) {
            return new Point2D.Double(fCenterRa, fCenterDec);
        }
        throw new RuntimeException("No WCS information.");
    }

    /** Set the center RA,Dec coordinates in degrees in the current equinox. */
    ///public void setWCSCenter(Point2D.Double p) {
    //wcsshift(p.x, p.y, radecsys);
    //}

    /**
     * Return the center coordinates in image pixels.
     */
    public Point2D.Double getImageCenter() {
        return new Point2D.Double(0.5 * nxpix, 0.5 * nypix);
    }

    /**
     * Return true if world coordinates conversion is available. This method
     * should be called to check before calling any of the world coordinates
     * conversion methods.
     */
    public boolean isWCS() {
        return isValid();
    }

    /**
     * Convert the given image coordinates to world coordinates degrees in the equinox
     * of the current image.
     *
     * @param p          The point to convert.
     * @param isDistance True if p should be interpreted as a distance instead
     *                   of a point.
     */
    public void imageToWorldCoords(Point2D.Double p, boolean isDistance) {
        if (!isValid()) {
            return;
        }

        if (isDistance) {
            p.x = Math.abs(p.x * degPerPixel.x);
            p.y = Math.abs(p.y * degPerPixel.y);
        } else {
            Point2D.Double r = pix2wcs(p.x, p.y);
            if (r != null) {
                p.setLocation(r.x, r.y);
            } else {
                throw new RuntimeException("Image coordinates out of WCS range: " + p);
            }
        }
    }

    /**
     * Convert the given world coordinates (degrees, in the equinox of the current image)
     * to image coordinates.
     *
     * @param p          The point to convert.
     * @param isDistance True if p should be interpreted as a distance instead
     *                   of a point.
     */
    public void worldToImageCoords(Point2D.Double p, boolean isDistance) {
        if (!isValid()) {
            return;
        }

        if (isDistance) {
            p.x = Math.abs(p.x / degPerPixel.x);
            p.y = Math.abs(p.y / degPerPixel.y);
        } else {
            Point2D.Double r = wcs2pix(p.x, p.y);
            if (r != null) {
                p.setLocation(r.x, r.y);
            } else {
                throw new RuntimeException("World coordinates out of range: " + p);
            }
        }
    }

    /**
     * return the width in deg
     */
    public double getWidthInDeg() {
        return fWidthDeg;
    }

    /**
     * return the height in deg
     */
    public double getHeightInDeg() {
        return fHeightDeg;
    }


    /**
     * Return the image width in pixels.
     */
    public double getWidth() {
        return nxpix;
    }

    /**
     * Return the image height in pixels.
     */
    public double getHeight() {
        return nypix;
    }

    // --- allan: added the methods above: 4/24/00 ---


    public void wcseq(WCSKeywordProvider head) {
        int ieq = 0;
        String wcstemp;

        // Set equinox from EQUINOX, EPOCH, or RADECSYS; default to 2000
        if ((head.findKey("EQUINOX"))) {
            this.equinox = head.getDoubleValue("EQUINOX");
            ieq = 2000; // Allan: added 12-02-2009
        } else if ((head.findKey("EPOCH"))) {
            ieq = (int) head.getDoubleValue("EPOCH", 0.0);

            if (ieq == 0) {
                ieq = 1950;
                this.equinox = 1950.0;
            } else {
                this.equinox = head.getDoubleValue("EPOCH");
            }
        } else if (head.findKey("RADECSYS")) {
            wcstemp = head.getStringValue("RADECSYS");

            if (wcstemp.startsWith("FK4")) {
                this.equinox = 1950.0;
                ieq = 1950;
            } else if (wcstemp.startsWith("FK5")) {
                this.equinox = 2000.0;
                ieq = 2000;
            } else if (wcstemp.startsWith("GAL")) {
                this.equinox = 2000.0;
                ieq = 2000;
            } else if (wcstemp.startsWith("ECL")) {
                this.equinox = 2000.0;
                ieq = 2000;
            }
        }

        if (ieq == 0) {
            this.equinox = 2000.0;
            ieq = 2000;
        }

        // Epoch of image (from observation date, if possible)
        // TBD: Should be version of "hgetdate"
        if ((this.epoch = head.getDoubleValue("DATE-OBS", 0.0)) == 0.0) {
            if ((this.epoch = head.getDoubleValue("EPOCH", 0.0)) == 0.0) {
                this.epoch = this.equinox;
            }
        }
        if (this.epoch == 0.0) {
            this.epoch = this.equinox;
        }

        // Set coordinate system from keyword, if it is present
        if (head.findKey("RADECSYS")) {
//            wcstemp = this.radecsys;
            if (this.radecsys.startsWith("FK4")) {
                this.equinox = 1950.0;
            } else if (this.radecsys.startsWith("FK5")) {
                this.equinox = 2000.0;
            } else if (this.radecsys.startsWith("GAL") && ieq == 0) {
                this.equinox = 2000.0;
            }
        }

        // Set galactic coordinates if GLON or GLAT are in C1TYPE
        else if (this.c1type.charAt(0) == 'G') {
            this.radecsys = "GALACTIC";
        } else if (this.c1type.charAt(0) == 'E') {
            this.radecsys = "ECLIPTIC";
        } else if (this.c1type.charAt(0) == 'S') {
            this.radecsys = "SGALACTC";
        } else if (this.c1type.charAt(0) == 'H') {
            this.radecsys = "HELIOECL";
        } else if (this.c1type.charAt(0) == 'A') {
            this.radecsys = "ALTAZ";
        }

        // Otherwise set coordinate system from equinox
        // Systemless coordinates cannot be translated using b, j, or g commands
        else {
            if (ieq > 1980) {
                this.radecsys = "FK5";
            } else {
                this.radecsys = "FK4";
            }
        }

    }

    public boolean isValid() {
        return wcson > 0;
    }

    /**
     * Reset the center of a WCS structure.
     *
     * @param rra     Reference pixel right ascension in degrees
     * @param rdec    Reference pixel declination in degrees
     * @param coorsys FK4 or FK5 coordinates (1950 or 2000)
     */
    public void wcsshift(double rra, double rdec, String coorsys) {
        if (!isValid()) {
            return;
        }

        // Approximate world coordinate system from a known plate scale
        fCenterRa = this.xref = rra;
        fCenterDec = this.yref = rdec;

        // Coordinate reference frame
        this.radecsys = coorsys;

        if (coorsys.startsWith("FK4")) {
            this.equinox = 1950.0;
        } else {
            this.equinox = 2000.0;
        }
    }


    /**
     * Return RA and Dec of image center, plus size in RA and Dec
     */
    public void wcssize() {
        double xpix, ypix;
        double width, height;

        // Find right ascension and declination of coordinates
        if (isValid()) {
            xpix = 0.5 * this.nxpix;
            ypix = 0.5 * this.nypix;

            Point2D.Double center = pix2wcs(xpix, ypix);
            if (center == null) {
                return;
            }

            fCenterRa = center.x;
            fCenterDec = center.y;

            // Compute image half-width in degrees of right ascension
            Point2D.Double pos1 = pix2wcs(1.0, ypix);
            Point2D.Double pos2 = pix2wcs(this.nxpix, ypix);
            if (pos1 == null || pos2 == null) {
                return;
            }

            if (!this.ptype.startsWith("LINEAR") && !this.ptype.startsWith("PIXEL")) {
                width = wcsdist(pos1.x, pos1.y, pos2.x, pos2.y);
                fHalfWidthRa = ((width * 0.5) / Math.cos(degrad(fCenterDec)));
            } else {
                fHalfWidthRa = Math.sqrt(((pos2.y - pos1.y) * (pos2.y - pos1.y)) +
                        ((pos2.x - pos1.x) * (pos2.x - pos1.x)));
            }

            // Compute image half-height in degrees of declination
            pos1 = pix2wcs(xpix, 1.0);
            pos2 = pix2wcs(xpix, this.nypix);
            if (pos1 == null || pos2 == null) {
                return;
            }

            if (!this.ptype.startsWith("LINEAR") && !this.ptype.startsWith("PIXEL")) {
                height = wcsdist(pos1.x, pos1.y, pos2.x, pos2.y);
                fHalfWidthDec = (height * 0.5);
            } else {
                fHalfWidthDec = Math.sqrt(((pos2.y - pos1.y) * (pos2.y - pos1.y)) +
                        ((pos2.x - pos1.x) * (pos2.x - pos1.x)));
            }
        }
    }

    /**
     * Set the RA and Dec of the image center, plus size in degrees
     */
    protected void wcsfull() {
        double xpix;
        double ypix;

        // Find right ascension and declination of coordinates
        if (isValid()) {
            xpix = 0.5 * this.nxpix;
            ypix = 0.5 * this.nypix;

            Point2D.Double center = pix2wcs(xpix, ypix);
            if (center == null) {
                return;
            }

            fCenterRa = center.x;
            fCenterDec = center.y;

            // Compute image width in degrees

            // Chris S. changed 1.0 to 0.0
            //Point2D.Double pos1 = pix2wcs(1.0, ypix);
            Point2D.Double pos1 = pix2wcs(0.0, ypix);
            Point2D.Double pos2 = pix2wcs(this.nxpix, ypix);
            if (pos1 == null || pos2 == null) {
                return;
            }

            if (!this.ptype.startsWith("LINEAR") &&
                    !this.ptype.startsWith("PIXEL")) {
                fWidthDeg = wcsdist(pos1.x, pos1.y, pos2.x, pos2.y);
            } else {
                fWidthDeg = Math.sqrt(((pos2.y - pos1.y) * (pos2.y - pos1.y)) +
                        ((pos2.x - pos1.x) * (pos2.x - pos1.x)));
            }

            // Compute image height in degrees

            // Chris S. changed 1.0 to 0.0
            //ypos1 = pix2wcs(xpix, 1.0);
            pos1 = pix2wcs(xpix, 0.0);
            pos2 = pix2wcs(xpix, this.nypix);
            if (pos1 == null || pos2 == null) {
                return;
            }

            if (!this.ptype.startsWith("LINEAR") &&
                    !this.ptype.startsWith("PIXEL")) {
                fHeightDeg = wcsdist(pos1.x, pos1.y, pos2.x, pos2.y);
            } else {
                fHeightDeg = Math.sqrt(((pos2.y - pos1.y) * (pos2.y - pos1.y)) +
                        ((pos2.x - pos1.x) * (pos2.x - pos1.x)));
            }
            degPerPixel = new Point2D.Double(fWidthDeg / nxpix, fHeightDeg / nypix);
        }
    }

    /**
     * Compute distance in degrees between two sky coordinates
     * (RA,Dec) or (Long,Lat) in degrees
     */
    public double wcsdist(double x1, double y1, double x2, double y2) {
        double xr1, xr2, yr1, yr2;
        double pos1[] = new double[3], pos2[] = new double[3], w, diff, cosb;
        int i;

        // Convert two vectors to direction cosines
        xr1 = degrad(x1);
        yr1 = degrad(y1);
        cosb = Math.cos(yr1);
        pos1[0] = Math.cos(xr1) * cosb;
        pos1[1] = Math.sin(xr1) * cosb;
        pos1[2] = Math.sin(yr1);

        xr2 = degrad(x2);
        yr2 = degrad(y2);
        cosb = Math.cos(yr2);
        pos2[0] = Math.cos(xr2) * cosb;
        pos2[1] = Math.sin(xr2) * cosb;
        pos2[2] = Math.sin(yr2);

        // Modulus squared of half the difference vector
        w = 0.0;
        for (i = 0; i < 3; i++) {
            w = w + (pos1[i] - pos2[i]) * (pos1[i] - pos2[i]);
        }

        w = w / 4.0;
        if (w > 1.0) {
            w = 1.0;
        }

        // Angle beween the vectors
        diff = 2.0 * Math.atan2(Math.sqrt(w), Math.sqrt(1.0 - w));
        diff = raddeg(diff);

        return (diff);
    }

    /**
     * Converts pixel coordinates to World Coordinates.
     * Returns null if the WCSTransform is not valid.
     */
    public Point2D.Double pix2wcs(double xpix, double ypix) {
        Point2D.Double position;

        if (!isValid()) {
            return null;
        }

        this.xpix = xpix;
        this.ypix = ypix;
        this.offscl = 0;

        // Convert image coordinates to sky coordinates
        if (this.plate_fit > 0) {
            if ((position = platepos.getPosition(xpix, ypix, this)) == null) {
                this.offscl = 1;
            }
        } else if ((position = worldpos.getPosition(xpix, ypix, this)) == null) {
            this.offscl = 1;
        }

        if (this.pcode > 0) {
            // Convert coordinates to FK4 or FK5
            if (this.radecsys.startsWith("FK4")) {
                if (this.equinox != 1950.0) {
                    position = wcscon.fk4prec(this.equinox, 1950.0, position);
                }
            } else if (this.radecsys.startsWith("FK5")) {
                if (this.equinox != 2000.0) {
                    position = wcscon.fk5prec(this.equinox, 2000.0, position);
                }
            }

            // Convert coordinates to desired output system
            if (this.changesys == 1) {
                position = wcscon.fk425e(position, this.epoch);
            } else if (this.changesys == 2) {
                position = wcscon.fk524e(position, this.epoch);
            } else if (this.changesys == 3) {
                position = wcscon.fk42gal(position);
            } else if (this.changesys == 4) {
                position = wcscon.fk52gal(position);
            }
        }

        if (this.offscl == 0 && position != null) {
            this.xpos = position.x;
            this.ypos = position.y;
        }

        return position;
    }

    /**
     * Converts World Coordinates to pixel coordinates.
     * Returns null if the WCSTransform is invalid, or if the
     * WCS position does not fall within the image.
     */
    public Point2D.Double wcs2pix(double xpos, double ypos) {
        Point2D.Double pixels;

        if (!isValid()) {
            return null;
        }

        this.xpos = xpos;
        this.ypos = ypos;

        Point2D.Double position = new Point2D.Double(xpos, ypos);

        // Convert coordinates to same system as image
        if (this.changesys == 1) {
            position = wcscon.fk524e(position, this.epoch);
        } else if (this.changesys == 2) {
            position = wcscon.fk425e(position, this.epoch);
        }

        // Convert coordinates from FK4 or FK5 to equinox used
        if (this.radecsys.startsWith("FK4")) {
            if (this.equinox != 1950.0) {
                position = wcscon.fk4prec(1950.0, this.equinox, position);
            }
        } else if (this.radecsys.startsWith("FK5")) {
            if (this.equinox != 2000.0) {
                position = wcscon.fk5prec(2000.0, this.equinox, position);
            }
        }

        // Convert sky coordinates to image coordinates
        if (this.plate_fit > 0) {
            if ((pixels = platepos.getPixels(position.x, position.y, this)) == null) {
                this.offscl = 1;
            }
        } else if ((pixels = worldpos.getPixels(position.x, position.y, this)) == null) {
            this.offscl = 1;
        }

        if (pixels != null) {
            this.xpix = pixels.x;
            this.ypix = pixels.y;
        }

        return pixels;
    }

    /**
     * modify the header to add WCS information
     *
     * @param header FITS header to to fill
     * @throws HeaderCardException if there is trouble from the addValue method
     */
    public void fillHeader(Header header) throws HeaderCardException {
        // Added by Chris S.
        // Use lookup table from http://tdc-www.harvard.edu/software/wcstools/wcs.h.html
        // CRVAL1 is xref; CRVAL2 is yref
        header.deleteKey("CRVAL1");
        header.addValue("CRVAL1", xref, "X reference coordinate value (degrees)");
        header.deleteKey("CRVAL2");
        header.addValue("CRVAL2", yref, "Y reference coordinate value (degrees)");
        // CRPIX1 is xrefpix; CRPIX2 is yrefpix
        header.deleteKey("CRPIX1");
        header.addValue("CRPIX1", xrefpix, "X reference pixel");
        header.deleteKey("CRPIX2");
        header.addValue("CRPIX2", yrefpix, "Y reference pixel");
        //if (rotmat <= 0) {
        //  // CDi_j is cdij
        //header.deleteKey("CD1_1");
        //header.addValue("CD1_1", cd11, "rotation matrix element 1,1");
        //header.deleteKey("CD2_1");
        //header.addValue("CD2_1", cd21, "rotation matrix element 2,1");
        //header.deleteKey("CD1_2");
        //header.addValue("CD1_2", cd12, "rotation matrix element 1,2");
        //header.deleteKey("CD2_2");
        //header.addValue("CD2_2", cd22, "rotation matrix element 2,2");
        //} else {
        // CDELT1 is xinc; CDELT2 is yinc; CROTA2 is rot
        header.deleteKey("CDELT1");
        header.addValue("CDELT1", xinc, "X coordinate increment (degrees)");
        header.deleteKey("CDELT2");
        header.addValue("CDELT2", yinc, "Y coordinate increment (degrees)");
        header.deleteKey("CROTA2");
        header.addValue("CROTA2", rot, "counterclockwise rotation around axis (degrees)");
        //}
        // EPOCH is equinox; DATE-OBS is epoch (years)
        header.deleteKey("EPOCH");
        header.addValue("EPOCH", equinox, "Equinox of coordinates, e.g. 1950.0");
        header.deleteKey("DATE-OBS");
        header.addValue("DATE-OBS", epoch, "Epoch of coordinates");
        // NAXIS1 is nxpix; NAXIS2 is nypix
        //header.deleteKey("NAXIS1");
        //header.addValue("NAXIS1", nxpix, "Number of pixels in X-dimension");
        //header.deleteKey("NAXIS2");
        //header.addValue("NAXIS2", nypix, "Number of pixels in Y-dimension");
        // CTYPE1 is c1type; CTYPE2 is c2type; RADECSYS is radecsys
        header.deleteKey("CTYPE1");
        header.addValue("CTYPE1", c1type + ptype, "1st coordinate type code:  RA--, GLON, ELON...");
        header.deleteKey("CTYPE2");
        header.addValue("CTYPE2", c2type + ptype, "2nd coordinate type code:  DEC-, GLAT, ELAT...");
        header.deleteKey("RADECSYS");
        header.addValue("RADECSYS", radecsys, "Reference frame:  FK4, FK4-NO-E, FK5, GAPPT");
    }

    /**
     * @return the x pixel size in arcseconds
     */
    public double getXArcsecPerPix() {
        return xinc * 3600;
    }


}

/* File libwcs/wcs.c
 * February 6, 1998
 * By Doug Mink, Harvard-Smithsonian Center for Astrophysics

 * Module:	wcs.c (World Coordinate Systems)
 * Purpose:	Convert FITS WCS to pixels and vice versa:
 * Subroutine:	wcsinit (hstring) sets a WCS structure from an image header
 * Subroutine:	wcsninit (hstring,lh) sets a WCS structure from an image header
 * Subroutine:	wcsset (cra,cdec,secpix,xrpix,yrpix,nxpix,nypix,rotate,equinox,epoch,proj)
 *		sets a WCS structure from arguments
 * Subroutine:	iswcs(wcs) returns 1 if WCS structure is filled, else 0
 * Subroutine:	nowcs(wcs) returns 0 if WCS structure is filled, else 1
 * Subroutine:	wcscent (wcs) prints the image center and size in WCS units
 * Subroutine:	wcssize (wcs, cra, cdec, dra, ddec) returns image center and size
 * Subroutine:	wcsfull (wcs, cra, cdec, width, height) returns image center and size
 * Subroutine:	wcsshift (wcs,cra,cdec) resets the center of a WCS structure
 * Subroutine:	wcsdist (x1,y1,x2,y2) compute angular distance between ra/dec or lat/long
 * Subroutine:	wcscominit (wcs,command) sets up a command format for execution by wcscom
 * Subroutine:	wcsoutinit (wcs,coor) sets up the output coordinate system
 * Subroutine:	wcsout(wcs) returns current output coordinate system
 * Subroutine:	wcscom (wcs,file,x,y) executes a command using the current world coordinates
 * Subroutine:	pix2wcs (wcs,xpix,ypix,xpos,ypos) pixel coordinates -> sky coordinates
 * Subroutine:	pix2wcst (wcs,xpix,ypix,wcstring,lstr) pixels -> sky coordinate string
 * Subroutine:	wcs2pix (wcs,xpos,ypos,xpix,ypix,offscl) sky coordinates -> pixel coordinates

 * Copyright:   1996 Smithsonian Astrophysical Observatory
 *              You may do anything you like with this file except remove
 *              this copyright.  The Smithsonian Astrophysical Observatory
 *              makes no representations about the suitability of this
 *              software for any purpose.  It is provided "as is" without
 *              express or implied warranty.

 */


/* Oct 28 1994	new program
 * Dec 21 1994	Implement CD rotation matrix
 * Dec 22 1994	Allow RA and DEC to be either x,y or y,x
 *
 * Mar  6 1995	Add Digital Sky Survey plate fit
 * May  2 1995	Add prototype of PIX2WCST to WCSCOM
 * May 25 1995	Print leading zero for hours and degrees
 * Jun 21 1995	Add WCS2PIX to get pixels from WCS
 * Jun 21 1995	Read plate scale from FITS header for plate solution
 * Jul  6 1995	Pass WCS structure as argument; malloc it in WCSINIT
 * Jul  6 1995	Check string lengths in PIX2WCST
 * Aug 16 1995	Add galactic coordinate conversion to PIX2WCST
 * Aug 17 1995	Return 0 from iswcs if wcs structure is not yet set
 * Sep  8 1995	Do not include malloc.h if VMS
 * Sep  8 1995	Check for legal WCS before trying anything
 * Sep  8 1995	Do not try to set WCS if missing key keywords
 * Oct 18 1995	Add WCSCENT and WCSDIST to print center and size of image
 * Nov  6 1995	Include stdlib.h instead of malloc.h
 * Dec  6 1995	Fix format statement in PIX2WCST
 * Dec 19 1995	Change MALLOC to CALLOC to initialize array to zeroes
 * Dec 19 1995	Explicitly initialize rotation matrix and yinc
 * Dec 22 1995	If SECPIX is set, use approximate WCS
 * Dec 22 1995	Always print coordinate system
 *
 * Jan 12 1996	Use plane-tangent, not linear, projection if SECPIX is set
 * Jan 12 1996  Add WCSSET to set WCS without an image
 * Feb 15 1996	Replace all calls to HGETC with HGETS
 * Feb 20 1996	Add tab table output from PIX2WCST
 * Apr  2 1996	Convert all equinoxes to B1950 or J2000
 * Apr 26 1996	Get and use image epoch for accurate FK4/FK5 conversions
 * May 16 1996	Clean up internal documentation
 * May 17 1996	Return width in right ascension degrees, not sky degrees
 * May 24 1996	Remove extraneous print command from WCSSIZE
 * May 28 1996	Add NOWCS and WCSSHIFT subroutines
 * Jun 11 1996	Drop unused variables after running lint
 * Jun 12 1996	Set equinox as well as system in WCSSHIFT
 * Jun 14 1996	Make DSS keyword searches more robust
 * Jul  1 1996	Allow for SECPIX1 and SECPIX2 keywords
 * Jul  2 1996	Test for CTYPE1 instead of CRVAL1
 * Jul  5 1996	Declare all subroutines in wcs.h
 * Jul 19 1996	Add subroutine WCSFULL to return real image size
 * Aug 12 1996	Allow systemless coordinates which cannot be converted
 * Aug 15 1996	Allow LINEAR WCS to pass numbers through transparently
 * Aug 15 1996	Add WCSERR to print error message under calling program control
 * Aug 16 1996	Add latitude and longitude as image coordinate types
 * Aug 26 1996	Fix arguments to HLENGTH in WCSNINIT
 * Aug 28 1996	Explicitly set OFFSCL in WCS2PIX if coordinates outside image
 * Sep  3 1996	Return computed pixel values even if they are offscale
 * Sep  6 1996	Allow filename to be passed by WCSCOM
 * Oct  8 1996	Default to 2000 for EQUINOX and EPOCH and FK5 for RADECSYS
 * Oct  8 1996	If EPOCH is 0 and EQUINOX is not set, default to 1950 and FK4
 * Oct 15 1996  Add comparison when testing an assignment
 * Oct 16 1996  Allow PIXEL CTYPE which means WCS is same as image coordinates
 * Oct 21 1996	Add WCS_COMMAND environment variable
 * Oct 25 1996	Add image scale to WCSCENT
 * Oct 30 1996	Fix bugs in WCS2PIX
 * Oct 31 1996	Fix CD matrix rotation angle computation
 * Oct 31 1996	Use inline degree <-> radian conversion functions
 * Nov  1 1996	Add option to change number of decimal places in PIX2WCST
 * Nov  5 1996	Set this.crot to 1 if rotation matrix is used
 * Dec  2 1996	Add altitide/azimuth coordinates
 * Dec 13 1996	Fix search format setting from environment
 *
 * Jan 22 1997	Add ifdef for Eric Mandel (SAOtng)
 * Feb  5 1997	Add wcsout for Eric Mandel
 * Mar 20 1997	Drop unused variable STR in WCSCOM
 * May 21 1997	Do not make pixel coordinates mod 360 in PIX2WCST
 * May 22 1997	Add PIXEL pcode = -1;
 * Jul 11 1997	Get center pixel x and y from header even if no WCS
 * Aug  7 1997	Add NOAO PIXSCALi keywords for default WCS
 * Oct 15 1997	Do not reset reference pixel in WCSSHIFT
 * Oct 20 1997	Set chip rotation
 * Oct 24 1997	Keep longitudes between 0 and 360, not -180 and +180
 * Nov  5 1997	Do no compute crot and srot; they are now computed in worldpos
 *
 * Feb  6 1998	Set deltas and rotation from CD matrix in WCSINIT()
 */
