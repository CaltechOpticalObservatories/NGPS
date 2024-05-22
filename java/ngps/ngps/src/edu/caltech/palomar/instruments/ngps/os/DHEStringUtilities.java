/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package edu.caltech.palomar.instruments.ngps.os; 
//=== File Prolog =============================================================
//	This code was developed by California Institute of Technology

import edu.caltech.palomar.dhe2.*;
//=== File Prolog =============================================================
//	This code was developed by California Institute of Technology
//      Caltech Optical Observatories, Palomar Observatory
//
//--- Contents ----------------------------------------------------------------
//	 DHEStringUtilities
//
//--- Description -------------------------------------------------------------
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
//
//      March 18, 2011 Jennifer Milburn
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
//	In no event shall Caltech be liable for any damages, including, but not
//	limited to direct, indirect, special or consequential damages, arising out
//	of, resulting from, or in any way connected with this software, whether or
//	not based upon warranty, contract, tort or otherwise, whether or not
//	injury was sustained by persons or property or otherwise, and whether or
//	not loss was sustained from or arose out of the results of, or use of,
//	their software or services provided hereunder.
//
//=== End File Prolog =========================================================
/*=============================================================================================
/      class DHEStringUtilities
/=============================================================================================*/
public class DHEStringUtilities {
   public java.lang.String instrumentName = new java.lang.String();
    /*=============================================================================================
/      class DHEStringUtilities
/=============================================================================================*/
   public void DHEStringUtilities(){
   }
/*=============================================================================================
/     stripInstrumentName()
/=============================================================================================*/
 public java.lang.String stripInstrumentName(java.lang.String response){
     java.lang.String response_final = new java.lang.String();
     try{
        if(instrumentName.length()+1 <= response.length()){
          response_final = response.substring(instrumentName.length()+1,response.length()) ;
        }
     }catch(Exception e){
     }
  return response_final;
 }
/*=============================================================================================
/     stripInstrumentName()
/=============================================================================================*/
 public java.lang.String stripUnits(java.lang.String response,java.lang.String units){
     java.lang.String response_final = new java.lang.String();
     try{
          response_final = response.replace(units, "");
     }catch(Exception e){
     }
  return response_final;
 }
/*=============================================================================================
/     java.lang.String stripInstrumentName(java.lang.String new_instrumentName, java.lang.String response)
/=============================================================================================*/
 public java.lang.String stripInstrumentName(java.lang.String new_instrumentName, java.lang.String response){
     instrumentName = new_instrumentName;
     try{
        if(instrumentName.length()+1 >= response.length())
        response = response.substring(instrumentName.length()+1,response.length()) ;
     }catch(Exception e){
        response = null;
     }
  return response;
 }
/*=============================================================================================
/     setInstrumentName(java.lang.String new_instrument_name)
/=============================================================================================*/
 public void setInstrumentName(java.lang.String new_instrument_name){
     instrumentName = new_instrument_name;
 }
/*================================================================================================
/       stripIdentifier(java.lang.String token)
/=================================================================================================*/
   public java.lang.String stripIdentifier(java.lang.String token){
      int index = token.indexOf("=");
      int length = token.length();
      token = token.substring(index+2, length);
      return token;
   }
/*================================================================================================
/       stripIdentifier(java.lang.String token)
/=================================================================================================*/
   public java.lang.String stripIdentifierNoSpace(java.lang.String token){
      int index = token.indexOf("=");
      int length = token.length();
      token = token.substring(index+1, length);
      return token;
   }
}
