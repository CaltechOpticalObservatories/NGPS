package edu.caltech.palomar.util.gui;
 
/**
 * This exception is thrown when a value is out of range.
 * @see FloatAction
 * @see IntAction
 * @see ValidationTextField
 * @see FloatTextField
 * @see IntTextField
 * @author Trey Roby
 */
public class OutOfRangeException extends Exception {

    /**
     * Create a new OutOfRange Exception.
     * @param String the error message.
     */
    boolean _warningOnly;


    /**
     * Create a new OutOfRange Exception.
     * @param String the error message.
     */
    public OutOfRangeException(String mess) {
        this(mess,false);
    }

    /**
     * Create a new OutOfRange Exception.
     * @param String the error message.
     * @param boolean this exception is only a warning this data is on
     *                really out of range but in some zone of concern.
     */
    public OutOfRangeException(String mess, boolean warningOnly) {
        super(mess);
        _warningOnly= warningOnly;
    }

    public boolean isWarningOnly() { return _warningOnly; }
}

