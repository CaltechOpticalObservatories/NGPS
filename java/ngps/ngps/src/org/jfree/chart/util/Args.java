/* ===========================================================
 * JFreeChart : a free chart library for the Java(tm) platform
 * ===========================================================
 *
 * (C) Copyright 2000-present, by David Gilbert and Contributors.
 *
 * Project Info:  http://www.jfree.org/jfreechart/index.html
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 *
 * [Oracle and Java are registered trademarks of Oracle and/or its affiliates. 
 * Other names may be trademarks of their respective owners.]
 *
 * ---------
 * Args.java
 * ---------
 * (C) Copyright 2011-present, by David Gilbert.
 *
 * Original Author:  David Gilbert;
 * Contributor(s):   -;
 *
 */

package org.jfree.chart.util;

/**
 * A utility class for checking method arguments.
 */
public class Args {

    /**
     * Throws an {@code IllegalArgumentException} if the supplied 
     * {@code param} is {@code null}.
     *
     * @param param  the parameter to check ({@code null} permitted).
     * @param name  the name of the parameter (to use in the exception message
     *     if {@code param} is {@code null}).
     *
     * @throws IllegalArgumentException  if {@code param} is 
     *     {@code null}.
     */
    public static void nullNotPermitted(Object param, String name) {
        if (param == null) {
            throw new IllegalArgumentException("Null '" + name + "' argument.");
        }
    }
    
    /**
     * Throws an {@code IllegalArgumentException} if {@code value} is negative.
     * 
     * @param value  the value.
     * @param name  the parameter name (for use in the exception message).
     */
    public static void requireNonNegative(int value, String name) {
        if (value < 0) {
            throw new IllegalArgumentException("Require '" + name + "' (" 
                    + value + ") to be non-negative.");
        }
    }
    
    /**
     * Throws an {@code IllegalArgumentException} if {@code value} is negative.
     * 
     * @param value  the value.
     * @param name  the parameter name (for use in the exception message).
     */
    public static void requireNonNegative(double value, String name) {
        if (value < 0) {
            throw new IllegalArgumentException("Require '" + name + "' (" 
                    + value + ") to be non-negative.");
        }
    }
    
    /**
     * Checks that the value falls within the specified range and, if it does
     * not, throws an {@code IllegalArgumentException}.
     * 
     * @param value  the value.
     * @param name  the parameter name.
     * @param lowerBound  the lower bound of the permitted range.
     * @param upperBound  the upper bound fo the permitted range.
     */
    public static void requireInRange(int value, String name, 
            int lowerBound, int upperBound) {
        if (value < lowerBound || value > upperBound) {
            throw new IllegalArgumentException("Require '" + name + "' (" 
                    + value + ") to be in the range " + lowerBound + " to " 
                    + upperBound);
        }
    }
    
    /**
     * Checks the supplied value is finite (neither infinite nor NaN) and 
     * throws an {@code IllegalArgumentException} if the requirement is not
     * met.
     * 
     * @param value  the value.
     * @param name  the parameter name (for use in the exception message).
     * 
     * @since 1.5.4
     */
    public static void requireFinite(double value, String name) {
        if (!Double.isFinite(value)) {
            throw new IllegalArgumentException("Require '" + name + "' (" 
                    + value + ") to be finite.");
        }
    }
}
