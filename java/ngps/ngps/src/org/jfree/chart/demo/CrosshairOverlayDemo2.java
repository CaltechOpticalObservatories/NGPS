/* --------------------------
 * CrosshairOverlayDemo2.java
 * --------------------------
 * (C) Copyright 2013-2020, by Object Refinery Limited.
 *
 */

package org.jfree.chart.demo;

import java.awt.BasicStroke;
import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.geom.Rectangle2D;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.SwingUtilities;
import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartMouseEvent;
import org.jfree.chart.ChartMouseListener;
import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.axis.ValueAxis;
import org.jfree.chart.panel.CrosshairOverlay;
import org.jfree.chart.plot.Crosshair;
import org.jfree.chart.plot.XYPlot;
import org.jfree.chart.ui.RectangleAnchor;
import org.jfree.chart.ui.RectangleEdge;
import org.jfree.data.general.DatasetUtils;
import org.jfree.data.xy.XYDataset;
import org.jfree.data.xy.XYSeries;
import org.jfree.data.xy.XYSeriesCollection;

/**
 * A demo showing crosshairs that follow the data points on an XYPlot.
 */
public class CrosshairOverlayDemo2 extends JFrame {
  
    static class MyDemoPanel extends JPanel implements ChartMouseListener {

        private static final int SERIES_COUNT = 4;
    
        private ChartPanel chartPanel;
    
        private Crosshair xCrosshair;
    
        private Crosshair[] yCrosshairs;

        public MyDemoPanel() {
            super(new BorderLayout());
            JFreeChart chart = createChart(createDataset());
            this.chartPanel = new ChartPanel(chart);
            this.chartPanel.addChartMouseListener(this);
            CrosshairOverlay crosshairOverlay = new CrosshairOverlay();
            this.xCrosshair = new Crosshair(Double.NaN, Color.GRAY, 
                    new BasicStroke(0.5f));
            this.xCrosshair.setLabelVisible(true);
            crosshairOverlay.addDomainCrosshair(xCrosshair);
            this.yCrosshairs = new Crosshair[SERIES_COUNT];
            for (int i = 0; i < SERIES_COUNT; i++) {
                this.yCrosshairs[i] = new Crosshair(Double.NaN, Color.GRAY, 
                        new BasicStroke(0.5f));
                this.yCrosshairs[i].setLabelVisible(true);
                if (i % 2 != 0) {
                    this.yCrosshairs[i].setLabelAnchor(
                            RectangleAnchor.TOP_RIGHT);
                }
                crosshairOverlay.addRangeCrosshair(yCrosshairs[i]);
            }
            chartPanel.addOverlay(crosshairOverlay);
            add(chartPanel);
        }
        
        /**
         * Creates a chart for the demo.
         * 
         * @param dataset  the dataset.
         * 
         * @return The chart.
         */
        private JFreeChart createChart(XYDataset dataset) {
            JFreeChart chart = ChartFactory.createXYLineChart(
                    "CrosshairOverlayDemo2", "X", "Y", dataset);
            return chart;
        }

        private XYDataset createDataset() {
            XYSeriesCollection dataset = new XYSeriesCollection();
            for (int i = 0; i < SERIES_COUNT; i++) {
                XYSeries series = new XYSeries("S" + i);
                for (int x = 0; x < 10; x++) {
                    series.add(x, x + Math.random() * 4.0);
                }
                dataset.addSeries(series);
            }
            return dataset;
        }
    
        @Override
        public void chartMouseClicked(ChartMouseEvent event) {
            // ignore
        }

        /**
         * Update the crosshairs.
         * 
         * @param event  the mouse event.
         */
        @Override
        public void chartMouseMoved(ChartMouseEvent event) {
            Rectangle2D dataArea = this.chartPanel.getScreenDataArea();
            JFreeChart chart = event.getChart();
            XYPlot plot = (XYPlot) chart.getPlot();
            ValueAxis xAxis = plot.getDomainAxis();
            double x = xAxis.java2DToValue(event.getTrigger().getX(), dataArea, 
                    RectangleEdge.BOTTOM);
            this.xCrosshair.setValue(x);
            for (int i = 0; i < SERIES_COUNT; i++) {
                double y = DatasetUtils.findYValue(plot.getDataset(), i, x);
                this.yCrosshairs[i].setValue(y);
            }
        }
    }
    
    /**
     * Creates a new instance.
     * 
     * @param title  the frame title. 
     */
    public CrosshairOverlayDemo2(String title) {
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
     * Entry point for the demo when run as a standalone application.
     * 
     * @param args  command line args (ignored). 
     */
    public static void main(String[] args) {
        SwingUtilities.invokeLater(new Runnable() {
            @Override
            public void run() {
                CrosshairOverlayDemo2 app = new CrosshairOverlayDemo2(
                        "JFreeChart: CrosshairOverlayDemo2.java");
                app.pack();
                app.setVisible(true);
            }
        });
    }

}

