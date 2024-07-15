/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package edu.caltech.palomar.instruments.ngps.object;

import java.beans.PropertyChangeListener;
import java.beans.PropertyChangeSupport;
import java.sql.Timestamp;

/**
 *
 * @author jennifermilburn
 */
public class OTMmodel extends java.lang.Object{
   transient protected PropertyChangeSupport propertyChangeListeners = new PropertyChangeSupport(this);
   private double OTMpa;
   private double OTMcass;
   private double OTMdPadt;
   private double OTMwait;
   private java.lang.String OTMflag = new java.lang.String();
   private Timestamp OTMslewgo;
   private java.lang.String OTMlast = new java.lang.String();
   private double OTMslew;
   private double OTMdead;
   private Timestamp OTMstart;
   private Timestamp OTMend;
   private double airmass_start;
   private double airmass_end;
   private double skymag;  
   private double OTMexpt;
   private double OTMslitwidth;
   private double OTMslitangle;
   private java.lang.String OTMSNR;
   private double OTMres;
   private java.lang.String OTMname;
   private java.lang.String OTMmoon;
   private double OTMseeing;
   private java.lang.String pointmode;
   private Timestamp notbefore;
/*================================================================================================
/   Constructor
/=================================================================================================*/
 public OTMmodel(){
   initialize();  
 }  
 public void initialize(){
    setOTMpa(0.0);
    setOTMcass(0.0);
    setOTMdPadt(0.0);
    setOTMwait(0);
    setOTMflag("");
    setOTMslewgo(new Timestamp(System.currentTimeMillis()));
    setOTMlast("");
    setOTMslew(0.0);
    setOTMdead(0.0);
    setOTMstart(new Timestamp(System.currentTimeMillis()));
    setOTMend(new Timestamp(System.currentTimeMillis()));
    setOTMAirmass_end(1.0);
    setOTMAirmass_start(1.0);
    setSkymag(21.0);
    setOTMexpt(1.0);
    setOTMslitwidth(1.0);  
    setOTMmoon("NONE");
    setOTMslitangle(0.0); 
    setOTMpointmode("SLIT");
 }
/*================================================================================================
/     setOTMname(java.lang.String new_OTMname) 
/=================================================================================================*/
  public void setOTMname(java.lang.String new_OTMname) {
    java.lang.String  old_OTMname = this.OTMname;
    OTMname = new_OTMname;
   propertyChangeListeners.firePropertyChange("OTMname", old_OTMname, new_OTMname);
  }
  public java.lang.String getOTMname() {
    return OTMname;
  } 
/*================================================================================================
/     setOTMpa(double new_OTMpa)
/=================================================================================================*/
  public void setOTMSNR(java.lang.String new_OTMSNR) {
    java.lang.String  old_OTMSNR = this.OTMSNR;
    OTMSNR = new_OTMSNR;
   propertyChangeListeners.firePropertyChange("OTMSNR", old_OTMSNR, new_OTMSNR);
  }
  public java.lang.String getOTMSNR() {
    return OTMSNR;
  } 
/*================================================================================================
/     setOTMpa(double new_OTMpa)
/=================================================================================================*/
  public void setOTMres(double new_OTMres) {
    double  old_OTMres = this.OTMres;
    OTMres = new_OTMres;
   propertyChangeListeners.firePropertyChange("OTMres", Double.valueOf(old_OTMres), Double.valueOf(new_OTMres));
  }
  public double getOTMres() {
    return OTMres;
  }  
/*================================================================================================
/     setOTMpa(double new_OTMpa)
/=================================================================================================*/
  public void setOTMpa(double new_OTMpa) {
    double  old_OTMpa = this.OTMpa;
    OTMpa = new_OTMpa;
   propertyChangeListeners.firePropertyChange("OTMpa", Double.valueOf(old_OTMpa), Double.valueOf(new_OTMpa));
  }
  public double getOTMpa() {
    return OTMpa;
  }   
/*================================================================================================
/     setOTMcass(double new_OTMpa)
/=================================================================================================*/
  public void setOTMcass(double new_OTMcass) {
    double  old_OTMcass = this.OTMcass;
    OTMcass = new_OTMcass;
   propertyChangeListeners.firePropertyChange("OTMcass", Double.valueOf(old_OTMcass), Double.valueOf(new_OTMcass));
  }
  public double getOTMcass() {
    return OTMcass;
  }  
 /*================================================================================================
/     setOTMcass(double new_OTMpa)
/=================================================================================================*/
  public void setOTMslitangle(double new_OTMslitangle) {
    double  old_OTMslitangle = this.OTMslitangle;
    OTMslitangle = new_OTMslitangle;
   propertyChangeListeners.firePropertyChange("OTMslitangle", Double.valueOf(old_OTMslitangle), Double.valueOf(new_OTMslitangle));
  }
  public double getOTMslitangle() {
    return OTMslitangle;
  }  
/*================================================================================================
/     setOTMdPadt(double new_OTMpa)
/=================================================================================================*/
  public void setOTMdPadt(double new_OTMdPadt) {
    double  old_OTMdPadt = this.OTMdPadt;
    OTMdPadt = new_OTMdPadt;
   propertyChangeListeners.firePropertyChange("OTMdPadt", Double.valueOf(old_OTMdPadt), Double.valueOf(new_OTMdPadt));
  }
  public double getOTMdPadt() {
    return OTMdPadt;
  }  
/*================================================================================================
/      setOTMwait(int new_nexp)
/=================================================================================================*/
  public void setOTMwait(double new_OTMwait) {
    double  old_OTMwait = this.OTMwait;
    this.OTMwait = new_OTMwait;
   propertyChangeListeners.firePropertyChange("OTMwait", Double.valueOf(old_OTMwait), Double.valueOf(new_OTMwait));
  }
  public double getOTMwait() {
    return OTMwait;
  }   
/*================================================================================================
/      setOTMflag(double new_OTMflag)
/=================================================================================================*/
  public void setOTMflag(java.lang.String new_OTMflag) {
    java.lang.String  old_OTMflag = this.OTMflag;
    OTMflag = new_OTMflag;
    propertyChangeListeners.firePropertyChange("OTMflag", old_OTMflag, new_OTMflag);
  }
  public java.lang.String getOTMflag() {
    return OTMflag;
  } 
/*================================================================================================
/      setOTMslewgo(Timestamp new_OTMslewgo)
/=================================================================================================*/
  public void setOTMslewgo(Timestamp new_OTMslewgo) {
    Timestamp  old_OTMslewgo = this.OTMslewgo;
    this.OTMslewgo = new_OTMslewgo;
   propertyChangeListeners.firePropertyChange("OTMslewgo", old_OTMslewgo, new_OTMslewgo);
  }
  public Timestamp getOTMslewgo() {
    return OTMslewgo;
  }
/*================================================================================================
/      setOTMlast(double new_OTMlast)
/=================================================================================================*/
  public void setOTMlast(java.lang.String new_OTMlast) {
    java.lang.String  old_OTMlast = this.OTMlast;
    OTMlast = new_OTMlast;
    propertyChangeListeners.firePropertyChange("OTMlast", old_OTMlast, new_OTMlast);
  }
  public java.lang.String getOTMlast() {
    return OTMlast;
  }  
/*================================================================================================
/     setOTMslew(double new_OTMslew)
/=================================================================================================*/
  public void setOTMslew(double new_OTMslew) {
    double  old_OTMslew = this.OTMslew;
    OTMslew = new_OTMslew;
   propertyChangeListeners.firePropertyChange("OTMslew", Double.valueOf(old_OTMslew), Double.valueOf(new_OTMslew));
  }
  public double getOTMslew() {
    return OTMslew;
  }    
/*================================================================================================
/     setOTMdead(double new_OTMslew)
/=================================================================================================*/
  public void setOTMdead(double new_OTMdead) {
    double  old_OTMdead = this.OTMdead;
    OTMdead = new_OTMdead;
   propertyChangeListeners.firePropertyChange("OTMdead", Double.valueOf(old_OTMdead), Double.valueOf(new_OTMdead));
  }
  public double getOTMdead() {
    return OTMdead;
  }   
/*================================================================================================
/      setOTMstart(Timestamp new_OTMstart)
/=================================================================================================*/
  public void setOTMstart(Timestamp new_OTMstart) {
    Timestamp  old_OTMstart = this.OTMstart;
    this.OTMstart = new_OTMstart;
   propertyChangeListeners.firePropertyChange("OTMstart", old_OTMstart, new_OTMstart);
  }
  public Timestamp getOTMstart() {
    return OTMstart;
  }
/*================================================================================================
/      setOTMend(Timestamp new_OTMend)
/=================================================================================================*/
  public void setOTMend(Timestamp new_OTMend) {
    Timestamp  old_OTMend = this.OTMend;
    this.OTMend = new_OTMend;
   propertyChangeListeners.firePropertyChange("OTMend", old_OTMend, new_OTMend);
  }
  public Timestamp getOTMend() {
    return OTMend;
  }  
/*================================================================================================
/     setAirmass(double new_airmass)
/=================================================================================================*/
  public void setOTMAirmass_start(double new_airmass_start) {
    double  old_airmass_start = this.airmass_start;
    airmass_start = new_airmass_start;
   propertyChangeListeners.firePropertyChange("airmass_start", Double.valueOf(old_airmass_start), Double.valueOf(new_airmass_start));
  }
  public double getOTMAirmass_start() {
    return airmass_start;
  } 
/*================================================================================================
/     setAirmass(double new_airmass)
/=================================================================================================*/
  public void setOTMAirmass_end(double new_airmass_end) {
    double  old_airmass_end = this.airmass_end;
    airmass_end = new_airmass_end;
   propertyChangeListeners.firePropertyChange("airmass_end", Double.valueOf(old_airmass_end), Double.valueOf(new_airmass_end));
  }
  public double getOTMAirmass_end() {
    return airmass_end;
  }   
/*================================================================================================
/     setSkymag(double new_skymag)
/=================================================================================================*/
  public void setSkymag(double new_skymag) {
    double  old_skymag = this.skymag;
    skymag = new_skymag;
   propertyChangeListeners.firePropertyChange("skymag", Double.valueOf(old_skymag), Double.valueOf(new_skymag));
  }
  public double getSkymag() {
    return skymag;
  }   
/*================================================================================================
/     setOTMexpt(double new_skymag)
/=================================================================================================*/
  public void setOTMexpt(double new_OTMexpt) {
    double  old_OTMexpt = this.OTMexpt;
    OTMexpt = new_OTMexpt;
   propertyChangeListeners.firePropertyChange("OTMexpt", Double.valueOf(old_OTMexpt), Double.valueOf(new_OTMexpt));
  }
  public double getOTMexpt() {
    return OTMexpt;
  }  
/*================================================================================================
/     setOTMexpt(double new_skymag)
/=================================================================================================*/
  public void setOTMslitwidth(double new_OTMslit) {
    double  old_OTMslit = this.OTMslitwidth;
    OTMslitwidth = new_OTMslit;
   propertyChangeListeners.firePropertyChange("OTMslit", Double.valueOf(old_OTMslit), Double.valueOf(new_OTMslit));
  }
  public double getOTMslitwidth() {
    return OTMslitwidth;
  }   
 /*================================================================================================
/     setOTMexpt(double new_skymag)
/=================================================================================================*/
  public void setOTMmoon(java.lang.String new_OTMmoon) {
    java.lang.String  old_OTMmoon = this.OTMmoon;
    OTMmoon = new_OTMmoon;
   propertyChangeListeners.firePropertyChange("OTMmoon", old_OTMmoon, new_OTMmoon);
  }
  public java.lang.String getOTMmoon() {
    return OTMmoon;
  }   
/*================================================================================================
/    setOTMseeing(double new_OTMseeing) 
/=================================================================================================*/
  public void setOTMseeing(double new_OTMseeing) {
    double  old_OTMseeing = this.OTMseeing;
    OTMseeing = new_OTMseeing;
   propertyChangeListeners.firePropertyChange("OTMseeing", Double.valueOf(old_OTMseeing), Double.valueOf(new_OTMseeing));
  }
  public double getOTMseeing() {
    return OTMseeing;
  }   
/*================================================================================================
/      setOTMlast(double new_OTMlast)
/=================================================================================================*/
  public void setOTMpointmode(java.lang.String new_OTMpointmode) {
    java.lang.String  old_OTMpointmode = this.pointmode;
    pointmode = new_OTMpointmode;
    propertyChangeListeners.firePropertyChange("OTMpointmode", old_OTMpointmode, new_OTMpointmode);
  }
  public java.lang.String getOTMpointmode() {
    return pointmode;
  }  
/*================================================================================================
/      setOTMslewgo(Timestamp new_OTMslewgo)
/=================================================================================================*/
  public void setOTMnotbefore(Timestamp new_OTMnotbefore) {
    Timestamp  old_OTMnotbefore = this.notbefore;
    this.notbefore = new_OTMnotbefore;
   propertyChangeListeners.firePropertyChange("OTMnotbefore", old_OTMnotbefore, new_OTMnotbefore);
  }
  public Timestamp getOTMnotbefore() {
    return notbefore;
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
public OTMmodel clone(){
    OTMmodel current = new OTMmodel();
    current.setOTMAirmass_end(this.getOTMAirmass_end());
    current.setOTMAirmass_start(this.getOTMAirmass_start());
    current.setOTMSNR(this.getOTMSNR());
    current.setOTMcass(this.getOTMcass());
    current.setOTMdPadt(this.getOTMdPadt());
    current.setOTMdead(this.getOTMdead());
    current.setOTMend(this.getOTMend());
    current.setOTMexpt(this.getOTMexpt());
    current.setOTMflag(this.getOTMflag());
    current.setOTMlast(this.getOTMlast());
    current.setOTMmoon(this.getOTMmoon());
    current.setOTMname(this.getOTMname());
    current.setOTMpa(this.getOTMpa());
    current.setOTMslew(this.getOTMslew());
    current.setOTMslewgo(this.getOTMslewgo());
    current.setOTMslitwidth(this.getOTMslitwidth());
    current.setOTMslitangle(this.getOTMslitangle());
    current.setOTMstart(this.getOTMstart());
    current.setOTMwait(this.getOTMwait());
    current.setSkymag(this.getSkymag());
    current.setOTMpointmode(this.getOTMpointmode());
    current.setOTMnotbefore(this.getOTMnotbefore());
    return current;
}  
  
}
