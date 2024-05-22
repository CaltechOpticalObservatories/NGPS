/* ------------------------
 * CategoryMarkerDemo1.java
 * ------------------------
 * (C) Copyright 2005-2020, by Object Refinery Limited.
 * 
 * http://www.object-refinery.com
 *
 */

package org.jfree.chart.demo;

import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Font;

import javax.swing.JPanel;

import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.axis.NumberAxis;
import org.jfree.chart.plot.CategoryMarker;
import org.jfree.chart.plot.CategoryPlot;
import org.jfree.chart.plot.PlotOrientation;
import org.jfree.chart.renderer.category.LineAndShapeRenderer;
import org.jfree.chart.ui.ApplicationFrame;
import org.jfree.chart.ui.Layer;
import org.jfree.chart.ui.RectangleInsets;
import org.jfree.chart.ui.TextAnchor;
import org.jfree.chart.ui.UIUtils;
import org.jfree.data.category.CategoryDataset;
import org.jfree.data.category.DefaultCategoryDataset;

/**
 * A simple line chart with a category marker to highlight one category.
 */
public class CategoryMarkerDemo1 extends ApplicationFrame {

    /**
     * Creates a new demo.
     *
     * @param title  the frame title.
     */
    public CategoryMarkerDemo1(String title) {
        super(title);
        JPanel chartPanel = createDemoPanel();
        chartPanel.setPreferredSize(new Dimension(500, 270));
        setContentPane(chartPanel);
    }

    /**
     * Creates a sample dataset.
     *
     * @return The dataset.
     */
    private static CategoryDataset createDataset() {
        DefaultCategoryDataset dataset = new DefaultCategoryDataset();
        dataset.addValue(21, "Series 1", "Category 1");
        dataset.addValue(50, "Series 1", "Category 2");
        dataset.addValue(152, "Series 1", "Category 3");
        dataset.addValue(184, "Series 1", "Category 4");
        dataset.addValue(299, "Series 1", "Category 5");
        return dataset;
    }

    /**
     * Creates a sample chart.
     *
     * @param dataset  a dataset.
     *
     * @return The chart.
     */
    private static JFreeChart createChart(CategoryDataset dataset) {

        // create the chart...
        JFreeChart chart = ChartFactory.createLineChart(
            "Category Marker Demo 1",        // chart title
            "Category",                      // domain axis label
            "Count",                         // range axis label
            dataset,                         // data
            PlotOrientation.VERTICAL,        // orientation
            true,                            // include legend
            true,                            // tooltips
            false                            // urls
        );


        CategoryPlot plot = (CategoryPlot) chart.getPlot();

        // customise the range axis...
        NumberAxis rangeAxis = (NumberAxis) plot.getRangeAxis();
        rangeAxis.setStandardTickUnits(NumberAxis.createIntegerTickUnits());

        // customise the renderer...
        LineAndShapeRenderer renderer
            = (LineAndShapeRenderer) plot.getRenderer();
        renderer.setSeriesShapesVisible(0, true);
        renderer.setDrawOutlines(true);
        renderer.setUseFillPaint(true);
        renderer.setDefaultFillPaint(Color.WHITE);

        // add a category marker
        CategoryMarker marker = new CategoryMarker("Category 4", Color.BLUE,
                new BasicStroke(1.0f));
        marker.setDrawAsLine(true);
        marker.setLabel("Marker Label");
        marker.setLabelFont(new Font("Dialog", Font.PLAIN, 11));
        marker.setLabelTextAnchor(TextAnchor.TOP_RIGHT);
        marker.setLabelOffset(new RectangleInsets(2, 5, 2, 5));
        plot.addDomainMarker(marker, Layer.BACKGROUND);

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
        CategoryMarkerDemo1 demo = new CategoryMarkerDemo1(
                "JFreeChart: CategoryMarkerDemo1.java");
        demo.pack();
        UIUtils.centerFrameOnScreen(demo);
        demo.setVisible(true);
    }

}
