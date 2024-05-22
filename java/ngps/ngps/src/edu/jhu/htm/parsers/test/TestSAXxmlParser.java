package edu.jhu.htm.parsers.test;

import edu.jhu.htm.core.*;
import edu.jhu.htm.parsers.*;
import java.io.*;
import junit.framework.*;

/** JUnitTest case for class: edu.jhu.htm.parsers.LegacyParser */
public class TestSAXxmlParser extends TestCase {

    public TestSAXxmlParser(String _name) {
	super(_name);
    }
	public static Test suite() {
		 return new TestSuite(TestSAXxmlParser.class);
	 }

	public void testParseMulti() throws Exception {
		String domain = "<area>\n <convex>\n "
					    +"<constraint x=\"0.7\" y=\"0.7\" z=\"0\" d=\"0.89\" />\n "
                        +"</convex>  "
					    +"<circle> <point ra=\"67.9\" dec=\"-8.7\"/> <radius r=\"5\"/> </circle> "
						+"</area>";
		DomainParser p = new SAXxmlParser();
		Domain d = p.parseString(domain);
		assertTrue(d!=null);
		assertEquals("No convexes ",2,d.getNumberOfConvexes());
	}
	public void testParseConvex() throws Exception {
		String domain = "<area>\n <convex>\n "
					    +"<constraint x=\"0.7\" y=\"0.7\" z=\"0\" d=\"0.89\" />\n "
					    +"<constraint x=\"0.7\" y=\"0.6\" z=\"0\" d=\"0.89\" />\n "
					    +"<constraint x=\"0.7\" y=\"0.5\" z=\"0.8\" d=\"0.89\" />\n "
                        +"</convex> </area>";
		DomainParser p = new SAXxmlParser();
		Domain d = p.parseString(domain);
		assertTrue(d!=null);
		assertEquals("No convexes ",1,d.getNumberOfConvexes());
	}

	public void testParseRect() throws Exception {
		String domain = "<area>\n <rect>\n <point ra=\"54\" dec=\"29\" />\n <point ra=\"55\" dec=\"30\"/>\n</rect>\n</area> ";
		DomainParser p = new SAXxmlParser();
		Domain d = p.parseString(domain);
		assertTrue(d!=null);
		assertEquals("No convexes ",1,d.getNumberOfConvexes());
	}

    /** Executes the test case */
    public static void main(String[] argv) {
	String[] testCaseList = {TestSAXxmlParser.class.getName()};
//	junit.swingui.TestRunner.main(testCaseList);
    }
}
