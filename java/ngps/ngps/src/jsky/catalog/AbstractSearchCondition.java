// Copyright 2003
// Association for Universities for Research in Astronomy, Inc.,
// Observatory Control System, Gemini Telescopes Project.
//
// $Id: AbstractSearchCondition.java,v 1.2 2009/03/03 21:55:51 abrighto Exp $

package jsky.catalog;

import java.io.Serializable;


/**
 * An abstract base class for SearchConditions. The derived classes determine
 * the value type.
 */
public abstract class AbstractSearchCondition implements SearchCondition, Serializable {

    private static final long serialVersionUID = 1L;
    
    /** Describes the column or parameter whose values are given here */
    private FieldDesc _fieldDesc;


    /**
     * Create a new AbstractSearchCondition for the given column or parameter description.
     */
    public AbstractSearchCondition(FieldDesc fieldDesc) {
        _fieldDesc = fieldDesc;
    }

    /** Return the column or parameter description. */
    public FieldDesc getFieldDesc() {
        return _fieldDesc;
    }

    /** Return the column or parameter name. */
    public String getName() {
        return _fieldDesc.getName();
    }

    /** Return the column or parameter id. */
    public String getId() {
        String s = _fieldDesc.getId();
        if (s == null)
            s = _fieldDesc.getName();
        return s;
    }

    /**
     * Return a string representation of this class in the form "name=val"
     */
    public String toString() {
        return getId() + "=" + getValueAsString();
    }

    public boolean isRegionArg() {
        return false;  
    }
}

