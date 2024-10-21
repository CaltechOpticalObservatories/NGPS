/**
 * @file    focusd.cpp
 * @brief   this is the main focus daemon
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "focus_server.h"

namespace Focus {


  /***** Focus::Server::exit_cleanly ******************************************/
  /**
   * @brief      shutdown nicely
   *
   */
  void Server::exit_cleanly(void) {
    std::string function = "Focus::Server::exit_cleanly";
    logwrite( function, "exiting" );

    exit(EXIT_SUCCESS);
  }
  /***** Focus::Server::exit_cleanly ******************************************/


  /***** Focus::Server::configure_focusd **************************************/
  /**
   * @brief      read and apply the configuration file for the focus daemon
   * @return     ERROR or NO_ERROR
   *
   */
  long Server::configure_focusd() {
    std::string function = "Focus::Server::configure_focusd";
    std::stringstream message;
    int applied=0;
    long error;

    // Clear the motormap map before loading new information from the config file
    //
    this->interface.motorinterface.clear_motormap();

    // loop through the entries in the configuration file, stored in config class
    //
    for (int entry=0; entry < this->config.n_entries; entry++) {

      // NBPORT -- nonblocking listening port for the focus daemon
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
        message.str(""); message << "FOCUSD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->interface.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // BLKPORT -- blocking listening port for the focus daemon
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
        message.str(""); message << "FOCUSD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->interface.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // ASYNCPORT -- asynchronous broadcast message port for the focus daemon
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
        message.str(""); message << "FOCUSD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->interface.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // ASYNCGROUP -- asynchronous broadcast group for the focus daemon
      //
      if ( config.param[entry].find( "ASYNCGROUP" ) == 0 ) {
        this->asyncgroup = config.arg[entry];
        message.str(""); message << "FOCUSD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->interface.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // MOTOR_CONTROLLER -- address and name of each PI motor controller in daisy-chain
      //                     Each CONTROLLER is stored in an STL map indexed by motorname
      //
      if ( config.param[entry].find( "MOTOR_CONTROLLER" ) == 0 ) {
        if ( this->interface.motorinterface.load_controller_config( config.arg[entry] ) == NO_ERROR ) {
          message.str(""); message << "FOCUSD:config:" << config.param[entry] << "=" << config.arg[entry];
          this->interface.async.enqueue_and_log( function, message.str() );
          applied++;
        }
      }

      // MOTOR_AXIS -- axis info for specified MOTOR_CONTROLLER
      //
      if ( config.param[entry].find( "MOTOR_AXIS" ) == 0 ) {

        Physik_Instrumente::AxisInfo AXIS;

        if ( AXIS.load_axis_info( config.arg[entry] ) == ERROR ) break;

        // Each AXIS is associated with a CONTROLLER by name, so a controller
        // of this name must have already been configured.
        //
        // loc checks if the motorname for this AXIS is found in the motormap
        //
        auto _motormap = this->interface.motorinterface.get_motormap();
        auto loc = _motormap.find( AXIS.motorname );
        if ( loc != _motormap.end() ) {
          this->interface.motorinterface.add_axis( AXIS );
          message.str(""); message << "FOCUSD:config:" << config.param[entry] << "=" << config.arg[entry];
          this->interface.async.enqueue_and_log( function, message.str() );
          applied++;
        }
        else {
          message.str(""); message << "ERROR motor name \"" << AXIS.motorname << "\" "
                                   << "has no matching name defined by MOTOR_CONTROLLER";
          logwrite( function, message.str() );
          message.str(""); message << "valid names are:";
          for ( auto mot : _motormap ) { message << " " << mot.first; }
          logwrite( function, message.str() );
          error = ERROR;
          break;
        }
      }

      // MOTOR_POS -- position info for nominal focus, the only named position
      //
      if ( config.param[entry].find( "MOTOR_POS" ) == 0 ) {

        // Create a local PosInfo object,
        // then use the PosInfo::load_pos_info() function to parse and load it.
        //
        Physik_Instrumente::PosInfo POS;

        if ( POS.load_pos_info( config.arg[entry] ) == ERROR ) break;

        // Check that the motorname associated with the position has already been
        // defined in the motormap. If so, then add the PosInfo to the motorinterface.
        //
        auto _motormap = this->interface.motorinterface.get_motormap();
        auto loc = _motormap.find( POS.motorname );

        if ( loc != _motormap.end() ) {
          this->interface.motorinterface.add_posmap( POS );  // add the PosInfo to the class here
          message.str(""); message << "FOCUSD:config:" << config.param[entry] << "=" << config.arg[entry];
          interface.async.enqueue_and_log( function, message.str() );
          applied++;
        }
        else {
          message.str(""); message << "ERROR motor name \"" << POS.motorname << "\" "
                                   << "has no matching name defined by MOTOR_CONTROLLER";
          logwrite( function, message.str() );
          logwrite( function, "valid names are:" );
          for ( auto mot : _motormap ) { logwrite( function, mot.first); }
          error = ERROR;
          break;
        }
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
    message << "applied " << applied << " configuration lines to focusd";
    logwrite(function, message.str());

    if ( error == NO_ERROR ) error = this->interface.initialize_class();

    return error;
  }
  /***** Focus::Server::configure_focusd **************************************/


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
      init_log( logpath, Focus::DAEMON_NAME );
    }
  }
  /***** Server::new_log_day **************************************************/


  /***** Server::block_main ***************************************************/
  /**
   * @brief      main function for blocking connection thread
   * @param[in]  focus  reference to Focus::Server object
   * @param[in]  sock   Network::TcpSocket socket object
   *
   * This function is run in a separate thread spawned by main() and
   * accepts a socket connection and processes the request by
   * calling function Focus::Server::doit()
   *
   * This thread never terminates.
   *
   */
  void Server::block_main( Focus::Server &focus, Network::TcpSocket sock ) {
    while(1) {
      sock.Accept();
      focus.doit(sock);             // call function to do the work
      sock.Close();
    }
    return;
  }
  /***** Server::block_main ***************************************************/


  /***** Server::thread_main **************************************************/
  /**
   * @brief      main function for all non-blocked threads
   * @param[in]  focus  reference to Focus::Server object
   * @param[in]  sock   Network::TcpSocket socket object
   *
   * This function is run in a separate thread spawned by main() and
   * accepts a socket connection and processes the request by
   * calling function Focus::Server::doit()
   *
   * There are N_THREADS-1 of these, one for each non-blocking connection.
   * These threads never terminate.
   *
   * This function differs from block_main only in that the call to Accept
   * is mutex-protected.
   *
   */
  void Server::thread_main( Focus::Server &focus, Network::TcpSocket sock ) {
    while (1) {
      {
      std::lock_guard<std::mutex> lock(focus.conn_mutex);
      sock.Accept();
      }
      focus.doit(sock);          // call function to do the work
      sock.Close();
    }
    return;
  }
  /***** Server::thread_main **************************************************/


  /***** Server::async_main ***************************************************/
  /**
   * @brief      asynchronous message sending thread
   * @param[in]  focus  reference to Focus::Server object
   * @param[in]  sock   Network::UdpSocket socket object
   *
   * This function is run in a separate thread spawned by main() and
   * loops forever, when a message arrives in the status message queue it is
   * sent out via multi-cast UDP datagram.
   *
   */
  void Server::async_main( Focus::Server &focus, Network::UdpSocket sock ) {
    std::string function = "Focus::Server::async_main";
    int retval;

    retval = sock.Create();                                   // create the UDP socket
    if (retval < 0) {
      logwrite(function, "error creating UDP multicast socket for asynchronous messages");
      focus.exit_cleanly();                                   // do not continue on error
    }
    if (retval==1) {                                          // exit this thread but continue with daemon
      logwrite(function, "asyncrhonous message port disabled by request");
    }

    while (1) {
      std::string message = focus.interface.async.dequeue();  // get the latest message from the queue (blocks)
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
    std::string function = "Focus::Server::doit";
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
                      for ( const auto &s : FOCUSD_SYNTAX ) { retstring.append( s ); retstring.append( "\n" ); }
                      ret = HELP;
      }
      else

      if ( cmd == "exit" ) {
                      this->exit_cleanly();                     // shutdown the daemon
      }
      else

      // isopen
      //
      if ( cmd == FOCUSD_ISOPEN ) {
                      ret = this->interface.is_open( args, retstring );
      }
      else

      // open
      //
      if ( cmd == FOCUSD_OPEN ) {
                      ret = this->interface.open();
      }
      else

      // close
      //
      if ( cmd == FOCUSD_CLOSE ) {
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
      if ( cmd == FOCUSD_HOME ) {
                      ret = this->interface.home( args, retstring );
      }
      else

      // is_home
      //
      if ( cmd == FOCUSD_ISHOME ) {
                      ret = this->interface.is_home( args, retstring );
      }
      else

      // set position
      //
      if ( cmd == FOCUSD_SET ) {
                      ret = this->interface.set( args, retstring );
      }
      else

      // get position
      //
      if ( cmd == FOCUSD_GET ) {
                      ret = this->interface.get( args, retstring );
      }
      else

      // native
      //
      if ( cmd == FOCUSD_NATIVE ) {
                      ret = this->interface.native( args, retstring );
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
      else

      // test routines
      //
      if ( cmd == FOCUSD_TEST ) {
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
  /***** Server::doit *********************************************************/

}
