/*
 * ESO Archive
 *
 * $Id: FieldFormat.java,v 1.2 2009/02/20 13:06:56 abrighto Exp $
 *
 * who             when        what
 * --------------  ----------  ----------------------------------------
 * Allan Brighton  1999/08/01  Created
 */

package jsky.catalog;

import jsky.coords.DMS;
import jsky.coords.HMS;
import jsky.util.StringUtil;
import jsky.util.gui.DialogUtil;


/**
 * This utility class provides a method to scan specially formatted
 * values from a string and return an object of the correct type.
 */
public class FieldFormat {

    /**
     * Return an object for the given field description by parsing the given string.
     *
     * @param fieldDesc describes the field
     * @param s the value in string format
     * @return an object of the given class, or null if it could not be parsed
     */
    public static Object getValue(FieldDesc fieldDesc, String s) {
        Class c = fieldDesc.getFieldClass();
        s = s.trim();
        if (c == String.class)
            return s;

        if (s.equals(""))
            return null;

        if (c == Double.class || c == Float.class) {
            if (fieldDesc.isRA()) {
                HMS hms = new HMS(s);
                if (c == Double.class)
                    return hms.getVal();
                else
                    return new Float(hms.getVal());
            } else if (fieldDesc.isDec()) {
                DMS dms = new DMS(s);
                if (c == Double.class)
                    return dms.getVal();
                else
                    return new Float(dms.getVal());
            }
        }

        try {
            if (c == Double.class) {
                return Double.valueOf(s);
            }

            if (c == Float.class) {
                return Float.valueOf(s);
            }

            if (c == Integer.class)
                return Integer.valueOf(s);

            if (c == Boolean.class)
                return Boolean.valueOf(s);
        } catch (NumberFormatException e) {
            DialogUtil.error("Invalid query syntax: " + s + ", expected a single value");
        }

        return null;
    }


    /**
     * Return a ValueRange object representing a range of values for the given
     * field by parsing the given string.
     *
     * @param fieldDesc describes the field
     *
     * @param s the value range in string format, which may be encoded as two
     *          values: "min max", as one value: "value" (For numerical types,
     *          tests for equality, for Strings, the start of the string). The
     *          symbols ">", or "<" may be used in the string for numerical types,
     *          to indicate that the value should be greater than or less than
     *          the (noninclusive) given values, for example: ">0.0 <1.0".
     *          If there are two values, they should be separated by a single space
     *          and the first value should be the minimum value (inclusive,
     *          unless ">" was specified).
     *
     * @return a ValueRange object representing the range of values, or null if they
     *         could not be parsed
     */
    public static ValueRange getValueRange(FieldDesc fieldDesc, String s) {
        Class c = fieldDesc.getFieldClass();
        s = s.trim();
        if (c == String.class)
            return new ValueRange(s);

        // look for two values separated by a space, or a single value (test for equality)
        String[] ar = StringUtil.split(s, ' ');
        boolean oneVal = false;  // true if only one value was specified
        if (ar == null) {
            ar = new String[2];
            ar[0] = ar[1] = s;
            oneVal = true;
        }

        // check for <, >, symbols and set inclusive flags
        boolean[] inclusive = new boolean[2];
        boolean insertMin = false, insertMax = false;

        // XXX error handling?)
        for (int i = 0; i < 2; i++) {
            inclusive[i] = true;
            if (ar[i].startsWith(">")) {
                if (ar[i].startsWith(">=")) {
                    ar[i] = ar[i].substring(2);
                } else {
                    inclusive[i] = false;
                    ar[i] = ar[i].substring(1);
                }
                if (oneVal) {
                    // x > value
                    insertMax = true;
                    break;
                }
            } else if (ar[i].startsWith("<")) {
                if (ar[i].startsWith("<=")) {
                    ar[i] = ar[i].substring(2);
                } else {
                    inclusive[i] = false;
                    ar[i] = ar[i].substring(1);
                }
                if (oneVal) {
                    // x < value
                    insertMin = true;
                    ar[1] = ar[0];
                    break;
                }
            }
        }

        try {
            if (c == Double.class) {
                if (insertMin)
                    ar[0] = Double.toString(Double.MIN_VALUE);
                else if (insertMax)
                    ar[1] = Double.toString(Double.MAX_VALUE);
                return new ValueRange(Double.valueOf(ar[0]), inclusive[0], Double.valueOf(ar[1]), inclusive[1]);
            }
            if (c == Float.class) {
                if (insertMin)
                    ar[0] = Float.toString(Float.MIN_VALUE);
                else if (insertMax)
                    ar[1] = Float.toString(Float.MAX_VALUE);
                return new ValueRange(Float.valueOf(ar[0]), inclusive[0], Float.valueOf(ar[1]), inclusive[1]);
            }
            if (c == Integer.class) {
                if (insertMin)
                    ar[0] = Integer.toString(Integer.MIN_VALUE);
                else if (insertMax)
                    ar[1] = Integer.toString(Integer.MAX_VALUE);
                return new ValueRange(Integer.valueOf(ar[0]), inclusive[0],Integer.valueOf(ar[1]), inclusive[1]);
            }
        } catch (NumberFormatException e) {
            DialogUtil.error("Invalid query syntax: " + s + ", expected: a value or value range expression (min max, >min, <=max, ...)");
        }

        return null;
    }
}
