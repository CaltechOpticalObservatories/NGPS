/*
 * $Id: HMS.java,v 1.4 2009/04/21 13:31:17 abrighto Exp $
 */

package jsky.coords;

import jsky.util.StringUtil;

import java.io.Serializable;
import java.text.NumberFormat;
import java.util.Locale;
import java.util.StringTokenizer;

/**
 * Class representing a value of the form "hours:min:sec".
 *
 * @author Allan Brighton
 * @version $Revision: 1.4 $
 */
public class HMS implements Serializable {

    /**
     * On the handling of -0: from the javadoc for Double.equals():
     * "If d1 represents +0.0 while d2 represents -0.0, or vice versa,
     * the equal test has the value false, even though +0.0==-0.0 has the
     * value true."
     * The test for 0.0 != -0.0 only works with Double.equals(minusZero).
     * This case shows up in HMS values with zero hours and negative values,
     * such as "-00 24 32"
     */
    private static final Double MINUS_ZERO = -0.0;

    // Number formats for 2 digit hours and minutes
    private static final NumberFormat NF = NumberFormat.getInstance(Locale.US);

    // Number formats for seconds
    private static final NumberFormat NF_SEC = NumberFormat.getInstance(Locale.US);

    static {
        NF.setMinimumIntegerDigits(2);
        NF.setMaximumIntegerDigits(2);
        NF.setMaximumFractionDigits(0);

        NF_SEC.setMinimumIntegerDigits(2);
        NF_SEC.setMaximumIntegerDigits(2);
        NF_SEC.setMinimumFractionDigits(3);
        NF_SEC.setMaximumFractionDigits(3);
    }

    /**
     * number of hours
     */
    private int hours;

    /**
     * number of minutes
     */
    private int min;

    /**
     * number of seconds
     */
    private double sec;

    /**
     * value converted to decimal
     */
    private double val;

    /**
     * set to 1 or -1
     */
    private byte sign = 1;

    /* true if value has been initialized */
    private boolean initialized = false;


    /**
     * Default constructor: initialize to null values
     */
    public HMS() {
    }

    /**
     * Initialize with the given hours, minutes and seconds.
     */
    public HMS(double hours, int min, double sec) {
        set(hours, min, sec);
    }

    /**
     * Initialize from a decimal hours value and calculate H:M:S.sss.
     */
    public HMS(double val) {
        setVal(val);
    }

    /**
     * Copy constructor
     */
    public HMS(HMS hms) {
        setVal(hms.val);
    }

    /**
     * Initialize from a string value, in format H:M:S.sss, hh, or H M
     * S.  If the value is not in H:M:S and is not an integer (has a
     * decimal point), assume the value is in deg convert to hours by
     * dividing by 15. (Reason: some catalog servers returns RA in h:m:s
     * while others return it in decimal deg.)
     */
    public HMS(String s) {
        this(s, false);
    }

    /**
     * Initialize from a string value, in format H:M:S.sss, hh, or
     * H M S.  If the value is not in H:M:S and is not an
     * integer (has a decimal point), and hflag is true,
     * assume the value is in deg and convert to hours by dividing by 15.
     *
     * @param s the RA string
     * @param hflag if true, assume RA is always in hours, otherwise, if it has a decimal point,
     * assume deg
     */
    public HMS(String s, boolean hflag) {
        s = StringUtil.replace(s, ",", "."); // Treat ',' like '.', by request
        double[] vals = {0.0, 0.0, 0.0};
        StringTokenizer tok = new StringTokenizer(s, ": ");
        int n = 0;
        while (n < 3 && tok.hasMoreTokens()) {
            vals[n++] = Double.valueOf(tok.nextToken());
        }

        if (n >= 2) {
            set(vals[0], (int) vals[1], vals[2]);
        } else if (n == 1) {
            if (!hflag && s.indexOf('.') != -1) {
                setVal(vals[0] / 15.);
            } else {
                setVal(vals[0]);
            }
        } else {
            throw new RuntimeException("Expected a string of the form hh:mm:ss.sss, but got: '" + s + "'");
        }
    }

    /**
     * Set the hours, minutes and seconds.
     */
    public void set(double hours, int min, double sec) {
        this.hours = (int) hours;
        this.min = min;
        this.sec = sec;

        val = (sec / 60.0 + min) / 60.0;

        if (hours < 0.0 || Double.valueOf(hours).equals(MINUS_ZERO)) {
            val = hours - val;
            this.hours = -this.hours;
            sign = -1;
        } else {
            val = this.hours + val;
            sign = 1;
        }
        initialized = true;
    }

    /**
     * Set from a decimal value (hours) and calculate H:M:S.sss.
     */
    public void setVal(double val) {
        this.val = val;

        double v = val; // check also for neg zero
        if (v < 0.0 || Double.valueOf(v).equals(MINUS_ZERO)) {
            sign = -1;
            v = -v;
        } else {
            sign = 1;
        }

        double dd = v + 0.0000000001;
        hours = (int) dd;
        double md = (dd - hours) * 60.;
        min = (int) md;
        sec = (md - min) * 60.;
        initialized = true;
    }

    /**
     * Return the value as a String in the form hh:mm:ss.sss.
     * Seconds are formatted with leading zero if needed.
     * The seconds are formatted with 3 digits of precision.
     */
    public String toString() {
        String secs = NF_SEC.format(sec);

        // sign
        String signStr;
        if (sign == -1) {
            signStr = "-";
        } else {
            signStr = "";
        }

        return signStr
                + NF.format(hours)
                + ":"
                + NF.format(min)
                + ":"
                + secs;
    }

    /**
     * Return the value as a String in the form hh:mm:ss.sss,
     * or if showSeconds is false, hh:mm.
     */
    public String toString(boolean showSeconds) {
        if (showSeconds) {
            return toString();
        }

        // sign
        String signStr;
        if (sign == -1) {
            signStr = "-";
        } else {
            signStr = " ";
        }

        return signStr
                + NF.format(hours)
                + ":"
                + NF.format(min);
    }

    /**
     * Return true if this object has been initialized with a valid value
     */
    public boolean isInitialized() {
        return initialized;
    }

    /**
     * Return the number of hours (not including minutes or seconds)
     */
    public int getHours() {
        return hours;
    }

    /**
     * Return the number of minutes (not including hours or seconds)
     */
    public int getMin() {
        return min;
    }

    /**
     * Return the number of seconds (not including hours and minutes)
     */
    public double getSec() {
        return sec;
    }

    /**
     * Return the value (fractional number of hours) as a double
     */
    public double getVal() {
        return val;
    }

    /**
     * Return the sign of the value
     */
    public byte getSign() {
        return sign;
    }

    /**
     * Define equality based on the value
     */
    public boolean equals(Object obj) {
        return (obj instanceof HMS && val == ((HMS) obj).val);
    }


    /**
     * Test cases
     */
    public static void main(String[] args) {

        HMS h = new HMS(3, 19, 48.23);
        System.out.println("HMS(3, 19, 48.23) == " + h + " == " + h.getVal());

        if (!(h.equals(new HMS(h.getVal())))) {
            System.out.println("Equality test failed: " + h + " != " + new HMS(h.getVal()));
        }

        h = new HMS(41, 30, 42.2);
        System.out.println("41 30 42.2 = " + h + " = " + h.getVal() + " = " + new HMS(h.getVal()));

        h = new HMS(-41, 30, 2.2);
        System.out.println("-41 30 2.2 = " + h + " = " + h.getVal() + " = " + new HMS(h.getVal()));

        h = new HMS("-41 30 42.2");
        System.out.println("-41 30 42.2 = " + h + " = " + h.getVal() + " = " + new HMS(h.getVal()));

        h = new HMS("1:01:02.34567");
        System.out.println("1:01:02.34567 = " + h + " = " + h.getVal() + " = " + new HMS(h.getVal()));

        h = new HMS("1:01:02.34567");
        System.out.println("1:01:02.34567 = " + h + " = " + h.getVal() + " = " + new HMS(h.getVal()));

        h = new HMS(-0., 15, 33.3333);
        System.out.println("-0 15 33.3333 = " + h + " = " + h.getVal() + " = " + new HMS(h.getVal()));

        h = new HMS(-0.0001);
        System.out.println("-0.0001 = " + h + " = " + h.getVal() + " = " + new HMS(h.getVal()));

        h = new HMS(121.39583332 / 15.);
        System.out.println("121.39583332/15. = " + h + " = " + h.getVal() + " = " + new HMS(h.getVal()));

        h = new HMS(121.09583332 / 15.);
        System.out.println("121.09583332/15. = " + h + " = " + h.getVal() + " = " + new HMS(h.getVal()));

        h = new HMS(-121.39583332 / 15.);
        System.out.println("-121.39583332/15. = " + h + " = " + h.getVal() + " = " + new HMS(h.getVal()));

        h = new HMS(-121.09583332 / 15.);
        System.out.println("-121.09583332/15. = " + h + " = " + h.getVal() + " = " + new HMS(h.getVal()));
    }
}
