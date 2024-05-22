package edu.jhu.htm.geometry;
import edu.jhu.htm.core.*;
/**********************************************************************

Classes which implement this must be able to return a Convex. Basically the geometric shapes can be constructed as one would expect but then they know how to produce a convex to represent themselves.

<pre>
 Current Version
 ===============
 ID:	$Id: ConvexProducer.java,v 1.3 2003/02/19 15:46:11 womullan Exp $
 Revision: 	$Revision: 1.3 $
 Date/time:	$Date: 2003/02/19 15:46:11 $
</pre>
 @author wil
 @revison $Revision: 1.3 $
*/
public interface ConvexProducer {
	Convex getConvex();
}
/**********************************************************************
 * Revision History
 * ================
 *
 * $Log: ConvexProducer.java,v $
 * Revision 1.3  2003/02/19 15:46:11  womullan
 * Updated comments mainly to make java docs better
 *
 * Revision 1.2  2003/02/14 21:46:23  womullan
 * *** empty log message ***
 *
 * Revision 1.1  2003/02/14 16:46:00  womullan
 *  Newly organised classes in the new packages
 *
 */
