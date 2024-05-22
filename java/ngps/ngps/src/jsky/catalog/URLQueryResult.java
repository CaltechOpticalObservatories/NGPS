/*
 * ESO Archive
 *
 * $Id: URLQueryResult.java,v 1.1.1.1 2009/02/17 22:59:29 abrighto Exp $
 *
 * who             when        what
 * --------------  ----------  ----------------------------------------
 * Allan Brighton  1999/05/10  Created
 */

package jsky.catalog;

import java.net.*;

/**
 * Represents a query result of some type pointed to by a URL.
 */
public class URLQueryResult implements QueryResult {

    // A URL pointing to the query result
    private URL _url;

    // The expected format of the result (may be specified in a format table column: for example: spectrum/fits)
    private String _format;

    /**
     * Create a URLQueryResult from the given URL.
     * @param url the URL
     */
    public URLQueryResult(URL url) {
        _url = url;
    }

    public URL getURL() {
        return _url;
    }

    public void setURL(URL url) {
        _url = url;
    }

    public String getFormat() {
        return _format;
    }

    public void setFormat(String format) {
        _format = format;
    }
}
