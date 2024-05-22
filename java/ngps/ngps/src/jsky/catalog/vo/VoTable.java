package jsky.catalog.vo;

import java.io.IOException;
import java.io.File;
import java.util.Vector;
import java.util.Arrays;
import java.util.List;
import java.util.ArrayList;
import java.net.URL;

import uk.ac.starlink.table.ColumnInfo;
import uk.ac.starlink.table.StarTable;
import uk.ac.starlink.table.RowSequence;
import uk.ac.starlink.table.StarTableFactory;
import uk.ac.starlink.util.DataSource;

import jsky.catalog.FieldDesc;
import jsky.catalog.MemoryCatalog;
import jsky.catalog.RowCoordinates;
import jsky.catalog.Catalog;

/**
 * Represents the tabular results of a query to a {@link VoCatalog}.
 */
public class VoTable extends MemoryCatalog {

    private static final long serialVersionUID = 1L;

    // Default max number of rows to display
    private static final int MAX_ROWS = 10000;

    /**
     * Generated ID column description
     */
    public static final String GENERATED_ID_DESC = "JSky Id column (generated)";

    // Saved instance of factory used to read tables
    private static StarTableFactory _starTableFactory;

    // Saved StarTable instance
    private transient StarTable _starTable;


    // Returns the factory object to use to read in tables
    private static StarTableFactory _getStarTableFactory() {
        if (_starTableFactory == null) {
            _starTableFactory = new StarTableFactory();
        }
        return _starTableFactory;
    }


    /**
     * Creates and returns an instance of VoTable with the contents of the given file.
     *
     * @param fileName a file in a format recognized by {@link StarTableFactory}
     * @return a VoTable
     * @throws java.io.IOException on error
     */
    public static VoTable createVoTable(String fileName) throws IOException {
        return createVoTable(new File(fileName).toURI().toURL(), null, null);
    }

    /**
     * Creates and returns an instance of VoTable based on the given arguments.
     *
     * @param url points to table data in a format recognized by {@link StarTableFactory}
     * @param id the table id
     * @param name display name for the catalog
     * @return a VoTable
     * @throws IOException on error
     */
    public static VoTable createVoTable(URL url, String id, String name)
            throws IOException {
        return createVoTable(url, id, name, null);
    }

    /**
     * Creates and returns an instance of VoTable based on the given arguments.
     *
     * @param url points to table data in a format recognized by {@link StarTableFactory}
     * @param id the table id
     * @param name display name for the catalog
     * @param catalog the catalog that made the query that resulted in this table
     * @return a VoTable
     * @throws IOException on error
     */
    public static VoTable createVoTable(URL url, String id, String name, Catalog catalog)
            throws IOException {
        StarTableFactory factory = _getStarTableFactory();
        StarTable starTable = factory.makeStarTable(url, null);
        VoTable voTable = createVoTable(starTable, catalog, MAX_ROWS);
        if (name == null) {
            name = starTable.getName();
        }
        if (name == null) {
            name = id;
        }
        if (name == null) {
            name = "Unknown";
        }
        voTable.setName(name);
        voTable.setId(id);
        return voTable;
    }


    /**
     * Creates and returns an instance of VoTable based on the given arguments.
     *
     * @param datsrc points to table's data in a format recognized by {@link StarTableFactory}
     * @param id the table id
     * @param name display name for the catalog
     * @return a VoTable
     * @throws IOException on error
     */
    public static VoTable createVoTable(DataSource datsrc, String id, String name) throws IOException {
        StarTableFactory factory = _getStarTableFactory();
        StarTable starTable = factory.makeStarTable(datsrc);
        VoTable voTable = createVoTable(starTable, null, MAX_ROWS);
        if (name == null) {
            name = starTable.getName();
        }
        if (name == null) {
            name = id;
        }
        if (name == null) {
            name = "Unknown";
        }
        voTable.setName(name);
        voTable.setId(id);
        return voTable;
    }

    /**
     * Creates and returns an instance of VoTable based on the given arguments.
     *
     * @param starTable the result of a VO query
     * @param catalog the catalog used to make the query
     * @param maxRows the max number of rows to include in the table
     * @return a new VoTable containing up to 'maxRows' rows of the query result
     * @throws IOException if there is an error reading the table data
     */
    public static VoTable createVoTable(StarTable starTable, Catalog catalog, int maxRows)
            throws IOException {
        int numCols = starTable.getColumnCount();
        boolean hasIdColumn = _hasIdColumn(starTable);
        boolean hasRaDecColumn = _hasRaDecColumn(starTable);
        VoFieldDesc[] fields;
        if (!hasIdColumn && hasRaDecColumn) {
            // Add a unique id column
            fields = new VoFieldDesc[numCols + 1];
            fields[0] = new VoFieldDesc(new ColumnInfo("Id"));
            fields[0].setIsId(true);
            fields[0].setDescription(GENERATED_ID_DESC);
            for (int i = 0; i < numCols; i++) {
                ColumnInfo colInfo = starTable.getColumnInfo(i);
                fields[i + 1] = new VoFieldDesc(colInfo);
            }
        } else {
            fields = new VoFieldDesc[numCols];
            for (int i = 0; i < numCols; i++) {
                ColumnInfo colInfo = starTable.getColumnInfo(i);
                fields[i] = new VoFieldDesc(colInfo);
            }
        }

        RowSequence rseq = starTable.getRowSequence();
        Vector<Vector<Object>> dataVector = new Vector<Vector<Object>>();
        int rowCount = 0;
        try {
            while (rseq.next()) {
                if (++rowCount > maxRows) {
                    break;
                }
                Object[] rowData = rseq.getRow();
                Vector<Object> row = new Vector<Object>(fields.length);
                List<Object> list = new ArrayList<Object>(Arrays.asList(rowData));
                if (fields.length > numCols) {
                    list.add(0, rowCount);
                }
                row.addAll(list);
                dataVector.add(row);
            }
        }
        finally {
            rseq.close();
        }

        return new VoTable(starTable, catalog, fields, dataVector, hasRaDecColumn);
    }

    // Returns true if the given table has an id column
    private static boolean _hasIdColumn(StarTable starTable) {
        int n = starTable.getColumnCount();
        for (int i = 0; i < n; i++) {
            ColumnInfo colInfo = starTable.getColumnInfo(i);
            String name = colInfo.getName();
            if (new UcdSupport(colInfo.getUCD()).isId() || "id".equalsIgnoreCase(name)) {
                return true;
            }
        }
        return false;
    }

    // Returns true if the given table contains RA/Dec columns
    private static boolean _hasRaDecColumn(StarTable starTable) {
        int n = starTable.getColumnCount();
        for (int i = 0; i < n; i++) {
            String ucd = starTable.getColumnInfo(i).getUCD();
            UcdSupport ucdSup = new UcdSupport(ucd);
            if (ucdSup.isRa() || ucdSup.isDec()) {
                return true;
            }
            String name = starTable.getColumnInfo(i).getName();
            if ("ra".equalsIgnoreCase(name)
                    || "dec".equalsIgnoreCase(name)) {
                return true;
            }
        }
        return false;
    }


    /**
     * Initialize from the given StarTable, catalog and row data.
     *
     * @param starTable the internal result of the VO query
     * @param catalog the source catalog that made the query
     * @param fields describes the table columns
     * @param dataVector the actual table data
     * @param hasRaDecColumn true if the table has RA,Dec columns
     */
    public VoTable(StarTable starTable, Catalog catalog, FieldDesc[] fields, Vector dataVector,
                   boolean hasRaDecColumn) {
        super(fields, dataVector);
        _starTable = starTable;
        setName(starTable.getName());
        setId(starTable.getName());
        if (catalog != null) {
            setCatalog(catalog);
            setTitle("Query Results from: " + catalog.getTitle());
        } else {
            setCatalog(this);
            setTitle("Query Results");
        }
        if (hasRaDecColumn) {
            _initRowCoordinates(fields);
        }
    }

    // Determine the coordinates of the RA,Dec columns (for table symbol plotting)
    private void _initRowCoordinates(FieldDesc[] fields) {
        int raCol = -1, decCol = -1, i = 0;
        for (FieldDesc fd : fields) {
            if (fd.isRaMain()) {
                raCol = i;
            } else if (fd.isDecMain()) {
                decCol = i;
            }
            i++;
        }
        if (raCol != -1 && decCol != -1) {
            // XXX What if table coords are not J2000? Where is that described?
            setRowCoordinates(new RowCoordinates(raCol, decCol, 2000));
        }
    }

    public StarTable getStarTable() {
        if (_starTable != null && _starTable.getRowCount() == getRowCount()) {
            // return the original StarTable
            return _starTable;
        }
        // Rows were added or removed, so return a modified StarTable
        return super.getStarTable();
    }

    public Class getColumnClass(int columnIndex) {
        return getColumnDesc(columnIndex).getFieldClass();
    }
}
