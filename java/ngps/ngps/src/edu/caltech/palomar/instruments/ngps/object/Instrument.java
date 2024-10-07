/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package edu.caltech.palomar.instruments.ngps.object;

import java.beans.PropertyChangeListener;
import java.beans.PropertyChangeSupport;

/**
 *
 * @author jennifermilburn
 */
public class Instrument extends java.lang.Object{
  transient protected PropertyChangeSupport propertyChangeListeners = new PropertyChangeSupport(this);
  public java.lang.String exposuretime; 
  public double           EXPTIME;
  public java.lang.String EXPTIME_CODE;
  public int              nexp;
  public double           slitwidth;
  public java.lang.String slitwidth_string;
  public double           slitoffset;
  public java.lang.String ccdmode;
  public int              bin_spec;
  public int              bin_space;
  public java.lang.String requestedSlitAngle;
  public double           slitangle;

  /*================================================================================================
/      Instrument() CONSTRUCTOR
/=================================================================================================*/
public Instrument(){
  initializeValues();  
}
private void initializeValues(){
    setNEXP(1);
    setSlitwidth(1.0);  // CHAZ
    setSlitOffset(0.0);
    setOBSMODE("default");
    setBIN_SPEC(1);
    setBIN_SPACE(1);
    setExposuretime("SET 1"); 
    setSlitwidth_string("SET 1");
    setRequestedSlitAngle("PA");
}

/*================================================================================================
/      setExposuretime(double new)
/=================================================================================================*/
  public void setExposuretime(java.lang.String new_exposuretime) {
    java.lang.String  old_exposuretime = this.exposuretime;
    exposuretime = new_exposuretime;
   propertyChangeListeners.firePropertyChange("exposuretime_string", old_exposuretime, new_exposuretime);
  }
  public java.lang.String getExposuretime() {
    return exposuretime;
  }
 /*================================================================================================
/      setExposuretime(double new)
/=================================================================================================*/
  public void setEXPTIME_CODE(java.lang.String new_EXPTIME_CODE) {
    java.lang.String  old_EXPTIME_CODE = this.EXPTIME_CODE;
    EXPTIME_CODE = new_EXPTIME_CODE;
   propertyChangeListeners.firePropertyChange("EXPTIME_CODE", old_EXPTIME_CODE, new_EXPTIME_CODE);
  }
  public java.lang.String getEXPTIME_CODE() {
    return EXPTIME_CODE;
  } 
 /*================================================================================================
/      setEXPTIME(double new_EXPTIME) 
/=================================================================================================*/
  public void setEXPTIME(double new_EXPTIME) {
    double  old_EXPTIME = this.EXPTIME;
    EXPTIME = new_EXPTIME;
   propertyChangeListeners.firePropertyChange("EXPTIME", Double.valueOf(old_EXPTIME), Double.valueOf(new_EXPTIME));
  }
  public double getEXPTIME() {
    return EXPTIME;
  } 
/*================================================================================================
/      setNEXP(int new_nexp)
/=================================================================================================*/
  public void setNEXP(int new_nexp) {
    int  old_nexp = this.nexp;
    this.nexp = new_nexp;
   propertyChangeListeners.firePropertyChange("nexp", Integer.valueOf(old_nexp), Integer.valueOf(new_nexp));
  }
  public int getNEXP() {
    return nexp;
  }  
 /*================================================================================================
/      setSlitwidth(double new)
/=================================================================================================*/
  public void setSlitwidth_string(java.lang.String new_slitwidth) {
    java.lang.String  old_slitwidth = this.slitwidth_string;
    slitwidth_string = new_slitwidth;
    propertyChangeListeners.firePropertyChange("slitwidth", old_slitwidth, new_slitwidth);
  }
  public java.lang.String getSlitwidth_string() {
    return slitwidth_string;
  } 

  /*================================================================================================
/      setSlitwidth(double new)
/=================================================================================================*/
  public void setSlitwidth(double new_slitwidth) {
    double  old_slitwidth = this.slitwidth;
    slitwidth = new_slitwidth;
    propertyChangeListeners.firePropertyChange("slitwidth", Double.valueOf(old_slitwidth), Double.valueOf(new_slitwidth));
  }
  public double getSlitwidth() {
    return slitwidth;
  }
 /*================================================================================================
/      setSlitwidth(double new)
/=================================================================================================*/
  public void setSlitOffset(Double new_slitoffset) {
    double  old_slitoffset = this.slitoffset;
    slitoffset = new_slitoffset;
    propertyChangeListeners.firePropertyChange("slitoffset", Double.valueOf(old_slitoffset), Double.valueOf(new_slitoffset));
  }
  public double getSlitOffset() {
    return slitoffset;
  } 
/*================================================================================================
/      setReadmode(java.lang.String new_Readmode)
/=================================================================================================*/
  public void setOBSMODE(java.lang.String new_ccdmode) {
    java.lang.String  old_ccdmode = this.ccdmode;
    this.ccdmode = new_ccdmode;
   propertyChangeListeners.firePropertyChange("ccdmode", (old_ccdmode), (new_ccdmode));
  }
  public java.lang.String getOBSMODE() {
    return ccdmode;
  }
/*================================================================================================
/      setBIN_SPEC(int new_bin_spec)
/=================================================================================================*/
  public void setBIN_SPEC(int new_bin_spec) {
    int  old_bin_spec = this.bin_spec;
    this.bin_spec = new_bin_spec;
   propertyChangeListeners.firePropertyChange("bin_spec", Integer.valueOf(old_bin_spec), Integer.valueOf(new_bin_spec));
  }
  public int getBIN_SPEC() {
    return bin_spec;
  }
/*================================================================================================
/      setBIN_SPEC(int new_bin_spec)
/=================================================================================================*/
  public void setBIN_SPACE(int new_bin_space) {
    int  old_bin_space = this.bin_space;
    this.bin_space = new_bin_space;
   propertyChangeListeners.firePropertyChange("bin_space", Integer.valueOf(old_bin_space), Integer.valueOf(new_bin_space));
  }
  public int getBIN_SPACE() {
    return bin_space;
  }  
/*================================================================================================
/      setCASS_RING_ANGLE(double newCASS_RING_ANGLE)
/=================================================================================================*/
  public void setRequestedSlitAngle(java.lang.String newCASS_RING_ANGLE) {
    java.lang.String  oldCASS_RING_ANGLE = this.requestedSlitAngle;
    this.requestedSlitAngle = newCASS_RING_ANGLE;
    propertyChangeListeners.firePropertyChange("cass_ring_angle", oldCASS_RING_ANGLE, newCASS_RING_ANGLE);
  }
  public java.lang.String getRequestedSlitAngle() {
    return requestedSlitAngle;
  } 
/*================================================================================================
/      setCASS_RING_ANGLE(double newCASS_RING_ANGLE)
/=================================================================================================*/
  public void setSLITANGLE(double newCASS_RING_ANGLE) {
    double  oldCASS_RING_ANGLE = this.slitangle;
    this.slitangle = newCASS_RING_ANGLE;
    propertyChangeListeners.firePropertyChange("casangle", oldCASS_RING_ANGLE, newCASS_RING_ANGLE);
  }
  public double getSLITANGLE() {
    return slitangle;
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
/*================================================================================================
/      add and remove Property Change Listeners
/=================================================================================================*/
public Instrument clone(){
    Instrument current = new Instrument();
    current.setBIN_SPACE(this.getBIN_SPACE());
    current.setBIN_SPEC(this.getBIN_SPEC());
    current.setRequestedSlitAngle(this.getRequestedSlitAngle());
    current.setEXPTIME(this.getEXPTIME());
    current.setEXPTIME_CODE(this.getEXPTIME_CODE());
    current.setExposuretime(this.getExposuretime());
    current.setNEXP(this.getNEXP());
    current.setOBSMODE(this.getOBSMODE());
    current.setSlitOffset(this.getSlitOffset());
    current.setSlitwidth(this.getSlitwidth());
    current.setSlitwidth_string(this.getSlitwidth_string());
  return current;  
}
}
