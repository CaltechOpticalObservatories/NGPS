/* -----------------------------
 * SimpleIntervalXYDataset2.java
 * -----------------------------
 * (C) Copyright 2003-2020, by Object Refinery Limited.
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
public class SimpleIntervalXYDataset2 extends AbstractIntervalXYDataset 
                                      implements IntervalXYDataset {

    /** The start values. */
    private Double[] yStart;
    
    /** The end values. */
    private Double[] yEnd = new Double[3];

    /** The x values. */
    private Double[] xValues = new Double[3];

    /**
     * Creates a new dataset.
     * 
     * @param itemCount  the number of items to generate.
     */
    public SimpleIntervalXYDataset2(int itemCount) {

        this.xValues = new Double[itemCount];
        this.yStart = new Double[itemCount];
        this.yEnd = new Double[itemCount];
        
        double base = 100;
        for (int i = 1; i <= itemCount; i++) {
            this.xValues[i - 1] = (double) i;
            base = base * (1 + (Math.random() / 10 - 0.05));
            this.yStart[i - 1] = base;
            this.yEnd[i - 1] = this.yStart[i - 1] + Math.random() * 30;
        }
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
        return this.xValues.length;
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
        return this.xValues[item];
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
        return this.yEnd[item];
    }

    /**
     * Returns the starting X value for the specified series and item.
     *
     * @param series  the series (zero-based index).
     * @param item  the item within a series (zero-based index).
     *
     * @return the start x value.
     */
    @Override
    public Number getStartX(int series, int item) {
        return this.xValues[item];
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
        return this.xValues[item];
    }

    /**
     * Returns the starting Y value for the specified series and item.
     *
     * @param series  the series (zero-based index).
     * @param item  the item within a series (zero-based index).
     *
     * @return the start y value.
     */
    @Override
    public Number getStartY(int series, int item) {
        return this.yStart[item];
    }

    /**
     * Returns the ending Y value for the specified series and item.
     *
     * @param series  the series (zero-based index).
     * @param item  the item within a series (zero-based index).
     *
     * @return the end y value.
     */
    @Override
    public Number getEndY(int series, int item) {
        return this.yEnd[item];
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
