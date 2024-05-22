/* -------------------
 * AreaChartDemo1.java
 * -------------------
 * (C) Copyright 2002-2020, by Object Refinery Limited.
 * 
 * http://www.object-refinery.com
 *
 */

package org.jfree.chart.demo;

import java.awt.Dimension;

import javax.swing.JPanel;

import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartPanel;
import org.jfree.chart.ChartUtils;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.axis.CategoryAxis;
import org.jfree.chart.axis.CategoryLabelPositions;
import org.jfree.chart.axis.NumberAxis;
import org.jfree.chart.plot.CategoryPlot;
import org.jfree.chart.renderer.AreaRendererEndType;
import org.jfree.chart.renderer.category.AreaRenderer;
import org.jfree.chart.title.TextTitle;
import org.jfree.chart.ui.ApplicationFrame;
import org.jfree.chart.ui.RectangleEdge;
import org.jfree.chart.ui.RectangleInsets;
import org.jfree.chart.ui.UIUtils;
import org.jfree.chart.ui.VerticalAlignment;
import org.jfree.chart.util.UnitType;
import org.jfree.data.category.CategoryDataset;
import org.jfree.data.category.DefaultCategoryDataset;

/**
 * A simple demonstration application showing how to create an area chart
 * using data from a {@link CategoryDataset}.
 */
public class AreaChartDemo1 extends ApplicationFrame {

    /**
     * Creates a new demo application.
     *
     * @param title  the frame title.
     */
    public AreaChartDemo1(String title) {
        super(title);
        JPanel chartPanel = createDemoPanel();
        chartPanel.setPreferredSize(new Dimension(500, 270));
        setContentPane(chartPanel);
    }

    /**
     * Creates a sample dataset.
     *
     * @return A sample dataset.
     */
    private static CategoryDataset createDataset() {
        DefaultCategoryDataset dataset = new DefaultCategoryDataset();
        dataset.addValue(1.0, "Series 1", "Type 1");
        dataset.addValue(4.0, "Series 1", "Type 2");
        dataset.addValue(3.0, "Series 1", "Type 3");
        dataset.addValue(5.0, "Series 1", "Type 4");
        dataset.addValue(5.0, "Series 1", "Type 5");
        dataset.addValue(7.0, "Series 1", "Type 6");
        dataset.addValue(7.0, "Series 1", "Type 7");
        dataset.addValue(8.0, "Series 1", "Type 8");
        dataset.addValue(5.0, "Series 2", "Type 1");
        dataset.addValue(7.0, "Series 2", "Type 2");
        dataset.addValue(6.0, "Series 2", "Type 3");
        dataset.addValue(8.0, "Series 2", "Type 4");
        dataset.addValue(4.0, "Series 2", "Type 5");
        dataset.addValue(4.0, "Series 2", "Type 6");
        dataset.addValue(2.0, "Series 2", "Type 7");
        dataset.addValue(1.0, "Series 2", "Type 8");
        dataset.addValue(4.0, "Series 3", "Type 1");
        dataset.addValue(3.0, "Series 3", "Type 2");
        dataset.addValue(2.0, "Series 3", "Type 3");
        dataset.addValue(3.0, "Series 3", "Type 4");
        dataset.addValue(6.0, "Series 3", "Type 5");
        dataset.addValue(3.0, "Series 3", "Type 6");
        dataset.addValue(4.0, "Series 3", "Type 7");
        dataset.addValue(3.0, "Series 3", "Type 8");
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

        JFreeChart chart = ChartFactory.createAreaChart("Area Chart", 
                "Category", "Value", dataset);
        TextTitle subtitle = new TextTitle(
            "An area chart demonstration.  We use this subtitle as an "
            + "example of what happens when you get a really long title or "
            + "subtitle."
        );
        subtitle.setPosition(RectangleEdge.TOP);
        subtitle.setPadding(new RectangleInsets(UnitType.RELATIVE, 0.05, 0.05,
                0.05, 0.05));
        subtitle.setVerticalAlignment(VerticalAlignment.BOTTOM);
        chart.addSubtitle(subtitle);

        CategoryPlot plot = (CategoryPlot) chart.getPlot();
        plot.setForegroundAlpha(0.5f);

        plot.setDomainGridlinesVisible(true);

        AreaRenderer renderer = (AreaRenderer) plot.getRenderer();
        renderer.setEndType(AreaRendererEndType.LEVEL);
        CategoryAxis domainAxis = plot.getDomainAxis();
        domainAxis.setCategoryLabelPositions(CategoryLabelPositions.UP_45);
        domainAxis.setLowerMargin(0.0);
        domainAxis.setUpperMargin(0.0);
        domainAxis.addCategoryLabelToolTip("Type 1", "The first type.");
        domainAxis.addCategoryLabelToolTip("Type 2", "The second type.");
        domainAxis.addCategoryLabelToolTip("Type 3", "The third type.");

        NumberAxis rangeAxis = (NumberAxis) plot.getRangeAxis();
        rangeAxis.setStandardTickUnits(NumberAxis.createIntegerTickUnits());
        rangeAxis.setLabelAngle(0 * Math.PI / 2.0);
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
     * Starting point for the demonstration application when it is run as
     * a stand-alone application.
     *
     * @param args  ignored.
     */
    public static void main(String[] args) {
        AreaChartDemo1 demo = new AreaChartDemo1(
                "JFreeChart: AreaChartDemo1.java");
        demo.pack();
        UIUtils.centerFrameOnScreen(demo);
        demo.setVisible(true);
    }

}
