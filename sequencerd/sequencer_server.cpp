/**
 * @file    sequencer_server.cpp
 * @brief   these are the main functions used by the Sequencer::Server
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 * This file defines the functions used by the Server class in the Sequencer namespace.
 *
 */

#include "sequencer_server.h"

namespace Sequencer {

  Server* Server::instance = nullptr;

  /***** Sequencer::Server::handle_signal *************************************/
  /**
   * @brief      handles ctrl-C and other signals
   * @param[in]  int signo
   *
   */
  void Server::handle_signal(int signo) {
    std::string function = "Sequencer::Server::handle_signal";
    std::stringstream message;

    switch (signo) {
      case SIGTERM:
      case SIGINT:
        logwrite(function, "received termination signal");
        message << "NOTICE:" << Sequencer::DAEMON_NAME << " exit";
        Server::instance->sequence.async.enqueue( message.str() );
        Server::instance->exit_cleanly();                      // shutdown the daemon
        break;
      case SIGHUP:  // TODO reconfigure?
        Server::instance->sequence.async.enqueue_and_log( function,
          "ERROR: caught unhandled HUP signal" );
        break;
      case SIGPIPE:
        logwrite(function, "ignored SIGPIPE");
        break;
      default:
        message << "received unknown signal " << strsignal(signo);
        logwrite( function, message.str() );
        message.str(""); message << "NOTICE:" << Sequencer::DAEMON_NAME << " exit";
        Server::instance->sequence.async.enqueue( message.str() );
        break;
    }
    return;
  }
  /***** Sequencer::Server::handle_signal *************************************/


  /***** Sequencer::Server::exit_cleanly **************************************/
  /**
   * @brief      shutdown nicely
   *
   */
  void Server::exit_cleanly(void) {
    std::string function = "Sequencer::Server::exit_cleanly";
    logwrite( function, "exiting" );

    exit(EXIT_SUCCESS);
  }
  /***** Sequencer::Server::exit_cleanly **************************************/


  /***** Sequencer::Server::configure_sequencer *******************************/
  /**
   * @brief      read and apply the configuration file
   * @return     ERROR or NO_ERROR
   *
   */
  long Server::configure_sequencer() {
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
      catch (const std::exception &e) {   // should be impossible
        message.str(""); message << "ERROR parsing entry " << entry << " of " << this->config.n_entries
                                 << ": " << e.what();
        this->sequence.async.enqueue_and_log( function, message.str() );
        return ERROR;
      }

#ifdef LOGLEVEL_DEBUG
      message.str(""); message << "[DEBUG] configkey " << configkey << "=" << configval;
      logwrite( function, message.str() );
#endif

      // NBPORT
      if ( configkey == "NBPORT" ) {
        try {
          this->nbport = std::stoi( configval );
        }
        catch (const std::exception &e) {
          message.str(""); message << "ERROR parsing NBPORT: " << e.what();
          this->sequence.async.enqueue_and_log( function, message.str() );
          return ERROR;
        }
        message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->sequence.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // BLKPORT
      if (config.param[entry] == "BLKPORT") {
        try {
          this->blkport = std::stoi( config.arg[entry] );
        }
        catch (const std::exception &e) {
          message.str(""); message << "ERROR parsing BLKPORT: " << e.what();
          this->sequence.async.enqueue_and_log( function, message.str() );
          return ERROR;
        }
        message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->sequence.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // ASYNCPORT
      if (config.param[entry] == "ASYNCPORT") {
        try {
          this->asyncport = std::stoi( config.arg[entry] );
        }
        catch (const std::exception &e) {
          message.str(""); message << "ERROR parsing ASYNCPORT: " << e.what();
          this->sequence.async.enqueue_and_log( function, message.str() );
          return ERROR;
        }
        message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->sequence.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // MESSAGEPORT
      if (config.param[entry] == "MESSAGEPORT") {
        try {
          this->messageport = std::stoi( config.arg[entry] );
        }
        catch (const std::exception &e) {
          message.str(""); message << "ERROR parsing MESSAGEPORT: " << e.what();
          this->sequence.async.enqueue_and_log( function, message.str() );
          return ERROR;
        }
        message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->sequence.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // MESSAGEGROUP
      if (config.param[entry] == "MESSAGEGROUP") {
        this->messagegroup = config.arg[entry];
        message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->sequence.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // ACAMD_PORT
      if (config.param[entry] == "ACAMD_PORT") {
        try {
          this->sequence.acamd.port = std::stoi( config.arg[entry] );
        }
        catch (const std::exception &e) {
          message.str(""); message << "ERROR parsing ACAMD_PORT: " << e.what();
          this->sequence.async.enqueue_and_log( function, message.str() );
          return ERROR;
        }
        message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->sequence.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // CAMERAD_PORT
      if (config.param[entry] == "CAMERAD_PORT") {
        try {
          this->sequence.camerad.port = std::stoi( config.arg[entry] );
        }
        catch (const std::invalid_argument &e) {
          message.str(""); message << "ERROR parsing CAMERAD_PORT: " << e.what();
          this->sequence.async.enqueue_and_log( function, message.str() );
          return ERROR;
        }
        message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->sequence.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // CAMERAD_NBPORT
      if (config.param[entry] == "CAMERAD_NBPORT") {
        try {
          this->sequence.camerad.nbport = std::stoi( config.arg[entry] );
        }
        catch (const std::exception &e) {
          message.str(""); message << "ERROR parsing CAMERAD_NBPORT: " << e.what();
          this->sequence.async.enqueue_and_log( function, message.str() );
          return ERROR;
        }
        message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->sequence.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // FLEXURED_PORT
      if (config.param[entry] == "FLEXURED_PORT") {
        try {
          this->sequence.flexured.port = std::stoi( config.arg[entry] );
        }
        catch (const std::exception &e) {
          message.str(""); message << "ERROR parsing FLEXURED_PORT: " << e.what();
          this->sequence.async.enqueue_and_log( function, message.str() );
          return ERROR;
        }
        message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->sequence.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // POWERD_PORT
      if (config.param[entry] == "POWERD_PORT") {
        try {
          this->sequence.powerd.port = std::stoi( config.arg[entry] );
        }
        catch (const std::exception &e) {
          message.str(""); message << "ERROR parsing POWERD_PORT: " << e.what();
          this->sequence.async.enqueue_and_log( function, message.str() );
          return ERROR;
        }
        message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->sequence.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // SLICECAMD_PORT
      if (config.param[entry] == "SLICECAMD_PORT") {
        try {
          this->sequence.slicecamd.port = std::stoi( config.arg[entry] );
        }
        catch (const std::exception &e) {
          message.str(""); message << "ERROR parsing SLICECAMD_PORT: " << e.what();
          this->sequence.async.enqueue_and_log( function, message.str() );
          return ERROR;
        }
        message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->sequence.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // SLITD_PORT
      if (config.param[entry] == "SLITD_PORT") {
        try {
          this->sequence.slitd.port = std::stoi( config.arg[entry] );
        }
        catch (const std::exception &e) {
          message.str(""); message << "ERROR parsing SLITD_PORT: " << e.what();
          this->sequence.async.enqueue_and_log( function, message.str() );
          return ERROR;
        }
        message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->sequence.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // TCSD_PORT
      if (config.param[entry] == "TCSD_PORT") {
        try {
          this->sequence.tcsd.port = std::stoi( config.arg[entry] );
        }
        catch (const std::exception &e) {
          message.str(""); message << "ERROR parsing TCSD_PORT: " << e.what();
          this->sequence.async.enqueue_and_log( function, message.str() );
          return ERROR;
        }
        message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->sequence.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // CALIBD_PORT
      if (config.param[entry] == "CALIBD_PORT") {
        try {
          this->sequence.calibd.port = std::stoi( config.arg[entry] );
        }
        catch (const std::exception &e) {
          message.str(""); message << "ERROR parsing CALIBD_PORT: " << e.what();
          this->sequence.async.enqueue_and_log( function, message.str() );
          return ERROR;
        }
        message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->sequence.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // FILTERD_PORT
      if (config.param[entry] == "FILTERD_PORT") {
        try {
          this->sequence.filterd.port = std::stoi( config.arg[entry] );
        }
        catch (const std::exception &e) {
          message.str(""); message << "ERROR parsing FILTERD_PORT: " << e.what();
          this->sequence.async.enqueue_and_log( function, message.str() );
          return ERROR;
        }
        message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->sequence.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // FOCUSD_PORT
      if (config.param[entry] == "FOCUSD_PORT") {
        try {
          this->sequence.focusd.port = std::stoi( config.arg[entry] );
        }
        catch (const std::exception &e) {
          message.str(""); message << "ERROR parsing FOCUSD_PORT: " << e.what();
          this->sequence.async.enqueue_and_log( function, message.str() );
          return ERROR;
        }
        message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->sequence.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // ACQUIRE_TIMEOUT
      if (config.param[entry] == "ACQUIRE_TIMEOUT") {
        try {
          this->sequence.acquisition_timeout = std::stod( config.arg[entry] );
        }
        catch (const std::exception &e) {
          message.str(""); message << "ERROR parsing ACQUIRE_TIMEOUT: " << e.what();
          this->sequence.async.enqueue_and_log( function, message.str() );
          return ERROR;
        }
        message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->sequence.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // ACQUIRE_RETRYS
      if (config.param[entry].compare(0, 14, "ACQUIRE_RETRYS")==0) {
        int rt=-1;
        try {
          rt = std::stoi( config.arg[entry] );
        }
        catch (std::invalid_argument &) {
          message.str(""); message << "ACQUIRE_RETRYS: unable to convert " << config.arg[entry] << " to integer. retry limit disabled.";
          this->sequence.async.enqueue_and_log( function, message.str() );
          this->sequence.acquisition_max_retrys = -1;
        }
        catch (std::out_of_range &) {
          this->sequence.async.enqueue_and_log( function, "ACQUIRE_RETRYS number out of integer range. retry limit disabled." );
          this->sequence.acquisition_max_retrys = -1;
        }
        this->sequence.acquisition_max_retrys = rt;
        message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->sequence.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // TCS_OFFSET_RATE_RA
      if (config.param[entry].compare(0, 18, "TCS_OFFSET_RATE_RA")==0) {
        double mrate;
        try {
          mrate = std::stod( config.arg[entry] );
          if ( mrate < 0 || mrate > 60 ) {
            message.str(""); message << "ERROR: TCS_OFFSET_RATE_RA " << mrate << " out of range {0:60}";
            this->sequence.async.enqueue_and_log( function, message.str() );
            return( ERROR );
          }
        }
        catch (std::invalid_argument &) {
          message.str(""); message << "ERROR: bad TCS_OFFSET_RATE_RA: unable to convert " << config.arg[entry] << " to double";
          this->sequence.async.enqueue_and_log( function, message.str() );
          return(ERROR);
        }
        catch (std::out_of_range &) {
          this->sequence.async.enqueue_and_log( function, "ERROR: TCS_OFFSET_RATE_RA number out of double range" );
          return(ERROR);
        }
        this->sequence.tcs_offsetrate_ra = mrate;
        message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->sequence.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // TCS_OFFSET_RATE_DEC
      if (config.param[entry].compare(0, 18, "TCS_OFFSET_RATE_DEC")==0) {
        double mrate;
        try {
          mrate = std::stod( config.arg[entry] );
          if ( mrate < 0 || mrate > 60 ) {
            message.str(""); message << "ERROR: TCS_OFFSET_RATE_DEC " << mrate << " out of range {0:60}";
            this->sequence.async.enqueue_and_log( function, message.str() );
            return( ERROR );
          }
        }
        catch (std::invalid_argument &) {
          message.str(""); message << "ERROR: bad TCS_OFFSET_RATE_DEC: unable to convert " << config.arg[entry] << " to double";
          this->sequence.async.enqueue_and_log( function, message.str() );
          return(ERROR);
        }
        catch (std::out_of_range &) {
          this->sequence.async.enqueue_and_log( function, "ERROR: TCS_OFFSET_RATE_DEC number out of double range" );
          return(ERROR);
        }
        this->sequence.tcs_offsetrate_dec = mrate;
        message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->sequence.async.enqueue_and_log( function, message.str() );
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
          this->sequence.async.enqueue_and_log( function, message.str() );
          return(ERROR);
        }
        catch (std::out_of_range &) {
          this->sequence.async.enqueue_and_log( function, "ERROR: TCS_SETTLE_TIMEOUT number out of double range" );
          return(ERROR);
        }
        this->sequence.tcs_settle_timeout = to;
        message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->sequence.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // TCS_SETTLE_STABLE
      if (config.param[entry].compare(0, 17, "TCS_SETTLE_STABLE")==0) {
        double stablet;
        try {
          stablet = std::stod( config.arg[entry] );
        }
        catch (std::invalid_argument &) {
          message.str(""); message << "ERROR: bad TCS_SETTLE_STABLE: unable to convert " << config.arg[entry] << " to double";
          this->sequence.async.enqueue_and_log( function, message.str() );
          return(ERROR);
        }
        catch (std::out_of_range &) {
          this->sequence.async.enqueue_and_log( function, "ERROR: TCS_SETTLE_STABLE number out of double range" );
          return(ERROR);
        }
        this->sequence.tcs_settle_stable = stablet;
        message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->sequence.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // TCS_DOMEAZI_READY
      if (config.param[entry].compare(0, 17, "TCS_DOMEAZI_READY")==0) {
        double domeazi;
        try {
          domeazi = std::stod( config.arg[entry] );
        }
        catch (std::invalid_argument &) {
          message.str(""); message << "ERROR: bad TCS_DOMEAZI_READY: unable to convert " << config.arg[entry] << " to double";
          this->sequence.async.enqueue_and_log( function, message.str() );
          return(ERROR);
        }
        catch (std::out_of_range &) {
          this->sequence.async.enqueue_and_log( function, "ERROR: TCS_DOMEAZI_READY number out of double range" );
          return(ERROR);
        }
        this->sequence.tcs_domeazi_ready = domeazi;
        message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->sequence.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // TCS_PREAUTH_TIME
      if (config.param[entry].compare(0, 16, "TCS_PREAUTH_TIME")==0) {
        double to;
        try {
          to = std::stod( config.arg[entry] );
        }
        catch (std::invalid_argument &) {
          this->sequence.async.enqueue_and_log( function, "ERROR: bad TCS_PREAUTH_TIME: unable to convert to double" );
          return(ERROR);
        }
        catch (std::out_of_range &) {
          this->sequence.async.enqueue_and_log( function, "ERROR: TCS_PREAUTH_TIME number out of double range" );
          return(ERROR);
        }
        this->sequence.tcs_preauth_time = to;
        message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->sequence.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // ACQUIRE_OFFSET_THRESHOLD
      if (config.param[entry].compare( 0, ACQUIRE_OFFSET_THRESHOLD.length(), ACQUIRE_OFFSET_THRESHOLD )==0) {
        double offset;
        try {
          offset = std::stod( config.arg[entry] );
        }
        catch (std::invalid_argument &) {
          this->sequence.async.enqueue_and_log( function, "ERROR: bad ACQUIRE_OFFSET_THRESHOLD: unable to convert to double" );
          return(ERROR);
        }
        catch (std::out_of_range &) {
          this->sequence.async.enqueue_and_log( function, "ERROR: ACQUIRE_OFFSET_THRESHOLD number out of double range" );
          return(ERROR);
        }
        this->sequence.target.offset_threshold = offset;
        message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->sequence.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // ACQUIRE_MIN_REPEAT
      if (config.param[entry].compare( 0, ACQUIRE_MIN_REPEAT.length(), ACQUIRE_MIN_REPEAT )==0) {
        int repeat;
        try {
          repeat = std::stoi( config.arg[entry] );
        }
        catch (std::invalid_argument &) {
          this->sequence.async.enqueue_and_log( function, "ERROR: bad ACQUIRE_MIN_REPEAT: unable to convert to int" );
          return(ERROR);
        }
        catch (std::out_of_range &) {
          this->sequence.async.enqueue_and_log( function, "ERROR: ACQUIRE_MIN_REPEAT number out of int range" );
          return(ERROR);
        }
        this->sequence.target.min_repeat = repeat;
        message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->sequence.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // ACQUIRE_TCS_MAX_OFFSET
      if (config.param[entry].compare( 0, ACQUIRE_TCS_MAX_OFFSET.length(), ACQUIRE_TCS_MAX_OFFSET )==0) {
        double offset;
        try {
          offset = std::stod( config.arg[entry] );
        }
        catch (std::invalid_argument &) {
          this->sequence.async.enqueue_and_log( function, "ERROR: bad ACQUIRE_TCS_MAX_OFFSET: unable to convert to double" );
          return(ERROR);
        }
        catch (std::out_of_range &) {
          this->sequence.async.enqueue_and_log( function, "ERROR: ACQUIRE_TCS_MAX_OFFSET number out of double range" );
          return(ERROR);
        }
        this->sequence.target.max_tcs_offset = offset;
        message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->sequence.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      //
      // To configure the database parameters, call configure_db( param, value ) with the
      // parameter name and the value. This function will parse and perform error checking and
      // assign to the correct private variable.  If this returns NO_ERROR then increment applied.
      //

      // DB_HOST
      if (config.param[entry].compare( 0, DB_HOST.length(), DB_HOST )==0) {
        if ( this->sequence.target.configure_db( DB_HOST, config.arg[entry] ) == NO_ERROR ) {
          applied++;
          message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
          this->sequence.async.enqueue_and_log( function, message.str() );
        }
      }

      // DB_PORT
      if (config.param[entry].compare( 0, DB_PORT.length(), DB_PORT )==0) {
        if ( this->sequence.target.configure_db( DB_PORT, config.arg[entry] ) == NO_ERROR ) {
          applied++;
          message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
          this->sequence.async.enqueue_and_log( function, message.str() );
        }
      }

      // DB_USER
      if (config.param[entry].compare( 0, DB_USER.length(), DB_USER )==0) {
        if ( this->sequence.target.configure_db( DB_USER, config.arg[entry] ) == NO_ERROR ) {
          applied++;
          message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
          this->sequence.async.enqueue_and_log( function, message.str() );
        }
      }

      // DB_PASS
      if (config.param[entry].compare( 0, DB_PASS.length(), DB_PASS )==0) {
        if ( this->sequence.target.configure_db( DB_PASS, config.arg[entry] ) == NO_ERROR ) {
          applied++;
          message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
          this->sequence.async.enqueue_and_log( function, message.str() );
        }
      }

      // DB_SCHEMA
      if (config.param[entry].compare( 0, DB_SCHEMA.length(), DB_SCHEMA )==0) {
        if ( this->sequence.target.configure_db( DB_SCHEMA, config.arg[entry] ) == NO_ERROR ) {
          applied++;
          message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
          this->sequence.async.enqueue_and_log( function, message.str() );
        }
      }

      // DB_ACTIVE
      if (config.param[entry].compare( 0, DB_ACTIVE.length(), DB_ACTIVE )==0) {
        if ( this->sequence.target.configure_db( DB_ACTIVE, config.arg[entry] ) == NO_ERROR ) {
          applied++;
          message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
          this->sequence.async.enqueue_and_log( function, message.str() );
        }
      }

      // DB_COMPLETED
      if (config.param[entry].compare( 0, DB_COMPLETED.length(), DB_COMPLETED )==0) {
        if ( this->sequence.target.configure_db( DB_COMPLETED, config.arg[entry] ) == NO_ERROR ) {
          applied++;
          message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
          this->sequence.async.enqueue_and_log( function, message.str() );
        }
      }

      // DB_SETS
      if (config.param[entry].compare( 0, DB_SETS.length(), DB_SETS )==0) {
        if ( this->sequence.target.configure_db( DB_SETS, config.arg[entry] ) == NO_ERROR ) {
          applied++;
          message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
          this->sequence.async.enqueue_and_log( function, message.str() );
        }
      }

      // CAMERA_PREAMBLE
      if (config.param[entry].compare( 0, CAMERA_PREAMBLE.length(), CAMERA_PREAMBLE )==0) {
        this->sequence.camera_preamble.push_back( this->config.arg[entry] );
        applied++;
        message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->sequence.async.enqueue_and_log( function, message.str() );
      }

      // CAMERA_EPILOGUE
      if (config.param[entry].compare( 0, CAMERA_EPILOGUE.length(), CAMERA_EPILOGUE )==0) {
        this->sequence.camera_epilogue.push_back( this->config.arg[entry] );
        applied++;
        message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->sequence.async.enqueue_and_log( function, message.str() );
      }

      //
      // configure the power switch parameters
      //

      // POWER_SLIT
      if (config.param[entry].compare( 0, POWER_SLIT.length(), POWER_SLIT )==0) {
        if ( this->sequence.power_switch[POWER_SLIT].configure( this->config.arg[entry] ) == NO_ERROR ) {
          applied++;
          message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
          this->sequence.async.enqueue_and_log( function, message.str() );
        }
      }

      // POWER_CAMERA
      if (config.param[entry].compare( 0, POWER_CAMERA.length(), POWER_CAMERA )==0) {
        if ( this->sequence.power_switch[POWER_CAMERA].configure( this->config.arg[entry] ) == NO_ERROR ) {
          applied++;
          message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
          this->sequence.async.enqueue_and_log( function, message.str() );
        }
      }

      // POWER_CALIB
      if (config.param[entry].compare( 0, POWER_CALIB.length(), POWER_CALIB )==0) {
        if ( this->sequence.power_switch[POWER_CALIB].configure( this->config.arg[entry] ) == NO_ERROR ) {
          applied++;
          message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
          this->sequence.async.enqueue_and_log( function, message.str() );
        }
      }

      // POWER_FLEXURE
      if (config.param[entry].compare( 0, POWER_FLEXURE.length(), POWER_FLEXURE )==0) {
        if ( this->sequence.power_switch[POWER_FLEXURE].configure( this->config.arg[entry] ) == NO_ERROR ) {
          applied++;
          message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
          this->sequence.async.enqueue_and_log( function, message.str() );
        }
      }

      // POWER_FILTER
      if (config.param[entry].compare( 0, POWER_FILTER.length(), POWER_FILTER )==0) {
        if ( this->sequence.power_switch[POWER_FILTER].configure( this->config.arg[entry] ) == NO_ERROR ) {
          applied++;
          message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
          this->sequence.async.enqueue_and_log( function, message.str() );
        }
      }

      // POWER_FOCUS
      if (config.param[entry].compare( 0, POWER_FOCUS.length(), POWER_FOCUS )==0) {
        if ( this->sequence.power_switch[POWER_FOCUS].configure( this->config.arg[entry] ) == NO_ERROR ) {
          applied++;
          message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
          this->sequence.async.enqueue_and_log( function, message.str() );
        }
      }

      // POWER_TELEM
      if (config.param[entry].compare( 0, POWER_TELEM.length(), POWER_TELEM )==0) {
        if ( this->sequence.power_switch[POWER_TELEM].configure( this->config.arg[entry] ) == NO_ERROR ) {
          applied++;
          message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
          this->sequence.async.enqueue_and_log( function, message.str() );
        }
      }

      // POWER_THERMAL
      if (config.param[entry].compare( 0, POWER_THERMAL.length(), POWER_THERMAL )==0) {
        if ( this->sequence.power_switch[POWER_THERMAL].configure( this->config.arg[entry] ) == NO_ERROR ) {
          applied++;
          message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
          this->sequence.async.enqueue_and_log( function, message.str() );
        }
      }

      // POWER_ACAM
      if (config.param[entry].compare( 0, POWER_ACAM.length(), POWER_ACAM )==0) {
        if ( this->sequence.power_switch[POWER_ACAM].configure( this->config.arg[entry] ) == NO_ERROR ) {
          applied++;
          message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
          this->sequence.async.enqueue_and_log( function, message.str() );
        }
      }

      // POWER_SLICECAM
      if (config.param[entry].compare( 0, POWER_SLICECAM.length(), POWER_SLICECAM )==0) {
        if ( this->sequence.power_switch[POWER_SLICECAM].configure( this->config.arg[entry] ) == NO_ERROR ) {
          applied++;
          message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
          this->sequence.async.enqueue_and_log( function, message.str() );
        }
      }

      // TELEM_PROVIDER : contains daemon name and port to contact for header telemetry info
      //
      if ( config.param[entry] == "TELEM_PROVIDER" ) {
        std::vector<std::string> tokens;
        Tokenize( config.arg[entry], tokens, " " );
        try {
          if ( tokens.size() == 2 ) {
            this->sequence.telemetry_providers[tokens.at(0)] = std::stod(tokens.at(1));
          }
          else {
            message.str(""); message << "ERROR bad format TELEM_PROVIDER=\"" << config.arg[entry] << "\": expected <name> <port>";
            logwrite( function, message.str() );
            return ERROR;
          }
        }
        catch ( const std::exception &e ) {
          message.str(""); message << "ERROR parsing TELEM_PROVIDER from " << config.arg[entry] << ": " << e.what();
          logwrite( function, message.str() );
          return ERROR;
        }
        message.str(""); message << "config:" << config.param[entry] << "=" << config.arg[entry];
        this->sequence.async.enqueue_and_log( "SEQUENCERD", function, message.str() );
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
    message << "applied " << applied << " configuration lines to sequencer";
    logwrite(function, message.str());

    return error;
  }
  /***** Sequencer::Server::configure_sequencer *******************************/


  /***** Sequencer::Server::new_log_day ***************************************/
  /**
   * @brief      creates a new logbook each day
   * @param[in]  logpath  path for the log file, read from config file
   *
   * This thread is started by main and never terminates.
   * It sleeps for the number of seconds that logentry determines
   * are remaining in the day, then closes and re-inits a new log file.
   *
   * The number of seconds until the next day "nextday" is a global which
   * is set by init_log.
   *
   */
  void Server::new_log_day( std::string logpath ) { 
    while (1) {
      std::this_thread::sleep_for( std::chrono::seconds( nextday ) );
      close_log();
      init_log( logpath, Sequencer::DAEMON_NAME );
    }
  }
  /***** Sequencer::Server::new_log_day ***************************************/


  /***** Server::block_main ***************************************************/
  /**
   * @brief      main function for blocking connection thread
   * @param[in]  seq   reference to Sequencer::Server object
   * @param[in]  sock  Network::TcpSocket socket object
   *
   * accepts a socket connection and processes the request by
   * calling function doit()
   *
   * This thread never terminates.
   *
   */
  void Server::block_main( Sequencer::Server &seq, Network::TcpSocket sock ) {
    while(1) {
      sock.Accept();
      sock.Write("CONNECTED\n");
      seq.doit(seq, sock);          // call function to do the work
      sock.Close();
    }
    return;
  }
  /***** Server::block_main ***************************************************/


  /**** Server::thread_main ***************************************************/
  /**
   * @brief      main function for all non-blocked threads
   * @param[in]  seq   reference to Sequencer::Server object
   * @param[in]  sock  Network::TcpSocket socket object
   *
   * accepts a socket connection and processes the request by
   * calling function doit()
   *
   * There are N_THREADS-1 of these, one for each non-blocking connection.
   * These threads never terminate.
   *
   * This function differs from block_main only in that the call to Accept
   * is mutex-protected.
   *
   */
  void Server::thread_main( Sequencer::Server &seq, Network::TcpSocket &sock ) {
    while (1) {
      seq.conn_mutex.lock();
      sock.Accept();
      seq.conn_mutex.unlock();
#ifdef LOGLEVEL_DEBUG
      std::stringstream message;
      message.str(""); message << "[DEBUG] thread " << sock.id << " spawning doit to handle connection on fd " << sock.getfd();
      logwrite( "Server::thread_main", message.str() );
#endif
      std::thread( doit, std::ref(seq), std::ref(sock) ).detach();  // spawn a thread to handle this connection
    }
    return;
  }
  /**** Server::thread_main ***************************************************/


  /**** Server::gui_main ******************************************************/
  /**
   * @brief      main function for gui thread
   * @param[in]  seq   reference to Sequencer::Server object
   * @param[in]  sock  Network::TcpSocket socket object
   *
   * accepts a socket connection and processes the request by
   * calling function doit()
   *
   * There is one of these, and it never terminates.
   *
   * This function differs from block_main only in that doit is spawned in its own thread.
   *
   */
  void Server::gui_main( Sequencer::Server &seq, Network::TcpSocket &sock ) {
    while (1) {
      sock.Accept();
      sock.Write("CONNECTED\n");
      std::thread( doit, std::ref(seq), std::ref(sock) ).detach();  // spawn a thread to handle this connection
    }
    return;
  }
  /**** Server::gui_main ******************************************************/


  /***** Server::async_main ***************************************************/
  /**
   * @brief      asynchronous message sending thread
   * @param[in]  seq   reference to Sequencer::Server object
   * @param[in]  sock  Network::udpSocket socket object
   *
   * Loops forever, when a message arrives in the status message queue it is
   * sent out via multi-cast UDP datagram.
   *
   */
  void Server::async_main( Sequencer::Server &seq, Network::UdpSocket sock ) {
    std::string function = "Sequencer::Server::async_main";
    std::stringstream message;
    int retval;

    retval = sock.Create();                                   // create the UDP socket
    if (retval < 0) {
      logwrite(function, "error creating UDP multicast socket for asynchronous messages");
      seq.exit_cleanly();                                     // do not continue on error
    }
    if (retval==1) {                                          // exit this thread but continue with daemon
      logwrite(function, "asyncrhonous message port disabled by request");
    }

    while (1) {
      std::string message = seq.sequence.async.dequeue();     // get the latest message from the queue (blocks)
      retval = sock.Send(message);                            // transmit the message
      if (retval < 0) {
        std::stringstream errstm;
        errstm << "error sending UDP message: " << message;
        logwrite(function, errstm.str());
      }
      if (message=="exit") {                                  // terminate this thread
        sock.Close();
        return;
      }
    }
    
    return;
  }
  /***** Server::async_main ***************************************************/


  /***** Server::doit *********************************************************/
  /**
   * @brief      the workhorse of each thread connetion
   * @param[in]  sock  Network::UdpSocket socket object
   *
   * stays open until closed by client
   *
   * commands come in the form: 
   *   "<device> [all|<app>] [_BLOCK_] <command> [<arg>]"
   *
   */
  void Server::doit( Sequencer::Server &seq, Network::TcpSocket &sock ) {
    std::string function = "Sequencer::Server::doit";
    long  ret;
    std::stringstream message;
    std::string cmd, args;        // arg string is everything after command
    std::vector<std::string> tokens;

    bool connection_open=true;

    message.str(""); message << "thread " << sock.id << " accepted "
                             << (sock.isasync() ? "ASYNC " : "" )
                             << (sock.isblocking() ? "BLOCKING " : "NON-BLOCKING " )
                             << "connection on fd " << sock.getfd()
                             << " port " << sock.getport();
    logwrite( function, message.str() );

    while ( connection_open ) {

      // Wait (poll) connected socket for incoming data...
      //
      int pollret;
      if ( ( pollret=sock.Poll() ) <= 0 ) {
        if (pollret==0) {
          message.str(""); message << "ERROR: Poll timeout on fd " << sock.getfd() << " thread " << sock.id;
          seq.sequence.async.enqueue_and_log( function, message.str() );
        }
        if ( pollret <0 && errno ) {
          message.str(""); message << "ERROR: Poll error on fd " << sock.getfd() << " thread " << sock.id << ": " << strerror(errno);
          seq.sequence.async.enqueue_and_log( function, message.str() );
        }
        break;                      // this will close the connection
      }

      // Data available, now read from connected socket...
      //
      std::string buf;
      char delim='\n';
      if ( ( ret=sock.Read( buf, delim ) ) <= 0 ) {
#ifdef LOGLEVEL_DEBUG
        message.str(""); message << "[DEBUG] ret=" << ret << " buf=\"" << buf << "\"";
        logwrite( function, message.str() );
#endif
        if ( ret<0 && errno ) {     // could be an actual read error
          message.str(""); message << "ERROR: Read error on fd " << sock.getfd() << ": " << strerror(errno);
          seq.sequence.async.enqueue_and_log( function, message.str() );
        }
        if (ret==-2) {              // or a timeout
          message.str(""); message << "ERROR: timeout reading from fd " << sock.getfd();
          seq.sequence.async.enqueue_and_log( function, message.str() );
        }
        break;                      // Breaking out of the while loop will close the connection.
                                    // This probably means that the client has terminated abruptly, 
                                    // having sent FIN but not stuck around long enough
                                    // to accept CLOSE and give the LAST_ACK.
      }

      // convert the input buffer into a string and remove any trailing linefeed
      // and carriage return
      //
      buf.erase(std::remove(buf.begin(), buf.end(), '\r' ), buf.end());
      buf.erase(std::remove(buf.begin(), buf.end(), '\n' ), buf.end());

      if (buf.empty()) {sock.Write("\n"); continue;}   // acknowledge empty command so client doesn't time out

      try {
        std::size_t cmd_sep = buf.find_first_of(" ");  // find the first space, which separates command from argument list

        cmd = buf.substr(0, cmd_sep);                  // cmd is everything up until that space

        if (cmd.empty()) {sock.Write("\n"); continue;} // acknowledge empty command so client doesn't time out

        if (cmd_sep == std::string::npos) {            // If no space was found,
          args.clear();                                // then the arg list is empty,
        }
        else {
          args= buf.substr(cmd_sep+1);                 // otherwise args is everything after that space.
        }

        sock.id = ++seq.cmd_num;
        if ( seq.cmd_num == INT_MAX ) seq.cmd_num = 0;

        message.str(""); message << "received command (" << sock.id << ") on fd " << sock.getfd() << ": " << cmd << " " << args;
        logwrite(function, message.str());
      }
      catch ( std::runtime_error &e ) {
        std::stringstream errstream; errstream << e.what();
        message.str(""); message << "ERROR: parsing arguments: " << errstream.str();
        seq.sequence.async.enqueue_and_log( function, message.str() );
        ret = -1;
      }
      catch ( ... ) {
        message.str(""); message << "ERROR: unknown error parsing arguments: " << args;
        seq.sequence.async.enqueue_and_log( function, message.str() );
        ret = -1;
      }

      /**
       * process commands here
       */
      ret = NOTHING;
      std::string retstring;

      if ( cmd == "-h" || cmd == "--help" || cmd == "help" || cmd == "?" ) {
                      retstring="seq { <CMD> } [<ARG>...]\n";
                      retstring.append( "  where <CMD> is one of:\n" );
                      for ( const auto &s : SEQUENCERD_SYNTAX ) {
                        retstring.append("  "); retstring.append( s ); retstring.append( "\n" );
                      }
                      ret = HELP;
      }
      else
      if ( cmd.compare( SEQUENCERD_EXIT )==0 ) {
                      seq.exit_cleanly();                        // shutdown the sequencer
      }
      else

      // These commands go to acamd
      //
      if ( cmd.compare( SEQUENCERD_ACAM )==0 ) {
                      if ( ( strncasecmp( args.c_str(), "help", 4 ) == 0 ) || ( args.find("?") != std::string::npos ) ) {
                        message.str(""); message << "ERROR command \"" << cmd << " " << args << "\" not allowed from sequencer";
                        logwrite( function, message.str() );
                        ret = ERROR;
                      }
                      else
                      if ( sock.isasync() ) {
                        std::thread( std::ref( Common::DaemonClient::dothread_command ), std::ref( seq.sequence.acamd ), args ).detach();
                      }
                      else {
                        ret = seq.sequence.acamd.command( args, retstring );
                        if ( !retstring.empty() ) {
                          message.str(""); message << "acamd reply (" << sock.id << "): " << retstring;
                          logwrite( function, message.str() );
                          retstring.append( " " );
                        }
                      }
      }
      else

      // These commands go to calibd
      //
      if ( cmd.compare( SEQUENCERD_CALIB )==0 ) {
                      if ( ( strncasecmp( args.c_str(), "help", 4 ) == 0 ) || ( args.find("?") != std::string::npos ) ) {
                        message.str(""); message << "ERROR command \"" << cmd << " " << args << "\" not allowed from sequencer";
                        logwrite( function, message.str() );
                        ret = ERROR;
                      }
                      else
                      if ( sock.isasync() ) {
                        std::thread( std::ref( Common::DaemonClient::dothread_command ), std::ref( seq.sequence.calibd ), args ).detach();
                      }
                      else {
                        ret = seq.sequence.calibd.command( args, retstring );
                        if ( !retstring.empty() ) {
                          message.str(""); message << "calibd reply (" << sock.id << "): " << retstring;
                          logwrite( function, message.str() );
                          retstring.append( " " );
                        }
                      }
      }
      else

      // These commands go to camerad
      //
      if ( cmd.compare( SEQUENCERD_CAMERA )==0 ) {
                      if ( ( strncasecmp( args.c_str(), "help", 4 ) == 0 ) || ( args.find("?") != std::string::npos ) ) {
                        message.str(""); message << "ERROR command \"" << cmd << " " << args << "\" not allowed from sequencer";
                        logwrite( function, message.str() );
                        ret = ERROR;
                      }
                      else
                      if ( sock.isasync() ) {
                        std::thread( std::ref( Common::DaemonClient::dothread_command ), std::ref( seq.sequence.camerad ), args ).detach();
                      }
                      else {
                        ret = seq.sequence.camerad.command( args, retstring );
                        if ( !retstring.empty() ) {
                          message.str(""); message << "camerad reply (" << sock.id << "): " << retstring;
                          logwrite( function, message.str() );
                          retstring.append( " " );
                        }
                      }
      }
      else

      // These commands go to filterd
      //
      if ( cmd.compare( SEQUENCERD_FILTER )==0 ) {
                      if ( ( strncasecmp( args.c_str(), "help", 4 ) == 0 ) || ( args.find("?") != std::string::npos ) ) {
                        message.str(""); message << "ERROR command \"" << cmd << " " << args << "\" not allowed from sequencer";
                        logwrite( function, message.str() );
                        ret = ERROR;
                      }
                      else
                      if ( sock.isasync() ) {
                        std::thread( std::ref( Common::DaemonClient::dothread_command ), std::ref( seq.sequence.filterd ), args ).detach();
                      }
                      else {
                        ret = seq.sequence.filterd.command( args, retstring );
                        if ( !retstring.empty() ) {
                          message.str(""); message << "filterd reply (" << sock.id << "): " << retstring;
                          logwrite( function, message.str() );
                          retstring.append( " " );
                        }
                      }
      }
      else

      // These commands go to powerd
      //
      if ( cmd.compare( SEQUENCERD_POWER )==0 ) {
                      if ( ( strncasecmp( args.c_str(), POWERD_LIST.c_str(), POWERD_LIST.size() ) == 0 ) ||
                           ( strncasecmp( args.c_str(), "help", 4 ) == 0 ) || ( args.find("?") != std::string::npos ) ) {
                        message.str(""); message << "ERROR command \"" << cmd << " " << args << "\" not allowed from sequencer";
                        logwrite( function, message.str() );
                        ret = ERROR;
                      }
                      else
                      if ( sock.isasync() ) {
                        std::thread( std::ref( Common::DaemonClient::dothread_command ),
                                     std::ref( seq.sequence.powerd ), args ).detach();
                      }
                      else {
                        ret = seq.sequence.powerd.command( args, retstring );
                        if ( !retstring.empty() ) {
                          message.str(""); message << "powerd reply (" << sock.id << "): " << retstring;
                          logwrite( function, message.str() );
                          retstring.append( " " );
                        }
                      }
      }
      else

      // These commands go to slitd
      //
      if ( cmd.compare( SEQUENCERD_SLIT )==0 ) {
                      if ( ( strncasecmp( args.c_str(), "help", 4 ) == 0 ) || ( args.find("?") != std::string::npos ) ) {
                        message.str(""); message << "ERROR command \"" << cmd << " " << args << "\" not allowed from sequencer";
                        logwrite( function, message.str() );
                        ret = ERROR;
                      }
                      else
                      if ( sock.isasync() ) {
                        std::thread( std::ref( Common::DaemonClient::dothread_command ), std::ref( seq.sequence.slitd ), args ).detach();
                      }
                      else {
                        ret = seq.sequence.slitd.command( args, retstring );
                        if ( !retstring.empty() ) {
                          message.str(""); message << "slitd reply (" << sock.id << "): " << retstring;
                          logwrite( function, message.str() );
                          retstring.append( " " );
                        }
                      }
      }
      else

      // These commands go to tcsd
      //
      if ( cmd.compare( SEQUENCERD_TCS )==0 ) {
                      if ( ( strncasecmp( args.c_str(), "help", 4 ) == 0 ) || ( args.find("?") != std::string::npos ) ) {
                        message.str(""); message << "ERROR command \"" << cmd << " " << args << "\" not allowed from sequencer";
                        logwrite( function, message.str() );
                        ret = ERROR;
                      }
                      else
                      if ( sock.isasync() ) {
                        std::thread( std::ref( Common::DaemonClient::dothread_command ), std::ref( seq.sequence.tcsd ), args ).detach();
                      }
                      else {
                        ret = seq.sequence.tcsd.command( args, retstring );
                        if ( !retstring.empty() ) {
                          message.str(""); message << "tcsd reply (" << sock.id << "): " << retstring;
                          logwrite( function, message.str() );
                          retstring.append( " " );
                        }
                      }
      }
      else

      // system startup (nightly)
      // This is needed before any sequences can be run.
      //
      if ( cmd.compare( SEQUENCERD_STARTUP ) == 0 ) {
///                   seq.sequence.is_tcs_ontarget.store( false );
                      seq.sequence.tcs_nowait.store( false );
                      seq.sequence.dome_nowait.store( true );
                      if ( sock.isasync() ) {
                        std::thread( std::ref( Sequencer::Sequence::dothread_startup ), std::ref( seq.sequence ) ).detach();
                      }
                      else {
                        ret = seq.sequence.startup( seq.sequence );
                      }
      }
      else

      // system shutdown
      // This shuts down the instrument.
      //
      if ( cmd.compare( SEQUENCERD_SHUTDOWN ) == 0 ) {
                      if ( sock.isasync() ) {
                        std::thread( std::ref( Sequencer::Sequence::dothread_shutdown ), std::ref( seq.sequence ) ).detach();
                      }
                      else {
                        ret = seq.sequence.shutdown( seq.sequence );
                      }
      }
      else

      // Sequence "start"
      //
      if ( cmd.compare( SEQUENCERD_START )==0 ) {
///                   seq.sequence.is_tcs_ontarget.store( false );
                      seq.sequence.tcs_nowait.store( false );
                      seq.sequence.dome_nowait.store( true );
                      // The Sequencer can only be started if it is SEQ_READY (and no other bits set)
                      //
                      if ( seq.sequence.seqstate.load() != Sequencer::SEQ_READY ) {
                        // log applicable causes
                        //
                        if ( seq.sequence.seq_state.is_set( Sequencer::SEQ_RUNNING ) ) {
                          seq.sequence.async.enqueue_and_log( function, "ERROR: sequencer already running" );
                        }
                        else
                        if ( seq.sequence.seq_state.is_any_set( Sequencer::SEQ_ABORTREQ, Sequencer::SEQ_STOPREQ ) ) {
                          seq.sequence.async.enqueue_and_log( function, "ERROR: sequencer waiting for stop" );
                        }
                        else
                        if ( seq.sequence.seq_state.is_set( Sequencer::SEQ_OFFLINE ) ) {
                          seq.sequence.async.enqueue_and_log( function, "ERROR: sequencer is offline. run startup" );
                        }
                        else {
                          seq.sequence.async.enqueue_and_log( function, "ERROR: unable to start sequencer" );
                        }
                        ret = ERROR;
                      }

                      // seqstate is SEQ_READY so change both it and reqstate to SEQ_RUNNING,
                      // then spawn a thread to start
                      //
                      else {
                        seq.sequence.seq_state.set_and_clear( Sequencer::SEQ_RUNNING, Sequencer::SEQ_READY );  // set RUNNING, clear READY
                        seq.sequence.req_state.set_and_clear( Sequencer::SEQ_RUNNING, Sequencer::SEQ_READY );
                        seq.sequence.broadcast_seqstate();

                        std::thread( std::ref( Sequencer::Sequence::dothread_sequence_start ),
                                     std::ref( seq.sequence) ).detach();
                        ret = NO_ERROR;
                      }
      }
      else

      // Stop an Exposure
      //
      if ( cmd.compare( SEQUENCERD_STOP ) == 0 ) {
                      // Can only stop during an active or paused exposure
                      //
                      if ( ! seq.sequence.seq_state.is_any_set( Sequencer::SEQ_PAUSE, Sequencer::SEQ_WAIT_EXPOSE ) ) {
                        seq.sequence.async.enqueue_and_log( function, "ERROR: can only stop during an active or paused exposure" );
                        ret = ERROR;
                      }

                      // To request a stop, set the STOPREQ bit in both seqstate and reqstate.
                      //
                      else {
                        logwrite( function, "stop requested" );

                        seq.sequence.req_state.set_and_clear( Sequencer::SEQ_STOPREQ, Sequencer::SEQ_RUNNING );
                        seq.sequence.seq_state.set_and_clear( {Sequencer::SEQ_STOPREQ}, {Sequencer::SEQ_RUNNING,Sequencer::SEQ_WAIT_EXPOSE} );  // set STOPREQ, clear RUNNING|EXPOSE
                        seq.sequence.broadcast_seqstate();

                        // If exposing then modify the exposure time to end immediately (-1)
                        //
                        if ( seq.sequence.seq_state.is_set( Sequencer::SEQ_WAIT_EXPOSE ) ) {
                          seq.sequence.dothread_modify_exptime( std::ref( seq.sequence ), -1 );
                        }

                        // If paused then send the RESUME command to the camera daemon.
                        //
                        if ( seq.sequence.seq_state.is_set( Sequencer::SEQ_PAUSE ) ) {
                          seq.sequence.camerad.async( CAMERAD_RESUME );                // tell the camera to resume exposure
                          seq.sequence.seq_state.clear( Sequencer::SEQ_PAUSE );        // clear the PAUSE bit
                          seq.sequence.broadcast_seqstate();
                        }

///                     // If not already running then spawn a thread to wait for this state,
///                     // which will send out any needed notifications.
///                     //
///                     if ( not seq.sequence.waiting_for_state.load() ) {
///                       std::thread( seq.sequence.dothread_wait_for_state, std::ref(seq.sequence) ).detach();
///                     }
                        ret = NO_ERROR;
                      }
      }
      else

      // Abort can abort nearly every operation (exposure, slew, etc.).
      // Only "stopping", "starting", and "aborting" cannot be aborted.
      // To abort, set the ABORTREQ bit in both seqstate and reqstate.
      //
      if ( cmd.compare( SEQUENCERD_ABORT ) == 0 ) {
                      // don't allow an abort unless SEQ_RUNNING or SEQ_PAUSED bit is set
                      //
                      if ( seq.sequence.seq_state.is_any_set( Sequencer::SEQ_RUNNING,
                                                              Sequencer::SEQ_PAUSE ) ) {
                        logwrite( function, "abort requested" );

                        // If exposing then asynchronously send the ABORT command to the camera daemon
                        //
                        if ( seq.sequence.seq_state.is_set( Sequencer::SEQ_WAIT_EXPOSE ) ) {
                          seq.sequence.camerad.async( CAMERAD_ABORT );                       // tell camera to abort exposure
                          seq.sequence.seq_state.clear( Sequencer::SEQ_WAIT_EXPOSE );        // clear the EXPOSE bit
                          seq.sequence.broadcast_seqstate();
                        }

//                      {
//                        I was here
//                      std::lock_guard<std::mutex> lock(seq.tcs_ontarget_mtx); // Lock the mutex
                        seq.sequence.req_state.set_and_clear( Sequencer::SEQ_ABORTREQ, Sequencer::SEQ_RUNNING );
                        seq.sequence.seq_state.set_and_clear( {Sequencer::SEQ_ABORTREQ},
                                                              {Sequencer::SEQ_RUNNING,Sequencer::SEQ_WAIT_EXPOSE} );  // clear RUNNING|EXPOSE
                        seq.sequence.broadcast_seqstate();

                        // Set the do-type to single-step
                        //
                        seq.sequence.dotype( "ONE" );

                        // Update the target state to unassigned
                        //
                        ret = seq.sequence.target.update_state( Sequencer::TARGET_UNASSIGNED );
                        message.str(""); message << ( ret==NO_ERROR ? "" : "ERROR " ) << "marking target " << seq.sequence.target.name
                                                 << " id " << seq.sequence.target.obsid << " order " << seq.sequence.target.obsorder
                                                 << " as " << Sequencer::TARGET_UNASSIGNED;
                        logwrite( function, message.str() );

                        // If this thread is not already running then spawn a thread to wait for this state,
                        // which will send out any needed notifications.
                        //
                        if ( ! seq.sequence.waiting_for_state.load() ) {
#ifdef LOGLEVEL_DEBUG
                          logwrite( function, "[DEBUG] spawning waiting_for_state thread" );
#endif
                          std::thread( seq.sequence.dothread_wait_for_state, std::ref(seq.sequence) ).detach();
                        }

//                      {
//                      std::lock_guard<std::mutex> lock(seq.tcs_ontarget_mtx); // Lock the mutex
//                      seq.set_seqstate(Sequencer::SEQ_ABORTREQ); // Update the state
//                      }
//                      seq.tcs_ontarget_cv.notify_all(); // Notify waiting threads

///                     seq.sequence.tcs_ontarget_cv.notify_all();
                        ret |= NO_ERROR;
                      }
                      else {
                        seq.sequence.async.enqueue_and_log( function, "ERROR: cannot abort unless sequencer is running or paused" );
                        ret = ERROR;
                      }
      }
      else

      // The TCS operator tells the Sequencer when the target is ready to observe
      // because there is currently no remote command that provides this information.
      // Clearing the SEQ_WAIT_TCSOP state means no longer waiting for the TCS operator.
      // This will notify the waiting thread which will proceed with the observation.
      //
      if ( cmd == SEQUENCERD_ONTARGET ) {
                      seq.sequence.seq_state.clear( Sequencer::SEQ_WAIT_TCSOP );
///                   seq.sequence.is_tcs_ontarget.store( true );
                      ret = NO_ERROR;
      }
      else

      // skip slew
      //
      if ( cmd.compare( SEQUENCERD_TCSNOWAIT ) == 0) {
                      seq.sequence.tcs_nowait.store( true );
                      ret = NO_ERROR;
      }
      else

      // skip dome
      //
      if ( cmd.compare( SEQUENCERD_DOMENOWAIT ) == 0) {
                      seq.sequence.dome_nowait.store( true );
                      ret = NO_ERROR;
      }
      else

      // Set the "Do-Type"
      // which can be single-step (do one) or continuous (do all).
      //
      if ( cmd.compare( SEQUENCERD_DOTYPE ) == 0) {
                      ret = seq.sequence.dotype( args, retstring );
                      retstring.append( " " );
      }
      else

      // Report the Sequencer State,
      // which will be returned, logged, and written to the async message port.
      //
      if ( cmd.compare( SEQUENCERD_STATE ) == 0) {
                      seq.sequence.broadcast_seqstate();
                      retstring = seq.sequence.seqstate_string( seq.sequence.seqstate.load() );
                      ret = NO_ERROR;
      }
      else

      // Set/Get the Target Set ID
      //
      if ( cmd.compare( SEQUENCERD_TARGETSET ) == 0) {
                      ret= seq.sequence.target.targetset( args, retstring );
                      message.str(""); message << "TARGETSET: " << retstring;
                      seq.sequence.async.enqueue( message.str() );
                      retstring.append( " " );
      }
      else

      // Pause an Exposure
      //
      if ( cmd.compare( SEQUENCERD_PAUSE ) == 0) {
                      // Can only pause during an exposure
                      //
                      if ( seq.sequence.seq_state.is_clear( Sequencer::SEQ_WAIT_EXPOSE ) ) {
                        seq.sequence.async.enqueue_and_log( function, "ERROR: can only pause during an active exposure" );
                        ret = ERROR;
                      }
                      else
                      // Can't already be paused
                      //
                      if ( seq.sequence.seq_state.is_set( Sequencer::SEQ_PAUSE ) ) {
                        seq.sequence.async.enqueue_and_log( function, "ERROR: already paused" );
                        ret = ERROR;
                      }
                      else {
                        seq.sequence.camerad.async( CAMERAD_PAUSE );
                        seq.sequence.seq_state.set_and_clear( Sequencer::SEQ_PAUSE, Sequencer::SEQ_RUNNING );  // set pause, clear running
                        seq.sequence.broadcast_seqstate();
                        ret = NO_ERROR;
                      }
      }
      else

      // Resume a Paused exposure
      //
      if ( cmd.compare( SEQUENCERD_RESUME ) == 0) {
                      // Can only resume when paused
                      //
                      if ( seq.sequence.seq_state.is_clear( Sequencer::SEQ_PAUSE ) ) {
                        seq.sequence.async.enqueue_and_log( function, "ERROR: can only resume when paused" );
                        ret = ERROR;
                      }
                      else {
                        seq.sequence.camerad.async( CAMERAD_RESUME );
                        seq.sequence.seq_state.set_and_clear( Sequencer::SEQ_RUNNING, Sequencer::SEQ_PAUSE );  // set running, clear pause
                        seq.sequence.broadcast_seqstate();
                        ret = NO_ERROR;
                      }
      }
      else

      // Call TCS Initialization
      //
      if ( cmd.compare( SEQUENCERD_TCSINIT ) == 0 ) {
                      ret = seq.sequence.tcs_init( args, retstring );
                      if ( ! retstring.empty() ) retstring.append( " " );
      }
      else

      // Call Test Routines,
      // routine specified by args.
      //
      if ( cmd.compare( SEQUENCERD_TEST ) == 0 ) {
                      ret = seq.sequence.test( args, retstring );
                      if ( not retstring.empty() ) retstring.append( " " );
      }
      else

      // Modify Exptime needs a single argument, the new exptime
      //
      if ( cmd.compare( SEQUENCERD_MODEXPTIME ) == 0 ) {
                      Tokenize( args, tokens, " " );
                      if ( tokens.size() != 1 ) {
                        seq.sequence.async.enqueue_and_log( function, "ERROR: expected MODEXPTIME <exptime>" );
                        ret = ERROR;
                      }
                      else {
                        // convert the single argument from string to double
                        //
                        double exptime_req=0;
                        try { exptime_req = std::stod( tokens.at(0) ); }
                        catch( std::out_of_range &e ) {
                          message.str(""); message << "ERROR: out of range parsing args " << args << ": " << e.what();
                          seq.sequence.async.enqueue_and_log( function, message.str() );
                          ret = ERROR;
                        }
                        catch( std::invalid_argument &e ) {
                          message.str(""); message << "ERROR: invalid argument parsing args " << args << ": " << e.what();
                          seq.sequence.async.enqueue_and_log( function, message.str() );
                          ret = ERROR;
                        }

                        // spawn a thread to modify the exposure time
                        //
                        std::thread( std::ref( Sequencer::Sequence::dothread_modify_exptime ),
                                     std::ref( seq.sequence), exptime_req ).detach();
                        ret = NO_ERROR;
                      }
      }
      else

      // Configure sequencer (read config file and apply)
      //
      if ( cmd.compare( SEQUENCERD_CONFIG ) == 0 ) {
                      ret = seq.config.read_config(seq.config);  // read configuration file specified on command line
                      if ( ret != NO_ERROR ) logwrite(function, "ERROR: unable to load config file");
                      else ret = seq.configure_sequencer();
      }
      else

      // send my telemetry upon request
      //
      if ( cmd == TELEMREQUEST ) {
                      if ( args=="?" || args=="help" ) {
                        retstring=TELEMREQUEST+"\n";
                        retstring.append( "  Returns a serialized JSON message containing my telemetry\n" );
                        retstring.append( "  information, terminated with \"EOF\\n\".\n" );
                        ret=HELP;
                      }
                      else {
                        seq.sequence.make_telemetry_message( retstring );
                        ret = JSON;
                      }
      }

      // Unknown commands generate an error
      //
      else {
        message.str(""); message << "ERROR: unknown command: " << cmd;
        seq.sequence.async.enqueue_and_log( function, message.str() );
        ret = ERROR;
      }

      // If this came in the asynchronous command port then write the ACK
      //
      if ( sock.isasync() ) {
        sock.Write( ret==ERROR ? Sequencer::ERR : Sequencer::ACK );
        ret = NOTHING;
      }

      if (ret != NOTHING) {
        if ( ! retstring.empty() ) retstring.append(" ");

        // If the retstring doesn't already have a DONE or ERROR in it,
        // then append that to the retstring.
        //
        if ( ret != HELP && ret != JSON &&
             ( retstring.find( "DONE" )  == std::string::npos ) &&
             ( retstring.find( "ERROR" ) == std::string::npos ) ) {
          retstring.append( ret == 0 ? "DONE" : "ERROR" );

          if ( ret == JSON ) {
            message.str(""); message << "command (" << seq.cmd_num << ") reply with JSON message";
            logwrite( function, message.str() );
          }
          else
          if ( ret != HELP && buf.find("help")==std::string::npos && buf.find("?")==std::string::npos ) {
            retstring.append( "\n" );
            message.str(""); message << "command (" << seq.cmd_num << ") reply: " << retstring;
            logwrite( function, message.str() );
          }
        }
        else retstring.append( "\n" );
        if ( sock.Write( retstring ) < 0 ) connection_open=false;
      }

      if (!sock.isblocking()) break;       // Non-blocking connection exits immediately.
                                           // Keep blocking connection open for interactive session.
    }

    connection_open = false;
    sock.Close();
    logwrite( function, "connection closed" );

    return;
  }
  /***** Server::doit *********************************************************/

}
