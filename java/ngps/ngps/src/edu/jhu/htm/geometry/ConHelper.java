package edu.jhu.htm.geometry;

import edu.jhu.htm.core.*;
/**********************************************************************

just provides implementation for getConvex and Get Domain

<pre>
 Current Version
 ===============
 ID:	$Id: ConHelper.java,v 1.2 2003/02/19 15:46:11 womullan Exp $
 Revision: 	$Revision: 1.2 $
 Date/time:	$Date: 2003/02/19 15:46:11 $
</pre>
 @author wil
 @revison $Revision: 1.2 $
*/
public class ConHelper {
	protected Convex conv = null;
	protected Domain domain = null;
	public Convex getConvex() {
        return conv;
    }

    public Domain getDomain() {
        if (domain== null) {
            domain = new Domain();
            domain.add(conv);
        }
        return domain;
    }

}
/**********************************************************************
 * Revision History
 * ================
 *
 * $Log: ConHelper.java,v $
 * Revision 1.2  2003/02/19 15:46:11  womullan
 * Updated comments mainly to make java docs better
 *
 * Revision 1.1  2003/02/14 16:46:00  womullan
 *  Newly organised classes in the new packages
 *
 */
