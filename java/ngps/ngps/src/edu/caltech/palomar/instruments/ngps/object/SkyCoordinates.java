/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package edu.caltech.palomar.instruments.ngps.object;

import edu.caltech.palomar.instruments.ngps.object.*;
import java.beans.PropertyChangeListener;
import java.beans.PropertyChangeSupport;
import javax.swing.tree.DefaultMutableTreeNode;
import jsky.science.Coordinates;
import jsky.coords.DMS;
import jsky.coords.HMS;
/**
 *
 * @author jennifermilburn
 */
public class SkyCoordinates extends java.lang.Object{
  transient protected PropertyChangeSupport propertyChangeListeners = new PropertyChangeSupport(this);
  private double      RA;
  private double      DEC;
//  private String      EPOCH;
  private String      right_ascension;
  private String      declination;
  private Coordinates myCoordinates       = new Coordinates();
  private String      equinox_string;
  private double      OFFSET_DEC;
  private double      OFFSET_RA;
  private String      airmass_max;
  public  DefaultMutableTreeNode  sky_node;
  public  DefaultMutableTreeNode  ra_node;
  public  DefaultMutableTreeNode  dec_node;
  public  DefaultMutableTreeNode  epoch_node;
  public  DefaultMutableTreeNode  offset_node;
  public  DefaultMutableTreeNode  ra_offset_node;
  public  DefaultMutableTreeNode  dec_offset_node;
/*================================================================================================
/        CONSTRUCTOR
/=================================================================================================*/
public SkyCoordinates(){
    initializeValues();
    constructTreeNode();
}
public void initializeValues(){
//    setEPOCH("J2000"); 
    setRightAscension(null);
    setDeclination(null);
}
/*================================================================================================
/          DefaultMutableTreeNode constructTreeNode()
/=================================================================================================*/
public DefaultMutableTreeNode constructTreeNode(){
    sky_node        = new DefaultMutableTreeNode("SKY COORDINATES");
    ra_node         = new DefaultMutableTreeNode("RA = "+right_ascension);
    dec_node        = new DefaultMutableTreeNode("DEC = "+declination);
//S    epoch_node      = new DefaultMutableTreeNode("EPOCH = "+EPOCH);
    offset_node     = new DefaultMutableTreeNode("OFFSETS");
    ra_offset_node  = new DefaultMutableTreeNode("RA OFFSET = "+OFFSET_RA);
    dec_offset_node = new DefaultMutableTreeNode("DEC OFFSET = "+OFFSET_DEC);
    sky_node.add(ra_node);
    sky_node.add(dec_node);
//    sky_node.add(epoch_node);
    sky_node.add(offset_node);
    offset_node.add(ra_offset_node);
    offset_node.add(ra_offset_node);
  return sky_node;
}
/*================================================================================================
/      setRA(double newRA)
/=================================================================================================*/
  public void setRA(double newRA) {
    double  oldRA = this.RA;
    this.RA = newRA;
    myCoordinates.setRa(newRA);
   propertyChangeListeners.firePropertyChange("RA", Double.valueOf(oldRA), Double.valueOf(newRA));
  }
  public double getRA() {
    return RA;
  }
/*================================================================================================
/      setDEC(double newDEC)
/=================================================================================================*/
  public void setDEC(double newDEC) {
    double  oldDEC = this.DEC;
    this.DEC = newDEC;
    myCoordinates.setDec(newDEC);
    propertyChangeListeners.firePropertyChange("DEC", Double.valueOf(oldDEC), Double.valueOf(newDEC));
  }
  public double getDEC() {
    return DEC;
  }
/*================================================================================================
/      setOFFSET_RA(double newRA)
/=================================================================================================*/
  public void setOFFSET_RA(double newOFFSET_RA) {
    double  oldOFFSET_RA = this.OFFSET_RA;
    this.OFFSET_RA = newOFFSET_RA;
   propertyChangeListeners.firePropertyChange("OFFSET_RA", Double.valueOf(oldOFFSET_RA), Double.valueOf(newOFFSET_RA));
  }
  public double getOFFSET_RA() {
    return OFFSET_RA;
  } 
 /*================================================================================================
/      setOFFSET_RA(double newRA)
/=================================================================================================*/
  public void setOFFSET_DEC(double newOFFSET_DEC) {
    double  oldOFFSET_DEC = this.OFFSET_DEC;
    this.OFFSET_DEC = newOFFSET_DEC;
   propertyChangeListeners.firePropertyChange("OFFSET_DEC", Double.valueOf(oldOFFSET_DEC), Double.valueOf(newOFFSET_DEC));
  }
  public double getOFFSET_DEC() {
    return OFFSET_DEC;
  }  
/*================================================================================================
/      setEQUINOX(double newEQUINOX)
/=================================================================================================*/
//  public void setEPOCH(java.lang.String newEQUINOX) {
//    java.lang.String  oldEQUINOX = this.EPOCH;
//    this.EPOCH = newEQUINOX;
//    propertyChangeListeners.firePropertyChange("EPOCH", oldEQUINOX, newEQUINOX);
//  }
//  public java.lang.String getEPOCH() {
//    return EPOCH;
//  }
/*================================================================================================
/          STRING REPRESENTATIONS OF THE TELESCOPE ORIENTATION
/=================================================================================================*/
/*================================================================================================
/      setRightAscension
/=================================================================================================*/
  public void setRightAscension(String right_ascension) {
      if(right_ascension != null){
          try{
              HMS myHMS = new HMS(right_ascension);
//     double newRA = Coordinates.valueOf(right_ascension, declination).getRa();
              setRA(myHMS.getVal());
              String  oldright_ascension = this.right_ascension;
              this.right_ascension = right_ascension;
              propertyChangeListeners.firePropertyChange("right_ascension", oldright_ascension, right_ascension);
          }catch(Exception e){
//             System.out.println(e.toString());
             this.right_ascension = "";
          }
      }
  }
  public String getRightAscension() {
    return right_ascension;
  }
/*================================================================================================
/      setDeclination
/=================================================================================================*/
  public void setDeclination(String new_declination) {
    if(new_declination != null)  {
        try{
           DMS myDMS = new DMS(new_declination);
 //     double newDEC = Coordinates.valueOf(right_ascension, declination).getDec();
           setDEC(myDMS.getVal());
           String  oldDeclination = this.declination;
           this.declination = new_declination;
           propertyChangeListeners.firePropertyChange("declination", oldDeclination, new_declination);
        }catch(Exception e){
//           System.out.println(e.toString());
           this.declination = "";
        }
    }
  }
  public String getDeclination() {
    return declination;
  }
/*================================================================================================
/       setEquinox(String equinox_string)
/=================================================================================================*/
  public void setEquinox(String equinox_string) {
      // We are expecting one of the following strings "J2000" or "B1950"
    String  oldequinox_string = this.equinox_string;
    this.equinox_string = equinox_string;
    propertyChangeListeners.firePropertyChange("equinox_string", oldequinox_string, equinox_string);
  }
  public String getEquinox() {
    return equinox_string;
  }
/*================================================================================================
/       setAIRMASS_MAX(String airmass_max_string)
/=================================================================================================*/
  public void setAIRMASS_MAX(String airmass_max_string) {
      // We are expecting one of the following strings "J2000" or "B1950"
    String  oldairmass_max = this.airmass_max;
    this.airmass_max = airmass_max_string;
    propertyChangeListeners.firePropertyChange("airmass_max", oldairmass_max, airmass_max);
  }
  public String getAIRMASS_MAX() {
    return airmass_max;
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
/      clone
/=================================================================================================*/
public SkyCoordinates clone(){
    SkyCoordinates current = new SkyCoordinates();
    current.setRA(this.getRA()); 
    current.setDEC(this.getDEC());
    current.setRightAscension(this.getRightAscension());
    current.setDeclination(this.getDeclination());
//    current.setEPOCH(this.getEPOCH());
    current.setEquinox(this.getEquinox());
    current.setAIRMASS_MAX(this.getAIRMASS_MAX());
    current.setOFFSET_RA(this.getOFFSET_RA());
    current.setOFFSET_DEC(this.getOFFSET_DEC());
  return current;  
}  
}
