package edu.jhu.htm.core;

import java.util.Iterator;

/**********************************************************************
 Simple iterator to return all IDs from a htm Range.
 Be warned this uses the badly implemented Iterator of HTMrange
 and calls reset when you start it up.
<pre>
 Current Version
 ===============
 ID:	$Id: HTMrangeIterator.java,v 1.3 2003/02/19 16:49:46 womullan Exp $
 Revision: 	$Revision: 1.3 $
 Date/time:	$Date: 2003/02/19 16:49:46 $
</pre>
 * @author  wil
 * @version $Revision: 1.3 $
 */

public class HTMrangeIterator implements Iterator {
	protected HTMrange range = null;
	protected boolean symbolic = false ; // return Symbolic names instead of numbers
	protected Object next = null;
	protected long[] currange = null;
	protected long nextval = -1L;

	public HTMrangeIterator(HTMrange range, boolean symbolic) throws HTMException{
		 this.range= range;
		 this.symbolic=symbolic;
		 range.reset();
		 currange = range.getNext();
		 nextval = currange[0] - 1;
		 getNext();

	}
	/* internal method to get the next ID and have it ready so we can answer
	 true or false to has next */
	protected void getNext() throws HTMException {
		if (currange == null ) {
			next = null;
			return;
		}

		nextval++;

		if (nextval > currange[1]) {
			 currange = range.getNext();
			if (currange == null  || currange[0]==0) {
				next = null;
				return;
			}
			 nextval = currange[0];
		}

		// so now we have a next val - we need either a string or
		// we need a Long object.

		if (symbolic) {
			next = HTMfunc.idToName(nextval);
		} else {
			next = new Long(nextval);
		}
	}

	public boolean hasNext() {
		return next != null;
	}

	public Object next() {
		Object ret = next;
		while (next != null && next==ret) {
            try {
                getNext();
            } catch (Exception e) {
               // return e.toString();
            }
		}
        return ret;
	}

	public String nextAsString() {
		return (String)next();
	}

	public Long nextAsLong() {
		return (Long)next();
	}

	public void remove() {
	/**@todo Implement this java.util.Iterator method*/
	throw new java.lang.UnsupportedOperationException("Method remove() not yet implemented.");
	}

}
/************************************************************
* Revision History
* ================
*
* $Log: HTMrangeIterator.java,v $
* Revision 1.3  2003/02/19 16:49:46  womullan
* small bug fix
*
* Revision 1.2  2003/02/19 15:46:11  womullan
* Updated comments mainly to make java docs better
*
* Revision 1.1  2003/02/14 16:46:00  womullan
*  Newly organised classes in the new packages
*
*
************************************************************************/
