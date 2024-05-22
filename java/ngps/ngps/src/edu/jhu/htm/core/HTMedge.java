package edu.jhu.htm.core;
import java.util.*;
import java.io.*;
/**********************************************************************

	 HTMedge is a helper class for the spatial index at construction
	 time.


	 Author:		 Peter Z. Kunszt, based on A. Szalay's code

	 Date:		   October 15, 1998

*/




/**

 Edge class

 The Edges are needed at construction time of the spatial index.
 They are used to generate the midpoints of the nodes in a certain layer.
 The interface is simple: construct a class giving it the HTMindexImp
 and the layer number. Then call makeMidPoints. The HTMindexImp will
 then have its midpoint constructed in every QuadNode.
<pre>
 Current Version
 ===============
 ID:	$Id: HTMedge.java,v 1.2 2003/02/19 15:46:11 womullan Exp $
 Revision: 	$Revision: 1.2 $
 Date/time:	$Date: 2003/02/19 15:46:11 $
</pre>
*/

public class HTMedge {

	public class Edge {
	public int start_;				// starting vertex index of edge
	public int end_;		  // index of end
	public int mid_;		  // index of center
	};

	// VARIABLES

	HTMindexImp tree_;		// reference to the tree class
	Edge[]  	 lTab_;		// Edges lookup table
	Edge[]   	 edges_;	// Edges array
	int 	 index_;	// index of the vertex that is built
	Layer layer_;	// layer to build edges in

	/** Constructor : give the tree and its layer
	 */
	HTMedge(HTMindexImp  tree, int layerindex) {
	tree_  = tree;
	layer_ = (Layer) tree_.getLayer(layerindex);
	edges_ = new Edge[layer_.nEdge_ +1];
	lTab_  = new Edge[layer_.nVert_*6];
  	index_ = layer_.nVert_;
	}


	/**
	   makeMidPoints: interface to this class. Set midpoints of every
	   node in this layer.
	*/
	void makeMidPoints() {
	  int index, c=0;

	  // build up the new edges

	  index = layer_.firstIndex_;
	  for(int i=0; i < layer_.nNode_; i++,index++){
		c = newEdge(c,index,0);
		c = newEdge(c,index,1);
		c = newEdge(c,index,2);
	  }
   }


	/**
	   Make a new edge, in the temporary edges_ at emindex, at
	   node_[index] using the k'th side. Since every edge belongs to
	   two faces, we have to check wether an edge has been already
	   processed or not (i.e. the midpoint has been constructed or
	   not). We have a lookup table for this purpose. Every edge is
	   stored at lTab[start_]. There may be up to 6 edges in every
	   vertex[start_] so if that table place is occupied, store it in
	   the next table position (and so on). So we only have to look up
	   6 positions at most.
	*/

	int newEdge(int emindex, int index, int k) {
	Edge en, em;
	int swap;

	em = new Edge();
	edges_[emindex] = em;
	QuadNode node = tree_.getNode(index);

	if (k==0) {
		em.start_ = node.v_[1];
		em.end_   = node.v_[2];
	} else if (k==1) {
		em.start_ = node.v_[0];
		em.end_   = node.v_[2];
	} else {
		em.start_ = node.v_[0];
		em.end_   = node.v_[1];
	}

	// sort the vertices by increasing index

	if(em.start_ > em.end_) {
		swap = em.start_;
		em.start_ = em.end_;
		em.end_ = swap;
	}

	// check all previous edges for a match, return pointer if
	// already present, log the midpoint with the new face as well

	if( (en=edgeMatch(em)) != null){
		node.w_[k] = en.mid_;
		return emindex;
	}

	// this is a new edge, immediately process the midpoint,
	// and save it with the nodes and the edge as well

	insertLookup(em);
	em.mid_	= getMidPoint(em);
	node.w_[k] = em.mid_;
	return ++emindex;
}


	/**
	   insertLookup: insert the edge em into the lookup table.
	   indexed by em->start_.
	   Every vertex has at most 6 edges, so only
	   that much lookup needs to be done.
	*/

	void insertLookup(Edge em) {
	int j = 6 * em.start_;
	int i;

	// do not loop beyond 6

	for(i=0; i<6; i++, j++)
		if ( lTab_[j] == null ) {
		lTab_[j] = em;
		return;
		}
	}


	/**
	   edgeMatch: fast lookup using the first index em->start_.
	   return pointer to edge if matches, null if not.
	*/
	Edge  edgeMatch(Edge em) {
	int i = 6 * em.start_;

	while ( lTab_[i] != null ) {
		if(em.end_ == lTab_[i].end_ ) return lTab_[i];
		i++;
	}
	return null;
	}

	/**
	   getMidPoint: compute the midpoint of the edge using vector
	   algebra and return its index in the vertex list
	*/
	int getMidPoint(Edge  em) {
	Vector3d v = tree_.getVertex(em.start_).add(
						   tree_.getVertex(em.end_));
	v.normalize();
	tree_.add(v,index_);
		return index_++;
	}
};



/**********************************************************************
* Revision History
* ================
*
* $Log: HTMedge.java,v $
* Revision 1.2  2003/02/19 15:46:11  womullan
* Updated comments mainly to make java docs better
*
* Revision 1.1  2003/02/14 16:46:00  womullan
*  Newly organised classes in the new packages
*
* Revision 1.1.1.2  2003/01/23 19:23:49  womullan
* Updated to 64bit and new algorithms 4 imes faster
*
************************************************************************/
