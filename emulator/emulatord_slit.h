/** ---------------------------------------------------------------------------
 * @file     emulatord_slit.h
 * @brief    
 * @author   David Hale <dhale@astro.caltech.edu>
 *
 */

#ifndef EMULATORD_SLIT_H
#define EMULATORD_SLIT_H

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <csignal>
#include <map>
#include <mutex>
#include <thread>
#include <vector>
#include <atomic>
#include <climits>

#include "config.h"
#include "network.h"
#include "common.h"
#include "slit.h"

#define  BUFSIZE      1024  ///< size of the input command buffer

/***** SlitEmulator ***********************************************************/
/**
 * @namespace SlitEmulator
 * @brief     this namespace contains everything for the slit emulator
 *
 */
namespace SlitEmulator {


  /***** SlitEmulator::Server *************************************************/
  /**
   * @class  Server
   * @brief  emulator server class
   *
   * This class contains everything needed to run the emulator server,
   * the server which responds as the slit controller.
   *
   */
  class Server {
    private:
    public:
      int port;
      std::string subsystem;             ///< subsystem name
      std::atomic<int> cmd_num;
      Config config;
      std::mutex conn_mutex;             ///< mutex to protect against simultaneous access to Accept()

      SlitEmulator::Interface interface;

      /***** SlitEmulator::Server *************************************************/
      /**
       * @fn         Server
       * @brief      class constructor
       * @param[in]  none
       * @return     none
       */
      Server() {
        this->port=-1;
        this->subsystem="slit";
        this->cmd_num=0;
      }
      /***** SlitEmulator::Server *************************************************/


      /***** SlitEmulator::~Server ************************************************/
      /**
       * @fn         ~Server
       * @brief      class deconstructor cleans up on exit
       * @param[in]  none
       * @return     none
       */
      ~Server() {
      }
      /***** SlitEmulator::~Server ************************************************/


      /***** SlitEmulator::Server::exit_cleanly ***********************************/
      /**
       * @fn         exit_cleanly
       * @brief      closes things nicely and exits
       * @param[in]  none
       * @return     none
       *
       */
      void exit_cleanly(void) {
        std::string function = "  (SlitEmulator::Server::exit_cleanly) ";
        std::cerr << get_timestamp() << function << "emulatord." << this->subsystem << " exiting\n";

        // close connection
        //
        if ( this->port > 0 ) close( this->port );
        exit( EXIT_SUCCESS );
      }
      /***** SlitEmulator::Server::exit_cleanly ***********************************/


      /***** SlitEmulator::Server::configure_emulator *****************************/
      /**
       * @fn         configure_emulator
       * @brief      
       * @param[in]  none
       * @return     ERROR or NO_ERROR
       *
       */
      long configure_emulator() {
        std::string function = "  (SlitEmulator::Server::configure_emulator) ";
        std::stringstream message;
        int applied=0;
        long error;

        // loop through the entries in the configuration file, stored in config class
        //
        for ( int entry=0; entry < this->config.n_entries; entry++ ) {

          // MOTOR_CONTROLLER
          if ( config.param[entry].compare( 0, 16, "MOTOR_CONTROLLER" ) == 0 ) {
            SlitEmulator::ControllerInfo c;
            if ( c.load_info( config.arg[entry] ) == NO_ERROR ) {
              this->interface.controller_info.push_back( c );
              std::cerr << get_timestamp() << function << "loaded config: " << config.param[entry] << "=" << config.arg[entry] << "\n";
              applied++;
            }
          }

          // EMULATOR_PORT
          if ( config.param[entry].find( "EMULATOR_PORT" ) == 0 ) {
            this->port = std::stoi( config.arg[entry] );
            std::cerr << get_timestamp() << function << "loaded config: " << config.param[entry] << "=" << config.arg[entry] << "\n";
            applied++;
          }

        } // end loop through the entries in the configuration file

        std::cerr << get_timestamp() << function ;

        if ( applied==0 ) {
          std::cerr << "ERROR: " ;
          error = ERROR;
        } 
        else {
          error = NO_ERROR;
        } 
        std::cerr << get_timestamp() << function << "applied " << applied << " configuration lines to emulatord." 
                  << this->subsystem << "\n";

        return error;
      }
      /***** SlitEmulator::Server::configure_emulator *****************************/

  };
  /***** SlitEmulator::Server *************************************************/

}
/***** SlitEmulator ***********************************************************/
#endif
