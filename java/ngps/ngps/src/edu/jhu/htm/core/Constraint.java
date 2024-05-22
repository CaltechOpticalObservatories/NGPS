package edu.jhu.htm.core;
import java.util.*;
/**********************************************************************

   The Constraint is really a cone on the sky-sphere. It is characterized
   by its direction a_, the opening angle s_ and its cosine -- the distance
   of the plane intersecting the sphere and the sphere center.
   If d_ = 0, we have a half-sphere. If it is negative, we have a 'hole'
   i.e. the room angle is larger than 90degrees.

   Example: positive distance
   <pre>
.                   ____
.                ---    ---
.               /        /|\
.              /        / |=\
.             |        /  |==|     this side is in the convex.
.            |        /\s |===|
.            |------------|---| -> direction a
.            |        \   |===|
.             |        \  |==|
.              \        \ |=/
.               \        \|/
.                ---____---
.
.
.                     <-d-> is positive (s < 90)

   </pre>
   Example: negative distance
   <pre>
.                   ____
.                ---====---
.  this side is /========/|\
.  in the      /========/=| \
.  convex     |==== s__/==|  |
.            |===== / /===|   |
.  dir. a <- |------------|---|  'hole' in the sphere
.            |========\===|   |
.             |========\==|  |
.              \========\=| /
.               \========\|/
.                ---____---
.
.
.                     <-d-> is negative (s > 90)
</pre>
 for d=0 we have a half-sphere. Combining such, we get triangles, rectangles
 etc on the sphere surface (pure ZERO convexes)

<pre>
 Current Version
 ===============
 ID:	$Id: Constraint.java,v 1.3 2003/02/19 15:46:11 womullan Exp $
 Revision: 	$Revision: 1.3 $
 Date/time:	$Date: 2003/02/19 15:46:11 $
</pre>
 @author Wil
 @version $Revision: 1.3 $
*/

public class  Constraint  extends  Sign {

    public Vector3d a_;			// normal vector
    public double        d_;			// distance from origin
    public double        s_;			// cone angle in radians

  /** Default Constructor */
    public Constraint() {
	super();
    };

  /** Construct by setting only the distance
      @param distance set the distance to the cap
   */
    public Constraint(double distance) {
	super();
	d_=distance;
    }

  /** Initialization constructor
      @param vector Vector3d specifying the direction of the cap
      @param distance The distance of the cutting plane from the origin
   */
    public Constraint(Vector3d vector, double distance) {
	super();
	a_=vector;
	d_=distance;
	a_.normalize();
  	s_ = Math.acos(d_);
  	if(d_ <= -HTMfunc.gEpsilon) sign_ = nEG;
  	if(d_ >= HTMfunc.gEpsilon ) sign_ = pOS;
    }


  /** Initialization constructor
      @param x x-direction of vector  specifying the direction of the cap
      @param y y-direction of vector  specifying the direction of the cap
      @param z z-direction of vector  specifying the direction of the cap
      @param distance The distance of the cutting plane from the origin
   */
    public Constraint(double x, double y, double z, double distance) {
	super();
	a_=new Vector3d(x,y,z);
	d_=distance;
	a_.normalize();
  	s_ = Math.acos(d_);
  	if(d_ <= -HTMfunc.gEpsilon) sign_ = nEG;
  	if(d_ >= HTMfunc.gEpsilon ) sign_ = pOS;
    }


  /** Copy constructor
      @param constraint The constraint to be copied
   */
    public Constraint( Constraint copy) {
	a_=copy.a_;
	d_=copy.d_;
	s_=copy.s_;
	sign_=copy.sign_;
    }


  /** Invert a constraint */
    public void invert() {
	d_ = -d_;
	s_ = Math.acos(d_);
	if(sign_ == nEG) sign_ = pOS;
	if(sign_ == pOS) sign_ = nEG;
    }

  /** check whether a vector is inside this constraint
      @param vector Vector3d to be checked
      @return True if inside constraint, False if outside
   */
    public boolean contains(Vector3d vector) {
	if ( Math.acos(vector.mul(a_)) < s_ ) return true;
	return false;
    }

  /** give back constraint direction
      @return Vector3d of constraint direction
   */
    public Vector3d  v() {
	return a_;
    }


  /** give back distance
      @return distance from origin
   */
    public double d() {
	return d_;
    }

  /** convert data to string
      @return string containing vector and distance (4 numbers)
   */
    public String toString() {
	return ""+a_.toString()+" "+d_+" "+printSign();
    }


};


/**********************************************************************
* Revision History
* ================
*
* $Log: Constraint.java,v $
* Revision 1.3  2003/02/19 15:46:11  womullan
* Updated comments mainly to make java docs better
*
* Revision 1.2  2003/02/14 16:46:00  womullan
*  Newly organised classes in the new packages
*
* Revision 1.1  2003/02/11 18:40:27  womullan
*  moving to new packages and names
*
*
************************************************************************/
