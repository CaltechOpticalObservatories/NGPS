package edu.caltech.palomar.util.gui;

 
import java.awt.event.TextListener;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import java.awt.event.TextEvent;
import java.awt.event.FocusListener;
import java.awt.event.FocusEvent;
import javax.swing.JTextField;
import javax.swing.JOptionPane;
import javax.swing.text.Document;
import javax.swing.text.PlainDocument;
import javax.swing.text.BadLocationException;
import javax.swing.text.AttributeSet;
import java.text.NumberFormat;
import java.text.ParseException;
import java.text.ParsePosition;
import java.beans.PropertyChangeListener;
import java.beans.PropertyChangeEvent;


public class ValidationHelper implements FocusListener,
                                         ActionListener,
                                         PropertyChangeListener {

       private boolean    _focusSelect= true;
       private TextAction _action;
       private JTextField _textField;

       private static String ERROR_DIALOG_TITLE  = "Error";

       /**
        * Get the model to use for this text field.
        * This constructor set the validation type to
        * ActionConst.RANGE_VALIDATION.
        * @param int number or columns
        * @param TextAction action
        */
       public ValidationHelper(JTextField textField,
                               TextAction action,
                               Document document) {
            _textField= textField;
            initListeners();
            initialize(action, document);
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

       public TextAction getTextAction() { return _action; }

       protected void initialize(TextAction action, Document document) {
            if (_action != null) _action.removePropertyChangeListener(this);
            _action= action;
            _action.addPropertyChangeListener(this);
             _textField.setDocument(document);
             _textField.setText(action.getValueString());
             _textField.setToolTipText(action.getTip());
       }


//========================================================================
//-------- Methods that implement FocusListener & ActionListener ---------
//========================================================================

       public void focusGained(FocusEvent e) {
            if (_focusSelect) _textField.selectAll();
       }
       public void focusLost(FocusEvent e) { takeEntry(); }
       public void actionPerformed(ActionEvent e) { takeEntry(); }

       public void propertyChange(PropertyChangeEvent e) {
            if (e.getPropertyName().equals("valueString")) {
                String str= (String)e.getNewValue();
                _textField.setText(str);
            }
       }

//========================================================================
//------------------- private /protected methods  & classes --------------
//========================================================================

       private void initListeners() {
            _textField.addFocusListener(this);
            _textField.addActionListener(this);
       }


       public void takeEntry() {
           try {
              _action.acceptField();
           } catch (OutOfRangeException e) {
               showValidationError(e.getMessage());
           }
        }

       protected void showValidationError(String str) {
           JOptionPane.showMessageDialog(_textField, str, ERROR_DIALOG_TITLE,
                                         JOptionPane.ERROR_MESSAGE);
           /*
            * The following repaint is there because in certain cases after
            * errors the JTextField does not put the correct value in until
            * the second paint... so this helps it get there faster.
            */
           _textField.repaint();
       }
}
