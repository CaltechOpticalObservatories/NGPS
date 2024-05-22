/* ---------------------
 * ScatterPlotDemo5.java
 * ---------------------
 * (C) Copyright 2013-2020, by Object Refinery Limited.
 * 
 * http://www.object-refinery.com
 *
 */

package org.jfree.chart.demo;

import java.awt.font.TextAttribute;
import java.text.AttributedString;
import javax.swing.JPanel;

import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.axis.NumberAxis;
import org.jfree.chart.plot.XYPlot;
import org.jfree.chart.renderer.xy.XYDotRenderer;
import org.jfree.chart.ui.ApplicationFrame;
import org.jfree.chart.ui.RectangleInsets;
import org.jfree.chart.ui.UIUtils;
import org.jfree.data.xy.XYDataset;
import org.jfree.data.xy.XYSeries;
import org.jfree.data.xy.XYSeriesCollection;

/**
 * A scatter plot demo using the {@link XYDotRenderer} class.
 */
public class ScatterPlotDemo5 extends ApplicationFrame {

    /**
     * A demonstration application showing a scatter plot.
     *
     * @param title  the frame title.
     */
    public ScatterPlotDemo5(String title) {
        super(title);
        JPanel chartPanel = createDemoPanel();
        chartPanel.setPreferredSize(new java.awt.Dimension(500, 270));
        setContentPane(chartPanel);
    }

    public static XYDataset createDataset() {
        XYSeriesCollection dataset = new XYSeriesCollection();
        XYSeries s1 = new XYSeries("S1");
        XYSeries s2 = new XYSeries("S2");
        for (int i = 0; i < 100; i++) {
            s1.add(Math.random() * 50, Math.random() * 100);
            s2.add(Math.random() * 50, Math.random() * 100);
        }
        dataset.addSeries(s1);
        dataset.addSeries(s2);
        return dataset;
    }
    
    /**
     * Creates a panel for the demo (used by SuperDemo.java).
     *
     * @return A panel.
     */
    public static JPanel createDemoPanel() {
        JFreeChart chart = ChartFactory.createScatterPlot("Scatter Plot Demo 5",
            "X", "Y", createDataset());

        XYPlot plot = (XYPlot) chart.getPlot();
        plot.setBackgroundPaint(null);
        plot.setAxisOffset(RectangleInsets.ZERO_INSETS);
        plot.setOutlineVisible(false);
        XYDotRenderer renderer = new XYDotRenderer();
        renderer.setDotWidth(4);
        renderer.setDotHeight(4);
        plot.setRenderer(renderer);
        plot.setDomainCrosshairVisible(true);
        plot.setRangeCrosshairVisible(true);

        NumberAxis xAxis = (NumberAxis) plot.getDomainAxis();
        AttributedString label = new AttributedString("H20");
        label.addAttribute(TextAttribute.SUPERSCRIPT, TextAttribute.SUPERSCRIPT_SUB, 1, 2);
        xAxis.setAttributedLabel(label);
        //xAxis.setLabelLocation(AxisLabelLocation.HIGH_END);
        xAxis.setPositiveArrowVisible(true);
        xAxis.setAutoRangeIncludesZero(false);
        
        NumberAxis yAxis = (NumberAxis) plot.getRangeAxis();
        //yAxis.setLabelLocation(AxisLabelLocation.HIGH_END);
        AttributedString s = new AttributedString("kg x 106");
        s.addAttribute(TextAttribute.SUPERSCRIPT, TextAttribute.SUPERSCRIPT_SUPER, 7, 8);
        yAxis.setAttributedLabel(s);
        
        yAxis.setPositiveArrowVisible(true);
        //yAxis.setLabelAngle(Math.PI / 2);
        ChartPanel panel = new ChartPanel(chart);
        panel.setMouseWheelEnabled(true);
        return panel;
    }

    /**
     * Starting point for the demonstration application.
     *
     * @param args  ignored.
     */
    public static void main(String[] args) {
        ScatterPlotDemo5 demo = new ScatterPlotDemo5(
                "JFreeChart: ScatterPlotDemo5.java");
        demo.pack();
        UIUtils.centerFrameOnScreen(demo);
        demo.setVisible(true);

    }

}

