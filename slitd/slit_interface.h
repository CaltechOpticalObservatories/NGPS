/** ---------------------------------------------------------------------------
 * @file    slit_interface.h
 * @brief   slit interface include
 * @details defines the classes used by the slit hardware interface
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#ifndef SLIT_INTERFACE_H
#define SLIT_INTERFACE_H

#include "network.h"
#include "pi.h"
#include "logentry.h"
#include "common.h"
#include "slitd_commands.h"
#include <sys/stat.h>
#include <map>
#include <condition_variable>
#include <atomic>

#define MOVE_TIMEOUT 25000  ///< number of milliseconds before a move fails
#define HOME_TIMEOUT 25000  ///< number of milliseconds before a home fails

/***** Slit *******************************************************************/
/**
 * @namespace Slit
 * @brief     namespace for the slit daemon
 *
 */
namespace Slit {

  const std::string DAEMON_NAME = "slitd";       ///< when run as a daemon, this is my name

  /***** Slit::Interface ******************************************************/
  /**
   * @class  Interface
   * @brief  interface class for a slit device
   *
   * This class defines the interface for each slit controller and
   * contains the functions used to communicate with it.
   *
   */
  class Interface {
    private:
      bool class_initialized;
      size_t numdev;
      float maxwidth;
      float minwidth;
    public:
      size_t con_A;
      size_t con_B;

      Interface();
      ~Interface();

      Common::Queue async;

      // PI Interface class for Servo type
      //
      Physik_Instrumente::Interface<Physik_Instrumente::ServoInfo> motorinterface;

      long initialize_class();
      long open();                               ///< opens the PI socket connection
      long close();                              ///< closes the PI socket connection
      long is_open( std::string arg, std::string &retstring );     ///< are motor controllers connected?
      long home( std::string args, std::string &help );            ///< home all daisy-chained motors
      long is_home( std::string arg, std::string &retstring );    ///< return the home state of the motors

      long set( Slit::Interface &iface, std::string args, std::string &retstring ); ///< set the slit width and offset
      long get( std::string args, std::string &retstring );                         ///< get the current width and offset
      long get( std::string &retstring );                                           ///< get the current width and offset

      static void dothread_move_abs( Slit::Interface &iface, int addr, float pos ); ///< threaded move_abs function

      long move_abs( int addr, float pos );      ///< send move-absolute command to specified controllers
      long move_rel( std::string args );         ///< send move-relative command to specified controllers
      long stop();                               ///< send the stop-all-motion command to all controllers
      long send_command( std::string args, std::string &retstring );      ///< writes the raw command as received to the master controller

      std::mutex pi_mutex;                       ///< mutex to protect multi-threaded access to PI controller

      volatile std::atomic<int> motors_running;  ///< number of motors that are running in threads
      volatile std::atomic<long> thr_error;      ///< error state of threads
      std::mutex wait_mtx;                       ///< mutex object for waiting for threads
      std::condition_variable cv;                ///< condition variable for waiting for threads

  };
  /***** Slit::Interface ******************************************************/

}
/***** Slit *******************************************************************/
#endif
