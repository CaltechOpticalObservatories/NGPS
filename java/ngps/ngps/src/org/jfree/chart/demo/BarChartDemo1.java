/* ------------------
 * BarChartDemo1.java
 * ------------------
 * (C) Copyright 2002-2020, by Object Refinery Limited.
 * 
 * http://www.object-refinery.com
 *
 */

package org.jfree.chart.demo;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Font;

import javax.swing.JPanel;

import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.axis.NumberAxis;
import org.jfree.chart.labels.StandardCategorySeriesLabelGenerator;
import org.jfree.chart.plot.CategoryPlot;
import org.jfree.chart.renderer.category.BarRenderer;
import org.jfree.chart.renderer.category.StandardBarPainter;
import org.jfree.chart.title.LegendTitle;
import org.jfree.chart.title.TextTitle;
import org.jfree.chart.ui.ApplicationFrame;
import org.jfree.chart.ui.HorizontalAlignment;
import org.jfree.chart.ui.RectangleEdge;
import org.jfree.chart.ui.UIUtils;
import org.jfree.data.category.CategoryDataset;
import org.jfree.data.category.DefaultCategoryDataset;

/**
 * A simple demonstration application showing how to create a bar chart.
 */
public class BarChartDemo1 extends ApplicationFrame {

    /**
     * Creates a new demo instance.
     *
     * @param title  the frame title.
     */
    public BarChartDemo1(String title) {
        super(title);
        JPanel chartPanel = createDemoPanel();
        chartPanel.setPreferredSize(new Dimension(720, 480));
        setContentPane(chartPanel);
    }

    /**
     * Returns a dataset.
     *
     * https://www.cdc.gov/nchs/products/databriefs/db377.htm
     * 
     * @return The dataset.
     */
    private static CategoryDataset createDatasetX() {
        String series1 = "18 and over";
        String series2 = "18 - 39";
        String series3 = "40 - 59";
        String series4 = "60 and over";

        DefaultCategoryDataset dataset = new DefaultCategoryDataset();

        String both = "Both sexes";
        String male = "Male";
        String female = "Female";
        
        dataset.addValue(13.2, series1, both);
        dataset.addValue(8.4, series1, male);
        dataset.addValue(17.7, series1, female);

        dataset.addValue(7.9, series2, both);
        dataset.addValue(5.5, series2, male);
        dataset.addValue(10.3, series2, female);

        dataset.addValue(14.4, series3, both);
        dataset.addValue(8.4, series3, male);
        dataset.addValue(20.1, series3, female);

        dataset.addValue(19.0, series4, both);
        dataset.addValue(12.8, series4, male);
        dataset.addValue(24.3, series4, female);

        return dataset;
    }

    /**
     * Returns a dataset.
     *
     * https://www.cdc.gov/nchs/products/databriefs/db377.htm
     * 
     * @return The dataset.
     */
    private static CategoryDataset createDataset() {
        DefaultCategoryDataset dataset = new DefaultCategoryDataset();

        String series1 = "Males";
        String series2 = "Females";

        String category1 = "18 to 39";
        String category2 = "40 - 59";
        String category3 = "60 and over";
        
        dataset.addValue(5.5, series1, category1);
        dataset.addValue(10.3, series2, category1);

        dataset.addValue(8.4, series1, category2);
        dataset.addValue(20.1, series2, category2);

        dataset.addValue(12.8, series1, category3);
        dataset.addValue(24.3, series2, category3);

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
        JFreeChart chart = ChartFactory.createBarChart("Antidepressant Medication Usage", 
                "Age Category", "Percent", dataset);

        // remove the legend temporarily
        LegendTitle legend = chart.getLegend();
        chart.removeLegend(); 
        
        // add some subtitles
        chart.addSubtitle(new TextTitle("Percentage of adults aged 18 and over who used antidepressant medication over past 30 days, by age and sex: United States, 2015-2018"));
        TextTitle source = new TextTitle("Source: https://www.cdc.gov/nchs/products/databriefs/db377.htm");
        source.setFont(new Font("SansSerif", Font.PLAIN, 10));
        source.setPosition(RectangleEdge.BOTTOM);
        source.setHorizontalAlignment(HorizontalAlignment.RIGHT);
        chart.addSubtitle(source);
        chart.addSubtitle(legend); // add back the legend
        CategoryPlot plot = (CategoryPlot) chart.getPlot();
        plot.setDomainGridlinesVisible(true);
        plot.setRangeCrosshairVisible(true);
        plot.setRangeCrosshairPaint(Color.BLUE);

        plot.getDomainAxis().setCategoryMargin(0.2);
        
        // set the range axis to display integers only...
        NumberAxis rangeAxis = (NumberAxis) plot.getRangeAxis();
        rangeAxis.setStandardTickUnits(NumberAxis.createIntegerTickUnits());

        // disable bar outlines...
        BarRenderer renderer = (BarRenderer) plot.getRenderer();
        renderer.setDrawBarOutline(false);
        renderer.setBarPainter(new StandardBarPainter());
        renderer.setItemMargin(0.06);

        renderer.setLegendItemToolTipGenerator(
                new StandardCategorySeriesLabelGenerator("Tooltip: {0}"));

        return chart;

    }

    /**
     * Creates a panel for the demo (used by JFreeChartDemo.java).
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
        BarChartDemo1 demo = new BarChartDemo1("JFreeChart: BarChartDemo1.java");
        demo.pack();
        UIUtils.centerFrameOnScreen(demo);
        demo.setVisible(true);
    }

}
