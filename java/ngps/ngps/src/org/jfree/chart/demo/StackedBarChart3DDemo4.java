/* ---------------------------
 * StackedBarChart3DDemo4.java
 * ---------------------------
 * (C) Copyright 2002-2020, by Object Refinery Limited.
 * 
 * http://www.object-refinery.com
 *
 */

package org.jfree.chart.demo;

import java.awt.Dimension;
import java.text.DecimalFormat;
import java.text.NumberFormat;

import javax.swing.JPanel;

import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.axis.NumberAxis;
import org.jfree.chart.labels.ItemLabelAnchor;
import org.jfree.chart.labels.ItemLabelPosition;
import org.jfree.chart.labels.StandardCategoryItemLabelGenerator;
import org.jfree.chart.plot.CategoryPlot;
import org.jfree.chart.plot.PlotOrientation;
import org.jfree.chart.renderer.category.StackedBarRenderer;
import org.jfree.chart.ui.ApplicationFrame;
import org.jfree.chart.ui.TextAnchor;
import org.jfree.chart.ui.UIUtils;
import org.jfree.data.category.CategoryDataset;
import org.jfree.data.category.DefaultCategoryDataset;

/**
 * A simple demonstration application showing how to create a stacked 3D bar
 * chart using data from a {@link CategoryDataset}.
 */
public class StackedBarChart3DDemo4 extends ApplicationFrame {

    /**
     * Creates a new demo.
     *
     * @param title  the frame title.
     */
    public StackedBarChart3DDemo4(String title) {
        super(title);
        JPanel chartPanel = createDemoPanel();
        chartPanel.setPreferredSize(new Dimension(500, 270));
        setContentPane(chartPanel);
    }

    /**
     * Creates and returns a {@link CategoryDataset} for the demo chart.
     *
     * @return a sample dataset.
     */
    public static CategoryDataset createDataset() {
        DefaultCategoryDataset dataset = new DefaultCategoryDataset();
        dataset.addValue(10.0, "Series 1", "C1");
        dataset.addValue(5.0, "Series 1", "C2");
        dataset.addValue(6.0, "Series 1", "C3");
        dataset.addValue(7.0, "Series 1", "C4");
        dataset.addValue(8.0, "Series 1", "C5");
        dataset.addValue(9.0, "Series 1", "C6");
        dataset.addValue(10.0, "Series 1", "C7");
        dataset.addValue(11.0, "Series 1", "C8");
        dataset.addValue(3.0, "Series 1", "C9");

        dataset.addValue(4.0, "Series 2", "C1");
        dataset.addValue(7.0, "Series 2", "C2");
        dataset.addValue(17.0, "Series 2", "C3");
        dataset.addValue(15.0, "Series 2", "C4");
        dataset.addValue(6.0, "Series 2", "C5");
        dataset.addValue(8.0, "Series 2", "C6");
        dataset.addValue(9.0, "Series 2", "C7");
        dataset.addValue(13.0, "Series 2", "C8");
        dataset.addValue(7.0, "Series 2", "C9");

        dataset.addValue(15.0, "Series 3", "C1");
        dataset.addValue(14.0, "Series 3", "C2");
        dataset.addValue(12.0, "Series 3", "C3");
        dataset.addValue(11.0, "Series 3", "C4");
        dataset.addValue(10.0, "Series 3", "C5");
        dataset.addValue(0.0, "Series 3", "C6");
        dataset.addValue(7.0, "Series 3", "C7");
        dataset.addValue(9.0, "Series 3", "C8");
        dataset.addValue(11.0, "Series 3", "C9");

        dataset.addValue(14.0, "Series 4", "C1");
        dataset.addValue(3.0, "Series 4", "C2");
        dataset.addValue(7.0, "Series 4", "C3");
        dataset.addValue(0.0, "Series 4", "C4");
        dataset.addValue(9.0, "Series 4", "C5");
        dataset.addValue(6.0, "Series 4", "C6");
        dataset.addValue(7.0, "Series 4", "C7");
        dataset.addValue(9.0, "Series 4", "C8");
        dataset.addValue(10.0, "Series 4", "C9");

        return dataset;

    }

    /**
     * Creates a chart.
     *
     * @param dataset  the dataset.
     *
     * @return The chart.
     */
    private static JFreeChart createChart(CategoryDataset dataset) {

        JFreeChart chart = ChartFactory.createStackedBarChart(
            "Stacked Bar Chart 3D Demo 4",  // chart title
            "Category",                   // domain axis label
            "Value",                      // range axis label
            dataset,                      // data
            PlotOrientation.HORIZONTAL,     // the plot orientation
            true,                         // include legend
            true,                         // tooltips
            false);                       // urls
        CategoryPlot plot = (CategoryPlot) chart.getPlot();
        NumberAxis yAxis = (NumberAxis) plot.getRangeAxis();
        yAxis.setNumberFormatOverride(new DecimalFormat("0%"));
        StackedBarRenderer renderer =
            (StackedBarRenderer) plot.getRenderer();
        renderer.setRenderAsPercentages(true);
        renderer.setDrawBarOutline(false);
        renderer.setDefaultItemLabelGenerator(new StandardCategoryItemLabelGenerator(
                "{3}", NumberFormat.getIntegerInstance(),
                new DecimalFormat("0.0%")));
        renderer.setDefaultItemLabelsVisible(true);
        renderer.setDefaultPositiveItemLabelPosition(new ItemLabelPosition(
                ItemLabelAnchor.CENTER, TextAnchor.CENTER));
        renderer.setDefaultNegativeItemLabelPosition(new ItemLabelPosition(
                ItemLabelAnchor.CENTER, TextAnchor.CENTER));
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
        StackedBarChart3DDemo4 demo = new StackedBarChart3DDemo4(
                "Stacked Bar Chart 3D Demo 4");
        demo.pack();
        UIUtils.centerFrameOnScreen(demo);
        demo.setVisible(true);
    }

}
