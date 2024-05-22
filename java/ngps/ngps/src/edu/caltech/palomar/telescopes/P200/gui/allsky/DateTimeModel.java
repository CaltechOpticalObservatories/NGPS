/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package edu.caltech.palomar.telescopes.P200.gui.allsky;
//=== File Prolog =============================================================
//	This code was developed by UCLA, Department of Physics and Astronomy,
//	for the Stratospheric Observatory for Infrared Astronomy (SOFIA) project.
//
//--- Contents ----------------------------------------------------------------
//	AllSkyImageProcessor  -
//
//--- Description -------------------------------------------------------------
//	Retrieves images from the All Sky Camera from the Website and display them
//
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
//
//      April 22, 2012      Jennifer W Milburn
//        Original Implementation
//
//--- DISCLAIMER---------------------------------------------------------------
//
//	This software is provided "as is" without any warranty of any kind, either
//	express, implied, or statutory, including, but not limited to, any
//	warranty that the software will conform to specification, any implied
//	warranties of merchantability, fitness for a particular purpose, and
//	freedom from infringement, and any warranty that the documentation will
//	conform to the program, or any warranty that the software will be error
//	free.
//
//	In no event shall UCLA be liable for any damages, including, but not
//	limited to direct, indirect, special or consequential damages, arising out
//	of, resulting from, or in any way connected with this software, whether or
//	not based upon warranty, contract, tort or otherwise, whether or not
//	injury was sustained by persons or property or otherwise, and whether or
//	not loss was sustained from or arose out of the results of, or use of,
//	their software or services provided hereunder.
/*=============================================================================================
/     Import packages section
/=============================================================================================*/
import java.beans.*;
public class DateTimeModel {
  transient private PropertyChangeSupport propertyChangeListeners = new PropertyChangeSupport(this);
 private int month;
 private int day;
 private int year;
 private int hour;
 private int minute;
 private int second;
/*================================================================================================
/       DateTimeModel()
/=================================================================================================*/
 public DateTimeModel(){    
 }
/*================================================================================================
/       set_month(int new_month)
/=================================================================================================*/
  public void set_month(int new_month) {
    int  old_month = this.month;
    this.month = new_month;
    propertyChangeListeners.firePropertyChange("month", Integer.valueOf(old_month), Integer.valueOf(new_month));
  }
  public int get_month() {
    return month;
  }
/*================================================================================================
/       set_day(int new_day)
/=================================================================================================*/
  public void set_day(int new_day) {
    int  old_day = this.day;
    this.day = new_day;
    propertyChangeListeners.firePropertyChange("day", Integer.valueOf(old_day),Integer.valueOf(new_day));
  }
  public int get_day() {
    return day;
  }
/*================================================================================================
/       set_year(int new_year)
/=================================================================================================*/
  public void set_year(int new_year) {
    int  old_year = this.year;
    this.year = new_year;
    propertyChangeListeners.firePropertyChange("year", Integer.valueOf(old_year),Integer.valueOf(new_year));
  }
  public int get_year() {
    return year;
  }
/*================================================================================================
/       set_hour(int new_hour)
/=================================================================================================*/
  public void set_hour(int new_hour) {
    int  old_hour = this.hour;
    this.hour = new_hour;
    propertyChangeListeners.firePropertyChange("hour", Integer.valueOf(old_hour), Integer.valueOf(new_hour));
  }
  public int get_hour() {
    return hour;
  }
  /*================================================================================================
/       set_minute(int new_minute)
/=================================================================================================*/
  public void set_minute(int new_minute) {
    int  old_minute = this.minute;
    this.minute = new_minute;
    propertyChangeListeners.firePropertyChange("minute", Integer.valueOf(old_minute), Integer.valueOf(new_minute));
  }
  public int get_minute() {
    return minute;
  }
/*================================================================================================
/       set_second(int new_second)
/=================================================================================================*/
  public void set_second(int new_second) {
    int  old_second = this.second;
    this.second = new_second;
    propertyChangeListeners.firePropertyChange("second", Integer.valueOf(old_second), Integer.valueOf(new_second));
  }
  public int get_second() {
    return second;
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


