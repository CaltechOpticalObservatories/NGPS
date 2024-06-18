/**
 * @file    emulatord_power.cpp
 * @brief   this is the power emulator daemon
 * @details 
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "emulatord_power.h"
#include "daemonize.h"

PowerEmulator::Server emulator;

/***** signal_handler ********************************************************/
/**
 * @fn         signal_handler
 * @brief      handles ctrl-C
 * @param[in]  int signo
 * @return     nothing
 *
 */
void signal_handler( int signo ) {
  std::string function = "  (PowerEmulator::signal_handler) ";
  switch ( signo ) {
    case SIGTERM:
    case SIGINT:
      std::cerr << get_timestamp() << function << emulator.subsystem << " received termination signal\n";
      emulator.exit_cleanly();                   // shutdown the daemon
      break;
    case SIGHUP:
      std::cerr << get_timestamp() << function << emulator.subsystem << " caught SIGHUP\n";
      emulator.configure_emulator();             // TODO can (/should) this be done while running?
      break;
    case SIGPIPE:
      std::cerr << get_timestamp() << function << emulator.subsystem << " caught SIGPIPE\n";
      break;
    default:
      std::cerr << get_timestamp() << function << emulator.subsystem << " received unknown signal\n";
      emulator.exit_cleanly();                   // shutdown the daemon
      break;
  }
  return;
}
/***** signal_handler ********************************************************/


int  main( int argc, char **argv );                 // main thread (just gets things started)
void block_main( Network::TcpSocket sock );         // this thread handles requests on blocking port
void doit( Network::TcpSocket sock );               // the worker thread


/***** main ******************************************************************/
/**
 * @fn         main
 * @brief      the main function
 * @param[in]  int argc, char** argv
 * @return     0
 *
 */
int main( int argc, char **argv ) {
  std::string function = "  (PowerEmulator::main) ";
  std::stringstream message;
  long ret=NO_ERROR;
  std::string daemon_in;     // daemon setting read from config file
  bool start_daemon = false; // don't start as daemon unless specifically requested

  // capture these signals
  //
  signal(SIGINT, signal_handler);
  signal(SIGPIPE, signal_handler);
  signal(SIGHUP, signal_handler);

  // check for "-f <filename>" command line option to specify config file
  //
  if ( cmdOptionExists( argv, argv+argc, "-f" ) ) {
    char* filename = getCmdOption( argv, argv+argc, "-f" );
    if ( filename ) {
      emulator.config.filename = std::string( filename );
    }
  }
  else

  // if no "-f <filename>" then as long as there's at least one arg,
  // assume that is the config file name.
  //
  if (argc>1) {
    emulator.config.filename = std::string( argv[1] );
  }
  else {
    std::cerr << get_timestamp() << function << emulator.subsystem
              << " ERROR: no configuration file specified\n";
    emulator.exit_cleanly();
  }

  if ( emulator.config.read_config(emulator.config) != NO_ERROR) {    // read configuration file specified on command line
    std::cerr << get_timestamp() << function << emulator.subsystem
              << " ERROR: unable to configure system\n";
    emulator.exit_cleanly();
  }

  for (int entry=0; entry < emulator.config.n_entries; entry++) {

    std::string configkey;          // the current configuration key read from file
    std::string configval;          // the current configuration value read from file
    
    try {
      configkey = emulator.config.param.at(entry);
      configval = emulator.config.arg.at(entry);
    }
    catch (std::out_of_range &) {   // should be impossible
      message.str(""); message << "ERROR: entry " << entry
                               << " out of range in config parameters (" << emulator.config.n_entries << ")";
      logwrite( function, message.str() );
      return(ERROR);
    }

    if ( configkey == "DAEMON")  daemon_in = configval;

  }

  if ( !daemon_in.empty() && daemon_in == "yes" ) start_daemon = true;
  else
  if ( !daemon_in.empty() && daemon_in == "no"  ) start_daemon = false;
  else {
    std::cerr << get_timestamp() << function << emulator.subsystem 
              << " ERROR: unrecognized argument DAEMON=" << daemon_in << ", expected { yes | no }\n";
    emulator.exit_cleanly();
  }

  // check for "-d" command line option last so that the command line
  // can override the config file to start as daemon
  //
  if ( cmdOptionExists( argv, argv+argc, "-d" ) ) {
    start_daemon = true;
  }

  if ( start_daemon ) {
    std::cerr << get_timestamp() << function << "starting emulator daemon for " << emulator.subsystem << "\n";
    std::string name = "emulatord." + emulator.subsystem;
    Daemon::daemonize( name, "/tmp", "", "", "" );
  }

  std::cerr << get_timestamp() << function << emulator.subsystem << " " 
            << emulator.config.n_entries << " lines read from " << emulator.config.filename << "\n";

  ret = emulator.configure_emulator();    // get needed values out of read-in configuration file for the daemon

  if (ret != NO_ERROR) {
    std::cerr << get_timestamp() << function << "ERROR: unable to configure emulator for " << emulator.subsystem << "\n";
    emulator.exit_cleanly();
  }

  // create a thread for each listening port
  // The TcpSocket object is instantiated with (PORT#, BLOCKING_STATE, POLL_TIMEOUT_MSEC, THREAD_ID#)
  //
  for ( auto nps : emulator.interface.nps_info ) {
    std::cerr << get_timestamp() << function << emulator.subsystem << " "
              << "configured nps" << nps.first << " on port " << nps.second.port << "\n";
    Network::TcpSocket s( nps.second.port, true, -1, nps.first );  // instantiate TcpSocket object
    if ( s.Listen() < 0 ) {                                        // create listening socket
      std::cerr << get_timestamp() << function << "ERROR: cannot create listening socket on port " << emulator.port << "\n";
      emulator.exit_cleanly();
    }
    std::thread( block_main, s ).detach();                         // spawn thread to handle requests
  }

  for (;;) pause();                                                // main thread suspends
  return 0;
}
/** main *********************************************************************/


/***** block_main ************************************************************/
/**
 * @fn         block_main
 * @brief      main function for blocking connection thread
 * @param[in]  Network::TcpSocket sock, socket object
 * @return     nothing
 *
 * accepts a socket connection and processes the request by
 * calling function doit()
 *
 * This thread never terminates.
 *
 */
void block_main( Network::TcpSocket sock ) {
  while(1) {
    int fd = sock.Accept();
    std::cerr << get_timestamp() << "  (PowerEmulator::block_main) Accept returns connection on port "
                                 << sock.getport() << " fd " << fd << "\n";
    doit( sock );                  // call function to do the work
    sock.Close();
  }
  return;
}
/***** block_main ************************************************************/


/***** doit ******************************************************************/
/**
 * @fn         doit
 * @brief      the workhorse of each thread connetion
 * @param[in]  int thr
 * @return     nothing
 *
 * stays open until closed by client
 *
 * commands come in the form: 
 * <device> [all|<app>] [_BLOCK_] <command> [<arg>]
 *
 */
void doit( Network::TcpSocket sock ) {
  std::string function = "  (PowerEmulator::doit) ";
  long  ret;
  std::stringstream message;
  std::string cmd, args;        // arg string is everything after command
  std::vector<std::string> tokens;
  char delim = '\r';           /// commands sent to me have been terminated with this
  std::string term = "\n";     /// my replies get terminated with this

  bool connection_open=true;

  std::cerr << get_timestamp() << function << "nps" << sock.id << ": accepted connection on port "
            << sock.getport() << " fd " << sock.getfd() << "\n";
  while (connection_open) {

    // Wait (poll) connected socket for incoming data...
    //
    int pollret;
    if ( ( pollret=sock.Poll() ) <= 0 ) {
      if (pollret==0) {
        std::cerr << get_timestamp() << function << emulator.subsystem << " nps" << sock.id << ": Poll timeout on port "
                  << sock.getport() << " fd " << sock.getfd() << " thread " << sock.id << "\n";
      }
      if (pollret <0) {
        std::cerr << get_timestamp() << function << emulator.subsystem << " nps" << sock.id << ": Poll error on port "
                  << sock.getport() << " fd " << sock.getfd() << " thread " << sock.id << ": " << strerror(errno) << "\n";
      }
      break;                      // this will close the connection
    }

    // Data available, now read from connected socket...
    //
    std::string sbuf;
    if ( (ret=sock.Read( sbuf, delim )) <= 0 ) {     // read until newline delimiter
      if (ret<0) {                // could be an actual read error
        std::cerr << get_timestamp() << function << emulator.subsystem 
                  << " nps" << sock.id << ": Read error on port " << sock.getport() << " fd " 
                  << sock.getfd() << ": " << strerror(errno) << "\n";
      }
      if (ret==-2) { std::cerr << get_timestamp() << function << emulator.subsystem 
                               << " nps" << sock.id << ": timeout reading from port " << sock.getport() 
                               << " fd " << sock.getfd() << "\n"; }
      break;                      // Breaking out of the while loop will close the connection.
                                  // This probably means that the client has terminated abruptly, 
                                  // having sent FIN but not stuck around long enough
                                  // to accept CLOSE and give the LAST_ACK.
    }

    // convert the input buffer into a string and remove any trailing linefeed
    // and carriage return
    //
    sbuf.erase(std::remove(sbuf.begin(), sbuf.end(), '\r' ), sbuf.end());
    sbuf.erase(std::remove(sbuf.begin(), sbuf.end(), '\n' ), sbuf.end());

    try {
      std::size_t cmd_sep = sbuf.find_first_of(" "); // find the first space, which separates command from argument list

      cmd = sbuf.substr(0, cmd_sep);                 // cmd is everything up until that space

      if (cmd.empty()) continue;                     // If no command then skip over everything.

      if (cmd_sep == std::string::npos) {            // If no space was found,
        args="";                                     // then the arg list is empty,
      }
      else {
        args= sbuf.substr(cmd_sep+1);                // otherwise args is everything after that space.
      }

      ++emulator.cmd_num;
      if ( emulator.cmd_num == INT_MAX ) emulator.cmd_num = 0;

      std::cerr << get_timestamp() << function << emulator.subsystem << " nps" << sock.id 
                << ": received command (" << emulator.cmd_num << ") on port " << sock.getport()
                << " fd " << sock.getfd() << " (" << sock.id << "): " << cmd << " " << args << "\n";
    }
    catch ( std::runtime_error &e ) {
      std::stringstream errstream; errstream << e.what();
      std::cerr << get_timestamp() << function << emulator.subsystem 
                << " nps" << sock.id << ": error parsing arguments: " << errstream.str() << "\n";
      ret = -1;
    }
    catch ( ... ) {
      std::cerr << get_timestamp() << function << emulator.subsystem << " nps" << sock.id
                << ": unknown error parsing arguments: " << args << "\n";
      ret = -1;
    }

    // process commands here
    //
    ret = NOTHING;
    std::string retstring="";   // string for the return value

    if ( cmd.compare( "exit" ) == 0 ) {
                    emulator.exit_cleanly();                                   // shutdown the daemon
    }
    else
    if ( cmd.compare( "close" ) == 0 ) {
                    sock.Close();
                    return;
    }

    else {  // if no matching command found then send it straight to the interface's command parser
      try {
        std::transform( sbuf.begin(), sbuf.end(), sbuf.begin(), ::toupper );   // make uppercase
      }
      catch (...) {
        std::cerr << get_timestamp() << function << "nps" << sock.id << ": error converting " << sbuf << " to uppercase\n";
        ret=ERROR;
      }
      ret = emulator.interface.parse_command( sock.id, sbuf, retstring );      // send the command to the parser
    }

#ifdef LOGLEVEL_DEBUG
//  std::cerr << get_timestamp() << function << "[DEBUG] nps" << sock.id << ": ret=" << ret << " retstring=" << retstring << "\n";
    std::string debug_reply = retstring;
    debug_reply= std::regex_replace( debug_reply, std::regex("\\r"), "\\r" );
    debug_reply= std::regex_replace( debug_reply, std::regex("\\n"), "\\n" );
    std::cerr << get_timestamp() << function << "[DEBUG] nps" << sock.id << ": ret=" << ret << " retstring=" << debug_reply << "\n";
#endif

//  if ( ret != NOTHING && !retstring.empty() ) {
      retstring.append( term );          // terminate my reply
      if ( sock.Write( retstring ) <0 ) connection_open=false;
//  }

    if (!sock.isblocking()) break;       // Non-blocking connection exits immediately.
                                         // Keep blocking connection open for interactive session.
  }

  connection_open = false;
  sock.Close();
  std::cerr << get_timestamp() << function << "nps" << sock.id << ": connection closed\n";
  return;
}
/***** doit ******************************************************************/

