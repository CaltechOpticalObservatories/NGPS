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
      std::string reftype;        ///< reference type
      bool servo;                 ///< servo state (true=on, false=off)
      bool ishome;                ///< is axis homed?
      bool ontarget;              ///< is axis on target?

      // @struct  stepper_pos_t
      // @brief   structure to contain parameters defined by STEPPER_POS in config file
      // @var     stepper_pos_t::id        internal 0-based ID number
      // @var     stepper_pos_t::position  real actuator position required to reach location ID
      // @var     stepper_pos_t::posname   name of that position (what the user will use)
      //
      struct stepper_pos_t {
        int id;
        float position;
        std::string posname;
      };

      std::map<std::string, stepper_pos_t> stepper;  ///< STL map of STEPPER_POS indexed by posname

      /**
       * @brief MotionInfo class constructor
       *
       */
      MotionInfo() {
        this->servo=false;
        this->ishome=false;
        this->ontarget=false;
        }

      /***** Acam::MotionInfo::load_controller_info ***************************/
      /**
       * @brief      loads STEPPER_CONTROLLER information from the config file into the class
       * @param[in]  input
       * @return     ERROR or NO_ERROR
       *
       * This function is called whenever the STEPPER_CONTROLLER key is found
       * in the configuration file, to parse and load all of the information
       * assigned by that key into the appropriate class variables.
       *
       * The input string specifies: "<address> <name> <reftype>"
       *
       */
      long load_controller_info( std::string &input ) {
        std::string function = "Acam::MotionInfo::load_controller_info";
        std::stringstream message;
        std::vector<std::string> tokens;

        Tokenize( input, tokens, " \"" );

        if ( tokens.size() != 3 ) {
          message.str(""); message << "ERROR bad number of tokens: " << tokens.size() << ". "
                                   << "Expected 3 { <addr> <name> <reftype> }";
          logwrite( function, message.str() );
          return( ERROR );
        }

        try {
          this->addr    = std::stoi( tokens.at(0) );
          this->name    = tokens.at(1);
          this->reftype = tokens.at(2);
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
      /***** Acam::MotionInfo::load_controller_info ***************************/


      /***** Acam::MotionInfo::load_pos_info **********************************/
      /**
       * @fn         load_pos_info 
       * @brief      loads STEPPER_POS information from the config file into the class
       * @param[in]  input
       * @return     ERROR or NO_ERROR
       *
       * This function is called whenever the STEPPER_POS key is found
       * in the configuration file, to parse and load all of the information
       * assigned by that key into the appropriate class variables.
       *
       * The input string specifies: "<name> <ID> <pos> <posname>"
       *
       */
      long load_pos_info( std::string &input ) {
        std::string function = "Acam::MotionInfo::load_pos_info";
        std::stringstream message;
        std::vector<std::string> tokens;

        Tokenize( input, tokens, " \"" );

        if ( tokens.size() != 4 ) {
          message.str(""); message << "ERROR bad number of tokens: " << tokens.size()
                                   << ". expected 4 { <name> <ID> <pos> <posname> }";
          logwrite( function, message.str() );
          return( ERROR );
        }

        int id;
        float position;
        std::string posname;

        try {
          this->name = tokens.at(0);
          id         = std::stoi( tokens.at(1) );
          position   = std::stof( tokens.at(2) );
          posname    = tokens.at(3);
        }
        catch ( std::invalid_argument &e ) {
          message.str(""); message << "ERROR loading tokens from input: " << input << ": " << e.what();
          logwrite( function, message.str() );
          return( ERROR );
        }
        catch ( std::out_of_range &e ) {
          message.str(""); message << "ERROR loading tokens from input: " << input << ": " << e.what();
          logwrite( function, message.str() );
          return( ERROR );
        }

        // ID starts at 0 (can't be negative)
        //
        if ( id < 0 ) {
          message.str(""); message << "ERROR: id " << id << " for \"" << this->name << "\" "
                                   << "must be >= 0"; 
          logwrite( function, message.str() );
          return( ERROR );
        }

        this->stepper[ posname ].id      = id;
        this->stepper[ posname].position = position;
        this->stepper[ posname ].posname = posname;

        return NO_ERROR;
      }
      /***** Acam::MotionInfo::load_pos_info **********************************/

  };
  /***** Acam::MotionInfo *****************************************************/


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
      size_t numdev;                                                 ///< size of motion_info (number of motor controllers)

    public:
      std::string name;
      std::string host;
      int port;

      MotionInterface();
      ~MotionInterface();

      Physik_Instrumente::ServoInterface pi;                         ///< Object for communicating with the PI

      std::map<std::string, Acam::MotionInfo> motion_info;           ///< map of all daisy-chain connected controllers

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
