package edu.jhu.htm.client;
import java.text.DecimalFormat;
import java.io.*;
import edu.jhu.htm.core.*;

/**
 * <p>Simple JSP bean for doing intersect with triangle or rectangle </p>
<pre>
 Current Version
 ===============
 ID:	$Id: IntersectBean.java,v 1.2 2003/02/19 15:46:11 womullan Exp $
 Date/time:	$Date: 2003/02/19 15:46:11 $
</pre>
 @author not attributable
 @version $Revision: 1.2 $
**********************************************************************/

public class IntersectBean {
	protected double[] RAs,Decs;
	protected Vector3d[] vecs = null;
	protected int level,olevel;
	protected String htmId;
	protected long htmIdNumber;
	protected HTMrange range = new HTMrange();
	protected boolean symbolic = true;
	protected boolean expand;

	public void setExpand(String expand) {
		this.expand = expand.startsWith("t") || expand.startsWith("T");
	}
	public void setExpand(boolean expand) {
		this.expand = expand;
	}

	public boolean isExpand() {
		return expand;
	}

	public void setHtmId(String htmId) {
		this.htmId = htmId;
	}

	public void setRAs(double[] RA) {
		this.RAs = RA;
	}

	public void setDecs(double[] Decs) {
		this.Decs = Decs;
	}

	public void setRAs(String[] RA) {
		if (RA[3].length() > 0) {
			this.RAs = new double[RA.length];
		} else {
			this.RAs= new double[3];
		}
		convertDouble(RA,this.RAs);
	}

	public void setDecs(String[] Decs) {
		if (Decs[3].length() > 0) {
			this.Decs = new double[Decs.length];
		} else {
			this.Decs = new double[3];
		}
		convertDouble(Decs,this.Decs);
	}

	public void setLevel(int level) {
		this.level = level;
		olevel = level;
	}

	public double[] getRAs() {
		return RAs;
	}

	public double[] getDecs() {
		return Decs;
	}

	public long getLevel() {
		return level;
	}

	public void computeIntersect() {
		fillVecs();
		HTMindexImp idx = new HTMindexImp(level,3);
		Convex conv = null;
		if (vecs.length == 3) { // Triangle
			conv = new Convex(vecs[0],vecs[1],vecs[2]);
		} else { // rectangle
			conv = new Convex(vecs[0],vecs[1],vecs[2],vecs[3]);
		}
		range.purge();
		conv.setOlevel(olevel);
		conv.intersect(idx,range,false);
	}

	public void outputHtms(Writer out) {
		try {
			if (expand) {
				HTMrangeIterator iter = new HTMrangeIterator(range,symbolic);
				while (iter.hasNext()) {
					out.write(iter.next().toString());
					out.write('\n');
					out.flush();
				}
			} else {
				out.write(range.toString(symbolic));
				out.write('\n');
				out.flush();
			}
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	public static final void convertDouble(String[] s, double[] v) {
		for (int i =0; i < v.length; i++) {
			v[i] = Double.parseDouble(s[i].trim());
		}
	}

	/* take the RAs and DECS and make spatial vectorss */
	protected void fillVecs() {
		vecs = new Vector3d[RAs.length];
		for (int v = 0 ; v < RAs.length; v++) {
			vecs[v]= new Vector3d(RAs[v],Decs[v]);
		}
	}

}

/**********************************************************************
* Revision History
* ================
*
* $Log: IntersectBean.java,v $
* Revision 1.2  2003/02/19 15:46:11  womullan
* Updated comments mainly to make java docs better
*
* Revision 1.1  2003/02/14 16:46:00  womullan
*  Newly organised classes in the new packages
*
* Revision 1.1  2003/02/07 19:36:59  womullan
*  new jsp methods for intersect
*
*
************************************************************************/

