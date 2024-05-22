package edu.caltech.palomar.util.gui;

import java.text.NumberFormat;
 
/**
 * A double object that allows you to control the toString output using
 * a NumberFormat classes.  This is very useful in options menus.
 * @see java.text.NumberFormat
 * @author Trey Roby
 */
public class FormatDouble extends Number {
   private Double        _f;
   private NumberFormat _nf;

   /**
    * Create a new FormatDouble class.
    * @param double the double values
    * @param NumberFormat the NumberFormat class will control how 
    * the double is outputted using the toString
    */
   public FormatDouble(double f, NumberFormat nf) {
      _f= Double.valueOf(f);
      _nf= nf;
      Assert.tst(nf);
   }

   /**
    * Create a new FormatDouble class.
    * @param String a string representing a double value
    * @param NumberFormat the NumberFormat class will control how 
    * the double is outputted using the toString
    */
   public FormatDouble(String s, NumberFormat nf) throws NumberFormatException {
      this(Double.parseDouble(s), nf);
   }
   /**
    * return this double as a stirng
    * @return  String a representation of the double
    */
   public String toString() {
      return _nf.format(_f.doubleValue());
   }
   /**
    * Compare this object against a Double or a FormatDouble object.
    * Any other type of object will return a false
    * @param object any object but only Double or FormatDouble will work.
    * @return boolean equal or not equal
    */
   public boolean equals(Object obj) { 
       boolean retval= false;
       if (obj instanceof Double)
             retval= _f.equals(obj);
       else if (obj instanceof FormatDouble) {
             retval= toString().equals(obj.toString());
       }
       return retval;
  }
 
  public NumberFormat getNumberFormat() { return _nf; }

  public byte   byteValue()  { return _f.byteValue(); }
  public float  floatValue() { return _f.floatValue(); }
  public double doubleValue(){ return _f.doubleValue(); }
  public int    intValue()   { return _f.intValue(); }
  public long   longValue()  { return _f.longValue(); }
  public short  shortValue() { return _f.shortValue(); }
}


