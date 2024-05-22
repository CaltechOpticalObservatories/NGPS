package edu.jhu.htm.geometry;
import edu.jhu.htm.core.*;

/**********************************************************************

 Classes which inplement this interface should be able to produce domains
 which can be used in intersection. IN some cases it will be easier to simply
 get a domain for shape - even if perhaps that domain contains only a single 
 convex. MOst geometric shapes will implement this and ConvexProducer.

<pre>
 Current Version
 ===============
 ID:	$Id: DomainProducer.java,v 1.2 2003/02/19 15:46:11 womullan Exp $
 Revision: 	$Revision: 1.2 $
 Date/time:	$Date: 2003/02/19 15:46:11 $
</pre>

 @author wil
 @revison $Revision: 1.2 $
*/
public interface DomainProducer {
	Domain getDomain() ;
}

/**********************************************************************
 * Revision History
 * ================
 *
 * $Log: DomainProducer.java,v $
 * Revision 1.2  2003/02/19 15:46:11  womullan
 * Updated comments mainly to make java docs better
 *
 * Revision 1.1  2003/02/14 16:46:00  womullan
 *  Newly organised classes in the new packages
 *
 */
