/* ===========================================================
 * JFreeChart : a free chart library for the Java(tm) platform
 * ===========================================================
 *
 * (C) Copyright 2000-present, by David Gilbert and Contributors.
 *
 * Project Info:  http://www.jfree.org/jfreechart/index.html
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 *
 * [Oracle and Java are registered trademarks of Oracle and/or its affiliates. 
 * Other names may be trademarks of their respective owners.]
 *
 * -------------------------
 * XYDrawableAnnotation.java
 * -------------------------
 * (C) Copyright 2003-present, by David Gilbert.
 *
 * Original Author:  David Gilbert;
 * Contributor(s):   Tracy Hiltbrand (equals/hashCode comply with EqualsVerifier);
 *
 */

package org.jfree.chart.annotations;

import java.awt.Graphics2D;
import java.awt.geom.AffineTransform;
import java.awt.geom.Rectangle2D;
import java.io.Serializable;
import java.util.Objects;

import org.jfree.chart.axis.ValueAxis;
import org.jfree.chart.plot.Plot;
import org.jfree.chart.plot.PlotOrientation;
import org.jfree.chart.plot.PlotRenderingInfo;
import org.jfree.chart.plot.XYPlot;
import org.jfree.chart.ui.Drawable;
import org.jfree.chart.ui.RectangleEdge;
import org.jfree.chart.util.Args;
import org.jfree.chart.util.PublicCloneable;

/**
 * A general annotation that can be placed on an {@link XYPlot}.
 */
public class XYDrawableAnnotation extends AbstractXYAnnotation
        implements Cloneable, PublicCloneable, Serializable {

    /** For serialization. */
    private static final long serialVersionUID = -6540812859722691020L;

    /** The scaling factor. */
    private double drawScaleFactor;

    /** The x-coordinate. */
    private double x;

    /** The y-coordinate. */
    private double y;

    /** The width. */
    private double displayWidth;

    /** The height. */
    private double displayHeight;

    /** The drawable object. */
    private Drawable drawable;

    /**
     * Creates a new annotation to be displayed within the given area.
     *
     * @param x  the x-coordinate for the area (must be finite).
     * @param y  the y-coordinate for the area (must be finite).
     * @param width  the width of the area (must be finite).
     * @param height  the height of the area (must be finite).
     * @param drawable  the drawable object ({@code null} not permitted).
     */
    public XYDrawableAnnotation(double x, double y, double width, double height,
                                Drawable drawable) {
        this(x, y, width, height, 1.0, drawable);
    }

    /**
     * Creates a new annotation to be displayed within the given area.  If you
     * specify a {@code drawScaleFactor} of 2.0, the {@code drawable}
     * will be drawn at twice the requested display size then scaled down to
     * fit the space.
     *
     * @param x  the x-coordinate for the area (must be finite).
     * @param y  the y-coordinate for the area (must be finite).
     * @param displayWidth  the width of the area (must be finite).
     * @param displayHeight  the height of the area (must be finite).
     * @param drawScaleFactor  the scaling factor for drawing (must be finite).
     * @param drawable  the drawable object ({@code null} not permitted).
     */
    public XYDrawableAnnotation(double x, double y, double displayWidth,
            double displayHeight, double drawScaleFactor, Drawable drawable) {
        super();
        Args.nullNotPermitted(drawable, "drawable");
        Args.requireFinite(x, "x");
        Args.requireFinite(y, "y");
        Args.requireFinite(displayWidth, "displayWidth");
        Args.requireFinite(displayHeight, "displayHeight");
        Args.requireFinite(drawScaleFactor, "drawScaleFactor");
        this.x = x;
        this.y = y;
        this.displayWidth = displayWidth;
        this.displayHeight = displayHeight;
        this.drawScaleFactor = drawScaleFactor;
        this.drawable = drawable;
    }

    /**
     * Returns the x-coordinate (set in the constructor).
     * 
     * @return The x-coordinate.
     */
    public double getX() {
        return x;
    }

    /**
     * Returns the y-coordinate (set in the constructor).
     * 
     * @return The y-coordinate.
     */
    public double getY() {
        return y;
    }

    /**
     * Returns the display width (set in the constructor).
     * 
     * @return The display width.
     */
    public double getDisplayWidth() {
        return displayWidth;
    }

    /**
     * Returns the display height (set in the constructor).
     * 
     * @return The display height.
     */
    public double getDisplayHeight() {
        return displayHeight;
    }

    /**
     * Returns the scale factor (set in the constructor).
     * 
     * @return The scale factor.
     */
    public double getDrawScaleFactor() {
        return drawScaleFactor;
    }

    /**
     * Draws the annotation.
     *
     * @param g2  the graphics device.
     * @param plot  the plot.
     * @param dataArea  the data area.
     * @param domainAxis  the domain axis.
     * @param rangeAxis  the range axis.
     * @param rendererIndex  the renderer index.
     * @param info  if supplied, this info object will be populated with
     *              entity information.
     */
    @Override
    public void draw(Graphics2D g2, XYPlot plot, Rectangle2D dataArea,
                     ValueAxis domainAxis, ValueAxis rangeAxis,
                     int rendererIndex,
                     PlotRenderingInfo info) {

        PlotOrientation orientation = plot.getOrientation();
        RectangleEdge domainEdge = Plot.resolveDomainAxisLocation(
                plot.getDomainAxisLocation(), orientation);
        RectangleEdge rangeEdge = Plot.resolveRangeAxisLocation(
                plot.getRangeAxisLocation(), orientation);
        float j2DX = (float) domainAxis.valueToJava2D(this.x, dataArea,
                domainEdge);
        float j2DY = (float) rangeAxis.valueToJava2D(this.y, dataArea,
                rangeEdge);
        Rectangle2D displayArea = new Rectangle2D.Double(
                j2DX - this.displayWidth / 2.0,
                j2DY - this.displayHeight / 2.0, this.displayWidth,
                this.displayHeight);

        // here we change the AffineTransform so we can draw the annotation
        // to a larger area and scale it down into the display area
        // afterwards, the original transform is restored
        AffineTransform savedTransform = g2.getTransform();
        Rectangle2D drawArea = new Rectangle2D.Double(0.0, 0.0,
                this.displayWidth * this.drawScaleFactor,
                this.displayHeight * this.drawScaleFactor);

        g2.scale(1 / this.drawScaleFactor, 1 / this.drawScaleFactor);
        g2.translate((j2DX - this.displayWidth / 2.0) * this.drawScaleFactor,
                (j2DY - this.displayHeight / 2.0) * this.drawScaleFactor);
        this.drawable.draw(g2, drawArea);
        g2.setTransform(savedTransform);
        String toolTip = getToolTipText();
        String url = getURL();
        if (toolTip != null || url != null) {
            addEntity(info, displayArea, rendererIndex, toolTip, url);
        }

    }

    /**
     * Tests this annotation for equality with an arbitrary object.
     *
     * @param obj  the object to test against.
     *
     * @return {@code true} or {@code false}.
     */
    @Override
    public boolean equals(Object obj) {
        if (obj == this) { // simple case
            return true;
        }
        if (!(obj instanceof XYDrawableAnnotation)) {
            return false;
        }
        XYDrawableAnnotation that = (XYDrawableAnnotation) obj;
        if (Double.doubleToLongBits(this.x) != Double.doubleToLongBits(that.x)) {
            return false;
        }
        if (Double.doubleToLongBits(this.y) != Double.doubleToLongBits(that.y)) {
            return false;
        }
        if (Double.doubleToLongBits(this.displayWidth) != 
            Double.doubleToLongBits(that.displayWidth)) {
            return false;
        }
        if (Double.doubleToLongBits(this.displayHeight) != 
            Double.doubleToLongBits(that.displayHeight)) {
            return false;
        }
        if (Double.doubleToLongBits(this.drawScaleFactor) != 
            Double.doubleToLongBits(that.drawScaleFactor)) {
            return false;
        }
        if (!Objects.equals(this.drawable, that.drawable)) {
            return false;
        }

        // fix the "equals not symmetric" problem
        if (!that.canEqual(this)) {
            return false;
        }

        return super.equals(obj);
    }

    /**
     * Ensures symmetry between super/subclass implementations of equals. For
     * more detail, see http://jqno.nl/equalsverifier/manual/inheritance.
     *
     * @param other Object
     * 
     * @return true ONLY if the parameter is THIS class type
     */
    @Override
    public boolean canEqual(Object other) {
        // fix the "equals not symmetric" problem
        return (other instanceof XYDrawableAnnotation);
    }
    
    /**
     * Returns a hash code.
     *
     * @return A hash code.
     */
    @Override
    public int hashCode() {
        int hash = super.hashCode(); // equals calls superclass, hashCode must also
        hash = 41 * hash + (int) (Double.doubleToLongBits(this.x) ^ 
                                 (Double.doubleToLongBits(this.x) >>> 32));
        hash = 41 * hash + (int) (Double.doubleToLongBits(this.y) ^ 
                                 (Double.doubleToLongBits(this.y) >>> 32));
        hash = 41 * hash + (int) (Double.doubleToLongBits(this.drawScaleFactor) ^ 
                                 (Double.doubleToLongBits(this.drawScaleFactor) >>> 32));
        hash = 41 * hash + (int) (Double.doubleToLongBits(this.displayWidth) ^ 
                                 (Double.doubleToLongBits(this.displayWidth) >>> 32));
        hash = 41 * hash + (int) (Double.doubleToLongBits(this.displayHeight) ^ 
                                 (Double.doubleToLongBits(this.displayHeight) >>> 32));
        hash = 41 * hash + Objects.hashCode(this.drawable);
        return hash;
    }

    /**
     * Returns a clone of the annotation.
     *
     * @return A clone.
     *
     * @throws CloneNotSupportedException  if the annotation can't be cloned.
     */
    @Override
    public Object clone() throws CloneNotSupportedException {
        return super.clone();
    }

}
