package jsky.catalog.vo;

import java.net.MalformedURLException;
import java.net.URL;

import uk.ac.starlink.table.ColumnInfo;
import jsky.catalog.FieldDescAdapter;
import jsky.catalog.TableQueryResult;
import jsky.catalog.URLQueryResult;
import jsky.util.Resources;

import javax.swing.Icon;

/**
 * Describes a table column returned from a query to a VoCatalog. This class extends the parent class to recognize
 * column types, based on the UCD of the table column.
 */
public class VoFieldDesc extends FieldDescAdapter {

    private static final long serialVersionUID = 1L;

    /**
     * Initialize from the StarTable's column info.
     *
     * @param colInfo provided by the StarTable returned from a query
     */
    public VoFieldDesc(ColumnInfo colInfo) {
        super(colInfo.getName());
        setFieldClass(colInfo.getContentClass());
        setDescription(colInfo.getDescription());
        setUnits(colInfo.getUnitString());
        String ucd = colInfo.getUCD();
        setUCD(ucd);
        UcdSupport ucdSup = new UcdSupport(ucd);
        setIsRAMain(ucdSup.isRaMain() || "ra".equalsIgnoreCase(getName()));
        setIsDecMain(ucdSup.isDecMain() || "dec".equalsIgnoreCase(getName()));
        setIsRA(isRaMain() || ucdSup.isRa());
        setIsDec(isDecMain() || ucdSup.isDec());
    }

    public String getLinkText(TableQueryResult tableQueryResult, Object value,
                              int row, int column) {
        String format = getLinkFormat(tableQueryResult, row);
        if (format != null) {
            if (format.startsWith("image/")) {
                return "Display Image";
            }
            if ("text/html".equals(format)) {
                return "Show in Browser";
            }
            if (format.startsWith("spectrum/")) {
                return "Display Spectrum";
            }
        }
        return "Display";
    }

    /**
     * Return a string describing the format of the link in the given row, if known.
     *
     * @param tableQueryResult the table
     * @param row the row index in the table
     * @return the format string, if known, otherwise null
     */
    public String getLinkFormat(TableQueryResult tableQueryResult, int row) {
        int numCols = tableQueryResult.getColumnCount();
        for (int col = 0; col < numCols; col++) {
            VoFieldDesc fd = (VoFieldDesc) tableQueryResult.getColumnDesc(col);
            if ("format".equalsIgnoreCase(tableQueryResult.getColumnName(col))
                    || new UcdSupport(fd.getUCD()).isFormat()) {
                return (String) tableQueryResult.getValueAt(row, col);
            }
        }
        return null;
    }

    public URLQueryResult getLinkValue(TableQueryResult tableQueryResult,
                                       Object value, int row) throws MalformedURLException {
        URLQueryResult result;
        try {
            result = new URLQueryResult(new URL(value.toString()));
            result.setFormat(getLinkFormat(tableQueryResult, row));
            return result;
        } catch (Exception e) {
            return null;
        }
    }

    public boolean hasLink() {
        return new UcdSupport(getUCD()).isLink();
    }
}
