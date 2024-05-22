/* ------------------
 * ImageMapDemo2.java
 * ------------------
 * (C) Copyright 2002-2011, by Object Refinery Limited and Contributors.
 *
 * Original Author:  David Gilbert (for Object Refinery Limited);
 * Contributor(s):   Richard Atkinson;
 * 
 */

package org.jfree.chart.demo.imagemap;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.io.PrintWriter;

import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartRenderingInfo;
import org.jfree.chart.ChartUtils;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.axis.CategoryAxis;
import org.jfree.chart.axis.NumberAxis;
import org.jfree.chart.axis.ValueAxis;
import org.jfree.chart.entity.StandardEntityCollection;
import org.jfree.chart.imagemap.ImageMapUtils;
import org.jfree.chart.labels.StandardCategoryToolTipGenerator;
import org.jfree.chart.plot.CategoryPlot;
import org.jfree.chart.plot.PlotOrientation;
import org.jfree.chart.renderer.category.BarRenderer;
import org.jfree.chart.urls.StandardCategoryURLGenerator;
import org.jfree.data.category.CategoryDataset;
import org.jfree.data.general.DatasetUtils;

/**
 * A demo showing how to create an HTML image map for a 3D bar chart.
 */
public class ImageMapDemo2 {

    /**
     * Default constructor.
     */
    public ImageMapDemo2() {
        super();
    }

    /**
     * Starting point for the demo.
     *
     * @param args  ignored.
     */
    public static void main(String[] args) {

        // create a chart
        double[][] data = new double[][] {
            {56.0, -12.0, 34.0, 76.0, 56.0, 100.0, 67.0, 45.0},
            {37.0, 45.0, 67.0, 25.0, 34.0, 34.0, 100.0, 53.0},
            {43.0, 54.0, 34.0, 34.0, 87.0, 64.0, 73.0, 12.0}
        };
        CategoryDataset dataset = DatasetUtils.createCategoryDataset(
                "Series ", "Type ", data);

        JFreeChart chart = null;
        boolean drilldown = true;

        if (drilldown) {
            CategoryAxis categoryAxis = new CategoryAxis("Category");
            ValueAxis valueAxis = new NumberAxis("Value");
            BarRenderer renderer = new BarRenderer();
            renderer.setDefaultToolTipGenerator(
                    new StandardCategoryToolTipGenerator());
            renderer.setDefaultItemURLGenerator(new StandardCategoryURLGenerator(
                    "bar_chart_detail.jsp"));
            CategoryPlot plot = new CategoryPlot(dataset, categoryAxis, 
                    valueAxis, renderer);
            plot.setOrientation(PlotOrientation.VERTICAL);
            chart = new JFreeChart("Bar Chart", JFreeChart.DEFAULT_TITLE_FONT, 
                    plot, true);
        }
        else {
            chart = ChartFactory.createBarChart(
                "Bar Chart",           // chart title
                "Category",            // domain axis label
                "Value",               // range axis label
                dataset,               // data
                PlotOrientation.VERTICAL,
                true,                  // include legend
                true,
                false
            );
        }
        chart.setBackgroundPaint(java.awt.Color.WHITE);

        // save it to an image
        try {
            ChartRenderingInfo info = new ChartRenderingInfo(
                    new StandardEntityCollection());
            File file1 = new File("barchart101.png");
            ChartUtils.saveChartAsPNG(file1, chart, 600, 400, info);

            // write an HTML page incorporating the image with an image map
            File file2 = new File("barchart101.html");
            OutputStream out = new BufferedOutputStream(new FileOutputStream(
                    file2));
            PrintWriter writer = new PrintWriter(out);
            writer.println("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"");
            writer.println("\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">");
            writer.println("<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"en\" xml:lang=\"en\">");
            writer.println("<head><title>JFreeChart Image Map Demo</title></head>");
            writer.println("<body><p>");
            ImageMapUtils.writeImageMap(writer, "chart", info);
            writer.println("<img src=\"barchart101.png\" "
                           + "width=\"600\" height=\"400\" usemap=\"#chart\" alt=\"barchart101.png\"/>");
            writer.println("</p></body>");
            writer.println("</html>");
            writer.close();

        }
        catch (IOException e) {
            System.out.println(e.toString());
        }

    }

}
