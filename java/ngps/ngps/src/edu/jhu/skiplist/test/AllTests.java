package edu.jhu.skiplist.test;

import junit.framework.*;

public class AllTests extends TestCase {

  public AllTests(String s) {
	super(s);
  }

  public static Test suite() {
	TestSuite suite = new TestSuite();
	suite.addTest(TestSkipList.suite());
	return suite;
  }
}
