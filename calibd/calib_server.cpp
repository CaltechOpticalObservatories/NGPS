/**
 * @file    calib_server.cpp
 * @brief   this is the main calib server
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "calib_server.h"

namespace Calib {

  Server* Server::instance = nullptr;

  /***** Calib::Server::handle_signal *****************************************/
  /**
   * @brief      handles ctrl-C and other signals
   * @param[in]  int signo
   *
   */
  void Server::handle_signal(int signo) {
    std::string function = "Calib::Server::handle_signal";
    std::stringstream message;

    switch (signo) {
      case SIGTERM:
      case SIGINT:
        logwrite(function, "received termination signal");
        message << "NOTICE:" << Calib::DAEMON_NAME << " exit";
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
        message.str(""); message << "NOTICE:" << Calib::DAEMON_NAME << " exit";
        Server::instance->interface.async.enqueue( message.str() );
        break;
    }
    return;
  }
  /***** Calib::Server::handle_signal *****************************************/


  /***** Calib::Server::exit_cleanly ******************************************/
  /**
   * @brief      shutdown nicely
   *
   */
  void Server::exit_cleanly(void) {
    Server::instance->interface.stop_subscriber_thread();
    logwrite( "Calib::Server::exit_cleanly", "exiting" );
    close_log();                        // flush and close logfile stream
    _exit(EXIT_SUCCESS);
  }
  /***** Calib::Server::exit_cleanly ******************************************/


  /***** Calib::Server::configure_calibd **************************************/
  /**
   * @brief      read and apply the configuration file for the calib daemon
   * @return     ERROR or NO_ERROR
   *
   */
  long Server::configure_calibd() {
    std::string function = "Calib::Server::configure_calibd";
    std::stringstream message;
    int applied=0;
    long error;

    // loop through the entries in the configuration file, stored in config class
    //
    for (int entry=0; entry < this->config.n_entries; entry++) {

      // NBPORT -- nonblocking listening port for the calib daemon
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
        message.str(""); message << "CALIBD:config:" << config.param[entry] << "=" << config.arg[entry];
        logwrite( function, message.str() );
        this->interface.async.enqueue( message.str() );
        applied++;
      }

      // BLKPORT -- blocking listening port for the calib daemon
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
        message.str(""); message << "CALIBD:config:" << config.param[entry] << "=" << config.arg[entry];
        logwrite( function, message.str() );
        this->interface.async.enqueue( message.str() );
        applied++;
      }

      // ASYNCPORT -- asynchronous broadcast message port for the calib daemon
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
        message.str(""); message << "CALIBD:config:" << config.param[entry] << "=" << config.arg[entry];
        logwrite( function, message.str() );
        this->interface.async.enqueue( message.str() );
        applied++;
      }

      // ASYNCGROUP -- asynchronous broadcast group for the calib daemon
      //
      if ( config.param[entry].find( "ASYNCGROUP" ) == 0 ) {
        this->asyncgroup = config.arg[entry];
        message.str(""); message << "CALIBD:config:" << config.param[entry] << "=" << config.arg[entry];
        logwrite( function, message.str() );
        this->interface.async.enqueue( message.str() );
        applied++;
      }

      // PUB_ENDPOINT -- my ZeroMQ socket endpoint for publishing
      //
      if ( config.param[entry] == "PUB_ENDPOINT" ) {
        this->interface.publisher_address = config.arg[entry];
        this->interface.publisher_topic = DAEMON_NAME;   // default publish topic is my name
        this->interface.async.enqueue_and_log( function, "CALIBD:config:"+config.param[entry]+"="+config.arg[entry] );
        applied++;
      }

      // SUB_ENDPOINT
      //
      if ( config.param[entry] == "SUB_ENDPOINT" ) {
        this->interface.subscriber_address = config.arg[entry];
        this->interface.async.enqueue_and_log( function, "CALIBD:config:"+config.param[entry]+"="+config.arg[entry] );
        applied++;
      }

      // MOTOR_CONTROLLER -- address and name of each PI motor controller in daisy-chain
      //                     Each controller is stored in STL map indexed by motorname
      //
      if ( config.param[entry].find( "MOTOR_CONTROLLER" ) == 0 ) {
        if ( this->interface.motion.motorinterface.load_controller_config( config.arg[entry] ) == NO_ERROR ) {
          message.str(""); message << "CALIBD:config:" << config.param[entry] << "=" << config.arg[entry];
          this->interface.async.enqueue_and_log( function, message.str() );
          applied++;
        }
      }

      // MOTOR_AXIS -- axis infor for specified MOTOR_CONTROLLER
      //
      if ( config.param[entry].find( "MOTOR_AXIS" ) == 0 ) {

        Physik_Instrumente::AxisInfo AXIS;

        if ( AXIS.load_axis_info( config.arg[entry] ) == ERROR ) break;

        // Each AXIS is associated with a CONTROLLER by name, so a controller
        // of this name must have already been configured.
        //
        // loc checks if the motorname for this AXIS is found in the motormap
        //
        auto _motormap = this->interface.motion.motorinterface.get_motormap();
        auto loc = _motormap.find( AXIS.motorname );
        if ( loc != _motormap.end() ) {
          this->interface.motion.motorinterface.add_axis( AXIS );
          message.str(""); message << "CALIBD:config:" << config.param[entry] << "=" << config.arg[entry];
          this->interface.async.enqueue_and_log( function, message.str() );
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

        // Create temporary local PosInfo object to load and parse config line
        //
        Physik_Instrumente::PosInfo POS;

        if ( POS.load_pos_info( config.arg[entry] ) == ERROR ) break;

        // Check that motorname associated with position has already been defined
        // in the motormap and if so, add PosInfo to the motorinterface.
        //
        auto _motormap = this->interface.motion.motorinterface.get_motormap();
        auto loc = _motormap.find( POS.motorname );

        if ( loc != _motormap.end() ) {
          this->interface.motion.motorinterface.add_posmap( POS );  // add the PosInfo to the class herre
          message.str(""); message << "CALIBD:config:" << config.param[entry] << "=" << config.arg[entry];
          this->interface.async.enqueue_and_log( function, message.str() );
          applied++;
        }
        else {
          message.str(""); message << "ERROR: MOTOR_POS name \"" << POS.posname << "\" "
                                   << "has no matching name defined by MOTOR_CONTROLLER";
          logwrite( function, message.str() );
          message.str(""); message << "valid names are:";
          for ( const auto &mot : _motormap ) { message << " " << mot.first; }
          logwrite( function, message.str() );
          error = ERROR;
          break;
        }
      }

      // LAMPMOD_HOST -- lamp modulator host info ( host port )
      //
      if ( config.param[entry].find( "LAMPMOD_HOST" ) == 0 ) {
        message.str(""); message << "CALIBD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->interface.async.enqueue_and_log( function, message.str() );
        this->interface.modulator.configure_host( config.arg[entry] );
        applied++;
      }

      // LAMPMOD_MOD -- lamp modulator modulator info
      //
      if ( config.param[entry].find( "LAMPMOD_MOD" ) == 0 ) {
        message.str(""); message << "CALIBD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->interface.async.enqueue_and_log( function, message.str() );
        this->interface.modulator.configure_mod( config.arg[entry] );
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
    message << "applied " << applied << " configuration lines to calibd";
    logwrite(function, message.str());

    if ( error == NO_ERROR ) error = this->interface.motion.configure_class();

    return error;
  }
  /***** Calib::Server::configure_calibd **************************************/


  /***** new_log_day **********************************************************/
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
      init_log( logpath, DAEMON_NAME );
      // ensure it doesn't immediately re-open
      std::this_thread::sleep_for( std::chrono::seconds(1) );
    }
  }
  /***** new_log_day **********************************************************/


  /***** Server::block_main ***************************************************/
  /**
   * @brief      main function for blocking connection thread
   * @param[in]  calib  reference to Calib::Server object
   * @param[in]  sock   Network::TcpSocket socket object
   *
   * This function is run in a separate thread spawned by main() and
   * accepts a socket connection and processes the request by
   * calling function Calib::Server::doit()
   *
   * This thread never terminates.
   *
   */
  void Server::block_main( Calib::Server &calib, Network::TcpSocket sock ) {
    while( true ) {
      sock.Accept();
      calib.doit(sock);             // call function to do the work
      sock.Close();
    }
    return;
  }
  /***** Server::block_main ***************************************************/


  /***** Server::thread_main **************************************************/
  /**
   * @brief      main function for all non-blocked threads
   * @param[in]  calib  reference to Calib::Server object
   * @param[in]  sock   Network::TcpSocket socket object
   *
   * This function is run in a separate thread spawned by main() and
   * accepts a socket connection and processes the request by
   * calling function Calib::Server::doit()
   *
   * There are N_THREADS-1 of these, one for each non-blocking connection.
   * These threads never terminate.
   *
   * This function differs from block_main only in that the call to Accept
   * is mutex-protected.
   *
   */
  void Server::thread_main( Calib::Server &calib, Network::TcpSocket sock ) {
    while ( true ) {
      calib.conn_mutex.lock();
      sock.Accept();
      calib.conn_mutex.unlock();
      calib.doit(sock);          // call function to do the work
      sock.Close();
    }
    return;
  }
  /***** Server::thread_main **************************************************/


  /***** Server::async_main ***************************************************/
  /**
   * @brief      asynchronous message sending thread
   * @param[in]  calib  reference to Calib::Server object
   * @param[in]  sock   Network::UdpSocket socket object
   *
   * This function is run in a separate thread spawned by main() and
   * loops forever, when a message arrives in the status message queue it is
   * sent out via multi-cast UDP datagram.
   *
   */
  void Server::async_main( Calib::Server &calib, Network::UdpSocket sock ) {
    std::string function = "Calib::Server::async_main";
    int retval;

    retval = sock.Create();                                   // create the UDP socket
    if (retval < 0) {
      logwrite(function, "error creating UDP multicast socket for asynchronous messages");
      calib.exit_cleanly();                                   // do not continue on error
    }
    if (retval==1) {                                          // exit this thread but continue with daemon
      logwrite(function, "asyncrhonous message port disabled by request");
    }

    while (1) {
      std::string message = calib.interface.async.dequeue();  // get the latest message from the queue (blocks)
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
    std::string function = "Calib::Server::doit";
    long  ret;
    std::stringstream message;
    std::string cmd, args;        // arg string is everything after command
    std::vector<std::string> tokens;

    bool connection_open=true;

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] accepted connection on fd " << sock.getfd();
    logwrite( function, message.str() );
#endif

    while ( connection_open ) {

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
                      for ( const auto &s : CALIBD_SYNTAX ) { retstring.append( s ); retstring.append( "\n" ); }
                      ret = HELP;
      }
      else

      if ( cmd == "exit" ) {
                      sock.Write( "\n" );  // write terminating char in case client is waiting for something
                      ret = EXIT;          // this will raise SIGUSR1
                      break;
      }
      else

      // isopen
      //
      if ( cmd == CALIBD_ISOPEN ) {
                      ret = this->interface.is_open( args, retstring );
      }
      else

      // open
      //
      if ( cmd == CALIBD_OPEN ) {
                      ret = this->interface.open(args, retstring);
      }
      else

      // close
      //
      if ( cmd == CALIBD_CLOSE ) {
                      ret = this->interface.close(args, retstring);
      }
      else

      // home
      //
      if ( cmd == CALIBD_HOME ) {
                      ret = this->interface.motion.home( args, retstring );
      }
      else

      // ishome
      //
      if ( cmd == CALIBD_ISHOME ) {
                      ret = this->interface.motion.is_home( args, retstring );
      }
      else

      // get state of one or both actuators
      //
      if ( cmd == CALIBD_GET ) {
                      ret = this->interface.motion.get( args, retstring );
      }
      else

      // set state of one or both actuators
      // args should be actuator=state [ actuator=state ]
      //
      if ( cmd == CALIBD_SET ) {
                      ret = this->interface.motion.set( args, retstring );
      }
      else

      // native
      //
      if ( cmd == CALIBD_NATIVE ) {
                      ret = this->interface.motion.send_command( args, retstring );
      }
      else

      // lampmod
      //
      if ( cmd == CALIBD_LAMPMOD ) {
                      ret = this->interface.modulator.control( args, retstring );
      }
      else

      // telemetry request
      //
      if ( cmd == SNAPSHOT || cmd == TELEMREQUEST ) {
                      if ( args=="?" || args=="help" ) {
                        retstring=TELEMREQUEST+"\n";
                        retstring.append( "  Returns a serialized JSON message containing telemetry\n" );
                        retstring.append( "  information, terminated with \"EOF\\n\".\n" );
                        ret=HELP;
                      }
                      else {
                        this->interface.publish_snapshot( retstring );
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

      if ( ret==NO_ERROR ) this->interface.publish_snapshot();

      if (!sock.isblocking()) break;       // Non-blocking connection exits immediately.
                                           // Keep blocking connection open for interactive session.
    }

    if ( ret == EXIT ) raise( SIGUSR1 );
    return;
  }
  /***** Server::doit *********************************************************/

}
