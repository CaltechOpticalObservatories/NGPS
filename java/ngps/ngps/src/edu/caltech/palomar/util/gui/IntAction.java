package edu.caltech.palomar.util.gui;

import java.text.NumberFormat;
import java.text.ParseException;
import java.text.ParsePosition;
import java.util.ArrayList;
import java.util.List;
import java.beans.VetoableChangeSupport;
import java.beans.PropertyVetoException;
 
/**
 * A class that validates a integer against a set of test.
 *
 * @author Trey Roby
 */
public class IntAction extends TextAction {
    private final static String PROP  = "IntAction";
    private final static ClassProperties _prop = new ClassProperties(PROP, IntAction.class);
    private static final String DOT    = ".".intern();
    private int             _max       = Integer.MAX_VALUE;
    private int             _min       = Integer.MIN_VALUE;
    private int             _def       = 0;
    private boolean         _mayDoNull = false;
    private NumberFormat    _nf        = NumberFormat.getInstance();
    private final static String NOT_FOUND = "&lt;Not Found&gt;";

    public IntAction(String propName, InputContainer inputContainer) {
       super(propName, inputContainer);
       getProperties(propName);
       setValueDontValidate(_def);
    }

    public IntAction(String propName) {
       this(propName, null);
    }

    public IntAction() {
       this(null);
       setValueDontValidate(_def);
    }


     /**
      * Get some properties from the property database. Start by getting
      * properties from the super method and then add more to it.<br>
      * These are the properties search for here:
      * <ul>
      * <li>Default - the default value for this action
      * <li>Min - the minimum value for this action
      * <li>Max - the maximum value for this action
      * <li>NullAllowed - this action may have a null value. Otherwise this 
      *                   action always has a value. False by default.
      * </ul>
      */
    protected void getProperties(String propName) {
      super.getProperties(propName);
      _def=    AppProperties.getIntProperty(
                    propName +DOT+ ActionConst.DEFAULT , _def, true);
      _min=    AppProperties.getIntProperty(
                    propName +DOT+ ActionConst.MIN , _min );
      _max=    AppProperties.getIntProperty(
                    propName +DOT+ ActionConst.MAX , _max );
      _mayDoNull= AppProperties.getBooleanProperty(
                    propName +DOT+ ActionConst.NULL_ALLOWED, _mayDoNull );
      if (_mayDoNull) _def= ActionConst.INT_NULL;

       /*
        * 1. Get the validation type from a property if it exist
        *        and set it on the field
        * 2. If there is no property then set the validation 
        *        only if the min and max are equal (typically both are 0).
        * 3. Otherwise don't set the validation at all.
        */
      int vtype= getValidationProp(propName);
      if (vtype >= 0 ) {
            setValidationType( vtype );
      }
      else if (_min == _max) {
            setValidationType(ActionConst.NO_VALIDATION);
      }
      if (vtype == ActionConst.LIST_VALIDATION)  {
         Integer validValues[];
         int i;
         String items=    AppProperties.getProperty(
                                propName +DOT+ ActionConst.ITEMS);

         if (items != null) {
              validValues= StringUtil.strToIntegers(items);
              setValidList(validValues);
              int selected= AppProperties.getIntProperty(
                    propName +DOT+ ActionConst.DEFAULT, 
                    ActionConst.INT_NULL);
              if (selected != ActionConst.INT_NULL) {
                    setEquiliventItem(Integer.valueOf(selected) );
              } // end if selected
         } // end if items
      } // end if vtype
    }

    public Object clone() {
       IntAction ia= (IntAction)clone();
       return ia;
    }


    /** 
     * Set the value of the IntAction and validate by doing range
     * checking.
     */
    public void setValue(int v){// should throw a value out of range excep.
        if (goodValue(v))  setValueDontValidate(v);
    }

    /** 
     * Set to the default value or null
     */
    protected void setToDefault() {
         setValueDontValidate(_mayDoNull ? ActionConst.INT_NULL  : _def);
    }

    /** 
     * Set the value of the IntTextField but don't do any validation.
     */
    public void setValueDontValidate(int v) {
         String old= _inputContainer.getCurrentString();
         if (_mayDoNull && isNull(v)) {
              _inputContainer.setCurrentString("");
         }
         else {
              _inputContainer.setCurrentString(v + "" );
         }
         if (getValidationType()== ActionConst.LIST_VALIDATION &&
             _validValues != null &&
             _validValues.length > 0) {
             setEquiliventItem( Integer.valueOf(v) );
         }
        _propChange.firePropertyChange ( "valueString", old, 
                                       _inputContainer.getCurrentString() );
    }

    protected boolean isNull(int v) { return v == ActionConst.INT_NULL; }

    public int getValue() { 
         int v;
         if (getValidationType()== ActionConst.LIST_VALIDATION) {
            v= ((Integer)getSelectedItem()).intValue();
         }
         else {
            v= getValueFromString(_inputContainer.getCurrentString());
         }
         return v;
    }

    /** 
     * Get the value of the IntTextField
     */
    private int getValueFromString(String ts) { 
        int v= 0;
        try {
           if (ts.length() == 0) {
               v= ActionConst.INT_NULL;
           } 
           else {
               Number num= _nf.parse(ts);
               v= num.intValue();
           }
        } catch (ParseException ex) {
               v= _mayDoNull ? ActionConst.INT_NULL : _def;
        }      
        return v;
    }

    public void acceptField() throws OutOfRangeException {
         try {
             int v= getValue();
             validate(v); 
             setValueDontValidate(v);
         } catch (OutOfRangeException e) {
             setToDefault();
             throw e;
         }
    }

    public void validate(int v) throws OutOfRangeException {
           if (!goodValue(v)) {
               throw new OutOfRangeException(getUserErrorString());
           }
           try {
               _vetoChange.fireVetoableChange("value", Integer.valueOf(v), null);
           } catch (PropertyVetoException e) {
               throw new OutOfRangeException(e.getMessage());
           }
    }

    public boolean goodValue(int v) {
           boolean retval= false;
           int vtype= getValidationType();
           if (_mayDoNull && isNull(v)) {
                   retval= true;
           }
           else if (!_mayDoNull && isNull(v)) {
                   retval= false;
           }
           else if (vtype == ActionConst.NO_VALIDATION) {
                   retval= true;
           }
           else if (vtype == ActionConst.MIN_VALIDATION) {
                   if (v >= _min) retval= true;
           }
           else if (vtype == ActionConst.MAX_VALIDATION) {
                   if (v <= _max) retval= true;
           }
           else if (vtype == ActionConst.RANGE_VALIDATION) {
                   if ((v <= _max) && (v >= _min)) retval= true;
           }
           else if (vtype == ActionConst.LIST_VALIDATION) {
                retval= false;
                Number validValues[]= getValidList();
                if (validValues != null) {
                    for(int i=0; (i<validValues.length && !retval); i++) {
                        if (validValues[i].intValue() == v) retval= true;
                    }
                }
           }
           return retval;
    }




       /** 
        * Set the minimum value for validation.
        * @param int the minimum value this field can contain.
        */
       public void setMin(int min)     { _min= min; }
       /** 
        * Get the minimum value for validation.
        */
       public int  getMin()            { return _min; }

       /** 
        * Get the minimum value for validation as a formatted String.
        */
       public String getMinString()      { return _nf.format(_min); }

       public String getValueAsString(Number n) { 
          return _nf.format(n.intValue()); 
       }

       /** 
        * Set the maximum value for validation.
        * @param int the maximum value this field can contain.
        */
       public void setMax(int max)     { _max= max; }
       /** 
        * Get the minimum value for validation.
        */
       public int  getMax()            { return _max; }

       /** 
        * Get the maximum value for validation as a formatted String.
        */
       public String getMaxString()      { return _nf.format(_max); }

       /** 
        * Set the default value for validation.
        * @param int the default value the is put in the field when the
        *      user enters one that is out of range
        */
       public void setDefault(int def) { _def= def; }
       /** 
        * Get the minimum value for validation.
        */
       public int  getDefault()        { return _def; }

       public String getDefaultString()      { return _nf.format(_def); }

       /**
        * set Enabled/Disabled int field having null values. When a null
        * value is enabled the field may be empty.
        * @param boolean Enabled/Disabled doing nulls
        */
       public void setNullAllowed(boolean b) { _mayDoNull= b; }

       /**
        * get Enabled/Disabled int field having null values. When a null
        * value is enabled the field may be empty.
        * @return boolean Enabled/Disabled doing nulls
        */
       public boolean isNullAllowed() { return _mayDoNull; }

       /**
        *  Collect info for HTML document
        * @param HtmlDocumentEntry the entry
        */
       public void document(HtmlDocumentEntry  entry) {
          List items = makeList(3);
          String description = ((getErrorDescription() != "") ?
                                            getErrorDescription() : NOT_FOUND);
          if (description.equals(NOT_FOUND))
            description = ((getDescription() != "") ? getDescription() : 
                                   NOT_FOUND);

          if ((getMax() != Integer.MAX_VALUE) && (getMin() != Integer.MIN_VALUE)) {
             items.add(_prop.getName("minValue") + " " + getMinString()); 
             items.add(_prop.getName("maxValue") + " " + getMaxString()); 
             items.add(_prop.getName("defaultValue") + " " + 
                             getDefaultString(getMax(), getMin(), getDefault())); 
             entry.insertEntry(description, items);
          }
          else if (getValidList().length > 0)  {
             items.add(_prop.getName("validValues") + " " + getValidValuesString()); 
             items.add(_prop.getName("defaultValue") + " " + 
                             getDefaultString(getValidList(), getDefault())); 
             entry.insertEntry(description, items);
          }
          else if (!description.equals(NOT_FOUND)) {
             items.add(NOT_FOUND); 
             entry.insertEntry(description, items);
          }
       }

       private String getDefaultString(int max, int min, int def) {
          return (((def <= max) && (def >= min)) ? getDefaultString() : 
                        NOT_FOUND);
       }
       
       private String getDefaultString(Number[] values, int def) {
          for (int i = 0; i < values.length; i++)
             if (values[i].intValue() == def) return getDefaultString();
          return NOT_FOUND;
       }
 
       protected List makeList(int total) { return new ArrayList(total); } 

}

