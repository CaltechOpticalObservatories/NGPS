package edu.jhu.htm.core.test;

import java.util.*;
import java.io.*;
import junit.framework.*;
import edu.jhu.htm.core.*;
import edu.jhu.htm.geometry.*;

/** JUnitTest case for class: edu.jhu.htm.core.HTMindexImp */
public class TestHTMindexImp extends TestCase {

	public TestHTMindexImp(String _name) {
	super(_name);
	}

	public static Test suite() {
		 return new TestSuite(TestHTMindexImp.class);
	 }

	public void testBarcelona() throws Exception {
		HTMindexImp index = (HTMindexImp) new HTMindexImp(12, 6);

        Vector3d[] v = new Vector3d[4];
        v[0] = new Vector3d(40.0d, 65.0d);
        v[1] = new Vector3d(40.0d, 66.0d);
        v[2] = new Vector3d(41.0d, 65.0d);
        v[3] = new Vector3d(41.0d, 66.0d);

        Convex convex = new Convex(v[0], v[1], v[2], v[3]);
		convex.simplify();

        Domain domain = new Domain();
        domain.add(convex);

		domain.setOlevel(12);
        HTMrange htmRange = new HTMrange();

        domain.intersect(index, htmRange, false);

        long current, max;
		htmRange.reset();
        long[] range = htmRange.getNext();
        while ((range != null) && (range[0] > 0))
        {
            System.err.println("Range: " + range[0] + " to " + range[1]);
            range = htmRange.getNext();
        }

		HTMrangeIterator iter = new HTMrangeIterator(htmRange,true);

		assertTrue(iter.hasNext()) ;

	}


	public void testResolutionConstruc()  throws Exception{
		    HTMindexImp htm = new HTMindexImp(1.0);
			assertEquals("Resolution is incorrect",7,htm.maxlevel_);

	}
	public void testIdByPoint10()  throws Exception{
		double ra = 10;
		double dec = 10;
		Vector3d vec = new Vector3d(ra,dec);// initialize Vector3d from ra,dec
		long id =0;
		 // build index
		HTMindexImp index = new HTMindexImp(10); // build the index
		id = index.lookupId(vec);	   // lookup id by Vector3d
		assertEquals("Incorrect Id returned",16305926,id);
	}

	public void testIdByPoint20() throws Exception {
		double ra = 10;
		double dec = 10;
		Vector3d vec = new Vector3d(ra,dec);// initialize Vector3d from ra,dec
		long id =0;
		 // build index
		HTMindexImp index = new HTMindexImp(20); // build the index
		id = index.lookupId(vec);	   // lookup id by Vector3d
		assertEquals("Incorrect Id returned",17098002819647L,id);
	}

	public void testnameById10() throws Exception{
		HTMindexImp index = new HTMindexImp(10); // build the index
		long id = 16305926;	   // know the answer to this one
		String name =index.idToName(id);
		assertEquals("Incorrect Id returned","N32030330012",name);
	}

	public void testnameById20() throws Exception{
		HTMindexImp index = new HTMindexImp(20); // build the index
		long id = 17098002819647L;	   // know the answer to this one
		String name =index.idToName(id);
		assertEquals("Incorrect Id returned","N320303300120212220333",name);
	}


	public void testLookup10() throws Exception{
	double ra = 10;
	double dec = 10;
	Vector3d vec = new Vector3d(ra,dec);// initialize Vector3d from ra,dec
	long id =0;
	long start = System.currentTimeMillis();
	 // build index
	HTMindexImp index = new HTMindexImp(10); // build the index
	id = index.lookupId(vec);	   // lookup id by Vector3d
	String name =index.idToName(id);
	assertEquals("Incorrect Id returned","N32030330012",name);

	vec.set(45,45);
	id = index.lookupId(vec);	   // lookup id by Vector3d
	name =index.idToName(id);
	assertEquals("Incorrect Id returned","N33313333303",name);

	vec.set(90,-45);
	id = index.lookupId(vec);	   // lookup id by Vector3d
	name =index.idToName(id);
	assertEquals("Incorrect Id returned","S01100000000",name);

	vec.set(180,85);
	id = index.lookupId(vec);	   // lookup id by Vector3d
	name =index.idToName(id);
	assertEquals("Incorrect Id returned","N21000200100",name);

	vec.set(275,-85);
	id = index.lookupId(vec);	   // lookup id by Vector3d
	name =index.idToName(id);
	assertEquals("Incorrect Id returned","S31000200223",name);
	float time = System.currentTimeMillis() - start ;
	System.err.println("Test took "+time + " miliseconds");
}

public void testLookup14() throws Exception{
	double ra = 10;
	double dec = 10;
	Vector3d vec = new Vector3d(ra,dec);// initialize Vector3d from ra,dec
	long id =0;
	long start = System.currentTimeMillis();
	 // build index
	HTMindexImp index = new HTMindexImp(14); // build the index
	id = index.lookupId(vec);	   // lookup id by Vector3d
	String name =index.idToName(id);
	assertEquals("Incorrect Id returned","N320303300120212",name);

	vec.set(45,45);
	id = index.lookupId(vec);	   // lookup id by Vector3d
	name =index.idToName(id);
	assertEquals("Incorrect Id returned","N333133333033333",name);

	vec.set(90,-45);
	id = index.lookupId(vec);	   // lookup id by Vector3d
	name =index.idToName(id);
	assertEquals("Incorrect Id returned","S011000000000000",name);

	vec.set(180,85);
	id = index.lookupId(vec);	   // lookup id by Vector3d
	name =index.idToName(id);
	assertEquals("Incorrect Id returned","N210002001002001",name);

	vec.set(275,-85);
	id = index.lookupId(vec);	   // lookup id by Vector3d
	name =index.idToName(id);
	assertEquals("Incorrect Id returned","S310002002230232",name);
	float time = System.currentTimeMillis() - start ;
	System.err.println("Test took "+time + " miliseconds");
}

	/** Executes the test case */
	public static void main(String[] argv) {
	String[] testCaseList = {TestHTMindexImp.class.getName()};
//	junit.swingui.TestRunner.main(testCaseList);
	}
}
