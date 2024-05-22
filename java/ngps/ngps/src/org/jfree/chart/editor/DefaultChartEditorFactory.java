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
 * ------------------------------
 * DefaultChartEditorFactory.java
 * ------------------------------
 * (C) Copyright 2005-present, by David Gilbert.
 *
 * Original Author:  David Gilbert;
 * Contributor(s):   ;
 *
 */

package org.jfree.chart.editor;

import org.jfree.chart.JFreeChart;

/**
 * A default implementation of the {@link ChartEditorFactory} interface.
 */
public class DefaultChartEditorFactory implements ChartEditorFactory {

    /**
     * Creates a new instance.
     */
    public DefaultChartEditorFactory() {
    }

    /**
     * Returns a new instance of a {@link ChartEditor}.
     *
     * @param chart  the chart.
     *
     * @return A chart editor for the given chart.
     */
    @Override
    public ChartEditor createEditor(JFreeChart chart) {
        return new DefaultChartEditor(chart);
    }

}
