package edu.jhu.htm.core;
import java.util.*;
import java.io.*;
/**********************************************************************

 These flags are used to keep track od nodes in Convex when 
 an intersect is being performed.

<p>
 There are two markups: node markup and vertex markup. Node markups are
<pre>
	dONTKNOW
	pARTIAL
	sWALLOWED
	fULL
	rEJECT
<pre>

Vertex markups are
<pre>
	vTrue
	vFalse
	vUndef
</pre>

<pre>
 Current Version
 ===============
 ID:	$Id: Markup.java,v 1.2 2003/02/19 15:46:11 womullan Exp $
 Revision: 	$Revision: 1.2 $
 Date/time:	$Date: 2003/02/19 15:46:11 $
</pre>
*/


public interface Markup {
   public static final short	 dONTKNOW = 0;
   public static final short	 pARTIAL = 1;
   public static final short	 fULL = 2;
   public static final short	 rEJECT = 3;
   public static final short	 vTrue = 1;
   public static final short	 vFalse = 2;
   public static final short	 vUndef = 0;
};





/**********************************************************************
* Revision History
* ================
*
* $Log: Markup.java,v $
* Revision 1.2  2003/02/19 15:46:11  womullan
* Updated comments mainly to make java docs better
*
* Revision 1.1  2003/02/14 16:46:00  womullan
*  Newly organised classes in the new packages
*
************************************************************************/
