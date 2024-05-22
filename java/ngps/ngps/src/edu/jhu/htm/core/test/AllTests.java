package edu.jhu.htm.core.test;

import junit.framework.*;

public class AllTests extends TestCase {

  public AllTests(String s) {
	super(s);
  }

  public static Test suite() {
	TestSuite suite = new TestSuite();
	suite.addTest(TestHTMfunc.suite());
	suite.addTest(TestVector3d.suite());
	suite.addTest(TestHTMrange.suite());
	suite.addTest(TestHTMrangeIterator.suite());
	suite.addTest(TestHTMindexImp.suite());
	return suite;
  }

  /** Executes the test case */
	public static void main(String[] argv) throws Exception{

	String[] testCaseList = {"edu.jhu.htm.core.test.AllTests"};
//	junit.swingui.TestRunner.main(testCaseList);
	}

}
