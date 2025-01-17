package edu.jhu.htm.core.test;

import java.util.*;
import java.io.*;
import junit.framework.*;
import edu.jhu.htm.core.*;

/** JUnitTest case for class: edu.jhu.htm.core.HTMindex */
public class TestHTMrange extends TestCase {
	 public static final long[] los = { 10, 230, 100000, 800000 };
	 public static final long[] highs = { 50, 330, 200000, 810000 };

	protected HTMrange range = new HTMrange();
	public TestHTMrange(String _name) {
	super(_name);
	}
	public static Test suite() {
		 return new TestSuite(TestHTMrange.class);
	 }
	 protected void setUp() {
		for (int i = 0; i < los.length; i++) {
			range.addRange(los[i],highs[i]);
		}
	 }
	 protected void tearDown() {
	 }

	 public void testNull() {
		 HTMrange r = new HTMrange();
		 long[] p = r.getNext();
		 assertEquals(p[0],0);
	 }

	public void testIsIn() {
		boolean isin = range.isIn(49);
		assertTrue(" Is in not working for 49",isin);
		isin = range.isIn(200001);
		assertTrue(" Is in not working for 200001",!isin);

	}

	public void testBestGap() {
		long gap = range.bestgap(10);
		assertEquals("Gap for deired > actual ",0,gap);
		gap = range.bestgap(3);
		assertTrue("Gap for deired < actual ",gap>0);
	}
	public void testDefrag() {
		assertEquals("number of ranges", 4,range.nranges());
		range.defrag(); // should do nothing
		assertEquals("number of ranges", 4,range.nranges());
		long gap = range.bestgap(3);
		range.defrag(gap);
		assertEquals("number of ranges", 3,range.nranges());
		gap = range.bestgap(1);
		range.defrag(gap);
		assertEquals("number of ranges", 1,range.nranges());
	}

	public void testPurge() {
		range.purge();
		assertEquals("purge ", 0,range.nranges());
	}
	/** Executes the test case */
	public static void main(String[] argv) {
	String[] testCaseList = {TestHTMrange.class.getName()};
//	junit.swingui.TestRunner.main(testCaseList);
	}
}
