/*
 * Copyright 2002 Association for Universities for Research in Astronomy, Inc.,
 * Observatory Control System, Gemini Telescopes Project.
 *
 * $Id: SwingUtil.java,v 1.2 2009/03/05 10:28:48 abrighto Exp $
 */

package jsky.util.gui;

import java.awt.Component;
import java.awt.Frame;
import java.awt.Window;
import java.awt.GraphicsConfiguration;
import java.awt.Point;
import java.awt.Rectangle;

import javax.swing.JFrame;


/**
 * Various Swing related utility methods.
 *
 * @version $Revision: 1.2 $
 * @author Allan Brighton
 */
public class SwingUtil {

     /**
     * Makes sure that the given JFrame is visible
     * @param frame the frame to show
     *
     */
    public static void showFrame(JFrame frame) {
        frame.setVisible(true);
        frame.setState(Frame.NORMAL);
    }

    /**
     * Return the JFrame containing the given window, or null if there is none.
     * @param window a window in the frame
     * @return the frame corresponding to the window 
     */
    public static JFrame getFrame(Component window) {
        if (window == null) {
            return DialogUtil.getActiveFrame();
        }
        if (window instanceof JFrame) {
            return (JFrame) window;
        }

        return getFrame(window.getParent());
    }

    /**
     * Locates one window 'after' another one - probably a bit lower and
     * to the right.  The second window is repositioned relative to the
     * first one.
     * <p>
     * Taken from TopCat AuxWindow.
     *
     * @param   first   first window, or <tt>null</tt>
     * @param   second  second window
     */
    public static void positionAfter( Component first, Window second ) {

        /* Only attempt anything if the two windows exist on the same
         * display device. */
        GraphicsConfiguration gc = second.getGraphicsConfiguration();
        if ( first == null || gc.equals( first.getGraphicsConfiguration() ) ) {

            /* Work out the position of the first window. */
            Point pos = null;
            if ( first != null ) {
                pos = first.getLocation();
            }
            if ( pos == null ) {
                pos = new Point( 20, 20 );
            }

            /* Set a new position relative to that. */
            pos.x += 60;
            pos.y += 60;

            /* As long as we won't go outside the bounds of the screen,
             * reposition the second window accordingly. */
            // This code, though well-intentioned, doesn't do anything very
            // useful since the bounds of the new window are typically
            // both zero (because it's not been posted to the screen yet
            // or something).  Not sure what to do about this.
            Rectangle newloc = new Rectangle( second.getBounds() );
            newloc.setLocation( pos );
            Rectangle screen = gc.getBounds();
            // The Rectangle.contains(Rectangle) method is no good here -
            // always returns false for height/width = 0 for some reason.
            if ( screen.x <= newloc.x &&
                 screen.y <= newloc.y &&
                 ( screen.x + screen.width ) >= ( newloc.x + newloc.width ) &&
                 ( screen.y + screen.height ) >= ( newloc.y + newloc.height ) ){
                second.setLocation( pos );
            }
        }
    }
}
