/** ---------------------------------------------------------------------------
 * @file    focus_interface.h
 * @brief   focus interface include
 * @details defines the classes used by the focus hardware interface
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#ifndef FOCUS_INTERFACE_H
#define FOCUS_INTERFACE_H

#include "network.h"
#include "pi.h"
#include "logentry.h"
#include "common.h"
#include "focusd_commands.h"
#include <sys/stat.h>
#include <map>
#include <condition_variable>
#include <atomic>

#define MOVE_TIMEOUT 25000  ///< number of milliseconds before a move fails
#define HOME_TIMEOUT 25000  ///< number of milliseconds before a home fails

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
      std::string name;
      std::string host;
      int port;

      Interface() : numdev(-1) {}

      Common::Queue async;

      // map of all daisy-chain connected motor controllers, indexed by name
      //
      std::map<std::string, Physik_Instrumente::ControllerInfo<Physik_Instrumente::StepperInfo>> motormap;

      bool isopen() { return this->pi.controller.isconnected(); }                   ///< is this interface connected to hardware?

      long initialize_class();
      long open();                               ///< opens the PI socket connection
      long close();                              ///< closes the PI socket connection
      long home( std::string args, std::string &help );            ///< home all daisy-chained motors
      long is_home( std::string &retstring );    ///< return the home state of the motors

      long set( Focus::Interface &iface, std::string args, std::string &retstring ); ///< set the slit width and offset
      long get( std::string &retstring );                                           ///< get the current width and offset

      static void dothread_move_abs( Focus::Interface &iface, int addr, float pos ); ///< threaded move_abs function
      static void dothread_home( Focus::Interface &iface, std::string name ); ///< threaded home function

      long move_abs( int addr, float pos );      ///< send move-absolute command to specified controllers
      long move_rel( std::string args );         ///< send move-relative command to specified controllers
      long stop();                               ///< send the stop-all-motion command to all controllers
      long send_command( std::string cmd );      ///< writes the raw command as received to the master controller, no reply
      long send_command( std::string cmd, std::string &retstring );                 ///< writes command?, reads reply

      Physik_Instrumente::Interface pi;          ///< Object for communicating with the PI

      std::mutex pi_mutex;                       ///< mutex to protect multi-threaded access to PI controller

      volatile std::atomic<int> motors_running;  ///< number of motors that are running in threads
      volatile std::atomic<long> thr_error;      ///< error state of threads
      std::mutex wait_mtx;                       ///< mutex object for waiting for threads
      std::condition_variable cv;                ///< condition variable for waiting for threads

  };
  /***** Focus::Interface *****************************************************/

}
/***** Focus ******************************************************************/
#endif
