/* ----------------------------
 * NormalDistributionDemo1.java
 * ----------------------------
 * (C) Copyright 2004-2020, by Object Refinery Limited.
 * 
 * http://www.object-refinery.com
 *
 */

package org.jfree.chart.demo;

import javax.swing.JPanel;

import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.ui.ApplicationFrame;
import org.jfree.chart.ui.UIUtils;
import org.jfree.data.function.Function2D;
import org.jfree.data.function.NormalDistributionFunction2D;
import org.jfree.data.general.DatasetUtils;
import org.jfree.data.xy.XYDataset;

/**
 * This demo shows a normal distribution function.
 */
public class NormalDistributionDemo1 extends ApplicationFrame {

    /**
     * A demonstration application showing a normal distribution.
     *
     * @param title  the frame title.
     */
    public NormalDistributionDemo1(String title) {
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
        return new ChartPanel(chart);
    }

    /**
     * Creates a dataset with sample values from the normal distribution
     * function.
     *
     * @return A dataset.
     */
    public static XYDataset createDataset() {
        Function2D normal = new NormalDistributionFunction2D(0.0, 1.0);
        XYDataset dataset = DatasetUtils.sampleFunction2D(normal, -5.0,
                5.0, 100, "Normal");
        return dataset;
    }

    /**
     * Creates a line chart using the data from the supplied dataset.
     *
     * @param dataset  the dataset.
     *
     * @return The chart.
     */
    public static JFreeChart createChart(XYDataset dataset) {
        JFreeChart chart = ChartFactory.createXYLineChart(
            "Normal Distribution",
            "X",
            "Y",
            dataset);
        return chart;
    }

    /**
     * Starting point for the demonstration application.
     *
     * @param args  ignored.
     */
    public static void main(String[] args) {
        NormalDistributionDemo1 demo = new NormalDistributionDemo1(
                "JFreeChart: NormalDistributionDemo1.java");
        demo.pack();
        UIUtils.centerFrameOnScreen(demo);
        demo.setVisible(true);
    }

}
