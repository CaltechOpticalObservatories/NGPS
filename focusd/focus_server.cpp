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
    this->interface.motormap.clear();

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

      // PI_NAME -- this is the name of the PI motor controller subsystem
      //
      if ( config.param[entry].find( "PI_NAME" ) == 0 ) {
        this->interface.name = config.arg[entry];
        message.str(""); message << "FOCUSD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->interface.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // PI_HOST -- hostname for the master PI motor controller
      //
      if ( config.param[entry].find( "PI_HOST" ) == 0 ) {
        this->interface.host = config.arg[entry];
        message.str(""); message << "FOCUSD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->interface.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // PI_PORT -- port number on PI_HOST for the master PI motor controller
      //
      if ( config.param[entry].find( "PI_PORT" ) == 0 ) {
        int port;
        try {
          port = std::stoi( config.arg[entry] );
        }
        catch (std::invalid_argument &) {
          logwrite(function, "ERROR: bad PI_PORT: unable to convert to integer");
          return(ERROR);
        }
        catch (std::out_of_range &) {
          logwrite(function, "PI_PORT number out of integer range");
          return(ERROR);
        }
        this->interface.port = port;
        message.str(""); message << "FOCUSD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->interface.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // MOTOR_CONTROLLER -- address and name of each PI motor controller in daisy-chain
      //
      if ( config.param[entry].find( "MOTOR_CONTROLLER" ) == 0 ) {
        // Create temporary object for parsing the config line. If no error
        // then this object gets copied into the class map of objects.
        //
        Physik_Instrumente::ControllerInfo<Physik_Instrumente::StepperInfo> MOT;

        if ( MOT.load_controller_info( config.arg[entry] ) == NO_ERROR ) {
          this->interface.motormap[ MOT.name ] = MOT;
          message.str(""); message << "FOCUSD:config:" << config.param[entry] << "=" << config.arg[entry];
          logwrite( function, message.str() );
          this->interface.async.enqueue( message.str() );
          applied++;
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
      focus.conn_mutex.lock();
      sock.Accept();
      focus.conn_mutex.unlock();
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

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] accepted connection on fd " << sock.getfd();
    logwrite( function, message.str() );
#endif

    while (connection_open) {

message.str(""); message << "[TEST] polltimeout = " << sock.polltimeout(); logwrite( function, message.str() );
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
          args="";                                     // then the arg list is empty,
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
      std::string retstring="";

      if ( cmd == "help" ) {
                      for ( auto s : FOCUSD_SYNTAX ) { sock.Write( s ); sock.Write( "\n" ); }
      }
      else

      if ( cmd == "exit" ) {
                      this->exit_cleanly();                     // shutdown the daemon
      }
      else

      // isopen
      //
      if ( cmd == FOCUSD_ISOPEN ) {
                      bool isopen = this->interface.isopen( );
                      if ( isopen ) retstring = "true"; else retstring = "false";
                      ret = NO_ERROR;
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
                      ret = this->interface.is_home( retstring );
      }
      else

      // set width and offset
      //
      if ( cmd == FOCUSD_SET ) {
                      ret = this->interface.set( std::ref( this->interface ), args, retstring );
      }
      else

      // get width and offset
      //
      if ( cmd == FOCUSD_GET ) {
                      ret = this->interface.get( retstring );
      }
      else

      // native
      //
      if ( cmd == FOCUSD_NATIVE ) {
                      ret = this->interface.send_command( args, retstring );
      }

      // unknown commands generate an error
      //
      else {
        message.str(""); message << "ERROR: unknown command: " << cmd;
        logwrite( function, message.str() );
        ret = ERROR;
      }

#ifdef LOGLEVEL_DEBUG
      message.str(""); message << "[DEBUG] cmd=" << cmd << " ret=" << ret << " retstring=" << retstring;
      logwrite( function, message.str() );
#endif

      if (ret != NOTHING) {
        if ( not retstring.empty() ) retstring.append( " " );
        std::string term=(ret==NO_ERROR?"DONE\n":"ERROR\n");
        retstring.append( term );
        if ( sock.Write( retstring ) < 0 ) connection_open=false;
      }

      if (!sock.isblocking()) break;       // Non-blocking connection exits immediately.
                                           // Keep blocking connection open for interactive session.
    }

    sock.Close();
    return;
  }
  /***** Server::doit *********************************************************/

}
