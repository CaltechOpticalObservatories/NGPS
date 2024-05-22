package edu.caltech.palomar.util.gui;


import javax.swing.JTextField;
 
/**
 * A subclass of JTextField that suports entering int values and range
 * checking.  Only a numeric number may by entered in this field and three
 * types of range checking are supported. <p>
 * The available types are range checking are:
 * <ul>
 * <li>ActionConst.MIN_VALIDATION - the entered value must be 
 *     greater then a minimum value.
 * <li>ActionConst.MAX_VALIDATION - the entered value must be greater 
 *     then a maximum value.
 * <li>ActionConst.RANGE_VALIDATION - the entered value must be 
 *     in a range between to minimum and a maximum.
 * <li>NO_VALIDATION - disable range checking.
 * </ul>
 * @author Trey Roby
 */
public class IntTextField extends JTextField {

      private ValidationHelper _helper;
      private IntAction        _action;


       /** 
        * Create a int text field
        * @param int number of columns
        * @param String The name of the property for this text field
        */
       public IntTextField(int col, String propName) {
           super(col);
           IntTextDocument itd= new IntTextDocument();
           _action= new IntAction(propName, itd);
           _helper= new ValidationHelper(this, _action, itd);
       }

       /** 
        * Get the model to use for this text field.
        * This constructor set the validation type to 
        * ActionConst.RANGE_VALIDATION.
        * @param int number of columns
        * @param int minimum value for range checking
        * @param int maximum value for range checking
        * @param int default value for range checking- set the field to this
        * @param boolean allow/disallow null values
        * value when there is an the user enters an our of range value
        */
       public IntTextField(int     col, 
                           int     min, 
                           int     max, 
                           int     def, 
                           boolean nullAllowed) {
           this(col, null);
           _action.setMin(min);
           _action.setMax(max);
           _action.setDefault(def);
           _action.setNullAllowed(nullAllowed);
           setValueDontValidate(def);
       }


       /** 
        * Set the value of the IntTextField and validate by doing range
        * checking.
        */
       public void setValue(int v){// should throw a value out of range excep.
            _action.setValue(v);
       }

       /** 
        * Set the value of the IntTextField but don't do any validation.
        */
       protected void setValueDontValidate(int v) {
            _action.setValueDontValidate(v);
       }

      

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
        * </ul>
        * @param int on of the above constants
        *
        */
       public void setValidationType(int type) { 
             _action.setValidationType(type);
       }

       /** 
        * Get the validation Method.
        */
       public int  getValidationType() { 
             return _action.getValidationType();
       }


       /** 
        * Set the minimum value for validation.
        * @param int the minimum value this field can contain.
        */
       public void setMin(int min) {
            _action.setMin(min);
       }
       /** 
        * Get the minimum value for validation.
        */
       public int  getMin() {
            return _action.getMin();
       }

       /** 
        * Set the maximum value for validation.
        * @param int the maximum value this field can contain.
        */
       public void setMax(int max) {
            _action.setMax(max);
       }
       /** 
        * Get the minimum value for validation.
        */
       public int  getMax() {
            return _action.getMax();
       }

       /** 
        * Set the default value for validation.
        * @param int the default value the is put in the field when the
        *      user enters one that is out of range
        */
       public void setDefault(int def) {
            _action.setDefault(def);
       }
       /** 
        * Get the minimum value for validation.
        */
       public int getDefault() {
            return _action.getDefault();
       }

       /**
        * set Enabled/Disabled int field having null values. When a null
        * value is enabled the field may be empty.
        * @param boolean Enabled/Disabled doing nulls
        */
       public void setNullAllowed(boolean b) {
            _action.setNullAllowed(b);
       }

       /**
        * get Enabled/Disabled int field having null values. When a null
        * value is enabled the field may be empty.
        * @return boolean Enabled/Disabled doing nulls
        */
       public boolean isNullAllowed() {
            return _action.isNullAllowed();
       }

       /** 
        * Get the value of the IntTextField.
        */
       public int getValue() { 
            return _action.getValue();
       }

       /** 
        * Get the Validated value of the IntTextField.
        */
       public int getValidatedValue() { 
           _helper.takeEntry();
           return getValue();
       }

       public IntAction getIntAction() { return _action; }
}


