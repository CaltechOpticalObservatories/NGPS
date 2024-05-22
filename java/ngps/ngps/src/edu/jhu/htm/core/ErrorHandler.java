package edu.jhu.htm.core;

/**********************************************************************
* It is intended that all core ERRORS are reported through this class thus
* making them uniform and allowing the possibility of redirecting them or whatever
* later.
* Also all erros need to have a number assigned to them. All this are here as statics.
* This is not necessarily the best java way to do this but it works and can easily
* be mapped to C.
* All error numbers are to be negative and status numbers positive
*
<pre>
 Current Version
 ===============
 ID:	$Id: ErrorHandler.java,v 1.3 2003/02/19 15:46:11 womullan Exp $
 Revision: 	$Revision: 1.3 $
 Date/time:	$Date: 2003/02/19 15:46:11 $
</pre>
 @revision $Revision: 1.3 $
 @author wil
 */

import java.util.*;
public class ErrorHandler {
	public static final int PANIC = -1;
	public static final int INVALIDID = -2;
	public static final int NONAME = -3;
	public static final int INVALIDNAME = -4;
	public static final int TOOFINE = -5;

	public static Map messages = new HashMap();
	static {
		messages.put(Integer.valueOf(PANIC),
								"PANIC - the laws of geometry have failed.");
		messages.put(Integer.valueOf(INVALIDID),"Invalid HTM Id");
		messages.put(Integer.valueOf(NONAME),"Empty or null HTM name");
		messages.put(Integer.valueOf(INVALIDNAME),"Invalid HTM name");
		messages.put(Integer.valueOf(TOOFINE),"HTM cannot handle the resolution specified min is 2.68E-6.");
	}

  public ErrorHandler() {
  }

	public static void handleError (int errorNo) throws HTMException {
		String msg = (String)messages.get(new Integer(errorNo));
		System.err.println(new Date()+" Error **********************");
		System.err.println("#"+errorNo+" "+msg);
		System.err.println("******************************************");
		throw new HTMException(errorNo,msg);
	}

}
/**********************************************************************
* Revision History
* ================
*
* $Log: ErrorHandler.java,v $
* Revision 1.3  2003/02/19 15:46:11  womullan
* Updated comments mainly to make java docs better
*
* Revision 1.2  2003/02/14 16:46:00  womullan
*  Newly organised classes in the new packages
*
* Revision 1.1  2003/02/07 23:02:51  womullan
* Major tidy up of HTM to HTMfunc addition of exceptions and error handler
*
* *********************************************************************/
