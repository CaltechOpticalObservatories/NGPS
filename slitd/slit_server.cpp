/**
 * @file    slitd.cpp
 * @brief   this is the main slit daemon
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "slit_server.h"

namespace Slit {

  Server* Server::instance = nullptr;

  /***** Slit::Server::handle_signal ******************************************/
  /**
   * @brief      handles ctrl-C and other signals
   * @param[in]  int signo
   *
   */
  void Server::handle_signal(int signo) {
    std::string function = "Slit::Server::handle_signal";
    std::stringstream message;

    switch (signo) {
      case SIGTERM:
      case SIGINT:
        logwrite(function, "received termination signal");
        message << "NOTICE:" << Slit::DAEMON_NAME << " exit";
        Server::instance->interface.async.enqueue( message.str() );
        Server::instance->exit_cleanly();                      // shutdown the daemon
        break;
      case SIGHUP:
        logwrite(function, "ignored SIGHUP");
        break;
      case SIGPIPE:
        logwrite(function, "ignored SIGPIPE");
        break;
      default:
        message << "received unknown signal " << strsignal(signo);
        logwrite( function, message.str() );
        message.str(""); message << "NOTICE:" << Slit::DAEMON_NAME << " exit";
        Server::instance->interface.async.enqueue( message.str() );
        break;
    }
    return;
  }
  /***** Slit::Server::handle_signal ******************************************/


  /***** Slit::Server::exit_cleanly *******************************************/
  /**
   * @brief      shutdown nicely
   *
   */
  void Server::exit_cleanly(void) {
    Server::instance->interface.stop_subscriber_thread();
    logwrite( "Slit::Server::exit_cleanly", "exiting" );
    exit(EXIT_SUCCESS);
  }
  /***** Slit::Server::exit_cleanly *******************************************/


  /***** Slit::Server::configure_slitd ****************************************/
  /**
   * @brief      read and apply the configuration file for the slit daemon
   * @return     ERROR or NO_ERROR
   *
   */
  long Server::configure_slitd() {
    std::string function = "Slit::Server::configure_slitd";
    std::stringstream message;
    int applied=0;
    long error;

    // Clear the motormap map before loading new information from the config file
    //
    this->interface.pi_interface->clear_motormap();

    // loop through the entries in the configuration file, stored in config class
    //
    for (int entry=0; entry < this->config.n_entries; entry++) {

      // NBPORT -- nonblocking listening port for the slit daemon
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
        this->interface.async.enqueue_and_log( function, "SLITD:config:"+config.param[entry]+"="+config.arg[entry] );
        applied++;
      }

      // BLKPORT -- blocking listening port for the slit daemon
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
        this->interface.async.enqueue_and_log( function, "SLITD:config:"+config.param[entry]+"="+config.arg[entry] );
        applied++;
      }

      // ASYNCPORT -- asynchronous broadcast message port for the slit daemon
      //
      if ( config.param[entry].find( "ASYNCPORT" ) == 0 ) {
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
        this->interface.async.enqueue_and_log( function, "SLITD:config:"+config.param[entry]+"="+config.arg[entry] );
        applied++;
      }

      // ASYNCGROUP -- asynchronous broadcast group for the slit daemon
      //
      if ( config.param[entry].find( "ASYNCGROUP" ) == 0 ) {
        this->asyncgroup = config.arg[entry];
        this->interface.async.enqueue_and_log( function, "SLITD:config:"+config.param[entry]+"="+config.arg[entry] );
        applied++;
      }

      // PUB_ENDPOINT -- my ZeroMQ socket endpoint for publishing
      //
      if ( config.param[entry] == "PUB_ENDPOINT" ) {
        this->interface.publisher_address = config.arg[entry];
        this->interface.publisher_topic = DAEMON_NAME;   // default publish topic is my name
        this->interface.async.enqueue_and_log( function, "SLITD:config:"+config.param[entry]+"="+config.arg[entry] );
        applied++;
      }

      // SUB_ENDPOINT
      //
      if ( config.param[entry] == "SUB_ENDPOINT" ) {
        this->interface.subscriber_address = config.arg[entry];
        this->interface.async.enqueue_and_log( function, "SLITD:config:"+config.param[entry]+"="+config.arg[entry] );
        applied++;
      }

      // MOTOR_CONTROLLER -- address and name of each PI motor controller in daisy-chain
      //
      if ( config.param[entry].find( "MOTOR_CONTROLLER" ) == 0 ) {
        std::istringstream iss(config.arg[entry]);
        std::string name, type, host;
        int port, addr, axes;
        long ret=NO_ERROR;

        if (!(iss >> name >> type >> host >> port >> addr >> axes)) {
          logwrite(function, "ERROR: bad config input. Expected { <name> <type> <host> <port> <addr> <axes> }");
          error=ERROR;
          continue;
        }

        // call the load_controller_config for the appropriate vendor
        if (type=="PI") {
          if (!this->interface.pi_interface) {
            // initialize a pointer if it doesn't already exist
            this->interface.pi_interface = std::make_unique<
            Physik_Instrumente::Interface<Physik_Instrumente::ServoInfo>>(MOVE_TIMEOUT,
                                                                          HOME_TIMEOUT,
                                                                          NAN);
          }
          this->interface.motors.emplace(name, MotionController::Name(this->interface.pi_interface.get(), name));
          ret = this->interface.pi_interface->load_controller_config(config.arg[entry]);
        }
        else {
          logwrite(function, "ERROR: unknown type. Expected { PI | GALIL }");
          error=ERROR;
          continue;
        }

        if (ret==NO_ERROR) {
          message.str(""); message << "SLITD:config:" << config.param[entry] << "=" << config.arg[entry];
          this->interface.async.enqueue_and_log( function, message.str() );
          applied++;
        }
      }

      // MOTOR_AXIS -- axis info for specified MOTOR_CONTROLLER
      //
      if ( config.param[entry].find( "MOTOR_AXIS" ) == 0 ) {
        std::istringstream iss(config.arg[entry]);
        std::string name, ref;
        int axis;
        double min, max, zero, def;

        if (!(iss >> name >> axis >> min >> max >> zero >> ref)) {
          logwrite(function, "ERROR: bad config input. Expected { <name> <axis> <min> <max> <zero> <ref> }");
          error=ERROR;
          continue;
        }
        if (!(iss >> def)) def=NAN;

        // Each AXIS is associated with a CONTROLLER by name, so a controller
        // of this name must have already been configured.
        //
        if (this->interface.motors.find(name) == this->interface.motors.end()) {
          logwrite(function, "ERROR bad config: motor name '"+name+"' has no match in MOTOR_CONTROLLER");
          error=ERROR;
          continue;
        }

        // parse the axis info into an object
        Physik_Instrumente::AxisInfo AXIS;
        if ( AXIS.load_axis_info( config.arg[entry] ) == ERROR ) break;

        // add that object to this motor
        if ( (error=this->interface.motors.at(name).add_axis(AXIS)) != NO_ERROR ) continue;
        message.str(""); message << "SLITD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->interface.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // MIN_WIDTH: minimum slit width in physical units (mm)
      //
      if ( config.param[entry] == "MIN_WIDTH_MM" ) {
        try {
          this->interface.minwidth = SlitDimension( std::stof( config.arg[entry] ), Unit::MM );
        }
        catch ( const std::exception &e ) {
          logwrite( function, "ERROR parsing MIN_WIDTH "+config.arg[entry]+": "+e.what() );
          error=ERROR;
          break;
        }
        this->interface.async.enqueue_and_log( function, "SLITD:config:"+config.param[entry]+"="+config.arg[entry] );
        applied++;
      }

      // SLIT_CENTER_MM: Position of center in actuator units (mm)
      //
      if ( config.param[entry] == "SLIT_CENTER_MM" ) {
        try {
          this->interface.center = SlitDimension( std::stof( config.arg[entry] ), Unit::MM );
        }
        catch ( const std::exception &e ) {
          logwrite( function, "ERROR parsing SLIT_CENTER_MM "+config.arg[entry]+": "+e.what() );
          error=ERROR;
          break;
        }
        this->interface.async.enqueue_and_log( function, "SLITD:config:"+config.param[entry]+"="+config.arg[entry] );
        applied++;
      }

      // ARCSEC_PER_MM: conversion of actuator units to arcseconds
      //
      if ( config.param[entry] == "ARCSEC_PER_MM" ) {
        try {
          SlitDimension::initialize_arcsec_per_mm(std::stof(config.arg[entry]));
        }
        catch ( const std::exception &e ) {
          message.str(""); message << "ERROR parsing ARCSEC_PER_MM " << config.arg[entry] << ": " << e.what();
          logwrite( function, message.str() );
          error=ERROR;
          break;
        }
        this->interface.async.enqueue_and_log( function, "SLITD:config:"+config.param[entry]+"="+config.arg[entry] );
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

    if ( error == NO_ERROR ) error = this->interface.initialize_class();

    return error;
  }
  /***** Slit::Server::configure_slitd ****************************************/


  /***** Slit::Server::new_log_day ********************************************/
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
      init_log( logpath, Slit::DAEMON_NAME );
      // ensure it doesn't immediately re-open
      std::this_thread::sleep_for( std::chrono::seconds(1) );
    }
  }
  /***** Slit::Server::new_log_day ********************************************/


  /***** Server::block_main ***************************************************/
  /**
   * @brief      main function for blocking connection thread
   * @param[in]  slit   reference to Slit::Server object
   * @param[in]  sock   Network::TcpSocket socket object
   *
   * This function is run in a separate thread spawned by main() and
   * accepts a socket connection and processes the request by
   * calling function Slit::Server::doit()
   *
   * This thread never terminates.
   *
   */
  void Server::block_main( Slit::Server &slit, Network::TcpSocket sock ) {
    while(1) {
      sock.Accept();
      slit.doit(sock);              // call function to do the work
      sock.Close();
    }
    return;
  }
  /***** Server::block_main ***************************************************/


  /***** Server::thread_main **************************************************/
  /**
   * @brief      main function for all non-blocked threads
   * @param[in]  slit   reference to Slit::Server object
   * @param[in]  sock   Network::TcpSocket socket object
   *
   * This function is run in a separate thread spawned by main() and
   * accepts a socket connection and processes the request by
   * calling function Slit::Server::doit()
   *
   * There are N_THREADS-1 of these, one for each non-blocking connection.
   * These threads never terminate.
   *
   * This function differs from block_main only in that the call to Accept
   * is mutex-protected.
   *
   */
  void Server::thread_main( Slit::Server &slit, Network::TcpSocket sock ) {
    while (1) {
      {
      std::lock_guard<std::mutex> lock( slit.conn_mutex );
      sock.Accept();
      }
      slit.doit(sock);           // call function to do the work
      sock.Close();
    }
    return;
  }
  /***** Server::thread_main **************************************************/


  /***** Server::async_main ***************************************************/
  /**
   * @brief      asynchronous message sending thread
   * @param[in]  slit   reference to Slit::Server object
   * @param[in]  sock   Network::UdpSocket socket object
   *
   * This function is run in a separate thread spawned by main() and
   * loops forever, when a message arrives in the status message queue it is
   * sent out via multi-cast UDP datagram.
   *
   */
  void Server::async_main( Slit::Server &slit, Network::UdpSocket sock ) {
    std::string function = "Slit::Server::async_main";
    int retval;

    retval = sock.Create();                                   // create the UDP socket
    if (retval < 0) {
      logwrite(function, "error creating UDP multicast socket for asynchronous messages");
      slit.exit_cleanly();                                    // do not continue on error
    }
    if (retval==1) {                                          // exit this thread but continue with daemon
      logwrite(function, "asyncrhonous message port disabled by request");
    }

    while (1) {
      std::string message = slit.interface.async.dequeue();   // get the latest message from the queue (blocks)
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
    std::string function = "Slit::Server::doit";
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

      if (buf.empty()) {sock.Write("\n"); continue;}   // acknowledge empty command so client doesn't time out

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

      if ( cmd == "help" || cmd == "?" ) {
                      for ( const auto &s : SLITD_SYNTAX ) { retstring.append( s ); retstring.append( "\n" ); }
                      ret = HELP;
      }
      else

      if ( cmd == "exit" ) {
                      this->exit_cleanly();                     // shutdown the daemon
      }
      else

      // isopen
      //
      if ( cmd == SLITD_ISOPEN ) {
                      ret = this->interface.is_open( args, retstring );
      }
      else

      // open
      //
      if ( cmd == SLITD_OPEN ) {
                      ret = this->interface.open();
      }
      else

      // close
      //
      if ( cmd == SLITD_CLOSE ) {
                      ret = this->interface.close();
      }
      else

      // echo
      //
      if ( cmd == "echo" ) {
                      sock.Write( args );
                      sock.Write( "\n" );
                      ret = NO_ERROR;
      }
      else

      // home
      //
      if ( cmd == SLITD_HOME ) {
                      ret = this->interface.home( args, retstring );
      }
      else

      // is_home
      //
      if ( cmd == SLITD_ISHOME ) {
                      ret = this->interface.is_home( args, retstring );
      }
      else

      // set offset only
      //
      if ( cmd == SLITD_OFFSET ) {
                      ret = this->interface.offset( args, retstring );
      }
      else

      // set width and offset
      //
      if ( cmd == SLITD_SET ) {
                      ret = this->interface.set( args, retstring );
      }
      else

      // get width and offset
      //
      if ( cmd == SLITD_GET ) {
                      ret = this->interface.get( args, retstring );
      }
      else

      // native
      //
      if ( cmd == SLITD_NATIVE ) {
                      ret = this->interface.send_command( args, retstring );
      }
      else

      // send telemetry on request
      //
      if ( cmd == TELEMREQUEST ) {
                    if ( args=="?" || args=="help" ) {
                      retstring=TELEMREQUEST+"\n";
                      retstring.append( "  Returns a serialized JSON message containing telemetry\n" );
                      retstring.append( "  information, terminated with \"EOF\\n\".\n" );
                      ret=HELP;
                    }
                    else {
                      this->interface.publish_snapshot(retstring);
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
