/*
 * ESO Archive
 *
 * $Id: BasicWindowMonitor.java,v 1.1.1.1 2009/02/17 22:24:35 abrighto Exp $
 *
 * who             when        what
 * --------------  ----------  ----------------------------------------
 * Allan Brighton  1999/05/03  Created (from book example)
 */

package jsky.util.gui;

import java.awt.event.*;
import java.awt.Window;

/**
 * This simple utility class is used to delete the main window
 * when an application exits.
 */
public class BasicWindowMonitor extends WindowAdapter {

    public void windowClosing(WindowEvent e) {
        Window w = e.getWindow();
        w.setVisible(false);
        w.dispose();
        System.exit(0);
    } 
}
