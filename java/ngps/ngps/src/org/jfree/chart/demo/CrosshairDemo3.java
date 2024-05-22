/* -------------------
 * CrosshairDemo3.java
 * -------------------
 * (C) Copyright 2003-2020, by Object Refinery Limited.
 * 
 * http://www.object-refinery.com
 *
 */

package org.jfree.chart.demo;

import java.awt.BorderLayout;
import java.text.SimpleDateFormat;

import javax.swing.JPanel;
import javax.swing.JSlider;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.axis.DateAxis;
import org.jfree.chart.axis.ValueAxis;
import org.jfree.chart.plot.XYPlot;
import org.jfree.chart.renderer.xy.XYItemRenderer;
import org.jfree.chart.renderer.xy.XYLineAndShapeRenderer;
import org.jfree.chart.ui.ApplicationFrame;
import org.jfree.chart.ui.UIUtils;
import org.jfree.data.Range;
import org.jfree.data.time.Month;
import org.jfree.data.time.TimeSeries;
import org.jfree.data.time.TimeSeriesCollection;
import org.jfree.data.xy.XYDataset;

/**
 * A crosshair demo.
 */
public class CrosshairDemo3 extends ApplicationFrame {

    static class MyDemoPanel extends DemoPanel implements ChangeListener {

        private JFreeChart chart;

        private JSlider slider;

        /**
         * Creates a new demo panel.
         */
        public MyDemoPanel() {
            super(new BorderLayout());
            XYDataset dataset = createDataset();
            this.chart = createChart(dataset);
            addChart(this.chart);
            ChartPanel chartPanel = new ChartPanel(this.chart);
            chartPanel.setPreferredSize(new java.awt.Dimension(500, 270));
            chartPanel.setMouseZoomable(true);

            JPanel controls = new JPanel(new BorderLayout());
            this.slider = new JSlider(0, 100, 50);
            this.slider.addChangeListener(this);
            controls.add(this.slider);

            add(chartPanel);
            add(controls, BorderLayout.SOUTH);
        }

        /**
         * Creates a chart.
         *
         * @param dataset  a dataset.
         *
         * @return A chart.
         */
        private JFreeChart createChart(XYDataset dataset) {

            JFreeChart c = ChartFactory.createTimeSeriesChart(
                "Legal & General Unit Trust Prices",
                "Date", "Price Per Unit",
                dataset,
                true,
                true,
                false
            );

            XYPlot plot = (XYPlot) c.getPlot();
            plot.setDomainCrosshairVisible(true);
            plot.setDomainCrosshairLockedOnData(false);
            plot.setRangeCrosshairVisible(false);

            XYItemRenderer renderer = plot.getRenderer();
            if (renderer instanceof XYLineAndShapeRenderer) {
                XYLineAndShapeRenderer rr = (XYLineAndShapeRenderer) renderer;
                rr.setDefaultShapesVisible(true);
                rr.setDefaultShapesFilled(true);
            }

            DateAxis axis = (DateAxis) plot.getDomainAxis();
            axis.setDateFormatOverride(new SimpleDateFormat("MMM-yyyy"));

            return c;

        }

        /**
         * Creates a dataset, consisting of two series of monthly data.
         *
         * @return the dataset.
         */
        private XYDataset createDataset() {

            TimeSeries s1 = new TimeSeries("L&G European Index Trust");
            s1.add(new Month(2, 2001), 181.8);
            s1.add(new Month(3, 2001), 167.3);
            s1.add(new Month(4, 2001), 153.8);
            s1.add(new Month(5, 2001), 167.6);
            s1.add(new Month(6, 2001), 158.8);
            s1.add(new Month(7, 2001), 148.3);
            s1.add(new Month(8, 2001), 153.9);
            s1.add(new Month(9, 2001), 142.7);
            s1.add(new Month(10, 2001), 123.2);
            s1.add(new Month(11, 2001), 131.8);
            s1.add(new Month(12, 2001), 139.6);
            s1.add(new Month(1, 2002), 142.9);
            s1.add(new Month(2, 2002), 138.7);
            s1.add(new Month(3, 2002), 137.3);
            s1.add(new Month(4, 2002), 143.9);
            s1.add(new Month(5, 2002), 139.8);
            s1.add(new Month(6, 2002), 137.0);
            s1.add(new Month(7, 2002), 132.8);

            TimeSeries s2 = new TimeSeries("L&G UK Index Trust");
            s2.add(new Month(2, 2001), 129.6);
            s2.add(new Month(3, 2001), 123.2);
            s2.add(new Month(4, 2001), 117.2);
            s2.add(new Month(5, 2001), 124.1);
            s2.add(new Month(6, 2001), 122.6);
            s2.add(new Month(7, 2001), 119.2);
            s2.add(new Month(8, 2001), 116.5);
            s2.add(new Month(9, 2001), 112.7);
            s2.add(new Month(10, 2001), 101.5);
            s2.add(new Month(11, 2001), 106.1);
            s2.add(new Month(12, 2001), 110.3);
            s2.add(new Month(1, 2002), 111.7);
            s2.add(new Month(2, 2002), 111.0);
            s2.add(new Month(3, 2002), 109.6);
            s2.add(new Month(4, 2002), 113.2);
            s2.add(new Month(5, 2002), 111.6);
            s2.add(new Month(6, 2002), 108.8);
            s2.add(new Month(7, 2002), 101.6);

            TimeSeriesCollection dataset = new TimeSeriesCollection();
            dataset.addSeries(s1);
            dataset.addSeries(s2);

            return dataset;

        }

        /**
         * Handles a state change event.
         *
         * @param event  the event.
         */
        @Override
        public void stateChanged(ChangeEvent event) {
            int value = this.slider.getValue();
            XYPlot plot = (XYPlot) this.chart.getPlot();
            ValueAxis domainAxis = plot.getDomainAxis();
            Range range = domainAxis.getRange();
            double c = domainAxis.getLowerBound()
                    + (value / 100.0) * range.getLength();
            plot.setDomainCrosshairValue(c);
        }

    }

    /**
     * A demonstration application showing how to create a simple time series
     * chart.  This example uses monthly data.
     *
     * @param title  the frame title.
     */
    public CrosshairDemo3(String title) {
        super(title);
        setContentPane(createDemoPanel());
    }

    /**
     * Creates a panel for the demo (used by SuperDemo.java).
     *
     * @return A panel.
     */
    public static JPanel createDemoPanel() {
        return new MyDemoPanel();
    }

    /**
     * Starting point for the demonstration application.
     *
     * @param args  ignored.
     */
    public static void main(String[] args) {
        CrosshairDemo3 demo = new CrosshairDemo3(
                "JFreeChart: CrosshairDemo3.java");
        demo.pack();
        UIUtils.centerFrameOnScreen(demo);
        demo.setVisible(true);
    }

}
