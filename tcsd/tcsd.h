/** ---------------------------------------------------------------------------
 * @file     tcsd.h
 * @brief    TCS daemon include file
 * @details  defines the classes used by the TCS daemon
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
#include "tcs_interface.h"
#include "tcsd_commands.h"

#define  N_THREADS    10    //!< total number of threads spawned by daemon, one for blocking and the remainder for non-blocking
#define  BUFSIZE      1024  //!< size of the input command buffer
#define  CONN_TIMEOUT 3000  //<! incoming (non-blocking) connection timeout in milliseconds

namespace TCS {

  /** Server ******************************************************************/
  /**
   * @class  Server
   * @brief  server class for tcs daemon
   *
   * This class defines what is needed by the tcs daemon server.
   *
   */
  class Server {
    private:
    public:
      Server() {
        this->nbport=-1;
        this->blkport=-1;
        this->asyncport=-1;
        this->cmd_num=0;
      }

      /** TCS::~Server ********************************************************/
      /**
       * @fn     ~Server
       * @brief  class deconstructor cleans up on exit
       */
      ~Server() {
        close(this->nonblocking_socket);
        close(this->blocking_socket);
        close_log();  // close the logfile, if open
      }
      /** TCS::~Server ********************************************************/

      int nbport;                        //!< non-blocking port
      int blkport;                       //!< blocking port
      int asyncport;                     //!< asynchronous message port
      std::string asyncgroup;            //!< asynchronous multicast group

      int nonblocking_socket;
      int blocking_socket;

      std::atomic<int> cmd_num;

      Config config;

      Interface interface;

      std::mutex conn_mutex;             //!< mutex to protect against simultaneous access to Accept()


      /** TCS::Server::exit_cleanly *******************************************/
      /**
       * @fn     signal_handler
       * @brief  handles ctrl-C and exits
       * @param  int signo
       * @return nothing
       *
       */
      void exit_cleanly(void) {
        std::string function = "TCS::Server::exit_cleanly";
        logwrite( function, "tcsd exiting" );

        exit(EXIT_SUCCESS);
      }
      /** TCS::Server::exit_cleanly *******************************************/


      /** TCS::Server::configure_tcs ******************************************/
      /**
       * @fn     configure_tcsd
       * @brief  read and apply the configuration file for the tcs daemon
       * @param  none
       * @return ERROR or NO_ERROR
       *
       */
      long configure_tcsd() {
        std::string function = "TCS::Server::configure_tcsd";
        std::stringstream message;
        int applied=0;
        long error;

        // loop through the entries in the configuration file, stored in config class
        //
        for (int entry=0; entry < this->config.n_entries; entry++) {

          // TCS_HOST -- hostname for the TCS
          //
          if ( config.param[entry].compare( 0, 8, "TCS_HOST" ) == 0 ) {
            this->interface.host = config.arg[entry];
            applied++;
          }

          // TCS_PORT -- port number on TCS_HOST for the TCS
          //
          if ( config.param[entry].compare( 0, 8, "TCS_PORT" ) == 0 ) {
            int port;
            try {
              port = std::stoi( config.arg[entry] );
            }
            catch (std::invalid_argument &) {
              logwrite(function, "ERROR: bad TCS_PORT: unable to convert to integer");
              return(ERROR);
            }
            catch (std::out_of_range &) {
              logwrite(function, "TCS_PORT number out of integer range");
              return(ERROR);
            }
            this->interface.port = port;
            applied++;
          }

          // NBPORT -- nonblocking listening port for the tcs daemon
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

          // BLKPORT -- blocking listening port for the tcs daemon
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

          // ASYNCPORT -- asynchronous broadcast message port for the tcs daemon
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

          // ASYNCGROUP -- asynchronous broadcast group for the tcs daemon
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
        message << "applied " << applied << " configuration lines to tcsd";
        logwrite(function, message.str());

        if ( error == NO_ERROR ) error = this->interface.initialize_class();

        return error;
      }
      /** TCS::Server::configure_tcs ******************************************/

  };
  /** Server ******************************************************************/

} // end namespace TCS
#endif
