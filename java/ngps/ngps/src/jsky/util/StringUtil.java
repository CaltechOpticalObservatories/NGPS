/*
 * ESO Archive
 *
 * $Id: StringUtil.java,v 1.3 2009/02/24 11:06:04 abrighto Exp $
 *
 * who             when        what
 * --------------  ----------  ----------------------------------------
 * Allan Brighton  1999/07/02  Created
 */

package jsky.util;

/**
 * Contains static String utility methods.
 */
public class StringUtil {

    /**
     * Replace all occurances of the given target string with the given
     * replacement string in the source string.
     *
     * @param source source string to be searched
     * @param target the target string to replace
     * @param replacement the value to replace the target string with
     * @return a new string with the target string replaced with the replacement
     *         string.
     */ 
    public static String replace(String source, String target, String replacement) {
//        // Note: the implementation could be more efficient...
//        StringBuffer sbuf = new StringBuffer(source);
//        int n = source.length();
//        int offset = 0;
//        for (int i = 0; i < n; i++) {
//            if (source.startsWith(target, i)) {
//                int tl = target.length(), rl = replacement.length();
//                sbuf.replace(i + offset, i + offset + tl, replacement);
//                offset += (rl - tl);
//                i += tl - 1;
//            }
//        }
//        return sbuf.toString();

        // Since java5, this is available in the String class
        return source.replace(target, replacement);
    }


    /**
     * Split the string s at the given separator char, if found, and return
     * an array containing the two resulting strings, or null if the separator
     * char was not found.
     */
    public static String[] split(String s, int sep) {
        int i = s.indexOf(sep);
        if (i > 0) {
            String[] ar;
            ar = new String[2];
            ar[0] = s.substring(0, i);
            ar[1] = s.substring(i + 1);
            return ar;
        }
        return null;
    }

    /**
     * Combine the given array of strings to a single string
     * and return the result.
     *
     * @param ar an array of Strings, one element for each Tcl list item
     * @param sep put this between the strings
     * @return a String in the format "s1SEPs2SEP..."
     */
    public static String combine(String[] ar, String sep) {
        if (ar == null) {
            return "";
        }

        StringBuffer sb = new StringBuffer();
        for (int i = 0; i < ar.length; i++) {
            sb.append(ar[i]);
            if (i + 1 < ar.length) {
                sb.append(sep);
            }
        }
        return sb.toString();
    }

    /**
     * Return true if the two strings are equal (like String.equals(), but
     * allowing null values for both operands).
     */
    public static boolean equals(String s1, String s2) {
        return s1 == null && s2 == null || !(s1 == null || s2 == null) && s1.equals(s2);
    }

    /**
     * Checks whether a string matches a given wildcard pattern.
     * Only does ? and * (or '%'), and multiple patterns separated by |.
     * (Taken from http://www.acme.com/java/software/Acme.Utils.html).
     */
    public static boolean match(String pattern, String string) {
        int sLen = string.length();
        int pLen = pattern.length();
        for (int p = 0; ; p++) {
            for (int s = 0; ; p++, s++) {
                boolean sEnd = (s >= sLen);
                boolean pEnd = (p >= pLen || pattern.charAt(p) == '|');

                // Make sure '*' or '%' also match the empty string
                if (sEnd && !pEnd && pattern.charAt(p) == '*' && (p == pLen - 1 || (p < pLen - 1 && pattern.charAt(p + 1) == '|'))) {
                    return true;
                }

                if (sEnd && pEnd) {
                    return true;
                }

                if (sEnd || pEnd) {
                    break;
                }

                if (pattern.charAt(p) == '?') {
                    continue;
                }

                if (pattern.charAt(p) == '*' || pattern.charAt(p) == '%') {
                    p++;

                    for (int i = sLen; i >= s; --i) {
                        if (match(pattern.substring(p), string.substring(i)))  /* not quite right */ {
                            return true;
                        }
                    }
                    break;
                }

                if (Character.toUpperCase(pattern.charAt(p)) != Character.toUpperCase(string.charAt(s))) {
                    break;
                }

            }

            p = pattern.indexOf('|', p);
            if (p == -1) {
                return false;
            }
        }
    }


    /**
     * Pad the given string to the given length with blanks on the left or right, depending
     * on the value of the leftJustify argument.
     *
     * @param s the source string
     * @param length fill the string with blanks if it is less than this length
     * @param leftJustify if true, add blanks after the s, otherwise before
     */
    public static String pad(String s, int length, boolean leftJustify) {
        StringBuffer sb = new StringBuffer(length);
        if (leftJustify) {
            sb.append(s);
            int n = s.length();
            while (n++ < length) {
                sb.append(' ');
            }
        } else {
            int n = s.length();
            while (n++ < length) {
                sb.append(' ');
            }
            sb.append(s);
        }
        return sb.toString();
    }

    /**
     * Return true if the given array contains the given String
     */
    public static boolean arrayContains(String[] ar, String s) {
        for (String str : ar) {
            if (s.equals(str)) {
                return true;
            }
        }
        return false;
    }


    /**
     * Split the input string into words and capitalize the first letter of each word.
     *
     * @param str the input string
     * @return the input string with the first letter of each word capitalized
     */
    public static String capitalize(String str) {
        if (str == null || str.length() == 0) {
            return str;
        }
        StringBuffer sb = new StringBuffer();
        String[] words = str.split("[\\s]+");
        for (String s : words) {
            String ss = s.trim();
            if (ss.length() != 0) {
                sb.append(ss.substring(0, 1).toUpperCase()).append(ss.substring(1).toLowerCase());
                sb.append(' ');
            }
        }
        return sb.toString().trim();
    }
}


