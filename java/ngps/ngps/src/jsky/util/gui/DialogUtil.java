/*
 * ESO Archive
 *
 * $Id: DialogUtil.java,v 1.2 2009/03/16 11:16:43 abrighto Exp $
 *
 * who             when        what
 * --------------  ----------  ----------------------------------------
 * Allan Brighton  1999/05/03  Created
 */
 
package jsky.util.gui;

import java.awt.*;
import java.net.UnknownHostException;

import javax.swing.*;

import jsky.util.ExceptionHandler;

import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.io.FileNotFoundException;

import org.xml.sax.SAXParseException;

/**
 * Utility class with static methods for commonly used dialogs.
 */
public class DialogUtil {

    // A list of ExceptionHandlers
    private static List<ExceptionHandler> _exceptionHandlerList;

    /**
     * Tries to guess the correct parent frame for a popup dialog by choosing
     * the active frame.
     *
     * @return the first active JFrame found
     */
    public static JFrame getActiveFrame() {
        for (Frame f : JFrame.getFrames()) {
            if (f instanceof JFrame && f.isActive()) {
                return (JFrame) f;
            }
        }
        return null;
    }

    /**
     * Add an exception handler, to be called whenever
     * DialogUtil.error(Exception) is called.
     *
     * @param handler the handler to add
     */
    public static void addExceptionHandler(ExceptionHandler handler) {
        if (_exceptionHandlerList == null) {
            _exceptionHandlerList = new LinkedList<ExceptionHandler>();
        }
        _exceptionHandlerList.add(handler);
    }

    /**
     * Remove an exception handler
     *
     * @param handler the handler to remove
     */
    public static void removeExceptionHandler(ExceptionHandler handler) {
        if (_exceptionHandlerList != null) {
            _exceptionHandlerList.remove(handler);
        }
    }


    /**
     * Report an error message.
     *
     * @param parentComponent display the dialog over the given component
     * @param msg the error message
     */
    public static void error(Component parentComponent, String msg) {
        JOptionPane.showMessageDialog(parentComponent, msg, "Error", JOptionPane.ERROR_MESSAGE);
    }

    /**
     * Report an error message.
     *
     * @param msg the error message
     */
    public static void error(String msg) {
        error(getActiveFrame(), msg);
    }


    /**
     * Report an error message based on the given exception.
     *
     * @param parentComponent display the dialog over the given component
     * @param e the exception containing the error information
     */
    public static void error(Component parentComponent, Exception e) {
        if (_exceptionHandlerList != null && _exceptionHandlerList.size() != 0) {
            Iterator it = _exceptionHandlerList.listIterator();
            while (it.hasNext()) {
                ExceptionHandler handler = (ExceptionHandler) it.next();
                if (handler.handleException(e)) {
                    return;
                }
            }
        }

        e.printStackTrace();

        // Get the root cause
        Throwable t = e;
        while (t.getCause() != null) {
            t = t.getCause();
        }

        String s = t.getMessage();
        if (t instanceof UnknownHostException) {
            s = "Unknown host: " + s;
        } else if (t instanceof FileNotFoundException) {
            s = "File not found: " + s;
        } else if (t instanceof SAXParseException) {
            s = "XML parsing error: " + s;
        } else if (s == null || s.trim().length() == 0) {
            s = t.toString();
        }

        JOptionPane.showMessageDialog(parentComponent, s, "Error", JOptionPane.ERROR_MESSAGE);
    }


    /**
     * Report an error message based on the given exception.
     *
     * @param e the exception containing the error information
     */
    public static void error(Exception e) {
        error(getActiveFrame(), e);
    }


    /**
     * Display an informational message.
     *
     * @param parentComponent display the dialog over the given component
     * @param msg the message
     */
    public static void message(Component parentComponent, String msg) {
        JOptionPane.showMessageDialog(parentComponent, msg, "Message", JOptionPane.INFORMATION_MESSAGE);
    }


    /**
     * Display an informational message.
     *
     * @param msg the message
     */
    public static void message(String msg) {
        message(getActiveFrame(), msg);
    }


    /**
     * Get an input string from the user and return it.
     *
     * @param parentComponent display the dialog over the given component
     * @param msg the message to display
     * @param initialValue the initial value to display
     * @return the value typed in by the user, or null if Cancel was pressed
     */
    public static String input(Component parentComponent, String msg, String initialValue) {
        return JOptionPane.showInputDialog(parentComponent, msg, initialValue);
    }

    /**
     * Get an input string from the user and return it.
     *
     * @param parentComponent display the dialog over the given component
     * @param msg the message to display
     * @return the value typed in by the user, or null if Cancel was pressed
     */
    public static String input(Component parentComponent, String msg) {
        return JOptionPane.showInputDialog(parentComponent, msg, "Input", JOptionPane.QUESTION_MESSAGE);
    }

    /**
     * Get an input string from the user and return it.
     *
     * @param msg the message to display
     * @return the value typed in by the user, or null if Cancel was pressed
     */
    public static String input(String msg) {
        return input(getActiveFrame(), msg);
    }

    /**
     * Get a choice from the user and return it.
     *
     * @param parentComponent display the dialog over the given component
     * @param msg the message to display
     * @param choices an array of items to choose from
     * @param initialValue the initial value to display
     * @return the value chosen by the user, or null if Cancel was pressed
     */
    public static Object input(Component parentComponent, String msg, Object[] choices, Object initialValue) {
        return JOptionPane.showInputDialog(parentComponent, msg, "Input", JOptionPane.QUESTION_MESSAGE,
                null, choices, initialValue);
    }


    /**
     * Display a confirm dialog with YES, NO, CANCEL buttons and return
     * a JOptionPane constant indicating the choice.
     *
     * @param parentComponent display the dialog over the given component
     * @param msg the message to display
     * @return a JOptionPane constant indicating the choice
     */
    public static int confirm(Component parentComponent, String msg) {
        return JOptionPane.showConfirmDialog(parentComponent, msg);
    }

    /**
     * Display a confirm dialog with YES, NO, CANCEL buttons and return
     * a JOptionPane constant indicating the choice.
     *
     * @param msg the message to display
     * @return a JOptionPane constant indicating the choice
     */
    public static int confirm(String msg) {
        return confirm(getActiveFrame(), msg);
    }
}


