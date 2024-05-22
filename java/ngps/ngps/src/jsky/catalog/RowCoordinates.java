/*
 * ESO Archive
 *
 * $Id: RowCoordinates.java,v 1.4 2009/03/02 11:56:03 abrighto Exp $
 *
 * who             when        what
 * --------------  ----------  ----------------------------------------
 * Allan Brighton  1999/05/10  Created
 */

package jsky.catalog;


import java.util.*;

import jsky.coords.Coordinates;
import jsky.coords.ImageCoords;
import jsky.coords.WorldCoords;

import java.lang.NumberFormatException;

/**
 * Stores information about which columns in a table contain
 * the coordinates for that row, and whether world coordinates or
 * image coordinates are supported. In addition, the index of the
 * id column may be stored here, if known.
 */
public class RowCoordinates {

    // The column containing the object id, or -1 if there isn't one
    protected int _idCol = -1;

    // The column index for the X or RA coordinate
    protected int _xCol = -1;

    // The column index for the Y or DEC coordinate
    protected int _yCol = -1;

    // True if the table has RA and DEC coordinate columns
    protected boolean _isWcs = false;

    // Value of the equinox for world coordinates, if it is known
    protected double _equinox = 2000.;

    // Index of a column containing the equnox for each row, or -1 if it is constant.
    protected int _equinoxCol = -1;

    // True if using image coordinates
    protected boolean _isPix = false;

    /**
     * Create an object that will extract a WorldCoords object from a
     * row using the given ra,dec indexes and the given equinox.
     */
    public RowCoordinates(int raCol, int decCol, double equinox) {
        _xCol = raCol;
        _yCol = decCol;
        _equinox = equinox;
        _isWcs = true;
    }

    /**
     * Create an object that will extract a WorldCoords object from a
     * row using the given ra,dec and equinox indexes.
     */
    public RowCoordinates(int raCol, int decCol, int equinoxCol) {
        _xCol = raCol;
        _yCol = decCol;
        _equinoxCol = equinoxCol;
        _isWcs = true;
    }


    /**
     * Create an object that will extract an ImageCoords object from a
     * row using the given x,y indexes.
     */
    public RowCoordinates(int xCol, int yCol) {
        _xCol = xCol;
        _yCol = yCol;
        _isPix = true;
    }

    /**
     * This constructor should be used when there are no coordinate columns.
     */
    public RowCoordinates() {
    }


    /**
     * Return true if coordinate columns, such as (ra, dec), were defined
     */
    public boolean hasCoordinates() {
        return (_isWcs || _isPix);
    }


    /**
     * Return true if the catalog has RA and DEC coordinate columns
     */
    public boolean isWCS() {
        return _isWcs;
    }

    /**
     * Return the equinox used for world coordinates
     */
    public double equinox() {
        return _equinox;
    }

    /**
     * Return true if the catalog has X and Y columns (assumed to be image pixel coordinates)
     */
    public boolean isPix() {
        return _isPix;
    }


    /**
     * Return a Coordinates object for the given row vector, or null if not found
     */
    public Coordinates getCoordinates(Vector row) {
        try {
            int n = row.size();
            if (_xCol < n && _yCol < n) {
                if (_isWcs) {
                    Object ra = row.get(_xCol), dec = row.get(_yCol);
                    if (ra != null && dec != null) {
                        if (ra instanceof String && dec instanceof String
                                && ((String) ra).length() != 0
                                && ((String) dec).length() != 0) {
                            return new WorldCoords((String) ra, (String) dec, _equinox);
                        } else if (ra instanceof Double && dec instanceof Double) {
                            return new WorldCoords((Double) ra, (Double) dec, _equinox);
                        } else if (ra instanceof Float && dec instanceof Float) {
                            return new WorldCoords((Float) ra, (Float) dec, _equinox);
                        }
                    }
                } else if (_isPix) {
                    Object x = row.get(_xCol), y = row.get(_yCol);
                    if (x != null && y != null) {
                        if (x instanceof Double && y instanceof Double) {
                            return new ImageCoords((Double) x, (Double) y);
                        } else if (x instanceof Float && y instanceof Float) {
                            return new ImageCoords((Float) x, (Float) y);
                        }
                    }
                }
            }
        } catch (NumberFormatException e) {
            // return null if there was a bad value
        }
        return null;
    }

    /**
     * Return the column index for the X coordinate, or -1 if not known.
     */
    public int getXCol() {
        return _isPix ? _xCol : -1;
    }

    /**
     * Return the column index for the Y coordinate, or -1 if not known.
     */
    public int getYCol() {
        return _isPix ? _yCol : -1;
    }

    /**
     * Return the column index for the RA coordinate, or -1 if not known.
     */
    public int getRaCol() {
        return _isWcs ? _xCol : -1;
    }

    /**
     * Return the column index for the DEC coordinate, or -1 if not known.
     */
    public int getDecCol() {
        return _isWcs ? _yCol : -1;
    }

    /**
     * Return the value of the equinox for world coordinates, if it is known (default: 2000).
     */
    public double getEquinox() {
        return _equinox;
    }

    /**
     * Return the Index of a column containing the equnox for each row, or -1 if it is constant.
     */
    public int getEquinoxCol() {
        return _equinoxCol;
    }

    /**
     * Return the column containing the object id, or -1 if there isn't one.
     */
    public int getIdCol() {
        return _idCol;
    }

    /**
     * Set the column containing the object id (-1 if there isn't one).
     */
    public void setIdCol(int idCol) {
        _idCol = idCol;
    }
}

