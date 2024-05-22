package edu.caltech.palomar.util.gui;
 

//import java.awt.*;
import edu.caltech.palomar.util.gui.Assert;
//import edu.caltech.ipac.util.CoordException;


/**
 * A subclass of RaDecTextField that supports entering RA or longitude values
 * and checking.  Only a valid RA or longitude may be entered in this field <p>
 *
 * @author Xiuqin Wu
 */
public class DecTextField extends RaDecTextField {

       /** 
        * Get the model to use for this RA/Longitude text field.
        * @param int number or columns
        */
       public DecTextField(int col, CoordSystemEquinox csys ) {
            super(col, true, csys);
       }
}






