/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package edu.caltech.palomar.instruments.ngps.charts;

import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;
import javax.swing.JFrame;
import javax.swing.JPanel; 
import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.axis.DateAxis;
import org.jfree.chart.demo.GanttDemo3;
import static org.jfree.chart.demo.GanttDemo3.createDataset;
import org.jfree.chart.labels.CategoryItemLabelGenerator;
import org.jfree.chart.labels.ItemLabelAnchor;
import org.jfree.chart.labels.ItemLabelPosition;
import org.jfree.chart.plot.CategoryPlot;
import org.jfree.chart.renderer.category.GanttRenderer;
import org.jfree.chart.ui.ApplicationFrame;
import org.jfree.chart.ui.TextAnchor;
import org.jfree.data.category.CategoryDataset;
import org.jfree.data.category.IntervalCategoryDataset;
import org.jfree.data.gantt.TaskSeries;
import org.jfree.data.gantt.TaskSeriesCollection;

/**
 *
 * @author developer
 */
/*================================================================================================
/    readOTMoutput()
/=================================================================================================*/
public class OTM_Gantt_Chart  extends JFrame {
  
    public OTM_Gantt_Chart(java.lang.String title,TaskSeriesCollection dataset){
        super(title);
        JPanel chartPanel = createDemoPanel(dataset);
        chartPanel.setPreferredSize(new java.awt.Dimension(500, 370));
        setContentPane(chartPanel);
        this.setSize(2000, 1000);
        setDefaultCloseOperation(JFrame.HIDE_ON_CLOSE);          
   }    
/*================================================================================================
/    readOTMoutput()
/================================================================================================*/   
    public static JPanel createDemoPanel(TaskSeriesCollection dataset) {
//        TaskSeriesCollection collection = new TaskSeriesCollection();
//        collection.add(dataset);
        JFreeChart chart = createChart(dataset);
        ChartPanel panel = new ChartPanel(chart);
        panel.setMouseWheelEnabled(true);
        return panel;
    }
/*================================================================================================
/    readOTMoutput()
/=================================================================================================*/   
    private static JFreeChart createChart(TaskSeriesCollection collection) {
        JFreeChart chart = ChartFactory.createGanttChart("Gantt Chart Demo", "Task", "Date", collection);
        CategoryPlot plot = (CategoryPlot) chart.getPlot();
        plot.setRangePannable(true);
        plot.getDomainAxis().setMaximumCategoryLabelWidthRatio(10.0f);

        DateAxis rangeAxis = (DateAxis) plot.getRangeAxis();
        rangeAxis.setUpperMargin(0.20);  // make room for labels

        GanttRenderer renderer = (GanttRenderer) plot.getRenderer();
        renderer.setDrawBarOutline(true);
        renderer.setDefaultItemLabelGenerator(new MyLabelGenerator(new SimpleDateFormat("kk:mm:ss")));
        renderer.setDefaultItemLabelsVisible(true);
        renderer.setDefaultPositiveItemLabelPosition(new ItemLabelPosition(ItemLabelAnchor.OUTSIDE3, TextAnchor.CENTER_LEFT));
        return chart;
    } 
/*================================================================================================
/    readOTMoutput()
/=================================================================================================*/
     static class MyLabelGenerator implements CategoryItemLabelGenerator {

        DateFormat df;

        /**
         * Creates a new instance.
         *
         * @param df  the date formatter.
         */
/*================================================================================================
/    readOTMoutput()
/=================================================================================================*/
        public MyLabelGenerator(DateFormat df) {
            this.df = df;
        }

        /**
         * Generate a custom label.
         *
         * @param dataset  the dataset.
         * @param row  the row index.
         * @param column  the column index.
         *
         * @return The label text.
         */
        @Override
/*================================================================================================
/    readOTMoutput()
/=================================================================================================*/
        public String generateLabel(CategoryDataset dataset, int row,
                int column) {
            Number n;
            if (dataset instanceof IntervalCategoryDataset) {
                IntervalCategoryDataset icd = (IntervalCategoryDataset) dataset;
                n = icd.getEndValue(row, column);
            }
            else {
                n = dataset.getValue(row, column);
            }
            if (n == null) {
                return "null";
            }
            long millis = n.longValue();
            Date d = new Date(millis);

            return this.df.format(d);
        }

        /**
         * This won't be used in the demo.
         *
         * @param dataset  the dataset.
         * @param column  the column index.
         *
         * @return The label text.
         */
        @Override
        public String generateColumnLabel(CategoryDataset dataset, int column) {
            return dataset.getColumnKey(column).toString();
        }

        /**
         * This won't be used in the demo.
         *
         * @param dataset  the dataset.
         * @param row  the row index.
         *
         * @return The label text.
         */
        @Override
        public String generateRowLabel(CategoryDataset dataset, int row) {
            return dataset.getRowKey(row).toString();
        }

    }
   
}
