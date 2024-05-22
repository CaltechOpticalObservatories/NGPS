package edu.jhu.htm.app;
import edu.jhu.htm.core.*;
import java.io.*;
import java.text.*;
import edu.jhu.util.StringUtil;


/**********************************************************************

 Prints a table of triangle areas v's level number.
 Assumes Triangles at ples and equator give max difference for
 min max so gives min max and average.
<pre>
 Current Version
 ===============
 ID:	$Id: TriangleAreas.java,v 1.2 2003/02/19 15:46:11 womullan Exp $
 Date/time:	$Date: 2003/02/19 15:46:11 $
 Revison $Revision: 1.2 $
</pre>
 @author wil
*/

public class TriangleAreas {
    public static void main(String[] args) throws Exception {
		    System.out.println("Level        Pole                  Inner                 Ave               Ratio         resolution");
          StringBuffer out = new StringBuffer();
		  StringBuffer pole = new StringBuffer("N1");
		  StringBuffer inner = new StringBuffer("N3");
		  double res = 90;
		  for (int lev = 0; lev < 26; lev++) {
			  HTMindex si = new HTMindexImp(lev);

		//	  String pole = si.lookup(new Vector3d(0.0,0.0,1.0));
			//  String equa = si.lookup(new Vector3d(0.0,1.0,0.0));

			  double pa = si.area(pole.toString());
			  double ea = si.area(inner.toString());
			  double ave = (pa+ea)/2;
			  float ratio = (float)(ea /pa);

			  StringUtil.buffer(out,""+lev,5);
			  out.append(" ,");
			  StringUtil.buffer(out,""+pa,22,false);
			  out.append(" ,");
			  StringUtil.buffer(out,""+ea,22,false);
			  out.append(" ,");
			  StringUtil.buffer(out,""+ave,22,false);
			  out.append(" ,");
			  StringUtil.buffer(out,""+ratio,10);
			  out.append(" ,");
			  StringUtil.buffer(out,""+res,22,false);

			   System.out.println(out.toString());
			  out.delete(0,out.length());
			  pole.append(0);
			  inner.append(3);
			  res /=2;

		  }
    }
}
/**********************************************************************
 * Revision History
 * ================
 *
 * $Log: TriangleAreas.java,v $
 * Revision 1.2  2003/02/19 15:46:11  womullan
 * Updated comments mainly to make java docs better
 *
 * Revision 1.1  2003/02/14 16:46:00  womullan
 *  Newly organised classes in the new packages
 *
 */
