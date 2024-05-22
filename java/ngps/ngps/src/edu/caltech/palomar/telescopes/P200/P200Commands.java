package edu.caltech.palomar.telescopes.P200;
//=== File Prolog =============================================================
//	This code was developed by California Institute of Technology
//      Caltech Optical Observatories, Palomar Observatory
//
//--- Contents ----------------------------------------------------------------
//	 P200Commands- This is the implementation of the P200 specific command syntax
//
//--- Description -------------------------------------------------------------
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
//
//      July 14, 2010 Jennifer Milburn
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
import java.util.StringTokenizer;
import jsky.coords.DMS;
import jsky.coords.HMS;
import jsky.science.Coordinates;
import jsky.science.CoordinatesOffset;
import edu.caltech.palomar.telescopes.P200.TelescopesIniReader;
/*================================================================================================
/        P200Commands() Class Declaration
/=================================================================================================*/
public class P200Commands {
  public static java.lang.String  getCoordinatesString = "";
  public static java.lang.String  String = "";
  public static java.lang.String  terminator = "\r";


  public static java.lang.String UNKNOWN_COMMAND = "-4444444";
  public static int       ACKNOWLEDGE          = 1000;
  public static int       SYNTAX_ERROR         = 2000;
  public static int       ERROR                = 3000;
  public static int       FAILURE              = 4000;
  public static int       INTERNAL_ERROR       = 5000;
  public static int       DONE                 = 6000;
  public static int       INTERMEDIATE         = 7000;
  public static java.lang.String   COLON       = ":";
  public static java.lang.String   NOTSET      = "NotSet";
  public static java.lang.String   NOTFOUND_NM = "NOTFOUND_NM";
  public static java.lang.String   NOTFOUND    = "NOTFOUND";
  private static java.lang.String  SPACE       = " ";
  private static java.lang.String  E           = "E";
  private static java.lang.String  W           = "W";
  private static java.lang.String  N           = "N";
  private static java.lang.String  S           = "S";
  private static java.lang.String  ES          = "ES";
  private static java.lang.String  WS          = "WS";
  private static java.lang.String  PT          = "PT";
  private static java.lang.String  F           = "F";

  public  static int               ARCSECONDS  = 0;
  public  static int               SECONDS     = 1;

  public                           TelescopesIniReader      myTelescopesIniReader;
/*================================================================================================
/        P200Commands() Class Declaration
/=================================================================================================*/
  public P200Commands(TelescopesIniReader      newTelescopesIniReader) {
    this.myTelescopesIniReader = newTelescopesIniReader;
  }
/*================================================================================================
/        moveE(double offset)
/=================================================================================================*/
public java.lang.String moveE(double offset,int units){
    java.lang.String commandString = new java.lang.String();
     switch(units){
      case 0:{commandString = "E";break;}   // the offset is in arcseconds
      case 1:{commandString = "ES";break;}  // the offset is in seconds
    }
    commandString = commandString + SPACE + Double.toString(offset) + terminator;
   return commandString; 
}
/*================================================================================================
/        moveW(double offset,int units)
/=================================================================================================*/
public java.lang.String moveW(double offset,int units){
    // the offset is in arcseconds
    java.lang.String commandString = new java.lang.String();
     switch(units){
      case 0:{commandString = "E";break;}   // the offset is in arcseconds
      case 1:{commandString = "ES";break;}  // the offset is in seconds
    }
    commandString = commandString + SPACE + Double.toString(offset) + terminator;
   return commandString;
}
/*================================================================================================
/        moveN(double offset)
/=================================================================================================*/
public java.lang.String moveN(double offset){
    // the offset is in arcseconds
    java.lang.String commandString = new java.lang.String();
    commandString = N + SPACE + Double.toString(offset) + terminator;
   return commandString;
}
/*================================================================================================
/        moveS(double offset)
/=================================================================================================*/
public java.lang.String moveS(double offset){
    // the offset is in arcseconds
    java.lang.String commandString = new java.lang.String();
    commandString = S + SPACE + Double.toString(offset) + terminator;
   return commandString;
}
/*================================================================================================
/        moveOffset(double offset_RA,double offset_DEC)
/=================================================================================================*/
public java.lang.String moveOffset(double offset_RA,double offset_DEC,int units){
    // the offset is in arcseconds
    java.lang.String commandString = new java.lang.String();
    switch(units){
      case 0:{commandString = "PT";break;}
      case 1:{commandString = "PTS";break;}
    }
    commandString = commandString + SPACE + Double.toString(offset_RA) + SPACE + Double.toString(offset_DEC)+ terminator;
   return commandString;
}
/*================================================================================================
/        moveHome()
/=================================================================================================*/
public java.lang.String moveHome(){
    // home is the position set by the last GO command, GO can only be run by the telescope operator
    java.lang.String commandString = new java.lang.String();
    commandString = F  + terminator;
   return commandString;
}
/*================================================================================================
/       java.lang.String evaluateTrackState(int newTrackState)
/=================================================================================================*/
  public java.lang.String evaluateTrackState(int newTrackState){
/*    Translation between integer value and meaning:
            <FieldValue value= "0" name="off"/>
            <FieldValue value= "0" name="none"/>
            <FieldValue value= "1" name="centroid"/>
            <FieldValue value= "3" name="rof"/>
            <FieldValue value= "5" name="limb"/>
            <FieldValue value= "9" name="offset"/>
            <FieldValue value="11" name="rof+offset"/>
            <FieldValue value="17" name="centroid+boresight"/>
            <FieldValue value="19" name="rof+boresight"/>
            <FieldValue value="25" name="offset+boresight"/>
            <FieldValue value="27" name="rof+offset+boresight"/>
            <FieldValue value="33" name="centroid+inertial"/>
            <FieldValue value="35" name="rof+inertial"/>
            <FieldValue value="41" name="offset+inertial"/>
            <FieldValue value="43" name="rof+offset+inertial"/>
            <FieldValue value="49" name="centroid+boresight+inertial"/>
            <FieldValue value="51" name="rof+boresight+inertial"/>
            <FieldValue value="57" name="offset+boresight+inertial"/>
            <FieldValue value="59" name="rof+offset+boresight+inertial"/>
 */
    int trackState = newTrackState;
    java.lang.String TrackStateString = new java.lang.String();
    switch(trackState){
      case 0:{TrackStateString = "off";break;}
      case 1:{TrackStateString = "centroid";break;}
      case 3:{TrackStateString = "rof";break;}
      case 5:{TrackStateString = "limb";break;}
      case 9:{TrackStateString = "offset";break;}
      case 11:{TrackStateString = "rof+offset";break;}
      case 17:{TrackStateString = "centroid+boresight";break;}
      case 19:{TrackStateString = "rof+boresight";break;}
      case 25:{TrackStateString = "offset+boresight";break;}
      case 27:{TrackStateString = "rof+offset+boresight";break;}
      case 33:{TrackStateString = "centroid+inertial";break;}
      case 35:{TrackStateString = "rof+inertial";break;}
      case 41:{TrackStateString = "offset+inertial";break;}
      case 43:{TrackStateString = "rof+offset+inertial";break;}
      case 49:{TrackStateString = "centroid+boresight+inertial";break;}
      case 51:{TrackStateString = "rof+boresight+inertial";break;}
      case 57:{TrackStateString = "offset+boresight+inertial";break;}
      case 59:{TrackStateString = "rof+offset+boresight+inertial";break;}
    }
   return TrackStateString;
}
 /*================================================================================================
 /       java.lang.String evaluateTrackState(int newTrackState)
 /=================================================================================================*/
   public java.lang.String evaluateOperationState(int newOperationState){
 /*    Translation between integer value and meaning:
  ta_tsc.tsc_mcs_hk.tsc_status -
  The operation mode of the servo system.
  Translation between the integer response
  you will get from the MCS and the physical meaning:
          <FieldValue value="1" name="SHUTDOWN_ENTER_POWER_ON"/>
          <FieldValue value="2" name="SHUTDOWN_ENTER_CAGED"/>
          <FieldValue value="4" name="SHUTDOWN_ONGOING"/>
          <FieldValue value="8" name="SHUTDOWN_EXIT_CAGED"/>
          <FieldValue value="16" name="SHUTDOWN_EXIT_POWER_OFF"/>
          <FieldValue value="32" name="CAGED_ENTER_SHUTDOWN"/>
          <FieldValue value="64" name="CAGED_ENTER_STANDBY"/>
          <FieldValue value="128" name="CAGED_ONGOING"/>
          <FieldValue value="256" name="CAGED_EXIT_SHUTDOWN"/>
          <FieldValue value="512" name="CAGED_EXIT_STANDBY"/>
          <FieldValue value="1024" name="STANDBY_ENTER_CAGED"/>
          <FieldValue value="2048" name="STANDBY_ENTER_STAB_LOCAL"/>
          <FieldValue value="4096" name="STANDBY_ONGOING"/>
          <FieldValue value="8192" name="STANDBY_EXIT_CAGED"/>
          <FieldValue value="16384" name="STANDBY_EXIT_STAB_LOCAL"/>
          <FieldValue value="16400" name="STANDBY_ENTER_ACTIVE"/>
          <FieldValue value="16416" name="ACTIVE_ENTER_STANDBY"/>
          <FieldValue value="16432" name="ACTIVE_ONGOING"/>
          <FieldValue value="32768" name="STAB_LOCAL_ENTER_STANDBY"/>
          <FieldValue value="65536" name="STAB_LOCAL_ENTER_STAB_INERTIAL"/>
          <FieldValue value="131072" name="STAB_LOCAL_ENTER_AUTO_BAL"/>
          <FieldValue value="262144" name="STAB_LOCAL_ONGOING"/>
          <FieldValue value="524288" name="STAB_LOCAL_EXIT_STANDBY"/>
          <FieldValue value="1048576" name="STAB_LOCAL_EXIT_STAB_INERTIAL"/>
          <FieldValue value="2097152" name="STAB_LOCAL_EXIT_AUTO_BAL"/>
          <FieldValue value="4194304" name="STAB_LOCAL_AUTO_BAL_ENTER"/>
          <FieldValue value="8388608" name="STAB_LOCAL_AUTO_BAL_ONGOING"/>
          <FieldValue value="16777216" name="STAB_LOCAL_AUTO_BAL_EXIT"/>
          <FieldValue value="33554432" name="STAB_INERTIAL_ENTER"/>
          <FieldValue value="67108864" name="STAB_INERTIAL_ONGOING"/>
          <FieldValue value="134217728" name="STAB_INERTIAL_EXIT"/>
          <FieldValue value="268435456" name="SERVICE"/>
  */
     int trackState = newOperationState;
     java.lang.String TrackStateString = new java.lang.String();
     switch(trackState){
       case 1:{         TrackStateString = "SHUTDOWN_ENTER_POWER_ON";      break;}
       case 2:{         TrackStateString = "SHUTDOWN_ENTER_CAGED";         break;}
       case 4:{         TrackStateString = "SHUTDOWN_ONGOING";             break;}
       case 8:{         TrackStateString = "SHUTDOWN_EXIT_CAGED";          break;}
       case 16:{        TrackStateString = "SHUTDOWN_EXIT_POWER_OFF";      break;}
       case 32:{        TrackStateString = "CAGED_ENTER_SHUTDOWN";         break;}
       case 64:{        TrackStateString = "CAGED_ENTER_STANDBY";          break;}
       case 128:{       TrackStateString = "CAGED_ONGOING";                break;}
       case 256:{       TrackStateString = "CAGED_EXIT_SHUTDOWN";          break;}
       case 512:{       TrackStateString = "CAGED_EXIT_STANDBY";           break;}
       case 1024:{      TrackStateString = "STANDBY_ENTER_CAGED";          break;}
       case 2048:{      TrackStateString = "STANDBY_ENTER_STAB_LOCAL";     break;}
       case 4096:{      TrackStateString = "STANDBY_ONGOING";              break;}
       case 8192:{      TrackStateString = "STANDBY_EXIT_CAGED";           break;}
       case 16384:{     TrackStateString = "STANDBY_EXIT_STAB_LOCAL";      break;}
       case 16400:{     TrackStateString = "STANDBY_ENTER_ACTIVE";         break;}
       case 16416:{     TrackStateString = "ACTIVE_ENTER_STANDBY";         break;}
       case 16432:{     TrackStateString = "ACTIVE_ONGOING";               break;}
       case 32768:{     TrackStateString = "STAB_LOCAL_ENTER_STANDBY";     break;}
       case 65536:{     TrackStateString = "STAB_LOCAL_ENTER_STAB_INERTIAL";break;}
       case 131072:{    TrackStateString = "STAB_LOCAL_ENTER_AUTO_BAL";    break;}
       case 262144:{    TrackStateString = "STAB_LOCAL_ONGOING";           break;}
       case 524288:{    TrackStateString = "STAB_LOCAL_EXIT_STANDBY";      break;}
       case 1048576:{   TrackStateString = "STAB_LOCAL_EXIT_STAB_INERTIAL";break;}
       case 2097152:{   TrackStateString = "STAB_LOCAL_EXIT_AUTO_BAL";     break;}
       case 4194304:{   TrackStateString = "STAB_LOCAL_AUTO_BAL_ENTER";    break;}
       case 8388608:{   TrackStateString = "STAB_LOCAL_AUTO_BAL_ONGOING";  break;}
       case 16777216:{  TrackStateString = "STAB_LOCAL_AUTO_BAL_EXIT";     break;}
       case 33554432:{  TrackStateString = "STAB_INERTIAL_ENTER";          break;}
       case 67108864:{  TrackStateString = "STAB_INERTIAL_ONGOING";        break;}
       case 134217728:{ TrackStateString = "STAB_INERTIAL_EXIT";           break;}
       case 268435456:{ TrackStateString = "SERVICE";                      break;}
     }
    return TrackStateString;
 }
/*================================================================================================
/       isNotSet(java.lang.String checkString)
/=================================================================================================*/
public boolean isNotSet(java.lang.String checkString){
    boolean match = false;
            match = checkString.regionMatches(0,NOTSET,0,NOTSET.length());
    return match;
}
/*================================================================================================
/       isNotSet(java.lang.String checkString)
/=================================================================================================*/
  public boolean isNotFound(java.lang.String checkString){
      boolean match  = false;
      boolean match1 = false;
      boolean match2 = false;
              match1 = checkString.regionMatches(0,NOTFOUND,0,NOTFOUND.length());
              match2 = checkString.regionMatches(0,NOTFOUND_NM,0,NOTFOUND_NM.length());
      if(match1 | match2){
        match  = true;
      }
      return match;
  }
/*================================================================================================
/       isValidResponse(java.lang.String checkString)
/=================================================================================================*/
 public boolean isValidResponse(java.lang.String checkString){
   boolean valid  = true;
   boolean valid1 = false;
   boolean valid2 = false;
   valid1 = isNotSet(checkString);
   valid2 = isNotFound(checkString);
   if(valid1 | valid2){
     valid  = false;
   }
  return valid;
 }
/*================================================================================================
/       isFinalResponse(java.lang.String checkString)
/=================================================================================================*/
  public boolean isFinalResponse(int newResponseCode){
    int responseCode = newResponseCode;
    boolean state   = false;
    if(responseCode == ERROR){
     state   = true;
    }
    if(responseCode == FAILURE){
     state   = true;
    }
    if(responseCode == SYNTAX_ERROR){
     state   = true;
    }
    if(responseCode == ACKNOWLEDGE){
     state   =  false;
    }
    if(responseCode == DONE){
     state   = true;
    }
    if(responseCode == INTERMEDIATE){
     state   = false;
    }
    return state;
  }
/*================================================================================================
/        boolean isCriticalError(int newResponseCode)
/=================================================================================================*/
    public boolean isCriticalError(int newResponseCode){
      int responseCode = newResponseCode;
      boolean state   = false;
      if(responseCode == ERROR){
       state   = true;
      }
      if(responseCode == FAILURE){
       state   = true;
      }
      if(responseCode == SYNTAX_ERROR){
       state   = true;
      }
      if(responseCode == ACKNOWLEDGE){
       state   =  false;
      }
      if(responseCode == DONE){
       state   = false;
      }
      if(responseCode == INTERMEDIATE){
       state   = false;
      }
      return state;
    }
/*================================================================================================
/       compactNotFound(java.lang.String responseString)
/=================================================================================================*/
public java.lang.String compactNotFound(java.lang.String responseString){
          java.lang.String compactedResponse = new java.lang.String();
          java.lang.String newString2        = new java.lang.String();
          java.lang.String newString3        = new java.lang.String();
          java.lang.String newString4        = new java.lang.String();
          newString2 = removeBracket(responseString);
          newString3 = removeQuote(newString2);
          newString4 = newString3.replaceAll("NotFound No match","NOTFOUND_NM");
          compactedResponse = newString4.replaceAll("NotFound","NOTFOUND");
    return compactedResponse;
}

/*================================================================================================
/       java.lang.String removeBracket(java.lang.String myString)
/=================================================================================================*/
  public java.lang.String removeBracket(java.lang.String myString){
    StringBuffer sb = new StringBuffer(myString);
    int idx = sb.indexOf("(");
    while (idx >= 0) {
        sb.deleteCharAt(idx);
        idx = sb.indexOf("(");
    }
    idx = sb.indexOf(")");
    while (idx >= 0) {
        sb.deleteCharAt(idx);
        idx = sb.indexOf(")");
    }
    java.lang.String finishedString = new java.lang.String(sb);
    return finishedString;
  }
/*================================================================================================
/       java.lang.String removeQuote(java.lang.String myString)
/=================================================================================================*/
  public java.lang.String removeQuote(java.lang.String myString){
    StringBuffer sb = new StringBuffer(myString);
    int idx = sb.indexOf("\"");
    while (idx >= 0) {
        sb.deleteCharAt(idx);
        idx = sb.indexOf("\"");
    }
    java.lang.String finishedString = new java.lang.String(sb);
    return finishedString;
  }

/*================================================================================================
/        trimToResponse(int commandNumber,java.lang.String newCommandString)
/=================================================================================================*/
   public java.lang.String trimToResponse(int commandNumber,java.lang.String newCommandString){
     java.lang.String commandString  = new java.lang.String();
     java.lang.String commandString2 = new java.lang.String();
     try{
         java.lang.Integer myInteger              = Integer.valueOf(commandNumber);
         java.lang.String  integerString          = myInteger.toString();
         int               commandNumberLength    = integerString.length();
         java.lang.String  commandNumberString    = newCommandString.substring(0,commandNumberLength);
         boolean           isCorrectCommandNumber = commandNumberString.regionMatches(0,integerString,0,commandNumberLength);
         java.lang.String  responseCodeString     = newCommandString.substring(commandNumberLength+1,commandNumberLength+2);
         commandString  = newCommandString.substring(commandNumberLength + 3,newCommandString.length());
         commandString2 = compactNotFound(commandString);
     }catch (Exception e) {
     }
     return commandString2;
   }
/*================================================================================================
/        java.lang.String addCommandNumber(int commandNumber,java.lang.String newCommandString)
/=================================================================================================*/
 public java.lang.String addCommandNumber(int commandNumber,java.lang.String newCommandString){
   java.lang.String commandString = new java.lang.String();
   try{
       java.lang.Integer myInteger              = Integer.valueOf(commandNumber);
       java.lang.String  integerString          = myInteger.toString();
       commandString = integerString + " " + newCommandString + terminator;
   }catch (Exception e) {
   }
   return commandString;
 }
/*================================================================================================
/         java.lang.String constructQueryList(java.lang.String[] myCommandArray)
/=================================================================================================*/
 public int evaluateResponse(int commandNumber,java.lang.String response){
   int responseCode =0;
    return responseCode;
 }
/*================================================================================================
/         java.lang.String constructPositionCommand(double RAtarget,double DECtarget)
/=================================================================================================*/
  public java.lang.String constructPositionCommand(int commandNumber,java.lang.String positionName,double RAtarget,double DECtarget){
    java.lang.String myPositionName = new java.lang.String();
    java.lang.String raString       = new java.lang.String();
    java.lang.String decString      = new java.lang.String();
    java.lang.String commandString          = new java.lang.String();
                     myPositionName = positionName;
//    107 coord.position name=track1(ra=9h31m47s dec=31d34m39s equinox=current)
//    108 coord.position name=track2(ra= 9h31m47s dec=31d34m37s equinox=current)
//    109 coord.position name=track3(ra= 9h31m47s dec=31d34m35s equinox=current)
    java.lang.Integer myCommandInteger       = Integer.valueOf(commandNumber);
    java.lang.String  integerString          = myCommandInteger.toString();
    raString       = createRAString(RAtarget);
    decString      = createDecString(DECtarget);
    commandString = integerString + " " + "coord.position name=" + positionName + "(" + "ra=" + raString + " " + "dec=" + decString + " " + "equinox=current" + ")" + terminator;
    return commandString;
  }
/*================================================================================================
/         java.lang.String constructPositionCommand(double RAtarget,double DECtarget)
/=================================================================================================*/
    public java.lang.String constructAOISetCommand(int commandNumber,java.lang.String calculationType,java.lang.String camera,java.lang.String positionName,
                                                   int threshold,java.lang.String centerPosition,int height,int width){
      java.lang.String finalCommandString          = new java.lang.String();
      //   120 coord.aoiset calculation_type=centroid camera=wfi name=track1aoi  threshold=20 center_pos=track1 height=80 width=80
      //   121 coord.aoiset calculation_type=centroid camera=wfi name=track2aoi  threshold=20 center_pos=track2 height=50 width=80
      //   122 coord.aoiset calculation_type=centroid camera=wfi name=track3aoi  threshold=20 center_pos=track3 height=50 width=120
      java.lang.Integer commandInteger       = Integer.valueOf(commandNumber);
      java.lang.Integer thresholdInteger     = Integer.valueOf(threshold);
      java.lang.Integer heightInteger        = Integer.valueOf(height);
      java.lang.Integer widthInteger         = Integer.valueOf(width);

      java.lang.String commandNumberString       = new java.lang.String();
      java.lang.String thresholdString           = new java.lang.String();
      java.lang.String heightString              = new java.lang.String();
      java.lang.String widthString               = new java.lang.String();

      commandNumberString = commandInteger.toString();
      thresholdString     = thresholdInteger.toString();
      heightString        = heightInteger.toString();
      widthString         = widthInteger.toString();

      finalCommandString = commandNumberString + " " + "coord.aoiset" + " " + "calculation_type=" + "centroid" + " " + "camera=" + camera + " " + "name=" + positionName + " " + "threshold=" + thresholdString + " " + "center_pos=" + centerPosition + " " + "height=" + heightString + " " + "width=" + widthString + terminator;
    return finalCommandString;
  }
/*================================================================================================
/         java.lang.String constructPositionCommand(double RAtarget,double DECtarget)
/=================================================================================================*/
  public java.lang.String constructPositionCommand(int commandNumber,java.lang.String positionName,double RAtarget,double DECtarget,java.lang.String centroid,
                                                   java.lang.String rof1,java.lang.String rof2){
    java.lang.String myPositionName = new java.lang.String();
    java.lang.String raString       = new java.lang.String();
    java.lang.String decString      = new java.lang.String();
    java.lang.String commandString          = new java.lang.String();
                     myPositionName = positionName;
//  130 coord.position name=target-name(ra=9h14m49s dec=31d35m54s equinox=current centroid=track1aoi rof1=track2aoi rof2=track3aoi boresight=yes inertial=yes )
    java.lang.Integer myCommandInteger       = Integer.valueOf(commandNumber);
    java.lang.String  integerString          = myCommandInteger.toString();
    raString       = createRAString(RAtarget);
    decString      = createDecString(DECtarget);
    commandString = integerString + " " + "coord.position name=" + positionName + "(" + "ra=" + raString + " " + "dec=" + decString + " " + "equinox=current" + " " +
                    "centroid=" + centroid + " " + "rof1=" + rof1 + " " + "rof2=" + rof2 + " " + "boresight=yes inertial=yes" + ")" + terminator;
    return commandString;
  }
/*================================================================================================
/         java.lang.String constructPositionCommand(double RAtarget,double DECtarget)
/=================================================================================================*/
    public java.lang.String constructPositionCommand(int commandNumber,java.lang.String positionName,double RAtarget,double DECtarget,java.lang.String centroid,
                                                     java.lang.String rof1){
      java.lang.String myPositionName = new java.lang.String();
      java.lang.String raString       = new java.lang.String();
      java.lang.String decString      = new java.lang.String();
      java.lang.String commandString          = new java.lang.String();
                       myPositionName = positionName;
//  130 coord.position name=target-name(ra=9h14m49s dec=31d35m54s equinox=current centroid=track1aoi rof1=track2aoi rof2=track3aoi boresight=yes inertial=yes )
      java.lang.Integer myCommandInteger       = Integer.valueOf(commandNumber);
      java.lang.String  integerString          = myCommandInteger.toString();
      raString       = createRAString(RAtarget);
      decString      = createDecString(DECtarget);
      commandString = integerString + " " + "coord.position name=" + positionName + "(" + "ra=" + raString + " " + "dec="+ decString + " " + "equinox=current" + " " +
                      "centroid=" + centroid + " " + "rof1=" + rof1 + " " +  "boresight=yes inertial=yes" + ")" + terminator;
      return commandString;
    }
/*================================================================================================
/         java.lang.String constructPositionCommand(double RAtarget,double DECtarget)
/=================================================================================================*/
    public java.lang.String constructPositionCommand(int commandNumber,java.lang.String positionName,double RAtarget,double DECtarget,java.lang.String centroid){
          java.lang.String myPositionName = new java.lang.String();
          java.lang.String raString       = new java.lang.String();
          java.lang.String decString      = new java.lang.String();
          java.lang.String commandString          = new java.lang.String();
                           myPositionName = positionName;
//  130 coord.position name=target-name(ra=9h14m49s dec=31d35m54s equinox=current centroid=track1aoi rof1=track2aoi rof2=track3aoi boresight=yes inertial=yes )
          java.lang.Integer myCommandInteger       = Integer.valueOf(commandNumber);
          java.lang.String  integerString          = myCommandInteger.toString();
          raString       = createRAString(RAtarget);
          decString      = createDecString(DECtarget);
          commandString = integerString + " " + "coord.position name=" + positionName + "(" + "ra=" + raString + " " + "dec=" + decString + " " + "equinox=current" + " " +
                          "centroid=" + centroid + " "  +  "boresight=yes inertial=yes" + ")" + terminator;
          return commandString;
  }
/*================================================================================================
/         constructGoToCommand(int commandNumber,java.lang.String newPositionName,java.lang.String newTrackMode)
/=================================================================================================*/
  public java.lang.String constructGoToCommand(int commandNumber,java.lang.String newPositionName,java.lang.String newTrackMode){
  java.lang.String positionName   = new java.lang.String();
  java.lang.String trackMode      = new java.lang.String();
  java.lang.String commandString  = new java.lang.String();
                   positionName   = newPositionName;
                   trackMode      = newTrackMode;
//    51 ta_pos.goto pos=target-name track_mode=On
  java.lang.Integer myCommandInteger   = Integer.valueOf(commandNumber);
  java.lang.String  integerString      = myCommandInteger.toString();
                    commandString      = integerString + " " + "ta_pos.goto" + " " + "pos=" + positionName + " " + "track_mode=" + trackMode + terminator;
  return commandString;
  }
/*================================================================================================
/     constructOffsetCommand(int commandNumber,java.lang.String newPositionName,double newDeltaRA,double newDeltaDEC)
/=================================================================================================*/
public java.lang.String constructOffsetCommand(int commandNumber,java.lang.String newPositionName,double newDeltaRA,double newDeltaDEC){
    java.lang.String positionName   = new java.lang.String();
    java.lang.String raString       = new java.lang.String();
    java.lang.String decString      = new java.lang.String();
    java.lang.String commandString  = new java.lang.String();
                     positionName   = newPositionName;
                     raString       = createRAString(newDeltaRA);
                     decString      = createDecString(newDeltaDEC);
//   ta_pos.offset delta=off_N(delta_ra=-30.0 delta_dec=-30.0)
    java.lang.Integer myCommandInteger   = Integer.valueOf(commandNumber);
    java.lang.String  integerString      = myCommandInteger.toString();
                      commandString      = integerString + " " + "ta_pos.offset" + " " + "delta=" + positionName + "(" + "delta_ra=" + raString + " " + "delta_dec=" + decString + ")" + terminator;
    return commandString;
}
/*================================================================================================
/     constructOffsetCommand(int commandNumber,java.lang.String newPositionName,double newDeltaRA,double newDeltaDEC)
/=================================================================================================*/
  public java.lang.String constructOffsetCommand(int commandNumber,java.lang.String newPositionName,int newDeltaX,int newDeltaY){
      java.lang.String positionName   = new java.lang.String();
      java.lang.String raString       = new java.lang.String();
      java.lang.String decString      = new java.lang.String();
      java.lang.String commandString  = new java.lang.String();
                       positionName   = newPositionName;
      int              deltaX         = newDeltaX;
      int              deltaY         = newDeltaY;
//   ta_pos.offset delta=off_1(delta_xsi=-67 delta_ysi=-0)
//   ta_pos.offset delta=off_N(delta_ra=-30.0 delta_dec=-30.0)
      java.lang.Integer myCommandInteger   = Integer.valueOf(commandNumber);
      java.lang.String  integerString      = myCommandInteger.toString();
                        commandString      = integerString + " " + "ta_pos.offset" + " " + "delta=" + positionName + "(" + "delta_xsi=" + deltaX + " " + "delta_ysi=" + deltaY + ")" + terminator;
      return commandString;
  }
/*================================================================================================
/         createRAString(double RA)
/=================================================================================================*/
  public java.lang.String createRAString(double RA){
     java.text.DecimalFormat df = new java.text.DecimalFormat("##0.000");
     java.lang.String raString = new java.lang.String();
     raString = df.format(RA);
     return raString;
  }
/*================================================================================================
/         createRAString(double RA)
/=================================================================================================*/
 public java.lang.String createDecString(double Dec){
   java.text.DecimalFormat df = new java.text.DecimalFormat("##0.000");
   java.lang.String decString = new java.lang.String();
   decString = df.format(Dec);
  return decString;
}
/*================================================================================================
/         parseDouble(java.lang.String myResponse)
/=================================================================================================*/
   public double parseDouble(java.lang.String myResponse){
     double myDouble = 0.0;
       java.lang.Double myDoubleValue = Double.valueOf(myResponse);
       myDouble = myDoubleValue.doubleValue();
     return  myDouble;
   }
/*================================================================================================
/         parseDouble(java.lang.String myResponse)
/=================================================================================================*/
   public java.lang.Double[] parseDoubleSet(int numberOfValues,java.lang.String myResponse){
     // This method is intentionally unfinished at this time
       java.lang.Double[]  myDoubleArray = new java.lang.Double[numberOfValues];
        return  myDoubleArray;
   }
/*================================================================================================
/         int parseInteger(java.lang.String myResponse)
/=================================================================================================*/
   public int parseInteger(java.lang.String myResponse){
        int myInt = 0;
          java.lang.Integer myIntegerValue = Integer.valueOf(myResponse);
          myInt = myIntegerValue.intValue();
        return  myInt;
   }
/*================================================================================================
/         parseRACoordinates()
/=================================================================================================*/
  public java.lang.String parseRACoordinate(java.lang.String myResponse){
    return  myResponse;
  }
/*================================================================================================
/         parseRACoordinates()
/=================================================================================================*/
  public java.lang.String parseDECCoordinate(java.lang.String myResponse){
    return  myResponse;
  }
/*================================================================================================
/         moveTelescope(int RAoffset,int DECoffset)
/=================================================================================================*/
 public java.lang.String moveTelescope(int RAoffset,int DECoffset){
    java.lang.String moveTelescopeString = new java.lang.String();
//    moveTelescopeString = moveTelescopeCommandString + " " + RAoffset + " " + DECoffset + " " + "SEC_ARC" + " " + "NOWAIT";
//    moveTelescopeString = moveTelescopeCommandString + " " + RAoffset + " " + DECoffset + " " + "SEC_ARC" + " " + "WAIT";
    return moveTelescopeString + terminator;
 }
/*================================================================================================
/         java.lang.String moveTelescope(double RAtarget,double DECtarget)
/=================================================================================================*/
  public java.lang.String moveTelescope(double RAtarget,double DECtarget){
     java.lang.String moveTelescopeString = new java.lang.String();
//    moveTelescopeString = moveTelescopeCommandString + " " + RAoffset + " " + DECoffset + " " + "SEC_ARC" + " " + "NOWAIT";
//    moveTelescopeString = moveTelescopeCommandString + " " + RAoffset + " " + DECoffset + " " + "SEC_ARC" + " " + "WAIT";
     return moveTelescopeString + terminator;
  }
/*================================================================================================
/         moveTelescope(int RAoffset,int DECoffset)
/=================================================================================================*/
 public boolean parseTelescopeMove(java.lang.String myResponse){
   boolean status = true;
   if(myResponse.equalsIgnoreCase("0")){
     status = true;
   }
   if(myResponse.equalsIgnoreCase("-5555555")){
     status = false;
   }
   if(myResponse.equalsIgnoreCase("-5555551")){
     status = false;
   }
   if(myResponse.equalsIgnoreCase("-5555552")){
     status = false;
   }
   if(myResponse.equalsIgnoreCase("-5555554")){
     status = false;
   }
   if(myResponse.equalsIgnoreCase("-5555553")){
     status = false;
   }
   return status;
 }
/*================================================================================================
/         java.lang.String constructPositionCoordinateQueryCommand(int commandNumber,java.lang.String positionName)
/=================================================================================================*/
public java.lang.String constructPositionCoordinateQueryCommand(int commandNumber,java.lang.String positionName){
       java.lang.String  commandString      = new java.lang.String();
       java.lang.Integer myCommandInteger   = Integer.valueOf(commandNumber);
       java.lang.String  integerString      = myCommandInteger.toString();
                         commandString      = integerString + " " + "get list=[" + positionName + ".ra" + " " + positionName + ".dec" + "]" + " showlabels=false" + terminator;
    return commandString;
}
/*================================================================================================
/         java.lang.Integer[] parsePositionCoordinateArray(java.lang.String newPositionCoordinateString)
/=================================================================================================*/
  public java.lang.Double[] parsePositionCoordinateArray(java.lang.String newPositionCoordinateString){
     java.lang.Double[] myCoordinateArray = new java.lang.Double[2];
     java.lang.String  myPositionCoordinateString = new java.lang.String();
     myPositionCoordinateString = newPositionCoordinateString;
     try{
       java.util.StringTokenizer tokenResponseString = new java.util.StringTokenizer(myPositionCoordinateString," ");
       java.lang.String          myRAString          = tokenResponseString.nextToken(" ");
       java.lang.String          myDecString         = tokenResponseString.nextToken(" ");
       double myRA = parseSexag(myRAString);
       double myDec = parseSexag(myDecString);

       myCoordinateArray[0] = myRA;
       myCoordinateArray[1] = myDec;
     }catch(Exception e2){
       System.out.println("Error parsing coordinates for a position. SOFIACommands.parsePositionCoordinateArray");
     }
    return myCoordinateArray;
  }
/*================================================================================================
/        parseSexag(String sexagStr)
/=================================================================================================*/
  /**
      * Given a sexag string (from MCS subscribe response) parse
      * it and return the double value. Do not know place of
      * separators (since not padded) nor the separators themselves.
      *
      * @param sexagStr String in sexag format; do not assume the first
      * element is padded, do assume the second and third are; i.e., may
      * have 4h04m04s but not 4h4m4s.
      */
     public static double parseSexag(String sexagStr) {

         // What to default to??
         double d = 0;

         boolean isNeg = sexagStr.charAt(0) == ('-');
         int idx = (isNeg) ? 1 : 0;

         // Find first separator
         for (; idx < sexagStr.length(); idx++) {
            if (!Character.isDigit(sexagStr.charAt(idx)) ) {
               break;
            }
         }

         // Get numeric values for element, assume second element
         // and non decimal part of third element are always 2 chars
         // long.
         String se[] = new String[3];
         se[0] = sexagStr.substring(0, idx);
         se[1] = sexagStr.substring(idx+1, idx+3);
         se[2] = sexagStr.substring(idx+4, sexagStr.length()-1);
         double de[] = new double[3];
         try {
            de[2] = Double.parseDouble(se[2]) / 60.0d;
         } catch(NumberFormatException nfe) {
            de[2] = 0;
         }
         try {
            de[1] = (Double.parseDouble(se[1]) + de[2])/60.0d;
         } catch(NumberFormatException nfe) {
            de[1] = 0;
         }
         try {
            d = Math.abs(Double.parseDouble(se[0])) + de[1];
         } catch(NumberFormatException nfe) {
         }
         if (isNeg) {
            d *= -1.0;
         }
         return d;
     }
/*================================================================================================
/         java.lang.String[] parsePositionListArray(java.lang.String newPositionList)
/=================================================================================================*/
   public java.lang.String[] parsePositionListArray(java.lang.String newPositionListString){
     java.lang.String   myPositionListString  = new java.lang.String();
     java.lang.String[] myPositionList        = new java.lang.String[100];
     java.lang.String[] myFinalPositionList;
                        myPositionListString  = newPositionListString;
     int                currentPositionNumber = 0;
      try{
        java.lang.String currentPositionListString = removeQuote(myPositionListString);
        int stringLength = currentPositionListString.length();
        int equalLocation = currentPositionListString.indexOf("=");
        java.lang.String currentPositionListString2 = currentPositionListString.substring(equalLocation+1,stringLength);
         java.util.StringTokenizer tokenResponseString = new java.util.StringTokenizer(currentPositionListString2," ");
         boolean listActive = true;
         while(listActive){
           try{
             myPositionList[currentPositionNumber] = tokenResponseString.nextToken(" ");
             currentPositionNumber                 = currentPositionNumber + 1;
           }catch(Exception e2){
             listActive = false;
           }
         }
      }// end of the try block
      catch(Exception e){
         System.out.println("An error occured while parsing the Positions List Command String. "+e);
      }
      myFinalPositionList       = new java.lang.String[currentPositionNumber];
      for(int i = 0;i < currentPositionNumber;i++){
        myFinalPositionList[i] = myPositionList[i];
      }
  return myFinalPositionList;
}
/*================================================================================================
/        java.lang.String constructGetSMAFocus(int newCommandNumber)
/=================================================================================================*/
 public java.lang.String constructGetSMAFocus(int newCommandNumber){
   java.lang.String command       = new java.lang.String();
   int              commandNumber = newCommandNumber;
   java.lang.Integer myCommandInteger   = Integer.valueOf(commandNumber);
   java.lang.String  integerString      = myCommandInteger.toString();
                     command      = integerString + " " + "get list=[" + "sma.focus.position" + "]" + " showlabels=false" + terminator;
return command;
 }
/*================================================================================================
/        java.lang.String constructGetSMAFocus()
/=================================================================================================*/
  public java.lang.String constructSetSMAFocus(int newCommandNumber,int newFocus){
    java.lang.String  command            = new java.lang.String();
    int               commandNumber      = newCommandNumber;
    int               focus              = newFocus;
    java.lang.Integer myCommandInteger   = Integer.valueOf(commandNumber);
    java.lang.String  integerString      = myCommandInteger.toString();
    java.lang.Integer myFocusInteger     = Integer.valueOf(focus);
    java.lang.String  focusString        = myFocusInteger.toString();
                      command      = integerString + " " + "sma.focus.position" + "=" + focusString + " showlabels=false" + terminator;
 return command;
  }
/*================================================================================================
/        java.lang.String constructOffsetSMAFocus(int newOffset)
/=================================================================================================*/
   public java.lang.String constructOffsetSMAFocus(int newCommandNumber,int newOffset){
     java.lang.String  command            = new java.lang.String();
     int               commandNumber      = newCommandNumber;
     int               offset             = newOffset;
     java.lang.Integer myCommandInteger   = Integer.valueOf(commandNumber);
     java.lang.String  integerString      = myCommandInteger.toString();
     java.lang.Integer myOffsetInteger    = Integer.valueOf(offset);
     java.lang.String  offsetString        = myOffsetInteger.toString();
                       command      = integerString + " " + "sma.focus" + " " + "offset=" + offsetString + terminator;
  return command;
   }
/*================================================================================================
/        java.lang.String constructZeroSMAFocus(int newOffset)
/=================================================================================================*/
   public java.lang.String constructZeroSMAFocus(int newCommandNumber){
     java.lang.String  command            = new java.lang.String();
     int               commandNumber      = newCommandNumber;
     java.lang.Integer myCommandInteger   = Integer.valueOf(commandNumber);
     java.lang.String  integerString      = myCommandInteger.toString();
                       command      = integerString + " " + "sma.focus.position" + "=" + "zero" + " showlabels=false" + terminator;
  return command;
   }
/*================================================================================================
/       End of the P200Commands() Class Declaration
/=================================================================================================*/
/*
REMOTE COMMANDS FOR THE PALOMAR TELESCOPE CONTROL SYSTEMS (TCS)    10/07/08


OVERVIEW

Remote commands are those commands that can be issued by another computer via
serial link or stream socket to the Telescope Control System (TCS).  The 
commands and their functions are listed below.

The command format is an ASCII string starting with the command name, followed
by the correct number of arguments and terminated with a '\r' (carriage-return
character, 015 octal).  Spaces are used to delimit fields.  Numeric arguments 
may be integer or floating-point.  Commands may be entered in upper or lower 
case.



REMOTE COMMANDS

(null command, '\r' only):  Get-header command.  (See also REQPOS and REQSTAT
commands below.)  Assembles current telescope data as native DEC (Digital 
Equipment Corp) binary numbers and returns to the caller 100 8-bit bytes.  
These are NOT ASCII characters, and must be interpreted as PDP11-format 
numbers ("shorts" are two byte short integer, LSB first;  and "floats" are 
modified IEEE format: MSB-1, MSB, LSB, LSB+1.  In other words, each 16-bit 
word is byte-swapped).  A "C" declaration for the data structure that is 
returned is provided below:

struct {				/* 100 BYTES TOTAL */ /*
        short 	ra_hours;		/* COORDINATES ALWAYS J2000.0 */
/*	short	ra_minutes;
	short	ra_seconds;
	short	ra_hundredth_seconds;
	short	dummyword_4;
	short	dec_degrees;
	short	dec_minutes;
	short	dec_seconds;
	short	dec_tenth_seconds;
	short	dec_sign;		/* 1 = POS, 0 = NEG */
/*	short	lst_hours;
	short	lst_minutes;
	short	lst_seconds;
	short	lst_tenth_seconds;
	short	dummyword_14;
	short	ha_hours;		/* HA IS APPARENT */
/*	short	ha_minutes;
	short	ha_seconds;
	short	ha_tenth_seconds;
	short	ha_direction;		/* 1 = EAST, 0 = WEST */
/*	short	ut_hours;
	short	ut_minutes;
	short	ut_seconds;
	short	ut_tenth_seconds;
	float	airmass;
	float	display_equinox;	/* J2000.0 */
/*	short	dummyword_28;
	short	dummyword_29;
	short	dummyword_30;
	short	dummyword_31;
	short	dummyword_32;
	short	dummyword_33;
	float	cass_ring_angle;
	float	telescope_focus_mm;
	float	telescope_tubelength_mm;
	float	ra_offset_arcsec;
	float	dec_offset_arcsec;
	float	ra_track_rate_arcsec;
	float	dec_track_rate_arcsec;
	short	dummyword_48;
	short	telescope_id;		/* 200 or 60 */
/*} telescope_data;

?NAME (no arguments): return the name of the currently observed position 
(the last position for which a GO command has been issued).

?PARALLACTIC (no arguments):  return the current telescope parallactic
angle in ASCII format, as follows: "PARALLACTIC = %.2f\n".  This is intended
for the Double Spectrograph instrument.

?WEATHER (no arguments): does not provide weather information!  Was created
to provide telescope information to the P200 weather system, acting as a
client.  The format of the returned string is:
----------------------------------------------------------------------------
RA=%05.2f\nDec=%+.2f\nHA=%+.2f\nLST=%05.2f\nAir mass=%06.3f\nAzimuth=%06.2f\n\
Zenith angle=%05.2f\nFocus Point=%05.2f\nDome Azimuth=%05.1f\n\
Dome shutters=%1d\nWindscreen position=%05.2f\nInstPos=%1d\nInstrument=%.12s\n\
Pumps=%1d\n
----------------------------------------------------------------------------
Apart from low precision telescope position information, the command indicates
whether the dome shutters are closed (0) or open (1), dome and windscreen
positions, and instrument name.

COORDS (five or six arguments):  supply a position to the TCS.  The arguments 
in order are  1) RA in decimal hours, 2) declination in decimal degrees,  
3) equinox of coordinates (value of zero indicates apparent equinox), 4) RA
motion, 5) dec motion, 6) motion flag.  (Motion flag absent or 0:  arguments 
4 and 5 are proper motion in RA units of .0001 sec/yr and dec units of .001 
arcsec/yr.  Motion flag = 1:  arguments 4 and 5 are offset tracking rates in 
arcsec/hr.  Motion flag = 2:  arguments 4 and 5 are offset tracking rates 
with arg 4 in seconds/hr and arg 5 in arcsec/hr.)  NOTE: a name string can 
now be provided, by adding an extra parameter enclosed in double quotes; 
this will preferably be at the end of the command line, though it can be 
inserted at any parameter position.

E (one argument):  move telescope east by number of arcseconds given in 
argument.  Rate of move is set by first argument of MRATES - see below.  
Move is precessed to display equinox.  Valid range is 0 to 6000 arcsec.

ES (one argument):  move telescope east by seconds of time given in 
argument.  Rate of move is set by first argument of MRATES - see below.  
Move is precessed to display equinox.  Valid range: 0 to 600 seconds.

F (no arguments):  move the telescope to the position established by the last
GO command (GO can only be issued from the Night Assistant's console).  This
move is done at 50 arcsec/sec.  Maximum move is 600 sec in RA and 6000 arcsec
in dec.  The telescope must be tracking stably.

FOCUSGO (one argument):  change telescope focus to the value given in the 
argument.  Valid range: 1.00 to 74.00 mm. (P200 only)

FOCUSINC (one argument):  change telescope focus by the offset given in the 
argument.  Valid range: -73.00 to 73.00 mm.  Limit checking is done. 
(P200 only)

GHEAD (one argument):  for instruments with internal mechanisms that
affect the orientation of North on the video guider display, this command
provides a way for the instrument computer to notify the TCS of changes
in orientation.  Valid range: 0 to 360 degrees.

GUIDE (two arguments):  set the RA and dec rates for the handpaddle Guide
buttons.  Valid range: 0 to 50 arcsec/sec.

MRATES (two arguments):  set rate of RA and dec moves to values given in
first and second arguments, respectively.  Valid range: 0 to 50 arcsec/sec.
Note:  these rates apply to moves as described in this note and are 
independent of rates for Night-Assistant-commanded moves.

N (one argument):  move telescope north by number of arcseconds given in 
argument.  Rate of move is set by second argument of MRATES - see above.
Move is precessed to display equinox.  Valid range: 0 to 6000 arcsec.

NPS (two arguments): control power to lamps and LWIR camera in prime focus
cage.  First argument: 0 = turn off port given in second argument; 1 = 
turn on port; 2 = get status of port ("OFF" or "ON ").  Special case:
NPS 0 0 turns off all lamps (ports 1, 2, 3).  Port allocations: 1 = low lamp;
2 = high lamp; 3 = arclamp; 7 = LWIR camera; 8 = LWIR camera shutter.

PT (two arguments):  move telescope in RA and dec simultaneously by
distance given in first and second arguments, respectively.  Distances are
in arcseconds.  Positive values move east and north.  Rates of moves are
set by MRATES - see above.  Moves are precessed to the display equinox.
Valid range: -6000 to 6000 arcsec in each axis.

PTS (two arguments):  move telescope in RA and dec simultaneously by
distance given in first and second arguments, respectively.  RA distance is
in seconds of time (range: -600 to 600 sec), dec is in arcseconds (range:
-6000 to 6000 arcsec).  Positive values move east and north.  Rates of moves 
are set by MRATES - see above.  Moves are precessed to the display equinox.

R (no arguments):  turn on offset tracking rates (non-sidereal).  The rates to
be used may be entered at the Night Assistant's console or with the RATES or 
RATESS commands - see below.

-R (no arguments):  turn off offset tracking rates.

RATES (two arguments):  define offsets to sidereal tracking rates for non-
sidereal objects.  First argument is RA rate in arcsec/hr, second argument 
is declination rate in arcsec/hr.  Rates are enabled with the R command -
see above. 

RATESS (two arguments):  define offsets to sidereal tracking rates for non-
sidereal objects.  First argument is RA rate in seconds/hr, second argument 
is declination rate in arcsec/hr.  Rates are enabled with the R command -
see above.

RAWDEC (no arguments):  returns the encoder value of declination in ASCII
format, as follows: "RAW_DEC = %.4f\n".  This is intended for LGS operations.

RAWPOS (no arguments):  returns the encoder values of hour angle (in hours)
and declination (in degrees) in ASCII format, as follows:
"HA= %.6f\nDEC= %.5f\n".  This is intended for LGS operations.

RCLEAR (no arguments):  disable offset tracking rates and set offset rates 
to zero.

REQPOS (no arguments):  obtain telescope position information in ASCII
format.  Format of returned string (dashed lines not included):
----------------------------------------------------------------------------
UTC = ddd hh:mm:ss.s, LST = hh:mm:ss.s\n
RA = hh:mm:ss.ss, DEC = [+/-]dd:mm:ss.s, HA = [W/E]hh:mm:ss.s\n
air mass = aa.aaa 
----------------------------------------------------------------------------
In UTC, ddd is UT day of year.  Fields are fixed width.  RA and DEC are 
J2000.  HA is apparent.  There is no terminating newline.  (See also Get-
header command above.)

REQSTAT (no arguments): obtain telescope status information in ASCII
format.  Format of returned string (dashed lines not included):
----------------------------------------------------------------------------
UTC = ddd hh:mm:ss.s\n
telescope ID = ttt, focus = ***** mm, tube length = tt.tt mm\n
offset RA = rrrrrrr.r arcsec, DEC = ddddddd.d arcsec\n
rate RA = rrrrrrr.r arcsec/hr, DEC = ddddddd.d arcsec/hr\n
Cass ring angle = ccc.cc
----------------------------------------------------------------------------
In UTC, ddd is UT day of year.  ID is 200 or 60.  At P200, focus format 
is ff.ff mm;  at P60 the focus reading is fffff and is in arbitrary units 
(not mm, although "mm" is displayed).  Tube length and Cass ring angle are
meaningless at P60.  Fields are fixed width.  There is no terminating 
newline.  (See also Get-header command above.)

RET (no arguments):  move the telescope so the DRA and DDEC offsets on the 
telescope status display go to zero.  (See Z command, below.)  Rate is 
50 arcsec/sec.  Maximum move: 600 sec in RA, 6000 arcsec in dec.  The 
telescope must be tracking stably.

RINGGO (one argument):  rotate the Cass ring to the position specified in 
the argument, in degrees.  If the position is available in more than one 
turn of the ring, the shortest move will be taken.  Valid range: 0 to 
359.99... degrees.

RINGMOVE (one argument):  rotate the Cass ring by the displacement specified
in the argument, in degrees.  Valid range: depends on current position with
respect to limits.  If out of range, "-3" (unable to execute) is returned.

S (one argument):  move telescope south by number of arcseconds given in 
argument.  Rate of move is set by second argument of MRATES - see above.  
Move is precessed to display equinox.  Valid range: 0 to 6000 arcsec.

SET (two arguments):  set the RA and dec rates for the handpaddle Set
buttons.  Valid range: 0 to 50 arcsec/sec.

TX (no arguments):  define the current position of the telescope to be the
position established with the last GO command (operator command only).
Similar to X -- see below, but pointing offsets are not saved, and the
operator can restore the pointing offsets associated with the last X.
DISABLED-- not available to clients; telescope operator only.

W (one argument):  move telescope west by number of arcseconds given in 
argument.  Rate of move is set by first argument of MRATES - see above.  
Move is precessed to display equinox.  Valid range: 0 to 6000 arcsec.

WS (one argument):  move telescope west by seconds of time given in 
argument.  Rate of move is set by first argument of MRATES - see above.  
Move is precessed to display equinox.  Valid range: 0 to 600 seconds.

X (no arguments):  define the current position of the telescope to be the
position established with the last GO command (operator command only).  
Normally done after GOing to a star with known good coordinates and centering 
the star in the field, to optimize local pointing.  X zeroes the displayed 
DRA and DDEC offsets.  Valid range:  change in pointing offsets less than
10 sec in RA and 300 arcsec in dec.  NOTE: this command must be used with
care -- if used at the wrong position, subsequent telescope pointing will be
bad.  DISABLED-- not available to clients; telescope operator only.

Z (no arguments):  zero the DRA and DDEC display offsets;  ie, make the 
current position of the telescope a reference point.



RETURNS

The get-header command and the ?NAME, ?PARALLACTIC, ?WEATHER, RAWDEC, RAWPOS, 
REQPOS, and REQSTAT commands return the requested information.  All other 
commands return an ASCII string to show completion status.  (Motion commands 
do not return until motion is complete.)  All ASCII strings have a null 
character appended.  The meaning of return strings is as follows:

	"0":  Successful completion.
	"-1": Unrecognized command.
	"-2": Invalid parameter(s).
	"-3": Unable to execute at this time.
	"-4": TCS Sparc host unavailable.



COMMUNICATIONS PROTOCOLS

SERIAL PROTOCOL:  9600 bps, 1 start bit, 8 data bits, 1 stop bit, no parity.
No handshaking.

STREAM SOCKET PARAMETERS:  
TCS IP address (P200): 198.202.125.194 (proxy address)
TCP service (port): 5004
[A sample client program, written in C, is available.]
*/
}
