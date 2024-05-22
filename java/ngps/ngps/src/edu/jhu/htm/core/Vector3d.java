package edu.jhu.htm.core;


/**********************************************************************

	 3-d vector class with conversion to ra/dec
<p>
The Vector3d is a standard 3D vector class with the addition
that each coordinate (x,y,z) is also kept in ra,dec since we expect
the vector to live on the surface of the unit sphere, i.e.
<pre>
  x<sup>2</sup> + y<sup>2</sup> + z<sup>2</sup>  = 1
</pre>

This is not enforced, so you can specify a vector that has not unit
length.  If you request the ra/dec of such a vector, it will be
automatically normalized to length 1 and you get the ra/dec of that
vector (the intersection of the vector's direction with the unit
sphere.

Vector multiplaction addition etc are inplemented on this class
<pre>
 Current Version
 ===============
 ID:	$Id: Vector3d.java,v 1.3 2003/02/19 15:46:11 womullan Exp $
 Revision: 	$Revision: 1.3 $
 Date/time:	$Date: 2003/02/19 15:46:11 $
</pre>
 @author:  		 Original Author Peter Z. Kunszt
					  java port by William O'Mullane
 @version $Revision: 1.3 $
*/

public class  Vector3d {

	double x_;
	double y_;
	double z_;
	double ra_;
	double dec_;
	boolean okRaDec_;


	/** Default cunstructor constructs (1,0,0), ra=0, dec=0.*/
	public Vector3d() {
	  x_=1; y_=0; z_=0; ra_=0; dec_=0; okRaDec_=true;
	}

	/** Constructor from three coordinates */
	public Vector3d(double x ,
			 double y ,
			 double z ) {
	x_=x; y_=y; z_=z;
	ra_=0; dec_=0; okRaDec_=false;
	}

	/** Construct from ra/dec */
	public Vector3d(double ra , double dec ) {
		ra_=ra; dec_=dec;
	okRaDec_=true;
	updateXYZ();
	}


	/** Copy constructor */
	public Vector3d( Vector3d  copy) {
	x_=copy.x_;
	y_=copy.y_;
	z_=copy.z_;
	ra_=copy.ra_;
	dec_=copy.dec_;
	okRaDec_=copy.okRaDec_;
   }


	/** Set member function: set values */
	public void set( double x,
			 double y,
			 double z) {
		x_=x; y_=y; z_=z;
	normalize();
	updateRaDec();
	}


	/** Set member function: set values using ra/dec */
	public void set( double ra,
			 double dec) {
		ra_=ra; dec_=dec;
	okRaDec_=true;
	updateXYZ();
	}


	/** Get x,y,z */
	public double[] get() {
	double ret[] = new double[3];
	ret[0]=x_; ret[1]=y_; ret[2]=z_;
	return ret;
	}

	/** return length of vector */
	public double length() {
	double sum = x_*x_ + y_*y_ + z_*z_;
	return Math.sqrt(sum);
	}


	/** return x (only as rvalue) */
	public double x() {
	return x_;
	}


	/** return y */
	public double y() {
	return y_;
	}


	/** return z */
	public double z() {
	return z_;
	}


	/** Normalize vector length to 1 */
	public void normalize() {
	double sum;
	sum = x_*x_ + y_*y_ + z_*z_;
	sum = Math.sqrt(sum);
	x_ /= sum;
	y_ /= sum;
	z_ /= sum;
	}

	/** convert to string : print x y z as "x y z"*/
	public String toString() {
	return "" +x_ + " " + y_ + " " + z_;
	}


	/** vector cross product */
	public Vector3d cross(Vector3d v) {
	return  new Vector3d(y_ * v.z_ - v.y_ * z_,
				  z_ * v.x_ - v.z_ * x_,
				  x_ * v.y_ - v.x_ * y_);
	}

	/** comparison */
	public boolean equal(Vector3d v) {
	return ( (x_ == v.x_ && y_ == v.y_ && z_ == v.z_) ? true : false );
	}

	/** multiply with a number */
	public Vector3d mul(double n) {
	return new Vector3d((n*x_),(n*y_),(n*z_));
	}

	/** dot product */
	public double mul(Vector3d v) {
	return (x_*v.x_)+(y_*v.y_)+(z_*v.z_);
	}

	/** vector addition */
	public Vector3d add(Vector3d v) {
	return  new Vector3d(x_+v.x_, y_+v.y_, z_+v.z_);
	}

	/** vector subtraction */
	public Vector3d sub(Vector3d v) {
	return  new Vector3d(x_-v.x_, y_-v.y_, z_-v.z_);
	}

	/** return dec */
	public double dec() {
		if(!okRaDec_) {
	  normalize();
	  updateRaDec();
	}
	return dec_;
	}
	/** return ra */
	public double ra() {
		if(!okRaDec_) {
	  normalize();
	  updateRaDec();
	}
	return ra_;
	}

	/** Update x_ y_ z_ from ra_ and dec_ variables */
	protected void updateXYZ() {
		double cd = Math.cos(dec_*HTMfunc.Pr);
	x_ = Math.cos(ra_*HTMfunc.Pr) * cd;
	y_ = Math.sin(ra_*HTMfunc.Pr) * cd;
	z_ = Math.sin(dec_*HTMfunc.Pr);
	}

	/** Update ra_ and dec_ from x_ y_ z_ variables */
	protected void updateRaDec() {
		// modifed form peters original due to plotting porblems - wil
	  dec_ = Math.asin(z_)/HTMfunc.Pr; // easy.
	  double cd = Math.cos(dec_*HTMfunc.Pr);
	  if(cd>HTMfunc.gEpsilon || cd<-HTMfunc.gEpsilon)
		if(y_>HTMfunc.gEpsilon || y_<-HTMfunc.gEpsilon) {
			if (y_ < 0.0)
			ra_ = 360 - Math.acos(x_/cd)/HTMfunc.Pr;
			else
			ra_ = Math.acos(x_/cd)/HTMfunc.Pr;
		} else {
		  ra_ = (x_ < 0.0 ? 180 : 0.0);
		  //ra_ = (x_ < 0.0 ? -1.0 : 1.0) * Math.asin(y_/cd)/HTMfunc.Pr;
		}
	  else
	ra_=0.0;
	  okRaDec_ = true;
	}


};




/**********************************************************************
* Revision History
* ================
*
* $Log: Vector3d.java,v $
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
