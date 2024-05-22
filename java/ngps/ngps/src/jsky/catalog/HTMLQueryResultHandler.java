/*
 * $Id: HTMLQueryResultHandler.java,v 1.2 2009/04/21 13:31:17 abrighto Exp $
 */


package jsky.catalog;

import java.net.*;

/**
 * This interface defines a method that classes can call when an
 * HTTP server unexpectedly returns an HTML page, which normally
 * contains an error message about a broken link, etc.
 */
public interface HTMLQueryResultHandler {

    /** Display the contents of the HTML page given by the the URL */
    public void displayHTMLPage(URL url);
} 
