package edu.jhu.htm.core;

 /**********************************************************************
 * These are the core routines for the HTM.
 * They are a direct lift from cc_aux but modifed along the lined of the new design
 * In pratically all cases below a double[] is a vector of 3 numbers representing
 * x,y,z in the unit sphere.
 *
<pre>
 Current Version
 ===============
 ID:	$Id: HTMfunc.java,v 1.5 2003/02/19 15:46:11 womullan Exp $
 Revision: 	$Revision: 1.5 $
 Date/time:	$Date: 2003/02/19 15:46:11 $
</pre>
 @version: 	$Revision: 1.5 $
**********************************************************************
 **/
public class HTMfunc {

	public static boolean verbose = Boolean.getBoolean("verbose");
	public static double Pi = 3.1415926535897932385E0 ;
	public static double Pr = 3.1415926535897932385E0/180.0;
	public static double Epsilon = 1.0E-15;
	public static double sqrt3	= 1.7320508075688772935;
	public static final int  IDSIZE=64;
	public static final long IDHIGHBIT = 1L << 63;
	public static final long IDHIGHBIT2 = 1L << 63;
	public static final int  HTMNAMEMAX   =32;
	static double gEpsilon = 1.0E-15;
	public static final double HTM_INVALID_ID=1;
	public static final int iS2=0;
	public static final int iN1=1;
	public static final int iS1=2;
	public static final int iN2=3;
	public static final int iS3=4;
	public static final int iN0=5;
	public static final int iS0=6;
	public static final int iN3=7;



	static Object[] anchor=  new Object[6];
	static Object S_indexes[]=  new Object[4];
	static Object N_indexes[]= new Object[4];

	// dont like static initializers but this is the only way I see to do this.
	static {
		int[] n0 = {1, 0, 4}; //N0
		N_indexes[0] = n0;
		int[] n1 = {4, 0, 3}; //N1
		N_indexes[1] = n1;
		int[] n2 = {3, 0, 2}; //N2
		N_indexes[2] = n2;
		int[] n3 = {2, 0, 1};//N3
		N_indexes[3] = n3;

		int[] s0 = {1, 5, 2}; //S0
		S_indexes[0] = s0;
		int[] s1 = {2, 5, 3}; //S1
		S_indexes[1] = s1;
		int[] s2 = {3, 5, 4}; //S2
		S_indexes[2] = s2;
		int[] s3 = {4, 5, 1};//S3
		S_indexes[3] = s3;

		double[] a0  = {0,  0,  1}; // 0
		anchor[0] =  a0;// 0
		double[] a1 = {1,  0,  0}; // 1
		anchor[1] =  a1;// 0
	 	double[] a2 =  {0,  1,  0}; // 2
		anchor[2] =  a2;// 0
	 	double[] a3 =  {-1,  0,  0}; // 3
		anchor[3] =  a3;// 0
	  	double[] a4 = {0, -1,  0}; // 4
		anchor[4] =  a4;// 0
	  	double[] a5 = {0,  0, -1}; // 5
		anchor[5] =  a5;// 0
	}

	static Base bases[] = {
		new Base("S2", 10, 3, 5, 4),
		new Base("N1", 13, 4, 0, 3),
		new Base("S1", 9, 2, 5, 3),
		new Base("N2", 14, 3, 0, 2),
		new Base("S3", 11, 4,5,1),
		new Base("N0", 12, 1, 0, 4),
		new Base("S0", 8, 1, 5, 2),
		new Base("N3", 15, 2, 0, 1)
	};




	/**
	 * where to start in the HTM quadtree when looking for a vectors home
	 * xin yin zin are thin input vector
	 * v1 v2 v3 and name are loaded with the initial tringle points
	 * and the name of the triangle
	 */
	static protected int startpane(
		 double[] v1, double[] v2, double[] v3,
		 double xin, double yin, double zin, StringBuffer name) throws HTMException
	{
	  double[] tvec;
	  int baseID;
	  int baseindex = 0;
	  if ((xin > 0) && (yin >= 0)){
	    baseindex = (zin >= 0) ? iN3 : iS0;

	  } else if ((xin <= 0) && (yin > 0)){
	    baseindex = (zin >= 0) ? iN2 : iS1;

	  } else if ((xin < 0) && (yin <= 0)){
	    baseindex = (zin >= 0) ? iN1 : iS2;

	  } else if ((xin >= 0) && (yin < 0)){
	    baseindex = (zin >= 0) ? iN0 : iS3;
	  } else {
	    ErrorHandler.handleError(ErrorHandler.PANIC);
	  }

	  baseID = bases[baseindex].ID;

	  tvec = (double[]) anchor[bases[baseindex].v1];
	  v1[0] = tvec[0];
	  v1[1] = tvec[1];
	  v1[2] = tvec[2];

	  tvec = (double[])anchor[bases[baseindex].v2];
	  v2[0] = tvec[0];
	  v2[1] = tvec[1];
	  v2[2] = tvec[2];

	  tvec = (double[])anchor[bases[baseindex].v3];
	  v3[0] = tvec[0];
	  v3[1] = tvec[1];
	  v3[2] = tvec[2];

	  name.append(bases[baseindex].name);
	  return baseID;
	}


	/* calculate midpoint fo vectors v1 and v2 the answer is put in
	 the provided w vector.
	 */
	protected static final void m4_midpoint(double[] v1, double[]
							v2, double[] w){
		w[0] = v1[0] + v2[0];
		w[1] = v1[1] + v2[1];
		w[2] = v1[2] + v2[2];
		double tmp = Math.sqrt(w[0] * w[0] + w[1] * w[1] + w[2]*w[2]);
		w[0] /= tmp;
		w[1] /= tmp;
		w[2] /= tmp;
	}


	/* for a given vector x,y,z return the HTM ID to the given depth*/
	static public String lookup(double x, double y, double z, int depth)
		throws HTMException
	{
	  long rstat = 0;
	  int startID;
	  StringBuffer name= new StringBuffer(80);

	  double v1[] = new double[3];
	  double v2[] = new double[3];
	  double v0[] = new double[3];
	  double w1[] = new double[3];
	  double w2[] = new double[3];
	  double w0[] = new double[3];
	  double p[] =  new double[3];
	  double dtmp = 0;

	  p[0] = x;
	  p[1] = y;
	  p[2] = z;

	  // Get the ID of the level0 triangle, and its starting vertices


	  startID = startpane(v0, v1, v2, x, y, z, name);

	  // Start searching for the children
	  //*/
	  while(depth-- > 0){
		m4_midpoint(v0, v1, w2);
		m4_midpoint(v1, v2, w0);
		m4_midpoint(v2, v0, w1);

		if (isinside(p, v0, w2, w1)) {
		  name.append('0');
		  copy_vec(v1, w2);
		  copy_vec(v2, w1);
		}
		else if (isinside(p, v1, w0, w2)) {
		  name.append('1');
		  copy_vec(v0, v1);
		  copy_vec(v1, w0);
		  copy_vec(v2, w2);
		}
		else if (isinside(p, v2, w1, w0)) {
		  name.append('2');
		  copy_vec(v0, v2);
		  copy_vec(v1, w1);
		  copy_vec(v2, w0);
		}
		else if (isinside(p, w0, w1, w2)) {
		  name.append('3');
		  copy_vec(v0, w0);
		  copy_vec(v1, w1);
		  copy_vec(v2, w2);
		}
		else {
		  ErrorHandler.handleError(ErrorHandler.PANIC);
		}
	  }
	  return name.toString();
	}

	/** looks up the name of a vector x,y,z and converts the name to an id */
	static public long lookupId(double x, double y, double z, int depth)
			throws HTMException {
	  String name = lookup(x,y,z,depth);
	  long rstat = nameToId(name.toString());
	  return rstat;
	}
	/**
		For given ra and dec lookup the HTMID to given depth
		HTM works in vectors so this basically converts ra dec to a vector and
		calls lookup for the vector.
	*/
	static public String lookup(double ra, double dec, int depth)
		throws HTMException
	{
		double v[] = radecToVector(ra,dec);
        return lookup(v[0],v[1],v[2],depth);
	}

	/** convert ra dec to a vector */
	static public double[] radecToVector(double ra, double dec) {
		double[] vec = new double[3];
		double cd = Math.cos( (dec * Pi) / 180.0);

		double diff;
		diff = 90.0 - dec;

		if (diff < Epsilon && diff > -Epsilon){
		    vec[0] = 1.0;
		    vec[1] = 0.0;
		    vec[2] = 1.0;
		    return vec;
		}

		diff = -90.0 - dec;
		if (diff < Epsilon && diff > -Epsilon){
		    vec[0] = 1.0;
		    vec[1] = 0.0;
		    vec[2] = -1.0;
		    return vec;
		}
		vec[2] = Math.sin((dec* Pi) / 180.0);
		double quadrant;
		double qint;
		int iint;
		quadrant = ra / 90.0; // how close is it to an integer?
		// if quadrant is (almost) an integer, force x, y to particular
		// values of quad:
		// quad,   (x,y)
		// 0       (1,0)
		// 1,      (0,1)
		// 2,      (-1,0)
		// 3,      (0,-1)
		// q>3, make q = q mod 4, and reduce to above
		// q mod 4 should be 0.
		qint = Math.round(quadrant);
		if(Math.abs(qint - quadrant) < Epsilon){
		iint = (int) qint;
		iint %= 4;
		if (iint < 0) iint += 4;

		switch(iint){
		      case 0:
			vec[0] = 1.0;
			vec[1] = 0.0;
			break;
		      case 1:
			vec[0] = 0.0;
			vec[1] = 1.0;
			break;
		      case 2:
			vec[0] = -1.0;
			vec[1] = 0.0;
			break;
		      case 3:
			vec[0] = 0.0;
			vec[1] = -1.0;
			break;
	      }
	      return vec;
	  }
        vec[0] = Math.cos((ra * Pi) / 180.0) * cd;
        vec[1] = Math.sin((ra * Pi) / 180.0) * cd;

		return vec;
	}

    /* same as lookup but converts the name to an id */
	static public long lookupId(double ra, double dec, int depth)
					throws HTMException{
		double x, y , z;
		StringBuffer name= new StringBuffer(80);
		double cd = Math.cos(dec * Pr);
		x = Math.cos(ra * Pr) * cd;
		y = Math.sin(ra * Pr) * cd;
		z = Math.sin(dec * Pr);
		return lookupId(x,y,z,depth);
	}

	/**
	* for a given vector p is it contained in the triangle whose corners are
	*  given by the vectors v1, v2,v3.
	*/
	static public boolean isinside(double[] p, double[] v1, double[] v2, double[] v3) // p need not be nromalilzed!!!
	{
	  double crossp[] = new double[3];

	  crossp[0] = v1[1] * v2[2] - v2[1] * v1[2];
	  crossp[1] = v1[2] * v2[0] - v2[2] * v1[0];
	  crossp[2] = v1[0] * v2[1] - v2[0] * v1[1];
	  if (p[0] * crossp[0] + p[1] * crossp[1] + p[2] * crossp[2] < -gEpsilon)
		return false;

	  crossp[0] = v2[1] * v3[2] - v3[1] * v2[2];
	  crossp[1] = v2[2] * v3[0] - v3[2] * v2[0];
	  crossp[2] = v2[0] * v3[1] - v3[0] * v2[1];
	  if (p[0] * crossp[0] + p[1] * crossp[1] + p[2] * crossp[2] < -gEpsilon)
		return false;


	  crossp[0] = v3[1] * v1[2] - v1[1] * v3[2];
	  crossp[1] = v3[2] * v1[0] - v1[2] * v3[0];
	  crossp[2] = v3[0] * v1[1] - v1[0] * v3[1];
	  if (p[0] * crossp[0] + p[1] * crossp[1] + p[2] * crossp[2] < -gEpsilon)
		return false;

	  return true;
	}

	/**
	* for a given name i.e. N301022 convert it to its 64bit htmId
	*  effectively this walks trough the string in reverse order and
	* sets the bits of a long number according to the charector in the
	* String.

	   The  name has always the same structure, it begins with
	   an N or S, indicating north or south cap and then numbers 0-3 follow
	   indicating which child to descend into. So for a depth-5-index we have
	   strings like
				 N012023  S000222  N102302  etc

	   Each of the numbers correspond to 2 bits of code (00 01 10 11) in the
	   uint64. The first two bits are 10 for S and 11 for N. For example

				 N 0 1 2 0 2 3
				 11000110001011  =  12683 (dec)

	   The leading bits are always 0

	**/
	static public long nameToId(String name) throws HTMException{
	  long out=0;
	  int i;
	  int siz = 0;

	  if(name == null || name.length()==0)			  // null pointer-name
		 ErrorHandler.handleError(ErrorHandler.NONAME);
	  if(name.charAt(0) != 'N' && name.charAt(0) != 'S')  // invalid name
		 ErrorHandler.handleError(ErrorHandler.INVALIDNAME);

	  siz = name.length();	   // determine string length
	  // at least size-2 required, don't exceed max
	  if(siz < 2)
		 ErrorHandler.handleError(ErrorHandler.INVALIDNAME);
	  if(siz > HTMNAMEMAX)
		 ErrorHandler.handleError(ErrorHandler.INVALIDNAME);

	  for(i = siz-1; i > 0; i--) {// set bits starting from the end
		if(name.charAt(i) > '3' || name.charAt(i) < '0') {// invalid name
		 ErrorHandler.handleError(ErrorHandler.INVALIDNAME);
		}
		out += ( (long)(name.charAt(i)-'0')) << 2*(siz - i -1);
	  }

	  i = 2;					 // set first pair of bits, first bit always set
	  if(name.charAt(0)=='N') i++;	  // for north set second bit too
	  long last = ((long)i << (2*siz - 2) );
	  out += last;
	  return out;
	}


	/**
	 * idLevel is a trusting method (assumes that the id is well formed and
	 * valid) that returns the level of the trixel represented by the given
	 * 64 bit htmId.
	 * This determines the index of the first bit set by continually left
	 * shifting the htmid 2 bits at a time until a set bit is found in the
	 * highest position.
	 * 2 bits are used as each number in an HTM Name like N012 is represented
	 * by two bits.
	 * Also the number of bits used dived by two is the level of the htmId +2
	 * we have +2 because of the N0,S1 etc  prefixes.
	 * So an id with bit 64 set is level 30 (64/2 -2)
	 *
	 **/
	static public int idLevel(long htmid)
	{
	  int size=0, i;


	  // determine index of first set bit
	  for(i = 0; i < IDSIZE; i+=2) {
		if ( ((htmid << i) & IDHIGHBIT )> 0) break;
	  }
	  size=(IDSIZE-i) >>> 1;
	  /* Size is the length of the string representing the name of the
		 trixel, the level is size - 2
	  **/
	  return size-2;
	}

	/**
	 * Walk the bits of the id and convert it to a string like N012.
	 * sucessivley look at each pair of bits and convert it to 0,1,2 or 3.
	 * Finally use the highest bit to set N or S in the string.
	 *
    * The first bit pair gives N (11) or S (10).
    * The subsequent bit-pairs give the numbers 0-3: (00, 01, 10, 11).

    Example: In the first level there are 8 nodes, and we get
    the names from numbers 8 through 15 giving S0,S1,S2,S3,N0,N1,N2,N3.

	   The order is always ascending starting from S0000.. to N3333...

	 */

	static public String idToName(long id) throws HTMException
	{
	  int size=0, i;
	  int c; // a spare character;


	  // determine index of first set bit
	  for(i = 0; i < IDSIZE; i+=2) {
			long x8 = ((id << i) & IDHIGHBIT);
			long x4 = ((id << i) & IDHIGHBIT2);
			if ( x8!=0 ) break;
			if ( x4!=0 ) {  // invalid id
				ErrorHandler.handleError(ErrorHandler.INVALIDID);
			}
	  }
	  if(id == 0) ErrorHandler.handleError(ErrorHandler.INVALIDID);

	  size=(IDSIZE-i) >>> 1;

	  char[] name = new char[size];
	  // fill characters starting with the last one
	  for(i = 0; i < size-1; i++) {
		c =  '0' + (int) ((id >>> i*2) & 3);
		name[size-i-1] = (char ) c;
	  }

	  // put in first character
	  if( ((id >> (size*2-2)) & 1) > 0 ) {
		name[0] = 'N';
	  } else {
		name[0] = 'S';
	  }

	  return new String(name);
	}

	/**
	 *  Like name2Id but instaed of returning the htmId
	 *  return v0,v1,v2 vetors representin the corners of the trinagle
	 *  @return Object[] which is holding 3 double[] for v0 v1 v2
	 **/

	static public Object[] nameToTriangle(String name)
	{
	  int rstat = 0;
	  double w0[] = new double[3];
	  double w1[] = new double[3];
	  double w2[] = new double[3];
	  double v0[] = new double[3];
	  double v1[] = new double[3];
	  double v2[] = new double[3];
	  double dtmp = 0;



	  // Get the top level hemi-demi-semi space

	  int k;
	  int[] anchor_offsets= new int[3];
	  k = (int) name.charAt(1) - '0';

	  if (name.charAt(0) == 'S') {
		anchor_offsets[0] = ((int[])S_indexes[k])[0];
		anchor_offsets[1] = ((int[])S_indexes[k])[1];
		anchor_offsets[2] = ((int[])S_indexes[k])[2];
	  } else {
		anchor_offsets[0] = ((int[])N_indexes[k])[0];
		anchor_offsets[1] = ((int[])N_indexes[k])[1];
		anchor_offsets[2] = ((int[])N_indexes[k])[2];
	  }
	  copy_vec(v0, (double[])anchor[anchor_offsets[0]]);
	  copy_vec(v1, (double[])anchor[anchor_offsets[1]]);
	  copy_vec(v2, (double[])anchor[anchor_offsets[2]]);

	  int offset = 2;
	  int len = name.length();
	  while(offset < len){
		char s = name.charAt(offset);
		m4_midpoint(v0, v1, w2);
		m4_midpoint(v1, v2, w0);
		m4_midpoint(v2, v0, w1);
		switch(s) {
		case '0':
		  copy_vec(v1, w2);
		  copy_vec(v2, w1);
		  break;
		case '1':
		  copy_vec(v0, v1);
		  copy_vec(v1, w0);
		  copy_vec(v2, w2);
		  break;
		case '2':
		  copy_vec(v0, v2);
		  copy_vec(v1, w1);
		  copy_vec(v2, w0);
		  break;
		case '3':
		  copy_vec(v0, w0);
		  copy_vec(v1, w1);
		  copy_vec(v2, w2);
		  break;
		}
		offset++;
	  }
	  Object [] ret = {v0,v1,v2};
	  return ret;
	}

	/**
	 * simple utility funtion which copies one vector over another.
	 * this uses System.arraycopy which should be efficient.
	 * It assumes both arrays are the same lenght however it will copy all
	 * odf s to d so theoretically d could be longer than s.
	 * @param d destination of copy
	 * @param s source of copy
	 */
	protected static void copy_vec(double d[] , double s[]) {
		System.arraycopy(s,0,d,0,s.length);
	}


	/**
	 * for a given ID get back the approximate center of the triangle.
	 * This may be used as an inverse of lookup however bear in mind many points
	 * may fall in a triangle while only the center point will be returned from
	 * this function.
	 * The name is used to get the triange corners and these are then averaged
	 * to get a center point.
	 * The resultant vector is not normalized.
	 */

	public static double[] idToPoint(String name) throws HTMException{
		Object [] tri = nameToTriangle(name);
		double[] ret = new double[3];
		double[] v0 = (double[]) tri[0];
		double[] v1 = (double[]) tri[1];
		double[] v2 = (double[]) tri[2];
		double center_x, center_y, center_z, sum;

		center_x = v0[0] + v1[0] + v2[0];
		center_y = v0[1] + v1[1] + v2[1];
		center_z = v0[2] + v1[2] + v2[2];
		sum = center_x * center_x + center_y * center_y + center_z * center_z;
		sum = Math.sqrt(sum);
		center_x /= sum;
		center_y /= sum;
		center_z /= sum;
		ret[0] = center_x;
		ret[1]= center_y;
		ret[2]= center_z; // I don't want it nomralized or radec to be set,

		return ret;

	}

	/**
	* gets the name from the id and calls idToPoint with it.
	*/
	public static double[] idToPoint(long htmId) throws HTMException{
		String name = idToName(htmId);
		return idToPoint(name);
	}

	/**
	 * return the angular distance between two vectors
	 * ACOS (V1 . V2)
	 */
	public static double distance (double[] v1, double[] v2) {
	 double prod = 0;
	 for (int i = 0; i < v1.length; i++ ) {
		  prod += v1[i]*v2[i] ;
	 }
	 double dist = Math.acos(prod);
	 return dist;
	}

	/**
	 * return the angular distance between two htmids
	 * gets the vectors of the mid points and uses thoose to compute distance
	 */
	public static double distance (long htmId1, long htmId2) throws HTMException{
		double[] v1 = idToPoint(htmId1);
		double[] v2 = idToPoint(htmId2);
		return distance(v1,v2);
	}

	/**
	 * return the angular distance between two htm names
	 * gets the viectors of the mid points and uses thoose to compute distance
	 */
	public static double distance (String htm1, String htm2)throws HTMException {
		double[] v1 = idToPoint(htm1);
		double[] v2 = idToPoint(htm2);
		return distance(v1,v2);
	}


}

/**********************************************************************
* Revision History
* ================
*
* $Log: HTMfunc.java,v $
* Revision 1.5  2003/02/19 15:46:11  womullan
* Updated comments mainly to make java docs better
*
* Revision 1.4  2003/02/14 21:46:22  womullan
* *** empty log message ***
*
* Revision 1.3  2003/02/14 16:46:00  womullan
*  Newly organised classes in the new packages
*
* Revision 1.2  2003/02/11 18:40:27  womullan
*  moving to new packages and names
*
* Revision 1.1  2003/02/07 23:02:51  womullan
* Major tidy up of HTM to HTMfunc addition of exceptions and error handler
*
* Revision 1.2  2003/02/07 19:36:03  womullan
*  new HTM for cc methods
*
* Revision 1.1  2003/02/05 23:00:07  womullan
* Added new timimig routines and HTM clas to do cc stuff from the c++ package
*
*
************************************************************************/

