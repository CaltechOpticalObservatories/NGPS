/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package edu.caltech.palomar.telescopes.telemetry;

/**
 *
 * @author developer
 */
public class TelemetryServerSocket extends BaseServerSocket{
   public TelemetryController myTelemetryController;
   public WeatherController   myWeatherController;
/*=============================================================================================
/               prepareResponse(java.lang.String commandString) method
/=============================================================================================*/
   public TelemetryServerSocket(int port){
      super(port);
      setListenMode(BaseServerSocket.CLIENT_LISTEN);
   }
 /*=============================================================================================
/               prepareResponse(java.lang.String commandString) method
/=============================================================================================*/
  public void setTelemetryController(TelemetryController newTelemetryController){
    myTelemetryController = newTelemetryController;  
  }  
  public void setWeatherController(WeatherController newWeatherController){
      myWeatherController = newWeatherController;
  }
/*=============================================================================================
/               prepareResponse(java.lang.String commandString) method
/=============================================================================================*/
  public java.lang.String prepareResponse(java.io.BufferedWriter client,java.lang.String commandString){
      // This is the method that is over-ridden by classes that extend the Server Socket
      // This method is called when a command string is received from a client and this is
      // where the parsing of the command string and the preparation of the response takes place
      java.lang.String response = new java.lang.String();
      commandString = commandString.toUpperCase();
      java.util.StringTokenizer st = new java.util.StringTokenizer(commandString," ");
      java.lang.String command = st.nextToken();
      if(command.matches("REQUEST_HEADER")){
         response = myTelemetryController.getCURRENT_FITS_HEADER_STRING(); 
      }      
      if(command.matches("REQUEST_JSON")){
         response = myTelemetryController.getCURRENT_GSON_TELEMETRY(); 
      }
      if(command.matches("QUERY")){
         java.lang.String date_string = st.nextToken();
         java.lang.String time_string = st.nextToken();
         java.lang.String timestamp_string = date_string + " "+time_string;
         response = myTelemetryController.queryToJSON(timestamp_string);
      }
      if(command.matches("QUERYRANGE")){
         java.lang.String start_string = st.nextToken(",");
         start_string = start_string.trim();
         java.lang.String end_string = st.nextToken(",");
         long start = System.currentTimeMillis();
         response = myTelemetryController.queryRangeToJSON(client,start_string,end_string);
         long end = System.currentTimeMillis();
         long duration = end - start;
         int  duration_seconds = (int)(duration/1000.0);
         System.out.println("Query Time = "+duration_seconds);
         response = response = "";
      }   
      if(command.matches("WEATHER_FITS")){
          response = myWeatherController.getWEATHER_FITS_HEADER();
      }
      if(command.matches("WEATHER_JSON")){
          try{
             response = myWeatherController.getWEATHER_JSON();              
          }catch(Exception e){
              System.out.println(e.toString());
          }          
      }
   return response;   
  }
}
