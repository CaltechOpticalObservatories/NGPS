/* -------------------
 * RingChartDemo2.java
 * -------------------
 * (C) Copyright 2014-2020, by Object Refinery Limited.
 * 
 * http://www.object-refinery.com
 */

package org.jfree.chart.demo;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.GradientPaint;
import java.awt.Point;
import java.text.DecimalFormat;
import javax.swing.JPanel;

import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.plot.CenterTextMode;
import org.jfree.chart.plot.RingPlot;
import org.jfree.chart.title.TextTitle;
import org.jfree.chart.ui.ApplicationFrame;
import org.jfree.chart.ui.HorizontalAlignment;
import org.jfree.chart.ui.RectangleInsets;
import org.jfree.chart.ui.UIUtils;
import org.jfree.data.general.DefaultPieDataset;
import org.jfree.data.general.PieDataset;

/**
 * A simple demonstration application showing how to create a ring chart using
 * data from a {@link DefaultPieDataset}.
 */
public class RingChartDemo2 extends ApplicationFrame {

    private static final long serialVersionUID = 1L;
    
    /**
     * Default constructor.
     *
     * @param title  the frame title.
     */
    public RingChartDemo2(String title) {
        super(title);
        setContentPane(createDemoPanel());
    }

    /**
     * Creates a sample dataset.
     *
     * @return A sample dataset.
     */
    private static PieDataset createDataset() {
        DefaultPieDataset dataset = new DefaultPieDataset();
        dataset.setValue("A", 0.653);
        dataset.setValue("B", 0.347);
        return dataset;
    }

    /**
     * Creates a chart.
     *
     * @param dataset  the dataset.
     *
     * @return A chart.
     */
    private static JFreeChart createChart(PieDataset dataset) {
        RingPlot plot = new RingPlot(dataset);
        plot.setCenterTextMode(CenterTextMode.VALUE);
        plot.setCenterTextFont(new Font(Font.SANS_SERIF, Font.BOLD, 24));
        plot.setCenterTextColor(Color.LIGHT_GRAY);
        plot.setCenterTextFormatter(new DecimalFormat("0.0%"));
        JFreeChart chart = new JFreeChart("Machine Capacity", 
                JFreeChart.DEFAULT_TITLE_FONT, plot, false);
        chart.setBackgroundPaint(new GradientPaint(new Point(0, 0), 
                new Color(20, 20, 20), new Point(400, 200), Color.DARK_GRAY));

        // customise the title position and font
        TextTitle t = chart.getTitle();
        t.setHorizontalAlignment(HorizontalAlignment.LEFT);
        t.setPaint(new Color(240, 240, 240));
        t.setFont(new Font("Arial", Font.BOLD, 26));

        plot.setBackgroundPaint(null);
        plot.setOutlineVisible(false);
        plot.setLabelGenerator(null);
        plot.setSectionPaint("A", Color.ORANGE);
        plot.setSectionPaint("B", new Color(100, 100, 100));
        plot.setSectionDepth(0.05);
        plot.setSectionOutlinesVisible(false);
        plot.setShadowPaint(null);

        return chart;
    }

    /**
     * Creates a panel for the demo (used by SuperDemo.java).
     *
     * @return A panel.
     */
    public static JPanel createDemoPanel() {
        JFreeChart chart = createChart(createDataset());
        chart.setPadding(new RectangleInsets(4, 8, 2, 2));
        ChartPanel panel = new ChartPanel(chart);
        panel.setMouseWheelEnabled(true);
        panel.setPreferredSize(new Dimension(600, 300));
        return panel;
    }

    /**
     * Starting point for the demonstration application.
     *
     * @param args  ignored.
     */
    public static void main(String[] args) {
        RingChartDemo2 demo = new RingChartDemo2("JFreeChart: Ring Chart Demo 2");
        demo.pack();
        UIUtils.centerFrameOnScreen(demo);
        demo.setVisible(true);
    }

}

