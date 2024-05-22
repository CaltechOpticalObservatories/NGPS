package edu.jhu.htm.app;
/*	 Filename:	   intersect.java

	 Intersect a domain with the index, returning the full and partial nodes

	 Author:		 Peter Z. Kunszt

	 Date:		   October 15, 1999


*/

import edu.jhu.htm.core.*;
import edu.jhu.htm.parsers.*;
import java.io.*;
import java.util.*;

/**
	*
SPecifiying -xml now lets this program read XML files structred like
	<area>
	    <convex>
			<constraint x="0.7" y="0.7" z="0" d="0.89" />
		</convex>
		<circle> <point ra="67.9" dec="-8.7"/> <radius r="5"/> </circle>
		<rect> <point ra="67.9" dec="-8.7"/> <point ra="68.1 dec="-8.5" /> </rect>
		<chull> <point ra="67.9" dec="-8.7"/>
	           <point ra="68.1 dec="-8.5" />
	           <point ra="68.0 dec="-8.6" />
	    </chull>
	</area>

   This example code demonstrates the intersection functionality of
  the HTMindexImp.
  It can be invoked by

 <pre>
 java edu.jhu.htm.app.intersect level savelevel domainfile [-b]
 </pre>

<p>
  where
<pre>
	 level	 : the level depth to build the index
	 domainfile: file containing a domain
	 [-nranges n]  : number of ranges to keep
</pre>

<p>
  The level and savelevel just indicate the index depth.  The <tt>-b</tt>
  switch might speed up things for depths up to 7 with large-area
  domains.

  <b>Domain files:</b><br>

<p>
  A domain is a set of convexes, connected by logical OR.
  A convex is a set of constraints, connected by logical AND.
  A constraint specifies a cap on the unit sphere by specifying
  a vector 'a' and a distance 'd' from the origin. 's' is the
  opening angle defining the cap.

<p>  Example: positive distance  a = (1,0,0), d = 0.5, s = 60 degrees

<pre>
.				   ____
.				---	---
.			   /		/|\
.			  /		/ |=\
.			 |		/  |==|	 this side is in the convex.
.			|		/\s |===|
.			|------------|---| -> direction a
.			|		\   |===|
.			 |		\  |==|
.			  \		\ |=/
.			   \		\|/
.				---____---
.
.
.					 <-d-> is positive (s < 90)
</pre>

<p>
 Example: negative distance  a = (-1,0,0), d = -0.5, s = 120 degrees

<pre>
.				   ____
.				---====---
.  this side is /========/|\
.  in the	  /========/=| \
.  convex	 |==== s__/==|  |
.			|===== / /===|   |
.  dir. a <- |------------|---|  'hole' in the sphere
.			|========\===|   |
.			 |========\==|  |
.			  \========\=| /
.			   \========\|/
.				---____---
.
</pre>

<p>
  A constraint is represented in the file as a set of 4 floatingpoint numbers
  <tt>x y z d</tt>  where <tt>a=(x,y,z)</tt>. Example:

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



  <b>Special Identifiers</b><br>

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

  Examples of a domain files:

<hr>

<p>
<b>Example 1</b>:

<pre>
#DOMAIN
1
#CONVEX
3
 0.5 0.707107 0.5 0.58077530122080200
 0.5 0.5 0.707107 0.63480775301220802
 0.707107 -0.5 0.5 0.8480775301220802
</pre>

<hr>

<p>
<b>Example 2</b>:

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

<hr>

<p>
<b>Example 3</b>:

<pre>
#DOMAIN
1
#RECTANGLE_RADEC
54 29
54 30
55 29
55 30
</pre>

<hr>

<p>
<b>Example 4</b>:

<pre>
#DOMAIN
1
#TRIANGLE
0 0 1
0 0.9 0.9
0.9 0 0.9
</pre>

<hr>

<p>
Example of a run of intersect:

<pre>
%intersect 4 2 example1
</pre>

OUTPUT:
<pre>
#DOMAIN
1
#CONVEX
3
0.707107 -0.5 0.5 0.848078
0.5 0.5 0.707107 0.634808
0.5 0.707107 0.5 0.580775



@author Peter Kunszt
@version $Revision : $
*/

public class intersect {

   public static void main (String[] argv) throws Exception{

	   //
	   // Initialization
	   //

	  String usage="Usage: intersect [-xml] [-expand] [-symbolic] [-nranges n] [-olevel o] [-varlength] [-verbose} [-save s] level domainfile ";

	  // initialize
	  if (argv.length < 2) {
		 throw new Exception(usage);
	  }


		int argc = 0;
		int savelevel = 5;
		int nranges = 100;
		int olevel = 20;
		boolean varlen = false;
		boolean compress = false;
		boolean symb = false;
		boolean expand = false;
		boolean xml = false;
		boolean verbose = false;

		while (argv[argc] != null && argv[argc].startsWith("-")) {
				boolean handled = false;
			   if (argv[argc].equalsIgnoreCase("-save")) {
				   savelevel = Integer.parseInt(argv[++argc]);
				   if (savelevel >8 || savelevel < 1) {
					   System.err.println("Save should be between 1 and 8.");
					   System.exit(2);
				   }
				   argc++;
				   handled = true;
			   }
			   if (argv[argc].equalsIgnoreCase("-olevel")) {
				   olevel = Integer.parseInt(argv[++argc]);
				   argc++;
				   handled = true;
			   }
			   if (argv[argc].equalsIgnoreCase("-nranges")) {
				   nranges = Integer.parseInt(argv[++argc]);
				   compress = true;
				   argc++;
				   handled = true;
			   }
			   if (argv[argc].equalsIgnoreCase("-varlength")) {
				   varlen=true;
				   argc++;
				   handled = true;
			   }
			   if (argv[argc].equalsIgnoreCase("-symbolic")) {
				   symb=true;
				   argc++;
				   handled = true;
			   }
			   if (argv[argc].equalsIgnoreCase("-xml")) {
				   xml=true;
				   argc++;
				   handled = true;
			   }
			   if (argv[argc].equalsIgnoreCase("-expand")) {
				   expand=true;
				   argc++;
				   handled = true;
			   }
			   if (argv[argc].equalsIgnoreCase("-verbose")) {
				   verbose=true;
				   argc++;
				   handled = true;
			   }
			   if (!handled) {
				   System.err.println(" Dont know how to "+argv[argc++]);
			   }
		}

	  Domain domain = new Domain();

	  // construct index with level=argv[0], savelevel=argv[1]
	  // beware: there is no checking whether argv[0] or argv[1] make sense
	  int level = 10;
	  try {
		  level = Integer.parseInt(argv[argc++]);
	  } catch (Exception nfe){
		  System.err.println (" you need to specify the max level  to intersect ");
		  System.exit(2);
	  }
	  String fname = argv[argc++];
	  HTMindexImp index = new HTMindexImp(level,savelevel); // generate index

	  // Read in domain and echo it to screen
	  DomainParser p = null ;
	  if (xml) {
		  p = new SAXxmlParser();
	  } else {
		  p=new LegacyParser();
	  }
      domain = p.parseFile(fname);

	  if (olevel > level ) olevel = level;
	  domain.setOlevel(olevel);

	  if (verbose) {
          System.out.println(domain.toString());
          System.out.println("level is		 :"+level);
          System.out.println("olevel is		:"+olevel);
          System.out.println("savelevel		:"+savelevel);
          System.out.println("nranges is	   :"+nranges);
          System.out.println("varlength		:"+varlen);
          System.out.println("expand		   :"+expand);
          System.out.println("xml is		 :"+xml);
	  }

		HTMrange htmRange = new HTMrange();
		long tstart = System.currentTimeMillis();
		domain.intersect(index,htmRange,varlen)	;
		long t = (System.currentTimeMillis() - tstart) ;
        if (verbose) {
            System.out.println(" Intersect  took "+ t +" milis");
        }

		if (!varlen) {
			htmRange.defrag();
			if (compress) {
				htmRange.defrag(nranges);
			}
			if (expand) {
				expandList(System.out,htmRange,symb);
			} else {
				System.out.println(htmRange.toString(symb));
			}
		} else {
				System.out.println(htmRange.toString(HTMrange.LOWS,symb));
		}
   }

   static public void expandList(PrintStream out, HTMrange range , boolean symbolic) throws Exception{
	   HTMrangeIterator iter = new HTMrangeIterator(range,symbolic);
	   while (iter.hasNext()) {
		   out.println(iter.next());
	   }
   }


}
