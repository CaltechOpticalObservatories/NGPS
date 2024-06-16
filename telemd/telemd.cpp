/**
 * @file    telemd.cpp
 * @brief   this is the main telemetryu daemon
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 * This file has the required global function main()
 *
 */

#include "telemd.h"

/***** main *******************************************************************/
/**
 * @brief      the main function
 * @param[in]  int argc
 * @param[in]  char** argv
 * @return     0
 *
 */
int main(int argc, char **argv) {
  std::string function = "Telemetry::main";
  std::stringstream message;
  std::string logpath; 
  long ret=NO_ERROR;
  std::string daemon_in;                  // daemon setting read from config file
  std::string daemon_stdout="/dev/null";  // where daemon sends stdout
  std::string daemon_stderr="/dev/null";  // where daemon sends stderr
  bool start_daemon = false;              // don't start as daemon unless specifically requested

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
      telemd.config.filename = std::string( filename );
    }
  }
  else

  // if no "-f <filename>" then as long as there's at least one arg,
  // assume that is the config file name.
  //
  if (argc>1) {
    telemd.config.filename = std::string( argv[1] );
  }
  else {
    logwrite(function, "ERROR: no configuration file specified");
    telemd.exit_cleanly();
  }

  if ( telemd.config.read_config(telemd.config) != NO_ERROR) {        // read configuration file specified on command line
    logwrite(function, "ERROR: unable to configure system");
    telemd.exit_cleanly();
  }

  for (int entry=0; entry < telemd.config.n_entries; entry++) {

    std::string configkey;          // the current configuration key read from file
    std::string configval;          // the current configuration value read from file

    try {
      configkey = telemd.config.param.at(entry);
      configval = telemd.config.arg.at(entry);
    }
    catch (std::out_of_range &) {   // should be impossible
      message.str(""); message << "ERROR: entry " << entry 
                               << " out of range in config parameters (" << telemd.config.n_entries << ")";
      logwrite( function, message.str() );
      return(ERROR);
    }

    if ( configkey == "LOGPATH") logpath = configval;
    if ( configkey == "DAEMON")  daemon_in = configval;
    if ( configkey == "STDOUT")  daemon_stdout = configval;
    if ( configkey == "STDERR")  daemon_stderr = configval;

    if (telemd.config.param[entry] == "TM_ZONE") {
      if ( telemd.config.arg[entry] != "UTC" && telemd.config.arg[entry] != "local" ) {
        message.str(""); message << "ERROR invalid TM_ZONE=" << telemd.config.arg[entry] << ": expected UTC|local";
        logwrite( function, message.str() );
        telemd.exit_cleanly();
      }
      tmzone_cfg = telemd.config.arg[entry];
      message.str(""); message << "config:" << telemd.config.param[entry] << "=" << telemd.config.arg[entry];
      logwrite( function, message.str() );
    }

  }
  if (logpath.empty()) {
    logwrite(function, "ERROR: LOGPATH not specified in configuration file");
    telemd.exit_cleanly();
  }

  if ( !daemon_in.empty() && daemon_in == "yes" ) start_daemon = true;
  else
  if ( !daemon_in.empty() && daemon_in == "no"  ) start_daemon = false;
  else {
    message.str(""); message << "ERROR: unrecognized argument DAEMON=" << daemon_in << ", expected { yes | no }";
    logwrite( function, message.str() );
    telemd.exit_cleanly();
  }

  // check for "-d" command line option last so that the command line
  // can override the config file to start as daemon
  //
  if ( cmdOptionExists( argv, argv+argc, "-d" ) ) {
    start_daemon = true;
  }

  if ( start_daemon ) {
    logwrite( function, "starting daemon" );
    Daemon::daemonize( Telemetry::DAEMON_NAME, "/tmp", daemon_stdout, daemon_stderr, "" );
  }

  if ( ( init_log( logpath, Telemetry::DAEMON_NAME ) != 0 ) ) {      // initialize the logging system
    logwrite(function, "ERROR: unable to initialize logging system");
    telemd.exit_cleanly();
  }

  message << "this version built " << BUILD_DATE << " " << BUILD_TIME;
  logwrite(function, message.str());

  message.str(""); message << telemd.config.n_entries << " lines read from " << telemd.config.filename;
  logwrite(function, message.str());

  if (ret==NO_ERROR) ret=telemd.configure_telemd();        // get needed values out of read-in configuration file for the daemon

  if (ret==NO_ERROR) ret=telemd.interface.configure_interface( telemd.config );

  if (ret != NO_ERROR) {
    logwrite(function, "ERROR: unable to configure system");
    telemd.exit_cleanly();
  }

  if (telemd.nbport == -1 || telemd.blkport == -1) {
    logwrite(function, "ERROR: telemd ports not configured");
    telemd.exit_cleanly();
  }

  // This will pre-thread N_THREADS threads.
  // The 0th thread is reserved for the blocking port, and the rest are for the non-blocking port.
  // Each thread gets a socket object. All of the socket objects are stored in a vector container.
  // The blocking thread socket object is of course unique.
  // For the non-blocking thread socket objects, create a listening socket with one object,
  // then the remaining objects are copies of the first.
  //
  // TcpSocket objects are instantiated with (PORT#, BLOCKING_STATE, POLL_TIMEOUT_MSEC, THREAD_ID#)
  //
  std::vector<Network::TcpSocket> socklist;          // create a vector container to hold N_THREADS TcpSocket objects
  socklist.reserve(N_THREADS);

  Network::TcpSocket s(telemd.blkport, true, -1, 0); // instantiate TcpSocket object with blocking port
  s.Listen();                                        // create a listening socket
  socklist.push_back(s);                             // add it to the socklist vector
  std::thread( std::ref(Telemetry::Server::block_main),
               std::ref(telemd),
               std::ref(socklist[0]) ).detach();     // spawn a thread to handle requests on this socket

  // pre-thread N_THREADS-1 detached threads to handle requests on the non-blocking port
  // thread #0 is reserved for the blocking port (above)
  //
  for (int i=1; i<N_THREADS; i++) {                  // create N_THREADS-1 non-blocking socket objects
    if (i==1) {                                      // first one only
      Network::TcpSocket s(telemd.nbport, false, CONN_TIMEOUT, i);  // TcpSocket object, non-blocking port, CONN_TIMEOUT timeout
      s.Listen();                                    // create a listening socket
      socklist.push_back(s);
    }
    else {                                           // subsequent socket objects are copies of the first
      Network::TcpSocket s = socklist[1];            // copy the first one, which has a valid listening socket
      s.id = i;
      socklist.push_back(s);
    }
    std::thread( std::ref(Telemetry::Server::thread_main),
                 std::ref(telemd),
                 socklist[i] ).detach();  // spawn a thread to handle each non-blocking socket request
  }

  // Instantiate a multicast UDP object and spawn a thread to send asynchronous messages
  //
  Network::UdpSocket async(telemd.asyncport, telemd.asyncgroup);
  std::thread( std::ref(Telemetry::Server::async_main),
               std::ref(telemd),
               async ).detach();

  // thread to start a new logbook each day
  //
  std::thread( std::ref(Telemetry::Server::new_log_day), logpath ).detach();

  for (;;) pause();                                  // main thread suspends
  return 0;
}
/***** main *******************************************************************/


/***** signal_handler *********************************************************/
/**
 * @brief      handles ctrl-C
 * @param[in]  int signo
 *
 */
void signal_handler(int signo) {
  std::string function = "Telemetry::signal_handler";
  std::stringstream message;

  switch (signo) {
    case SIGTERM:
    case SIGINT:
      logwrite(function, "received termination signal");
      message << "NOTICE:" << Telemetry::DAEMON_NAME << " exit";
      telemd.interface.async.enqueue( message.str() );
      telemd.exit_cleanly();                     // shutdown the daemon
      break;
    case SIGHUP:
      telemd.interface.configure_interface( telemd.config );
      break;
    case SIGPIPE:
      logwrite(function, "ignored SIGPIPE");
      break;
    default:
      message << "received unknown signal " << strsignal(signo);
      logwrite( function, message.str() );
      message.str(""); message << "NOTICE:" << Telemetry::DAEMON_NAME << " exit";
      telemd.interface.async.enqueue( message.str() );
      break;
  }
  return;
}
/***** signal_handler *********************************************************/

