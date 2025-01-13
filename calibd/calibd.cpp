/**
 * @file    calibd.cpp
 * @brief   this is the main calib daemon for communicating with the calibrator system
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "calibd.h"


/***** main *******************************************************************/
/**
 * @brief      the main function
 * @param[in]  argc  argument count
 * @param[in]  argv  character array that holds all arguments passed to the “main()” function through the command line
 * @return     0
 *
 */
int main(int argc, char **argv) {
  std::string function = "Calib::main";
  std::stringstream message;
  bool start_daemon = true;

  // Allow running in the foreground
  //
  if ( cmdOptionExists( argv, argv+argc, "--foreground" ) ) {
    start_daemon = false;
  }

  // TODO make configurable
  //
  std::string daemon_stdout="/dev/null";                              // where daemon sends stdout
  std::string daemon_stderr="/tmp/"+Calib::DAEMON_NAME+".stderr";     // where daemon sends stderr

  // daemonize, but don't close all file descriptors, required for the Andor camera
  //
  if ( start_daemon ) {
    logwrite( function, "starting daemon" );
    Daemon::daemonize( Calib::DAEMON_NAME, "/tmp", daemon_stdout, daemon_stderr, "", false );
  }

  logwrite( function, "daemonized. child process running" );

  // Now the child process instantiates a Server object
  //
  Calib::Server calibd;

  std::string logpath;
  long ret=NO_ERROR;

  // check for "-f <filename>" command line option to specify config file
  //
  if ( cmdOptionExists( argv, argv+argc, "-f" ) ) {
    char* filename = getCmdOption( argv, argv+argc, "-f" );
    if ( filename ) {
      calibd.config.filename = std::string( filename );
    }
  }
  else

  // if no "-f <filename>" then as long as there's at least one arg,
  // assume that is the config file name.
  //
  if (argc>1) {
    calibd.config.filename = std::string( argv[1] );
  }
  else {
    logwrite(function, "ERROR: no configuration file specified");
    calibd.exit_cleanly();
  }

  if ( calibd.config.read_config(calibd.config) != NO_ERROR) {        // read configuration file specified on command line
    logwrite(function, "ERROR: unable to configure system");
    calibd.exit_cleanly();
  }

  for (int entry=0; entry < calibd.config.n_entries; entry++) {
    if (calibd.config.param[entry] == "LOGPATH") logpath = calibd.config.arg[entry];

    if (calibd.config.param[entry] == "TM_ZONE") {
      if ( calibd.config.arg[entry] != "UTC" && calibd.config.arg[entry] != "local" ) {
        message.str(""); message << "ERROR invalid TM_ZONE=" << calibd.config.arg[entry] << ": expected UTC|local";
        logwrite( function, message.str() );
        calibd.exit_cleanly();
      }
      tmzone_cfg = calibd.config.arg[entry];
      message.str(""); message << "config:" << calibd.config.param[entry] << "=" << calibd.config.arg[entry];
      logwrite( function, message.str() );
    }

  }

  if (logpath.empty()) {
    logwrite(function, "ERROR: LOGPATH not specified in configuration file");
    calibd.exit_cleanly();
  }

  if ( ( init_log( logpath, Calib::DAEMON_NAME ) != 0 ) ) {          // initialize the logging system
    logwrite(function, "ERROR: unable to initialize logging system");
    calibd.exit_cleanly();
  }

  message << "this version built " << BUILD_DATE << " " << BUILD_TIME;
  logwrite(function, message.str());

  message.str(""); message << calibd.config.n_entries << " lines read from " << calibd.config.filename;
  logwrite(function, message.str());

  if (ret==NO_ERROR) ret=calibd.configure_calibd();        // get needed values out of read-in configuration file for the daemon

  if (ret != NO_ERROR) {
    logwrite(function, "ERROR: unable to configure system");
    calibd.exit_cleanly();
  }

  if (calibd.nbport == -1 || calibd.blkport == -1) {
    logwrite(function, "ERROR: calibd ports not configured");
    calibd.exit_cleanly();
  }

  // initialize the pub/sub handler and give it time to start
  //
  if ( calibd.interface.init_pubsub() == ERROR ) {
    logwrite(function, "ERROR initializing publisher-subscriber handler");
    calibd.exit_cleanly();
  }
  std::this_thread::sleep_for( std::chrono::milliseconds(500) );

  // publish snapshot of my telemetry so the world knows I'm online
  calibd.interface.publish_snapshot();

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

  Network::TcpSocket s(calibd.blkport, true, -1, 0); // instantiate TcpSocket object with blocking port
  if ( s.Listen() < 0 ) {                            // create a listening socket
    message.str(""); message << "ERROR could not create listening socket on port " << calibd.blkport;
    logwrite( function, message.str() );
    calibd.exit_cleanly();
  }
  socklist.push_back(s);                             // add it to the socklist vector
  std::thread( std::ref(Calib::Server::block_main),
               std::ref(calibd),
               std::ref(socklist[0]) ).detach();     // spawn a thread to handle requests on this socket

  // pre-thread N_THREADS-1 detached threads to handle requests on the non-blocking port
  // thread #0 is reserved for the blocking port (above)
  //
  for (int i=1; i<N_THREADS; i++) {                  // create N_THREADS-1 non-blocking socket objects
    if (i==1) {                                      // first one only
      Network::TcpSocket s(calibd.nbport, false, CONN_TIMEOUT, i);   // instantiate TcpSocket object, non-blocking port, CONN_TIMEOUT timeout
      if ( s.Listen() < 0 ) {                        // create a listening socket
        message.str(""); message << "ERROR could not create listening socket on port " << calibd.nbport;
        logwrite( function, message.str() );
        calibd.exit_cleanly();
      }
      socklist.push_back(s);
    }
    else {                                           // subsequent socket objects are copies of the first
      Network::TcpSocket s = socklist[1];            // copy the first one, which has a valid listening socket
      s.id = i;
      socklist.push_back(s);
    }
    std::thread( std::ref(Calib::Server::thread_main),
                 std::ref(calibd),
                 std::ref(socklist[i]) ).detach();  // spawn a thread to handle each non-blocking socket request
  }

  // Instantiate a multicast UDP object and spawn a thread to send asynchronous messages
  //
  Network::UdpSocket async(calibd.asyncport, calibd.asyncgroup);
  std::thread( std::ref(Calib::Server::async_main),
               std::ref(calibd),
               async ).detach();

  // thread to start a new logbook each day
  //
  std::thread( std::ref(Calib::Server::new_log_day), logpath ).detach();

  for (;;) pause();                                  // main thread suspends
  return 0;
}
/***** main *******************************************************************/
