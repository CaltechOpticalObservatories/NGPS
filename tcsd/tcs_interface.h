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


  /***** TCS::Interface *******************************************************/
  /**
   * @class  Interface
   * @brief  interface class for a tcs device
   *
   * This class defines the interface for each tcs controller and
   * contains the functions used to communicate with it.
   *
   */
  class Interface {
    private:
      bool class_initialized;
    public:
      std::string host;
      int port;

      Interface();
      ~Interface();

      long initialize_class();
      long open();
      long close();
      long get_weather_coords( std::string &retstring );
      long get_coords( std::string &retstring );
      long get_cass( std::string &retstring );
      long get_dome( std::string &retstring );
      long get_motion( std::string &retstring );
      long ringgo( std::string args, std::string &retstring );
      long send_command( std::string cmd, std::string &reply );

      bool isopen() { return this->tcs.isconnected(); }    ///< is this interface connected to hardware?

      Common::Queue async;                                 ///< asynchronous message queue object

      Network::TcpSocket tcs;                              ///< socket object connects to the real TCS
  };
  /***** TCS::Interface *******************************************************/

}
/***** TCS ********************************************************************/
#endif
