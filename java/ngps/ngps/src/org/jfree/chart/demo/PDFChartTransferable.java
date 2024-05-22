/* -------------------------
 * PDFChartTransferable.java
 * -------------------------
 * (C) Copyright 2009-2016, by Object Refinery Limited.
 * 
 * http://www.object-refinery.com
 *
 */

package org.jfree.chart.demo;

import java.awt.datatransfer.DataFlavor;
import java.awt.datatransfer.Transferable;
import java.awt.datatransfer.UnsupportedFlavorException;
import java.awt.geom.Rectangle2D;
import java.io.ByteArrayInputStream;
import java.io.IOException;
import org.jfree.chart.JFreeChart;
import com.orsonpdf.PDFDocument;
import com.orsonpdf.PDFGraphics2D;
import com.orsonpdf.Page;

/**
 * PDF format transferable.
 */
public class PDFChartTransferable implements Transferable {

    /** The data flavor. */
    final DataFlavor pdfFlavor = new DataFlavor("application/pdf", "PDF");

    /** The chart. */
    private JFreeChart chart;

    /** The width of the chart on the clipboard. */
    private int width;

    /** The height of the chart on the clipboard. */
    private int height;

    /**
     * Creates a new chart selection.
     *
     * @param chart  the chart.
     * @param width  the chart width.
     * @param height  the chart height.
     */
    public PDFChartTransferable(JFreeChart chart, int width, int height) {
        this(chart, width, height, true);
    }

    /**
     * Creates a new chart selection.
     *
     * @param chart  the chart.
     * @param width  the chart width.
     * @param height  the chart height.
     * @param cloneData  clone the dataset(s)?
     */
    public PDFChartTransferable(JFreeChart chart, int width, int height,
            boolean cloneData) {

        // we clone the chart because presumably there can be some delay
        // between putting this instance on the system clipboard and
        // actually having the getTransferData() method called...
        try {
            this.chart = (JFreeChart) chart.clone();
        }
        catch (CloneNotSupportedException e) {
            this.chart = chart;
        }
        this.width = width;
        this.height = height;
        // FIXME: we've cloned the chart, but the dataset(s) aren't cloned
        // and we should do that
    }

    /**
     * Returns the data flavors supported.
     *
     * @return The data flavors supported.
     */
    @Override
    public DataFlavor[] getTransferDataFlavors() {
        return new DataFlavor[] {this.pdfFlavor};
    }

    /**
     * Returns {@code true} if the specified flavor is supported.
     *
     * @param flavor  the flavor.
     *
     * @return A boolean.
     */
    @Override
    public boolean isDataFlavorSupported(DataFlavor flavor) {
        return this.pdfFlavor.equals(flavor);
    }

    /**
     * Returns the content for the requested flavor, if it is supported.
     *
     * @param flavor  the requested flavor.
     *
     * @return The content.
     *
     * @throws java.awt.datatransfer.UnsupportedFlavorException
     * @throws java.io.IOException
     */
    @Override
    public Object getTransferData(DataFlavor flavor)
            throws UnsupportedFlavorException, IOException {
        if (this.pdfFlavor.equals(flavor)) {
            PDFDocument pdfDoc = new PDFDocument();
            Rectangle2D bounds = new Rectangle2D.Double(0, 0, 
                    this.width, this.height);
            Page page = pdfDoc.createPage(bounds);
            PDFGraphics2D g2 = page.getGraphics2D();
            this.chart.draw(g2, bounds);
            return new ByteArrayInputStream(pdfDoc.getPDFBytes());
        }
        else {
            throw new UnsupportedFlavorException(flavor);
        }
    }

}
