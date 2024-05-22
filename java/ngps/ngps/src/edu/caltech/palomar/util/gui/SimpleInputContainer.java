package edu.caltech.palomar.util.gui;
 
public class SimpleInputContainer implements InputContainer {

   String _str= null;

   public boolean getUserEnteredExponent() { return false; }
   public String  getCurrentString()       { return _str;  }
   public void    setCurrentString(String s) { _str= s; }
}
