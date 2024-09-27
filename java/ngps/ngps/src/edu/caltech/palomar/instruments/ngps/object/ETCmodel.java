/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package edu.caltech.palomar.instruments.ngps.object;

import java.beans.PropertyChangeListener;
import java.beans.PropertyChangeSupport;
import javax.swing.tree.DefaultMutableTreeNode;

/**
 *
 * @author jennifermilburn
 */
public class ETCmodel extends java.lang.Object{
    transient protected PropertyChangeSupport propertyChangeListeners = new PropertyChangeSupport(this);
    private double SNR;
    private int WRANGE_LOW;
    private int WRANGE_HIGH;
    private java.lang.String channel = new java.lang.String();
    private double magnitude;    
    private java.lang.String magref  = new java.lang.String();
    private java.lang.String magref_system  = new java.lang.String();
    private java.lang.String magref_filter  = new java.lang.String();   
    private double seeing;    
    private java.lang.String srcmodel  = new java.lang.String();
    public static java.lang.String CHANNEL_U = "U";
    public static java.lang.String CHANNEL_G = "G";
    public static java.lang.String CHANNEL_R = "R";
    public static java.lang.String CHANNEL_I = "I";
    public DefaultMutableTreeNode etc_node;
    public DefaultMutableTreeNode snr_node;
    public DefaultMutableTreeNode wrange_node;
    public DefaultMutableTreeNode wrange_low_node;
    public DefaultMutableTreeNode wrange_high_node;
    public DefaultMutableTreeNode channel_node;
    public DefaultMutableTreeNode magnitude_node;
    public DefaultMutableTreeNode magrefsystem_node;
    public DefaultMutableTreeNode magreffilter_node;
    public DefaultMutableTreeNode srcmodel_node;
/*================================================================================================
/   Constructor
/=================================================================================================*/
 public ETCmodel(){
     initializeValues();
     constructTreeNode();
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
/          DefaultMutableTreeNode constructTreeNode()
/=================================================================================================*/
public DefaultMutableTreeNode constructTreeNode(){
    etc_node          = new DefaultMutableTreeNode("EXPOSURE TIME CALCULATOR");
    snr_node          = new DefaultMutableTreeNode("SNR = "+SNR);
    wrange_node       = new DefaultMutableTreeNode("WRANGE");
    wrange_low_node   = new DefaultMutableTreeNode("WAVELENGTH LOW"+WRANGE_LOW);
    wrange_high_node  = new DefaultMutableTreeNode("WAVELENGTH HIGH"+WRANGE_HIGH);
    channel_node      = new DefaultMutableTreeNode("CHANNEL = "+channel);    
    magnitude_node    = new DefaultMutableTreeNode("MAGNITUDE = "+magnitude);
    magrefsystem_node = new DefaultMutableTreeNode("MAGNITUDE REFERENCE SYSTEM = "+magref_system);
    magreffilter_node = new DefaultMutableTreeNode("MAGNITUDE REFERENCE FILTER = "+magref_filter);    
    srcmodel_node     = new DefaultMutableTreeNode("SRCMODEL = "+srcmodel);
    etc_node.add(snr_node);
    wrange_node.add(wrange_low_node);
    wrange_node.add(wrange_high_node);
    etc_node.add(wrange_node);
    etc_node.add(magnitude_node);
    etc_node.add(magrefsystem_node);
    etc_node.add(magreffilter_node);
    etc_node.add(srcmodel_node);
  return etc_node;
}
/*================================================================================================
/     setSNR(double new_SNR)
/=================================================================================================*/
  public void setSNR(double new_SNR) {
    double  old_SNR = this.SNR;
    SNR = new_SNR;
   propertyChangeListeners.firePropertyChange("SNR", Double.valueOf(old_SNR), Double.valueOf(new_SNR));
  }
  public double getSNR() {
    return SNR;
  }
/*================================================================================================
/     setWRANGE_LOW(double new_WRANGE_LOW)
/=================================================================================================*/
  public void setWRANGE_LOW(int new_WRANGE_LOW) {
    int  old_WRANGE_LOW = this.WRANGE_LOW;
    WRANGE_LOW = new_WRANGE_LOW;
   propertyChangeListeners.firePropertyChange("WRANGE_LOW", Integer.valueOf(old_WRANGE_LOW), Integer.valueOf(new_WRANGE_LOW));
  }
  public int getWRANGE_LOW() {
    return WRANGE_LOW;
  }  
/*================================================================================================
/     setWRANGE_HIGH(double new_WRANGE_HIGH)
/=================================================================================================*/
  public void setWRANGE_HIGH(int new_WRANGE_HIGH) {
    int  old_WRANGE_HIGH = this.WRANGE_HIGH;
    WRANGE_HIGH = new_WRANGE_HIGH;
   propertyChangeListeners.firePropertyChange("WRANGE_HIGH", Integer.valueOf(old_WRANGE_HIGH), Integer.valueOf(new_WRANGE_HIGH));
  }
  public int getWRANGE_HIGH() {
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
   propertyChangeListeners.firePropertyChange("magnitude", Double.valueOf(old_magnitude), Double.valueOf(new_magnitude));
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
   propertyChangeListeners.firePropertyChange("seeing", Double.valueOf(old_seeing), Double.valueOf(new_seeing));
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
