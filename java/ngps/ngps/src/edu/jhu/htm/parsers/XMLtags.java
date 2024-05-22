package edu.jhu.htm.parsers;
/**
 * this speeds up the matching of elements by using a hash function instead of
 * string compares. Hit is taken at startup. Means we can use a switch in
 * the parser though instead of loads of ifs
 * Basically the String becomes a key to hashmap of numbers
 * we use the numbers in the code rather than the string
<pre>
 Current Version
 ===============
 ID:		$Id: XMLtags.java,v 1.3 2003/02/19 15:46:11 womullan Exp $
 Revision: 	$Revision: 1.3 $
 Date/time:	$Date: 2003/02/19 15:46:11 $
</pre>
 @author wil
 @version: 	$Revision: 1.3 $ 
 */
import java.util.*;
public class XMLtags {
	public static final String[] tags = {
		"area",     //0
		"convex",   //1
		"contraint",//2
		"circle",   //3
		"point",    //4
		"radius",   //5
		"rect",     //6
		"chull",    //7
		"poly"     //8
	};

	public static final int AREA = 0;
	public static final int CONVEX = 1;
	public static final int CONSTRAINT = 2;
	public static final int CIRCLE = 3;
	public static final int POINT = 4;
	public static final int RADIUS = 5;
	public static final int RECT = 6;
	public static final int CHULL = 7;
	public static final int POLY = 8;

	public static final Map tagMap = new HashMap();

	static {
		for (int i = 0 ; i < tags.length; i++) {
			tagMap.put(tags[i],new Integer(i));
		}
	}

	public static int getTag(String name) throws ParseException {
		Integer i =(Integer)tagMap.get(name.toLowerCase());
		if (i==null) throw new ParseException("Tag not found "+name);
		return i.intValue();
	}
}

/* 
   $Log: XMLtags.java,v $
   Revision 1.3  2003/02/19 15:46:11  womullan
   Updated comments mainly to make java docs better
 

*/
