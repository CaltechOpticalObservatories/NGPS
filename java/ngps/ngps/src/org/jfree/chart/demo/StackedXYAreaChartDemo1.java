/* ----------------------------
 * StackedXYAreaChartDemo1.java
 * ----------------------------
 * (C) Copyright 2004-2020, by Object Refinery Limited.
 * 
 * http://www.object-refinery.com
 *
 */

package org.jfree.chart.demo;

import java.awt.Color;

import javax.swing.JPanel;

import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.labels.StandardXYToolTipGenerator;
import org.jfree.chart.plot.XYPlot;
import org.jfree.chart.renderer.xy.StackedXYAreaRenderer;
import org.jfree.chart.ui.ApplicationFrame;
import org.jfree.chart.ui.UIUtils;
import org.jfree.data.xy.DefaultTableXYDataset;
import org.jfree.data.xy.TableXYDataset;
import org.jfree.data.xy.XYSeries;

/**
 * This demo shows the creation of a stacked XY area chart.  I created this 
 * demo while investigating bug report 882890.
 */
public class StackedXYAreaChartDemo1 extends ApplicationFrame {

    /**
     * Creates a new demo.
     *
     * @param title  the frame title.
     */
    public StackedXYAreaChartDemo1(String title) {
        super(title);
        JPanel chartPanel = createDemoPanel();
        chartPanel.setPreferredSize(new java.awt.Dimension(500, 270));
        setContentPane(chartPanel);
    }
    
    /**
     * Creates a sample dataset.
     * 
     * @return a sample dataset.
     */
    private static TableXYDataset createDataset() {
        DefaultTableXYDataset dataset = new DefaultTableXYDataset();
        XYSeries s1 = new XYSeries("Series 1", true, false);
        s1.add(5.0, 5.0);
        s1.add(10.0, 15.5);
        s1.add(15.0, 9.5);
        s1.add(20.0, 7.5);
        dataset.addSeries(s1);
        XYSeries s2 = new XYSeries("Series 2", true, false);
        s2.add(5.0, 5.0);
        s2.add(10.0, 15.5);
        s2.add(15.0, 9.5);
        s2.add(20.0, 3.5);
        dataset.addSeries(s2);
        return dataset;
    }
    
    /**
     * Creates a sample chart.
     * 
     * @param dataset  the dataset for the chart.
     * 
     * @return a sample chart.
     */
    private static JFreeChart createChart(TableXYDataset dataset) {
        JFreeChart chart = ChartFactory.createStackedXYAreaChart(
            "Stacked XY Area Chart Demo 1", "X Value", "Y Value", dataset);
        XYPlot plot = (XYPlot) chart.getPlot();
        StackedXYAreaRenderer renderer = new StackedXYAreaRenderer();
        renderer.setSeriesPaint(0, Color.LIGHT_GRAY);
        renderer.setDefaultToolTipGenerator(new StandardXYToolTipGenerator());
        plot.setRenderer(0, renderer);
        plot.setDomainCrosshairVisible(true);
        plot.setRangeCrosshairVisible(true);
        return chart;
    }

    /**
     * Creates a panel for the demo (used by SuperDemo.java).
     * 
     * @return A panel.
     */
    public static JPanel createDemoPanel() {
        JFreeChart chart = createChart(createDataset());
        return new ChartPanel(chart);
    }

    /**
     * Starting point for the demonstration application.
     *
     * @param args  ignored.
     */
    public static void main(String[] args) {
        StackedXYAreaChartDemo1 demo = new StackedXYAreaChartDemo1(
                "Stacked XY Area Chart Demo 1");
        demo.pack();
        UIUtils.centerFrameOnScreen(demo);
        demo.setVisible(true);
    }

}
