package edu.jhu.htm.geometry;

import edu.jhu.htm.core.*;
/**********************************************************************

represents a rectangle on the sky .

<pre>
 Current Version
 ===============
 ID:	$Id: Rect.java,v 1.2 2003/02/19 15:46:11 womullan Exp $
 Revision: 	$Revision: 1.2 $
 Date/time:	$Date: 2003/02/19 15:46:11 $
</pre>

 @author wil
 @revison $Revision: 1.2 $
*/

/** specify four corners order imaterial */
public class Rect extends ConHelper implements ConvexProducer, DomainProducer {
	public Rect(Vector3d tl, Vector3d tr, Vector3d br, Vector3d bl) {
		conv = new Convex(tl,tr,br,bl);
	}
	/** specify corners in ra dec  order should not matter
	 * this assumes your box is ra1,dec1, ra2,dec1, ra1,dec2, ra2,dec2
	 *
	 * **/
	public Rect(double ra1, double dec1, double ra2, double dec2) {
		Vector3d tl = new Vector3d(ra1,dec1);
		Vector3d tr = new Vector3d(ra2,dec1);
		Vector3d bl = new Vector3d(ra1,dec2);
		Vector3d br = new Vector3d(ra2,dec2);
		conv = new Convex(tl,tr,br,bl);
	}


}
/**********************************************************************
 * Revision History
 * ================
 *
 * $Log: Rect.java,v $
 * Revision 1.2  2003/02/19 15:46:11  womullan
 * Updated comments mainly to make java docs better
 *
 * Revision 1.1  2003/02/14 16:46:00  womullan
 *  Newly organised classes in the new packages
 *
 */
