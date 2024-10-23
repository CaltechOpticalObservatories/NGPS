/**
 * @file    thermald.cpp
 * @brief   this is the main thermal daemon
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "thermal_server.h"

namespace Thermal {


  /***** Thermal::Server::exit_cleanly ****************************************/
  /**
   * @brief      shutdown nicely
   *
   */
  void Server::exit_cleanly(void) {
    std::string function = "Thermal::Server::exit_cleanly";

    logwrite( function, "closing Lakeshores" );
    this->interface.close_lakeshores();
    this->interface.close_campbell();

    logwrite( function, "exiting" );

    exit(EXIT_SUCCESS);
  }
  /***** Thermal::Server::exit_cleanly ****************************************/


  /***** Thermal::Server::configure_thermald **********************************/
  /**
   * @brief      read and apply the configuration file for the thermal daemon
   * @return     ERROR or NO_ERROR
   *
   */
  long Server::configure_thermald() {
    std::string function = "Thermal::Server::configure_thermald";
    std::stringstream message;
    int applied=0;
    long error;

    // loop through the entries in the configuration file, stored in config class
    //
    for (int entry=0; entry < this->config.n_entries; entry++) {

      // NBPORT -- nonblocking listening port for the thermal daemon
      //
      if (config.param[entry].compare(0, 6, "NBPORT")==0) {
        int port;
        try {
          port = std::stoi( config.arg[entry] );
        }
        catch (const std::exception &e) {
          message.str(""); message << "ERROR parsing NBPORT: " << e.what();
          logwrite(function, message.str());
          return ERROR;
        }
        this->nbport = port;
        message.str(""); message << DAEMON_NAME << ":config:" << config.param[entry] << "=" << config.arg[entry];
        this->interface.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // BLKPORT -- blocking listening port for the thermal daemon
      //
      if (config.param[entry].compare(0, 7, "BLKPORT")==0) {
        int port;
        try {
          port = std::stoi( config.arg[entry] );
        }
        catch (const std::exception &e) {
          message.str(""); message << "ERROR parsing BLKPORT: " << e.what();
          logwrite(function, message.str());
          return(ERROR);
        }
        this->blkport = port;
        message.str(""); message << DAEMON_NAME << ":config:" << config.param[entry] << "=" << config.arg[entry];
        this->interface.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // ASYNCPORT -- asynchronous broadcast message port for the thermal daemon
      //
      if (config.param[entry].compare(0, 9, "ASYNCPORT")==0) {
        int port;
        try {
          port = std::stoi( config.arg[entry] );
        }
        catch (const std::exception &e) {
          message.str(""); message << "ERROR parsing ASYNCPORT: " << e.what();
          logwrite(function, message.str());
          return ERROR;
        }
        this->asyncport = port;
        message.str(""); message << DAEMON_NAME << ":config:" << config.param[entry] << "=" << config.arg[entry];
        this->interface.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // ASYNCGROUP -- asynchronous broadcast group for the thermal daemon
      //
      if (config.param[entry].compare(0, 10, "ASYNCGROUP")==0) {
        this->asyncgroup = config.arg[entry];
        message.str(""); message << DAEMON_NAME << ":config:" << config.param[entry] << "=" << config.arg[entry];
        this->interface.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // TELEM_PROVIDER : contains daemon name and port to contact for header telemetry info
      //
      if ( config.param[entry] == "TELEM_PROVIDER" ) {
        std::vector<std::string> tokens;
        Tokenize( config.arg[entry], tokens, " " );
        try {
          if ( tokens.size() == 2 ) {
            this->interface.telemetry_providers[tokens.at(0)] = std::stod(tokens.at(1));
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
        this->interface.async.enqueue_and_log( "THERMALD", function, message.str() );
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
    message << "applied " << applied << " configuration lines to thermald";
    logwrite(function, message.str());

    return error;
  }
  /***** Thermal::Server::configure_thermald **********************************/


  /***** Thermal::Server::parse_lks_unit **************************************/
  /**
   * @brief      parse a line of the config file for key LKS_UNIT
   * @param[in]  input   line from config file
   * @param[out] lksnum  LKS# which cross-references unit with chan
   * @param[out] name    Lakeshore name, helpful identifier in logging and UI
   * @param[out] host    hostname for connecting to this Lakeshore
   * @param[out] port    port number on host for connecting to this Lakeshore
   * @return     ERROR | NO_ERROR
   *
   */
  long Server::parse_lks_unit( std::string &input, 
                               int &lksnum, std::string &name, std::string &host, int &port ) {
    std::string function = "Thermal::Server::parse_lks_unit";
    std::stringstream message;
    std::vector<std::string> tokens;

    Tokenize( input, tokens, " \"" );

    if ( tokens.size() != 4 ) {
      message.str(""); message << "ERROR bad number of tokens: " << tokens.size() << ". expected 4";
      logwrite( function, message.str() );
      return( ERROR );
    }

    try {
      lksnum = std::stoi( tokens.at(0) );
      name   = tokens.at(1);
      host   = tokens.at(2);
      port   = std::stoi( tokens.at(3) );
    }
    catch ( const std::exception &e ) {
      message.str(""); message << "ERROR parsing tokens from input: " << input << ": " << e.what();
      logwrite( function, message.str() );
      return ERROR;
    }

    if ( lksnum < 0 ) {
      message.str(""); message << "ERROR bad LKS unit number " << lksnum << ": must be >= 0";
      logwrite( function, message.str() );
      return( ERROR );
    }

    return( NO_ERROR );

  }
  /***** Thermal::Server::parse_lks_unit **************************************/


  /***** Thermal::Server::parse_lks_chan **************************************/
  /**
   * @brief      parse a line of the config file for LKS_CHAN
   * @param[in]  input   line from config file
   * @param[out] lksnum  LKS# which cross-references unit with chan
   * @param[out] chan    standard Lakeshore channel name
   * @param[out] heater  Lakeshore heater name
   * @param[out] label   label for the channel must correspond to MySQL column
   * @return     ERROR | NO_ERROR
   *
   */
  long Server::parse_lks_chan( std::string &input, 
                               int &lksnum, std::string &chan, bool &heater, std::string &label ) {
    std::string function = "Thermal::Server::parse_lks_chan";
    std::stringstream message;
    std::vector<std::string> tokens;

    Tokenize( input, tokens, " \"" );

    if ( tokens.size() != 3 ) {
      message.str(""); message << "ERROR bad number of tokens: " << tokens.size() << ". expected 3";
      logwrite( function, message.str() );
      return( ERROR );
    }

    try {
      lksnum = std::stoi( tokens.at(0) );
      chan   = tokens.at(1);
      label  = tokens.at(2);
    }
    catch ( const std::exception &e ) {
      message.str(""); message << "ERROR parsing tokens from input: " << input << ": " << e.what();
      logwrite( function, message.str() );
      return ERROR;
    }

    if ( lksnum < 0 ) {
      message.str(""); message << "ERROR bad LKS unit number " << lksnum << ": must be >= 0";
      logwrite( function, message.str() );
      return( ERROR );
    }

    // Heater chans are preceeded by an "H", e.g. "H1"
    // Remove that "H" and set the chan to just the number, e.g. "1"
    //
    std::size_t heaterchar;
    if ( ( heaterchar = chan.find("H") ) != std::string::npos ) {  // Is this a heater?
      chan = chan.substr( heaterchar+1 );                          // Get just the number
      heater = true;                                               // flag as a heater so caller knows how to use it
    }
    else {
      heater = false;                                              // otherwise not a heater
    }

    return( NO_ERROR );
  }
  /***** Thermal::Server::parse_lks_chan **************************************/


  /***** Thermal::Server::parse_camp_chan *************************************/
  /**
   * @brief      parse a line of the config file for LKS_CHAN and add to channel_names map
   * @param[in]  input  line from config file
   * @return     ERROR | NO_ERROR
   *
   */
  long Server::parse_camp_chan( std::string &input ) {
    std::string function = "Thermal::Server::parse_camp_chan";
    std::stringstream message;
    std::vector<std::string> tokens;
    int chan=-1;
    std::string label="undef";

    Tokenize( input, tokens, " \"" );

    if ( tokens.size() < 2 ) {
      message.str(""); message << "ERROR bad number of tokens: " << tokens.size() << ". expected 2";
      logwrite( function, message.str() );
      return ERROR;
    }

    try {
      chan  = std::stoi( tokens.at(0) );
      label = tokens.at(1);
    }
    catch ( const std::exception &e ) {
      message.str(""); message << "ERROR parsing tokens from input: " << input << ": " << e.what();
      logwrite( function, message.str() );
      return ERROR;
    }

    if ( chan < 1 ) {
      message.str(""); message << "ERROR bad Campbell channel number " << chan << ": must be >= 1";
      logwrite( function, message.str() );
      return ERROR;
    }

    // before adding this channel number to the map, make sure it doesn't already exist
    //
    if ( this->interface.campbell.sensor_names.find(chan) != this->interface.campbell.sensor_names.end() ) {
      message.str(""); message << "ERROR duplicate channel \"" << chan << "\" already defined";
      logwrite( function, message.str() );
      return ERROR;
    }

    this->interface.campbell.sensor_names[chan] = label;

    return NO_ERROR;
  }
  /***** Thermal::Server::parse_camp_chan *************************************/


  /***** Thermal::Server::configure_telemetry *********************************/
  /**
   * @brief      read and apply the configuration file for the telemetry
   * @return     ERROR or NO_ERROR
   *
   */
  long Server::configure_telemetry() {
    std::string function = "Thermal::Server::configure_telemetry";
    std::stringstream message;
    int applied=0;
    long error;

    std::string db_host;
    std::string db_port;
    std::string db_user;
    std::string db_pass;
    std::string db_schema;
    std::string db_table;

    this->db_info.clear();

    // loop through the entries in the configuration file, stored in config class
    //
    for (int entry=0; entry < this->config.n_entries; entry++) {

      // DB_HOST
      if (config.param[entry].compare( 0, 7, "DB_HOST" )==0) {
        db_host = config.arg[entry];
        message.str(""); message << DAEMON_NAME << ":config:" << config.param[entry] << "=" << config.arg[entry];
        this->interface.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // DB_PORT
      if (config.param[entry].compare( 0, 7, "DB_PORT" )==0) {
        try { std::stoi( config.arg[entry] ); }
        catch ( std::invalid_argument & ) { logwrite(function, "ERROR: bad DB_PORT: unable to convert to integer"); return(ERROR); }
        catch ( std::out_of_range & ) { logwrite(function, "ERROR: DB_PORT number out of integer range"); return(ERROR); }
        db_port = config.arg[entry];
        message.str(""); message << DAEMON_NAME << ":config:" << config.param[entry] << "=" << config.arg[entry];
        this->interface.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // DB_USER
      if (config.param[entry].compare( 0, 7, "DB_USER" )==0) {
        db_user = config.arg[entry];
        message.str(""); message << DAEMON_NAME << ":config:" << config.param[entry] << "=" << config.arg[entry];
        this->interface.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // DB_PASS
      if (config.param[entry].compare( 0, 7, "DB_PASS" )==0) {
        db_pass = config.arg[entry];
        message.str(""); message << DAEMON_NAME << ":config:" << config.param[entry] << "=" << config.arg[entry];
        this->interface.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // DB_SCHEMA
      if (config.param[entry].compare( 0, 9, "DB_SCHEMA" )==0) {
        db_schema = config.arg[entry];
        message.str(""); message << DAEMON_NAME << ":config:" << config.param[entry] << "=" << config.arg[entry];
        this->interface.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // DB_TABLE
      if (config.param[entry].compare( 0, 8, "DB_TABLE" )==0) {
        db_table = config.arg[entry];
        message.str(""); message << DAEMON_NAME << ":config:" << config.param[entry] << "=" << config.arg[entry];
        this->interface.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // TELEM_PERIOD
      if (config.param[entry].compare( 0, 12, "TELEM_PERIOD" )==0) {
        try { this->telem_period = std::stoi( config.arg[entry] ); }
        catch ( std::invalid_argument & ) { logwrite(function, "ERROR: bad DB_PORT: unable to convert to integer"); return(ERROR); }
        catch ( std::out_of_range & ) { logwrite(function, "ERROR: DB_PORT number out of integer range"); return(ERROR); }
        message.str(""); message << DAEMON_NAME << ":config:" << config.param[entry] << "=" << config.arg[entry];
        this->interface.async.enqueue_and_log( function, message.str() );
        applied++;
      }

    } // end loop through the entries in the configuration file

    this->db_info = { db_host, db_port, db_user, db_pass, db_schema, db_table };

    message.str("");
    if (applied!=7) {
      message << "ERROR: expected 7 but ";
      error = ERROR;
    }
    else {
      error = NO_ERROR;
    }
    message << "applied " << applied << " configuration lines to thermald";
    logwrite(function, message.str());

    return error;

  }
  /***** Thermal::Server::configure_telemetry *********************************/


  /***** Thermal::Server::configure_devices ***********************************/
  /**
   * @brief      read and apply the configuration file for the hardware
   * @details    This function reads and parses keys for the Lakeshore(s) and
   *             the Campbell(s).
   * @return     ERROR or NO_ERROR
   *
   */
  long Server::configure_devices() {
    std::string function = "Thermal::Server::configure_devices";
    std::stringstream message;
    int applied=0;
    long error;

    // initialize all maps so they are built fresh with each read of the config file
    //
    this->interface.campbell.sensor_names.clear();
    this->interface.lakeshore.clear();
    this->interface.thermal_info.clear();

    // loop through the entries in the configuration file, stored in config class
    //
    for (int entry=0; entry < this->config.n_entries; entry++) {

      // LKS_UNIT: Lakeshore units
      if (config.param[entry]=="LKS_UNIT") {
        std::string name, host;
        int lksnum, port;

        // parse the LKS_UNIT configuration line here
        //
        error = this->parse_lks_unit( config.arg[entry], lksnum, name, host, port );

        if ( error == NO_ERROR ) {
          Thermal::Lakeshore Lakeshore;                   // temporary Thermal::Lakeshore interface object
          Lakeshore.lks = new Network::Interface( name, host, port );     // assign lks object to constructed LKS communication object pointer

          this->interface.lakeshore.insert( { lksnum, Lakeshore } );      // insert Lakeshore object into the map

          message.str(""); message << "THERMALD:config:" << config.param[entry] << "=" << config.arg[entry];
          logwrite( function, message.str() );
          this->interface.async.enqueue( message.str() );
          applied++;
        }
      }

      // LKS_CHAN: Lakeshore channels for each unit
      if (config.param[entry]=="LKS_CHAN") {
        int lksnum;
        std::string label, chan;
        bool heater;
        if ( this->parse_lks_chan( config.arg[entry], lksnum, chan, heater, label ) == NO_ERROR ) {

          // First, this lksnum must have a matching lksnum in the lakeshore map
          //
          auto lksnum_found = this->interface.lakeshore.find( lksnum );
          if ( lksnum_found != this->interface.lakeshore.end() ) {  // found lksnum in lakeshore map
            if ( heater ) {
              this->interface.lakeshore[ lksnum ].heat_info[ chan ] = label;
            }
            else {
              this->interface.lakeshore[ lksnum ].temp_info[ chan ] = label;
            }
          }
          else {                                           // lksnum not found in lakeshore map
            message.str(""); message << "ERROR channel " << chan << "\"" << label << "\""
                                     << " has no matching LKS_UNIT number " << lksnum;
            this->interface.async.enqueue_and_log( function, message.str() );
            error = ERROR;
            continue;
          }

          // Second, insert this info into another STL map for indexing by label.
          // The label must be unique in order for this to be effective.
          //
          auto name = this->interface.lakeshore.at( lksnum ).lks->get_name();
          auto label_found = this->interface.thermal_info.find( label );
          if ( label_found != this->interface.thermal_info.end() ) {  // found label in info map
            message.str(""); message << "ERROR: LKS unit " << lksnum << " duplicate label \"" << label
                                     << "\" already exists for unit " << this->interface.thermal_info[label].unit;
            this->interface.async.enqueue_and_log( function, message.str() );
            error = ERROR;
            continue;
          }
          else {
            this->interface.thermal_info[ label ] = { "LKS", lksnum, name, chan, label };
          }
        }
        message.str(""); message << "THERMALD:config:" << config.param[entry] << "=" << config.arg[entry];
        logwrite( function, message.str() );
        this->interface.async.enqueue( message.str() );
        applied++;
      }

      // CAMP_UNIT: Campbell unit
      if (config.param[entry]=="CAMP_UNIT") {
        this->interface.campbell.set_device( config.arg[entry] );
        message.str(""); message << "THERMALD:config:" << config.param[entry] << "=" << config.arg[entry];
        logwrite( function, message.str() );
        this->interface.async.enqueue( message.str() );
        applied++;
      }

      // CAMP_CHAN: Campbell channels
      if (config.param[entry]=="CAMP_CHAN") {
        if ( this->parse_camp_chan( config.arg[entry] ) == NO_ERROR ) {
          message.str(""); message << "THERMALD:config:" << config.param[entry] << "=" << config.arg[entry];
          logwrite( function, message.str() );
          this->interface.async.enqueue( message.str() );
          applied++;
        }
      }

    }  // end for entry=0; entry < n_entries

    return( NO_ERROR );
  }
  /***** Thermal::Server::configure_devices ***********************************/


  /***** Thermal::Server::telemetry_watchdog *********************************/
  /**
   * @brief      watchdog runs at 1Hz to keep the telemetry thread running
   * @details    this thread must be spawned by main() and never exit
   * @param[in]  thermal  reference to Thermal::Server
   *
   */
  void Server::telemetry_watchdog( Thermal::Server &server ) {
    std::string function = "Thermal::Server::telemetry_watchdog";
    std::stringstream message;

    logwrite( function, "telemetry watchdog active" );

    while ( true ) {

      if ( server.telem_sleeptimer.running() && !server.telem_running ) {
        logwrite( function, "starting telemetry thread" );
        std::thread( std::ref(server.dothread_telemetry), std::ref(server) ).detach();
      }

      std::this_thread::sleep_for ( std::chrono::seconds( 1 ) );
    }
    return;
  }
  /***** Thermal::Server::telemetry_watchdog *********************************/


  /***** Thermal::Server::dothread_telemetry *********************************/
  /**
   * @brief      This is the main telemetry thread.
   * @details    Runs until stopped by command or exception.
   * @param[in]  thermal  reference to Thermal::Server
   *
   */
  void Server::dothread_telemetry( Thermal::Server &server ) {
    std::string function = "Thermal::Server::dothread_telemetry";
    std::stringstream message;

    logwrite( function, "telemetry thread running" );

    server.telem_running = true;

    // The telemetry utility function can clear telem_running to
    // cause this thread to exit, but normally it will run forever.
    //
    while ( server.telem_running ) {

      logwrite( function, "NOTICE:thermald telemetry has started" );

      try {
        // Creating a Database object here connects to the database
        // specified by the db_info vector. When this object goes
        // out of scope it is destructed and the connection
        // automatically closes.
        //
        Database::Database database( server.db_info );

        int duration=server.telem_period;

        if ( duration >= 60 ) {            // For 1 minute or more,
          timeout( 0, "min" );             // wait until the next integral minute.
        }

        if ( duration >= 1 ) duration--;   // allows timeout(0,sec) to end on next integral second

        // Stopping the SleepTimer will break this loop, close
        // the database, and block until it has been restarted.
        //
        while ( server.telem_sleeptimer.running() ) {
          // Gather the data, each source stores in its own map
          //
          server.interface.get_external_telemetry();  // collect telemetry from other daemons
          server.interface.lakeshore_readall();       // read all Lakeshores
          server.interface.campbell.read_data();      // read Campbell CR1000

          // erase the telemdata map,
          // timestamp it now, then merge each source into that
          //
          server.interface.telemdata.clear();
          server.interface.telemdata["datetime"] = get_datetime();
          server.interface.telemdata.merge( server.interface.externaldata );
          server.interface.telemdata.merge( server.interface.campbell.datamap );
          server.interface.telemdata.merge( server.interface.lakeshoredata );

          // write the telemdata map to the database
          //
          database.write( server.interface.telemdata );

          server.telem_sleeptimer.sleepFor( std::chrono::seconds( duration ) );
          timeout( 0, "sec" );
        }

        // Database is destructed when it leaves scope, and the destructor
        // will close it, but this is tidy. close can throw an exception.
        //
        database.close();
        logwrite( function, "telemetry database closed" );
      }
      catch ( const mysqlx::Error &err ) {
        message.str(""); message << "ERROR: " << err;
        logwrite( function, message.str() );
        break;
      }
      catch ( std::exception &e ) {
        message.str(""); message << "ERROR: " << e.what();
        logwrite( function, message.str() );
        break;
      }
      catch ( ... ) {
        logwrite( function, "ERROR: unknown exception." );
        break;
      }

      logwrite( function, "NOTICE:thermald telemetry has stopped" );
    }

    server.telem_running = false;
    logwrite( function, "NOTICE:thermald telemetry thread terminated" );
    return;
  }
  /***** Thermal::Server::dothread_telemetry *********************************/


  /***** Thermal::Server::telemetry ******************************************/
  /**
   * @brief      telemetry utility to start, stop or check status
   * @param[in]  args       can be { start, stop, status, ? }
   * @param[out] retstring  returns status string
   * @return     ERROR or NO_ERROR
   *
   */
  long Server::telemetry( std::string args, std::string &retstring ) {
    std::string function = "Thermal::Server::telemetry";
    std::stringstream message;

    // "?" or no arg displays usage and possible inputs, then return
    //
    if ( args == "?" ) {
      retstring = THERMALD_TELEMETRY;
      retstring.append( " start | stop | status\n" );
      retstring.append( "  Starts, stops or returns the status of the thermald telemetry.\n" );
      retstring.append( "  Telemetry starts automatically and should always be running,\n" );
      retstring.append( "  so this command is for test/engineering purposes only.\n" );
      retstring.append( "\n" );
      retstring.append( "  Reply will be \"running\", \"not running\", or \"starting\",\n" );
      retstring.append( "  where starting means the watchdog will start the telemetry.\n" );
      return( NO_ERROR );
    }

    if ( args == "start" ) {
      // Start the SleepTimer, rely on the watchdog to now spawn
      // the telemetry thread, which will set telem_running.
      //
      this->telem_sleeptimer.start();
    }
    else
    if ( args == "stop" ) {
      // Stop the SleepTimer and clear telem_running, which
      // causes the telemetry thread to exit.
      //
      this->telem_sleeptimer.stop();
      this->telem_running = false;
    }
    else
    if ( args == "status" || args.empty() ) {  // don't need to do anything, status is implied
    }
    else {
      message.str(""); message << "ERROR: unrecognized \"" << args << "\". expected { start, stop, status }";
      logwrite( function, message.str() );
      retstring = "bad_arg";
      return( ERROR );
    }

    if ( this->telem_sleeptimer.running() && this->telem_running ) retstring  = "running";
    else
    if ( this->telem_sleeptimer.running() && !this->telem_running ) retstring = "starting";
    else
    if ( !this->telem_sleeptimer.running() ) retstring = "not running";
    else retstring = "unknown";

    return( NO_ERROR );
  }
  /***** Thermal::Server::telemetry ******************************************/


  /***** Server::new_log_day **************************************************/
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
      init_log( logpath, Thermal::DAEMON_NAME );
    }
  }
  /***** Server::new_log_day **************************************************/


  /***** Server::block_main ***************************************************/
  /**
   * @brief      main function for blocking connection thread
   * @param[in]  thermal  reference to Thermal::Server object
   * @param[in]  sock     Network::TcpSocket socket object
   *
   * This function is run in a separate thread spawned by main() and
   * accepts a socket connection and processes the request by
   * calling function Thermal::Server::doit()
   *
   * This thread never terminates.
   *
   */
  void Server::block_main( Thermal::Server &thermal, Network::TcpSocket sock ) {
    while(1) {
      sock.Accept();
      thermal.doit(sock);           // call function to do the work
      sock.Close();
    }
    return;
  }
  /***** Server::block_main ***************************************************/


  /***** Server::thread_main **************************************************/
  /**
   * @brief      main function for all non-blocked threads
   * @param[in]  thermal  reference to Thermal::Server object
   * @param[in]  sock     Network::TcpSocket socket object
   *
   * This function is run in a separate thread spawned by main() and
   * accepts a socket connection and processes the request by
   * calling function Thermal::Server::doit()
   *
   * There are N_THREADS-1 of these, one for each non-blocking connection.
   * These threads never terminate.
   *
   * This function differs from block_main only in that the call to Accept
   * is mutex-protected.
   *
   */
  void Server::thread_main( Thermal::Server &thermal, Network::TcpSocket sock ) {
    while (1) {
      thermal.conn_mutex.lock();
      sock.Accept();
      thermal.conn_mutex.unlock();
      thermal.doit(sock);        // call function to do the work
      sock.Close();
    }
    return;
  }
  /***** Server::thread_main **************************************************/


  /***** Server::async_main ***************************************************/
  /**
   * @brief      asynchronous message sending thread
   * @param[in]  thermal  reference to Thermal::Server object
   * @param[in]  sock     Network::UdpSocket socket object
   *
   * This function is run in a separate thread spawned by main() and
   * loops forever, when a message arrives in the status message queue it is
   * sent out via multi-cast UDP datagram.
   *
   */
  void Server::async_main( Thermal::Server &thermal, Network::UdpSocket sock ) {
    std::string function = "Thermal::Server::async_main";
    int retval;

    retval = sock.Create();                                   // create the UDP socket
    if (retval < 0) {
      logwrite(function, "error creating UDP multicast socket for asynchronous messages");
      thermal.exit_cleanly();                                 // do not continue on error
    }
    if (retval==1) {                                          // exit this thread but continue with daemon
      logwrite(function, "asyncrhonous message port disabled by request");
    }

    while (1) {
      std::string message = thermal.interface.async.dequeue();   // get the latest message from the queue (blocks)
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
   * "<device> [all|<app>] [_BLOCK_] <command> [<arg>]"
   *
   * Valid commands are listed in acamd_commands.h
   *
   */
  void Server::doit(Network::TcpSocket sock) {
    std::string function = "Thermal::Server::doit";
    long  ret;
    std::stringstream message;
    std::string cmd, args;        // arg string is everything after command
    std::vector<std::string> tokens;

    bool connection_open=true;

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] accepted connection on fd " << sock.getfd();
    logwrite( function, message.str() );
#endif

    while (connection_open) {

      // Wait (poll) connected socket for incoming data...
      //
      int pollret;
      if ( ( pollret=sock.Poll() ) <= 0 ) {
        if (pollret==0) {
          message.str(""); message << "Poll timeout on fd " << sock.getfd() << " thread " << sock.id;
          logwrite(function, message.str());
        }
        if (pollret <0) {
          message.str(""); message << "Poll error on fd " << sock.getfd() << " thread " << sock.id << ": " << strerror(errno);
          logwrite(function, message.str());
        }
        break;                      // this will close the connection
      }

      // Data available, now read from connected socket...
      //
      std::string buf;
      char delim='\n';
      if ( ( ret=sock.Read( buf, delim ) ) <= 0 ) {
        if (ret<0) {                // could be an actual read error
          message.str(""); message << "Read error on fd " << sock.getfd() << ": " << strerror(errno);
          logwrite(function, message.str());
        }
        if (ret==-2) {
          message.str(""); message << "timeout reading from fd " << sock.getfd();
          logwrite( function, message.str() );
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

      if ( buf.empty() ) buf="help";                   // no command automatically displays help

      try {
        std::size_t cmd_sep = buf.find_first_of(" ");  // find the first space, which separates command from argument list

        cmd = buf.substr(0, cmd_sep);                  // cmd is everything up until that space

        // If cmd is "poll" then set a polling flag to indicate not to log incoming command.
        // Then shift the buf to the next part of the string after "poll" and look again
        // for a command.
        //
        bool polling = false;
        if ( cmd == "poll" ) {
          buf = buf.substr( cmd_sep+1 );               // shift buf to start after the space
          cmd_sep = buf.find_first_of(" ");            // find again the first space, which separates command from argument list
          cmd = buf.substr(0, cmd_sep);                // cmd is everything up until that space
          polling = true;
        }

        if (cmd.empty()) {sock.Write("\n"); continue;} // acknowledge empty command so client doesn't time out

        if (cmd_sep == std::string::npos) {            // If no space was found,
          args.clear();                                // then the arg list is empty,
        }
        else {
          args= buf.substr(cmd_sep+1);                 // otherwise args is everything after that space.
        }

        sock.id = ++this->cmd_num;
        if ( this->cmd_num == INT_MAX ) this->cmd_num = 0;

        if ( not polling ) {
          message.str(""); message << "received command on fd " << sock.getfd() << " (" << sock.id << "): " << cmd << " " << args;
          logwrite(function, message.str());
        }
      }
      catch ( const std::runtime_error &e ) {
        std::stringstream errstream; errstream << e.what();
        message.str(""); message << "error parsing arguments: " << errstream.str();
        logwrite(function, message.str());
        ret = -1;
      }
      catch ( ... ) {
        message.str(""); message << "unknown error parsing arguments: " << args;
        logwrite(function, message.str());
        ret = -1;
      }

      // process commands here
      //
      ret = NOTHING;
      std::string retstring;

      if ( cmd.compare( "help" ) == 0 || cmd.compare( "?" ) == 0 ) {
                      for ( const auto &s : THERMALD_SYNTAX ) { retstring.append( s ); retstring.append( "\n" ); }
                      ret= HELP;
      }
      else
      if ( cmd.compare( THERMALD_EXIT ) == 0 ) {
                      this->exit_cleanly();                     // shutdown the daemon
      }
      else

      // ECHO
      //
      if ( cmd.compare( THERMALD_ECHO ) == 0 ) {
                      if ( args == "?" ) {
                        retstring = THERMALD_ECHO;
                        retstring.append( +" <string>\n  server receives and writes back <string> to the client.\n" );
                        retstring.append( "  Used to test if the server is responsive.\n" );
                        ret = HELP;
                      }
                      else {
                        sock.Write( args );
                        sock.Write( "\n" );
                      }
      }
      else

      // GET
      //
      if ( cmd.compare( THERMALD_GET ) == 0 ) {
                      ret = this->interface.get( args, retstring );
      }
      else

      // print lakeshore labels
      //
      if ( cmd.compare( THERMALD_PRINTLABELS ) == 0 ) {
                      ret = this->interface.print_labels( args, retstring );
      }
      else

      // NATIVE
      //
      if ( cmd.compare( THERMALD_NATIVE ) == 0 ) {
                      ret = this->interface.native( args, retstring );
      }
      else

      // RECONNECT
      // Must run configure_* functions first because a device may have been
      // removed from the maps if they had an error.
      //
      if ( cmd.compare( THERMALD_RECONNECT ) == 0 ) {
                      if ( args=="?" ) {
                        retstring = THERMALD_RECONNECT;
                        retstring.append( "\n" );
                        retstring.append( "  Stops telemetry, re-reads config file, applies configuration,\n" );
                        retstring.append( "  closes and re-opens connections to hardware, starts telemetry.\n" );
                        ret = HELP;
                      }
                      else {
                        ret = this->telemetry( "stop", retstring );
                        ret = this->configure_devices();
                        ret = this->interface.reconnect( args, retstring );
                        ret = this->telemetry( "start", retstring );
                      }
      }
      else

      // setpoint
      //
      if ( cmd.compare( THERMALD_SETPOINT ) == 0 ) {
                      ret = this->interface.setpoint( args, retstring );
      }
      else

      // telemetry
      //
      if ( cmd.compare( THERMALD_TELEMETRY ) == 0 ) {
                      ret = this->telemetry( args, retstring );
      }
      else
      if ( cmd == THERMALD_SHOWTELEM ) {
                      ret = this->interface.show_telemdata( args, retstring );
      }
      else

      // send telemetry upon request
      //
      if ( cmd == TELEMREQUEST ) {
                      if ( args=="?" || args=="help" ) {
                        retstring=TELEMREQUEST+"\n";
                        retstring.append( "  Returns a serialized JSON message containing telemetry\n" );
                        retstring.append( "  information, terminated with \"EOF\\n\".\n" );
                        ret=HELP;
                      }
                      else {
                        this->interface.make_telemetry_message( retstring );
                        ret = JSON;
                      }
      }

      // unknown commands generate an error
      //
      else {
        message.str(""); message << "ERROR: unknown command: " << cmd;
        logwrite( function, message.str() );
        ret = ERROR;
      }

      // If retstring not empty then append "DONE" or "ERROR" depending on value of ret,
      // and log the reply along with the command number. Write the reply back to the socket.
      //
      // Don't append anything nor log the reply if the command was just requesting help.
      //
      if (ret != NOTHING) {
        if ( ! retstring.empty() ) retstring.append( " " );
        if ( ret != HELP && ret != JSON ) retstring.append( ret == NO_ERROR ? "DONE" : "ERROR" );

        if ( ret == JSON ) {
          message.str(""); message << "command (" << this->cmd_num << ") reply with JSON message";
          logwrite( function, message.str() );
        }
        else
        if ( ! retstring.empty() && ret != HELP ) {
          retstring.append( "\n" );
          message.str(""); message << "command (" << this->cmd_num << ") reply: " << retstring;
          logwrite( function, message.str() );
        }

        if ( sock.Write( retstring ) < 0 ) connection_open=false;
      }

      if (!sock.isblocking()) break;       // Non-blocking connection exits immediately.
                                           // Keep blocking connection open for interactive session.
    }

    return;
  }
  /***** Server::doit *********************************************************/


}
