package edu.jhu.htm.core;
import java.util.*;
import java.io.*;
/**********************************************************************

	The Convex class encapsulates all the code for intersection
	and lookup. It represents a convex on the unit sphere (see Domain).

<pre>
 Current Version
 ===============
 ID:	$Id: Convex.java,v 1.3 2003/02/19 15:46:11 womullan Exp $
 Revision: 	$Revision: 1.3 $
 Date/time:	$Date: 2003/02/19 15:46:11 $
</pre>
 @version  	$Revision: 1.3 $
*/

public class  Convex  extends  Sign {

	List  constraints_ = new ArrayList(); // The vector of constraints
	List  corners_	 = new ArrayList(); // The corners of a zERO convex
	Constraint boundingCircle_;	  // For zERO convexes, the bc.
	protected int olevel = 20; // output level for the ranges

	/** Default Constructor */
	public Convex() {
	super();
	};

	/** Constructor from a triangle
	Initialize domain from a triangle. The corners of these vectors
	form a triangle, so we just add three ZERO convexes to the domain
	where the direction is given by the cross product of the corners.
	Of course, the sign has to be determined (on which side of the triangle
	are we?) If the three points lie on one line, no convexes are added.
	*/
	public Convex( Vector3d  v1,
			  Vector3d  v2,
			  Vector3d  v3) {
	super();
	Vector3d a1 = v2.cross(v3); // set directions of half-spheres
	Vector3d a2 = v3.cross(v1);
	Vector3d a3 = v1.cross(v2);
	double s1 = a1.mul(v1);	   // we really need only the signs of these
	double s2 = a2.mul(v2);
	double s3 = a3.mul(v3);

	if((s1 * s2 * s3) > 0) {		  // this is nonzero if not on one line
		if(s1 < 0.0) a1 = a1.mul(-1) ;  // change sign if necessary
		if(s2 < 0.0) a2 = a2.mul(-1) ;
		if(s3 < 0.0) a3 = a3.mul(-1) ;
		constraints_.add(new Constraint(a1,0.0));// we don't care
		constraints_.add(new Constraint(a2,0.0));// about the order
		constraints_.add(new Constraint(a3,0.0));// all angles > 90
	}
	sign_ = zERO;
	simplify();
	}


	/** Constructor from a rectangle
	Initialize convex from a rectangle. The vectors that form a rectangle
	may be in any order, the code finds the edges by itself.
	If one of the vectors lies within the triangle formed by the
	other three vectors, the previous constructor is used.
	*/
	public Convex( Vector3d  v1,
			  Vector3d  v2,
			  Vector3d  v3,
			  Vector3d  v4) {
	super();
	int i,j,k,l,m;  // indices
	// to simplify things, copy input into a 4-array
	Vector3d v[] = {v1,v2,v3,v4};
	Vector3d d[] = new Vector3d[6];
	double s[][]	  = new double[6][2];
	for (i = 0, k = 0; i < 4 ; i++)
		for (j = i+1; j < 4; j++, k++) {// set directions of half-spheres
		d[k] = v[i].cross(v[j]);	// two of these are diagonals.
		d[k].normalize();
		for (l = 0, m = 0; l < 4; l++) // set the 'sign'
			if(l != i && l != j)s[k][m++] = d[k].mul(v[l]);
		}

	// the sides are characterized by having both other corners
	// to the same (inner) side. so it is easy to find the edges.
	// again, the sign has to be taken care of -> direction of d
	// the nice thing here is that if one of the corners is inside
	// a triangles formed by the other three, only 3 constraints get
	// added.
	for(i = 0; i < 6; i++)
		if((s[i][0] * s[i][1]) > 0.0){ // not >= because we don't want aligned corners
		constraints_.add(new Constraint(((s[i][0] > 0.0 )?
							d[i] : (d[i].mul(-1))),
						   0.0));
	   }

	// Special cases: 1
	// if three of the corners are aligned, we end up with
	// only two constraints. Find the third and append it.
	// Indeed, there are 3 identical constraints among the d[],
	// so the first that qualifies gets appended.
	if(constraints_.size() == 2) {
		for(i = 0; i < 6; i++)
		if(s[i][0] == 0.0 || s[i][1] == 0.0) {
			constraints_.add(new Constraint(
						(((s[i][0]+s[i][1]) > 0.0) ?
						 d[i] : (d[i].mul(-1))),
								   0.0));
			break;
		}
	}
	// Special cases: 2
	// if all four corners are aligned, no constraints have been appended.
	sign_ = zERO;
	simplify();
	}


	/** Add a constraint. Multiple contrains make up a complex area on
	the sphere. After adding a ne constraint the constraints are
	ordered by ascending opening angle. i
	 */
	public void add(Constraint  c) {
	constraints_.add(c);

	//Since we append always at the end,
	//we only need one ordering sweep starting at the end
	for ( int i = constraints_.size() - 1; i > 0; i-- ) {
		Constraint ci = getConstraint(i);
		Constraint ch = getConstraint(i-1);
		if ( ci.s_ <  ch.s_ ) {
		constraints_.set(i,ch);
		constraints_.set(i-1,ci);
		}
	}

	if(constraints_.size() == 1) {  // first constraint
		sign_ = c.sign_;
		return;
	}

	switch (sign_) {
	case nEG:
		if(c.sign_ == pOS) sign_ = mIXED;
		break;
	case pOS:
		if(c.sign_ == nEG) sign_ = mIXED;
		break;
	case zERO:
		sign_ = c.sign_;
		break;
	}
	}


/**
   simplify0: simplify zERO convexes. calculate corners of convex
   and the bounding circle.

   zERO convexes are made up of constraints which are all great
   circles. It can happen that some of the constraints are redundant.
   For example, if 3 of the great circles define a triangle as the convex
   which lies fully inside the half sphere of the fourth constraint,
   that fourth constraint is redundant and will be removed.

   The algorithm is the following:

   zero-constraints are half-spheres, defined by a single normalized
   vector v, pointing in the direction of that half-sphere.

   Two zero-constraints intersect at

	i	=  +- v  x v
	 1,2		1	2

	 the vector cross product of their two defining vectors.

	 The two vectors i1,2 are tested against every other constraint in
	 the convex if they lie within their half-spheres. Those
	 intersections i which lie within every other constraint, are stored
	 into corners_.

	 Constraints that do not have a single corner on them, are dropped.
*/
	void simplify0() {
	if (HTMfunc.verbose )
		System.out.println("Simplif0 : "+constraints_.size());

	if(constraints_.size() == 1) {
		boundingCircle_ = getConstraint(0);//for one constraint, BC is self
		return;
	// For 2 constraints, take the bounding circle a 0-constraint...
	// this is by no means optimal, but the code is optimized for at least
	// 3 zERO constraints... so this is acceptable.
	} else if(constraints_.size() == 2) {
		// test for constraints being identical - rule 1 out
		if( ((Constraint)constraints_.get(0)).v().equal(
		((Constraint)constraints_.get(1)).v())){
		  constraints_.remove(1);
		  boundingCircle_ = (Constraint)constraints_.get(0);
		  return;
		}
		// test for constraints being two disjoint half spheres - empty convex!
		if(((Constraint)constraints_.get(0)).v().equal(
			   ((Constraint)constraints_.get(1)).v().mul(-1.0))) {
		constraints_.remove(0);
		constraints_.remove(1);
		return;
		}
		Vector3d v = new Vector3d(getConstraint(0).v().add(getConstraint(1).v()));
		v.normalize();
		if (HTMfunc.verbose )
		  System.out.println("2Constr bounding circle: "+v.toString());

		boundingCircle_ = new Constraint(v,0);
	}

	int i,j,k;
	Vector3d vi1=null, vi2=null;
	List cornerConstr1, cornerConstr2, removeConstr, corner;
	cornerConstr1 = new ArrayList();
	cornerConstr2 = new ArrayList();
	removeConstr = new ArrayList();
	corner = new ArrayList();

	// Go over all pairs of constraints
	for(i = 0; i < constraints_.size() - 1; i++) {
		boolean ruledout = true;
		for(j = i+1; j < constraints_.size(); j++) {
		// vi1 and vi2 are their intersection points
		vi1 = (getConstraint(i).a_).cross((getConstraint(j)).a_) ;
		if(vi1.length() == 0)break; // i and j are the same constraint.
		vi1.normalize();
		vi2 = vi1.mul(-1);
		boolean vi1ok =true , vi2ok = true;
		// now test whether vi1 or vi2 or both are
		// inside every other constraint.
		// if yes, store them in the corner array.
		for(k = 0; k < constraints_.size(); k++) {
			if(k == i || k == j) continue;
			Constraint ck = getConstraint(k);
			if(vi1ok && (vi1.mul(ck.a_)) <= 0 ) vi1ok = false;
			if(vi2ok && (vi2.mul(ck.a_)) <= 0 ) vi2ok = false;
			if(!vi1ok && !vi2ok)break;
		}
		if(vi1ok) {
			corner.add(vi1);
			cornerConstr1.add(Integer.valueOf(i));
			cornerConstr2.add(Integer.valueOf(j));
			ruledout = false;
		}
		if(vi2ok) {
			corner.add(vi2);
			cornerConstr1.add(Integer.valueOf(i));
			cornerConstr2.add(Integer.valueOf(j));
			ruledout = false;
		}
		}
		// is this constraint ruled out? i.e. none of its intersections
		// with other constraints are corners...
		//remove it from constraints_ list.
		if(ruledout) removeConstr.add(Integer.valueOf(i));
	}
	// Now set the corners into their correct order, which is an
	// anti-clockwise walk around the polygon.
	//
	// start at any corner. so take the first.

	corners_.clear();
	corners_.add(corner.get(0));

	// The trick is now to start off into the correct direction.
	// this corner has two edges it can walk. we have to take the
	// one where the convex lies on its left side.

	// the i'th constraint and j'th constraint;
	// intersect at 0'th corner
	i = ((Integer)cornerConstr1.get(0)).intValue();
	j = ((Integer)cornerConstr2.get(0)).intValue();

	int c1=0,c2=0,k1=0,k2=0;
	// Now find the other corner where the i'th and j'th
	// constraints intersect.  Store the corner in vi1 and vi2,
	// and the other constraint indices in c1,c2.
	for( k = 1; k < cornerConstr1.size(); k ++) {
		if(((Integer)cornerConstr1.get(k)).intValue() == i) {
		vi1 = (Vector3d)corner.get(k);
		c1 = ((Integer)cornerConstr2.get(k)).intValue();
		k1 = k;
		}
		if(((Integer)cornerConstr2.get(k)).intValue() == i) {
		vi1 = (Vector3d)corner.get(k);
		c1 = ((Integer)cornerConstr1.get(k)).intValue();
		k1 = k;
		}
		if(((Integer)cornerConstr1.get(k)).intValue() == j) {
		vi2 = (Vector3d)corner.get(k);
		c2 = ((Integer)cornerConstr2.get(k)).intValue();
		k2 = k;
		}
		if(((Integer)cornerConstr2.get(k)).intValue() == j) {
		vi2 = (Vector3d)corner.get(k);
		c2 = ((Integer)cornerConstr1.get(k)).intValue();
		k2 = k;
		}
	}
	// Now test i'th constraint-edge ( corner 0 and corner k ) whether
	// it is on the correct side (left)
	//
	//  ( (corner(k) - corner(0)) x constraint(i) )  corner(0)
	//
	// is >0 if yes, <0 if no...
	//
	 int c,currentCorner;
	 if((vi1.sub((Vector3d)corner.get(0)).cross(
			getConstraint(i).a_)).mul(
			(Vector3d)corner.get(0)) > 0 ) {
		 corners_.add(vi1);
		 c = c1;
		 currentCorner = k1;
	 } else {
		 corners_.add(vi2);
		 c = c2;
		 currentCorner = k2;
	 }
	 // now append the corners that match the index c until we got
	 // corner 0 again currentCorner holds the current corners
	 // index c holds the index of the constraint that has just
	 // been intersected with
	 // So:
	 // x We are on a constraint now (i or j from before), the
	 //   second corner is the one intersecting with constraint c.
	 // x Find other corner for constraint c.
	 // x Save that corner, and set c to the constraint that
	 //   intersects with c at that corner. Set currentcorner to
	 //   that corners index.
	 // x Loop until 0th corner reached.
	 while( currentCorner != 0 ) {
		 for (k = 0; k < cornerConstr1.size(); k++) {
		 if(k == currentCorner) continue;
		 if(((Integer)cornerConstr1.get(k)).intValue() == c ) {
			 if( (currentCorner = k) ==0 ) break;
			 corners_.add(corner.get(k));
			 c =((Integer)cornerConstr2.get(k)).intValue();
			 break;
		 }
		 if(((Integer)cornerConstr2.get(k)).intValue() == c ) {
			 if( (currentCorner = k) ==0 ) break;
			 corners_.add(corner.get(k));
			 c = ((Integer)cornerConstr1.get(k)).intValue();
			 break;
		 }
		 }
	 }
	 // Remove all redundant constraints
	 for ( i = 0; i < removeConstr.size(); i++)
		constraints_.remove(((Integer)removeConstr.get(i)).intValue());

	 // Now calculate the bounding circle for the convex.
	 // We take it as the bounding circle of the triangle with
	 // the widest opening angle. All triangles made out of 3 corners
	 // are considered.
	 if (constraints_.size() >=3 ) {
	   boundingCircle_ = new Constraint(1.0);
	   for(i = 0; i < corners_.size(); i++)
		 for(j = i+1; j < corners_.size(); j++)
		   for(k = j+1; k < corners_.size(); k++) {
		 Vector3d v = ( getCorner(j).sub(
						  getCorner(i))).cross(
					   ( getCorner(k).sub(getCorner(j))) );
		 v.normalize();
		 // Set the correct opening angle: Since the plane cutting
		 // out the triangle also correctly cuts out the bounding cap
		 // of the triangle on the sphere, we can take any corner to
		 // calculate the opening angle
		 double d = v.mul(getCorner(i));
		 if(boundingCircle_.d_ > d)
		   boundingCircle_ = new Constraint(v,d);
		   }
	 }
	}


/**
   simplify: We have the following decision tree for the
			 simplification of convexes:

  Always test two constraints against each other. We have
  * If both constraints are pOS
	  If they intersect: keep both
	  If one lies in the other: drop the larger one
	  Else: disjunct. Empty convex, stop.

  * If both constraints are nEG
	  If they intersect or are disjunct: ok
	  Else: one lies in the other, drop smaller 'hole'

  * Mixed: one pOS, one nEG
	  No intersection, disjunct: pOS is redundant
	  Intersection: keep both
	  pOS within nEG: empty convex, stop.
	  nEG within pOS: keep both.

*/
	public void simplify() {

	if(sign_ == zERO ) {
	  simplify0();		// treat zERO convexes separately
	  if (HTMfunc.verbose )
		System.out.println(" simplify0 ");
	  return;
	}

	int i,j;
	int clen;
	boolean redundancy = true;
	while(redundancy) {
		redundancy = false;
		clen = constraints_.size();

	for(i = 0; i < clen; i++) {
	  for(j = 0; j < i; j++) {
		int test;
		Constraint ci = getConstraint(i);
		Constraint cj = getConstraint(j);
		// don't bother with two zero constraints
		if( ci.sign_ == zERO && cj.sign_ == zERO)
		  continue;

		// both pos or zero
		if( ( ci.sign_ == pOS
		  || ci.sign_ == zERO ) &&
		( cj.sign_ == pOS
		  || cj.sign_ == zERO ) ) {
		// intersection?
		if ( (test = testConstraints(i,j)) == 0 ) continue;
		if ( test < 0 ) { // disjoint ! convex is empty
			constraints_.clear();
			return;
		}
		// one is redundant
		if(test == 1)   constraints_.remove(j);
		else if(test==2)constraints_.remove(i);
		else continue;	 // intersection
		// we did cut out a constraint -> do the loop again
		redundancy = true;
		break;
		}

		// both neg or zero
		if( ( ci.sign_ == nEG ) &&
		( cj.sign_ == nEG ) ) {
		  if ( (test = testConstraints(i,j)) <= 0 ) continue; // ok
		  // one is redundant
		  if(test == 1)   constraints_.remove(j);
		  else if(test==2)constraints_.remove(i);
		  else continue; // intersection
		  // we did cut out a constraint -> do the loop again
		  redundancy = true;
		  break;
		}

		// one neg, one pos/zero
		if( (test = testConstraints(i,j)) == 0) continue; // ok: intersect
		if( test < 0 ) { // neg is redundant
		if ( ci.sign_ == nEG )
			constraints_.remove(i);
		else constraints_.remove(j);
		// we did cut out a constraint -> do the loop again
		redundancy = true;
		break;
		}
		// if the negative constraint is inside the positive: continue
		if ( (ci.sign_ == nEG && test == 2) ||
		 (cj.sign_ == nEG && test == 1) )continue;
		// positive constraint in negative: convex is empty!
		constraints_.clear();
		return;
	  } // loop over j
	  if(redundancy)break;
	} // loop over i

	} // while redundancy loop

	// reset the sign of the convex
	sign_ = getConstraint(0).sign_;
	for(i = 1; i < constraints_.size(); i++) {
	  Constraint ci = getConstraint(i);
	  switch (sign_) {
	  case nEG:
		if(ci.sign_ == pOS) sign_ = mIXED;
		break;
	  case pOS:
		if(ci.sign_ == nEG) sign_ = mIXED;
		break;
	  case zERO:
		sign_ = ci.sign_;
		break;
	  }
	}
   }

/**
   Test for constraint relative position; intersect, one in the
   other, disjoint.
   testConstraints: Test for the relative position of two constraints.
				  Returns 0  if they intersect
				  Returns -1 if they are disjoint
				  Returns 1  if j is in i
				  Returns 2  if i is in j

*/
	int testConstraints(int i, int j) {
	Constraint ci = getConstraint(i);
	Constraint cj = getConstraint(j);
	double phi = (
		   ((ci.sign_ == nEG) ? (ci.a_.mul(-1)) : ci.a_ ).mul(
		   ((cj.sign_ == nEG) ? (cj.a_.mul(-1)) : cj.a_ ))
		   );
	phi = ((phi <= -1.0 + HTMfunc.gEpsilon) ? HTMfunc.Pi : Math.acos(phi));
	double a1 = (ci.sign_ == pOS ?
			 ci.s_ : HTMfunc.Pi - ci.s_);
	double a2 = (cj.sign_ == pOS ?
			 cj.s_ : HTMfunc.Pi - cj.s_);

	if ( phi > a1 + a2 ) return -1;
	if ( a1 > phi + a2 ) return 1;
	if ( a2 > phi + a1 ) return 2;
	return 0;
	}


	/**
	   Intersect with index. The HTMrange for the
	   result must be given. Specifying varlen = true will use adaptive
	   depth HTM for the result so ranges will be in the resolution best suited
	   to the shape of the convex.
	*/
	public void intersect( HTMindexImp  idx, HTMrange  htmrange, boolean varlen) {
		if (constraints_.size() == 0) return ; // nothing to do
			for (int i=1 ; i <=8 ; i++) {
			testTrixel(i,idx, htmrange,varlen);
		}
	}

	/** adds the given trixel tot he HTMrange */
	protected void saveTrixel(long htmid, HTMrange hr, boolean varlen)
	{

	  // Some application want a complete htmid20 range

	  int level, i, shifts;
	  long lo, hi;
	  if(varlen){
		hr.mergeRange(htmid, htmid);
		return;
	  }

	  for(i = 0; i < HTMfunc.IDSIZE; i+=2) {
		if ( ((htmid << i) & HTMfunc.IDHIGHBIT) != 0 ) break;
	  }

	  level = (HTMfunc.IDSIZE-i) >>> 1;
	  level -= 2;
	  if (level < olevel){
		/* Size is the length of the string representing the name of the
		   trixel, the level is size - 2
		**/
		shifts = (olevel - level) << 1;
		lo = htmid << shifts;
		hi = lo + ((long) 1 << shifts) -1;
	  } else {
		lo = hi = htmid;
	  }
	  hr.mergeRange(lo, hi);

	  return;
	}

 ///////////Trixeltest/////////////////////////////////
	/**
	   TestTrixel: this is the main test of a trixel vs a Convex.
	*/
   short testTrixel(int id, HTMindexImp idx, HTMrange range, boolean varlen) {
	 short mark;
	 int childID;
	 long tid;

	QuadNode indexNode = (QuadNode)idx.nodes_.get(id);

	 // do the face test on the triangle
	 mark = testNode(id,idx);

	 switch(mark){
	 case Markup.fULL:
	   tid = indexNode.id_;
	   saveTrixel(tid,range,varlen);
	   return mark;
	 case Markup.rEJECT:
	   tid = indexNode.id_;
	   return mark;
	 default:
	   break;
	 }


	   childID = indexNode.childID_[0];
	   if ( childID != 0){
		 ////////////// [inted:split]
		 tid = indexNode.id_;
		 childID = indexNode.childID_[0];  testTrixel(childID,idx,range,varlen);
		 childID = indexNode.childID_[1];  testTrixel(childID,idx,range,varlen);
		 childID = indexNode.childID_[2];  testTrixel(childID,idx,range,varlen);
		 childID = indexNode.childID_[3];  testTrixel(childID,idx,range,varlen);
	   } else { /// No children...
		 if (idx.addlevel_ > 0){
		   testPartial(idx.addlevel_, indexNode.id_,
					   idx.getVertex(indexNode.v_[0]),
					   idx.getVertex(indexNode.v_[1]),
					   idx.getVertex(indexNode.v_[2]),
					   0, idx,range, varlen);
		 } else {
		   saveTrixel(indexNode.id_,range,varlen);
		 }
	   }



	 /* NEW NEW NEW
		If rejected, then we return [done]
		If full, then we list the id (propagate to children) [done]

		If partial, then we look ahead to see how many children are rejected.
		But ah, next iteration could benefit from having computed this already.

		If two chidlren are rejected, then we stop
		If one or 0 nodes are rejected, then we
	 **/
	 return mark;
   }


	/**
	   test a triangle's subtriangles whether they are partial.
	   If level is nonzero, recurse:
	*/
void
 testPartial(int level, long id,
			   Vector3d v0,
			   Vector3d v1,
			 Vector3d v2, int PPrev, HTMindexImp idx, HTMrange range, boolean varlen)
{
  long ids[] = new long[4];
  long id0;
  short m[] = new short[4];
  int P=0, F=0;// count number of partials and fulls

  Vector3d w0 = v1.add(v2); w0.normalize();
  Vector3d w1 = v0.add(v2); w1.normalize();
  Vector3d w2 = v1.add(v0); w2.normalize();

  ids[0] = id0 = id << 2;
  ids[1] = id0 + 1;
  ids[2] = id0 + 2;
  ids[3] = id0 + 3;

  m[0] = testNode(v0, w2, w1);
  m[1] = testNode(v1, w0, w2);
  m[2] = testNode(v2, w1, w0);
  m[3] = testNode(w0, w1, w2);

	for (int i = 0 ; i < 4 ; i++) {
		if ( m[i] == Markup.fULL ) F++;
		if ( m[i] == Markup.pARTIAL ) P++;
	}


  // Several interesting cases for saving this (the parent) trixel.
  // Case P==4, all four children are partials, so pretend parent is full, we save and return
  // Case P==3, and F==1, most of the parent is in, so pretend that parent is full again
  // Case P==2 or 3, but the previous testPartial had three partials, so parent was in an arc
  // as opposed to previous partials being fewer, so parent was in a tiny corner...

  if ((level-- <= 0) || ((P == 4) || (F >= 2) || (P == 3 && F == 1) || (P > 1 && PPrev == 3))){
	saveTrixel(id,range,varlen);
	return;
  } else {
	// look at each child, see if some need saving;
	for(int i=0; i<4; i++){
	  if (m[i] == Markup.fULL){
	saveTrixel(ids[i],range,varlen);
	  }
	}
	// look at the four kids again, for partials

	if (m[0] == Markup.pARTIAL) testPartial(level, ids[0], v0, w2, w1, P,idx,range,varlen);
	if (m[1] == Markup.pARTIAL) testPartial(level, ids[1], v1, w0, w2, P,idx,range,varlen);
	if (m[2] == Markup.pARTIAL) testPartial(level, ids[2], v2, w1, w0, P,idx,range,varlen);
	if (m[3] == Markup.pARTIAL) testPartial(level, ids[3], w0, w1, w2, P,idx,range,varlen);
  }
}

	/**
	   the same routine as above, but for a given saved node
	*/
	short testNode(int nodeIndex, HTMindexImp idx) {
	// Start with testing the vertices for the QuadNode with this convex.
	QuadNode N = idx.getNode(nodeIndex);

	Vector3d V0 = idx.getVertex(N.v_[0]);
	Vector3d V1 = idx.getVertex(N.v_[1]);
	Vector3d V2 = idx.getVertex(N.v_[2]);
	int vsum = testVertex(V0) +
			   testVertex(V1) +
			   testVertex(V2);
	if (HTMfunc.verbose ) {
		try {
		System.out.println(HTMfunc.idToName(idx.getNode(nodeIndex).id_));
		} catch (Exception e) {};
	}
	short mark = testTriangle( V0, V1,V2 ,vsum);

	// If we are down at the leaf nodes here, it is dONTKNOW,
	// really.. but since these are the leaf nodes here and we want to
	// be on the safe side, mark them as partial.
	if ( (N.childID_[0] == 0 )&& (mark == Markup.dONTKNOW))
		mark = Markup.pARTIAL;

	if (HTMfunc.verbose ) {
		System.out.println(" Mark = "+printMark(mark));
		System.out.println(V0.toString()+" ; "+V1.toString()+" ; "+V2.toString());
	}
	return mark;
   }
	   /**
	test each QuadNode for intersections. Calls testTriangle after having
	tested the vertices using testVertex.
	 */
	 short testNode( Vector3d  v0,
			 Vector3d  v1,
			 Vector3d  v2) {
		 // Start with testing the vertices for the QuadNode with this convex.
		 int vsum = testVertex(v0) + testVertex(v1) + testVertex(v2);
		 short mark = testTriangle( v0, v1, v2, vsum);

		 // since we cannot play games using the on-the-fly triangles,
		 // substitute dontknow with partial.
		 if ( mark == Markup.dONTKNOW)
			 mark = Markup.pARTIAL;
		 return mark;
	 }

	/**
	   testTriangle: tests a triangle given by 3 vertices if
	   it intersects the convex. Here the whole logic of deciding
	   whether it is partial, full, swallowed or unknown is handled.
	*/
	short testTriangle( Vector3d  v0,
			Vector3d  v1,
			Vector3d  v2,
			int vsum) {

	if(vsum == 1 || vsum == 2) return Markup.pARTIAL;

	// If vsum = 3 then we have all vertices inside the convex.
	// Now use the following decision tree:
	//
	// * If the sign of the convex is pOS or zERO : mark as fULL
	//   intersection.
	//
	// * Else, test for holes inside the triangle. A 'hole' is a
	//   nEG constraint that has its center inside the triangle. If
	//   there is such a hole, return pARTIAL intersection.
	//
	// * Else (no holes, sign nEG or mIXED) test for intersection of nEG
	//   constraints with the edges of the triangle. If there are such,
	//   return pARTIAL intersection.
	//
	// * Else return fULL intersection.

	if(vsum == 3) {
		if(sign_ == pOS || sign_ == zERO)
		return Markup.fULL;
		if ( testHole(v0,v1,v2) ) return Markup.pARTIAL;
		if ( testEdge(v0,v1,v2) ) return Markup.pARTIAL;
		return Markup.fULL;
	}

	// If we have reached that far, we have vsum=0. There is no
	// definite decision making possible here with our methods,
	// the markup may result in dONTKNOW. The decision tree is the
	// following:
	//
	// * Test with bounding circle of the triangle.
	//
	//   # If the sign of the convex zERO test with the precalculated
	//	 bounding circle of the convex. If it does not intersect with the
	//	 triangle's bounding circle, bREJECT.
	//
	//   # If the sign of the convex is nonZERO: if the bounding circle
	//	 lies outside of one of the constraints, bREJECT.
	//
	// * Else: there was an intersection with the bounding circle.
	//
	//   # For zERO convexes, test whether the convex intersects the edges.
	//	 If none of the edges of the convex intersects with the edges of
	//	 the triangle, we have a rEJECT. Else, pARTIAL.
	//
	//   # If sign of convex is pOS, or miXED and the smallest
	//	 constraint does not intersect the edges and has its
	//	 center inside the triangle, return sWALLOW. If no
	//	 intersection of edges and center outside triangle, return
	//	 rEJECT.
	//
	//   # So the smallest constraint DOES intersect with the edges. If
	//	 there is another pOS constraint which does not intersect with
	//	 the edges, and has its center outside the triangle, return
	//	 rEJECT. If its center is inside the triangle return sWALLOW.
	//	 Else, return pARTIAL for pOS and dONTKNOW for mIXED signs.
	//
	// * If we are here, return dONTKNOW. There is an intersection
	//   with the bounding circle, none of the vertices is inside
	//   the convex and we have very strange possibilities left for
	//   pOS and mIXED signs. For nEG, i.e. all constraints
	//   negative, we also have some complicated things left for
	//   which we cannot test further.

	if (HTMfunc.verbose )
		System.out.println(" vsum = 0 ");
	if ( !testBoundingCircle(v0,v1,v2) ) return Markup.rEJECT;

	if ((sign_==pOS) || (sign_==mIXED) || (sign_ == zERO && constraints_.size() == 2)) {
		// Does the smallest constraint intersect with the edges?
		if ( testEdgeConstraint(v0,v1,v2,0) ) {
		// Is there another positive constraint that does NOT
		// intersect with the edges?
		int cIndex = testOtherPosNone(v0,v1,v2);
		if ( cIndex > 0 ) {
			// Does that constraint lie inside or outside of
			// the triangle?
			if ( testConstraintInside(v0,v1,v2, cIndex) )
			return Markup.pARTIAL;
			// Does the triangle lie completely within that constr?
			else if( getConstraint(cIndex).contains(v0) )
			return Markup.pARTIAL;
			else  return Markup.rEJECT;

		} else {
			if(sign_ == pOS || sign_ == zERO)
			  return Markup.pARTIAL;
			else  return Markup.dONTKNOW;
		}
		} else {
		if (sign_ ==  pOS || sign_ == zERO) {
			// Does the smallest lie inside or outside the triangle?
			if( testConstraintInside(v0,v1,v2, 0) )
			return Markup.pARTIAL;
			else return Markup.rEJECT;
		}   else return Markup.dONTKNOW;
		}
	} else if (sign_ == zERO) {
		if ( corners_.size() > 0 && testEdge0(v0,v1,v2) )
		  return Markup.pARTIAL;
		else return Markup.rEJECT;
	}
	return Markup.pARTIAL;
	}


	/** Test if vertices are inside the convex  */
	int testVertex( Vector3d  v) {
		for ( int i = 0; i < constraints_.size(); i++) {
			Constraint ci = getConstraint(i);
			if ( (ci.a_.mul(v) )  < ci.d_ )
			return 0;
		}
		return 1;
	}

	/**
	   testHole : look for 'holes', i.e. negative constraints that have their
	   centers inside the node with the three corners v0,v1,v2.
	*/
	boolean testHole( Vector3d  v0,
			  Vector3d  v1,
			  Vector3d  v2) {

		boolean test = false;

		for(int i = 0; i < constraints_.size(); i++) {
			Constraint ci = getConstraint(i);
			if ( ci.sign_ == nEG ) {  // test only 'holes'

		  // If (a ^ b * c) < 0, vectors abc point clockwise.
		  // -> center c not inside triangle, since vertices a,b are ordered
		  // counter-clockwise. The comparison here is the other way
		  // round because c points into the opposite direction as the hole

			if (((v0.cross(v1)).mul(ci.a_)) > 0.0 ) continue;
			if (((v1.cross(v2)).mul(ci.a_)) > 0.0 ) continue;
			if (((v2.cross(v0)).mul(ci.a_)) > 0.0 ) continue;
			test = true;
			break;
			}
		}
		return test;
	}


  /** testEdge0: test the edges of the triangle against the edges of the
	  zERO convex. The convex is stored in corners_ so that the convex
	  is always on the left-hand-side of an edge corners_(i) - corners_(i+1).
	  (just like the triangles). This makes testing for intersections with
	  the edges easy. */
	boolean testEdge0( Vector3d  v0,
			   Vector3d  v1,
			   Vector3d  v2) {
	// We have constructed the corners_ array in a certain direction.
	// now we can run around the convex, check each side against the 3
	// triangle edges. If any of the sides has its intersection INSIDE
	// the side, return true. At the end test if a corner lies inside
	// (because if we get there, none of the edges intersect, but it
	// can be that the convex is fully inside the triangle. so to test
	// one single edge is enough)

	class edgeStruct {
		Vector3d e;			// The half-sphere this edge delimits
		double		l;			// length of edge
		Vector3d e1; 		// first end
		Vector3d e2; 		// second end
	} ;

	edgeStruct[] edge = new edgeStruct[3];
	for (int i = 0 ; i< 3 ; i++)  edge[i] = new edgeStruct();

	// fill the edge structure for each side of this triangle
	edge[0].e = v0.cross(v1); edge[0].e1 = v0; edge[0].e2 = v1;
	edge[1].e = v1.cross(v2); edge[1].e1 = v1; edge[1].e2 = v2;
	edge[2].e = v2.cross(v0); edge[2].e1 = v2; edge[2].e2 = v0;
	edge[0].l = Math.acos(v0.mul(v1));
	edge[1].l = Math.acos(v1.mul(v2));
	edge[2].l = Math.acos(v2.mul(v0));

	for(int i = 0; i < corners_.size(); i++) {
		int j = 0;
		if(i < corners_.size() - 1) j = i+1;
		Vector3d a1;
		// lengths of the arcs from intersection to edge corners
		double l1,l2;
		Vector3d ci = getCorner(i);
		Vector3d cj = getCorner(j);
		double cedgelen = Math.acos(ci.mul(cj));// length of edge of convex
		if (HTMfunc.verbose )
		  System.out.println(" corners =  "+corners_.size());
		// calculate the intersection - all 3 edges
		for (int iedge = 0; iedge < 3; iedge++) {
		a1 =  edge[iedge].e.cross( ci.cross(cj));
		a1.normalize();
		// if the intersection a1 is inside the edge of the convex,
		// its distance to the corners is smaller than the edgelength.
		// this test has to be done for both the edge of the convex and
		// the edge of the triangle.
		for(int k = 0; k < 2; k++) {
			l1 = Math.acos(ci.mul(a1));
			l2 = Math.acos(cj.mul(a1));
			if( l1 - cedgelen <= HTMfunc.gEpsilon &&
			l2 - cedgelen <= HTMfunc.gEpsilon) {
			l1 = Math.acos( edge[iedge].e1.mul(a1) );
			l2 = Math.acos( edge[iedge].e2.mul(a1) );
			if( (l1 - edge[iedge].l <= HTMfunc.gEpsilon) &&
				(l2 - edge[iedge].l <= HTMfunc.gEpsilon) )
				return true;
			}
			a1 = a1.mul(-1.0);// do the same for the other intersection
		}
		}
	}
	return testVectorInside(v0,v1,v2,getCorner(0));
	}

	/**
	  testEdge: look whether one of the constraints intersects with one of
	  the edges of node with the corners v0,v1,v2.
	*/
	boolean testEdge( Vector3d  v0,
			  Vector3d  v1,
			  Vector3d  v2) {
	for(int i = 0; i < constraints_.size(); i++) {
		// test only 'holes'
		if ( getConstraint(i).sign_ == nEG ) {
		if ( eSolve(v0, v1, i) ) return true;
		if ( eSolve(v1, v2, i) ) return true;
		if ( eSolve(v2, v0, i) ) return true;
		}
	}
	return false;
	}

	/**
	   eSolve: solve the quadratic equation for the edge v1,v2 of
	   constraint[cIndex]
	*/
	boolean eSolve( Vector3d  v1,
			Vector3d  v2, int cIndex) {
	Constraint con = getConstraint(cIndex);
	double gamma1 = v1.mul( con.a_ );
	double gamma2 = v2.mul( con.a_ );
	double mu	 = v1.mul( v2);
	double u2	 = (1 - mu) / (1 + mu);

	double a	  = - u2 * (gamma1 + con.d_);
	double b	  = gamma1 * ( u2 - 1 ) + gamma2 * ( u2 + 1 );
	double c	  = gamma1 - con.d_;

	double D	  = b * b - 4 * a * c;

	if( D < 0.0 ) return false; // no intersection

	// calculate roots a'la Numerical Recipes

	double q = -0.5 * ( b + ( (b<0 ? -1:(b>0 ? 1:0)) * Math.sqrt(D) ) );

	double root1=-1, root2=-1;
	int i = 0;

	if ( a > HTMfunc.gEpsilon || a < -HTMfunc.gEpsilon ) { root1 = q / a; i++; }
	if ( q > HTMfunc.gEpsilon || q < -HTMfunc.gEpsilon ) { root2 = c / q; i++; }

	// Check whether the roots lie within [0,1]. If not, the intersection
	// is outside the edge.

	if (i == 0) return false; // no solution
	if ( (root1 >= 0.0) && (root1 <= 1.0) ) return true;
	if ( i == 2 && ( ((root1 >= 0.0) && (root1 <= 1.0) ) ||
			 ((root2 >= 0.0) && (root2 <= 1.0) ) ) ) return true;

	return false;
	}


	/** Test if bounding circle intersects with a constraint */
	boolean testBoundingCircle( Vector3d  v0,
				Vector3d  v1,
				Vector3d  v2) {
	// Set the correct direction: The normal vector to the triangle plane
	Vector3d c =  (v1.sub(v0)).cross( v2.sub(v1));
	c.normalize();

	// Set the correct opening angle: Since the plane cutting out
	// the triangle also correctly cuts out the bounding cap of
	// the triangle on the sphere, we can take any corner to
	// calculate the opening angle
	double d = Math.acos (c.mul(v0));

	// for zero convexes, we have calculated a bounding circle for
	// the convex.  only test with this Math.single bounding
	// circle.

	if(sign_ == zERO) {
		double tst = c.mul(boundingCircle_.a_);
		if ( ((tst<-1.0 + HTMfunc.gEpsilon) ? HTMfunc.Pi:Math.acos(tst)) >
		 ( d + boundingCircle_.s_) ) return false;
		return true;
	}

	// for all other convexes, test every constraint. If the bounding
	// circle lies completely outside of one of the constraints, reject.
	// else, accept.

	int i;
	for(i = 0; i < constraints_.size(); i++) {
		Constraint ci = getConstraint(i);
		double cci = c.mul(ci.a_);
		if ( ((cci<-1.0 + HTMfunc.gEpsilon) ? HTMfunc.Pi : Math.acos(cci)) >
		 ( d + ci.s_ ) ) return false;
	}
	return true;
   }

	/** Test if a constraint intersects the edges */
	boolean testEdgeConstraint( Vector3d  v0,
				Vector3d  v1,
				Vector3d  v2,
				int cIndex) {
	if ( eSolve(v0, v1, cIndex) ) return true;
	if ( eSolve(v1, v2, cIndex) ) return true;
	if ( eSolve(v2, v0, cIndex) ) return true;
	return false;
	}


	/** Look for any positive constraint that does not intersect the edges */
	int testOtherPosNone( Vector3d  v0,
			  Vector3d  v1,
			  Vector3d  v2) {
	int i = 1;
	if (constraints_.size() > 1) {
		while ( (i < constraints_.size()) &&
			(getConstraint(i).sign_ == pOS) ) {
		if ( !testEdgeConstraint ( v0,v1,v2, i ) ) return i;
		i++;
		}
	}
	return 0;
	}


	/** Test for a constraint lying inside or outside of triangle */
	boolean testConstraintInside( Vector3d  v0,
				  Vector3d  v1,
				  Vector3d  v2,
				  int cIndex) {
	return testVectorInside(v0,v1,v2, getConstraint(cIndex).a_);
	}


	/** Test for a vector lying inside or outside of triangle
	For vectors abc, if (a ^ b * c) < 0, abc are ordered clockwise.
	-> center c not inside triangle, since vertices are ordered
	counter-clockwise.
	*/
	boolean testVectorInside( Vector3d  v0,
				  Vector3d  v1,
				  Vector3d  v2,
				  Vector3d  v) {

	if( ( (v0.cross(v1)).mul(v) < 0 ) ||
		( (v1.cross(v2)).mul(v) < 0 ) ||
		( (v2.cross(v0)).mul(v) < 0 ) )
		return false;
	return true;
	}

	public Constraint getConstraint (int c) {
	return (Constraint)constraints_.get(c);
	}

	public int getNumberOfConstraints () {
	return constraints_.size();
	}

	public Vector3d getCorner (int c) {
	return (Vector3d)corners_.get(c);
	}

	int NC(HTMindexImp idx,int n, int m) {
 	return idx.getNode(n).childID_[m];
	}

	public String toString() {
	String out = "#CONVEX\n"+constraints_.size()+" "+printSign()+'\n';
	int i;
	for( i = 0; i < constraints_.size(); i++)
		out = out+getConstraint(i).toString()+'\n';
	return out;
	}


	//* print markup */
	public static String printMark(short mark) {

	switch (mark) {
		case  0:
	return " dONTKNOW";
		case  1:
	return " pARTIAL";
		case  2:
	return " fULL";
		case  3:
	return " rEJECT";
	}
	return "";
	}

	public void setOlevel(int lev) {
		olevel = lev;
	}

	public int getOlevel() {
		return olevel;
	}

	public boolean contains(Vector3d p) {
		boolean ret = false;
		for (int c=0; c < constraints_.size(); c++) {
			if ( getConstraint(c).contains(p) ) {
				return true;
			}
		}
		return ret;
	}
};


/**
*/

/**********************************************************************
* Revision History
* ================
*
* $Log: Convex.java,v $
* Revision 1.3  2003/02/19 15:46:11  womullan
* Updated comments mainly to make java docs better
*
* Revision 1.2  2003/02/14 16:46:00  womullan
*  Newly organised classes in the new packages
*
* Revision 1.1  2003/02/11 18:40:27  womullan
*  moving to new packages and names
*
* Revision 1.3  2003/02/05 23:00:07  womullan
* Added new timimig routines and HTM clas to do cc stuff from the c++ package
*
* Revision 1.2  2003/01/28 21:50:24  womullan
* bug fixes
*
* Revision 1.1.1.1  2003/01/23 19:23:49  womullan
* Updated to 64bit and new algorithms 4 imes faster
*
************************************************************************/
