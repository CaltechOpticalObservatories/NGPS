package edu.jhu.htm.app;

/*	 Filename:	   lookup.java
	 specify a point on the sphere, return its ID/Name to a certain dept
	 Author:		 Peter Z. Kunszt
*/



import edu.jhu.htm.core.*;

import edu.jhu.htm.core.*;



/**

  This example code demonstrates the lookup functionality of
  the HTMindexImp.
  It can be invoked by
  <pre>
  java edu.jhu.htm.app.lookup level x y z
  </pre>
	or
  <pre>
  java edu.jhu.htm.app.lookup level ra dec

  </pre>

  where

<pre>
	 level	 : the level depth to go to (2 - 14)
	 x,y,z	 : define a point on the unit sphere (can be non-unit vector)
	 ra,dec	: "

</pre>

<p>
  it echoes the vector and returns its ID/Name for the given level.
<p>

<b>Example 1</b>: level 5, ra,dec = 10,25
<pre>

%java edu.jhu.htm.app.lookup 5 10 25

	(x,y,z) = 0.892539 0.157379 0.422618
	(ra,dec) = 10,25
	ID/Name = 16020 N322110
</pre>

<p>
<b>Example 2</b>: level 14, x,y,z = -1,2,-23  (output is normed version)
<pre>
%java edu.jhu.htm.app.lookup 14 -1 2 -23

	(x,y,z) = -0.0432742 0.0865485 -0.995307
	(ra,dec) = 116.565,-84.4471
	ID/Name = 2486622255 S110031231200233

</pre>

*/



public class lookup {

   public static void main (String[] argv) throws Exception{
//   public static void main (String[] argv2) throws Exception{
//           java.lang.String[] argv = new java.lang.String[3];
//           argv[0] = "7";
//           argv[1] = "116.565";
//           argv[2] = "-84.4471";
	   //
	   // Initialization
	   //

	  String usage="Usage: depth x y z\n or depth ra dec";
	  // read command line arguments
	  if (argv.length < 3) {
		 throw new Exception(usage);
	  }
	  int depth = Integer.parseInt(argv[0]);
	  // read point and initialize
	  double x,y,z,ra,dec;
	  Vector3d vec;
	  if (argv.length == 3) {
		ra = Double.parseDouble(argv[1]);
		dec = Double.parseDouble(argv[2]);
		vec = new Vector3d(ra,dec);// initialize Vector3d from ra,dec
	  } else {
		x = Double.parseDouble(argv[1]);
		y = Double.parseDouble(argv[2]);
		z = Double.parseDouble(argv[3]);
		vec = new Vector3d(x,y,z); // initialize from x,y,z
		vec.normalize();
	  }
	HTMindexImp si = new HTMindexImp(depth,5);
	  //
	  // Lookup
	  //
	  long id =0;
	  id = HTMfunc.lookupId(vec.x(),vec.y(),vec.z(),depth);	   // lookup id by Vector3d

	  long sid =si.lookupId(vec);
	  //
	  // Print result
	  //
	  System.out.println("(x,y,z)  = "+vec.toString());
	  System.out.println("(ra,dec) = "+vec.ra()+" "+vec.dec());
	  System.out.println("ID/Name cc  = "+id+" "+HTMfunc.idToName(id));
	  System.out.println("ID/Name spatialIndex  = "+sid+" "+si.idToName(id));
   }

}

