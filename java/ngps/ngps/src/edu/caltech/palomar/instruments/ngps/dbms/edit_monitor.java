/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package edu.caltech.palomar.instruments.ngps.dbms;

import java.beans.PropertyChangeListener;
import java.beans.PropertyChangeSupport;

/**
 *
 * @author developer
 */
/*================================================================================================
/      edit_monitor
/=================================================================================================*/
public class edit_monitor {
    transient public PropertyChangeSupport propertyChangeListeners = new PropertyChangeSupport(this);
    private       boolean edited;
/*================================================================================================
/      edit_monitor
/=================================================================================================*/
public edit_monitor(){
    setEdited(false);
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
/      setEdited(boolean new_edited) 
/=================================================================================================*/
  public void setEdited(boolean new_edited) {
    boolean  old_edited = this.edited;
    this.edited = new_edited;
    propertyChangeListeners.firePropertyChange("edited", Boolean.valueOf(old_edited), Boolean.valueOf(new_edited));
  }
  public boolean isEdited() {
    return edited;
  }    
}
