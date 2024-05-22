package edu.jhu.htm.parsers;
import edu.jhu.htm.core.Domain;
import java.io.IOException;
/**********************************************************************

 Simple interface for parsers which parse files or strings into domains.
<pre>
 Current Version
 ===============
 ID:	$Id: DomainParser.java,v 1.2 2003/02/19 15:46:11 womullan Exp $
 Revision: 	$Revision: 1.2 $
 Date/time:	$Date: 2003/02/19 15:46:11 $
</pre>

 @author wil
 @revison $Revision: 1.2 $

*/
public interface DomainParser {
	    Domain parseFile(String fname) throws ParseException ,IOException;
	    Domain parseString(String domain) throws ParseException, IOException;
}

/**********************************************************************
 * Revision History
 * ================
 *
 * $Log: DomainParser.java,v $
 * Revision 1.2  2003/02/19 15:46:11  womullan
 * Updated comments mainly to make java docs better
 *
 * Revision 1.1  2003/02/14 16:46:00  womullan
 *  Newly organised classes in the new packages
 *
 */
