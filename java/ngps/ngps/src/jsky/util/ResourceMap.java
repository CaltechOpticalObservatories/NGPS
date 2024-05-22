// Copyright 1999-2001
// Association for Universities for Research in Astronomy, Inc.,
// Observatory Control System, Gemini Telescopes Project.
// See the file COPYRIGHT for complete details.
//
// $Id: ResourceMap.java,v 1.2 2009/02/21 16:43:10 abrighto Exp $
//

package jsky.util;

import java.util.HashMap;
import java.util.Map;

import javax.swing.Icon; 


/**
 * A ResourceMap provdes a map of resources such that frequently used
 * resources, such as images and icons can be reused.
 * <p>
 * Currently, this is a simple {@link HashMap}, but could
 * be evolved to use weak references.
 * <p>
 * Currently only Icon special routines are implemented.
 */
public final class ResourceMap {

    // Private variable to indicate the rough size of the map
    private static final int DEFAULT_CACHE_CAPACITY = 25;
    // The private HashMap
    private Map<String,Icon> _map;

    /**
     * Create a new ResourceMap.
     */
    public ResourceMap() {
        this(DEFAULT_CACHE_CAPACITY);
    }

    /**
     * Create a new ResourceMap with a start up capacity.
     */
    public ResourceMap(int capacity) {
        _map = new HashMap<String, Icon>(capacity);
    }

    /**
     * Checks and returns a cached {@link Icon}.
     *
     * @return the cached <code>Icon</code> or null if not present.
     */
    public Icon getIcon(String iconName) {
        return _map.get(iconName);
    }


    /**
     * Stores an Icon with the given resourceName.
     *
     */
    public void storeIcon(String iconName, Icon icon) {
        // If already present, it's overwritten
        _map.put(iconName, icon);
    }

}
