/* -------------------------
 * PlotOrientationDemo2.java
 * -------------------------
 * (C) Copyright 2004-2020, by Object Refinery Limited.
 * 
 * http://www.object-refinery.com
 *
 */

package org.jfree.chart.demo;

import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.GridLayout;
import java.awt.geom.Rectangle2D;

import javax.swing.JPanel;

import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartPanel;
import org.jfree.chart.ChartUtils;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.annotations.XYLineAnnotation;
import org.jfree.chart.annotations.XYShapeAnnotation;
import org.jfree.chart.axis.NumberAxis;
import org.jfree.chart.axis.ValueAxis;
import org.jfree.chart.plot.IntervalMarker;
import org.jfree.chart.plot.PlotOrientation;
import org.jfree.chart.plot.XYPlot;
import org.jfree.chart.renderer.xy.XYLineAndShapeRenderer;
import org.jfree.chart.ui.ApplicationFrame;
import org.jfree.chart.ui.Layer;
import org.jfree.chart.ui.UIUtils;
import org.jfree.data.xy.XYDataset;
import org.jfree.data.xy.XYSeries;
import org.jfree.data.xy.XYSeriesCollection;

/**
 * A demo showing eight plots with various inverted axis and plot orientation
 * combinations.
 */
public class PlotOrientationDemo2 extends ApplicationFrame {

    /** The number of charts. */
    private static final int CHART_COUNT = 8;

    static class MyDemoPanel extends DemoPanel {

        /** The datasets. */
        private XYDataset[] datasets = new XYDataset[CHART_COUNT];

        /** The charts. */
        private JFreeChart[] charts = new JFreeChart[CHART_COUNT];

        /** The chart panels. */
        private ChartPanel[] panels = new ChartPanel[CHART_COUNT];

        /**
         * Creates a new self-contained demo panel.
         */
        public MyDemoPanel() {
            super(new GridLayout(2, 4));
            for (int i = 0; i < CHART_COUNT; i++) {
                this.datasets[i] = createDataset(i);
                this.charts[i] = createChart(i, this.datasets[i]);
                XYPlot plot = (XYPlot) this.charts[i].getPlot();
                plot.setDomainPannable(true);
                plot.setRangePannable(true);
                XYShapeAnnotation a1 = new XYShapeAnnotation(
                        new Rectangle2D.Double(1.0, 2.0, 2.0, 3.0),
                        new BasicStroke(1.0f), Color.BLUE);
                XYLineAnnotation a2 = new XYLineAnnotation(0.0, -5.0, 10.0, -5.0);

                plot.addAnnotation(a1);
                plot.addAnnotation(a2);
                plot.addDomainMarker(new IntervalMarker(5.0, 10.0), Layer.BACKGROUND);
                plot.addRangeMarker(new IntervalMarker(-2.0, 0.0), Layer.BACKGROUND);
                addChart(this.charts[i]);
                this.panels[i] = new ChartPanel(this.charts[i], false);
            }
            XYPlot plot1 = (XYPlot) this.charts[1].getPlot();
            XYPlot plot2 = (XYPlot) this.charts[2].getPlot();
            XYPlot plot3 = (XYPlot) this.charts[3].getPlot();
            XYPlot plot4 = (XYPlot) this.charts[4].getPlot();
            XYPlot plot5 = (XYPlot) this.charts[5].getPlot();
            XYPlot plot6 = (XYPlot) this.charts[6].getPlot();
            XYPlot plot7 = (XYPlot) this.charts[7].getPlot();
            plot1.getDomainAxis().setInverted(true);
            plot2.getRangeAxis().setInverted(true);
            plot3.getDomainAxis().setInverted(true);
            plot3.getRangeAxis().setInverted(true);

            plot5.getDomainAxis().setInverted(true);
            plot6.getRangeAxis().setInverted(true);
            plot4.getDomainAxis().setInverted(true);
            plot4.getRangeAxis().setInverted(true);

            plot4.setOrientation(PlotOrientation.HORIZONTAL);
            plot5.setOrientation(PlotOrientation.HORIZONTAL);
            plot6.setOrientation(PlotOrientation.HORIZONTAL);
            plot7.setOrientation(PlotOrientation.HORIZONTAL);

            add(this.panels[0]);
            add(this.panels[1]);
            add(this.panels[4]);
            add(this.panels[5]);
            add(this.panels[2]);
            add(this.panels[3]);
            add(this.panels[6]);
            add(this.panels[7]);

            setPreferredSize(new Dimension(800, 600));
        }
    }
    /**
     * Creates a new demo instance.
     *
     * @param title  the frame title.
     */
    public PlotOrientationDemo2(String title) {

        super(title);
        setContentPane(new MyDemoPanel());

    }

    /**
     * Creates a sample dataset.
     *
     * @param index  the dataset index.
     *
     * @return A dataset.
     */
    private static XYDataset createDataset(int index) {
        XYSeries series1 = new XYSeries("Series " + (index + 1));
        series1.add(-10.0, -5.0);
        series1.add(10.0, 5.0);
        XYSeriesCollection dataset = new XYSeriesCollection();
        dataset.addSeries(series1);
        return dataset;
    }

    /**
     * Creates a sample chart.
     *
     * @param index  the chart index.
     * @param dataset  the dataset.
     *
     * @return A chart.
     */
    private static JFreeChart createChart(int index, XYDataset dataset) {

        // create the chart...
        JFreeChart chart = ChartFactory.createXYLineChart(
            "Chart " + (index + 1),   // chart title
            "X",                      // x axis label
            "Y",                      // y axis label
            dataset,                  // data
            PlotOrientation.VERTICAL,
            false,                    // include legend
            false,                    // tooltips
            false                     // urls
        );

        // get a reference to the plot for further customisation...
        XYPlot plot = (XYPlot) chart.getPlot();

        XYLineAndShapeRenderer renderer
                = (XYLineAndShapeRenderer) plot.getRenderer();
        renderer.setDefaultShapesVisible(true);
        renderer.setDefaultShapesFilled(true);
        // change the auto tick unit selection to integer units only...
        ValueAxis domainAxis = plot.getDomainAxis();
        domainAxis.setStandardTickUnits(NumberAxis.createIntegerTickUnits());
        ValueAxis rangeAxis = plot.getRangeAxis();
        rangeAxis.setStandardTickUnits(NumberAxis.createIntegerTickUnits());

        ChartUtils.applyCurrentTheme(chart);

        return chart;

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
     * The starting point for the demo.
     *
     * @param args  ignored.
     */
    public static void main(String[] args) {
        PlotOrientationDemo2 demo = new PlotOrientationDemo2(
                "JFreeChart: PlotOrientationDemo2.java");
        demo.pack();
        UIUtils.centerFrameOnScreen(demo);
        demo.setVisible(true);
    }

}
