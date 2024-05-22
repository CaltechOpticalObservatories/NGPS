package edu.caltech.palomar.util.gui;


import javax.swing.text.PlainDocument;
import javax.swing.text.BadLocationException;
import javax.swing.text.AttributeSet;
import java.text.ParsePosition;
import java.text.NumberFormat;
import java.awt.Toolkit;
import javax.swing.JLabel;
 

/**
 * The document to use for this IntTextField.
 *
 * @author Trey Roby
 */
class IntTextDocument extends PlainDocument implements InputContainer {

    private String       _currentStr= "".intern();
    private Toolkit      _toolkit   = new JLabel().getToolkit();

    public boolean getUserEnteredExponent()   { return false; }
    public String  getCurrentString()         { return _currentStr; }
    public void    setCurrentString(String s) { _currentStr= s; }

    public void insertString(int offs, String str, AttributeSet a) 
                                             throws BadLocationException {
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
                  _toolkit.beep();
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
