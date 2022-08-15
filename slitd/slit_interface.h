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

#define MOVE_TIMEOUT 20000

namespace Slit {

  /** ControllerInfo **********************************************************/
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
      int addr;                   //!< controller address
      float pos;                  //!< current position of this actuator
      std::string name;           //!< controller name
      float min, max;             //!< min,max travel range of motor connected to this controller
      bool servo;                 /// servo state (true=on, false=off)
      bool ishome;                /// is axis homed?
      bool ontarget;              /// is axis on target?

      ControllerInfo() {
        this->servo=false;
        this->ishome=false;
        this->ontarget=false;
        }

      long load_info( std::string &input ) {
        std::string function = "Slit::ControllerInfo::load_info";
        std::stringstream message;
        std::vector<std::string> tokens;

        Tokenize( input, tokens, " \"" );

        if ( tokens.size() != 4 ) {
          message.str(""); message << "bad number of tokens: " << tokens.size() << ". expected 4";
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
          message.str(""); message << "error loading tokens from input: " << input << ": " << e.what();
          logwrite( function, message.str() );
          return( ERROR );
        }
        catch ( std::out_of_range &e ) {
          message.str(""); message << "error loading tokens from input: " << input << ": " << e.what();
          logwrite( function, message.str() );
        }

        if ( this->addr < 1 ) {
          message.str(""); message << "error: addr " << this->addr << " cannot be < 1";
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
  };
  /** ControllerInfo **********************************************************/


  /** Interface ***************************************************************/
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
      int numdev;
      float maxwidth;
      float minwidth;
    public:
      std::string name;
      std::string host;
      int port;
      size_t leftcon;
      size_t rightcon;

      Interface();
      ~Interface();

      std::vector<Slit::ControllerInfo> controller_info;                            /// vector of all daisy-chain connected controllers

      bool isopen() { return this->pi.controller.isconnected(); }                   /// is this interface connected to hardware?

      long initialize_class();
      long open();                               /// opens the PI socket connection
      long close();                              /// closes the PI socket connection
      long home();                               /// home all daisy-chained motors using the neg limit switch
      long is_home( std::string &retstring );    /// return the home state of the motors

      long set( Slit::Interface &iface, std::string args, std::string &retstring ); /// set the slit width and offset
      long get( std::string &retstring );                                           /// get the current width and offset

      static void dothread_move_abs( Slit::Interface &iface, std::string movstr );  /// threaded move_abs function

      long move_abs( std::string args );         /// send move-absolute command to specified controllers
      long move_rel( std::string args );         /// send move-relative command to specified controllers
      long stop();                               /// send the stop-all-motion command to all controllers
      long send_command( std::string cmd );      /// writes the raw command as received to the master controller, no reply
      long send_command( std::string cmd, std::string &retstring );                 /// writes command?, reads reply

      Physik_Instrumente::ServoInterface pi;     /// Object for communicating with the PI

      std::mutex pi_mutex;                       /// mutex to protect multi-threaded access to PI controller

      volatile std::atomic<int> motors_running;  /// number of motors that are running in threads
      volatile std::atomic<long> thr_error;      /// error state of threads
      std::mutex wait_mtx;                       /// mutex object for waiting for threads
      std::condition_variable cv;                /// condition variable for waiting for threads

  };
  /** Interface ***************************************************************/

}
#endif
