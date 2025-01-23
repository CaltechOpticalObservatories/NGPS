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

      // PUB_ENDPOINT -- my ZeroMQ socket endpoint for publishing
      //
      if ( config.param[entry] == "PUB_ENDPOINT" ) {
        this->sequence.publisher_address = config.arg[entry];
        this->sequence.publisher_topic = DAEMON_NAME;
        this->sequence.async.enqueue_and_log(function, "SEQUENCERD:config:"+config.param[entry]+"="+config.arg[entry]);
        applied++;
      }

      // SUB_ENDPOINT
      //
      if ( config.param[entry] == "SUB_ENDPOINT" ) {
        this->sequence.subscriber_address = config.arg[entry];
        this->sequence.async.enqueue_and_log(function, "SEQUENCERD:config:"+config.param[entry]+"="+config.arg[entry]);
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

      // TCS_WHICH -- which TCS to connect to, defults to real if not specified
      if ( config.param[entry] == "TCS_WHICH" ) {
        if ( config.arg[entry] != "sim" && config.arg[entry] != "real" ) {
          this->sequence.async.enqueue_and_log( function, "ERROR TCS_WHICH expected { sim real }" );
          return ERROR;
        }
        this->sequence.tcs_which = config.arg[entry];
        this->sequence.async.enqueue_and_log( function, "SEQUENCERD:config:"+config.param[entry]+"="+config.arg[entry] );
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

      // CAMERA_PROLOGUE
      if (config.param[entry].compare( 0, CAMERA_PROLOGUE.length(), CAMERA_PROLOGUE )==0) {
        this->sequence.camera_prologue.push_back( this->config.arg[entry] );
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

      // SLIT_DEFAULT
      if (config.param[entry]=="SLIT_DEFAULT") {
        this->sequence.slit_default = this->config.arg[entry];
        applied++;
        message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->sequence.async.enqueue_and_log( function, message.str() );
      }

      // ACAM_FILTER_DEFAULT
      if (config.param[entry]=="ACAM_FILTER_DEFAULT") {
        this->sequence.acam_filter_default = this->config.arg[entry];
        applied++;
        message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->sequence.async.enqueue_and_log( function, message.str() );
      }

      // ACAM_COVER_DEFAULT
      if (config.param[entry]=="ACAM_COVER_DEFAULT") {
        this->sequence.acam_cover_default = this->config.arg[entry];
        applied++;
        message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->sequence.async.enqueue_and_log( function, message.str() );
      }

      // CALIB_COVER_DEFAULT
      if (config.param[entry]=="CALIB_COVER_DEFAULT") {
        this->sequence.calib_cover_default = this->config.arg[entry];
        applied++;
        message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->sequence.async.enqueue_and_log( function, message.str() );
      }

      // CALIB_DOOR_DEFAULT
      if (config.param[entry]=="CALIB_DOOR_DEFAULT") {
        this->sequence.calib_door_default = this->config.arg[entry];
        applied++;
        message.str(""); message << "SEQUENCERD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->sequence.async.enqueue_and_log( function, message.str() );
      }

      // VIRTUAL_SLIT_ACQUIRE
      if ( config.param[entry] == "VIRTUAL_SLIT_ACQUIRE" ) {
        try { this->sequence.slitoffsetacquire = std::stof(this->config.arg[entry]); }
        catch ( const std::exception &e ) {
          message.str(""); message << "ERROR parsing VIRTUAL_SLIT_ACQUIRE from " << config.arg[entry] << ": " << e.what();
          logwrite( function, message.str() );
          return ERROR;
        }
      }

      // VIRTUAL_SLIT_EXPOSE
      if ( config.param[entry] == "VIRTUAL_SLIT_EXPOSE" ) {
        try { this->sequence.slitoffsetexpose = std::stof(this->config.arg[entry]); }
        catch ( const std::exception &e ) {
          message.str(""); message << "ERROR parsing VIRTUAL_SLIT_EXPOSE from " << config.arg[entry] << ": " << e.what();
          logwrite( function, message.str() );
          return ERROR;
        }
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
   * It sleeps until the next occurrence of 12:01:00, at which time it
   * closes the current log and initializes a new log file.
   *
   */
  void Server::new_log_day( std::string logpath ) {
    while (true) {
      // sleep until 12:01:00
      auto newlogtime = next_occurrence( 12, 01, 00 );
      std::this_thread::sleep_until( newlogtime );
      close_log();
      init_log( logpath, Sequencer::DAEMON_NAME );
      // ensure it doesn't immediately re-open
      std::this_thread::sleep_for( std::chrono::seconds(1) );
    }
  }
  /***** Sequencer::Server::new_log_day ***************************************/


  /***** Server::block_main ***************************************************/
  /**
   * @brief      main function for blocking connection thread
   * @param[in]  server  reference to Sequencer::Server object
   * @param[in]  sock    Network::TcpSocket socket object
   *
   * accepts a socket connection and processes the request by
   * calling function doit()
   *
   * This thread never terminates.
   *
   */
  void Server::block_main( Sequencer::Server &server, std::shared_ptr<Network::TcpSocket> sock ) {
    server.threads_active.fetch_add(1);  // atomically increment threads_busy counter
    sock->Write("CONNECTED\n");
    server.doit(*sock);
    sock->Close();
    server.threads_active.fetch_sub(1);  // atomically increment threads_busy counter
    server.id_pool.release_number( sock->id );
    return;
  }
  /***** Server::block_main ***************************************************/


  /**** Server::thread_main ***************************************************/
  /**
   * @brief      main function for all non-blocked threads
   * @param[in]  server  reference to Sequencer::Server object
   * @param[in]  sock    Network::TcpSocket socket object
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
  void Server::thread_main( Sequencer::Server &server, std::shared_ptr<Network::TcpSocket> sock ) {
    while (1) {
      {
      std::lock_guard<std::mutex> lock(server.conn_mutex);
      sock->Accept();
      }
      // Spawn a thread to handle this command in the background,
      // which is responsible for closing this socket.
      std::thread( &Sequencer::Server::doit, &server, std::ref(*sock) ).detach();
    }
    return;
  }
  /**** Server::thread_main ***************************************************/


  /**** Server::gui_main ******************************************************/
  /**
   * @brief      main function for gui thread
   * @param[in]  server  reference to Sequencer::Server object
   * @param[in]  sock    Network::TcpSocket socket object
   *
   * accepts a socket connection and processes the request by
   * calling function doit()
   *
   * There is one of these, and it never terminates.
   *
   * This function differs from block_main only in that doit is spawned in its own thread.
   *
   */
  void Server::gui_main( Sequencer::Server &server, std::shared_ptr<Network::TcpSocket> sock ) {
    while (1) {
      {
      std::lock_guard<std::mutex> lock(server.conn_mutex);
      sock->Accept();
      }
      sock->Write("CONNECTED\n");
      // spawn a thread to handle this command in the background
      std::thread( &Sequencer::Server::doit, &server, std::ref(*sock) ).detach();
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
  void Server::doit( Network::TcpSocket &sock ) {
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
          this->sequence.async.enqueue_and_log( function, message.str() );
        }
        if ( pollret <0 && errno ) {
          message.str(""); message << "ERROR: Poll error on fd " << sock.getfd() << " thread " << sock.id << ": " << strerror(errno);
          this->sequence.async.enqueue_and_log( function, message.str() );
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
          this->sequence.async.enqueue_and_log( function, message.str() );
        }
        if (ret==-2) {              // or a timeout
          message.str(""); message << "ERROR: timeout reading from fd " << sock.getfd();
          this->sequence.async.enqueue_and_log( function, message.str() );
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

        sock.id = ++this->cmd_num;
        if ( this->cmd_num == INT_MAX ) this->cmd_num = 0;

        message.str(""); message << "received command (" << sock.id << ") on fd " << sock.getfd() << ": " << cmd << " " << args;
        logwrite(function, message.str());
      }
      catch ( std::runtime_error &e ) {
        std::stringstream errstream; errstream << e.what();
        message.str(""); message << "ERROR: parsing arguments: " << errstream.str();
        this->sequence.async.enqueue_and_log( function, message.str() );
        ret = -1;
      }
      catch ( ... ) {
        message.str(""); message << "ERROR: unknown error parsing arguments: " << args;
        this->sequence.async.enqueue_and_log( function, message.str() );
        ret = -1;
      }

      // process commands here
      //
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
      if ( cmd == SEQUENCERD_EXIT ) {
                  this->exit_cleanly();                        // shutdown the sequencer
      }
      else

      // These commands go to acamd
      //
      if ( cmd == SEQUENCERD_ACAM ) {
                  if ( ( strncasecmp( args.c_str(), "help", 4 ) == 0 ) || ( args.find("?") != std::string::npos ) ) {
                    message.str(""); message << "ERROR command \"" << cmd << " " << args << "\" not allowed from sequencer";
                    logwrite( function, message.str() );
                    ret = ERROR;
                  }
                  else
                  if ( sock.isasync() ) {
                    std::thread( std::ref( Common::DaemonClient::dothread_command ), std::ref( this->sequence.acamd ), args ).detach();
                  }
                  else {
                    ret = this->sequence.acamd.command( args, retstring );
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
      if ( cmd == SEQUENCERD_CALIB ) {
                  if ( ( strncasecmp( args.c_str(), "help", 4 ) == 0 ) || ( args.find("?") != std::string::npos ) ) {
                    message.str(""); message << "ERROR command \"" << cmd << " " << args << "\" not allowed from sequencer";
                    logwrite( function, message.str() );
                    ret = ERROR;
                  }
                  else
                  if ( sock.isasync() ) {
                    std::thread( std::ref( Common::DaemonClient::dothread_command ), std::ref( this->sequence.calibd ), args ).detach();
                  }
                  else {
                    ret = this->sequence.calibd.command( args, retstring );
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
      if ( cmd == SEQUENCERD_CAMERA ) {
                  if ( ( strncasecmp( args.c_str(), "help", 4 ) == 0 ) || ( args.find("?") != std::string::npos ) ) {
                    message.str(""); message << "ERROR command \"" << cmd << " " << args << "\" not allowed from sequencer";
                    logwrite( function, message.str() );
                    ret = ERROR;
                  }
                  else
                  if ( sock.isasync() ) {
                    std::thread( std::ref( Common::DaemonClient::dothread_command ), std::ref( this->sequence.camerad ), args ).detach();
                  }
                  else {
                    ret = this->sequence.camerad.command( args, retstring );
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
      if ( cmd == SEQUENCERD_FILTER ) {
                  if ( ( strncasecmp( args.c_str(), "help", 4 ) == 0 ) || ( args.find("?") != std::string::npos ) ) {
                    message.str(""); message << "ERROR command \"" << cmd << " " << args << "\" not allowed from sequencer";
                    logwrite( function, message.str() );
                    ret = ERROR;
                  }
                  else
                  if ( sock.isasync() ) {
                    std::thread( std::ref( Common::DaemonClient::dothread_command ), std::ref( this->sequence.filterd ), args ).detach();
                  }
                  else {
                    ret = this->sequence.filterd.command( args, retstring );
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
      if ( cmd == SEQUENCERD_POWER ) {
                  if ( ( strncasecmp( args.c_str(), POWERD_LIST.c_str(), POWERD_LIST.size() ) == 0 ) ||
                       ( strncasecmp( args.c_str(), "help", 4 ) == 0 ) || ( args.find("?") != std::string::npos ) ) {
                    message.str(""); message << "ERROR command \"" << cmd << " " << args << "\" not allowed from sequencer";
                    logwrite( function, message.str() );
                    ret = ERROR;
                  }
                  else
                  if ( sock.isasync() ) {
                    std::thread( std::ref( Common::DaemonClient::dothread_command ),
                                 std::ref( this->sequence.powerd ), args ).detach();
                  }
                  else {
                    ret = this->sequence.powerd.command( args, retstring );
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
      if ( cmd == SEQUENCERD_SLIT ) {
                  if ( ( strncasecmp( args.c_str(), "help", 4 ) == 0 ) || ( args.find("?") != std::string::npos ) ) {
                    message.str(""); message << "ERROR command \"" << cmd << " " << args << "\" not allowed from sequencer";
                    logwrite( function, message.str() );
                    ret = ERROR;
                  }
                  else
                  if ( sock.isasync() ) {
                    std::thread( std::ref( Common::DaemonClient::dothread_command ), std::ref( this->sequence.slitd ), args ).detach();
                  }
                  else {
                    ret = this->sequence.slitd.command( args, retstring );
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
      if ( cmd == SEQUENCERD_TCS ) {
                  if ( ( strncasecmp( args.c_str(), "help", 4 ) == 0 ) || ( args.find("?") != std::string::npos ) ) {
                    message.str(""); message << "ERROR command \"" << cmd << " " << args << "\" not allowed from sequencer";
                    logwrite( function, message.str() );
                    ret = ERROR;
                  }
                  else
                  if ( sock.isasync() ) {
                    std::thread( std::ref( Common::DaemonClient::dothread_command ), std::ref( this->sequence.tcsd ), args ).detach();
                  }
                  else {
                    ret = this->sequence.tcsd.command( args, retstring );
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
      if ( cmd == SEQUENCERD_STARTUP ) {
                  if ( !sock.isblocking() ) {
                    std::thread( &Sequencer::Sequence::startup, std::ref(this->sequence) ).detach();
                  }
                  else {
                    ret = this->sequence.startup();
                  }
      }
      else

      // system shutdown
      // This shuts down the instrument.
      //
      if ( cmd == SEQUENCERD_SHUTDOWN ) {
                  if ( sock.isasync() ) {
                    std::thread( &Sequencer::Sequence::shutdown, std::ref( this->sequence ) ).detach();
                  }
                  else {
                    ret = this->sequence.shutdown();
                  }
      }
      else

      // Sequence "start"
      //
      if ( cmd == SEQUENCERD_START ) {
                  std::thread( &Sequencer::Sequence::sequence_start, std::ref(this->sequence), args ).detach();
                  ret = NO_ERROR;
      }
      else

      // start a single target
      // args is expected to contain an OBSID to get from the database
      //
      if ( cmd == SEQUENCERD_STARTONE ) {
                  std::thread( &Sequencer::Sequence::sequence_start, std::ref(this->sequence), args ).detach();
                  ret = NO_ERROR;
      }
      else

      // repeat the last exposure
      //
      if ( cmd == SEQUENCERD_REPEAT ) {
                  if ( !this->sequence.seq_state_manager.is_set( Sequencer::SEQ_READY ) ) {
                    this->sequence.async.enqueue_and_log( function, "ERROR cannot start exposure: not ready" );
                    ret = ERROR;
                  }
                  else {
                    std::thread( &Sequencer::Sequence::repeat_exposure, std::ref(this->sequence) ).detach();
                    ret = NO_ERROR;
                  }
      }
      else

      // Stop an Exposure
      //
      if ( cmd == SEQUENCERD_STOP ) {
                  logwrite( function, "requesting stop" );
                  std::thread( &Sequencer::Sequence::stop_exposure, std::ref(this->sequence) ).detach();
                  ret=NO_ERROR;
      }
      else

      // Abort everything possible
      //
      if ( cmd == SEQUENCERD_ABORT ) {
                  logwrite( function, "requesting abort" );
                  std::thread( &Sequencer::Sequence::abort_process, std::ref(this->sequence) ).detach();
                  ret=NO_ERROR;
      }
      else

      // The TCS operator tells the Sequencer when the target is ready to observe
      // because there is currently no remote command that provides this information.
      // Clearing the SEQ_WAIT_TCSOP state means no longer waiting for the TCS operator.
      // This will notify the waiting thread which will proceed with the observation.
      //
      if ( cmd == SEQUENCERD_ONTARGET ) {
                  this->sequence.ontarget();
                  std::thread( &Sequencer::Sequence::reset_ontarget, std::ref(this->sequence) ).detach();
                  ret = NO_ERROR;
      }
      else
      if ( cmd == SEQUENCERD_USERCONTINUE ) {
                  this->sequence.usercontinue();
                  std::thread( &Sequencer::Sequence::reset_usercontinue, std::ref(this->sequence) ).detach();
                  ret = NO_ERROR;
      }
      else

      // 
      //
      if ( cmd == SEQUENCERD_GETONETARGET ) {
                  this->sequence.target.get_specified_target( args, retstring );
                  ret = NO_ERROR;
      }
      else

      // apply target offsets from database
      //
      if ( cmd == SEQUENCERD_TARGETOFFSET ) {
                  this->sequence.target_offset();
                  ret = NO_ERROR;
      }
      else

      // Set the "Do-Type"
      // which can be single-step (do one) or continuous (do all).
      //
      if ( cmd == SEQUENCERD_DOTYPE ) {
                  ret = this->sequence.dotype( args, retstring );
                  retstring.append( " " );
      }
      else

      // Report the Sequencer State,
      // which will be returned, logged, and written to the async message port.
      //
      if ( cmd == SEQUENCERD_STATE ) {
                  this->sequence.broadcast_seqstate();
                  retstring = this->sequence.seq_state_manager.get_set_states();
                  ret = NO_ERROR;
      }
      else

      // Report the Wait State(s),
      // which will be returned, logged, and written to the async message port.
      //
      if ( cmd == SEQUENCERD_WSTATE ) {
                  this->sequence.broadcast_waitstate();
                  retstring = this->sequence.seq_state_manager.get_set_states();
                  ret = NO_ERROR;
      }
      else

      // Set/Get the Target Set ID
      //
      if ( cmd == SEQUENCERD_TARGETSET ) {
                  ret= this->sequence.target.targetset( args, retstring );
                  message.str(""); message << "TARGETSET: " << retstring;
                  this->sequence.async.enqueue( message.str() );
                  retstring.append( " " );
      }
      else

      // Pause an Exposure
      //
      if ( cmd == SEQUENCERD_PAUSE ) {
                  // Can only pause during an exposure
                  //
                  if ( ! this->sequence.seq_state_manager.is_set( Sequencer::SEQ_WAIT_EXPOSE ) ) {
                    this->sequence.async.enqueue_and_log( function, "ERROR: can only pause during an active exposure" );
                    ret = ERROR;
                  }
                  else
                  // Can't already be paused
                  //
                  if ( this->sequence.seq_state_manager.is_set( Sequencer::SEQ_PAUSED ) ) {
                    this->sequence.async.enqueue_and_log( function, "ERROR: already paused" );
                    ret = ERROR;
                  }
                  else {
                    this->sequence.camerad.async( CAMERAD_PAUSE );
//                  probably set these states automatically:
//                  this->sequence.seq_state.set_and_clear( Sequencer::SEQ_PAUSED, Sequencer::SEQ_RUNNING );  // set pause, clear running
                    this->sequence.broadcast_seqstate();
                    ret = NO_ERROR;
                  }
      }
      else

      // Resume a Paused exposure
      //
      if ( cmd.compare( SEQUENCERD_RESUME ) == 0) {
                      // Can only resume when paused
                      //
                      if ( ! this->sequence.seq_state_manager.is_set( Sequencer::SEQ_PAUSED ) ) {
                        this->sequence.async.enqueue_and_log( function, "ERROR: can only resume when paused" );
                        ret = ERROR;
                      }
                      else {
                        this->sequence.camerad.async( CAMERAD_RESUME );
//                      probably set these states automatically:
//                      this->sequence.seq_state.set_and_clear( Sequencer::SEQ_RUNNING, Sequencer::SEQ_PAUSED );  // set running, clear pause
                        this->sequence.broadcast_seqstate();
                        ret = NO_ERROR;
                      }
      }
      else

      // Call Test Routines,
      // routine specified by args.
      //
      if ( cmd.compare( SEQUENCERD_TEST ) == 0 ) {
                      ret = this->sequence.test( args, retstring );
                      if ( ! retstring.empty() ) retstring.append( " " );
      }
      else

      // Modify Exptime needs a single argument, the new exptime
      //
      if ( cmd.compare( SEQUENCERD_MODEXPTIME ) == 0 ) {
                      Tokenize( args, tokens, " " );
                      if ( tokens.size() != 1 ) {
                        this->sequence.async.enqueue_and_log( function, "ERROR: expected MODEXPTIME <exptime>" );
                        ret = ERROR;
                      }
                      else {
                        // convert the single argument from string to double
                        //
                        double exptime_req=0;
                        try { exptime_req = std::stod( tokens.at(0) ); }
                        catch( std::out_of_range &e ) {
                          message.str(""); message << "ERROR: out of range parsing args " << args << ": " << e.what();
                          this->sequence.async.enqueue_and_log( function, message.str() );
                          ret = ERROR;
                        }
                        catch( std::invalid_argument &e ) {
                          message.str(""); message << "ERROR: invalid argument parsing args " << args << ": " << e.what();
                          this->sequence.async.enqueue_and_log( function, message.str() );
                          ret = ERROR;
                        }

                        // spawn a thread to modify the exposure time
                        //
                        std::thread( &Sequencer::Sequence::modify_exptime,
                                     std::ref( this->sequence), exptime_req ).detach();
                        ret = NO_ERROR;
                      }
      }
      else

      // Configure sequencer (read config file and apply)
      //
      if ( cmd.compare( SEQUENCERD_CONFIG ) == 0 ) {
                      ret = this->config.read_config(this->config);  // read configuration file specified on command line
                      if ( ret != NO_ERROR ) logwrite(function, "ERROR: unable to load config file");
                      else ret = this->configure_sequencer();
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
                        this->sequence.make_telemetry_message( retstring );
                        ret = JSON;
                      }
      }

      // Unknown commands generate an error
      //
      else {
        message.str(""); message << "ERROR: unknown command: " << cmd;
        this->sequence.async.enqueue_and_log( function, message.str() );
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
            message.str(""); message << "command (" << this->cmd_num << ") reply with JSON message";
            logwrite( function, message.str() );
          }
          else
          if ( ret != HELP && buf.find("help")==std::string::npos && buf.find("?")==std::string::npos ) {
            retstring.append( "\n" );
            message.str(""); message << "command (" << this->cmd_num << ") reply: " << retstring;
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
