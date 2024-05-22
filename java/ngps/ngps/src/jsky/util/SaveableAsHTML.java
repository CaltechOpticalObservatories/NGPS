/*
 * $Id: SaveableAsHTML.java,v 1.2 2009/04/21 13:31:17 abrighto Exp $
 */

package jsky.util;

import java.io.IOException;


/**
 * An interface for objects that can be saved to a file in HTML format.
 * This is intended to be used to implement "Save as HTML..." menu items.
 */
public abstract interface SaveableAsHTML {

    /**
     * Save the current object to the given file in HTML format.
     */
    public void saveAsHTML(String filename) throws IOException;
}
 