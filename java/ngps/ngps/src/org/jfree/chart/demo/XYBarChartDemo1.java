/* --------------------
 * XYBarChartDemo1.java
 * --------------------
 * (C) Copyright 2002-2020, by Object Refinery Limited.
 * 
 * http://www.object-refinery.com
 *
 */

package org.jfree.chart.demo;

import java.awt.Font;
import java.text.DecimalFormat;
import java.text.SimpleDateFormat;

import javax.swing.JPanel;

import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartPanel;
import org.jfree.chart.ChartUtils;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.axis.DateAxis;
import org.jfree.chart.axis.DateTickMarkPosition;
import org.jfree.chart.labels.StandardXYToolTipGenerator;
import org.jfree.chart.plot.PlotOrientation;
import org.jfree.chart.plot.XYPlot;
import org.jfree.chart.renderer.xy.XYBarRenderer;
import org.jfree.chart.title.TextTitle;
import org.jfree.chart.ui.ApplicationFrame;
import org.jfree.chart.ui.UIUtils;
import org.jfree.data.time.TimeSeries;
import org.jfree.data.time.TimeSeriesCollection;
import org.jfree.data.time.Year;
import org.jfree.data.xy.IntervalXYDataset;

/**
 * A simple demonstration application showing how to create a bar chart using
 * an {@link XYPlot}.
 */
public class XYBarChartDemo1 extends ApplicationFrame {

    /**
     * Constructs the demo application.
     *
     * @param title  the frame title.
     */
    public XYBarChartDemo1(String title) {

        super(title);
        JPanel chartPanel = createDemoPanel();
        chartPanel.setPreferredSize(new java.awt.Dimension(500, 270));
        setContentPane(chartPanel);

    }

    private static JFreeChart createChart(IntervalXYDataset dataset) {
        JFreeChart chart = ChartFactory.createXYBarChart(
            "State Executions - USA",
            "Year",
            true,
            "Number of People",
            dataset,
            PlotOrientation.VERTICAL,
            true,
            false,
            false
        );

        // then customise it a little...
        chart.addSubtitle(new TextTitle(
                "Source: http://www.amnestyusa.org/abolish/listbyyear.do",
                new Font("Dialog", Font.ITALIC, 10)));

        XYPlot plot = (XYPlot) chart.getPlot();
        XYBarRenderer renderer = (XYBarRenderer) plot.getRenderer();
        StandardXYToolTipGenerator generator = new StandardXYToolTipGenerator(
            "{1} = {2}", new SimpleDateFormat("yyyy"), new DecimalFormat("0"));
        renderer.setDefaultToolTipGenerator(generator);
        renderer.setMargin(0.10);

        DateAxis axis = (DateAxis) plot.getDomainAxis();
        axis.setTickMarkPosition(DateTickMarkPosition.MIDDLE);
        axis.setLowerMargin(0.01);
        axis.setUpperMargin(0.01);

        ChartUtils.applyCurrentTheme(chart);

        return chart;
    }

    /**
     * Creates a sample dataset.
     *
     * @return A dataset.
     */
    private static IntervalXYDataset createDataset() {

        TimeSeries t1 = new TimeSeries("Executions", "Year", "Count");
        try {
            t1.add(new Year(1976), 0);
            t1.add(new Year(1977), 1);
            t1.add(new Year(1978), 0);
            t1.add(new Year(1979), 2);
            t1.add(new Year(1980), 0);
            t1.add(new Year(1981), 1);
            t1.add(new Year(1982), 2);
            t1.add(new Year(1983), 5);
            t1.add(new Year(1984), 21);
            t1.add(new Year(1985), 18);
            t1.add(new Year(1986), 18);
            t1.add(new Year(1987), 25);
            t1.add(new Year(1988), 11);
            t1.add(new Year(1989), 16);
            t1.add(new Year(1990), 23);
            t1.add(new Year(1991), 14);
            t1.add(new Year(1992), 31);
            t1.add(new Year(1993), 38);
            t1.add(new Year(1994), 31);
            t1.add(new Year(1995), 56);
            t1.add(new Year(1996), 45);
            t1.add(new Year(1997), 74);
            t1.add(new Year(1998), 68);
            t1.add(new Year(1999), 98);
            t1.add(new Year(2000), 85);
            t1.add(new Year(2001), 66);
            t1.add(new Year(2002), 71);
            t1.add(new Year(2003), 65);
            t1.add(new Year(2004), 59);
            t1.add(new Year(2005), 60);

        }
        catch (Exception e) {
            System.err.println(e.getMessage());
        }
        TimeSeriesCollection tsc = new TimeSeriesCollection(t1);
        return tsc;

    }

    /**
     * Creates a panel for the demo.
     *
     * @return A panel.
     */
    public static JPanel createDemoPanel() {
        return new ChartPanel(createChart(createDataset()), false);
    }

    /**
     * Starting point for the demonstration application.
     *
     * @param args  ignored.
     */
    public static void main(String[] args) {
        XYBarChartDemo1 demo = new XYBarChartDemo1("State Executions - USA");
        demo.pack();
        UIUtils.centerFrameOnScreen(demo);
        demo.setVisible(true);
    }

}
