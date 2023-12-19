/** ---------------------------------------------------------------------------
 * @file    tcs_interface.h
 * @brief   tcs interface include
 * @details defines the classes used by the tcs hardware interface
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#ifndef TCS_INTERFACE_H
#define TCS_INTERFACE_H

#include "network.h"
#include "logentry.h"
#include "common.h"
#include "tcs_constants.h"
#include "tcsd_commands.h"
#include <sys/stat.h>
#include <map>
#include <math.h>

/***** TCS ********************************************************************/
/**
 * @namespace TCS
 * @brief     namespace for the TCS daemon
 *
 */
namespace TCS {


  /***** TCS::TcsIO ***********************************************************/
  /**
   * @class   TcsIO
   * @brief   interface class provides the actual I/O for a TCS device
   * @details There are (at least) two TCS "devices", the real and simulated.
   *          For safety, the client must specify which TCS is to be used,
   *          which prevents a casual "open" command from accidentally opening
   *          the real TCS when the simulated was intended.
   *
   * This class performs the I/O with the TCS.
   *
   */
  class TcsIO {
    public:

      /**
       * @var    tcs  pointer to Network::Interface
       * @brief  provides standard TCP/IP socket iterface
       */
      std::unique_ptr< Network::Interface > tcs;

      /**
       * convenience functions provide access to the Network::Interface functions
       */
      inline std::string device_name() const { return this->tcs->get_name(); }
      inline long open() const { return tcs->open(); }
      inline long close() const { return tcs->close(); }
      inline bool isconnected() const { return tcs->sock.isconnected(); }
      inline int fd() const { return tcs->sock.getfd(); }
      inline std::string host() const { return tcs->get_host(); }
      inline std::string name() const { return tcs->get_name(); }
      inline int port() const { return tcs->get_port(); }
      inline long send( std::string cmd ) { return tcs->send_command( cmd ); }
      inline long send( std::string cmd, std::string &retstring ) { return tcs->send_command( cmd, retstring ); }
  };
  /***** TCS::TcsIO ***********************************************************/


  /***** TCS::Interface *******************************************************/
  /**
   * @class   Interface
   * @brief   interface class for a tcs device
   * @details This is the upper-level interface. The TcsIO class provides
   *          the lover-level socket I/O.
   *
   * This class defines the main interface for each tcs controller and
   * contains the functions used to communicate with it.
   *
   */
  class Interface {
    public:
      std::string name;                            ///< the name of the currently open tcs device

      std::map< std::string, TCS::TcsIO > tcsmap;  ///< STL map of TcsIO objects indexed by name

      Interface();
      ~Interface();

      /**
       * These are the functions for communicating with the TCS
       */
      void list( std::string &retstring );
      void llist( std::string &retstring );
      long open( std::string arg, std::string &retstring );
      long isopen( std::string &retstring );
      long isopen( std::string help, std::string &retstring );
      long close();
      long get_weather_coords( std::string help, std::string &retstring );
      long get_coords( std::string help, std::string &retstring );
      long get_cass( std::string help, std::string &retstring );
      long get_dome( std::string help, std::string &retstring );
      long get_focus( std::string help, std::string &retstring );
      long get_motion( std::string help, std::string &retstring );
      long ringgo( std::string arg, std::string &retstring );
      long coords( std::string args, std::string &retstring );
      long send_command( std::string cmd, std::string &reply );
      void parse_reply_code( std::string codein, std::string &reply );

      Common::Queue async;                                 ///< asynchronous message queue object
  };
  /***** TCS::Interface *******************************************************/

}
/***** TCS ********************************************************************/
#endif
