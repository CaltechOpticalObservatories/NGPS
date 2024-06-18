/** ---------------------------------------------------------------------------
 * @file     emulatord_acam.h
 * @brief    
 * @author   David Hale <dhale@astro.caltech.edu>
 *
 */

#ifndef EMULATORD_ACAM_H
#define EMULATORD_ACAM_H

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
#include "acam.h"

#define  BUFSIZE      1024  ///< size of the input command buffer

/***** AcamEmulator ***********************************************************/
/**
 * @namespace AcamEmulator
 * @brief     this namespace contains everything for the acam emulator
 *
 */
namespace AcamEmulator {


  /***** AcamEmulator::Server *************************************************/
  /**
   * @class  Server
   * @brief  emulator server class
   *
   * This class contains everything needed to run the emulator server,
   * the server which responds as the acam controller.
   *
   */
  class Server {
    private:
    public:
      int port;
      const std::string subsystem;       ///< subsystem name
      std::atomic<int> cmd_num;

      Config config;
      std::mutex conn_mutex;             ///< mutex to protect against simultaneous access to Accept()

      AcamEmulator::Interface interface;

      Server() : port(-1), subsystem("acam"), cmd_num(0) { }

      /***** AcamEmulator::Server::exit_cleanly ***********************************/
      /**
       * @fn         exit_cleanly
       * @brief      closes things nicely and exits
       * @param[in]  none
       * @return     none
       *
       */
      void exit_cleanly(void) {
        std::string function = "  (AcamEmulator::Server::exit_cleanly) ";
        std::cerr << get_timestamp() << function << "emulatord." << this->subsystem << " exiting\n";

        // close connection
        //
        if ( this->port > 0 ) close( this->port );
        exit( EXIT_SUCCESS );
      }
      /***** AcamEmulator::Server::exit_cleanly ***********************************/


      /***** AcamEmulator::Server::configure_emulator *****************************/
      /**
       * @fn         configure_emulator
       * @brief      
       * @param[in]  none
       * @return     ERROR or NO_ERROR
       *
       */
      long configure_emulator() {
        std::string function = "  (AcamEmulator::Server::configure_emulator) ";
        std::stringstream message;
        int applied=0;
        long error;

        // loop through the entries in the configuration file, stored in config class
        //
        for ( int entry=0; entry < this->config.n_entries; entry++ ) {

          // MOTOR_CONTROLLER
          if ( config.param[entry].compare( 0, 16, "MOTOR_CONTROLLER" ) == 0 ) {
            AcamEmulator::ControllerInfo c;
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
      /***** AcamEmulator::Server::configure_emulator *****************************/

  };
  /***** AcamEmulator::Server *************************************************/

}
/***** AcamEmulator ***********************************************************/
#endif
