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
 * ------------------------
 * XYDatasetTableModel.java
 * ------------------------
 * (C)opyright 2003-present, by Bryan Scott and Contributors.
 *
 * Original Author:  Bryan Scott;
 * Contributor(s):   David Gilbert;
 *
 */

package org.jfree.data.xy;

import javax.swing.table.AbstractTableModel;
import javax.swing.table.TableModel;

import org.jfree.data.general.DatasetChangeEvent;
import org.jfree.data.general.DatasetChangeListener;

/**
 * A READ-ONLY wrapper around a {@link TableXYDataset} to convert it to a
 * table model for use in a JTable.  The first column of the table shows the
 * x-values, the remaining columns show the y-values for each series (series 0
 * appears in column 1, series 1 appears in column 2, etc).
 * <P>
 * TO DO:
 * <ul>
 * <li>implement proper naming for x axis (getColumnName)</li>
 * <li>implement setValueAt to remove READ-ONLY constraint (not sure how)</li>
 * </ul>
 */
public class XYDatasetTableModel extends AbstractTableModel
        implements TableModel, DatasetChangeListener  {

    /** The dataset. */
    TableXYDataset model = null;

    /**
     * Default constructor.
     */
    public XYDatasetTableModel() {
        super();
    }

    /**
     * Creates a new table model based on the specified dataset.
     *
     * @param dataset  the dataset.
     */
    public XYDatasetTableModel(TableXYDataset dataset) {
        this();
        this.model = dataset;
        this.model.addChangeListener(this);
    }

    /**
     * Sets the model (dataset).
     *
     * @param dataset  the dataset.
     */
    public void setModel(TableXYDataset dataset) {
        this.model = dataset;
        this.model.addChangeListener(this);
        fireTableDataChanged();
    }

    /**
     * Returns the number of rows.
     *
     * @return The row count.
     */
    @Override
    public int getRowCount() {
        if (this.model == null) {
            return 0;
        }
        return this.model.getItemCount();
    }

    /**
     * Gets the number of columns in the model.
     *
     * @return The number of columns in the model.
     */
    @Override
    public int getColumnCount() {
        if (this.model == null) {
            return 0;
        }
        return this.model.getSeriesCount() + 1;
    }

    /**
     * Returns the column name.
     *
     * @param column  the column index.
     *
     * @return The column name.
     */
    @Override
    public String getColumnName(int column) {
        if (this.model == null) {
            return super.getColumnName(column);
        }
        if (column < 1) {
            return "X Value";
        }
        else {
            return this.model.getSeriesKey(column - 1).toString();
        }
    }

    /**
     * Returns a value of the specified cell.
     * Column 0 is the X axis, Columns 1 and over are the Y axis
     *
     * @param row  the row number.
     * @param column  the column number.
     *
     * @return The value of the specified cell.
     */
    @Override
    public Object getValueAt(int row, int column) {
        if (this.model == null) {
            return null;
        }
        if (column < 1) {
            return this.model.getX(0, row);
        }
        else {
            return this.model.getY(column - 1, row);
        }
    }

    /**
     * Receives notification that the underlying dataset has changed.
    *
     * @param event  the event
     *
     * @see DatasetChangeListener
     */
    @Override
    public void datasetChanged(DatasetChangeEvent event) {
        fireTableDataChanged();
    }

    /**
     * Returns a flag indicating whether or not the specified cell is editable.
     *
     * @param row  the row number.
     * @param column  the column number.
     *
     * @return {@code true} if the specified cell is editable.
     */
    @Override
    public boolean isCellEditable(int row, int column) {
        return false;
   }

    /**
     * Updates the {@link XYDataset} if allowed.
     *
     * @param value  the new value.
     * @param row  the row.
     * @param column  the column.
     */
    @Override
    public void setValueAt(Object value, int row, int column) {
        if (isCellEditable(row, column)) {
            // XYDataset only provides methods for reading a dataset...
        }
    }

}
