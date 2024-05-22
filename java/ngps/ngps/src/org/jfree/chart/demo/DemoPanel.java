/* --------------
 * DemoPanel.java
 * --------------
 * (C) Copyright 2008-2020, by Object Refinery Limited.
 * 
 * http://www.object-refinery.com
 *
 */

package org.jfree.chart.demo;

import java.awt.LayoutManager;
import java.util.ArrayList;
import java.util.List;

import javax.swing.JPanel;

import org.jfree.chart.JFreeChart;

/**
 * A panel that is used in the demo applications.
 */
public class DemoPanel extends JPanel {

    List<JFreeChart> charts;

    /**
     * Creates a new demo panel with the specified layout manager.
     *
     * @param layout  the layout manager.
     */
    public DemoPanel(LayoutManager layout) {
        super(layout);
        this.charts = new ArrayList<>();
    }

    /**
     * Records a chart as belonging to this panel.  It will subsequently be
     * returned by the getCharts() method.
     *
     * @param chart  the chart.
     */
    public void addChart(JFreeChart chart) {
        this.charts.add(chart);
    }

    /**
     * Returns an array containing the charts within this panel.
     *
     * @return The charts.
     */
    public JFreeChart[] getCharts() {
        int chartCount = this.charts.size();
        JFreeChart[] charts = new JFreeChart[chartCount];
        for (int i = 0; i < chartCount; i++) {
            charts[i] = (JFreeChart) this.charts.get(i);
        }
        return charts;
    }

}
