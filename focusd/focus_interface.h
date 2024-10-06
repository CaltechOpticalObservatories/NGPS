/** ---------------------------------------------------------------------------
 * @file    focus_interface.h
 * @brief   focus interface include
 * @details defines the classes used by the focus hardware interface
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#pragma once

#include "network.h"
#include "pi.h"
#include "logentry.h"
#include "common.h"
#include "focusd_commands.h"
#include <sys/stat.h>
#include <map>
#include <condition_variable>
#include <atomic>

#define FOCUS_MOVE_TIMEOUT      5000      ///< timeout in msec for moves
#define FOCUS_HOME_TIMEOUT      5000      ///< timeout in msec for home
#define FOCUS_POSNAME_TOLERANCE    0.001  ///< tolerance to determine posname from position

/***** Focus ******************************************************************/
/**
 * @namespace Focus
 * @brief     namespace for the focus daemon
 *
 */
namespace Focus {

  const std::string DAEMON_NAME = "focusd";      ///< when run as a daemon, this is my name

  /***** Focus::Interface *****************************************************/
  /**
   * @class  Interface
   * @brief  interface class for a focus device
   *
   * This class defines the interface for each focus controller and
   * contains the functions used to communicate with it.
   *
   */
  class Interface {
    private:
      size_t numdev;
      bool class_initialized;
    public:
      Interface() : numdev(-1), motorinterface( FOCUS_MOVE_TIMEOUT, FOCUS_HOME_TIMEOUT, FOCUS_POSNAME_TOLERANCE ) {}

      Common::Queue async;

      // PI Interface class for the Stepper type
      //
      Physik_Instrumente::Interface<Physik_Instrumente::StepperInfo> motorinterface;

      long initialize_class();
      long open();                                              ///< opens the PI socket connection
      long close();                                             ///< closes the PI socket connection
      long is_open( std::string arg, std::string &retstring );  ///< are motor controllers connected?
      long home( std::string name_in, std::string &retstring ); ///< home all daisy-chained motors
      long is_home( std::string arg, std::string &retstring );  ///< return the home state of the motors

      long set( std::string args, std::string &retstring );     ///< set focus motor position
      long get( std::string name, std::string &retstring );     ///< get focus motor position

      static void dothread_move_abs( Focus::Interface &iface, int addr, float pos ); ///< threaded move_abs function

      long move_abs( int addr, float pos );      ///< send move-absolute command to specified controller
      long move_rel( std::string args );         ///< send move-relative command to specified controller
      long native( std::string args, std::string &retstring );  ///< send native command to a PI controller
      long stop();                               ///< send the stop-all-motion command to all controllers
      long send_command( const std::string &name, std::string cmd );      ///< writes the raw command as received to the master controller, no reply
      long send_command( const std::string &name, std::string cmd, std::string &retstring );  ///< writes command?, reads reply
      void make_telemetry_message( std::string &retstring );  ///< assembles a telemetry message

      long test( std::string args, std::string &retstring );

      std::mutex pi_mutex;                       ///< mutex to protect multi-threaded access to PI controller

      volatile std::atomic<int> motors_running;  ///< number of motors that are running in threads
      volatile std::atomic<long> thr_error;      ///< error state of threads

  };
  /***** Focus::Interface *****************************************************/

}
/***** Focus ******************************************************************/
