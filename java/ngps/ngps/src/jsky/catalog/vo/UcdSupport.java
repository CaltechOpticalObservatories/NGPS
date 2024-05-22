package jsky.catalog.vo;

/**
 * Utilities for working with UCD1 and UCD1+ values
 * 
 * @author Allan Brighton
 * @since Feb 25, 2009
 */
public class UcdSupport {

    private String _ucd;
    private String _normalized;

    /**
     * Initialize with the UCD string.
     * 
     * @param ucd the UCD (UCD1 or UCD1+)
     */
    public UcdSupport(String ucd) {
        _ucd = ucd != null ? ucd.toLowerCase() : "";
        _normalized = _normalize();
    }

    /**
     * @return true if the UCD represents an RA column
     */
    public boolean isRa() {
        return _normalized.equals("pos.eq.ra") || isRaMain();
    }

    /**
     * @return true if the UCD represents a Dec column
     */
    public boolean isDec() {
        return _normalized.equals("pos.eq.dec") || isDecMain();
    }

    /**
     * @return true if the UCD represents the main RA column 
     */
    public boolean isRaMain() {
        return _ucd.equals("pos.eq.ra;meta.main") || _normalized.equals("pos.eq.ra.main");
    }

    /**
     * @return true if the UCD represents the main Dec column
     */
    public boolean isDecMain() {
        return _ucd.equals("pos.eq.dec;meta.main") || _normalized.equals("pos.eq.dec.main");
    }

    /**
     * @return true if the UCD is for an ID column
     */
    public boolean isId() {
        return _normalized.equals("meta.id");
    }

    /**
     * @return true if the UCD represents a hyperlink URL to some data that we can display
     */
    public boolean isLink() {
        return _normalized.equals("image.accessreference")
                || _normalized.equals("data.link")
                || _normalized.equals("meta.ref.url");
    }


    /**
     * @return true if the UCD represents a format column
     */
    public boolean isFormat() {
        return _normalized.equals("image.format")
                || _normalized.equals("spectrum.format");

    }

    // Returns the UCD with anything trailing the first ';'
    // removed. Any namespace (ivoa:) is removed. All underscores are converted
    // to '.'.
    private String _normalize() {
        String s = _ucd.replace("_", ".");
        int i = s.indexOf(':');
        if (i != -1) {
            s = s.substring(i+1);
        }
        i = s.indexOf(';');
        if (i != -1) {
            s = s.substring(0, i);
        }
        return s;
    }
}
