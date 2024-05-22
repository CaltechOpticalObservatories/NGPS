/* ----------------------
 * XYBlockChartDemo1.java
 * ----------------------
 * (C) Copyright 2006-2020, by Object Refinery Limited.
 * 
 * http://www.object-refinery.com
 *
 */

package org.jfree.chart.demo;

import java.awt.Color;
import java.awt.Font;

import javax.swing.JPanel;

import org.jfree.chart.ChartPanel;
import org.jfree.chart.ChartUtils;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.axis.AxisLocation;
import org.jfree.chart.axis.NumberAxis;
import org.jfree.chart.block.BlockBorder;
import org.jfree.chart.plot.XYPlot;
import org.jfree.chart.renderer.GrayPaintScale;
import org.jfree.chart.renderer.PaintScale;
import org.jfree.chart.renderer.xy.XYBlockRenderer;
import org.jfree.chart.title.PaintScaleLegend;
import org.jfree.chart.ui.ApplicationFrame;
import org.jfree.chart.ui.RectangleEdge;
import org.jfree.chart.ui.RectangleInsets;
import org.jfree.chart.ui.UIUtils;
import org.jfree.data.DomainOrder;
import org.jfree.data.general.DatasetChangeListener;
import org.jfree.data.general.DatasetGroup;
import org.jfree.data.xy.XYZDataset;

/**
 * A simple demonstration application showing a chart created using
 * the {@link XYBlockRenderer}.
 */
public class XYBlockChartDemo1 extends ApplicationFrame {

    /**
     * Constructs the demo application.
     *
     * @param title  the frame title.
     */
    public XYBlockChartDemo1(String title) {
        super(title);
        JPanel chartPanel = createDemoPanel();
        chartPanel.setPreferredSize(new java.awt.Dimension(500, 270));
        setContentPane(chartPanel);
    }

    /**
     * Creates a sample chart.
     *
     * @param dataset  the dataset.
     *
     * @return A sample chart.
     */
    private static JFreeChart createChart(XYZDataset dataset) {
        NumberAxis xAxis = new NumberAxis("X");
        xAxis.setStandardTickUnits(NumberAxis.createIntegerTickUnits());
        xAxis.setLowerMargin(0.0);
        xAxis.setUpperMargin(0.0);
        xAxis.setAxisLinePaint(Color.WHITE);
        xAxis.setTickMarkPaint(Color.WHITE);

        NumberAxis yAxis = new NumberAxis("Y");
        yAxis.setStandardTickUnits(NumberAxis.createIntegerTickUnits());
        yAxis.setLowerMargin(0.0);
        yAxis.setUpperMargin(0.0);
        yAxis.setAxisLinePaint(Color.WHITE);
        yAxis.setTickMarkPaint(Color.WHITE);
        XYBlockRenderer renderer = new XYBlockRenderer();
        PaintScale scale = new GrayPaintScale(-2.0, 1.0);
        renderer.setPaintScale(scale);
        XYPlot plot = new XYPlot(dataset, xAxis, yAxis, renderer);
        plot.setBackgroundPaint(Color.LIGHT_GRAY);
        plot.setDomainGridlinesVisible(false);
        plot.setRangeGridlinePaint(Color.WHITE);
        plot.setAxisOffset(new RectangleInsets(5, 5, 5, 5));
        plot.setOutlinePaint(Color.BLUE);
        JFreeChart chart = new JFreeChart("XYBlockChartDemo1", plot);
        chart.removeLegend();
        NumberAxis scaleAxis = new NumberAxis("Scale");
        scaleAxis.setAxisLinePaint(Color.WHITE);
        scaleAxis.setTickMarkPaint(Color.WHITE);
        scaleAxis.setTickLabelFont(new Font("Dialog", Font.PLAIN, 7));
        PaintScaleLegend legend = new PaintScaleLegend(new GrayPaintScale(),
                scaleAxis);
        legend.setStripOutlineVisible(false);
        legend.setSubdivisionCount(20);
        legend.setAxisLocation(AxisLocation.BOTTOM_OR_LEFT);
        legend.setAxisOffset(5.0);
        legend.setMargin(new RectangleInsets(5, 5, 5, 5));
        legend.setFrame(new BlockBorder(Color.RED));
        legend.setPadding(new RectangleInsets(10, 10, 10, 10));
        legend.setStripWidth(10);
        legend.setPosition(RectangleEdge.LEFT);
        //legend.setBackgroundPaint(new Color(120, 120, 180));
        chart.addSubtitle(legend);
        //chart.setBackgroundPaint(new Color(180, 180, 250));
        ChartUtils.applyCurrentTheme(chart);
        return chart;
    }

    /**
     * Creates a sample dataset.
     *
     * @return A dataset.
     */
    private static XYZDataset createDataset() {
        return new XYZDataset() {
            @Override
            public int getSeriesCount() {
                return 1;
            }
            @Override
            public int getItemCount(int series) {
                return 10000;
            }
            @Override
            public Number getX(int series, int item) {
                return getXValue(series, item);
            }
            @Override
            public double getXValue(int series, int item) {
                return item / 100 - 50;
            }
            @Override
            public Number getY(int series, int item) {
                return getYValue(series, item);
            }
            @Override
            public double getYValue(int series, int item) {
                return item - (item / 100) * 100 - 50;
            }
            @Override
            public Number getZ(int series, int item) {
                return getZValue(series, item);
            }
            @Override
            public double getZValue(int series, int item) {
                double x = getXValue(series, item);
                double y = getYValue(series, item);
                return Math.sin(Math.sqrt(x * x + y * y) / 5.0);
            }
            @Override
            public void addChangeListener(DatasetChangeListener listener) {
                // ignore - this dataset never changes
            }
            @Override
            public void removeChangeListener(DatasetChangeListener listener) {
                // ignore
            }
            @Override
            public DatasetGroup getGroup() {
                return null;
            }
            @Override
            public void setGroup(DatasetGroup group) {
                // ignore
            }
            @Override
            public Comparable getSeriesKey(int series) {
                return "sin(sqrt(x + y))";
            }
            @Override
            public int indexOf(Comparable seriesKey) {
                return 0;
            }
            @Override
            public DomainOrder getDomainOrder() {
                return DomainOrder.ASCENDING;
            }
        };
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
        XYBlockChartDemo1 demo = new XYBlockChartDemo1(
                "JFreeChart: XYBlockChartDemo1");
        demo.pack();
        UIUtils.centerFrameOnScreen(demo);
        demo.setVisible(true);
    }

}
