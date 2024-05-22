package edu.jhu.util;
/**
* Simlpe class to hold max min and running ave of a long value
* @version      $Revision: 1.1 $
* @author  wil
**/
////////////////////////////////////////////
// Class definition and attributes
////////////////////////////////////////////

public class LongStatistic {
    protected long count = 0;
    protected long last;
    protected long min = Long.MAX_VALUE;
    protected long max = 0;
    protected long ave;

    public void addValue(long val) {
        ave= ((count*ave) + val)/(count+1);
        count++;
        if (val > max) {
            max = val;
        }
        if (val < min ) {
            min = val;
        }
    }

    public long getAve() {
        return ave;
    }
    public long getCount() {
        return count;
    }
    public long getLast() {
        return last;
    }
    public long getMax() {
        return max;
    }
    public long getMin() {
        return min;
    }
    public void getAveMaxMin(StringBuffer sb) {
        sb.append(ave);
        sb.append("\t");
        sb.append(max);
        sb.append("\t");
        sb.append(min);
    }

	public String toString() {
		StringBuffer sb = new StringBuffer();
		getAveMaxMin(sb);
		return sb.toString();
	}

}
