/* ---------------------
 * ScatterPlotDemo6.java
 * ---------------------
 * (C) Copyright 2017-2020, by Object Refinery Limited.
 * 
 * http://www.object-refinery.com
 *
 */

package org.jfree.chart.demo;

import javax.swing.JPanel;
import javax.swing.SwingUtilities;

import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.plot.XYPlot;
import org.jfree.chart.renderer.xy.XYDotRenderer;
import org.jfree.chart.ui.ApplicationFrame;
import org.jfree.chart.ui.RectangleInsets;
import org.jfree.chart.ui.UIUtils;
import org.jfree.data.xy.XYDataset;
import org.jfree.data.xy.XYSeries;
import org.jfree.data.xy.XYSeriesCollection;

/**
 * A scatter plot demo using the {@link XYDotRenderer} class.
 */
public class ScatterPlotDemo6 extends ApplicationFrame {

    /**
     * A demonstration application showing a scatter plot.
     *
     * @param title  the frame title.
     */
    public ScatterPlotDemo6(String title) {
        super(title);
        JPanel chartPanel = createDemoPanel();
        chartPanel.setPreferredSize(new java.awt.Dimension(500, 270));
        setContentPane(chartPanel);
    }

    private static XYSeries createSeries(String name, int items) {
        XYSeries s1 = new XYSeries(name);
        for (int i = 0; i < items; i++) {
            double theta = Math.random() * Math.PI * 2.0;
            double r = Math.random() * 50.0;
            double rho = r * r;
            s1.add(Math.cos(theta) * rho, Math.sin(theta) * rho);
        } 
        return s1;
    }
    
    public static XYDataset createDataset() {
        XYSeriesCollection dataset = new XYSeriesCollection();
        dataset.addSeries(createSeries("S1", 25000));
        dataset.addSeries(createSeries("S2", 25000));
        dataset.addSeries(createSeries("S3", 25000));
        dataset.addSeries(createSeries("S4", 25000));
        return dataset;
    }
    
    /**
     * Creates a panel for the demo (used by SuperDemo.java).
     *
     * @return A panel.
     */
    public static JPanel createDemoPanel() {
        JFreeChart chart = ChartFactory.createScatterPlot("Scatter Plot Demo 6",
            "X", "Y", createDataset());

        XYPlot plot = (XYPlot) chart.getPlot();
        plot.setBackgroundPaint(null);
        plot.setAxisOffset(RectangleInsets.ZERO_INSETS);
        plot.setOutlineVisible(false);
        XYDotRenderer renderer = new XYDotRenderer();
        renderer.setDotWidth(2);
        renderer.setDotHeight(2);
        plot.setRenderer(renderer);
        plot.setDomainCrosshairVisible(true);
        plot.setRangeCrosshairVisible(true);
        
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
        SwingUtilities.invokeLater(new Runnable() {
            @Override
            public void run() {
                ScatterPlotDemo6 demo = new ScatterPlotDemo6(
                    "JFreeChart: ScatterPlotDemo6.java");
                demo.pack();
                UIUtils.centerFrameOnScreen(demo);
                demo.setVisible(true);
            }
        });
    }

}


