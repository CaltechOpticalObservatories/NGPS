/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package edu.caltech.palomar.telescopes.guider.catalog.tycho2;
//=== File Prolog =============================================================
//
//
//      It has been refactored and modified by Jennifer Milburn for
//      use at the Palomar Observatory.
//
//      Caltech Optical Observatories, Palomar Observatory
//
//--- Contents ----------------------------------------------------------------
//       tycho2index
//
//--- Description -------------------------------------------------------------
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
// TERMS OF USE --
//      June 21,2011   - original development Jennifer Milburn
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
import edu.dartmouth.jskycalc.util.Conv;
/*================================================================================================
/        tycho2index
/=================================================================================================*/
public class tycho2index {
    public double ra_min;
    public double ra_max;
    public double dec_min;
    public double dec_max;
    public java.lang.String    TYC1;
    public int    rec_t2;
    public int    rec_s1;
    public int    last_index;
    public int    length;
/*================================================================================================
/        tycho2index
/=================================================================================================*/
   public tycho2index(int new_TYC1,byte[] raw){
       this.TYC1 = (Integer.toString(new_TYC1));
/*--------------------------------------------------------------------------------

Byte-by-byte Description of file: index.dat
--------------------------------------------------------------------------------
   Bytes Format  Units   Label     Explanations
--------------------------------------------------------------------------------
   1-  7  I7     ---     rec_t2    +  Tycho-2 rec. of 1st star in region (1)
   9- 14  I6     ---     rec_s1    += Suppl-1 rec. of 1st star in region (1)
  16- 21  F6.2   deg     RAmin     [-0.01,] smallest RA in region (2)
  23- 28  F6.2   deg     RAmax     [,360.00] largest RA in region (2)
  30- 35  F6.2   deg     DEmin     smallest Dec in this region (2)
  37- 42  F6.2   deg     DEmax     largest Dec in this region (2)
--------------------------------------------------------------------------------

Note (1): The catalogue is sorted according to the GSC region numbers.
    The line i of the index file gives the record number in Tycho-2 of
    the first star in GSC region i. Line i+1 gives the record number +1
    of the last star in GSC region i. For Supplement-1, some regions are
    empty and line i and line i+1 give the same record number.

Note (2): a safe rounding was applied. Minimum values are always
    rounded down and maximum values up.
*/
 java.lang.String record = new java.lang.String(raw);
 try{
      rec_t2 = Integer.parseInt((record.substring(    1,  7)).trim());
      rec_s1 = Integer.parseInt((record.substring(    9, 14)).trim());
      ra_min  = Double.parseDouble(record.substring(15, 21));
      ra_max  = Double.parseDouble(record.substring(22, 28));
      dec_min = Double.parseDouble(record.substring(30, 35));
      dec_max = Double.parseDouble(record.substring(37, 42));
 } catch (Exception e2) {
   System.out.println("Error parsing the catalog file: " + raw);
 }
}
/*================================================================================================
/        tycho2index
/=================================================================================================*/
   public tycho2index(int new_TYC1,java.lang.String record){
        this.TYC1 = (Integer.toString(new_TYC1));
try{
//     String test1 = (record.substring(    1,  7)).trim();
//     String test2 = (record.substring(    9, 14)).trim();
//     String test3 = record.substring(    16, 21);
//     String test4 = record.substring(    23, 28);
//     String test5 = record.substring(    30, 35);
//     String test6 = record.substring(    37, 42);
      rec_t2 = Integer.parseInt((record.substring(    1,  7)).trim());
      rec_s1 = Integer.parseInt((record.substring(    9, 14)).trim());
      ra_min  = Double.parseDouble(record.substring(15, 21));
      ra_max  = Double.parseDouble(record.substring(22, 28));
      dec_min = Double.parseDouble(record.substring(30, 35));
      dec_max = Double.parseDouble(record.substring(37, 42));
 } catch (Exception e2) {
   System.out.println("Error parsing the catalog file: " + record);
 }
}
/*================================================================================================
/        tycho2index
/=================================================================================================*/
   public tycho2index(java.lang.String record){
try{
//     String test1 = (record.substring(    1,  7)).trim();
//     String test2 = (record.substring(    9, 14)).trim();
//     String test3 = record.substring(    16, 21);
//     String test4 = record.substring(    23, 28);
//     String test5 = record.substring(    30, 35);
//     String test6 = record.substring(    37, 42);
 java.util.StringTokenizer st = new java.util.StringTokenizer(record);
      TYC1        = st.nextToken(",");
      last_index = Integer.parseInt(st.nextToken(","));
      length      = Integer.parseInt(st.nextToken(","));
      ra_min      = Double.parseDouble(st.nextToken(","));
      ra_max      = Double.parseDouble(st.nextToken(","));
      dec_min     = Double.parseDouble(st.nextToken(","));
      dec_max     = Double.parseDouble(st.nextToken(","));
 } catch (Exception e2) {
   System.out.println("Error parsing the catalog file: " + record);
 }
}
}

