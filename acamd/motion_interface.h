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
#include <sys/stat.h>
#include <map>
#include <condition_variable>
#include <atomic>

#define MOVE_TIMEOUT 20000   ///< timeout in sec before a move fails

/***** Acam *******************************************************************/
/**
 * @namespace Acam
 * @brief     namespace for acquisition and guide camera
 *
 */
namespace Acam {

  /***** Acam::MotionInfo *****************************************************/
  /**
   * @class  MotionInfo
   * @brief  acam motor controller information class
   *
   * This class contains the information for each acam controller
   * and a function for loading the information from the configuration file.
   *
   */
  class MotionInfo {
    public:
      int addr;                   ///< controller address
      float pos;                  ///< current position of this actuator
      std::string name;           ///< controller name
      float min, max;             ///< min,max travel range of motor connected to this controller
      bool servo;                 ///< servo state (true=on, false=off)
      bool ishome;                ///< is axis homed?
      bool ontarget;              ///< is axis on target?

      MotionInfo() {
        this->servo=false;
        this->ishome=false;
        this->ontarget=false;
        }

      /***** Acam::MotionInfo::load_info **************************************/
      /**
       * @brief      loads information from the configuration file into the class
       * @param[in]  input
       * @return     ERROR or NO_ERROR
       *
       * This function is called whenever the MOTOR_CONTROLLER key is found
       * in the configuration file, to parse and load all of the information
       * assigned by that key into the appropriate class variables.
       *
       * The input string specifies: "<address> <name> <min> <max>"
       *
       */
      long load_info( std::string &input ) {
        std::string function = "Acam::MotionInfo::load_info";
        std::stringstream message;
        std::vector<std::string> tokens;

        Tokenize( input, tokens, " \"" );

        if ( tokens.size() != 4 ) {
          message.str(""); message << "ERROR bad number of tokens: " << tokens.size() << ". expected 4";
          logwrite( function, message.str() );
          return( ERROR );
        }

        try {
          this->addr = std::stoi( tokens.at(0) );
          this->name = tokens.at(1);
          this->min =  std::stof( tokens.at(2) );
          this->max =  std::stof( tokens.at(3) );
        }
        catch ( std::invalid_argument &e ) {
          message.str(""); message << "ERROR loading tokens from input: " << input << ": " << e.what();
          logwrite( function, message.str() );
          return( ERROR );
        }
        catch ( std::out_of_range &e ) {
          message.str(""); message << "ERROR loading tokens from input: " << input << ": " << e.what();
          logwrite( function, message.str() );
        }

        if ( this->addr < 1 ) {
          message.str(""); message << "ERROR: addr " << this->addr << " cannot be < 1";
          logwrite( function, message.str() );
          return( ERROR );
        }

        if ( this->min < 0 ) {
          message.str(""); message << "error: min " << this->min << " cannot be < 0";
          logwrite( function, message.str() );
          return( ERROR );
        }

        return( NO_ERROR );
      }
      /***** Acam::MotionInfo::load_info **************************************/
  };
  /***** Acam::MotionInfo *****************************************************/


  /***** Acam::MotionInterface ************************************************/
  /**
   * @class  MotionInterface
   * @brief  acam motion interface class
   *
   * This class defines the interface to the PI controllers used for the filter and dust cover.
   *
   */
  class MotionInterface {
    private:
      bool class_initialized;
      size_t numdev;                                                 ///< size of motion_info (number of motor controllers)

    public:
      std::string name;
      std::string host;
      int port;

      MotionInterface();
      ~MotionInterface();

      Physik_Instrumente::ServoInterface pi;                         ///< Object for communicating with the PI

      std::vector<Acam::MotionInfo> motion_info;                     ///< vector of all daisy-chain connected controllers

      std::mutex pi_mutex;                                           ///< mutex to protect multi-threaded access to PI controller

      volatile std::atomic<int> motors_running;                      ///< number of motors that are running in threads
      volatile std::atomic<long> thr_error;                          ///< error state of threads
      std::mutex wait_mtx;                                           ///< mutex object for waiting for threads
      std::condition_variable cv;                                    ///< condition variable for waiting for threads

      long initialize_class();
      bool isopen() { return this->pi.controller.isconnected(); }    ///< is this interface connected to hardware?
      long open();                                                   ///< opens the PI socket connection
      long close();                                                  ///< closes the PI socket connection
      long home();                                                   ///< home all daisy-chained motors using the neg limit switch
      long is_home( std::string &retstring );                        ///< return the home state of the motors
      long send_command( std::string cmd );                          ///< writes command as received to the master controller, no reply
      long send_command( std::string cmd, std::string &retstring );  ///< writes command?, reads reply
      long filter( Acam::MotionInterface &iface, std::string args, std::string &retstring ); ///< set or get the filter
      long cover( Acam::MotionInterface &iface, std::string args, std::string &retstring );  ///< set or get the dust cover

  };
  /***** Acam::MotionInterface ************************************************/

}
/***** Acam *******************************************************************/
#endif
