package edu.jhu.htm.core.test;

import edu.jhu.htm.core.*;

import junit.framework.*;

/** JUnitTest case for class: edu.jhu.htm.core.Vector3d */
public class TestVector3d extends TestCase {

	public TestVector3d(String _name) {
	super(_name);
	}
	public static Test suite() {
		 return new TestSuite(TestHTMfunc.class);
	 }

	public void testVector() throws Exception {
		Vector3d v = new Vector3d(1,1,1);
		System.err.println(v.ra()+" "+v.dec());
		Vector3d v2 = new Vector3d(v.ra(),v.dec());

		assertEquals("X is wronmg",v2.x(),v.x(),0.001);
		assertEquals("Y is wronmg",v2.y(),v.y(),0.001);
		assertEquals("Z is wronmg",v2.z(),v.z(),0.001);
	}
	/** Executes the test case */
	public static void main(String[] argv) {
	String[] testCaseList = {TestVector3d.class.getName()};
//	junit.swingui.TestRunner.main(testCaseList);
	}
}