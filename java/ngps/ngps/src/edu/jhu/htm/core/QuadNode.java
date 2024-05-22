package edu.jhu.htm.core;
/**********************************************************************
	 QuadNode structure
QuadNode is used as an internal structure in HTMIndexImp.
<pre>
 Current Version
 ===============
 ID:	$Id: QuadNode.java,v 1.2 2003/02/19 15:46:11 womullan Exp $
 Revision: 	$Revision: 1.2 $
 Date/time:	$Date: 2003/02/19 15:46:11 $
</pre>
	 @author:  		 Peter Z. Kunszt
			  		 initial java port by William O'Mullane
*/

  class QuadNode {
	int index_;				// its own index
	int v_[] = new int[3];		// The three vertex vector indices
	int w_[] = new int[3];		// The three middlepoint vector indices
	int childID_[] = new int[4];	// ids of children
	int parent_;				// id of the parent node (sorting)
	long id_;				// numeric id -> name
  };

/**********************************************************************
* Revision History
* ================
*
* $Log: QuadNode.java,v $
* Revision 1.2  2003/02/19 15:46:11  womullan
* Updated comments mainly to make java docs better
*
* Revision 1.1  2003/02/14 16:46:00  womullan
*  Newly organised classes in the new packages
*
************************************************************************/
