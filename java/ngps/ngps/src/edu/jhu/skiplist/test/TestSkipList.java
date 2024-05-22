package edu.jhu.skiplist.test;

import junit.framework.*;
import edu.jhu.skiplist.*;
import java.util.Random;    // Randomizer to produce test input

/** JUnitTest case for class: edu.jhu.skiplist.SkipList */
public class TestSkipList extends TestCase {
	///////////////////////////////////////////////////////////////////////////
	// array of constants to stear the test:
	//   Make changes here to verify performance of other parameters
	//
	public static final float[]   PROBABILITY      = { 0.50f  };
	public static final int[]     INITIAL_LOAD     = { 20000, 40000, 80000 };
	public static final boolean[] INPUT_IS_SORTED  = { false, true };
	public static final int NUM_OF_TEST_OPERATIONS = 1000;


    public TestSkipList(String _name) {
	super(_name);

    }

	public void testInsert(){
		long lo = 13211319402496L;
	    SkipList my_los = new SkipList(0.5f);
		my_los.insert(lo, 33);
		long val = my_los.search(lo);
        assertTrue("Insert failed ",val>SkipList.NOT_FOUND);
	}

	public void testAll (){

		// perform tests for all kind of input keys:
  for (int sortSel=0; sortSel<INPUT_IS_SORTED.length; sortSel++) {

	String s = (INPUT_IS_SORTED[sortSel]? "sorted." : "random order.");
	System.out.println("Keys are " + s);

	// perform tests for all number of keys basically in list
	for (int numSel=0; numSel<INITIAL_LOAD.length; numSel++) {

	  System.out.println("  Size of list: " + INITIAL_LOAD[numSel]
							  + " elements.");
	  //
	  // prepare the test input in an array;
	  //
	  Random rnd = new Random(1234);   // get a randomizer with seed=1234
	  int[] keys = new int[INITIAL_LOAD[numSel]];
	  for (int i=0; i<INITIAL_LOAD[numSel]; i++) {
		keys[i] = (INPUT_IS_SORTED[sortSel]? i : Math.abs(rnd.nextInt()-1) );
	  }

	  // perform tests for all probabilities:
	  for (int probSel=0; probSel<PROBABILITY.length; probSel++) {

		System.out.println("    Probability: " + PROBABILITY[probSel]);

		//
		// init the skip list as wished:
		//
		float prob = PROBABILITY[probSel];
		int numb = INITIAL_LOAD[numSel];

		// calculate maxLevel according to paper:
		int maxLevel = (int)Math.ceil(Math.log(numb) / Math.log(1/prob))-1;

		// use constructor (2) to build a skip exactly as wished:
		SkipList aSkipList = new SkipList(prob, maxLevel);

		// insert great number of elements before we measure:

		// execute the insertions:
		for (int i=0; i<INITIAL_LOAD[numSel]; i++) {
		  aSkipList.insert(keys[i], 0);
		}


		// begin with time measurements:

		///////////////////////////////////////////////////////////////////
		// search some elements:
		//

		// get system time before start:
		long start = System.currentTimeMillis();

		long val = 0;
		// execute the searches:
		for (int i=0; i<NUM_OF_TEST_OPERATIONS; i++) {
			val = aSkipList.search(keys[i]);
            assertTrue("Insert failed ",val>SkipList.NOT_FOUND);
		}

		// get system time after search and calculate duration:
		long stop = System.currentTimeMillis();
		float duration = stop - start;

		// output results:
		System.out.println("      Search took " + duration/NUM_OF_TEST_OPERATIONS
									  + " ms per element.");

		///////////////////////////////////////////////////////////////////
		// delete some elements:
		//

		// get system time before start:
		start = System.currentTimeMillis();

		// execute the deletions:
		for (int i=0; i<NUM_OF_TEST_OPERATIONS; i++) {
            aSkipList.delete(keys[i]);
			val = aSkipList.search(keys[i]);
            assertEquals("Delete failed ",val,SkipList.NOT_FOUND);
		}

		// get system time after deletion and calculate duration:
		stop = System.currentTimeMillis();
		duration = stop - start;

		// output results:
		System.out.println("      Delete took " + duration/NUM_OF_TEST_OPERATIONS
									 + " ms per element.");


		///////////////////////////////////////////////////////////////////
		// insert some elements:
		//

		// get system time before start:
		start = System.currentTimeMillis();

		// execute the deletions:
		for (int i=0; i<NUM_OF_TEST_OPERATIONS; i++) {
		  aSkipList.insert(keys[i],0);
		}

		// get system time after deletion and calculate duration:
		stop = System.currentTimeMillis();
		duration = stop - start;

		// output results:
		System.out.println("      Insert took " + duration/NUM_OF_TEST_OPERATIONS
									   + " ms per element.");
		System.out.println();
	  }
	}
	}
	}

	public static Test suite() {
        return new TestSuite(TestSkipList.class);
	}

    /** Executes the test case */
    public static void main(String[] argv) {
        String[] testCaseList = {TestSkipList.class.getName()};
//        junit.swingui.TestRunner.main(testCaseList);
    }
}
