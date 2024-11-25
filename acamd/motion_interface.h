/** ---------------------------------------------------------------------------
 * @file    motion_interface.h
 * @brief   acam motion interface include
 * @details The motion interface is for the ACAM filter and dust covers
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#pragma once

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

#define ACAM_POSNAME_TOLERANCE     0.01  ///< tolerance to determine posname from position

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
      size_t numdev;                                                 ///< size of motors map (number of motor controllers)
      bool class_initialized;
      std::string current_filter_name;                               ///< current filter name, updated whenever set or read

    public:
      MotionInterface() : numdev(-1), 
                          class_initialized(false),
                          current_filter_name("unknown"),
                          motorinterface( ACAMD_MOVE_TIMEOUT, ACAMD_HOME_TIMEOUT, ACAM_POSNAME_TOLERANCE ) { }
                          // timeouts are defined in common/acamd_commands.h

      // map of all motor controllers
      //
      Physik_Instrumente::Interface<Physik_Instrumente::StepperInfo> motorinterface;

      Common::Queue async;                                           ///< asynchronous message queue object

      std::mutex pi_mutex;                                           ///< mutex to protect multi-threaded access to PI controller

      volatile std::atomic<int> motors_running;                      ///< number of motors that are running in threads
      volatile std::atomic<long> thr_error;                          ///< error state of threads
      std::mutex wait_mtx;                                           ///< mutex object for waiting for threads
      std::condition_variable cv;                                    ///< condition variable for waiting for threads

      inline std::string get_current_filter_name() { return this->current_filter_name; }

      long initialize_class();
      long open();                                                   ///< opens the PI socket connection
      long close();                                                  ///< closes the PI socket connection
      bool is_open();  ///< are motor controllers connected?
      long is_open( std::string arg, std::string &retstring );  ///< are motor controllers connected?
      long home( std::string name_in, std::string &help );           ///< home specified motors
      long is_home( std::string name_in, std::string &retstring );   ///< return the home state of the specified motors
      long move_abs( std::string name, float pos );
      long send_command( const std::string &name, std::string cmd );                          ///< writes command as received to the master controller, no reply
      long send_command( const std::string &name, std::string cmd, std::string &retstring );  ///< writes command?, reads reply
      long motion( std::string args, std::string &retstring );       ///< basic motion test commands
      long filter( std::string args, std::string &retstring );       ///< set or get the filter
      long get_current_filter( std::string &currname );              ///< get current filter posname
      long get_current_filter( std::string &currname, int &currid, float &currpos );  ///< get current filter posname,id,pos
      long cover( std::string posname, std::string &retstring );  ///< set or get the dust cover

  };
  /***** Acam::MotionInterface ************************************************/

}
/***** Acam *******************************************************************/
