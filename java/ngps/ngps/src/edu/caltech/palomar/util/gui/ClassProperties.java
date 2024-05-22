package edu.caltech.palomar.util.gui;
 
import edu.caltech.palomar.util.gui.AppProperties;
import javax.swing.Action;

import java.util.StringTokenizer;


/**
 * You set the base of the property in the constructor.
 * You should only use this class when you know that the properties you are
 * looking for must exist.  If any of the methods do not find a property 
 * value then they return the property string as the value.  This is very 
 * useful for debugging.  It shows you on the widget the actual string that 
 * you are searching for.
 * @author Trey Roby
 */
public class ClassProperties  {

  protected String _root;
  protected String _base;

  public ClassProperties(String base, Class c) {
      _root= base;
      _base= base + ".";
      if (c!=null) AppProperties.loadClassProperties(c);
  }

  public ClassProperties(Class c) {
      this(getShortName(c), c); 
  }

  public ClassProperties(String base) { this(base,null); }

  public String getBase() {
     return _root;
  }

  public String makeBase(String s) {
     return (_base + s);
  }

  public String getName()            { return Prop.getName(_root); }
  public String getName(String prop) { return Prop.getName(_base + prop); }

  public String[] getNames(String prop[]) {
     return Prop.getNames(_base, prop);
  }

  public String getTitle()            { return Prop.getTitle(_root); }
  public String getTitle(String prop) { return Prop.getTitle(_base + prop); }

  public String getColumnName(String prop) {
     return Prop.getColumnName(_base + prop);
  }

  public boolean getSelected(String prop) {
     return Prop.getSelected(_base + prop);
  }

  public float getFloatValue(String prop) {
     return Prop.getFloatValue(_base + prop);
  }

  public String getError(String prop) {
     return Prop.getError(_base + prop);
  }

  public String getErrorDescription(String prop) {
     return Prop.getErrorDescription(_base + prop);
  }

  private static String getShortName(Class c) {
     StringTokenizer st     = new StringTokenizer(c.getName(), ".");
     String          retval = "";
     int             len    = st.countTokens();
     for (int i= 0; (i<len); retval= st.nextToken(), i++);
     return retval;
  }
}

