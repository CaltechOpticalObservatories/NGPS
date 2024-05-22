// Copyright 2002
// Association for Universities for Research in Astronomy, Inc.,
// Observatory Control System, Gemini Telescopes Project.
//
// $Id: NameValue.java,v 1.1.1.1 2009/02/17 22:24:39 abrighto Exp $

package jsky.util;

import java.io.Serializable;

/**
 * A simple class containing a name and an associated value.
 *
 * @version $Revision: 1.1.1.1 $
 * @author Allan Brighton
 */
public class NameValue implements Serializable {

    private String _name;
    private Object _value;

    public NameValue() { 
    }

    public NameValue(String name, Object value) {
        _name = name;
        _value = value;
    }

    public void setName(String name) {
        _name = name;
    }

    public String getName() {
        return _name;
    }

    public void setValue(Object value) {
        _value = value;
    }

    public Object getValue() {
        return _value;
    }

    public String toString() {
        return _name;
    }
}

