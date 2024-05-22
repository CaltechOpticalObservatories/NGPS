package edu.caltech.palomar.util.gui;

 
import java.io.InputStream;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.File;
import java.security.AccessControlException;
import java.net.URL;
import java.util.Properties;
import java.util.List;
import java.util.LinkedList;
import java.util.Iterator;
import java.util.StringTokenizer;

import java.beans.PropertyChangeListener;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeSupport;


/**
 * <em>Not yet finished!</em>
 * A utility class for working with properties.  Reads in properity files,
 * manages user properties apart from application properites, and has get 
 * methods for various types of properties.
 * @author Trey Roby
 */
public class AppProperties {

  private static PropertyChangeSupport _propChange= new PropertyChangeSupport(
                                                           AppProperties.class);
  private static String  _appName= "";
  /**
   * alt properties are set if the system propterty database is not 
   * accessable such as in an applet.  the alt or system properites 
   * hold only non user settable properites
   */
  private static Properties _altProperties=   new Properties();
  /**
   * user properties will hold only properites that the end user sets.
   */
  private static Properties _userProperties         = new Properties();
  private static Properties _preferencesProperties  = new Properties();
  private static Properties _requestedUserProperties= new Properties();
  private static File       _preferencesFile        = null;
  private static List       _loadedResources        = new LinkedList();


  public static Properties getUserProp()             { return _userProperties;}
  public static Properties getAltSystemProp()        { return _altProperties;}
  public static String     getAppName()              { return _appName;}
  public static void       setAppName(String appName){ _appName= appName;}

//=========================================================================
//------------------------- Float properties ------------------------------
//=========================================================================

  public static float getFloatProperty(String key,boolean userAlso) {
      return getFloatProperty(key,0.0F,userAlso);
  }
  public static float getFloatProperty(String key) {
      return getFloatProperty(key,0.0F,false);
  }
  public static float getFloatProperty(String key, float def) {
      return getFloatProperty(key,def,false);
  }

  public static float getFloatProperty(String  key, 
                                       float   def, 
                                       boolean userAlso) {
      String val=    getProperty(key,userAlso);
      float    retval= 0;
      try {
          if (val != null) retval= Float.parseFloat(val);
          else             retval= def;
      } catch (NumberFormatException e) {
          retval= def;
      }
      return retval;
  }
//=========================================================================
//------------------------- Double properties ------------------------------
//=========================================================================

  public static double getDoubleProperty(String key,boolean userAlso) {
      return getDoubleProperty(key,0.0F,userAlso);
  }
  public static double getDoubleProperty(String key) {
      return getDoubleProperty(key,0.0F,false);
  }
  public static double getDoubleProperty(String key, double def) {
      return getDoubleProperty(key,def,false);
  }

  public static double getDoubleProperty(String  key, 
                                         double   def, 
                                         boolean userAlso) {
      String val=    getProperty(key,userAlso);
      double    retval= 0;
      try {
          if (val != null) retval= Double.parseDouble(val);
          else             retval= def;
      } catch (NumberFormatException e) {
          retval= def;
      }
      return retval;
  }

//=========================================================================
//------------------------- Int properties --------------------------------
//=========================================================================


  public static int getIntProperty(String key, boolean userAlso) {
      return getIntProperty(key,0,userAlso);
  }
  public static int getIntProperty(String key) {
      return getIntProperty(key,0,false);
  }
  public static int getIntProperty(String key, int def) {
      return getIntProperty(key,def,false);
  }

  public static int getIntProperty(String key, int def, boolean userAlso) {
      String val=    getProperty(key,userAlso);
      int    retval= 0;
      try {
          if (val != null) retval= Integer.parseInt(val);
          else             retval= def;
      } catch (NumberFormatException e) {
          retval= def;
      }
      return retval;
  }

//=========================================================================
//------------------------- Boolean properties -------------------------------
//=========================================================================

  public static boolean getBooleanProperty(String key) {
      return getBooleanProperty(key,false);
  }
  public static boolean getBooleanProperty(String key, boolean def) {
      return getBooleanProperty(key,def,false);
  }

  public static boolean getBooleanProperty(String  key, 
                                           boolean def, 
                                           boolean userAlso) {
      String  val=    getProperty(key,userAlso);
      boolean retval= false;
      if (val != null) retval= Boolean.valueOf(val).booleanValue();
      else             retval= def;
      return retval;
  }

//=========================================================================
//------------------------- String properties -----------------------------
//=========================================================================

  public static String getProperty(String key) {
        return getProperty(key,null,false);
  }

  public static String getProperty(String key, boolean userAlso) {
        return getProperty(key,null,userAlso);
  }

  public static String getProperty(String key, String def) {
        return getProperty(key,def,false);
  }

  public static String getProperty(String key, String def, boolean userAlso) {
      String value;
      try {
           value= System.getProperty(key, def);
           //System.out.println("App: the system found: " + value);
      } catch (AccessControlException ace) {
           value= _altProperties.getProperty(key, def);
      }
      if (userAlso) {
           String userValue= _userProperties.getProperty(key, def);
           //System.out.println("App: the user found: " + userValue);
           if (userValue != def) {
                  value= userValue;
           }
           //System.out.println("App: the return: " + value);
           if (_requestedUserProperties != null) {
                   //System.out.println("putting= " + key+", " + value);
                   String sv= (value == null) ? "" : value;
                   _requestedUserProperties.setProperty(key, sv);
           }
      }
      return value;
  }

//=========================================================================
//--------------------- Preference Methods --------------------------------
//=========================================================================

  public static void setPreference(String key, String value) {
      Object oldValue= _preferencesProperties.getProperty(key, value);
      _preferencesProperties.setProperty(key, value);
      _userProperties.setProperty(key, value);
      dumpPreferences();
      _propChange.firePropertyChange(key,oldValue, value);
  }

  public static void dumpPreferences() {
      // write out the current preference file
     try {
          if (_preferencesFile != null) {
             FileOutputStream fs= new FileOutputStream( _preferencesFile );
             _preferencesProperties.store(fs, 
                    "-------- User Preferences - Automaticly Generated -"+
                    " Do not Edit -------- ");
             fs.close();
          }
     }
     catch (IOException e) {
          System.out.println("AppProperties.dumpPreference: " + e);
     }
  }

  public static void setAndLoadPreferenceFile(File f) {
      _preferencesFile= f;
      if (f.exists()) {
         try {
             FileInputStream fs= new FileInputStream( f );
             addPropertiesFromStream(fs, _preferencesProperties);
             fs.close();
             addUserProperties(f);
         } catch (IOException e) {
             System.out.println("AppProperties.setAndLoadPreferenceFile: " + e);
         }
      }
  }
  
//=========================================================================
//-------------------------------------------------------------------------
//=========================================================================


  public static String makeProperty(String pname) {
      String newpname= (pname.charAt(0) == '.') ? _appName + pname : pname;
      return newpname;
  }

  public static void dumpUserProperties(File f) {
      try {
          FileOutputStream fs= new FileOutputStream( f );
          Properties dumpProps =  new Properties();
          dumpProps.putAll(_requestedUserProperties);
          dumpProps.putAll(_userProperties);
          dumpProps.store(fs, "-------- Current User properties -------- ");
          System.out.println("User properties written to: " + f);
          fs.close();
      }
      catch (IOException e) {
          System.out.println(e);
      }
  }


  public static void addUserProperties(File f) { addUserProperties(f,false); }

  public static void addUserProperties(File f, boolean showError) {
      try {
          InputStream fs= new FileInputStream( f );
          System.out.println("Loading Properties From: " + f);
          addPropertiesFromStream(fs, _userProperties);
      } catch (FileNotFoundException e) {
          if (showError) System.out.println(e);
      } catch (AccessControlException ace) {
          if (showError) System.out.println(ace);
      }

  }

  public static void addUserProperties(String prop_file, String where_prop) {
      String path= System.getProperty(where_prop);
      File f= new File( path, prop_file);
      addUserProperties(f);
  }

  public static void addSystemProperties(String prop_file, Class c) {
      addSystemProperties(prop_file, c, true);
  }

  public static void addSystemProperties(String  prop_file, 
                                         Class   c,
                                         boolean doprint) {
      try {
          InputStream prop_stream= null;
          URL url;
          //System.out.println("prop_file= " + prop_file);
          //System.out.println("getClass()= " + getClass());
          //System.out.println("loader()= " + getClass().getClassLoader() );

          ClassLoader cl = c.getClassLoader();
          if (cl==null) {
             url= ClassLoader.getSystemResource(prop_file);
             if (notLoaded(url)) {
                prop_stream= url.openStream();
                synchronized (_loadedResources) {
                   _loadedResources.add(url);
                }
             }
          }
          else { 
             url= cl.getResource(prop_file);
             if (notLoaded(url)) {
                prop_stream= url.openStream();
                synchronized (_loadedResources) {
                   _loadedResources.add(url);
                }
             }
          }

          if (doprint)
            System.out.println("Loading System Properties From: " + prop_file);
          addPropertiesFromStream(prop_stream,null);
      } catch (IOException e) {
          //System.out.println(
          //     "AppProperties: System properties not found: " + prop_file);
          //System.out.println(e);
      } catch (NullPointerException ne) {
          //System.out.println(
          //   "AppProperties: could not load properties from: " + prop_file);
      }
  }

  public static void addSystemProperties(String prop_file) {
          addSystemProperties(prop_file, AppProperties.class);
  }

  /**
   * Load this classes class specific properties.  If this methoed is
   * class multiple times it will only load the class properties the first 
   * time.  The properties are loaded into the system property database.
   * The file this method will attemp to load is build on the classname, in
   * a "resources" subdirectory of the packages directory and will have an
   * an extension of ".prop".
   * If the class name is "Abc" then the file name built is
   * "resources/Abc.prop".
   */
  public static boolean loadClassProperties(Class objClass) {
       Assert.tst(objClass);
       StringTokenizer st= new StringTokenizer(objClass.getName(), "."); 
       int len= st.countTokens();
       for(int i= 0; (i<len-1); st.nextToken(), i++);
       String fname= "resources/" + st.nextToken() + ".prop";
       boolean retval= loadClassProperties(objClass, fname);
       return retval;
  }
  /**
   * Load this classes class specific properties.  If this methoed is
   * called multiple times it will only load the class properties the first 
   * time.  The properties are loaded into the system property database.
   */
  public static boolean loadClassProperties(Class objClass, String resource) {
      boolean loaded= false;
      try {
          URL url= objClass.getResource(resource);
          if (notLoaded(url)) {
                addPropertiesFromStream(url.openStream(),null);
                synchronized (_loadedResources) {
                    _loadedResources.add(url);
                }
          }
          loaded= true;
      } catch (Exception e) {
          System.out.println(
                 "AppProperties: System properties not found: " + resource);
          System.out.println(e);
      }
      return loaded;
  }

      // load current prerference files into preference database and combine
  public static synchronized void addPropertiesFromStream(
                                             InputStream prop_stream, 
                                             Properties  pdb) {
      Properties p;
      if (pdb != null) {
          try {
                pdb.load(prop_stream);
          } catch (IOException e) {
                System.out.println( 
                            "AppProperties: could not load user properties.");
                System.out.println(e);
          }
      } // end if
      else {
          try {
              p = new Properties(System.getProperties());
              p.load(prop_stream);
              System.setProperties(p);
          } catch (IOException e) {
              System.out.println("AppProperties: could not load properties.");
              System.out.println(e);
          } catch (AccessControlException ace) {
                 System.out.println(ace);
          }
          try {
                _altProperties.load(prop_stream);
          } catch (IOException e) {
                System.out.println(
                  "AppProperties: could not load alt properties.");
                System.out.println(e);
          }
      } // end else
      try {
           prop_stream.close();
      } catch (IOException e) {}
  }

  public static boolean notLoaded(URL url) { 
      boolean found= false;
      synchronized (_loadedResources) {
         Iterator i= _loadedResources.iterator();
         while(i.hasNext() && !found) {
             found= url.sameFile( (URL)i.next() );
         }
      }
      return (!found);
  }

//=====================================================================
//----------- add / remove property Change listener methods -----------
//=====================================================================

    /**
     * Add a property changed listener.
     * @param PropertyChangeListener  the listener
     */
    public static void addPropertyChangeListener (PropertyChangeListener p) {
       _propChange.addPropertyChangeListener (p);
    }

    /**
     * Remove a property changed listener.
     * @param PropertyChangeListener  the listener
     */
    public static void removePropertyChangeListener (PropertyChangeListener p) {
       _propChange.removePropertyChangeListener (p);
    }
}


