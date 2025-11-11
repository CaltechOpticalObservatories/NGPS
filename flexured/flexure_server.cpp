/**
 * @file    flexured.cpp
 * @brief   this is the main flexure daemon
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "flexure_server.h"

namespace Flexure {


  /***** Flexure::Server::exit_cleanly ****************************************/
  /**
   * @brief      shutdown nicely
   *
   */
  void Server::exit_cleanly(void) {
    std::string function = "Flexure::Server::exit_cleanly";
    logwrite( function, "exiting" );

    exit(EXIT_SUCCESS);
  }
  /***** Flexure::Server::exit_cleanly ****************************************/


  /***** Flexure::Server::configure_flexured **********************************/
  /**
   * @brief      read and apply the configuration file for the flexure daemon
   * @return     ERROR or NO_ERROR
   *
   */
  long Server::configure_flexured() {
    std::string function = "Flexure::Server::configure_flexured";
    std::stringstream message;
    int numapplied=0, lastapplied=0;
    long error;

    // Clear the motormap map before loading new information from the config file
    //
    this->interface.motorinterface.clear_motormap();

    // loop through the entries in the configuration file, stored in config class
    //
    for (int entry=0; entry < this->config.n_entries; entry++) {

      lastapplied=numapplied;

      // NBPORT -- nonblocking listening port for the flexure daemon
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
        numapplied++;
      }
      else

      // BLKPORT -- blocking listening port for the flexure daemon
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
        numapplied++;
      }
      else

      // ASYNCPORT -- asynchronous broadcast message port for the flexure daemon
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
        numapplied++;
      }
      else

      // ASYNCGROUP -- asynchronous broadcast group for the flexure daemon
      //
      if ( config.param[entry].find( "ASYNCGROUP" ) == 0 ) {
        this->asyncgroup = config.arg[entry];
        numapplied++;
      }
      else

      // MOTOR_CONTROLLER -- address and name of each PI motor controller in daisy-chain
      //                     Each CONTROLLER is stored in an STL map indexed by motorname
      //
      if ( config.param[entry].find( "MOTOR_CONTROLLER" ) == 0 ) {
        if ( this->interface.motorinterface.load_controller_config( config.arg[entry] ) == NO_ERROR ) {
          numapplied++;
        }
      }
      else

      // PUB_ENDPOINT -- my ZeroMQ socket endpoint for publishing
      //
      if ( config.param[entry] == "PUB_ENDPOINT" ) {
        this->interface.publisher_address = config.arg[entry];
        this->interface.publisher_topic = DAEMON_NAME;   // default publish topic is my name
        numapplied++;
      }
      else

      // SUB_ENDPOINT
      //
      if ( config.param[entry] == "SUB_ENDPOINT" ) {
        this->interface.subscriber_address = config.arg[entry];
        numapplied++;
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
          numapplied++;
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
      else

      // POSITION_COEFFICIENTS
      //
      if ( config.param[entry] == "POSITION_COEFFICIENTS" ) {
        error=interface.compensator.load_vector_from_config(config.arg[entry], VectorType::POSITION_COEFFICIENTS);
        if (error==NO_ERROR) {
          numapplied++;
        }
        else return ERROR;
      }
      else

      // FLEXURE_POLYNOMIALS
      //
      if ( config.param[entry] == "FLEXURE_POLYNOMIALS" ) {
        error=interface.compensator.load_vector_from_config(config.arg[entry], VectorType::FLEXURE_POLYNOMIALS);
        if (error==NO_ERROR) {
          numapplied++;
        }
        else return ERROR;
      }
      else

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
        numapplied++;
      }

      if (numapplied>lastapplied) {
        message.str(""); message << "config:" << config.param[entry] << "=" << config.arg[entry];
        this->interface.async.enqueue_and_log( to_uppercase(DAEMON_NAME), function, message.str() );
      }
    } // end loop through the entries in the configuration file

//logwrite(function, "will start subscriber thread");
//    this->interface.start_subscriber_thread();

    message.str("");
    if (numapplied==0) {
      message << "ERROR: ";
      error = ERROR;
    }
    else {
      error = NO_ERROR;
    }
    message << "applied " << numapplied << " configuration lines to flexured";
    logwrite(function, message.str());

    if ( error == NO_ERROR ) error = this->interface.initialize_class();

    return error;
  }
  /***** Flexure::Server::configure_flexured **********************************/


  /***** Flexure::Server::new_log_day *****************************************/
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
      init_log( logpath, Flexure::DAEMON_NAME );
      // ensure it doesn't immediately re-open
      std::this_thread::sleep_for( std::chrono::seconds(1) );
    }
  }
  /***** Flexure::Server::new_log_day *****************************************/


  /***** Server::block_main ***************************************************/
  /**
   * @brief      main function for blocking connection thread
   * @param[in]  server  reference to Flexure::Server object
   * @param[in]  sock    shared pointer to Network::TcpSocket socket object
   *
   * This function is run in a separate thread spawned by main() and
   * accepts a socket connection and processes the request by
   * calling function Flexure::Server::doit()
   *
   */
  void Server::block_main( Flexure::Server &server, std::shared_ptr<Network::TcpSocket> sock ) {
    server.threads_active.fetch_add(1);  // atomically increment thread_busy counter
    server.doit(*sock);
    sock->Close();
    server.threads_active.fetch_sub(1);  // atomically decrement threads_active counter
    server.id_pool.release_number( sock->id );
    return;
  }
  /***** Server::block_main ***************************************************/


  /***** Server::thread_main **************************************************/
  /**
   * @brief      main function for all non-blocked threads
   * @param[in]  server  reference to Flexure::Server object
   * @param[in]  sock    shared pointer to Network::TcpSocket socket object
   *
   * This function is run in a separate thread spawned by main() and
   * accepts a socket connection and processes the request by
   * calling function Flexure::Server::doit()
   *
   * There are N_THREADS-1 of these, one for each non-blocking connection.
   * These threads never terminate.
   *
   */
  void Server::thread_main( Flexure::Server &server, std::shared_ptr<Network::TcpSocket> sock ) {
    while (1) {
      {
      std::lock_guard<std::mutex> lock(server.conn_mutex);
      sock->Accept();
      }
      server.doit(*sock);
      sock->Close();
    }
    return;
  }
  /***** Server::thread_main **************************************************/


  /***** Server::async_main ***************************************************/
  /**
   * @brief      asynchronous message sending thread
   * @param[in]  flexure  reference to Flexure::Server object
   * @param[in]  sock     Network::UdpSocket socket object
   *
   * This function is run in a separate thread spawned by main() and
   * loops forever, when a message arrives in the status message queue it is
   * sent out via multi-cast UDP datagram.
   *
   */
  void Server::async_main( Flexure::Server &flexure, Network::UdpSocket sock ) {
    std::string function = "Flexure::Server::async_main";
    int retval;

    retval = sock.Create();                                   // create the UDP socket
    if (retval < 0) {
      logwrite(function, "error creating UDP multicast socket for asynchronous messages");
      flexure.exit_cleanly();                                 // do not continue on error
    }
    if (retval==1) {                                          // exit this thread but continue with daemon
      logwrite(function, "asyncrhonous message port disabled by request");
    }

    while (1) {
      std::string message = flexure.interface.async.dequeue();  // get the latest message from the queue (blocks)
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
   * @param[in]  sock  reference to Network::TcpSocket socket object
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
    std::string function = "Flexure::Server::doit";
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

      if ( cmd == "-h" || cmd == "--help" || cmd == "help" || cmd == "?" ) {
                      retstring="flexure { <CMD> } [<ARG>...]\n";
                      retstring.append( "  where <CMD> is one of:\n" );
                      for ( const auto &s : FLEXURED_SYNTAX ) { retstring.append( s ); retstring.append( "\n" ); }
                      ret = HELP;
      }
      else

      if ( cmd == "exit" ) {
                      this->exit_cleanly();                     // shutdown the daemon
      }
      else

      // isopen
      //
      if ( cmd == FLEXURED_ISOPEN ) {
                      ret = this->interface.is_open( args, retstring );
      }
      else

      // open
      //
      if ( cmd == FLEXURED_OPEN ) {
                      ret = this->interface.open();
      }
      else

      // close
      //
      if ( cmd == FLEXURED_CLOSE ) {
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

      // set
      //
      if ( cmd == FLEXURED_SET ) {
                      ret = this->interface.set( args, retstring );
      }
      else

      // get
      //
      if ( cmd == FLEXURED_GET ) {
                      ret = this->interface.get( args, retstring );
      }
      else

      // default positions
      //
      if ( cmd == FLEXURED_DEFAULTPOS ) {
                      ret = this->interface.motorinterface.move_to_default();
      }
      else

      // compensate
      //
      if ( cmd == FLEXURED_COMPENSATE ) {
                      ret = this->interface.compensate( args, retstring );
      }
      else

      // native
      //
      if ( cmd == FLEXURED_NATIVE ) {
                      ret = this->interface.send_command( args, retstring );
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
      if ( cmd == FLEXURED_TEST ) {
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
