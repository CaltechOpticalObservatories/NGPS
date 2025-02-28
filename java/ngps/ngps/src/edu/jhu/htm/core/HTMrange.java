package edu.jhu.htm.core;

import edu.jhu.skiplist.*;
import java.text.DecimalFormat;
/**********************************************************************
Handles a range of HTM ids using skip lists. This is a neat way to
handle large numbers of HTMids. Basically not all ids are stored rather a
set of ranges of ids are kept whereby all ids in the range are in the set.
This also allows for ranges of different depths to be included in the same list if so required.
<pre>
 Current Version
 ===============
 ID:	$Id: HTMrange.java,v 1.3 2003/03/05 19:22:24 womullan Exp $
 Revision: 	$Revision: 1.3 $
 Date/time:	$Date: 2003/03/05 19:22:24 $
</pre>
@author George Fekete java port by wil
@version $Revision $
*/
public class  HTMrange {
	public static final int LOWS	=1;
	public static final int HIGHS	=2;
	public static final int INSIDE	=1;
	public static final int OUTSIDE  =-1;
	public static final int INTERSECT =0;
	public static final int GAP_HISTO_SIZE=10000;

	public static final float SKIP_PROB=(0.5f);
	public static final int InclOutside = 0;
	public static final int InclInside = 1;
	public static final int InclLo = 1;/* number is on low end of an interval **/
	public static final int InclHi = 1;/* number is on high end of an interval **/
	public static final int InclAdjacentXXX = 1;

	protected SkipList my_los;
	protected  SkipList my_his;

	public void finalize(){
		purge();
		my_los = null;
		my_his = null;
	};

	public HTMrange() {
	  my_los = new SkipList(SKIP_PROB);
	 my_his = new SkipList(SKIP_PROB);
	}

	/** this is not imlemented properly */
	public int compare(HTMrange  other) {
	  int rstat = 0;
	  if (my_los.getLength() == other.my_los.getLength()){
		rstat = 1;
	  }
	  return rstat;
	}

	/**
	Check if the range a to b is contained in this set.
	If both lo and hi are inside some range,
	and the range numbers agree...
	**/
	public int isIn(long a, long b) {
	  long GH_a, GL_a, SH_a, SL_a;
	  long GH_b, GL_b, SH_b, SL_b;
	  long param[] = new long[8];
	  int i;
	  int rstat = 0;
	  i = 0;
	  param[i++] = GL_a = my_los.findMAX(a);
	  param[i++] = GH_a = my_his.findMAX(a);

	  param[i++] = SL_a = my_los.findMIN(a);
	  param[i++] = SH_a = my_his.findMIN(a);

	  param[i++] = GL_b = my_los.findMAX(b);
	  param[i++] = GH_b = my_his.findMAX(b);

	  param[i++] = SL_b = my_los.findMIN(b);
	  param[i++] = SH_b = my_his.findMIN(b);

	  if(GH_a < GL_a && GL_b < GH_b){
		rstat = 0;
	  } else if (GL_b == a && SH_a == b){
		rstat = 1;
	  } else if (GL_b > GL_a) {
		rstat = 0;
	  } else if (GH_a < GL_a) {
		rstat = 1;
	  } else if (SL_a == b) {
		rstat = 0;
	  } else {
		rstat = -1;
	  }

	  return rstat;
	}

	/** does the range contain this key
	 * calls inside and simplifies answer to boolean
	 */
	public boolean isIn(long key) {
	  int incl;
	  boolean rstat = false;
	  incl = tinside(key);
	  rstat = (incl != InclOutside);
	  return rstat;
	}


	/**
	* Check if the range contains the given range.
	* for each range in otheRange, see if there is an intersection,
	* total incluwsion or total exclusion
	**/
	public int isIn(HTMrange  otherRange) {
	  HTMrange o = otherRange;
	  long a, b;
	  int rel, sav_rel;
	  o.reset();			// the builtin lows and highs lists

	  // Inside, Outside, Intersect.
	  sav_rel = rel = -2;			// nothing

	  while((a = o.my_los.getkey()) > 0){
		b = o.my_his.getkey();

		rel = isIn(a, b);

		// decide what type rel is, and keep looking for a
		// different type

		if (sav_rel != -2){ // first time
		  switch (rel) {
		  case INTERSECT:
		break;
		  case OUTSIDE:
		  case INSIDE:
		if (sav_rel != rel){
		  // some out, some in, therefore intersect
		  break;
		}
		  }
		}
		sav_rel = rel;
		o.my_his.step();
		o.my_los.step();
	  }
	  return rel;
	}

	/** check if mid is included in this set */
	public int tinside(long mid)
	{
	  // clearly out, inside, share a bounday, off by one to some boundary
	  int result, t1, t2;
	  long GH, GL, SH, SL;

	  GH = my_his.findMAX(mid);
	  GL = my_los.findMAX(mid);

	  if (GH < GL) { // GH < GL
		t1 = InclInside;
	  } else {
		t1 = InclOutside;
	  }

	  SH = my_his.findMIN(mid);
	  SL = my_los.findMIN(mid);
	  if (SH < SL) { // SH < SL
		t2 = InclInside;
	  } else {
		t2 = InclOutside;
	  }
	  if (t1 == t2){
		result = t1;
	  } else {
		result = (t1 == InclInside ? InclHi : InclLo);
	  }

	  return result;
	}
	/**
	* Add the given range to this set. This may subsume other ranges.
	* Or it may be contained in an existing range
	*
	**/
	public void mergeRange(long lo, long hi) {

		int lo_flag = tinside(lo);
		int hi_flag = tinside(hi);

	  // delete all nodes (key) in los where lo < key < hi
	  my_his.freeRange(lo, hi);
	  my_los.freeRange(lo, hi);

	  // add if not inside a pre-existing interval

	  if (lo_flag == InclHi) {
	  } else if (lo_flag == InclLo ||
		   (lo_flag ==  InclOutside)
		   ){
		my_los.insert(lo, 33);
	  }

	  if (hi_flag == InclLo){
	  }
	  else if (hi_flag == InclOutside ||
			 hi_flag == InclHi) {
		my_his.insert(hi, 33);
	  }
	}

	/** simply add this range - no checks performed
	 *  the lo is added to the lows and the high is added to the highs
	*/
	public void addRange(long lo, long hi) {
	  my_los.insert(lo, (long) 0);
	  my_his.insert(hi, (long) 0);
	  return;
	}

	/**
	 * checks that the gap between intervals is at least gap.
	 * hence  for any hi the gap from the previosu low is at least GAP.
	 * This can be used to maintain a maximum number of ranges while
	 * introducing some redundancy.
	 * Use bestGap to do a calculation for this.
	**/
	public void defrag(long gap) {
	  long hi, lo, save_key;
	  my_los.reset();
	  my_his.reset();

	  // compare lo at i+1 to hi at i.
	  // so step lo by one before anything

	  my_los.step();
	  while((lo = my_los.getkey()) >= 0){
		hi = my_his.getkey();
		// System.out.println( "compare " + hi + "---" + lo );

		// If no gap, then if (hi + 1 == lo) is same as if (hi + 1 + gap  == lo)

		if (hi + (long) 1 + gap >= lo){

		  // merge to intervals, delete the lo and the high
		  // is iter pointing to the right thing?
		  // need setIterator(key past the one you are about to delete

		  // System.out.println( "Will delete " + lo + " from lo " );

		  // Look ahead, so you can set iter to it

		  my_los.step();
		  save_key = my_los.getkey();
		  my_los.delete(lo);
		  // System.out.println( "Deleted " + lo + " from lo " );
		  if (save_key >= 0) {
			my_los.search(save_key, 1); //resets iter for stepping
		  }
		  // System.out.println( "Look ahead to " + save_key  );

		  // System.out.println( "Will delete " + hi + " from hi " );
		  my_his.step();
		  save_key = my_his.getkey();
		  my_his.delete(hi);
		  // System.out.println( "Deleted " + hi + " from hi " );
		  if (save_key >= 0){
		my_his.search(save_key, 1); //resets iter for stepping
		  }
		  // System.out.println( "Look ahead to " + save_key  );

		} else {
		  my_los.step();
		  my_his.step();
		}
	  }
	  // System.out.println( "DONE looping"  );
	  // my_los.list(System.out.println());
	  // my_his.list(System.out.println());
	}

	/** merge contiguous ranges */
	public void defrag() {
	  long hi, lo, save_key;
	  my_los.reset();
	  my_his.reset();

	  // compare lo at i+1 to hi at i.
	  // so step lo by one before anything

	  my_los.step();
	  while((lo = my_los.getkey()) >= 0){
		hi = my_his.getkey();
		// System.out.println( "compare " + hi + "---" + lo );

		if (hi + 1 == lo){

		  // merge to intervals, delete the lo and the high
		  // is iter pointing to the right thing?
		  // need setIterator(key past the one you are about to delete

		  // Look ahead, so you can set iter to it

		  my_los.step();
		  save_key = my_los.getkey();
		  my_los.delete(lo);
		  // System.out.println( "Deleted " + lo + " from lo " );
		  if (save_key >= 0) {
		my_los.search(save_key, 1); //resets iter for stepping
		  }

		  my_his.step();
		  save_key = my_his.getkey();
		  my_his.delete(hi);
		  if (save_key >= 0){
		my_his.search(save_key, 1); //resets iter for stepping
		  }

		} else {
		  my_los.step();
		  my_his.step();
		}
	  }
	}
	/** not implemented */
	public void levelto(int depth) {
	  return;
	}

	public void purge() {
	  my_los.freeRange(-1, Integer.MAX_VALUE);
	  my_his.freeRange(-1, Integer.MAX_VALUE);
	}

	/** reset the hi and lo iterators */
	public void reset() {
	  my_los.reset();
	  my_his.reset();

	}

	/**  return the number of ranges*/
	public int nranges() {
	  long lo, hi;
	  int n_ranges;
	  n_ranges = 0;
	  my_los.reset();
	  my_his.reset();

	  while((lo = my_los.getkey()) > 0){
		n_ranges++;
		hi = my_his.getkey();
		my_los.step();
		my_his.step();
	  }
	  return n_ranges;
	}


	/** return the smallest gapsize at which rangelist would be smaller
 	  * than  desired size
	*/
	public long bestgap(int desiredSize)
	{
	  SkipList sortedgaps = new SkipList(SKIP_PROB);
	  long gapsize = 0;
	  long key;
	  long val;

	  long lo, hi, oldhi = 0, del;

	  int n_ranges, left_ranges;

	  n_ranges = 0;
	  my_los.reset();
	  my_his.reset();

	  while((lo = my_los.getkey()) > 0){
		hi = my_his.getkey();
		n_ranges++;
		if (oldhi > 0){
		  del = lo - oldhi - (long) 1;
		  val = sortedgaps.search(del);
		  if (val == SkipList.NOT_FOUND) {
		// System.err.println( "Insert " + del + ", " + 1 );
		sortedgaps.insert(del, 1); // value is number of times this gapsize appears
		  } else {
		// System.err.println( "Adding " + del + ", " + val+1 );
		sortedgaps.insert(del, (long) (val+1));
		  }
		}
		oldhi = hi;
		my_los.step();
		my_his.step();
	  }
	  if (n_ranges <= desiredSize) // no need to do anything else
		return 0;

	  // increase gapsize until you find that
	  // the if you remove the sum of all
	  // gapsizes less than or equal to gapsize
	  // leaves you with fewer than the deisred number of ranges

	  left_ranges = n_ranges;
	  sortedgaps.reset();
	  while((key = sortedgaps.getkey()) >= 0){
		gapsize = key;
		val = sortedgaps.getvalue();
		// System.err.println( "Ranges left " + left_ranges);
		left_ranges -= (long) val;
		if (left_ranges <= desiredSize)
		  break;
		sortedgaps.step();
	  }

	  sortedgaps = null;
	  return gapsize;
	}

	public long stats(int desiredSize)
	{
	  long lo, hi;
	  long oldhi = -1;
	  int del;

	  long max_gap = 0;

	  long histo[] = new long[GAP_HISTO_SIZE];
	  int i, huges, n_ranges;
	  long cumul[] = new long[GAP_HISTO_SIZE];
	  long bestgap = 0;
	  boolean keeplooking;
	  System.err.println( "STATS=========================" );
	  for(i=0; i<GAP_HISTO_SIZE; i++){
		cumul[i] = histo[i] = 0;
	  }
	  n_ranges = 0;
	  huges = 0;
	  max_gap = 0;
	  my_los.reset();
	  my_his.reset();

	  while((lo = my_los.getkey()) > 0){
		// System.err.println( "Compare lo = "  + lo);
		n_ranges++;
		hi = my_his.getkey();
		if (oldhi > 0){

		  del = (int)(lo - oldhi - 1);
		  if (del < GAP_HISTO_SIZE){
		histo[del] ++;
		  } else {
		max_gap = max_gap < del ? del : max_gap;
		huges++;
		  }
		  // System.err.println( " with hi = " + oldhi );
		} else {
		  // System.err.println( " with nothing" );
		}
		oldhi = hi;

		// Look at histogram of gaps


		my_los.step();
		my_his.step();
	  }

	  if (n_ranges <= desiredSize){
		System.err.println( "No work needed, n_ranges is leq desired size: " + n_ranges + " <= " + desiredSize );
		return -1;			// Do not do unnecessary work
	  }

	  keeplooking = true;
	  System.err.println( "Start looking with n_ranges = " + n_ranges );


	  // SUBTRACT FROM n_ranges, the number of 0 gaps, because they
	  // will me merged automatically
	  // VERY IMPORTANT!!!

	  n_ranges -= histo[0];

	  for(i=0; i<GAP_HISTO_SIZE; i++){
		if(i>0){
		  cumul[i] = cumul[i-1];
		}
//	 else {
//	   System.err.println( setw(3) + i + ": " + setw(6) + histo[i] );
//	   System.err.println( ", " + setw(6) + cumul[i] );
//	   System.err.println( " => " + setw(6) + n_ranges - cumul[i]);
//	   System.err.println( endl);
//	 }

		if(histo[i] > 0){
		  cumul[i] += histo[i];
		  System.err.println(  i + ": " + histo[i] );
		  System.err.println( ", " + cumul[i] );
		  System.err.println( " => " +  (n_ranges - cumul[i]));
		  // You can stop looking when desiredSize becomes greater than or
		  // equal to the n_ranges - the cumulative ranges removed

		  keeplooking = (desiredSize < (n_ranges - cumul[i]));
		  if (!keeplooking){ break; }
		  bestgap = i;
		}
	  }
	  return bestgap;
	}


	/* calls toString(false)  catches xception*/
	public String toString()  {
		 try {
			return toString(false);
		} catch (Exception e) {
			return e.toString();
		}
	}

	/** converts the lo,his pairs to HTM names if sybolic is true */
	public String toString(boolean symbolic) throws HTMException {
	  long lo, hi;
	  StringBuffer buffer = new StringBuffer(256);
	  // os + "Start Range " ;
	  my_los.reset();
	  my_his.reset();
	  while((lo = my_los.getkey()) > 0){
		hi = my_his.getkey();
		if (symbolic) {
			buffer.append(HTMfunc.idToName(lo));
			buffer.append(" ");
			buffer.append(HTMfunc.idToName(hi));
		} else {
			buffer.append(lo);
			buffer.append(" ");
			buffer.append(hi);
		}
		buffer.append("\n");
		my_los.step();
		my_his.step();
	  }
	  // os + "End Range ";
	  return buffer.toString();
	}


	/** returns the next hlo,hi pair in the long[]
		this is like an iterator but it can only be used by one thread
		i.e. anyone calling getNext advances the iterator.
		Call reset to reset the iterator to the begining.
		You must call reset before using this the first time - this
		is the only way to ensure you are at the start of the sequence.

		this will return a [0,0] if there is no more data.
	*/
	public long[] getNext() {
		long[] ret = new long[2];
		try {
            ret[0] = my_los.getkey();
		} catch (Throwable t) { // get null pointer if we are empty
		}
		if (ret[0] <= (long) 0){
			ret[0] = (long) 0;
			ret[1] = (long) 0;
		} else {
			ret[1] = my_his.getkey();
			my_his.step();
			my_los.step();
		}
		return ret;
	}

	/** return only his or lows (statics defined on this class) according
	 * to what is passed in what. Symbolic=true converts thes to HTM names
	 */
	public String toString(int what, boolean symbolic) throws Exception{

	 long hi, lo;
	 StringBuffer tmp_buf= new StringBuffer(256);
	 // Though we always print either low or high here,
	 // the code cycles through both skiplists as if
	 // both were printed. Saves code, looks neater
	 // and since it is ascii IO, who cares if it is fast

	 my_los.reset();
	 my_his.reset();

	 while((lo = my_los.getkey()) > 0){
	   hi = my_his.getkey();
	   if (symbolic){
		 String name = HTMfunc.idToName(what == LOWS ? lo : hi);
		 tmp_buf.append(name);
	   } else {
		 tmp_buf.append( what == LOWS ? lo : hi);
	   }
	   tmp_buf.append('\n');

	   my_los.step();
	   my_his.step();
	 }
	 return tmp_buf.toString();
   }

};


/**********************************************************************
* Revision History
* ================
*
* $Log: HTMrange.java,v $
* Revision 1.3  2003/03/05 19:22:24  womullan
* new test small bug fix in HTMrange
*
* Revision 1.2  2003/02/19 15:46:11  womullan
* Updated comments mainly to make java docs better
*
* Revision 1.1  2003/02/14 16:46:00  womullan
*  Newly organised classes in the new packages
*
*
*
************************************************************************/
