/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package edu.caltech.palomar.dhe2;

import java.beans.PropertyChangeListener;
import java.beans.PropertyChangeSupport;

/**
 *
 * @author jennifermilburn
 */
public class ObservationSequencerObject {
    transient public PropertyChangeSupport propertyChangeListeners = new PropertyChangeSupport(this);
    private java.lang.String state = new java.lang.String();
/*================================================================================================
/      add and remove Property Change Listeners
/=================================================================================================*/
public ObservationSequencerObject(){
    
}
/*================================================================================================
/      setReadmode(java.lang.String new_Readmode)
/=================================================================================================*/
  public void setSTATE(java.lang.String new_state) {
    java.lang.String  old_state = this.state;
    this.state = new_state;
   propertyChangeListeners.firePropertyChange("STATE", (old_state), (new_state));
  }
  public java.lang.String getSTATE() {
    return state;
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
