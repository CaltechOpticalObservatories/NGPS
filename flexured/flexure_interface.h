/** ---------------------------------------------------------------------------
 * @file    flexure_interface.h
 * @brief   flexure interface include
 * @details defines the classes used by the flexure hardware interface
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#pragma once

#include "network.h"
#include "pi.h"
#include "logentry.h"
#include "common.h"
#include "flexured_commands.h"
#include <sys/stat.h>
#include <map>
#include <condition_variable>
#include <atomic>

#define FLEXURE_MOVE_TIMEOUT      1000       ///< timeout in msec for moves
#define FLEXURE_POSNAME_TOLERANCE    0.0001  ///< tolerance to determine posname from position

/***** Flexure ****************************************************************/
/**
 * @namespace Flexure
 * @brief     namespace for the flexure daemon
 *
 */
namespace Flexure {

  const std::string DAEMON_NAME = "flexured";    ///< when run as a daemon, this is my name

  /***** Flexure::Interface ***************************************************/
  /**
   * @class  Interface
   * @brief  interface class for a flexure device
   *
   * This class defines the interface for each flexure controller and
   * contains the functions used to communicate with it.
   *
   */
  class Interface {
    private:
      size_t numdev;
      bool class_initialized;
    public:

      Interface() : numdev(-1), motorinterface( FLEXURE_MOVE_TIMEOUT, 0, FLEXURE_POSNAME_TOLERANCE ) {}

      std::map<std::string, int> telemetry_providers;  ///< map of port[daemon_name] for external telemetry providers

      Common::Queue async;

      // PI Interface class for the Piezo type
      //
      Physik_Instrumente::Interface<Physik_Instrumente::PiezoInfo> motorinterface;

      long initialize_class();
      long open();                               ///< opens the PI socket connection
      long close();                              ///< closes the PI socket connection
      long is_open( std::string arg, std::string &retstring );     ///< are motor controllers connected?

      long set( std::string args, std::string &retstring ); ///< set the slit width and offset
      long get( std::string args, std::string &retstring ); ///< get the current width and offset
      long compensate( std::string args, std::string &retstring );  ///< perform flexure compensation

      long stop();                               ///< send the stop-all-motion command to all controllers
      long send_command( const std::string &name, std::string cmd );      ///< writes the raw command as received to the master controller, no reply
      long send_command( const std::string &name, std::string cmd, std::string &retstring );                 ///< writes command?, reads reply
      void make_telemetry_message( std::string &retstring );  ///< assembles a telemetry message
      void get_external_telemetry();                          ///< collect telemetry from other daemon(s)
      long handle_json_message( std::string message_in );     ///< parses incoming telemetry messages
      long test( std::string args, std::string &retstring );                 ///< test routines

      std::mutex pi_mutex;                       ///< mutex to protect multi-threaded access to PI controller

      volatile std::atomic<int> motors_running;  ///< number of motors that are running in threads
      volatile std::atomic<long> thr_error;      ///< error state of threads
      std::mutex wait_mtx;                       ///< mutex object for waiting for threads
      std::condition_variable cv;                ///< condition variable for waiting for threads

  };
  /***** Flexure::Interface ***************************************************/

}
/***** Flexure ****************************************************************/
