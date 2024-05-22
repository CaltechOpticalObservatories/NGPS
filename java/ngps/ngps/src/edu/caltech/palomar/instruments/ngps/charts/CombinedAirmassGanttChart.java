/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package edu.caltech.palomar.instruments.ngps.charts;
import java.awt.Color;
import java.awt.geom.Ellipse2D;

import javax.swing.JPanel;
import javax.swing.JFrame;

import org.jfree.chart.ChartPanel;
import org.jfree.chart.ChartUtils;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.axis.DateAxis;
import org.jfree.chart.axis.NumberAxis;
import org.jfree.chart.axis.SymbolAxis;
import org.jfree.chart.plot.CombinedDomainXYPlot;    
import org.jfree.chart.plot.XYPlot;
import org.jfree.chart.renderer.xy.XYBarRenderer;
import org.jfree.chart.renderer.xy.XYLineAndShapeRenderer;
import org.jfree.chart.ui.ApplicationFrame;
import org.jfree.chart.ui.UIUtils;
import org.jfree.data.gantt.Task;
import org.jfree.data.gantt.TaskSeries;
import org.jfree.data.gantt.TaskSeriesCollection;
import org.jfree.data.gantt.XYTaskDataset;                                     
import org.jfree.data.time.Day;
import org.jfree.data.time.Hour;
import org.jfree.data.time.TimeSeries;
import org.jfree.data.time.TimeSeriesCollection;
import org.jfree.data.xy.IntervalXYDataset;
import org.jfree.data.xy.XYDataset;
/**
 *
 * @author developer
 */
public class CombinedAirmassGanttChart extends JFrame{
    public CombinedAirmassGanttChart(String title,XYDataset dataset_airmass,IntervalXYDataset dataset_gantt) {
        super(title);
        JPanel chartPanel = createDemoPanel(dataset_airmass,dataset_gantt);
        chartPanel.setPreferredSize(new java.awt.Dimension(500, 300));
        setContentPane(chartPanel);
    }
    /**
     * Creates a subplot.
     *
     * @param dataset  the dataset.
     *
     * @return A subplot.
     */
    private static XYPlot createSubplot1(XYDataset dataset) {
        XYLineAndShapeRenderer renderer = new XYLineAndShapeRenderer();
        renderer.setUseFillPaint(true);
        renderer.setDefaultFillPaint(Color.WHITE);
        renderer.setDefaultShape(new Ellipse2D.Double(-4.0, -4.0, 8.0, 8.0));
        renderer.setAutoPopulateSeriesShape(false);
        NumberAxis yAxis = new NumberAxis("Y");
        yAxis.setLowerMargin(0.1);
        yAxis.setUpperMargin(0.1);
        XYPlot plot = new XYPlot(dataset, new DateAxis("Time"), yAxis,renderer);
        return plot;
    }

    /**
     * Creates a subplot.
     *
     * @param dataset  the dataset.
     *
     * @return A subplot.
     */
    private static XYPlot createSubplot2(IntervalXYDataset dataset) {
        DateAxis xAxis = new DateAxis("Date/Time");
        SymbolAxis yAxis = new SymbolAxis("Resources", new String[] {"Team A", "Team B", "Team C", "Team D", "Team E"});
        yAxis.setGridBandsVisible(false);
        XYBarRenderer renderer = new XYBarRenderer();
        renderer.setUseYInterval(true);
        XYPlot plot = new XYPlot(dataset, xAxis, yAxis, renderer);
        return plot;
    }

    /**
     * Creates a demo chart.
     *
     * @return A demo chart.
     */
    private static JFreeChart createChart(XYDataset dataset_airmass,IntervalXYDataset dataset_gantt) {
        CombinedDomainXYPlot plot = new CombinedDomainXYPlot(
                new DateAxis("Date/Time"));
        plot.setDomainPannable(true);
        plot.add(createSubplot1(dataset_airmass));
        plot.add(createSubplot2(dataset_gantt));
        JFreeChart chart = new JFreeChart("XYTaskDatasetDemo2", plot);
        chart.setBackgroundPaint(Color.WHITE);
        ChartUtils.applyCurrentTheme(chart);
        return chart;
    }

    /**
     * Creates a panel for the demo.
     *
     * @return A panel.
     */
    public static JPanel createDemoPanel(XYDataset dataset_airmass,IntervalXYDataset dataset_gantt) {
        return new ChartPanel(createChart(dataset_airmass,dataset_gantt), false);
    }


    /**
     * Creates a dataset for the demo.  Normally a dataset wouldn't be hard
     * coded like this - it would be read from a file or a database or some
     * other source.
     *
     * @return A dataset.
     */



    /**
     * Starting point for the demonstration application.
     *
     * @param args  ignored.
     */
    public static void main(String[] args) {
//        CombinedGanttAirmassChart demo = new CombinedGanttAirmassChart("JFreeChart : XYTaskDatasetDemo2.java");
 //       demo.pack();
 //       UIUtils.centerFrameOnScreen(demo);
 //       demo.setVisible(true);
    }
}    

