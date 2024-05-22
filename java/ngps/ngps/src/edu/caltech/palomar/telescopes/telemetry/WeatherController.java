/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package edu.caltech.palomar.telescopes.telemetry;

import edu.caltech.palomar.io.ClientSocket;
import edu.caltech.palomar.telescopes.telemetry.SFTPRetrieveWeather;
import edu.caltech.palomar.telescopes.telemetry.P60WeatherObject;

/** 
 *
 * @author developer
 */
/*=============================================================================================
/     Class Declaration WeatherController()
/=============================================================================================*/
public class WeatherController {
    public P200WeatherObject   myP200WeatherObject   = new P200WeatherObject();
    public P60WeatherObject    myP60WeatherObject    = new P60WeatherObject();
    public P48WeatherObject    myP48WeatherObject    = new P48WeatherObject();
    private java.lang.String   TERMINATOR            = new java.lang.String("\n");
/*=============================================================================================
/        public constructor  WeatherController()
/=============================================================================================*/
    public WeatherController(){
       boolean test = false;
       if(test){
          process(); 
       }
    }
    public void process(){
        myP200WeatherObject.execute_process();
        waitForResponseMilliseconds(1000);
        myP60WeatherObject.execute_process();
        waitForResponseMilliseconds(1000);
        myP48WeatherObject.execute_process();
    }
/*================================================================================================
/      getWEATHER_FITS_HEADER()
/=================================================================================================*/
    public java.lang.String getWEATHER_FITS_HEADER(){
       java.lang.String weather_record_current = new java.lang.String();
       java.lang.String p200_weather = myP200WeatherObject.getCURRENT_FITS_HEADER_STRING();
       java.lang.String p60_weather  = myP60WeatherObject.getCURRENT_FITS_HEADER_STRING();
       java.lang.String p48_weather  = myP48WeatherObject.getCURRENT_FITS_HEADER_STRING();
       System.out.println("P200 WEATHER RECORD");        
       System.out.println(p200_weather);        
       System.out.println("P60 WEATHER RECORD");        
       System.out.println(p60_weather);        
       System.out.println("P48 WEATHER RECORD");        
       System.out.println(p48_weather);        
       weather_record_current = p200_weather  + p60_weather + p48_weather;
       return weather_record_current;
    }
 /*================================================================================================
/      getWEATHER_FITS_HEADER()
/=================================================================================================*/
    public java.lang.String getWEATHER_JSON(){
        java.lang.String weather_record_current = new java.lang.String();
       try{
           weather_record_current = new java.lang.String();
           weather_record_current = "["+myP200WeatherObject.getCURRENT_GSON_WEATHER() + "," + myP60WeatherObject.getCURRENT_GSON_WEATHER()+"," + myP48WeatherObject.getCURRENT_GSON_WEATHER()+"]";           
       }catch(Exception e){
           System.out.println(e.toString());
       } 
       return weather_record_current;
    }   
/*================================================================================================
/      waitForResponse(int newDelay)
/=================================================================================================*/
public void waitForResponseMilliseconds(int newDelay){
     try{
        Thread.currentThread().sleep(newDelay);} // sleep for awhile to let the messages pile up
      catch (Exception e){
    }
}   
/*================================================================================================
/      public static void main(String args[])
/=================================================================================================*/
      public static void main(String args[]) {
        /* Set the Nimbus look and feel */
       new WeatherController(); //<editor-fold defaultstate="collapsed" desc=" Look and feel setting code (optional) ">
        /* If Nimbus (introduced in Java SE 6) is not available, stay with the default look and feel.
         * For details see http://download.oracle.com/javase/tutorial/uiswing/lookandfeel/plaf.html 
         */
        try {
            for (javax.swing.UIManager.LookAndFeelInfo info : javax.swing.UIManager.getInstalledLookAndFeels()) {
                if ("Nimbus".equals(info.getName())) {
                    javax.swing.UIManager.setLookAndFeel(info.getClassName());
                    break;
                }
            }
        } catch (ClassNotFoundException ex) {
            java.util.logging.Logger.getLogger(SFTPRetrieveWeather.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (InstantiationException ex) {
            java.util.logging.Logger.getLogger(SFTPRetrieveWeather.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (IllegalAccessException ex) {
            java.util.logging.Logger.getLogger(SFTPRetrieveWeather.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (javax.swing.UnsupportedLookAndFeelException ex) {
            java.util.logging.Logger.getLogger(SFTPRetrieveWeather.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        }
        //</editor-fold>
    }
}
