// Copyright 2002
// Association for Universities for Research in Astronomy, Inc.,
// Observatory Control System, Gemini Telescopes Project.
//
// $Id: BasicQueryArgs.java,v 1.2 2009/02/20 13:06:56 abrighto Exp $

package jsky.catalog;

import java.util.ArrayList;
import java.util.List;

import jsky.coords.CoordinateRadius;


/**
 * Represents the values of the arguments to a catalog query.
 */
public class BasicQueryArgs implements QueryArgs {

    /** Catalog we are accesing */
    private Catalog _catalog;

    /** Array of parameter values corresponding to the catalog parameters */
    private Object[] _values;

    /** Optional limit on the number of rows returned from a query */
    private int _maxRows;

    /** Optional object id, if searching by object id */
    private String _id;

    /** Optional query region (center position and radius range) for query */
    private CoordinateRadius _region;

    /** Optional query type, which may be used by the catalog to determine the type of query */
    private String _queryType;


    /**
     * Create a BasicQueryArgs object for the given catalog, with no parameter
     * values (or only default values) set.
     */
    public BasicQueryArgs(Catalog catalog) {
        _catalog = catalog;
        int n = _catalog.getNumParams();
        if (n != 0) {
            _values = new Object[n];
            for (int i = 0; i < n; i++) {
                FieldDesc param = _catalog.getParamDesc(i);
                if (param != null)
                    _values[i] = param.getDefaultValue();
                else
                    _values[i] = null;
            }
        }
    }


    /** Set the value for the ith parameter */
    public void setParamValue(int i, Object value) {
        _values[i] = value;
    }

    /** Set the value for the parameter with the given label */
    public void setParamValue(String label, Object value) {
        int n = _catalog.getNumParams();
        for (int i = 0; i < n; i++) {
            FieldDesc param = _catalog.getParamDesc(i);
            if (param != null) {
                String name = param.getName();
                String id = param.getId();
                if ((id != null && id.equalsIgnoreCase(label)) || (name != null && name.equalsIgnoreCase(label))) {
                    setParamValue(i, value);
                    return;
                }
            }
        }
    }

    /** Set the min and max values for the parameter with the given label */
    public void setParamValueRange(String label, Object minValue, Object maxValue) {
        int n = _catalog.getNumParams();
        for (int i = 0; i < n; i++) {
            FieldDesc param = _catalog.getParamDesc(i);
            if (param != null) {
                String name = param.getName();
                String id = param.getId();
                if ((id != null && id.equalsIgnoreCase(label)) || (name != null && name.equalsIgnoreCase(label))) {
                    if (param.isMin()) {
                        setParamValue(i, minValue);
                    } else if (param.isMax()) {
                        setParamValue(i, maxValue);
                    }
                }
            }
        }
    }

    /** Set the int value for the parameter with the given label */
    public void setParamValue(String label, int value) {
        setParamValue(label, Integer.valueOf(value));
    }

    /** Set the double value for the parameter with the given label */
    public void setParamValueRange(String label, double minValue, double maxValue) {
        setParamValueRange(label, Double.valueOf(minValue),Double.valueOf(maxValue));
    }

    /** Set the double value for the parameter with the given label */
    public void setParamValue(String label, double value) {
        setParamValue(label, Double.valueOf(value));
    }


    /** Set the array of parameter values directly. */
    public void setParamValues(Object[] values) {
        _values = values;
    }


    /** Get the value of the ith parameter */
    public Object getParamValue(int i) {
        return _values != null ? _values[i] : null;
    }

    /** Get the value of the named parameter
     *
     * @param label the parameter name or id
     * @return the value of the parameter, or null if not specified
     */
    public Object getParamValue(String label) {
        int n = _catalog.getNumParams();
        for (int i = 0; i < n; i++) {
            FieldDesc param = _catalog.getParamDesc(i);
            if (param != null) {
                String name = param.getName();
                String id = param.getId();
                if ((id != null && id.equalsIgnoreCase(label)) || (name != null && name.equalsIgnoreCase(label)))
                    return getParamValue(i);
            }
        }
        return null;
    }


    /**
     * Get the value of the named parameter as an integer.
     *
     * @param label the parameter label
     * @param defaultValue the default value, if the parameter was not specified
     * @return the value of the parameter
     */
    public int getParamValueAsInt(String label, int defaultValue) {
        Object o = getParamValue(label);
        if (o == null)
            return defaultValue;
        if (o instanceof Number)
            return ((Number) o).intValue();
        if (o instanceof String)
            return Integer.parseInt((String) o);
        return defaultValue;
    }

    /**
     * Get the value of the named parameter as a double.
     *
     * @param label the parameter label
     * @param defaultValue the default value, if the parameter was not specified
     * @return the value of the parameter
     */
    public double getParamValueAsDouble(String label, double defaultValue) {
        Object o = getParamValue(label);
        if (o == null)
            return defaultValue;
        if (o instanceof Number)
            return ((Number) o).doubleValue();
        if (o instanceof String)
            return Double.parseDouble((String) o);
        return defaultValue;
    }

    /**
     * Get the value of the named parameter as a String.
     *
     * @param label the parameter label
     * @param defaultValue the default value, if the parameter was not specified
     * @return the value of the parameter
     */
    public String getParamValueAsString(String label, String defaultValue) {
        Object o = getParamValue(label);
        if (o == null)
            return defaultValue;
        if (o instanceof String)
            return (String) o;
        return o.toString();
    }


    /**
     * Return the object id being searched for, or null if none was defined.
     */
    public String getId() {
        return _id;
    }

    /**
     * Set the object id to search for.
     */
    public void setId(String id) {
        _id = id;
    }


    /**
     * Return an object describing the query region (center position and
     * radius range), or null if none was defined.
     */
    public CoordinateRadius getRegion() {
        return _region;
    }

    /**
     * Set the query region (center position and radius range) for
     * the search.
     */
    public void setRegion(CoordinateRadius region) {
        _region = region;
    }


    /** Return the catalog we are accesing. */
    public Catalog getCatalog() {
        return _catalog;
    }


    /**
     * Return an array of SearchCondition objects indicating the
     * values or range of values to search for.
     */
    public SearchCondition[] getConditions() {
        if (_values == null)
            return null;

        int n = _catalog.getNumParams();
        List<SearchCondition> v = new ArrayList<SearchCondition>(n);
        for (int i = 0; i < n; i++) {
            if (_values[i] != null) {
                FieldDesc p = _catalog.getParamDesc(i);
                FieldDesc nextP = null;
                if (i+1 < n) {
                    nextP = _catalog.getParamDesc(i+1);
                }
                if (p != null) {
                    if (_values[i] instanceof ValueRange) {
                        ValueRange r = (ValueRange) _values[i];
                        v.add(new RangeSearchCondition(p, r.getMinValue(), r.isMinInclusive(),
                                                       r.getMaxValue(), r.isMaxInclusive()));
                    } else if (_values[i] instanceof Comparable) {
                        // The min/max param handling assumes that the max param follows the min param
                        if (p.isMin()) {
                            // parameter has two values: min and max: save the min value here
                            if (nextP != null && nextP.isMax() && nextP.getId().equals(p.getId())) {
                                v.add(new RangeSearchCondition(p, (Comparable)_values[i], (Comparable)_values[i+1]));
                                i++; // skip the next param, which is the max value for this min value param
                            } else {
                                // Only a min value was specified
                                v.add(new RangeSearchCondition(p, (Comparable)_values[i], null));
                            }
                        } else if (p.isMax()) {
                            // If this wasn't handled above, there must be no min value
                            v.add(new RangeSearchCondition(p, null, (Comparable)_values[i]));
                        } else {
                            v.add(new ValueSearchCondition(p, (Comparable) _values[i]));
                        }
                    } else if (_values[i] instanceof Object[]) {
                        v.add(new ArraySearchCondition(p, (Object[]) _values[i]));
                    }
                }
            }
        }

        // convert result vector to array for return
        n = v.size();
        if (n == 0)
            return null;

        SearchCondition[] sc = new SearchCondition[n];
        v.toArray(sc);
        return sc;
    }

    /** Returns the max number of rows to be returned from a table query */
    public int getMaxRows() {
        return _maxRows;
    }

    /** Set the max number of rows to be returned from a table query */
    public void setMaxRows(int maxRows) {
        _maxRows = maxRows;
    }


    /** Returns the query type (an optional string, which may be interpreted by some catalogs) */
    public String getQueryType() {
        return _queryType;
    }

    /** Set the query type (an optional string, which may be interpreted by some catalogs) */
    public void setQueryType(String queryType) {
        _queryType = queryType;
    }


    /** Return a string of the form: arg=value&arg=value, ...*/
    public String toString() {
        SearchCondition[] sc = getConditions();
        StringBuffer sb = new StringBuffer();
        for (int i = 0; i < sc.length; i++) {
            sb.append(sc[i].toString());
            if (i < sc.length - 1)
                sb.append("&");
        }
        return sb.toString();
    }
}
