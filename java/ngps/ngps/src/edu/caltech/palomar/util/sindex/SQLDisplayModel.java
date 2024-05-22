package edu.caltech.palomar.util.sindex; 
//=== File Prolog =============================================================
//	This code was developed by UCLA, Department of Physics and Astronomy,
//	for the Stratospheric Observatory for Infrared Astronomy (SOFIA) project.
//
//--- Contents ----------------------------------------------------------------
//	SQLDisplayModel   - A "Document Model" for inserting SQL statements into
//                          a document.
//
//--- Description -------------------------------------------------------------
//
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
//
//      06/10/04        J. Milburn
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
//
//=== End File Prolog =========================================================
import javax.swing.JPanel;
import javax.swing.SwingUtilities;
import java.beans.*;
import java.io.FileWriter;
import java.io.File;
import javax.swing.text.StyledDocument;
import java.text.DateFormat;
import javax.swing.JComponent;
import javax.swing.text.BadLocationException;
import java.lang.Short;
import java.util.GregorianCalendar;
import java.util.Calendar;
import java.util.SimpleTimeZone;
import java.util.TimeZone;
import javax.swing.text.StyleContext;
import javax.swing.text.Style;
import javax.swing.text.StyleConstants;
import java.lang.Math;
import javax.swing.JTextArea;
import java.util.StringTokenizer;
/*================================================================================================
/      SQLDisplayModel Class Declaration
/=================================================================================================*/
public class SQLDisplayModel  {
/*================================================================================================
/        SQLDisplayModel - variable and object declarations
/=================================================================================================*/
  public static int maximumLength = 100; // The maximum number of lines in the log document
  public static int removeLength  = 1; // The maximum number of lines in the log document
  public javax.swing.JTextArea myTextArea;
  private transient PropertyChangeSupport propertyChangeListeners = new PropertyChangeSupport(this);
// The following Files are used for logging the server records and the instrument data
  private Style newCommentStyle;
 // Text String that are passed to the XMLFileControlPanel
// Plain Documents used for holding the message log text that is to be displayed on the screen
  public javax.swing.text.DefaultStyledDocument commentMessageDocument  = new javax.swing.text.DefaultStyledDocument();

  public java.lang.String SEP                        = System.getProperty("file.separator");
  public java.lang.String USERDIR                    = System.getProperty("user.dir");
  public java.lang.String terminator                 = new java.lang.String("\n");
/*================================================================================================
/       SQLDisplayModel() parameterless constructor
/=================================================================================================*/
  public SQLDisplayModel() {
     jbInit();
  }
/*================================================================================================
/       jbInit() Initialization Method
/=================================================================================================*/
  private void jbInit(){
      initializeCommentStyle();
  }
/*================================================================================================
/        Property Change Listener Support Methods
/=================================================================================================*/
  public synchronized void removePropertyChangeListener(PropertyChangeListener l) {
    propertyChangeListeners.removePropertyChangeListener(l);
  }
  public synchronized void addPropertyChangeListener(PropertyChangeListener l) {
    propertyChangeListeners.addPropertyChangeListener(l);
  }
/*================================================================================================
/             initialize Document Display Styles
/=================================================================================================*/
  public void initializeCommentStyle(){
     StyleContext context = StyleContext.getDefaultStyleContext();
     newCommentStyle   = context.getStyle(StyleContext.DEFAULT_STYLE);
     StyleConstants.setFontFamily(newCommentStyle,"Ariel");
     StyleConstants.setFontSize(newCommentStyle,10);
     StyleConstants.setBold(newCommentStyle,true);
     getCommentDocument().addStyle("normal",newCommentStyle);
    }
/*================================================================================================
/            Methods for inserting messages into both the XML file and the TextDocument
/  public void insertCommentMessage(java.lang.String newCommentMessage){
/=================================================================================================*/
/*================================================================================================
/        insertCommentMessage()
/=================================================================================================*/
  public synchronized void insertCommentMessage(java.lang.String newCommentMessage){
       try{
//         commentMessageDocument.getDefaultRootElement().getDocument().insertString(0,(newCommentMessage + terminator),newCommentStyle);
         RunTrimDocument myRunTrimDocument = new RunTrimDocument(newCommentMessage);
         SwingUtilities.invokeLater(myRunTrimDocument);
//         checkCommentDocumentLength();
      }catch(Exception bl){
         System.out.print("Error while insert CommentDocumentModel Line " + bl.toString() + "\n");
      }
   }
/*================================================================================================
/    Methods for checking the length of Documents and deleting the end of the file
/  public void checkCommentDocumentLength(){
/=================================================================================================*/
/*================================================================================================
/        checkCommentDocumentLength() method
/=================================================================================================*/
  public synchronized void checkCommentDocumentLength(){
        int myLength = commentMessageDocument.getDefaultRootElement().getDocument().getLength();
        if(myLength > maximumLength){
          try{
//             commentMessageDocument.remove(myLength - removeLength,removeLength);
             commentMessageDocument.getDefaultRootElement().getDocument().remove(myLength - removeLength,removeLength);
          }catch(javax.swing.text.BadLocationException bl){
          }
      }// End of the If check
  }
/*================================================================================================
/                  getDocuments() methods
/  public javax.swing.text.DefaultStyledDocument getCommentDocument(){
/=================================================================================================*/
  public javax.swing.text.DefaultStyledDocument getCommentDocument(){
    return commentMessageDocument;
  }
  public void setTextArea(javax.swing.JTextArea newTextArea){
    myTextArea = newTextArea;
  }
/*================================================================================================
/       parseComments() - this method parses all of the comments in the document and
/                         adds them as FITS comments in the FITS header
/=================================================================================================*/
  public synchronized void parseComments(){
   java.lang.String endLine     = new java.lang.String("\n");
   StringTokenizer  tokenResponseString;
   java.lang.String currentLine = new java.lang.String();
   int commentLines             = 0;
   int commentLength            = 72;
   int lineCount = myTextArea.getLineCount();
   java.lang.String currentText = new java.lang.String();
   java.lang.String commentLine = new java.lang.String();
   int docLength = 0;

   currentText = myTextArea.getText();
   docLength   = currentText.length();
   try{
    tokenResponseString = new java.util.StringTokenizer(currentText,endLine);
    for(int i = 0; i < lineCount;i++){
          currentLine = tokenResponseString.nextToken(endLine);
          int lineLength = currentLine.length();
          if(lineLength > commentLength){
              commentLine = currentLine.substring(0,commentLength);
          }// end of the if line greater than maximum
          if(lineLength <= commentLength){
             commentLine = currentLine;
          }// end of if the line is less than the maximum
          currentLine = "";
    }// end of the for loop
   }catch(Exception e){
        System.out.print("Error in parsing comments : " + e.toString() + "\n");
   }// end of the catch block
   try{
      myTextArea.replaceRange("",0,docLength);
      myTextArea.repaint();
   }catch(Exception e){
        System.out.print("Error in clearing the TextArea : " + e.toString() + "\n");
     }
  }
/*=============================================================================================
/                      RunTrimDocument Inner Class of the FTPCommandLogModel
/=============================================================================================*/
  public class RunTrimDocument implements Runnable{
  private Thread myThread;
  private java.lang.String newCommentMessage = new java.lang.String();
  public RunTrimDocument(java.lang.String newMessage){
    newCommentMessage = newMessage;
  }
  public void run(){
    try{
      commentMessageDocument.getDefaultRootElement().getDocument().insertString(0,(newCommentMessage + terminator),newCommentStyle);
      checkCommentDocumentLength();
      }catch(javax.swing.text.BadLocationException bl){
         System.out.print("An error occurred while trying to insert a line into the CommentDocumentModel" + "\n");
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
/*=================================================================================
/         finalize() method
/=================================================================================*/
  protected void finalize() throws Throwable {
  // This method closes the file writers associated with each of the log files when the
  // program exits.
     try{
    }catch(Exception e2){
           System.out.println("A file I/O error occured while closing the Server File Writer. " + e2);
    }// end of the catch statement block
    super.finalize();
  }
/*================================================================================================
/         End of the CommentDocumentModel Class
/=================================================================================================*/
}
