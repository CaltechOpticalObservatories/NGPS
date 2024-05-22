/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package edu.caltech.palomar.instruments.ngps.util;

public interface Reorderable {
   public void reorder(int fromIndex, int toIndex);
   public void reorder(int[] fromIndex, int toIndex);
}
