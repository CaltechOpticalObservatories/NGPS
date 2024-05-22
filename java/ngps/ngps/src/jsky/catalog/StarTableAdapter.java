package jsky.catalog;

import uk.ac.starlink.table.*;

import java.util.List;
import java.io.IOException;

import jsky.catalog.vo.VoTable;

/**
 * Wraps a TableQueryResult to make a StarTable
 */
public class StarTableAdapter extends AbstractStarTable {
    // Original table
    private TableQueryResult _cat;

    // Describes the table columns
    private ColumnInfo[] _colInfo;

    // Set to 1 to ignore the first (generated id) column
    private int _colOffset = 0;


    public StarTableAdapter(TableQueryResult cat) {
        _cat = cat;

        _colInfo = _getColInfo();
        if (_colInfo.length > 0 && VoTable.GENERATED_ID_DESC.equals(_colInfo[0].getDescription())) {
            _colOffset = 1; // ignore generated id col
        }
    }

    public int getColumnCount() {
        return _cat.getColumnCount() - _colOffset;
    }

    public long getRowCount() {
        return _cat.getRowCount();
    }

    public String getName() {
        return _cat.getName();
    }

    public void setName(String name) {
        _cat.setName(name);
    }

    public ColumnInfo getColumnInfo(int icol) {
        return _colInfo[icol + _colOffset];
    }

    public RowSequence getRowSequence() throws IOException {
        return new RowSequence() {
            private int _row = -1;

            public boolean next() throws IOException {
                return ++_row < _cat.getRowCount();
            }

            public Object getCell(int icol) throws IOException {
                return _cat.getValueAt(_row, icol + _colOffset);
            }

            public Object[] getRow() throws IOException {
                return StarTableAdapter.this.getRow(_row);
            }

            public void close() throws IOException {
            }
        };
    }

    public boolean isRandom() {
        return true;
    }

    public Object getCell(long irow, int icol) throws IOException {
        return _cat.getValueAt((int) irow, icol + _colOffset);
    }

    public Object[] getRow(long irow) throws IOException {
        List<Object> rowData = _cat.getRowData((int) irow);
        Object[] ar = new Object[rowData.size() - _colOffset];
        if (_colOffset == 1) {
            rowData.remove(0);
        }
        return rowData.toArray(ar);
    }

    // Returns an array of objects describing the table columns
    private ColumnInfo[] _getColInfo() {
        /* Get values for special column indices. */
        RowCoordinates rowCoords = _cat.getRowCoordinates();
        int idIndex = rowCoords.getIdCol();
        int raIndex = rowCoords.getRaCol();
        int decIndex = rowCoords.getDecCol();
        int xIndex = rowCoords.getXCol();
        int yIndex = rowCoords.getYCol();

        List colNames = _cat.getColumnIdentifiers();
        int numCols = _cat.getColumnCount();
        ColumnInfo[] colInfos = new ColumnInfo[numCols];
        for (int i = 0; i < numCols; i++) {
            ColumnInfo info = new ColumnInfo((String) colNames.get(i));
            info.setContentClass(_cat.getColumnClass(i));
            info.setDescription(_cat.getColumnDesc(i).getDescription());
            info.setUnitString(_cat.getColumnDesc(i).getUnits());
            info.setUCD(_cat.getColumnDesc(i).getUCD());
            colInfos[i] = info;
        }
        if (raIndex >= 0 && raIndex < numCols) {
            ColumnInfo info = colInfos[raIndex];
            if (info.getUCD() == null) {
                info.setUCD("pos.eq.ra;meta.main");
                if (Number.class.isAssignableFrom(info.getContentClass())) {
                    info.setUnitString("deg");
                }
            }
        }
        if (decIndex >= 0 && decIndex < numCols) {
            ColumnInfo info = colInfos[decIndex];
            if (info.getUCD() == null) {
                info.setUCD("pos.eq.dec;meta.main");
                if (Number.class.isAssignableFrom(info.getContentClass())) {
                    info.setUnitString("deg");
                }
            }
        }
        if (idIndex >= 0 && idIndex < numCols) {
            ColumnInfo info = colInfos[idIndex];
            if (info.getUCD() == null) {
                info.setUCD("meta.id");
            }
        }
        if (xIndex >= 0 && xIndex < numCols) {
            ColumnInfo info = colInfos[xIndex];
            if (info.getUCD() == null) {
                info.setUCD("pos.cartesian.x");
            }
        }
        if (yIndex >= 0 && yIndex < numCols) {
            ColumnInfo info = colInfos[yIndex];
            if (info.getUCD() == null) {
                info.setUCD("pos.cartesian.y");
            }
        }

        return colInfos;
    }
}
