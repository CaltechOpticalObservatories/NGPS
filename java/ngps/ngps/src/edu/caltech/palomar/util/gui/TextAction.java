package edu.caltech.palomar.util.gui;

import java.util.ArrayList; 
import java.util.Iterator;
import java.util.List;
import javax.swing.Action;
import javax.swing.ComboBoxModel;
import javax.swing.event.ListDataListener;
import javax.swing.event.ListDataEvent;
import java.beans.PropertyChangeListener;
import java.beans.PropertyChangeSupport;
import java.beans.VetoableChangeListener;
import java.beans.VetoableChangeSupport;

/**
 * This class is the base class for the FloatAction, DoubleAction, and 
 * IntAction.  All these actions do validation of a value within a range or
 * validation of a 1 of many values.  This class provides all the elements 
 * common to floats, ints, and doubles.  Through these classes contain methods
 * to configure them; the typical use is to define properties in a property
 * file that is loaded in the property database.  At initialization this class
 * searches the property database for specific properties and the configures
 * itself.  All properties are build from the property passed to the 
 * constructor.
 * 
 * These are the types of validation that can be done:
 * <ul>
 * <li>range - the value must be between a min and max inclusive.
 * <li>min   - the value must greater than a minimum value
 * <li>max   - the value must lass than a maximum value
 * <li>list  - the value must be a specific value for a list of possible values.
 * <li>none  - no validation, any value is valid.
 * </ul>
 *
 * @see FloatAction
 * @see IntAction
 * @see DoubleAction
 * @see ActionConst
 * 
 * @author Trey Roby
 */
public abstract class TextAction implements ComboBoxModel {

    protected PropertyChangeSupport _propChange= 
                                new PropertyChangeSupport(this);
    protected VetoableChangeSupport _vetoChange= 
                                new VetoableChangeSupport(this);
    private   String _propName       = null;
    private   String _tip            = null;
    private   String _desc           = "";
    private   String _errorDesc      = "";
    protected Number _validValues[]  = null;
    private   Number _selected       = null;
    private   int    _validationType = ActionConst.RANGE_VALIDATION;
    private   List   _listDataList   = new ArrayList(3);
    protected InputContainer  _inputContainer;

    protected static final String DEF_MIN_ERR_STRING  = 
                                 "Value must be greater than or equal to ";
    protected static final String DEF_MAX_ERR_STRING  = 
                                 "Value must be less than or equal to ";
    protected static final String DEF_RANGE_ERR_STRING= 
                                 "Value must be between ";
    private static final String DEF_START_LIST_ERR_STRING= 
                                 "Value must be one of the following: ";
    private final String MIN_ERR_STRING       = DEF_MIN_ERR_STRING;
    private final String MAX_ERR_STRING       = DEF_MAX_ERR_STRING;
    private final String RANGE_ERR_STRING     = DEF_RANGE_ERR_STRING;
    private final String START_LIST_ERR_STRING= DEF_START_LIST_ERR_STRING;

    public TextAction(String propName, InputContainer inputContainer) {
       _propName= propName;
       _inputContainer= (inputContainer == null) ? new SimpleInputContainer() :
                                                       inputContainer; 
    }

    /**
     * return the current value as a string.
     */
    public String getValueString() { 
         return _inputContainer.getCurrentString();
    }

     /*
      * Get some properties from the property database.<br>
      * These are the properties search for here.
      * <ul>
      * <li>Name - the use used for the label.
      * <li>ErrorDescription - The name for the action used in error messages.
      *                        If the property is not found then Name is used
      *                        in error messages.
      * <li>ShortDescrption - used for the tooltip
      * </ul>
      */
    protected void getProperties(String propName) {
      _desc=  AppProperties.getProperty( propName +"."+ Action.NAME, "");
      _errorDesc=  AppProperties.getProperty(
                        propName +"."+ ActionConst.ERROR_DESCRIPTION, _desc);
      _tip =  AppProperties.getProperty(
                       propName +"."+ Action.SHORT_DESCRIPTION, "");
    }

    /**
     * Get the integer validation type constant from the property database.
     */
    protected int getValidationProp(String propName) {
          int retval= -99;
          String vtype= AppProperties.getProperty(
                                 propName +"."+ ActionConst.VALIDATION );
    
          if (vtype != null) {
             if      (vtype.equals(ActionConst.RANGE_VALIDATION_STR)) 
                              retval= ActionConst.RANGE_VALIDATION;
             else if (vtype.equals(ActionConst.MIN_VALIDATION_STR)) 
                              retval= ActionConst.MIN_VALIDATION;
             else if (vtype.equals(ActionConst.MAX_VALIDATION_STR)) 
                              retval= ActionConst.MAX_VALIDATION;
             else if (vtype.equals(ActionConst.LIST_VALIDATION_STR)) 
                              retval= ActionConst.LIST_VALIDATION;
             else if (vtype.equals(ActionConst.NO_VALIDATION_STR)) 
                              retval= ActionConst.NO_VALIDATION;
          }
          return retval;
    }

    /**
     * return an array of numbers that are valid.
     */
   public Number [] getValidList() {
         return _validValues; 
   }

    /**
     * set an array of numbers that are valid.
     */
   protected void setValidList(Number validValues[]) {
         _validValues= validValues; 
         if (_validValues != null && _validValues.length > 0) 
                 _selected= _validValues[0];
   }

    public abstract void acceptField() throws OutOfRangeException;
    /**
     * return the mininum valid number as a string.
     */
    public abstract String   getMinString();
    /**
     * return the maximum valid number as a string.
     */
    public abstract String   getMaxString();
    /**
     * return the number as a string correctly formatted.
     */
    public abstract String   getValueAsString(Number n);

    /**
     * A Text description of this field used for Errors.
     */
    public void setErrorDescription(String s) { _errorDesc= s; }

    /**
     * Get the text description of this field used for Errors.
     */
    public String getErrorDescription() { return _errorDesc; }
    /**
     * A Text description of this field.
     */
    public void setDescription(String s) { _desc= s; }

    /**
     * Get the text description of this field.
     */
    public String getDescription() { return _desc; }
    public String getPropName() { return _propName; }
    /**
     * Set the validation Method.
     * Validation may be done in one of several ways.
     * <ul>
     * <li>ActionConst.NO_VALIDATION- validation is turned off.
     * <li>ActionConst.MIN_VALIDATION- value must be greater than or 
     *                      equal than a minimum
     * <li>ActionConst.MAX_VALIDATION- value must be less 
                                     than or equal than a maximum
     * <li>ActionConst.RANGE_VALIDATION- value must be in a range
     * <li>ActionConst.LIST_VALIDATION- must be one of several values
     * </ul>
     * @param int on of the above constants
     *
     */
    public void setValidationType(int type) {
           if  (type == ActionConst.NO_VALIDATION    ||
                type == ActionConst.MIN_VALIDATION   ||
                type == ActionConst.MAX_VALIDATION   ||
                type == ActionConst.RANGE_VALIDATION ||
                type == ActionConst.LIST_VALIDATION )
                         _validationType= type;
           else
              System.out.println(
                "setValidationType: wrong type passed- type= "+ type);
    }


    /** 
     * Get the validation Method.
     */
    public int  getValidationType() { return _validationType; }

    /**
     * Get the string to describe this error
     */
    protected String getUserErrorString() {
        String errString= null;
        int vtype= _validationType;
        if      (vtype == ActionConst.MIN_VALIDATION)
                errString= _errorDesc + " " + MIN_ERR_STRING + getMinString();
        else if (vtype == ActionConst.MAX_VALIDATION)
                errString= _errorDesc + " " + MAX_ERR_STRING + getMaxString();
        else if (vtype == ActionConst.RANGE_VALIDATION)
                errString= _errorDesc + " " + RANGE_ERR_STRING + 
                          getMinString() +" and "+ getMaxString();
        else if (vtype == ActionConst.LIST_VALIDATION) {
                StringBuffer outstr= new StringBuffer(100);
                outstr.append(_errorDesc);
                outstr.append(" ");
                outstr.append(START_LIST_ERR_STRING);
                outstr.append( getValidValuesString() );
                errString= outstr.toString();
        }
        else 
                Assert.tst(false,  
                          "Bad option passed to showValidationError.");
        return errString;
    }

    public String getValidValuesString() {
       StringBuffer outstr= new StringBuffer(100);
       for(int i=0; (i<_validValues.length); i++) {
           outstr.append(" ");
           outstr.append(getValueAsString(_validValues[i]));
       }
       return outstr.toString();
    }


    /**
     * Get the Tool tip
     */
    public String getTip() { return _tip; }

    /**
     * Set the Tool tip
     */
    public void setTip(String t) { _tip= t; }


    public Object getSelectedItem() {
         return _selected;
    }

    public void setSelectedItem(Object anItem) {
         int i;
         for(i= 0; (i<_validValues.length); i++) {
                 if (anItem == _validValues[i]) break;
         }
         if (i>=_validValues.length) i=0;
         _selected= _validValues[i];
         ListDataEvent ev= new ListDataEvent(this, 
                             ListDataEvent.CONTENTS_CHANGED, -1, -1);
         for(Iterator iter= _listDataList.iterator(); (iter.hasNext());) {
              ListDataListener listener= (ListDataListener)iter.next();
              listener.contentsChanged(ev);
         }
    }

    public void setEquiliventItem(Number anItem) {
         boolean found= false;
         int i;
         for(i=0; (i < _validValues.length  && !found); i++ ) {
              if (anItem.equals(_validValues[i])) 
                     found= true;
         }
         if (found) setSelectedItem(_validValues[i-1]);
    }

    public Object getElementAt(int i) {
        return _validValues[i];
    }

    public int getSize() {
        int retval= 0;
        if (_validValues != null) retval= _validValues.length;
        return retval;
    }

    public void addListDataListener(ListDataListener l) {
      _listDataList.add(l);
    }
    public void removeListDataListener(ListDataListener l) {
      _listDataList.remove(l);
    }



    /**
     * Add a property changed listener.
     * @param PropertyChangeListener  the listener
     */
    public void addPropertyChangeListener (PropertyChangeListener p) {
       _propChange.addPropertyChangeListener (p);
    }

    /**
     * Remove a property changed listener.
     * @param PropertyChangeListener  the listener
     */
    public void removePropertyChangeListener (PropertyChangeListener p) {
       _propChange.removePropertyChangeListener (p);
    }

    /**
     * Add a vetoable change listener.
     * @param PropertyChangeListener  the listener
     */
    public void addVetoableChangeListener (VetoableChangeListener p) {
       _vetoChange.addVetoableChangeListener (p);
    }

    /**
     * Remove a vetoable change listener.
     * @param PropertyChangeListener  the listener
     */
    public void removeVetoableChangeListener (VetoableChangeListener p) {
       _vetoChange.removeVetoableChangeListener (p);
    }
}
