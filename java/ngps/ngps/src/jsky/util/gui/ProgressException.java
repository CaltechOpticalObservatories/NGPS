/*
 * $Id: ProgressException.java,v 1.2 2009/04/21 13:31:17 abrighto Exp $
 */

package jsky.util.gui;

import java.io.IOException;

/**
 * An exception that is thrown when (or at some point after) the user
 * presses the Stop button in a ProgressPanel.
 *
 * @version $Revision: 1.2 $
 * @author Allan Brighton
 */
public class ProgressException extends IOException {

    public ProgressException(String msg) {
        super(msg);
    }
}

 