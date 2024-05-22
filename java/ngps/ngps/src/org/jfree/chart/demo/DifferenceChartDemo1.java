/* ------------------------
 * DifferenceChartDemo.java
 * ------------------------
 * (C) Copyright 2003-2020, by Object Refinery Limited.
 * 
 * http://www.object-refinery.com
 *
 */

package org.jfree.chart.demo;


import java.awt.Color;
import javax.swing.JPanel;
import org.jfree.chart.ChartFactory;

import org.jfree.chart.ChartPanel;
import org.jfree.chart.ChartUtils;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.axis.DateAxis;
import org.jfree.chart.axis.ValueAxis;
import org.jfree.chart.plot.XYPlot;
import org.jfree.chart.renderer.xy.XYDifferenceRenderer;
import org.jfree.chart.ui.ApplicationFrame;
import org.jfree.chart.ui.UIUtils;
import org.jfree.data.time.Day;
import org.jfree.data.time.TimeSeries;
import org.jfree.data.time.TimeSeriesCollection;
import org.jfree.data.xy.XYDataset;

/**
 * A simple demonstration of the {@link XYDifferenceRenderer}.
 */
public class DifferenceChartDemo1 extends ApplicationFrame {

    /**
     * Creates a sample dataset.
     *
     * @return A sample dataset.
     */
    public static XYDataset createDataset() {
        TimeSeries series1 = new TimeSeries("Random 1");
        TimeSeries series2 = new TimeSeries("Random 2");
        double value1 = 0.0;
        double value2 = 0.0;
        Day day = new Day();
        for (int i = 0; i < 200; i++) {
            value1 = value1 + Math.random() - 0.5;
            value2 = value2 + Math.random() - 0.5;
            series1.add(day, value1);
            series2.add(day, value2);
            day = (Day) day.next();
        }
        TimeSeriesCollection dataset = new TimeSeriesCollection();
        dataset.addSeries(series1);
        dataset.addSeries(series2);
        return dataset;
    }

    /**
     * Creates a chart.
     *
     * @param dataset  the dataset.
     *
     * @return The chart.
     */
    public static JFreeChart createChart(XYDataset dataset) {
        JFreeChart chart = ChartFactory.createTimeSeriesChart(
                "Difference Chart Demo 1", "Time", "Value", dataset);

        XYPlot plot = (XYPlot) chart.getPlot();
        plot.setDomainPannable(true);
        XYDifferenceRenderer r = new XYDifferenceRenderer(Color.GREEN,
                Color.RED, false);
        r.setRoundXCoordinates(true);
        plot.setDomainCrosshairLockedOnData(true);
        plot.setRangeCrosshairLockedOnData(true);
        plot.setDomainCrosshairVisible(true);
        plot.setRangeCrosshairVisible(true);
        plot.setRenderer(r);

        ValueAxis domainAxis = new DateAxis("Time");
        domainAxis.setLowerMargin(0.0);
        domainAxis.setUpperMargin(0.0);
        plot.setDomainAxis(domainAxis);
        plot.setForegroundAlpha(0.5f);
        ChartUtils.applyCurrentTheme(chart);
        return chart;
    }
    
    /**
     * Creates a new demo.
     *
     * @param title  the frame title.
     */
    public DifferenceChartDemo1(String title) {
        super(title);
        JPanel chartPanel = createDemoPanel();
        chartPanel.setPreferredSize(new java.awt.Dimension(500, 270));
        setContentPane(chartPanel);
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
        DifferenceChartDemo1 demo = new DifferenceChartDemo1(
                "JFreeChart: DifferenceChartDemo1.java");
        demo.pack();
        UIUtils.centerFrameOnScreen(demo);
        demo.setVisible(true);
    }

}
