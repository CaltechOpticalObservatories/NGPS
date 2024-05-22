package edu.jhu.htm.core;

/**********************************************************************
 * Simple class used inside HTMfunc.
</pre>
 Current Version
 ===============
 ID:	$Id: Base.java,v 1.3 2003/02/19 15:46:11 womullan Exp $
 Revision: 	$Revision: 1.3 $
 Date/time:	$Date: 2003/02/19 15:46:11 $
</pre>
**********************************************************************
*/
class  Base {
	String name;
	int ID;
	int v1, v2, v3;
	public Base(String n, int id, int v1, int v2, int v3) {
		this.name = n;
		this.ID=id;
		this.v1=v1;
		this.v2=v2;
		this.v3=v3;
	}
}
/**********************************************************************
* Revision History
* ================
*
* $Log: Base.java,v $
* Revision 1.3  2003/02/19 15:46:11  womullan
* Updated comments mainly to make java docs better
*
* Revision 1.2  2003/02/07 23:02:51  womullan
* Major tidy up of HTM to HTMfunc addition of exceptions and error handler
*
* Revision 1.1  2003/02/05 23:00:07  womullan
* Added new timimig routines and HTM clas to do cc stuff from the c++ package
*
*
************************************************************************/
