package edu.jhu.htm.core.test;

import java.util.Iterator;
import junit.framework.*;
import edu.jhu.htm.core.*;
/** JUnitTest case for class: edu.jhu.htm.core.HTMrangeIterator */
public class TestHTMrangeIterator extends TestCase {

	protected HTMrange range = new HTMrange();
	public TestHTMrangeIterator(String _name) {
	super(_name);
	}
	protected void setUp() {
	   for (int i = 0; i < TestHTMrange.los.length; i++) {
		   range.addRange(TestHTMrange.los[i],TestHTMrange.highs[i]);
	   }
	}
	public static Test suite() {
		 return new TestSuite(TestHTMrangeIterator.class);
	 }


	public void testNext() throws Exception{
		HTMrangeIterator iter = new HTMrangeIterator(range,false);
		Long n = iter.nextAsLong();
		long val = TestHTMrange.los[0];
		assertEquals(" Iterator is wrong" , val, n.longValue());
		// skip to next range almost
		long hiv = TestHTMrange.highs[0];
		for (long i = val; i < hiv; i++) {
			n = iter.nextAsLong();
		}
		assertEquals(" Iterator is wrong" , hiv, n.longValue());
		n = iter.nextAsLong();
		val = TestHTMrange.los[1];
		assertEquals(" Iterator is wrong" , val, n.longValue());


	}
	/** Executes the test case */
	public static void main(String[] argv) {
	String[] testCaseList = {TestHTMrangeIterator.class.getName()};
//	junit.swingui.TestRunner.main(testCaseList);
	}
}
