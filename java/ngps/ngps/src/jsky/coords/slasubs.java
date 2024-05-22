//=== File Prolog =============================================================
//  This code was developed by NASA, Goddard Space Flight Center, Code 588
//  for the Scientist's Expert Assistant (SEA) project.
//
//--- Contents ----------------------------------------------------------------
//  class slasubs
//
//--- Description -------------------------------------------------------------
//
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
//
//  07/16/98    J. Jones / 588
//
//      Original implementation.
//
//--- DISCLAIMER---------------------------------------------------------------
//
//  This software is provided "as is" without any warranty of any kind, either
//  express, implied, or statutory, including, but not limited to, any
//  warranty that the software will conform to specification, any implied
//  warranties of merchantability, fitness for a particular purpose, and
//  freedom from infringement, and any warranty that the documentation will
//  conform to the program, or any warranty that the software will be error
//  free.
//
//  In no event shall NASA be liable for any damages, including, but not
//  limited to direct, indirect, special or consequential damages, arising out
//  of, resulting from, or in any way connected with this software, whether or
//  not based upon warranty, contract, tort or otherwise, whether or not
//  injury was sustained by persons or property or otherwise, and whether or
//  not loss was sustained from or arose out of the results of, or use of,
//  their software or services provided hereunder.
//
//=== End File Prolog =========================================================

/* File slasubs.c
 *** Starlink subroutines by Patrick Wallace used by wcscon.c subroutines
 *** November 4, 1996
 */

package jsky.coords;

import java.awt.geom.*;

public class slasubs {

    // constants
    /**
     * defines the maximum disagreement for quantities
     */
    private static final double TINY = 1.0E-6;

    /* Right ascension in radians */
    /* Declination in radians */
    /* x,y,z unit vector (returned) */
    /*
    **  slaDcs2c: Spherical coordinates to direction cosines.
    **
    **  The spherical coordinates are longitude (+ve anticlockwise
    **  looking from the +ve latitude pole) and latitude.  The
    **  Cartesian coordinates are right handed, with the x axis
    **  at zero longitude and latitude, and the z axis at the
    **  +ve latitude pole.
    **
    **  P.T.Wallace   Starlink   31 October 1993
    */
    public static double[] slaDcs2c(double a, double b) {
        double cosb = Math.cos(b);

        double[] v = new double[3];
        v[0] = Math.cos(a) * cosb;
        v[1] = Math.sin(a) * cosb;
        v[2] = Math.sin(b);

        return v;
    }

    /* 3x3 Matrix */
    /* Vector */
    /* Result vector (returned) */
    /*
    **  slaDmxv:
    **  Performs the 3-d forward unitary transformation:
    **     vector vb = matrix dm * vector va
    **
    **  P.T.Wallace   Starlink   31 October 1993
    */
    public static double[] slaDmxv(double[][] dm, double[] va) {
        int i, j;
        double w;
        double[] vw = new double[3], vb = new double[3];

        /* Matrix dm * vector va -> vector vw */
        for (j = 0; j < 3; j++) {
            w = 0.0;
            for (i = 0; i < 3; i++) {
                w += dm[j][i] * va[i];
            }
            vw[j] = w;
        }

        /* Vector vw -> vector vb */
        for (j = 0; j < 3; j++) {
            vb[j] = vw[j];
        }

        return vb;
    }

    /* x,y,z vector */
    /* Right ascension in radians */
    /* Declination in radians */
    /*
    **  slaDcc2s:
    **  Direction cosines to spherical coordinates.
    **
    **  Returned:
    **     *a,*b  double      spherical coordinates in radians
    **
    **  The spherical coordinates are longitude (+ve anticlockwise
    **  looking from the +ve latitude pole) and latitude.  The
    **  Cartesian coordinates are right handed, with the x axis
    **  at zero longitude and latitude, and the z axis at the
    **  +ve latitude pole.
    **
    **  If v is null, zero a and b are returned.
    **  At either pole, zero a is returned.
    **
    **  P.T.Wallace   Starlink   31 October 1993
    */
    public static Point2D.Double slaDcc2s(double[] v) {
        double x, y, z, r;

        x = v[0];
        y = v[1];
        z = v[2];
        r = Math.sqrt(x * x + y * y);

        Point2D.Double result = new Point2D.Double();

        result.x = (r != 0.0) ? Math.atan2(y, x) : 0.0;
        result.y = (z != 0.0) ? Math.atan2(z, r) : 0.0;

        return result;
    }


    /* 2pi */
    public static final double D2PI = 6.2831853071795864769252867665590057683943387987502;

    /*
    **  slaDranrm:
    **  Normalize angle into range 0-2 pi.
    **  The result is angle expressed in the range 0-2 pi (double).
    **  Defined in slamac.h:  D2PI
    **
    **  P.T.Wallace   Starlink   30 October 1993
    */
    /* angle in radians */
    public static double slaDranrm(double angle) {
        double w;

        w = Math.IEEEremainder(angle, D2PI); // was fmod
        return (w >= 0.0) ? w : w + D2PI;
    }

    /*
    **  slaDeuler:
    **  Form a rotation matrix from the Euler angles - three successive
    **  rotations about specified Cartesian axes.
    **
    **  A rotation is positive when the reference frame rotates
    **  anticlockwise as seen looking towards the origin from the
    **  positive region of the specified axis.
    **
    **  The characters of order define which axes the three successive
    **  rotations are about.  A typical value is 'zxz', indicating that
    **  rmat is to become the direction cosine matrix corresponding to
    **  rotations of the reference frame through phi radians about the
    **  old z-axis, followed by theta radians about the resulting x-axis,
    **  then psi radians about the resulting z-axis.
    **
    **  The axis names can be any of the following, in any order or
    **  combination:  x, y, z, uppercase or lowercase, 1, 2, 3.  Normal
    **  axis labelling/numbering conventions apply;  the xyz (=123)
    **  triad is right-handed.  Thus, the 'zxz' example given above
    **  could be written 'zxz' or '313' (or even 'zxz' or '3xz').  Order
    **  is terminated by length or by the first unrecognised character.
    **
    **  Fewer than three rotations are acceptable, in which case the later
    **  angle arguments are ignored.  Zero rotations produces a unit rmat.
    **
    **  P.T.Wallace   Starlink   17 November 1993
    */
    /* specifies about which axes the rotations occur */
    /* 1st rotation (radians) */
    /* 2nd rotation (radians) */
    /* 3rd rotation (radians) */
    /* 3x3 Rotation matrix (returned) */
    public static double[][] slaDeuler(String order, double phi, double theta, double psi) {
        int j, i, l, n, k;
        double result[][] = new double[3][3], rotn[][] = new double[3][3], angle, s, c, w, wm[][] = new double[3][3];
        char axis;

        /* Initialize result matrix */
        for (j = 0; j < 3; j++) {
            for (i = 0; i < 3; i++) {
                result[i][j] = (i == j) ? 1.0 : 0.0;
            }
        }

        /* Establish length of axis string */
        l = order.length();

        /* Look at each character of axis string until finished */
        for (n = 0; n < 3; n++) {
            if (n <= l) {

                /* Initialize rotation matrix for the current rotation */
                for (j = 0; j < 3; j++) {
                    for (i = 0; i < 3; i++) {
                        rotn[i][j] = (i == j) ? 1.0 : 0.0;
                    }
                }

                /* Pick up the appropriate Euler angle and take sine & cosine */
                switch (n) {
                    case 0:
                    default:
                        angle = phi;
                        break;
                    case 1:
                        angle = theta;
                        break;
                    case 2:
                        angle = psi;
                        break;
                }
                s = Math.sin(angle);
                c = Math.cos(angle);

                /* Identify the axis */
                axis = order.charAt(n);
                if ((axis == 'X') || (axis == 'x') || (axis == '1')) {
                    /* Matrix for x-rotation */
                    rotn[1][1] = c;
                    rotn[1][2] = s;
                    rotn[2][1] = -s;
                    rotn[2][2] = c;
                } else if ((axis == 'Y') || (axis == 'y') || (axis == '2')) {
                    /* Matrix for y-rotation */
                    rotn[0][0] = c;
                    rotn[0][2] = -s;
                    rotn[2][0] = s;
                    rotn[2][2] = c;
                } else if ((axis == 'Z') || (axis == 'z') || (axis == '3')) {
                    /* Matrix for z-rotation */
                    rotn[0][0] = c;
                    rotn[0][1] = s;
                    rotn[1][0] = -s;
                    rotn[1][1] = c;
                } else {
                    /* Unrecognized character - fake end of string */
                    l = 0;
                }

                /* Apply the current rotation (matrix rotn x matrix result) */
                for (i = 0; i < 3; i++) {
                    for (j = 0; j < 3; j++) {
                        w = 0.0;
                        for (k = 0; k < 3; k++) {
                            w += rotn[i][k] * result[k][j];
                        }
                        wm[i][j] = w;
                    }
                }
                for (j = 0; j < 3; j++) {
                    for (i = 0; i < 3; i++) {
                        result[i][j] = wm[i][j];
                    }
                }
            }
        }

        return result;
    }
    /*
     * Nov  4 1996  New file
     */


    /**
     * slDE2H code from Starlink adopted to Java for testing
     * EquatorialToHorizonPositionMap class against
     * Notes:
     * <p/>
     * 1)  All the arguments are angles in radians.
     * <p/>
     * 2)  Azimuth is returned in the range 0-2pi;  north is zero,
     * and east is +pi/2.  Elevation is returned in the range
     * +/-pi/2.
     * <p/>
     * 3)  The latitude must be geodetic.  In critical applications,
     * corrections for polar motion should be applied.
     * <p/>
     * 4)  In some applications it will be important to specify the
     * correct type of hour angle and declination in order to
     * produce the required type of azimuth and elevation.  In
     * particular, it may be important to distinguish between
     * elevation as affected by refraction, which would
     * require the "observed" HA,Dec, and the elevation
     * in vacuo, which would require the "topocentric" HA,Dec.
     * If the effects of diurnal aberration can be neglected, the
     * "apparent" HA,Dec may be used instead of the topocentric
     * HA,Dec.
     * <p/>
     * 5)  No range checking of arguments is carried out.
     * <p/>
     * 6)  In applications which involve many such calculations, rather
     * than calling the present routine it will be more efficient to
     * use inline code, having previously computed fixed terms such
     * as sine and cosine of latitude, and (for tracking a star)
     * sine and cosine of declination.
     *
     * @param HA  double containing the hour angle in radians
     * @param DEC double containing the declination angle (rad.)
     * @param PHI double containing the observatory latitude (rad.)
     * @return AZEL    double[2] array containing the
     *         0: azimuth angle (radians)
     *         1: elevation angle (radians)
     */
    public static double[] slDE2H(double HA, double DEC, double PHI) {
        double[] AZEL = new double[2];

        // Useful trig functions
        double SH = Math.sin(HA);
        double CH = Math.cos(HA);
        double SD = Math.sin(DEC);
        double CD = Math.cos(DEC);
        double SP = Math.sin(PHI);
        double CP = Math.cos(PHI);

        // Az,El as x,y,z
        double X = -CH * CD * SP + SD * CP;
        double Y = -SH * CD;
        double Z = CH * CD * CP + SD * SP;

        // To spherical
        double R = Math.sqrt(X * X + Y * Y);
        double A;
        if (R == 0.) {
            A = 0.;
        } else {
            A = Math.atan2(Y, X);
        }
        if (A < 0.0) {
            A = A + 2. * Math.PI;
        }
        AZEL[0] = A;
        AZEL[1] = Math.atan2(Z, R);

        return AZEL;
    }

    /**
     * Given the direction cosines of a star and of the
     * tangent point, determine the star's tangent-plane
     * coordinates.
     * <p/>
     * (double precision)
     * <p/>
     * Given:
     * V         d(3)   direction cosines of star
     * V0        d(3)   direction cosines of tangent point
     * <p/>
     * Returned:
     * XI,ETA    d      tangent plane coordinates of star
     * J         i      status:   0 = OK
     * 1 = error, star too far from axis
     * 2 = error, antistar on tangent plane
     * 3 = error, antistar too far from axis
     * <p/>
     * Notes:
     * <p/>
     * 1  If vector V0 is not of unit length, or if vector V is of zero
     * length, the results will be wrong.
     * <p/>
     * 2  If V0 points at a pole, the returned XI,ETA will be based on the
     * arbitrary assumption that the RA of the tangent point is zero.
     * <p/>
     * 3  This routine is the Cartesian equivalent of the routine slDSTP.
     * <p/>
     * P.T.Wallace   Starlink   27 November 1996
     * <p/>
     * Copyright (C) 1996 Rutherford Appleton Laboratory
     * Copyright (C) 1995 Association of Universities for Research in
     * Astronomy Inc.
     */
    public static double[] slDVTP(double[] V, double[] V0) {
        int J;              // identifies errors
        double X = V[0];
        double Y = V[1];
        double Z = V[2];
        double X0 = V0[0];
        double Y0 = V0[1];
        double Z0 = V0[2];
        double R2 = X0 * X0 + Y0 * Y0;
        double R = Math.sqrt(R2);
        if (R == 0.) {
            R = 1E-20;
            X0 = R;
        }
        double W = X * X0 + Y * Y0;
        double D = W + Z * Z0;
        if (D > TINY) {
            J = 0;
        } else if (D >= 0.) {
            J = 1;
            D = TINY;
        } else if (D > -TINY) {
            J = 2;
            D = -TINY;
        } else {
            J = 3;
        }
        D = D * R;
        double XI = (Y * X0 - X * Y0) / D;
        double ETA = (Z * R2 - Z0 * W) / D;
        if (J != 0) {
            System.out.println("ERROR: From slDVTP, J = " + J);
        }
        return new double[]{XI, ETA};
    }

}
