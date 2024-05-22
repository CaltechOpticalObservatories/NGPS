package edu.caltech.palomar.util.gui;


import javax.swing.JTextField;

 
/**
 * A subclass of JTextField that supports entering double values and range
 * checking.  Only a numeric number may by entered in this field and three
 * types of range checking are supported. The class also allows the programmer
 * to set the number of decimal places that the field will display.<p>
 * The available types are range checking are:
 * <ul>
 * <li>ActionConst.MIN_VALIDATION - the entered value must be 
 *     greater then a minimum value.
 * <li>ActionConst.MAX_VALIDATION - the entered value must be 
 *     greater then a maximum value.
 * <li>ActionConst.RANGE_VALIDATION - the entered value must be 
 *     in a range between to minimum and a maximum.
 * <li>NO_VALIDATION - disable range checking.
 * </ul>
 * @author Trey Roby
 */
public class DoubleTextField extends JTextField {


      private ValidationHelper _helper;
      public  DoubleAction     _action;
      private int     default_col              = 10;
      private int     default_numDecimalPlaces = 5;
      private double  default_min              = 0.0;
      private double  default_max              = 10000.0;
      private double  default_default          = 10.0;
      private boolean default_nullAllowed      = false;
       /**
        * Create a double text field
        * @param double number of columns
        * @param String The name of the property for this text field
        */
       public DoubleTextField() {
           this(10, null);
           _action.setPrecision(default_numDecimalPlaces); 
           _action.setMin(default_min);
           _action.setMax(default_max);
           _action.setDefault(default_default);
           _action.setNullAllowed(default_nullAllowed);
           setValueDontValidate(default_default);
       }

      /** 
        * Create a double text field
        * @param double number of columns
        * @param String The name of the property for this text field
        */
       public DoubleTextField(int col, String propName) {
           super(col);
           FloatTextDocument ftd= new FloatTextDocument();
           _action= new DoubleAction(propName, ftd);
           _helper= new ValidationHelper(this, _action, ftd);
       }

       /** 
        * Get the model to use for this text field.
        * This constructor set the validation type to ActionConst.NO_VALIDATION.
        * @param double number of columns
        * @param double the precision, how may decimal places a field may have
        */
       public DoubleTextField(int col, int numDecimalPlaces) {
           this(col, null);
           _action.setPrecision(numDecimalPlaces);
       }

       /** 
        * Get the model to use for this text field.
        * This constructor set the validation type to 
        * ActionConst.RANGE_VALIDATION.
        * @param int number of columns
        * @param double minimum value for range checking
        * @param double maximum value for range checking
        * @param double default value for range checking- set the field to this
        * @param double the precision, how may decimal places a field may have
        * @param boolean allow/disallow null values
        * value when there is an the user enters an our of range value
        */
       public DoubleTextField(int     col, 
                              double  min, 
                              double  max, 
                              double  def, 
                              int     numDecimalPlaces,
                              boolean nullAllowed) {
           this(col, null);
           _action.setPrecision(numDecimalPlaces);
           _action.setMin(min);
           _action.setMax(max);
           _action.setDefault(def);
           _action.setNullAllowed(nullAllowed);
           setValueDontValidate(def);
       }

       /** 
        * Set the value of the DoubleTextField and validate by doing range
        * checking.
        */
       public void setValue(double v){//should throw a value out of range excep.
            _action.setValue(v);
       }

       /** 
        * Set the value of the DoubleTextField but don't do any validation.
        */
       protected void setValueDontValidate(double v) {
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
        * @param double the minimum value this field can contain.
        */
       public void setMin(double min) {
            _action.setMin(min);
       }
       /** 
        * Get the minimum value for validation.
        */
       public double  getMin() {
            return _action.getMin();
       }

       /** 
        * Set the maximum value for validation.
        * @param double the maximum value this field can contain.
        */
       public void setMax(double max) {
            _action.setMax(max);
       }
       /** 
        * Get the minimum value for validation.
        */
       public double  getMax() {
            return _action.getMax();
       }

       /** 
        * Set the default value for validation.
        * @param double the default value the is put in the field when the
        *      user enters one that is out of range
        */
       public void setDefault(double def) {
            _action.setDefault(def);
       }
       /** 
        * Get the minimum value for validation.
        */
       public double getDefault() {
            return _action.getDefault();
       }

       /**
        * set Enabled/Disabled double field having null values. When a null
        * value is enabled the field may be empty.
        * @param boolean Enabled/Disabled doing nulls
        */
       public void setNullAllowed(boolean b) {
            _action.setNullAllowed(b);
       }

       /**
        * get Enabled/Disabled double field having null values. When a null
        * value is enabled the field may be empty.
        * @return boolean Enabled/Disabled doing nulls
        */
       public boolean isNullAllowed() {
            return _action.isNullAllowed();
       }

       /** 
        * Get the value of the DoubleTextField.
        */
       public double getValue() { 
            return _action.getValue();
       }

       /** 
        * Get the Validated value of the DoubleTextField.
        */
       public double getValidatedValue() { 
           _helper.takeEntry();
           return getValue();
       }

       public DoubleAction getDoubleAction() { return _action; }
}
