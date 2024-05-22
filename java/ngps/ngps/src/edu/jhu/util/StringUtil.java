package edu.jhu.util;
/*
**********************************************************************
*   Has a String utility to strip new lines.
    and may more
* ID:	$Id: StringUtil.java,v 1.2 2003/02/19 15:46:11 womullan Exp $
* @version	$Revision: 1.2 $	
* @author   wil
**/

public class StringUtil {

    private static final char[] numericArray = {'0','1','2','3','4','5','6','7','8','9'};

    private static final String numericString = new String(numericArray);

    public static String stripNewLines(String s){
        // different version - strips by ascii code..
        StringBuffer sb = new StringBuffer();
        byte[] b = s.getBytes();
        for (int i = 0; i<b.length;i++){
            if ((b[i] == 13)||b[i]==10){
                b[i] = 44;
            }
        }
        try{
            return new String(b,"UTF-8");
        }catch(Exception e){
            e.printStackTrace();
        }
        return "";
    }

    /* strip a give token=value& from a request String */
    public static String stripToken(String token, String inpString) {
        int ind = inpString.indexOf(token);
        if (ind < 0) return inpString;
        int stripind = ind;
        if (ind > 0)  stripind--;
        StringBuffer sb = new StringBuffer(inpString.substring(0,stripind));
        int endInd = inpString.indexOf('&',ind) ;
        if (endInd > 0) {
            sb.append(inpString.substring(endInd,inpString.length()));
        }
        return stripToken(token, sb.toString()); // easy recursion
    }

    public static void main(String args[]){
        String s = "This \n is a \r Str\ning with \r new lines\n";
        System.out.println("Original String:");
        System.out.println(s);
        System.out.println("New String:");
        System.out.println(StringUtil.stripNewLines(s));

        String inp = "par=par&opt=debug&someother=2";
        System.out.println("Original String:");
        System.out.println(inp);
        System.out.println("New String:");
        System.out.println(StringUtil.stripToken("opt",inp));



    }

    /**
    * A method for checking that all the characters of a string are
    * numeric, could try to create an Integer or Long, but this is more flexible
    * as far as length is concerned.
    *
    * @param inStr the String to be checked
    * @return int -1 if all the characters are numeric, or the int of
    * the first character that is not numeric.
    */
    public static int isNumeric(String inStr){

        int size = inStr.length();
        int retValue = -1;

        // loop through each character and check it is numeric
        for (int i=0;i < size; i++){

            char checkChar = inStr.charAt(i);

            if (numericString.indexOf(checkChar)== -1) {  // this character is not numeric
                return i; // return the index of the bad character from the inStr
            }
        }

        return retValue;
    }


    /** put the given string in ot the string buffer but buffer it to
     *  the given number of chars with spaces
     */
     public static void buffer(StringBuffer sb, String s, int size) {
        buffer(sb,s,size,true);
     }
     /* buffer a string to size - put the buffer spaces left or rigt
       according tot he boolean spaceonleft
       */
     public static void buffer(StringBuffer sb, String s, int size,boolean spaceonleft) {
        int len = 0;
        if (s!=null) len = s.length();
        int dif = size - len;
        if (!spaceonleft && s!=null) {
          sb.append(s);
        }
        for (int i= 0; i < dif; i++) {
            sb.append(" ");
        }
        if (spaceonleft && s!=null) {
          sb.append(s);
        }
     }
}
/********************************************
* Revision History
* ================
*
* $Log: StringUtil.java,v $
* Revision 1.2  2003/02/19 15:46:11  womullan
* Updated comments mainly to make java docs better
*
* Revision 1.1  2003/02/14 16:46:00  womullan
*  Newly organised classes in the new packages
*
*/
