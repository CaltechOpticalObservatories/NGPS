/**
 * @file    thermald.cpp
 * @brief   this is the main thermal daemon for communicating with temperature controllers
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "thermald.h"


/***** main *******************************************************************/
/**
 * @brief      the main function
 * @param[in]  argc  argument count
 * @param[in]  argv  character array that holds all arguments passed to the “main()” function through the command line
 * @return     0
 *
 */
int main(int argc, char **argv) {
  std::string function = "Thermal::main";
  std::stringstream message;
  bool start_daemon = true;

  // Allow running in the foreground
  //
  if ( cmdOptionExists( argv, argv+argc, "--foreground" ) ) {
    start_daemon = false;
  }

  // TODO make configurable
  //
  std::string daemon_stdout="/dev/null";                             // where daemon sends stdout
  std::string daemon_stderr="/tmp/"+Thermal::DAEMON_NAME+".stderr";  // where daemon sends stderr

  if ( start_daemon ) {
    logwrite( function, "starting daemon" );
    Daemon::daemonize( Thermal::DAEMON_NAME, "/tmp", daemon_stdout, daemon_stderr, "" );
  }

  logwrite( function, "daemonized. child process running" );

  std::string logpath;
  long ret=NO_ERROR;

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
      thermald.config.filename = std::string( filename );
    }
  }
  else

  // if no "-f <filename>" then as long as there's at least one arg,
  // assume that is the config file name.
  //
  if (argc>1) {
    thermald.config.filename = std::string( argv[1] );
  }
  else {
    logwrite(function, "ERROR: no configuration file specified");
    thermald.exit_cleanly();
  }

  if ( thermald.config.read_config(thermald.config) != NO_ERROR) {    // read configuration file specified on command line
    logwrite(function, "ERROR: unable to configure system");
    thermald.exit_cleanly();
  }

  for (int entry=0; entry < thermald.config.n_entries; entry++) {
    if (thermald.config.param[entry] == "LOGPATH") logpath = thermald.config.arg[entry];

    if (thermald.config.param[entry] == "TM_ZONE") {
      if ( thermald.config.arg[entry] != "UTC" && thermald.config.arg[entry] != "local" ) {
        message.str(""); message << "ERROR invalid TM_ZONE=" << thermald.config.arg[entry] << ": expected UTC|local";
        logwrite( function, message.str() );
        thermald.exit_cleanly();
      }
      tmzone_cfg = thermald.config.arg[entry];
      message.str(""); message << "config:" << thermald.config.param[entry] << "=" << thermald.config.arg[entry];
      logwrite( function, message.str() );
    }

  }

  if (logpath.empty()) {
    logwrite(function, "ERROR: LOGPATH not specified in configuration file");
    thermald.exit_cleanly();
  }

  if ( ( init_log( logpath, Thermal::DAEMON_NAME ) != 0 ) ) {       // initialize the logging system
    logwrite(function, "ERROR: unable to initialize logging system");
    thermald.exit_cleanly();
  }

  message << "this version built " << BUILD_DATE << " " << BUILD_TIME;
  logwrite(function, message.str());

  message.str(""); message << thermald.config.n_entries << " lines read from " << thermald.config.filename;
  logwrite(function, message.str());

  if (ret==NO_ERROR) ret=thermald.configure_thermald();   // get values from config file for the daemon
  if (ret==NO_ERROR) ret=thermald.configure_telemetry();  // get values from config file for telemetry
  if (ret==NO_ERROR) ret=thermald.configure_devices();    // get values from config file for Campbell and Lakeshores

  if (ret==NO_ERROR) thermald.interface.open_lakeshores();  // open connection to all configured Lakeshore devices (allowed to fail)
  if (ret==NO_ERROR) thermald.interface.open_campbell();    // open connection to Campbell CR1000 (allowed to fail)

  if (ret != NO_ERROR) {
    logwrite(function, "ERROR: unable to configure system");
    thermald.exit_cleanly();
  }

  if (thermald.nbport == -1 || thermald.blkport == -1) {
    logwrite(function, "ERROR: thermald ports not configured");
    thermald.exit_cleanly();
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

  Network::TcpSocket s(thermald.blkport, true, -1, 0);  // instantiate TcpSocket object with blocking port
  if ( s.Listen() < 0 ) {                            // create a listening socket
    message.str(""); message << "ERROR: cannot create listening socket on port " << thermald.blkport;
    logwrite( function, message.str() );
    thermald.exit_cleanly();
  }
  socklist.push_back(s);                             // add it to the socklist vector
  std::thread( std::ref(Thermal::Server::block_main),
               std::ref(thermald),
               std::ref(socklist[0]) ).detach();     // spawn a thread to handle requests on this socket

  // pre-thread N_THREADS-1 detached threads to handle requests on the non-blocking port
  // thread #0 is reserved for the blocking port (above)
  //
  for (int i=1; i<N_THREADS; i++) {                  // create N_THREADS-1 non-blocking socket objects
    if (i==1) {                                      // first one only
      Network::TcpSocket s(thermald.nbport, false, CONN_TIMEOUT, i);   // instantiate TcpSocket object, non-blocking port, CONN_TIMEOUT timeout
      if ( s.Listen() < 0 ) {                        // create a listening socket
        message.str(""); message << "ERROR: cannot create listening socket on port " << thermald.nbport;
        logwrite( function, message.str() );
        thermald.exit_cleanly();
      }
      socklist.push_back(s);
    }
    else {                                           // subsequent socket objects are copies of the first
      Network::TcpSocket s = socklist[1];            // copy the first one, which has a valid listening socket
      s.id = i;
      socklist.push_back(s);
    }
    std::thread( std::ref(Thermal::Server::thread_main),
                 std::ref(thermald),
                 std::ref(socklist[i]) ).detach();  // spawn a thread to handle each non-blocking socket request
  }

  // Instantiate a multicast UDP object and spawn a thread to send asynchronous messages
  //
  Network::UdpSocket async(thermald.asyncport, thermald.asyncgroup);
  std::thread( std::ref(Thermal::Server::async_main),
               std::ref(thermald),
               async ).detach();

  // thread to start a new logbook each day
  //
  std::thread( std::ref(Thermal::Server::new_log_day), logpath ).detach();

  // start the telemetry watchdog thread
  // the watchdog will in turn start the telemetry thread
  //
  std::thread( std::ref(thermald.telemetry_watchdog), std::ref(thermald) ).detach();

  logwrite( function, "daemon is running" );

  for (;;) pause();                                  // main thread suspends
  return 0;
}
/***** main *******************************************************************/


/***** signal_handler *********************************************************/
/**
 * @brief      handles ctrl-C
 * @param[in]  signo
 *
 */
void signal_handler(int signo) {
  std::string function = "Thermal::signal_handler";
  std::stringstream message;

  switch (signo) {
    case SIGTERM:
    case SIGINT:
      logwrite(function, "received termination signal");
      message << "NOTICE:" << Thermal::DAEMON_NAME << " exit";
      thermald.interface.async.enqueue( message.str() );
      thermald.exit_cleanly();                   // shutdown the daemon
      break;
    case SIGHUP:
      logwrite(function, "caught SIGHUP");
      thermald.configure_telemetry();            // read and apply telemetry configuration
      thermald.configure_devices();              // read and apply device configuration
      break;
    case SIGPIPE:
      logwrite(function, "caught SIGPIPE");
      break;
    default:
      message << "received unknown signal " << strsignal(signo);
      logwrite( function, message.str() );
      message.str(""); message << "NOTICE:" << Thermal::DAEMON_NAME << " exit";
      thermald.interface.async.enqueue( message.str() );
      thermald.exit_cleanly();                   // shutdown the daemon
      break;
  }
  return;
}
/***** signal_handler *********************************************************/
