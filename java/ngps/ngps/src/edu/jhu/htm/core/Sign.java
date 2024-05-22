package edu.jhu.htm.core;
import java.util.*;
import java.io.*;
/**********************************************************************

	 Sign: Store the sign of the constraint/convex
<p>
 The sign class is inherited by Constraint and Convex. Assignment and
 copy operators are used in both scopes.
 Valid signs are

<center>
<table border=1>
<tr>
<td><center><b>Value</b></center></td>
<td><center><b>Description</b></center></td>
<td><center><b>Constraint</b></center></td>
<td><center><b>Convex</b></center></td>
</tr>
<tr>
<td><center><b>nEG</b></center></td>
<td><center>negative</center></td>
<td><center><pre>d &lt; 0</pre></center></td>
<td>only negative and zero constraits</td>
</tr>
<tr>
<td><center><b>zERO</b></center></td>
<td><center>zero</center></td>
<td><center><pre>d = 0</pre></center></td>
<td>only zero constraints</td>
</tr>
<tr>
<td><center><b>pOS</b></center></td>
<td><center>positive</center></td>
<td><center><pre>d > 0</pre></center></td>
<td>only positive and zero constraints</td>
</tr>
<tr>
<td><center><b>mIXED</b></center></td>
<td><center>both negative and positive</center></td>
<td><center><pre>--</pre></center></td>
<td>all kinds of constraints, at least one negative and one positive.</td>
</tr>
</table>
</center>
<pre>
 Current Version
 ===============
 ID:	$Id: Sign.java,v 1.3 2003/02/19 15:46:11 womullan Exp $
 Revision: 	$Revision: 1.3 $
 Date/time:	$Date: 2003/02/19 15:46:11 $
</pre>

@author Peter Kunzt - port by wil
@version $Revision $ 
*/

public class  Sign {
	public final static short nEG=0;
	public final static short zERO=1;
	public final static short pOS=2;
	public final static short mIXED=3;
	public short sign_;					// int value

	// Default Constructor
	Sign() {
	sign_ = zERO;
	}

	// Constructor
	Sign(short sign  ) {
	sign_=sign;
	}

	public String printSign() {
	switch (sign_) {
	case nEG:
		return "NEG";
	case pOS:
		return "POS";
	case zERO:
		return "ZERO";
	}
	return "MIXED";
	}
};


/**********************************************************************
* Revision History
* ================
*
* $Log: Sign.java,v $
* Revision 1.3  2003/02/19 15:46:11  womullan
* Updated comments mainly to make java docs better
*
* Revision 1.2  2003/02/14 16:46:00  womullan
*  Newly organised classes in the new packages
*
* Revision 1.1  2003/02/11 18:40:27  womullan
*  moving to new packages and names
*
*
************************************************************************/
