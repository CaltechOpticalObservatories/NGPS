/**
 * @file    pi.h
 * @brief   
 * @details 
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

namespace Physik_Instrumente {


  /** ServoInterface **********************************************************/
  /**
   * @class  ServoInterface
   * @brief  servo interface class
   *
   */
  class ServoInterface {
    private:
      std::string name;        //!< a name for info purposes
      std::string host;        //!< host name for the motor controller
      int port;                //!< port number for motor controller on host
      bool initialized;

    public:
      bool is_initialized() { return this->initialized; };
      long open();
      long close();
      long get_error( int addr, int &errcode );
      long set_servo( int addr, bool state );
      long set_servo( int addr, int axis, bool state);
      long move_abs( int addr, float pos );
      long move_abs( int addr, int axis, float pos );
      long move_rel( int addr, float pos );
      long move_rel( int addr, int axis, float pos );
      long home_axis( int addr, std::string ref );
      long home_axis( int addr, int axis, std::string ref );
      long is_home( int addr, int axis, bool &state );
      long on_target( int addr, bool &retstring );
      long on_target( int addr, int axis, bool &state );
      long get_pos( int addr, float &pos );
      long get_pos( int addr, int axis, float &pos );
      long stop_motion( int addr );
      long send_command( std::string cmd );
      long send_command( std::string cmd, std::string &retstring );

      template <typename T> long parse_reply( int axis, std::string &reply, T &retval );

      ServoInterface();
      ServoInterface( std::string host, int port );
      ServoInterface( std::string name, std::string host, int port );
      ~ServoInterface();

      Network::TcpSocket controller;

  };
  /** ServoInterface **********************************************************/

}
#endif
