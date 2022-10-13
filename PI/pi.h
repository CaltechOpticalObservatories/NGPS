/**
 * @file    pi.h
 * @brief   this file contains the definitions for the PI interface
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#ifndef PHYSIK_INSTRUMENTE_H
#define PHYSIK_INSTRUMENTE_H

#include "common.h"
#include "network.h"
#include "utilities.h"
#include "pi_error.h"

#include <string>
#include <vector>

/***** Physik_Instrumente *****************************************************/
/**
 * @namespace Physik_Instrumente
 * @brief     namespace for Physik Instrumente hardware
 *
 * There is a class for each type of controller (servo, stepper, etc.)
 *
 */
namespace Physik_Instrumente {


  /***** ServoInterface *******************************************************/
  /**
   * @class  ServoInterface
   * @brief  servo interface class
   *
   */
  class ServoInterface {
    private:
      std::string name;        ///< a name for info purposes
      std::string host;        ///< host name for the motor controller
      int port;                ///< port number for motor controller on host
      bool initialized;

    public:
      bool is_initialized() { return this->initialized; };
      long open();                                                   ///< open a connection to the servo controller
      long close();                                                  ///< close the connection to the servo controller
      long get_error( int addr, int &errcode );                      ///< read the error status
      long set_servo( int addr, bool state );                        ///< set the servo on|off
      long set_servo( int addr, int axis, bool state);               ///< set the servo on|off for specific axis
      long move_abs( int addr, float pos );                          ///< send move command in absolute coordinates
      long move_abs( int addr, int axis, float pos );                ///< send move command in absolute coordinates for specific axis
      long move_rel( int addr, float pos );                          ///< move in relative coordinates
      long move_rel( int addr, int axis, float pos );                ///< move in relative coordinates for specific axis
      long home_axis( int addr, std::string ref );                   ///< home an axis by moving to reference switch
      long home_axis( int addr, int axis, std::string ref );         ///< home an axis by moving to reference switch for specific axis
      long is_home( int addr, int axis, bool &state );               ///< queries whether referencing has been done
      long on_target( int addr, bool &retstring );                   ///< query the on target state for given addr
      long on_target( int addr, int axis, bool &state );             ///< query the on target state for given addr and axis
      long get_pos( int addr, float &pos );                          ///< get the current position of a motor
      long get_pos( int addr, int axis, float &pos );                ///< get the current position of a motor for specific axis
      long stop_motion( int addr );                                  ///< stop all movement on all axes
      long send_command( std::string cmd );                          ///< send a command string to the controller
      long send_command( std::string cmd, std::string &retstring );  ///< send a command string to the controller

      template <typename T> long parse_reply( int axis, std::string &reply, T &retval );

      ServoInterface();
      ServoInterface( std::string host, int port );
      ServoInterface( std::string name, std::string host, int port );
      ~ServoInterface();

      Network::TcpSocket controller;                                 ///< TCP/IP socket object to communicate with controller

  };
  /***** ServoInterface *******************************************************/

}
/***** Physik_Instrumente *****************************************************/
#endif
