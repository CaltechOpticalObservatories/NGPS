
package edu.caltech.palomar.util.general;   
//=== File Prolog =============================================================
//	This code was developed by California Institute of Technology
//      Caltech Optical Observatories, Palomar Observatory
//
//--- Contents ----------------------------------------------------------------
//	 P200ServerSocket- This is the TCP/IP server socket for the P200 Simulator
//
//--- Description -------------------------------------------------------------
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
//
//      July 16, 2010 Jennifer Milburn
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
//	In no event shall Caltech be liable for any damages, including, but not
//	limited to direct, indirect, special or consequential damages, arising out
//	of, resulting from, or in any way connected with this software, whether or
//	not based upon warranty, contract, tort or otherwise, whether or not
//	injury was sustained by persons or property or otherwise, and whether or
//	not loss was sustained from or arose out of the results of, or use of,
//	their software or services provided hereunder.
//
//=== End File Prolog =========================================================
import java.util.*;
import java.beans.*;
/*=============================================================================================
/         TimeUtilities Class Declaration
/=============================================================================================*/
public class TimeUtilities  extends java.lang.Object{
  private transient PropertyChangeSupport propertyChangeListeners = new PropertyChangeSupport(this);
  private java.lang.String UTCSTART       = new java.lang.String();
  private java.lang.String UTCEND         = new java.lang.String();
  private java.lang.String DATE_OBS       = new java.lang.String();
  private java.lang.String DATE           = new java.lang.String();
  private java.lang.String currentTimeTag = new java.lang.String();
  public  static java.lang.String UTCLabel  = "UTC = ";
  public  static java.lang.String LSTLabel  = "LST = ";
  public static int        UT_ZONE               = 0;
  public static int        PST_ZONE              = 1;
  public static int        UTCSTART_STAMP        = 1;
  public static int        UTCEND_STAMP          = 2;
  public static int        OBSID_YEAR_STAMP      = 1;
  public static int        DATE_OBS_YEAR_STAMP   = 2;
  private TimeMonitorThread myTimeMonitorThread;
/*=============================================================================================
/         TimeUtilities Constructor
/=============================================================================================*/
  public TimeUtilities(){

  }
//  UTCSTART 	UTC of exposure start any [hh:mm:ss.s] [str] [] ['09:30:01.00'] absolute :
//  UTCEND 	UTC of exposure end any [hh:mm:ss.s] [str] [] ['09:30:01.00'] absolute :
/*================================================================================================
/   setUTCSTART(java.lang.String newUTCSTART)
/=================================================================================================*/
    public java.lang.String getUTCSTART() {
      return UTCSTART;
    }
    public void setUTCSTART(java.lang.String newUTCSTART) {
      java.lang.String  oldUTCSTART = this.UTCSTART;
      this.UTCSTART = newUTCSTART;
      propertyChangeListeners.firePropertyChange("UTCSTART", oldUTCSTART, newUTCSTART);
    }
/*================================================================================================
/   setUTCEND(java.lang.String newUTCEND)
/=================================================================================================*/
    public java.lang.String getUTCEND() {
          return UTCEND;
    }
    public void setUTCEND(java.lang.String newUTCEND) {
       java.lang.String  oldUTCEND = this.UTCEND;
       this.UTCEND = newUTCEND;
       propertyChangeListeners.firePropertyChange("UTCEND", oldUTCEND, newUTCEND);
    }
/*================================================================================================
/   setDATE_OBS(java.lang.String newDATE_OBS)
/=================================================================================================*/
   public java.lang.String getDATE_OBS() {
      return DATE_OBS;
   }
   public void setDATE_OBS(java.lang.String newDATE_OBS) {
      java.lang.String  oldDATE_OBS = this.DATE_OBS;
      this.DATE_OBS = newDATE_OBS;
      propertyChangeListeners.firePropertyChange("DATE_OBS", oldDATE_OBS, newDATE_OBS);
   }
/*================================================================================================
/   setDATE(java.lang.String newDATE)
/=================================================================================================*/
      public java.lang.String getDATE() {
         return DATE;
      }
      public void setDATE(java.lang.String newDATE) {
         java.lang.String  oldDATE = this.DATE;
         this.DATE = newDATE;
         propertyChangeListeners.firePropertyChange("DATE", oldDATE, newDATE);
      }
/*=============================================================================================
/       setCurrentTimeTag(java.lang.String newCurrentTimeTag) Methods
/=============================================================================================*/
    public void setCurrentTimeTag(java.lang.String newCurrentTimeTag){
      java.lang.String oldCurrentTimeTag  = currentTimeTag;
      currentTimeTag  = newCurrentTimeTag;
      propertyChangeListeners.firePropertyChange("currentTimeTag",oldCurrentTimeTag,newCurrentTimeTag);
    }
    public java.lang.String getCurrentTimeTag(){
      return currentTimeTag;
    }
/*=================================================================================
/       constructPalomarUTC()
/=================================================================================*/
 public java.lang.String constructPalomarUTC(){
     java.lang.String timeString = new java.lang.String();
     constructUTCStamp(UTCSTART_STAMP);
     java.lang.String dayOfYear  = constructDateOfYearTag();
     java.lang.String timeStamp  = getUTCSTART();
     timeString = UTCLabel + dayOfYear + " " + timeStamp;
  return timeString;
  }
/*=================================================================================
/       constructPalomarUTC()
/=================================================================================*/
 public java.lang.String constructPalomarLST(){
     java.lang.String timeString = new java.lang.String();
     constructLSTStamp(UTCSTART_STAMP);
     java.lang.String dayOfYear  = constructDateOfYearTag();
     java.lang.String timeStamp  = getUTCSTART();
     timeString = LSTLabel + timeStamp;
//The mean or apparent sidereal time locally is found by obtaining the local longitude in degrees,
//converting it to hours by dividing by 15, and then adding it to or subtracting it from the
//Greenwich time depending on whether the local position is east (add) or west (subtract) of Greenwich.

   return timeString;
  }
/*=================================================================================
/       constructDateOfYearTag()
/=================================================================================*/
  public java.lang.String constructDateOfYearTag(){
        java.lang.String   dateTag = new java.lang.String();
        // create a Pacific Standard Time time zone
        String[] ids = TimeZone.getAvailableIDs(0);
        // if no ids were returned, something is wrong. get out.
        java.util.SimpleTimeZone pdt = new java.util.SimpleTimeZone(0, ids[0]);
//      java.util.SimpleTimeZone pdt = new java.util.SimpleTimeZone(-8 * 60 * 60 * 1000, ids[0]);
        // set up rules for daylight savings time
         pdt.setStartRule(Calendar.JANUARY, 1, Calendar.SUNDAY, 2 * 60 * 60 * 1000);
         pdt.setEndRule(Calendar.DECEMBER, -1, Calendar.SUNDAY, 2 * 60 * 60 * 1000);
        //  Create the Current Gregorian Calendar based on the system time
          long currentTime = System.currentTimeMillis();
          long               myTimeLong          = System.currentTimeMillis();
          java.util.Date     myDate              = new java.util.Date(myTimeLong);
          java.util.Calendar myGregorianCalendar = new GregorianCalendar(pdt);
          myGregorianCalendar.setTime(myDate);
         //   Get the integer representation of the date and time
            int day         = myGregorianCalendar.get(Calendar.DAY_OF_YEAR);
            java.lang.Integer  dayNumber       = Integer.valueOf(day);
            java.lang.String   dayTag          = new java.lang.String();
            if(dayNumber < 10){
                dayTag = "00" + dayNumber.toString();
            }
            if(dayNumber < 100){
                 dayTag = "0" + dayNumber.toString();
            }
            if(dayNumber >= 100){
                 dayTag = dayNumber.toString();
            }
    return dayTag;
  }
/*=================================================================================
/       constructDateTag()
/=================================================================================*/
  public java.lang.String constructDateTag(int dateMode){
        java.lang.String   dateTag = new java.lang.String();
        // create a Pacific Standard Time time zone
        String[] ids = TimeZone.getAvailableIDs(0);
        // if no ids were returned, something is wrong. get out.
        java.util.SimpleTimeZone pdt = new java.util.SimpleTimeZone(0, ids[0]);
//      java.util.SimpleTimeZone pdt = new java.util.SimpleTimeZone(-8 * 60 * 60 * 1000, ids[0]);
        // set up rules for daylight savings time
         pdt.setStartRule(Calendar.JANUARY, 1, Calendar.SUNDAY, 2 * 60 * 60 * 1000);
         pdt.setEndRule(Calendar.DECEMBER, -1, Calendar.SUNDAY, 2 * 60 * 60 * 1000);
        //  Create the Current Gregorian Calendar based on the system time
          long currentTime = System.currentTimeMillis();
          long               myTimeLong          = System.currentTimeMillis();
          java.util.Date     myDate              = new java.util.Date(myTimeLong);
          java.util.Calendar myGregorianCalendar = new GregorianCalendar(pdt);
          myGregorianCalendar.setTime(myDate);
         //   Get the integer representation of the date and time
            int day         = myGregorianCalendar.get(Calendar.DAY_OF_MONTH);
            int month       = myGregorianCalendar.get(Calendar.MONTH) + 1;
            int year        = myGregorianCalendar.get(Calendar.YEAR);

            java.lang.Integer  monthNumber       = Integer.valueOf(month);
            java.lang.String   monthNumberString = new java.lang.String();
            if(month < 10){
              monthNumberString = "0" + monthNumber.toString();
            }
            if(month >= 10){
              monthNumberString = monthNumber.toString();
            }
            java.lang.Integer  dayNumber       = Integer.valueOf(day);
            java.lang.String   dayNumberString = new java.lang.String();
            if(day < 10){
              dayNumberString = "0" + dayNumber.toString();
            }
            if(day >= 10){
              dayNumberString = dayNumber.toString();
            }
            java.lang.Integer  yearNumber            = Integer.valueOf(year);
            java.lang.String   yearNumberString      = new java.lang.String();
            java.lang.String   yearString            = new java.lang.String();
                               yearNumberString      = yearNumber.toString();

            if(dateMode == OBSID_YEAR_STAMP){
                                 int stringLength =  yearNumberString.length();
              char[]             myYearNumberArray = yearNumberString.toCharArray();
              char[]             myFinalYearNumberArray = new char[2];
                                 myFinalYearNumberArray[0] = myYearNumberArray[2];
                                 myFinalYearNumberArray[1] = myYearNumberArray[3];
              java.lang.String   finalYearNumberString = new java.lang.String(myFinalYearNumberArray);
                                 yearString            = finalYearNumberString;
              dateTag = yearString +  monthNumberString +  dayNumberString;
            }
            if(dateMode == DATE_OBS_YEAR_STAMP){
                  yearString  = yearNumberString;
                  dateTag = yearString + "-" +  monthNumberString + "-" + dayNumberString;
            }
    return dateTag;
  }
/*=================================================================================
/         createTimeTag()
/=================================================================================*/
  public java.lang.String createTimeTag(){
      // create a Pacific Standard Time time zone
      String[] ids = TimeZone.getAvailableIDs(0);
      // if no ids were returned, something is wrong. get out.
      java.util.SimpleTimeZone pdt = new java.util.SimpleTimeZone(0, ids[0]);
//      java.util.SimpleTimeZone pdt = new java.util.SimpleTimeZone(-8 * 60 * 60 * 1000, ids[0]);
      // set up rules for daylight savings time
       pdt.setStartRule(Calendar.APRIL, 1, Calendar.SUNDAY, 2 * 60 * 60 * 1000);
       pdt.setEndRule(Calendar.NOVEMBER, -1, Calendar.SUNDAY, 2 * 60 * 60 * 1000);
      //  Create the Current Gregorian Calendar based on the system time
        long currentTime = System.currentTimeMillis();
        long               myTimeLong          = System.currentTimeMillis();
        java.util.Date     myDate              = new java.util.Date(myTimeLong);
        java.util.Calendar myGregorianCalendar = new GregorianCalendar(pdt);
        myGregorianCalendar.setTime(myDate);
       //   Get the integer representation of the date and time
          int day         = myGregorianCalendar.get(Calendar.DAY_OF_MONTH);
          int month       = myGregorianCalendar.get(Calendar.MONTH);
          int year        = myGregorianCalendar.get(Calendar.YEAR);
          int hour        = myGregorianCalendar.get(Calendar.HOUR_OF_DAY);
          int minute      = myGregorianCalendar.get(Calendar.MINUTE);
          int second      = myGregorianCalendar.get(Calendar.SECOND);
          int millisecond = myGregorianCalendar.get(Calendar.MILLISECOND);
          java.lang.String myMonth = new java.lang.String();
                if(month == Calendar.JANUARY){
                   myMonth = "Jan";
                }
                if(month == Calendar.FEBRUARY){
                   myMonth = "Feb";
                }
                if(month == Calendar.MARCH){
                   myMonth = "Mar";
                }
                if(month == Calendar.APRIL){
                   myMonth = "April";
                }
                if(month == Calendar.MAY){
                   myMonth = "May";
                }
                if(month == Calendar.JUNE){
                   myMonth = "June";
                }
                if(month == Calendar.JULY){
                   myMonth = "July";
                }
                if(month == Calendar.AUGUST){
                   myMonth = "Aug";
                }
                if(month == Calendar.SEPTEMBER){
                   myMonth = "Sept";
                }
                if(month == Calendar.OCTOBER){
                   myMonth = "Oct";
                }
                if(month == Calendar.NOVEMBER){
                   myMonth = "Nov";
                }
                if(month == Calendar.DECEMBER){
                   myMonth = "Dec";
                }
      java.lang.String   timeTagComplete = myMonth + "-" + day + "-" + year + "-" + hour + ":" + minute + ":" + second;
      setCurrentTimeTag(timeTagComplete);
      java.lang.String   timeTag         = myMonth + "-" + day + "-" + year + "-";
  return timeTag;
}
/*=================================================================================
/         createTimeTag_milliseconds()
/=================================================================================*/
  public java.lang.String createTimeTag_milliseconds(){
      // create a Pacific Standard Time time zone
      String[] ids = TimeZone.getAvailableIDs(0);
      // if no ids were returned, something is wrong. get out.
      java.util.SimpleTimeZone pdt = new java.util.SimpleTimeZone(0, "UTC");
//      java.util.SimpleTimeZone pdt = new java.util.SimpleTimeZone(-8 * 60 * 60 * 1000, ids[0]);
      // set up rules for daylight savings time
       pdt.setStartRule(Calendar.APRIL, 1, Calendar.SUNDAY, 2 * 60 * 60 * 1000);
       pdt.setEndRule(Calendar.NOVEMBER, -1, Calendar.SUNDAY, 2 * 60 * 60 * 1000);
      //  Create the Current Gregorian Calendar based on the system time
        long currentTime = System.currentTimeMillis();
        long               myTimeLong          = System.currentTimeMillis();
        java.util.Date     myDate              = new java.util.Date(myTimeLong);
        java.util.Calendar myGregorianCalendar = new GregorianCalendar(pdt);
        myGregorianCalendar.setTime(myDate);
       //   Get the integer representation of the date and time
          int day         = myGregorianCalendar.get(Calendar.DAY_OF_MONTH);
          int month       = myGregorianCalendar.get(Calendar.MONTH);
          int year        = myGregorianCalendar.get(Calendar.YEAR);
          int hour        = myGregorianCalendar.get(Calendar.HOUR_OF_DAY);
          int minute      = myGregorianCalendar.get(Calendar.MINUTE);
          int second      = myGregorianCalendar.get(Calendar.SECOND);
          int millisecond = myGregorianCalendar.get(Calendar.MILLISECOND);
          java.lang.String myMonth = new java.lang.String();
                if(month == Calendar.JANUARY){
                   myMonth = "Jan";
                }
                if(month == Calendar.FEBRUARY){
                   myMonth = "Feb";
                }
                if(month == Calendar.MARCH){
                   myMonth = "Mar";
                }
                if(month == Calendar.APRIL){
                   myMonth = "April";
                }
                if(month == Calendar.MAY){
                   myMonth = "May";
                }
                if(month == Calendar.JUNE){
                   myMonth = "June";
                }
                if(month == Calendar.JULY){
                   myMonth = "July";
                }
                if(month == Calendar.AUGUST){
                   myMonth = "Aug";
                }
                if(month == Calendar.SEPTEMBER){
                   myMonth = "Sept";
                }
                if(month == Calendar.OCTOBER){
                   myMonth = "Oct";
                }
                if(month == Calendar.NOVEMBER){
                   myMonth = "Nov";
                }
                if(month == Calendar.DECEMBER){
                   myMonth = "Dec";
                }
      java.lang.String   timeTagComplete = myMonth + "-" + day + "-" + year + "-" + hour + ":" + minute + ":" + second+"."+millisecond;
      setCurrentTimeTag(timeTagComplete);
  return timeTagComplete;
}
/*================================================================================================
/  void constructUTCSTART()
/=================================================================================================*/
    public void constructUTCStamp(int utcMode) {
        constructTimeStamp(UT_ZONE,utcMode);
    }
/*================================================================================================
/  void constructLSTSTART()
/=================================================================================================*/
    public void constructLSTStamp(int utcMode) {
        constructTimeStamp(PST_ZONE,utcMode);
    }
/*================================================================================================
/  void constructUTCSTART()
/=================================================================================================*/
    public java.lang.String constructTimeStamp(int timeZone,int utcMode) {
      // create a Pacific Standard Time time zone
      String[] ids = TimeZone.getAvailableIDs(0);
      // if no ids were returned, something is wrong. get out.
     java.util.SimpleTimeZone pdt = new java.util.SimpleTimeZone(0, TimeZone.getTimeZone("GMT-8").getID());;
     if(timeZone == UT_ZONE){
          pdt = new java.util.SimpleTimeZone(0, TimeZone.getTimeZone("GMT-8").getID());
      }
     if(timeZone == PST_ZONE){
//The mean or apparent sidereal time locally is found by obtaining the local longitude in degrees,
//converting it to hours by dividing by 15, and then adding it to or subtracting it from the
//Greenwich time depending on whether the local position is east (add) or west (subtract) of Greenwich.
// This way just gives us the Pacific Standard Time or clock time NOT the LST
          pdt = new java.util.SimpleTimeZone(-8 * 60 * 60 * 1000, "America/Los_Angeles");
          
     }     
//      java.util.SimpleTimeZone pdt = new java.util.SimpleTimeZone(-8 * 60 * 60 * 1000, ids[0]);
      // set up rules for daylight savings time
       pdt.setStartRule(Calendar.JANUARY, 1, Calendar.SUNDAY, 2 * 60 * 60 * 1000);
       pdt.setEndRule(Calendar.DECEMBER, -1, Calendar.SUNDAY, 2 * 60 * 60 * 1000);
      //  Create the Current Gregorian Calendar based on the system time
        long currentTime = System.currentTimeMillis();
        long               myTimeLong          = System.currentTimeMillis();
        java.util.Date     myDate              = new java.util.Date(myTimeLong);
        java.util.Calendar myGregorianCalendar = new GregorianCalendar(pdt);
        myGregorianCalendar.setTime(myDate);
       //   Get the integer representation of the date and time
        int hour        = myGregorianCalendar.get(Calendar.HOUR_OF_DAY);
        int minute      = myGregorianCalendar.get(Calendar.MINUTE);
        int second      = myGregorianCalendar.get(Calendar.SECOND);
        int millisecond = myGregorianCalendar.get(Calendar.MILLISECOND);

          java.lang.Integer  hourNumber       = Integer.valueOf(hour);
          java.lang.String   hourNumberString = new java.lang.String();
          if(hour < 10){
            hourNumberString = "0" + hourNumber.toString();
          }
          if(hour >= 10){
            hourNumberString = hourNumber.toString();
          }
          java.lang.Integer  minuteNumber       = Integer.valueOf(minute);
          java.lang.String   minuteNumberString = new java.lang.String();
          if(minute < 10){
            minuteNumberString = "0" + minuteNumber.toString();
          }
          if(minute >= 10){
            minuteNumberString = minuteNumber.toString();
          }
          java.lang.Integer  secondNumber       = Integer.valueOf(second);
          java.lang.String   secondNumberString = new java.lang.String();
          if(second < 10){
            secondNumberString = "0" + secondNumber.toString();
          }
          if(second >= 10){
            secondNumberString = secondNumber.toString();
          }
          java.lang.Integer  millisecondNumber       = Integer.valueOf(millisecond);
          java.lang.String   millisecondNumberString = new java.lang.String();
            millisecondNumberString = millisecondNumber.toString();
          int stringLength = millisecondNumberString.length();
          java.lang.String milliseconds = new java.lang.String();
          if(stringLength == 3){
             milliseconds = millisecondNumberString.substring(0, 2) + "." + millisecondNumberString.substring(2,3);
          }
          if(stringLength == 2){
             milliseconds = millisecondNumberString.substring(0, 2) + ".0";
          }
          if(stringLength == 1){
             milliseconds = "0" + millisecondNumberString + ".0";
          }
//  UTCSTART 	UTC of exposure start any [hh:mm:ss.s] [str] [] ['09:30:01.00'] absolute :
          java.lang.String   utcString      = new java.lang.String();
          utcString = hourNumberString + ":" + minuteNumberString + ":" + secondNumberString + "." + milliseconds;
          if(utcMode == UTCSTART_STAMP){
            setUTCSTART(utcString);
          }
          if(utcMode == UTCEND_STAMP){
            setUTCEND(utcString);
          }
      return utcString;
    }
/*=================================================================================
/         monitorTime()
/=================================================================================*/
  public void monitorTime(){
      myTimeMonitorThread = new TimeMonitorThread();
      myTimeMonitorThread.start();
  }
    public void stop(){
      myTimeMonitorThread.stop();
  }
/*=================================================================================
/          Property Change Listener Methods
/=================================================================================*/
  public synchronized void removePropertyChangeListener(PropertyChangeListener l) {
    propertyChangeListeners.removePropertyChangeListener(l);
  }
  public synchronized void addPropertyChangeListener(PropertyChangeListener l) {
     propertyChangeListeners.addPropertyChangeListener(l);
  }
/*=============================================================================================
/       INNER CLASS Constantly monitors the time and date
/=============================================================================================*/
    public class TimeMonitorThread  implements Runnable{
        transient protected PropertyChangeSupport propertyChangeListeners = new PropertyChangeSupport(this);
        private Thread myThread;
        private double           delay    = 0.0;
        private int              POLLING_DELAY = 1000;
        private boolean          monitor;
        private java.lang.String date = new java.lang.String();
        private java.lang.String time = new java.lang.String();
/*=============================================================================================
/          ImageMonitorThread()
/=============================================================================================*/
    private TimeMonitorThread(){
    }
/*================================================================================================
/      setMonitor(boolean new_monitor)
/=================================================================================================*/
public void setMonitor(boolean new_monitor){
    monitor = new_monitor;
}
/*================================================================================================
/      stop()
/=================================================================================================*/
public void stop(){
    monitor = false;
}
/*================================================================================================
/      waitForResponse(int newDelay)
/=================================================================================================*/
private void waitForResponseMilliseconds(int newDelay){
     try{
        Thread.currentThread().sleep(newDelay);} // sleep for awhile to let the messages pile up
      catch (Exception e){
    }
}
/*=============================================================================================
/           run()
/===========================================================================crsw==================*/
    public void run(){
      monitor = true;
      while(monitor){
        date = constructDateTag(DATE_OBS_YEAR_STAMP);
//          date = constructPalomarUTC();
          time = constructTimeStamp(UT_ZONE,UTCSTART_STAMP);
        setDATE(date);
        setCurrentTimeTag(time);
        waitForResponseMilliseconds(POLLING_DELAY);         
      }  
     }
/*=============================================================================================
/         start()
/=============================================================================================*/
    public void start(){
     myThread = new Thread(this);
     myThread.start();
     }
    }// TimeMonitorThread
}