package edu.caltech.palomar.util.gui;

import java.text.NumberFormat;
 
/**
 * A float object that allows you to control the toString output using
 * a NumberFormat classes.  This is very useful in options menus.
 * @see java.text.NumberFormat
 * @author Trey Roby
 */
public class FormatFloat extends Number {
   private Float        _f;
   private NumberFormat _nf;

   /**
    * Create a new FormatFloat class.
    * @param float the float values
    * @param NumberFormat the NumberFormat class will control how 
    * the float is outputted using the toString
    */
   public FormatFloat(float f, NumberFormat nf) {
      _f= Float.valueOf(f);
      _nf= nf;
      Assert.tst(nf);
   }

   /**
    * Create a new FormatFloat class.
    * @param String a string representing a float value
    * @param NumberFormat the NumberFormat class will control how 
    * the float is outputted using the toString
    */
   public FormatFloat(String s, NumberFormat nf) throws NumberFormatException {
      this(Float.parseFloat(s), nf);
   }
   /**
    * return this float as a stirng
    * @return  String a representation of the float
    */
   public String toString() {
      return _nf.format(_f.floatValue());
   }
   /**
    * Compare this object against a Float or a FormatFloat object.
    * Any other type of object will return a false
    * @param object any object but only Float or FormatFloat will work.
    * @return boolean equal or not equal
    */
   public boolean equals(Object obj) { 
       boolean retval= false;
       if (obj instanceof Float)
             retval= _f.equals(obj);
       else if (obj instanceof FormatFloat) {
             //retval= (_f.floatValue() == ((FormatFloat)obj).floatValue());
             retval= toString().equals(obj.toString());
       }
       return retval;
  }
 
  public NumberFormat getNumberFormat() { return _nf; }

  public byte   byteValue()  { return _f.byteValue(); }
  public double doubleValue(){ return _f.doubleValue(); }
  public float  floatValue() { return _f.floatValue(); }
  public int    intValue()   { return _f.intValue(); }
  public long   longValue()  { return _f.longValue(); }
  public short  shortValue() { return _f.shortValue(); }
}


