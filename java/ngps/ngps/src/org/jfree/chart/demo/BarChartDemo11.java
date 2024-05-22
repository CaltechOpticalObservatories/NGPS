/* -------------------
 * BarChartDemo11.java
 * -------------------
 * (C) Copyright 2006-2020, by Object Refinery Limited.
 * 
 * http://www.object-refinery.com
 *
 */

package org.jfree.chart.demo;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.GradientPaint;
import java.text.DecimalFormat;

import javax.swing.JPanel;

import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.axis.NumberAxis;
import org.jfree.chart.labels.StandardCategoryToolTipGenerator;
import org.jfree.chart.plot.CategoryPlot;
import org.jfree.chart.plot.PlotOrientation;
import org.jfree.chart.renderer.category.BarRenderer;
import org.jfree.chart.title.TextTitle;
import org.jfree.chart.ui.ApplicationFrame;
import org.jfree.chart.ui.RectangleEdge;
import org.jfree.chart.ui.UIUtils;
import org.jfree.data.category.CategoryDataset;
import org.jfree.data.category.DefaultCategoryDataset;

/**
 * A bar chart showing licence statistics for open source projects listed
 * at Freshmeat.
 */
public class BarChartDemo11 extends ApplicationFrame {

    /**
     * Creates a new demo instance.
     *
     * @param title  the frame title.
     */
    public BarChartDemo11(String title) {
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
    private static CategoryDataset createDataset() {
        DefaultCategoryDataset dataset = new DefaultCategoryDataset();
        dataset.addValue(33, "S1", "GNU General Public License (GPL) 2.0");
        dataset.addValue(13, "S1", "Apache License 2.0");
        dataset.addValue(12, "S1", "GNU General Public License (GPL) 3.0");
        dataset.addValue(11, "S1", "MIT License");
        dataset.addValue(7, "S1", "BSD License 2.0");
        dataset.addValue(6, "S1", "Artistic Licence (Perl)");
        dataset.addValue(6, "S1", "GNU Lesser General Public License (LGPL) 2.1");
        dataset.addValue(3, "S1", "GNU Lesser General Public License (LGPL) 3.0");
        dataset.addValue(2, "S1", "Eclipse Public License");
        dataset.addValue(1, "S1", "Code Project 1.02 License");
        return dataset;
    }

    /**
     * Creates a sample chart.
     *
     * @param dataset  the dataset.
     *
     * @return The chart.
     */
    private static JFreeChart createChart(CategoryDataset dataset) {

        // create the chart...
        JFreeChart chart = ChartFactory.createBarChart(
                "Open Source Projects By License", "License", "Percent", 
                dataset);
        chart.removeLegend();
        
        TextTitle source = new TextTitle(
                "Source: http://www.blackducksoftware.com/resources/data/top-20-licenses (as at 30 Aug 2013)",
                new Font("Dialog", Font.PLAIN, 9));
        source.setPosition(RectangleEdge.BOTTOM);
        chart.addSubtitle(source);

        // get a reference to the plot for further customisation...
        CategoryPlot plot = (CategoryPlot) chart.getPlot();
        plot.setOrientation(PlotOrientation.HORIZONTAL);
        plot.setDomainGridlinesVisible(true);

        plot.getDomainAxis().setMaximumCategoryLabelWidthRatio(0.8f);
        // set the range axis to display integers only...
        NumberAxis rangeAxis = (NumberAxis) plot.getRangeAxis();
        rangeAxis.setStandardTickUnits(NumberAxis.createIntegerTickUnits());

        // disable bar outlines...
        BarRenderer renderer = (BarRenderer) plot.getRenderer();
        renderer.setDrawBarOutline(false);

        StandardCategoryToolTipGenerator tt
                = new StandardCategoryToolTipGenerator("{1}: {2} percent",
                new DecimalFormat("0"));
        renderer.setDefaultToolTipGenerator(tt);

        // set up gradient paints for series...
        GradientPaint gp0 = new GradientPaint(0.0f, 0.0f, Color.BLUE,
                0.0f, 0.0f, new Color(0, 0, 64));
        renderer.setSeriesPaint(0, gp0);

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
        BarChartDemo11 demo = new BarChartDemo11(
                "JFreeChart: BarChartDemo11.java");
        demo.pack();
        UIUtils.centerFrameOnScreen(demo);
        demo.setVisible(true);
    }

}
