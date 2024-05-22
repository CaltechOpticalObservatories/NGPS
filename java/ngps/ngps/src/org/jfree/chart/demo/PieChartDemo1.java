/* ------------------
 * PieChartDemo1.java
 * ------------------
 * (C) Copyright 2021, by Object Refinery Limited.
 * 
 * http://www.object-refinery.com
 *
 */

package org.jfree.chart.demo;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Font;
import java.text.DecimalFormat;

import javax.swing.JPanel;

import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.labels.PieSectionLabelGenerator;
import org.jfree.chart.labels.StandardPieSectionLabelGenerator;
import org.jfree.chart.plot.PiePlot;
import org.jfree.chart.title.TextTitle;
import org.jfree.chart.ui.ApplicationFrame;
import org.jfree.chart.ui.RectangleInsets;
import org.jfree.chart.ui.UIUtils;
import org.jfree.data.general.DefaultPieDataset;
import org.jfree.data.general.PieDataset;

/**
 * A simple demonstration application showing how to create a pie chart using
 * data from a {@link DefaultPieDataset}.
 */
public class PieChartDemo1 extends ApplicationFrame {

    private static final long serialVersionUID = 1L;

    /**
     * Default constructor.
     *
     * @param title  the frame title.
     */
    public PieChartDemo1(String title) {
        super(title);
        setContentPane(createDemoPanel());
    }

    /**
     * Creates a sample dataset.
     * 
     * Source: https://www.cognac.fr/en/press/cognac-shipments-and-harvest-2019/
     *
     * @return A sample dataset.
     */
    private static PieDataset<String> createDataset() {
        DefaultPieDataset<String> dataset = new DefaultPieDataset<>();
        dataset.setValue("NAFTA", 105.6);
        dataset.setValue("Europe", 37.8);
        dataset.setValue("Far East", 59.5);
        dataset.setValue("Other", 13.6);
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
        JFreeChart chart = ChartFactory.createPieChart("Cognac Exports 2019", dataset);
        chart.addSubtitle(new TextTitle("Millions of Bottles"));
        chart.addSubtitle(new TextTitle("https://www.cognac.fr/en/press/cognac-shipments-and-harvest-2019/", new Font("Courier New", Font.PLAIN, 10)));
        PiePlot<String> plot = (PiePlot) chart.getPlot();
        plot.setBackgroundPaint(new Color(169, 191, 191));
        plot.setSectionPaint("NAFTA", new Color(175, 115, 75));
        plot.setSectionPaint("Europe", new Color(71, 70, 76));
        plot.setSectionPaint("Far East", new Color(161, 152, 94));
        plot.setSectionPaint("Other", new Color(241, 208, 158));
        PieSectionLabelGenerator generator = new StandardPieSectionLabelGenerator("{0} = {1} ({2})", new DecimalFormat("0.0"), new DecimalFormat("0.0%"));
        plot.setLabelGenerator(generator);
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
        PieChartDemo1 demo = new PieChartDemo1("JFreeChart: Pie Chart Demo 1");
        demo.pack();
        UIUtils.centerFrameOnScreen(demo);
        demo.setVisible(true);
    }

}
