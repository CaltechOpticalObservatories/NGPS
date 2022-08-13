/** ---------------------------------------------------------------------------
 * @file     emulatord_tcs.h
 * @brief    
 * @author   David Hale <dhale@astro.caltech.edu>
 * @date     
 * @modified 
 *
 */

#ifndef EMULATORD_TCS_H
#define EMULATORD_TCS_H

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
#include "tcs.h"
#include "tcsd_commands.h"

#define  BUFSIZE      1024  //!< size of the input command buffer

namespace Emulator {


  /** Server ******************************************************************/
  /**
   * @class  Server
   * @brief  emulator server class
   *
   * This class contains everything needed to run the emulator server,
   * the server which responds as the TCS.
   *
   */
  class Server {
    private:
    public:
      int port;
      std::string subsystem;             //!< subsystem name
      std::atomic<int> cmd_num;
      Config config;
      std::mutex conn_mutex;             //!< mutex to protect against simultaneous access to Accept()

      Tcs::Interface interface;

      /** Emulator::Server ****************************************************/
      /**
       * @fn     Server
       * @brief  class constructor
       */
      Server() {
        this->port=-1;
        this->subsystem="tcs";
        this->cmd_num=0;
      }
      /** Emulator::Server ****************************************************/


      /** Emulator::~Server ***************************************************/
      /**
       * @fn     ~Server
       * @brief  class deconstructor cleans up on exit
       */
      ~Server() {
      }
      /** Emulator::~Server ***************************************************/


      /** Emulator::Server::exit_cleanly **************************************/
      /**
       * @fn     exit_cleanly
       * @brief  closes things nicely and exits
       * @param  none
       * @return none
       *
       */
      void exit_cleanly(void) {
        std::string function = " (Emulator::Server::exit_cleanly) ";
        std::cerr << get_timestamp() << function << "emulatord." << this->subsystem << " exiting\n";

        // close connection
        //
        if ( this->port > 0 ) close( this->port );
        exit( EXIT_SUCCESS );
      }
      /** Emulator::Server::exit_cleanly **************************************/


      /** Emulator::Server::configure_emulator ********************************/
      /**
       * @fn     configure_emulator
       * @brief  
       * @param  none
       * @return ERROR or NO_ERROR
       *
       */
      long configure_emulator() {
        std::string function = " (Emulator::Server::configure_emulator) ";
        std::stringstream message;
        int applied=0;
        long error;

        // loop through the entries in the configuration file, stored in config class
        //
        for ( int entry=0; entry < this->config.n_entries; entry++ ) {

          try {
            // EMULATOR_PORT
            if ( config.param[entry].compare( 0, 13, "EMULATOR_PORT" ) == 0 ) {
              this->port = std::stoi( config.arg[entry] );
              applied++;
            }

            // EMULATOR_SLEWRATE_RA
            if ( config.param[entry].compare( 0, 20, "EMULATOR_SLEWRATE_RA" ) == 0 ) {
              this->interface.telescope.slewrate_ra = std::stof( config.arg[entry] );
              applied++;
            }

            // EMULATOR_SLEWRATE_DEC
            if ( config.param[entry].compare( 0, 21, "EMULATOR_SLEWRATE_DEC" ) == 0 ) {
              this->interface.telescope.slewrate_dec = std::stof( config.arg[entry] );
              applied++;
            }

            // EMULATOR_SLEWRATE_CASANGLE
            if ( config.param[entry].compare( 0, 26, "EMULATOR_SLEWRATE_CASANGLE" ) == 0 ) {
              this->interface.telescope.slewrate_casangle = std::stof( config.arg[entry] );
              applied++;
            }

            // EMULATOR_SETTLE_RA
            if ( config.param[entry].compare( 0, 18, "EMULATOR_SETTLE_RA" ) == 0 ) {
              this->interface.telescope.settle_ra = std::stof( config.arg[entry] );
              applied++;
            }

            // EMULATOR_SETTLE_DEC
            if ( config.param[entry].compare( 0, 19, "EMULATOR_SETTLE_DEC" ) == 0 ) {
              this->interface.telescope.settle_dec = std::stof( config.arg[entry] );
              applied++;
            }

            // EMULATOR_SETTLE_CASANGLE
            if ( config.param[entry].compare( 0, 24, "EMULATOR_SETTLE_CASANGLE" ) == 0 ) {
              this->interface.telescope.settle_casangle = std::stof( config.arg[entry] );
              applied++;
            }
          }
          catch ( std::invalid_argument &e ) {
            std::cerr << get_timestamp() << function << "ERROR interpreting string as number for " << config.arg[entry] << "\n";
            return( ERROR );
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
        std::cerr << "applied " << applied << " configuration lines to emulatord." << this->subsystem << "\n";

        return error;
      }
      /** Emulator::Server::configure_emulator ********************************/

  };
  /** Server ******************************************************************/

} // end namespace Emulator
#endif
