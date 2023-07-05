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
      std::string name;    ///< the name of this tcs is specified in the config file, expected to be {real|sim}

      /**
       * @struct tcshost_t
       * @brief  structure to contain tcs host name and port number
       */
      typedef struct {
        std::string host;                         ///< host name
        int port;                                 ///< port number
      } tcshost_t;

      std::map< std::string, tcshost_t > tcsmap;  ///< STL map of tcs host/port indexed by name

      Interface();
      ~Interface();

      void list( std::string &retstring );
      void llist( std::string &retstring );
      long open( std::string args, std::string &retstring );
      long isopen( std::string &retstring );
      long close();
      long get_weather_coords( std::string &retstring );
      long get_coords( std::string &retstring );
      long get_cass( std::string &retstring );
      long get_dome( std::string &retstring );
      long get_focus( std::string &retstring );
      long get_motion( std::string &retstring );
      long ringgo( std::string args, std::string &retstring );
      long coords( std::string args, std::string &retstring );
      long send_command( std::string cmd, std::string &reply );
      void parse_reply_code( std::string codein, std::string &reply );

      Common::Queue async;                                 ///< asynchronous message queue object

      Network::TcpSocket tcs;                              ///< socket object connects to the real TCS
  };
  /***** TCS::Interface *******************************************************/

}
/***** TCS ********************************************************************/
#endif
