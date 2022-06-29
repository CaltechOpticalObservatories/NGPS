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
      long move_abs( int addr, float pos );
      long move_abs( int addr, int axis, float pos );
      long move_rel( int addr, float pos );
      long move_rel( int addr, int axis, float pos );
      long home_axis( int addr );
      long home_axis( int addr, int axis );
      long is_home( int addr, std::string &retstring );
      long is_home( int addr, int axis, std::string &retstring );
      long on_target( int addr, std::string &retstring );
      long on_target( int addr, int axis, std::string &retstring );
      long get_pos( int addr, std::string &retstring );
      long get_pos( int addr, int axis, std::string &retstring );
      long stop_motion( int addr );
      long send_command( std::string cmd );
      long send_command( std::string cmd, std::string &retstring );

      ServoInterface();
      ServoInterface( std::string host, int port );
      ServoInterface( std::string name, std::string host, int port );
      ~ServoInterface();

      Network::TcpSocket controller;
  };
  /** ServoInterface **********************************************************/

}
#endif
