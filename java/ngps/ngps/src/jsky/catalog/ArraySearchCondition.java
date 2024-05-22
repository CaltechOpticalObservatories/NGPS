// Copyright 2003
// Association for Universities for Research in Astronomy, Inc.,
// Observatory Control System, Gemini Telescopes Project.
//
// $Id: ArraySearchCondition.java,v 1.2 2009/02/20 13:06:56 abrighto Exp $

package jsky.catalog;

import java.lang.StringBuffer;


/**
 * Represents a search condition with an array of possible values.
 * The condition evaluates to true, if any one of the values in the array match.
 */
public class ArraySearchCondition extends AbstractSearchCondition {

    private static final long serialVersionUID = 1L;

    /**
     * The condition will match any of these values
     */
    private Object[] _values;


    /**
     * Create a new ArraySearchCondition with the given values.
     */
    public ArraySearchCondition(FieldDesc fieldDesc, Object[] values) {
        super(fieldDesc);
        _values = values;
    }


    /**
     * Return the array of values
     */
    public Object[] getValues() {
        return _values;
    }

    /**
     * Return true if the condition is true for the given value.
     *
     * @param val The value to be checked against the condition.
     * @return true if the value satisfies the condition.
     */
    public boolean isTrueFor(Comparable val) {
        if (_values == null || _values.length == 0) {
            return true;
        }

        for (Object value : _values) {
            if (val == null && value == null) {
                return true;
            }
            //noinspection unchecked
            if (val instanceof Comparable && value instanceof Comparable
                    && ((Comparable) value).compareTo(val) == 0) {
                return true;
            }
        }
        return false;
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


    /**
     * Return the values as a String in the format "val1,val2,val3".
     */
    public String getValueAsString() {
        String sep = "";
        StringBuffer sb = new StringBuffer();
        for (Object _value : _values) {
            sb.append(sep);
            sb.append(_value.toString());
            sep = ",";
        }
        return sb.toString();
    }


    /**
     * Test cases
     */
    public static void main(String[] args) {
        ArraySearchCondition s = new ArraySearchCondition(new FieldDescAdapter("X"), new String[]{"AAA", "BBB", "CCC"});

        if (!s.isTrueFor("BBB")) {
            throw new RuntimeException("test failed for BBB: " + s);
        }
        if (s.isTrueFor("CXX")) {
            throw new RuntimeException("test failed for CXX: " + s);
        }

        System.out.println("All tests passed");
    }

}





