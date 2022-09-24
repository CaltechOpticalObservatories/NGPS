/** ---------------------------------------------------------------------------
 * @file     sequencerd.h
 * @brief    
 * @author   David Hale <dhale@astro.caltech.edu>
 * @date     
 * @modified 
 *
 */

#ifndef SEQUENCERD_H
#define SEQUENCERD_H

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
#include <condition_variable>
#include <cmath>
#include <ctgmath>

#include "logentry.h"
#include "config.h"
#include "network.h"
#include "common.h"
#include "sequencer_interface.h"
#include "sequence.h"
#include "sequencerd_commands.h"

#define  N_THREADS    10    /// total number of threads spawned by sequencer, one for blocking and the remainder for non-blocking
#define  BUFSIZE      1024  /// size of the input command buffer
#define  CONN_TIMEOUT 3000  /// incoming (non-blocking) connection timeout in milliseconds


namespace Sequencer {

  const std::string DAEMON_NAME = "sequencerd";  /// when run as a daemon, this is my name

  class Server {
    private:
    public:
      Server() {
        this->nbport=-1;
        this->blkport=-1;
        this->asyncport=-1;
        this->cmd_num=0;

        // The names of all of the daemons defined in the Sequencer::Sequence class
        // are initialized here. The names are useful just for logging.
        //
        this->sequence.calibd.name   = "calibd";
        this->sequence.camerad.name  = "camerad";
        this->sequence.filterd.name  = "filterd";
        this->sequence.flexured.name = "flexured";
        this->sequence.focusd.name   = "focusd";
        this->sequence.powerd.name   = "powerd";
        this->sequence.slitd.name    = "slitd";
        this->sequence.tcsd.name     = "tcsd";
      }

      /** Sequencer::~Server *******************************************************/
      /**
       * @fn     ~Server
       * @brief  class deconstructor cleans up on exit
       */
      ~Server() {
        close(this->nonblocking_socket);
        close(this->blocking_socket);
        close_log();  // close the logfile, if open
      }
      /** Sequencer::~Server *******************************************************/

      int nbport;                        /// sequencer daemon non-blocking port
      int blkport;                       /// sequencer daemon blocking port
      int asyncport;                     /// sequencer daemon asynchronous message port
      std::string asyncgroup;            /// sequencer daemon asynchronous multicast group

      int nonblocking_socket;
      int blocking_socket;

      std::atomic<int> cmd_num;

      Config config;

      Sequence sequence;

      std::mutex conn_mutex;             //!< mutex to protect against simultaneous access to Accept()

      /** Sequencer::Server::exit_cleanly ******************************************/
      /**
       * @fn     signal_handler
       * @brief  handles ctrl-C and exits
       * @param  int signo
       * @return nothing
       *
       */
      void exit_cleanly(void) {
        std::string function = "Sequencer::Server::exit_cleanly";
        logwrite( function, "sequencer exiting" );
        exit(EXIT_SUCCESS);
      }
      /** Sequencer::Server::exit_cleanly ******************************************/


      /** Sequencer::Server::configure_sequencer ***********************************/
      /**
       * @fn     configure_sequencer
       * @brief  
       * @param  none
       * @return ERROR or NO_ERROR
       *
       */
      long configure_sequencer() {
        std::string function = "Sequencer::Server::configure_sequencer";
        std::stringstream message;
        int applied=0;
        long error;

        // loop through the entries in the configuration file, stored in config class
        //
        for (int entry=0; entry < this->config.n_entries; entry++) {

          std::string configkey;          // the current configuration key read from file
          std::string configval;          // the current configuration value read from file

          try {
            configkey = config.param.at(entry);
            configval = config.arg.at(entry);
          }
          catch (std::out_of_range &) {   // should be impossible
            message.str(""); message << "ERROR: entry " << entry 
                                     << " out of range in config parameters (" << this->config.n_entries << ")";
            logwrite( function, message.str() );
            return(ERROR);
          }

#ifdef LOGLEVEL_DEBUG
          message.str(""); message << "[DEBUG] configkey " << configkey << "=" << configval;
          logwrite( function, message.str() );
#endif

          // NBPORT
          if ( configkey.compare(0, 6, "NBPORT")==0 ) {
            int port;
            try {
              port = std::stoi( configval );
            }
            catch (std::invalid_argument &) {
              message.str(""); message << "ERROR: bad NBPORT: unable to convert " << configval << " to integer";
              logwrite( function, message.str() );
              return(ERROR);
            }
            catch (std::out_of_range &) {
              logwrite(function, "NBPORT number out of integer range");
              return(ERROR);
            }
            this->nbport = port;
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
            applied++;
          }

          // ASYNCGROUP
          if (config.param[entry].compare(0, 10, "ASYNCGROUP")==0) {
            this->asyncgroup = config.arg[entry];
            applied++;
          }

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
            this->sequence.camerad.port =  port;
            applied++;
          }

          // FLEXURED_PORT
          if (config.param[entry].compare(0, 13, "FLEXURED_PORT")==0) {
            int port;
            try {
              port = std::stoi( config.arg[entry] );
            }
            catch (std::invalid_argument &) {
              logwrite(function, "ERROR: bad FLEXURED_PORT: unable to convert to integer");
              return(ERROR);
            }
            catch (std::out_of_range &) {
              logwrite(function, "FLEXURED_PORT number out of integer range");
              return(ERROR);
            }
            this->sequence.flexured.port =  port;
            applied++;
          }

          // POWERD_PORT
          if (config.param[entry].compare(0, 11, "POWERD_PORT")==0) {
            int port;
            try {
              port = std::stoi( config.arg[entry] );
            }
            catch (std::invalid_argument &) {
              logwrite(function, "ERROR: bad POWERD_PORT: unable to convert to integer");
              return(ERROR);
            }
            catch (std::out_of_range &) {
              logwrite(function, "POWERD_PORT number out of integer range");
              return(ERROR);
            }
            this->sequence.powerd.port =  port;
            applied++;
          }

          // SLITD_PORT
          if (config.param[entry].compare(0, 10, "SLITD_PORT")==0) {
            int port;
            try {
              port = std::stoi( config.arg[entry] );
            }
            catch (std::invalid_argument &) {
              logwrite(function, "ERROR: bad SLITD_PORT: unable to convert to integer");
              return(ERROR);
            }
            catch (std::out_of_range &) {
              logwrite(function, "SLITD_PORT number out of integer range");
              return(ERROR);
            }
            this->sequence.slitd.port =  port;
            applied++;
          }

          // TCSD_PORT
          if (config.param[entry].compare(0, 9, "TCSD_PORT")==0) {
            int port;
            try {
              port = std::stoi( config.arg[entry] );
            }
            catch (std::invalid_argument &) {
              logwrite(function, "ERROR: bad TCSD_PORT: unable to convert to integer");
              return(ERROR);
            }
            catch (std::out_of_range &) {
              logwrite(function, "TCSD_PORT number out of integer range");
              return(ERROR);
            }
            this->sequence.tcsd.port =  port;
            applied++;
          }

          // CALIBD_PORT
          if (config.param[entry].compare(0, 11, "CALIBD_PORT")==0) {
            int port;
            try {
              port = std::stoi( config.arg[entry] );
            }
            catch (std::invalid_argument &) {
              logwrite(function, "ERROR: bad CALIBD_PORT: unable to convert to integer");
              return(ERROR);
            }
            catch (std::out_of_range &) {
              logwrite(function, "CALIBD_PORT number out of integer range");
              return(ERROR);
            }
            this->sequence.calibd.port =  port;
            applied++;
          }

          // FILTERD_PORT
          if (config.param[entry].compare(0, 12, "FILTERD_PORT")==0) {
            int port;
            try {
              port = std::stoi( config.arg[entry] );
            }
            catch (std::invalid_argument &) {
              logwrite(function, "ERROR: bad FILTERD_PORT: unable to convert to integer");
              return(ERROR);
            }
            catch (std::out_of_range &) {
              logwrite(function, "FILTERD_PORT number out of integer range");
              return(ERROR);
            }
            this->sequence.filterd.port =  port;
            applied++;
          }

          // FOCUSD_PORT
          if (config.param[entry].compare(0, 11, "FOCUSD_PORT")==0) {
            int port;
            try {
              port = std::stoi( config.arg[entry] );
            }
            catch (std::invalid_argument &) {
              logwrite(function, "ERROR: bad FOCUSD_PORT: unable to convert to integer");
              return(ERROR);
            }
            catch (std::out_of_range &) {
              logwrite(function, "FOCUSD_PORT number out of integer range");
              return(ERROR);
            }
            this->sequence.focusd.port =  port;
            applied++;
          }

          // TCS_SETTLE_TIMEOUT
          if (config.param[entry].compare(0, 18, "TCS_SETTLE_TIMEOUT")==0) {
            double to;
            try {
              to = std::stod( config.arg[entry] );
            }
            catch (std::invalid_argument &) {
              message.str(""); message << "ERROR: bad TCS_SETTLE_TIMEOUT: unable to convert " << config.arg[entry] << " to double";
              logwrite(function, message.str());
              return(ERROR);
            }
            catch (std::out_of_range &) {
              logwrite(function, "TCS_SETTLE_TIMEOUT number out of double range");
              return(ERROR);
            }
            this->sequence.tcs_settle_timeout = to;
            applied++;
          }

          // TCS_PREAUTH_TIME
          if (config.param[entry].compare(0, 16, "TCS_PREAUTH_TIME")==0) {
            double to;
            try {
              to = std::stod( config.arg[entry] );
            }
            catch (std::invalid_argument &) {
              logwrite(function, "ERROR: bad TCS_PREAUTH_TIME: unable to convert to double");
              return(ERROR);
            }
            catch (std::out_of_range &) {
              logwrite(function, "TCS_PREAUTH_TIME number out of double range");
              return(ERROR);
            }
            this->sequence.tcs_preauth_time = to;
            applied++;
          }

          //
          // To configure the database parameters, call configure_db( param, value ) with the
          // parameter name and the value. This function will parse and perform error checking and
          // assign to the correct private variable.  If this returns NO_ERROR then increment applied.
          //

          // DB_HOST
          if (config.param[entry].compare( 0, DB_HOST.length(), DB_HOST )==0) {
            if ( this->sequence.target.configure_db( DB_HOST, config.arg[entry] ) == NO_ERROR ) applied++;
          }

          // DB_PORT
          if (config.param[entry].compare( 0, DB_PORT.length(), DB_PORT )==0) {
            if ( this->sequence.target.configure_db( DB_PORT, config.arg[entry] ) == NO_ERROR ) applied++;
          }

          // DB_USER
          if (config.param[entry].compare( 0, DB_USER.length(), DB_USER )==0) {
            if ( this->sequence.target.configure_db( DB_USER, config.arg[entry] ) == NO_ERROR ) applied++;
          }

          // DB_PASS
          if (config.param[entry].compare( 0, DB_PASS.length(), DB_PASS )==0) {
            if ( this->sequence.target.configure_db( DB_PASS, config.arg[entry] ) == NO_ERROR ) applied++;
          }

          // DB_SCHEMA
          if (config.param[entry].compare( 0, DB_SCHEMA.length(), DB_SCHEMA )==0) {
            if ( this->sequence.target.configure_db( DB_SCHEMA, config.arg[entry] ) == NO_ERROR ) applied++;
          }

          // DB_ACTIVE
          if (config.param[entry].compare( 0, DB_ACTIVE.length(), DB_ACTIVE )==0) {
            if ( this->sequence.target.configure_db( DB_ACTIVE, config.arg[entry] ) == NO_ERROR ) applied++;
          }

          // DB_COMPLETED
          if (config.param[entry].compare( 0, DB_COMPLETED.length(), DB_COMPLETED )==0) {
            if ( this->sequence.target.configure_db( DB_COMPLETED, config.arg[entry] ) == NO_ERROR ) applied++;
          }

          //
          // configure the power switch parameters
          //

          // POWER_SLIT
          if (config.param[entry].compare( 0, POWER_SLIT.length(), POWER_SLIT )==0) {
            if ( this->sequence.power_switch[POWER_SLIT].configure( this->config.arg[entry] ) == NO_ERROR ) applied++;
          }

          // POWER_CAMERA
          if (config.param[entry].compare( 0, POWER_CAMERA.length(), POWER_CAMERA )==0) {
            if ( this->sequence.power_switch[POWER_CAMERA].configure( this->config.arg[entry] ) == NO_ERROR ) applied++;
          }

          // POWER_CALIB
          if (config.param[entry].compare( 0, POWER_CALIB.length(), POWER_CALIB )==0) {
            if ( this->sequence.power_switch[POWER_CALIB].configure( this->config.arg[entry] ) == NO_ERROR ) applied++;
          }

          // POWER_FLEXURE
          if (config.param[entry].compare( 0, POWER_FLEXURE.length(), POWER_FLEXURE )==0) {
            if ( this->sequence.power_switch[POWER_FLEXURE].configure( this->config.arg[entry] ) == NO_ERROR ) applied++;
          }

          // POWER_FILTER
          if (config.param[entry].compare( 0, POWER_FILTER.length(), POWER_FILTER )==0) {
            if ( this->sequence.power_switch[POWER_FILTER].configure( this->config.arg[entry] ) == NO_ERROR ) applied++;
          }

          // POWER_FOCUS
          if (config.param[entry].compare( 0, POWER_FOCUS.length(), POWER_FOCUS )==0) {
            if ( this->sequence.power_switch[POWER_FOCUS].configure( this->config.arg[entry] ) == NO_ERROR ) applied++;
          }

          // POWER_TELEM
          if (config.param[entry].compare( 0, POWER_TELEM.length(), POWER_TELEM )==0) {
            if ( this->sequence.power_switch[POWER_TELEM].configure( this->config.arg[entry] ) == NO_ERROR ) applied++;
          }

          // POWER_THERMAL
          if (config.param[entry].compare( 0, POWER_THERMAL.length(), POWER_THERMAL )==0) {
            if ( this->sequence.power_switch[POWER_THERMAL].configure( this->config.arg[entry] ) == NO_ERROR ) applied++;
          }

          // POWER_AG
          if (config.param[entry].compare( 0, POWER_AG.length(), POWER_AG )==0) {
            if ( this->sequence.power_switch[POWER_AG].configure( this->config.arg[entry] ) == NO_ERROR ) applied++;
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
        message << "applied " << applied << " configuration lines to sequencer";
        logwrite(function, message.str());

        return error;
      }
      /** Sequencer::Server::configure_sequencer ***********************************/

  };  // end class Server

} // end namespace Sequencer
#endif
