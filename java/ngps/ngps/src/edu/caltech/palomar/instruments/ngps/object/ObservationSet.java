/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package edu.caltech.palomar.instruments.ngps.object;

import java.beans.PropertyChangeListener;
import java.beans.PropertyChangeSupport;
import java.sql.Timestamp;
import java.time.ZoneId;
import java.time.ZonedDateTime;
import java.util.HashMap;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.DefaultTreeSelectionModel;
import javax.swing.tree.DefaultTreeModel;

/**
 *
 * @author jennifermilburn
 */
public class ObservationSet {
   transient protected PropertyChangeSupport propertyChangeListeners = new PropertyChangeSupport(this);
   private java.lang.String set_name = new java.lang.String();
   private java.lang.String owner = new java.lang.String();
   private java.lang.String state = new java.lang.String();
   private int              num_observations;
   private Timestamp        creation_timestamp;
   private Timestamp        lastupdate_timestamp;
   private int              set_id;
   public HashMap<String, Target> observationHashMap = new HashMap<String, Target>();  
   public static java.lang.String PENDING   = "PENDING";
   public static java.lang.String COMPLETED = "COMPLETED";
   public static java.lang.String RUNNING   = "RUNNING";
   public DefaultMutableTreeNode observation_set_node;
   public DefaultMutableTreeNode owner_node;
   public DefaultMutableTreeNode setid_node;
   public DefaultMutableTreeNode state_node;
   public DefaultMutableTreeNode creation_date_node;
   public DefaultMutableTreeNode last_updated_node;
   public DefaultTreeModel observation_set_tree_model;
/*================================================================================================
/     ObservationSet()
/=================================================================================================*/
   public ObservationSet(){
       constructTreeNode();
   } 
/*================================================================================================
/     HashMap<String, Observation> getObservationHashMap()
/=================================================================================================*/
 public DefaultTreeModel getTreeModel(){
    return observation_set_tree_model; 
 }
/*================================================================================================
/     HashMap<String, Observation> getObservationHashMap()
/=================================================================================================*/
public  HashMap<String, Target> getObservationHashMap(){
   return observationHashMap; 
} 
 /*================================================================================================
/          DefaultMutableTreeNode constructTreeNode()
/=================================================================================================*/
public DefaultMutableTreeNode constructTreeNode(){
    observation_set_node = new DefaultMutableTreeNode("TARGET SET = "+set_name);
    owner_node           = new DefaultMutableTreeNode("OWNER = "+owner);
    setid_node           = new DefaultMutableTreeNode("SET ID = "+set_id);
    state_node           = new DefaultMutableTreeNode("STATE = "+state);
 //   creation_date_node   = new DefaultMutableTreeNode("CREATION DATE = "+creation_timestamp.toString());
 //   last_updated_node    = new DefaultMutableTreeNode("LAST UPDATE DATE = "+last_updated_node.toString());
    observation_set_node.add(owner_node);
    observation_set_node.add(setid_node);
    observation_set_node.add(state_node);
//    observation_set_node.add(creation_date_node);
//    observation_set_node.add(last_updated_node);
    observation_set_tree_model = new DefaultTreeModel(observation_set_node);
  return observation_set_node;
}
/*================================================================================================
/     Timestamp constructTimestamp()
/=================================================================================================*/
public Timestamp constructTimestamp(){
   ZonedDateTime now     = ZonedDateTime.now();
   ZonedDateTime now_utc = now.withZoneSameInstant(ZoneId.of("UTC"));
   Timestamp current_timestamp = new Timestamp(now_utc.getYear()-1900,now_utc.getMonthValue()-1,now_utc.getDayOfMonth(),
                                                    now_utc.getHour(),now_utc.getMinute(),now_utc.getSecond(),0);
   return current_timestamp;
}   
/*================================================================================================
/      setSET_NAME(java.lang.String new_set_name)
/=================================================================================================*/
  public void setSET_NAME(java.lang.String new_set_name) {
    java.lang.String  old_set_name = this.set_name;
    this.set_name = new_set_name;
   propertyChangeListeners.firePropertyChange("set_name", (old_set_name), (new_set_name));
  }
  public java.lang.String getSET_NAME() {
    return set_name;
  }   
/*================================================================================================
/      setOWNER(java.lang.String new_owner)
/=================================================================================================*/
  public void setOWNER(java.lang.String new_owner) {
    java.lang.String  old_owner = this.owner;
    this.owner = new_owner;
   propertyChangeListeners.firePropertyChange("owner", (old_owner), (new_owner));
  }
  public java.lang.String getOWNER() {
    return owner;
  }  
 /*================================================================================================
/      setOWNER(java.lang.String new_owner)
/=================================================================================================*/
  public void setSTATE(java.lang.String new_state) {
    java.lang.String  old_state = this.state;
    this.state = new_state;
   propertyChangeListeners.firePropertyChange("state", (old_state), (new_state));
  }
  public java.lang.String getSTATE() {
    return state;
  }   
/*================================================================================================
/      setNUM_OBSERVATIONS(int new_num_observations)
/=================================================================================================*/
  public void setNUM_OBSERVATIONS(int new_num_observations) {
    int  old_num_observations = this.num_observations;
    this.num_observations = new_num_observations;
   propertyChangeListeners.firePropertyChange("num_observations", Integer.valueOf(old_num_observations), Integer.valueOf(new_num_observations));
  }
  public int getNUM_OBSERVATIONS() {
    return num_observations;
  }  
/*================================================================================================
/      setNUM_OBSERVATIONS(int new_num_observations)
/=================================================================================================*/
  public void setSET_ID(int new_set_id) {
    int  old_set_id = this.set_id;
    this.set_id = new_set_id;
   propertyChangeListeners.firePropertyChange("set_id", Integer.valueOf(old_set_id), Integer.valueOf(new_set_id));
  }
  public int getSET_ID() {
    return set_id;
  }  
/*================================================================================================
/      setCreation_Timestamp(Timestamp new_creation_timestamp)
/=================================================================================================*/
  public void setCreation_Timestamp(Timestamp new_creation_timestamp) {
    Timestamp  old_creation_timestamp = this.creation_timestamp;
    this.creation_timestamp = new_creation_timestamp;
   propertyChangeListeners.firePropertyChange("creation_timestamp", old_creation_timestamp, new_creation_timestamp);
  }
  public Timestamp getCreation_Timestamp() {
    return creation_timestamp;
  }
/*================================================================================================
/      setCreation_Timestamp(Timestamp new_creation_timestamp)
/=================================================================================================*/
  public void setLastUpdate_Timestamp(Timestamp new_lastupdate_timestamp) {
    Timestamp  old_lastupdate_timestamp = this.lastupdate_timestamp;
    this.lastupdate_timestamp = new_lastupdate_timestamp;
   propertyChangeListeners.firePropertyChange("lastupdate_timestamp", old_lastupdate_timestamp, new_lastupdate_timestamp);
  }
  public Timestamp getLastUpdate_Timestamp() {
    return lastupdate_timestamp;
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
