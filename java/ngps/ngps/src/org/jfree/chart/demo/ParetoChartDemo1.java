/* ---------------------
 * ParetoChartDemo1.java
 * ---------------------
 * (C) Copyright 2003-2020, by Object Refinery Limited.
 * 
 * http://www.object-refinery.com
 *
 */

package org.jfree.chart.demo;

import java.awt.Font;
import java.text.NumberFormat;

import javax.swing.JPanel;

import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartPanel;
import org.jfree.chart.ChartUtils;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.axis.CategoryAxis;
import org.jfree.chart.axis.CategoryLabelPositions;
import org.jfree.chart.axis.NumberAxis;
import org.jfree.chart.plot.CategoryPlot;
import org.jfree.chart.plot.DatasetRenderingOrder;
import org.jfree.chart.renderer.category.LineAndShapeRenderer;
import org.jfree.chart.title.TextTitle;
import org.jfree.chart.ui.ApplicationFrame;
import org.jfree.chart.ui.HorizontalAlignment;
import org.jfree.chart.ui.RectangleEdge;
import org.jfree.chart.ui.UIUtils;
import org.jfree.chart.util.SortOrder;
import org.jfree.data.DataUtils;
import org.jfree.data.DefaultKeyedValues;
import org.jfree.data.KeyedValues;
import org.jfree.data.category.CategoryDataset;
import org.jfree.data.general.DatasetUtils;

/**
 * A demo showing the creation of a pareto chart.
 */
public class ParetoChartDemo1 extends ApplicationFrame {

    /**
     * Creates a new demo instance.
     *
     * @param title  the frame title.
     */
    public ParetoChartDemo1(String title) {
        super(title);
        JPanel chartPanel = createDemoPanel();
        chartPanel.setPreferredSize(new java.awt.Dimension(550, 270));
        setContentPane(chartPanel);
    }

    /**
     * Creates the chart.
     *
     * @param datasets  the datasets.
     *
     * @return The chart.
     */
    public static JFreeChart createChart(CategoryDataset[] datasets) {
        // create the chart...
        JFreeChart chart = ChartFactory.createBarChart(
            "TIOBE Index of Programming Languages", 
            null, "Index Value", datasets[0]);

        chart.addSubtitle(new TextTitle("As at August 2013"));
        chart.removeLegend();
        // get a reference to the plot for further customisation...
        CategoryPlot plot = (CategoryPlot) chart.getPlot();
        CategoryAxis domainAxis = plot.getDomainAxis();
        domainAxis.setLowerMargin(0.02);
        domainAxis.setUpperMargin(0.02);
        domainAxis.setCategoryLabelPositions(CategoryLabelPositions.UP_90);
        // set the range axis to display integers only...
        NumberAxis rangeAxis = (NumberAxis) plot.getRangeAxis();
        rangeAxis.setStandardTickUnits(NumberAxis.createIntegerTickUnits());

        LineAndShapeRenderer renderer2 = new LineAndShapeRenderer();

        NumberAxis axis2 = new NumberAxis("Percent");
        axis2.setNumberFormatOverride(NumberFormat.getPercentInstance());
        plot.setRangeAxis(1, axis2);
        plot.setDataset(1, datasets[1]);
        plot.setRenderer(1, renderer2);
        plot.mapDatasetToRangeAxis(1, 1);

        plot.setDatasetRenderingOrder(DatasetRenderingOrder.FORWARD);

        ChartUtils.applyCurrentTheme(chart);
        TextTitle source = new TextTitle("http://www.tiobe.com/index.php/content/paperinfo/tpci/index.html", 
                new Font("Monospaced", Font.PLAIN, 10));
        source.setPosition(RectangleEdge.BOTTOM);
        source.setHorizontalAlignment(HorizontalAlignment.RIGHT);
        chart.addSubtitle(source);
        return chart;
    }

    /**
     * Creates the sample datasets.
     *
     * @return The sample datasets.
     */
    public static CategoryDataset[] createDatasets() {
        DefaultKeyedValues data = new DefaultKeyedValues();
        data.addValue("C", 15.974);
        data.addValue("C++", 9.371);
        data.addValue("C#", 6.117);
        data.addValue("Java", 15.978);
        data.addValue("Javascript", 2.093);
        data.addValue("Obj-C", 8.082);
        data.addValue("PHP", 6.694);
        data.addValue("Python", 3.603);
        data.addValue("Ruby", 2.067);
        data.addValue("VB", 3.873);

        data.sortByValues(SortOrder.DESCENDING);
        KeyedValues cumulative = DataUtils.getCumulativePercentages(data);
        CategoryDataset dataset = DatasetUtils.createCategoryDataset(
                "Languages", data);
        CategoryDataset dataset2 = DatasetUtils.createCategoryDataset(
                "Cumulative", cumulative);
        return new CategoryDataset[] { dataset, dataset2 };
    }

    /**
     * Creates a panel for the demo (used by SuperDemo.java).
     *
     * @return A panel.
     */
    public static JPanel createDemoPanel() {
        CategoryDataset[] datasets = createDatasets();
        JFreeChart chart = createChart(datasets);
        return new ChartPanel(chart);
    }

    /**
     * Starting point for the demonstration application.
     *
     * @param args  ignored.
     */
    public static void main(String[] args) {
        ParetoChartDemo1 demo = new ParetoChartDemo1(
                "JFreeChart: ParetoChartDemo1.java");
        demo.pack();
        UIUtils.centerFrameOnScreen(demo);
        demo.setVisible(true);
    }

}
