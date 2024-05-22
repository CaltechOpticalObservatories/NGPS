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
public class Owner {
  transient protected PropertyChangeSupport propertyChangeListeners = new PropertyChangeSupport(this);
  public java.lang.String owner_id           = new java.lang.String();
  public java.lang.String proposal_id        = new java.lang.String();
  public java.lang.String proposal_title     = new java.lang.String();
  public java.lang.String password           = new java.lang.String();
  public java.lang.String encrypted_password = new java.lang.String();
/*================================================================================================
/     Owner
/=================================================================================================*/
   public Owner(){     
   }  
/*================================================================================================
/      setOwner_ID(java.lang.String new_owner_id)
/=================================================================================================*/
  public void setOwner_ID(java.lang.String new_owner_id) {
    java.lang.String  old_owner_id = this.owner_id;
    this.owner_id = new_owner_id;
   propertyChangeListeners.firePropertyChange("owner_id", (old_owner_id), new_owner_id);
  }
  public java.lang.String getOwner_ID() {
    return owner_id;
  }    
/*================================================================================================
/      setProposal_ID(java.lang.String new_proposal_id) 
/=================================================================================================*/
  public void setProposal_ID(java.lang.String new_proposal_id) {
    java.lang.String  old_proposal_id = this.proposal_id;
    this.proposal_id = new_proposal_id;
   propertyChangeListeners.firePropertyChange("proposal_id", old_proposal_id, new_proposal_id);
  }
  public java.lang.String getProposal_ID() {
    return proposal_id;
  }  
/*================================================================================================
/      setProposal_ID(java.lang.String new_proposal_id) 
/=================================================================================================*/
  public void setProposal_Title(java.lang.String new_proposal_title) {
    java.lang.String  old_proposal_title = this.proposal_title;
    this.proposal_title = new_proposal_title;
   propertyChangeListeners.firePropertyChange("proposal_title", old_proposal_title, new_proposal_title);
  }
  public java.lang.String getProposal_Title() {
    return proposal_title;
  }    
/*================================================================================================
/      setPassword(java.lang.String new_password)
/=================================================================================================*/
  public void setPassword(java.lang.String new_password) {
    java.lang.String  old_password = this.password;
    this.password = new_password;
   propertyChangeListeners.firePropertyChange("password", old_password, new_password);
  }
  public java.lang.String getPassword() {
    return password;
  }     
/*================================================================================================
/     setEncryptedPassword(java.lang.String new_encrypted_password) 
/=================================================================================================*/
  public void setEncryptedPassword(java.lang.String new_encrypted_password) {
    java.lang.String  old_encrypted_password = this.encrypted_password;
    this.encrypted_password = new_encrypted_password;
   propertyChangeListeners.firePropertyChange("encrypted_password", old_encrypted_password, new_encrypted_password);
  }
  public java.lang.String getEncryptedPassword() {
    return encrypted_password;
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
