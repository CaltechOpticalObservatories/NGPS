/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor. 
 */ 
package edu.caltech.palomar.util.general;
//=== File Prolog =============================================================
//	This code was developed by California Institute of Technology
//      Caltech Optical Observatories, Palomar Observatory
//
//--- Contents ----------------------------------------------------------------
//	 TargetListModel 
//
//--- Description -------------------------------------------------------------
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
//
//      July 14, 2010 Jennifer Milburn
//        Original Implementation - derived from a similar UCLA FLITECAM class
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
//
//=== End File Prolog =========================================================
import java.awt.Color;
import javax.swing.text.StyleContext;
import javax.swing.text.Style;
import javax.swing.text.StyleConstants;
import javax.swing.SwingUtilities;
import java.util.Vector;
import java.awt.event.*;
/*=============================================================================================
/   CommandLog Document Model
/=============================================================================================*/
public class TargetListDocumentModel extends Object{
  private transient Vector actionListeners;
  private Style newTargetStyle;
  private Style newErrorStyle;
  public static int COMMAND  = 100;
  public static int RESPONSE = 200;
  public static int ERROR    = 300;
  public java.lang.String terminator                 = new java.lang.String("\n");
  public static int maximumLength = 10000; // The maximum number of lines in the log document
  public static int removeLength  = 1000; // The maximum number of lines in the log document
// Plain Documents used for holding the message log text that is to be displayed on the screen
  public javax.swing.text.DefaultStyledDocument MessageDocument    = new javax.swing.text.DefaultStyledDocument();
  public     static  int             UPDATE_SCROLLBAR_POSITION   = 100;
/*=============================================================================================
/  TargetListModel Document Model
/=============================================================================================*/
   public TargetListDocumentModel(){
       initializeStyle();
   }
/*================================================================================================
/             initialize Document Display Styles
/=================================================================================================*/
  public void initializeStyle(){
     StyleContext context = StyleContext.getDefaultStyleContext();
     Style def   = context.getStyle(StyleContext.DEFAULT_STYLE);
     StyleConstants.setFontFamily(def,"Ariel");
     StyleConstants.setFontSize(def,14);
     StyleConstants.setBold(def,true);
     Style normal = getDocument().addStyle("normal",def);
//     newResponseStyle   = context.getStyle(StyleContext.DEFAULT_STYLE);
     Style s = getDocument().addStyle("command", normal);
     StyleConstants.setForeground(s, Color.blue);
           s = getDocument().addStyle("response", normal);
     StyleConstants.setForeground(s, new java.awt.Color(0,100,0));
           s = getDocument().addStyle("error",normal);
     StyleConstants.setForeground(s, new java.awt.Color(100,0,0));
  }
/*================================================================================================
/        insertMessage()
/=================================================================================================*/
  public  void insertMessage(java.lang.String newMessage){
         RunTrimDocument myRunTrimDocument = new RunTrimDocument(this,newMessage);
         SwingUtilities.invokeLater(myRunTrimDocument);
   }
/*================================================================================================
/       insertMessage(int message_type,java.lang.String newMessage)
/=================================================================================================*/
  public  void insertMessage(int message_type,java.lang.String newMessage){
         RunTrimDocument myRunTrimDocument = new RunTrimDocument(this,newMessage,message_type);
         SwingUtilities.invokeLater(myRunTrimDocument);
   }
  public javax.swing.text.DefaultStyledDocument getDocument(){
    return MessageDocument;
  }
/*================================================================================================
/       getText()
/=================================================================================================*/
public java.lang.String getText(){
   java.lang.String text = new java.lang.String();
    try{
       text = MessageDocument.getText(0, MessageDocument.getLength());
   }catch(Exception e){
       System.out.println("Problem getting the text of the document");
   }
   return text;
 }
/*================================================================================================
/       clearDocument()
/=================================================================================================*/
  public void clearDocument(){
    try{
       MessageDocument.remove(0, MessageDocument.getLength());
    }catch(Exception e){
        System.out.println("error clearing the target list text document");
    }  
  }
/*=============================================================================================
/   ActionEvent Handling Code - used to informing registered listeners that a FITS
/      file is ready to be retrieved from the server
/  removeActionListener()
/  addActionListener()
/  fireActionPerformed()
/=============================================================================================*/
/*=============================================================================================
/     synchronized void removeActionListener(ActionListener l)
/=============================================================================================*/
  public synchronized void removeActionListener(ActionListener l) {
    if (actionListeners != null && actionListeners.contains(l)) {
      Vector v = (Vector) actionListeners.clone();
      v.removeElement(l);
      actionListeners = v;
    }
  }
/*=============================================================================================
/    synchronized void addActionListener(ActionListener l)
/=============================================================================================*/
  public synchronized void addActionListener(ActionListener l) {
    Vector v = actionListeners == null ? new Vector(2) : (Vector) actionListeners.clone();
    if (!v.contains(l)) {
      v.addElement(l);
      actionListeners = v;
    }
  }
/*=============================================================================================
/     protected void fireActionPerformed(ActionEvent e)
/=============================================================================================*/
  public void fireActionPerformed(ActionEvent e) {
    if (actionListeners != null) {
      Vector listeners = actionListeners;
      int count = listeners.size();
      for (int i = 0; i < count; i++) {
        ((ActionListener) listeners.elementAt(i)).actionPerformed(e);
      }
    }
  }
/*=============================================================================================
/                      RunTrimDocument Inner Class of the FTPCommandLogModel
/=============================================================================================*/
  public class RunTrimDocument implements Runnable{
  private Thread myThread;
  private java.lang.String newMessage = new java.lang.String();
  private TargetListDocumentModel parent;
  private int             default_message_type = COMMAND;
  private int             current_message_type;

  public RunTrimDocument(TargetListDocumentModel parent,java.lang.String nextMessage){
    this.parent = parent;
    current_message_type = default_message_type;
    newMessage = nextMessage;
  }
    public RunTrimDocument(TargetListDocumentModel parent,java.lang.String nextMessage,int message_type){
    this.parent = parent;
    current_message_type = message_type;
    newMessage = nextMessage;
  }
  public void run(){
    try{
//      FTPMessageDocument.getDefaultRootElement().getDocument().insertString(0,(newFTPMessage + terminator), newFTPStyle);
//      checkFTPDocumentLength();
     if(current_message_type == COMMAND){
       MessageDocument.insertString(MessageDocument.getLength(),(newMessage + terminator), getDocument().getStyle("command"));
     }
     if(current_message_type == RESPONSE){
       MessageDocument.insertString(MessageDocument.getLength(),(newMessage + terminator), getDocument().getStyle("response"));
     }
     if(current_message_type == ERROR){
      MessageDocument.insertString(MessageDocument.getLength(),(newMessage + terminator), getDocument().getStyle("error"));
     }
      parent.fireActionPerformed(new java.awt.event.ActionEvent(parent,UPDATE_SCROLLBAR_POSITION,"Update ScrollBar Position"));
    }catch(javax.swing.text.BadLocationException bl){
        System.out.print("TargetListDocumentModel - Error inserting Message Line 175");
     }
   }
  public void start(){
   myThread = new Thread(this);
   myThread.start();
   }
  }//// End of the RunTrimDocument Inner Class
/*=============================================================================================
/                           End of the RunTrimDocument Inner Class
/=============================================================================================*/
/*=============================================================================================
/     End of the FTPCommandLogModel Class
/=============================================================================================*/
}
