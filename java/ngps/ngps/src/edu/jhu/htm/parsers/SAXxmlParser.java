package edu.jhu.htm.parsers;

import java.io.IOException;
import java.io.FileReader;
import org.xml.sax.XMLReader;
import org.xml.sax.Attributes;
import org.xml.sax.InputSource;
import org.xml.sax.helpers.XMLReaderFactory;
import org.xml.sax.helpers.DefaultHandler;
import java.io.*;
import java.util.*;

import edu.jhu.htm.core.*;
import edu.jhu.htm.geometry.*;
/**********************************************************************

SAX parser for reading XML HTM files/strings and returning a domain.
Grammer still to be finalised
<pre>
 Current Version
 ===============
 ID:	$Id: SAXxmlParser.java,v 1.3 2003/02/19 15:46:11 womullan Exp $
 Revision: 	$Revision: 1.3 $
 Date/time:	$Date: 2003/02/19 15:46:11 $
</pre>

 @author wil
 @revison $Revision: 1.3 $
*/

public class SAXxmlParser extends DefaultHandler implements DomainParser {
	Domain d = null ; // this is the one we will return .
	protected  List points = null; // Vector3d list of points
	protected Convex convex;
	protected double radius;

	/** default to xerces - you may override with org.xml.sax.driver */
	static {
		if (System.getProperty("org.xml.sax.driver") == null) {
			System.setProperty("org.xml.sax.driver",
							   "org.apache.xerces.parsers.SAXParser");
		}
	}
	public SAXxmlParser () {
        super();
	}

	/** load and parse the given file */

    public Domain parseFile(String fname) throws ParseException, IOException {
        Reader in = new FileReader (fname);
		return parse(in);

    }
	/** parse the given string */
    public Domain parseString(String domain) throws ParseException , IOException{
		Reader in = new StringReader (domain);
		return parse(in);
    }

	/** parse input from the reader */
	public Domain parse(Reader in) throws ParseException, IOException{
		BufferedReader bin = new BufferedReader (in);
        SAXxmlParser handler = null;
        try {
            points = new ArrayList();
            d = new Domain(); // replace any old one
            XMLReader xr = XMLReaderFactory.createXMLReader();
            handler = this;
            xr.setContentHandler(handler);
            xr.setErrorHandler(handler);
            InputSource is = new InputSource(bin);
            xr.parse(is);
        } catch (org.xml.sax.SAXException e) {
            e.printStackTrace();
            throw new ParseException(e.getMessage());
        }
        return d;
	}




	////////////////////////////////////////////////////////////////////
	// Event handlers.
	////////////////////////////////////////////////////////////////////


	public void startDocument () {
        System.out.println("Starting parse");
	}


	public void endDocument () {
	System.out.println("Finished Parser");
	}


    /** names come in name - .*/
	public void startElement (String uri, String name,
				  String qName, Attributes atts)
	{
		System.out.println("Start element: {" + uri + "}" + name);
		try {
            int tag = XMLtags.getTag(name);
            switch(tag) {
                case XMLtags.POINT : handlePoint(uri,name,qName,atts); break;
                case XMLtags.CONSTRAINT : handleConstraint(uri,name,qName,atts); break;
                case XMLtags.CONVEX : handleConvex(uri,name,qName,atts); break;
                case XMLtags.RADIUS : handleRadius(uri,name,qName,atts); break;
            };
		} catch (ParseException pe) {
			System.err.println(pe);
		}
	}


	/** put r att value in radius variable so we can make a cirle later */
	public void handleRadius(String uri, String name,
				  String qName, Attributes atts) {
		radius = getDouble(uri,"r",atts);
	}
	// grab x,y,z, and d make a contraint with them and add to current convex */
	public void handleConstraint(String uri, String name,
				  String qName, Attributes atts) {
		double x  = getDouble(uri,"x",atts);
		double y  = getDouble(uri,"y",atts);
		double z  = getDouble(uri,"z",atts);
		double d  = getDouble(uri,"d",atts);
		Constraint c = new Constraint(x,y,z,d);
		convex.add(c);
	}
	// grab coords make a vector3d add it to points List */
	public void handlePoint(String uri, String name,
				  String qName, Attributes atts) {
		double ra  = getDouble(uri,"ra",atts);
		double dec  = getDouble(uri,"dec",atts);
		Vector3d v = new Vector3d(ra,dec);
		points.add(v);
	}

	/** Create a convex to put contraints into  - its java so
	 * we can alreadsy add it to the domain and not worry about the end tag.
	 */
	public void handleConvex(String uri, String name, String qName,Attributes atts) {
		int ind = 0;
		convex = new Convex();
		d.add(convex);
	}

	/** here we actually construct rectangels etc ...
	 * points should have been gatherd and be waiting in points
	 */
	public void endElement (String uri, String name, String qName)
	{
		try {
            int tag = XMLtags.getTag(name);
            switch(tag) {
                case XMLtags.RECT : handleRect(uri,name,qName); break;
                case XMLtags.CIRCLE : handleCircle(uri,name,qName); break;
                case XMLtags.CHULL : handleChull(uri,name,qName); break;
            };
		} catch (ParseException pe) {
			System.err.println(pe);
		}
	}


	// make a rectangle using points - get the convex - add to damain */
	public void handleRect(String uri, String name, String qName) {
		int ind = 0;
		Vector3d tl = (Vector3d)points.get(ind++);
		Vector3d br = (Vector3d)points.get(ind++);
		points.clear();// we have used them
		ConvexProducer r = new Rect(tl.ra(),tl.dec(),br.ra(),br.dec());
		d.add(r.getConvex());
	}

	// make a circle using the point in points - get the convex - add to damain */
	public void handleCircle(String uri, String name, String qName) {
		int ind = 0;
		Vector3d c = (Vector3d)points.get(ind++);
		points.clear();// we have used them
		ConvexProducer r = new Circle(c.ra(),c.dec(),radius);
		d.add(r.getConvex());
	}

	// make a circle using the point in points - get the convex - add to damain */
	public void handleChull(String uri, String name, String qName) {
		int ind = 0;
		Chull chull = new Chull();
		Iterator iter = points.iterator();
		while (iter.hasNext()) {
			chull.add((Vector3d)iter.next());
		}
		d.add(chull.getConvex());
		points.clear();// we have used them
	}

	/** allways pass lower case name this tries name and name.upper case
	 * to get the value
	 */
	public static double getDouble(String uri, String name, Attributes atts) {
        int ind = atts.getIndex(uri,name);
        if (ind< 0) ind = atts.getIndex(uri,name.toUpperCase());
        double x = Double.parseDouble(atts.getValue(ind));
		return x;
	}

	public void characters (char ch[], int start, int length)
	{
	System.out.print("Characters:    \"");
	for (int i = start; i < start + length; i++) {
		switch (ch[i]) {
		case '\\':
		System.out.print("\\\\");
		break;
		case '"':
		System.out.print("\\\"");
		break;
		case '\n':
		System.out.print("\\n");
		break;
		case '\r':
		System.out.print("\\r");
		break;
		case '\t':
		System.out.print("\\t");
		break;
		default:
		System.out.print(ch[i]);
		break;
		}
	}
	System.out.print("\"\n");
	}

}


