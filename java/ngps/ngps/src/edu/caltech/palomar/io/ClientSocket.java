package edu.caltech.palomar.io;  
  
//=== File Prolog =============================================================
//	This code was developed by California Institute of Technology
//      Caltech Optical Observatories, Palomar Observatory
//
//--- Contents ----------------------------------------------------------------
//	 TelescopeObject- This is the object model for a basic telescope
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

import java.net.*;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.net.ConnectException;
import java.io.IOException;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.BufferedInputStream;
import java.lang.InterruptedException;
import java.beans.*;
import nom.tam.fits.Fits;
import java.io.File;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.DataOutputStream;
import java.io.DataInputStream;
import java.io.FileOutputStream;
import edu.caltech.palomar.util.general.CommandLogModel;
import edu.caltech.palomar.util.general.OutputFile;
/*=============================================================================================
/      Class Definition  :   public class ClientSocket
/=============================================================================================*/
public class ClientSocket extends java.lang.Object{
   private java.lang.String serverName               = new java.lang.String();
   private java.lang.String defaultServerName        = new java.lang.String();
   private java.lang.String       TERMINATOR         = new java.lang.String("\n");
   public static java.lang.String DEFAULT_SERVER     = "localhost";
   public static int              DEFAULT_SERVERPORT = 3012;
   public static int              USE_INETADDRESS    = 1;
   public static int              USE_HOSTNAME       = 2;
   public int                     READ_DELAY         = 50;
   public int                     REPEAT_DELAY       = 10;
   public long                    SECOND_TO_MILLI_CONVERSION    = 1000;
      // Thread for Sending a Command to the Server
   public SendCommandThread mySendCommandThread      = new SendCommandThread();
      //  Socket and associated Buffered Readers and Writers
   public  java.net.Socket serverSocket;
   public  java.io.BufferedReader   myBufferedReader;
   public  java.io.BufferedWriter   myBufferedWriter;
   public  java.io.BufferedInputStream myBufferedInputStream;
   private java.io.DataOutputStream myDataOutputStream;
   private java.io.DataInputStream  myDataInputStream;
     // global scope objects that are not instantiated until needed
   private InetAddress serverInetAddress;
   private CommandLogModel errorCommandLogModel   = new CommandLogModel();
   private CommandLogModel sendCommandLogModel    = new CommandLogModel();
   private CommandLogModel receiveCommandLogModel = new CommandLogModel();
   private int serverPort;
   private boolean isLogging;
   private transient PropertyChangeSupport propertyChangeListeners = new PropertyChangeSupport(this);
   private boolean connected;
   public  java.lang.String fitsFileName    = new java.lang.String();
   public  boolean          sendFitsFile;
   public  boolean          simulator;
   public   java.lang.String USERDIR           = System.getProperty("user.dir");
   public   java.lang.String SEP               = System.getProperty("file.separator");
   private OutputFile        _outputFile;
   int     logFileSize       = 10000;
   int     logFileNumber     = 1;
/*=============================================================================================
/                                Constructors
/   public ClientSocket()
/   public ClientSocket(java.lang.String newServerName,int newServerPort)
/   public ClientSocket(java.net.InetAddress newInetAddress,int newServerPort)
/=============================================================================================*/
  public ClientSocket() {
     setDefaultServerName(DEFAULT_SERVER);
     setDefaultServerInetAddress();
     setServerPort(DEFAULT_SERVERPORT);
     setConnected(false);
     setLogging(false);
     setSimulator(true);
     setSendFitsFile(false);
  }
  public ClientSocket(java.lang.String newServerName,int newServerPort) {
     setServerName(newServerName);
     setServerInetAddress(newServerName);
     setServerPort(newServerPort);
     setConnected(false);
     setLogging(false);
     setSimulator(true);
     setSendFitsFile(false);
  }
  public ClientSocket(java.net.InetAddress newInetAddress,int newServerPort) {
     findServerName(newInetAddress);
     setServerInetAddress(newInetAddress);
     setServerPort(newServerPort);
     setConnected(false);
     setLogging(false);
     setSimulator(true);
     setSendFitsFile(false);
  }
/*=============================================================================================
/  void constructLogFile(java.lang.String logFileName)
/=============================================================================================*/
  public void constructLogFile(java.lang.String logFileName){
    java.lang.String directoryPath = new java.lang.String();
    directoryPath = USERDIR + SEP + "Logs" + SEP;
    _outputFile = new OutputFile(directoryPath,logFileName,logFileSize,logFileNumber);
//    setLogging(true);
  }
/*=============================================================================================
/  void constructLogFile(java.lang.String logFileName)
/=============================================================================================*/
    public void writeToLogFile(java.lang.String message){
      if(isLogging()){
         WriteToLogFileThread myWriteToLogFileThread = new WriteToLogFileThread(message);
         myWriteToLogFileThread.start();
      }
    }
/*=============================================================================================
/  java.net.Socket getServerSocket()
/=============================================================================================*/
  public java.net.Socket getServerSocket(){
    return serverSocket;
  }
/*=============================================================================================
/  boolean isDataAvailable()
/=============================================================================================*/
  public boolean isDataAvailable(){
    java.lang.String myResponse = new java.lang.String();
    boolean          dataReady  = false;
    if(isConnected()){
           if(myBufferedReader != null){
             try{
              dataReady = myBufferedReader.ready();
              }// end of the try block
             catch(IOException  ex2){
                  logErrorMessage("An IOException occured while checking the socket: " + ex2);
                  setConnected(false);
                  if(serverSocket == null){
                      logErrorMessage("The connection to the server has been lost. ");
                  }
             }// end of catching the IOException
          }// end of checking the validity of the BufferedReader
    }// end of checking to make sure the socket has connected to the server
  return dataReady;
  }
/*=============================================================================================
/   testConnection()
/=============================================================================================*/
  public boolean testConnection(){
      setConnected(false);
      try{
   // First we create a new socket and attach the input and output streams
           java.net.Socket testServerSocket = new java.net.Socket(getServerName() ,getServerPort());
           Thread.currentThread().yield();
     if(testServerSocket != null){
         setConnected(true);
     }// a probably unecessary check to make sure the socket was actually created
     if(testServerSocket == null){
         setConnected(false);
     }// a probably unecessary check to make sure the socket was actually created
   }
    catch(UnknownHostException ex1){
       logErrorMessage("Host not found: " + getServerInetAddress().toString() + " " + ex1);
        System.out.print("Client Socket Line 177" + "Host not found: " + getServerInetAddress().toString() + " " + ex1 +"\n");
      setConnected(false);
    }
    catch(IOException  ex2){
       logErrorMessage("An IO Exception occured: " + ex2);
       System.out.print("Client Socket Line 181" + "An IO Exception occured: " + ex2 + "\n");
       setConnected(false);
    }
  return isConnected();
    }
/*=============================================================================================
/   startConnection() method
/=============================================================================================*/
  public boolean startConnection(int connectionMethod){
      try{ closeConnection(); }
      catch (Exception e) {;}
      
   try{
   // First we create a new socket and attach the input and output streams
       if(connectionMethod == USE_INETADDRESS){
           serverSocket = new java.net.Socket(getServerInetAddress(),getServerPort());
        }// if we are going to connect using the InetAddress
       if(connectionMethod == USE_HOSTNAME){
           Thread.currentThread().yield();
           serverSocket = new java.net.Socket(getServerName() ,getServerPort());
        }// if we are going to connect using the InetAddress
     if(serverSocket != null){
         // Now we need to create the Readers and Writers to the Socket if everything worked
         myBufferedReader      = new BufferedReader( new InputStreamReader(serverSocket.getInputStream()));
         myBufferedInputStream = new BufferedInputStream( serverSocket.getInputStream());
         myBufferedWriter      = new BufferedWriter( new OutputStreamWriter(serverSocket.getOutputStream()));
     }// a probably unecessary check to make sure the socket was actually created
    // If we've made it this far then we can set the connection equal to true
    setConnected(true);
   }
    catch(UnknownHostException ex1){
       logErrorMessage("Host not found: " + getServerInetAddress().toString() + " " + ex1);
       System.out.print("Client Socket Line 177" + "Host not found: " + getServerInetAddress().toString() + " " + ex1 +"\n");
    }
    catch(IOException  ex2){
       logErrorMessage("An IO Exception occured: " + ex2);
       System.out.print("Client Socket Line 181" + "An IO Exception occured: " + ex2 + "\n");
    }
  return isConnected();
  }
/*=============================================================================================
/  reinitializeBufferedReader()
/=============================================================================================*/
public void reinitializeBufferedReader(){
      if(serverSocket != null){
        try{
           myBufferedReader      = new BufferedReader( new InputStreamReader(serverSocket.getInputStream()));
           myBufferedInputStream = new BufferedInputStream( serverSocket.getInputStream());            
           setConnected(true);
        }catch(Exception e){
           setConnected(false);
           logErrorMessage("An IO Exception occured: " + e.toString());           
        }  
     }// a probably unecessary check to make sure the socket was actually created   
 }
/*=============================================================================================
/   closeConnection() method
/=============================================================================================*/
  public boolean closeConnection(){
   try{
        if(myBufferedReader != null){
            myBufferedReader.close();
          }
        if(myBufferedWriter != null){
            myBufferedWriter.close();
         }
        if(myBufferedInputStream != null){
            myBufferedInputStream.close();
        }
        if(serverSocket != null){
          serverSocket.close();
        }
     setConnected(false);
    }
    catch(IOException  ex2){
       logErrorMessage("An IO Exception occured while closing the connection: " + ex2);
    }
  return isConnected();
 }
/*=============================================================================================
/   closeConnection() method
/=============================================================================================*/
  public boolean forcecloseConnection(){
   try{
         if(serverSocket != null){
          serverSocket.shutdownInput();
          serverSocket.shutdownOutput();
          serverSocket.close();
        }
     setConnected(false);
    }
    catch(IOException  ex2){
       logErrorMessage("An IO Exception occured while closing the connection: " + ex2);
    }
  return isConnected();
 }
/*=============================================================================================
/   sendNewLine()
/=============================================================================================*/
 public void sendNewLine(){
    if(isConnected()){
        if(serverSocket != null){
          try{  
             myBufferedWriter.write(TERMINATOR);
             myBufferedWriter.flush();
          }catch(java.io.IOException eio){
                System.out.println("error sending new line to the socket");
          }
    }// a probably unecessary check to make sure the socket was actually created
    }   
 }
/*=============================================================================================
/   finalize() method - closes all the streams and the socket if not already accomplished
/=============================================================================================*/
 protected void finalize(){
  try{
    closeConnection();
    // call the finalize method of the Object Super Class
    super.finalize();
  }
  catch(Throwable t){
       logErrorMessage("An Exception occured while closing the connection: " + t);
    }
 }
/*=============================================================================================
/     sendCommand Method - This method is used to send a string to the server
/                          checks to see if the connection is active before sending
/=============================================================================================*/
   public synchronized java.lang.String getNextResponse(){
     java.lang.String myResponse = new java.lang.String();
     if(isConnected()){
       // Only try to read from the socket if a response is expected to this command
           if(myBufferedReader != null){
              try{
                // Some consideration must be given to whether this will block forever
                // or if it will time-out and how to handle a lack of response.  This
                // consideration has not been implemented yet.
             // Wait for the BufferReader to have data.  If the stream doesn't have data
             // in it yet then wait for the READ_DELAY period and check again.  If the
             // stream still lacks data then repeat the check for REPEAT_DELAY times and then
             // report that the connection timed out.
//  DEALING WITH THE PROBLEM OF THE RESPONSE FROM THE SERVER NOT BEING READY
            if(!myBufferedReader.ready()) {
                int count = 0;
                while((!myBufferedReader.ready()) && (count < REPEAT_DELAY)){
                  Thread.currentThread().sleep(READ_DELAY);
                  count = count + 1;
                }// end of While !myBufferedReader.ready()
                if(!myBufferedReader.ready()){
                   logErrorMessage("Connection timed out without response. 1");
                   return myResponse;
                }// end of logging error message if the response is still not here
            }// end of if !myBufferedReader.ready()
// ACTUALLY READING FROM THE SOCKET
               if(myBufferedReader.ready()){
                   myResponse = myBufferedReader.readLine();
                   writeToLogFile(myResponse);
               }// end of if myBufferedReader.ready()
                if(isLogging()){
                      logReceiveMessage(myResponse);
                 }// if isLogging is true
               }// end of the try block
              catch(IOException  ex2){
                   logErrorMessage("An IOException occured while reading from the socket: " + ex2);
                   setConnected(false);
                   if(serverSocket == null){
                       logErrorMessage("The connection to the server has been lost. ");
                   }
              }// end of catching the IOException
              catch(java.lang.InterruptedException ex3){
                    logErrorMessage("An error occurred while waiting to read from the server: "+ex3);
              }// end of the catch
           }// end of checking the validity of the BufferedReader
     }// end of checking to make sure the socket has connected to the server
   return myResponse;
   }// end of the sendCommand method
/*=============================================================================================
/     sendCommand Method - This method is used to send a string to the server
/                          checks to see if the connection is active before sending
/=============================================================================================*/
  public synchronized java.lang.String sendCommand(java.lang.String newCommand,boolean responseExpected){
    java.lang.String myResponse = new java.lang.String();
    if(isConnected()){
       if(myBufferedWriter != null){
         try{
           myBufferedWriter.write(newCommand);
           myBufferedWriter.flush();
           if(isLogging()){
                 logSendMessage(newCommand);
            }// if isLogging is true
          }// end of the try block
         catch(IOException  ex1){
              logErrorMessage("An IOException occured while writing to the socket: " + ex1);
              setConnected(false);
              if(serverSocket == null){
                  logErrorMessage("The connection to the server has been lost. ");
              }// is the socket has gone null then flag that the connection is lost
         }// end of catching the IOException
       }// end of checking to make sure the writer has been initialized
      // Only try to read from the socket if a response is expected to this command
      if(responseExpected){
          if(myBufferedReader != null){
             try{
               // Some consideration must be given to whether this will block forever
               // or if it will time-out and how to handle a lack of response.  This
               // consideration has not been implemented yet.
            // Wait for the BufferReader to have data.  If the stream doesn't have data
            // in it yet then wait for the READ_DELAY period and check again.  If the
            // stream still lacks data then repeat the check for REPEAT_DELAY times and then
            // report that the connection timed out.
//  DEALING WITH THE PROBLEM OF THE RESPONSE FROM THE SERVER NOT BEING READY
           if(!myBufferedReader.ready()) {
               int count = 0;
               while((!myBufferedReader.ready()) && (count < REPEAT_DELAY)){
                 Thread.currentThread().sleep(READ_DELAY);
                 count = count + 1;
               }// end of While !myBufferedReader.ready()
               if(!myBufferedReader.ready()){
                  logErrorMessage("Connection timed out without response. 2");
                  return myResponse;
              }// end of logging error message if the response is still not here
           }// end of if !myBufferedReader.ready()
// ACTUALLY READING FROM THE SOCKET
           if(myBufferedReader.ready()){
                  if(myBufferedReader.ready()){
//                      System.out.println("reading from socket");
                      myResponse = myBufferedReader.readLine();
//                      System.out.println("finished reading from socket");
                 }
             }// end of if myBufferedReader.ready()
               if(isLogging()){
                     logReceiveMessage(myResponse);
                }// if isLogging is true
              }// end of the try block
             catch(IOException  ex2){
                  logErrorMessage("An IOException occured while reading from the socket: " + ex2);
                  setConnected(false);
                  if(serverSocket == null){
                      logErrorMessage("The connection to the server has been lost. ");
                  }
             }// end of catching the IOException
             catch(java.lang.Exception ex3){
                   logErrorMessage("An error occurred while waiting to read from the server: "+ex3);
             }// end of the catch
          }// end of checking the validity of the BufferedReader
     }// end of if responseExpected
    }// end of checking to make sure the socket has connected to the server
  return myResponse;
  }// end of the sendCommand method
/*=============================================================================================
/     sendRecieveCommand(java.lang.String newCommand)
/=============================================================================================*/
  public java.lang.String sendReceiveCommand(java.lang.String newCommand){
      double expectedDelay = 0.0;
     java.lang.String response = sendReceiveCommand(newCommand,expectedDelay);
     return response;
  }
/*================================================================================================
/      wait(double newDelay)
/=================================================================================================*/
public void wait(double newDelay){
     // Seconds are passed into the method and then converted to milliseconds
     long waitTime = (long)newDelay*SECOND_TO_MILLI_CONVERSION;
     try{
        Thread.currentThread().sleep(waitTime);} // sleep for awhile to let the messages pile up
      catch (Exception e){
    }
}
/*================================================================================================
/      waitTest(double newDelay)
/=================================================================================================*/
public void waitTest(double newDelay){
     // Seconds are passed into the method and then converted to milliseconds
    long DWELL_TIME = 100;
    long waitTime = (long)newDelay*SECOND_TO_MILLI_CONVERSION;
    int NUM_DWELL = (int)(waitTime/DWELL_TIME);
     try{
        for(int i=0;i < NUM_DWELL;i++) {
             if(myBufferedReader != null){
                 if(myBufferedReader.ready()){
                     i=NUM_DWELL;
                 }
           }
           Thread.currentThread().sleep(DWELL_TIME);
         }
        } // sleep for awhile to let the messages pile up
      catch (Exception e){
    }
}
/*=============================================================================================
/     sendRecieveCommand(java.lang.String newCommand)
/=============================================================================================*/
  public java.lang.String sendReceiveCommand(java.lang.String newCommand,double expectedDelay){
    java.lang.String myResponse = new java.lang.String();
    if(isConnected()){
       if(myBufferedWriter != null){
         try{
           myBufferedWriter.write(newCommand);
           myBufferedWriter.flush();
           writeToLogFile(newCommand);
           if(isLogging()){
                 logSendMessage(newCommand);
            }// if isLogging is true
          }// end of the try block
         catch(IOException  ex1){
              logErrorMessage("An IOException occured while writing to the socket: " + ex1);
              setConnected(false);
              if(serverSocket == null){
                  logErrorMessage("The connection to the server has been lost. ");
              }// is the socket has gone null then flag that the connection is lost
         }// end of catching the IOException
       }// end of checking to make sure the writer has been initialized
      // Only try to read from the socket if a response is expected to this command
          if(myBufferedReader != null){
          waitTest(expectedDelay);
          try{
//  DEALING WITH THE PROBLEM OF THE RESPONSE FROM THE SERVER NOT BEING READY
           if(!myBufferedReader.ready()) {
               int count = 0;
               while((!myBufferedReader.ready()) && (count < REPEAT_DELAY)){
                 Thread.currentThread().sleep(READ_DELAY);
                 count = count + 1;
               }// end of While !myBufferedReader.ready()
               if(!myBufferedReader.ready()){
                  logErrorMessage("Connection timed out without response. 3");
                  System.out.println("Connection timed out without response. 3");
                  return myResponse;
              }// end of logging error message if the response is still not here
           }// end of if !myBufferedReader.ready()
// ACTUALLY READING FROM THE SOCKET
           if(myBufferedReader.ready()){
             char[] myChar = new char[512];
             int index = 0;
             boolean state = myBufferedReader.ready();
             while(state){
                 myChar[index] = (char)myBufferedReader.read();
                 state         = myBufferedReader.ready();
               if (index >= 511){
                   state = false;
               }
               index = index + 1;
             }
             myResponse = new java.lang.String(myChar);
             writeToLogFile(myResponse);
             }// end of if myBufferedReader.ready()
               if(isLogging()){
                     logReceiveMessage(myResponse);
                }// if isLogging is true
              }// end of the try block
             catch(IOException  ex2){
                  logErrorMessage("An IOException occured while reading from the socket: " + ex2);
                  setConnected(false);
                  if(serverSocket == null){
                      logErrorMessage("The connection to the server has been lost. ");
                  }
             }// end of catching the IOException
             catch(java.lang.Exception ex3){
                   logErrorMessage("An error occurred while waiting to read from the server: "+ex3);
             }// end of the catch
          }// end of checking the validity of the BufferedReader
     }// end of checking to make sure the socket has connected to the server
  return myResponse;
  }// end of the sendCommand method
/*=============================================================================================
/     sendRecieveCommand(java.lang.String newCommand)
/=============================================================================================*/
  public java.lang.String sendReceiveCommandSequencer(java.lang.String newCommand){
      
      logErrorMessage(newCommand);
      
    String myResponse = null;
    int old_REPEAT_DELAY = REPEAT_DELAY;
    REPEAT_DELAY = 20;
//    if(isConnected()){
       if(myBufferedWriter != null){
         try{
           myBufferedWriter.write(newCommand);
           myBufferedWriter.flush();
           writeToLogFile(newCommand);
           if(isLogging()){
                 logSendMessage(newCommand);
            }// if isLogging is true
          }// end of the try block
         catch(IOException  ex1){
              logErrorMessage("An IOException occured while writing to the socket: " + ex1);
              setConnected(false);
              if(serverSocket == null){
                  logErrorMessage("The connection to the server has been lost. ");
              }// is the socket has gone null then flag that the connection is lost
         }// end of catching the IOException
       }// end of checking to make sure the writer has been initialized
      // Only try to read from the socket if a response is expected to this command
          if(myBufferedReader != null){
          try{
//  DEALING WITH THE PROBLEM OF THE RESPONSE FROM THE SERVER NOT BEING READY
           if(!myBufferedReader.ready()) {
               int count = 0;
               while((!myBufferedReader.ready()) && (count < REPEAT_DELAY)){
                 Thread.currentThread().sleep(100);
                 count = count + 1;
               }// end of While !myBufferedReader.ready()
               System.out.println("TOTAL READ TRIES: "+String.valueOf(count));
               logErrorMessage("TOTAL READ TRIES: "+String.valueOf(count));
               if(!myBufferedReader.ready()){
                  String msg = "Connection timed out without response. 6";
                  logErrorMessage(msg);
                  System.out.println(msg);
               //   return myResponse;
              }// end of logging error message if the response is still not here
           }// end of if !myBufferedReader.ready()
// ACTUALLY READING FROM THE SOCKET

        if(myBufferedReader.ready()){
            
            System.out.println("READER READY");
             
             myResponse = myBufferedReader.readLine();
             // System.out.println("RESPONSE: "+myResponse);
             } else {System.out.println("READER DID NOT READ");}// end of if myBufferedReader.ready()

               if(isLogging()){
                     logReceiveMessage(myResponse);
                }// if isLogging is true
              }// end of the try block
             catch(IOException  ex2){
                  logErrorMessage("An IOException occured while reading from the socket: " + ex2);
                  setConnected(false);
                  if(serverSocket == null){
                      logErrorMessage("The connection to the server has been lost. ");
                  }
             }// end of catching the IOException
             catch(java.lang.Exception ex3){
                   logErrorMessage("An error occurred while waiting to read from the server: "+ex3);
             }// end of the catch
          }// end of checking the validity of the BufferedReader
//    }// end of if(isConnected)
    REPEAT_DELAY  = old_REPEAT_DELAY;
    
  if(myResponse == null){ setConnected(false); }
  else { setConnected(true); }
  return myResponse;
  }// end of the sendCommand method  
/*=============================================================================================
/                               Server InetAddress Methods
/     public void setDefaultServerInetAddress(
/     public void setServerInetAddress(java.lang.String newServerName)
/     public void setServerInetAddress(java.net.InetAddress newInetAddress)
/     public InetAddress getServerInetAddress()
/=============================================================================================*/
  public void setDefaultServerInetAddress(){
    try{
        serverInetAddress = InetAddress.getLocalHost();
        setServerInetAddress(serverInetAddress);
    }
    catch(UnknownHostException ex){
       logErrorMessage("Host not found: localhost");
    }
  }
  public void setServerInetAddress(java.lang.String newServerName){
    try{
        serverInetAddress = InetAddress.getByName(newServerName);
        setServerInetAddress(serverInetAddress);
    }
    catch(UnknownHostException ex){
       logErrorMessage("Host not found: " + newServerName);
    }
  }
  public void setServerInetAddress(java.net.InetAddress newInetAddress){
     java.net.InetAddress oldInetAddress  = serverInetAddress;
     serverInetAddress                    = newInetAddress;
     propertyChangeListeners.firePropertyChange("serverInetAddress", oldInetAddress, newInetAddress);
  }
  public InetAddress getServerInetAddress(){
     return serverInetAddress;
  }
/*=============================================================================================
/                            Server Name Related Methods
/     public void setDefaultServerName(java.lang.String newServerName)
/     public java.lang.String getDefaultServerName()
/     public void setServerName(java.lang.String newServerName)
/     public java.lang.String getServerName()
/     public java.lang.String findServerName(java.net.InetAddress newInetAddress)
/=============================================================================================*/
  public void setDefaultServerName(java.lang.String newServerName){
     defaultServerName = newServerName;
     setServerName(defaultServerName);
  }
  public java.lang.String getDefaultServerName(){
     return defaultServerName;
  }
  public void setServerName(java.lang.String newServerName){
     java.lang.String oldServerName = serverName;
     serverName = newServerName;
     propertyChangeListeners.firePropertyChange("serverName", new String(oldServerName), new String(newServerName));
  }
  public java.lang.String getServerName(){
     return serverName;
  }
  public java.lang.String findServerName(java.net.InetAddress newInetAddress){
         java.lang.String myServerName = newInetAddress.getHostName();
        if(myServerName == null){
               logErrorMessage("Host not found for InetAddress: " + newInetAddress.toString());
               myServerName = getDefaultServerName();
               setServerName(myServerName);
        } // end of the == null check
        else if (myServerName != null){
             setServerName(myServerName);
        }  // end of the != null check
    return  myServerName;
  }// end of the findServerName method
/*=============================================================================================
/                             Set and Get Server Port Number
/     public void setServerPort(int newServerPort)
/     public int  getServerPort()
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
/                             Simulator Controls Methods
/     public void setSimulator(boolean newSimulator)
/     public boolean isSimulator()
/     Used to configure the clientSocket as a DV simulator
/=============================================================================================*/
  public void setSimulator(boolean newSimulator){
      boolean oldSimulator = simulator;
      simulator = newSimulator;
      propertyChangeListeners.firePropertyChange("simulator", Boolean.valueOf(oldSimulator), Boolean.valueOf(newSimulator));
  }
  public boolean isSimulator(){
     return simulator;
  }
/*=============================================================================================
/                             Send Fits File Controls Methods
/     public void setSendFitsFile(boolean newSimulator)
/     public boolean isSendFitsFile()
/     Used to tell the MKIRSocketMediator that the ClientSocket should sent a FITS file to
/     the client DVServerSocket.
/=============================================================================================*/
  public void setSendFitsFile(boolean newSendFitsFile){
      boolean oldSendFitsFile = sendFitsFile;
      sendFitsFile = newSendFitsFile;
      propertyChangeListeners.firePropertyChange("sendFitsFile", Boolean.valueOf(oldSendFitsFile), Boolean.valueOf(newSendFitsFile));
  }
  public boolean isSendFitsFile(){
     return sendFitsFile;
  }
/*=============================================================================================
/                            Log Message Methods
/      public void logErrorMessage(java.lang.String newMessage)
/      public void logSendMessage(java.lang.String newMessage)
/      public void logReceiveMessage(java.lang.String newMessage)
/=============================================================================================*/
  public void logErrorMessage(java.lang.String newMessage){
      if(isLogging){
         if(errorCommandLogModel != null){
              errorCommandLogModel.insertMessage(newMessage);
              System.out.println(newMessage);
         }// checking to make sure the error actually exists
      }// check to make sure we are logging things
  }// end of the logErrorMessage
  public void logSendMessage(java.lang.String newMessage){
      if(isLogging){
         if(sendCommandLogModel != null){
              sendCommandLogModel.insertMessage(newMessage);
         }// checking to make sure the sendLogTextArea actually exists
      }// check to make sure we are logging things
  }// end of the logSendMessage

  public void logReceiveMessage(java.lang.String newMessage){
      if(isLogging){
         if(receiveCommandLogModel != null){
              receiveCommandLogModel.insertMessage(newMessage);
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
/                          Document Model Methods
/   getErrorLoggingDocument()
/   getSendLoggingDocument()
/   getReceiveLoggingDocument()
/=============================================================================================*/
  public javax.swing.text.DefaultStyledDocument getErrorLoggingDocument(){
      return errorCommandLogModel.getDocument();
  }
  public javax.swing.text.DefaultStyledDocument getSendLoggingDocument(){
      return sendCommandLogModel.getDocument();
  }
  public javax.swing.text.DefaultStyledDocument getReceiveLoggingDocument(){
      return receiveCommandLogModel.getDocument();
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
    propertyChangeListeners.firePropertyChange("connected", Boolean.valueOf(oldConnected), Boolean.valueOf(newConnected));
  }
  public boolean isConnected() {
    return connected;
  }
/*=============================================================================================
/                          Fits File Related Functions
/    The Fits file name
/    These method allow the name of the test fits file to be specified
/=============================================================================================*/
  public void setFitsFileName(java.lang.String newFitsFileName){
     fitsFileName = newFitsFileName;
  }
  public java.lang.String getFitsFileName(){
     return fitsFileName;
  }
/*=============================================================================================
/                       getSendCommandThread()
/=============================================================================================*/
  public SendCommandThread getSendCommandThread(){
     SendCommandThread newSendCommandThread = new SendCommandThread();
     return newSendCommandThread;
  }
/*=============================================================================================
/                     SendCommandThread Inner Class of the ClientSocket
/     This Inner class sends a command to the server socket.  Since the readLine method in
/     sendCommand can block will waiting for a response from the server you can call the start
/     method of this class to send the command on a separate thread.  You still need to wait
/     before you send the next command but this allows you to send the command and then go away
/     and do other things before you get the response.
/=============================================================================================*/
  public class SendCommandThread implements Runnable{
      private Thread myThread;
      private java.lang.String myCommand = new java.lang.String();
      private boolean isResponseExpected;
      private boolean mySendingCommand;
      private java.lang.String myResponse = new java.lang.String();
/*=============================================================================================
/                         run() Method
/=============================================================================================*/
  public void run(){
      setIsSendingCommand(true);
        myResponse = sendCommand(myCommand,isResponseExpected);
      setIsSendingCommand(false);
     }
/*=============================================================================================
/                          setCommandParameters
/=============================================================================================*/
  public synchronized void setCommandParameters(java.lang.String newCommand,boolean newResponseExpected){
     myCommand = newCommand;
     isResponseExpected = newResponseExpected;
  }
/*=============================================================================================
/                          SetIsSendingCommand methods
/=============================================================================================*/
  private synchronized void setIsSendingCommand(boolean newIsSendingCommand){
     mySendingCommand = newIsSendingCommand;
  }
  public synchronized boolean isSendingCommand(){
     return mySendingCommand;
  }
/*=============================================================================================
/                          GetResponse Method
/=============================================================================================*/
  public synchronized java.lang.String getResponse(){
     return myResponse;
  }
/*=============================================================================================
/                         start() Method
/=============================================================================================*/
  public void start(){
   myThread = new Thread(this);
   myThread.start();
   }
  }// End of the startListening Inner Class of the ClientDescription Inner Class
/*=============================================================================================
/                     INNER CLASS that writes to a log file
/=============================================================================================*/
    public class WriteToLogFileThread  implements Runnable{
    private Thread myThread;
    private java.lang.String message = new java.lang.String();
    public WriteToLogFileThread(java.lang.String newMessage){
      message = newMessage;
    }
    public void run(){
         _outputFile.PrintToFile(message);
     }
    public void start(){
     myThread = new Thread(this);
     myThread.start();
     }
    }// End of the WriteToLogFileThread Inner Class
/*=============================================================================================
/                           End of the ClientSocket Class
/=============================================================================================*/
}