package edu.jhu.htm.geometry;

import java.util.*;

import edu.jhu.htm.core.*;
/**********************************************************************

use a set of points to construct a domain
 construct with the set or create one and add points

<pre>
 Current Version
 ===============
 ID:	$Id: Chull.java,v 1.2 2003/02/19 15:46:11 womullan Exp $
 Revision: 	$Revision: 1.2 $
 Date/time:	$Date: 2003/02/19 15:46:11 $
</pre>
 @author wil
 @revison $Revision: 1.2 $
*/
public class Chull extends ConHelper implements ConvexProducer, DomainProducer {
	List corners  = new ArrayList();

	/** use this f you are going to add all points then call makeConvex() */
	public Chull() {};

	/** pass points in two arrays */
	public Chull(double[]ras, double[] decs) throws Exception {
		if (ras.length != decs.length) {
			    throw new Exception ("The number of ras do not match the number of dec ");
        }

		for (int c = 0 ; c < ras.length; c++) {
			add(ras[c],decs[c]);
		}
		makeConvex();
	}

	/** call this if you are done adding points  to actually construct the
	 * convex for the points get Convex does this anyway though
	 * if conv is null..
	 */
	public void makeConvex()  throws Exception{
		if(corners.size() < 3)
		throw new Exception(" empty hull: points on one line");

        Vector3d v;
		conv = new Convex();


        // The constraint we have for each side is a 0-constraint (great circle)
        // passing through the 2 corners. Since we are in counterclockwise order,
        // the vector product of the two successive corners just gives the correct
        // raint.
        int i, len = corners.size();
        for(i = 0; i < len; i++) {
			   int other = ((i+1)==len ? 0 : (i+1));
               v = ((Vector3d)corners.get(i)).cross((Vector3d)corners.get(1));
               v.normalize();
               Constraint c = new Constraint(v,0);
			   conv.add(c);
        }
	}

	/** add point to polygon - checks for existing points */
	public void add(double ra, double dec) {
		Vector3d v = new Vector3d(ra,dec);
		add(v);
	}

	public void add(Vector3d v) {
		Iterator iter = corners.iterator();
		int len = corners.size();
		while (iter.hasNext()) {
			Vector3d c = (Vector3d)iter.next();
			// no need to add a duplicate
			if (c.equal(v)) return;
		}
		if (len < 2)  { // just add it
            corners.add(v);
		} else if (len == 2) { // triangle work out orientation
			Vector3d prod = ((Vector3d)corners.get(0)).cross((Vector3d)corners.get(1));
			double mul = prod.mul(v);
			if (mul > 0)	{
			   corners.add(v);
			} else if (mul < 0) { // put it in 1 but move 1 to end
               corners.add(corners.get(1));
			   corners.remove(1);
               corners.add(1,v);
			}
		} else {
			boolean[] inside = new boolean[corners.size()];
			boolean[] replace = new boolean[corners.size()];
			// Now we set the flags for the existing polygon.
            // if the new point is inside (i.e. to the left) of
            // the half-sphere defined by the points polyCorners_[i],[i+1]
            // we set inside[i] to true.

            // if it is outside, and the previous side was also outside,
            // set the replace_ flag to true(this corner will be dropped)
            // (be careful on the edges - that's the trackoutside flag)

            boolean polyTrackOutside = false;
            for(int i = 0 ; i < len; i++) {
               replace[i] = false;
               inside[i]  = false;

               // test if new point is inside the raint given by a,b
			   int other = ((i+1)==len ? 0 : (i+1));
               Vector3d prod = ((Vector3d)corners.get(i)).cross((Vector3d)corners.get(1));
               double mul = prod.mul(v);
               if (mul > 0)	{
                         inside[i] = true;
                         polyTrackOutside = false;
               } else {
                         if(polyTrackOutside) replace[i] = true;
                         polyTrackOutside = true;
               }
            }
            if(polyTrackOutside && !inside[0])
               replace[0] = true;
                // now delete all corners that have the 'replace' flag set
                // If called from test11, seems to die in the loop. Only when len > 2 (GYF)
				int count = 0;
				boolean inserted = false;
				// count deleted ones as when we delete the the array is one less from
				// that point on true also from inside ...
                for (int i = 0 ; i < replace.length; i++) {
                    if(!inside[i]) {
                        corners.add(i-count+1,v);
						inserted =true;
						count--;
                    }
                    if(replace[i]) {
                        corners.remove(i - count);
                        count++;
                    }
            }
            // now find first corner that is not inside (there is only one)
            // and insert the point after that.
            // if all points are inside we did nothing...
			// commbined in loop above

            for(int i = 0; i < len; i++) {
                if(!inside[i]) {
					corners.add(i+1,v);
					break;
                }
            }

		}
	}

	/** may not have a convex if makeCOnvex was not called */
	public Convex getConvex() {
		if (conv == null) {
			try {
                makeConvex();
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
		return conv;
	}

}
/**********************************************************************
 * Revision History
 * ================
 *
 * $Log: Chull.java,v $
 * Revision 1.2  2003/02/19 15:46:11  womullan
 * Updated comments mainly to make java docs better
 *
 * Revision 1.1  2003/02/14 16:46:00  womullan
 *  Newly organised classes in the new packages
 *
 */
