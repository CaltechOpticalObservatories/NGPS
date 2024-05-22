package edu.jhu.htm.parsers;
import edu.jhu.htm.core.*;
import java.io.*;
/**
 * This is inded to do the legacy text io
 * for io of convexes etc. The new Parser class should be used to
 * load files etc. An XMLParser shouls also be available to work with XML.
 *
<pre>
 Current Version
 ===============
 ID:	$Id: Htmio.java,v 1.2 2003/02/19 15:46:11 womullan Exp $
 Revision: 	$Revision: 1.2 $
 Date/time:	$Date: 2003/02/19 15:46:11 $
</pre>
 * @author wil
 * @version $Revision: 1.2 $
 */

public class Htmio {
	/** read from a stream as X Y Z D
	@see java.io.Reader
	@param in takes a Reader (an input stream)
 */
 static public void read(Reader in, Constraint c) throws IOException {
    StreamTokenizer tokens = new StreamTokenizer(in);

    tokens.parseNumbers();
    tokens.commentChar('#');
    double x,y,z;
    //	try {
    tokens.nextToken();
    if ( tokens.ttype != StreamTokenizer.TT_NUMBER) return;
    x = tokens.nval;
    tokens.nextToken();
    if ( tokens.ttype != StreamTokenizer.TT_NUMBER) return;
    y = tokens.nval;
    tokens.nextToken();
    if ( tokens.ttype != StreamTokenizer.TT_NUMBER) return;
    z = tokens.nval;
    tokens.nextToken();
    if ( tokens.ttype != StreamTokenizer.TT_NUMBER) return;
    c.d_ = tokens.nval;
    c.a_ = new Vector3d(x,y,z);
    c.a_.normalize();
    c.s_ = Math.acos(c.d_);
    if(c.d_ <= -HTMfunc.Epsilon) c.sign_ = Constraint.nEG;
    if(c.d_ >= HTMfunc.Epsilon ) c.sign_ = Constraint.pOS;
  }

/** read from a stream as RA DEC D*/
static  public void readRaDec(Reader in,Constraint c) throws IOException {
  StreamTokenizer tokens = new StreamTokenizer(in);

  tokens.parseNumbers();
  tokens.commentChar('#');
  double ra,dec;
  //	try {
  tokens.nextToken();
  if ( tokens.ttype != StreamTokenizer.TT_NUMBER) return;
  ra = tokens.nval;
  tokens.nextToken();
  if ( tokens.ttype != StreamTokenizer.TT_NUMBER) return;
  dec = tokens.nval;
  tokens.nextToken();
  if ( tokens.ttype != StreamTokenizer.TT_NUMBER) return;
  c.d_ = tokens.nval;
  c.a_ = new Vector3d(ra,dec);
	c.s_ = Math.acos(c.d_);
	if(c.d_ <= -HTMfunc.Epsilon) c.sign_ = Constraint.nEG;
	if(c.d_ >= HTMfunc.Epsilon ) c.sign_ = Constraint.pOS;
  }

 static public void read(Reader in, Convex rc) throws IOException {
  StreamTokenizer tokens = new StreamTokenizer(in);
  int i,nConstraints;
  tokens.parseNumbers();
  tokens.commentChar('#');

  tokens.nextToken();
  if ( tokens.ttype != StreamTokenizer.TT_NUMBER) return;
  nConstraints = (int)tokens.nval;
  for ( i = 0; i < nConstraints; i++) {
	  Constraint constr = new Constraint();
	  read(in,constr);
	  rc.add(constr);
  }
  }

  static public void readRaDec(Reader in, Convex rc) throws IOException {
  StreamTokenizer tokens = new StreamTokenizer(in);
  int i,nConstraints;
  tokens.parseNumbers();
  tokens.commentChar('#');

  tokens.nextToken();
  if ( tokens.ttype != StreamTokenizer.TT_NUMBER) return;
  nConstraints = (int)tokens.nval;
  for ( i = 0; i < nConstraints; i++) {
	  Constraint constr = new Constraint();
	  readRaDec(in,constr);
	  rc.add(constr);
  }
  }

  /** read from a stream as X Y Z*/
   static public void read(Reader in, Vector3d vec) throws IOException {
   StreamTokenizer tokens = new StreamTokenizer(in);
	double x,y,z ;
   tokens.parseNumbers();
   tokens.commentChar('#');

   tokens.nextToken();
   if ( tokens.ttype != StreamTokenizer.TT_NUMBER) return;
   x = tokens.nval;
   tokens.nextToken();
   if ( tokens.ttype != StreamTokenizer.TT_NUMBER) return;
   y = tokens.nval;
   tokens.nextToken();
   if ( tokens.ttype != StreamTokenizer.TT_NUMBER) return;
   z = tokens.nval;
    vec.set(x,y,z);
   }

 /** read from a stream as RA DEC*/
   static public void readRaDec(Reader in, Vector3d vec) throws IOException {
   StreamTokenizer tokens = new StreamTokenizer(in);
	double ra,dec;
   tokens.parseNumbers();
   tokens.commentChar('#');

   tokens.nextToken();
   if ( tokens.ttype != StreamTokenizer.TT_NUMBER) return;
   ra = tokens.nval;
   tokens.nextToken();
   if ( tokens.ttype != StreamTokenizer.TT_NUMBER) return;
   dec = tokens.nval;

   vec.set(ra,dec);
   }

   /** Read in a domain from a Reader (input stream).

	   <h3>Domain files format</h3>

 <ul>
 <li>  A domain is a set of convexes, connected by logical OR.
 <li>  A convex is a set of constraints, connected by logical AND.
 <li>  A constraint specifies a cap on the unit sphere by specifying
   a vector 'a' and a distance 'd' from the origin. 's' is the
   opening angle defining the cap.
 </ul>


 <p>
   A constraint is represented in the file as a set of 4 floatingpoint numbers
   x y z d  where a=(x,y,z). Example:

 <pre>
   1 0 0 0.5
 </pre>

 <p>
   A convex is the area defined by several constraints on the sphere
   that is common to all constraints. Of course such an area has to be convex.

 <p>
   A convex in the file is defined by a number (number of constraints
   in the convex) followed by as many convexes (each on one line.) Example:

 <pre>
   4
   0.5 0.707107 0.5 0.58077530122080200
   0.5 0.707107 0.5 0.84077530122080200
   -0.5 -0.907107 0.3  -0.87530122080200
   0.2 -0.907107 0.3  -0.77530122080200
 </pre>

 <p>
   A domain is an accumulation of convexes. A domain in the domainfile
   is represented by a number (number of convexes) followed by that many
   convexes. A domainfile may only contain one domain. It may contain
   comment lines, starting with #.



   <h4>Special Identifiers</h4>


 <p>
   The domain file accepts several special convex identifiers to
   read in special formats. These identifiers must come as a single
   comment line before the data, and no additional comments are allowed
   These are

 <pre>
	 #TRIANGLE			- the next three lines are triangle corners
	 #TRIANGLE_RADEC		- same, given in ra/dec
	 #RECTANGLE			- the next four lines are rectangle corners
	 #RECTANGLE_RADEC		- same, given in ra/dec
	 #CONVEX			- read convex in default format
	 #CONVEX_RADEC		- read the convex in ra/dec format
 </pre>

   <h4>Examples of a domain files</h4>

 Example 1:

 <pre>
 #DOMAIN
 1
 #CONVEX
 3
  0.5 0.707107 0.5 0.58077530122080200
  0.5 0.5 0.707107 0.63480775301220802
  0.707107 -0.5 0.5 0.8480775301220802
 <pre>

 <hr>

 Example 2:

 <pre>
 #DOMAIN
 2
 #CONVEX
 4
  0 0 1 0.3
  -1 -3 -5 -0.97
  5 -3 -10 -0.97
  2 2 -5 -0.97
 #CONVEX_RADEC
 1
 25.23 -55.9 0.99
 </pre>


 Example 3:

 <pre>
 #DOMAIN
 1
 #RECTANGLE_RADEC
 54 29
 54 30
 55 29
 55 30
 <pre>


 Example 4:

 <pre>
 #DOMAIN
 1
 #TRIANGLE
 0 0 1
 0 0.9 0.9
 0.9 0 0.9
 <pre>

   */
	 public static void read(Reader in,Domain d) throws IOException {
	 StreamTokenizer tokens = new StreamTokenizer(in);
	 int n,i,nConvexes;
	 d.clear();
	 tokens.parseNumbers();
	 tokens.wordChars('_','_');

	 //	tokens.commentChar('#');


	 tokens.nextToken();
	 while(tokens.ttype!=tokens.TT_NUMBER) // ignore comments
		 tokens.nextToken();
	 nConvexes = (int)tokens.nval;
	 //System.out.println("Number of convexes: "+nConvexes);

	 String specifier;

	 for(n = 0; n < nConvexes; n++) {
	   tokens.nextToken();
	   specifier = "";
	   if(tokens.ttype!=tokens.TT_WORD && tokens.ttype!=tokens.TT_NUMBER) {
		 tokens.nextToken();
		 specifier = tokens.sval;
	   } else if(tokens.ttype == tokens.TT_WORD)return;

	   if(specifier == "TRIANGLE") {
		   Vector3d v1 = new Vector3d();
		   Vector3d v2 = new Vector3d();
		   Vector3d v3 = new Vector3d();
		   read(in,v1);
		   read(in,v2);
		   read(in,v3);
		   Convex cvx = new Convex(v1,v2,v3);
		   d.add(cvx);
	   } else if(specifier.equalsIgnoreCase("RECTANGLE")) {
		   Vector3d v1 = new Vector3d();
		   Vector3d v2 = new Vector3d();
		   Vector3d v3 = new Vector3d();
		   Vector3d v4 = new Vector3d();
		   read(in,v1);
		   read(in,v2);
		   read(in,v3);
		   read(in,v4);
		   Convex cvx = new Convex(v1,v2,v3,v4);
		   d.add(cvx);
	   } else if(specifier == "TRIANGLE_RADEC") {
		   Vector3d v1 = new Vector3d();
		   Vector3d v2 = new Vector3d();
		   Vector3d v3 = new Vector3d();
		   readRaDec(in,v1);
		   readRaDec(in,v2);
		   readRaDec(in,v3);
		   Convex cvx = new Convex(v1,v2,v3);
		   d.add(cvx);
	   } else if(specifier.equalsIgnoreCase("RECTANGLE_RADEC")) {
		   Vector3d v1 = new Vector3d();
		   Vector3d v2 = new Vector3d();
		   Vector3d v3 = new Vector3d();
		   Vector3d v4 = new Vector3d();
		   readRaDec(in,v1);
		   readRaDec(in,v2);
		   readRaDec(in,v3);
		   readRaDec(in,v4);
		   Convex cvx = new Convex(v1,v2,v3,v4);
		   cvx.simplify(); // wil just trying it out
		   d.add(cvx);
	   } else  if(specifier == "CONVEX_RADEC") {
		   Convex conv = new Convex();
		   Htmio.readRaDec(in,conv);
		   d.add(conv);
	   } else {
		   Convex conv = new Convex();
		   Htmio.read(in,conv);
		   d.add(conv);
	   }
	 }
	 }

}
