/** ---------------------------------------------------------------------------
 * @file     camerad.h
 * @brief    
 * @author   David Hale <dhale@astro.caltech.edu>
 *
 */

#pragma once

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

#ifdef ASTROCAM
#include "astrocam.h"       //!< for any Leach interface, ARC-64 or ARC-66
#elif STA_ARCHON
#include "archon.h"         //!< for STA-Archon
#endif

#include "logentry.h"
#include "config.h"
#include "network.h"
#include "camerad_commands.h"

#define  N_THREADS    10    //!< total number of threads spawned by server, one for blocking and the remainder for non-blocking
#define  BUFSIZE      1024  //!< size of the input command buffer
#define  CONN_TIMEOUT 3000  //<! incoming (non-blocking) connection timeout in milliseconds

/***** Camera *****************************************************************/
/**
 * @namespace Camera
 * @brief     namespace for the camera
 *
 */
namespace Camera {

  const std::string DAEMON_NAME = "camerad";     /// when run as a daemon, this is my name

  /***** Camera::Server *******************************************************/
  /**
   * @class Server
   * @brief server class for the camera daemon
   *
   * This class defines what is needed by the camera daemon server.
   * Right not it inherits the appropriate interface, AstroCam::Interface or Archon::Interface
   *
   */
#ifdef ASTROCAM
  class Server : public AstroCam::Interface, public NewInterface<AstroCam::NewAstroCam> {
#elif STA_ARCHON
  class Server : public Archon::Interface {
#endif
    private:
    public:
      /***** Camera::Server ***************************************************/
      /**
       * @brief  class constructor
       */
      Server() : nbport(-1), blkport(-1), asyncport(-1), cmd_num(0) { }
      /***** Camera::Server ***************************************************/


      /***** Camera::~Server **************************************************/
      /**
       * @brief  class deconstructor cleans up on exit
       */
      ~Server() {
        close(this->nonblocking_socket);
        close(this->blocking_socket);
        close_log();  // close the logfile, if open
      }
      /***** Camera::~Server **************************************************/

      int nbport;                        //!< non-blocking port
      int blkport;                       //!< blocking port
      int asyncport;                     //!< asynchronous message port
      std::string asyncgroup;            //!< asynchronous multicast group

      int nonblocking_socket;
      int blocking_socket;

      std::atomic<int> cmd_num;

      std::vector<int> jclient_ports;

      Network::TcpSocket nonblocking;

      std::mutex conn_mutex;             //!< mutex to protect against simultaneous access to Accept()

      /***** Camera::Server::exit_cleanly *************************************/
      /**
       * @brief      handles ctrl-C and exits
       * @param[in]  signo
       *
       */
      void exit_cleanly(void) {
        std::string function = "Camera::Server::exit_cleanly";
        this->disconnect_controller();
        this->camera.shutter.shutdown();
        logwrite(function, "server exiting");
        exit(EXIT_SUCCESS);
      }
      /***** Camera::Server::exit_cleanly *************************************/


      /***** Camera::Server::configure_server *********************************/
      /**
       * @brief  
       * @return     ERROR or NO_ERROR
       *
       */
      long configure_server() {
        std::string function = "Camera::Server::configure_server";
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
              this->camera.log_error( function, "bad NBPORT: unable to convert to integer" );
              return(ERROR);
            }
            catch (std::out_of_range &) {
              this->camera.log_error( function, "NBPORT number out of integer range" );
              return(ERROR);
            }
            this->nbport = port;
            message.str(""); message << "CAMERAD:config:" << config.param[entry] << "=" << config.arg[entry];
            logwrite( function, message.str() );
            this->camera.async.enqueue( message.str() );
            applied++;
          }

          // BLKPORT
          if (config.param[entry].compare(0, 7, "BLKPORT")==0) {
            int port;
            try {
              port = std::stoi( config.arg[entry] );
            }
            catch (std::invalid_argument &) {
              this->camera.log_error( function, "bad BLKPORT: unable to convert to integer" );
              return(ERROR);
            }
            catch (std::out_of_range &) {
              this->camera.log_error( function, "BLKPORT number out of integer range" );
              return(ERROR);
            }
            this->blkport = port;
            message.str(""); message << "CAMERAD:config:" << config.param[entry] << "=" << config.arg[entry];
            logwrite( function, message.str() );
            this->camera.async.enqueue( message.str() );
            applied++;
          }

          // TELEM_PROVIDER : contains daemon name and port to contact for header telemetry info
          //
          if ( config.param[entry] == "TELEM_PROVIDER" ) {
            std::vector<std::string> tokens;
            Tokenize( config.arg[entry], tokens, " " );
            try {
              if ( tokens.size() == 2 ) {
                this->telemetry_providers[tokens.at(0)] = std::stod(tokens.at(1));
              }
              else {
                message.str(""); message << "bad format \"" << config.arg[entry] << "\": expected <name> <port>";
                this->camera.log_error( function, message.str() );
                return ERROR;
              }
            }
            catch ( const std::exception &e ) {
              message.str(""); message << "parsing TELEM_PROVIDER from " << config.arg[entry] << ": " << e.what();
              this->camera.log_error( function, message.str() );
              return ERROR;
            }
            message.str(""); message << "config:" << config.param[entry] << "=" << config.arg[entry];
            this->camera.async.enqueue_and_log( "CAMERAD", function, message.str() );
            applied++;
          }

          // ASYNCPORT
          if (config.param[entry].compare(0, 9, "ASYNCPORT")==0) {
            int port;
            try {
              port = std::stoi( config.arg[entry] );
            }
            catch (std::invalid_argument &) {
              this->camera.log_error( function, "bad ASYNCPORT: unable to convert to integer" );
              return(ERROR);
            }
            catch (std::out_of_range &) {
              this->camera.log_error( function, "ASYNCPORT number out of integer range" );
              return(ERROR);
            }
            this->asyncport = port;
            message.str(""); message << "CAMERAD:config:" << config.param[entry] << "=" << config.arg[entry];
            logwrite( function, message.str() );
            this->camera.async.enqueue( message.str() );
            applied++;
          }

          // ASYNCGROUP
          if (config.param[entry].compare(0, 10, "ASYNCGROUP")==0) {
            this->asyncgroup = config.arg[entry];
            message.str(""); message << "CAMERAD:config:" << config.param[entry] << "=" << config.arg[entry];
            logwrite( function, message.str() );
            this->camera.async.enqueue( message.str() );
            applied++;
          }

          // PUB_ENDPOINT
          //
          if (config.param[entry]=="PUB_ENDPOINT") {
            this->publisher_address=config.arg[entry];
            this->publisher_topic=DAEMON_NAME;
            this->camera.async.enqueue_and_log("CAMERAD", function, "CAMERAD:config:"+config.param[entry]+"="+config.arg[entry]);
              applied++;
          }

          // SUB_ENDPOINT
          //
          if (config.param[entry]=="SUB_ENDPOINT") {
            this->subscriber_address=config.arg[entry];
            this->camera.async.enqueue_and_log("CAMERAD", function, "CAMERAD:config:"+config.param[entry]+"="+config.arg[entry]);
              applied++;
          }

          // USERKEYS_PERSIST: should userkeys persist or be cleared after each exposure
          //
          if ( config.param[entry] == "USERKEYS_PERSIST" ) {
            this->camera.is_userkeys_persist = caseCompareString( config.arg[entry], "yes" );
            message.str(""); message << "config:" << config.param[entry] << "=" << config.arg[entry];
            this->camera.async.enqueue_and_log( "CAMERAD", function, message.str() );
            applied++;
          }

          // LONGERROR
          if (config.param[entry].compare(0, 9, "LONGERROR")==0) {
            std::string dontcare;
            if ( this->camera.longerror( config.arg[entry], dontcare ) == ERROR ) {
              this->camera.log_error( function, "setting longerror" );
              return( ERROR );
            }
            message.str(""); message << "CAMERAD:config:" << config.param[entry] << "=" << config.arg[entry];
            logwrite( function, message.str() );
            this->camera.async.enqueue( message.str() );
            applied++;
          }

          // GIT_HASH
          if (config.param[entry].compare(0, 8, "GIT_HASH")==0) {
            this->camera_info.systemkeys.primary().addkey( "GIT_HASH", config.arg[entry], "software git hash" );
            message.str(""); message << "CAMERAD:config:" << config.param[entry] << "=" << config.arg[entry];
            logwrite( function, message.str() );
            this->camera.async.enqueue( message.str() );
            applied++;
          }

          // PROJ_BUILD_DATE
          if (config.param[entry].compare(0, 15, "PROJ_BUILD_DATE")==0) {
            this->camera_info.systemkeys.primary().addkey( "SW_BUILD", config.arg[entry], "software build date" );
            message.str(""); message << "CAMERAD:config:" << config.param[entry] << "=" << config.arg[entry];
            logwrite( function, message.str() );
            this->camera.async.enqueue( message.str() );
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
        message << "applied " << applied << " configuration lines to server";
        error==NO_ERROR ? logwrite(function, message.str()) : this->camera.log_error( function, message.str() );
        return error;
      }
      /***** Camera::Server::configure_server *********************************/


      /***** Camera::Server::configure_constkeys ******************************/
      /**
       * @brief      processes the CONSTKEY_* keywords from the config file
       * @details    constant keywords allow insertion into all FITS files
       *             header keywords that won't change
       * @return     ERROR or NO_ERROR
       *
       */
      long configure_constkeys() {
        std::string function = "Camera::Server::configure_constkeys";
        std::stringstream message;
        int applied=0;
        long error=NO_ERROR;

        // loop through the entries in the configuration file, stored in config class
        //
        for (int entry=0; entry < this->config.n_entries; entry++) {

          // CONSTKEY_*
          //
          if (config.param[entry].compare(0, 9, "CONSTKEY_")==0) {
            // convert the arg into a vector and use the vector form of addkey()
            //
            std::vector<std::string> tokens;
            Tokenize( config.arg[entry], tokens, " " );

            if ( config.param[entry].compare( 9, 3, "PRI" )==0 ) error = this->camera_info.systemkeys.primary().addkey( tokens );
            else
            if ( config.param[entry].compare( 9, 3, "EXT" )==0 ) this->camera_info.systemkeys.add_key(tokens[0],tokens[1],"",true,"all");
//          if ( config.param[entry].compare( 9, 3, "EXT" )==0 ) error = this->camera_info.systemkeys.extension().addkey( tokens );
            else
            continue;

            if ( error == ERROR ) {
              message.str(""); message << "ERROR: parsing config " << config.param[entry] << "=" << config.arg[entry];
              logwrite( function, message.str() );
              return( ERROR );
            }

            message.str(""); message << "CAMERAD:config:" << config.param[entry] << "=" << config.arg[entry];
            logwrite( function, message.str() );
            applied++;
          }

        } // end loop through the entries in the configuration file

        message.str(""); message << "applied " << applied << " constant FITS keys";
        error==NO_ERROR ? logwrite(function, message.str()) : this->camera.log_error( function, message.str() );
        return error;
      }
      /***** Camera::Server::configure_constkeys ******************************/

  };
  /***** Camera::Server *******************************************************/

}
/***** Camera *****************************************************************/
