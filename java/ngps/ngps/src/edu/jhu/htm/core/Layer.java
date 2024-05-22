package edu.jhu.htm.core;
import java.util.*;
import java.io.*;
/**********************************************************************
Layer structure - used internally by HTMindex.
Stores layer level, number of vertices in that level, number
of edges, nodes, the index of the first leaf node and the index
of the first new vertex.
<pre>
 Current Version
 ===============
 ID:	$Id: Layer.java,v 1.2 2003/02/19 15:46:11 womullan Exp $
 Revision: 	$Revision: 1.2 $
 Date/time:	$Date: 2003/02/19 15:46:11 $
</pre>
 @author:  		 Peter Z. Kunszt
				 initial java port by William O'Mullane
 */

class Layer {
	public int level_;		// layer level
	public int nVert_;		// number of vertices in this layer
	public int nNode_;		// number of nodes
	public int nEdge_;		// number of edges
	public int firstIndex_;	// index of first node of this layer
	public int firstVertex_;	// index of first vertex of this layer
  };

/**********************************************************************
* Revision History
* ================
*
* $Log: Layer.java,v $
* Revision 1.2  2003/02/19 15:46:11  womullan
* Updated comments mainly to make java docs better
*
* Revision 1.1  2003/02/14 16:46:00  womullan
*  Newly organised classes in the new packages
*
************************************************************************/
