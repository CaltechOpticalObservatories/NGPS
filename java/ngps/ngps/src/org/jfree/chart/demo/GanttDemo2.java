/* ---------------
 * GanttDemo2.java
 * ---------------
 * (C) Copyright 2003-2020, by Object Refinery Limited.
 * 
 * http://www.object-refinery.com
 *
 */

package org.jfree.chart.demo;

import java.awt.Color;
import java.util.Calendar;
import java.util.Date;

import javax.swing.JPanel;

import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.plot.CategoryPlot;
import org.jfree.chart.renderer.category.CategoryItemRenderer;
import org.jfree.chart.ui.ApplicationFrame;
import org.jfree.chart.ui.UIUtils;
import org.jfree.data.category.IntervalCategoryDataset;
import org.jfree.data.gantt.Task;
import org.jfree.data.gantt.TaskSeries;
import org.jfree.data.gantt.TaskSeriesCollection;

/**
 * A simple demonstration application showing how to create a Gantt chart with
 * multiple bars per task.
 */
public class GanttDemo2 extends ApplicationFrame {

    /**
     * Creates a new demo.
     *
     * @param title  the frame title.
     */
    public GanttDemo2(String title) {
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
    private static JFreeChart createChart(IntervalCategoryDataset dataset) {
        JFreeChart chart = ChartFactory.createGanttChart(
            "Gantt Chart Demo",  // chart title
            "Task",              // domain axis label
            "Date",              // range axis label
            dataset,             // data
            true,                // include legend
            true,                // tooltips
            false                // urls
        );
        CategoryPlot plot = (CategoryPlot) chart.getPlot();
        plot.setRangePannable(true);
        plot.getDomainAxis().setMaximumCategoryLabelWidthRatio(10.0f);
        CategoryItemRenderer renderer = plot.getRenderer();
        renderer.setSeriesPaint(0, Color.BLUE);
        return chart;
    }

    /**
     * Creates a sample dataset for a Gantt chart, using sub-tasks.  In
     * general, you won't hard-code the dataset in this way - it's done here so
     * that the demo is self-contained.
     *
     * @return The dataset.
     */
    private static IntervalCategoryDataset createDataset() {

        TaskSeries s1 = new TaskSeries("Scheduled");

        Task t1 = new Task("Write Proposal",
            date(1, Calendar.APRIL, 2001), date(5, Calendar.APRIL, 2001));
        t1.setPercentComplete(1.00);
        s1.add(t1);

        Task t2 = new Task(
            "Obtain Approval",
            date(9, Calendar.APRIL, 2001), date(9, Calendar.APRIL, 2001));
        t2.setPercentComplete(1.00);
        s1.add(t2);

        // here is a task split into two subtasks...
        Task t3 = new Task(
            "Requirements Analysis",
            date(10, Calendar.APRIL, 2001), date(5, Calendar.MAY, 2001));
        Task st31 = new Task(
            "Requirements 1",
            date(10, Calendar.APRIL, 2001), date(25, Calendar.APRIL, 2001));
        st31.setPercentComplete(1.0);
        Task st32 = new Task(
            "Requirements 2",
            date(1, Calendar.MAY, 2001), date(5, Calendar.MAY, 2001));
        st32.setPercentComplete(1.0);
        t3.addSubtask(st31);
        t3.addSubtask(st32);
        s1.add(t3);

        // and another...
        Task t4 = new Task(
            "Design Phase",
            date(6, Calendar.MAY, 2001), date(30, Calendar.MAY, 2001));
        Task st41 = new Task(
             "Design 1",
             date(6, Calendar.MAY, 2001), date(10, Calendar.MAY, 2001)
        );
        st41.setPercentComplete(1.0);
        Task st42 = new Task(
            "Design 2",
            date(15, Calendar.MAY, 2001), date(20, Calendar.MAY, 2001)
        );
        st42.setPercentComplete(1.0);
        Task st43 = new Task(
            "Design 3",
            date(23, Calendar.MAY, 2001), date(30, Calendar.MAY, 2001)
        );
        st43.setPercentComplete(0.50);
        t4.addSubtask(st41);
        t4.addSubtask(st42);
        t4.addSubtask(st43);
        s1.add(t4);

        Task t5 = new Task(
            "Design Signoff",
            date(2, Calendar.JUNE, 2001), date(2, Calendar.JUNE, 2001)
        );
        s1.add(t5);

        Task t6 = new Task(
            "Alpha Implementation",
            date(3, Calendar.JUNE, 2001), date(31, Calendar.JULY, 2001)
        );
        t6.setPercentComplete(0.60);

        s1.add(t6);

        Task t7 = new Task(
            "Design Review",
            date(1, Calendar.AUGUST, 2001), date(8, Calendar.AUGUST, 2001)
        );
        t7.setPercentComplete(0.0);
        s1.add(t7);

        Task t8 = new Task(
            "Revised Design Signoff",
            date(10, Calendar.AUGUST, 2001), date(10, Calendar.AUGUST, 2001)
        );
        t8.setPercentComplete(0.0);
        s1.add(t8);

        Task t9 = new Task(
            "Beta Implementation",
            date(12, Calendar.AUGUST, 2001), date(12, Calendar.SEPTEMBER, 2001)
        );
        t9.setPercentComplete(0.0);
        s1.add(t9);

        Task t10 = new Task(
            "Testing",
            date(13, Calendar.SEPTEMBER, 2001), date(31, Calendar.OCTOBER, 2001)
        );
        t10.setPercentComplete(0.0);
        s1.add(t10);

        Task t11 = new Task(
            "Final Implementation",
            date(1, Calendar.NOVEMBER, 2001), date(15, Calendar.NOVEMBER, 2001)
        );
        t11.setPercentComplete(0.0);
        s1.add(t11);

        Task t12 = new Task(
            "Signoff",
            date(28, Calendar.NOVEMBER, 2001), date(30, Calendar.NOVEMBER, 2001)
        );
        t12.setPercentComplete(0.0);
        s1.add(t12);

        TaskSeriesCollection collection = new TaskSeriesCollection();
        collection.add(s1);

        return collection;
    }

    /**
     * Utility method for creating <code>Date</code> objects.
     *
     * @param day  the date.
     * @param month  the month.
     * @param year  the year.
     *
     * @return A date.
     */
    private static Date date(int day, int month, int year) {
        Calendar calendar = Calendar.getInstance();
        calendar.set(year, month, day);
        Date result = calendar.getTime();
        return result;
    }

    /**
     * Creates a panel for the demo (used by SuperDemo.java).
     *
     * @return A panel.
     */
    public static JPanel createDemoPanel() {
        JFreeChart chart = createChart(createDataset());
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
        GanttDemo2 demo = new GanttDemo2("JFreeChart: GanttDemo2.java");
        demo.pack();
        UIUtils.centerFrameOnScreen(demo);
        demo.setVisible(true);
    }

}
