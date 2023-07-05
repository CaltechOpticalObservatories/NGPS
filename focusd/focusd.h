/** ---------------------------------------------------------------------------
 * @file     focusd.h
 * @brief    
 * @author   David Hale <dhale@astro.caltech.edu>
 * @date     
 * @modified 
 *
 */

#ifndef FOCUS_H
#define FOCUS_H

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
#include "focus_interface.h"

#define  N_THREADS    10    //!< total number of threads spawned by daemon, one for blocking and the remainder for non-blocking
#define  BUFSIZE      1024  //!< size of the input command buffer
#define  CONN_TIMEOUT 3000  //<! incoming (non-blocking) connection timeout in milliseconds

namespace Focus {

  class Server {
    private:
    public:
      Server() {
        this->nbport=-1;
        this->blkport=-1;
        this->asyncport=-1;
//      this->camerad_port=-1;
        this->cmd_num=0;
      }

      /** Focus::~Server ***********************************************************/
      /**
       * @fn     ~Server
       * @brief  class deconstructor cleans up on exit
       */
      ~Server() {
        close(this->nonblocking_socket);
        close(this->blocking_socket);
        close_log();  // close the logfile, if open
      }
      /** Focus::~Server ***********************************************************/

//    int camerad_port;                  //!< camerad port TODO replace with something else?
      int nbport;                        //!< non-blocking port
      int blkport;                       //!< blocking port
      int asyncport;                     //!< asynchronous message port
      std::string asyncgroup;            //!< asynchronous multicast group

      int nonblocking_socket;
      int blocking_socket;

      std::atomic<int> cmd_num;

      Config config;

      Interface interface;               //!< the interface class connects to the hardware
      std::mutex conn_mutex;             //!< mutex to protect against simultaneous access to Accept()

      /** Focus::Server::exit_cleanly **********************************************/
      /**
       * @fn     signal_handler
       * @brief  handles ctrl-C and exits
       * @param  int signo
       * @return nothing
       *
       */
      void exit_cleanly(void) {
        std::string function = "Focus::Server::exit_cleanly";
        logwrite( function, "focusd exiting" );

        // close connections to daemons
        //
//      this->camerad.socket.Close();
        exit(EXIT_SUCCESS);
      }
      /** Focus::Server::exit_cleanly **********************************************/

      /** Focus::Server::configure_focus *******************************************/
      /**
       * @fn     configure_focusd
       * @brief  
       * @param  none
       * @return ERROR or NO_ERROR
       *
       */
      long configure_focusd() {
        std::string function = "Focus::Server::configure_focusd";
        std::stringstream message;
        int applied=0;
        long error;

        // loop through the entries in the configuration file, stored in config class
        //
        for (int entry=0; entry < this->config.n_entries; entry++) {

          // NBPORT
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
            message.str(""); message << "FOCUSD:config:" << config.param[entry] << "=" << config.arg[entry];
            logwrite( function, message.str() );
            this->interface.async.enqueue( message.str() );
            applied++;
          }

          // BLKPORT
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
            message.str(""); message << "FOCUSD:config:" << config.param[entry] << "=" << config.arg[entry];
            logwrite( function, message.str() );
            this->interface.async.enqueue( message.str() );
            applied++;
          }

          // ASYNCPORT
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
            message.str(""); message << "FOCUSD:config:" << config.param[entry] << "=" << config.arg[entry];
            logwrite( function, message.str() );
            this->interface.async.enqueue( message.str() );
            applied++;
          }

          // ASYNCGROUP
          if (config.param[entry].compare(0, 10, "ASYNCGROUP")==0) {
            this->asyncgroup = config.arg[entry];
            message.str(""); message << "FOCUSD:config:" << config.param[entry] << "=" << config.arg[entry];
            logwrite( function, message.str() );
            this->interface.async.enqueue( message.str() );
            applied++;
          }

/***
          // CAMERAD_PORT
          if (config.param[entry].compare(0, 12, "CAMERAD_PORT")==0) {
            int port;
            try {
              port = std::stoi( config.arg[entry] );
            }
            catch (std::invalid_argument &) {
              logwrite(function, "ERROR: bad CAMERAD_PORT: unable to convert to integer");
              return(ERROR);
            }
            catch (std::out_of_range &) {
              logwrite(function, "CAMERAD_PORT number out of integer range");
              return(ERROR);
            }
            this->camerad_port = port;
            applied++;
          }
***/

        } // end loop through the entries in the configuration file

        message.str("");
        if (applied==0) {
          message << "ERROR: ";
          error = ERROR;
        } 
        else {
          error = NO_ERROR;
        } 
        message << "applied " << applied << " configuration lines to focusd";
        logwrite(function, message.str());

/***
        // initialize connection to camerad
        //
        if ( this->camerad_port == -1 ) {
          logwrite( function, "ERROR: camerad port not configured" );
          error = ERROR;
        }
        else {
          if ( this->camerad.socket.isconnected() ) {
            logwrite( function, "closing connection to camerad" );
            this->camerad.socket.Close();
          }
          this->camerad.socket.setport( this->camerad_port );
          if ( this->camerad.socket.Connect() < 0 ) {
            logwrite( function, "ERROR: connecting to camerad" );
            error = ERROR;
          } else {
            logwrite( function, "connected to camerad" );
          }
        }

***/
        return error;
      }
      /** Focus::Server::configure_focus *******************************************/

  };  // end class Server

} // end namespace Focus
#endif
