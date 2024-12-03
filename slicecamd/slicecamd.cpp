/**
 * @file    slicecamd.cpp
 * @brief   this is the main slicecam daemon
 * @details the slicecam daemon is for the Acquisition (and guide) CAMera system
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 * This file has the required global function main()
 *
 */

#include "slicecamd.h"

/***** main *******************************************************************/
/**
 * @brief      the main function
 * @param[in]  int argc
 * @param[in]  char** argv
 * @return     0
 *
 */
int main(int argc, char **argv) {
  std::string function = "Slicecam::main";
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
  std::string daemon_stderr="/tmp/"+Slicecam::DAEMON_NAME+".stderr";  // where daemon sends stderr

  // daemonize, but don't close all file descriptors, required for the Andor camera
  //
  if ( start_daemon ) {
    logwrite( function, "starting daemon" );
    Daemon::daemonize( Slicecam::DAEMON_NAME, "/tmp", daemon_stdout, daemon_stderr, "", false );
  }

  logwrite( function, "daemonized. child process running" );

  // Now the child process instantiates a Server object
  //
  Slicecam::Server slicecamd;

  // Python should not be started by the parent. Initialize the Python objects
  // only by the child process, after daemonizing.
  //
  PyEval_SaveThread();

  std::string logpath;
  long ret=NO_ERROR;

  // check for "-f <filename>" command line option to specify config file
  //
  if ( cmdOptionExists( argv, argv+argc, "-f" ) ) {
    char* filename = getCmdOption( argv, argv+argc, "-f" );
    if ( filename ) {
      slicecamd.config.filename = std::string( filename );
    }
  }
  else

  // if no "-f <filename>" then as long as there's at least one arg,
  // assume that is the config file name.
  //
  if (argc>1) {
    slicecamd.config.filename = std::string( argv[1] );
  }
  else {
    logwrite(function, "ERROR: no configuration file specified");
    slicecamd.exit_cleanly();
  }

  if ( slicecamd.config.read_config(slicecamd.config) != NO_ERROR) {          // read configuration file specified on command line
    logwrite(function, "ERROR: unable to configure system");
    slicecamd.exit_cleanly();
  }

  for (int entry=0; entry < slicecamd.config.n_entries; entry++) {

    std::string configkey;          // the current configuration key read from file
    std::string configval;          // the current configuration value read from file

    try {
      configkey = slicecamd.config.param.at(entry);
      configval = slicecamd.config.arg.at(entry);
    }
    catch (std::out_of_range &) {   // should be impossible
      message.str(""); message << "ERROR: entry " << entry 
                               << " out of range in config parameters (" << slicecamd.config.n_entries << ")";
      logwrite( function, message.str() );
      return(ERROR);
    }

    if ( configkey == "LOGPATH") logpath = configval;

    if (slicecamd.config.param[entry] == "TM_ZONE") {
      if ( slicecamd.config.arg[entry] != "UTC" && slicecamd.config.arg[entry] != "local" ) {
        message.str(""); message << "ERROR invalid TM_ZONE=" << slicecamd.config.arg[entry] << ": expected UTC|local";
        logwrite( function, message.str() );
        slicecamd.exit_cleanly();
      }
      tmzone_cfg = slicecamd.config.arg[entry];
      message.str(""); message << "config:" << slicecamd.config.param[entry] << "=" << slicecamd.config.arg[entry];
      logwrite( function, message.str() );
      slicecamd.interface.async.enqueue( message.str() );
    }

  }

  if (logpath.empty()) {
    logwrite(function, "ERROR: LOGPATH not specified in configuration file");
    slicecamd.exit_cleanly();
  }

  if ( ( init_log( logpath, Slicecam::DAEMON_NAME ) != 0 ) ) {           // initialize the logging system
    logwrite(function, "ERROR: unable to initialize logging system");
    slicecamd.exit_cleanly();
  }

  message.str(""); message << "this version built " << BUILD_DATE << " " << BUILD_TIME;
  logwrite(function, message.str());

  message.str(""); message << slicecamd.config.n_entries << " lines read from " << slicecamd.config.filename;
  logwrite(function, message.str());

  if (ret==NO_ERROR) ret=slicecamd.configure_slicecamd();  // get needed values out of read-in configuration file for the daemon

  if (ret==NO_ERROR) ret=slicecamd.interface.configure_interface( slicecamd.config );

  if (ret != NO_ERROR) {
    logwrite(function, "ERROR: unable to configure system");
    slicecamd.exit_cleanly();
  }

  if (slicecamd.nbport == -1 || slicecamd.blkport == -1) {
    logwrite(function, "ERROR: slicecamd ports not configured");
    slicecamd.exit_cleanly();
  }

  // initialize the pub/sub handler, which
  // takes a list of subscription topics
  //
  if ( slicecamd.interface.init_pubsub({"slitd"}) == ERROR ) {
    logwrite(function, "ERROR initializing publisher-subscriber handler");
    slicecamd.exit_cleanly();
  }

  std::this_thread::sleep_for( std::chrono::milliseconds(100) );
  slicecamd.interface.publish_snapshot();

  std::this_thread::sleep_for( std::chrono::milliseconds(100) );
  slicecamd.interface.request_snapshot();

  // This will pre-thread N_THREADS threads.
  // The 0th thread is reserved for the blocking port, and the rest are for the non-blocking port.
  // Each thread gets a socket object. All of the socket objects are stored in a vector container.
  // The blocking thread socket object is of course unique.
  // For the non-blocking thread socket objects, create a listening socket with one object,
  // then the remaining objects are copies of the first.
  //
  // TcpSocket objects are instantiated with (PORT#, BLOCKING_STATE, POLL_TIMEOUT_MSEC, THREAD_ID#)
  //
  std::vector<Network::TcpSocket> socklist;              // create a vector container to hold N_THREADS TcpSocket objects
  socklist.reserve(Slicecam::N_THREADS);

  Network::TcpSocket s(slicecamd.blkport, true, -1, 0);  // instantiate TcpSocket object with blocking port
  if ( s.Listen() < 0 ) {                                // create a listening socket
    logwrite( function, "ERROR could not create listening socket" );
    slicecamd.exit_cleanly();
  }
  socklist.push_back(s);                                 // add it to the socklist vector
  std::thread( std::ref(Slicecam::Server::block_main),
               std::ref(slicecamd),
               std::ref(socklist[0]) ).detach();         // spawn a thread to handle requests on this socket

  // pre-thread N_THREADS-1 detached threads to handle requests on the non-blocking port
  // thread #0 is reserved for the blocking port (above)
  //
  for (int i=1; i<Slicecam::N_THREADS; i++) {            // create N_THREADS-1 non-blocking socket objects
    if (i==1) {                                          // first one only
      Network::TcpSocket s(slicecamd.nbport, false, CONN_TIMEOUT, i);   // TcpSocket object, non-blocking port, CONN_TIMEOUT timeout
      if ( s.Listen() < 0 ) {                            // create a listening socket
        logwrite( function, "ERROR could not create listening socket" );
        slicecamd.exit_cleanly();
      }
      socklist.push_back(s);
    }
    else {                                           // subsequent socket objects are copies of the first
      Network::TcpSocket s = socklist[1];            // copy the first one, which has a valid listening socket
      s.id = i;
      socklist.push_back(s);
    }
    std::thread( std::ref(Slicecam::Server::thread_main),
                 std::ref(slicecamd),
                 std::ref(socklist[i]) ).detach();  // spawn a thread to handle each non-blocking socket request
  }

  // Instantiate a multicast UDP object and spawn a thread to send asynchronous messages
  //
  Network::UdpSocket async(slicecamd.asyncport, slicecamd.asyncgroup);
  std::thread( std::ref(Slicecam::Server::async_main),
               std::ref(slicecamd),
               async ).detach();

  // thread to start a new logbook each day
  //
  std::thread( std::ref(Slicecam::Server::new_log_day), logpath ).detach();

  for (;;) pause();                                  // main thread suspends

  return 0;
}
/***** main *******************************************************************/
