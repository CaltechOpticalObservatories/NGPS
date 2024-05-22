package edu.jhu.htm.core;
import java.util.*;
import java.io.*;
/**********************************************************************

	A Domain manages a list of Convexes. This is the
	data structure that can define any area on the sphere. With the
	intersect method, the htm index returns the trixels that
	intersect with the area specified by the current Domain
	instance. There are two lists returned: one for the nodes fully
	contained in the area and one for the triangles which lie only
	partially in the domain.
<pre>
 Current Version
 ===============
 ID:	$Id: Domain.java,v 1.2 2003/02/19 15:46:11 womullan Exp $
 Revision: 	$Revision: 1.2 $
 Date/time:	$Date: 2003/02/19 15:46:11 $
</pre>

*/

public class Domain {


	List 	convexes_ = new ArrayList();

	protected int olevel = 20; // output level for the ranges
	/** Default constructor */
	public Domain() {
	}

	public void setOlevel(int lev) {
		olevel = lev;
		if (this.convexes_ != null && this.convexes_.size() > 0) {
			Iterator iter = convexes_.iterator();
			while(iter.hasNext()) {
				((Convex)iter.next()).setOlevel(olevel);
			}
		}
	}

	public int getOlevel() {
		return olevel;
	}


	/** Add a convex */
	public void add(Convex  c) {
		convexes_.add(c);
		c.setOlevel(olevel);
	}

	/**
	   Intersect with index. Return the range set  of the nodes that are
	   intersected by this domain. setting varlen true makes this
	   adaptive giving HTM ranges of different levels suiting the
	   shape of the area.
	*/
	public boolean intersect( HTMindexImp  idx,
				  HTMrange range , boolean varlen ) {
		int i;
		for(i = 0; i < convexes_.size(); i++) {  // intersect every convex
			getConvex(i).intersect(idx, range, varlen);
		}
		return true;
	}



  /** Fetch the convex number i */
	public Convex getConvex(int i) {
	return (Convex) convexes_.get(i);
	}

  /** Return the number of convexes in this domain */
	public int getNumberOfConvexes() {
	return convexes_.size();
	}

  /** Convert domain to a string. This string can be written into a
	  file and can recover the domain using the legacy parser.
  */
	public String toString() {
	String out = "#DOMAIN\n"+convexes_.size()+'\n';
	int i;
	for( i = 0; i < convexes_.size(); i++)
		out = out+getConvex(i).toString();
	return out;
	}

	/** Clear domain, empty convex list */
	public void clear() {
	convexes_.clear();
	}

	/* call simplify for all convexes */
	public Domain simplify() {
		for( int i = 0; i < convexes_.size(); i++) {
            getConvex(i).simplify();
		}
		return this;
	}

	/* See if any convex contains this point */
	public boolean contains(Vector3d p) {
		boolean ret = false;
		for( int i = 0; i < convexes_.size(); i++) {
            if (getConvex(i).contains(p)) return true;
		}
		return ret;
	}

	/** Add all convexces of given domain to this one - simple union */
	public void add(Domain d) {
        for( int i = 0; i < d.convexes_.size(); i++) {
			add(d.getConvex(i));
        }
	}

}
/**********************************************************************
* Revision History
* ================
*
* $Log: Domain.java,v $
* Revision 1.2  2003/02/19 15:46:11  womullan
* Updated comments mainly to make java docs better
*
* Revision 1.1  2003/02/14 16:46:00  womullan
*  Newly organised classes in the new packages
*
************************************************************************/
