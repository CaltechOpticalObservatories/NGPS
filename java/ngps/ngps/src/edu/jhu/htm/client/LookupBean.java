package edu.jhu.htm.client;

import edu.jhu.htm.core.*;
import java.text.DecimalFormat;
/**
	USE bean for JSP form which does a htm lookup
<pre>
 Current Version
 ===============
 ID:		$Id: LookupBean.java,v 1.2 2003/02/19 15:46:11 womullan Exp $
 Revision:	$Revision $
 Date/time:	$Date: 2003/02/19 15:46:11 $
</pre>
	@author  wil
	@version $Revision $
*/
public class  LookupBean {
	protected double RA,Dec;
	protected int level;
	protected String htmId;
	protected long htmIdNumber;

	public void setHtmId(String htmId) {
		this.htmId = htmId;
	}

	public void setRA(double RA) {
		this.RA = RA;
	}

	public void setDec(double Dec) {
		this.Dec = Dec;
	}

	public void setLevel(int level) {
		this.level = level;
	}

	public double getRA() {
		return RA;
	}

	public double getDec() {
		return Dec;
	}

	public long getLevel() {
		return level;
	}

	public String getHtmId() {
		return htmId;
	}

	public long getHtmIdNumber() {
		return htmIdNumber;
	}
	public String computeHtmId() {
		try {
			htmId = HTMfunc.lookup(RA,Dec,level);
			htmIdNumber = HTMfunc.nameToId(htmId);
		} catch (Exception e) {
			e.printStackTrace();
			return e.getMessage();
		}
		return htmId;
	}
}

/**********************************************************************
* Revision History
* ================
*
* $Log: LookupBean.java,v $
* Revision 1.2  2003/02/19 15:46:11  womullan
* Updated comments mainly to make java docs better
*
* Revision 1.1  2003/02/14 16:46:00  womullan
*  Newly organised classes in the new packages
*
* Revision 1.2  2003/02/05 23:00:07  womullan
* Added new timimig routines and HTM clas to do cc stuff from the c++ package
*
*
************************************************************************/

