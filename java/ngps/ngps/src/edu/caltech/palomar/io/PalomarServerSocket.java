package edu.caltech.palomar.io; 
//=== File Prolog =============================================================
//	This code was developed by California Institute of Technology
//      Caltech Optical Observatories, Palomar Observatory
//
//--- Contents ----------------------------------------------------------------
//	 PalomarServerSocket
//
//--- Description -------------------------------------------------------------
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
//
//      July 14, 2010 Jennifer Milburn
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

import java.io.IOException;
import java.io.BufferedReader; 
import java.io.BufferedWriter;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.util.Enumeration;
import java.beans.*;
import java.net.InetAddress;
import edu.caltech.palomar.util.general.CommandLogModel;
/*=============================================================================================
/      Class Definition  :   public class PalomarServerSocket
/=============================================================================================*/
public class  PalomarServerSocket extends java.lang.Object{
   protected transient PropertyChangeSupport propertyChangeListeners = new PropertyChangeSupport(this);
   private java.lang.String       clientName         = new java.lang.String();
   private java.lang.String       TERMINATOR         = new java.lang.String("\n");
   private java.lang.String       CR                 = new java.lang.String("\r");
   public static int              DEFAULT_SERVERPORT = 49200;
   public static int              USE_INETADDRESS    = 1;
   public static int              USE_HOSTNAME       = 2;
   public static int              LINGER_TIME        = 65000;
   public static int              DEFAULT_TELCO_PORT = 49200;
      //  Vector of clients connected to the server
   public java.util.Vector        clientVector   = new java.util.Vector(0,1);
   public AcceptClient            myAcceptClient = new AcceptClient();
      //  Socket and associated Buffered Readers and Writers
   public java.net.ServerSocket   serverSocket;
   public int                     mode;
   private int                    serverPort;
   private boolean                isLogging;
      // global scope objects that are not instantiated until needed
   private InetAddress           serverInetAddress;
   private javax.swing.JTextArea errorLogTextArea   = new javax.swing.JTextArea();
   private javax.swing.JTextArea sendLogTextArea    = new javax.swing.JTextArea();
   private javax.swing.JTextArea receiveLogTextArea = new javax.swing.JTextArea();
   public  CommandLogModel       myCommandLogModel  = new CommandLogModel();
   private   boolean             connected;
   protected boolean             serverRunning;
   public  java.lang.String      RESPONSE_STRING = new java.lang.String();
   SimulationThread              mySimulationThread = new SimulationThread(true);
/*=============================================================================================
/                                Constructors
/   public PalomarServerSocket()
/   public PalomarServerSocket(int newServerPort)
/=============================================================================================*/
/*=============================================================================================
/         PalomarServerSocket(int newServerPort) Constructors
/=============================================================================================*/
  public  PalomarServerSocket() {
     setServerPort(DEFAULT_SERVERPORT);
     setLogging(true);
     RESPONSE_STRING = "OK";
  }
/*=============================================================================================
/             ServerSocket(int newServerPort) Constructors
/=============================================================================================*/
  public  PalomarServerSocket(int newServerPort) {
     setServerPort(newServerPort);
     setLogging(true);
     RESPONSE_STRING = "OK";
  }
/*=============================================================================================
/               prepareResponse(java.lang.String commandString) method
/=============================================================================================*/
  public java.lang.String prepareResponse(java.lang.String commandString){
      // This is the method that is over-ridden by classes that extend the Server Socket
      // This method is called when a command string is received from a client and this is
      // where the parsing of the command string and the preparation of the response takes place
      java.lang.String response = new java.lang.String();
   return response;   
  }
/*=============================================================================================
/               simulationUpdate() method
/=============================================================================================*/
  public void simulationUpdate(){
      // This is the method that is over-ridden by classes that extend the Server Socket
      // This method is called within the SimulationThread and is used to allow internal 
      // parameters to be changed and update in a systematic fashion.  Place any slowly
      // varying values that are not deterministic to be updated in this method.  For example
      // the airmass, zenith angle, hour angle can be gradually changed so they cover the
      // entire range of possible values returned by a server. 
  }
/*=============================================================================================
/                    startServer() method
/=============================================================================================*/
  public void startServer(){
    try{
          serverSocket = new java.net.ServerSocket(serverPort);
          setServerRunning(true);
          myAcceptClient.start();
          mySimulationThread.start();
    }
    catch(Exception  ex2){
       logErrorMessage("An error occured while trying to start the server on port number: "+ getServerPort() + ex2);
    }// end of the catch block
  }// end of the startServerMethod
/*=============================================================================================
/   addClient(java.net.Socket newClientSocket) method
/=============================================================================================*/
  public void addClient(java.net.Socket newClientSocket){
    ClientDescription myClientDescription = new ClientDescription(newClientSocket);
    try{
       myClientDescription.startIO();
       clientVector.addElement(myClientDescription);
    }// end of the try block
   catch(Exception  ex2){
       logErrorMessage("An IO Exception occured while adding a new client: " + ex2);
       setConnected(false);
       removeClient(myClientDescription.getClient());
    }// end of the catch block
  }
/*=============================================================================================
/   removeClient(ClientDescription removeClientInstance) method
/=============================================================================================*/
  public void removeClient(ClientDescription removeClientInstance){
      boolean removalResult = clientVector.removeElement(removeClientInstance);
  }
/*=============================================================================================
/   closeConnections() method
/=============================================================================================*/
  public boolean closeConnections(){
   try{
     if(!(clientVector.isEmpty())){
               for(Enumeration clientEnumeration = clientVector.elements();clientEnumeration.hasMoreElements();){
                      ClientDescription myClient = ((ClientDescription)clientEnumeration.nextElement());
                      myClient.closeClient();
               }// end of the for loop for each client
        // Now we remove all of the clients from the clientVector
        clientVector.removeAllElements();
     }// end of the if !myCyclesVector.isEmpty() check
       if(serverSocket != null){
          serverSocket.close();
        }
     setConnected(false);
     setServerRunning(false);
    }
    catch(IOException  ex2){
       logErrorMessage("An IO Exception occured while closing the connection: " + ex2);
    }
  return isConnected();
 }
/*=============================================================================================
/   finalize() method - closes all the streams and the socket if not already accomplished
/=============================================================================================*/
 protected void finalize(){
  try{
    closeConnections();
    // call the finalize method of the Object Super Class
    super.finalize();
  }
  catch(Throwable t){
       logErrorMessage("An Exception occured while closing the connection: " + t);
    }
 }
/*=============================================================================================
/                             Set and Get Server Port Number
/     public void setServerPort(int newServerPort)
/     public int getServerPort()
/=============================================================================================*/
  public void setServerPort(int newServerPort) {
    int  oldServerPort = serverPort;
    serverPort = newServerPort;
    propertyChangeListeners.firePropertyChange("serverPort", Integer.valueOf(oldServerPort), Integer.valueOf(newServerPort));
  }
  public int getServerPort() {
    return serverPort;
  }
/*=============================================================================================
/                            Log Message Methods
/      public void logErrorMessage(java.lang.String newMessage)
/      public void logSendMessage(java.lang.String newMessage)
/      public void logReceiveMessage(java.lang.String newMessage)
/=============================================================================================*/
  public void logErrorMessage(java.lang.String newMessage){
      if(isLogging){
         if(errorLogTextArea != null){
              errorLogTextArea.append(newMessage + TERMINATOR);
              myCommandLogModel.insertMessage(CommandLogModel.ERROR, newMessage+TERMINATOR);
         }// checking to make sure the errorLogTextArea actually exists
      }// check to make sure we are logging things
  }// end of the logErrorMessage
  public void logSendMessage(java.lang.String newMessage){
      if(isLogging){
         if(sendLogTextArea != null){
              sendLogTextArea.append(newMessage + TERMINATOR);
              myCommandLogModel.insertMessage(CommandLogModel.COMMAND, newMessage+TERMINATOR);
         }// checking to make sure the sendLogTextArea actually exists
      }// check to make sure we are logging things
  }// end of the logSendMessage

  public void logReceiveMessage(java.lang.String newMessage){
      if(isLogging){
         if(receiveLogTextArea != null){
              receiveLogTextArea.append(newMessage + TERMINATOR);
              myCommandLogModel.insertMessage(CommandLogModel.RESPONSE, newMessage+TERMINATOR);
         }// checking to make sure the receiveLogTextArea actually exists
      }// check to make sure we are logging things
  }// end of the logReceiveMessage
/*=============================================================================================
/                             Logging Control Methods
/     public void setLogging(boolean newLogging)
/     public boolean isLogging()
/=============================================================================================*/
  public void setLogging(boolean newLogging){
      boolean oldLogging = isLogging;
      isLogging = newLogging;
      propertyChangeListeners.firePropertyChange("isLogging", Boolean.valueOf(oldLogging), Boolean.valueOf(newLogging));
  }
  public boolean isLogging(){
     return isLogging;
  }
/*=============================================================================================
/                             Mode Control Methods
/     public void setMode(int newMode)
/     public int getMode()
/=============================================================================================*/
  public void setMode(int newMode){
      int oldMode = mode;
      mode = newMode;
      propertyChangeListeners.firePropertyChange("mode", Integer.valueOf(oldMode), Integer.valueOf(newMode));
  }
  public int getMode(){
     return mode;
  }
/*=============================================================================================
/                            Text Area controls for logging messages
/     public void                  setSendLoggingTextArea(javax.swing.JTextArea newTextArea)
/     public javax.swing.JTextArea getSendLoggingTextArea()
/     public void                  setReceiveLoggingTextArea(javax.swing.JTextArea newTextArea)
/     public javax.swing.JTextArea getReceiveLoggingTextArea()
/     public void                  configureTextAreas(javax.swing.JTextArea newErrorTextArea,
/                                                     javax.swing.JTextArea newSendTextArea,
/                                                     javax.swing.JTextArea newReceiveTextArea)
/     public void                  configureTextAreas(javax.swing.JTextArea newTextArea)
/=============================================================================================*/
  public void configureTextAreas(javax.swing.JTextArea newErrorTextArea,
                                 javax.swing.JTextArea newSendTextArea,
                                 javax.swing.JTextArea newReceiveTextArea){
  // This is a convenience method that allows you to configure which JTextArea's will
  // be used for logging errors, sent commands, and responses.  The same JTextArea
  // may be used for all three of the areas.
       setErrorLoggingTextArea(newErrorTextArea);
       setSendLoggingTextArea(newSendTextArea);
       setReceiveLoggingTextArea(newReceiveTextArea);
  }// end of the configureTextAreas method

  public void configureTextAreas(javax.swing.JTextArea newTextArea){
   // This is a convenience method that allows you to configure the JTextAreas so that
   // all of the messages are sent to the same JTextArea
       setErrorLoggingTextArea(newTextArea);
       setSendLoggingTextArea(newTextArea);
       setReceiveLoggingTextArea(newTextArea);
  }// end of the configureTextAreas method

  public void setErrorLoggingTextArea(javax.swing.JTextArea newTextArea){
      errorLogTextArea  = newTextArea;
  }
  public javax.swing.JTextArea getErrorLoggingTextArea(){
      return errorLogTextArea;
  }
  public void setSendLoggingTextArea(javax.swing.JTextArea newTextArea){
      sendLogTextArea  = newTextArea;
  }
  public javax.swing.JTextArea getSendLoggingTextArea(){
      return sendLogTextArea;
  }
  public void setReceiveLoggingTextArea(javax.swing.JTextArea newTextArea){
      receiveLogTextArea  = newTextArea;
  }
  public javax.swing.JTextArea getReceiveLoggingTextArea(){
      return receiveLogTextArea;
  }
/*=============================================================================================
/                           ServerRunning Methods
/=============================================================================================*/
  public void setServerRunning(boolean newServerRunning){
      boolean oldServerRunning = serverRunning;
      serverRunning = newServerRunning;
      propertyChangeListeners.firePropertyChange("isServerRunning", Boolean.valueOf(oldServerRunning), Boolean.valueOf(newServerRunning));
 }
 public boolean isServerRunning(){
      return serverRunning;
 }
/*=============================================================================================
/                           Property Change Listeners Methods
/    public synchronized void removePropertyChangeListener(PropertyChangeListener l)
/    public synchronized void addPropertyChangeListener(PropertyChangeListener l)
/=============================================================================================*/
  public synchronized void removePropertyChangeListener(PropertyChangeListener l) {
    propertyChangeListeners.removePropertyChangeListener(l);
  }
  public synchronized void addPropertyChangeListener(PropertyChangeListener l) {
    propertyChangeListeners.addPropertyChangeListener(l);
  }
/*=============================================================================================
/                           Connected Methods
/  These methods can be used to determine if the socket is currently connected to the server
/     public void setConnected(boolean newConnected)
/     public boolean isConnected()
/=============================================================================================*/
  public void setConnected(boolean newConnected) {
    boolean  oldConnected = connected;
    connected = newConnected;
    propertyChangeListeners.firePropertyChange("connected", Boolean.valueOf(oldConnected),Boolean.valueOf(newConnected));
  }
  public boolean isConnected() {
    return connected;
  }
/*=============================================================================================
/                          AcceptClient Inner Class -
/    This class spawns a separate thread and listens for the client to connect.
/=============================================================================================*/
  public class AcceptClient implements Runnable{
  private Thread myThread;
  public void run(){
    while(isServerRunning()){
     try{
         java.net.Socket newClientSocket =  serverSocket.accept();
         addClient(newClientSocket);
      }// end of the try block
       catch(IOException  ex2){
       logErrorMessage("An IO Exception occured while trying to add a new socket for a client: " + ex2);
     }// end of the catch block
    }// end of the while loop
  }// end of the run method
  public void start(){
   myThread = new Thread(this);
   myThread.start();
   }
  }// End of the AcceptClient Inner Class
/*=============================================================================================
/                      ClientDescription Inner Class -
/    This class describes an individual client connection
/=============================================================================================*/
  public class ClientDescription {
   public java.net.Socket        clientSocket;
   public java.io.BufferedReader clientBufferedReader;
   public java.io.BufferedWriter clientBufferedWriter;
   public java.net.InetAddress   clientInetAddress;
   public java.lang.String       clientHostName = new java.lang.String();
   public boolean                clientConnected = false;
   public int                    READ_DELAY = 25;
   public boolean                listening;
   public StartListening         myStartListening = new StartListening();
/*=============================================================================================
/            public ClientDescription(java.net.Socket newClientSocket)
/=============================================================================================*/
   public ClientDescription(java.net.Socket newClientSocket){
       this.clientSocket = newClientSocket;
       setListening(false);
       try{
           // The following statement lets us wait for input into the socket
           clientSocket.setSoLinger(true,LINGER_TIME);
           clientInetAddress = clientSocket.getInetAddress();
           clientHostName    = clientInetAddress.getHostName();
         }// end of the try block
         catch(IOException  ex2){
             logErrorMessage("An IO Exception while creating a ClientDescription: " + ex2);
           }// end of the catch block
   }
/*=============================================================================================
/                          public void startIO()
/=============================================================================================*/
   public void startIO(){
     try{
     if(clientSocket != null){
         // Now we need to create the Readers and Writers to the Socket if everything worked
         clientBufferedReader = new BufferedReader( new InputStreamReader(clientSocket.getInputStream()));
         clientBufferedWriter = new BufferedWriter( new OutputStreamWriter(clientSocket.getOutputStream()));
         clientConnected = true;
         setListening(true);
         myStartListening.start();
     }// a probably unecessary check to make sure the socket was actually created
   }// end of the try block
   catch(IOException  ex2){
       logErrorMessage("An IO Exception occured while setting up the readers and writers for the new client: " + ex2);
     }// end of the catch block
   }
// We need to spawn threads to listen to the Socket of each client and respond to any messages
/*=============================================================================================
/                             public void listen()
/=============================================================================================*/
   public void listen(){
     java.lang.String newMessage  = new java.lang.String();
     java.lang.String newResponse = new java.lang.String();
     java.lang.String sendingFITS = new java.lang.String();
   try{
     if(clientSocket != null){
       while(isListening()){
        if(clientBufferedReader.ready()){
           // Get the incoming Message from the BufferedReader
               newMessage = clientBufferedReader.readLine();
               logSendMessage(newMessage);
               java.lang.String response = prepareResponse(newMessage);
           // Now we write the response to the client's socket
             if(response.length() > 0){
               clientBufferedWriter.write(response + CR+TERMINATOR);
               clientBufferedWriter.flush();                
               logReceiveMessage(response);
             }
        }
        Thread.currentThread().sleep(READ_DELAY);
       }
     }// a probably unecessary check to make sure the socket was actually created
   }// end of the try block
   catch(IOException  ex1){
       logErrorMessage("An IO Exception occured while writing to client: " + ex1);
   }// end of the catch block
   catch(java.lang.InterruptedException ex2){
         logErrorMessage("An error occurred while waiting to read from the client: "+ex2);
   }// end of the catch
}// end of the Listen Method
/*=============================================================================================
/                             isListening Methods
/=============================================================================================*/
  public void setListening(boolean newListening){
      boolean oldListening = listening;
      listening = newListening;
     propertyChangeListeners.firePropertyChange("listening", Boolean.valueOf(oldListening),Boolean.valueOf(newListening));
  }
  public boolean isListening(){
    return listening;
  }
/*=============================================================================================
/                             public void closeClient()
/=============================================================================================*/
   public void closeClient(){
   try{
       setListening(false);
        if(clientBufferedReader != null){
            clientBufferedReader.close();
          }
        if(clientBufferedWriter != null){
            clientBufferedWriter.close();
         }
        if(clientSocket != null){
          clientSocket.close();
        }
      clientConnected = false;
    }// end of the try block
    catch(IOException  ex2){
       logErrorMessage("An IO Exception occured while closing the connection: " + ex2);
    }// end of the catch block
   }// end of close client method
/*=============================================================================================
/                              protected void finalize()
/=============================================================================================*/
   protected void finalize(){
    try{
       closeClient();
       // call the finalize method of the Object Super Class
       super.finalize();
    }// end of the try block
    catch(Throwable t){
       logErrorMessage("An Exception occured while closing the connection: " + t);
    }// end of the catch block
  }// end of the finalize Method
/*=============================================================================================
/                              public ClientDescription getClient()
/    This method returns a reference to the Client Description class
/=============================================================================================*/
 public ClientDescription getClient(){
   return this;
 }
/*=============================================================================================
/                     StartListening Inner Class of the ClientDescription Class
/     This Inner class listens for a request from this client and sends back the stock response
/=============================================================================================*/
  public class StartListening implements Runnable{
  private Thread myThread;
  public void run(){
       listen();
   }
  public void start(){
   myThread = new Thread(this);
   myThread.start();
   }
  }// End of the startListening Inner Class of the ClientDescription Inner Class
/*=============================================================================================
/                           End of the ClientDescription Inner Class
/=============================================================================================*/
}// end of the ClientDescription Inner Class
/*=============================================================================================
/                      SimulationThread Inner Class
/=============================================================================================*/
   public class SimulationThread implements Runnable{
   private Thread  myThread;
   private boolean isRunning = false;
   private long    pollingPeriod = 500;
/*=============================================================================================
/                      SimulationThread Constructor
/=============================================================================================*/
   public SimulationThread(boolean start){
     isRunning = start;
   }
   public void setRunning(boolean newRunning){
     isRunning = newRunning;
   }
/*=============================================================================================
/          run()
/=============================================================================================*/
   public void run(){
    while(isRunning){
      try{
            simulationUpdate();
            Thread.currentThread().sleep(pollingPeriod);
      }
      catch (Exception e) {
         System.out.println(e.toString());
      }
    }// End of the While Loop
   }
/*=============================================================================================
/      start()
/=============================================================================================*/
   public void start(){
    myThread = new Thread(this);
    myThread.start();
    }
   }//// End of the SimulationThread Inner Class
/*=============================================================================================
/      End of the ServerSocket Class
/=============================================================================================*/
}



