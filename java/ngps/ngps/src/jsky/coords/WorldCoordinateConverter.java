/*
 * $Id: WorldCoordinateConverter.java,v 1.2 2009/04/21 13:31:17 abrighto Exp $
 */

package jsky.coords;

import java.awt.geom.Point2D;

import javax.swing.event.ChangeListener; 


/**
 * This defines the interface for converting between image and world coordinates.
 *
 * @version $Revision: 1.2 $
 * @author Allan Brighton
 */
public abstract interface WorldCoordinateConverter {

    /**
     * Return true if world coordinates conversion is available. This method
     * should be called to check before calling any of the world coordinates
     * conversion methods.
     */
    public boolean isWCS();

    /** Return the equinox used for coordinates (usually the equionx of the image) */
    public double getEquinox();

    /**
     * Convert the given image coordinates to world coordinates degrees in the equinox
     * of the current image.
     *
     * @param p The point to convert.
     * @param isDistance True if p should be interpreted as a distance instead
     *                   of a point.
     */
    public void imageToWorldCoords(Point2D.Double p, boolean isDistance);

    /**
     * Convert the given world coordinates (degrees, in the equinox of the current image)
     * to image coordinates.
     *
     * @param p The point to convert.
     * @param isDistance True if p should be interpreted as a distance instead
     *                   of a point.
     */
    public void worldToImageCoords(Point2D.Double p, boolean isDistance);

    /** Return the center RA,Dec coordinates in degrees. */
    public Point2D.Double getWCSCenter();

    /** Set the center RA,Dec coordinates in degrees. */
    //public void setWCSCenter(Point2D.Double p);

    /** return the width in deg */
    public double getWidthInDeg();

    /** return the height in deg */
    public double getHeightInDeg();

    /** Return the image center coordinates in pixels (image coordinates). */
    public Point2D.Double getImageCenter();

    /** Return the image width in pixels. */
    public double getWidth();

    /** Return the image height in pixels. */
    public double getHeight();
}

