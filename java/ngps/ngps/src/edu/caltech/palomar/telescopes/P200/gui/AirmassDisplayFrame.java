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
//    AirmassDisplayFrame   - Display JFrame for Airmass Display Graphs
//
//--- Description -------------------------------------------------------------
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
//       DERIVED DIRECTLY FROM THE JSKYCALC by JOHN THORSTENSEN, Dartmouth University
//       Original Implementation : John Thorstensen copyright 2007,
//
//       August 15, 2010  Jennifer Milburn, refactor for use at the Palomar Observatory
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
import edu.dartmouth.jskycalc.gui.AirmassDisplay;
import edu.dartmouth.jskycalc.objects.Observation;
import edu.dartmouth.jskycalc.objects.NightlyAlmanac;
import edu.caltech.palomar.telescopes.P200.gui.tables.AstroObjectsModel;
import edu.dartmouth.jskycalc.JSkyCalcModel;
/*=============================================================================================
/     Class Declaration: AirmassDisplayFrame()
/=============================================================================================*/
public class AirmassDisplayFrame extends javax.swing.JFrame {
//    public AirmassDisplay    myAirmassDisplay;
    public AstroObjectsModel aom;
    public NightlyAlmanac    Nightly;
    public JSkyCalcModel     jscm;

/*=============================================================================================
/    Constructor AirmassDisplayFrame()
/=============================================================================================*/
    /** Creates new form AirmassDisplayFrame */
    public AirmassDisplayFrame() {
        initComponents();
    }
/*=============================================================================================
/     void main(String args[])
/=============================================================================================*/
    public void initializeDisplay(NightlyAlmanac Nightly, AstroObjectsModel aom,JSkyCalcModel jscm){
        this.aom = aom;
        this.jscm = jscm;
        myAirmassDisplay = new AirmassDisplay();
        myAirmassDisplay.setComponentSize(800,500);
        myAirmassDisplay.initializeAirmassDisplay(Nightly,aom,jscm);
        myAirmassDisplay.Update();
        AirmassPanel.add(myAirmassDisplay);
        AirmassPanel.repaint();
    }
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        AirmassPanel = new javax.swing.JPanel();
        jLabel3 = new javax.swing.JLabel();
        myAirmassDisplay = new edu.dartmouth.jskycalc.gui.AirmassDisplay();
        jLabel1 = new javax.swing.JLabel();
        jScrollPane1 = new javax.swing.JScrollPane();
        jList1 = new javax.swing.JList();
        jLabel2 = new javax.swing.JLabel();

        setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);
        setBackground(new java.awt.Color(0, 0, 0));

        AirmassPanel.setBackground(new java.awt.Color(0, 0, 0));
        AirmassPanel.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(255, 255, 255), 4));

        jLabel3.setText("jLabel3");

        org.jdesktop.layout.GroupLayout myAirmassDisplayLayout = new org.jdesktop.layout.GroupLayout(myAirmassDisplay);
        myAirmassDisplay.setLayout(myAirmassDisplayLayout);
        myAirmassDisplayLayout.setHorizontalGroup(
            myAirmassDisplayLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(0, 921, Short.MAX_VALUE)
        );
        myAirmassDisplayLayout.setVerticalGroup(
            myAirmassDisplayLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(0, 359, Short.MAX_VALUE)
        );

        org.jdesktop.layout.GroupLayout AirmassPanelLayout = new org.jdesktop.layout.GroupLayout(AirmassPanel);
        AirmassPanel.setLayout(AirmassPanelLayout);
        AirmassPanelLayout.setHorizontalGroup(
            AirmassPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(AirmassPanelLayout.createSequentialGroup()
                .add(5, 5, 5)
                .add(jLabel3)
                .addContainerGap(919, Short.MAX_VALUE))
            .add(AirmassPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                .add(AirmassPanelLayout.createSequentialGroup()
                    .add(28, 28, 28)
                    .add(myAirmassDisplay, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addContainerGap()))
        );
        AirmassPanelLayout.setVerticalGroup(
            AirmassPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(AirmassPanelLayout.createSequentialGroup()
                .add(5, 5, 5)
                .add(jLabel3)
                .addContainerGap(378, Short.MAX_VALUE))
            .add(AirmassPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                .add(org.jdesktop.layout.GroupLayout.TRAILING, AirmassPanelLayout.createSequentialGroup()
                    .addContainerGap()
                    .add(myAirmassDisplay, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addContainerGap()))
        );

        jLabel1.setBackground(new java.awt.Color(0, 0, 0));
        jLabel1.setFont(new java.awt.Font("Arial", 1, 18));
        jLabel1.setForeground(new java.awt.Color(0, 255, 255));
        jLabel1.setText("Astronomical Object Air Mass Graphs ");

        jScrollPane1.setVerticalScrollBarPolicy(javax.swing.ScrollPaneConstants.VERTICAL_SCROLLBAR_ALWAYS);

        jList1.setModel(new javax.swing.AbstractListModel() {
            String[] strings = { "Item 1", "Item 2", "Item 3", "Item 4", "Item 5" };
            public int getSize() { return strings.length; }
            public Object getElementAt(int i) { return strings[i]; }
        });
        jScrollPane1.setViewportView(jList1);

        jLabel2.setBackground(new java.awt.Color(0, 0, 0));
        jLabel2.setFont(new java.awt.Font("Arial", 1, 14));
        jLabel2.setForeground(new java.awt.Color(0, 255, 255));
        jLabel2.setText("Astronomical Objects List");

        org.jdesktop.layout.GroupLayout layout = new org.jdesktop.layout.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(layout.createSequentialGroup()
                        .add(11, 11, 11)
                        .add(jLabel2))
                    .add(layout.createSequentialGroup()
                        .add(28, 28, 28)
                        .add(jScrollPane1, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 152, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)))
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(layout.createSequentialGroup()
                        .add(307, 307, 307)
                        .add(jLabel1))
                    .add(layout.createSequentialGroup()
                        .add(18, 18, 18)
                        .add(AirmassPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)))
                .add(29, 29, 29))
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .addContainerGap()
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel2, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 16, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(jLabel1, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 16, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(18, 18, 18)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(AirmassPanel, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .add(jScrollPane1, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 407, Short.MAX_VALUE))
                .addContainerGap())
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents
/*=============================================================================================
/     void main(String args[])
/=============================================================================================*/

    /**
    * @param args the command line arguments
    */
    public static void main(String args[]) {
        java.awt.EventQueue.invokeLater(new Runnable() {
            public void run() {
                new AirmassDisplayFrame().setVisible(true);
            }
        });
    }

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JPanel AirmassPanel;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JLabel jLabel3;
    private javax.swing.JList jList1;
    private javax.swing.JScrollPane jScrollPane1;
    private edu.dartmouth.jskycalc.gui.AirmassDisplay myAirmassDisplay;
    // End of variables declaration//GEN-END:variables

}
