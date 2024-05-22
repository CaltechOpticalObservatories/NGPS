package edu.caltech.palomar.util.gui;


import javax.swing.text.PlainDocument;
import javax.swing.text.BadLocationException;
import javax.swing.text.AttributeSet;
import java.text.ParsePosition;
import java.text.NumberFormat;
import java.awt.Toolkit;
import javax.swing.JLabel;
 
/**
 * The document to use for a FloatTextField.
 *
 * @author Trey Roby
 */
class FloatTextDocument extends PlainDocument implements InputContainer {

    private boolean      _userEnteredExp= false;
    private NumberFormat _nf            = NumberFormat.getInstance();
    private String       _currentStr    = "".intern();
    private Toolkit      _toolkit       = new JLabel().getToolkit();

    public boolean getUserEnteredExponent()   { return _userEnteredExp; }
    public String  getCurrentString()         { return _currentStr; }
    public void    setCurrentString(String s) { _currentStr= s; }

    public void insertString(int offs, String str, AttributeSet a) 
                                             throws BadLocationException {
         int len= getLength();
         String ts= super.getText(0,len);
         StringBuffer tmp= new StringBuffer(ts).insert(offs, str);
         ts= tmp.toString();
         String convert_str;
         ts= ts.toUpperCase();
         if (ts.indexOf('E') > -1) {
            _userEnteredExp= true;
         }
         else {
            _userEnteredExp= false;
         }

         if (len == 0) {
                  ts= new String("0");
         }
         else if ( (ts.length() == 1) && (ts.equals("-")) ){
                  ts= new String("-0");
         }
         else if ( ts.charAt(0) == '+' ){
                  ts= ts.substring(1, ts.length() -1);
         }
         else if ( ts.endsWith("E") && !ts.startsWith("E") ) {
                  ts= ts.substring(0, ts.length() -1);
         }
         else if ( ts.endsWith("E+")  || ts.endsWith("E-") && 
                  !ts.startsWith("E") ) {
                  ts= ts.substring(0, ts.length() -2);
         }
         ParsePosition pos = new ParsePosition(0);
         Number result = _nf.parse(ts, pos);
         if (pos.getIndex() != ts.length()) {
                  str= ""; 
                  if (_toolkit != null) _toolkit.beep();
         }
         super.insertString(offs,str,a);
         len= getLength();
         _currentStr= super.getText(0,len);
    }
 
    public void remove(int offs, int len) throws BadLocationException {
         super.remove(offs,len);
         len= getLength();
         _currentStr= super.getText(0,len);
    }
}

