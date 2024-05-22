/* ------------------------
 * CombinedXYPlotDemo5.java
 * ------------------------
 * (C) Copyright 2003-2020, by Object Refinery Limited.
 * 
 * http://www.object-refinery.com
 *
 */

package org.jfree.chart.demo;

import java.awt.Font;
import java.awt.Point;
import java.awt.geom.Rectangle2D;

import org.jfree.chart.ChartMouseEvent;
import org.jfree.chart.ChartMouseListener;

import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.annotations.XYTextAnnotation;
import org.jfree.chart.axis.AxisLocation;
import org.jfree.chart.axis.NumberAxis;
import org.jfree.chart.axis.ValueAxis;
import org.jfree.chart.plot.CombinedDomainXYPlot;
import org.jfree.chart.plot.PlotOrientation;
import org.jfree.chart.plot.XYPlot;
import org.jfree.chart.renderer.xy.StandardXYItemRenderer;
import org.jfree.chart.renderer.xy.XYItemRenderer;
import org.jfree.chart.ui.ApplicationFrame;
import org.jfree.chart.ui.RectangleEdge;
import org.jfree.chart.ui.RectangleInsets;
import org.jfree.chart.ui.UIUtils;
import org.jfree.data.xy.XYDataset;
import org.jfree.data.xy.XYSeries;
import org.jfree.data.xy.XYSeriesCollection;

/**
 * A demonstration application showing how to create a combined chart.
 */
public class CombinedXYPlotDemo5 extends ApplicationFrame 
        implements ChartMouseListener {

    private final ChartPanel chartPanel;
    
    /**
     * Constructs a new demonstration application.
     *
     * @param title  the frame title.
     */
    public CombinedXYPlotDemo5(String title) {
        super(title);
        JFreeChart chart = createCombinedChart();
        this.chartPanel = new ChartPanel(chart);
        this.chartPanel.setPreferredSize(new java.awt.Dimension(500, 270));
        this.chartPanel.addChartMouseListener(this);
        setContentPane(this.chartPanel);
    }

    /**
     * Creates a combined chart.
     *
     * @return The combined chart.
     */
    private static JFreeChart createCombinedChart() {

        // create subplot 1...
        XYDataset data1 = createDataset1();
        XYItemRenderer renderer1 = new StandardXYItemRenderer();
        NumberAxis rangeAxis1 = new NumberAxis("Range 1");
        XYPlot subplot1 = new XYPlot(data1, null, rangeAxis1, renderer1);
        subplot1.setRangeAxisLocation(AxisLocation.BOTTOM_OR_LEFT);

        XYTextAnnotation annotation = new XYTextAnnotation("Hello!", 50.0,
                10000.0);
        annotation.setFont(new Font("SansSerif", Font.PLAIN, 9));
        annotation.setRotationAngle(Math.PI / 4.0);
        subplot1.addAnnotation(annotation);

        // create subplot 2...
        XYDataset data2 = createDataset2();
        XYItemRenderer renderer2 = new StandardXYItemRenderer();
        NumberAxis rangeAxis2 = new NumberAxis("Range 2");
        rangeAxis2.setAutoRangeIncludesZero(false);
        XYPlot subplot2 = new XYPlot(data2, null, rangeAxis2, renderer2);
        subplot2.setRangeAxisLocation(AxisLocation.TOP_OR_LEFT);

        // parent plot...
        NumberAxis sharedAxis = new NumberAxis("Domain");
        sharedAxis.setTickMarkInsideLength(5.0f);
        CombinedDomainXYPlot plot = new CombinedDomainXYPlot(sharedAxis);
        plot.setAxisOffset(RectangleInsets.ZERO_INSETS);
        plot.setGap(10.0);

        // add the subplots...
        plot.add(subplot1, 1);
        plot.add(subplot2, 1);
        plot.setOrientation(PlotOrientation.VERTICAL);

        // return a new chart containing the overlaid plot...
        JFreeChart chart = new JFreeChart("CombinedDomainXYPlotDemo5",
                JFreeChart.DEFAULT_TITLE_FONT, plot, true);
        //ChartUtilities.applyCurrentTheme(chart);
        return chart;

    }

    /**
     * Creates a sample dataset.
     *
     * @return Series 1.
     */
    private static XYDataset createDataset1() {

        // create dataset 1...
        XYSeries series1 = new XYSeries("Series 1");
        series1.add(10.0, 12353.3);
        series1.add(20.0, 13734.4);
        series1.add(30.0, 14525.3);
        series1.add(40.0, 13984.3);
        series1.add(50.0, 12999.4);
        series1.add(60.0, 14274.3);
        series1.add(70.0, 15943.5);
        series1.add(80.0, 14845.3);
        series1.add(90.0, 14645.4);
        series1.add(100.0, 16234.6);
        series1.add(110.0, 17232.3);
        series1.add(120.0, 14232.2);
        series1.add(130.0, 13102.2);
        series1.add(140.0, 14230.2);
        series1.add(150.0, 11235.2);

        XYSeries series2 = new XYSeries("Series 2");
        series2.add(10.0, 15000.3);
        series2.add(20.0, 11000.4);
        series2.add(30.0, 17000.3);
        series2.add(40.0, 15000.3);
        series2.add(50.0, 14000.4);
        series2.add(60.0, 12000.3);
        series2.add(70.0, 11000.5);
        series2.add(80.0, 12000.3);
        series2.add(90.0, 13000.4);
        series2.add(100.0, 12000.6);
        series2.add(110.0, 13000.3);
        series2.add(120.0, 17000.2);
        series2.add(130.0, 18000.2);
        series2.add(140.0, 16000.2);
        series2.add(150.0, 17000.2);

        XYSeriesCollection collection = new XYSeriesCollection();
        collection.addSeries(series1);
        collection.addSeries(series2);
        return collection;

    }

    /**
     * Creates a sample dataset.
     *
     * @return Series 2.
     */
    private static XYDataset createDataset2() {

        // create dataset 2...
        XYSeries series2 = new XYSeries("Series 3");

        series2.add(10.0, 16853.2);
        series2.add(20.0, 19642.3);
        series2.add(30.0, 18253.5);
        series2.add(40.0, 15352.3);
        series2.add(50.0, 13532.0);
        series2.add(100.0, 12635.3);
        series2.add(110.0, 13998.2);
        series2.add(120.0, 11943.2);
        series2.add(130.0, 16943.9);
        series2.add(140.0, 17843.2);
        series2.add(150.0, 16495.3);
        series2.add(160.0, 17943.6);
        series2.add(170.0, 18500.7);
        series2.add(180.0, 19595.9);

        return new XYSeriesCollection(series2);

    }

    /**
     * Starting point for the demonstration application.
     *
     * @param args  ignored.
     */
    public static void main(String[] args) {
        CombinedXYPlotDemo5 demo = new CombinedXYPlotDemo5(
                "JFreeChart: CombinedDomainXYPlotDemo5.java");
        demo.pack();
        UIUtils.centerFrameOnScreen(demo);
        demo.setVisible(true);
    }

    @Override
    public void chartMouseClicked(ChartMouseEvent event) {
        int mx = event.getTrigger().getX();
        int my = event.getTrigger().getY();
        Rectangle2D dataArea = this.chartPanel.getScreenDataArea(mx, my);
        JFreeChart chart = event.getChart();
        CombinedDomainXYPlot plot = (CombinedDomainXYPlot) chart.getPlot();
        XYPlot subplot = plot.findSubplot(
                this.chartPanel.getChartRenderingInfo().getPlotInfo(), 
                new Point(mx, my));
        if (subplot != null) {
            ValueAxis xAxis = subplot.getDomainAxis();
            ValueAxis yAxis = subplot.getRangeAxis();
            double x = xAxis.java2DToValue(mx, dataArea, RectangleEdge.BOTTOM);
            double y = yAxis.java2DToValue(my, dataArea, RectangleEdge.LEFT);
            System.out.println("You clicked the point (" + x + ", " + y + ")");
        }
    }

    @Override
    public void chartMouseMoved(ChartMouseEvent cme) {
        // let's ignore this
    }

}
