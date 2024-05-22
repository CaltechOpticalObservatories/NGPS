/*
 * ESO Archive
 *
 * $Id: CatalogDirectory.java,v 1.3 2009/02/23 21:23:43 abrighto Exp $
 *
 * who             when        what
 * --------------  ----------  ----------------------------------------
 * Allan Brighton  2000/01/05  Created
 */

package jsky.catalog;

import java.net.URL;
import java.util.List;

import javax.swing.tree.TreeModel;


/**
 * This defines the interface for a catalog directory, allowing catalogs
 * to be organized in a hierarchy.
 */
public abstract interface CatalogDirectory extends Catalog, TreeModel {

    /**
     * @return the number of catalogs in this directory
     */
    public int getNumCatalogs();

    /**
     * @param i index in catalog list
     * @return the ith catalog in the directory
     */
    public Catalog getCatalog(int i);

    /**
     * @param catalogName the catalog name
     * @return the named catalog, if found in this directory, otherwise null
     */
    public Catalog getCatalog(String catalogName);

    /**
     * Finds a catalog by name, looking recursively in subdirs.
     *
     * @param catalogName the catalog name
     * @return the named catalog, if found in this directory or any of its catalog subdirectories,
     *         otherwise null
     */
    public Catalog findCatalog(String catalogName);

    /**
     * Set the list of catalogs in this catalog directory.
     *
     * @param catalogs the list of catalogs
     */
    public void setCatalogs(List<Catalog> catalogs);

    /**
     * @return a copy of the list of catalogs in this catalog directory.
     */
    public List<Catalog> getCatalogs();

    /**
     * @param cat the target catalog
     * @return the index of the given catalog in the directory
     */
    public int indexOf(Catalog cat);

    /**
     * @return a memory catalog describing the list of catalogs in the directory
     */
    public TableQueryResult getCatalogList();

    /**
     * @param cat the target catalog
     * @return an array of catalogs describing the path to the given catalog or catalog directory.
     */
    public Catalog[] getPath(Catalog cat);

    /**
     * @return the URL of the file describing this catalog directory.
     */
    public URL getURL();

    /**
     * @return true if the catalog directory needs to be given the server URL,
     *         false if it knows where to look.
     */
    public boolean configNeedsUrl();

    /**
     * Add the given catalog to the catalog list. An error message is displayed if the
     * catalog is already in the list. If a separate catalog with the same name is in the list,
     * the user is asked if it should be removed.
     *
     * @param cat tha catalog to add
     */
    public void addCatalog(Catalog cat);

    /**
     * Add the given catalog to the catalog list at the given index. An error message is displayed if the
     * catalog is already in the list. If a separate catalog with the same name is in the list,
     * the user is asked if it should be removed.
     *
     * @param index the index in the catalog list
     * @param cat   the catalog to add
     */
    public void addCatalog(int index, Catalog cat);

    /**
     * Remove the given catalog from the catalog list.
     *
     * @param cat the catalog to remove
     */
    public void removeCatalog(Catalog cat);

    /**
     * Replace the given old catalog with the given new catalog in the catalog list.
     *
     * @param oldCat the catalog to remove
     * @param newCat the catalog to add
     */
    public void replaceCatalog(Catalog oldCat, Catalog newCat);

    /**
     * Move the the given catalog up or down in the list.
     *
     * @param cat the catalog to move
     * @param up  true if the catalog should be moved up in the list
     */
    public void moveCatalog(Catalog cat, boolean up);

    /**
     * Move the the given catalog all the way up or down in the list, as far as possible.
     *
     * @param cat the catalog to move
     * @param up  true if the catalog should be moved up in the list
     */
    public void moveCatalogToEnd(Catalog cat, boolean up);

    /**
     * Save the contents of this catalog directory to make it permanent
     * (for example, in a config file under ~/.jsky3/...).
     */
    public void save();

    /**
     * Attempt to read a catalog subdirectory from the given URL and insert
     * the object for it in the catalog tree.
     *
     * @param url the URL to read
     * @return the new CatalogDirectory
     * @throws RuntimeException if the catalog directory could not be created
     */
    public CatalogDirectory loadSubDir(URL url);

    /**
     * @return a list of name servers (Catalogs with type
     *         equal to "namesvr") to use to resolve astronomical object names.
     */
    public List<Catalog> getNameServers();
}


