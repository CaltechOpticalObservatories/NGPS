/** ---------------------------------------------------------------------------
 * @file     emulatord_calib.h
 * @brief    
 * @author   David Hale <dhale@astro.caltech.edu>
 * @date     
 * @modified 
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

#include "calib.h"

#define  N_THREADS    10    //!< total number of threads spawned by daemon, one for blocking and the remainder for non-blocking
#define  BUFSIZE      1024  //!< size of the input command buffer
#define  CONN_TIMEOUT 3000  //<! incoming (non-blocking) connection timeout in milliseconds

namespace Emulator {

  class Server : public Calib::Interface {
    private:
    public:
      Server() {
        this->subsystem="calib";
        this->emulatorport=-1;
        this->cmd_num=0;
      }

      /** Emulator::~Server ********************************************************/
      /**
       * @fn     ~Server
       * @brief  class deconstructor cleans up on exit
       */
      ~Server() {
        close( this->emulatorport );
      }
      /** Emulator::~Server ********************************************************/

      int emulatorport;                  //!< emulator port
      std::string subsystem;             //!< subsystem name

      std::atomic<int> cmd_num;

      Config config;

      std::mutex conn_mutex;             //!< mutex to protect against simultaneous access to Accept()


      /** Emulator::Server::exit_cleanly *******************************************/
      /**
       * @fn     exit_cleanly
       * @brief  closes things nicely and exits
       * @param  none
       * @return none
       *
       */
      void exit_cleanly(void) {
        std::string function = "(Emulator::Server::exit_cleanly) ";
        std::cerr << function << "emulatord." << this->subsystem << " exiting\n";

        // close connections to daemons
        //
        close( this->emulatorport );
        exit( EXIT_SUCCESS );
      }
      /** Emulator::Server::exit_cleanly *******************************************/


      /** Emulator::Server::configure_emulator *************************************/
      /**
       * @fn     configure_emulator
       * @brief  
       * @param  none
       * @return ERROR or NO_ERROR
       *
       */
      long configure_emulator() {
        std::string function = "(Emulator::Server::configure_emulator) ";
        std::stringstream message;
        int applied=0;
        long error;

        // loop through the entries in the configuration file, stored in config class
        //
        for ( int entry=0; entry < this->config.n_entries; entry++ ) {

          // EMULATOR_PORT
          if ( config.param[entry].compare( 0, 13, "EMULATOR_PORT" ) ==0 ) {
            int port;
            try {
              port = std::stoi( config.arg[entry] );
            }
            catch ( std::invalid_argument & ) {
              std::cerr << function << "ERROR: bad EMULATOR_PORT: unable to convert to integer\n";
              return( ERROR );
            }
            catch ( std::out_of_range & ) {
              std::cerr << function << "EMULATOR_PORT number out of integer range\n";
              return( ERROR );
            }
            this->emulatorport = port;
            applied++;
          }

        } // end loop through the entries in the configuration file

        std::cerr << function ;

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
      /** Emulator::Server::configure_emulator *************************************/

  };  // end class Server

} // end namespace Emulator
#endif
