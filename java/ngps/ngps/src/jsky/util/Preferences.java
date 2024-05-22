/*
 * $Id: Preferences.java,v 1.4 2009/04/21 13:31:17 abrighto Exp $
 */

package jsky.util;

import java.awt.Component;
import java.awt.Dimension;
import java.awt.event.ComponentAdapter;
import java.awt.event.ComponentEvent;
import java.io.*;
import java.util.Properties;
import javax.swing.JComponent;
import java.awt.Toolkit;
import java.net.URL; 

/**
 * Simple class to save and reload user preferences.  Preferences in
 * this case are represented as {key, value} string pairs. The caller
 * should use a unique key based on the package and class name to avoid any
 * name conflicts.
 *
 * @author Allan Brighton
 * @version $Revision: 1.4 $
 */
public class Preferences {

    /**
     * The name of the directory under the user's home dir in which to save preferences
     */
    public static final String SAVE_DIR = ".jsky3";

    /**
     * The file suffix used for serialized objects
     */
    public static final String SERIAL_SUFFIX = ".ser";

    /**
     * Stores the key/value pairs
     */
    private Properties _properties = new Properties();

    /**
     * Name of the directory to use to store the preferences
     */
    private File _dir;

    /**
     * Directory used to cache downloaded files.
     */
    private File _cacheDir;

    /**
     * The system path separator
     */
    private String _sep;

    /**
     * File to use to store the preferences
     */
    private File _file;

    /**
     * A single, global instance of this class.
     */
    private static Preferences _preferences;


    /**
     * Load the preferences from the default location (~/.jsky3/jsky.properties)
     */
    public Preferences() {
        String home = System.getProperty("user.home");
        _sep = System.getProperty("file.separator");
        String dirName = home + _sep + SAVE_DIR;
        _dir = new File(dirName);
        _cacheDir = new File(dirName + _sep + "cache");
        _file = new File(dirName + _sep + "jsky.properties");
        try {
            if (!_dir.isDirectory()) {
                if (!_dir.mkdirs()) {
                    throw new RuntimeException();
                }
            }
            if (!_cacheDir.isDirectory()) {
                if (!_cacheDir.mkdir()) {
                    throw new RuntimeException();
                }
            }
        } catch (Exception ignored) {
        }
        _load();
    }

    /**
     * Load the preferences from the given filename.
     */
    public Preferences(String filename) {
        _file = new File(filename);
        _dir = _file.getParentFile();
        _load();
    }

    /**
     * Return the name of the directory to use to store the preferences.
     */
    public File getDir() {
        return _dir;
    }

    /**
     * Return the name of the directory to use to save downloaded files.
     */
    public File getCacheDir() {
        return _cacheDir;
    }

    /**
     * Return the system path separator.
     */
    public String getSep() {
        return _sep;
    }

    /**
     * Return the full path name of the file to use to store the preferences
     */
    public File getFile() {
        return _file;
    }

    /**
     * Return the Properties object used to store the key/value pairs.
     */
    public Properties getProperties() {
        return _properties;
    }

    /**
     * Return the global instance of this class, creating one first if needed
     */
    public static Preferences getPreferences() {
        if (_preferences == null) {
            _preferences = new Preferences();
        }
        return _preferences;
    }

    /**
     * Set the one global instance of this class.
     */
    public static void setPreferences(Preferences p) {
        _preferences = p;
    }

    /**
     * Add a key,value pair to the set of preferences.
     */
    public void setPreference(String key, String value) {
        if (value != null) {
            _properties.setProperty(key, value);
        } else {
            _properties.remove(key);
        }
    }

    /**
     * Remove a key from the set of preferences.
     */
    public void unsetPreference(String key) {
        _properties.remove(key);
    }

    /**
     * Return the preference value for the given key, or null if not found.
     */
    public String getPreference(String key) {
        return _properties.getProperty(key);
    }

    /**
     * Return the preference value for the given key, or the default value if not found.
     */
    public String getPreference(String key, String defaultValue) {
        return _properties.getProperty(key, defaultValue);
    }

    /**
     * Add a key,value pair to the set of preferences.
     */
    public static void set(String key, String value) {
        if (_preferences == null) {
            _preferences = new Preferences();
        }
        _preferences.setPreference(key, value);
    }

    /**
     * Add a key,value pair to the set of preferences.
     */
    public static void set(String key, boolean value) {
        if (_preferences == null) {
            _preferences = new Preferences();
        }
        _preferences.setPreference(key, String.valueOf(value));
    }

    /**
     * Remove a key from the set of preferences.
     */
    public static void unset(String key) {
        if (_preferences == null) {
            return;
        }
        _preferences.unsetPreference(key);
    }

    /**
     * Return the preference value for the given key, or null if not found.
     */
    public static String get(String key) {
        if (_preferences == null) {
            _preferences = new Preferences();
        }
        return _preferences.getPreference(key);
    }

    /**
     * Return the preference value for the given key, or the default value if not found.
     */
    public static String get(String key, String defaultValue) {
        if (_preferences == null) {
            _preferences = new Preferences();
        }
        return _preferences.getPreference(key, defaultValue);
    }

    /**
     * Return the preference value for the given key, or the default value if not found.
     */
    public static boolean get(String key, boolean defaultValue) {
        if (_preferences == null) {
            _preferences = new Preferences();
        }
        String s = _preferences.getPreference(key);
        if (s == null) {
            return defaultValue;
        }

        return Boolean.valueOf(s);
    }

    /**
     * Load the previously saved preferences and arrange
     * for them to be saved when the application exits.
     */
    private void _load() {
        load();
        Runtime.getRuntime().addShutdownHook(new Thread() {
            public void run() {
                save();
            }
        });
    }

    /**
     * Load the previously saved preferences
     */
    public void load() {
        if (_file.exists()) {
            try {
                _properties.load(new FileInputStream(_file));
            } catch (Exception e) {
                System.out.println("Error reading preferences file: "
                        + _file.getPath() + ": " + e.toString());
            }
        }
    }

    /**
     * Save the current preferences to this._file.
     */
    public void save() {
        try {
            _properties.store(new FileOutputStream(_file), "JSky User Preferences");
        } catch (Exception e) {
            System.out.println("Error saving preferences file: " + _file.getPath() + ": " + e.toString());
        }
    }


    /**
     * Attempt to serialize an object to a file with the given base name.
     * The actual file name is made up of the preferences "dir" {@link #getDir}
     * plus the name plus the suffix {@link #SERIAL_SUFFIX}.
     */
    public void serialize(String name, Object object) throws IOException {

        String filename = _dir + _sep + name + SERIAL_SUFFIX;
        ObjectOutputStream out = new ObjectOutputStream(new FileOutputStream(filename));
        out.writeObject(object);
        out.close();
    }

    /**
     * Attempt to read a serialized object from a file with the given base name.
     * The actual file name is made up of the preferences "dir" {@link #getDir}
     * plus the name plus the suffix {@link #SERIAL_SUFFIX}. An exception is thrown if the file
     * does not exist or is corrupt.
     */
    public Object deserialize(String name) throws ClassNotFoundException, IOException {
        File file = new File(_dir + _sep + name + SERIAL_SUFFIX);
        if (file.exists()) {
            ObjectInputStream in = new ObjectInputStream(new FileInputStream(file));
            Object object = in.readObject();
            in.close();
            return object;
        }
        throw new FileNotFoundException(file.getPath());
    }

    /**
     * Remove the previously serialized object file with the given base name.
     * The actual file name is made up of the preferences "dir" {@link #getDir}
     * plus the name plus the suffix {@link #SERIAL_SUFFIX}.
     */
    public void removeSerializedFile(String name) throws IOException {
        String filename = _dir + _sep + name + SERIAL_SUFFIX;
        try {
            //noinspection ResultOfMethodCallIgnored
            new File(filename).delete();
        } catch (Exception e) {
            // ignore
        }
    }


    /**
     * Tracks changes in the location of the given component (normally
     * a JFrame).  If previous settings are known,
     * apply them now.  In any case, arrange to save future changes in
     * the component so that the location will be restored the next
     * time the application runs.
     *
     * @param component a component whose location should be restored when the
     * application restarts
     */
    public static void manageLocation(Component component) {
        Preferences.manageLocation(component, -1, -1);
    }

    /**
     * Tracks changes in the location of the given component (normally
     * a JFrame).  If previous settings are known,
     * apply them now.  In any case, arrange to save future changes in
     * the component so that the location will be restored the next
     * time the application runs.
     *
     * @param component a component whose location should be restored when the
     * application restarts
     * @param x the default x location of the component
     * @param y the default y location of the component
     */
    public static void manageLocation(Component component, int x, int y) {
        Preferences.manageLocation(component, x, y, component.getClass().getName() + ".pos");
    }

    /**
     * Tracks changes in the location of the given component (normally
     * a JFrame).  If previous settings are known,
     * apply them now.  In any case, arrange to save future changes in
     * the component so that the location will be restored the next
     * time the application runs.
     *
     * @param component a component whose location should be restored when the
     * application restarts
     * @param x the default x location of the component
     * @param y the default y location of the component
     * @param name the name to store this information under
     */
    public static void manageLocation(Component component, int x, int y, final String name) {
        // restore any previous settings
        String s = Preferences.get(name);
        if (s != null) {
            try {
                // use TclUtil for convenience in splitting the list of numbers
                String[] ar = TclUtil.splitList(s);
                if (ar.length == 2) {
                    component.setLocation(Integer.parseInt(ar[0]), Integer.parseInt(ar[1]));
                }
            } catch (Exception e) {
                System.out.println("Warning: error reading component location from preferences file: "
                        + _preferences._file.getPath());
                if (x >= 0 && y >= 0) {
                    component.setLocation(x, y);
                } else {
                    _centerComponent(component);
                }
            }
        } else if (x >= 0 && y >= 0) {
            component.setLocation(x, y);
        } else {
            _centerComponent(component);
        }

        // track changes in the component's geometry
        component.addComponentListener(new ComponentAdapter() {
            private void saveLocation(ComponentEvent e) {
                Component c = e.getComponent();
                String[] ar = new String[2];
                ar[0] = Integer.toString(c.getX());
                ar[1] = Integer.toString(c.getY());
                Preferences.set(name, TclUtil.makeList(ar));
            }

            public void componentResized(ComponentEvent e) {
                saveLocation(e);
            }

            public void componentMoved(ComponentEvent e) {
                saveLocation(e);
            }
        });
    }


    // Center the given frame on the screen
    private static void _centerComponent(Component component) {
        Dimension dim = component.getPreferredSize();
        if (dim.width > 1 && dim.height > 1) {
            Dimension screen = Toolkit.getDefaultToolkit().getScreenSize();
            component.setLocation(screen.width / 2 - dim.width / 2, screen.height / 2 - dim.height / 2);
        }
    }


    /**
     * Track changes in the size of the given component.  If previous
     * settings are known, apply them now.  In any case, arrange to
     * save future changes in the component so that the size will be
     * restored the next time the application runs.
     *
     * @param component a component whose size should be restored when the
     * application restarts
     * @param defaultSize the default size of the component, or null for the default
     */
    public static void manageSize(JComponent component, Dimension defaultSize) {
        Preferences.manageSize(component, defaultSize, component.getClass().getName() + ".size");
    }

    /**
     * Track changes in the size of the given component.  If previous
     * settings are known, apply them now.  In any case, arrange to
     * save future changes in the component so that the size will be
     * restored the next time the application runs.
     *
     * @param component a component whose size should be restored when the
     * application restarts
     * @param defaultSize the default size of the component
     * @param name the name to store this information under
     */
    public static void manageSize(JComponent component, Dimension defaultSize, final String name) {
        // restore any previous settings
        String s = Preferences.get(name);
        if (s != null) {
            try {
                // use TclUtil for convenience in splitting the list of numbers
                String[] ar = TclUtil.splitList(s);
                if (ar.length == 2) {
                    component.setPreferredSize(new Dimension(Integer.parseInt(ar[0]), Integer.parseInt(ar[1])));
                }
            } catch (Exception e) {
                System.out.println("Warning: error reading component size from preferences file: "
                        + _preferences._file.getPath());
                component.setPreferredSize(defaultSize);
            }
        } else if (defaultSize != null) {
            component.setPreferredSize(defaultSize);
        }

        // track changes in the component's geometry
        component.addComponentListener(new ComponentAdapter() {

            private void saveSize(ComponentEvent e) {
                Component c = e.getComponent();
                String[] ar = new String[2];
                ar[0] = Integer.toString(c.getWidth());
                ar[1] = Integer.toString(c.getHeight());
                Preferences.set(name, TclUtil.makeList(ar));
            }

            public void componentResized(ComponentEvent e) {
                saveSize(e);
            }

            public void componentMoved(ComponentEvent e) {
                saveSize(e);
            }
        });
    }

}
