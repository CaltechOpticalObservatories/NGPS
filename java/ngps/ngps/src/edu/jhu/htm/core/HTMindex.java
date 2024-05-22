package edu.jhu.htm.core;
/**********************************************************************
 *
 * The HTMindex is the main interface for the HTM implementation.
 * It provides the functionality called for in "Redesiging the HTM interface".
 * The HTMindexImp provides an implementation of this.
 * HTMfunc provides some of this funtionality albeit with different signatures.
 * HTMindexImp uses HTMfunc where appropriate haence answers from the two should
 * not differ.
<pre>
 Current Version
 ===============
 ID:	$Id: HTMindex.java,v 1.2 2003/02/19 15:46:11 womullan Exp $
 Revision: 	$Revision: 1.2 $
 Date/time:	$Date: 2003/02/19 15:46:11 $
</pre>
*
**/
public interface HTMindex {


	/** return a HTM name like N2121 for a given vector */
	public String lookup(Vector3d point)throws HTMException ;
	/** return a HTM name like N2121 for a given position */
	public String lookup(double ra, double dec)throws HTMException ;
	/** return a HTM id number for a given vector */
	public long lookupId(Vector3d point)throws HTMException ;
	/** return a HTM id number for a given position */
	public long lookupId(double ra, double dec)throws HTMException ;
	/** return center vector of the tringle which htmId relates to*/
	public Vector3d idToPoint(long htmId)throws HTMException ;
	/** return center vector of the tringle which htmName relates to*/
	public Vector3d idToPoint(String htmName)throws HTMException ;
	/** Convert Symbolic name to an id number*/
	public long nameToId(String  name)throws HTMException ;
	/** Convert id number to Symbolic name  */
	public String idToName(long  htmId)throws HTMException ;
	/** return a HTM id number for a given position */
	public Domain simplify(Domain d);
	/** give the area of the triangle secrad for HTM name given */
	public double area(String htmName);
	/** does the domain coaintain thie given point */
	public boolean contains (Domain d, Vector3d p);
	/** angular distance between the two ids */
	public double distance(long htmId1, long htmId2) throws HTMException;
	/** angular distance between the two names */
	public double distance(String htmName1, String htmName2) throws HTMException;
	/** Return all HTMs which fall inside the given domain */
	public HTMrange intersect(Domain d);
	/** Return a new domain contiainig common parts of the two domains*/
	public Domain intersection(Domain d1, Domain d2);
	/** Return a new domain contiainig all parts of the two domains*/
	public Domain union(Domain d1, Domain d2);
	/** Return a new domain which contains everything not in the given domain*/
	public Domain compliment(Domain d);
	/** Perform lossy simplification of the domain*/
	public Domain smooth(Domain d);

}
/**********************************************************************
* Revision History
* ================
*
* $Log: HTMindex.java,v $
* Revision 1.2  2003/02/19 15:46:11  womullan
* Updated comments mainly to make java docs better
*
* Revision 1.1  2003/02/14 16:46:00  womullan
*  Newly organised classes in the new packages
*
*
***/
