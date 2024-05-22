/*
 * ESO Archive
 *
 * $Id: TableQueryResult.java,v 1.2 2009/04/07 08:03:23 abrighto Exp $
 *
 * who             when        what
 * --------------  ----------  ----------------------------------------
 * Allan Brighton  1999/05/10  Created
 */

package jsky.catalog;

import java.util.Vector; 
import java.util.List;

import javax.swing.table.TableModel;

import jsky.coords.Coordinates;
import jsky.coords.WorldCoordinates;
import uk.ac.starlink.table.StarTable;


/**
 * This interface defines the methods required to access tabular query results.
 * It extends QueryResult, since it represents the result of a catalog query.
 * It extends TableModel to make it easy to display in a JTable.
 * It also extends Catalog, so that it is posible to search again in the result
 * of a previous query.
 */
public abstract interface TableQueryResult extends Catalog, TableModel {

    /**
     * Returns the Vector of Vectors that contains the table's data values.
     * The vectors contained in the outer vector are each a single row of values.
     */
    public Vector getDataVector();

    /**
     * Return a description of the ith table column field
     */
    public FieldDesc getColumnDesc(int i);

    /**
     * Return the table column index for the given column name
     */
    public int getColumnIndex(String name);

    /**
     * Return a vector of column headings for this table.
     */
    public Vector getColumnIdentifiers();

    /**
     * Return true if the table has coordinate columns, such as (ra, dec)
     */
    public boolean hasCoordinates();

    /**
     * Return a Coordinates object based on the appropriate columns in the given row,
     * or null if there are no coordinates available for the row.
     */
    public Coordinates getCoordinates(int rowIndex);

    /**
     * Return an object describing the columns that can be used to search
     * this catalog.
     */
    public RowCoordinates getRowCoordinates();

    /**
     * Return the center coordinates for this table from the query arguments,
     * if known, otherwise return the coordinates of the first row, or null
     * if there are no world coordinates available.
     */
    public WorldCoordinates getWCSCenter();

    /**
     * Return the object representing the arguments to the query that resulted in this table,
     * if known, otherwise null.
     */
    public QueryArgs getQueryArgs();

    /**
     * Set the object representing the arguments to the query that resulted in this table.
     */
    public void setQueryArgs(QueryArgs queryArgs);

    /**
     * Set the id or short name for the table
     */
    public void setId(String id);
    
    /**
     * Return true if the result was truncated and more data would have been available
     */
    public boolean isMore();

    /**
     * Return the catalog used to create this table,
     * or a dummy, generated catalog object, if not known.
     */
    public Catalog getCatalog();

    /**
     * Sets the catalog used to create this table
     * @param cat catalog used to create this table
     */
    public void setCatalog(Catalog cat);


    /**
     * Returns a list containing the values for the given row
     *
     * @param row the row index
     * @return the list of values
     */
    public List<Object> getRowData(int row);


    /**
     * @return a StarTable with the contents of the given table
     */
    public StarTable getStarTable();

}
