/* ----------------------
 * SampleXYZDataset2.java
 * ----------------------
 * (C) Copyright 2005-2020, by Object Refinery Limited.
 * 
 * http://www.object-refinery.com
 *
 */

package org.jfree.chart.demo;

import org.jfree.data.xy.AbstractXYZDataset;
import org.jfree.data.xy.XYZDataset;

/**
 * A quick-and-dirty implementation of the {@link XYZDataset interface}.  
 * Hard-coded and not useful beyond the demo.
 */
public class SampleXYZDataset2 extends AbstractXYZDataset 
        implements XYZDataset {

    /** The x values. */
    private double[][] xVal = {{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}};

    /** The y values. */
    private double[][] yVal = {{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}};

    /** The z values. */
    private double[][] zVal = {{1.1, 2.2, 3.3}, {4.4, 5.5, 6.6}};

    /**
     * Returns the number of series in the dataset.
     *
     * @return The series count.
     */
    @Override
    public int getSeriesCount() {
        return this.xVal.length;
    }

    /**
     * Returns the key for a series.
     *
     * @param series  the series (zero-based index).
     *
     * @return The key for the series.
     */
    @Override
    public Comparable getSeriesKey(int series) {
        return "Series " + series;
    }

    /**
     * Returns the number of items in a series.
     *
     * @param series  the series (zero-based index).
     *
     * @return The number of items within the series.
     */
    @Override
    public int getItemCount(int series) {
        return this.xVal[0].length;
    }

    /**
     * Returns the x-value for an item within a series.
     * <P>
     * The implementation is responsible for ensuring that the x-values are
     * presented in ascending order.
     *
     * @param series  the series (zero-based index).
     * @param item  the item (zero-based index).
     *
     * @return The x-value.
     */
    @Override
    public Number getX(int series, int item) {
        return this.xVal[series][item];
    }

    /**
     * Returns the y-value for an item within a series.
     *
     * @param series  the series (zero-based index).
     * @param item  the item (zero-based index).
     *
     * @return The y-value.
     */
    @Override
    public Number getY(int series, int item) {
        return this.yVal[series][item];
    }

    /**
     * Returns the z-value for the specified series and item.
     *
     * @param series  the series index (zero-based).
     * @param item  the item index (zero-based).
     *
     * @return The value.
     */
    @Override
    public Number getZ(int series, int item) {
        return this.zVal[series][item];
    }
    
}
