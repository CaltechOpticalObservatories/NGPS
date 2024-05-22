package edu.jhu.htm.parsers.test;

import edu.jhu.htm.core.*;
import edu.jhu.htm.parsers.*;
import java.io.*;
import junit.framework.*;

/** JUnitTest case for class: edu.jhu.htm.parsers.LegacyParser */
public class TestLegacyParser extends TestCase {

    public TestLegacyParser(String _name) {
	super(_name);
    }
	public static Test suite() {
		 return new TestSuite(TestLegacyParser.class);
	 }


	public void testParseString() throws Exception {
		String domain = "#DOMAIN\n 1\n #RECTANGLE_RADEC\n 54 29\n 54 30\n 55 29\n 55 30\n ";
		DomainParser p = new LegacyParser();
		Domain d = p.parseString(domain);
		assertTrue(d!=null);
	}

    /** Executes the test case */
    public static void main(String[] argv) {
	String[] testCaseList = {TestLegacyParser.class.getName()};
//	junit.swingui.TestRunner.main(testCaseList);
    }
}
