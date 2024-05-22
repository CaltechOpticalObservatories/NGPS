// Copyright 2003
// Association for Universities for Research in Astronomy, Inc.,
// Observatory Control System, Gemini Telescopes Project.
//
// $Id: ValueSearchCondition.java,v 1.3 2009/03/03 21:55:51 abrighto Exp $

package jsky.catalog;

import jsky.util.StringUtil;


/**
 * Represents a search condition for values in a given table column,
 * or parameters to a query or request.
 */
public class ValueSearchCondition extends AbstractSearchCondition {

    /** The minimum value */
    private Comparable _val;


    /**
     * Create a new ValueSearchCondition for the given column or parameter description.
     */
    public ValueSearchCondition(FieldDesc fieldDesc, Comparable val) {
        super(fieldDesc);
        _val = val;
    }

    /**
     * Create a new numerical ValueSearchCondition for the given column or parameter description.
     */
    public ValueSearchCondition(FieldDesc fieldDesc, double val) {
        this(fieldDesc, Double.valueOf(val));
    }

    /**
     * Create a new String ValueSearchCondition for the given column or parameter description.
     */
    public ValueSearchCondition(FieldDesc fieldDesc, String val) {
        this(fieldDesc, (Comparable) val.trim());
    }

    /** Return the value (actually a Double or String) */
    public Comparable getVal() {
        return _val;
    }

    /**
     * Return true if the condition is true for the given value.
     *
     * @param val The value to be checked against the condition.
     * @return true if the value satisfies the condition.
     */
    @SuppressWarnings({"unchecked"})
    public boolean isTrueFor(Comparable val) {
        if (_val instanceof String && val instanceof String) {
            return StringUtil.match((String) _val, (String) val);
        }
        return (_val.compareTo(val) == 0);
    }


    /**
     * Return true if the condition is true for the given numeric value.
     * If the condition was specified as a String, the return value is false.
     *
     * @param val The value to be checked against the condition.
     * @return true if the value satisfies the condition.
     */
    public boolean isTrueFor(double val) {
        return isTrueFor(Double.valueOf(val));
    }


    /** Return the value as a String. */
    public String getValueAsString() {
        return _val.toString();
    }

    public boolean isRegionArg() {
        FieldDesc fd = getFieldDesc();
        return fd != null && (fd.isRA() || fd.isDec() || fd.getName().equalsIgnoreCase("equinox"));
    }

    /**
     * Test cases
     */
    public static void main(String[] args) {
        ValueSearchCondition s = new ValueSearchCondition(new FieldDescAdapter("X"), 22);

        if (!s.isTrueFor(22.))
            throw new RuntimeException("test failed for 22.: " + s);
        if (s.isTrueFor(23.))
            throw new RuntimeException("test failed for 23.: " + s);

        s = new ValueSearchCondition(new FieldDescAdapter("S"), "aaa");
        if (!s.isTrueFor("aaa"))
            throw new RuntimeException("test failed for \"aaa\": " + s);
        if (s.isTrueFor("mmm"))
            throw new RuntimeException("test failed for \"mmm\": " + s);

        System.out.println("All tests passed");
    }
}





