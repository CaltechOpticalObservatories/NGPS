/*
 * ESO Archive
 *
 * $Id: CatalogFactory.java,v 1.3 2009/03/19 17:38:08 abrighto Exp $
 *
 * who             when        what
 * --------------  ----------  ----------------------------------------
 * Allan Brighton  1999/05/17  Created
 */

package jsky.catalog;

import java.util.*;


/**
 * Used to manage access to a known list of catalogs.
 * Catalogs may be registered by name and later searched for by name.
 */
public class CatalogFactory {

    // Maps catalog name to catalog instance
    private static Map<String,Catalog> _map = new HashMap<String, Catalog>();

    // The list of catalogs, unsorted, in the order they were registered
    private static List<Catalog> _list = new ArrayList<Catalog>();

    /**
     * Register the given catalog. The argument may be any object that
     * implements the Catalog interface and will be used for any access
     * to that catalog. Since the catalog may not actually be used, the
     * constructor should not open any connections until needed.
     *
     * @param catalog An object to use to query the catalog.
     * @param overwrite if true, the given catalog object replaces any
     * previously defined catalog with the same name,
     * otherwise only the first catalog registered with a
     * given name is actually registered.
     */
    public static void registerCatalog(Catalog catalog, boolean overwrite) {
        if (_map.containsKey(catalog.getName())) {
            if (overwrite) {
                _map.remove(catalog.getName());
                _list.remove(catalog);
            } else {
                return;
            }
        }
        _map.put(catalog.getName(), catalog);
        _list.add(catalog);
    }


    /**
     * This method returns a Catalog object that can be used to query
     * the given catalog, or null if no such object was found.
     *
     * @param catalogName The name of a registered catalog
     * @return The object to use to query the catalog, or null if not found.
     */
    public static Catalog getCatalogByName(String catalogName) {
        return _map.get(catalogName);
    }

    /**
     * This method returns a list of Catalog objects that have the given type,
     * in the order in which they were registered.
     *
     * @param type The catalog type (as returned by <code>Catalog.getType()</code>)
     * @return the list of Catalog objects found
     */
    public synchronized static List<Catalog> getCatalogsByType(String type) {
        List<Catalog> l = new ArrayList<Catalog>();
        for (Catalog cat : _list) {
            String s = cat.getType();
            if (s != null && s.equals(type)) {
                l.add(cat);
            }
        }

        return l;
    }


    /**
     * Unregister the given catalog, removing it from the list of known
     * catalogs.
     *
     * @param catalog The catalog to be removed from the list.
     */
    public static void unregisterCatalog(Catalog catalog) {
        _map.remove(catalog.getName());
        _list.remove(catalog);
    }
}
