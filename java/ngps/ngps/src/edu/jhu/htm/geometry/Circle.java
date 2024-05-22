package edu.jhu.htm.geometry;

import edu.jhu.htm.core.*;
/**********************************************************************

This class represents a circle on the sky . It may be constructed in a number of
 ways and it will produce a convex or a domain to represtn the circle.

<pre>
 Current Version
 ===============
 ID:	$Id: Circle.java,v 1.2 2003/02/19 15:46:11 womullan Exp $
 Revision: 	$Revision: 1.2 $
 Date/time:	$Date: 2003/02/19 15:46:11 $
</pre>
 @author wil
 @revison $Revision: 1.2 $
*/
public class Circle extends ConHelper implements ConvexProducer, DomainProducer {

    /** constructor specifying center vector and radius in arcmin*/
    public Circle(Vector3d vec, double arcMinRad) {
        double  d = Math.cos(HTMfunc.Pi * (arcMinRad / 10800.00));
        conv = new Convex();
        Constraint c = new Constraint(vec,d);
        conv.add(c);
    }
    /** constructor specifying center vector and radius in arcmin*/
    public Circle(double x, double y , double z, double arcMinRad) {
        this(new Vector3d(x,y,z),arcMinRad);
    }
    /** constructor specifying center vector and radius in arcmin*/
    public Circle(double ra, double dec ,double arcMinRad) {
        this(new Vector3d(ra,dec),arcMinRad);
    }


}
/**********************************************************************
 * Revision History
 * ================
 *
 * $Log: Circle.java,v $
 * Revision 1.2  2003/02/19 15:46:11  womullan
 * Updated comments mainly to make java docs better
 *
 * Revision 1.1  2003/02/14 16:46:00  womullan
 *  Newly organised classes in the new packages
 *
 */
