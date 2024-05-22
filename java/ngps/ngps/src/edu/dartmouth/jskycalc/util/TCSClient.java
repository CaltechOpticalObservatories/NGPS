package edu.dartmouth.jskycalc.util;
//=== File Prolog =============================================================
//	This code was developed by John Thorstensen of Dartmouth University
//
//      It has been refactored and modified by Jennifer Milburn for
//      use at the Palomar Observatory.
//
//      Caltech Optical Observatories, Palomar Observatory
//
//--- Contents ----------------------------------------------------------------
//	 TCSClient
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
import java.io.*;
import java.net.*; // needed for sockets.
import java.util.*;
import java.util.Scanner;
import java.util.regex.*;
import java.util.List.*;
import java.awt.*;
import java.awt.print.*;
import java.awt.event.*;
import java.awt.geom.*;
import java.awt.font.*;
// import java.awt.PointerInfo.*;
// import java.awt.MouseInfo.*;
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.table.*;
import javax.swing.text.html.*;
import javax.swing.border.*;
/*================================================================================================
/        TCSClient () Class Declaration
/=================================================================================================*/
// for sending commands to XTCS (2.4m only).

public class TCSClient {
   static int tcsport = 6127;  // hard-wired
//   static String xtcshostname = "minuet";   // for testing
//   static String xtcshostname = "partita";   // for testing
   static String xtcshostname = "hiltner.kpno.noao.edu";  // for real
   InetAddress address;
   DatagramSocket socket = null;
   DatagramPacket packet;
   String replystr;
   int seqno = 1; // not clear this is used
   byte [] sendBuf;
   double ra_send, dec_send, equinox_send;   // to send to xtcs
   double ra_read, dec_read, equinox_read;   // read from xtcs
/*================================================================================================
/
/=================================================================================================*/
   public TCSClient() {  // set up socket, load address.
      try {
          socket = new DatagramSocket(); // should this be persistent?
          socket.setSoTimeout(5000);     // 5 seconds.
      }
      catch (Exception e) {
          System.out.printf("Socket exception.");
      }
      try {
          address = InetAddress.getByName(xtcshostname);
      } catch (Exception e) { System.out.printf("Problem resolving address.\n");}

      // System.out.printf("xmishost %s resolves to %s.\n",xmishostname,address);
      sendBuf = new byte[256];
      seqno = 1;
      ra_send = 0.;  dec_send = 0.; equinox_send = 2000.;
      ra_read = -1.; dec_read = -1.; equinox_read = 2000.;
      replystr = "";
   }
/*================================================================================================
/
/=================================================================================================*/
   public void send_msg(String cmdstr) {
      try {
          sendBuf = cmdstr.getBytes("US-ASCII");
      } catch (Exception e) { System.out.printf("getBytes exception.\n"); }

      packet = new DatagramPacket(sendBuf, sendBuf.length, address, tcsport);

      // System.out.printf("about to send %s %s\n ",packet,new String(sendBuf));
      try {
         socket.send(packet);
      } catch (IOException e) { System.out.printf("socket send exception.\n"); }

   }
/*================================================================================================
/
/=================================================================================================*/
   public int receive_msg() {
      try {
         DatagramPacket inpack = new DatagramPacket(new byte[256],256);
         socket.receive(inpack);
         replystr = new String(inpack.getData());
         // System.out.printf("Sockettest received %s from %s.\n",
         //    replystr,inpack.getAddress());
         return 0;
      }
      catch (Exception e) {
         System.out.printf("Reply timed out after 5000 msec.\n");
         replystr = "";
         return -1;
      }
   }
/*================================================================================================
/
/=================================================================================================*/
   public void closeout() {
      socket.close();
   }
/*================================================================================================
/
/=================================================================================================*/
   public int query_xtcs() {
      String reqstr;
      String [] fields;
      int reply_recvd = -1;

//      java.util.Arrays.fill(sendBuf,(byte)0);  // blank byte array for this one.

      reqstr = String.format("%d %d 1 getcoords",socket.getLocalPort(),seqno);
      send_msg(reqstr);
      reply_recvd = receive_msg();
      if(reply_recvd > -1) {
         System.out.printf("replystr = %s\n",replystr);
         fields = replystr.split("\\s+");
         try {
             System.out.printf("fields.length = %d\n",fields.length);
             ra_read = Double.parseDouble(fields[3]);
             dec_read = Double.parseDouble(fields[4]);
             equinox_read = Double.parseDouble(fields[5]);
         }
         catch (Exception e) {
             System.out.printf("Couldn't parse XTCS's reply to get coords.\n");
             return -1;
         }
      }
      return 0;
   }
/*================================================================================================
/
/=================================================================================================*/
   public int send_coords() {  // send next object coords without checking.
       String reqstr;

       reqstr = String.format("%d %d 9 SETCOORDS %8.5f %8.4f %7.2f",socket.getLocalPort(),seqno,
            ra_send, dec_send, equinox_send);
       send_msg(reqstr);
       System.out.printf("%s - sent.\n",reqstr);

       return 0;
   }
}