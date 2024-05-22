/* --------------
 * DialDemo4.java
 * --------------
 * (C) Copyright 2006-2017, by Object Refinery Limited.
 * 
 * http://www.object-refinery.com
 * 
 */

package org.jfree.chart.demo;

import java.awt.BasicStroke;
import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.GradientPaint;
import java.awt.Point;

import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JSlider;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.plot.dial.ArcDialFrame;
import org.jfree.chart.plot.dial.DialBackground;
import org.jfree.chart.plot.dial.DialPlot;
import org.jfree.chart.plot.dial.DialPointer;
import org.jfree.chart.plot.dial.StandardDialScale;
import org.jfree.chart.ui.GradientPaintTransformType;
import org.jfree.chart.ui.StandardGradientPaintTransformer;
import org.jfree.data.general.DefaultValueDataset;

/**
 * A sample application showing the use of a {@link DialPlot}.
 */
public class DialDemo4 extends JFrame {

    static class DemoPanel extends JPanel implements ChangeListener {

        /** A slider to update the dataset value. */
        JSlider slider;

        /** The dataset. */
        DefaultValueDataset dataset;

        /**
         * Creates a new demo panel.
         */
        public DemoPanel() {
            super(new BorderLayout());
            this.dataset = new DefaultValueDataset(50);
            // get data for diagrams
            DialPlot plot = new DialPlot();
            plot.setView(0.78, 0.37, 0.22, 0.26);
            plot.setDataset(this.dataset);

            ArcDialFrame dialFrame = new ArcDialFrame(-10.0, 20.0);
            dialFrame.setInnerRadius(0.70);
            dialFrame.setOuterRadius(0.90);
            dialFrame.setForegroundPaint(Color.darkGray);
            dialFrame.setStroke(new BasicStroke(3.0f));
            plot.setDialFrame(dialFrame);

            GradientPaint gp = new GradientPaint(new Point(),
                    new Color(255, 255, 255), new Point(),
                    new Color(240, 240, 240));
            DialBackground sdb = new DialBackground(gp);
            sdb.setGradientPaintTransformer(new StandardGradientPaintTransformer(
                    GradientPaintTransformType.VERTICAL));
            plot.addLayer(sdb);

            StandardDialScale scale = new StandardDialScale(0, 100, -8, 16.0,
                    10.0, 4);
            scale.setTickRadius(0.82);
            scale.setTickLabelOffset(-0.04);
            scale.setMajorTickIncrement(25.0);
            scale.setTickLabelFont(new Font("Dialog", Font.PLAIN, 14));

            plot.addScale(0, scale);

            DialPointer needle = new DialPointer.Pin();
            needle.setRadius(0.84);
            plot.addLayer(needle);
            JFreeChart chart1 = new JFreeChart(plot);
            chart1.setTitle("Dial Demo 4");
            ChartPanel cp1 = new ChartPanel(chart1, false);

            cp1.setPreferredSize(new Dimension(400, 250));
            this.slider = new JSlider(0, 100);
            this.slider.setMajorTickSpacing(10);
            this.slider.setPaintLabels(true);
            this.slider.addChangeListener(this);
            add(cp1);
            add(this.slider, BorderLayout.SOUTH);
        }

        /**
         * Handle a change in the slider by updating the dataset value.  This
         * automatically triggers a chart repaint.
         *
         * @param e  the event.
         */
        @Override
        public void stateChanged(ChangeEvent e) {
            this.dataset.setValue(this.slider.getValue());
        }

    }

    /**
     * Creates a demo panel.  This method is called by SuperDemo.java.
     *
     * @return A demo panel.
     */
    public static JPanel createDemoPanel() {
        return new DemoPanel();
    }

    /**
     * Creates a new instance.
     *
     * @param title  the frame title.
     */
    public DialDemo4(String title) {
        super(title);
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setContentPane(createDemoPanel());
    }

    /**
     * Starting point for the demo application.
     *
     * @param args  ignored.
     */
    public static void main(String[] args) {
        DialDemo4 app = new DialDemo4("JFreeChart: DialDemo4.java");
        app.pack();
        app.setVisible(true);
    }
}
