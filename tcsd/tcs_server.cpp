/**
 * @file    tcs_server.cpp
 * @brief   these are the main functions used by the TCS::Server
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "tcs_server.h"

namespace TCS {


  /***** TCS::Server::exit_cleanly ********************************************/
  /**
   * @brief      shutdown nicely
   *
   */
  void Server::exit_cleanly(void) {
    std::string function = "TCS::Server::exit_cleanly";
    logwrite( function, "exiting" );

    exit(EXIT_SUCCESS);
  }
  /***** TCS::Server::exit_cleanly ********************************************/


  /***** TCS::Server::load_tcs_info *******************************************/
  /**
   * @brief      load tcs host info from config file
   * @details    this will also construct a map of TcsIO objects
   * @param[in]  input  input string expected to contain "name host port"
   * @return     ERROR or NO_ERROR
   *
   */
  long Server::load_tcs_info( std::string input ) {
    std::string function = "TCS::Server::load_tcs_info";
    std::stringstream message;
    std::vector<std::string> tokens;
    std::string tryname, tryhost;
    int tryport=-1;

    // Extract the name, host and port from the input string
    //
    Tokenize( input, tokens, " \"" );

    if ( tokens.size() != 3 ) {
      message.str(""); message << "ERROR bad number of parameters in \"" << input << "\": expected 3 but received " << tokens.size();
      logwrite( function, message.str() );
      return ERROR;
    }

    try {
      tryname = tokens.at(0);
      tryhost = tokens.at(1);
      tryport = std::stoi( tokens.at(2) );
    }
    catch ( std::invalid_argument &e ) {
      message.str(""); message << "ERROR loading tokens from input: " << input << ": " << e.what();
      logwrite( function, message.str() );
      return ERROR;
    }
    catch ( std::out_of_range &e ) {
      message.str(""); message << "ERROR loading tokens from input: " << input << ": " << e.what();
      logwrite( function, message.str() );
      return ERROR;
    }

    // Check that (potentially) valid values have been extracted
    //
    if ( tryport < 1 ) {
      message.str(""); message << "ERROR port " << tryport << " must be greater than 0";
      logwrite( function, message.str() );
      return ERROR;
    }
    if ( tryname.empty() ) {
      logwrite( function, "ERROR name cannot be empty" );
      return ERROR;
    }
    if ( tryhost.empty() ) {
      logwrite( function, "ERROR host cannot be empty" );
      return ERROR;
    }

    // Insert a new element into the tcsmap with the name loaded from the config file,
    // constructing a TCS::TcsIO object at the same time, using the loaded {name,host,port}
    // and the terminating characters for TCS writes (\r) and reads (\0).
    //
    this->interface.tcsmap.emplace( tryname, 
                                    TCS::TcsIO{std::make_unique<Network::Interface>(tryname, tryhost, tryport, '\r', '\0')});

    return NO_ERROR;
  }
  /***** TCS::Server::load_tcs_info *******************************************/


  /***** TCS::Server::configure_tcsd ******************************************/
  /**
   * @brief      read and apply the configuration file for the tcs daemon
   * @return     ERROR or NO_ERROR
   *
   */
  long Server::configure_tcsd() {
    std::string function = "TCS::Server::configure_tcsd";
    std::stringstream message;
    int applied=0;
    long error;

    // loop through the entries in the configuration file, stored in config class
    //
    for (int entry=0; entry < this->config.n_entries; entry++) {

      // TCS_HOST -- contains "name host port" for the TCS
      //
      if ( config.param[entry].compare( 0, 8, "TCS_HOST" ) == 0 ) {
        message.str(""); message << "TCSD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->interface.async.enqueue_and_log( function, message.str() );
        this->load_tcs_info( config.arg[entry] );
        applied++;
      }

      // NBPORT -- nonblocking listening port for the tcs daemon
      //
      if (config.param[entry].compare(0, 6, "NBPORT")==0) {
        int port;
        try {
          port = std::stoi( config.arg[entry] );
        }
        catch (std::invalid_argument &) {
          logwrite(function, "ERROR: bad NBPORT: unable to convert to integer");
          return ERROR;
        }
        catch (std::out_of_range &) {
          logwrite(function, "NBPORT number out of integer range");
          return ERROR;
        }
        this->nbport = port;
        message.str(""); message << "TCSD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->interface.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // BLKPORT -- blocking listening port for the tcs daemon
      //
      if (config.param[entry].compare(0, 7, "BLKPORT")==0) {
        int port;
        try {
          port = std::stoi( config.arg[entry] );
        }
        catch (std::invalid_argument &) {
          logwrite(function, "ERROR: bad BLKPORT: unable to convert to integer");
          return ERROR;
        }
        catch (std::out_of_range &) {
          logwrite(function, "BLKPORT number out of integer range");
          return ERROR;
        }
        this->blkport = port;
        message.str(""); message << "TCSD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->interface.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // ASYNCPORT -- asynchronous broadcast message port for the tcs daemon
      //
      if (config.param[entry].compare(0, 9, "ASYNCPORT")==0) {
        int port;
        try {
          port = std::stoi( config.arg[entry] );
        }
        catch (std::invalid_argument &) {
          logwrite(function, "ERROR: bad ASYNCPORT: unable to convert to integer");
          return ERROR;
        }
        catch (std::out_of_range &) {
          logwrite(function, "ASYNCPORT number out of integer range");
          return ERROR;
        }
        this->asyncport = port;
        message.str(""); message << "TCSD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->interface.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // ASYNCGROUP -- asynchronous broadcast group for the tcs daemon
      //
      if (config.param[entry].compare(0, 10, "ASYNCGROUP")==0) {
        this->asyncgroup = config.arg[entry];
        message.str(""); message << "TCSD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->interface.async.enqueue_and_log( function, message.str() );
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
    message << "applied " << applied << " configuration lines to tcsd";
    logwrite(function, message.str());

    return error;
  }
  /***** TCS::Server::configure_tcsd ******************************************/


  /***** TCS::Server::configure_interface *************************************/
  /**
   * @brief      read and apply the configuration file for the tcs interface
   * @return     ERROR | NO_ERROR
   *
   */
  long Server::configure_interface() {
    std::string function = "TCS::Server::configure_interface";
    std::stringstream message;
    int applied=0;
    long error;

    // loop through the entries in the configuration file, stored in config class
    //
    for (int entry=0; entry < this->config.n_entries; entry++) {

      // TCS_OFFSET_RATE_RA -- offset rate for RA ("MRATE") in arcsec per second
      //
      if ( config.param[entry] == "TCS_OFFSET_RATE_RA" ) {
        int rate;
        try {
          rate = std::stoi( config.arg[entry] );
          if ( rate < 1 || rate > 50 ) {
            message.str(""); message << "ERROR TCS_OFFSET_RATE_RA " << rate << " outside range { 1 : 50 }";
            logwrite( function, message.str() );
            return ERROR;
          }
        }
        catch ( std::invalid_argument &e ) {
          message.str(""); message << "ERROR invalid TCS_OFFSET_RATE_RA: " << e.what();
          logwrite( function, message.str() );
          return ERROR;
        }
        catch ( std::out_of_range &e ) {
          message.str(""); message << "ERROR invalid TCS_OFFSET_RATE_RA: " << e.what();
          logwrite( function, message.str() );
          return ERROR;
        }
        message.str(""); message << "TCSD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->interface.async.enqueue_and_log( function, message.str() );
        this->interface.offsetrate_ra = rate;
        applied++;
      }

      // TCS_OFFSET_RATE_DEC -- offset rate for RA ("MRATE") in arcsec per second
      //
      if ( config.param[entry] == "TCS_OFFSET_RATE_DEC" ) {
        int rate;
        try {
          rate = std::stoi( config.arg[entry] );
          if ( rate < 1 || rate > 50 ) {
            message.str(""); message << "ERROR TCS_OFFSET_RATE_DEC " << rate << " outside range { 1 : 50 }";
            logwrite( function, message.str() );
            return ERROR;
          }
        }
        catch ( std::invalid_argument &e ) {
          message.str(""); message << "ERROR invalid TCS_OFFSET_RATE_DEC: " << e.what();
          logwrite( function, message.str() );
          return ERROR;
        }
        catch ( std::out_of_range &e ) {
          message.str(""); message << "ERROR invalid TCS_OFFSET_RATE_DEC: " << e.what();
          logwrite( function, message.str() );
          return ERROR;
        }
        message.str(""); message << "TCSD:config:" << config.param[entry] << "=" << config.arg[entry];
        this->interface.async.enqueue_and_log( function, message.str() );
        this->interface.offsetrate_dec = rate;
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
    message << "applied " << applied << " configuration lines to tcs interface";
    logwrite(function, message.str());

    return error;

  }
  /***** TCS::Server::configure_interface *************************************/


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
      init_log( logpath, TCS::DAEMON_NAME );
    }
  }
  /***** Server::new_log_day **************************************************/


  /***** Server::block_main ***************************************************/
  /**
   * @brief      main function for blocking connection thread
   * @param[in]  server  reference to TCS::Server object
   * @param[in]  sock    Network::TcpSocket socket object
   *
   * This function is run in a separate thread spawned by main() and
   * accepts a socket connection and processes the request by
   * calling function TCS::Server::doit()
   *
   * This thread never terminates.
   *
   */
  void Server::block_main( TCS::Server &server, std::shared_ptr<Network::TcpSocket> sock ) {
    server.threads_active.fetch_add(1);  // atomically increment threads_busy counter
    server.doit(*sock);
    sock->Close();
    server.threads_active.fetch_sub(1);  // atomically increment threads_busy counter
    server.id_pool.release_number( sock->id );
    return;
  }
  /***** Server::block_main ***************************************************/


  /***** Server::thread_main **************************************************/
  /**
   * @brief      main function for all non-blocked threads
   * @param[in]  server  reference to TCS::Server object
   * @param[in]  sock    Network::TcpSocket socket object
   *
   * This function is run in a separate thread spawned by main() and
   * accepts a socket connection and processes the request by
   * calling function TCS::Server::doit()
   *
   * There are N_THREADS-1 of these, one for each non-blocking connection.
   * These threads never terminate.
   *
   * This function differs from block_main only in that the call to Accept
   * is mutex-protected.
   *
   */
  void Server::thread_main( TCS::Server &server, std::shared_ptr<Network::TcpSocket> sock ) {
    while (1) {
      {
      std::lock_guard<std::mutex> lock(server.conn_mutex);
      sock->Accept();
      }
      server.doit(*sock);                // call function to do the work
      sock->Close();
    }
    return;
  }
  /***** Server::thread_main **************************************************/


  /***** Server::handle_new_connection ****************************************/
  /**
   * @brief      handles extra connections if main threads are busy
   * @param[in]  server  reference to TCS::Server object
   * @param[in]  sock    shared pointer to Network::TcpSocket socket object
   *
   */
  void Server::handle_new_connection( TCS::Server &server, std::shared_ptr<Network::TcpSocket> sock ) {
    server.doit(*sock);                  // call function to do the work
    sock->Close();
    server.remove_socket( sock->id );    // this socket won't be needed again
    return;
  }
  /***** Server::handle_new_connection ****************************************/


  /***** Server::async_main ***************************************************/
  /**
   * @brief      asynchronous message sending thread
   * @param[in]  server  reference to TCS::Server object
   * @param[in]  sock    Network::UdpSocket socket object
   *
   * This function is run in a separate thread spawned by main() and
   * loops forever, when a message arrives in the status message queue it is
   * sent out via multi-cast UDP datagram.
   *
   */
  void Server::async_main( TCS::Server &server, Network::UdpSocket sock ) {
    std::string function = "TCS::Server::async_main";
    int retval;

    retval = sock.Create();                                   // create the UDP socket
    if (retval < 0) {
      logwrite(function, "error creating UDP multicast socket for asynchronous messages");
      server.exit_cleanly();                                  // do not continue on error
    }
    if (retval==1) {                                          // exit this thread but continue with daemon
      logwrite(function, "asyncrhonous message port disabled by request");
    }

    while (1) {
      std::string message = server.interface.async.dequeue(); // get the latest message from the queue (blocks)
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
  void Server::doit(Network::TcpSocket &sock) {
    std::string function = "TCS::Server::doit";
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

      bool polling = false;

      try {
        std::size_t cmd_sep = buf.find_first_of(" ");  // find the first space, which separates command from argument list

        cmd = buf.substr(0, cmd_sep);                  // cmd is everything up until that space

        // If cmd is "poll" then set a polling flag to indicate not to log incoming command.
        // Then shift the buf to the next part of the string after "poll" and look again
        // for a command.
        //
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

        ++this->cmd_num;
        if ( this->cmd_num == INT_MAX ) this->cmd_num = 0;

        if ( polling ) {
          args="poll";
        }
        else {
          message.str(""); message << "received command on fd " << sock.getfd() << " (" << this->cmd_num << "): " << cmd << " " << args;
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

      /**
       * process commands here
       */
      ret = NOTHING;
      std::string retstring;

      if ( cmd.compare( "help" ) == 0 || cmd.compare( "?" ) == 0 ) {
                      for ( const auto &s : TCSD_SYNTAX ) { retstring.append( s ); retstring.append( "\n" ); }
                      ret = HELP;
      }
      else
      if ( cmd.compare( TCSD_EXIT ) == 0 ) {
                      this->exit_cleanly();                     // shutdown the daemon
      }
      else

      // open
      //
      if ( cmd.compare( TCSD_OPEN ) == 0 ) {
                      ret = this->interface.open( args, retstring );
      }
      else

      // close
      //
      if ( cmd.compare( TCSD_CLOSE ) == 0 ) {
                      ret = this->interface.close();
      }
      else

      // list
      //
      if ( cmd.compare( TCSD_LIST ) == 0 ) {
                      ret = this->interface.list( args, retstring );
      }
      else

      // llist
      //
      if ( cmd.compare( TCSD_LLIST ) == 0 ) {
                      ret = this->interface.llist( args, retstring );
      }
      else

      // isopen
      //
      if ( cmd.compare( TCSD_ISOPEN ) == 0 ) {
                      ret = this->interface.isopen( args, retstring );
      }
      else

      // getname
      //
      if ( cmd.compare( TCSD_GET_NAME ) == 0 ) {
                      ret = this->interface.get_name( args, retstring );
      }
      else

      if ( cmd.compare( TCSD_GET_CASS ) == 0 ) {
                      ret = this->interface.get_cass( args, retstring );
      }
      else

      if ( cmd.compare( TCSD_GET_COORDS ) == 0 ) {
                      ret = this->interface.get_coords( args, retstring );
      }
      else

      if ( cmd.compare( TCSD_GET_DOME ) == 0 ) {
                      ret = this->interface.get_dome( args, retstring );
      }
      else

      if ( cmd.compare( TCSD_GET_FOCUS ) == 0 ) {
                      ret = this->interface.get_focus( args, retstring );
      }
      else

      if ( cmd.compare( TCSD_GET_OFFSETS ) == 0 ) {
                      ret = this->interface.get_offsets( args, retstring );
      }
      else

      if ( cmd.compare( TCSD_SET_FOCUS ) == 0 ) {
                      ret = this->interface.set_focus( args, retstring );
      }
      else

      if ( cmd.compare( TCSD_OFFSETRATE ) == 0 ) {
                      ret = this->interface.offsetrate( args, retstring );
      }
      else

      if ( cmd.compare( TCSD_WEATHER_COORDS ) == 0 ) {
                      ret = this->interface.get_weather_coords( args, retstring );
      }
      else

      if ( cmd.compare( TCSD_GET_MOTION ) == 0 ) {
                      ret = this->interface.get_motion( args, retstring );
      }
      else

      if ( caseCompareString( cmd, TCSD_RETOFFSETS ) ) {
                      ret = this->interface.ret_offsets( args, retstring );
      }
      else

      if ( caseCompareString( cmd, TCSD_RINGGO ) ) {
                      ret = this->interface.ringgo( args, retstring );
      }
      else

      if ( caseCompareString( cmd, TCSD_COORDS ) ) {
                      ret = this->interface.coords( args, retstring );
      }
      else

      if ( caseCompareString( cmd, TELEMREQUEST ) ) {
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

      if ( caseCompareString( cmd, TCSD_PTOFFSET ) ) {
                      ret = this->interface.pt_offset( args, retstring );
      }
      else

      if ( caseCompareString( cmd, TCSD_NATIVE ) ) {
                      ret = this->interface.native( args, retstring );
      }
      // Unknown commands generate an error
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
        if ( ! retstring.empty() && ret != HELP && !polling ) {
          retstring.append( "\n" );
          message.str(""); message << "command (" << this->cmd_num << ") reply: " << retstring;
          logwrite( function, message.str() );
        }

        if ( polling ) retstring.append("\n");

        if ( sock.Write( retstring ) < 0 ) connection_open=false;
      }

      if (!sock.isblocking()) break;       // Non-blocking connection exits immediately.
                                           // Keep blocking connection open for interactive session.
    }

    return;
  }
  /***** Server::doit *********************************************************/

}
