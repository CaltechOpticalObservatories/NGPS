
package edu.jhu.htm.app;

import java.util.*;
import edu.jhu.htm.core.*;
import edu.jhu.util.*;

/**
 * <p> Time hashing for a set of random points
 *  The assumption here is that looup is like a hash so it
 *  is interesting to see its relative performance.
 *  a Simple has migh append RA and dec and then Hash them.
 * </p>
 * @author wil
 * @version $revision: $
 */

public class TimeHash {
	double[] ras,decs;
	int numlooks = 1000;

	public void initRaDec() {
		 Random ran = new Random(38989898989898L);
		ras = new double[numlooks];
		decs = new double[numlooks];
		for (int i = 0 ; i < numlooks; i++) {
			ras[i] =ran.nextDouble()*360;
			decs[i] =ran.nextDouble()*180 - 90;
		}

	}

	public void timeHash() {
		long st = 0,dur=0;
		int  hash = 0;
		String hin = null;
		st = System.currentTimeMillis();
		for (int rep = 0; rep < ras.length; rep++) {
			hin = ras[rep]+":"+decs[rep];
			hash  = hin.hashCode();
		}
		dur =  System.currentTimeMillis() - st;
		System.out.print(ras.length +" hashesh took "+ dur +" milis " );
		float ave = (float)(dur) / ras.length;
		System.out.print(" Ave "+ave);
		float persec = Float.POSITIVE_INFINITY;
		if (ave > 0) persec = 1000/ave;
		System.out.println(" "+persec+" hashes/second");
	}

	public void run() {
		initRaDec();
		timeHash();
	}

	public TimeHash(int numlooks) {
		this.numlooks = numlooks;
	}

	public static void main(String[] args) {
		TimeHash tl = new TimeHash(10000);
		tl.run();
	}
}
