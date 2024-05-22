package edu.jhu.htm.parsers;

import edu.jhu.htm.core.*;
import java.io.*;

/**********************************************************************

 parse domain files in the old format using HTMio
 see  public read(Reader in,Domain d) in Htmio

<pre>
 Current Version
 ===============
 ID:	$Id: LegacyParser.java,v 1.2 2003/02/19 15:46:11 womullan Exp $
 Revision: 	$Revision: 1.2 $
 Date/time:	$Date: 2003/02/19 15:46:11 $
</pre>
 @author wil
 @revison $Revision: 1.2 $
*/

public class LegacyParser implements DomainParser{
	/** just create a file reader,a domain  and call htmio.read return the domain */
	public Domain parseFile(String fname) throws ParseException ,IOException {
		Reader in = new FileReader (fname);
		BufferedReader bin = new BufferedReader (in);
		Domain d = new Domain();
		Htmio.read(in,d);
		return d;
	}
	/** just create a string reader,a domain  and call htmio.read return the domain */
	public Domain parseString(String domain) throws ParseException , IOException{
		Reader in = new StringReader (domain);
		BufferedReader bin = new BufferedReader (in);
		Domain d = new Domain();
		Htmio.read(in,d);
		return d;

	}


}

/**********************************************************************
 * Revision History
 * ================
 *
 * $Log: LegacyParser.java,v $
 * Revision 1.2  2003/02/19 15:46:11  womullan
 * Updated comments mainly to make java docs better
 *
 * Revision 1.1  2003/02/14 16:46:00  womullan
 *  Newly organised classes in the new packages
 *
 */
