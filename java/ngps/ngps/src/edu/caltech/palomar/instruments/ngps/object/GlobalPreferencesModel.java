/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package edu.caltech.palomar.instruments.ngps.object;

import java.beans.PropertyChangeListener;
import java.beans.PropertyChangeSupport;
import java.io.FileInputStream;
import java.util.Properties;

/**
 *
 * @author developer
 */
public class GlobalPreferencesModel {
  transient protected PropertyChangeSupport propertyChangeListeners = new PropertyChangeSupport(this);
  public java.lang.String          USERDIR = System.getProperty("user.dir");
  public java.lang.String          SEP     = System.getProperty("file.separator");
  public java.lang.String          CONFIG  = new java.lang.String("config");
  public java.lang.String          NGPS_PROPERTIES = new java.lang.String("ngps.ini");
  public java.lang.String          IMAGE_CACHE     = new java.lang.String("images");
  public double                    DEFAULT_SEEING;
  public int                       DEFAULT_WAVELENGTH;
  public double                    DEFAULT_AIRMASS_LIMIT;
  private double                   global_airmass_max;
/*================================================================================================
/      GlobalPreferencesModel()
/=================================================================================================*/
   public GlobalPreferencesModel(){
      readProperties(); 
   } 
/*=============================================================================================
/    readProperties()
/=============================================================================================*/ 
private void readProperties(){
   try{
     java.lang.String NGPS_PROPERTIES_FILE = USERDIR + SEP + CONFIG + SEP + NGPS_PROPERTIES;
      FileInputStream  ngps_properties_file            = new FileInputStream(NGPS_PROPERTIES_FILE);
      Properties       ngps_properties                 = new Properties();
      ngps_properties.load(ngps_properties_file);
      ngps_properties_file.close();
      DEFAULT_SEEING         = Double.parseDouble(ngps_properties.getProperty("DEFAULT_SEEING"));
      DEFAULT_WAVELENGTH     = Integer.parseInt(ngps_properties.getProperty("DEFAULT_WAVELENGTH"));
      DEFAULT_AIRMASS_LIMIT  = Double.parseDouble(ngps_properties.getProperty("DEFAULT_AIRMASS_LIMIT"));
      setGLOBAL_AIRMASS_MAX(DEFAULT_AIRMASS_LIMIT);     
   }catch(Exception e){
       System.out.println(e.toString());
   }  
}
/*================================================================================================
/       setGLOBAL_AIRMASS_MAX(double global_airmass_max_string)
/=================================================================================================*/
  public void setGLOBAL_AIRMASS_MAX(double global_airmass_max_string) {
      // We are expecting one of the following strings "J2000" or "B1950"
    double  old_global_airmass_max = this.global_airmass_max;
    this.global_airmass_max = global_airmass_max_string;
    propertyChangeListeners.firePropertyChange("global_airmass_max", Double.valueOf(old_global_airmass_max), Double.valueOf(global_airmass_max));
  }
  public double getGLOBAL_AIRMASS_MAX() {
    return global_airmass_max;
  }
/*================================================================================================
/      add and remove Property Change Listeners
/=================================================================================================*/
  public synchronized void removePropertyChangeListener(PropertyChangeListener l) {
    propertyChangeListeners.removePropertyChangeListener(l);
  }
  public synchronized void addPropertyChangeListener(PropertyChangeListener l) {
    propertyChangeListeners.addPropertyChangeListener(l);
  }     
}
