/*
 * $Id: Resources.java,v 1.4 2009/04/21 13:31:17 abrighto Exp $
 */

package jsky.util;

import java.io.BufferedInputStream;
import java.io.InputStream;
import java.io.IOException;
import java.net.URL;
import java.util.Properties;
import java.awt.*;

import javax.swing.*;

 
/**
 * Resources provides a central class with methods for accessing
 * project resources.
 */
public final class Resources {

    /** Base path to resources. */
    public static final String RESOURCE_PATH = "";

    /** Subpath to images, within the resources area. */
    public static final String IMAGES_SUBPATH = "images";

    /** Path to images, within the resources area. */
    public static final String IMAGES_PATH = RESOURCE_PATH + "/" + IMAGES_SUBPATH + "/";

    /** Subpath to config files, within the resources area. */
    public static final String CONFIG_SUBPATH = "conf";

    // ImageCache provides a cache of already present icons.
    private static ResourceMap _rmap = new ResourceMap();

    // Disallow instances
    private Resources() {
    }

    /**
     * Gets a URL associated with the given resource.
     *
     * @return a URL pointing to the file, null otherwise
     */
    public static URL getResource(String resource) {
        String path = RESOURCE_PATH + '/' + resource;
        //System.out.println("getResource: " + path);
        return Resources.class.getResource(path);
    }


    /**
     * Gets a resource as an {@link InputStream}.
     *
     * @return a InputStream for the resource else null.
     */
    public static InputStream getResourceAsStream(String resource) {
        String path = RESOURCE_PATH + '/' + resource;
        //System.out.println("getResource: " + path);
        return Resources.class.getResourceAsStream(path);
    }


    /**
     * Returns an Icon from the specified filename.
     *
     * @param iconFileName The relative path name to the image file.
     * For example, "flag.gif".  It is assumed the image file is
     * being properly installed to the resources directory.
     *
     * @return Icon constructed from data in <code>iconFileName</code>.
     * Even though the method interface can't guarentee this, the icon
     * implementation will be <code>Serializable</code>.
     * Returns null (does not throw an Exception) if specified
     * resource is not found.
     */
    public static Icon getIcon(String iconFileName) {
        // First check the map under the iconFileName
        Icon icon = _rmap.getIcon(iconFileName);
        if (icon != null) {
            return icon;
        }

        // Turn the file name into a resource path
        URL url = getResource(IMAGES_SUBPATH + "/" + iconFileName);
        if (url == null) return null;

        // If constructor fails, don't want to store null reference
        icon = new ImageIcon(url);
        _rmap.storeIcon(iconFileName, icon);
        return icon;
    }

    /**
     * Returns a standard system icon
     *
     * @param iconName The icon name (see http://nadeausoftware.com/node/88 for Mac icons, for example)
     *
     * @return the icon, or null if not found
     */
    public static Icon getSystemIcon(String iconName) {
        Icon icon = _rmap.getIcon(iconName);
        if (icon != null) {
            return icon;
        }
        Toolkit toolkit = Toolkit.getDefaultToolkit();
        Image image = toolkit.getImage(iconName);
        return new ImageIcon(image);
    }

    /**
     * Loads an installed <code>Properties</code> file.
     *
     * @param fileName relative path to the configuration file (which must be
     * loadable by the <code>java.util.Properties</code> class)
     *
     * @return a Properties object created from the configuration file; null if
     * the file does not exist
     */
    public static Properties getProperties(String fileName)
            throws IOException {
        URL url = getResource(CONFIG_SUBPATH + "/" + fileName);
        if (url == null) {
            return null;
        }

        Properties props = null;
        BufferedInputStream bis = null;

        try {
            bis = new BufferedInputStream(url.openStream());
            props = new Properties();
            props.load(bis);
        } finally {
            if (bis != null)
                try {
                    bis.close();
                } catch (Exception ignored) {
                }
        }

        return props;
    }
}
