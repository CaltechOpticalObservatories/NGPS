package edu.caltech.palomar.util.gui;

import edu.caltech.palomar.util.gui.AppProperties;
import javax.swing.Action;
 
/**
 * Property utilities for setting properties that must exist
 * on swing components.
 * You should only use this class when you know that the properties you are
 * looking for must exist.  If any of the methods do not find a property 
 * value then they return the property string as the value.  This is very 
 * useful for debugging.  It shows you on the widget the actual string that 
 * you are searching for.
 * @author Trey Roby
 */
public class Prop  {

  public static String getTip(String propBase) {
     String prop= propBase + "." + Action.SHORT_DESCRIPTION;
     return AppProperties.getProperty(prop,prop);
  }

  public static String getName(String propBase) {
     String prop= propBase + "." + Action.NAME;
     return AppProperties.getProperty(prop,prop);
  }

  public static String[] getNames(String base, String prop[]) {
     String retval[]= null;
     if (base != null) {
         if (prop != null) {
            retval= new String[prop.length];
            for (int i= 0; (i<prop.length); i++)
                     retval[i]= getName(base + "." + prop[i]);
         }
     }
     else {
         retval= getNames(prop); 
     }
     return retval;
  }

  public static String[] getNames(String propBase[]) {
     String retval[]= null;
     if (propBase != null) {
        retval= new String[propBase.length];
        for (int i= 0; (i<propBase.length); i++)
                retval[i]= getName(propBase[i]);
     }
     return retval;
  }

  public static String getTitle(String propBase) {
     String prop= propBase + "." + ActionConst.TITLE;
     return AppProperties.getProperty(prop,prop);
  }

  public static String getColumnName(String propBase) {
     String prop= propBase + "." + ActionConst.COLUMN_NAME;
     return AppProperties.getProperty(prop,prop);
  }

  public static String getColumnName(String propBase, String def) {
     String prop= propBase + "." + ActionConst.COLUMN_NAME;
     return AppProperties.getProperty(prop,def);
  }

  public static boolean getSelected(String propBase) {
     String prop= propBase + "." + ActionConst.SELECTED;
     return AppProperties.getBooleanProperty(prop);
  }

  public static float getFloatValue(String propBase) {
     String prop= propBase + "." + ActionConst.VALUE;
     return AppProperties.getFloatProperty(prop);
  }

  public static String getError(String propBase) {
     String prop= propBase + "." + ActionConst.ERROR;
     return AppProperties.getProperty(prop,prop);
  }

  public static String getErrorDescription(String propBase) {
     String prop= propBase + "." + ActionConst.ERROR_DESCRIPTION;
     return AppProperties.getProperty(prop,prop);
  }


}

