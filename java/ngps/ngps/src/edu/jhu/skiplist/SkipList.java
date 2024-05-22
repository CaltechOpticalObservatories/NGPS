package edu.jhu.skiplist;
//
// File:        SkipList.java
//
// Language:    Java 1.02
// Description: Class for a "SkipList" proposed by William Pugh.
//              His paper is available at ftp://ftp.cs.umd.edu/pub/skipLists
//
//              Change: the levels start at 0 (original work at 1)
//
// Author:  Thomas Wenger, Jan-7-1998
//


public class SkipList {

  //
  // Constants:
  //
  public static final long NOT_FOUND  = -1;    // value=-1 means not found
  public static final long HEADER_KEY = -2;    // key=-2 means header element
  public static final long SKIPLIST_MAXLEVEL = 6;    // key=-2 means header element
  // all keys have to be smaller than this one:
  public static final long NIL_KEY    = Long.MAX_VALUE;

  public static final float OPT_PROB = 0.25f; // optimum probability

  protected  int myLength = 0;

  public SkipList(float prob) {
	this(prob,
		(int)Math.ceil(Math.log(SKIPLIST_MAXLEVEL)/Math.log(1/OPT_PROB))-1);
  }

  ///////////////////////////////////////////////////////////////////////////
  // Constructor (1):
  //   Constructs a new skip list optimized for the given
  //   expected upper bound for the number of nodes.
  //   So if you expect to have 10000 nodes, call the
  //   constructor as SkipList(10000).
  //
  public SkipList(long maxNodes) {
	// call the constructor (2) with a calculated maximum level
	// and probability set to OPT_PROP=0.25
	// Maximum level of list is depending on expected number of nodes
	// (see paper for mathematical background)
	this(OPT_PROB,
		(int)Math.ceil(Math.log(maxNodes)/Math.log(1/OPT_PROB))-1);
  }


  ///////////////////////////////////////////////////////////////////////////
  // Constructor (2):
  //   Constructs a new skip list, where you can directly set the
  //   probability to increase the level of a new node (often 0.25)
  //   and maximum level of node in the list.
  //   If you are not sure, take constructor (1)!
  //
  public SkipList(float probability, int maxLevel) {
	myLevel = 0;                  // level of empty list
	myProbability = probability;
	myMaxLevel = maxLevel;

	// generate the header of the list:
	myHeader = new SkipListElement(myMaxLevel, HEADER_KEY, 0);

	// append the "NIL" element to the header:
	SkipListElement nilElement =
		new SkipListElement(myMaxLevel, NIL_KEY, 0);
	for (int i=0; i<=myMaxLevel; i++) {
	  myHeader.forward[i] = nilElement;
	}

  }


  ///////////////////////////////////////////////////////////////////////////
  // generateRandomLevel():
  //   Generates with help of randomizer the level of a new element.
  //   The higher a level, the less probable it is (see paper).
  //   Levels begin at 0 (not at 1 like in the paper).
  //
  protected int generateRandomLevel() {
	int newLevel = 0;
	while (newLevel<myMaxLevel && Math.random()<myProbability ) {
	  newLevel++;
	}
	return newLevel;
  }


  ///////////////////////////////////////////////////////////////////////////
  // insert():
  //   Inserts a new node into the list.
  //   If the key already exists, its node is updated to the new value.
  //
  public void insert(long searchKey, long value) {
	// update holds pointers to next elements on each level;
	// levels run from 0 up to myMaxLevel:
	SkipListElement[] update = new SkipListElement[myMaxLevel+1];

	// init "cursor" element to header:
	SkipListElement element = myHeader;

	// find place to insert the new node:
	for (int i=myLevel; i>=0; i--) {
	  while (element.forward[i].key < searchKey) {
		element = element.forward[i];
	  }
	  update[i] = element;
	}
	element = element.forward[0];

	// element with same key is overwritten:
	if (element.key == searchKey) {
	  element.value = value;
	}

	// or an additional element is inserted:
	else {
	  int newLevel = generateRandomLevel();
	  // element has biggest level seen in this list: update list
	  if (newLevel > myLevel) {
		for (int i=myLevel+1; i<=newLevel; i++) {
		  update[i] = myHeader;
		}
		myLevel = newLevel;
	  }

	  // allocate new element:
	  element = new SkipListElement(newLevel, searchKey, value);
	  myLength++;
	  for (short i=0; i<=newLevel; i++) {
		element.forward[i] = update[i].forward[i];
		update[i].forward[i] = element;
	  }
	}
  }


  ///////////////////////////////////////////////////////////////////////////
  // search():
  //   Search for a given key in list. you get the value associated
  //   with that key or the NOT_FOUND constant.
  //
  public long search(long searchKey) {
	// init "cursor"-element to header:
	SkipListElement element = myHeader;

	// find element in list:
	for (int i=myLevel; i>=0; i--) {
	  SkipListElement nextElement = element.forward[i];
	  while (nextElement.key < searchKey) {
		element = nextElement;
		nextElement = element.forward[i];
	  }
	}
	element = element.forward[0];

	// if key exists return value else return predefined NOT_FOUND:
	if (element.key == searchKey) {
	  return element.value;
	}
	else {
	  return NOT_FOUND;
	}
  }


  ///////////////////////////////////////////////////////////////////////////
  // delete():
  //   If a node with the given key exists, remove it from list.
  //
  public void delete(long searchKey) {
	// update holds pointers to next elements of each level
	SkipListElement update[] = new SkipListElement[myMaxLevel+1];

	// init "cursor"-element to header:
	SkipListElement element = myHeader;

	// find element in list:
	for (int i=myLevel; i>=0; i--) {
	  SkipListElement nextElement = element.forward[i];
	  while (nextElement.key < searchKey) {
		element = nextElement;
		nextElement = element.forward[i];
	  }
	  update[i] = element;
	}
	element = element.forward[0];

	// element found, so rebuild list without node:
	if (element.key == searchKey) {
	  for (int i=0; i<=myLevel; i++) {
		if (update[i].forward[i] == element) {
		  update[i].forward[i] = element.forward[i];
		}
	  }
	  // element can be freed now (would happen automatically):
	  element = null;               // garbage collector does the rest...
      myLength--;
	  // maybe we have to downcorrect the level of the list:
	  while (myLevel>0  &&  myHeader.forward[myLevel].key==NIL_KEY) {
		myLevel--;
	  }
	}
  }


  ///////////////////////////////////////////////////////////////////////////
  // toString() overwrites java.lang.Object.toString()
  //   Composes a multiline-string describing this list:
  //
  public String toString() {
	// inits:
	String result = "";

	// header info:
	result += "SkipList:\n";
	result += "  probability = " + myProbability + "\n";
	result += "  level       = " + myLevel + "\n";
	result += "  max. level  = " + myMaxLevel + "\n";

	// traverse the list and collect the levels:
	SkipListElement element = myHeader.forward[0];
	int[] countLevel = new int[myMaxLevel+1];
	while (element.key != NIL_KEY) {
	  countLevel[element.getLevel()]++;
	  element = element.forward[0];
	}

	for (int i=myMaxLevel; i>=0; i--) {
	  result += "    Number of Elements at level " + i + " = " + countLevel[i] +"\n";
	}
	return result;
  }


  ///////////////////////////////////////////////////////////////////////////
  // elementsToString()
  //   Composes a multiline-string describing the elements of this list:
  //
  public String elementsToString() {
	// inits:
	String result = "Elements:\n";

	// all elements:
	SkipListElement element = myHeader;
	while (element.key < NIL_KEY) {
	  element = element.forward[0];
	  result += element.toString() + "\n";
	}

	return result;
  }

  ///////////////////////////////////////////////////////////////////////////
  // Access to members:
  //

  // returns the current level of the list:
  public int getLevel() { return myLevel; }

  // returns the maximum level, which can be reached:
  public int getMaxLevel() { return myMaxLevel; }

  // returns the probability:
  public float getProbability() { return myProbability; }

  // returns the header element:
  public SkipListElement getHeader() { return myHeader; }


  ///////////////////////////////////////////////////////////////////////////
  // private data members:
  //
  private float myProbability;            // probability to increase level
  private int myMaxLevel;                 // upper bound of levels
  private int myLevel;                    // greatest level so far
  private SkipListElement myHeader;       // the header element of list

// stuff from george
  protected SkipListElement iter;
  public long getNthint(int n){
			int n_now = n-1;
				iter = myHeader.forward[0];
				while(n_now > 0){
						  if (iter.key == NIL_KEY)
								  break;
							iter = iter.forward[0];
							  n_now--;
							  }
					if (iter.key != NIL_KEY)
						  return iter.key;

					return -1;
					  }
  public void reset() {iter = myHeader.forward[0];}
  public boolean step() {
			  iter = iter.forward[0];
		  return (iter.key != NIL_KEY);
	  }
	public long getkey() {
        if (iter.key != NIL_KEY)
                  return iter.key;
            else
                  return -1;
   }
	public long getvalue() {
		return iter.value;
	}
// end iter

///////////////////////////////////////////////////////////////////////////////*/
					  public long findMAX(long search)
						// greatest key less than searchint.. almost completely, but not
						// quite entirely unlike a search ...
					  {
						int i;
						SkipListElement element;
						SkipListElement nextElement;
						long retlong;

						element = myHeader;
						for(i=myHeader.getLevel(); i>=0; i--) {
						  nextElement = element.forward[i];
						  while( (nextElement.key != NIL_KEY) && (nextElement.key < search) ) {
							element=nextElement;
							nextElement = element.forward[i];
						  }
						}
						// now nextElement is >= searhcint
						// and element is < searchint
						// therefore elemnt  key is largest key less than current

						// if this were search:
						// element=element.forward[0]; // skip to >=


						if(element.key != NIL_KEY) {
						  retlong= element.key;
						  return( retlong== myMaxLevel ? (-myMaxLevel) : retlong);
						}
						else
						  return((long) myMaxLevel);
					  }

					  public long searchAlt(long searchlong)
					  {
						int i;
						SkipListElement element;
						SkipListElement nextElement;

						element = myHeader;
						for(i=myHeader.getLevel(); i>=0; i--) {
						  nextElement = element.forward[i];
						  while( (nextElement.key != NIL_KEY) && (nextElement.key < searchlong) ) {
							element=nextElement;
							nextElement = element.forward[i];
						  }
						}

						element=element.forward[0]; // key is < searchlong

						// if key exists return value else ERROR
						if( (element.key != NIL_KEY) && (element.key == searchlong) )
						  return(element.key);
						else
						  return(SkipList.NOT_FOUND);
					  }
					  public long findMIN(long searchlong)
						// smallest greater than searchint.. almost completely, but not
						// quite entirely unlike a search ...
					  {
						int i;
						SkipListElement element;
						SkipListElement nextElement = null;
						long retlong;

						element = myHeader;
						for(i=myHeader.getLevel(); i>=0; i--) {
						  nextElement = element.forward[i];
						  while( (nextElement.key != NIL_KEY) && (nextElement.key <= searchlong) ) {
							element=nextElement;
							nextElement = element.forward[i];
						  }
						}
						// now nextElement is > searhclong
						// element is <= , make it >

						element = nextElement;
						if(element.key != NIL_KEY) {
						  retlong = element.key;
						  return( retlong == myMaxLevel ? (-myMaxLevel) : retlong);
						}
						else
						  return((long) myMaxLevel);
					  }

					  public long search(long searchlong, long iterator_flag)
					  {
						int i;
						SkipListElement element;
						SkipListElement nextElement;

						element = myHeader;
						for(i=myHeader.getLevel(); i>=0; i--) {
						  nextElement = element.forward[i];
						  while( (nextElement.key != NIL_KEY) && (nextElement.key < searchlong) ) {
							element=nextElement;
							nextElement = element.forward[i];
						  }
						}

						element=element.forward[0]; // key is < searchlong

						// if key exists return value else ERROR
						if( (element.key != NIL_KEY) && (element.key == searchlong) ) {
						  if (iterator_flag > 0){
							iter = element;
						  }
						  return(element.key);
						}
						else
						  return(SkipList.NOT_FOUND);
					  }

///////////////////////////////////////////////////////////////////////////////*/
					  public void freeRange(long lo, long hi)
					  {
						/* Very similar to free, but frees a range of keys
						   from lo to hi, inclusive **/

						int i;
						SkipListElement element;
						SkipListElement nextElement;


						// scan all levels while key < searchint
						// starting with header in his level

						//  System.err.println( "freeRange from " + lo+ " to " + hi);
						element = myHeader;
						for(i=myHeader.getLevel(); i>=0; i--) {
						  nextElement = element.forward[i];
						  while( (nextElement.key != NIL_KEY) && (nextElement.key < lo) ) {
							element=nextElement;
							nextElement = element.forward[i];
						  }
						}

						element=element.forward[0];  // key is < lo

						// [ed:diff] while keys exists

						//  System.err.println( "freeRange begins deleting at  " + element.key );
						while( (element.key != NIL_KEY) && (element.key <= hi) ) {
						  //    System.err.println( "freeRange wants to delete " + element.key );
						  nextElement = element.forward[0];
						  delete(element.key);
						  element = nextElement;
						}
					  }
					  public void stat()
					  {
						int count = 0;
						SkipListElement element;
						SkipListElement nextElement;

						element = myHeader;
						nextElement = element.forward[0];

						System.out.println( "Counting elements..." );
						while( (nextElement.key != NIL_KEY) ) {
						  count++;
						  element=nextElement;
						  nextElement = element.forward[0];
						}
						System.out.println( "Have number of elements ... " + count );
						System.out.println( "Size ...................... " + myLength );
						{
						  int[] hist;
						  hist = new int[20];
						  long totalPointers, usedPointers;
						  for(int i=0; i<20; i++){
							hist[i] = 0;
						  }
						  element = myHeader;
						  count = 0;
						  nextElement = element.forward[0];
						  while( (nextElement.key != NIL_KEY) ) {
							count++;
							hist[nextElement.getLevel()]++;
							element=nextElement;
							nextElement = element.forward[0];
						  }

						  // There are SKIPLIST_MAXLEVEL * count available pointers

						  totalPointers = this.myMaxLevel * count;
						  usedPointers = 0;

						  // of these every node that is not at the max level wastes some

						  for(int i=0; i<20; i++){
							if(hist[i] > 0) System.out.println(  i + ": " +  hist[i] );
							usedPointers += hist[i] * (1 + i);
						  }
						  System.out.println( "Used  pointers " + usedPointers );
						  System.out.println( "Total pointers " + totalPointers + " efficiency = " +
							(double) usedPointers / (double) totalPointers );
						  hist = null;

						}

						return;
					  }

	public int getLength() {
		    return myLength;
	}

}
