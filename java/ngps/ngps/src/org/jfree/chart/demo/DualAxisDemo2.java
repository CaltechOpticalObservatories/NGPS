/* ------------------
 * DualAxisDemo2.java
 * ------------------
 * (C) Copyright 2002-2020, by Object Refinery Limited.
 * 
 * http://www.object-refinery.com
 *
 */

package org.jfree.chart.demo;

import java.awt.Color;
import java.text.SimpleDateFormat;

import javax.swing.JPanel;

import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartPanel;
import org.jfree.chart.ChartUtils;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.axis.DateAxis;
import org.jfree.chart.axis.NumberAxis;
import org.jfree.chart.block.BlockContainer;
import org.jfree.chart.block.BorderArrangement;
import org.jfree.chart.block.EmptyBlock;
import org.jfree.chart.labels.StandardXYToolTipGenerator;
import org.jfree.chart.plot.XYPlot;
import org.jfree.chart.renderer.xy.XYItemRenderer;
import org.jfree.chart.renderer.xy.XYLineAndShapeRenderer;
import org.jfree.chart.title.CompositeTitle;
import org.jfree.chart.title.LegendTitle;
import org.jfree.chart.ui.ApplicationFrame;
import org.jfree.chart.ui.RectangleEdge;
import org.jfree.chart.ui.UIUtils;
import org.jfree.data.time.Month;
import org.jfree.data.time.TimeSeries;
import org.jfree.data.time.TimeSeriesCollection;
import org.jfree.data.xy.XYDataset;

/**
 * An example of a time series chart.  For the most part, default settings are
 * used, except that  the renderer is modified to show filled shapes (as well
 * as lines) at each data point.
 */
public class DualAxisDemo2 extends ApplicationFrame {

    /**
     * A demonstration application showing how to create a time series chart
     * with dual axes.
     *
     * @param title  the frame title.
     */
    public DualAxisDemo2(String title) {
        super(title);
        JPanel chartPanel = createDemoPanel();
        chartPanel.setPreferredSize(new java.awt.Dimension(500, 270));
        setContentPane(chartPanel);
    }

    /**
     * Creates a sample dataset.
     *
     * @return The dataset.
     */
    private static XYDataset createDataset1() {

        TimeSeries s1 = new TimeSeries("Random Data 1");
        s1.add(new Month(2, 2016), 181.8);
        s1.add(new Month(3, 2016), 167.3);
        s1.add(new Month(4, 2016), 153.8);
        s1.add(new Month(5, 2016), 167.6);
        s1.add(new Month(6, 2016), 158.8);
        s1.add(new Month(7, 2016), 148.3);
        s1.add(new Month(8, 2016), 153.9);
        s1.add(new Month(9, 2016), 142.7);
        s1.add(new Month(10, 2016), 123.2);
        s1.add(new Month(11, 2016), 131.8);
        s1.add(new Month(12, 2016), 139.6);
        s1.add(new Month(1, 2017), 142.9);
        s1.add(new Month(2, 2017), 138.7);
        s1.add(new Month(3, 2017), 137.3);
        s1.add(new Month(4, 2017), 143.9);
        s1.add(new Month(5, 2017), 139.8);
        s1.add(new Month(6, 2017), 137.0);
        s1.add(new Month(7, 2017), 132.8);

        TimeSeriesCollection dataset = new TimeSeriesCollection();
        dataset.addSeries(s1);

        return dataset;

    }

    /**
     * Creates a sample dataset.
     *
     * @return The dataset.
     */
    private static XYDataset createDataset2() {

        TimeSeries s2 = new TimeSeries("Random Data 2");
        s2.add(new Month(2, 2016), 429.6);
        s2.add(new Month(3, 2016), 323.2);
        s2.add(new Month(4, 2016), 417.2);
        s2.add(new Month(5, 2016), 624.1);
        s2.add(new Month(6, 2016), 422.6);
        s2.add(new Month(7, 2016), 619.2);
        s2.add(new Month(8, 2016), 416.5);
        s2.add(new Month(9, 2016), 512.7);
        s2.add(new Month(10, 2016), 501.5);
        s2.add(new Month(11, 2016), 306.1);
        s2.add(new Month(12, 2016), 410.3);
        s2.add(new Month(1, 2017), 511.7);
        s2.add(new Month(2, 2017), 611.0);
        s2.add(new Month(3, 2017), 709.6);
        s2.add(new Month(4, 2017), 613.2);
        s2.add(new Month(5, 2017), 711.6);
        s2.add(new Month(6, 2017), 708.8);
        s2.add(new Month(7, 2017), 501.6);

        TimeSeriesCollection dataset = new TimeSeriesCollection();
        dataset.addSeries(s2);

        return dataset;

    }

    private static JFreeChart createChart() {
        XYDataset dataset = createDataset1();
        JFreeChart chart = ChartFactory.createTimeSeriesChart(
            "Dual Axis Demo 2",
            "Date",
            "Price Per Unit",
            dataset,
            false,     // legend? no, we'll create our own
            true,      // tooltips?
            false      // URLs?
        );

        XYPlot plot = (XYPlot) chart.getPlot();
        plot.setDomainPannable(true);
        plot.setRangePannable(true);
        plot.setDomainCrosshairVisible(true);
        plot.setRangeCrosshairVisible(true);
        plot.setDomainCrosshairLockedOnData(true);
        plot.setRangeCrosshairLockedOnData(true);
        NumberAxis axis2 = new NumberAxis("Secondary");
        axis2.setAutoRangeIncludesZero(false);
        plot.setRangeAxis(1, axis2);
        plot.setDataset(1, createDataset2());
        plot.mapDatasetToRangeAxis(1, 1);
        XYItemRenderer renderer = plot.getRenderer();
        renderer.setDefaultToolTipGenerator(
                StandardXYToolTipGenerator.getTimeSeriesInstance());
        if (renderer instanceof XYLineAndShapeRenderer) {
            XYLineAndShapeRenderer rr = (XYLineAndShapeRenderer) renderer;
            rr.setDefaultShapesVisible(true);
            rr.setDefaultShapesFilled(true);
        }

        XYLineAndShapeRenderer renderer2 = new XYLineAndShapeRenderer();
        renderer2.setSeriesPaint(0, Color.black);
        renderer2.setDefaultShapesVisible(true);
        renderer2.setDefaultToolTipGenerator(
                StandardXYToolTipGenerator.getTimeSeriesInstance());
        plot.setRenderer(1, renderer2);

        DateAxis axis = (DateAxis) plot.getDomainAxis();
        axis.setDateFormatOverride(new SimpleDateFormat("MMM-yyyy"));

        // create two legends and put them into a composite title
        LegendTitle legend1 = new LegendTitle(renderer);
        LegendTitle legend2 = new LegendTitle(renderer2);
        BlockContainer container = new BlockContainer(new BorderArrangement());
        container.add(legend1, RectangleEdge.LEFT);
        container.add(legend2, RectangleEdge.RIGHT);
        container.add(new EmptyBlock(2000, 0));
        CompositeTitle legends = new CompositeTitle(container);
        legends.setPosition(RectangleEdge.BOTTOM);
        chart.addSubtitle(legends);

        ChartUtils.applyCurrentTheme(chart);
        return chart;
    }

    /**
     * Creates a panel for the demo (used by SuperDemo.java).
     *
     * @return A panel.
     */
    public static JPanel createDemoPanel() {
        JFreeChart chart = createChart();
        ChartPanel panel = new ChartPanel(chart);
        panel.setMouseWheelEnabled(true);
        return panel;
    }

    /**
     * Starting point for the demonstration application.
     *
     * @param args  ignored.
     */
    public static void main(String[] args) {
        DualAxisDemo2 demo = new DualAxisDemo2(
                "JFreeChart: DualAxisDemo2.java");
        demo.pack();
        UIUtils.centerFrameOnScreen(demo);
        demo.setVisible(true);
    }

}
