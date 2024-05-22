/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package edu.caltech.palomar.instruments.ngps.os;

import edu.caltech.palomar.io.ClientSocket;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.BufferedInputStream;
import java.net.Socket;
import java.io.InputStream;
import java.io.OutputStream;
/**
 *
 * @author developer
 */
public class ConnectionTester {
    public ClientSocket                 myCommandSocket    = new ClientSocket(); 
    public ClientSocket                 myBlockingSocket   = new ClientSocket();
    private static java.lang.String     TERMINATOR         ="\n";
    public  IniReader                   myIniReader = new IniReader();
    public Socket                       serverSocket;
    public  java.io.BufferedReader      myBufferedReader;
    public  java.io.BufferedWriter      myBufferedWriter;
    public  java.io.BufferedInputStream myBufferedInputStream;
/*================================================================================================
/      executeCommand(int socket,java.lang.String command,double delay)
/=================================================================================================*/
  public  ConnectionTester(){
     initialize();
     boolean conduct_test = true; 
     if(conduct_test){
//         daves_code("state");
       while(conduct_test){
        java.lang.String response =  simple_code("state");
        System.out.println(response);
//          test();
       }
     }    
  }
/*================================================================================================
/      executeCommand(int socket,java.lang.String command,double delay)
/=================================================================================================*/
 public void test(){
     System.out.println("Server Name = "+myIniReader.SERVERNAME+" Port Number = "+myIniReader.BLOCKING_SERVERPORT);
     java.lang.String command = "state";
     java.lang.String response  = all_in_one(command+TERMINATOR);
     java.lang.String response2 = executeCommandClientSocket(command,100);
     java.lang.String response3 = executeCommandLocal(command,100);
     System.out.println("Command = "+command);
     System.out.println("All in One Version = "+response);
     System.out.println("Client Socket Version = "+response2);
     System.out.println("Local Socket Version = "+response3);
     java.lang.String response4 = simple_code(command);
     System.out.println("Dave's Version = "+response4);
}
 /*================================================================================================
/      executeCommand(int socket,java.lang.String command,double delay)
/=================================================================================================*/
 public void initialize(){
    myIniReader = new IniReader();  
    myCommandSocket = new ClientSocket(myIniReader.SERVERNAME,myIniReader.BLOCKING_SERVERPORT);
 } 
/*================================================================================================
/      executeCommand(int socket,java.lang.String command,double delay)
/=================================================================================================*/
  public java.lang.String executeCommandClientSocket(java.lang.String command,double delay){
      java.lang.String response = new java.lang.String();
         myCommandSocket.startConnection(ClientSocket.USE_HOSTNAME);
           if(myCommandSocket.isConnected()){
              response = myCommandSocket.sendReceiveCommandARCHON(command+TERMINATOR);
            }
         myCommandSocket.closeConnection();
    return response;
  } 
/*================================================================================================
/      executeCommand(int socket,java.lang.String command,double delay)
/=================================================================================================*/
  public java.lang.String executeCommandLocal(java.lang.String command,double delay){
      java.lang.String response = new java.lang.String();
         boolean state = startConnection();
           if(state){
              response = sendReceiveCommand(command+TERMINATOR);
            }
         closeConnection();
    return response;
  } 
 /*================================================================================================
/      executeCommand(int socket,java.lang.String command,double delay)
/=================================================================================================*/
 public boolean startConnection(){
    boolean state = false;
    try{
         serverSocket = new java.net.Socket(myIniReader.SERVERNAME,myIniReader.COMMAND_SERVERPORT);
     if(serverSocket != null){
         state = true;
         myBufferedReader      = new BufferedReader( new InputStreamReader(serverSocket.getInputStream()));
         myBufferedInputStream = new BufferedInputStream( serverSocket.getInputStream());
         myBufferedWriter      = new BufferedWriter( new OutputStreamWriter(serverSocket.getOutputStream()));
     }
    }catch(Exception ex1){
       System.out.println(ex1.toString());
    } 
  return state;
  }
/*=============================================================================================
/     sendRecieveCommand(java.lang.String newCommand)
/=============================================================================================*/
  public java.lang.String sendReceiveCommand(java.lang.String newCommand){
    int BUFFER_SIZE = 4096;
    int REPEAT_DELAY = 10;
       java.lang.String myResponse = new java.lang.String();
       if(myBufferedWriter != null){
         try{
           myBufferedWriter.write(newCommand);
           myBufferedWriter.flush();
          }catch(Exception  ex1){
              System.out.println(ex1.toString());
          }
       }
       if(myBufferedReader != null){
          try{
           if(!myBufferedReader.ready()) {
               int count = 0;
               while((!myBufferedReader.ready()) && (count < REPEAT_DELAY)){
                 Thread.currentThread().sleep(1);
                 count = count + 1;
               }// end of While !myBufferedReader.ready()
               if(!myBufferedReader.ready()){
                  System.out.println("Connection timed out without response. 6");
                  return myResponse;
              }// end of logging error message if the response is still not here
           }// end of if !myBufferedReader.ready()
        if(myBufferedReader.ready()){
         char[] myChar = new char[BUFFER_SIZE];
         int index = 0;
         boolean state = myBufferedReader.ready();
         while(state){
             myChar[index] = (char)myBufferedReader.read();
             if(myChar[index] == '\n'){
                state = false; 
             }
           if (index >= BUFFER_SIZE){
               state = false;
           }
           index = index + 1;                 
             }
      myResponse = new java.lang.String(myChar);
   }// end of if myBufferedReader.ready()
   }catch(Exception  ex2){
      System.out.println(ex2.toString());
   } 
   }
  return myResponse;
  }// end of the sendCommand method 
/*=============================================================================================
/   closeConnection() method
/=============================================================================================*/
  public void closeConnection(){
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
    }
    catch(Exception  ex2){
       System.out.println(ex2.toString());
    }
  }
/*=============================================================================================
/   main() method
/=============================================================================================*/
public java.lang.String all_in_one(java.lang.String command){
    boolean state = false;
    try{
         serverSocket = new java.net.Socket(myIniReader.SERVERNAME,myIniReader.BLOCKING_SERVERPORT);
     if(serverSocket != null){
         state = true;
         myBufferedReader      = new BufferedReader( new InputStreamReader(serverSocket.getInputStream()));
         myBufferedInputStream = new BufferedInputStream( serverSocket.getInputStream());
         myBufferedWriter      = new BufferedWriter( new OutputStreamWriter(serverSocket.getOutputStream()));
     }
    }catch(Exception ex1){
       System.out.println(ex1.toString());
    } 
    int BUFFER_SIZE = 4096;
    int REPEAT_DELAY = 10;
       java.lang.String myResponse = new java.lang.String();
       if(myBufferedWriter != null){
         try{
           myBufferedWriter.write(command);
           myBufferedWriter.flush();
          }catch(Exception  ex1){
              System.out.println(ex1.toString());
          }
       }
       if(myBufferedReader != null){
          try{
           if(!myBufferedReader.ready()) {
               int count = 0;
               while((!myBufferedReader.ready()) && (count < REPEAT_DELAY)){
                 Thread.currentThread().sleep(1);
                 count = count + 1;
               }// end of While !myBufferedReader.ready()
               if(!myBufferedReader.ready()){
                  System.out.println("Connection timed out without response. 6");
                  return myResponse;
              }// end of logging error message if the response is still not here
           }// end of if !myBufferedReader.ready()
        if(myBufferedReader.ready()){
         char[] myChar = new char[BUFFER_SIZE];
         int index = 0;
         boolean status = myBufferedReader.ready();
         while(status){
             myChar[index] = (char)myBufferedReader.read();
             if(myChar[index] == '\n'){
                status = false; 
             }
           if (index >= BUFFER_SIZE){
               status = false;
           }
           index = index + 1;                 
             }
      myResponse = new java.lang.String(myChar);
   }// end of if myBufferedReader.ready()
   }catch(Exception  ex2){
      System.out.println(ex2.toString());
   } 
   }
   return myResponse;
} 
/*=============================================================================================
/   Dave's Example
/=============================================================================================*/
public java.lang.String simple_code(java.lang.String command){
 //   String command = "";
   OutputStream       outputStream;
   OutputStreamWriter outputStreamWriter;
   InputStream        inputStream;
   InputStreamReader  inputStreamReader;
   String host    = myIniReader.SERVERNAME;
   int port       = myIniReader.COMMAND_SERVERPORT;
   char term      = '\n';
    // build the command from the command-line args
    //
 //   for ( String val : args ) command += ( val + " " );
 
    command.trim();    // strip the last space
    command += term;   // add the terminating character
    java.lang.String response = new java.lang.String();
    try {
      // connect to the host socket
      //
      Socket socket = new Socket( host, port );
 
      outputStream       = socket.getOutputStream();
      outputStreamWriter = new OutputStreamWriter( outputStream );
 
      outputStreamWriter.write(command);  // send command
      outputStreamWriter.flush();
 
      inputStream         = socket.getInputStream();
      inputStreamReader   = new InputStreamReader( inputStream );
      StringBuilder reply = new StringBuilder();
 
      // read reply until terminating character received from host
      //
      int chin;
      while ( ( chin = inputStreamReader.read() ) != term ) {
        reply.append( (char)chin );
      }
      response = reply.toString();
 
      System.out.println( reply );   // print the reply
      socket.close();                // close the connection
    }
    catch ( Exception exception ) {
      System.out.println( exception );
    }
    return response;
  }  
/*=============================================================================================
/   main() method
/=============================================================================================*/
      public static void main(String args[]) {
        /* Set the Nimbus look and feel */
        /* Create and display the form */
        java.awt.EventQueue.invokeLater(new Runnable() {
            public void run() {
                new ConnectionTester();
            }
        });
    }
 }
  