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
 * ----------------
 * YisSymbolic.java
 * ----------------
 * (C) Copyright 2006-present, by Anthony Boulestreau and Contributors.
 *
 * Original Author:  Anthony Boulestreau;
 * Contributor(s):   David Gilbert;
 *
 */

package org.jfree.data.xy;

/**
 * Represent a data set where Y is a symbolic values. Each symbolic value is
 * linked with an Integer.
 */
public interface YisSymbolic {

    /**
     * Returns the list of symbolic values.
     *
     * @return The symbolic values.
     */
    String[] getYSymbolicValues();

    /**
     * Returns the symbolic value of the data set specified by
     * {@code series} and {@code item} parameters.
     *
     * @param series  the series index (zero-based).
     * @param item  the item index (zero-based).
     *
     * @return The symbolic value.
     */
    String getYSymbolicValue(int series, int item);

    /**
     * Returns the symbolic value linked with the specified {@code Integer}.
     *
     * @param val  value of the integer linked with the symbolic value.
     *
     * @return The symbolic value.
     */
    String getYSymbolicValue(Integer val);

}
