/** ---------------------------------------------------------------------------
 * @file     emulatord_andorserver.h
 * @brief    
 * @author   David Hale <dhale@astro.caltech.edu>
 * @date     
 * @modified 
 *
 */

#ifndef EMULATORD_ANDORSERVER_H
#define EMULATORD_ANDORSERVER_H

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

#include "andorserver.h"
#include "config.h"
#include "network.h"
#include "common.h"
#include "acamd_commands.h"

#define  BUFSIZE      1024  ///< size of the input command buffer

/***** AndorServerEmulator ****************************************************/
/**
 * @namespace AndorServerEmulator
 * @brief     this namespace contains everything for the andor camera server emulator
 *
 */
namespace AndorServerEmulator {


  /***** AndorServerEmulator::Server ******************************************/
  /**
   * @class  Server
   * @brief  andor camera server emulator server class
   *
   * This class contains everything needed to run the emulator server,
   * the server which responds as the external camera server.
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

      AndorServerEmulator::Interface interface;  ///< create an emulater interface

      /***** AndorServerEmulator::Server **************************************/
      /**
       * @fn         Server
       * @brief      class constructor
       * @param[in]  none
       * @return     none
       */
      Server() {
        this->port=-1;
        this->subsystem="andorserver";
        this->cmd_num=0;
      }
      /***** AndorServerEmulator::Server **************************************/


      /***** AndorServerEmulator::~Server *************************************/
      /**
       * @fn         ~Server
       * @brief      class deconstructor cleans up on exit
       * @param[in]  none
       * @return     none
       */
      ~Server() {
      }
      /***** AndorServerEmulator::~Server *************************************/


      /***** AndorServerEmulator::Server::exit_cleanly ************************/
      /**
       * @fn         exit_cleanly
       * @brief      closes things nicely and exits
       * @param[in]  none
       * @return     none
       *
       */
      void exit_cleanly(void) {
        std::string function = "  (AndorServerEmulator::Server::exit_cleanly) ";
        std::cerr << get_timestamp() << function << "emulatord." << this->subsystem << " exiting\n";

        // close connection
        //
        if ( this->port > 0 ) close( this->port );
        exit( EXIT_SUCCESS );
      }
      /***** AndorServerEmulator::Server::exit_cleanly ************************/


      /***** AndorServerEmulator::Server::configure_emulator ******************/
      /**
       * @fn         configure_emulator
       * @brief      
       * @param[in]  none
       * @return     ERROR or NO_ERROR
       *
       */
      long configure_emulator() {
        std::string function = "  (AndorServerEmulator::Server::configure_emulator) ";
        std::stringstream message;
        int applied=0;
        long error;

        // loop through the entries in the configuration file, stored in config class
        //
        for ( int entry=0; entry < this->config.n_entries; entry++ ) {

          try {
            // CAMERASERVER_PORT
            if ( config.param[entry].compare( 0, 17, "CAMERASERVER_PORT" ) == 0 ) {
              this->port = std::stoi( config.arg[entry] );
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
      /***** AndorServerEmulator::Server::configure_emulator ******************/

  };
  /***** AndorServerEmulator::Server ******************************************/

}
/***** AndorServerEmulator ****************************************************/
#endif
