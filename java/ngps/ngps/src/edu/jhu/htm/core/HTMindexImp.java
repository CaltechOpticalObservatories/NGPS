package edu.jhu.htm.core;
import java.util.*;
import java.io.*;
/**********************************************************************

	 HTMindex is the class for the the sky indexing routines.

	 Author:  		 Peter Z. Kunszt
			  		 initial java port by William O'Mullane

   The Spatial Index is a quad tree of spherical triangles. The tree
   is built in the following way: Start out with 8 triangles on the
   sphere using the 3 main circles to determine them. Then, every
   triangle can be decomposed into 4 new triangles by drawing main
   circles between midpoints of its edges:
   <pre>
.
.					    /\
.					   /  \
.					  /____\
.					 /\    /\
.					/  \  /  \
.				       /____\/____\
.
   </pre>
   This is how the quad tree is built up to a certain level by decomposing
   every triangle again and again.
   For a complete descrption, please go to the
   <a href="http://www.sdss.jhu.edu/" target="_top">HTM index website</a>

   <p>
   This implementation builds the index up to a specified level (maxlevel)
   You can specify that it should keep the first few levels in memory with
   the second constructor argument (buildlevel). The default buildlevel is 5.

<pre>
 Current Version
 ===============
 ID:	$Id: HTMindexImp.java,v 1.2 2003/02/19 15:46:11 womullan Exp $
 Revision: 	$Revision: 1.2 $
 Date/time:	$Date: 2003/02/19 15:46:11 $
</pre>
@author Peter Kunszt
@version $Revision: 1.2 $
*/



public class  HTMindexImp implements HTMindex{
	// VARIABLES
	public int   maxlevel_;	// the depth of the Layer
	public int   buildlevel_;	// the depth of the Layer stored
	public int addlevel_ ; // maxlevel - build level
	protected int  leaves_;	// number of leaf nodes
	protected int  storedleaves_;	// number of stored leaf nodes
	protected List nodes_;	// the array of nodes
	protected List layers_;	// array of layers
	protected List vertices_;	// array of vertices
	protected int  index_;	// the current index_ of vertices
	protected int  nNodes_;	// Number of nodes to store
	protected int  nVertices_;	// Number of vertices to store
	protected int  lastSavedLeafIndex_;



	static final int IOFFSET = 9;

	/** COnstructor for index - pass your desired resolution in degrees
	 * this will contruct a HTM of level such that the triabgle widht is just less than
	 * the desired resolution. Starts at level 5 2.8 degrees.
	 */

	public HTMindexImp(double degResolution) throws HTMException{
		int lev =  5;
		double htmwidth = 2.8125;
		while (htmwidth > degResolution  && lev < 25) {
            htmwidth /= 2;
			lev = lev+1;
            System.err.println(htmwidth+" "+lev);
		}
		if (htmwidth > degResolution) ErrorHandler.handleError(ErrorHandler.TOOFINE);

		init(lev,5);
	}
	/** Constructor: depth 20 - keeping 5 levels in memory by default */
	public HTMindexImp() {
		this(20,5);
	}
	/** Constructor: give only the level to build - keeping 5 levels
		in memory by default */
	public HTMindexImp(int maxlevel) {
	this(maxlevel,5);
	}

  /** Constructor : give the level of the index
	 and optionally the level to build - i.e. the depth to keep in memory.
	 if maxlevel - buildlevel > 0 , that many levels are generated on the
	 fly each time the index is called.*/

	public HTMindexImp(int maxlevel, int buildlevel  ) {
		init(maxlevel,buildlevel);
	}

	protected void init(int maxlevel, int buildlevel  ) {
	maxlevel_=maxlevel;
	buildlevel_ = ((buildlevel==0||buildlevel >maxlevel) ?
				maxlevel: buildlevel);
    addlevel_ = maxlevel - buildlevel_;
	layers_ = new ArrayList(buildlevel_);
	vMax();

	if (HTMfunc.verbose) {
		System.out.println(nNodes_+" Nodes "+nVertices_+" Vertices ");
		System.out.println(leaves_+" Leaf Nodes "+storedleaves_+" Stored");
	}

	nodes_ = new ArrayList(nNodes_);	 // allocate space for all nodes
	vertices_= new ArrayList(nVertices_);// allocate space for all vertices

	QuadNode n0 = new QuadNode(); 		 // initialize invalid node
	n0.index_=0;
	nodes_.add(0,n0);

	// initialize first layer
	Layer l0= new Layer();
	l0.level_ = 0;
	l0.nVert_ = 6;
	l0.nNode_ = 8;
	l0.nEdge_ = 12;
	l0.firstIndex_ = 1;
	l0.firstVertex_ = 0;
	layers_.add(0,l0);

	// set the first 6 vertices
	double v[][] = {{0.0,  0.0,  1.0},  // 0
			{1.0,  0.0,  0.0},  // 1
			{0.0,  1.0,  0.0},  // 2
			   {-1.0,  0.0,  0.0},  // 3
			{0.0, -1.0,  0.0},  // 4
			{0.0,  0.0, -1.0} };// 5

	for(int i = 0; i < 6; i++) {
		Vector3d sv = new Vector3d( v[i][0], v[i][1], v[i][2]);
		vertices_.add(i,sv);
	}

	// create the first 8 nodes - index 1 through 8
	index_ = 1;
	newNode(1,5,2,8,0);  // S0
	newNode(2,5,3,9,0);  // S1
	newNode(3,5,4,10,0); // S2
	newNode(4,5,1,11,0); // S3
	newNode(1,0,4,12,0); // N0
	newNode(4,0,3,13,0); // N1
	newNode(3,0,2,14,0); // N2
	newNode(2,0,1,15,0); // N3

	//  loop through maxlevel steps, and build the nodes for each layer
	int pl=0;
	int level = buildlevel_;
	while(level-->0) {
		HTMedge edge = new HTMedge(this, pl);
		edge.makeMidPoints();
		makeNewLayer(pl);
		++pl;
	}
	//if (HTMfunc.verbose) System.out.println ("Done edges "+new Date());
	sortIndex();
	//if (HTMfunc.verbose) System.out.println ("Sorted "+new Date());
   }

	//===============================================================

	/**
	   showVertices: print every vertex to the output stream
	*/
	public void showVertices(PrintStream  out) {
	for(int i = 0; i < vertices_.size()-1; i++)
		out.println (vertices_.get(i));
	}


	//===============================================================

	/** nodeVertex: return index of vertices for a node
	*/
	public int[] nodeVertexIds( int idx) {
	return getNode(idx).v_;
	}

	//===============================================================

	/**
	   nodeVertex: return the vectors of the vertices, based on a bitlist leaf
	   index
	*/

	public Vector3d[] nodeVertex( int leaf ) {
	Vector3d ret[] = new Vector3d[3];

	if(buildlevel_ == maxlevel_) {
		int idx = leaf + IOFFSET;
		QuadNode N = getNode(idx);
		for (int i =0; i<3; i++) {
		ret[i] = getVertex(N.v_[i]);
		}
		return ret;
	}

	// buildlevel < maxlevel
	// get the id of the stored leaf that we are in
	// and get the vertices of the node we want
	int id = idByLeafNumber(leaf);
	int sid = id >> ((maxlevel_ - buildlevel_)*2);
	int idx = sid - storedleaves_ + IOFFSET;
	QuadNode N = getNode(idx);
	for (int i =0; i<3; i++) {
		ret[i] = getVertex(N.v_[i]);
	}

	// get name of leaf node
	String name = nameByLeafNumber(leaf);
	// loop through additional name characters and
	// pick the correct triangle accordingly, storing the
	// vertices in v1,v2,v3
	for(int i = buildlevel_ + 2; i < maxlevel_ + 2; i++) {
		Vector3d w0 = ret[1].add(ret[2]); w0.normalize();
		Vector3d w1 = ret[0].add(ret[2]); w1.normalize();
		Vector3d w2 = ret[1].add(ret[0]); w2.normalize();

		switch(name.charAt(i)) {
		case '0':
		ret[1] = w2;
		ret[2] = w1;
		break;
		case '1':
		ret[0] = ret[1];
		ret[1] = w0;
		ret[2] = w2;
		break;
		case '2':
		ret[0] = ret[2];
		ret[1] = w1;
		ret[2] = w0;
		break;
		case '3':
		ret[0] = w0;
		ret[1] = w1;
		ret[2] = w2;
		break;
		}
	}
	return ret;
	}

	//===============================================================

	/**
	   makeNewLayer: generate a new layer and the nodes in it
	   Here it is where the construction actually happens,
	   generating new triangles out of old ones.
	*/
	void makeNewLayer(int oldlayer) {

	int index;
	long	id;
	int newlayer = oldlayer + 1;

	Layer newl = new Layer();
	layers_.add(newlayer,newl);
	Layer oldl = (Layer) layers_.get(oldlayer);

	newl.level_	   = oldl.level_+1;
	newl.nVert_	   = oldl.nVert_ + oldl.nEdge_;
	newl.nNode_	   = 4 * oldl.nNode_;
	newl.nEdge_	   = newl.nNode_ + newl.nVert_ - 2;
	newl.firstIndex_  = index_;
	newl.firstVertex_ = oldl.firstVertex_ + oldl.nVert_;

	int ioffset	   = oldl.firstIndex_ ;
	for(index = ioffset; index < ioffset + oldl.nNode_; index++){
		QuadNode N = getNode(index);
		id = N.id_ << 2;
		N.childID_[0] = newNode(N.v_[0],N.w_[2],N.w_[1],id++,index);
		N.childID_[1] = newNode(N.v_[1],N.w_[0],N.w_[2],id++,index);
		N.childID_[2] = newNode(N.v_[2],N.w_[1],N.w_[0],id++,index);
		N.childID_[3] = newNode(N.w_[0],N.w_[1],N.w_[2],id,index);
	}
	}

	//===============================================================

	/**
	   Make a new Node:
	   insert a new node_[] into the list. The vertex indices are given by
	   v1,v2,v3 and the id of the node is set.
	*/
	public int newNode(int v1, int v2, int v3, long id, int parent) {
	QuadNode last = new QuadNode();
	last.v_[0] = v1;		  // vertex indices
	last.v_[1] = v2;
	last.v_[2] = v3;
	last.w_[0] = 0;		   // middle point indices
	last.w_[1] = 0;
	last.w_[2] = 0;
	last.childID_[0] = 0;	   // child indices
	last.childID_[1] = 0;	   // index 0 is invalid node.
	last.childID_[2] = 0;
	last.childID_[3] = 0;

	last.id_ = id;		   // set the id
	last.index_ = index_;	// set the index
	last.parent_ = parent;   // set the parent

	nodes_.add(index_,last);
	return index_++;
	}

	//===============================================================

	/**
	   The area in steradians for a given index ID
	*/
	public double area(String htmName) {
        Object n[] = HTMfunc.nameToTriangle(htmName);
        Vector3d vec[] = new Vector3d[3];
		double [] vn = null;
		for (int v = 0; v < 3 ; v ++ ) {
			vn = (double[])n[v];
			vec[v] = new Vector3d(vn[0],vn[1],vn[2]);
		}
        return area(vec[0],vec[1],vec[2]);
	}

	//===============================================================

   /**
	   area: routine to precompute the area of a node using
	   AREA = 4*arctan sqrt(tan(s/2)tan((s-a)/2)tan((s-b)/2)tan((s-c)/2))
	   with s = (a+b+c)/2
	   (with many thanks to Eduard Masana @ University of Barcelona)
	*/

	public double area ( Vector3d  n0,
						 Vector3d  n1,
						 Vector3d  n2) {

		double a = Math.acos( n0.mul(n1));
		double b = Math.acos( n1.mul(n2));
		double c = Math.acos( n2.mul(n0));

		double s = (a + b + c)/2.0;

		double area = 4.0*Math.atan(Math.sqrt(Math.tan(s/2.0)*
									Math.tan((s-a)/2.0)*
									Math.tan((s-b)/2.0)*
									Math.tan((s-c)/2.0)));
		return area;
	}

	//===============================================================

	/**
	   vMax: compute the maximum number of vertices for the
	   polyhedron after buildlevel of subdivisions and
	   the total number of nodes that we store
	   also, calculate the number of leaf nodes that we eventually have.
	*/
	void vMax() {
	int nv = 6;	// initial values
	int ne = 12;
	int nf = 8;
	int i  = buildlevel_;
	nNodes_ = nf;

	while(i-->0){
		nv += ne;
		nf *= 4;
		ne  = nf + nv - 2;
		nNodes_ += nf;
	}
	nVertices_ = nv;
	storedleaves_ = nf;

	// calculate number of leaves
	i = maxlevel_ - buildlevel_;
	while(i-- > 0)
		nf *= 4;
	leaves_ = nf;
	}

	//===============================================================

	/**
	   sortIndex: sort the index so that the first node is the invalid node
	   (index 0), the next 8 nodes are the root nodes
	   and then we put all the leaf nodes in the following block
	   in ascending id-order.
	   All the rest of the nodes is at the end.
	*/
	void sortIndex() {
	// create a copy of the node list
	QuadNode[] oldnodes = (QuadNode[]) nodes_.toArray(new QuadNode[0]);
	int index;
	int nonleaf;
	int leaf;


	// now refill the nodes_ list according to our sorting.
	for( index=IOFFSET, leaf=IOFFSET, nonleaf=nodes_.size()-1;
		 index < nodes_.size(); index++) {

		if( oldnodes[index].childID_[0] == 0 ) { // childnode
		// set leaf into list
		nodes_.set(leaf, oldnodes[index]);
		// set parent's pointer to this leaf
		for (int i = 0; i < 4; i++) {
			QuadNode leafn = (QuadNode)nodes_.get(leaf);
			QuadNode parent = (QuadNode)nodes_.get(leafn.parent_);
			if(parent.childID_[i] == index) {
			parent.childID_[i] = leaf;
			break;
			}
		}
		leaf++;
		} else {
		// set nonleaf into list from the end
		// set parent of the children already to this
		// index, they come later in the list.
		QuadNode N = oldnodes[index] ;
		nodes_.set(nonleaf, N);
		oldnodes[N.childID_[0]].parent_ = nonleaf;
		oldnodes[N.childID_[1]].parent_ = nonleaf;
		oldnodes[N.childID_[2]].parent_ = nonleaf;
		oldnodes[N.childID_[3]].parent_ = nonleaf;
		// set parent's pointer to this leaf
		for (int i = 0; i < 4; i++) {
			QuadNode pnl = (QuadNode) nodes_.get(N.parent_);
			if(pnl.childID_[i] == index) {
			pnl.childID_[i] = nonleaf;
			break;
			}
		}
		nonleaf--;
		}
	}
	lastSavedLeafIndex_ = leaf;
	}

	//===============================================================

	/**
	 * This calls HTMfunc
.

	*/
	public long nameToId( String name ) throws HTMException{
        return HTMfunc.nameToId(name);
   }

	//===============================================================

	/**
	   calls HTMfunc

	*/
	public String idToName(long id ) throws HTMException{
		return HTMfunc.idToName(id);
	}

	//===============================================================

	/**
	   find a node by giving a vector. The ID of the node is returned.
	*/
	public long lookupId(Vector3d  v) throws HTMException {
		  return HTMfunc.lookupId(v.x(),v.y(),v.z(),this.maxlevel_);
	}
	/**
	   find a node by giving a vector. The Name of the node is returned.
	*/
	public String lookup(Vector3d  v)throws HTMException  {
		  return HTMfunc.lookup(v.x(),v.y(),v.z(),this.maxlevel_);
	}

	//===============================================================

	/**
	   Test whether a vector v is inside a triangle v0,v1,v2. Input
	   triangle has to be sorted in a counter-clockwise direction.
	*/
	boolean isInside( Vector3d  v,  Vector3d  v0,
			  Vector3d  v1,  Vector3d  v2) {
	if( (v0.cross(v1).mul(v)) < 0.0) return false;
	if( (v1.cross(v2).mul(v)) < 0.0) return false;
	if( (v2.cross(v0).mul(v)) < 0.0) return false;
	return true;
	}

	//===============================================================

	/**
	   leafCount: return number of leaf nodes
	*/
	public int leafCount() {
	return leaves_;
	}

	//===============================================================

	/**
	   nVertices: return number of vertices
	*/
	public int nVertices() {
	return vertices_.size();
	}

	//===============================================================

	/**
	   return leaf number in bitlist for a certain ID. Since the ID here
	   means the number computed from the name, this is simply returning
	   ID -leafCount().
	*/
	public int leafNumberById(long id) {
	return ((int)(id - leafCount()));
	}

	//===============================================================

	/**
	   return leaf id for a certain bitlist index. Same as the function above
	*/
	public int idByLeafNumber(int n) {
	return (leafCount() + n);
	}

	//===============================================================

	/**
	   return name for a certain leaf index
	   This function is simply shorthand for nameById(n + leafCount()).
	*/
	public String nameByLeafNumber(int n) {
		try {
            return idToName(idByLeafNumber(n));
		} catch (Exception e) {
			e.printStackTrace();
			return null;
		}
	}

	//===============================================================

	/**
	   find a node by giving a ra,dec in degrees.
	*/
	public String lookup( double  ra,  double  dec) throws HTMException {
        return HTMfunc.lookup(ra,dec,this.maxlevel_);
	}

	/**
	   find a node by giving a ra,dec in degrees.
	*/
	public long lookupId( double  ra,  double  dec) throws HTMException {
        return HTMfunc.lookupId(ra,dec,this.maxlevel_);
	}


	//===============================================================
	// These are internals to simplify the code. (By Wil)


	Vector3d getVertex (int x) {
	return (Vector3d) vertices_.get(x);
	}
	Vector3d V (int index, int x) {
	return (Vector3d) vertices_.get(getNode(index).v_[x]);
	}
	QuadNode getNode(int index) {
	return (QuadNode)nodes_.get(index);
	}
	Layer getLayer(int index) {
	return (Layer)layers_.get(index);
	}
	int noNodes() {
	return nodes_.size();
	}
	int noVertices() {
	return vertices_.size();
	}
	void add(Vector3d v,int i) {
	vertices_.add(i,v);
	}
	boolean isLeaf(int i) {
	return ((i < lastSavedLeafIndex_) ? true : false );

	}

	/** for the given id return the vector at the center of the triangle
	 * calls HTMfunc
	 * */
    public Vector3d idToPoint(long htmId) throws HTMException {
		double[] point = HTMfunc.idToPoint(htmId);
		return new Vector3d(point[0],point[1],point[2]);
    }
	/** for the given id return the vector at the center of the triangle
	calls HTMfunc */
    public Vector3d idToPoint(String htmName) throws HTMException {
		double[] point = HTMfunc.idToPoint(htmName);
		return new Vector3d(point[0],point[1],point[2]);
    }

    public Domain simplify(Domain d) {
		return d.simplify();
    };



    public boolean contains (Domain d, Vector3d p) {
		  return d.contains(p);
    }
	/** calls HTMfunc */
    public double distance(long htmId1, long htmId2) throws HTMException{
		return HTMfunc.distance(htmId1,htmId2);
    }
	/** calls HTMfunc */
    public double distance(String htmName1, String htmName2) throws HTMException{
		return HTMfunc.distance(htmName1,htmName2);
    }

	/** performs the intersection by calling domain.intersect */
    public HTMrange intersect(Domain d) {
		HTMrange range = new HTMrange();
		d.intersect(this,range,true);
		return range;
    }
	/** not implemented yet*/
	public Domain intersection(Domain d1, Domain d2) {
		throw new java.lang.UnsupportedOperationException(
				  "Method intersection() not yet implemented.");
	}

	/** creates a new domain which contains all convexces of d1 and d2 */
	public Domain union(Domain d1, Domain d2) {
		Domain ret = new Domain();
		ret.add(d1);
		ret.add(d2);
		return ret;
	}

	/** not implemented yet*/
	public Domain compliment(Domain d){
		throw new java.lang.UnsupportedOperationException(
				  "Method compliment() not yet implemented.");
	}

	/** not implemented yet*/
	public Domain smooth(Domain d){
		throw new java.lang.UnsupportedOperationException(
				  "Method smooth() not yet implemented.");
	}



};



/**********************************************************************
* Revision History
* ================
*
* $Log: HTMindexImp.java,v $
* Revision 1.2  2003/02/19 15:46:11  womullan
* Updated comments mainly to make java docs better
*
* Revision 1.1  2003/02/14 16:46:00  womullan
*  Newly organised classes in the new packages
*
* Revision 1.2  2003/02/07 19:36:03  womullan
*  new HTM for cc methods
*
* Revision 1.1.1.2  2003/01/23 19:23:49  womullan
* Updated to 64bit and new algorithms 4 imes faster
*
************************************************************************/
