/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package edu.caltech.palomar.instruments.ngps.util;
import java.awt.Color;
import javax.swing.table.*;
import javax.swing.JTable;
import java.awt.Component;

public class ColumnColorRenderer extends DefaultTableCellRenderer {
   Color backgroundColor, foregroundColor;
   public ColumnColorRenderer(Color backgroundColor, Color foregroundColor) {
      super();
      this.backgroundColor = backgroundColor;
      this.foregroundColor = foregroundColor;
   }
   public Component getTableCellRendererComponent(JTable table, Object value, boolean isSelected,   boolean hasFocus, int row, int column) {
      Component cell = super.getTableCellRendererComponent(table, value, isSelected, hasFocus, row, column);
//      cell.setBackground(backgroundColor);
      cell.setForeground(foregroundColor);
      return cell;
   }
}