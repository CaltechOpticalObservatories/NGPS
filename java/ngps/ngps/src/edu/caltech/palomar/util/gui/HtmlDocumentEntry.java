package edu.caltech.palomar.util.gui;

import java.util.List;

/**
 * This interface defines the HTML document entry.
 * @author Michael Nguyen
 */
 
public interface HtmlDocumentEntry
{
  public void insertEntry(String description, List items);
  public void clearEntry();
	public String toHtmlString() throws HtmlAttributeException;
}


