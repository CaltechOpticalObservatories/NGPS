package edu.caltech.palomar.util.gui;

/**
 * This exception is thrown when an attribute is invalid.
 * @author Michael Nguyen
 */
 
public class HtmlAttributeException extends Exception 
{
  /**
  * Default constructor
  * @param String the message.
  */
  public HtmlAttributeException (String message) 
  { 
    super(message); 
  }
}


