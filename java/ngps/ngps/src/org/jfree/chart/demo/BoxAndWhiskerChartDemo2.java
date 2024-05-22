/* ----------------------------
 * BoxAndWhiskerChartDemo2.java
 * ----------------------------
 * (C) Copyright 2005-2020, by Object Refinery Limited.
 * 
 * http://www.object-refinery.com
 *
 */

package org.jfree.chart.demo;

import java.awt.Color;
import java.awt.Dimension;
import java.util.ArrayList;
import java.util.List;

import javax.swing.JPanel;

import org.jfree.chart.ChartPanel;
import org.jfree.chart.ChartUtils;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.axis.DateAxis;
import org.jfree.chart.axis.NumberAxis;
import org.jfree.chart.plot.XYPlot;
import org.jfree.chart.renderer.xy.XYBoxAndWhiskerRenderer;
import org.jfree.chart.renderer.xy.XYItemRenderer;
import org.jfree.chart.ui.ApplicationFrame;
import org.jfree.chart.ui.UIUtils;
import org.jfree.data.statistics.BoxAndWhiskerCalculator;
import org.jfree.data.statistics.BoxAndWhiskerXYDataset;
import org.jfree.data.statistics.DefaultBoxAndWhiskerXYDataset;
import org.jfree.data.time.Day;
import org.jfree.data.time.RegularTimePeriod;

/**
 * A simple demonstration application showing how to create a box-and-whisker
 * chart (on an XYPlot).
 */
public class BoxAndWhiskerChartDemo2 extends ApplicationFrame {

    /**
     * Creates a new demo instance.
     *
     * @param title  the frame title.
     */
    public BoxAndWhiskerChartDemo2(String title) {
        super(title);
        JPanel chartPanel = createDemoPanel();
        chartPanel.setPreferredSize(new Dimension(500, 270));
        setContentPane(chartPanel);
    }

    /**
     * Returns a sample dataset.
     *
     * @return The dataset.
     */
    private static BoxAndWhiskerXYDataset createDataset() {
        final int VALUE_COUNT = 20;
        DefaultBoxAndWhiskerXYDataset result
                = new DefaultBoxAndWhiskerXYDataset("Series Name");
        RegularTimePeriod t = new Day();
        for (int i = 0; i < 10; i++) {
            List values = createValueList(0, 20.0, VALUE_COUNT);
            result.add(t.getStart(),
                    BoxAndWhiskerCalculator.calculateBoxAndWhiskerStatistics(
                    values));
            t = t.next();
        }
        return result;
    }

    private static List<Double> createValueList(double lowerBound, 
            double upperBound, int count) {
        List<Double> result = new ArrayList<>();
        for (int i = 0; i < count; i++) {
            double v = lowerBound + (Math.random() * (upperBound - lowerBound));
            result.add(v);
        }
        return result;
    }

    /**
     * Creates a sample chart.
     *
     * @param dataset  the dataset.
     *
     * @return The chart.
     */
    private static JFreeChart createChart(BoxAndWhiskerXYDataset dataset) {

        DateAxis domainAxis = new DateAxis("Day");
        NumberAxis rangeAxis = new NumberAxis("Value");
        XYItemRenderer renderer = new XYBoxAndWhiskerRenderer();
        XYPlot plot = new XYPlot(dataset, domainAxis, rangeAxis, renderer);
        JFreeChart chart = new JFreeChart("Box-and-Whisker Chart Demo 2", plot);

        chart.setBackgroundPaint(Color.WHITE);
        plot.setBackgroundPaint(Color.LIGHT_GRAY);
        plot.setDomainGridlinePaint(Color.WHITE);
        plot.setDomainGridlinesVisible(true);
        plot.setRangeGridlinePaint(Color.WHITE);
        plot.setDomainPannable(true);
        plot.setRangePannable(true);
        rangeAxis.setStandardTickUnits(NumberAxis.createIntegerTickUnits());
        ChartUtils.applyCurrentTheme(chart);
        return chart;

    }

    /**
     * Creates a panel for the demo (used by SuperDemo.java).
     *
     * @return A panel.
     */
    public static JPanel createDemoPanel() {
        JFreeChart chart = createChart(createDataset());
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
        BoxAndWhiskerChartDemo2 demo = new BoxAndWhiskerChartDemo2(
            "JFreeChart: BoxAndWhiskerChartDemo2.java");
        demo.pack();
        UIUtils.centerFrameOnScreen(demo);
        demo.setVisible(true);
    }

}
