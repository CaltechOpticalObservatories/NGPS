package edu.caltech.palomar.util.gui;
 

//import java.awt.*;

import java.awt.event.TextListener;
import java.awt.event.TextEvent;
import java.awt.event.FocusListener;
import java.awt.event.ActionListener;
import java.awt.event.FocusEvent;
import java.awt.event.ActionEvent;
import java.text.NumberFormat;
import javax.swing.JTextField;
import javax.swing.JOptionPane;
import javax.swing.text.Document;
import javax.swing.text.PlainDocument;
import javax.swing.text.BadLocationException;
import javax.swing.text.AttributeSet;
import javax.swing.SwingUtilities;
import java.text.NumberFormat;
import java.text.ParseException;

/**
 * A subclass of JTextField that suports entering RA or longitude values and 
 * checking.  Only a valid RA or longitude may be entered in this field <p>
 *
 * @author Xiuqin Wu
 */
public class RaDecTextField extends JTextField implements 
                                         FocusListener, ActionListener {

       private boolean _mayDoNull= true;
       private boolean _focusSelect= true;
       private String  _property= null;
       private String  _desc= "";
       private String  _errorDesc= "";
       private String  _def= "";
       private String  _errString= "";
       private String  _valid_buf= "";
       private CoordSystemEquinox _csys;
       private boolean _isLat;

       private static String ERROR_DIALOG_TITLE  = "Error";
       private boolean _showedValidationError = false;

       /** 
        * Get the model to use for this RaDec text field.
        * @param int number or columns
        * @param boolean isLat (Latitude, longitude)
        */
       public RaDecTextField(int col, boolean isLat, CoordSystemEquinox csys ) {
            super(col);
            _csys =csys;
            _isLat = isLat;
            init();
       }

       /** 
        * Get the model to use for this text field.  This method is called
        * by the superclass.
        */
       protected Document createDefaultModel() {
            return new RaDecDocument();
       }

       private void init() {
            setRequestFocusEnabled(true);
            setValueDontValidate(_def);
            addFocusListener(this);
            addActionListener(this);
       }

       /**
        * A Text description of this field.
        */
       public void setDescription(String s) { _desc= s; }

       /**
        * A Text error description of this field.
        */
       public void setErrorDescription(String s) { _errorDesc= s; }

       /** 
       /** 
        * Set the value of the RaDecTextField and validate by doing range
        * checking.
        */
       public void setValue(String v){// should throw a value out of range excep.
	    //System.out.println("setValue() in RaDecTextField: "+ v);
            if (validate(v))  
               setValueDontValidate(_valid_buf);
            else 
               setValueDontValidate(_def);
       }

       /** 
        * Set the value of the RaDecTextField but don't do any validation.
        */
       protected void setValueDontValidate(String v) {
	      setText(v);
       }

       /** 
        * Select the number in the text field anytime the user enters the
        * field.  When enabled this allows the user to click in a text
        * field and type a new entry without having to delete the old
        * one.  When data is selected it will be automaticly deleted as soon
        * as the user begins to type.
        * @param boolean enable or disable
        */
       public void setSelectOnFocus(boolean b) { _focusSelect= b; }
      


       /** 
        * Set the default value for validation.
        * @param int the default value the is put in the field when the
        *      user enters one that is out of range
        */
       public void setDefault(String def) { _def= def; }
       /** 
        * Get the default value for validation.
        */
       public String  getDefault()        { return _def; }

       /**
        * set Enabled/Disabled float field having null values. When a null
        * value is enabled the field may be empty.
        * @param boolean Enabled/Disabled doing nulls
        */
       public void setMayDoNull(boolean b) { _mayDoNull= b; }

       /**
        * get Enabled/Disabled float field having null values. When a null
        * value is enabled the field may be empty.
        * @return boolean Enabled/Disabled doing nulls
        */
       public boolean getMayDoNull() { return _mayDoNull; }


       /** 
        * Get a value and make sure it has been validated.
        */
       public String getValidatedValue() { 
           takeEntry();
           return getValue();
       }

       /** 
        * Get the value of the RaDecTextField.
        */
       public String getValue() { 
           return getText();
       }


       /**
        * Validate the RaDecTextField.
        */
       public boolean validate(String v) {
           boolean retval= false;
	   Coord coord;

	   //System.out.println("validate: "+ v);
	   if (_mayDoNull && v.equals("")) {
	      _valid_buf = _def;
	      retval = true;
	      }
	   else {
	      /*
	      coord = new Coord(v, _isLat, _csys.isEquatorial());
	      coord.c_setprecision(5);
	      try {
		   _valid_buf = coord.getCanonicalString();
		   //setValueDontValidate(_valid_buf);
	      //System.out.println("validated: "+ _valid_buf);
		   retval = true;
		  } catch (CoordException cex) {
		     _errString = _errorDesc + " "+
				  cex.getMessage() + ": " + getValue();
		     }
	      */
	      try {
		   _valid_buf = CoordUtil.format(v, _isLat,
					   _csys.isEquatorial(), 5);
		   retval = true;
		  } catch (CoordException cex) {
		     _errString = _errorDesc + " "+
				  cex.getMessage() + ": " + getValue();
		     }
	      }
	   return retval;
       }


//========================================================================
//-------- Methods that implement FocusListener & ActionListener ---------
//========================================================================

       public void focusGained(FocusEvent e) { 
          if (_focusSelect) selectAll();
	  _showedValidationError = false;
	  }
       public void focusLost(FocusEvent e) {
          //System.out.println("focusLost");
          takeEntry();
          }
       public void actionPerformed(ActionEvent e) {
          //System.out.println("actionPerformed");
          takeEntry();
          //transferFocus();
          }

//========================================================================
//------------------- private /protected methods  & classes --------------
//========================================================================

       private void takeEntryNoMessage() {
           boolean goodValue= validate(getValue());
           if (goodValue) {
	      setValueDontValidate(_valid_buf);
	      }
	   else  {
	      setValueDontValidate(_def);
	      }
	   }

       private void takeEntry() {
           boolean goodValue= validate(getValue());
           if (goodValue) {
	      setValueDontValidate(_valid_buf);
	      }
           else {
	      setValueDontValidate(_def);
	      showValidationError();
              }
       }

       protected void showValidationError() {
           if (!_showedValidationError) {
	      _showedValidationError = true;
	      JOptionPane.showMessageDialog(
                      SwingUtilities.windowForComponent(this), _errString, 
	              ERROR_DIALOG_TITLE, JOptionPane.ERROR_MESSAGE);
	      }
	   //requestFocus();
	   //grabFocus();
       }

       /**
        * The document to use for this RaDecTextField.
        */
       class RaDecDocument extends PlainDocument {
           public void insertString(int offs, String str, AttributeSet a) 
                                             throws BadLocationException {
                /*
                int len= getLength();
                String ts= super.getText(0,len);
                StringBuffer tmp= new StringBuffer(ts).insert(offs, str);
                ts= tmp.toString();
                String convert_str;
                int v;
                if (ts.length() == 0) 
                         ts= new String("0");
                else if ( (ts.length() == 1) && (ts.equals("-")) )
                         ts= new String("-0");
                try {
                         v= Integer.parseInt(ts);
                } catch (NumberFormatException ex) {
                         str= ""; 
                }      
                */
                super.insertString(offs,str,a);
           }
       }

}





