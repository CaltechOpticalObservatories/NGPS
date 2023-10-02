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
#include <sys/stat.h>
#include <map>
#include <condition_variable>
#include <atomic>

#define MOVE_TIMEOUT 25000  ///< number of milliseconds before a move fails

/***** Slit *******************************************************************/
/**
 * @namespace Slit
 * @brief     namespace for the slit daemon
 *
 */
namespace Slit {

  const std::string DAEMON_NAME = "slitd";       ///< when run as a daemon, this is my name

  /***** Slit::ControllerInfo *************************************************/
  /**
   * @class  ControllerInfo
   * @brief  slit motor controller information class
   *
   * This class contains the information for each slit controller
   * and a function for loading the information from the configuration file.
   *
   */
  class ControllerInfo {
    public:
      int addr;                   ///< controller address
      float pos;                  ///< current position of this actuator
      std::string name;           ///< controller name
      float min;                  ///< min travel range of motor connected to this controller
      float max;                  ///< max travel range of motor connected to this controller
      float zeropos;              ///< defined zero position of motor
      bool servo;                 ///< servo state (true=on, false=off)
      bool ishome;                ///< is axis homed?
      bool ontarget;              ///< is axis on target?

      /**
       * ControllerInfo
       * class constructor
       */
      ControllerInfo() {
        this->servo=false;
        this->ishome=false;
        this->ontarget=false;
        }

      /***** Slit::ControllerInfo::load_info **********************************/
      /**
       * @fn         load_info
       * @brief      loads information from the configuration file into the class
       * @param[in]  input
       * @return     ERROR or NO_ERROR
       *
       * This function is called whenever the MOTOR_CONTROLLER key is found
       * in the configuration file, to parse and load all of the information
       * assigned by that key into the appropriate class variables.
       *
       * The input string specifies: "<address> <name> <min> <max> <zeropos>"
       *
       */
      long load_info( std::string &input ) {
        std::string function = "Slit::ControllerInfo::load_info";
        std::stringstream message;
        std::vector<std::string> tokens;

        Tokenize( input, tokens, " \"" );

        if ( tokens.size() != 5 ) {
          message.str(""); message << "ERROR bad number of tokens: " << tokens.size() << ". expected 5";
          logwrite( function, message.str() );
          return( ERROR );
        }

        try {
          this->addr    = std::stoi( tokens.at(0) );
          this->name    = tokens.at(1);
          this->min     = std::stof( tokens.at(2) );
          this->zeropos = std::stof( tokens.at(4) );
          this->max     = std::stof( tokens.at(3) ) - this->zeropos;  // max is reduced from actuator max by zeropos
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
          message.str(""); message << "ERROR: min " << this->min << " cannot be < 0";
          logwrite( function, message.str() );
          return( ERROR );
        }

        if ( this->zeropos < 0 || this->zeropos >= this->max ) {
          message.str(""); message << "ERROR: zeropos " << this->zeropos << " must be >=0 and <" << this->max;
          logwrite( function, message.str() );
          return( ERROR );
        }

        return( NO_ERROR );
      }
      /***** Slit::ControllerInfo::load_info **********************************/
  };
  /***** Slit::ControllerInfo *************************************************/


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
      std::string name;
      std::string host;
      int port;
      size_t con_A;
      size_t con_B;

      Interface();
      ~Interface();

      Common::Queue async;

      std::vector<Slit::ControllerInfo> controller_info;                            ///< vector of all daisy-chain connected controllers

      bool isopen() { return this->pi.controller.isconnected(); }                   ///< is this interface connected to hardware?

      long initialize_class();
      long open();                               ///< opens the PI socket connection
      long close();                              ///< closes the PI socket connection
      long home();                               ///< home all daisy-chained motors using the neg limit switch
      long is_home( std::string &retstring );    ///< return the home state of the motors

      long set( Slit::Interface &iface, std::string args, std::string &retstring ); ///< set the slit width and offset
      long get( std::string &retstring );                                           ///< get the current width and offset

      static void dothread_move_abs( Slit::Interface &iface, int addr, float pos ); ///< threaded move_abs function
      static void dothread_home( Slit::Interface &iface, int con ); ///< threaded home function

      long move_abs( int addr, float pos );      ///< send move-absolute command to specified controllers
      long move_rel( std::string args );         ///< send move-relative command to specified controllers
      long stop();                               ///< send the stop-all-motion command to all controllers
      long send_command( std::string cmd );      ///< writes the raw command as received to the master controller, no reply
      long send_command( std::string cmd, std::string &retstring );                 ///< writes command?, reads reply

      Physik_Instrumente::ServoInterface pi;     ///< Object for communicating with the PI

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
