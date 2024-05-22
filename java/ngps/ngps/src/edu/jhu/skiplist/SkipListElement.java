package edu.jhu.skiplist;

//
// File:        SkipListElement.java
//
// Language:    Java 1.02
// Description: Class for a single node in a "SkipList"
//              as proposed by William Pugh
//
//              To get faster data access to the nodes, you have to direct
//              access data.
//
// Author:  Thomas Wenger, Jan-7-1998
//

public class SkipListElement {

  ///////////////////////////////////////////////////////////////////////////
  // Constructor:
  //   Constructs a new element of a skip list.
  //   level, key and value of the new node are given
  //
  public SkipListElement(int level, long key, long value) {
	this.key = key;
	this.value = value;
	forward = new SkipListElement[level+1];
  }


  ///////////////////////////////////////////////////////////////////////////
  // toString() overwrites java.lang.Object.toString()
  //   composes a multiline-string describing this node:
  //
  public String toString() {
	String result = "";

	// element data:
	result += " k:" + key + ", v:" + value + ", l:" + getLevel() + ". fwd k: ";

	// key of forward pointed nodes:
	for (int i=0; i<=getLevel(); i++) {
	  if (forward[i]!=null) {
		result += i + ": " + forward[i].key + ", ";
	  }
	  else {
		result += i + ": nil, ";
	  }
	}

	return result;
  }


  ///////////////////////////////////////////////////////////////////////////
  // getLevel():
  //   returns the level of this node (count starting at 0)
  //
  int getLevel() {
	return forward.length-1;
  }


  ///////////////////////////////////////////////////////////////////////////
  // accessible data members:
  //   access is "friendly", this way all classes in this package can access
  //   the data member directly: performance is increased
  long key;                        // key data (sort and search criterion)
  long value;                      // associated value
  SkipListElement forward[];      // array of forward pointers

}
