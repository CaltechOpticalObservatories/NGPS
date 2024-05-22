package edu.jhu.htm.core;

/**********************************************************************
* Exception thrown by erroHandler when an error is reported
<pre>
 Current Version
 ===============
 ID:	$Id: HTMException.java,v 1.3 2003/02/19 15:46:11 womullan Exp $
 Revision: 	$Revision: 1.3 $
 Date/time:	$Date: 2003/02/19 15:46:11 $
</pre>
* @revision $Revision: 1.3 $
* @author wil
 */

public class HTMException extends Exception {

  public int errorNum;

   public HTMException(int errorNum, String message) {
	super(message);
	this.errorNum = errorNum;
  }

  public int getErrorNum() {
	return errorNum;
  }

}

/**********************************************************************
* Revision History
* ================
*
* $Log: HTMException.java,v $
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
