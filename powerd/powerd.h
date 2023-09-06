/** ---------------------------------------------------------------------------
 * @file     powerd.h
 * @brief    power daemon include file
 * @author   David Hale <dhale@astro.caltech.edu>
 * @details 
 *
 */

#ifndef POWERD_H
#define POWERD_H

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
#include "power_interface.h"
#include "powerd_commands.h"

#define  N_THREADS    10    ///< total number of threads spawned by daemon, one for blocking and the remainder for non-blocking
#define  BUFSIZE      1024  ///< size of the input command buffer
#define  CONN_TIMEOUT 3000  ///< incoming (non-blocking) connection timeout in milliseconds


/***** Power ******************************************************************/
/**
 * @namespace Power
 * @brief     namespace for power control
 *
 */
namespace Power {

  /***** Power::Server ********************************************************/
  /**
   * @class Server
   * @brief power server class contains what's needed to run a server
   *
   */
  class Server {
    private:
    public:
      /**
       * Server class default constructor
       *
       */
      Server() {
        this->nbport=-1;
        this->blkport=-1;
        this->asyncport=-1;
        this->cmd_num=0;
      }

      /***** Power::~Server ***************************************************/
      /**
       * @fn         ~Server
       * @brief      class deconstructor cleans up on exit
       * @param[in]  none
       * @return     none
       */
      ~Server() {
        close(this->nonblocking_socket);
        close(this->blocking_socket);
        close_log();  // close the logfile, if open
      }
      /***** Power::~Server ***************************************************/

      int nbport;                        ///< non-blocking port
      int blkport;                       ///< blocking port
      int asyncport;                     ///< asynchronous message port
      std::string asyncgroup;            ///< asynchronous multicast group

      int nonblocking_socket;
      int blocking_socket;

      std::atomic<int> cmd_num;

      Config config;

      Interface interface;               ///< the Interface class connects to the hardware

      std::vector<int> nps_units;

      std::mutex conn_mutex;             ///< mutex to protect against simultaneous access to Accept()

      /***** Power::Server::exit_cleanly **************************************/
      /**
       * @fn         signal_handler
       * @brief      handles ctrl-C and exits
       * @param[in]  int signo
       * @return     nothing
       *
       */
      void exit_cleanly(void) {
        std::string function = "Power::Server::exit_cleanly";
        logwrite( function, "powerd exiting" );

        exit(EXIT_SUCCESS);
      }
      /***** Power::Server::exit_cleanly **************************************/


      /***** Power::Server::configure_powerd **********************************/
      /**
       * @fn         configure_powerd
       * @brief      parse and apply the configuration file
       * @param[in]  none
       * @return     ERROR or NO_ERROR
       *
       */
      long configure_powerd() {
        std::string function = "Power::Server::configure_powerd";
        std::stringstream message;
        int applied=0;
        long error = NO_ERROR;

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
            message.str(""); message << "POWERD:config:" << config.param[entry] << "=" << config.arg[entry];
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
            message.str(""); message << "POWERD:config:" << config.param[entry] << "=" << config.arg[entry];
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
            message.str(""); message << "POWERD:config:" << config.param[entry] << "=" << config.arg[entry];
            logwrite( function, message.str() );
            this->interface.async.enqueue( message.str() );
            applied++;
          }

          // ASYNCGROUP
          if (config.param[entry].compare(0, 10, "ASYNCGROUP")==0) {
            this->asyncgroup = config.arg[entry];
            message.str(""); message << "POWERD:config:" << config.param[entry] << "=" << config.arg[entry];
            logwrite( function, message.str() );
            this->interface.async.enqueue( message.str() );
            applied++;
          }

          // NPS_UNIT
          if (config.param[entry].compare(0, 8, "NPS_UNIT")==0) {
            // Create a local NpsInfo object for checking the config file input.
            //
            Power::NpsInfo npsinfo;

            // The nps_info map is indexed by nps number.
            // Pass this variable by reference to the load_nps_info() function, which will
            // set it to a valid value for insertion into the nps_info map.
            //
            int npsnum=-1;

            if ( npsinfo.load_nps_info( config.arg[entry], npsnum ) == NO_ERROR ) {
              this->interface.nps_info.insert( { npsnum, npsinfo } );  // insert this into the nps_info map
              this->interface.configure_interface( npsinfo );          // create an interface to the NPS described by this object
              message.str(""); message << "POWERD:config:" << config.param[entry] << "=" << config.arg[entry];
              logwrite( function, message.str() );
              this->interface.async.enqueue( message.str() );
              applied++;
            }
          }

          // NPS_PLUG
          if (config.param[entry].compare(0, 8, "NPS_PLUG")==0) {
            // Create a local NpsInfo object for checking the config file input.
            //
            Power::NpsInfo npsinfo;

            // The following variables (plugname, npsnum, plugnum) are extracted from this config file
            // by the load_plug_info() function.
            //
            std::stringstream plugid;
            std::string plugname;
            int         npsnum;
            int         plugnum;

            if ( npsinfo.load_plug_info( config.arg[entry], npsnum, plugnum, plugname ) == NO_ERROR ) {

              // load_plug_info() cannot check for maxplugs so check for that now
              //
              int maxplugs=-1;

              // Make sure the plug's npsnum has a matching NPS unit definition in the interface's nps_info map.
              //
              auto loc = this->interface.nps_info.find( npsnum );      // find this plug's npsnum in the nps_info map.

              if ( loc != this->interface.nps_info.end() ) {           // found
                maxplugs = loc->second.maxplugs;                       // here's the maxplugs for this unit
              }
              else {                                                   // else not found
                message.str(""); message << "ERROR plug " << plugnum << " " << plugname
                                         << " has no matching NPS unit number " << npsnum << " defined by NPS_UNIT";
                logwrite( function, message.str() );
                error = ERROR;
                break;
              }

              if ( plugnum > maxplugs ) {
                message.str(""); message << "ERROR bad plug number " << plugnum << " for " << plugname
                                         << " on unit " << npsnum << ": must be < " << maxplugs;
                logwrite( function, message.str() );
                error = ERROR;
                break;
              }

              // This plug passed all the tests so insert the plug into the plugmap.
              // This map provides for locating the NPS Unit and Plug by only the plug name.
              //
              this->interface.plugmap.insert( { plugname, { npsnum, plugnum } } );

              // plugname is an STL map of plug names indexed by plugid which
              // is a unique ID string consisting of "npsnum plugnum". This enables
              // lookup of the plugname for any given nps and plug.
              //
              plugid << npsnum << " " << plugnum;
              this->interface.plugname.insert( { plugid.str(), plugname } );

              message.str(""); message << "POWERD:config:" << config.param[entry] << "=" << config.arg[entry];
              logwrite( function, message.str() );
              this->interface.async.enqueue( message.str() );
              applied++;
            }
            else error = ERROR;
          }

        } // end loop through the entries in the configuration file

        message.str("");
        if (applied==0) {
          message << "ERROR: ";
          error = ERROR;
        } 
        message << "applied " << applied << " configuration lines to powerd";
        logwrite(function, message.str());

        // Initialize the class using the config parameters just read
        //
        if ( error == NO_ERROR ) error = this->interface.initialize_class();

        return error;
      }
      /***** Power::Server::configure_powerd **********************************/

  };
  /***** Power::Server ********************************************************/

}
/***** Power ******************************************************************/
#endif
