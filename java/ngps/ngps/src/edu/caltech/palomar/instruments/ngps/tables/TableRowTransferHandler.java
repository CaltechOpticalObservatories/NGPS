/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package edu.caltech.palomar.instruments.ngps.tables;
import edu.caltech.palomar.instruments.ngps.util.Reorderable;
import javax.swing.JTable;
import javax.swing.TransferHandler;
import java.awt.datatransfer.DataFlavor;
import javax.activation.ActivationDataFlavor;
import javax.swing.JComponent;
import java.awt.datatransfer.Transferable;
import javax.activation.DataHandler;
import java.awt.dnd.DragSource;
import java.awt.Cursor;
/**
 * Handles drag & drop row reordering
 */
public class TableRowTransferHandler extends TransferHandler {
   private final DataFlavor localObjectFlavor = new ActivationDataFlavor(Integer.class, "application/x-java-Integer;class=java.lang.Integer", "Integer Row Index");
   private JTable           table             = null;
   private int              selected_row;

   public TableRowTransferHandler(JTable table) {
      this.table = table;
   }

   @Override
   protected Transferable createTransferable(JComponent c) {
      assert (c == table);
      return new DataHandler(Integer.valueOf(table.getSelectedRow()), localObjectFlavor.getMimeType());
   }

   @Override
   public boolean canImport(TransferHandler.TransferSupport info) {
      boolean b = info.getComponent() == table && info.isDrop() && info.isDataFlavorSupported(localObjectFlavor);
      table.setCursor(b ? DragSource.DefaultMoveDrop : DragSource.DefaultMoveNoDrop);
      return b;
   }

   @Override
   public int getSourceActions(JComponent c) {
      return TransferHandler.COPY_OR_MOVE;
   }

   @Override
   public boolean importData(TransferHandler.TransferSupport info) {
      JTable target = (JTable) info.getComponent();
      JTable.DropLocation dl = (JTable.DropLocation) info.getDropLocation();
      int index = dl.getRow();
      int max = table.getModel().getRowCount();
      if (index < 0 || index > max) index = max;
      target.setCursor(Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
      try {
         Integer rowFrom = (Integer) info.getTransferable().getTransferData(localObjectFlavor);
         if (rowFrom != -1 && rowFrom != index) {
            if (index > rowFrom) index--;
            ((Reorderable)table.getModel()).reorder(rowFrom, index);
            target.getSelectionModel().addSelectionInterval(index, index);
            return true;
         }
      } catch (Exception e) {
         e.printStackTrace();
      }
      return false;
   }

   @Override
   protected void exportDone(JComponent c, Transferable t, int act) {
      if ((act == TransferHandler.MOVE) || (act == TransferHandler.NONE)) {
         table.setCursor(Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
      }
   }

}