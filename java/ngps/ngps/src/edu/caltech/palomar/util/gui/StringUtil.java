package edu.caltech.palomar.util.gui;

import java.util.StringTokenizer;
import java.util.NoSuchElementException;
import java.util.ArrayList;
import java.text.NumberFormat;
 


/**
 * A class with static methods that do operations on Strings.
 * @author Trey Roby Xiuqin Wu
 */
public class StringUtil {

    public static final String NO_EXTENSION = "NO-EXTENSION";
   /**
    * Parse a stirng into an array of strings.  If the stirng contains quotes
    * then every things inside the quotes will be in one strings.  Works
    * much like the unix command line.  Therefore the String:
    * <ul><li><code>aaa "bbb ccc ddd" eee</code></li></ul>
    * should become:
    * <ol>
    * <li><code>aaa</code>
    * <li><code>bbb ccc ddd</code>
    * <li><code>eee</code>
    * </ol>
    * @param String the string to be parsed
    * @return a String array of the tokens
    */
   public static String [] strToStrings(String instr) {
       String astr, delim= " ", tmpstr;
       String [] retval;
       ArrayList tokens= new ArrayList(12);
       Assert.tst(instr != null);
       StringTokenizer st = new StringTokenizer(instr, delim);
       try {
           while (true) {
               astr= st.nextToken(delim);
               if (astr.charAt(0) == '"') {
                   StringBuffer adder= new StringBuffer(astr);
                   adder.deleteCharAt(0);
                   if (adder.toString().endsWith("\"")) {
                       adder.deleteCharAt(adder.length()-1);
                   }
                   else {
                       try {
                          tmpstr= st.nextToken("\"");
                          adder.append( tmpstr );
                       } catch (NoSuchElementException e) {
                          adder.append( st.nextToken(delim) );
                       }
                       st.nextToken(delim);
                   }
                   astr= adder.toString(); 
               } // end if
               tokens.add(astr);
           }  // end while
       } catch (NoSuchElementException e) { }
       retval = new String[tokens.size()];
       for(int i= 0; (i<retval.length); i++) 
                             retval[i]= (String)tokens.get(i);
       return retval;
   }


   /**
    * Return an array of Integers from a string that contains
    * space separated integers. If any of the space separated
    * tokens is not an integer then a zero is placed in that position.
    * @param String the string to be parsed that should 
                    contain space separated integers
    * @return a Integer array
    */
   public static Integer [] strToIntegers(String instr) {
       String [] strary= strToStrings(instr);
       Integer[] intary= new Integer[strary.length];
       for(int i= 0; (i<strary.length); i++ ) {
           try {
                   intary[i]= Integer.valueOf(strary[i]);      
           }
           catch (NumberFormatException e) {
                   System.out.println("StringUtil.stringtoItegers: " +
                         "Could not convert " + strary[i] + " to Integer");
                   intary[i]= Integer.valueOf(0);
           }
       }
       return intary;
   }

   /**
    * Return an array of Floats from a string that contains
    * space separated floats. If any of the space separated
    * tokens is not a float then a zero is placed in that position.
    * @param String the string to be parsed that should 
                    contain space separated integers
    * @return a Float array
    */
   public static Float [] strToFloats(String instr) {
       String [] strary= strToStrings(instr);
       Float[] fary= new Float[strary.length];
       for(int i= 0; (i<strary.length); i++ ) {
           try {
                   fary[i]= Float.valueOf(strary[i]);      
           }
           catch (NumberFormatException e) {
                   System.out.println("StringUtil.stringtoItegers: " +
                         "Could not convert " + strary[i] + " to Float");
                   fary[i]= Float.valueOf(0);
           }
       }
       return fary;
   }

   /**
    * Return an array of Doubles from a string that contains
    * space separated doubles. If any of the space separated
    * tokens is not a double then a zero is placed in that position.
    * @param String the string to be parsed that should 
                    contain space separated integers
    * @return a Double array
    */
   public static Double [] strToDoubles(String instr) {
       String [] strary= strToStrings(instr);
       Double[] fary= new Double[strary.length];
       for(int i= 0; (i<strary.length); i++ ) {
           try {
                   fary[i]= Double.valueOf(strary[i]);      
           }
           catch (NumberFormatException e) {
                   System.out.println("StringUtil.stringtoItegers: " +
                         "Could not convert " + strary[i] + " to Double");
                   fary[i]= Double.valueOf(0);
           }
       }
       return fary;
   }

   public static FormatFloat[] strToFormatFloats(String       instr, 
                                                 NumberFormat nf) {
       String [] strary= strToStrings(instr);
       FormatFloat[] fary= new FormatFloat[strary.length];
       for(int i= 0; (i<strary.length); i++ ) {
           try {
                   fary[i]= new FormatFloat(strary[i], nf);      
           }
           catch (NumberFormatException e) {
                   System.out.println("StringUtil.stringtoItegers: " +
                         "Could not convert " + strary[i] + " to FormatFloat");
                   fary[i]= new FormatFloat(0F,nf);
           }
       }
       return fary;
   }

   public static FormatDouble[] strToFormatDoubles(String       instr, 
                                                   NumberFormat nf) {
       String [] strary= strToStrings(instr);
       FormatDouble[] fary= new FormatDouble[strary.length];
       for(int i= 0; (i<strary.length); i++ ) {
           try {
                   fary[i]= new FormatDouble(strary[i], nf);      
           }
           catch (NumberFormatException e) {
                   System.out.println("StringUtil.stringtoItegers: " +
                         "Could not convert " + strary[i] + " to FormatDouble");
                   fary[i]= new FormatDouble(0F,nf);
           }
       }
       return fary;
   }

   /** 
    * return true if there are only spaces in the string
    */
   public static boolean isSpaces(String s) {
      int     length = s.length();
      boolean retval = true;

      for (int i=0; i<length; i++) {
         if (!Character.isWhitespace(s.charAt(i))) {
            retval = false;
            break;
            }
         }
      return retval;
   }
   
   /** 
    * removes extra spaces from a string.
    * <ul><li><code>" bbb    ccc  ddd"</code></li></ul>
    * should become:
    * <ul><li><code>aaa "bbb ccc ddd" eee</code></li></ul>
    */
    public static String crunch (String s) {
        if (s != null)
        {
            int counter = 0;
            StringTokenizer st = new StringTokenizer(s);
            StringBuffer sb = new StringBuffer();
            String token = "";
            while (st.hasMoreTokens()){
                token = st.nextToken();
                if (token.trim().length() > 0){//if there is something to write
                    if (counter > 0)
                    {
                        sb.append(" " + token);
                    }
                    else if  (counter == 0){
                        sb.append(token);
                        counter ++;
                    }
                }
            }
            s = sb.toString();
        }
        return s;
    }




   public static void main(String args[]) {
       //String original= "\"asdf\" xxx";
       //String original= "\"asdf\" \"xxx\"";
       String original= "asdf \"qqq rrr\" xxx \"bbbb   \" ccc   " +
                        "\"   ffff\" \" ggg \" \" rrr sss ttt \" ";
       //String original= "\"asdf \"";
       System.out.println("original: "+ original);
       String out[]= strToStrings(original);
       System.out.println("out.length: "+ out.length);
       for (int i= 0; (i<out.length); i++) {
             System.out.println("<" + out[i]+">");
       }
   }
}


