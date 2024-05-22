/* -------------------------
 * ScatterRendererDemo1.java
 * -------------------------
 * (C) Copyright 2007-2020, by Object Refinery Limited.
 * 
 * http://www.object-refinery.com
 *
 */

package org.jfree.chart.demo;

import java.awt.Color;
import java.util.ArrayList;
import java.util.List;

import javax.swing.JPanel;

import org.jfree.chart.ChartPanel;
import org.jfree.chart.ChartUtils;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.axis.CategoryAxis;
import org.jfree.chart.axis.NumberAxis;
import org.jfree.chart.plot.CategoryPlot;
import org.jfree.chart.renderer.category.ScatterRenderer;
import org.jfree.chart.ui.ApplicationFrame;
import org.jfree.chart.ui.RectangleInsets;
import org.jfree.chart.ui.UIUtils;
import org.jfree.data.statistics.DefaultMultiValueCategoryDataset;
import org.jfree.data.statistics.MultiValueCategoryDataset;

/**
 * A simple demonstration for the {@link ScatterRenderer} class.
 */
public class ScatterRendererDemo1 extends ApplicationFrame {

    /**
     * Creates a new demo instance.
     *
     * @param title  the frame title.
     */
    public ScatterRendererDemo1(String title) {
        super(title);
        JPanel chartPanel = createDemoPanel();
        chartPanel.setPreferredSize(new java.awt.Dimension(500, 270));
        setContentPane(chartPanel);
    }

    private static List<Double> listOfValues(double[] values) {
        List<Double> result = new ArrayList<>();
        for (int i = 0; i < values.length; i++) {
            result.add(values[i]);
        }
        return result;
    }

    /**
     * Creates a sample dataset.
     *
     * @return A dataset.
     */
    private static MultiValueCategoryDataset createDataset() {
        DefaultMultiValueCategoryDataset dataset
                = new DefaultMultiValueCategoryDataset();
        dataset.add(listOfValues(new double[] {1.0, 2.0, 3.0}), "Series 1", "C1");
        dataset.add(listOfValues(new double[] {1.2, 2.2, 3.2}), "Series 1", "C2");
        dataset.add(listOfValues(new double[] {1.4, 2.4, 3.4}), "Series 1", "C3");
        dataset.add(listOfValues(new double[] {1.0, 3.0}), "Series 2", "C1");
        dataset.add(listOfValues(new double[] {1.2, 3.2}), "Series 2", "C2");
        dataset.add(listOfValues(new double[] {1.4, 3.6}), "Series 2", "C3");
        return dataset;
    }

    /**
     * Creates a chart.
     *
     * @param dataset  the dataset.
     *
     * @return A chart.
     */
    private static JFreeChart createChart(MultiValueCategoryDataset dataset) {
        CategoryPlot plot = new CategoryPlot(dataset,
                new CategoryAxis("Category"), new NumberAxis("Value"),
                new ScatterRenderer());
        plot.setBackgroundPaint(Color.LIGHT_GRAY);
        plot.setDomainGridlinePaint(Color.WHITE);
        plot.setRangeGridlinePaint(Color.WHITE);
        plot.setAxisOffset(new RectangleInsets(4, 4, 4, 4));
        JFreeChart chart = new JFreeChart("ScatterRendererDemo1", plot);
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
        return new ChartPanel(chart);
    }

    /**
     * Starting point for the demonstration application.
     *
     * @param args  ignored.
     */
    public static void main(String[] args) {

        ScatterRendererDemo1 demo = new ScatterRendererDemo1(
                "JFreeChart: ScatterRendererDemo1.java");
        demo.pack();
        UIUtils.centerFrameOnScreen(demo);
        demo.setVisible(true);

    }

}
