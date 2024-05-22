/* --------------------
 * YieldCurveDemo1.java
 * --------------------
 * (C) Copyright 2005-2020, by Object Refinery Limited.
 * 
 * http://www.object-refinery.com
 *
 */

package org.jfree.chart.demo;


import java.awt.Font;
import java.awt.Graphics2D;
import java.awt.geom.Rectangle2D;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;
import java.util.GregorianCalendar;
import java.util.List;
import javax.swing.JPanel;
import org.jfree.chart.ChartFactory;

import org.jfree.chart.ChartPanel;
import org.jfree.chart.ChartUtils;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.axis.AxisState;
import org.jfree.chart.axis.DateAxis;
import org.jfree.chart.axis.DateTick;
import org.jfree.chart.plot.XYPlot;
import org.jfree.chart.renderer.xy.XYItemRenderer;
import org.jfree.chart.renderer.xy.XYLineAndShapeRenderer;
import org.jfree.chart.title.TextTitle;
import org.jfree.chart.ui.ApplicationFrame;
import org.jfree.chart.ui.HorizontalAlignment;
import org.jfree.chart.ui.RectangleEdge;
import org.jfree.chart.ui.TextAnchor;
import org.jfree.chart.ui.UIUtils;
import org.jfree.data.time.Day;
import org.jfree.data.time.TimeSeries;
import org.jfree.data.time.TimeSeriesCollection;
import org.jfree.data.xy.XYDataset;

/**
 * A yield curve demo.  This illustrates how to override the refreshTicks()
 * method in the DateAxis class to get the axis to display a custom set of
 * tick labels.
 */
public class YieldCurveDemo1 extends ApplicationFrame {

    /**
     * A custom date axis.
     */
    static class CustomDateAxis extends DateAxis {

        private final Date base;

        /**
         * Creates a new axis.
         *
         * @param label  the axis label.
         * @param base  the base date.
         */
        public CustomDateAxis(String label, Date base) {
            super(label);
            this.base = base;
        }

        /**
         * Return a custom list of ticks.  This is a crude implementation, it
         * doesn't take into account the plot orientation or the axis location
         * (assumes the bottom of the chart) or attempt to change the number
         * of tick labels to avoid overlapping (we've omitted the 3m and 6m
         * labels manually, but it would be possible to write code to compute
         * this).
         *
         * @param g2  the graphics target.
         * @param state  the axis state.
         * @param dataArea  the data area.
         * @param edge  the edge along which the axis lies.
         *
         * @return A list of ticks.
         */
        @Override
        public List refreshTicks(Graphics2D g2, AxisState state,
                Rectangle2D dataArea, RectangleEdge edge) {
            List<DateTick> result = new ArrayList<>();
            // start with base
            GregorianCalendar cal = new GregorianCalendar();
            cal.setTime(this.base);
            cal.add(Calendar.MONTH, 1);
            result.add(new DateTick(cal.getTime(), "1M", TextAnchor.TOP_CENTER,
                    TextAnchor.CENTER, 0.0));
            cal.add(Calendar.MONTH, 11);
            result.add(new DateTick(cal.getTime(), "1Y", TextAnchor.TOP_CENTER,
                    TextAnchor.CENTER, 0.0));
            cal.add(Calendar.YEAR, 1);
            result.add(new DateTick(cal.getTime(), "2Y", TextAnchor.TOP_CENTER,
                    TextAnchor.CENTER, 0.0));
            cal.add(Calendar.YEAR, 1);
            result.add(new DateTick(cal.getTime(), "3Y", TextAnchor.TOP_CENTER,
                    TextAnchor.CENTER, 0.0));
            cal.add(Calendar.YEAR, 2);
            result.add(new DateTick(cal.getTime(), "5Y", TextAnchor.TOP_CENTER,
                    TextAnchor.CENTER, 0.0));
            cal.add(Calendar.YEAR, 5);
            result.add(new DateTick(cal.getTime(), "10Y", TextAnchor.TOP_CENTER,
                    TextAnchor.CENTER, 0.0));
            cal.add(Calendar.YEAR, 10);
            result.add(new DateTick(cal.getTime(), "20Y", TextAnchor.TOP_CENTER,
                    TextAnchor.CENTER, 0.0));
            return result;
        }
    }

    /**
     * Creates a dataset.
     * 
     * https://www.treasury.gov/resource-center/data-chart-center/interest-rates/Pages/TextView.aspx?data=yield
     * 
     * @return the dataset.
     */
    public static XYDataset createDataset() {
        TimeSeries s1 = new TimeSeries("US$ Treasury Yields");
        Day m1 = new Day(24, 4, 2016);
        Day m3 = new Day(24, 6, 2016);
        Day m6 = new Day(24, 10, 2016);
        Day y1 = new Day(24, 3, 2017);
        Day y2 = new Day(24, 3, 2018);
        Day y3 = new Day(24, 3, 2019);
        Day y5 = new Day(24, 3, 2021);
        Day y7 = new Day(24, 3, 2023);
        Day y10 = new Day(24, 3, 2026);
        Day y20 = new Day(24, 3, 2036);

        s1.add(m1, 0.24);
        s1.add(m3, 0.30);
        s1.add(m6, 0.46);
        s1.add(y1, 0.63);
        s1.add(y2, 0.89);
        s1.add(y3, 1.05);
        s1.add(y5, 1.39);
        s1.add(y7, 1.70);
        s1.add(y10, 1.91);
        s1.add(y20, 2.28);

        TimeSeriesCollection dataset = new TimeSeriesCollection();
        dataset.addSeries(s1);
        return dataset;
    }
    
    /**
     * Creates a chart.
     *
     * @param dataset  a dataset.
     *
     * @return A chart.
     */
    public static JFreeChart createChart(XYDataset dataset) {
        JFreeChart chart = ChartFactory.createTimeSeriesChart(
                "US$ Treasury Yields", "Date", "Yield", dataset);
        chart.removeLegend();
        XYPlot plot = (XYPlot) chart.getPlot();
        GregorianCalendar cal = new GregorianCalendar(2016, Calendar.MARCH,
                23);
        plot.setDomainAxis(new CustomDateAxis("Date", cal.getTime()));
        plot.setDomainCrosshairVisible(true);
        plot.setRangeCrosshairVisible(true);

        XYItemRenderer r = plot.getRenderer();
        if (r instanceof XYLineAndShapeRenderer) {
            XYLineAndShapeRenderer renderer = (XYLineAndShapeRenderer) r;
            renderer.setDefaultShapesVisible(true);
            renderer.setDefaultShapesFilled(true);
        }

        DateAxis axis = (DateAxis) plot.getDomainAxis();
        axis.setDateFormatOverride(new SimpleDateFormat("MMM-yyyy"));
        chart.addSubtitle(new TextTitle("March 2016"));
        TextTitle source = new TextTitle(
                "Source: https://www.treasury.gov/resource-center/data-chart-center/interest-rates/");
        ChartUtils.applyCurrentTheme(chart);
        source.setFont(new Font("Dialog", Font.PLAIN, 9));
        source.setPosition(RectangleEdge.BOTTOM);
        source.setHorizontalAlignment(HorizontalAlignment.RIGHT);
        chart.addSubtitle(source);
        return chart;
    }

    /**
     * A demonstration application showing how to create a simple time series
     * chart.  This example uses monthly data.
     *
     * @param title  the frame title.
     */
    public YieldCurveDemo1(String title) {
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
     * Starting point for the demonstration application.
     *
     * @param args  ignored.
     */
    public static void main(String[] args) {
        YieldCurveDemo1 demo = new YieldCurveDemo1(
                "JFreeChart: YieldCurveDemo1.java");
        demo.pack();
        UIUtils.centerFrameOnScreen(demo);
        demo.setVisible(true);
    }

}
