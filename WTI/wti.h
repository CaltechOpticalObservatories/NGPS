/**
 * @file    wti.h
 * @brief   this file contains the definitions for the WTI hardware interface
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#ifndef WTI_H
#define WTI_H

#include "common.h"
#include "network.h"
#include "utilities.h"

#include <string>
#include <vector>
#include <regex>

/***** WTI ********************************************************************/
/**
 * @namespace WTI
 * @brief     namespace for the WTI interface
 *
 * This namespace is meant to cover all WTI devices and will contain a class
 * for each specific device type.
 *
 */
namespace WTI {


  /***** WTI::Interface *******************************************************/
  /**
   * @class  Interface
   * @brief  interface class is generic interfacing to WTI hardware via sockets
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

      long open();                ///< open a connection to WTI device
      long close();               ///< close the connection to the WTI device
      long send_command( std::string cmd );
      long send_command( std::string cmd, std::string &reply );
      long do_send_command( std::string cmd, std::string &retstring );

      Interface( std::string name, std::string host, int port );
      Interface();
      ~Interface();

      Network::TcpSocket sock;    ///< provides the network communication

  };
  /***** WTI::Interface *******************************************************/


  /***** WTI::NPS *************************************************************/
  /**
   * @class  NPS
   * @brief  NPS class is for communicating with the WTI NPS
   *
   * This class contains the NPS-specific functions, and
   * includes an Interface object to provide for the actual communication.
   *
   */
  class NPS {
    private:
    public:
      NPS();
      ~NPS();

      Interface interface;        ///< interface object provides the socket layer for communication

      bool isconnected() { return this->interface.sock.isconnected(); }  ///< is a socket connection open to hardware?
      long set_switch( int plugnum, int action );                        ///< turn the selected plug on|off
      long get_switch( int plugnum, std::string &state );                ///< get the selected plug's on/off state
      long get_all( int maxplugs, std::string &state );                  ///< get on/off state of all plugs
  };
  /***** WTI::NPS *************************************************************/

}
/***** WTI ********************************************************************/
#endif
