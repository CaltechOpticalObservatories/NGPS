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
public class ETCmodel extends java.lang.Object{
    transient protected PropertyChangeSupport propertyChangeListeners = new PropertyChangeSupport(this);
    private double SNR;
    private float WRANGE_LOW;
    private float WRANGE_HIGH;
    private java.lang.String channel = new java.lang.String();
    private double magnitude;    
    private java.lang.String magref  = new java.lang.String();
    private java.lang.String magref_system  = new java.lang.String();
    private java.lang.String magref_filter  = new java.lang.String();   
    private double seeing;    
    private java.lang.String srcmodel  = new java.lang.String();
/*================================================================================================
/   Constructor
/=================================================================================================*/
 public ETCmodel(){
     initializeValues();
 }   
/*================================================================================================
/     setSNR(double new_SNR)
/=================================================================================================*/
 public void initializeValues(){
    setSNR(5.0);
    setWRANGE_LOW(500);
    setWRANGE_HIGH(510);
    setChannel("G");
    setMagnitude(18.0);
    setMagref("AB match");
    setSeeing(1.25);
    setSrcmodel(" "); //-model constant
 } 
/*================================================================================================
/     setSNR(double new_SNR)
/=================================================================================================*/
  public void setSNR(double new_SNR) {
    double  old_SNR = this.SNR;
    SNR = new_SNR;
   propertyChangeListeners.firePropertyChange("SNR", old_SNR, new_SNR);
  }
  public double getSNR() {
    return SNR;
  }
/*================================================================================================
/     setWRANGE_LOW(double new_WRANGE_LOW)
/=================================================================================================*/
  public void setWRANGE_LOW(float new_WRANGE_LOW) {
    float  old_WRANGE_LOW = this.WRANGE_LOW;
    WRANGE_LOW = new_WRANGE_LOW;
   propertyChangeListeners.firePropertyChange("WRANGE_LOW", old_WRANGE_LOW, new_WRANGE_LOW);
  }
  public float getWRANGE_LOW() {
    return WRANGE_LOW;
  }  
/*================================================================================================
/     setWRANGE_HIGH(double new_WRANGE_HIGH)
/=================================================================================================*/
  public void setWRANGE_HIGH(float new_WRANGE_HIGH) {
    float  old_WRANGE_HIGH = this.WRANGE_HIGH;
    WRANGE_HIGH = new_WRANGE_HIGH;
   propertyChangeListeners.firePropertyChange("WRANGE_HIGH", old_WRANGE_HIGH, new_WRANGE_HIGH);
  }
  public float getWRANGE_HIGH() {
    return WRANGE_HIGH;
  }   
/*================================================================================================
/      setChannel(java.lang.String channel)
/=================================================================================================*/
  public void setChannel(java.lang.String new_channel) {
    java.lang.String  old_channel = this.channel;
    channel = new_channel.toUpperCase();
    propertyChangeListeners.firePropertyChange("channel", old_channel, new_channel);
  }
  public java.lang.String getChannel() {
    return channel.toUpperCase();
  } 
/*================================================================================================
/     setMagnitude(double new_magnitude)
/=================================================================================================*/
  public void setMagnitude(double new_magnitude) {
    double  old_magnitude = this.magnitude;
    magnitude = new_magnitude;
   propertyChangeListeners.firePropertyChange("magnitude", old_magnitude, new_magnitude);
  }
  public double getMagnitude() {
    return magnitude;
  }
/*================================================================================================
/      setMagref(java.lang.String channel)
/=================================================================================================*/
  public void setMagref(java.lang.String new_magref) {
    java.lang.String  old_magref = this.magref;
    magref = new_magref;
    try{
      java.util.StringTokenizer st4 = new java.util.StringTokenizer(new_magref," ");
      java.lang.String    magref_system  = st4.nextToken();
      java.lang.String    mageref_filter = st4.nextToken();              
              setMagref_system(magref_system);
              setMagref_filter(mageref_filter);
    }catch(Exception e){
        
    }
    propertyChangeListeners.firePropertyChange("magref", old_magref, new_magref);
  }
  public java.lang.String getMagref() {
    return magref;
  } 
/*================================================================================================
/      setMagref_system(java.lang.String channel)
/=================================================================================================*/
  public void setMagref_system(java.lang.String new_magref_system) {
    java.lang.String  old_magref_system = this.magref_system;
    magref_system = new_magref_system;
    propertyChangeListeners.firePropertyChange("magref_system", old_magref_system, new_magref_system);
  }
  public java.lang.String getMagref_system() {
    return magref_system;
  } 
/*================================================================================================
/      setMagref_system(java.lang.String channel)
/=================================================================================================*/
  public void setMagref_filter(java.lang.String new_magref_filter) {
    java.lang.String  old_magref_filter = this.magref_filter;
    magref_filter = new_magref_filter;
    propertyChangeListeners.firePropertyChange("magref_filter", old_magref_filter, new_magref_filter);
  }
  public java.lang.String getMagref_filter() {
    return magref_filter;
  }   
/*================================================================================================
/     setSeeing(double new_seeing)
/=================================================================================================*/
  public void setSeeing(double new_seeing) {
    double  old_seeing = this.seeing;
    seeing = new_seeing;
   propertyChangeListeners.firePropertyChange("seeing", old_seeing, new_seeing);
  }
  public double getSeeing() {
    return seeing;
  }  
/*================================================================================================
/      setSrcmodel(java.lang.String srcmodel)
/=================================================================================================*/
  public void setSrcmodel(java.lang.String new_srcmodel) {
    java.lang.String  old_srcmodel = this.srcmodel;
    srcmodel = new_srcmodel;
    propertyChangeListeners.firePropertyChange("srcmodel", old_srcmodel, new_srcmodel);
  }
  public java.lang.String getSrcmodel() {
    return srcmodel;
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
/    clone()
/=================================================================================================*/
 public ETCmodel clone(){
      ETCmodel current = new ETCmodel();
      current.setChannel(this.getChannel());
      current.setMagnitude(this.getMagnitude());
      current.setMagref(this.getMagref());
      current.setMagref_filter(this.getMagref_filter());
      current.setMagref_system(this.getMagref_system());
      current.setSNR(this.getSNR());
      current.setSeeing(this.getSeeing());
      current.setSrcmodel(this.getSrcmodel());
      current.setWRANGE_HIGH(this.getWRANGE_HIGH());
      current.setWRANGE_LOW(this.getWRANGE_LOW());
    return current;
  }
}
