// Copyright 2002
// Association for Universities for Research in Astronomy, Inc.,
// Observatory Control System, Gemini Telescopes Project.
//
// $Id: ExceptionHandler.java,v 1.1.1.1 2009/02/17 22:24:40 abrighto Exp $

package jsky.util;

/**
 * An interface for handling exceptions
 */
public abstract interface ExceptionHandler {

    /**
     * Returns true if the exception was handled.
     */
    public boolean handleException(Exception e);
}
 
