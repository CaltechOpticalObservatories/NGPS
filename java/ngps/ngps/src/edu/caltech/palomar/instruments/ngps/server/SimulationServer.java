/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package edu.caltech.palomar.instruments.ngps.server;

import edu.caltech.palomar.io.PalomarServerSocket;
import edu.caltech.palomar.dhe2.ObservationSequencerObject;
/**
 *
 * @author jennifermilburn
 */
public class SimulationServer extends PalomarServerSocket{
    public java.lang.String            USERDIR         = System.getProperty("user.dir");
    private java.lang.String           SEP             = System.getProperty("file.separator");
    private java.lang.String           CONFIG          = new java.lang.String("config");
    public ObservationSequencerObject  myObservationSequencerObject;
/*=============================================================================================
/             SimulationServer()
/=============================================================================================*/
    public SimulationServer(){
       super(); 
    }
/*=============================================================================================
/             SimulationServer()
/=============================================================================================*/
  public void setSequencerObject(ObservationSequencerObject current){
      myObservationSequencerObject = current; 
  }
/*=============================================================================================
/               prepareResponse(java.lang.String commandString) method
/=============================================================================================*/
  public java.lang.String prepareResponse(java.lang.String commandString){
      // This is the method that is over-ridden by classes that extend the Server Socket
      // This method is called when a command string is received from a client and this is
      // where the parsing of the command string and the preparation of the response takes place
      java.lang.String response = new java.lang.String();
      java.util.StringTokenizer st = new java.util.StringTokenizer(commandString);
      int tokenCount = st.countTokens();
      if(tokenCount == 3){
          java.lang.String command   = st.nextToken(" ");
          java.lang.String object    = st.nextToken(" ");
          java.lang.String value     = st.nextToken(" ");
          if(command.matches("SET")){
              if(object.matches("STATE")){
                  myObservationSequencerObject.setSTATE(value);
              }
          }           
      }               
   return response;   
  }    
}
