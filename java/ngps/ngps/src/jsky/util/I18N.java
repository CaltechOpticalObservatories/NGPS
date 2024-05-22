/*
 * Copyright 2002 Association for Universities for Research in Astronomy, Inc.,
 * Observatory Control System, Gemini Telescopes Project.
 *
 * $Id: I18N.java,v 1.2 2009/03/17 22:56:43 abrighto Exp $
 */

package jsky.util; 

import java.text.MessageFormat;
import java.util.*;
import java.util.logging.Logger;


/**
 * Simple utility class for accessing property file resource bundles for
 * internationalization.
 * <p/>
 * This class assumes one property file per package, as needed. The convention
 * used here is to store the property files in a subdirectory of the package
 * named <code>i18n</code>. The base name is <code>gui</code>, so the default
 * property file is <code>i18n/gui.properties</code>. The German version would then
 * be <code>i18n/gui_de.properties</code> and the French version would be
 * <code>i18n/gui_fr.properties</code>. The property files need to be installed
 * in the same relative directory in the classes dir or jar file before use.
 *
 * @author Allan Brighton (modified original version by Guillaume Helle)
 * @version $Revision: 1.2 $
 */
public class I18N {

    private static final Logger LOG = Logger.getLogger(I18N.class.getName());

    // Maps package names to resource bundles.
    // (Note that the ResourceBundle class itself has an internal cache.)
    private static Map<String, I18N> _pkgBundles = new Hashtable<String, I18N>();


    // The base name of the I18N properties files
    private String _baseName;

    // Current Locale
    private Locale _locale = Locale.getDefault();

    /**
     * Return an instance of I18N, initialized to use i18n/gui_<locale>.properties,
     * relative to the package directory for the given class.
     * @param c the class
     * @return the instance
     */
    public static I18N getInstance(Class c) {
        String pkgName = c.getPackage().getName();
        I18N i18n = _pkgBundles.get(pkgName);
        if (i18n != null) {
            return i18n;
        }
        i18n = new I18N(c);
        _pkgBundles.put(pkgName, i18n);
        return i18n;
    }

    // Initialize to use i18n/gui_<locale>.properties, relative to the
    // package directory for the given class.
    private I18N(Class c) {
        _baseName = c.getPackage().getName() + ".i18n.gui";
    }

    /**
     * Set the current locale.
     * @param locale the locale to set
     */
    public void setLocale(Locale locale) {
        _locale = locale;
    }

    /**
     * Get the string for the given key
     * @param key the key to look for
     * @return the string for the specified key in the current locale.
     */
    public String getString(String key) {
        try {
            ResourceBundle rb = ResourceBundle.getBundle(_baseName, _locale);
            String text = rb.getString(key);
            return text != null ? text : key;
        } catch (MissingResourceException mre) {
            if (_locale != Locale.US) {
                _locale = Locale.US;
                return getString(key);
            }
            LOG.warning(mre.getMessage());
            return key;
        } catch (Exception e) {
            LOG.warning(e.getMessage());
            return key;
        }
    }

    /**
     * Return the string for the specified key in the current locale after substituting
     * the given parameters using the MessageFormat class.
     *
     * @param key the key to look for
     * @param params the parameters to insert in the string
     * @return the string for the specified key
     * @see java.text.MessageFormat
     */
    public String getString(String key, Object... params) {
        String pattern = getString(key);
        if (pattern == null) {
            return null;
        }
        MessageFormat mf = new MessageFormat(pattern);
        mf.setLocale(_locale);
        return mf.format(params);
    }

    /**
     * test main
     * @param args not used
     */
    public static void main(String[] args) {
        // Base name for the default I18N property file
        I18N i18n = I18N.getInstance(I18N.class);

        System.out.println("hello = " + i18n.getString("hello"));
        System.out.println("test1 = " + i18n.getString("test1"));
        System.out.println("test1 with 2 args = " + i18n.getString("test1", "One", "Two"));
        System.out.println("test1 with 3 args = " + i18n.getString("test1", "One", "Two", 3));
    }
}
