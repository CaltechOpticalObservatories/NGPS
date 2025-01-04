/**
 * @file    acam_server.cpp
 * @brief   these are the main functions used by the Acam::Server
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 * This file defines the functions used by the Server class in the Acam namespace.
 *
 */

#include "acam_server.h"

namespace Acam {

  Server* Server::instance = nullptr;

  /***** Acam::Server::handle_signal ******************************************/
  /**
   * @brief      handles ctrl-C and other signals
   * @param[in]  int signo
   *
   */
  void Server::handle_signal(int signo) {
    std::string function = "Acam::Server::handle_signal";
    std::stringstream message;

    switch (signo) {
      case SIGTERM:
      case SIGINT:
        logwrite(function, "received termination signal");
        message << "NOTICE:" << Acam::DAEMON_NAME << " exit";
        Server::instance->interface.async.enqueue( message.str() );
        Server::instance->exit_cleanly();                      // shutdown the daemon
        break;
      case SIGHUP:
        if ( Server::instance->interface.configure_interface( Server::instance->config ) != NO_ERROR ) {
          logwrite( function, "ERROR unable to configure interface" );
          Server::instance->interface.async.enqueue_and_log( function, message.str() );
        }
        break;
      case SIGPIPE:
        logwrite(function, "ignored SIGPIPE");
        break;
      default:
        message << "received unknown signal " << strsignal(signo);
        logwrite( function, message.str() );
        message.str(""); message << "NOTICE:" << Acam::DAEMON_NAME << " exit";
        Server::instance->interface.async.enqueue( message.str() );
        break;
    }
    return;
  }
  /***** Acam::Server::handle_signal ******************************************/


  /***** Acam::Server::exit_cleanly *******************************************/
  /**
   * @brief      shutdown nicely
   *
   */
  void Server::exit_cleanly(void) {
    Server::instance->interface.close();
    Server::instance->interface.stop_subscriber_thread();
    logwrite( "Acam::Server::exit_cleanly", "exiting" );
    _exit(EXIT_SUCCESS);
  }
  /***** Acam::Server::exit_cleanly *******************************************/


  /***** Acam::Server::configure_acam *****************************************/
  /**
   * @brief      read and apply the configuration file
   * @return     ERROR or NO_ERROR
   *
   */
  long Server::configure_acamd() {
    std::string function = "Acam::Server::configure_acamd";
    std::stringstream message;
    int applied=0;
    long error;

    // Clear the motormap map before loading new information from the config file
    //
    this->interface.motion.motorinterface.clear_motormap();

    // loop through the entries in the configuration file, stored in config class
    //
    for (int entry=0; entry < this->config.n_entries; entry++) {

      // MOTOR_CONTROLLER -- address and name of each PI motor controller in daisy-chain
      //
      // Each CONTROLLER is stored in an STL map indexed by motorname
      //
      if ( config.param[entry].find( "MOTOR_CONTROLLER" ) == 0 ) {
        if ( this->interface.motion.motorinterface.load_controller_config( config.arg[entry] ) == NO_ERROR ) {
          message.str(""); message << "ACAMD:config:" << config.param[entry] << "=" << config.arg[entry];
          logwrite( function, message.str() );
          this->interface.async.enqueue( message.str() );
          applied++;
        }
      }

      // MOTOR_AXIS -- axis info for specified MOTOR_CONTROLLER
      //
      if ( config.param[entry].find( "MOTOR_AXIS" ) == 0 ) {

        // Create a temporary local AxisInfo object for parsing the config line.
        //
        Physik_Instrumente::AxisInfo AXIS;

        if ( AXIS.load_axis_info( config.arg[entry] ) == ERROR ) break;

        // Each AXIS is associated with a CONTROLLER by name, so a controller
        // of this name must have already been configured.
        //
        // loc checks if the motorname for this AXIS is found in the motormap
        //
        auto _motormap = this->interface.motion.motorinterface.get_motormap();  // get a local copy of motormap
        auto loc = _motormap.find( AXIS.motorname );                            // find motorname in motormap
        if ( loc != _motormap.end() ) {
          this->interface.motion.motorinterface.add_axis( AXIS );
          message.str(""); message << "ACAMD:config:" << config.param[entry] << "=" << config.arg[entry];
          logwrite( function, message.str() );
          this->interface.async.enqueue( message.str() );
          applied++;
        }
        else {
          message.str(""); message << "ERROR motor name \"" << AXIS.motorname << "\" "
                                   << "has no matching name defined by MOTOR_CONTROLLER";
          logwrite( function, message.str() );
          message.str(""); message << "valid names are:";
          for ( const auto &mot : _motormap ) { message << " " << mot.first; }
          logwrite( function, message.str() );
          error = ERROR;
          break;
        }
      }

      // MOTOR_POS -- detailed position info for each named motor controller
      //
      if ( config.param[entry].find( "MOTOR_POS" ) == 0 ) {
        // Create a local PosInfo object for parsing the config line
        //
        Physik_Instrumente::PosInfo POS;

        if ( POS.load_pos_info( config.arg[entry] ) == ERROR ) break;

        // Check that the motorname associated with the position has already been
        // defined in the motormap and if so, add the PosInfo to the motorinterface.
        //
        auto _motormap = this->interface.motion.motorinterface.get_motormap();  // get a local copy of motormap
        auto loc = _motormap.find( POS.motorname );                             // find motorname in motormap

        if ( loc != _motormap.end() ) {
          this->interface.motion.motorinterface.add_posmap( POS );              // add the PosInfo to the class
          message.str(""); message << "ACAMD:config:" << config.param[entry] << "=" << config.arg[entry];
          logwrite( function, message.str() );
          this->interface.async.enqueue( message.str() );
          applied++;
        }
        else {
          message.str(""); message << "ERROR motor name \"" << POS.motorname << "\" "
                                   << "has no matching name defined by MOTOR_CONTROLLER";
          logwrite( function, message.str() );
          message.str(""); message << "valid names are:";
          for ( const auto &mot : _motormap ) { message << " " << mot.first; }
          logwrite( function, message.str() );
          error = ERROR;
          break;
        }
      }

      // NBPORT -- nonblocking listening port for the acam daemon
      //
      if ( config.param[entry].find( "NBPORT" ) == 0 ) {
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
        message.str(""); message << "ACAMD:config:" << config.param[entry] << "=" << config.arg[entry];
        logwrite( function, message.str() );
        this->interface.async.enqueue( message.str() );
        applied++;
      }

      // BLKPORT -- blocking listening port for the acam daemon
      //
      if ( config.param[entry].find( "BLKPORT" ) == 0 ) {
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
        message.str(""); message << "ACAMD:config:" << config.param[entry] << "=" << config.arg[entry];
        logwrite( function, message.str() );
        this->interface.async.enqueue( message.str() );
        applied++;
      }

      // MESSAGEPORT -- asynchronous broadcast message port for the acam daemon
      //
      if ( config.param[entry].find( "MESSAGEPORT" ) == 0 ) {
        int port;
        try {
          port = std::stoi( config.arg[entry] );
        }
        catch (std::invalid_argument &) {
          logwrite(function, "ERROR: bad MESSAGEPORT: unable to convert to integer");
          return(ERROR);
        }
        catch (std::out_of_range &) {
          logwrite(function, "MESSAGEPORT number out of integer range");
          return(ERROR);
        }
        this->asyncport = port;
        message.str(""); message << "ACAMD:config:" << config.param[entry] << "=" << config.arg[entry];
        logwrite( function, message.str() );
        this->interface.async.enqueue( message.str() );
        applied++;
      }

      // MESSAGEGROUP -- asynchronous broadcast group for the acam daemon
      //
      if ( config.param[entry].find( "MESSAGEGROUP" ) == 0 ) {
        this->asyncgroup = config.arg[entry];
        message.str(""); message << "ACAMD:config:" << config.param[entry] << "=" << config.arg[entry];
        logwrite( function, message.str() );
        this->interface.async.enqueue( message.str() );
        applied++;
      }

      // PUB_ENDPOINT -- my ZeroMQ socket endpoint for publishing
      //
      if ( config.param[entry] == "PUB_ENDPOINT" ) {
        this->interface.publisher_address = config.arg[entry];
        this->interface.publisher_topic = DAEMON_NAME;   // default publish topic is my name
        this->interface.async.enqueue_and_log( function, "ACAMD:config:"+config.param[entry]+"="+config.arg[entry] );
        applied++;
      }

      // SUB_ENDPOINT
      //
      if ( config.param[entry] == "SUB_ENDPOINT" ) {
        this->interface.subscriber_address = config.arg[entry];
        this->interface.async.enqueue_and_log( function, "ACAMD:config:"+config.param[entry]+"="+config.arg[entry] );
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
    message << "applied " << applied << " configuration lines to acamd";
    logwrite(function, message.str());

    // Initialize the class using the config parameters just read
    //
    if ( error == NO_ERROR ) error = this->interface.motion.initialize_class();

    return error;
  }
  /***** Acam::Server::configure_acam *****************************************/


  /***** Acam::Server::configure_telemetry ************************************/
  /**
   * @brief      read and apply the configuration file for the telemetry
   * @return     ERROR or NO_ERROR
   *
   */
  long Server::configure_telemetry() {
    std::string function = "Acam::Server::configure_telemetry";
    std::stringstream message;
    int applied=0;
    long error;

    std::string db_host;
    std::string db_port;
    std::string db_user;
    std::string db_pass;
    std::string db_schema;
    std::string db_table;

    this->interface.db_info.clear();

    // loop through the entries in the configuration file, stored in config class
    //
    for (int entry=0; entry < this->config.n_entries; entry++) {

      // DB_HOST
      if (config.param[entry] == "DB_HOST") {
        db_host = config.arg[entry];
        message.str(""); message << DAEMON_NAME << ":config:" << config.param[entry] << "=" << config.arg[entry];
        this->interface.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // DB_PORT
      if (config.param[entry] == "DB_PORT") {
        try { std::stoi( config.arg[entry] ); }
        catch ( const std::exception &e ) {
          message.str(""); message << "ERROR parsing DB_PORT: " << e.what();
          logwrite(function, message.str());
          return(ERROR);
        }
        db_port = config.arg[entry];
        message.str(""); message << "config:" << config.param[entry] << "=" << config.arg[entry];
        this->interface.async.enqueue_and_log( DAEMON_NAME, function, message.str() );
        applied++;
      }

      // DB_USER
      if (config.param[entry] == "DB_USER") {
        db_user = config.arg[entry];
        message.str(""); message << "config:" << config.param[entry] << "=" << config.arg[entry];
        this->interface.async.enqueue_and_log( DAEMON_NAME, function, message.str() );
        applied++;
      }

      // DB_PASS
      if (config.param[entry] == "DB_PASS") {
        db_pass = config.arg[entry];
        message.str(""); message << "config:" << config.param[entry] << "=" << config.arg[entry];
        this->interface.async.enqueue_and_log( DAEMON_NAME, function, message.str() );
        applied++;
      }

      // DB_SCHEMA
      if (config.param[entry] == "DB_SCHEMA") {
        db_schema = config.arg[entry];
        message.str(""); message << "config:" << config.param[entry] << "=" << config.arg[entry];
        this->interface.async.enqueue_and_log( DAEMON_NAME, function, message.str() );
        applied++;
      }

      // DB_TABLE
      if (config.param[entry] == "DB_TABLE") {
        db_table = config.arg[entry];
        message.str(""); message << "config:" << config.param[entry] << "=" << config.arg[entry];
        this->interface.async.enqueue_and_log( DAEMON_NAME, function, message.str() );
        applied++;
      }

      // TELEM_PROVIDER : contains daemon name and port to contact for header telemetry info
      // (these don't get counted with "applied++")
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
        this->interface.async.enqueue_and_log( to_uppercase(DAEMON_NAME), function, message.str() );
      }

    } // end loop through the entries in the configuration file

    this->interface.start_subscriber_thread();

    this->interface.db_info = { db_host, db_port, db_user, db_pass, db_schema, db_table };

    message.str("");
    if (applied!=6) {
      message << "ERROR: expected 6 but ";
      error = ERROR;
    }
    else {
      error = NO_ERROR;
    }

    return error;
  }
  /***** Acam::Server::configure_telemetry ************************************/


  /***** new_log_day **********************************************************/
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
      init_log( logpath, DAEMON_NAME );
    }
  }
  /***** new_log_day **********************************************************/


  /***** block_main ***********************************************************/
  /**
   * @brief      main function for blocking connection thread
   * @param[in]  server  reference to Acam::Server object
   * @param[in]  sock    pointer to Network::TcpSocket socket object
   *
   * accepts a socket connection and processes the request by
   * calling function doit()
   *
   * This thread never terminates.
   *
   */
  void Server::block_main( Acam::Server &server, std::shared_ptr<Network::TcpSocket> sock ) {
    server.threads_active.fetch_add(1);  // atomically increment threads_busy counter
    server.doit(*sock);
    sock->Close();
    server.threads_active.fetch_sub(1);  // atomically increment threads_busy counter
    server.id_pool.release_number( sock->id );
    return;
  }
  /***** block_main ***********************************************************/


  /***** thread_main **********************************************************/
  /**
   * @brief      main function for all non-blocked threads
   * @param[in]  server  reference to Acam::Server object
   * @param[in]  sock    pointer to Network::TcpSocket socket object
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
  void Server::thread_main( Acam::Server &server, std::shared_ptr<Network::TcpSocket> sock ) {
    while (true) {
      {
      std::lock_guard<std::mutex> lock(server.conn_mutex);
      sock->Accept();
      }
      server.doit(*sock);                // call function to do the work
      sock->Close();
    }
    return;
  }
  /***** thread_main **********************************************************/


  /***** async_main ***********************************************************/
  /**
   * @brief      asynchronous message sending thread
   * @param[in]  acam  reference to Acam::Server object
   * @param[in]  sock  Network::UdpSocket socket object
   *
   * Loops forever, when a message arrives in the status message queue it is
   * sent out via multi-cast UDP datagram.
   *
   */
  void Server::async_main( Acam::Server &acam, Network::UdpSocket sock ) {
    std::string function = "Acam::Server::async_main";
    int retval;

    retval = sock.Create();                                   // create the UDP socket
    if (retval < 0) {
      logwrite(function, "error creating UDP multicast socket for asynchronous messages");
      acam.exit_cleanly();                                    // do not continue on error
    }
    if (retval==1) {                                          // exit this thread but continue with daemon
      logwrite(function, "asyncrhonous message port disabled by request");
    }

    while (1) {
      std::string message = acam.interface.async.dequeue();   // get the latest message from the queue (blocks)
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
  /***** async_main ***********************************************************/


  /***** doit *****************************************************************/
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
  void Server::doit( Network::TcpSocket sock ) {
    std::string function = "Acam::Server::doit";
    long  ret;
    std::stringstream message;
    std::string cmd, args;        // arg string is everything after command
    std::vector<std::string> tokens;

    bool connection_open=true;

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
          message.str(""); message << "Read error on fd " << sock.getfd() << ": " << strerror(errno); logwrite(function, message.str());
        }
        if (ret==-2) {              // or a timeout
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

        message.str(""); message << "received command on fd " << sock.getfd() << " (" << sock.id << "): " << cmd << " " << args;
        logwrite(function, message.str());
      }
      catch ( std::runtime_error &e ) {
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

      if ( this->interface.is_shutting_down ) {
                      message.str(""); message << "BUSY: shutdown in progress. ignoring command: " << cmd;
                      logwrite( function, message.str() );
      }
      else
      if ( cmd == "-h" || cmd == "--help" || cmd == "help" || cmd == "?" ) {
                      retstring="acam { <CMD> } [<ARG>...]\n";
                      retstring.append( "  where <CMD> is one of:\n" );
                      for ( const auto &s : ACAMD_SYNTAX ) {
                        retstring.append("  "); retstring.append( s ); retstring.append( "\n" );
                      }
                      ret = HELP;
      }
      else
      if ( cmd == ACAMD_EXIT ) {
                      this->exit_cleanly();                      // shutdown the daemon
      }
      else

      if ( cmd == ACAMD_TCSINIT ) {
                      ret = this->interface.tcs_init( args, retstring );
      }
      else

      if ( cmd == ACAMD_SHUTDOWN ) {
                      ret = this->interface.shutdown( args, retstring );
      }
      else
      if ( cmd == ACAMD_TCSISOPEN ) {
                      ret = this->interface.tcsd.client.is_open();
                      retstring = ( ret ? "true" : "false" );
                      ret = NO_ERROR;
      }
      else
      if ( cmd == ACAMD_TCSISCONNECTED ) {
                      std::string tcsname;
                      ret = this->interface.tcsd.client.is_connected(retstring);
                      if ( retstring=="true" ) {
                        this->interface.tcsd.poll_name(tcsname);
                        retstring.append(" "); retstring.append(tcsname);
                      }
      }
      else
      if ( cmd == ACAMD_TCSGET ) {
                      if ( args.empty() ) {
                        double casangle;
                        this->interface.tcsd.get_cass( casangle );
                        double ra_h, dec_d;
                        this->interface.tcsd.get_coords( ra_h, dec_d );
                        retstring = std::to_string(casangle) + " " + std::to_string(ra_h) + " " + std::to_string(dec_d);
                        ret = NO_ERROR;
                      }
                      else
                      if ( args == "?" || args == "help" ) {
                        retstring = ACAMD_TCSGET;
                        retstring.append( "\n" );
                        retstring.append( "  Returns <cassangle> <ra> <dec>\n" );
                        ret = HELP;
                      }
                      else {
                        logwrite( function, "ERROR invalid argument" );
                        retstring = "invalid_argument";
                        ret = ERROR;
                      }
      }
      else
      if ( cmd == ACAMD_AVGFRAMES ) {
                      ret = this->interface.avg_frames( args, retstring );
      }
      else
      if ( cmd == ACAMD_ACQUIRE ) {
                      ret = this->interface.acquire( args, retstring );
      }
      else
      if ( cmd == ACAMD_ISACQUIRED ) {
                      retstring = ( this->interface.is_target_acquired() ? "true" : "false" );
                      ret = NO_ERROR;
      }
      else
      if ( cmd == ACAMD_COORDS ) {
                      ret = this->interface.target_coords( args, retstring );
      }
      else
      if ( cmd == ACAMD_LOADCALIBRATION ) {
                      ret = this->interface.fpoffsets.load_calibration();
      }
//    else
//    if ( cmd == ACAMD_OFFRATE ) {
//                    ret = this->interface.target_coords( args, retstring );
//    }
      else
      // commands for the Andor camera direct
      //
      if ( cmd == "camerafits" ) {
                      ret = this->interface.test_image( );
      }
      else
      if ( cmd == "cameragetframe" ) {
                      ret = this->interface.camera.get_frame( );
      }
      else
      if ( cmd == "cameratest" ) {
                      ret = this->interface.camera.andor.test( );
      }
      else
      if ( cmd == "testacquire" ) {
                      ret = this->interface.camera.andor.acquire_one();
      }
      else
      if ( cmd == "testsave" ) {
                      ret = this->interface.camera.andor.save_acquired( args, retstring );

      }
      else

      // open connections to all devices, camera and motion
      //
      if ( cmd == ACAMD_OPEN ) {
                      ret = this->interface.open( args, retstring );
      }
      else

      // isopen
      //
      if ( cmd == ACAMD_ISOPEN ) {
                      bool isopen;
                      ret = this->interface.isopen( args, isopen, retstring );
//                    if ( retstring.empty() ) { if ( isopen ) retstring = "true"; else retstring = "false"; }
      }
      else

      // close
      //
      if ( cmd == ACAMD_CLOSE ) {
                      ret = this->interface.close( args, retstring );
      }
      else
      if ( cmd == ACAMD_BIN ) {
                      ret = this->interface.bin( args, retstring );
      }
      else
      if ( cmd == ACAMD_IMFLIP ) {
                      ret = this->interface.camera.imflip( args, retstring );
      }
      else
      if ( cmd == ACAMD_IMROT ) {
                      ret = this->interface.camera.imrot( args, retstring );
      }
      else
      if ( cmd == ACAMD_EXPTIME ) {
                      ret  = this->interface.exptime( args, retstring );         // set exptime
          if (ret==NO_ERROR) this->interface.guider_settings_control();          // update Guider GUI display igores ret
      }
      else
      if ( cmd == ACAMD_FAN ) {
                      ret = this->interface.fan_mode( args, retstring );
      }
      else
      if ( cmd == ACAMD_GAIN ) {
                      ret  = this->interface.camera.gain( args, retstring );     // set gain
          if (ret==NO_ERROR) this->interface.guider_settings_control();          // update Guider GUI display igores ret
      }
      else
      if ( cmd == ACAMD_GUIDESET ) {
                      ret = this->interface.guider_settings_control( args, retstring );
      }
      else
      if ( cmd == ACAMD_SPEED ) {
                      ret = this->interface.camera.speed( args, retstring );
      }
      else
      if ( cmd == ACAMD_TEMP ) {
                      ret = this->interface.camera.temperature( args, retstring );
      }
      else
      if ( cmd == ACAMD_EMULATOR ) {
                      ret = this->interface.camera.emulator( args, retstring );
      }
      else

      // config
      //
      if ( cmd == ACAMD_CONFIG ) {
                      if ( args == "?" ) sock.Write( ACAMD_CONFIG
                                                     +"\n  re-read, parse, and re-apply config file: "
                                                     +this->config.filename+"\n" );
                      else ret = this->interface.configure_interface( config );
      }
      else

      // commands for the Python functions
      //
      if ( cmd == ACAMD_SOLVE ) {
                      ret = this->interface.solve( args, retstring );
      }
      else

      if ( cmd == ACAMD_QUALITY ) {
                      ret = this->interface.image_quality( args, retstring );
      }
      else

      if ( cmd == ACAMD_ECHO ) {
                      if ( args == "?" ) sock.Write( ACAMD_ECHO
                                                     +" <string>\n  Daemon receives and writes back <string> to the client.\n"
                                                     +"  Used to test if the daemon is responsive.\n" );
                      else {
                        sock.Write( args );
                        sock.Write( "\n" );
                      }
      }
      else
      if ( cmd == ACAMD_OFFSETCAL ) {
                      ret = this->interface.offset_cal( args, retstring );
      }
      else
      if ( cmd == ACAMD_OFFSETGOAL ) {
                      ret = this->interface.offset_goal( args, retstring );
      }
      else
      if ( cmd == ACAMD_OFFSETPERIOD ) {
                      ret = this->interface.offset_period( args, retstring );
      }
      else
      if ( cmd == ACAMD_PUTONSLIT ) {
                      ret = this->interface.put_on_slit( args, retstring );
      }
      else
      if ( cmd == ACAMD_FRAMEGRAB ) {
                      ret = this->interface.framegrab( args, retstring );
      }
      else
      if ( cmd == ACAMD_FRAMEGRABFIX ) {
                      ret = this->interface.framegrab_fix( args, retstring );
      }
      else
      if ( cmd == ACAMD_SAVEFRAMES ) {
                      ret = this->interface.saveframes( args, retstring );
      }
      else
      if ( cmd == ACAMD_SKIPFRAMES ) {
                      ret = this->interface.skipframes( args, retstring );
      }
      else
      if ( cmd == ACAMD_INIT ) {
                      this->interface.init_names();
                      ret = NO_ERROR;  // init_names() returns void, never fails
      }
      else

      // motion control commands
      //
      if ( cmd == ACAMD_MOTION ) {
                      ret = this->interface.motion.motion( args, retstring );
      }
      else

      if ( cmd == ACAMD_HOME ) {
                      ret = this->interface.motion.home( args, retstring );
      }
      else

      // is_home
      //
      if ( cmd == ACAMD_ISHOME ) {
                      ret = this->interface.motion.is_home( args, retstring );
      }
      else

      // set/get filter
      //
      if ( cmd == ACAMD_FILTER ) {
                      ret = this->interface.motion.filter( args, retstring );
         if (ret==NO_ERROR) this->interface.guider_settings_control();          // update Guider GUI display igores ret
      }
      else

      // set/get dust cover
      //
      if ( cmd == ACAMD_COVER ) {
                      ret = this->interface.motion.cover( args, retstring );
      }
      else

      // test commands
      //
      if ( cmd == ACAMD_TEST ) {
                      ret = this->interface.test( args, retstring );
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
  /***** doit *****************************************************************/

}
