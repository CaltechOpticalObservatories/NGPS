package edu.caltech.palomar.util.general;  
//=== File Prolog =============================================================
//	This code was developed by John Thorstensen of Dartmouth University
//
//      It has been refactored and modified by Jennifer Milburn for
//      use at the Palomar Observatory.
//
//      Caltech Optical Observatories, Palomar Observatory 
//
//--- Contents ----------------------------------------------------------------
//     ExposureTimer
//
//--- Description -------------------------------------------------------------
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
//
// TERMS OF USE --
// Anyone is free to use this software for any purpose, and to
// modify it for their own purposes, provided that credit is given to the author
// in a prominent place.  For the present program that means that the green
// title and author banner appearing on the main window must not be removed,
// and may not be altered without premission of the author.
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
//	In no event shall Caltech be liable for any damages, including, but not
//	limited to direct, indirect, special or consequential damages, arising out
//	of, resulting from, or in any way connected with this software, whether or
//	not based upon warranty, contract, tort or otherwise, whether or not
//	injury was sustained by persons or property or otherwise, and whether or
//	not loss was sustained from or arose out of the results of, or use of,
//	their software or services provided hereunder.
//
//=== End File Prolog =========================================================
import javax.swing.Timer;
import java.awt.event.ActionListener;
import java.beans.*;
/*===============================================================================================================
/       CLASS DEFINITION:      ExposureTimer
/===============================================================================================================*/
public class ExposureTimer extends Timer {
  private long startTime;
  private long endTime;
  private transient PropertyChangeSupport propertyChangeListeners = new PropertyChangeSupport(this);

/*===============================================================================================================
/       Constructor:   ExposureTimer
/===============================================================================================================*/
  public ExposureTimer(int newDelay,java.awt.event.ActionListener defaultListener) {
    super(newDelay,defaultListener);
    startTime = 0;
    endTime   = 0;
   }
/*===============================================================================================================
/       Set and Get Start Time Methods for the Exposure Timer
/===============================================================================================================*/
  public long getStartTime() {
    return startTime;
  }
  public void setStartTime(long newStartTime) {
    long  oldStartTime = startTime;
    startTime = newStartTime;
    propertyChangeListeners.firePropertyChange("startTime", Long.valueOf(oldStartTime),Long.valueOf(newStartTime));
  }
/*===============================================================================================================
/       Set and Get Start Time Methods for the Exposure Timer
/===============================================================================================================*/
  public long getEndTime() {
    return endTime;
  }
  public void setEndTime(long newEndTime) {
    long  oldEndTime = endTime;
    endTime = newEndTime;
    propertyChangeListeners.firePropertyChange("endTime", Long.valueOf(oldEndTime), Long.valueOf(newEndTime));
  }
/*===============================================================================================================
/       Property Change Listener Supporting Classes
/===============================================================================================================*/
  public synchronized void removePropertyChangeListener(PropertyChangeListener l) {
    propertyChangeListeners.removePropertyChangeListener(l);
  }

  public synchronized void addPropertyChangeListener(PropertyChangeListener l) {
    propertyChangeListeners.addPropertyChangeListener(l);
  }
}
