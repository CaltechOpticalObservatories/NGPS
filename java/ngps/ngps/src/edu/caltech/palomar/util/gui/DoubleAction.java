package edu.caltech.palomar.util.gui;
 
import java.text.NumberFormat;
import java.text.DecimalFormat;
import java.text.ParseException;
import java.beans.PropertyChangeSupport;
import java.util.ArrayList;
import java.util.List;
import java.beans.VetoableChangeSupport;
import java.beans.PropertyVetoException;

/**
 * A class that validates a double against a set of test.
 *
 * @see TextAction
 * @see ActionConst
 * @see ActionConst
 * @see java.text.NumberFormat
 *
 * @author Trey Roby
 */
public class DoubleAction extends TextAction {

    private final static String PROP  = "DoubleAction";
    private final static ClassProperties _prop = new ClassProperties(PROP, DoubleAction.class);
    private static final String EXP_PATTERN= "0.###E0";
    private static final String DOT =        ".".intern();
    private double       _max       = Double.MAX_VALUE;
    private double       _min       = Double.MIN_VALUE;
    private double       _def       = 0.0;
    private NumberFormat _nf        = NumberFormat.getInstance();
    private NumberFormat _nfExp     = NumberFormat.getInstance();
    private boolean      _mayDoNull = false;
    private boolean      _expAllowed= true;
    private String       _pattern   = null;
    private int          _precision = 2;
    private final static String NOT_FOUND = "&lt;Not Found&gt;";

 

    /**
     * Construct a double action base on a property name.
     */
    public DoubleAction(String propName, InputContainer inputContainer) { 
       super(propName, inputContainer);
       if (propName != null) getProperties(propName);
       if (_pattern == null) setPrecision(_precision);
       else                ((DecimalFormat)_nf).applyPattern(_pattern);
       ((DecimalFormat)_nfExp).applyPattern(EXP_PATTERN);
       setValueDontValidate(_def);
    }

    public DoubleAction(String propName) {
       this(propName, null);
    }

    public DoubleAction() {
       this(null, null);
    }


     /**
      * Get some properties from the property database. Start by getting
      * properties from the super method and then add more to it.<br>
      * These are the properties search for here:
      * <ul>
      * <li>Default - the default value for this action
      * <li>Min - the minimum value for this action
      * <li>Max - the maximum value for this action
      * <li>Precision - how many decimal places this action can have.
      * <li>Pattern - overrides precision. What the number looks like to the
      *               user.  The pattern is directly passed to the NumberFormat
      *               class to control how this number is displayed.
      * <li>NullAllowed - this action may have a null value. Otherwise this 
      *                   action always has a value. False by default.
      * <li>ScientificAllowed -  The use may enter scientific notation.
      *                          True by default.
      * </ul>
      */
    protected void getProperties(String propName) {
      super.getProperties(propName);
      _def=    AppProperties.getDoubleProperty(
                    propName +DOT+ ActionConst.DEFAULT , _def, true);
      _min=    AppProperties.getDoubleProperty(
                    propName +DOT+ ActionConst.MIN , _min );
      _max=    AppProperties.getDoubleProperty(
                    propName +DOT+ ActionConst.MAX , _max );
      _precision= AppProperties.getIntProperty(
                    propName +DOT+ ActionConst.PRECISION , _precision );
      _pattern= AppProperties.getProperty(
                    propName +DOT+ ActionConst.PATTERN , null );
      _mayDoNull= AppProperties.getBooleanProperty(
                    propName +DOT+ ActionConst.NULL_ALLOWED, _mayDoNull );
      _expAllowed= AppProperties.getBooleanProperty(
                  propName +DOT+ ActionConst.SCIENTIFIC_ALLOWED, _expAllowed );
      if (_mayDoNull) _def= ActionConst.FLOAT_NULL;

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
         FormatDouble validValues[];
         int i;
         String items=    AppProperties.getProperty(
                                propName +DOT+ ActionConst.ITEMS);
         if (items != null) { 
               validValues= StringUtil.strToFormatDoubles(items,_nf);
               setValidList(validValues);
               double selected= AppProperties.getDoubleProperty(
                          propName +DOT+ ActionConst.DEFAULT, 
                          ActionConst.FLOAT_NULL);
               if (selected != ActionConst.FLOAT_NULL) {
                     setEquiliventItem(new FormatDouble(selected,_nf) );
               } // end if selected
         } // end if items
      } // end if vtype
    }

    public Object clone() {
       DoubleAction fa= (DoubleAction)clone();
       fa._nf= NumberFormat.getInstance();
       if (_pattern == null) {
            fa.setPrecision(_precision);
       }
       else {
          fa._pattern= _pattern;
          ((DecimalFormat)_nf).applyPattern(_pattern);
       }
       return fa;
    }

    /** 
     * Set the precision, i.e. the number of decimal places of this double.
     */
    public void setPrecision(int p) {
       _precision= p;
       _nf.setMaximumFractionDigits(p);
       _nf.setMinimumFractionDigits(p);
    }

    /** 
     * Set the value of the DoubleAction and validate by doing range
     * checking.
     */
    public void setValue(double v){// should throw a value out of range excep.
        if (goodValue(v))  setValueDontValidate(v);
    }

    /** 
     * Set to the default value or null
     */
    public void setToDefault() {
         setValueDontValidate(_mayDoNull ? Double.NaN  : _def);
    }

    /** 
     * Set the value of the DoubleAction but don't do any validation.
     */
    public void setValueDontValidate(double v) {
         String old= _inputContainer.getCurrentString();
         if (_mayDoNull && isNull(v)) {
             _inputContainer.setCurrentString("".intern());
         }
         else {
             if (_inputContainer.getUserEnteredExponent() && _expAllowed) {
                 _inputContainer.setCurrentString(_nfExp.format(v));
             }
             else {
                 _inputContainer.setCurrentString(_nf.format(v));
             }
         }
         if (getValidationType()== ActionConst.LIST_VALIDATION &&
             _validValues != null &&
             _validValues.length > 0) {
             setEquiliventItem( new FormatDouble(v,_nf) );
         }
         _propChange.firePropertyChange ( "valueString", old, 
                                          _inputContainer.getCurrentString());
    }

    protected boolean isNull(double v) { return Double.isNaN(v); }

    public double getValue() {
         return getValueFromString(_inputContainer.getCurrentString());
    }

    /** 
     * Get the value of the DoubleAction.
     */
    private double getValueFromString(String ts) { 
        double v= 0.0;
        if (getValidationType()== ActionConst.LIST_VALIDATION) {
            v= ((FormatDouble)getSelectedItem()).doubleValue();
        }
        else {
           try {
              if (ts.length() == 0) {
                  v= ActionConst.FLOAT_NULL;
              }  // end if
              else {
                  if ( ts.charAt(0) == '+' ) ts= ts.substring(1);
                  ts= ts.toUpperCase();
                  Number num= _nf.parse(ts);
                  v= num.doubleValue();
              } // end else
           } catch (ParseException ex) {
                  v= _mayDoNull ? ActionConst.FLOAT_NULL  : _def;
           } // end catch
        } // end else
        return v;
    }

    public void acceptField() throws OutOfRangeException {
         try {
             double v= getValue();
             validate(v); 
             setValueDontValidate(v);
         } catch (OutOfRangeException e) {
             setToDefault();
             throw e;
         }
    }

    public void validate(double v) throws OutOfRangeException {
           if (!goodValue(v)) {
               throw new OutOfRangeException(getUserErrorString());
           }
           try {
               _vetoChange.fireVetoableChange("value", Double.valueOf(v), null);
           } catch (PropertyVetoException e) {
               throw new OutOfRangeException(e.getMessage());
           }
    }

    public boolean goodValue(double v) {
           boolean retval= false;
           int vtype= getValidationType();
           if (_mayDoNull && isNull(v)) {
                   retval= true;
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
                        if (validValues[i].doubleValue() == v) retval= true;
                    }
                }
           }
           return retval;
    }



       /** 
        * Set the minimum value for validation.
        * @param double the minimum value this field can contain.
        */
       public void setMin(double min)     { _min= min; }
       /** 
        * Get the minimum value for validation.
        */
       public double  getMin()            { return _min; }

       /** 
        * Get the minimum value for validation as a formatted String.
        */
       public String getMinString()      { return _nf.format(_min); }

       public String getValueAsString(Number n) { 
          return _nf.format(n.doubleValue()); 
       }

       /** 
        * Set the maximum value for validation.
        * @param double the maximum value this field can contain.
        */
       public void setMax(double max)     { _max= max; }
       /** 
        * Get the minimum value for validation.
        */
       public double  getMax()            { return _max; }

       /** 
        * Get the maximum value for validation as a formatted String.
        */
       public String getMaxString()      { return _nf.format(_max); }

       /** 
        * Set the default value for validation.
        * @param double the default value the is put in the field when the
        *      user enters one that is out of range
        */
       public void setDefault(double def) { _def= def; }
       /** 
        * Get the minimum value for validation.
        */
       public double  getDefault()        { return _def; }

       /** 
        * Get the default value for validation as a formatted String.
        */
       public String getDefaultString()      { return _nf.format(_def); }

       /**
        * set Enabled/Disabled double field having null values. When a null
        * value is enabled the field may be empty.
        * @param boolean Enabled/Disabled doing nulls
        */
       public void setNullAllowed(boolean b) { _mayDoNull= b; }

       /**
        * get Enabled/Disabled double field having null values. When a null
        * value is enabled the field may be empty.
        * @return boolean Enabled/Disabled doing nulls
        */
       public boolean isNullAllowed() { return _mayDoNull; }

       /**
        *  Collect info for HTML document
        * @param HtmlDocumentEntry the entry
        */
       public void document(HtmlDocumentEntry  entry) {
          List items = makeList(4);
          String description = ((getErrorDescription() != "") ? 
                                            getErrorDescription() : NOT_FOUND);
          if (description.equals(NOT_FOUND))
             description = ((getDescription() != "") ? getDescription() : 
                                  NOT_FOUND);

          if ((getMax() != Double.MAX_VALUE) && (getMin() != Double.MIN_VALUE)) {
             items.add(_prop.getName("minValue") + " "  + getMinString()); 
             items.add(_prop.getName("maxValue") + " "  + getMaxString()); 
             items.add(_prop.getName("defaultValue") + " "  + 
                              getDefaultString(getMax(), getMin(), getDefault())); 
             items.add(_prop.getName("Precision") + " "  + String.valueOf(_precision)); 
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

       private String getDefaultString(double max, double min, double def) {
           return (((def <= max) && (def >= min)) ? getDefaultString() : 
                        NOT_FOUND);
        }
       
        private String getDefaultString(Number[] values, double def) {
           for (int i = 0; i < values.length; i++)
             if (values[i].doubleValue() == def) return getDefaultString();
                return NOT_FOUND;
         }

	 protected List makeList(int total) { return new ArrayList(total); } 
}

