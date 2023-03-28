/** ---------------------------------------------------------------------------
 * @file    telem_interface.h
 * @brief   telemetry interface include
 * @details defines the classes used by the telemetry interface
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#ifndef TELEM_INTERFACE_H
#define TELEM_INTERFACE_H

#include "network.h"
#include "logentry.h"
#include "common.h"
#include "telemd_commands.h"
#include "config.h"

/***** Telemetry **************************************************************/
/**
 * @namespace Telemetry
 * @brief     namespace for acquisition and guide camera
 *
 */
namespace Telemetry {

  const std::string DAEMON_NAME = "telemd";      ///< when run as a daemon, this is my name

  /***** Telemetry::Module ****************************************************/
  /**
   * @class    Module
   * @brief    module class for telemd
   * @details  contains all of the information for each module, how to communicate
   *           with it and holds its data
   *
   */
  class Module {
    private:
    public:
      Module();
      ~Module();

      std::string name;                           ///< this module name

      std::vector<std::string> header;            ///< all possible keys for a record

      std::map<std::string, std::string> record;  ///< each record stores key, value

      Network::TcpSocket socket;                  ///< socket object for communicating with daemons

      long get_header();
  };

  /***** Telemetry::Interface *************************************************/
  /**
   * @class  Interface
   * @brief  interface class for telemd
   *
   */
  class Interface {
    private:
      bool class_initialized;

    public:

      Interface();
      ~Interface();

      Common::Queue async;                     /// asynchronous message queue

      long configure_interface( Config &config );  /// optional configuration
  };
  /***** Telemetry::Interface *************************************************/

}
/***** Telemetry **************************************************************/
#endif
