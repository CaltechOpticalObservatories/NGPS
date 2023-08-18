/**
 * @file    lks.h
 * @brief   this file contains the definitions for the LKS hardware interface
 * @details interfaces to Lakeshore controllers and monitors
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#ifndef LKS_H
#define LKS_H

#include "common.h"
#include "network.h"
#include "utilities.h"

#include <string>
#include <vector>
#include <regex>

/***** LKS ********************************************************************/
/**
 * @namespace LKS
 * @brief     namespace for the LKS interface
 *
 * This namespace is meant to cover all LKS devices and will contain a class
 * for each specific device type.
 *
 */
namespace LKS {


  /***** LKS::Interface *******************************************************/
  /**
   * @class  Interface
   * @brief  interface class is generic interfacing to LKS hardware via sockets
   *
   */
  class Interface {
    private:
      std::string name;           ///< a name for info purposes
      std::string host;           ///< host name for the device
      int port;                   ///< port number for device on host
      bool initialized;           ///< has the class been initialized?

    public:
      /// has the class been initialized?
      bool is_initialized() { return this->initialized; };
      /// what is the name of the device?
      std::string get_name() { return this->name; };

      long open();                ///< open a connection to LKS device
      long close();               ///< close the connection to the LKS device
      long send_command( std::string cmd );
      long send_command( std::string cmd, std::string &retstring );

      Interface( std::string name, std::string host, int port );
      Interface();
      ~Interface();

      Network::TcpSocket sock;    ///< provides the network communication

  };
  /***** LKS::Interface *******************************************************/


}
/***** LKS ********************************************************************/
#endif
