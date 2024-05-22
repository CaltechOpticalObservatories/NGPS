/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Interface.java to edit this template
 */
package edu.caltech.palomar.instruments.ngps.tables;
import edu.caltech.palomar.instruments.ngps.object.Target;
/**
 *
 * @author jennifermilburn
 */
public interface EditTableInterface {
   public void reorder(int fromIndex, int toIndex);
   public void reorder_table();
   public void clearTable();
   public void addTarget(Target current);
   public void insert(int index,Target current);
   public void delete(int index);
   public  Target getRecord(int recordNumber);   
   public int getRowCount();
}



