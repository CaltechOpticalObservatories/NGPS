/**
 * @file    galil.h
 * @brief   this file contains the definitions for the Galil hardware interface
 * @details interfaces to Lakeshore controllers and monitors
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#pragma once

#include "common.h"
#include "network.h"
#include "utilities.h"

#include <string>
#include <vector>
#include <regex>
#include <mutex>

/***** Galil ******************************************************************/
/**
 * @namespace Galil
 * @brief     namespace for the Galil interface
 *
 * This namespace is meant to cover all Galil devices and will contain a class
 * for each specific device type.
 *
 */
namespace Galil {


  /***** Galil::Interface *****************************************************/
  /**
   * @class  Interface
   * @brief  interface class is generic interfacing to Galil hardware via sockets
   *
   */
  class Interface {
    private:
      std::string name;           ///< a name for info purposes
      std::string host;           ///< host name for the device
      int port;                   ///< port number for device on host
      bool is_initialized;        ///< has the class been initialized?
      std::mutex mtx;

    public:
      /// has the class been initialized?
      bool get_initialized() const { return this->is_initialized; };
      /// what is the name of the device?
      std::string get_name() const { return this->name; };

      long open();                ///< open a connection to Galil device
      long close();               ///< close the connection to the Galil device
      long send_command( std::string cmd );
      long send_command( std::string cmd, std::string &retstring );

      inline bool isopen() { std::lock_guard<std::mutex> lock( this->mtx ); return this->sock.isconnected(); }

      Interface(const std::string &name, const std::string &host, const int port);
      Interface();
      ~Interface();

      Network::TcpSocket sock;    ///< provides the network communication

  };
  /***** Galil::Interface *****************************************************/

}
/***** Galil ******************************************************************/
