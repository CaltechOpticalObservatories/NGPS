/**
 * @file    power_server.cpp
 * @brief   this is the main power server
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "power_server.h"

namespace Power {

  Server* Server::instance = nullptr;

  /***** Power::Server::handle_signal *****************************************/
  /**
   * @brief      handles ctrl-C and other signals
   * @param[in]  int signo
   *
   */
  void Server::handle_signal(int signo) {
    std::string function = "Power::Server::handle_signal";
    std::stringstream message;

    switch (signo) {
      case SIGTERM:
      case SIGINT:
        logwrite(function, "received termination signal");
        message << "NOTICE:" << Power::DAEMON_NAME << " exit";
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
        message.str(""); message << "NOTICE:" << Power::DAEMON_NAME << " exit";
        Server::instance->interface.async.enqueue( message.str() );
        break;
    }
    return;
  }
  /***** Power::Server::handle_signal *****************************************/


  /***** Power::Server::exit_cleanly ******************************************/
  /**
   * @brief      shutdown nicely
   *
   */
  void Server::exit_cleanly(void) {
    Server::instance->interface.stop_subscriber_thread();
    logwrite( "Power::Server::exit_cleanly", "exiting" );
    close_log();                        // flush and close logfile stream
    _exit(EXIT_SUCCESS);
  }
  /***** Power::Server::exit_cleanly ******************************************/


  /***** Power::Server::configure_powerd **************************************/
  /**
   * @brief      read and apply the configuration file for the power daemon
   * @return     ERROR or NO_ERROR
   *
   */
  long Server::configure_powerd() {
    const std::string function("Power::Server::configure_powerd");
    std::stringstream message;
    int applied=0;
    long error;

    // loop through the entries in the configuration file, stored in config class
    //
    for (int entry=0; entry < this->config.n_entries; entry++) {

      // NBPORT -- nonblocking listening port for the power daemon
      //
      if ( config.param[entry] == "NBPORT" ) {
        int port;
        try {
          port = std::stoi( config.arg[entry] );
        }
        catch ( const std::exception &e ) {
          logwrite(function, "ERROR parsing NBPORT: "+std::string(e.what()));
          return ERROR;
        }
        this->nbport = port;
        message.str(""); message << "POWERD:config:" << config.param[entry] << "=" << config.arg[entry];
        logwrite( function, message.str() );
        this->interface.async.enqueue( message.str() );
        applied++;
      }

      // BLKPORT -- blocking listening port for the power daemon
      //
      if ( config.param[entry]=="BLKPORT" ) {
        int port;
        try {
          port = std::stoi( config.arg[entry] );
        }
        catch ( const std::exception &e ) {
          logwrite(function, "ERROR parsing BLKPORT: "+std::string(e.what()));
          return ERROR;
        }
        this->blkport = port;
        message.str(""); message << "POWERD:config:" << config.param[entry] << "=" << config.arg[entry];
        logwrite( function, message.str() );
        this->interface.async.enqueue( message.str() );
        applied++;
      }

      // ASYNCPORT -- asynchronous broadcast message port for the power daemon
      //
      if ( config.param[entry]=="ASYNCPORT" ) {
        int port;
        try {
          port = std::stoi( config.arg[entry] );
        }
        catch (const std::invalid_argument &e) {
          logwrite(function, "ERROR parsing ASYNCPORT: "+std::string(e.what()));
          return ERROR;
        }
        this->asyncport = port;
        message.str(""); message << "POWERD:config:" << config.param[entry] << "=" << config.arg[entry];
        logwrite( function, message.str() );
        this->interface.async.enqueue( message.str() );
        applied++;
      }

      // ASYNCGROUP -- asynchronous broadcast group for the power daemon
      //
      if ( config.param[entry]=="ASYNCGROUP" ) {
        this->asyncgroup = config.arg[entry];
        message.str(""); message << "POWERD:config:" << config.param[entry] << "=" << config.arg[entry];
        logwrite( function, message.str() );
        this->interface.async.enqueue( message.str() );
        applied++;
      }

      // PUB_ENDPOINT -- my ZeroMQ socket endpoint for publishing
      //
      if ( config.param[entry] == "PUB_ENDPOINT" ) {
        this->interface.publisher_address = config.arg[entry];
        this->interface.publisher_topic = DAEMON_NAME;   // default publish topic is my name
        this->interface.async.enqueue_and_log( function, "POWERD:config:"+config.param[entry]+"="+config.arg[entry] );
        applied++;
      }

      // SUB_ENDPOINT
      //
      if ( config.param[entry] == "SUB_ENDPOINT" ) {
        this->interface.subscriber_address = config.arg[entry];
        this->interface.async.enqueue_and_log( function, "POWERD:config:"+config.param[entry]+"="+config.arg[entry] );
        applied++;
      }

      // OPEN_ON_START
      if (config.param[entry]=="OPEN_ON_START") {
        if ( !config.arg[entry].empty() && config.arg[entry]=="yes" ) {
          this->open_on_start = true;
        }
        else {
          this->open_on_start = false;
        }
        message.str(""); message << "POWERD:config:" << config.param[entry] << "=" << config.arg[entry];
        logwrite( function, message.str() );
        this->interface.async.enqueue( message.str() );
        applied++;
      }

      // NPS_UNIT
      if (config.param[entry]=="NPS_UNIT") {
        // Create a local NpsInfo object for checking the config file input.
        //
        Power::NpsInfo npsinfo;

        // The nps_info map is indexed by nps number.
        // Pass this variable by reference to the load_nps_info() function, which will
        // set it to a valid value for insertion into the nps_info map.
        //
        int npsnum=-1;

        if ( npsinfo.load_nps_info( config.arg[entry], npsnum ) == NO_ERROR ) {
          this->interface.nps_info.insert( { npsnum, npsinfo } );  // insert this into the nps_info map
          this->interface.configure_interface( npsinfo );          // create an interface to the NPS described by this object
          message.str(""); message << "POWERD:config:" << config.param[entry] << "=" << config.arg[entry];
          logwrite( function, message.str() );
          this->interface.async.enqueue( message.str() );
          applied++;
        }
      }

      // NPS_PLUG
      if (config.param[entry]=="NPS_PLUG") {
        // Create a local NpsInfo object for checking the config file input.
        //
        Power::NpsInfo npsinfo;

        // The following variables (plugname, npsnum, plugnum) are extracted from this config file
        // by the load_plug_info() function.
        //
        std::stringstream plugid;
        std::string plugname;
        int         npsnum;
        int         plugnum;

        if ( npsinfo.load_plug_info( config.arg[entry], npsnum, plugnum, plugname ) == NO_ERROR ) {

          // load_plug_info() cannot check for maxplugs so check for that now
          //
          int maxplugs=-1;

          // Make sure the plug's npsnum has a matching NPS unit definition in the interface's nps_info map.
          //
          auto loc = this->interface.nps_info.find( npsnum );      // find this plug's npsnum in the nps_info map.

          if ( loc != this->interface.nps_info.end() ) {           // found
            maxplugs = loc->second.maxplugs;                       // here's the maxplugs for this unit
          }
          else {                                                   // else not found
            message.str(""); message << "ERROR plug " << plugnum << " " << plugname
                                     << " has no matching NPS unit number " << npsnum << " defined by NPS_UNIT";
            logwrite( function, message.str() );
            error = ERROR;
            break;
          }

          if ( plugnum > maxplugs ) {
            message.str(""); message << "ERROR bad plug number " << plugnum << " for " << plugname
                                     << " on unit " << npsnum << ": must be < " << maxplugs;
            logwrite( function, message.str() );
            error = ERROR;
            break;
          }

          // This plug passed all the tests so insert the plug into the plugmap.
          // This map provides for locating the NPS Unit and Plug by only the plug name.
          //
          this->interface.plugmap.insert( { plugname, { npsnum, plugnum } } );

          // plugname is an STL map of plug names indexed by plugid which
          // is a unique ID string consisting of "npsnum plugnum". This enables
          // lookup of the plugname for any given nps and plug.
          //
          plugid << npsnum << " " << plugnum;
          this->interface.plugname.insert( { plugid.str(), plugname } );

          message.str(""); message << "POWERD:config:" << config.param[entry] << "=" << config.arg[entry];
          logwrite( function, message.str() );
          this->interface.async.enqueue( message.str() );
          applied++;
        }
        else error = ERROR;
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
    message << "applied " << applied << " configuration lines to powerd";
    logwrite(function, message.str());

    return error;
  }
  /***** Power::Server::configure_powerd **************************************/


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
   * @param[in]  power  reference to Power::Server object
   * @param[in]  sock   Network::TcpSocket socket object
   *
   * This function is run in a separate thread spawned by main() and
   * accepts a socket connection and processes the request by
   * calling function Power::Server::doit()
   *
   * This thread never terminates.
   *
   */
  void Server::block_main( Power::Server &power, Network::TcpSocket sock ) {
    while( true ) {
      sock.Accept();
      power.doit(sock);             // call function to do the work
      sock.Close();
    }
    return;
  }
  /***** Server::block_main ***************************************************/


  /***** Server::thread_main **************************************************/
  /**
   * @brief      main function for all non-blocked threads
   * @param[in]  power  reference to Power::Server object
   * @param[in]  sock   Network::TcpSocket socket object
   *
   * This function is run in a separate thread spawned by main() and
   * accepts a socket connection and processes the request by
   * calling function Power::Server::doit()
   *
   * There are N_THREADS-1 of these, one for each non-blocking connection.
   * These threads never terminate.
   *
   * This function differs from block_main only in that the call to Accept
   * is mutex-protected.
   *
   */
  void Server::thread_main( Power::Server &power, Network::TcpSocket sock ) {
    while ( true ) {
      power.conn_mutex.lock();
      sock.Accept();
      power.conn_mutex.unlock();
      power.doit(sock);          // call function to do the work
      sock.Close();
    }
    return;
  }
  /***** Server::thread_main **************************************************/


  /***** Server::async_main ***************************************************/
  /**
   * @brief      asynchronous message sending thread
   * @param[in]  power  reference to Power::Server object
   * @param[in]  sock   Network::UdpSocket socket object
   *
   * This function is run in a separate thread spawned by main() and
   * loops forever, when a message arrives in the status message queue it is
   * sent out via multi-cast UDP datagram.
   *
   */
  void Server::async_main( Power::Server &power, Network::UdpSocket sock ) {
    std::string function = "Power::Server::async_main";
    int retval;

    retval = sock.Create();                                   // create the UDP socket
    if (retval < 0) {
      logwrite(function, "error creating UDP multicast socket for asynchronous messages");
      power.exit_cleanly();                                   // do not continue on error
    }
    if (retval==1) {                                          // exit this thread but continue with daemon
      logwrite(function, "asyncrhonous message port disabled by request");
    }

    while (1) {
      std::string message = power.interface.async.dequeue();  // get the latest message from the queue (blocks)
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
    std::string function = "Power::Server::doit";
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

      if ( cmd == "help" || cmd == "?" || cmd == "-h" ) {
                      for ( const auto &s : POWERD_SYNTAX ) { retstring.append( s ); retstring.append( "\n" ); }
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
      if ( cmd == POWERD_ISOPEN ) {
                    bool isopen = this->interface.isopen( );
                    if ( isopen ) retstring = "true"; else retstring = "false";
                    ret = NO_ERROR;
      }
      else

      // open
      //
      if ( cmd == POWERD_OPEN ) {
                    ret = this->interface.open( );
                    retstring = this->interface.missing;
      }
      else

      // close
      //
      if ( cmd == POWERD_CLOSE ) {
                    ret = this->interface.close( );
      }
      else

      // reopen
      //
      if ( cmd == POWERD_REOPEN ) {
                    if ( args=="?" ) { sock.Write( POWERD_REOPEN+"\n  closes, reads cfg file, then re-opens connections to hardware\n" ); }
                    else {
                      ret  = this->interface.close();
                      usleep( 100000 );
                      ret |= this->configure_powerd();
                      ret |= this->interface.open();
                      retstring = this->interface.missing;
                    }
      }
      else

      // list devices
      //
      if ( cmd == POWERD_LIST ) {
                    this->interface.list( args, retstring );
                    retstring.append( " DONE\n" );
                    if ( sock.Write( retstring ) < 0 ) connection_open=false;
      }
      else

      // power status
      //
      if ( cmd == POWERD_STATUS ) {
                    ret = this->interface.status( args, retstring );
                    if ( ret==NO_ERROR ) {
                      ret=NOTHING;
                      if ( sock.Write( retstring ) < 0 ) connection_open=false;
                    }
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

      // all other commands go to the powerd interface for parsing
      //
      else {
                      try {
                        std::transform( buf.begin(), buf.end(), buf.begin(), ::toupper );   // make uppercase
                      }
                      catch( ...) {
                        logwrite( function, "ERROR converting command to uppercase" );
                        ret = ERROR;
                      }
                      ret = this->interface.command( buf, retstring );                      // send the command
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
