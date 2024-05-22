/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * DateTimeChooserFrame.java
 *
 * Created on Apr 24, 2012, 3:24:08 AM
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
import com.toedter.calendar.JDateChooser;
import com.toedter.calendar.JCalendar;
import javax.swing.SpinnerNumberModel;
import javax.swing.ButtonGroup;
import java.util.Calendar;
import java.util.TimeZone;
import java.util.GregorianCalendar;
import java.beans.PropertyChangeEvent;
import edu.caltech.palomar.telescopes.P200.gui.allsky.DateTimeModel;
import edu.dartmouth.jskycalc.JSkyCalcModel;
import javax.swing.JFrame;
/*=============================================================================================
/     Import packages section
/=============================================================================================*/

public class DateTimeChooserFrame extends javax.swing.JFrame {
    public JCalendar          myJCalendar = new JCalendar();
    public DateTimeModel      myDateTimeModel = new DateTimeModel();
    public SpinnerNumberModel hoursSpinnerModel;
    public SpinnerNumberModel minutesSpinnerModel;
    public SpinnerNumberModel secondsSpinnerModel;
    private int               hours;
    private int               minutes;
    private int               seconds;
    private int               month;
    private int               day;
    private int               year;
    private int               hour;
    private int               minute;
    private int               second;
    private int               millisecond;
    private ButtonGroup       sourceButtonGroup = new ButtonGroup();
    private int               time_source;
    private UpdateTimeThread  myUpdateTimeThread;
    public  JSkyCalcModel     myJSkyCalcModelTelescope;
    public  JSkyCalcModel     myJSkyCalcModelObjects;
    public static int         NOW           = 100;
    public static int         SELECTED_TIME = 200;
/*=============================================================================================
/     Import packages section
/=============================================================================================*/
    /** Creates new form DateTimeChooserFrame */
    public DateTimeChooserFrame() {
        initComponents();
        initializeSpinners();
        time_source = NOW;
        CurrentTimeRadioButton.setSelected(true);
        startTimeMonitor();
        initializeCalendar();
//        mainPanel.add(myJDateChooser, new org.netbeans.lib.awtextra.AbsoluteConstraints(50, 80, 200, 200));
        DatePanel.add(myJCalendar, java.awt.BorderLayout.CENTER);
        this.setDefaultCloseOperation(JFrame.HIDE_ON_CLOSE);
    }
/*=============================================================================================
/     setJSkyCalcModel(JSkyCalcModel newJSkyCalcModel)
/=============================================================================================*/
public void setJSkyCalcModelTelescope(JSkyCalcModel newJSkyCalcModel){
    myJSkyCalcModelTelescope = newJSkyCalcModel;
}
/*=============================================================================================
/     setJSkyCalcModel(JSkyCalcModel newJSkyCalcModel)
/=============================================================================================*/
public void setJSkyCalcModelObjects(JSkyCalcModel newJSkyCalcModel){
    myJSkyCalcModelObjects = newJSkyCalcModel;
    updateDateTime();
}
/*=============================================================================================
/     constructJSkyCalcDateTime()
/=============================================================================================*/
public java.lang.String[] constructJSkyCalcDateTime(){
    java.lang.String[] results     = new java.lang.String[2];
    java.lang.String hoursString   = new java.lang.String();
    java.lang.String minutesString = new java.lang.String();
    java.lang.String secondsString = new java.lang.String();
    results[0] = year+" "+constructMonthString(month)+" "+day;
    if(hours < 10){
        hoursString = "0"+hours;
    }
    if(hours >= 10){
        hoursString = Integer.toString(hours);
    }
    if(minutes < 10){
        minutesString = "0"+minutes;
    }
    if(minutes >= 10){
        minutesString = Integer.toString(minutes);
    }
    if(seconds < 10){
        secondsString = "0"+seconds;
    }
    if(seconds >= 10){
        secondsString = Integer.toString(seconds);
    }
    results[1] = hoursString + " "+ minutesString + " "+ secondsString;
  return results;
}
/*=============================================================================================
/     constructMonthString(int new_month)
/=============================================================================================*/
public java.lang.String constructMonthString(int new_month){
  java.lang.String myMonth = new java.lang.String();
        if(new_month == Calendar.JANUARY){
           myMonth = "Jan";
        }
        if(new_month == Calendar.FEBRUARY){
           myMonth = "Feb";
        }
        if(new_month == Calendar.MARCH){
           myMonth = "Mar";
        }
        if(new_month == Calendar.APRIL){
           myMonth = "April";
        }
        if(new_month == Calendar.MAY){
           myMonth = "May";
        }
        if(new_month == Calendar.JUNE){
           myMonth = "June";
        }
        if(new_month == Calendar.JULY){
           myMonth = "July";
        }
        if(new_month == Calendar.AUGUST){
           myMonth = "Aug";
        }
        if(new_month == Calendar.SEPTEMBER){
           myMonth = "Sept";
        }
        if(new_month == Calendar.OCTOBER){
           myMonth = "Oct";
        }
        if(new_month == Calendar.NOVEMBER){
           myMonth = "Nov";
        }
        if(new_month == Calendar.DECEMBER){
           myMonth = "Dec";
        }
  return myMonth;
}
/*================================================================================================
/     startTimeMonitor()
/=================================================================================================*/
 public void startTimeMonitor(){
     myUpdateTimeThread = new  UpdateTimeThread();
     myUpdateTimeThread.start();
 }
 public void stopTimeMonitor(){
     myUpdateTimeThread.setRunning(false);
 }
/*================================================================================================
/     initializeCalendar()
/=================================================================================================*/
 public void initializeCalendar(){
     myJCalendar.getMonthChooser().addPropertyChangeListener(new java.beans.PropertyChangeListener() {
        public void propertyChange(java.beans.PropertyChangeEvent e) {
            myMonth_propertyChange(e);
          }
        });
     myJCalendar.getDayChooser().addPropertyChangeListener(new java.beans.PropertyChangeListener() {
        public void propertyChange(java.beans.PropertyChangeEvent e) {
            myDay_propertyChange(e);
          }
     }); 
     myJCalendar.getYearChooser().addPropertyChangeListener(new java.beans.PropertyChangeListener() {
        public void propertyChange(java.beans.PropertyChangeEvent e) {
            myYear_propertyChange(e);
          }
     }); 
     myDateTimeModel.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
        public void propertyChange(java.beans.PropertyChangeEvent e) {
            myDateTimeModel_propertyChange(e);
          }
     }); 
 }
 /*=============================================================================================
/     myMonth_propertyChange(PropertyChangeEvent e)
/=============================================================================================*/
   private void myDateTimeModel_propertyChange(PropertyChangeEvent e)  {
     java.lang.String propertyName = e.getPropertyName();
     if(propertyName == "month"){
        month = ((java.lang.Integer)e.getNewValue()).intValue();
        System.out.println("Month = "+ month);
        updateSelectedTimeDate();
     }
     if(propertyName == "day"){
        day = ((java.lang.Integer)e.getNewValue()).intValue();
        updateSelectedTimeDate();
     }
     if(propertyName == "year"){
        year = ((java.lang.Integer)e.getNewValue()).intValue();
        updateSelectedTimeDate();
     }
     if(propertyName == "hour"){
        month = ((java.lang.Integer)e.getNewValue()).intValue();
        HoursSpinner.setValue(Integer.valueOf(hours));
     }
     if(propertyName == "minute"){
        month = ((java.lang.Integer)e.getNewValue()).intValue();
        MinutesSpinner.setValue(Integer.valueOf(minutes));
    }
     if(propertyName == "second"){
        month = ((java.lang.Integer)e.getNewValue()).intValue();
        SecondsSpinner.setValue(Integer.valueOf(seconds));
     }    
  }
/*=============================================================================================
/     myMonth_propertyChange(PropertyChangeEvent e)
/=============================================================================================*/
   private void myMonth_propertyChange(PropertyChangeEvent e)  {
     java.lang.String propertyName = e.getPropertyName();
     if(propertyName == "month"){
        month = ((java.lang.Integer)e.getNewValue()).intValue();
        myDateTimeModel.set_month(month+1);
        int new_day  = myJCalendar.getDayChooser().getDay();
        int new_year = myJCalendar.getYearChooser().getYear();
        myDateTimeModel.set_day(new_day);
        myDateTimeModel.set_year(new_year);
//        System.out.println("Month = "+month);
     }
  }
/*=============================================================================================
/     myMonth_propertyChange(PropertyChangeEvent e)
/=============================================================================================*/
   private void myDay_propertyChange(PropertyChangeEvent e)  {
     java.lang.String propertyName = e.getPropertyName();
     if(propertyName == "day"){
        day = ((java.lang.Integer)e.getNewValue()).intValue();
        myDateTimeModel.set_day(day);
        int new_month  = myJCalendar.getMonthChooser().getMonth();
        int new_year   = myJCalendar.getYearChooser().getYear();
        myDateTimeModel.set_month(new_month+1);
        myDateTimeModel.set_year(new_year);
//          System.out.println("Day = "+day);
     }
  }
/*=============================================================================================
/     myMonth_propertyChange(PropertyChangeEvent e)
/=============================================================================================*/
   private void myYear_propertyChange(PropertyChangeEvent e)  {
     java.lang.String propertyName = e.getPropertyName();
     if(propertyName == "year"){
        year = ((java.lang.Integer)e.getNewValue()).intValue();
        myDateTimeModel.set_year(year);
        int new_day    = myJCalendar.getDayChooser().getDay();
        int new_month  = myJCalendar.getMonthChooser().getMonth();
        myDateTimeModel.set_month(new_month+1);
        myDateTimeModel.set_day(new_day);
//        System.out.println("Year = "+year);
     }
  }
/*================================================================================================
/     initializeSpinners()
/=================================================================================================*/
 private void initializeSpinners(){
    hoursSpinnerModel      =  new SpinnerNumberModel(0,0,24,1);
    minutesSpinnerModel    =  new SpinnerNumberModel(0,0,60,1);
    secondsSpinnerModel    =  new SpinnerNumberModel(0,0,60,1);
    HoursSpinner.setModel(     hoursSpinnerModel);
    MinutesSpinner.setModel(   minutesSpinnerModel);
    SecondsSpinner.setModel(   secondsSpinnerModel);
    sourceButtonGroup.add(SelectedTimeRadioButton);
    sourceButtonGroup.add(CurrentTimeRadioButton);
 }
/*================================================================================================
/     updateSelectedTimeDate()
/=================================================================================================*/
 private void updateSelectedTimeDate(){
    java.lang.String timeText = new java.lang.String();
    java.lang.String dateText = new java.lang.String();
    timeText = hours+":"+minutes+":"+seconds;
    dateText = constructMonthString(month)+"-"+day+"-"+year;
    SelectedTimeTextField.setText(timeText);
    SelectedDateTextField.setText(dateText);
 }
/*=================================================================================
/         updateDateTime()
/=================================================================================*/
public void updateDateTime(){
        java.lang.String[] date_time_string = constructJSkyCalcDateTime();
        if(date_time_string != null){
           myJSkyCalcModelTelescope.setSelectedDateTime(date_time_string);
           myJSkyCalcModelObjects.setSelectedDateTime(date_time_string);
           try{
             myJSkyCalcModelTelescope.setToDate(date_time_string[0], date_time_string[1]);
           }catch(Exception e){
              System.out.println(e.toString()); 
           }
           try{
              myJSkyCalcModelObjects.setToDate(date_time_string[0], date_time_string[1]);  
            }catch(Exception e2){
              System.out.println(e2.toString()); 
           }
          
        }
}
/*=================================================================================
/         createDateTime()
/=================================================================================*/
  public java.lang.String[] createDateTime(){
      java.lang.String[] date_time = new java.lang.String[2];
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
           day         = myGregorianCalendar.get(Calendar.DAY_OF_MONTH);
           month       = myGregorianCalendar.get(Calendar.MONTH);
           year        = myGregorianCalendar.get(Calendar.YEAR);
           hour        = myGregorianCalendar.get(Calendar.HOUR_OF_DAY);
           minute      = myGregorianCalendar.get(Calendar.MINUTE);
           second      = myGregorianCalendar.get(Calendar.SECOND);
           millisecond = myGregorianCalendar.get(Calendar.MILLISECOND);
          java.lang.String myMonth = new java.lang.String();
          myMonth = this.constructMonthString(month);
      date_time[0] = myMonth + "-" + day + "-" + year;
      date_time[1] = hour + ":" + minute + ":" + second;
  return date_time;
}
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        mainPanel = new javax.swing.JPanel();
        DatePanel = new javax.swing.JPanel();
        jLabel3 = new javax.swing.JLabel();
        TimePanel = new javax.swing.JPanel();
        HoursSpinner = new javax.swing.JSpinner();
        jLabel1 = new javax.swing.JLabel();
        jLabel4 = new javax.swing.JLabel();
        SecondsSpinner = new javax.swing.JSpinner();
        jLabel5 = new javax.swing.JLabel();
        MinutesSpinner = new javax.swing.JSpinner();
        jPanel1 = new javax.swing.JPanel();
        CurrentTimeRadioButton = new javax.swing.JRadioButton();
        SelectedTimeRadioButton = new javax.swing.JRadioButton();
        jLabel2 = new javax.swing.JLabel();
        jPanel2 = new javax.swing.JPanel();
        jLabel6 = new javax.swing.JLabel();
        SelectedDateTextField = new javax.swing.JTextField();
        jLabel7 = new javax.swing.JLabel();
        SelectedTimeTextField = new javax.swing.JTextField();
        UpdateButton = new javax.swing.JButton();

        setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);

        mainPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        DatePanel.setBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 0, 0), 2, true));
        DatePanel.setLayout(new java.awt.BorderLayout());

        jLabel3.setFont(new java.awt.Font("DejaVu LGC Sans", 1, 13));
        jLabel3.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel3.setText("Select Date");
        DatePanel.add(jLabel3, java.awt.BorderLayout.PAGE_START);

        mainPanel.add(DatePanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 20, 240, 140));

        TimePanel.setBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 0, 0), 2, true));
        TimePanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        HoursSpinner.addChangeListener(new javax.swing.event.ChangeListener() {
            public void stateChanged(javax.swing.event.ChangeEvent evt) {
                HoursSpinnerStateChanged(evt);
            }
        });
        TimePanel.add(HoursSpinner, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 30, 70, -1));

        jLabel1.setFont(new java.awt.Font("DejaVu LGC Sans", 1, 13));
        jLabel1.setText("Hours");
        TimePanel.add(jLabel1, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 10, -1, -1));

        jLabel4.setFont(new java.awt.Font("DejaVu LGC Sans", 1, 13));
        jLabel4.setText("Seconds");
        TimePanel.add(jLabel4, new org.netbeans.lib.awtextra.AbsoluteConstraints(160, 10, -1, -1));

        SecondsSpinner.addChangeListener(new javax.swing.event.ChangeListener() {
            public void stateChanged(javax.swing.event.ChangeEvent evt) {
                SecondsSpinnerStateChanged(evt);
            }
        });
        TimePanel.add(SecondsSpinner, new org.netbeans.lib.awtextra.AbsoluteConstraints(160, 30, 60, -1));

        jLabel5.setFont(new java.awt.Font("DejaVu LGC Sans", 1, 13));
        jLabel5.setText("Minutes");
        TimePanel.add(jLabel5, new org.netbeans.lib.awtextra.AbsoluteConstraints(90, 10, -1, -1));

        MinutesSpinner.addChangeListener(new javax.swing.event.ChangeListener() {
            public void stateChanged(javax.swing.event.ChangeEvent evt) {
                MinutesSpinnerStateChanged(evt);
            }
        });
        TimePanel.add(MinutesSpinner, new org.netbeans.lib.awtextra.AbsoluteConstraints(90, 30, 60, -1));

        mainPanel.add(TimePanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 170, 240, 70));

        jPanel1.setBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 0, 0), 2, true));

        CurrentTimeRadioButton.setText("Current Time");
        CurrentTimeRadioButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                CurrentTimeRadioButtonActionPerformed(evt);
            }
        });

        SelectedTimeRadioButton.setText("Selected Time");
        SelectedTimeRadioButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                SelectedTimeRadioButtonActionPerformed(evt);
            }
        });

        javax.swing.GroupLayout jPanel1Layout = new javax.swing.GroupLayout(jPanel1);
        jPanel1.setLayout(jPanel1Layout);
        jPanel1Layout.setHorizontalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel1Layout.createSequentialGroup()
                .addComponent(CurrentTimeRadioButton)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addComponent(SelectedTimeRadioButton, javax.swing.GroupLayout.PREFERRED_SIZE, 123, javax.swing.GroupLayout.PREFERRED_SIZE))
        );
        jPanel1Layout.setVerticalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel1Layout.createSequentialGroup()
                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(CurrentTimeRadioButton)
                    .addComponent(SelectedTimeRadioButton))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        mainPanel.add(jPanel1, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 250, 240, 30));

        jLabel2.setFont(new java.awt.Font("DejaVu LGC Sans", 1, 13));
        jLabel2.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel2.setText("Ephemeris Time Selector");
        mainPanel.add(jLabel2, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 0, 260, -1));

        jPanel2.setBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 0, 0), 2, true));

        jLabel6.setText("Selected Date:");

        SelectedDateTextField.setEditable(false);
        SelectedDateTextField.setFont(new java.awt.Font("DejaVu LGC Sans", 1, 14)); // NOI18N
        SelectedDateTextField.setHorizontalAlignment(javax.swing.JTextField.CENTER);
        SelectedDateTextField.setText(" ");

        jLabel7.setText("Selected Time:");

        SelectedTimeTextField.setEditable(false);
        SelectedTimeTextField.setFont(new java.awt.Font("DejaVu LGC Sans", 1, 14)); // NOI18N
        SelectedTimeTextField.setHorizontalAlignment(javax.swing.JTextField.CENTER);
        SelectedTimeTextField.setText(" ");

        javax.swing.GroupLayout jPanel2Layout = new javax.swing.GroupLayout(jPanel2);
        jPanel2.setLayout(jPanel2Layout);
        jPanel2Layout.setHorizontalGroup(
            jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel2Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel2Layout.createSequentialGroup()
                        .addComponent(jLabel6)
                        .addGap(3, 3, 3))
                    .addGroup(jPanel2Layout.createSequentialGroup()
                        .addComponent(jLabel7)
                        .addGap(3, 3, 3)))
                .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(SelectedDateTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 115, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(SelectedTimeTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 115, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addGap(40, 40, 40))
        );
        jPanel2Layout.setVerticalGroup(
            jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel2Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(jLabel6)
                    .addComponent(SelectedDateTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(jLabel7)
                    .addComponent(SelectedTimeTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        mainPanel.add(jPanel2, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 290, 240, 80));

        UpdateButton.setText("Update");
        UpdateButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                UpdateButtonActionPerformed(evt);
            }
        });
        mainPanel.add(UpdateButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(80, 380, 100, -1));

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addComponent(mainPanel, javax.swing.GroupLayout.PREFERRED_SIZE, 271, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(mainPanel, javax.swing.GroupLayout.DEFAULT_SIZE, 418, Short.MAX_VALUE)
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents

    private void HoursSpinnerStateChanged(javax.swing.event.ChangeEvent evt) {//GEN-FIRST:event_HoursSpinnerStateChanged
        // TODO add your handling code here:
         hours = ((Integer)HoursSpinner.getValue()).intValue();
         updateSelectedTimeDate();
    }//GEN-LAST:event_HoursSpinnerStateChanged

    private void MinutesSpinnerStateChanged(javax.swing.event.ChangeEvent evt) {//GEN-FIRST:event_MinutesSpinnerStateChanged
         minutes = ((Integer)MinutesSpinner.getValue()).intValue();
         updateSelectedTimeDate();
        // TODO add your handling code here:
    }//GEN-LAST:event_MinutesSpinnerStateChanged

    private void SecondsSpinnerStateChanged(javax.swing.event.ChangeEvent evt) {//GEN-FIRST:event_SecondsSpinnerStateChanged
         seconds = ((Integer)SecondsSpinner.getValue()).intValue();
         updateSelectedTimeDate();
        // TODO add your handling code here:
    }//GEN-LAST:event_SecondsSpinnerStateChanged

    private void CurrentTimeRadioButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_CurrentTimeRadioButtonActionPerformed
        // TODO add your handling code here:
        boolean state = CurrentTimeRadioButton.isSelected();
        if(state){
            time_source = NOW;
            java.lang.String[] date_time = createDateTime();
            SelectedDateTextField.setText(date_time[0]);
            SelectedTimeTextField.setText(date_time[1]);
            myJSkyCalcModelTelescope.setTimeSource(NOW);
            myJSkyCalcModelObjects.setTimeSource(NOW);
        }
        if(!state){
            time_source = SELECTED_TIME;
            myJSkyCalcModelTelescope.setTimeSource(SELECTED_TIME);
            myJSkyCalcModelObjects.setTimeSource(SELECTED_TIME);
            updateSelectedTimeDate();
        }
    }//GEN-LAST:event_CurrentTimeRadioButtonActionPerformed
    private void SelectedTimeRadioButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_SelectedTimeRadioButtonActionPerformed
       boolean state = SelectedTimeRadioButton.isSelected();
        if(state){
            time_source = SELECTED_TIME;
            myJSkyCalcModelTelescope.setTimeSource(SELECTED_TIME);
            myJSkyCalcModelObjects.setTimeSource(SELECTED_TIME);
            updateSelectedTimeDate();
        }
        if(!state){
            time_source = NOW;
            java.lang.String[] date_time = createDateTime();
            SelectedDateTextField.setText(date_time[0]);
            SelectedTimeTextField.setText(date_time[1]);
            myJSkyCalcModelTelescope.setTimeSource(NOW);
            myJSkyCalcModelObjects.setTimeSource(NOW);
         }
    }//GEN-LAST:event_SelectedTimeRadioButtonActionPerformed
    private void UpdateButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_UpdateButtonActionPerformed
        // TODO add your handling code here:
        updateDateTime();
    }//GEN-LAST:event_UpdateButtonActionPerformed
 /*=============================================================================================
/       INNER CLASS that sends a set of coordinates to the telescope
/=============================================================================================*/
    public class UpdateTimeThread  implements Runnable{
    private Thread myThread;
    private boolean running;
    private long    polling_time;
    private java.util.StringTokenizer st;

 /*=============================================================================================
/          UpdateTimeThread()
/=============================================================================================*/
    private UpdateTimeThread(){
    }
/*================================================================================================
/      waitForResponse(int newDelay)
/=================================================================================================*/
public void Wait(long newDelay){
     try{
        Thread.currentThread().sleep(newDelay);} // sleep for awhile to let the messages pile up
      catch (Exception e){
    }
}
/*=============================================================================================
/        setRunning(boolean new_running)
/=============================================================================================*/
public void setRunning(boolean new_running){
    running = new_running;
}
public boolean isRunning(){
    return running;
}
/*=============================================================================================
/       setPollingTime(long new_polling_time)
/=============================================================================================*/
public void setPollingTime(long new_polling_time){
    polling_time = new_polling_time;
}
/*=============================================================================================
/           run()
/=============================================================================================*/
    public void run(){
       int image_number = 0;
       setRunning(true);
       java.lang.String[] date_time = createDateTime();
       st = new java.util.StringTokenizer(date_time[1],":");
       while(running){
           if(time_source == NOW){
            date_time = createDateTime();
            SelectedDateTextField.setText(date_time[0]);
            SelectedTimeTextField.setText(date_time[1]);
            try{
              hours   = Integer.parseInt(st.nextToken());
              minutes = Integer.parseInt(st.nextToken());
              seconds = Integer.parseInt(st.nextToken());
              myDateTimeModel.set_hour(hours);
              myDateTimeModel.set_minute(minutes);
              myDateTimeModel.set_second(seconds);
            }catch(Exception e){
                      
            }
           }
           Wait(1000);
        }
     }
/*=============================================================================================
/         start()
/=============================================================================================*/
    public void start(){
     myThread = new Thread(this);
     myThread.start();
     }
    }// End of the MonitorAllSkyImageThread Inner Class
/*=============================================================================================
/      End of the MonitorAllSkyImageThread Class
/=============================================================================================*/

    /**
    * @param args the command line arguments
    */
    public static void main(String args[]) {
        java.awt.EventQueue.invokeLater(new Runnable() {
            public void run() {
                new DateTimeChooserFrame().setVisible(true);
            }
        });
    }

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JRadioButton CurrentTimeRadioButton;
    private javax.swing.JPanel DatePanel;
    private javax.swing.JSpinner HoursSpinner;
    private javax.swing.JSpinner MinutesSpinner;
    private javax.swing.JSpinner SecondsSpinner;
    private javax.swing.JTextField SelectedDateTextField;
    private javax.swing.JRadioButton SelectedTimeRadioButton;
    private javax.swing.JTextField SelectedTimeTextField;
    private javax.swing.JPanel TimePanel;
    private javax.swing.JButton UpdateButton;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JLabel jLabel3;
    private javax.swing.JLabel jLabel4;
    private javax.swing.JLabel jLabel5;
    private javax.swing.JLabel jLabel6;
    private javax.swing.JLabel jLabel7;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JPanel jPanel2;
    private javax.swing.JPanel mainPanel;
    // End of variables declaration//GEN-END:variables

}
