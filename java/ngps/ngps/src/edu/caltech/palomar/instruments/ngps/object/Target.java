/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package edu.caltech.palomar.instruments.ngps.object;

//import edu.caltech.palomar.instruments.ngps.object.*;
import java.beans.PropertyChangeListener;
import java.beans.PropertyChangeSupport;
import edu.caltech.palomar.instruments.ngps.object.Instrument;
import edu.caltech.palomar.instruments.ngps.object.SkyCoordinates;
import javax.swing.tree.DefaultMutableTreeNode;
/**
 *
 * @author jennifermilburn
 */
public class Target extends java.lang.Object{
  transient protected PropertyChangeSupport propertyChangeListeners = new PropertyChangeSupport(this);
  public java.lang.String name;
  public int              set_id;
  public int              order;
  public int              observationID;
  public int              sequence_number;
  public int              target_number;
  public java.lang.String state;
  public SkyCoordinates   sky        = new SkyCoordinates();
  public Instrument       instrument = new Instrument();
  public OTMmodel         otm        = new OTMmodel();
  public ETCmodel         etc        = new ETCmodel();
  private int             index;
  public DefaultMutableTreeNode target_node;
  public DefaultMutableTreeNode name_node;
  public DefaultMutableTreeNode state_node;
  public DefaultMutableTreeNode obsid_node;
  public DefaultMutableTreeNode order_node;
  public DefaultMutableTreeNode setid_node;
  public DefaultMutableTreeNode target_number_node;
  public DefaultMutableTreeNode sequence_number_node;
  public DefaultMutableTreeNode indexes_node;
  public static java.lang.String PENDING   = "PENDING";
  public static java.lang.String COMPLETED = "COMPLETED";
  public static java.lang.String EXPOSING  = "EXPOSING";
  private boolean                selected;
  public static int              MISSING_ID = -99;
  public java.lang.String        comment = new java.lang.String();
  public java.lang.String        note    = new java.lang.String();
  private boolean                active;
  private java.lang.String       owner;
/*================================================================================================
/      Observation()
/=================================================================================================*/
  public Target(){   
      constructTreeNode();
      initializeValues();
      setObservationID(MISSING_ID);
  }
/*================================================================================================
/      Observation()
/=================================================================================================*/
  public Target(java.lang.String new_name){      
     setName(new_name);
     constructTreeNode();
     initializeValues();
  }
 private void initializeValues(){
     setSTATE(PENDING);
     setSelected(false);
     setActive(true);
 }
/*================================================================================================
/          DefaultMutableTreeNode constructTreeNode()
/=================================================================================================*/
public DefaultMutableTreeNode constructTreeNode(){
    target_node             = new DefaultMutableTreeNode("TARGET: "+name);
    name_node               = new DefaultMutableTreeNode("NAME = "+name);
    state_node              = new DefaultMutableTreeNode("STATE = "+state);
    indexes_node            = new DefaultMutableTreeNode("INDEXES");
    obsid_node              = new DefaultMutableTreeNode("OBSID"+observationID);
    setid_node              = new DefaultMutableTreeNode("SET_ID +"+set_id);
    target_number_node      = new DefaultMutableTreeNode("TARGET NUMBER = "+target_number); 
    sequence_number_node    = new DefaultMutableTreeNode("SEQUENCE NUMBER = "+sequence_number); 
    target_node.add(name_node);
    target_node.add(state_node);
    indexes_node.add(obsid_node);
    indexes_node.add(setid_node);
    indexes_node.add(target_number_node);
    indexes_node.add(sequence_number_node);
    target_node.add(indexes_node);
    target_node.add(sky.constructTreeNode());
    target_node.add(instrument.constructTreeNode());
    target_node.add(etc.constructTreeNode());
  return target_node;
}
/*================================================================================================
/      setExposuretime(double new)
/=================================================================================================*/
  public void setName(java.lang.String new_name) {
    java.lang.String  old_name = this.name;
    name = new_name;
   propertyChangeListeners.firePropertyChange("name", old_name, new_name);
  }
  public java.lang.String getName() {
    return name;
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
/     setSTATE(java.lang.String new_state)
/=================================================================================================*/
  public void setSTATE(java.lang.String new_state) {
    java.lang.String  old_state = this.state;
    state = new_state;
   propertyChangeListeners.firePropertyChange("state", old_state, new_state);
  }
  public java.lang.String getSTATE() {
    return state;
  }
public void setActive(boolean new_active){
    boolean old_active = this.active;
    active = new_active;
    if(!active){
        setSTATE("INACTIVE");
    }else if(active){
        setSTATE("PENDING");
    }
    this.propertyChangeListeners.firePropertyChange("active", Boolean.valueOf(old_active),Boolean.valueOf(new_active));
}
public boolean isActive(){
    return active;
}
/*================================================================================================
/     setCOMMENT(java.lang.String new_comment)
/=================================================================================================*/
  public void setCOMMENT(java.lang.String new_comment) {
    java.lang.String  old_comment = this.comment;
    comment = new_comment;
   propertyChangeListeners.firePropertyChange("comment", old_comment, new_comment);
  }
  public java.lang.String getCOMMENT() {
    return comment;
  }   
/*================================================================================================
/     setCOMMENT(java.lang.String new_comment)
/=================================================================================================*/
  public void setNOTE(java.lang.String new_note){
    java.lang.String  old_note = this.note;
    note = new_note;
   propertyChangeListeners.firePropertyChange("note", old_note, new_note);
  }
  public java.lang.String getNOTE() {
    return note;
  }     
 /*================================================================================================
/     setSelected(boolean new_selected)
/=================================================================================================*/
  public void setSelected(boolean new_selected) {
    boolean  old_selected = this.selected;
    selected = new_selected;
   propertyChangeListeners.firePropertyChange("state", Boolean.valueOf(old_selected), Boolean.valueOf(new_selected));
  }
  public boolean isSelected() {
    return selected;
  }  
/*================================================================================================
/     setOrder(int new_order)
/=================================================================================================*/
  public void setObservationID(int new_observationID) {
    int  old_observationID = this.observationID;
    observationID = new_observationID;
   propertyChangeListeners.firePropertyChange("observationID", Integer.valueOf(old_observationID), Integer.valueOf(new_observationID));
  }
  public int getObservationID() {
    return observationID;
  }
/*================================================================================================
/     setIndex(int new_index)
/=================================================================================================*/
  public void setIndex(int new_index) {
    int  old_index = this.index;
    index = new_index;
   propertyChangeListeners.firePropertyChange("index", Integer.valueOf(old_index), Integer.valueOf(new_index));
  }
  public int getIndex() {
    return index;
  } 
/*================================================================================================
/     setOrder(int new_order)
/=================================================================================================*/
  public void setOrder(int new_order) {
    int  old_order = this.order;
    order = new_order;
   propertyChangeListeners.firePropertyChange("name", Integer.valueOf(old_order), Integer.valueOf(new_order));
  }
  public int getOrder() {
    return order;
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
/      setTarget_Number(int new_target_number) 
/=================================================================================================*/
  public void setTarget_Number(int new_target_number) {
    int  old_target_number = this.target_number;
    this.target_number = new_target_number;
   propertyChangeListeners.firePropertyChange("target_number", Integer.valueOf(old_target_number), Integer.valueOf(new_target_number));
  }
  public int getTarget_Number() {
    return target_number;
  }  
/*================================================================================================
/      setTarget_Number(int new_target_number) 
/=================================================================================================*/
  public void setSequence_Number(int new_sequence_number) {
    int  old_sequence_number = this.sequence_number;
    this.sequence_number = new_sequence_number;
   propertyChangeListeners.firePropertyChange("sequence_number", Integer.valueOf(old_sequence_number), Integer.valueOf(new_sequence_number));
  }
  public int getSequence_Number() {
    return sequence_number;
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
/     clone()
/=================================================================================================*/
public Target clone(){
    Target current = new Target();    
    current.setActive(this.isActive());
    current.setCOMMENT(this.getCOMMENT());
    current.setIndex(this.getIndex());
    current.setNOTE(this.getNOTE());
    current.setName(this.getName());
    current.setOWNER(this.getOWNER());
//    current.setObservationID(this.getObservationID());
    current.setOrder(this.getOrder());
    current.setSET_ID(this.getSET_ID());
    current.setSTATE(this.getSTATE());
    current.setSelected(false);
    current.setSequence_Number(this.getSequence_Number());
    current.setTarget_Number(this.getTarget_Number());
    current.etc        = this.etc.clone();
    current.instrument = this.instrument.clone();
    current.otm        = this.otm.clone();
    current.sky        = this.sky.clone();   
    return current;
}
}
