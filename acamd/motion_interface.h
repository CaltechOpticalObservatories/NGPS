/** ---------------------------------------------------------------------------
 * @file    motion_interface.h
 * @brief   acam motion interface include
 * @details The motion interface is for the ACAM filter and dust covers
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#ifndef ACAM_MOTION_INTERFACE_H
#define ACAM_MOTION_INTERFACE_H

#include "network.h"
#include "pi.h"
#include "logentry.h"
#include "common.h"
#include "acamd_commands.h"
#include "utilities.h"
#include <sys/stat.h>
#include <map>
#include <condition_variable>
#include <atomic>
#include <cmath>  // for NAN

#define MOVE_TIMEOUT 20000   ///< timeout in sec before a move fails
#define HOME_TIMEOUT 45000   ///< timeout in sec before a home fails

/***** Acam *******************************************************************/
/**
 * @namespace Acam
 * @brief     namespace for acquisition and guide camera
 *
 */
namespace Acam {

  /***** Acam::MotionInterface ************************************************/
  /**
   * @class  MotionInterface
   * @brief  acam motion interface class
   *
   * This class defines the interface to the PI controllers used for the
   * filter wheel and dust cover.
   *
   */
  class MotionInterface {
    private:
      bool class_initialized;
      size_t numdev;                                                 ///< size of motors map (number of motor controllers)

    public:
      std::string name;
      std::string host;
      int port;

      MotionInterface();
      ~MotionInterface();

      Physik_Instrumente::Interface pi;                              ///< Object for communicating with the PI

      // map of all daisy-chain connected motor controllers
      std::map<std::string, Physik_Instrumente::ControllerInfo<Physik_Instrumente::StepperInfo>> motormap;

      Common::Queue async;                                           ///< asynchronous message queue object

      std::mutex pi_mutex;                                           ///< mutex to protect multi-threaded access to PI controller

      volatile std::atomic<int> motors_running;                      ///< number of motors that are running in threads
      volatile std::atomic<long> thr_error;                          ///< error state of threads
      std::mutex wait_mtx;                                           ///< mutex object for waiting for threads
      std::condition_variable cv;                                    ///< condition variable for waiting for threads

      long initialize_class();
      bool isopen() { return this->pi.controller.isconnected(); }    ///< is this interface connected to hardware?
      long open();                                                   ///< opens the PI socket connection
      long close();                                                  ///< closes the PI socket connection
      long home( std::string name_in, std::string &help );           ///< home specified motors
      static void dothread_home( Acam::MotionInterface &iface, std::string name );
      long is_home( std::string name_in, std::string &retstring );   ///< return the home state of the specified motors
      long move_abs( std::string name, float pos );
      long send_command( std::string cmd );                          ///< writes command as received to the master controller, no reply
      long send_command( std::string cmd, std::string &retstring );  ///< writes command?, reads reply
      long motion( std::string args, std::string &retstring );       ///< basic motion test commands
      long filter( std::string args, std::string &retstring );       ///< set or get the filter
      long current_filter( std::string &currname );                 ///< get current filter posname
      long current_filter( std::string &currname, int &currid, float &currpos );  ///< get current filter posname
      long cover( std::string posname, std::string &retstring );  ///< set or get the dust cover

  };
  /***** Acam::MotionInterface ************************************************/

}
/***** Acam *******************************************************************/
#endif
