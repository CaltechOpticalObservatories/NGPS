package edu.jhu.htm.parsers;
/**********************************************************************
 * exception which may be thrown by parsers
<pre>
 Current Version
 ===============
 ID:	$Id: ParseException.java,v 1.2 2003/02/19 15:46:11 womullan Exp $
 Revision: 	$Revision: 1.2 $
 Date/time:	$Date: 2003/02/19 15:46:11 $
</pre>
 @version: 	$Revision: 1.2 $
**************************************************************** **/

public class ParseException extends Exception {
	public ParseException(String msg) {
		super(msg);
	}
}

/***********************************
  $Log: ParseException.java,v $
  Revision 1.2  2003/02/19 15:46:11  womullan
  Updated comments mainly to make java docs better

*/
