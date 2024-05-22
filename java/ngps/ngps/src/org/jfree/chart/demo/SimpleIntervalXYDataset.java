/* ----------------------------
 * SimpleIntervalXYDataset.java
 * ----------------------------
 * (C) Copyright 2002-2020, by Object Refinery Limited.
 * 
 * http://www.object-refinery.com
 *
 */

package org.jfree.chart.demo;

import org.jfree.data.general.DatasetChangeListener;
import org.jfree.data.xy.AbstractIntervalXYDataset;
import org.jfree.data.xy.IntervalXYDataset;

/**
 * A quick and dirty sample dataset.
 */
public class SimpleIntervalXYDataset extends AbstractIntervalXYDataset 
            implements IntervalXYDataset {

    /** The start values. */
    private Double[] xStart = new Double[3];
    
    /** The end values. */
    private Double[] xEnd = new Double[3];

    /** The y values. */
    private Double[] yValues = new Double[3];

    /**
     * Creates a new dataset.
     */
    public SimpleIntervalXYDataset() {
        this.xStart[0] = 0.0;
        this.xStart[1] = 2.0;
        this.xStart[2] = 3.5;

        this.xEnd[0] = 2.0;
        this.xEnd[1] = 3.5;
        this.xEnd[2] = 4.0;

        this.yValues[0] = 3.0;
        this.yValues[1] = 4.5;
        this.yValues[2] = 2.5;
    }

    /**
     * Returns the number of series in the dataset.
     *
     * @return the number of series in the dataset.
     */
    @Override
    public int getSeriesCount() {
        return 1;
    }

    /**
     * Returns the key for a series.
     *
     * @param series the series (zero-based index).
     *
     * @return The series key.
     */
    @Override
    public Comparable getSeriesKey(int series) {
        return "Series 1";
    }

    /**
     * Returns the number of items in a series.
     *
     * @param series the series (zero-based index).
     *
     * @return the number of items within a series.
     */
    @Override
    public int getItemCount(int series) {
        return 3;
    }

    /**
     * Returns the x-value for an item within a series.
     * <P>
     * The implementation is responsible for ensuring that the x-values are presented in ascending
     * order.
     *
     * @param series  the series (zero-based index).
     * @param item  the item (zero-based index).
     *
     * @return  the x-value for an item within a series.
     */
    @Override
    public Number getX(int series, int item) {
        return this.xStart[item];
    }

    /**
     * Returns the y-value for an item within a series.
     *
     * @param series  the series (zero-based index).
     * @param item  the item (zero-based index).
     *
     * @return the y-value for an item within a series.
     */
    @Override
    public Number getY(int series, int item) {
        return this.yValues[item];
    }

    /**
     * Returns the starting X value for the specified series and item.
     *
     * @param series  the series (zero-based index).
     * @param item  the item within a series (zero-based index).
     *
     * @return The value.
     */
    @Override
    public Number getStartX(int series, int item) {
        return this.xStart[item];
    }

    /**
     * Returns the ending X value for the specified series and item.
     *
     * @param series  the series (zero-based index).
     * @param item  the item within a series (zero-based index).
     *
     * @return the end x value.
     */
    @Override
    public Number getEndX(int series, int item) {
        return this.xEnd[item];
    }

    /**
     * Returns the starting Y value for the specified series and item.
     *
     * @param series  the series (zero-based index).
     * @param item  the item within a series (zero-based index).
     *
     * @return The value.
     */
    @Override
    public Number getStartY(int series, int item) {
        return this.yValues[item];
    }

    /**
     * Returns the ending Y value for the specified series and item.
     *
     * @param series  the series (zero-based index).
     * @param item  the item within a series (zero-based index).
     *
     * @return The value.
     */
    @Override
    public Number getEndY(int series, int item) {
        return this.yValues[item];
    }

    /**
     * Registers an object for notification of changes to the dataset.
     *
     * @param listener  the object to register.
     */
    @Override
    public void addChangeListener(DatasetChangeListener listener) {
        // ignored
    }

    /**
     * Deregisters an object for notification of changes to the dataset.
     *
     * @param listener  the object to deregister.
     */
    @Override
    public void removeChangeListener(DatasetChangeListener listener) {
        // ignored
    }

}
