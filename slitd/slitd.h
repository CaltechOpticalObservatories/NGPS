/** ---------------------------------------------------------------------------
 * @file     slitd.h
 * @brief    slit daemon include file
 * @details  defines the classes used by the slit daemon
 * @author   David Hale <dhale@astro.caltech.edu>
 *
 */

#ifndef SLITD_H
#define SLITD_H

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

#include "logentry.h"
#include "config.h"
#include "network.h"
#include "common.h"
#include "slit_interface.h"
#include "slitd_commands.h"

#define  N_THREADS    10    /// total number of threads spawned by daemon, one for blocking and the remainder for non-blocking
#define  BUFSIZE      1024  /// size of the input command buffer
#define  CONN_TIMEOUT 3000  /// incoming (non-blocking) connection timeout in milliseconds

namespace Slit {

  const std::string DAEMON_NAME = "slitd";       /// when run as a daemon, this is my name

  /** Server ******************************************************************/
  /**
   * @class  Server
   * @brief  server class for slit daemon
   *
   * This class defines what is needed by the slit daemon server.
   *
   */
  class Server {
    private:
    public:

      /** Slit::Server ********************************************************/
      /**
       * @fn         Server
       * @brief      class constructor
       * @param[in]  none
       * @return     none
       *
       */
      Server() {
        this->nbport=-1;
        this->blkport=-1;
        this->asyncport=-1;
        this->cmd_num=0;
      }
      /** Slit::Server ********************************************************/


      /** Slit::~Server *******************************************************/
      /**
       * @fn         ~Server
       * @brief      class deconstructor cleans up on exit
       * @param[in]  none
       * @return     none
       *
       */
      ~Server() {
        close_log();  // close the logfile, if open
      }
      /** Slit::~Server *******************************************************/


      int nbport;                        /// non-blocking port
      int blkport;                       /// blocking port
      int asyncport;                     /// asynchronous message port
      std::string asyncgroup;            /// asynchronous multicast group

      std::atomic<int> cmd_num;          /// keep a running tally of number of commands received by slitd

      Config config;                     /// create a Config object for reading the configuration file

      Interface interface;               /// create an Interface object for the slit hardware

      std::mutex conn_mutex;             /// mutex to protect against simultaneous access to Accept()


      /** Slit::Server::exit_cleanly ******************************************/
      /**
       * @fn         signal_handler
       * @brief      handles ctrl-C and exits
       * @param[in]  int signo
       * @return     nothing
       *
       */
      void exit_cleanly(void) {
        std::string function = "Slit::Server::exit_cleanly";
        logwrite( function, "slitd exiting" );

        exit(EXIT_SUCCESS);
      }
      /** Slit::Server::exit_cleanly ******************************************/


      /** Slit::Server::configure_slit ****************************************/
      /**
       * @fn         configure_slitd
       * @brief      read and apply the configuration file for the slit daemon
       * @param[in]  none
       * @return     ERROR or NO_ERROR
       *
       */
      long configure_slitd() {
        std::string function = "Slit::Server::configure_slitd";
        std::stringstream message;
        int applied=0;
        long error;

        // loop through the entries in the configuration file, stored in config class
        //
        for (int entry=0; entry < this->config.n_entries; entry++) {

          // PI_NAME -- this is the name of the PI motor controller subsystem
          //
          if ( config.param[entry].compare( 0, 7, "PI_NAME" ) == 0 ) {
            this->interface.name = config.arg[entry];
            applied++;
          }

          // PI_HOST -- hostname for the master PI motor controller
          //
          if ( config.param[entry].compare( 0, 7, "PI_HOST" ) == 0 ) {
            this->interface.host = config.arg[entry];
            applied++;
          }

          // PI_PORT -- port number on PI_HOST for the master PI motor controller
          //
          if ( config.param[entry].compare( 0, 7, "PI_PORT" ) == 0 ) {
            int port;
            try {
              port = std::stoi( config.arg[entry] );
            }
            catch (std::invalid_argument &) {
              logwrite(function, "ERROR: bad PI_PORT: unable to convert to integer");
              return(ERROR);
            }
            catch (std::out_of_range &) {
              logwrite(function, "PI_PORT number out of integer range");
              return(ERROR);
            }
            this->interface.port = port;
            applied++;
          }

          // MOTOR_CONTROLLER -- address and name of each PI motor controller in daisy-chain
          //
          if ( config.param[entry].compare( 0, 16, "MOTOR_CONTROLLER" ) == 0 ) {
            Slit::ControllerInfo s;
            if ( s.load_info( config.arg[entry] ) == NO_ERROR ) {
              this->interface.controller_info.push_back( s );
              applied++;
            }
          }

          // NBPORT -- nonblocking listening port for the slit daemon
          //
          if (config.param[entry].compare(0, 6, "NBPORT")==0) {
            int port;
            try {
              port = std::stoi( config.arg[entry] );
            }
            catch (std::invalid_argument &) {
              logwrite(function, "ERROR: bad NBPORT: unable to convert to integer");
              return(ERROR);
            }
            catch (std::out_of_range &) {
              logwrite(function, "NBPORT number out of integer range");
              return(ERROR);
            }
            this->nbport = port;
            applied++;
          }

          // BLKPORT -- blocking listening port for the slit daemon
          //
          if (config.param[entry].compare(0, 7, "BLKPORT")==0) {
            int port;
            try {
              port = std::stoi( config.arg[entry] );
            }
            catch (std::invalid_argument &) {
              logwrite(function, "ERROR: bad BLKPORT: unable to convert to integer");
              return(ERROR);
            }
            catch (std::out_of_range &) {
              logwrite(function, "BLKPORT number out of integer range");
              return(ERROR);
            }
            this->blkport = port;
            applied++;
          }

          // ASYNCPORT -- asynchronous broadcast message port for the slit daemon
          //
          if (config.param[entry].compare(0, 9, "ASYNCPORT")==0) {
            int port;
            try {
              port = std::stoi( config.arg[entry] );
            }
            catch (std::invalid_argument &) {
              logwrite(function, "ERROR: bad ASYNCPORT: unable to convert to integer");
              return(ERROR);
            }
            catch (std::out_of_range &) {
              logwrite(function, "ASYNCPORT number out of integer range");
              return(ERROR);
            }
            this->asyncport = port;
            applied++;
          }

          // ASYNCGROUP -- asynchronous broadcast group for the slit daemon
          //
          if (config.param[entry].compare(0, 10, "ASYNCGROUP")==0) {
            this->asyncgroup = config.arg[entry];
            applied++;
          }

        } // end loop through the entries in the configuration file

        message.str("");
        if (applied==0) {
          message << "ERROR: ";
          error = ERROR;
        } 
        else {
          error = NO_ERROR;
        } 
        message << "applied " << applied << " configuration lines to slitd";
        logwrite(function, message.str());

        // Initialize the class using the config parameters just read
        //
        if ( error == NO_ERROR ) error = this->interface.initialize_class();

        return error;
      }
      /** Slit::Server::configure_slit ****************************************/

  };
  /** Server ******************************************************************/

} // end namespace Slit
#endif
