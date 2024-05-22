package edu.caltech.palomar.telescopes.P200.gui;
//=== File Prolog =============================================================
//	This code was developed by John Thorstensen of Dartmouth University
//
//      It has been refactored and modified by Jennifer Milburn for
//      use at the Palomar Observatory.
//
//      Caltech Optical Observatories, Palomar Observatory
//
//--- Contents ----------------------------------------------------------------
//    ObjectSelectionPanel
//
//--- Description -------------------------------------------------------------
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
//       DERIVED DIRECTLY FROM THE JSKYCALC by JOHN THORSTENSEN, Dartmouth University
//       Original Implementation : John Thorstensen copyright 2007,
//
//       July 24, 2010  Jennifer Milburn, refactor for use at the Palomar Observatory
//
//
//   JSkyCalc24m.java -- 2009 January.  I liked the 1.3m so much I decided
//   to do something similar for the 2.4m.  This has the capability of reading
//   the telescope coords and commenting on them, but it doesn't slew the
//   telescope.  It'll be nice to automatically have the telescope coords there,
//   though. */
//
// JSkyCalc13m.java -- 2009 January.  This version adds an interface to
// * Dick Treffers' 1.3m telescope control server, which communicates via
// * the BAIT protocol.  The target-selection facilities here can then be
// * used. */
//
// JSkyCalc.java -- copyright 2007, John Thorstensen, Dartmouth College. */
//
// TERMS OF USE --
// Anyone is free to use this software for any purpose, and to
// modify it for their own purposes, provided that credit is given to the author
// in a prominent place.  For the present program that means that the green
// title and author banner appearing on the main window must not be removed,
// and may not be altered without premission of the author.
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
import edu.caltech.palomar.telescopes.P200.gui.tables.AstroObjectsModel;
import edu.caltech.palomar.telescopes.P200.gui.tables.AstroObject;
import edu.caltech.palomar.telescopes.P200.gui.tables.AstroObjectNameListModel;
import javax.swing.JList;
import java.util.Vector;
import javax.swing.event.*;
import javax.swing.event.ListSelectionListener;
import javax.swing.ListModel;
import javax.swing.DefaultListModel;
import javax.swing.JTable;

/*=============================================================================================
/    ObjectSelectionPanel extends JFrame
/=============================================================================================*/
public class ObjectSelectionPanel extends javax.swing.JPanel implements  ListSelectionListener {
    public AstroObjectsModel myAstroObjectsModel;
    public int []            selectedIndices;
    private String           selected_key         = new java.lang.String();
    public  AirmassFrame     myAirmassFrame;
/*=============================================================================================
/     ObjectSelectionPanel Constructor
/=============================================================================================*/
    /** Creates new form ObjectSelectionPanel */
    public ObjectSelectionPanel() {
        initComponents();
 //       this.setSize(250,500);
    }
/*=============================================================================================
/     valueChanged(ListSelectionEvent l)
/=============================================================================================*/
  public void setAirmassFrame(AirmassFrame newAirmassFrame){
      myAirmassFrame = newAirmassFrame;
  }    
/*=============================================================================================
/     valueChanged(ListSelectionEvent l)
/=============================================================================================*/
    public void valueChanged(ListSelectionEvent l) {
     try {
         if (!l.getValueIsAdjusting()) {
             myAstroObjectsModel.getSelectedObjects().clear();
             selectedIndices = ObjectList.getSelectedRows();
             int selectedNumber = selectedIndices.length;
             for(int i=0;i < selectedNumber;i++){
                 AstroObject   currentObject = new AstroObject();
                 selected_key  = (String)myAstroObjectsModel.getObjectListModel().getRecord(selectedIndices[i]);
                 currentObject = myAstroObjectsModel.presenterKey.get(selected_key);
                 myAstroObjectsModel.getSelectedObjects().put(currentObject.name, currentObject);
             }
             myAstroObjectsModel.fireSelectionChanged();
         }
        } catch (Exception e) {
           System.out.println("Error line 104: ObjectSelectionPanel value change event for selected objects list");
        }
    }
/*=============================================================================================
/     getListTable()
/=============================================================================================*/
public JTable getListTable(){
    return ObjectList;
}
/*=============================================================================================
/      getAstroObjectModel()
/=============================================================================================*/
 public AstroObjectsModel getAstroObjectModel(){
     return myAstroObjectsModel;
 }
/*=============================================================================================
/      setAstroObjectModel()
/=============================================================================================*/
 public void setAstroObjectModel(AstroObjectsModel newAstroObjectsModel){
     myAstroObjectsModel =  newAstroObjectsModel;
     ObjectList.getSelectionModel().addListSelectionListener(this);
     ObjectList.setModel(myAstroObjectsModel.getObjectListModel());
     myAstroObjectsModel.addActionListener( new java.awt.event.ActionListener()  {
        public void actionPerformed(java.awt.event.ActionEvent e) {
           if(e.ACTION_PERFORMED == AstroObjectsModel.LIST_CHANGED){
              update();
           }
        }
      });
 }
/*=============================================================================================
/     update()
/=============================================================================================*/
 private void update(){
    ObjectList.repaint(); 
    this.repaint(); 
 }
 /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        jScrollPane1 = new javax.swing.JScrollPane();
        ObjectList = new javax.swing.JTable();
        jLabel1 = new javax.swing.JLabel();
        jLabel2 = new javax.swing.JLabel();
        ResetDisplayButton = new javax.swing.JButton();

        setBackground(new java.awt.Color(0, 0, 0));
        setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 153, 51), 5));
        setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        jScrollPane1.setVerticalScrollBarPolicy(javax.swing.ScrollPaneConstants.VERTICAL_SCROLLBAR_ALWAYS);

        ObjectList.setModel(new javax.swing.table.DefaultTableModel(
            new Object [][] {
                {null, null, null, null},
                {null, null, null, null},
                {null, null, null, null},
                {null, null, null, null}
            },
            new String [] {
                "Title 1", "Title 2", "Title 3", "Title 4"
            }
        ));
        jScrollPane1.setViewportView(ObjectList);

        add(jScrollPane1, new org.netbeans.lib.awtextra.AbsoluteConstraints(17, 53, 140, 400));

        jLabel1.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        jLabel1.setForeground(new java.awt.Color(51, 255, 0));
        jLabel1.setText("Airmass Display");
        add(jLabel1, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 10, -1, -1));

        jLabel2.setFont(new java.awt.Font("Arial", 1, 14)); // NOI18N
        jLabel2.setForeground(new java.awt.Color(51, 255, 0));
        jLabel2.setText("Selection");
        add(jLabel2, new org.netbeans.lib.awtextra.AbsoluteConstraints(60, 30, -1, -1));

        ResetDisplayButton.setText("Reset Display");
        ResetDisplayButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                ResetDisplayButtonActionPerformed(evt);
            }
        });
        add(ResetDisplayButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(30, 460, -1, 20));
    }// </editor-fold>//GEN-END:initComponents

    private void ResetDisplayButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_ResetDisplayButtonActionPerformed
        myAirmassFrame.resetAirmassDisplay();
    }//GEN-LAST:event_ResetDisplayButtonActionPerformed


    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JTable ObjectList;
    private javax.swing.JButton ResetDisplayButton;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JScrollPane jScrollPane1;
    // End of variables declaration//GEN-END:variables

}
