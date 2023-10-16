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
        catch (std::invalid_argument &) {
          logwrite(function, "ERROR: bad NBPORT: unable to convert to integer");
          return(ERROR);
        }
        catch (std::out_of_range &) {
          logwrite(function, "NBPORT number out of integer range");
          return(ERROR);
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
        catch (std::invalid_argument &) {
          logwrite(function, "ERROR: bad BLKPORT: unable to convert to integer");
          return(ERROR);
        }
        catch (std::out_of_range &) {
          logwrite(function, "BLKPORT number out of integer range");
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
        catch (std::invalid_argument &) {
          logwrite(function, "ERROR: bad ASYNCPORT: unable to convert to integer");
          return(ERROR);
        }
        catch (std::out_of_range &) {
          logwrite(function, "ASYNCPORT number out of integer range");
          return(ERROR);
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

    if ( error == NO_ERROR ) error = this->interface.initialize_class();

    return error;
  }
  /***** Thermal::Server::configure_thermald **********************************/


  /***** Thermal::Server::parse_lks_unit **************************************/
  /**
   * @brief      
   * @return     ERROR or NO_ERROR
   *
   */
  long Server::parse_lks_unit( std::string &input, 
                               int &lksnum, std::string &model, std::string &name, std::string &host, int &port ) {
    std::string function = "Thermal::Server::parse_lks_unit";
    std::stringstream message;
    int applied=0;
    long error;
    std::vector<std::string> tokens;

    Tokenize( input, tokens, " \"" );

    if ( tokens.size() != 5 ) {
      message.str(""); message << "ERROR bad number of tokens: " << tokens.size() << ". expected 5";
      logwrite( function, message.str() );
      return( ERROR );
    }

    try {
      lksnum = std::stoi( tokens.at(0) );
      model  = tokens.at(1);
      name   = tokens.at(2);
      host   = tokens.at(3);
      port   = std::stoi( tokens.at(4) );
    }
    catch ( std::invalid_argument &e ) {
      message.str(""); message << "ERROR loading tokens from input: " << input << ": " << e.what();
      logwrite( function, message.str() );
      return( ERROR );
    }
    catch ( std::out_of_range &e ) {
      message.str(""); message << "ERROR loading tokens from input: " << input << ": " << e.what();
      logwrite( function, message.str() );
      return( ERROR );
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
   * @brief      
   * @return     ERROR or NO_ERROR
   *
   */
  long Server::parse_lks_chan( std::string &input, 
                               int &lksnum, std::string &chan, bool &heater, std::string &label ) {
    std::string function = "Thermal::Server::parse_lks_chan";
    std::stringstream message;
    int applied=0;
    long error;
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
    catch ( std::invalid_argument &e ) {
      message.str(""); message << "ERROR loading tokens from input: " << input << ": " << e.what();
      logwrite( function, message.str() );
      return( ERROR );
    }
    catch ( std::out_of_range &e ) {
      message.str(""); message << "ERROR loading tokens from input: " << input << ": " << e.what();
      logwrite( function, message.str() );
      return( ERROR );
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


  /***** Thermal::Server::configure_lakeshore *********************************/
  /**
   * @brief      read and apply the configuration file for the Lakeshore(s)
   * @details    This function reads and parses keys for LKS_UNIT and LKS_CHAN
   * @return     ERROR or NO_ERROR
   *
   */
  long Server::configure_lakeshore() {
    std::string function = "Thermal::Server::configure_lakeshore";
    std::stringstream message;
    int applied=0;
    long error;

    // loop through the entries in the configuration file, stored in config class
    //
    for (int entry=0; entry < this->config.n_entries; entry++) {

      // LKS_UNIT
      if (config.param[entry].compare(0, 8, "LKS_UNIT")==0) {
        std::string model, name, host;
        int lksnum, port;

        // parse the LKS_UNIT configuration line here
        //
        error = this->parse_lks_unit( config.arg[entry], lksnum, model, name, host, port );

        if ( error == NO_ERROR ) {
          LKS::Interface lks( model, name, host, port );  // temporary LKS communication object initialized
          Thermal::Lakeshore Lakeshore;                   // temporary Thermal::Lakeshore interface object
          Lakeshore.lks = lks;                            // assign the lks communication object

          this->interface.lakeshore.insert( { lksnum, Lakeshore } );  // insert it into the map

          message.str(""); message << "THERMALD:config:" << config.param[entry] << "=" << config.arg[entry];
          logwrite( function, message.str() );
          this->interface.async.enqueue( message.str() );
          applied++;
        }
      }

      // LKS_CHAN
      if (config.param[entry].compare(0, 8, "LKS_CHAN")==0) {
        int lksnum;
        std::string label, chan;
        bool heater;
        if ( this->parse_lks_chan( config.arg[entry], lksnum, chan, heater, label ) == NO_ERROR ) {

          // First, this lksnum must have a matching lksnum in the lakeshore map
          //
          auto lksnum_found = this->interface.lakeshore.find( lksnum );
          if ( lksnum_found != this->interface.lakeshore.end() ) {  // found lksnum in lakeshore map
            if ( heater ) {
              this->interface.lakeshore.at( lksnum ).heaters.push_back( chan );
              this->interface.lakeshore.at( lksnum ).heatlabels.push_back( label );
            }
            else {
              this->interface.lakeshore.at( lksnum ).tempchans.push_back( chan );
              this->interface.lakeshore.at( lksnum ).templabels.push_back( label );
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
          auto name = this->interface.lakeshore.at( lksnum ).lks.get_name();
          auto model = this->interface.lakeshore.at( lksnum ).lks.get_model();
          auto label_found = this->interface.info.find( label );
          if ( label_found != this->interface.info.end() ) {  // found label in info map
            message.str(""); message << "ERROR: LKS unit " << lksnum << " duplicate label \"" << label
                                     << "\" already exists for unit " << this->interface.info[label].unit;
            this->interface.async.enqueue_and_log( function, message.str() );
            error = ERROR;
            continue;
          }
          else {
            this->interface.info[ label ] = { "LKS", lksnum, model, name, chan, label };
          }
        }
        message.str(""); message << "THERMALD:config:" << config.param[entry] << "=" << config.arg[entry];
        logwrite( function, message.str() );
        this->interface.async.enqueue( message.str() );
        applied++;
      }

    }  // end for entry=0; entry < n_entries

    return( NO_ERROR );
  }
  /***** Thermal::Server::configure_lakeshore *********************************/


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

      if ( cmd.compare( "help" ) == 0 || cmd.compare( "?" ) == 0 ) {
                      for ( auto s : THERMALD_SYNTAX ) { sock.Write( s ); sock.Write( "\n" ); }
      }
      else
      if ( cmd.compare( THERMALD_EXIT ) == 0 ) {
                      this->exit_cleanly();                     // shutdown the daemon
      }
      else

      // ECHO
      //
      if ( cmd.compare( THERMALD_ECHO ) == 0 ) {
                      if ( args == "?" ) sock.Write( THERMALD_ECHO
                                                     +" <string>\n  server receives and writes back <string> to the client.\n"
                                                     +"  Used to test if the server is responsive.\n" );
                      else {
                        sock.Write( args );
                        sock.Write( "\n" );
                      }
      }
      else

      // get
      //
      if ( cmd.compare( THERMALD_GET ) == 0 ) {
                      ret = this->interface.get( args, retstring );
      }
      else

      // LOGALL
      //
      if ( cmd.compare( THERMALD_LOGALL ) == 0 ) {
                      ret = this->interface.log_all( args, retstring );
      }
      else

      // native
      //
      if ( cmd.compare( THERMALD_NATIVE ) == 0 ) {
                      ret = this->interface.native( args, retstring );
      }
      else

      // READALL
      //
      if ( cmd.compare( THERMALD_READALL ) == 0 ) {
                      ret = this->interface.read_all( args, retstring );
      }
      else

      // setpoint
      //
      if ( cmd.compare( THERMALD_SETPOINT ) == 0 ) {
                      ret = this->interface.setpoint( args, retstring );
      }

      // unknown commands generate an error
      //
      else {
        message.str(""); message << "ERROR: unknown command: " << cmd;
        logwrite( function, message.str() );
        ret = ERROR;
      }

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
