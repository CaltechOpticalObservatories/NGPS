/** ---------------------------------------------------------------------------
 * @file     emulatord_acam_pi.h
 * @brief    
 * @author   David Hale <dhale@astro.caltech.edu>
 * @date     
 * @modified 
 *
 */

#ifndef EMULATORD_ACAM_PI_H
#define EMULATORD_ACAM_PI_H

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
#include "acam_pi.h"

#define  BUFSIZE      1024  ///< size of the input command buffer

/***** AcamPIEmulator *********************************************************/
/**
 * @namespace AcamPIEmulator
 * @brief     this namespace contains everything for the acam_pi emulator
 *
 */
namespace AcamPIEmulator {


  /***** AcamPIEmulator::Server ***********************************************/
  /**
   * @class  Server
   * @brief  emulator server class
   *
   * This class contains everything needed to run the emulator server,
   * the server which responds as the acam_pi controller.
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

      AcamPIEmulator::Interface interface;

      /***** AcamPIEmulator::Server *******************************************/
      /**
       * @fn         Server
       * @brief      class constructor
       * @param[in]  none
       * @return     none
       */
      Server() {
        this->port=-1;
        this->subsystem="acam_pi";
        this->cmd_num=0;
      }
      /***** AcamPIEmulator::Server *******************************************/


      /***** AcamPIEmulator::~Server ******************************************/
      /**
       * @fn         ~Server
       * @brief      class deconstructor cleans up on exit
       * @param[in]  none
       * @return     none
       */
      ~Server() {
      }
      /***** AcamPIEmulator::~Server ******************************************/


      /***** AcamPIEmulator::Server::exit_cleanly *****************************/
      /**
       * @fn         exit_cleanly
       * @brief      closes things nicely and exits
       * @param[in]  none
       * @return     none
       *
       */
      void exit_cleanly(void) {
        std::string function = "  (AcamPIEmulator::Server::exit_cleanly) ";
        std::cerr << get_timestamp() << function << "emulatord." << this->subsystem << " exiting\n";

        // close connection
        //
        if ( this->port > 0 ) close( this->port );
        exit( EXIT_SUCCESS );
      }
      /***** AcamPIEmulator::Server::exit_cleanly *****************************/


      /***** AcamPIEmulator::Server::configure_emulator ***********************/
      /**
       * @fn         configure_emulator
       * @brief      
       * @param[in]  none
       * @return     ERROR or NO_ERROR
       *
       */
      long configure_emulator() {
        std::string function = "  (AcamPIEmulator::Server::configure_emulator) ";
        std::stringstream message;
        int applied=0;
        long error;

        // loop through the entries in the configuration file, stored in config class
        //
        for ( int entry=0; entry < this->config.n_entries; entry++ ) {

          // PI_PORT
          if ( config.param[entry].compare( 0, 7, "PI_PORT" ) == 0 ) {
            this->port = std::stoi( config.arg[entry] );
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
        std::cerr << "applied " << applied << " configuration lines to emulatord." << this->subsystem << "\n";

        return error;
      }
      /***** AcamPIEmulator::Server::configure_emulator ***********************/

  };
  /***** AcamPIEmulator::Server ***********************************************/

}
/***** AcamPIEmulator *********************************************************/
#endif
