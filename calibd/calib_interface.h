/**
 * @file    calib_interface.h
 * @brief   contains the Calib::Interface class functions
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#ifndef CALIB_INTERFACE_H
#define CALIB_INTERFACE_H

#include <map>

#include "network.h"
#include "pi.h"
#include "logentry.h"
#include "common.h"
#include "calibd_commands.h"
#include <sys/stat.h>
#include <map>
#include <condition_variable>
#include <atomic>

#define MOVE_TIMEOUT 25000  ///< number of milliseconds before a move fails

/***** Calib ******************************************************************/
/**
 * @namespace Calib
 * @brief     namespace for the calibrator
 *
 */
namespace Calib {

  /***** Calib::ControllerInfo ************************************************/
  /**
   * @class  ControllerInfo
   * @brief  calib motor controller information class
   *
   * This class contains the information for each calib motor controller
   * and a function for loading the information from the configuration file.
   *
   */
  class ControllerInfo {
    public:
      int addr;                   ///< controller address
      float pos;                  ///< current position of this actuator
      std::string name;           ///< controller name
      std::string reftype;        ///< reference type
      float openpos;              ///< open position for motor connected to this controller
      float closepos;             ///< close position for motor connected to this controller
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

      /***** Calib::ControllerInfo::load_info *********************************/
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
       * The input string specifies: "<address> <name> <reftype> <openpos> <closepos>"
       *
       */
      long load_info( std::string &input ) {
        std::string function = "Calib::ControllerInfo::load_info";
        std::stringstream message;
        std::vector<std::string> tokens;

        Tokenize( input, tokens, " \"" );

        if ( tokens.size() != 5 ) {
          message.str(""); message << "ERROR bad number of tokens: " << tokens.size() << ". expected 5";
          logwrite( function, message.str() );
          return( ERROR );
        }

        try {
          this->addr     = std::stoi( tokens.at(0) );
          this->name     = tokens.at(1);
          this->reftype  = tokens.at(2);
          this->openpos  = std::stof( tokens.at(3) );
          this->closepos = std::stof( tokens.at(4) );
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

        if ( this->openpos < 0 ) {
          message.str(""); message << "ERROR: openpos " << this->openpos << " cannot be < 0 for " << this->name;
          logwrite( function, message.str() );
          return( ERROR );
        }

        if ( this->closepos < 0 ) {
          message.str(""); message << "ERROR: closepos " << this->closepos << " cannot be < 0 for " << this->name;
          logwrite( function, message.str() );
          return( ERROR );
        }

        // The specified reftype must be one of the PI valid reftypes.
        //
        if ( std::find( Physik_Instrumente::valid_reftypes.begin(),
                        Physik_Instrumente::valid_reftypes.end(),
                        this->reftype ) == Physik_Instrumente::valid_reftypes.end() ) {
          message.str(""); message << "ERROR: reftype \"" << this->reftype << "\" invalid. Expected { ";
          for ( auto ref : Physik_Instrumente::valid_reftypes ) message << ref << " ";
          message << "}";
          logwrite( function, message.str() );
          return( ERROR );
        }

        return( NO_ERROR );
      }
      /***** Calib::ControllerInfo::load_info *********************************/
  };
  /***** Calib::ControllerInfo ************************************************/


  /***** Calib::Interface *****************************************************/
  /**
   * @class  Interface
   * @brief  interface class for the calib hardware
   *
   * This class defines the interface for all of the calib hardware and
   * contains the functions used to communicate with it.
   *
   */
  class Interface {
    private:
      bool class_initialized;
      size_t numdev;

    public:
      std::string name;
      std::string host;
      int port;

      Interface();
      ~Interface();

      long initialize_class();
      bool isopen() { return this->pi.controller.isconnected(); }    ///< is this interface connected to hardware?
      long open();                               ///< opens the PI socket connection
      long close();                              ///< closes the PI socket connection
      long home( std::string arg, std::string &help );    ///< home all daisy-chained motors
      static void dothread_home( Calib::Interface &iface, std::string name );
      long is_home( std::string &retstring );    ///< return the home state of the motors

      long get( std::string name_in, std::string &retstring );  ///< get state of named actuator(s)
      long set( std::string input, std::string &retstring );    ///< set state(s) of named actuator(s)
      long send_command( std::string cmd );      ///< writes the raw command as received to the master controller, no reply
      long send_command( std::string cmd, std::string &retstring );  ///< writes command?, reads reply

      static void dothread_move_abs( Calib::Interface &iface, std::string name, float pos ); ///< threaded move_abs function
      long move_abs( std::string name, float pos );

      Common::Queue async;                       ///< asynchronous message queue object

      std::map<std::string, ControllerInfo> controller_info;

      Physik_Instrumente::ServoInterface pi;     ///< object for communicating with the PI

      std::mutex pi_mutex;                       ///< mutex to protect multi-threaded access to PI controller

      volatile std::atomic<int> motors_running;  ///< number of motors that are running in threads
      volatile std::atomic<long> thr_error;      ///< error state of threads
      std::mutex wait_mtx;                       ///< mutex object for waiting for threads
      std::condition_variable cv;                ///< condition variable for waiting for threads

  };
  /***** Calib::Interface *****************************************************/

}
/***** Calib ******************************************************************/
#endif
