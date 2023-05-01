/**
 * @file    telem_server.cpp
 * @brief   these are the main functions used by the Telemetry::Server
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 * This file defines the functions used by the Server class in the Telemetry namespace.
 *
 */

#include "telem_server.h"

namespace Telemetry {

  /***** Telemetry::Server::exit_cleanly **************************************/
  /**
   * @brief      shutdown nicely
   *
   */
  void Server::exit_cleanly(void) {
    std::string function = "Telemetry::Server::exit_cleanly";
    logwrite( function, "exiting" );

    exit(EXIT_SUCCESS);
  }
  /***** Telemetry::Server::exit_cleanly **************************************/


  /***** Telemetry::Server::configure_telem ***********************************/
  /**
   * @brief      read and apply the configuration file
   * @return     ERROR or NO_ERROR
   *
   */
  long Server::configure_telemd() {
    std::string function = "Telemetry::Server::configure_telemd";
    std::stringstream message;
    int applied=0;
    long error;

    // loop through the entries in the configuration file, stored in config class
    //
    for (int entry=0; entry < this->config.n_entries; entry++) {

      // NBPORT -- nonblocking listening port for the telemetry daemon
      //
      if (config.param[entry].compare(0, 6, "NBPORT")==0) {
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
        message.str(""); message << "CONFIG:[" << DAEMON_NAME << "] " << config.param[entry] << "=" << config.arg[entry];
        this->interface.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // BLKPORT -- blocking listening port for the telemetry daemon
      //
      if (config.param[entry].compare(0, 7, "BLKPORT")==0) {
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
        message.str(""); message << "CONFIG:[" << DAEMON_NAME << "] " << config.param[entry] << "=" << config.arg[entry];
        this->interface.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // MESSAGEPORT -- asynchronous broadcast message port for the telemetry daemon
      //
      if (config.param[entry].compare(0, 11, "MESSAGEPORT")==0) {
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
        message.str(""); message << "CONFIG:[" << DAEMON_NAME << "] " << config.param[entry] << "=" << config.arg[entry];
        this->interface.async.enqueue_and_log( function, message.str() );
        applied++;
      }

      // MESSAGEGROUP -- asynchronous broadcast group for the telemetry daemon
      //
      if (config.param[entry].compare(0, 12, "MESSAGEGROUP")==0) {
        this->asyncgroup = config.arg[entry];
        message.str(""); message << "CONFIG:[" << DAEMON_NAME << "] " << config.param[entry] << "=" << config.arg[entry];
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
    message << "applied " << applied << " configuration lines to telemd";
    logwrite(function, message.str());

    return error;
  }
  /***** Telemetry::Server::configure_telem ***********************************/


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
      init_log( logpath, Telemetry::DAEMON_NAME );
    }
  }
  /***** new_log_day **********************************************************/


  /***** block_main ***********************************************************/
  /**
   * @brief      main function for blocking connection thread
   * @param[in]  telem  reference to Telemetry::Server object
   * @param[in]  sock   Network::TcpSocket socket object
   *
   * accepts a socket connection and processes the request by
   * calling function doit()
   *
   * This thread never terminates.
   *
   */
  void Server::block_main( Telemetry::Server &telem, Network::TcpSocket sock ) {
    while(1) {
      sock.Accept();
      telem.doit(sock);             // call function to do the work
      sock.Close();
    }
    return;
  }
  /***** block_main ***********************************************************/


  /***** thread_main **********************************************************/
  /**
   * @brief      main function for all non-blocked threads
   * @param[in]  telem  reference to Telemetry::Server object
   * @param[in]  sock   Network::TcpSocket socket object
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
  void Server::thread_main( Telemetry::Server &telem, Network::TcpSocket sock ) {
    while (1) {
      telem.conn_mutex.lock();
      sock.Accept();
      telem.conn_mutex.unlock();
      telem.doit(sock);          // call function to do the work
      sock.Close();
    }
    return;
  }
  /***** thread_main **********************************************************/


  /***** async_main ***********************************************************/
  /**
   * @brief      asynchronous message sending thread
   * @param[in]  telem  reference to Telemetry::Server object
   * @param[in]  sock   Network::UdpSocket socket object
   *
   * Loops forever, when a message arrives in the status message queue it is
   * sent out via multi-cast UDP datagram.
   *
   */
  void Server::async_main( Telemetry::Server &telem, Network::UdpSocket sock ) {
    std::string function = "Telemetry::Server::async_main";
    int retval;

    retval = sock.Create();                                   // create the UDP socket
    if (retval < 0) {
      logwrite(function, "error creating UDP multicast socket for asynchronous messages");
      telem.exit_cleanly();                                   // do not continue on error
    }
    if (retval==1) {                                          // exit this thread but continue with daemon
      logwrite(function, "asyncrhonous message port disabled by request");
    }

    while (1) {
      std::string message = telem.interface.async.dequeue();  // get the latest message from the queue (blocks)
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
   * Valid commands are listed in telemd_commands.h
   *
   */
  void Server::doit( Network::TcpSocket sock ) {
    std::string function = "Telemetry::Server::doit";
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
        if (ret==0) {               // or a timeout
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
          args="";                                     // then the arg list is empty,
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
      std::string retstring="";

      if ( cmd.compare( "help" ) == 0 ) {
                      for ( auto s : TELEMD_SYNTAX ) { sock.Write( s ); sock.Write( "\n" ); }
      }
      else
      if ( cmd.compare( TELEMD_EXIT ) == 0 ) {
                      this->exit_cleanly();                      // shutdown the daemon
      }
      else

      if ( cmd.compare( TELEMD_ECHO ) == 0 ) {
                      sock.Write( args );
                      sock.Write( "\n" );
      }

      // unknown commands generate an error
      //
      else {
        message.str(""); message << "ERROR: unknown command: " << cmd;
        logwrite( function, message.str() );
        ret = ERROR;
      }

      if (ret != NOTHING) {
        if ( !retstring.empty() ) retstring.append( " " );
        std::string term=(ret==0?"DONE\n":"ERROR\n");
        retstring.append( term );
        if ( sock.Write( retstring ) < 0 ) connection_open=false;
      }

      if (!sock.isblocking()) break;       // Non-blocking connection exits immediately.
                                           // Keep blocking connection open for interactive session.
    }

    sock.Close();
    return;
  }
  /***** doit *****************************************************************/

}
