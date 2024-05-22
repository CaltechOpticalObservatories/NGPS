/* -------------------------
 * StackedBarChartDemo1.java
 * -------------------------
 * (C) Copyright 2002-2020, by Object Refinery Limited.
 * 
 * http://www.object-refinery.com
 *
 */

package org.jfree.chart.demo;

import java.awt.Color;
import java.awt.font.TextAttribute;
import java.text.AttributedString;
import javax.swing.JPanel;

import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.axis.CategoryAxis;
import org.jfree.chart.axis.CategoryLabelPositions;
import org.jfree.chart.labels.StandardCategoryItemLabelGenerator;
import org.jfree.chart.plot.CategoryPlot;
import org.jfree.chart.renderer.category.StackedBarRenderer;
import org.jfree.chart.renderer.category.StandardBarPainter;
import org.jfree.chart.title.TextTitle;
import org.jfree.chart.ui.ApplicationFrame;
import org.jfree.chart.ui.UIUtils;
import org.jfree.data.category.CategoryDataset;
import org.jfree.data.category.DefaultCategoryDataset;

/**
 * A simple demonstration application showing how to create a stacked bar chart
 * using data from a {@link CategoryDataset}.
 */
public class StackedBarChartDemo1 extends ApplicationFrame {

    /**
     * Creates a new demo.
     *
     * @param title  the frame title.
     */
    public StackedBarChartDemo1(String title) {
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
    private static CategoryDataset createDataset() {
        DefaultCategoryDataset dataset = new DefaultCategoryDataset();
        
        dataset.addValue(197, "Agricultural", "Brazil");
        dataset.addValue(64, "Domestic", "Brazil");
        dataset.addValue(57, "Industrial", "Brazil");
        
        dataset.addValue(339, "Agricultural", "Indonesia");
        dataset.addValue(30, "Domestic", "Indonesia");
        dataset.addValue(4, "Industrial", "Indonesia");
        
        dataset.addValue(279, "Agricultural", "China");
        dataset.addValue(27, "Domestic", "China");
        dataset.addValue(107, "Industrial", "China");

        dataset.addValue(92, "Agricultural", "Germany");
        dataset.addValue(55, "Domestic", "Germany");
        dataset.addValue(313, "Industrial", "Germany");

        dataset.addValue(96, "Agricultural", "Russia");
        dataset.addValue(102, "Domestic", "Russia");
        dataset.addValue(337, "Industrial", "Russia");

        dataset.addValue(403, "Agricultural", "Turkey");
        dataset.addValue(82, "Domestic", "Turkey");
        dataset.addValue(60, "Industrial", "Turkey");
        
        dataset.addValue(536, "Agricultural", "Bangladesh");
        dataset.addValue(17, "Domestic", "Bangladesh");
        dataset.addValue(6, "Industrial", "Bangladesh");
        
        dataset.addValue(508, "Agricultural", "India");
        dataset.addValue(47, "Domestic", "India");
        dataset.addValue(30, "Industrial", "India");
        
        dataset.addValue(428, "Agricultural", "Japan");
        dataset.addValue(138, "Domestic", "Japan");
        dataset.addValue(124, "Industrial", "Japan");

        dataset.addValue(325, "Agricultural", "Italy");
        dataset.addValue(130, "Domestic", "Italy");
        dataset.addValue(268, "Industrial", "Italy");
        
        dataset.addValue(569, "Agricultural", "Mexico");
        dataset.addValue(126, "Domestic", "Mexico");
        dataset.addValue(37, "Industrial", "Mexico");

        dataset.addValue(576, "Agricultural", "Vietnam");
        dataset.addValue(68, "Domestic", "Vietnam");
        dataset.addValue(203, "Industrial", "Vietnam");
        
        dataset.addValue(794, "Agricultural", "Egypt");
        dataset.addValue(74, "Domestic", "Egypt");
        dataset.addValue(55, "Industrial", "Egypt");

        dataset.addValue(954, "Agricultural", "Iran");
        dataset.addValue(21, "Domestic", "Iran");
        dataset.addValue(73, "Industrial", "Iran");

        dataset.addValue(1029, "Agricultural", "Pakistan");
        dataset.addValue(21, "Domestic", "Pakistan");
        dataset.addValue(21, "Industrial", "Pakistan");

        dataset.addValue(1236, "Agricultural", "Thailand");
        dataset.addValue(26, "Domestic", "Thailand");
        dataset.addValue(26, "Industrial", "Thailand");

        dataset.addValue(165, "Agricultural", "Canada");
        dataset.addValue(274, "Domestic", "Canada");
        dataset.addValue(947, "Industrial", "Canada");
        
        dataset.addValue(1363, "Agricultural", "Iraq");
        dataset.addValue(44, "Domestic", "Iraq");
        dataset.addValue(74, "Industrial", "Iraq");
        
        dataset.addValue(656, "Agricultural", "US");
        dataset.addValue(208, "Domestic", "US");
        dataset.addValue(736, "Industrial", "US");
        
        dataset.addValue(2040, "Agricultural", "Uzbekistan");
        dataset.addValue(110, "Domestic", "Uzbekistan");
        dataset.addValue(44, "Industrial", "Uzbekistan");
        
        return dataset;
    }

    /**
     * Creates a sample chart.
     *
     * @param dataset  the dataset for the chart.
     *
     * @return a sample chart.
     */
    private static JFreeChart createChart(CategoryDataset dataset) {

        JFreeChart chart = ChartFactory.createStackedBarChart(
            "Freshwater Usage By Country", "Country", "Value", dataset);
        chart.addSubtitle(new TextTitle("Source: http://en.wikipedia.org/wiki/Peak_water#Water_supply"));
        CategoryPlot plot = (CategoryPlot) chart.getPlot();

        CategoryAxis xAxis = plot.getDomainAxis();
        xAxis.setLowerMargin(0.01);
        xAxis.setUpperMargin(0.01);
        xAxis.setCategoryLabelPositions(CategoryLabelPositions.UP_90);
        AttributedString yLabel = new AttributedString("m3/person/year");
        yLabel.addAttribute(TextAttribute.WEIGHT, 
                TextAttribute.WEIGHT_ULTRABOLD);
        yLabel.addAttribute(TextAttribute.SIZE, 14);
        yLabel.addAttribute(TextAttribute.SUPERSCRIPT, 
                TextAttribute.SUPERSCRIPT_SUPER, 1, 2);
        plot.getRangeAxis().setAttributedLabel(yLabel);
        StackedBarRenderer renderer = (StackedBarRenderer) plot.getRenderer();
        renderer.setDrawBarOutline(false);
        renderer.setBarPainter(new StandardBarPainter());
        renderer.setDefaultItemLabelsVisible(true);
        renderer.setDefaultItemLabelGenerator(
                new StandardCategoryItemLabelGenerator());
        renderer.setDefaultItemLabelPaint(Color.WHITE);
        renderer.setSeriesPaint(0, new Color(0, 55, 122));
        renderer.setSeriesPaint(1, new Color(24, 123, 58));
        renderer.setSeriesPaint(2, Color.RED);
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
        StackedBarChartDemo1 demo = new StackedBarChartDemo1(
                "JFreeChart: StackedBarChartDemo1");
        demo.pack();
        UIUtils.centerFrameOnScreen(demo);
        demo.setVisible(true);
    }

}
