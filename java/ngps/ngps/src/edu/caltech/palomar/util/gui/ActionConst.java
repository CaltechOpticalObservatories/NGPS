package edu.caltech.palomar.util.gui;
 
/**
 * A class of constants for the action package. 
 * These constants are used with property files as part of the keys in 
 * the files.
 *
 * @author Trey Roby
 *
 */
public class ActionConst {

       /**
        * specifies that this button is the default button
        */
    public static String DEFAULT_BUTTON= "DefaultButton"; // a boolean value

       /**
        * 
        */
    public static String ACTION_COMMAND= "ActionCommand"; // a String value

       /**
        * defines an accelerator in a pulldown menu
        */
    public static String MNEMONIC      = "Mnemonic";      // a Character value

       /**
        * defines an mnemonic in a pulldown menu
        */
    public static String ACCELERATOR   = "Accelerator";   // a KeyStroke value

       /**
        * Sepcifies the current radio value from a list
        */
    public static String RADIO_VALUE   = "RadioValue";    // a Strig value

       /**
        * Sepcifies that this property is selected or not.
        */
    public static String SELECTED      = "Selected";      // a boolean value

       /**
        * specifies a list of items. 
        */
    public static String ITEMS         = "Items";         // a String[]

       /**
        * Used with pulldowns to specify if this pulldown is a help pulldown
        */
    public static String ISHELP        = "IsHelpMenu";    // a boolean value

       /**
        * the integer value associated with a property. Used with radio.  A 
        * radio my defines a string value and a integer value.
        */
    public static String INT_VALUE     = "IntValue";      // a int value

       /**
        * Do not do any validation.  This constant disables all validation
        */ 
    public static int NO_VALIDATION      =0;
       /**
        * The value must be greater than a minimum to validate.
        */ 
    public static int MIN_VALIDATION     =1;
       /**
        * The value must be less than a maximum to validate.
        */ 
    public static int MAX_VALIDATION     =2;
       /**
        * The value must be between a minimum and a maximum to validate.
        */ 
    public static int RANGE_VALIDATION   =3;
       /**
        * The value must be between one a list of valid values
        */ 
    public static int LIST_VALIDATION   =4;
       /**
        * Do not do any validation.  This constant disables all validation
        * This string will be used in property files.
        */ 
    public static String NO_VALIDATION_STR      = "none";
       /**
        * The value must be greater than a minimum to validate.
        * This string will be used in property files.
        */ 
    public static String MIN_VALIDATION_STR     = "min";
       /**
        * The value must be less than a maximum to validate.
        * This string will be used in property files.
        */ 
    public static String MAX_VALIDATION_STR     = "max";
       /**
        * The value must be between a minimum and a maximum to validate.
        * This string will be used in property files.
        */ 
    public static String RANGE_VALIDATION_STR   = "range";
       /**
        * The value must be one of a list of valid values
        * This string will be used in property files.
        */ 
    public static String LIST_VALIDATION_STR   = "list";

      /**
       * The minimum value a text field may contain.
       * value is a int or float depending on context
       */
    public static String MIN         = "Min";         // a int or float
      /**
       * The maximum value a text field may contain.
       * value is a int or float depending on context
       */
    public static String MAX         = "Max";         // a int or float
      /**
       * The default value of a text field.  This also may be used for
       * the initial value.
       * value is a int or float depending on context
       */
    public static String DEFAULT       = "Default";       // a int or float
      /**
       * The generic value.
       * value is a int or float depending on context
       */
    public static String VALUE         = "Value";       // a int or float
      /**
       * The number of decemal places a float should be displayed with.
       * value is a int.
       */
    public static String PRECISION   = "Precision";   // a int
      /**
       * The exact way to display the number this overrides precision
       * value is a String
       */
    public static String PATTERN   = "Pattern";   // a String
      /**
       * How to display a number
       * value is a CHAR. either I, F, E for integer, float, or exponetail
       */
    public static String DISPLAY_TYPE= "DisplayType"; // a char I, F, E
      /**
       * How to validate a field.  The value is a string.
       */
    public static String VALIDATION  = "Validation";  // a String
      /**
       * This field may have null values.
       * value is a boolean.
       */
    public static String NULL_ALLOWED  = "NullAllowed";  // a boolean
      /**
       * This field will echo scientific notation if the user enters it 
       * in that format.
       * value is a boolean.
       */
    public static String SCIENTIFIC_ALLOWED= "ScientificAllowed";  // a boolean
       /**
       * This field may have null values.
       * value is a boolean.
       */

    public static String IS_EDITABLE  = "IsEditable";  // a boolean

      /**
       * Represent a float field that the use choose not to enter a value
       * in the GUI.
       */
    public static float FLOAT_NULL= Float.NaN;

      /**
       * Represent a int field that the use choose not to enter a value
       * in the GUI.
       */
    public static int INT_NULL= Integer.MIN_VALUE;

      /**
       * Used when a text field will support nulls.
       */
    public static String NULL_STR   = "null";

      /**
       * A title property
       */
    public final static String TITLE= "Title";

      /**
       * A Column Name property. Used for getting the column names of a table.
       */
    public final static String COLUMN_NAME= "ColumnName";

      /**
       * A Error String property.
       */
    public final static String ERROR= "Error";

      /**
       * A String to describe the field that is generating an error
       */
    public final static String ERROR_DESCRIPTION= "ErrorDescription";
      /**
       * A String to describe all the characters a text field may not have
       */
    public final static String INVALID_CHARACTERS= "InvalidCharacters";

      /**
       * A int - the maximum number of charaters that may be in this string
       */
    public final static String MAX_LENGTH= "MaxLength";
}

