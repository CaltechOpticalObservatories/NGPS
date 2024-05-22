/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package edu.caltech.palomar.telescopes.telemetry;

import edu.caltech.palomar.telescopes.telemetry.TelemetryServerFrame;
import edu.caltech.palomar.telescopes.telemetry.TelemetryController;
import edu.caltech.palomar.util.general.CommandLogModel;
/**
 *
 * @author developer
 */
/*=============================================================================================
/    Class Definition TelemetryServer
/=============================================================================================*/
public class TelemetryServer {
    public TelemetryController  myTelemetryController;
    public WeatherController    myWeatherController;
    public TelemetryServerFrame myTelemetryServerFrame;
    public TelemetryServerSocket myTelemetryServerSocket;
    public static int            DEFAULT_SERVER_PORT = 4698;
/*=============================================================================================
/     constructor TelemetryServer()
/=============================================================================================*/
    public TelemetryServer(){
      initializeTelemetryController();   
      initializeWeatherController();
      initializeServer();
//      testQuery();
    }
    private void testQuery(){
        myTelemetryController.queryTimestamp("'2019-08-26 08:14:07");
    }
    private void initializeTelemetryController(){
      myTelemetryController = new TelemetryController();
      myTelemetryServerFrame = new TelemetryServerFrame();
      myTelemetryServerFrame.initialize(myTelemetryController);
      myTelemetryServerFrame.setVisible(true);
      myTelemetryController.connect();
      myTelemetryController.startPolling();
    }
    private void initializeWeatherController(){
        myWeatherController = new WeatherController();
        myWeatherController.process();
    }
    private void initializeServer(){
      myTelemetryServerSocket = new TelemetryServerSocket(DEFAULT_SERVER_PORT);  
      myTelemetryServerSocket.setTelemetryController(myTelemetryController);
      myTelemetryServerSocket.setWeatherController(myWeatherController);
      myTelemetryServerFrame.setWeatherController(myWeatherController);
      myTelemetryServerFrame.setComsLog(myTelemetryServerSocket.myCommandLogModel);
      myTelemetryServerFrame.setConnectionLog(myTelemetryServerSocket.myConnectionsLogModel);
      myTelemetryServerSocket.startServer();
    }
    public static void main(String args[]) {
        java.awt.EventQueue.invokeLater(new Runnable() {
            public void run() {
                new TelemetryServer();
            }
        });
    }
}
