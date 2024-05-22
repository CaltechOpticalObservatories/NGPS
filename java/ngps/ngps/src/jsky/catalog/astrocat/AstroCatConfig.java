/*
 * Copyright 2002 Association for Universities for Research in Astronomy, Inc.,
 * Observatory Control System, Gemini Telescopes Project.
 *
 * $Id: AstroCatConfig.java,v 1.6 2009/03/17 22:56:43 abrighto Exp $
 */


package jsky.catalog.astrocat;

import java.io.File;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.List;
import java.util.ArrayList;

import jsky.catalog.AbstractCatalogDirectory;
import jsky.catalog.Catalog;
import jsky.catalog.CatalogDirectory;
import jsky.catalog.CatalogFactory;
import jsky.catalog.FieldDescAdapter;
import jsky.catalog.HTMLQueryResultHandler;
import jsky.catalog.skycat.SkycatConfigFile;
import jsky.util.FileUtil;
import jsky.util.Resources;
import jsky.util.Preferences;
import jsky.util.I18N;
import jsky.util.gui.DialogUtil;

import javax.swing.event.EventListenerList;


/**
 * Reads an AstroCat XML catalog description file and stores information about the catalogs defined there. Since catalog
 * config files may point to other catalog config files to form a hierarchy, this class also implements the
 * CatalogDirectory interface.
 *
 * @author Allan Brighton
 * @version $Revision: 1.6 $
 */
public class AstroCatConfig extends AbstractCatalogDirectory {

    // Used to access internationalized strings (see i18n/gui*.proprties)
    private static final I18N _I18N = I18N.getInstance(AstroCatConfig.class);
    
    // Top level config file
    private static AstroCatConfig _configFile;

    // The URL for the default catalog config file, if set.
    private static URL _defaultURL = null;

    // Set to true to force reloading
    private static boolean _reloadFlag = false;


    /**
     * Creates an empty catalog directory
     */
    public AstroCatConfig() {
        super(_I18N.getString("catalogs"));
        setCatalogs(new ArrayList<Catalog>());
    }

    /**
     * Parse the AstroCat XML catalog config file pointed to by the given URL.
     *
     * @param name the display name for the config file
     * @param url the URL of the config file
     */
    public AstroCatConfig(String name, URL url) {
        super(name);
        setURL(url);
        _load();
    }


    /**
     * Parse the given AstroCat XML file or URL.
     *
     * @param name the display name for the config file
     * @param configFileOrURL the file name or URL of the config file
     */
    public AstroCatConfig(String name, String configFileOrURL) {
        this(name, FileUtil.makeURL(null, configFileOrURL));
    }


    /**
     * Parse the given AstroCat XML file or URL.
     *
     * @param configFileOrURL the file name or URL of the config file
     */
    public AstroCatConfig(String configFileOrURL) {
        this(configFileOrURL, FileUtil.makeURL(null, configFileOrURL));
    }


    /**
     * Parse the AstroCat XML file from the already opened input stream. The URL is passed as a reference.
     *
     * @param url the URL of the config file
     * @param handler used to report HTML errors from the HTTP server
     */
    public AstroCatConfig(URL url, HTMLQueryResultHandler handler) {
        super(new File(url.toString()).getPath());
        setURL(url);
        setHTMLQueryResultHandler(handler);
        _load();
    }


    /**
     * Load an AstroCat XML file from the given URL and store any catalogs found there in the catalogs list.
     */
    private void _load() {
        URL url = getURL();
        if (url == null) {
            return;
        }

        AstroCatXML astroCatXML = new AstroCatXML();
        astroCatXML.parse(url);
        setCatalogs(astroCatXML.getCatalogs());
    }


    /**
     * This method is called once at startup to load the top level catalog directory
     *
     * @return the main catalog directory
     */
    public static CatalogDirectory getDirectory() {
        return getConfigFile();
    }

    /**
     * This method is called to load catalog directories defined in the config file (AtroCat.xml)
     *
     * @param cat the entry from the config file
     * @return the catalog directory
     */
    public static CatalogDirectory getDirectory(AstroCatalog cat) {
        CatalogDirectory dir = getConfigFile();
        dir.setName(cat.getName()); // XXX allow more configuration
        return dir;
    }


    /**
     * If the top level catalog config file has already been loaded, return the object describing the contents,
     * otherwise search for an AstroCat XML catalog config file, load the contents if found, and return the object for it.
     * <p/>
     * First the <em>jsky.catalog.astrocat.config</em> system property is checked. If set, it should be the URL string
     * or file name of the config file.
     * <p/>
     * Next, the file ~/.jsky3/AstroCat.xml is checked. This file is created automatically when the user makes any
     * changes in the catalog configuration or plot symbol settings in the table display/configure window.
     * <p/>
     * Finally, a default URL s used. It may be set by calling "setConfigFile" and defaults to a config file included in
     * this package (as a resource file: AstroCat.xml).
     *
     * @param load if true and the config file has not been loaded yet, load the file, otherwise return an instance
     * with no catalogs (as a placeholder for later: Any listeners added to this instance will be kept
     * when the config file is loaded later on)
     * @return a AstroCatConfig object constructed from the file.
     */
    public static AstroCatConfig getConfigFile(boolean load) {
        boolean reload = _reloadFlag;
        _reloadFlag = false;
        if (_configFile != null && _configFile.getNumCatalogs() != 0 && !reload) {
            return _configFile;
        }

        if (!load && !reload) {
            if (_configFile != null) {
                return _configFile;
            }
            _configFile = new AstroCatConfig(); // return empty instance
            return _configFile;
        }

        // An empty configFile is used as a placeholder during initialization. Copy
        // the listeners to the new instance, so that they can be notified when initialization
        // is complete.
        EventListenerList savedListeners = null;
        if (_configFile != null) {
            savedListeners = _configFile._listenerList;
        }

        List<URL> urls = _getConfigFileLocations();

        for (final URL url : urls) {
            // use the first URL found for the main catalog list, use default URL on error
            try {
                _configFile = new AstroCatConfig(_I18N.getString("catalogs"), url);

                // Force reload of any catalog subdirectories
                if (reload && _configFile != null) {
                    int n = _configFile.getNumCatalogs();
                    List<Catalog> catList = _configFile.getCatalogs();
                    for (int i = 0; i < n; i++) {
                        Catalog cat = catList.get(i);
                        catList.set(i, cat.reload());
                    }
                }
                break;
            } catch (Exception e) {
                if (url.equals(_defaultURL)) {
                    throw new RuntimeException(e);
                }
            }
        }

        // Restore listeners from above and notify
        if (savedListeners != null && savedListeners.getListenerCount() != 0) {
            _configFile._listenerList = savedListeners;
            _configFile._fireTreeStructureChanged(_configFile._getTreeModelEvent(_configFile));
        }

        return _configFile;
    }

    /**
     * @return the top level catalog directory, loading it from the AstroCat.xml file if needed.
     */
    public static AstroCatConfig getConfigFile() {
        return getConfigFile(true);
    }

    // Returns a list of URLs pointing to config file locations to try, in order.
    // The users version (~/.jsky3/AstroCat.xml) comes before the default version
    // included in the jar file. If the system property jsky.catalog.astrocat.config
    // is a URL pointing to a config file, it is tried first.
    private static List<URL> _getConfigFileLocations() {
        List<URL> urls = new ArrayList<URL>();

        // check the system property
        String urlStr = System.getProperty("jsky.catalog.astrocat.config");
        if (urlStr != null && urlStr.length() != 0) {
            try {
                urls.add(new URL(urlStr));
            } catch (MalformedURLException e) {
                e.printStackTrace();
            }
        }

        // check for ~/.jsky3/astrocat/AstroCat.xml
        String sep = System.getProperty("file.separator");
        String filename = Preferences.getPreferences().getDir() + sep + "AstroCat.xml";
        File file = new File(filename);
        if (file.exists()) {
            try {
                urls.add(file.toURI().toURL());
            } catch (MalformedURLException e) {
                e.printStackTrace();
            }
        }

        // use the default resource
        if (_defaultURL == null) {
            _defaultURL = Resources.getResource("AstroCat.xml");
        }
        if (_defaultURL == null) {
            throw new RuntimeException("Can't find the default catalog config file resource (AstroCat.xml).");
        }
        urls.add(_defaultURL);
        return urls;
    }

    /**
     * Reload the catalog config file and return the new object for it.
     */
    public Catalog reload() {
        if (this == _configFile) {
            // reload top level config file all subdirs.
            _reloadFlag = true;
            CatalogDirectory catDir = null;
            try {
                // Find and read in AstroCat.xml file to get any changes in the catalog list
                catDir = getConfigFile();
            } finally {
                _reloadFlag = false;
            }
            return catDir;
        } else {
            // reload only a subdir
            AstroCatConfig config = new AstroCatConfig(getName(), getURL());
            int n = config.getNumCatalogs();
            List<Catalog> catList = config.getCatalogs();
            for (int i = 0; i < n; i++) {
                Catalog cat = catList.get(i);
                catList.set(i, cat.reload());
            }
            return config;
        }
    }


    /**
     * Attempt to read a catalog subdirectory from the given URL and return a CatalogDirectory object for it.
     *
     * @return the new CatalogDirectory
     * @throws RuntimeException if the catalog directory could not be created
     */
    public CatalogDirectory loadSubDir(URL url) {
        String filename = url.getFile();
        if (filename.endsWith(".xml")) {
            return new AstroCatConfig(url, getHTMLQueryResultHandler());
        }

        // for compatibility, also allow Skycat config files here
        if (filename.endsWith(".cfg")) {
            return new SkycatConfigFile(url, getHTMLQueryResultHandler());
        }
        throw new RuntimeException("Expected an AstroCat XML file, or a Skycat style .cfg file");
    }


    /**
     * Set the URL to use for the default catalog config file.
     *
     * @param url points to the AstroCat XML catalog config file
     */
    public static void setConfigFile(URL url) {
        _defaultURL = url;
    }


    /**
     * Add a catalog directory to the catalog list.
     *
     * @param urlStr the URL of a catalog config file.
     */
    public void addCatalogDirectory(String urlStr) {
        if (getCatalog(urlStr) != null) {
            return;
        }

        try {
            URL url = new URL(urlStr);
            AstroCatConfig cat = new AstroCatConfig(urlStr, url);
            addCatalog(cat);
        } catch (MalformedURLException e) {
            e.printStackTrace();
        }
    }


    /**
     * Return a list of name servers (Catalogs with serv_type equal to "namesvr") to use to resolve astronomical object
     * names.
     */
    public List<Catalog> getNameServers() {
        List<Catalog> l = super.getNameServers();
        if (l.size() != 0) {
            return l;
        }
        _addDefaultNameServers();
        return CatalogFactory.getCatalogsByType(Catalog.NAME_SERVER);
    }

    /**
     * Register the default name servers (SIMBAD and NED) to use to resolve astronomical object names.
     */
    private void _addDefaultNameServers() {
        AstroCatalog cat = new AstroCatalog();
        cat.setId("simbad_ns@eso");
        cat.setName("SIMBAD Names");
        cat.setType(Catalog.NAME_SERVER);
        cat.setHost("archive.eso.org");
        cat.setURLPath("/skycat/servers/sim-server");
        FieldDescAdapter[] params = new FieldDescAdapter[1];
        params[0] = new FieldDescAdapter();
        FieldDescAdapter param = params[0];
        param.setName("Object Name");
        param.setId("o");
        param.setDescription("Enter the name of the object (star, galaxy)");
        cat.setParams(params);
        CatalogFactory.registerCatalog(cat, false);

        cat = new AstroCatalog();
        cat.setId("ned@eso");
        cat.setName("NED Names");
        cat.setType(Catalog.NAME_SERVER);
        cat.setHost("archive.eso.org");
        cat.setURLPath("/skycat/servers/ned-server");
        params = new FieldDescAdapter[1];
        params[0] = new FieldDescAdapter();
        param = params[0];
        param.setName("Object Name");
        param.setId("o");
        param.setDescription("Enter the name of the object (star, galaxy)");
        cat.setParams(params);
        CatalogFactory.registerCatalog(cat, false);
    }

    public boolean configNeedsUrl() {
        return true;
    }

    /**
     * Save the catalog list in the default location (~/.jsky3/AstroCat.xml)
     */
    public void save() {
        String sep = System.getProperty("file.separator");
        String fileName = Preferences.getPreferences().getDir() + sep + "AstroCat.xml";
        try {
            save(fileName);
        } catch (Exception e) {
            DialogUtil.error(e);
        }
    }


    /**
     * Save the catalog list to the given file (in the AstroCat XML format). Since the user can only edit the catalog
     * list and the symbol definitions, only that information is actually saved. The rest of the details are still read
     * from the original, default XML file.
     *
     * @param filename the file name in which to store the catalog information
     */
    public void save(String filename) {
        File file = new File(filename + ".tmp");
        File dir = file.getParentFile();
        if (!dir.isDirectory()) {
            if (!dir.mkdirs()) {
                DialogUtil.error("Can't create directory: " + dir);
                return;
            }
        }

        try {
            AstroCatXML.save(file, getCatalogs());
        } catch (Exception e) {
            if (!file.delete()) {
                // ignore
            }
            throw new RuntimeException(e);
        }

        File newFile = new File(filename);
        if (!newFile.delete()) {
        } // needed under Windows!
        if (!file.renameTo(newFile)) {
            DialogUtil.error("Rename " + file + " to " + filename + " failed");
        }
    }


    /**
     * Test cases
     *
     * @param args not used
     */
    public static void main(String[] args) {
        AstroCatConfig configFile = AstroCatConfig.getConfigFile();
        String catalogName = "Guide Star Catalog at ESO";
        Catalog cat = configFile.getCatalog(catalogName);
        if (cat == null) {
            System.out.println("Can't find entry for catalog: " + catalogName);
            System.exit(1);
        } else {
            System.out.println("Test passed");
        }
    }
}
